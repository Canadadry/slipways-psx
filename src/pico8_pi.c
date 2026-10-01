#include "pico8_pi.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_hud.h"
#include "pico8_layout.h"
#include "pico8_str.h"
#include "pico8_vec.h"

typedef struct {
    char name[8];
    pico8_pi_fn fn;
} pico8_pi_dispatch_entry_t;

static pico8_pi_dispatch_entry_t g_pi_dispatch[PICO8_PI_DISPATCH_CAPACITY];
static int g_pi_dispatch_count = 0;

/* Borrowed, never owned: set by pico8_bl/pico8_bl_default right before
 * each dispatch, so the functions below can reach a real drawing function
 * that an article cannot carry. */
static pico8_gfx_t *g_pi_gfx = NULL;
static pico8_mem_t *g_pi_mem = NULL;

void pico8_pi_dispatch_register(const char *name, pico8_pi_fn fn) {
    int i;
    for (i = 0; i < g_pi_dispatch_count; i++) {
        if (strcmp(g_pi_dispatch[i].name, name) == 0) {
            g_pi_dispatch[i].fn = fn;
            return;
        }
    }
    assert(g_pi_dispatch_count < PICO8_PI_DISPATCH_CAPACITY && "pi dispatch table full");
    assert(strlen(name) < sizeof g_pi_dispatch[0].name && "pi dispatch name would be truncated");
    strncpy(g_pi_dispatch[g_pi_dispatch_count].name, name, sizeof g_pi_dispatch[g_pi_dispatch_count].name - 1);
    g_pi_dispatch[g_pi_dispatch_count].name[sizeof g_pi_dispatch[g_pi_dispatch_count].name - 1] = '\0';
    g_pi_dispatch[g_pi_dispatch_count].fn = fn;
    g_pi_dispatch_count++;
}

pico8_pi_fn pico8_pi_dispatch_lookup(const char *name) {
    int i;
    for (i = 0; i < g_pi_dispatch_count; i++) {
        if (strcmp(g_pi_dispatch[i].name, name) == 0) {
            return g_pi_dispatch[i].fn;
        }
    }
    return NULL;
}

void pico8_pi_dispatch_clear(void) {
    g_pi_dispatch_count = 0;
}

/* `e[i]`, 1-based; NULL past the end, where the cart's `oo` unpacked
 * nil. */
static const pico8_jb_item_t *pico8_pi_arg(const pico8_jb_row_t *e, int i) {
    const pico8_jb_item_t *item;

    if (i > e->count) {
        return NULL;
    }
    item = &e->items[i - 1];
    assert(item->row == NULL && "an article of bl holds no nested row");
    return item;
}

static int pico8_pi_is_number(const pico8_jb_row_t *e, int i) {
    const pico8_jb_item_t *item = pico8_pi_arg(e, i);
    if (item == NULL || item->s != NULL) {
        return 0;
    }
    if (item->is_false || item->is_ref) {
        return 0;
    }
    return 1;
}

/* 0 when `e[i]` is not a number. */
static pico8_fixed_t pico8_pi_number(const pico8_jb_row_t *e, int i) {
    const pico8_jb_item_t *item = pico8_pi_arg(e, i);
    if (!pico8_pi_is_number(e, i)) {
        return pico8_fixed_from_int(0);
    }
    return item->is_fixed ? item->fixed : pico8_fixed_from_int(item->n);
}

/* NULL when `e[i]` is not a pointer. */
static const void *pico8_pi_ref(const pico8_jb_row_t *e, int i) {
    const pico8_jb_item_t *item = pico8_pi_arg(e, i);
    return (item != NULL && item->s == NULL && !item->is_false && item->is_ref) ? item->ref : NULL;
}

/* An absent `col` draws in colour 0 here, where PICO-8 would keep the
 * current pen colour -- no bl()-dispatched call site omits it. */
static void pico8_pi_rk_draw(const pico8_jb_row_t *e) {
    pico8_fixed_t y1 = pico8_pi_number(e, 1);
    int col;
    col = pico8_fixed_flr_int(pico8_pi_number(e, 3));
    if (pico8_pi_is_number(e, 2)) {
        pico8_rk(g_pi_gfx, g_pi_mem, y1, pico8_pi_number(e, 2), col);
    } else {
        pico8_rk_y1(g_pi_gfx, g_pi_mem, y1, col);
    }
}

