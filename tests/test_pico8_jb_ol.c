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
#include "../src/pico8_mem.h"
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
    static const pico8_jb_item_t items[] = {{.s = "alpha"}, {.s = "beta 42"}};
    static const pico8_jb_row_t spec = {.count = 2, .items = items};

    check("bare", draw_jb(pico8_jb_new(&spec), pico8_fixed_from_int(0), pico8_fixed_from_int(0)), BARE_STRINGS_HASH);
}

/* No line: an empty `ls`, so only the frame. */
static void test_no_overrides_draws_frame_only(void) {
    check("none", draw_jb(pico8_jb_new(NULL), pico8_fixed_from_int(60), pico8_fixed_from_int(0)), NO_OVERRIDES_HASH);
}

/* Both constructions give the struct form, sharing `jb`'s class, which
 * has an `ol`; pico8_jb_ol() draws the first `dr` entry. */
static void test_jb_is_a_struct_with_its_method_table(void) {
    const pico8_jb_t *direct = pico8_jb_new(NULL);
    const pico8_jb_t *generic = pico8_jb_new(NULL);

    assert(pico8_type_tag_of(direct) == PICO8_TYPE_TAG_STRUCT);
    assert(pico8_type_tag_of(generic) == PICO8_TYPE_TAG_STRUCT);
    assert(direct->header.cls == generic->header.cls);
    assert(direct->header.cls->ol != NULL);
    assert(direct->ls != NULL && direct->ls_count == 0);

    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    pico8_gfx_init(&gfx);
    pico8_cls(&mem);
    pico8_jb_ol(&gfx, &mem, pico8_jb_new(&pico8_dr_table[0]), pico8_fixed_from_int(127), pico8_fixed_from_int(1));
    check("table ol", screen_hash(), DR_FIRST_ENTRY_HASH);

    /* The class's `ol` gets a vector no launcher passes it: `om` is 0. */
    {
        uint32_t via_slot;

        pico8_cls(&mem);
        direct->header.cls->ol((void *)pico8_jb_new(&pico8_dr_table[0]),
                               pico8_vec_v(pico8_fixed_from_int(127), pico8_fixed_from_int(0)), pico8_fixed_from_int(1));
        via_slot = screen_hash();
        pico8_cls(&mem);
        pico8_jb_ol(&gfx, &mem, pico8_jb_new(&pico8_dr_table[0]), pico8_fixed_from_int(0), pico8_fixed_from_int(1));
        assert(screen_hash() == via_slot);
    }
}

/* Reads `jb` only through pico8_jb_ol(), then its pixels, so it holds on
 * either layout of `ls`. */
static uint32_t draw_by_name(pico8_jb_t *jb, pico8_fixed_t om, pico8_fixed_t a) {
    uint32_t blank;
    uint32_t got;

    pico8_gfx_init(&gfx);
    pico8_cls(&mem);
    blank = screen_hash();
    pico8_jb_ol(&gfx, &mem, jb, om, a);
    got = screen_hash();
    assert(got != blank);
    return got;
}

static void test_jb_state_by_name(void) {
    pico8_jb_t *jb = pico8_jb_new(NULL);
    const pico8_struct_class_t *cls = jb->header.cls;
    static const pico8_jb_item_t row_items[] = {{.s = "gamma"}};
    static const pico8_jb_row_t row = {.count = 1, .items = row_items};
    static const pico8_jb_item_t mixed_items[] = {{.s = "alpha"}, {.row = &row}, {.n = 5}, {.s = "delta"}};
    static const pico8_jb_row_t mixed = {.count = 4, .items = mixed_items};
    uint32_t h = 2166136261u;
    int i;

    assert(cls->ol != NULL);

    pico8_dispatch_set_gfx(&gfx);
    pico8_dispatch_set_mem(&mem);
    for (i = 1; i <= 7; i++) {
        h ^= draw_by_name(pico8_jb_new(&pico8_dr_table[i - 1]), pico8_fixed_from_int(127), pico8_fixed_from_int(1));
        h *= 16777619u;
    }
    printf("[dr all by name] 0x%08Xu\n", (unsigned)h);
    fflush(stdout);
    assert(h == DR_ALL_BY_NAME_HASH);

    h = draw_by_name(pico8_jb_new(&mixed), pico8_fixed_from_int(0), PICO8_FIXED_LIT(0x00008000, 0.5));
    printf("[mixed by name] 0x%08Xu\n", (unsigned)h);
    fflush(stdout);
    assert(h == MIXED_BY_NAME_HASH);
}

int main(void) {
    pico8_mem_init(&mem);
    assert(pico8_load_gfx_json(&mem, "pico8/gfx.json"));
    assert(pico8_load_map_json(&mem, "pico8/map.json"));
    pico8_cu(&mem);
    pico8_data_init();
    pico8_layout_init();

    test_dr_entry_draws_as_before();
    test_bare_strings_draw_as_before();
    test_no_overrides_draws_frame_only();
    test_jb_is_a_struct_with_its_method_table();
    test_jb_state_by_name();

    printf("test_pico8_jb_ol: all tests passed\n");
    return 0;
}
