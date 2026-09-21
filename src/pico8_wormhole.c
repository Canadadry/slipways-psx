#include "pico8_wormhole.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_audio.h"
#include "pico8_collect.h"
#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_ke.h"
#include "pico8_planet.h"
#include "pico8_probe.h"
#include "pico8_tech_gate.h"
#include "pico8_wormhole_display.h"

/* See pico8_wormhole.h's own top comment for the full source/trace of `mz`
 * -- the real Lua source for every method, the bm-vs-mv()-routed-vs-pq
 * classification, and the "SELF.ET REPRESENTATION" finding this module's
 * own `g_mz_jj_wrapper` resolves. `pico8_mz_truthy`/`pico8_mz_field_fixed`/
 * `pico8_mz_value_vec`/`pico8_mz_field_vec` below are private duplicates of
 * the SAME pattern already established independently by `pico8_probe.c`'s
 * own `pico8_probe_truthy`/`pico8_probe_field_fixed`/`pico8_probe_value_
 * vec`/`pico8_probe_field_vec` (and, further back, `pico8_planet.c`/
 * `pico8_planet_family.c`/etc.'s own per-module copies) -- documented,
 * accepted duplication, not a new pattern (grepped before writing). */

static pico8_class_t *g_mz_class = NULL;
static pico8_instance_t *g_mz_jj_wrapper = NULL;

static int pico8_mz_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

static pico8_fixed_t pico8_mz_field_fixed(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v) && v.kind == PICO8_VALUE_FIXED) {
        return v.as.fixed;
    }
    return pico8_fixed_from_int(0);
}

static pico8_vec_t pico8_mz_value_vec(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static pico8_vec_t pico8_mz_field_vec(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v)) {
        return pico8_mz_value_vec(v);
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

/* function(p) return p.fx end -- mz:pq's own jp() predicate closure
 * (pico8/lua.lua:763-764). Every `hv` element is a real `pico8_instance_t*`
 * REF (pico8_entity.h's own documented `hv` shape), so `e.as.ref` is always
 * safely castable; see pico8_wormhole.h's own `pico8_mz_pq` comment for the
 * full trace of what `.fx` truthy actually collects. */
static int pico8_mz_pq_ox_predicate(pico8_value_t e, void *ctx) {
    pico8_instance_t *p;
    pico8_value_t fx_val;
    (void)ctx;
    if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
        return 0;
    }
    p = (pico8_instance_t *)e.as.ref;
    if (!pico8_instance_get_field(p, "fx", &fx_val)) {
        return 0;
    }
    return pico8_mz_truthy(fx_val);
}

/* function mz:pq()rn(53)self.ox=jp(hv,function(p)
 * return p.fx
 * end)end
 * (pico8/lua.lua:762-764.) See pico8_wormhole.h's own comment on this
 * declaration for the full trace. `self.ox` OWNS this fresh heap list for
 * the rest of the instance's own lifetime -- never freed here (this port's
 * pervasive "GC-deferred" convention), and `pq` only ever runs once per
 * instance, so there is no re-entry/leak concern to guard against. */
void pico8_mz_pq(pico8_instance_t *self) {
    pico8_list_t *heap_ox;

    if (self == NULL) {
        return;
    }

    pico8_rn(pico8_value_fixed(pico8_fixed_from_int(53)));

    heap_ox = malloc(sizeof *heap_ox);
    *heap_ox = pico8_jp(pico8_hv(), pico8_mz_pq_ox_predicate, NULL);
    pico8_ob_table_set_name(self->fields, "ox", pico8_value_ref(heap_ox));
}

/* function mz:hq(...)bv(self,...)local nq
 * nq,self.ij=ij(self.p,self.ox)if nq>ov("space folding",0.19,0.27)then
 * self.et=mz.jj
 * end
 * end
 * (pico8/lua.lua:765-770.) See pico8_wormhole.h's own comment on this
 * declaration for the full trace, including the "SELF.ET REPRESENTATION"
 * resolution `pico8_mz_jj_wrapper()` implements. `bv(self,...)` is called
 * FIRST, unconditionally, exactly as the real source's own statement
 * ordering requires (it may itself already set a real `self.et`, which
 * this function's own `if nq>threshold` write can still unconditionally
 * overwrite with the "too far" wrapper below, matching the real source's
 * own lack of an `else` branch preserving `bv`'s own result). */
