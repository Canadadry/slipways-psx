#include "pico8_planet_trade.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "pico8_collect.h"
#include "pico8_entity.h"
#include "pico8_map_tables.h"
#include "pico8_planet.h"
#include "pico8_trade.h"
#include "pico8_ref_audit.h"
#include "pico8_slipway.h"

static int pico8_jo_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

/* Membership by trade equality (same endpoints), not pico8_gu's pointer
 * identity: every candidate offered here is a freshly built route, which
 * identity would never match. NULL-safe on `list`. */
static int gu_trade(const pico8_list_t *list, pico8_value_t v) {
    pico8_list_iter_t it;
    pico8_value_t e;

    if (list == NULL) {
        return 0;
    }
    it = pico8_list_all(list);
    while (pico8_list_iter_next(&it, &e)) {
        if (e.kind == PICO8_VALUE_REF && v.kind == PICO8_VALUE_REF &&
            pico8_trade_eq((const pico8_trade_t *)e.as.ref, (const pico8_trade_t *)v.as.ref)) {
            return 1;
        }
    }
    return 0;
}

static void pico8_jo_ib_kf_closure(pico8_jo_trade_proposal_t *self, pico8_value_t a) {
    if (self == NULL || self->ib == NULL) {
        return;
    }
    if (!gu_trade(self->ib, a)) {
        pico8_list_add(self->ib, a);
        self->mu -= pico8_fixed_from_int(1);
    }
}

void pico8_jo_kf(pico8_jo_t *self, pico8_struct_header_t *sw) {
    pico8_list_t *oe = NULL;
    pico8_struct_header_t *other = NULL;

    if (self == NULL) {
        return;
    }

    oe = self->oe;
    if (oe != NULL) {
        pico8_list_add(oe, pico8_value_ref(sw));
    }

    if (sw != NULL) {
        assert(sw->cls == pico8_jc_struct_class() && "jo:kf: sw is the slipway jc:ey registers");
        other = pico8_jc_eb((const pico8_jc_t *)sw, (const pico8_struct_header_t *)self);
    }
    if (other != NULL) {
        pico8_value_t fx_val;
        if (pico8_entity_get_field(other, "fx", &fx_val) &&
            pico8_jo_truthy(fx_val)) {
            self->fx = 1;
            self->has_fx = 1;
        }
    }

    pico8_jo_kd(self);
}

void pico8_jo_cn(pico8_jo_t *self, pico8_jo_trade_proposal_t *lw) {
    int qe;
    pico8_value_t r_e = pico8_value_ref(NULL);
    pico8_value_t r_q = pico8_value_ref(NULL);
    int have_r_e;
    int have_r_q;
    pico8_trade_t *t;
    pico8_value_t t_qe = pico8_value_ref(NULL);
    pico8_list_t *jy = NULL;
    pico8_list_iter_t it;
    pico8_value_t e;

    if (self == NULL || lw == NULL) {
        return;
    }
    t = lw->t;
    if (t != NULL) {
        t_qe = pico8_trade_qe(t);
    }

    have_r_e = pico8_r_lookup("e", &qe);
    if (have_r_e) {
        r_e = pico8_value_fixed(pico8_fixed_from_int(qe));
    }
    have_r_q = pico8_r_lookup("?", &qe);
    if (have_r_q) {
        r_q = pico8_value_fixed(pico8_fixed_from_int(qe));
    }

    jy = self->jy;

    if (have_r_e && pico8_value_equal(t_qe, r_e) && !self->has_jh) {
        it = pico8_list_all(jy);
        while (pico8_list_iter_next(&it, &e)) {
            if (e.kind == PICO8_VALUE_REF && e.as.ref != NULL) {
                const pico8_jy_t *i = e.as.ref;
                pico8_fixed_t gc = i->has_gc ? i->gc : pico8_fixed_from_int(0);
                int qe_is_r_q;

                qe_is_r_q = have_r_q && pico8_value_equal(pico8_jy_qe(i), r_q);
                if (gc == pico8_fixed_from_int(0) && !qe_is_r_q) {
                    pico8_list_del(jy, e);
                    break;
                }
            }
        }

        {
            pico8_fixed_t e_qe = r_e.as.fixed;
            pico8_jy_t *fresh = pico8_jy_new(&e_qe);
            pico8_list_t prefix;
            pico8_list_t merged;
            pico8_list_t *merged_heap;

            pico8_list_init(&prefix);
            pico8_list_add(&prefix, pico8_value_ref(fresh));
            merged = pico8_la(&prefix, jy);
            pico8_list_free(&prefix);

            merged_heap = malloc(sizeof *merged_heap);
            *merged_heap = merged;
            self->jy = merged_heap;
            jy = merged_heap;
        }
        self->jh = pico8_fixed_from_int(1);
        self->has_jh = 1;
    }

    it = pico8_list_all(jy);
    while (pico8_list_iter_next(&it, &e)) {
        pico8_jy_t *i;
        pico8_value_t qe_val;

        if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
            continue;
        }
        i = e.as.ref;
        qe_val = pico8_jy_qe(i);

        if (have_r_q && pico8_value_equal(qe_val, r_q)) {
            pico8_value_t ba_val;
            int accepted = 0;

            if (pico8_entity_get_field(self, "ba", &ba_val) && ba_val.kind == PICO8_VALUE_REF &&
                ba_val.as.ref != NULL) {
                accepted = t_qe.kind == PICO8_VALUE_FIXED &&
                           pico8_int_seq_has_fixed((const pico8_int_seq_t *)ba_val.as.ref, t_qe.as.fixed);
            }
            if (accepted) {
                assert(t_qe.kind == PICO8_VALUE_FIXED && "a want's qe is a resource of r, never a boolean");
                i->row.has_qe = 1;
                i->row.qe = t_qe.as.fixed;
                qe_val = t_qe;
            }
        }

        if (pico8_value_equal(t_qe, qe_val) && !(have_r_q && pico8_value_equal(qe_val, r_q))) {
            pico8_trade_t *extended = pico8_trade_kg(t, pico8_value_ref(self), pico8_value_ref(NULL));
            if (lw->kf != NULL) {
                lw->kf(lw, pico8_value_ref(extended));
            }
            return;
        }
    }
}

