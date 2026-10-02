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
    const allocator_t *zone = pico8_jo_zone_construction();
    entity_list_t *ox;
    entity_list_iter_t it;
    entity_t *p;
    int n = 0;

    if (self == NULL) {
        return;
    }

    p8_play_sfx(p8_fixed_from_int(53));

    it = entity_list_all(live_entities());
    while (entity_list_iter_next(&it, &p)) {
        n += pico8_mz_ancrage(p);
    }
    ox = (entity_list_t *)allocator_alloc(zone, sizeof *ox);
    entity_list_init_reserve(ox, zone, n);
    it = entity_list_all(live_entities());
    while (entity_list_iter_next(&it, &p)) {
        if (pico8_mz_ancrage(p)) {
            entity_list_add(ox, p);
        }
    }
    assert(ox->count == n && ox->capacity == n);
    self->ox = ox;
}

void pico8_mz_hq(pico8_mz_t *self, vec_t *mp, vec_t *rp) {
    vec_t p_vec;
    entity_t *nearest = NULL;
    p8_fixed_t nq;
    p8_fixed_t threshold;

    pico8_bv((entity_t *)self, mp, rp);
    if (self == NULL) {
        return;
    }

    p_vec = self->p != NULL ? *self->p : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    nq = entity_geom_nearest(p_vec, self->ox, &nearest);
    self->ij = nearest;

    threshold = pico8_ov("space folding", P8_FIXED_LIT(0x000030a3, 0.19), P8_FIXED_LIT(0x0000451e, 0.27));

    if (nq > threshold) {
        self->probe->et = (entity_t *)pico8_mz_jj_wrapper();
    }
}

static void pico8_mz_hq_dispatch(entity_t *self, vec_t *qa, vec_t *ra) {
    pico8_mz_hq((pico8_mz_t *)self, qa, ra);
}

void pico8_mz_kz(pico8_mz_t *self, p8_fixed_t t) {
    p8_fixed_t threshold;
    p8_fixed_t one;
    p8_fixed_t td15;
    p8_fixed_t min_val;
    p8_fixed_t sin_term;
    p8_fixed_t result;

    if (self == NULL) {
        return;
    }

    threshold = pico8_ov("space folding", p8_fixed_from_int(29), p8_fixed_from_int(35));

    one = p8_fixed_from_int(1);
    td15 = p8_fixed_div(t, p8_fixed_from_int(15));
    min_val = (td15 < one) ? td15 : one;
    sin_term =
        p8_fixed_mul(p8_fixed_sin(p8_fixed_div(t, p8_fixed_from_int(40))), P8_FIXED_LIT(0x00001999, 0.1));

    result = p8_fixed_mul(threshold, p8_fixed_add(min_val, sin_term));
    self->oi = result;
}

static void pico8_mz_kz_dispatch(entity_t *self, p8_fixed_t t) {
    pico8_mz_kz((pico8_mz_t *)self, t);
}

void pico8_mz_ey(pico8_mz_t *self) {
    p8_fixed_t t;
    int result;

    if (self == NULL) {
        return;
    }

    t = self->header.state_time;

    /* The `or` short-circuits: `d_`'s side effects must not run before tick
     * 15, so this call cannot be hoisted out of the else branch. */
    if (t < p8_fixed_from_int(15)) {
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

void pico8_mz_pr(pico8_mz_t *self, p8_fixed_t t) {
    p8_fixed_t sr;
    vec_t self_p;
    const entity_list_t *jo_list;
    int i;

    if (self == NULL) {
        return;
    }

    sr = p8_fixed_add(p8_fixed_div(pico8_mz_ej(self, t), p8_fixed_from_int(128)),
                          P8_FIXED_LIT(0x000007ef, 0.031));

    self_p = self->p != NULL ? *self->p : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    jo_list = entities_by_tag("planet");
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
                vec_t pt_p =
                    pt->p != NULL ? *pt->p : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
                vec_t diff = vec_sub(pt_p, self_p);
                if (vec_length(diff) < sr) {
                    pico8_jo_hh(pt);
                }
            }
        }
    }

    self->p_ = t > p8_fixed_from_int(25);
}

static void pico8_mz_pr_dispatch(entity_t *self, p8_fixed_t t) {
    pico8_mz_pr((pico8_mz_t *)self, t);
}

p8_fixed_t pico8_mz_ej(pico8_mz_t *self, p8_fixed_t t) {
    p8_fixed_t sq = p8_fixed_sqrt(p8_fixed_div(t, p8_fixed_from_int(15)));
    p8_fixed_t one = p8_fixed_from_int(1);
    p8_fixed_t min_val = (sq < one) ? sq : one;
    p8_fixed_t oi = p8_fixed_from_int(0);

    if (self != NULL) {
        oi = self->oi;
    }
    return p8_fixed_mul(min_val, oi);
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
    self = (pico8_mz_t *)allocator_alloc(pico8_jo_zone_construction(), sizeof *self);

    entity_init_instance(&self->header, &g_mz_struct_class, pico8_mz_def.me, pico8_mz_def.t);
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
    pico8_class_coverage_record("wormhole");
#endif
    pico8_mz_pq(self);
    pico8_jo_prise_fermer();
    entity_construct(&self->header);
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