void pico8_mz_hq(pico8_instance_t *self, pico8_value_t mp, pico8_value_t rp) {
    pico8_vec_t p_vec;
    pico8_value_t ox_val;
    const pico8_list_t *ox_list = NULL;
    pico8_value_t nearest = pico8_value_ref(NULL);
    pico8_fixed_t nq;
    pico8_value_t threshold_val;
    pico8_fixed_t threshold;

    pico8_bv(self, mp, rp);
    if (self == NULL) {
        return;
    }

    p_vec = pico8_mz_field_vec(self, "p");
    if (pico8_instance_get_field(self, "ox", &ox_val) && ox_val.kind == PICO8_VALUE_REF && ox_val.as.ref != NULL) {
        ox_list = (const pico8_list_t *)ox_val.as.ref;
    }

    nq = pico8_ij(p_vec, ox_list, &nearest);
    pico8_ob_table_set_name(self->fields, "ij", nearest);

    threshold_val = pico8_ov(pico8_value_string("space folding"), pico8_value_fixed(pico8_fixed_from_double(0.19)),
                              pico8_value_fixed(pico8_fixed_from_double(0.27)));
    threshold = (threshold_val.kind == PICO8_VALUE_FIXED) ? threshold_val.as.fixed : pico8_fixed_from_int(0);

    if (nq > threshold) {
        pico8_ob_table_set_name(self->fields, "et", pico8_value_ref(pico8_mz_jj_wrapper()));
    }
}

/* Bridges pico8_value_function_t's (self,argc,arg0,arg1) shape to
 * pico8_mz_hq's own signature; `ui:bh()`'s own `mv(held,"hq",self.qa,
 * je.ra)` passes argc=2 every frame the wormhole stays held, matching
 * `pico8_bg_hq_dispatch`'s/`pico8_bv_dispatch`'s own identical 2-arg-
 * bridge precedent. `mz:hq` returns nothing (nil) in the real source. */
static pico8_value_t pico8_mz_hq_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    pico8_mz_hq((pico8_instance_t *)self, arg0, arg1);
    return pico8_value_ref(NULL);
}

/* function mz:kz(t)self.oi=ov("space folding",29,35)*
 * (min(t/15,1)+sin(t/40)*0.1)end
 * (pico8/lua.lua:771-772.) See pico8_wormhole.h's own comment on this
 * declaration for the full trace (the ease-in-then-wobble curve, and why
 * `min` is ported as a plain C ternary rather than a new shared helper). */
void pico8_mz_kz(pico8_instance_t *self, pico8_fixed_t t) {
    pico8_value_t threshold_val;
    pico8_fixed_t threshold;
    pico8_fixed_t one;
    pico8_fixed_t td15;
    pico8_fixed_t min_val;
    pico8_fixed_t sin_term;
    pico8_fixed_t result;

    if (self == NULL) {
        return;
    }

    threshold_val = pico8_ov(pico8_value_string("space folding"), pico8_value_fixed(pico8_fixed_from_int(29)),
                              pico8_value_fixed(pico8_fixed_from_int(35)));
    threshold = (threshold_val.kind == PICO8_VALUE_FIXED) ? threshold_val.as.fixed : pico8_fixed_from_int(0);

    one = pico8_fixed_from_int(1);
    td15 = pico8_fixed_div(t, pico8_fixed_from_int(15));
    min_val = (td15 < one) ? td15 : one;
    sin_term =
        pico8_fixed_mul(pico8_fixed_sin(pico8_fixed_div(t, pico8_fixed_from_int(40))), pico8_fixed_from_double(0.1));

    result = pico8_fixed_mul(threshold, pico8_fixed_add(min_val, sin_term));
    pico8_ob_table_set_name(self->fields, "oi", pico8_value_fixed(result));
}

/* Bridges to pico8_mz_kz's own signature; pico8_dg()'s own generic per-
 * tick self.me-named dispatch always calls with argc=1, arg0=self.t
 * (pico8_bootstrap.c's own `pico8_mv(qc, me_val.as.string, 1, t_arg,...)`)
 * -- unlike pico8_nf_ph_dispatch's own identical-shaped bridge, this one
 * genuinely reads arg0 and forwards it, since kz(t) uses it directly. */
static pico8_value_t pico8_mz_kz_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_fixed_t t = pico8_fixed_from_int(0);
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_FIXED) {
        t = arg0.as.fixed;
    }
    pico8_mz_kz((pico8_instance_t *)self, t);
    return pico8_value_ref(NULL);
}

/* function mz:ey()self.p_=
 * self.t<15 or not d_(self)end
 * (pico8/lua.lua:773-774.) See pico8_wormhole.h's own comment on this
 * declaration for the full trace -- both operands of this `or` are real
 * booleans (unlike bv's own table-valued and/or chain), so `self.p_`
 * always ends up a genuine PICO8_VALUE_BOOL, never Lua nil. `A` short-
 * circuits `d_(self)` entirely when `self.t<15` (ported as a real early
 * exit, not evaluate-and-discard, so `d_`'s own side effects are correctly
 * skipped during the wormhole's own first 15 ticks). */