pico8_list_t *pico8_jo_ib(pico8_jo_t *self) {
    pico8_list_t *ib;
    pico8_trade_keys_t keys = {0};
    pico8_fixed_t mu = pico8_fixed_from_int(0);
    const pico8_list_t *oe = NULL;
    pico8_list_iter_t it;
    pico8_value_t w_val;

    if (self == NULL) {
        return NULL;
    }

    if (self->by != NULL) {
        ib = self->by;
    } else {
        ib = malloc(sizeof *ib);
        pico8_list_init(ib);
    }

    if (self->kw != NULL) {
        if (self->kw->has_qe) {
            keys.has_qe = 1;
            keys.qe = self->kw->qe;
        }
        mu = self->kw->mu;
    }

    oe = self->oe;

    it = pico8_list_all(oe);
    while (pico8_list_iter_next(&it, &w_val)) {
        pico8_struct_header_t *w;
        pico8_jo_trade_proposal_t lw;
        pico8_list_t *lp_list;
        pico8_list_t *rl_list;
        pico8_struct_header_t *other;

        if (w_val.kind != PICO8_VALUE_REF || w_val.as.ref == NULL) {
            continue;
        }
        w = (pico8_struct_header_t *)w_val.as.ref;

        lp_list = malloc(sizeof *lp_list);
        pico8_list_init(lp_list);
        pico8_list_add(lp_list, pico8_value_ref(self));
        rl_list = malloc(sizeof *rl_list);
        pico8_list_init(rl_list);
        pico8_list_add(rl_list, pico8_value_ref(w));

        keys.lp = lp_list;
        keys.rl = rl_list;
        lw.t = pico8_trade_new(&keys);
        /* Recomputed each iteration: an earlier proposal's `kf` may already
         * have grown `ib`. */
        lw.mu = mu - pico8_fixed_from_int(ib->count);
        lw.kf = pico8_jo_ib_kf_closure;
        lw.ib = ib;

        /* TROU-7 */
        if (lw.mu == pico8_fixed_from_int(0)) {
            break;
        }

        /* `lw` is on the stack: a callee that keeps the REF past this
         * iteration holds a dangling pointer. `nr:cn` copies it instead. */
        assert(w->cls == pico8_jc_struct_class() && "jo.oe holds slipways only: jo:kf adds to it");
        other = pico8_jc_eb((const pico8_jc_t *)w, (const pico8_struct_header_t *)self);
        if (other != NULL && other->cls->cn != NULL) {
            other->cls->cn(other, &lw);
        }
    }

    return ib;
}
