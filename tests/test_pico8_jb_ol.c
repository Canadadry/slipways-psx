/* `jb:ol` draws a tooltip only when a replay hovers a `js` or a research
 * icon, which the net may never do: these hashes pin its pixels. They were
 * taken on the code where `jb` was still a class instance and must not be
 * re-taken. */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "../src/pico8_assets.h"
#include "../src/pico8_class.h"
#include "../src/pico8_data_init.h"
#include "../src/pico8_dispatch_context.h"
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

static uint32_t draw_jb(const pico8_ob_table_t *overrides, pico8_fixed_t om, pico8_fixed_t a) {
    pico8_gfx_init(&gfx);
    pico8_cls(&mem);
    pico8_jb_ol(&gfx, &mem, pico8_jb_new(overrides), om, a);
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
    pico8_value_t v;

    assert(pico8_ob_table_get_index(pico8_dr(), 1, &v) && v.kind == PICO8_VALUE_REF);
    check("dr[1]", draw_jb(v.as.ref, pico8_fixed_from_int(127), pico8_fixed_from_int(1)), DR_FIRST_ENTRY_HASH);
}

/* The research icon's call, on bare strings that `jb:pq` wraps as `{ln}`. */
static void test_bare_strings_draw_as_before(void) {
    pico8_ob_table_t *spec = pico8_ob_table_new();

    pico8_ob_table_set_index(spec, 1, pico8_value_string("alpha"));
    pico8_ob_table_set_index(spec, 2, pico8_value_string("beta 42"));
    check("bare", draw_jb(spec, pico8_fixed_from_int(0), pico8_fixed_from_int(0)), BARE_STRINGS_HASH);
    pico8_ob_table_free(spec);
}

/* No overrides: an empty `ls`, so only the frame. */
static void test_no_overrides_draws_frame_only(void) {
    check("none", draw_jb(NULL, pico8_fixed_from_int(60), pico8_fixed_from_int(0)), NO_OVERRIDES_HASH);
}

/* Both constructions give the struct form, sharing `jb`'s method table,
 * whose `ol` entry draws what pico8_jb_ol() draws. */
static void test_jb_is_a_struct_with_its_method_table(void) {
    const pico8_jb_t *direct = pico8_jb_new(NULL);
    const pico8_jb_t *generic = (const pico8_jb_t *)pico8_class_instantiate(pico8_jb_class(), NULL);
    pico8_value_t v;

    assert(pico8_type_tag_of(direct) == PICO8_TYPE_TAG_STRUCT);
    assert(pico8_type_tag_of(generic) == PICO8_TYPE_TAG_STRUCT);
    assert(direct->header.methods == generic->header.methods);
    assert(direct->header.methods[PICO8_METHOD_OL] != NULL);
    assert(direct->ls.kind == PICO8_VALUE_REF && pico8_type_tag_of(direct->ls.as.ref) == PICO8_TYPE_TAG_LIST &&
           ((const pico8_list_t *)direct->ls.as.ref)->count == 0);

    assert(pico8_ob_table_get_index(pico8_dr(), 1, &v) && v.kind == PICO8_VALUE_REF);
    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_gfx_init(&gfx);
    pico8_cls(&mem);
    direct->header.methods[PICO8_METHOD_OL]((void *)pico8_jb_new(v.as.ref), 2,
                                            pico8_value_fixed(pico8_fixed_from_int(127)),
                                            pico8_value_fixed(pico8_fixed_from_int(1)));
    check("table ol", screen_hash(), DR_FIRST_ENTRY_HASH);
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

    printf("test_pico8_jb_ol: all tests passed\n");
    return 0;
}
