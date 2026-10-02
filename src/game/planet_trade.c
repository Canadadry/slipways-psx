#include "game/planet_trade.h"

#include <assert.h>
#include <stddef.h>

#include "engine/collect.h"
#include "game/map_tables.h"
#include "game/planet.h"
#include "game/trade.h"
#include "game/slipway.h"
#include "engine/frame_zone.h"
#include "engine/game_zone.h"

/* Membership by trade equality (same endpoints), not pico8_ents_has's pointer
 * identity: every candidate offered here is a freshly built route, which
 * identity would never match. NULL-safe on `list`. */
static int gu_trade(const entity_list_t *list, const pico8_trade_t *v) {
    entity_list_iter_t it;
    entity_t *e;

    if (list == NULL) {
        return 0;
    }
    it = entity_list_all(list);
    while (entity_list_iter_next(&it, &e)) {
        if (pico8_trade_eq((const pico8_trade_t *)e, v)) {
            return 1;
        }
    }
    return 0;
}

static void pico8_jo_ib_kf_closure(pico8_jo_trade_proposal_t *self, pico8_trade_t *a) {
    if (self == NULL || self->ib == NULL) {
        return;
    }
    if (!gu_trade(self->ib, a)) {
        assert(self->ib->zone == game_zone() &&
               "jo:ib: a kept route is copied into the generation; a list of another zone would outlive it");
        entity_list_add(self->ib, &pico8_trade_copy(game_zone(), a)->header);
        self->mu -= p8_fixed_from_int(1);
    }
}

void pico8_jo_kf(pico8_jo_t *self, entity_t *sw) {
    entity_list_t *oe = NULL;
    entity_t *other = NULL;

    if (self == NULL) {
        return;
    }

    oe = self->oe;
    if (oe != NULL) {
        entity_list_add(oe, sw);
    }

    if (sw != NULL) {
        assert(sw->class == pico8_jc_struct_class() && "jo:kf: sw is the slipway jc:ey registers");
        other = pico8_jc_eb((const pico8_jc_t *)sw, (const entity_t *)self);
    }
    if (other != NULL) {
        assert(pico8_jo_is_family(other) && "jo:kf: the other end of a registered slipway is a planet");
        if (((const pico8_jo_t *)other)->fx != 0) {
            self->fx = p8_fixed_from_int(1);
        }
    }

    pico8_jo_kd(self);
}

void pico8_jo_cn(pico8_jo_t *self, pico8_jo_trade_proposal_t *lw) {
    int qe;
    pico8_qe_t r_e = {.kind = PICO8_QE_NIL};
    pico8_qe_t r_q = {.kind = PICO8_QE_NIL};
    int have_r_e;
    int have_r_q;
    pico8_trade_t *t;
    pico8_qe_t t_qe = {.kind = PICO8_QE_NIL};
    pointer_list_t *jy = NULL;
    pointer_list_iter_t it;
    void *e;

    if (self == NULL || lw == NULL) {
        return;
    }
    t = lw->t;
    if (t != NULL) {
        t_qe = pico8_trade_qe(t);
    }

    have_r_e = pico8_r_lookup("e", &qe);
    if (have_r_e) {
        r_e = (pico8_qe_t){.kind = PICO8_QE_FIXED, .n = p8_fixed_from_int(qe)};
    }
    have_r_q = pico8_r_lookup("?", &qe);
    if (have_r_q) {
        r_q = (pico8_qe_t){.kind = PICO8_QE_FIXED, .n = p8_fixed_from_int(qe)};
    }

    jy = self->jy;

    if (have_r_e && pico8_qe_equal(t_qe, r_e) && !self->has_jh) {
        it = pointer_list_all(jy);
        while (pointer_list_iter_next(&it, &e)) {
            if (e != NULL) {
                const pico8_jy_t *i = e;
                p8_fixed_t gc = i->has_gc ? i->gc : p8_fixed_from_int(0);
                int qe_is_r_q;

                qe_is_r_q = have_r_q && pico8_qe_equal(pico8_jy_qe(i), r_q);
                if (gc == p8_fixed_from_int(0) && !qe_is_r_q) {
                    pointer_list_del(jy, e);
                    break;
                }
            }
        }

        {
            p8_fixed_t e_qe = r_e.n;
            pico8_jy_t *fresh = pico8_jy_new(game_zone(), &e_qe);
            pointer_list_t prefix;
            pointer_list_t merged;
            pointer_list_t *merged_own;

            pointer_list_init(&prefix, frame_zone());
            pointer_list_add(&prefix, fresh);
            merged = collect_concat_pointer_list(game_zone(), &prefix, jy);

            merged_own = (pointer_list_t *)allocator_alloc(game_zone(), sizeof *merged_own);
            *merged_own = merged;
            self->jy = merged_own;
            jy = merged_own;
        }
        self->jh = p8_fixed_from_int(1);
        self->has_jh = 1;
    }

    it = pointer_list_all(jy);
    while (pointer_list_iter_next(&it, &e)) {
        pico8_jy_t *i;
        pico8_qe_t qe_val;

        if (e == NULL) {
            continue;
        }
        i = e;
        qe_val = pico8_jy_qe(i);

        if (have_r_q && pico8_qe_equal(qe_val, r_q)) {
            int accepted = 0;

            if ((self->header.class->lineage & LINEAGE(LINEAGE_STRUCTURE)) && self->def->ba != NULL) {
                accepted = t_qe.kind == PICO8_QE_FIXED && pico8_int_seq_has_fixed(self->def->ba, t_qe.n);
            }
            if (accepted) {
                assert(t_qe.kind == PICO8_QE_FIXED && "a want's qe is a resource of r, never a boolean");
                i->row.has_qe = 1;
                i->row.qe = t_qe.n;
                qe_val = t_qe;
            }
        }

        if (pico8_qe_equal(t_qe, qe_val) && !(have_r_q && pico8_qe_equal(qe_val, r_q))) {
            pico8_trade_t *extended = pico8_trade_kg(frame_zone(), t, &self->header, NULL);
            if (lw->kf != NULL) {
                lw->kf(lw, extended);
            }
            return;
        }
    }
}

