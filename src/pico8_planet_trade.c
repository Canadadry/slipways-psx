#include "pico8_planet_trade.h"

#include <stddef.h>
#include <stdlib.h>

#include "pico8_collect.h"
#include "pico8_entity.h"
#include "pico8_ob.h"
#include "pico8_planet.h"
#include "pico8_stash.h"
#include "pico8_trade.h"

/* Phase 3, Chunk 13, FOURTH sub-lot (docs/prd/triage/phase3-etape1-
 * function-by-function-c-port.md): jo:kf/jo:cn/jo:ib, the jo-side of the
 * mutually-recursive trade cluster. See pico8_planet_trade.h's own top
 * comment and each function's own comment there for the full Lua-source
 * trace; this file carries the port itself. pico8_jo_truthy is this
 * module's own private copy of the same truthiness rule already
 * independently duplicated by pico8_planet.c/pico8_planet_display.c/
 * pico8_economy*.c/etc (documented, accepted duplication, not a new
 * pattern). */
static int pico8_jo_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

/* Not a port of any named Lua function: private per-module bridge from
 * "positional entries of a pico8_ob_table_t" to a pico8_list_t (used here
 * for jo:cn's own `gu(self.ba,t.qe)`) -- the same established pattern
 * already independently duplicated by pico8_jo_positional_values
 * (pico8_planet_display.c)/pico8_research_display_positional_values/
 * pico8_layout_positional_values/pico8_msgbox_positional_values/
 * pico8_data_init_positional_values (documented, accepted duplication). */
static pico8_list_t positional_values(const pico8_ob_table_t *table) {
    pico8_list_t list;
    int i;

    pico8_list_init(&list);
    if (table == NULL) {
        return list;
    }
    for (i = 0; i < table->count; i++) {
        if (table->entries[i].key.kind == PICO8_OB_KEY_INDEX) {
            pico8_list_add(&list, table->entries[i].value);
        }
    }
    return list;
}

/* Resolves the documented gap pico8_trade.h's own top comment flags
 * ("a future chunk that needs gu()/del()-style membership on a list of
 * trade instances needs its own pico8_trade_eq-aware sibling ... not a
 * silent reuse of the generic one") -- this IS that future chunk, at
 * jo:ib's own `gu(ib,a)` call site (below), which really does need
 * "same start and end planet" trade equality (pico8_trade_eq, Chunk 3),
 * not pico8_gu's own pointer-identity rule (every `a` here is a BRAND
 * NEW trade instance from t:kg(), so pointer identity would never match
 * an already-accepted route with the same start/end). Matches
 * pico8_gu_content's own precedent (pico8_collect.h) of a small,
 * type-aware sibling rather than widening the generic pico8_gu. NULL-safe
 * on `list`, matching pico8_gu's own convention. */
static int gu_trade(const pico8_list_t *list, pico8_value_t v) {
    pico8_list_iter_t it;
    pico8_value_t e;

    if (list == NULL) {
        return 0;
    }
    it = pico8_list_all(list);
    while (pico8_list_iter_next(&it, &e)) {
        if (e.kind == PICO8_VALUE_REF && v.kind == PICO8_VALUE_REF &&
            pico8_trade_eq((const pico8_instance_t *)e.as.ref, (const pico8_instance_t *)v.as.ref)) {
            return 1;
        }
    }
    return 0;
}

/* `function(self,a)if not gu(ib,a)then add(ib,a)self.mu-=1 end end`
 * (pico8/lua.lua:505-509) -- the real body of jo:ib's own local closure
 * field, assigned as `lw.kf` by pico8_jo_ib below. See
 * pico8_planet_trade.h's own top comment and pico8_jo_trade_proposal_kf_fn's
 * own comment for the full "why a plain struct + function pointer, not
 * the generic mv()-dispatch technique" trace. `self` is THIS proposal
 * (the struct itself, via `lw:kf(a)`'s own colon-call sugar), not the
 * enclosing planet. A NULL `self` or NULL `self->ib` is a safe no-op
 * (never a real case: pico8_jo_ib below always sets both before this is
 * ever reachable). */
