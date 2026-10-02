#include "game/wormhole.h"

#include <assert.h>
#include <string.h>

#include "p8/audio.h"
#include "engine/entity_registry.h"
#include "engine/entity_geom.h"
#include "engine/entity.h"
#include "game/planet.h"
#include "game/targeting.h"
#include "game/tech_gate.h"
#include "engine/allocator.h"

static int g_mz_up = 0;

static const entity_class_t g_mz_jj_struct_class = {0};

/* The only one: never allocated, never freed; no writer, so a write
 * reaching it stops the run. */
static pico8_mz_jj_t g_mz_jj_wrapper = {
    {.class = &g_mz_jj_struct_class},
    pico8_mz_jj_qs,
};

static int pico8_mz_ancrage(const entity_t *p) {
    return p != NULL && p->fold_anchor != NULL && *p->fold_anchor != 0;
}

void pico8_mz_pq(pico8_mz_t *self) {
    const pico8_zone_choisie_t *zone = pico8_jo_zone_construction();
    pico8_ents_t *ox;
    pico8_ents_iter_t it;
    entity_t *p;
    int n = 0;

    if (self == NULL) {
        return;
    }

    pico8_rn(pico8_fixed_from_int(53));

    it = pico8_ents_all(pico8_hv());
    while (pico8_ents_iter_next(&it, &p)) {
        n += pico8_mz_ancrage(p);
    }
    ox = (pico8_ents_t *)pico8_zone_alloc(zone, sizeof *ox);
    pico8_ents_init_reserve(ox, zone, n);
    it = pico8_ents_all(pico8_hv());
    while (pico8_ents_iter_next(&it, &p)) {
        if (pico8_mz_ancrage(p)) {
            pico8_ents_add(ox, p);
        }
    }
    assert(ox->count == n && ox->capacity == n);
    self->ox = ox;
}

void pico8_mz_hq(pico8_mz_t *self, pico8_vec_t *mp, pico8_vec_t *rp) {
    pico8_vec_t p_vec;
    entity_t *nearest = NULL;
    pico8_fixed_t nq;
    pico8_fixed_t threshold;

    pico8_bv((entity_t *)self, mp, rp);
    if (self == NULL) {
        return;
    }

    p_vec = self->p != NULL ? *self->p : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    nq = pico8_ij_ents(p_vec, self->ox, &nearest);
    self->ij = nearest;

    threshold = pico8_ov("space folding", PICO8_FIXED_LIT(0x000030a3, 0.19), PICO8_FIXED_LIT(0x0000451e, 0.27));

    if (nq > threshold) {
        self->probe->et = (entity_t *)pico8_mz_jj_wrapper();
    }
}

static void pico8_mz_hq_dispatch(entity_t *self, pico8_vec_t *qa, pico8_vec_t *ra) {
    pico8_mz_hq((pico8_mz_t *)self, qa, ra);
}

void pico8_mz_kz(pico8_mz_t *self, pico8_fixed_t t) {
    pico8_fixed_t threshold;
    pico8_fixed_t one;
    pico8_fixed_t td15;
    pico8_fixed_t min_val;
    pico8_fixed_t sin_term;
    pico8_fixed_t result;

    if (self == NULL) {
        return;
    }

    threshold = pico8_ov("space folding", pico8_fixed_from_int(29), pico8_fixed_from_int(35));

    one = pico8_fixed_from_int(1);
    td15 = pico8_fixed_div(t, pico8_fixed_from_int(15));
    min_val = (td15 < one) ? td15 : one;
    sin_term =
        pico8_fixed_mul(pico8_fixed_sin(pico8_fixed_div(t, pico8_fixed_from_int(40))), PICO8_FIXED_LIT(0x00001999, 0.1));

    result = pico8_fixed_mul(threshold, pico8_fixed_add(min_val, sin_term));
    self->oi = result;
}

static void pico8_mz_kz_dispatch(entity_t *self, pico8_fixed_t t) {
    pico8_mz_kz((pico8_mz_t *)self, t);
}

void pico8_mz_ey(pico8_mz_t *self) {
    pico8_fixed_t t;
    int result;

    if (self == NULL) {
        return;
    }

    t = self->header.state_time;

    /* The `or` short-circuits: `d_`'s side effects must not run before tick
     * 15, so this call cannot be hoisted out of the else branch. */
    if (t < pico8_fixed_from_int(15)) {
        result = 1;
    } else {
        result = !pico8_d_((entity_t *)self);
    }

    self->p_ = result;
}

static void pico8_mz_ey_dispatch(entity_t *self, entity_t *kv) {
    (void)kv;
    pico8_mz_ey((pico8_mz_t *)self);
}