/* `a1` is a number as often as a string, and print() renders a number as
 * its decimals: substituting "" for one blanks the year and the
 * technology counter in the top menu bar. Other kinds do fall back to "". */
static void pico8_pi_io_draw(const pico8_jb_row_t *e) {
    const pico8_jb_item_t *a1 = pico8_pi_arg(e, 1);
    char *owned = pico8_pi_is_number(e, 1) ? pico8_tostr_fixed(pico8_pi_number(e, 1)) : NULL;
    const char *t = a1 != NULL && a1->s != NULL ? a1->s : (owned != NULL ? owned : "");
    pico8_fixed_t x = pico8_pi_number(e, 2);
    pico8_fixed_t y = pico8_pi_number(e, 3);
    int c = pico8_fixed_flr_int(pico8_pi_number(e, 4));
    if (pico8_pi_is_number(e, 5)) {
        pico8_fixed_t a_val = pico8_pi_number(e, 5);
        pico8_io(g_pi_gfx, g_pi_mem, t, x, y, c, &a_val);
    } else {
        pico8_io_default(g_pi_gfx, g_pi_mem, t, x, y, c);
    }
    free(owned);
}

static void pico8_pi_eq_draw(const pico8_jb_row_t *e) {
    pico8_eq(g_pi_gfx, g_pi_mem, pico8_pi_number(e, 1), pico8_pi_number(e, 2), pico8_pi_number(e, 3));
}

/* w/h are supplied both or neither, as in real spr(). */
static void pico8_pi_spr_draw(const pico8_jb_row_t *e) {
    int n = pico8_fixed_flr_int(pico8_pi_number(e, 1));
    int x = pico8_fixed_flr_int(pico8_pi_number(e, 2));
    int y = pico8_fixed_flr_int(pico8_pi_number(e, 3));
    if (pico8_pi_is_number(e, 4) && pico8_pi_is_number(e, 5)) {
        int w = pico8_fixed_flr_int(pico8_pi_number(e, 4));
        int h = pico8_fixed_flr_int(pico8_pi_number(e, 5));
        pico8_spr(g_pi_gfx, g_pi_mem, n, x, y, w, h);
    } else {
        pico8_spr_default(g_pi_gfx, g_pi_mem, n, x, y);
    }
}

/* lq.ol is registered UNBOUND, so e[1] is lq:ol's own `self`, e[2] the
 * position vector and e[3] the alignment factor. */
static void pico8_pi_ld_draw(const pico8_jb_row_t *e) {
    const pico8_lq_t *self = pico8_pi_ref(e, 1);
    const pico8_vec_t *at = pico8_pi_ref(e, 2);
    pico8_vec_t p = at != NULL
                        ? *at
                        : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    pico8_fixed_t a = pico8_pi_number(e, 3);
    pico8_lq_ol(g_pi_gfx, g_pi_mem, self, p, a);
}

/* oo(pi[e.fn],e) -- bl()'s inner step, shared by both entry points. The
 * dispatch context has to be set before every call, not once per list. */
static void pico8_bl_dispatch_one(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_jb_row_t *e) {
    pico8_pi_fn fn = NULL;

    g_pi_gfx = gfx;
    g_pi_mem = mem;

    if (e->fn != NULL) {
        fn = pico8_pi_dispatch_lookup(e->fn);
    }
    if (fn != NULL) {
        fn(e);
    }
}

void pico8_bl(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_jb_row_t *const *rb, int n, pico8_fixed_t nc) {
    int i;

    for (i = 0; i < n; i++) {
        pico8_camera(gfx, pico8_fixed_flr_int(nc), 0);
        nc = pico8_fixed_neg(nc);
        pico8_bl_dispatch_one(gfx, mem, rb[i]);
    }
}

void pico8_bl_default(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_jb_row_t *const *rb, int n) {
    int i;

    for (i = 0; i < n; i++) {
        pico8_bl_dispatch_one(gfx, mem, rb[i]);
    }
}

void pico8_pi_init(void) {
    pico8_pi_dispatch_register("br", pico8_pi_rk_draw);
    pico8_pi_dispatch_register("qr", pico8_pi_io_draw);
    pico8_pi_dispatch_register("rf", pico8_pi_eq_draw);
    pico8_pi_dispatch_register("s", pico8_pi_spr_draw);
    pico8_pi_dispatch_register("ld", pico8_pi_ld_draw);
}