void pico8_mz_ey(pico8_instance_t *self) {
    pico8_value_t t_val;
    pico8_fixed_t t = pico8_fixed_from_int(0);
    int result;

    if (self == NULL) {
        return;
    }

    if (pico8_instance_get_field(self, "t", &t_val) && t_val.kind == PICO8_VALUE_FIXED) {
        t = t_val.as.fixed;
    }

    if (t < pico8_fixed_from_int(15)) {
        result = 1;
    } else {
        result = !pico8_mz_truthy(pico8_d_(self));
    }

    pico8_ob_table_set_name(self->fields, "p_", pico8_value_bool(result));
}

/* Bridges to pico8_mz_ey's own signature; `ui:bh()`'s own `mv(held,"ey",
 * nw and nw.kv)` dispatch passes argc=1 (its own extra argument unused by
 * this body, matching g_.ey=d_'s own identical treatment,
 * pico8_d__dispatch). mz:ey returns nothing (nil) in the real source. */
static pico8_value_t pico8_mz_ey_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_mz_ey((pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

/* function mz:pr(t)local sr=self:ej(t)/128+0.031
 * for pt in all(cg.jo)do
 * if pt.me=="unknown"and (pt.p-self.p):qx()<sr then
 * pt:hh()end
 * end
 * self.p_=t>25
 * end
 * (pico8/lua.lua:775-781.) See pico8_wormhole.h's own comment on this
 * declaration for the full trace -- reveals nearby still-"unknown" planets
 * as the wormhole's own post-commit ripple expands, using REAL Euclidean
 * distance (:qx(), pico8_vec_qx), not the squared-distance shortcut ij/bv
 * use elsewhere. */
void pico8_mz_pr(pico8_instance_t *self, pico8_fixed_t t) {
    pico8_fixed_t sr;
    pico8_vec_t self_p;
    const pico8_list_t *jo_list;
    int i;

    if (self == NULL) {
        return;
    }

    sr = pico8_fixed_add(pico8_fixed_div(pico8_mz_ej(self, t), pico8_fixed_from_int(128)),
                          pico8_fixed_from_double(0.031));

    self_p = pico8_mz_field_vec(self, "p");
    jo_list = pico8_cg("jo");
    if (jo_list != NULL) {
        for (i = 0; i < jo_list->count; i++) {
            pico8_value_t pt_val = jo_list->items[i];
            pico8_instance_t *pt;
            pico8_value_t me_val;

            if (pt_val.kind != PICO8_VALUE_REF || pt_val.as.ref == NULL) {
                continue;
            }
            pt = (pico8_instance_t *)pt_val.as.ref;

            if (pico8_instance_get_field(pt, "me", &me_val) && me_val.kind == PICO8_VALUE_STRING &&
                me_val.as.string != NULL && strcmp(me_val.as.string, "unknown") == 0) {
                pico8_vec_t pt_p = pico8_mz_field_vec(pt, "p");
                pico8_vec_t diff = pico8_vec_sub(pt_p, self_p);
                if (pico8_vec_qx(diff) < sr) {
                    pico8_jo_hh(pt);
                }
            }
        }
    }

    pico8_ob_table_set_name(self->fields, "p_", pico8_value_bool(t > pico8_fixed_from_int(25)));
}

/* Bridges to pico8_mz_pr's own signature -- same argc=1/arg0=t shape as
 * pico8_mz_kz_dispatch above, reached by pico8_dg()'s own generic dispatch
 * once self.me=="pr" (after d_'s own self:ks(self.dq) transition). */
static pico8_value_t pico8_mz_pr_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_fixed_t t = pico8_fixed_from_int(0);
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_FIXED) {
        t = arg0.as.fixed;
    }
    pico8_mz_pr((pico8_instance_t *)self, t);
    return pico8_value_ref(NULL);
}

/* function mz:ej(t)
 * return min(sqrt(t/15),1)*self.oi
 * end
 * (pico8/lua.lua:794-796.) See pico8_wormhole.h's own comment on this
 * declaration for the full trace. Direct same-class helper, no dispatch
 * table -- called only by pico8_mz_pr above and pico8_mz_kj
 * (pico8_wormhole_display.c). */
pico8_fixed_t pico8_mz_ej(pico8_instance_t *self, pico8_fixed_t t) {
    pico8_fixed_t sq = pico8_fixed_sqrt(pico8_fixed_div(t, pico8_fixed_from_int(15)));
    pico8_fixed_t one = pico8_fixed_from_int(1);
    pico8_fixed_t min_val = (sq < one) ? sq : one;
    pico8_fixed_t oi = pico8_mz_field_fixed(self, "oi");
    return pico8_fixed_mul(min_val, oi);
}