void pico8_mz_pr(pico8_mz_t *self, pico8_fixed_t t) {
    pico8_fixed_t sr;
    pico8_vec_t self_p;
    const pico8_ents_t *jo_list;
    int i;

    if (self == NULL) {
        return;
    }

    sr = pico8_fixed_add(pico8_fixed_div(pico8_mz_ej(self, t), pico8_fixed_from_int(128)),
                          PICO8_FIXED_LIT(0x000007ef, 0.031));

    self_p = self->p != NULL ? *self->p : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    jo_list = pico8_cg("jo");
    if (jo_list != NULL) {
        for (i = 0; i < jo_list->count; i++) {
            entity_t *pt_val = jo_list->items[i];
            pico8_jo_t *pt;

            if (pt_val == NULL) {
                continue;
            }
            assert(pico8_jo_is_family(pt_val) && "mz:pr: cg.jo holds planets only");
            pt = (pico8_jo_t *)pt_val;

            if (pt->header.state != NULL && strcmp(pt->header.state, "unknown") == 0) {
                pico8_vec_t pt_p =
                    pt->p != NULL ? *pt->p : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
                pico8_vec_t diff = pico8_vec_sub(pt_p, self_p);
                if (pico8_vec_qx(diff) < sr) {
                    pico8_jo_hh(pt);
                }
            }
        }
    }

    self->p_ = t > pico8_fixed_from_int(25);
}

static void pico8_mz_pr_dispatch(entity_t *self, pico8_fixed_t t) {
    pico8_mz_pr((pico8_mz_t *)self, t);
}

pico8_fixed_t pico8_mz_ej(pico8_mz_t *self, pico8_fixed_t t) {
    pico8_fixed_t sq = pico8_fixed_sqrt(pico8_fixed_div(t, pico8_fixed_from_int(15)));
    pico8_fixed_t one = pico8_fixed_from_int(1);
    pico8_fixed_t min_val = (sq < one) ? sq : one;
    pico8_fixed_t oi = pico8_fixed_from_int(0);

    if (self != NULL) {
        oi = self->oi;
    }
    return pico8_fixed_mul(min_val, oi);
}

static void pico8_mz_rendre_dispatch(entity_t *self) {
    pico8_jo_prise_arene_rendue(((pico8_mz_t *)self)->ox->zone);
}

/* Lua's `mz.hq=bv` is rebound three statements later by `function mz:hq`,
 * so only that second definition is ever reachable. Putting
 * pico8_bv_dispatch in the HQ entry instead would look faithful and would
 * drop the nearest-anchor search. */
static const entity_class_t g_mz_struct_class = {
    .draw = pico8_mz_kj_dispatch,
    .draw_overlay = pico8_dl_dispatch,
    .drag = pico8_mz_hq_dispatch,
    .drop = pico8_mz_ey_dispatch,
    .release = pico8_mz_rendre_dispatch,
    .states =
        {
            [ENTITY_STATE_PLACING] = pico8_mz_kz_dispatch,
            [ENTITY_STATE_UNFOLDING] = pico8_mz_pr_dispatch,
        },
    .tags = TAG(TAG_WORMHOLE),
};

static pico8_mz_t *pico8_mz_struct_new(void) {
    pico8_mz_t *self;

    pico8_jo_prise_ouvrir();
    self = (pico8_mz_t *)pico8_zone_alloc(pico8_jo_zone_construction(), sizeof *self);

    pico8_ke_struct_init(&self->header, &g_mz_struct_class, pico8_mz_def.me, pico8_mz_def.t);
    self->def = &pico8_mz_def;
    self->header.layer = &self->def->fa;
    self->header.pos = &self->ra;
    self->header.map_pos = &self->p;
    self->header.targeting = &self->probe_own;
    self->probe = &self->probe_own;
    self->probe->def = &self->def->probe;
    self->oi = self->def->oi;
    self->probe->od = self->def->probe.od;
    self->probe->has_od = self->def->probe.has_od;
    self->jj = &g_mz_jj_wrapper;
    self->ox = NULL;
    self->ra = NULL;
    self->p = NULL;
    self->probe->et = NULL;
    self->ij = NULL;
    self->header.is_dead = &self->p_;
    self->p_ = 0;
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("mz");
#endif
    pico8_mz_pq(self);
    pico8_jo_prise_fermer();
    pico8_ke_struct_pq(&self->header);
    return self;
}

void pico8_wormhole_init(void) {
    pico8_wormhole_shutdown();
    g_mz_up = 1;
}

void pico8_wormhole_shutdown(void) {
    g_mz_up = 0;
}

int pico8_mz_ready(void) {
    return g_mz_up;
}

const entity_class_t *pico8_mz_struct_class(void) {
    return &g_mz_struct_class;
}

pico8_mz_t *pico8_mz_new(void) {
    return pico8_mz_struct_new();
}

pico8_mz_jj_t *pico8_mz_jj_wrapper(void) {
    return &g_mz_jj_wrapper;
}

const entity_class_t *pico8_mz_jj_struct_class(void) {
    return &g_mz_jj_struct_class;
}