entity_list_t *pico8_jo_ib(pico8_jo_t *self) {
    entity_list_t *ib;
    pico8_trade_keys_t keys = {0};
    p8_fixed_t mu = p8_fixed_from_int(0);
    const entity_list_t *oe = NULL;
    entity_list_iter_t it;
    entity_t *w_val;

    if (self == NULL) {
        return NULL;
    }

    if (self->by != NULL) {
        ib = self->by;
    } else {
        ib = (entity_list_t *)allocator_alloc(game_zone(), sizeof *ib);
        entity_list_init(ib, game_zone());
    }

    if (self->kw != NULL) {
        if (self->kw->has_qe) {
            keys.has_qe = 1;
            keys.qe = self->kw->qe;
        }
        mu = self->kw->mu;
    }

    oe = self->oe;

    it = entity_list_all(oe);
    while (entity_list_iter_next(&it, &w_val)) {
        entity_t *w;
        pico8_jo_trade_proposal_t lw;
        entity_list_t *lp_list;
        entity_list_t *rl_list;
        entity_t *other;

        if (w_val == NULL) {
            continue;
        }
        w = w_val;

        lp_list = (entity_list_t *)allocator_alloc(frame_zone(), sizeof *lp_list);
        entity_list_init(lp_list, frame_zone());
        entity_list_add(lp_list, &self->header);
        rl_list = (entity_list_t *)allocator_alloc(frame_zone(), sizeof *rl_list);
        entity_list_init(rl_list, frame_zone());
        entity_list_add(rl_list, w);

        keys.lp = lp_list;
        keys.rl = rl_list;
        lw.t = pico8_trade_new(frame_zone(), &keys);
        /* Recomputed each iteration: an earlier proposal's `kf` may already
         * have grown `ib`. */
        lw.mu = mu - p8_fixed_from_int(ib->count);
        lw.kf = pico8_jo_ib_kf_closure;
        lw.ib = ib;

        if (lw.mu == p8_fixed_from_int(0)) {
            break;
        }

        /* `lw` is on the stack: a callee that keeps the REF past this
         * iteration holds a dangling pointer. `nr:cn` copies it instead. */
        assert(w->class == pico8_jc_struct_class() && "jo.oe holds slipways only: jo:kf adds to it");
        other = pico8_jc_eb((const pico8_jc_t *)w, (const entity_t *)self);
        if (other != NULL && other->class->offer_trade != NULL) {
            other->class->offer_trade(other, &lw);
        }
    }

    return ib;
}

void pico8_jo_kf_dispatch(entity_t *self, entity_t *sw) {
    pico8_jo_kf((pico8_jo_t *)self, sw);
}

void pico8_jo_cn_dispatch(entity_t *self, struct pico8_jo_trade_proposal *lw) {
    pico8_jo_cn((pico8_jo_t *)self, lw);
}