/* Bridges to pico8_mz_kj's own signature (pico8_wormhole_display.c),
 * reading gfx/mem from the shared dispatch context -- the same split as
 * pico8_jo_kj_dispatch (pico8_planet.c). `kj` IS one of bm's 5 real
 * dispatch names, so a future dc("kj") reaches this too; real call
 * convention is `qc[nt](qc,qc.ra)`, so `arg0` (a REF to a heap
 * pico8_vec_t) is read as `p`, matching pico8_jo_kj_dispatch's own
 * identical treatment exactly. */
static pico8_value_t pico8_mz_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_vec_t p = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        p = *(const pico8_vec_t *)arg0.as.ref;
    }
    pico8_mz_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_instance_t *)self, p);
    return pico8_value_ref(NULL);
}

/* mz=ke:jr([[
 * nu=o("mz"),
 * me="kz",dw=0,
 * dq="pr",
 * spawn_snd=52,ik=49,
 * oi=0,
 * od=3,fp=1,
 * fa=5,
 * kk=o(7,7,6,13,5,1,1,1,0),
 * jj=o(qs="-too far-"),
 * ]])mz.hq=bv
 * mz.ev=dl
 * (pico8/lua.lua:752-761.) See pico8_wormhole.h's own top comment for the
 * full field-by-field trace and the bm-vs-mv()-routed-vs-pq classification
 * of every one of `mz`'s own real dispatch fields. Builds `g_mz_jj_wrapper`
 * right after the class itself -- see this file's own top comment and
 * pico8_wormhole.h's "SELF.ET REPRESENTATION" section for why this wrapper
 * exists and why its own `->fields` is BORROWED (read directly out of the
 * freshly-declared class's own "jj" default) rather than copied. */
void pico8_wormhole_init(void) {
    pico8_ob_table_t *jj_table = NULL;
    pico8_value_t jj_val;

    pico8_wormhole_shutdown();

    g_mz_class = pico8_class_declare(pico8_ke_class(),
                                      "\nnu=o(\"mz\"),\nme=\"kz\",dw=0,\ndq=\"pr\",\nspawn_snd=52,ik=49,\noi=0,"
                                      "\nod=3,fp=1,\nfa=5,\nkk=o(7,7,6,13,5,1,1,1,0),\njj=o(qs=\"-too far-\"),\n");
    pico8_class_set_pq(g_mz_class, pico8_mz_pq);
    pico8_ob_table_set_name(g_mz_class->defaults, "hq", pico8_value_function(pico8_mz_hq_dispatch));
    pico8_ob_table_set_name(g_mz_class->defaults, "ev", pico8_value_function(pico8_dl_dispatch));
    pico8_ob_table_set_name(g_mz_class->defaults, "kz", pico8_value_function(pico8_mz_kz_dispatch));
    pico8_ob_table_set_name(g_mz_class->defaults, "ey", pico8_value_function(pico8_mz_ey_dispatch));
    pico8_ob_table_set_name(g_mz_class->defaults, "pr", pico8_value_function(pico8_mz_pr_dispatch));
    pico8_ob_table_set_name(g_mz_class->defaults, "kj", pico8_value_function(pico8_mz_kj_dispatch));
    pico8_class_register_named("mz", g_mz_class);

    if (pico8_ob_table_get_name(g_mz_class->defaults, "jj", &jj_val) && jj_val.kind == PICO8_VALUE_REF &&
        jj_val.as.ref != NULL) {
        jj_table = (pico8_ob_table_t *)jj_val.as.ref;
    }
    g_mz_jj_wrapper = malloc(sizeof *g_mz_jj_wrapper);
    g_mz_jj_wrapper->fields = jj_table;
    g_mz_jj_wrapper->cls = NULL;
}

/* Frees the class itself, and ONLY the wrapper `pico8_instance_t` struct
 * (`free()`, never `pico8_instance_free()`, which would incorrectly also
 * free the wrapper's own BORROWED `->fields` -- `mz.jj`'s own real table,
 * still owned by the class's own defaults for the whole class lifetime --
 * see pico8_wormhole.h's own "SELF.ET REPRESENTATION" section). */
void pico8_wormhole_shutdown(void) {
    if (g_mz_class != NULL) {
        pico8_class_free(g_mz_class);
        g_mz_class = NULL;
    }
    if (g_mz_jj_wrapper != NULL) {
        free(g_mz_jj_wrapper);
        g_mz_jj_wrapper = NULL;
    }
}

pico8_class_t *pico8_mz_class(void) {
    return g_mz_class;
}

pico8_instance_t *pico8_mz_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_mz_class, overrides);
}

pico8_instance_t *pico8_mz_jj_wrapper(void) {
    return g_mz_jj_wrapper;
}
