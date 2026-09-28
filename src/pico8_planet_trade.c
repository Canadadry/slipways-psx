#include "pico8_planet_trade.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "pico8_collect.h"
#include "pico8_entity.h"
#include "pico8_ob.h"
#include "pico8_planet.h"
#include "pico8_stash.h"
#include "pico8_trade.h"
#include "pico8_ref_audit.h"

static int pico8_jo_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

/* The returned list is by value and borrows its values: the caller frees
 * it with pico8_list_free. */
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
    pico8_value_t eb_result;

    if (self == NULL) {
        return;
    }

    oe = self->oe;
    if (oe != NULL) {
        pico8_list_add(oe, pico8_value_ref(sw));
    }

    if (sw != NULL && pico8_mv(sw, "eb", 1, pico8_value_ref(self), pico8_value_ref(NULL), &eb_result) &&
        eb_result.kind == PICO8_VALUE_REF && eb_result.as.ref != NULL) {
        pico8_value_t fx_val;
        if (pico8_entity_get_field((const pico8_struct_header_t *)eb_result.as.ref, "fx", &fx_val) &&
            pico8_jo_truthy(fx_val)) {
            self->fx = 1;
            self->has_fx = 1;
        }
    }

    pico8_jo_kd(self);
}

void pico8_jo_cn(pico8_jo_t *self, pico8_jo_trade_proposal_t *lw) {
    const pico8_ob_table_t *r;
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

    r = pico8_r();
    have_r_e = pico8_ob_table_get_name(r, "e", &r_e);
    have_r_q = pico8_ob_table_get_name(r, "?", &r_q);

    jy = self->jy;

    if (have_r_e && pico8_value_equal(t_qe, r_e) && !self->has_jh) {
        it = pico8_list_all(jy);
        while (pico8_list_iter_next(&it, &e)) {
            if (e.kind == PICO8_VALUE_REF && e.as.ref != NULL) {
                const pico8_ob_table_t *i_table;
                pico8_value_t gc_val;
                pico8_value_t qe_val;
                pico8_fixed_t gc = pico8_fixed_from_int(0);
                int qe_is_r_q;

                assert(pico8_type_tag_of(e.as.ref) == PICO8_TYPE_TAG_OB_TABLE &&
                       "a jy entry read as a want record is not a table");
                i_table = (const pico8_ob_table_t *)e.as.ref;

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
            self->jy = merged_heap;
            jy = merged_heap;
        }
        self->jh = pico8_fixed_from_int(1);
        self->has_jh = 1;
    }

    it = pico8_list_all(jy);
    while (pico8_list_iter_next(&it, &e)) {
        pico8_ob_table_t *i_table;
        pico8_value_t qe_val;
        int have_qe;

        if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
            continue;
        }
        assert(pico8_type_tag_of(e.as.ref) == PICO8_TYPE_TAG_OB_TABLE &&
               "a jy entry read as a want record is not a table");
        i_table = (pico8_ob_table_t *)e.as.ref;
        have_qe = pico8_ob_table_get_name(i_table, "qe", &qe_val);

        if (have_qe && have_r_q && pico8_value_equal(qe_val, r_q)) {
            pico8_value_t ba_val;
            const pico8_ob_table_t *ba_table = NULL;
            int accepted = 0;

            if (pico8_entity_get_field(self, "ba", &ba_val) && ba_val.kind == PICO8_VALUE_REF &&
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
    pico8_value_t qe = pico8_value_ref(NULL);
    pico8_fixed_t mu = pico8_fixed_from_int(0);
    const pico8_list_t *oe = NULL;
    pico8_list_iter_t it;
    pico8_value_t w_val;

    if (self == NULL) {
        return NULL;
    }

    if (self->by.kind == PICO8_VALUE_REF && self->by.as.ref != NULL) {
        ib = PICO8_REF_AS_LIST(self->by.as.ref);
    } else {
        ib = malloc(sizeof *ib);
        pico8_list_init(ib);
    }

    if (self->kw != NULL) {
        const pico8_ob_table_t *kw = self->kw;
        pico8_value_t mu_val;

        if (!pico8_ob_table_get_name(kw, "qe", &qe)) {
            qe = pico8_value_ref(NULL);
        }
        if (pico8_ob_table_get_name(kw, "mu", &mu_val) && mu_val.kind == PICO8_VALUE_FIXED) {
            mu = mu_val.as.fixed;
        }
    }

    oe = self->oe;

    it = pico8_list_all(oe);
    while (pico8_list_iter_next(&it, &w_val)) {
        pico8_struct_header_t *w;
        pico8_jo_trade_proposal_t lw;
        pico8_ob_table_t *overrides;
        pico8_list_t *lp_list;
        pico8_list_t *rl_list;
        pico8_value_t eb_result;

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

        overrides = pico8_ob_table_new();
        pico8_ob_table_set_name(overrides, "lp", pico8_value_ref(lp_list));
        pico8_ob_table_set_name(overrides, "rl", pico8_value_ref(rl_list));
        pico8_ob_table_set_name(overrides, "qe", qe);

        lw.t = pico8_trade_new(overrides);
        pico8_ob_table_free(overrides);
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
        if (pico8_mv(w, "eb", 1, pico8_value_ref(self), pico8_value_ref(NULL), &eb_result) &&
            eb_result.kind == PICO8_VALUE_REF && eb_result.as.ref != NULL) {
            pico8_value_t discard;
            pico8_mv((pico8_struct_header_t *)eb_result.as.ref, "cn", 1, pico8_value_ref(&lw), pico8_value_ref(NULL),
                     &discard);
        }
    }

    return ib;
}
