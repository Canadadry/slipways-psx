#include "game/draw_functions.h"

#include <assert.h>

#include "game/hud.h"
#include "game/layout.h"
#include "p8/str.h"
#include "engine/vec.h"
#include "engine/frame_zone.h"

/* Borrowed, never owned: set by draw_functions_draw_rows(_default) right
 * before each dispatch, so the functions below can reach a real drawing
 * function that a row cannot carry. */
static p8_graphics_t *g_draw_functions_gfx = NULL;
static p8_vram_t *g_draw_functions_mem = NULL;

/* The row's item at a 1-based index, as the data tables number the
 * arguments; NULL past the end. */
static const data_item_t *draw_functions_arg(const data_row_t *e, int i) {
    const data_item_t *item;

    if (i > e->count) {
        return NULL;
    }
    item = &e->items[i - 1];
    assert(item->row == NULL && "a row drawn by draw_functions_draw_rows holds no nested row");
    return item;
}

static int is_number(const data_row_t *e, int i) {
    const data_item_t *item = draw_functions_arg(e, i);
    if (item == NULL || item->text != NULL) {
        return 0;
    }
    if (item->is_false || item->is_ref) {
        return 0;
    }
    return 1;
}

/* 0 when that item is not a number. */
static p8_fixed_t number(const data_row_t *e, int i) {
    const data_item_t *item = draw_functions_arg(e, i);
    if (!is_number(e, i)) {
        return p8_fixed_from_int(0);
    }
    return item->is_fixed ? item->fixed : p8_fixed_from_int(item->number);
}

/* NULL when that item is not a pointer. */
static const void *draw_functions_ref(const data_row_t *e, int i) {
    const data_item_t *item = draw_functions_arg(e, i);
    return (item != NULL && item->text == NULL && !item->is_false && item->is_ref) ? item->ref : NULL;
}

/* An absent color draws in color 0 here, where PICO-8 would keep the
 * current pen color -- every draw_bar row of the data passes one. */
static void draw_bar_entry(const data_row_t *e) {
    p8_fixed_t y1 = number(e, 1);
    int col;
    col = p8_fixed_flr_int(number(e, 3));
    if (is_number(e, 2)) {
        hud_draw_bar(g_draw_functions_gfx, g_draw_functions_mem, y1, number(e, 2), col);
    } else {
        hud_draw_bar_y1(g_draw_functions_gfx, g_draw_functions_mem, y1, col);
    }
}

/* `a1` is a number as often as a string, and print() renders a number as
 * its decimals: substituting "" for one blanks the year and the
 * technology counter in the top menu bar. Other kinds do fall back to "". */
static void print_outlined_entry(const data_row_t *e) {
    const data_item_t *a1 = draw_functions_arg(e, 1);
    char *owned = is_number(e, 1) ? p8_tostr_fixed(frame_zone(), number(e, 1)) : NULL;
    const char *t = a1 != NULL && a1->text != NULL ? a1->text : (owned != NULL ? owned : "");
    p8_fixed_t x = number(e, 2);
    p8_fixed_t y = number(e, 3);
    int c = p8_fixed_flr_int(number(e, 4));
    if (is_number(e, 5)) {
        p8_fixed_t a_val = number(e, 5);
        hud_print_outlined(g_draw_functions_gfx, g_draw_functions_mem, t, x, y, c, &a_val);
    } else {
        hud_print_outlined_default(g_draw_functions_gfx, g_draw_functions_mem, t, x, y, c);
    }
}

static void draw_help_frame_entry(const data_row_t *e) {
    hud_draw_help_frame(g_draw_functions_gfx, g_draw_functions_mem, number(e, 1), number(e, 2), number(e, 3));
}

/* Width and height come both or neither, as p8_spr() takes them. */
static void spr_entry(const data_row_t *e) {
    int n = p8_fixed_flr_int(number(e, 1));
    int x = p8_fixed_flr_int(number(e, 2));
    int y = p8_fixed_flr_int(number(e, 3));
    if (is_number(e, 4) && is_number(e, 5)) {
        int w = p8_fixed_flr_int(number(e, 4));
        int h = p8_fixed_flr_int(number(e, 5));
        p8_spr(g_draw_functions_gfx, g_draw_functions_mem, n, x, y, w, h);
    } else {
        p8_spr_default(g_draw_functions_gfx, g_draw_functions_mem, n, x, y);
    }
}

/* Item 1 is the layout row itself (layout_row_draw_at()'s `self`), item 2
 * the position vector and item 3 the alignment factor. */
static void draw_row_entry(const data_row_t *e) {
    const layout_row_t *self = draw_functions_ref(e, 1);
    const vec_t *at = draw_functions_ref(e, 2);
    vec_t p = at != NULL
                        ? *at
                        : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    p8_fixed_t a = number(e, 3);
    layout_row_draw_at(g_draw_functions_gfx, g_draw_functions_mem, self, p, a);
}

static const draw_functions_fn_t draw_functions_by_id[DRAW_FUNCTION_COUNT] = {
    [DRAW_FUNCTION_NONE] = NULL,
    [DRAW_FUNCTION_DRAW_BAR] = draw_bar_entry,
    [DRAW_FUNCTION_PRINT_OUTLINED] = print_outlined_entry,
    [DRAW_FUNCTION_DRAW_HELP_FRAME] = draw_help_frame_entry,
    [DRAW_FUNCTION_SPR] = spr_entry,
    [DRAW_FUNCTION_DRAW_ROW] = draw_row_entry,
};

/* One row: its draw_function looked up and called on it, for both entry
 * points. The dispatch context is set before every call, not once per list. */
static void draw_rows_dispatch_one(p8_graphics_t *gfx, p8_vram_t *mem, const data_row_t *e) {
    draw_functions_fn_t fn;

    assert((unsigned)e->draw_function < DRAW_FUNCTION_COUNT && "a row's draw_function is one of draw_function_t");
    fn = draw_functions_by_id[e->draw_function];
    g_draw_functions_gfx = gfx;
    g_draw_functions_mem = mem;

    if (fn != NULL) {
        fn(e);
    }
}

void draw_functions_draw_rows(p8_graphics_t *gfx, p8_vram_t *mem, const data_row_t *const *rb, int n, p8_fixed_t nc) {
    int i;

    for (i = 0; i < n; i++) {
        p8_camera(gfx, p8_fixed_flr_int(nc), 0);
        nc = p8_fixed_neg(nc);
        draw_rows_dispatch_one(gfx, mem, rb[i]);
    }
}

void draw_functions_draw_rows_default(p8_graphics_t *gfx, p8_vram_t *mem, const data_row_t *const *rb, int n) {
    int i;

    for (i = 0; i < n; i++) {
        draw_rows_dispatch_one(gfx, mem, rb[i]);
    }
}
