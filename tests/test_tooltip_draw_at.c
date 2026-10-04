/* tooltip_draw_at() draws only when a replay hovers a button or a structure
 * icon, which the e2e replays may never do: these hashes pin its pixels.
 * They were taken before the tooltip became a plain struct and must not be
 * re-taken. */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "p8/assets.h"
#include "game/data_init.h"
#include "engine/dispatch_context.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "game/layout.h"
#include "p8/vram.h"
#include "engine/entity_class.h"
#include "heap_allocator.h"

#define ICON_TOOLTIPS_FIRST_ENTRY_HASH 0x38233998u
#define BARE_STRINGS_HASH 0x5A3E5741u
#define NO_OVERRIDES_HASH 0xFF43E245u
#define ICON_TOOLTIPS_ALL_BY_NAME_HASH 0xD2ED8589u
#define MIXED_BY_NAME_HASH 0x6109F220u

static p8_vram_t memory;
static p8_graphics_t graphics;

static uint32_t screen_hash(void) {
    uint32_t h = 2166136261u;
    for (int y = 0; y < P8_SCREEN_HEIGHT; y++) {
        for (int x = 0; x < P8_SCREEN_WIDTH; x++) {
            h ^= (uint32_t)memory.screen[y][x];
            h *= 16777619u;
        }
    }
    return h;
}

static uint32_t draw_tooltip(const tooltip_t *jb, p8_fixed_t om, p8_fixed_t a) {
    p8_gfx_init(&graphics);
    p8_cls(&memory);
    tooltip_draw_at(&graphics, &memory, jb, om, a);
    return screen_hash();
}

static void check(const char *label, uint32_t got, uint32_t want) {
    uint32_t blank;

    p8_cls(&memory);
    blank = screen_hash();
    printf("[%s] 0x%08Xu\n", label, (unsigned)got);
    fflush(stdout);
    assert(got != blank);
    assert(got == want);
}

/* A button's call (ui_button.c), on the first entry of icon_tooltips_table. */
static void test_icon_tooltips_entry_draws_as_before(void) {
    check("icon_tooltips_first_entry", draw_tooltip(tooltip_new(&heap_allocator, &icon_tooltips_table[0]), p8_fixed_from_int(127), p8_fixed_from_int(1)),
          ICON_TOOLTIPS_FIRST_ENTRY_HASH);
}

/* A structure icon's call, on bare strings, each a one-item row. */
static void test_bare_strings_draw_as_before(void) {
    static const data_item_t items[] = {{.text = "alpha"}, {.text = "beta 42"}};
    static const data_row_t spec = {.count = 2, .items = items};

    check("bare", draw_tooltip(tooltip_new(&heap_allocator, &spec), p8_fixed_from_int(0), p8_fixed_from_int(0)), BARE_STRINGS_HASH);
}

/* No line: empty `rows`, so only the frame. */
static void test_no_overrides_draws_frame_only(void) {
    check("none", draw_tooltip(tooltip_new(&heap_allocator, NULL), p8_fixed_from_int(60), p8_fixed_from_int(0)), NO_OVERRIDES_HASH);
}

/* Both constructions give the struct form, sharing the tooltip class, which
 * has a draw_at; tooltip_draw_at() draws the first icon_tooltips_table
 * entry. */
static void test_tooltip_is_a_struct_with_its_method_table(void) {
    const tooltip_t *direct = tooltip_new(&heap_allocator, NULL);
    const tooltip_t *generic = tooltip_new(&heap_allocator, NULL);

    assert(direct->header.class == tooltip_class());
    assert(generic->header.class == tooltip_class());
    assert(direct->header.class == generic->header.class);
    assert(direct->header.class->draw_at != NULL);
    assert(direct->rows != NULL && direct->rows_count == 0);

    dispatch_context_set_gfx(&graphics);
    dispatch_context_set_mem(&memory);
    p8_gfx_init(&graphics);
    p8_cls(&memory);
    tooltip_draw_at(&graphics, &memory, tooltip_new(&heap_allocator, &icon_tooltips_table[0]), p8_fixed_from_int(127), p8_fixed_from_int(1));
    check("icon_tooltips_first_entry drawn", screen_hash(), ICON_TOOLTIPS_FIRST_ENTRY_HASH);

    /* The class's draw_at gets a vector no caller passes it: the top is 0. */
    {
        uint32_t via_slot;

        p8_cls(&memory);
        direct->header.class->draw_at((void *)tooltip_new(&heap_allocator, &icon_tooltips_table[0]),
                               vec_make(p8_fixed_from_int(127), p8_fixed_from_int(0)), p8_fixed_from_int(1));
        via_slot = screen_hash();
        p8_cls(&memory);
        tooltip_draw_at(&graphics, &memory, tooltip_new(&heap_allocator, &icon_tooltips_table[0]), p8_fixed_from_int(0), p8_fixed_from_int(1));
        assert(screen_hash() == via_slot);
    }
}

/* Reads the tooltip only through tooltip_draw_at(), then its pixels, so it
 * holds on either layout of `rows`. */
static uint32_t draw_by_name(tooltip_t *jb, p8_fixed_t om, p8_fixed_t a) {
    uint32_t blank;
    uint32_t got;

    p8_gfx_init(&graphics);
    p8_cls(&memory);
    blank = screen_hash();
    tooltip_draw_at(&graphics, &memory, jb, om, a);
    got = screen_hash();
    assert(got != blank);
    return got;
}

static void test_tooltip_state_by_name(void) {
    tooltip_t *jb = tooltip_new(&heap_allocator, NULL);
    const entity_class_t *cls = jb->header.class;
    static const data_item_t row_items[] = {{.text = "gamma"}};
    static const data_row_t row = {.count = 1, .items = row_items};
    static const data_item_t mixed_items[] = {{.text = "alpha"}, {.row = &row}, {.number = 5}, {.text = "delta"}};
    static const data_row_t mixed = {.count = 4, .items = mixed_items};
    uint32_t h = 2166136261u;
    int i;

    assert(cls->draw_at != NULL);

    dispatch_context_set_gfx(&graphics);
    dispatch_context_set_mem(&memory);
    for (i = 1; i <= 7; i++) {
        h ^= draw_by_name(tooltip_new(&heap_allocator, &icon_tooltips_table[i - 1]), p8_fixed_from_int(127), p8_fixed_from_int(1));
        h *= 16777619u;
    }
    printf("[icon_tooltips_all_by_name] 0x%08Xu\n", (unsigned)h);
    fflush(stdout);
    assert(h == ICON_TOOLTIPS_ALL_BY_NAME_HASH);

    h = draw_by_name(tooltip_new(&heap_allocator, &mixed), p8_fixed_from_int(0), P8_FIXED_LIT(0x00008000, 0.5));
    printf("[mixed by name] 0x%08Xu\n", (unsigned)h);
    fflush(stdout);
    assert(h == MIXED_BY_NAME_HASH);
}

int main(void) {
    p8_vram_init(&memory);
    assert(p8_load_gfx_json(&memory, "pico8/gfx.json"));
    data_init();
    layout_init();

    test_icon_tooltips_entry_draws_as_before();
    test_bare_strings_draw_as_before();
    test_no_overrides_draws_frame_only();
    test_tooltip_is_a_struct_with_its_method_table();
    test_tooltip_state_by_name();

    printf("test_tooltip_draw_at: all tests passed\n");
    return 0;
}
