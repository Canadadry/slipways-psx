#include "pico8_bg.h"

#include <stddef.h>
#include <stdlib.h>

#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_fixed.h"
#include "pico8_gen.h"
#include "pico8_ke.h"
#include "pico8_vec.h"

/* See pico8_bg.h's own top comment for the full source/trace. */

static pico8_class_t *g_bg_class = NULL;

/* Truncating fixed->int conversion, same convention as e.g.
 * pico8_ui_button.c's own pico8_ui_to_int -- a private per-module copy,
 * not shared/exported, matching this port's established reasoning for not
 * widening another module's header surface for a few-line helper. */
static int pico8_bg_to_int(pico8_fixed_t x) {
    return (int)pico8_fixed_to_double(x);
}

/* Defensive vector-field reader, same shape as pico8_scroll.c's own
 * pico8_qb_field_vec -- private per-module copy, same reasoning. */
static pico8_vec_t pico8_bg_field_vec(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static pico8_vec_t pico8_bg_value_vec(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

/* Bridges pico8_class_pq_fn's fixed shape to pico8_bg_pq's own real
 * signature -- see pico8_bg.h's own comment on pico8_bg_pq. */
static void pico8_bg_pq_dispatch(pico8_instance_t *instance) {
    pico8_bg_pq(pico8_dispatch_mem(), instance);
}

/* Bridges pico8_value_function_t's (self,argc,arg0,arg1) shape to
 * pico8_bg_kj's own signature; dc()'s own "kj" dispatch passes argc=1 with
 * `qc.ra` as arg0, unused (bg:kj's real body never references it). */
static pico8_value_t pico8_bg_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_bg_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (const pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

/* Bridges to pico8_bg_ldown's own signature; `ui:jz(rm,...)`'s own
 * `mv(h,rm,...)` dispatch passes argc=2 (self.qa, je.ra) here -- only
 * arg0 (mp) is forwarded, matching bg:ldown's own declared arity; arg1 is
 * unused (bg:ldown's real body references neither parameter anyway). */
static pico8_value_t pico8_bg_ldown_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg1;
    return pico8_bg_ldown((pico8_instance_t *)self, arg0);
}

/* Bridges to pico8_bg_rdown's own signature; same argc=2 real call shape
 * as ldown above. */
static pico8_value_t pico8_bg_rdown_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    return pico8_bg_rdown((pico8_instance_t *)self, arg0, arg1);
}

/* Bridges to pico8_bg_hq's own signature; `ui:bh()`'s own
 * `mv(held,"hq",self.qa,je.ra)` dispatch passes argc=2 every frame the
 * button stays held. */
static pico8_value_t pico8_bg_hq_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    pico8_bg_hq((pico8_instance_t *)self, arg0, arg1);
    return pico8_value_ref(NULL);
}

/* bg=ke:jr([[
 * fa=1,
 * hd=1,
 * pl=o(0,0,0,72,73,74,75,88,89,90,91,88,91,89,91),
 * ]]) -- see pico8_bg.h's own top comment for the full trace. No
 * pico8_pi_init() call needed here (unlike pico8_mouse_reader.c/
 * pico8_scroll.c's own nf/qb): bg's spec has no nested `v(...)` constructor
 * call for `pi`'s char-keyed registry to serve -- `pl=o(...)`'s own
 * entries are all plain numbers. */
void pico8_bg_init(void) {
    pico8_bg_shutdown();
    g_bg_class = pico8_class_declare(
        pico8_ke_class(), "\nfa=1,hd=1,pl=o(0,0,0,72,73,74,75,88,89,90,91,88,91,89,91),\n");
    pico8_class_set_pq(g_bg_class, pico8_bg_pq_dispatch);
    pico8_ob_table_set_name(g_bg_class->defaults, "kj", pico8_value_function(pico8_bg_kj_dispatch));
    pico8_ob_table_set_name(g_bg_class->defaults, "ldown", pico8_value_function(pico8_bg_ldown_dispatch));
    pico8_ob_table_set_name(g_bg_class->defaults, "rdown", pico8_value_function(pico8_bg_rdown_dispatch));
    pico8_ob_table_set_name(g_bg_class->defaults, "hq", pico8_value_function(pico8_bg_hq_dispatch));
}

