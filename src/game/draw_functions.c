#include "game/draw_functions.h"

#include <assert.h>
#include <string.h>

#include "game/hud.h"
#include "game/layout.h"
#include "p8/str.h"
#include "engine/vec.h"
#include "engine/frame_zone.h"

typedef struct {
    char name[16];
    draw_functions_fn_t draw;
} draw_functions_dispatch_entry_t;

static draw_functions_dispatch_entry_t g_draw_functions_method[DRAW_FUNCTIONS_DISPATCH_CAPACITY];
static int g_draw_functions_dispatch_count = 0;

/* Borrowed, never owned: set by draw_functions_draw_rows(_default) right
 * before each dispatch, so the functions below can reach a real drawing
 * function that an article cannot carry. */
static p8_graphics_t *g_draw_functions_gfx = NULL;
static p8_memory_t *g_draw_functions_mem = NULL;

void draw_functions_dispatch_register(const char *name, draw_functions_fn_t fn) {
    int i;
    for (i = 0; i < g_draw_functions_dispatch_count; i++) {
        if (strcmp(g_draw_functions_method[i].name, name) == 0) {
            g_draw_functions_method[i].draw = fn;
            return;
        }
    }
    assert(g_draw_functions_dispatch_count < DRAW_FUNCTIONS_DISPATCH_CAPACITY && "pi dispatch table full");
    assert(strlen(name) < sizeof g_draw_functions_method[0].name && "pi dispatch name would be truncated");
    strncpy(g_draw_functions_method[g_draw_functions_dispatch_count].name, name, sizeof g_draw_functions_method[g_draw_functions_dispatch_count].name - 1);
    g_draw_functions_method[g_draw_functions_dispatch_count].name[sizeof g_draw_functions_method[g_draw_functions_dispatch_count].name - 1] = '\0';
    g_draw_functions_method[g_draw_functions_dispatch_count].draw = fn;
    g_draw_functions_dispatch_count++;
}

draw_functions_fn_t draw_functions_dispatch_lookup(const char *name) {
    int i;
    for (i = 0; i < g_draw_functions_dispatch_count; i++) {
        if (strcmp(g_draw_functions_method[i].name, name) == 0) {
            return g_draw_functions_method[i].draw;
        }
    }
    return NULL;
}

void draw_functions_dispatch_clear(void) {
    g_draw_functions_dispatch_count = 0;
}

/* `e[i]`, 1-based; NULL past the end, where the cart's `oo` unpacked
 * nil. */
static const data_item_t *draw_functions_arg(const data_row_t *e, int i) {
    const data_item_t *item;

    if (i > e->count) {
        return NULL;
    }
    item = &e->items[i - 1];
    assert(item->row == NULL && "an article of bl holds no nested row");
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

/* 0 when `e[i]` is not a number. */
static p8_fixed_t number(const data_row_t *e, int i) {
    const data_item_t *item = draw_functions_arg(e, i);
    if (!is_number(e, i)) {
        return p8_fixed_from_int(0);
    }
    return item->is_fixed ? item->fixed : p8_fixed_from_int(item->number);
}

/* NULL when `e[i]` is not a pointer. */
static const void *draw_functions_ref(const data_row_t *e, int i) {
    const data_item_t *item = draw_functions_arg(e, i);
    return (item != NULL && item->text == NULL && !item->is_false && item->is_ref) ? item->ref : NULL;
}

/* An absent `col` draws in colour 0 here, where PICO-8 would keep the
 * current pen colour -- no bl()-dispatched call site omits it. */
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

/* w/h are supplied both or neither, as in real spr(). */
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

/* lq.ol is registered UNBOUND, so e[1] is lq:ol's own `self`, e[2] the
 * position vector and e[3] the alignment factor. */
static void draw_row_entry(const data_row_t *e) {
    const layout_row_t *self = draw_functions_ref(e, 1);
    const vec_t *at = draw_functions_ref(e, 2);
    vec_t p = at != NULL
                        ? *at
                        : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    p8_fixed_t a = number(e, 3);
    layout_row_draw_at(g_draw_functions_gfx, g_draw_functions_mem, self, p, a);
}

/* oo(pi[e.fn],e) -- bl()'s inner step, shared by both entry points. The
 * dispatch context has to be set before every call, not once per list. */
static void draw_rows_dispatch_one(p8_graphics_t *gfx, p8_memory_t *mem, const data_row_t *e) {
    draw_functions_fn_t fn = NULL;

    g_draw_functions_gfx = gfx;
    g_draw_functions_mem = mem;

    if (e->draw_function != NULL) {
        fn = draw_functions_dispatch_lookup(e->draw_function);
    }
    if (fn != NULL) {
        fn(e);
    }
}

void draw_functions_draw_rows(p8_graphics_t *gfx, p8_memory_t *mem, const data_row_t *const *rb, int n, p8_fixed_t nc) {
    int i;

    for (i = 0; i < n; i++) {
        p8_camera(gfx, p8_fixed_flr_int(nc), 0);
        nc = p8_fixed_neg(nc);
        draw_rows_dispatch_one(gfx, mem, rb[i]);
    }
}

void draw_functions_draw_rows_default(p8_graphics_t *gfx, p8_memory_t *mem, const data_row_t *const *rb, int n) {
    int i;

    for (i = 0; i < n; i++) {
        draw_rows_dispatch_one(gfx, mem, rb[i]);
    }
}

void draw_functions_init(void) {
    draw_functions_dispatch_register("br", draw_bar_entry);
    draw_functions_dispatch_register("qr", print_outlined_entry);
    draw_functions_dispatch_register("rf", draw_help_frame_entry);
    draw_functions_dispatch_register("s", spr_entry);
    draw_functions_dispatch_register("draw_row", draw_row_entry);
}
