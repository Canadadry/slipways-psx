#include "pico8_pi.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_geom.h"
#include "pico8_hud.h"
#include "pico8_layout.h"
#include "pico8_vec.h"

typedef struct {
    char name[8];
    pico8_ob_constructor_fn fn;
} pico8_pi_dispatch_entry_t;

static pico8_pi_dispatch_entry_t g_pi_dispatch[PICO8_PI_DISPATCH_CAPACITY];
static int g_pi_dispatch_count = 0;

/* Borrowed, never owned: set by pico8_bl/pico8_bl_default right before
 * each dispatch, so the adapters below can reach a real drawing function
 * that pico8_ob_constructor_fn's five-value shape cannot carry. */
static pico8_gfx_t *g_pi_gfx = NULL;
static pico8_mem_t *g_pi_mem = NULL;

void pico8_pi_dispatch_register(const char *name, pico8_ob_constructor_fn fn) {
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

pico8_ob_constructor_fn pico8_pi_dispatch_lookup(const char *name) {
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

/* An absent `col` draws in colour 0 here, where PICO-8 would keep the
 * current pen colour -- no bl()-dispatched call site omits it. */
static pico8_value_t pico8_pi_rk_constructor(pico8_value_t a1, pico8_value_t a2, pico8_value_t a3, pico8_value_t a4,
                                              pico8_value_t a5) {
    int col;
    (void)a4;
    (void)a5;
    col = (a3.kind == PICO8_VALUE_FIXED) ? pico8_fixed_flr_int(a3.as.fixed) : 0;
    if (a2.kind == PICO8_VALUE_FIXED) {
        pico8_rk(g_pi_gfx, g_pi_mem, a1.as.fixed, a2.as.fixed, col);
    } else {
        pico8_rk_y1(g_pi_gfx, g_pi_mem, a1.as.fixed, col);
    }
    return pico8_value_ref(NULL);
}

/* `a1` is a number as often as a string, and print() renders a number as
 * its decimals: substituting "" for one blanks the year and the
 * technology counter in the top menu bar. Other kinds do fall back to "". */
static pico8_value_t pico8_pi_io_constructor(pico8_value_t a1, pico8_value_t a2, pico8_value_t a3, pico8_value_t a4,
                                              pico8_value_t a5) {
    char *owned = (a1.kind == PICO8_VALUE_FIXED) ? pico8_value_tostr(a1) : NULL;
    const char *t = (a1.kind == PICO8_VALUE_STRING) ? a1.as.string : (owned != NULL ? owned : "");
    pico8_fixed_t x = (a2.kind == PICO8_VALUE_FIXED) ? a2.as.fixed : pico8_fixed_from_int(0);
    pico8_fixed_t y = (a3.kind == PICO8_VALUE_FIXED) ? a3.as.fixed : pico8_fixed_from_int(0);
    int c = (a4.kind == PICO8_VALUE_FIXED) ? pico8_fixed_flr_int(a4.as.fixed) : 0;
    if (a5.kind == PICO8_VALUE_FIXED) {
        pico8_fixed_t a_val = a5.as.fixed;
        pico8_io(g_pi_gfx, g_pi_mem, t, x, y, c, &a_val);
    } else {
        pico8_io_default(g_pi_gfx, g_pi_mem, t, x, y, c);
    }
    free(owned);
    return pico8_value_ref(NULL);
}

static pico8_value_t pico8_pi_eq_constructor(pico8_value_t a1, pico8_value_t a2, pico8_value_t a3, pico8_value_t a4,
                                              pico8_value_t a5) {
    (void)a4;
    (void)a5;
    pico8_eq(g_pi_gfx, g_pi_mem, a1.as.fixed, a2.as.fixed, a3.as.fixed);
    return pico8_value_ref(NULL);
}

/* Shared by the char-keyed 's' and the string-keyed "s" registration.
 * w/h are supplied both or neither, as in real spr(). */
static pico8_value_t pico8_pi_spr_constructor(pico8_value_t a1, pico8_value_t a2, pico8_value_t a3, pico8_value_t a4,
                                               pico8_value_t a5) {
    int n = pico8_fixed_flr_int(a1.as.fixed);
    int x = pico8_fixed_flr_int(a2.as.fixed);
    int y = pico8_fixed_flr_int(a3.as.fixed);
    if (a4.kind == PICO8_VALUE_FIXED && a5.kind == PICO8_VALUE_FIXED) {
        int w = pico8_fixed_flr_int(a4.as.fixed);
        int h = pico8_fixed_flr_int(a5.as.fixed);
        pico8_spr(g_pi_gfx, g_pi_mem, n, x, y, w, h);
    } else {
        pico8_spr_default(g_pi_gfx, g_pi_mem, n, x, y);
    }
    return pico8_value_ref(NULL);
}

/* The returned REF points at a heap pico8_vec_t, the shape every
 * vector-valued field in this port carries. */
/* TROU-7 */
static pico8_value_t pico8_pi_vec_v_constructor(pico8_value_t a1, pico8_value_t a2, pico8_value_t a3,
                                                 pico8_value_t a4, pico8_value_t a5) {
    pico8_vec_t *v;
    (void)a3;
    (void)a4;
    (void)a5;
    v = malloc(sizeof *v);
    *v = pico8_vec_v(a1.as.fixed, a2.as.fixed);
    return pico8_value_ref(v);
}

/* lq.ol is registered UNBOUND, so oo() hands a1 over as lq:ol's own
 * `self`, a2 as the position vector and a3 as the alignment factor. */
static pico8_value_t pico8_pi_ld_constructor(pico8_value_t a1, pico8_value_t a2, pico8_value_t a3, pico8_value_t a4,
                                              pico8_value_t a5) {
    const pico8_instance_t *self = (a1.kind == PICO8_VALUE_REF) ? (const pico8_instance_t *)a1.as.ref : NULL;
    pico8_vec_t p = (a2.kind == PICO8_VALUE_REF && a2.as.ref != NULL)
                        ? *(const pico8_vec_t *)a2.as.ref
                        : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    pico8_fixed_t a = (a3.kind == PICO8_VALUE_FIXED) ? a3.as.fixed : pico8_fixed_from_int(0);
    (void)a4;
    (void)a5;
    pico8_lq_ol(g_pi_gfx, g_pi_mem, self, p, a);
    return pico8_value_ref(NULL);
}

/* oo(pi[e.fn],e) -- bl()'s inner step, shared by both entry points. The
 * dispatch context has to be set before every call, not once per list. */
static void pico8_bl_dispatch_one(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_value_t e_val) {
    pico8_ob_table_t *e;
    pico8_value_t fn_val;
    pico8_ob_constructor_fn fn = NULL;

    g_pi_gfx = gfx;
    g_pi_mem = mem;

    e = (pico8_ob_table_t *)e_val.as.ref;
    if (pico8_ob_table_get_name(e, "fn", &fn_val) && fn_val.kind == PICO8_VALUE_STRING) {
        fn = pico8_pi_dispatch_lookup(fn_val.as.string);
    }
    pico8_oo(fn, e);
}

void pico8_bl(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_list_t *rb, pico8_fixed_t nc) {
    pico8_list_iter_t it = pico8_list_all(rb);
    pico8_value_t e_val;

    while (pico8_list_iter_next(&it, &e_val)) {
        pico8_camera(gfx, (int)pico8_fixed_to_double(nc), 0);
        nc = pico8_fixed_neg(nc);
        pico8_bl_dispatch_one(gfx, mem, e_val);
    }
}

void pico8_bl_default(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_list_t *rb) {
    pico8_list_iter_t it = pico8_list_all(rb);
    pico8_value_t e_val;

    while (pico8_list_iter_next(&it, &e_val)) {
        pico8_bl_dispatch_one(gfx, mem, e_val);
    }
}

void pico8_pi_init(void) {
    pico8_qz_register_constructor();
    pico8_ob_register_constructor('v', pico8_pi_vec_v_constructor);
    /* TROU-4 */
    pico8_ob_register_constructor('s', pico8_pi_spr_constructor);

    pico8_pi_dispatch_register("br", pico8_pi_rk_constructor);
    pico8_pi_dispatch_register("qr", pico8_pi_io_constructor);
    pico8_pi_dispatch_register("rf", pico8_pi_eq_constructor);
    pico8_pi_dispatch_register("s", pico8_pi_spr_constructor);
    pico8_pi_dispatch_register("ld", pico8_pi_ld_constructor);
}