void pico8_bg_shutdown(void) {
    if (g_bg_class != NULL) {
        pico8_class_free(g_bg_class);
        g_bg_class = NULL;
    }
}

pico8_class_t *pico8_bg_class(void) {
    return g_bg_class;
}

pico8_instance_t *pico8_bg_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_bg_class, overrides);
}

/* function bg:pq()local vs,vh,oh,m=
 * {},0,jf(0.1,0)for y=0,31 do
 * for x=0,31 do
 * vh=oh()*8-4+
 * (vh+(vs[x]or 1))*0.5
 * vs[x]=vh
 * mset(x,y,rnd()<0.004
 * and 122+rnd(5)or bg.pl[flr(vh+rnd(4))])end
 * end
 * end
 * (pico8/lua.lua:363-372.) See pico8_bg.h's own comment on this
 * declaration for the full trace. `vs`/`vs_set` model Lua's own `vs[x]or 1`
 * fallback (only ever taken during row y=0, since every later row already
 * wrote `vs[x]` on the previous pass); `m` (the real source's own dead,
 * never-read 4th local) is simply never declared here, same observable
 * effect. `bg.pl` is read from `g_bg_class->defaults` directly (the CLASS
 * table, matching the real source's own `bg.pl`, not `self.pl`). */
void pico8_bg_pq(pico8_mem_t *mem, pico8_instance_t *self) {
    pico8_fixed_t vs[32];
    int vs_set[32];
    pico8_fixed_t vh = pico8_fixed_from_int(0);
    pico8_jf_state_t *oh;
    const pico8_ob_table_t *pl_table = NULL;
    pico8_value_t pl_val;
    int x, y;

    (void)self;

    if (mem == NULL || g_bg_class == NULL) {
        return;
    }

    for (x = 0; x < 32; x++) {
        vs_set[x] = 0;
    }

    if (pico8_ob_table_get_name(g_bg_class->defaults, "pl", &pl_val) && pl_val.kind == PICO8_VALUE_REF &&
        pl_val.as.ref != NULL) {
        pl_table = (const pico8_ob_table_t *)pl_val.as.ref;
    }

    oh = pico8_jf(pico8_fixed_from_double(0.1), pico8_fixed_from_int(0));

    for (y = 0; y <= 31; y++) {
        for (x = 0; x <= 31; x++) {
            pico8_fixed_t prev = vs_set[x] ? vs[x] : pico8_fixed_from_int(1);
            pico8_fixed_t step =
                pico8_fixed_sub(pico8_fixed_mul(pico8_jf_next(oh), pico8_fixed_from_int(8)), pico8_fixed_from_int(4));
            pico8_fixed_t roll;
            int tile;

            vh = pico8_fixed_add(step, pico8_fixed_mul(pico8_fixed_add(vh, prev), pico8_fixed_from_double(0.5)));
            vs[x] = vh;
            vs_set[x] = 1;

            roll = pico8_fixed_rnd_default();
            if (roll < pico8_fixed_from_double(0.004)) {
                tile = 122 + pico8_bg_to_int(pico8_fixed_rnd(pico8_fixed_from_int(5)));
            } else {
                pico8_fixed_t idx_f = pico8_fixed_flr(pico8_fixed_add(vh, pico8_fixed_rnd(pico8_fixed_from_int(4))));
                int idx = pico8_bg_to_int(idx_f);
                pico8_value_t pl_entry;

                tile = 0;
                if (pl_table != NULL && pico8_ob_table_get_index(pl_table, idx, &pl_entry) &&
                    pl_entry.kind == PICO8_VALUE_FIXED) {
                    tile = pico8_bg_to_int(pl_entry.as.fixed);
                }
            }

            pico8_mset(mem, x, y, tile);
        }
    }

    pico8_jf_free(oh);
}

