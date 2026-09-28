/* `jb:ol` draws a tooltip only when a replay hovers a `js` or a research
 * icon, which the net may never do: these hashes pin its pixels. They were
 * taken on the code where `jb` was still a class instance and must not be
 * re-taken. */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_data_init.h"
#include "../src/pico8_dispatch_context.h"
#include "../src/pico8_entity.h"
#include "../src/pico8_fixed.h"
#include "../src/pico8_gfx.h"
#include "../src/pico8_hud.h"
#include "../src/pico8_layout.h"
#include "../src/pico8_list.h"
#include "../src/pico8_mem.h"
#include "../src/pico8_ob.h"
#include "../src/pico8_struct.h"
#include "../src/pico8_type_tag.h"
#include "../src/pico8_value.h"

#define DR_FIRST_ENTRY_HASH 0x8B2796B5u
#define BARE_STRINGS_HASH 0x2B14E741u
#define NO_OVERRIDES_HASH 0x47B9E645u
#define DR_ALL_BY_NAME_HASH 0x71C88A4Eu
#define MIXED_BY_NAME_HASH 0xC5324FA0u

static pico8_mem_t mem;
static pico8_gfx_t gfx;

static uint32_t screen_hash(void) {
    uint32_t h = 2166136261u;
    int i;

    for (i = 0; i < PICO8_SCREEN_WIDTH * PICO8_SCREEN_HEIGHT; i++) {
        h ^= (uint32_t)pico8_peek(&mem, PICO8_ADDR_SCREEN + i);
        h *= 16777619u;
    }
    return h;
}

static uint32_t draw_jb(const pico8_jb_t *jb, pico8_fixed_t om, pico8_fixed_t a) {
    pico8_gfx_init(&gfx);
    pico8_cls(&mem);
    pico8_jb_ol(&gfx, &mem, jb, om, a);
    return screen_hash();
}

static void check(const char *label, uint32_t got, uint32_t want) {
    uint32_t blank;

    pico8_cls(&mem);
    blank = screen_hash();
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got != blank);
    assert(got == want);
}

/* `js:ev`'s call, on the cart's own first `dr` entry. */
static void test_dr_entry_draws_as_before(void) {
    check("dr[1]", draw_jb(pico8_jb_new(&pico8_dr_table[0]), pico8_fixed_from_int(127), pico8_fixed_from_int(1)),
          DR_FIRST_ENTRY_HASH);
}

/* The research icon's call, on bare strings, each a one-item row. */
static void test_bare_strings_draw_as_before(void) {
    pico8_ob_table_t *spec = pico8_ob_table_new();

    pico8_ob_table_set_index(spec, 1, pico8_value_string("alpha"));
    pico8_ob_table_set_index(spec, 2, pico8_value_string("beta 42"));
    check("bare", draw_jb(pico8_jb_new_table(spec), pico8_fixed_from_int(0), pico8_fixed_from_int(0)),
          BARE_STRINGS_HASH);
    pico8_ob_table_free(spec);
}

/* No line: an empty `ls`, so only the frame. */
static void test_no_overrides_draws_frame_only(void) {
    check("none", draw_jb(pico8_jb_new(NULL), pico8_fixed_from_int(60), pico8_fixed_from_int(0)), NO_OVERRIDES_HASH);
}

/* Both constructions give the struct form, sharing `jb`'s method table,
 * whose `ol` entry draws what pico8_jb_ol() draws. */
static void test_jb_is_a_struct_with_its_method_table(void) {
    const pico8_jb_t *direct = pico8_jb_new(NULL);
    const pico8_jb_t *generic = pico8_jb_new(NULL);

    assert(pico8_type_tag_of(direct) == PICO8_TYPE_TAG_STRUCT);
    assert(pico8_type_tag_of(generic) == PICO8_TYPE_TAG_STRUCT);
    assert(direct->header.cls == generic->header.cls);
    assert(direct->header.cls->methods[PICO8_METHOD_OL] != NULL);
    assert(direct->ls != NULL && pico8_type_tag_of(direct->ls) == PICO8_TYPE_TAG_LIST && direct->ls->count == 0);

    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_gfx_init(&gfx);
    pico8_cls(&mem);
    direct->header.cls->methods[PICO8_METHOD_OL]((void *)pico8_jb_new(&pico8_dr_table[0]), 2,
                                                 pico8_value_fixed(pico8_fixed_from_int(127)),
                                                 pico8_value_fixed(pico8_fixed_from_int(1)));
    check("table ol", screen_hash(), DR_FIRST_ENTRY_HASH);
}

/* Reads `jb` only by name -- `ol` through the method table, then its
 * pixels -- so it holds on either layout of `ls`. */
static uint32_t draw_by_name(pico8_jb_t *jb, pico8_fixed_t om, pico8_fixed_t a) {
    pico8_value_t out;
    uint32_t blank;
    uint32_t got;

    pico8_gfx_init(&gfx);
    pico8_cls(&mem);
    blank = screen_hash();
    assert(pico8_mv(&jb->header, "ol", 2, pico8_value_fixed(om), pico8_value_fixed(a), &out));
    got = screen_hash();
    assert(got != blank);
    return got;
}

static void test_jb_state_by_name(void) {
    pico8_jb_t *jb = pico8_jb_new(NULL);
    const pico8_struct_class_t *cls = jb->header.cls;
    pico8_ob_table_t *mixed = pico8_ob_table_new();
    pico8_ob_table_t *row = pico8_ob_table_new();
    pico8_value_t v;
    uint32_t h = 2166136261u;
    int i;

    assert(pico8_entity_get_field(jb, "ol", &v));
    assert(v.kind == PICO8_VALUE_FUNCTION && v.as.function == cls->methods[PICO8_METHOD_OL]);
    assert(cls->field == NULL && cls->set_field == NULL);

    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    for (i = 1; i <= 7; i++) {
        h ^= draw_by_name(pico8_jb_new(&pico8_dr_table[i - 1]), pico8_fixed_from_int(127), pico8_fixed_from_int(1));
        h *= 16777619u;
    }
    printf("[dr all by name] 0x%08Xu\n", (unsigned)h);
    fflush(stdout);
    assert(h == DR_ALL_BY_NAME_HASH);

    pico8_ob_table_set_index(row, 1, pico8_value_string("gamma"));
    pico8_ob_table_set_index(mixed, 1, pico8_value_string("alpha"));
    pico8_ob_table_set_index(mixed, 2, pico8_value_ref(row));
    pico8_ob_table_set_index(mixed, 3, pico8_value_fixed(pico8_fixed_from_int(5)));
    pico8_ob_table_set_index(mixed, 5, pico8_value_string("delta"));
    h = draw_by_name(pico8_jb_new_table(mixed), pico8_fixed_from_int(0), PICO8_FIXED_LIT(0x00008000, 0.5));
    printf("[mixed by name] 0x%08Xu\n", (unsigned)h);
    fflush(stdout);
    assert(h == MIXED_BY_NAME_HASH);
}

int main(void) {
    pico8_mem_init(&mem);
    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));
    pico8_cu(&mem);
    pico8_data_init(&mem);
    pico8_layout_init();

    test_dr_entry_draws_as_before();
    test_bare_strings_draw_as_before();
    test_no_overrides_draws_frame_only();
    test_jb_is_a_struct_with_its_method_table();
    test_jb_state_by_name();

    printf("test_pico8_jb_ol: all tests passed\n");
    return 0;
}
