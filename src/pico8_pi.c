#include "pico8_pi.h"

#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_geom.h"
#include "pico8_hud.h"
#include "pico8_vec.h"

/* pi={b=qz,v=v,br=rk,qr=io,rf=eq,s=spr} (pico8/lua.lua:133). See
 * pico8_pi.h's own top comment for the two-registries decision this
 * module implements, and the per-declaration comments there for each
 * piece's own trace. */

/* Fixed-capacity, linear-strcmp string->pico8_ob_constructor_fn table --
 * see pico8_pi.h's own comment on PICO8_PI_DISPATCH_CAPACITY for why this
 * is deliberately NOT pico8_ob_table_t and NOT a doubling-capacity
 * growable array. */
typedef struct {
    char name[8];
    pico8_ob_constructor_fn fn;
} pico8_pi_dispatch_entry_t;

static pico8_pi_dispatch_entry_t g_pi_dispatch[PICO8_PI_DISPATCH_CAPACITY];
static int g_pi_dispatch_count = 0;

/* Small private "current gfx/mem" context, read only by this module's own
 * adapters (pico8_pi_rk_constructor et al. below) that need to call
 * through to a real Phase 1 drawing function -- pico8_ob_constructor_fn's
 * fixed 5-pico8_value_t-argument shape has no room to carry gfx/mem
 * itself. Set by pico8_bl/pico8_bl_default (the only real callers of this
 * dispatch path in the whole cart) right before each dispatch; see
 * pico8_pi.h's own comment on pico8_bl for the scope/limits of this. Not
 * exposed in the header -- nothing outside this module needs to set it. */
static const pico8_gfx_t *g_pi_gfx = NULL;
static pico8_mem_t *g_pi_mem = NULL;