static void pico8_jo_ib_kf_closure(pico8_jo_trade_proposal_t *self, pico8_value_t a) {
    if (self == NULL || self->ib == NULL) {
        return;
    }
    if (!gu_trade(self->ib, a)) {
        pico8_list_add(self->ib, a);
        self->mu -= pico8_fixed_from_int(1);
    }
}

/* function jo:kf(sw)add(self.oe,sw)
 * if (sw:eb(self).fx)self.fx=true
 * self:kd()end
 * (pico8/lua.lua:465-467.) See pico8_planet_trade.h's own comment on this
 * declaration for the full trace. */
void pico8_jo_kf(pico8_instance_t *self, pico8_instance_t *sw) {
    pico8_value_t oe_val;
    pico8_list_t *oe = NULL;
    pico8_value_t eb_result;

    if (self == NULL) {
        return;
    }

    if (pico8_instance_get_field(self, "oe", &oe_val) && oe_val.kind == PICO8_VALUE_REF && oe_val.as.ref != NULL) {
        oe = (pico8_list_t *)oe_val.as.ref;
    }
    if (oe != NULL) {
        pico8_list_add(oe, pico8_value_ref(sw));
    }

    if (sw != NULL && pico8_mv(sw, "eb", 1, pico8_value_ref(self), pico8_value_ref(NULL), &eb_result) &&
        eb_result.kind == PICO8_VALUE_REF && eb_result.as.ref != NULL) {
        pico8_value_t fx_val;
        if (pico8_instance_get_field((const pico8_instance_t *)eb_result.as.ref, "fx", &fx_val) &&
            pico8_jo_truthy(fx_val)) {
            pico8_ob_table_set_name(self->fields, "fx", pico8_value_bool(1));
        }
    }

    pico8_jo_kd(self);
}

/* function jo:cn(lw)local t=lw.t
 * if t.qe==r.e
 * and not self.jh then
 * for i in all(self.jy)do
 * if i.gc==0 and i.qe~=r["?"]then
 * del(self.jy,i)break
 * end
 * end
 * self.jy,self.jh=
 * la({{qe=r.e}},self.jy),1
 * end
 * for i in all(self.jy)do
 * if i.qe==r["?"]and gu(self.ba,t.qe)then
 * i.qe=t.qe
 * end
 * if t.qe==i.qe and i.qe~=r["?"]then
 * lw:kf(t:kg(self))
 * return
 * end
 * end
 * end
 * (pico8/lua.lua:479-499.) See pico8_planet_trade.h's own comment on this
 * declaration for the full trace. */