/* function bg:kj()map(0,0,0,0,32,32)fillp(0b1010010110100101)for xy=3,262,32 do
 * rectfill(xy,0,xy,262,1)rectfill(0,xy,262,xy)end
 * fillp()rect(0,0,262,262,5)end
 * (pico8/lua.lua:373-375.) See pico8_bg.h's own comment on this
 * declaration for the full trace, including why the second rectfill's
 * omitted color argument is resolved to `1` here (the statically-known
 * "last color drawn" at this exact call site, not a guess). Binary literal
 * `0b1010010110100101` = `0xA5A5`, converted to hex for the C call while
 * this comment keeps the exact Lua source above. */
void pico8_bg_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self) {
    int xy;

    (void)self;

    if (gfx == NULL || mem == NULL) {
        return;
    }

    pico8_map(gfx, mem, 0, 0, 0, 0, 32, 32);
    pico8_fillp(gfx, 0xA5A5u);
    for (xy = 3; xy <= 262; xy += 32) {
        pico8_rectfill(gfx, mem, xy, 0, xy, 262, 1);
        pico8_rectfill(gfx, mem, 0, xy, 262, xy, 1);
    }
    pico8_fillp_reset(gfx);
    pico8_rect(gfx, mem, 0, 0, 262, 262, 5);
}

/* function bg:ldown(mp)
 * return (#(cg.g_ or {})>0 and mz or g_)()end
 * (pico8/lua.lua:376-377.) See pico8_bg.h's own comment on this
 * declaration for the full forward-reference trace (pico8_class_lookup_named,
 * pico8_class.h). Returns NULL-ref ("nothing held") if the looked-up class
 * hasn't registered itself yet -- today's real state, before Chunks 10/16
 * land. */
pico8_value_t pico8_bg_ldown(pico8_instance_t *self, pico8_value_t mp) {
    const pico8_list_t *g_tag;
    int count;
    pico8_class_t *cls;
    pico8_instance_t *made;

    (void)self;
    (void)mp;

    g_tag = pico8_cg("g_");
    count = (g_tag != NULL) ? g_tag->count : 0;
    cls = pico8_class_lookup_named(count > 0 ? "mz" : "g_");

    if (cls == NULL) {
        return pico8_value_ref(NULL);
    }

    made = pico8_class_instantiate(cls, NULL);
    return pico8_value_ref(made);
}

/* function bg:rdown(mp,rp)self.mq=rp
 * return self
 * end
 * (pico8/lua.lua:378-380.) See pico8_bg.h's own comment on this
 * declaration. */
pico8_value_t pico8_bg_rdown(pico8_instance_t *self, pico8_value_t mp, pico8_value_t rp) {
    (void)mp;

    if (self != NULL) {
        pico8_ob_table_set_name(self->fields, "mq", rp);
    }
    return pico8_value_ref(self);
}

/* function bg:hq(mp,rp)mo.v=self.mq-rp
 * self.mq=rp
 * end
 * (pico8/lua.lua:381-383.) See pico8_bg.h's own comment on this
 * declaration. `self.mq` (the OLD value) is read into `mq` before being
 * overwritten below, preserving the real source's own evaluation order. */
void pico8_bg_hq(pico8_instance_t *self, pico8_value_t mp, pico8_value_t rp) {
    pico8_vec_t mq;
    pico8_vec_t rp_vec;
    pico8_instance_t *mo;

    (void)mp;

    if (self == NULL) {
        return;
    }

    mq = pico8_bg_field_vec(self, "mq");
    rp_vec = pico8_bg_value_vec(rp);
    mo = pico8_mo();
    if (mo != NULL) {
        pico8_vec_t *v_new = malloc(sizeof *v_new);
        *v_new = pico8_vec_sub(mq, rp_vec);
        pico8_ob_table_set_name(mo->fields, "v", pico8_value_ref(v_new));
    }

    pico8_ob_table_set_name(self->fields, "mq", rp);
}