void pico8_pi_dispatch_register(const char *name, pico8_ob_constructor_fn fn) {
    int i;
    for (i = 0; i < g_pi_dispatch_count; i++) {
        if (strcmp(g_pi_dispatch[i].name, name) == 0) {
            g_pi_dispatch[i].fn = fn;
            return;
        }
    }
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

/* pi.br = rk (function rk(y1,y2,...)rectfill(0,y1,127,y2 or y1,...)end,
 * pico8_hud.h/.c). Distinguishes a2 (y2) present from omitted via
 * a2.kind==PICO8_VALUE_FIXED, the same "absent argument" convention
 * pico8_oo (pico8_collect.h) already established for oo()'s own a[1]..a[5]
 * unpacking. `...` (col, a3) is never omitted at any real bl()-dispatched
 * call site this chunk's own scope evidences; a3 absent defaults to 0
 * rather than guessing at PICO-8's own "col omitted -> last color drawn"
 * state (pico8_hud.h's own rk comment already flags that gap for a
 * DIFFERENT, non-bl() call site, Chunk 18). */
static pico8_value_t pico8_pi_rk_constructor(pico8_value_t a1, pico8_value_t a2, pico8_value_t a3, pico8_value_t a4,
                                              pico8_value_t a5) {
    int col;
    (void)a4;
    (void)a5;
    col = (a3.kind == PICO8_VALUE_FIXED) ? (int)pico8_fixed_to_double(a3.as.fixed) : 0;
    if (a2.kind == PICO8_VALUE_FIXED) {
        pico8_rk(g_pi_gfx, g_pi_mem, a1.as.fixed, a2.as.fixed, col);
    } else {
        pico8_rk_y1(g_pi_gfx, g_pi_mem, a1.as.fixed, col);
    }
    return pico8_value_ref(NULL);
}

/* pi.qr = io (function io(t,x,y,c,a) ... end, pico8_hud.h/.c). `a5` (io's
 * own optional centering factor `a`) absent -> pico8_io_default, present
 * -> pico8_io with a pointer to the unpacked value (matching pico8_io's
 * own const pico8_fixed_t* convention for an optional trailing arg). `a1`
 * (the text) not being PICO8_VALUE_STRING defensively falls back to "" --
 * matches pico8_oo's own established "field absence stays a documented,
 * defensive default" divergence, never exercised by a real spec string. */
static pico8_value_t pico8_pi_io_constructor(pico8_value_t a1, pico8_value_t a2, pico8_value_t a3, pico8_value_t a4,
                                              pico8_value_t a5) {
    const char *t = (a1.kind == PICO8_VALUE_STRING) ? a1.as.string : "";
    pico8_fixed_t x = (a2.kind == PICO8_VALUE_FIXED) ? a2.as.fixed : pico8_fixed_from_int(0);
    pico8_fixed_t y = (a3.kind == PICO8_VALUE_FIXED) ? a3.as.fixed : pico8_fixed_from_int(0);
    int c = (a4.kind == PICO8_VALUE_FIXED) ? (int)pico8_fixed_to_double(a4.as.fixed) : 0;
    if (a5.kind == PICO8_VALUE_FIXED) {
        pico8_fixed_t a_val = a5.as.fixed;
        pico8_io(g_pi_gfx, g_pi_mem, t, x, y, c, &a_val);
    } else {
        pico8_io_default(g_pi_gfx, g_pi_mem, t, x, y, c);
    }
    return pico8_value_ref(NULL);
}

/* pi.rf = eq (function eq(x1,y1,x2)rect(...)rectfill(...)spr(...)spr(...)
 * end, pico8_hud.h/.c) -- 3 fixed positional arguments, never optional at
 * this function's own single real Lua definition. */
static pico8_value_t pico8_pi_eq_constructor(pico8_value_t a1, pico8_value_t a2, pico8_value_t a3, pico8_value_t a4,
                                              pico8_value_t a5) {
    (void)a4;
    (void)a5;
    pico8_eq(g_pi_gfx, g_pi_mem, a1.as.fixed, a2.as.fixed, a3.as.fixed);
    return pico8_value_ref(NULL);
}

/* pi.s = spr (the Phase 1 host builtin, pico8_gfx.h's pico8_spr/
 * pico8_spr_default). Shared, unmodified, between BOTH of pi's real
 * registrations of this same target ('s' char-keyed, "s" string-keyed,
 * pico8_pi_init below) -- one adapter, not duplicated. `a4`/`a5` (w/h)
 * both present -> pico8_spr; either absent -> pico8_spr_default (matches
 * real spr(n,x,y,w,h)'s own w/h-both-or-neither PICO-8 convention -- no
 * real call site in scope ever supplies just one of the two). */
static pico8_value_t pico8_pi_spr_constructor(pico8_value_t a1, pico8_value_t a2, pico8_value_t a3, pico8_value_t a4,
                                               pico8_value_t a5) {
    int n = (int)pico8_fixed_to_double(a1.as.fixed);
    int x = (int)pico8_fixed_to_double(a2.as.fixed);
    int y = (int)pico8_fixed_to_double(a3.as.fixed);
    if (a4.kind == PICO8_VALUE_FIXED && a5.kind == PICO8_VALUE_FIXED) {
        int w = (int)pico8_fixed_to_double(a4.as.fixed);
        int h = (int)pico8_fixed_to_double(a5.as.fixed);
        pico8_spr(g_pi_gfx, g_pi_mem, n, x, y, w, h);
    } else {
        pico8_spr_default(g_pi_gfx, g_pi_mem, n, x, y);
    }
    return pico8_value_ref(NULL);
}

/* pi.v = v (function v(x,y) return setmetatable({x=x,y=y},km)end,
 * pico8_vec.h's pico8_vec_v). Heap-allocates the resulting pico8_vec_t and
 * wraps it as a PICO8_VALUE_REF -- the same "a vector field value is a REF
 * to a heap pico8_vec_t" convention Chunk 1's own pico8_qz_constructor
 * (pico8_collect.c) already established, and pico8_entity_geom.h's own
 * consumers (gk/gd/na/ma/ij) already rely on. */
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

/* oo(pi[e.fn],e) -- bl()'s own inner dispatch step, factored out since
 * both pico8_bl (nc provided) and pico8_bl_default (nc omitted) need it
 * identically; see pico8_pi.h's own comment on pico8_bl for why `e` is
 * expected to be a PICO8_VALUE_REF to a pico8_ob_table_t. Sets this
 * module's own private gfx/mem context (see its declaration above) before
 * every dispatch, since pico8_oo's fn call has no way to receive gfx/mem
 * itself. e.fn absent, or present but not (yet) registered -- see
 * pico8_pi_init's own note on "ld" -- leaves fn NULL, and pico8_oo(NULL,e)
 * is a documented, faithful no-op (see pico8_pi.h's own comment on
 * pico8_bl for why the "or a" fallback gap already flagged on pico8_oo
 * itself never matters here: bl() discards oo()'s return value). */
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

/* function bl(rb,nc)for e in all(rb)do
 * if nc then
 * camera(nc,0)nc=-nc
 * end
 * oo(pi[e.fn],e)end
 * end
 * (pico8/lua.lua:597-602.) See pico8_pi.h's own comment on this
 * declaration for the full design trace (rb's pico8_list_t
 * representation, nc's two-entry-point optional-argument convention,
 * gfx/mem threading). This entry point is the `nc` PROVIDED case: every
 * iteration sets camera(nc,0) then flips nc's own sign for the next
 * entity, exactly mirroring the real `nc=-nc` reassignment (which stays
 * inside the `if nc then` branch, so once nc is truthy it stays truthy --
 * a plain pico8_fixed_t works fine here without needing a separate
 * "provided" flag, since the branch is only ever entered by callers who
 * already know they have a real nc). */
void pico8_bl(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_list_t *rb, pico8_fixed_t nc) {
    pico8_list_iter_t it = pico8_list_all(rb);
    pico8_value_t e_val;

    while (pico8_list_iter_next(&it, &e_val)) {
        pico8_camera(gfx, (int)pico8_fixed_to_double(nc), 0);
        nc = pico8_fixed_neg(nc);
        pico8_bl_dispatch_one(gfx, mem, e_val);
    }
}

/* bl()'s own `nc` OMITTED case (nc stays Lua nil for every iteration, so
 * the `if nc then...end` branch never runs at all -- see pico8_pi.h's own
 * comment on pico8_bl for why this needs its own entry point rather than
 * a sentinel fixed-point value, matching pico8_dv/pico8_rk/pico8_io's own
 * established convention in this exact area of the port). */
void pico8_bl_default(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_list_t *rb) {
    pico8_list_iter_t it = pico8_list_all(rb);
    pico8_value_t e_val;

    while (pico8_list_iter_next(&it, &e_val)) {
        pico8_bl_dispatch_one(gfx, mem, e_val);
    }
}

/* See pico8_pi.h's own comment on this declaration for the full rationale
 * (grouped bootstrap, the deliberately-left "ld" gap for Chunk 3, "ready
 * but unwired" status). */
void pico8_pi_init(void) {
    pico8_qz_register_constructor();
    pico8_ob_register_constructor('v', pico8_pi_vec_v_constructor);
    pico8_ob_register_constructor('s', pico8_pi_spr_constructor);

    pico8_pi_dispatch_register("br", pico8_pi_rk_constructor);
    pico8_pi_dispatch_register("qr", pico8_pi_io_constructor);
    pico8_pi_dispatch_register("rf", pico8_pi_eq_constructor);
    pico8_pi_dispatch_register("s", pico8_pi_spr_constructor);
}