void pico8_jo_cn(pico8_instance_t *self, pico8_jo_trade_proposal_t *lw) {
    const pico8_ob_table_t *r;
    pico8_value_t r_e = pico8_value_ref(NULL);
    pico8_value_t r_q = pico8_value_ref(NULL);
    int have_r_e;
    int have_r_q;
    pico8_instance_t *t;
    pico8_value_t t_qe = pico8_value_ref(NULL);
    pico8_value_t jh_val;
    pico8_value_t jy_val;
    pico8_list_t *jy = NULL;
    pico8_list_iter_t it;
    pico8_value_t e;

    if (self == NULL || lw == NULL) {
        return;
    }
    t = lw->t;
    if (t != NULL) {
        pico8_instance_get_field(t, "qe", &t_qe);
    }

    r = pico8_r();
    have_r_e = pico8_ob_table_get_name(r, "e", &r_e);
    have_r_q = pico8_ob_table_get_name(r, "?", &r_q);

    if (pico8_instance_get_field(self, "jy", &jy_val) && jy_val.kind == PICO8_VALUE_REF && jy_val.as.ref != NULL) {
        jy = (pico8_list_t *)jy_val.as.ref;
    }

    if (have_r_e && pico8_value_equal(t_qe, r_e) &&
        !(pico8_instance_get_field(self, "jh", &jh_val) && pico8_jo_truthy(jh_val))) {
        it = pico8_list_all(jy);
        while (pico8_list_iter_next(&it, &e)) {
            if (e.kind == PICO8_VALUE_REF && e.as.ref != NULL) {
                const pico8_ob_table_t *i_table = (const pico8_ob_table_t *)e.as.ref;
                pico8_value_t gc_val;
                pico8_value_t qe_val;
                pico8_fixed_t gc = pico8_fixed_from_int(0);
                int qe_is_r_q;

                if (pico8_ob_table_get_name(i_table, "gc", &gc_val) && gc_val.kind == PICO8_VALUE_FIXED) {
                    gc = gc_val.as.fixed;
                }
                qe_is_r_q = have_r_q && pico8_ob_table_get_name(i_table, "qe", &qe_val) &&
                            pico8_value_equal(qe_val, r_q);
                if (gc == pico8_fixed_from_int(0) && !qe_is_r_q) {
                    pico8_list_del(jy, e);
                    break;
                }
            }
        }

        {
            pico8_ob_table_t *fresh = pico8_ob_table_new();
            pico8_list_t prefix;
            pico8_list_t merged;
            pico8_list_t *merged_heap;

            pico8_ob_table_set_name(fresh, "qe", r_e);
            pico8_list_init(&prefix);
            pico8_list_add(&prefix, pico8_value_ref(fresh));
            merged = pico8_la(&prefix, jy);
            pico8_list_free(&prefix);

            merged_heap = malloc(sizeof *merged_heap);
            *merged_heap = merged;
            pico8_ob_table_set_name(self->fields, "jy", pico8_value_ref(merged_heap));
            jy = merged_heap;
        }
        pico8_ob_table_set_name(self->fields, "jh", pico8_value_fixed(pico8_fixed_from_int(1)));
    }

    it = pico8_list_all(jy);
    while (pico8_list_iter_next(&it, &e)) {
        pico8_ob_table_t *i_table;
        pico8_value_t qe_val;
        int have_qe;

        if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
            continue;
        }
        i_table = (pico8_ob_table_t *)e.as.ref;
        have_qe = pico8_ob_table_get_name(i_table, "qe", &qe_val);

        if (have_qe && have_r_q && pico8_value_equal(qe_val, r_q)) {
            pico8_value_t ba_val;
            const pico8_ob_table_t *ba_table = NULL;
            int accepted = 0;

            if (pico8_instance_get_field(self, "ba", &ba_val) && ba_val.kind == PICO8_VALUE_REF &&
                ba_val.as.ref != NULL) {
                ba_table = (const pico8_ob_table_t *)ba_val.as.ref;
            }
            if (ba_table != NULL) {
                pico8_list_t ba_list = positional_values(ba_table);
                accepted = pico8_gu(&ba_list, t_qe);
                pico8_list_free(&ba_list);
            }
            if (accepted) {
                pico8_ob_table_set_name(i_table, "qe", t_qe);
                qe_val = t_qe;
                have_qe = 1;
            }
        }

        if (have_qe && pico8_value_equal(t_qe, qe_val) && !(have_r_q && pico8_value_equal(qe_val, r_q))) {
            pico8_instance_t *extended = pico8_trade_kg(t, pico8_value_ref(self), pico8_value_ref(NULL));
            if (lw->kf != NULL) {
                lw->kf(lw, pico8_value_ref(extended));
            }
            return;
        }
    }
}

/* function jo:ib()local ib=self.by or {}
 * local qe,mu=
 * self.kw.qe,self.kw.mu
 * for w in all(self.oe)do
 * local lw={t=trade({lp={self},rl={w},qe=qe
 * }),mu=mu-#ib,kf=function(self,a)if not gu(ib,a)then
 * add(ib,a)
 * self.mu-=1
 * end
 * end
 * }
 * if (lw.mu==0)break
 * w:eb(self):cn(lw)end
 * return ib
 * end
 * (pico8/lua.lua:500-514.) See pico8_planet_trade.h's own comment on this
 * declaration for the full trace. */
pico8_list_t *pico8_jo_ib(pico8_instance_t *self) {
    pico8_value_t by_val;
    pico8_list_t *ib;
    pico8_value_t kw_val;
    pico8_value_t qe = pico8_value_ref(NULL);
    pico8_fixed_t mu = pico8_fixed_from_int(0);
    pico8_value_t oe_val;
    const pico8_list_t *oe = NULL;
    pico8_list_iter_t it;
    pico8_value_t w_val;

    if (self == NULL) {
        return NULL;
    }

    if (pico8_instance_get_field(self, "by", &by_val) && by_val.kind == PICO8_VALUE_REF && by_val.as.ref != NULL) {
        ib = (pico8_list_t *)by_val.as.ref;
    } else {
        ib = malloc(sizeof *ib);
        pico8_list_init(ib);
    }

    if (pico8_instance_get_field(self, "kw", &kw_val) && kw_val.kind == PICO8_VALUE_REF && kw_val.as.ref != NULL) {
        const pico8_ob_table_t *kw = (const pico8_ob_table_t *)kw_val.as.ref;
        pico8_value_t mu_val;

        if (!pico8_ob_table_get_name(kw, "qe", &qe)) {
            qe = pico8_value_ref(NULL);
        }
        if (pico8_ob_table_get_name(kw, "mu", &mu_val) && mu_val.kind == PICO8_VALUE_FIXED) {
            mu = mu_val.as.fixed;
        }
    }

    if (pico8_instance_get_field(self, "oe", &oe_val) && oe_val.kind == PICO8_VALUE_REF && oe_val.as.ref != NULL) {
        oe = (const pico8_list_t *)oe_val.as.ref;
    }

    it = pico8_list_all(oe);
    while (pico8_list_iter_next(&it, &w_val)) {
        pico8_instance_t *w;
        pico8_jo_trade_proposal_t lw;
        pico8_ob_table_t *overrides;
        pico8_list_t *lp_list;
        pico8_list_t *rl_list;
        pico8_value_t eb_result;

        if (w_val.kind != PICO8_VALUE_REF || w_val.as.ref == NULL) {
            continue;
        }
        w = (pico8_instance_t *)w_val.as.ref;

        lp_list = malloc(sizeof *lp_list);
        pico8_list_init(lp_list);
        pico8_list_add(lp_list, pico8_value_ref(self));
        rl_list = malloc(sizeof *rl_list);
        pico8_list_init(rl_list);
        pico8_list_add(rl_list, pico8_value_ref(w));

        overrides = pico8_ob_table_new();
        pico8_ob_table_set_name(overrides, "lp", pico8_value_ref(lp_list));
        pico8_ob_table_set_name(overrides, "rl", pico8_value_ref(rl_list));
        pico8_ob_table_set_name(overrides, "qe", qe);

        lw.t = pico8_trade_new(overrides);
        pico8_ob_table_free(overrides);
        lw.mu = mu - pico8_fixed_from_int(ib->count);
        lw.kf = pico8_jo_ib_kf_closure;
        lw.ib = ib;

        if (lw.mu == pico8_fixed_from_int(0)) {
            break;
        }

        if (pico8_mv(w, "eb", 1, pico8_value_ref(self), pico8_value_ref(NULL), &eb_result) &&
            eb_result.kind == PICO8_VALUE_REF && eb_result.as.ref != NULL) {
            pico8_value_t discard;
            pico8_mv((pico8_instance_t *)eb_result.as.ref, "cn", 1, pico8_value_ref(&lw), pico8_value_ref(NULL),
                     &discard);
        }
    }

    return ib;
}
