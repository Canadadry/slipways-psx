#include "game/planet_family.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "game/scroll.h"
#include "engine/entity_class.h"
#include "engine/dispatch_context.h"
#include "engine/entity_registry.h"
#include "engine/entity.h"
#include "game/particle.h"
#include "game/draw_functions.h"
#include "game/planet.h"
#include "game/planet_display.h"
#include "game/probe.h"
#include "engine/game_zone.h"

static int g_family_up = 0;
/* Every burst's `by`, which `ib` extends in place; each init empties it. */
static entity_list_t g_ga_by_shared;

void pico8_qh_ly_dispatch(entity_t *self, p8_fixed_t t) {
    pico8_na(self, t);
}

static const entity_class_t g_qh_struct_class = {
    .draw = pico8_jo_kj_dispatch,
    .draw_overlay = pico8_dl_dispatch,
    .hit_test = pico8_jo_hd_dispatch,
    .new_year = pico8_jo_hk_dispatch,
    .refresh = pico8_jo_hb_dispatch,
    .release = pico8_jo_rendre_dispatch,
    .left_press = pico8_jo_ldown_dispatch,
    .accept_drop = pico8_jo_gv_dispatch,
    .drag = pico8_bv_dispatch,
    .drop = pico8_d__dispatch,
    .attach_slipway = pico8_jo_kf_dispatch,
    .offer_trade = pico8_jo_cn_dispatch,
    .states =
        {
            [ENTITY_STATE_VANISHING] = pico8_jo_ll_dispatch,
            [ENTITY_STATE_SETTLED] = pico8_qh_ly_dispatch,
        },
    .tags = TAG(TAG_STRUCTURE) | TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING),
    .lineage = LINEAGE(LINEAGE_PLANET) | LINEAGE(LINEAGE_STRUCTURE),
};

void pico8_qh_struct_init(pico8_jo_t *self, const entity_class_t *cls, const pico8_jo_def_t *def) {
    pico8_jo_struct_init(self, cls, def);
    self->header.targeting = &self->probe_own;
    self->probe = &self->probe_own;
    self->probe->def = &self->def->probe;
    self->probe->od = self->def->probe.od;
    self->probe->has_od = self->def->probe.has_od;
    self->s = self->def->s;
    self->probe->et = NULL;
}

static pico8_jo_t *pico8_qh_struct_new(const planet_keys_t *keys) {
    pico8_jo_t *self = (pico8_jo_t *)allocator_alloc(pico8_jo_zone_construction(), sizeof *self);

    assert(g_family_up && "pico8_planet_family_init() must run first");
    pico8_qh_struct_init(self, &g_qh_struct_class, &pico8_qh_def);
    if (keys != NULL) {
        assert(keys->map_pos == NULL && keys->pos == NULL &&
               "qh: the only construction key is id (ie:ldown); another key has no member "
               "-- stop, do not add one");
        if (keys->planet_id != NULL) {
            self->id = keys->planet_id;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("structure");
#endif
    pico8_jo_pq(self);
    entity_construct(&self->header);
    return self;
}

void pico8_nl_ly(pico8_jo_t *self, p8_fixed_t t) {
    if (self == NULL) {
        return;
    }
    pico8_na((entity_t *)self, t);

    assert(self->has_mi && "nl:ly reads self.mi unguarded, unlike hj:kj");
    if (self->has_mi && self->mi >= p8_fixed_from_int(2)) {
        self->qw =
            p8_fixed_add(p8_fixed_from_int(8),
                            p8_fixed_div(p8_fixed_mod(t, p8_fixed_from_int(42)), p8_fixed_from_int(6)));
    }
}

static void pico8_nl_ly_dispatch(entity_t *self, p8_fixed_t t) {
    pico8_nl_ly((pico8_jo_t *)self, t);
}

static const entity_class_t g_nl_struct_class = {
    .draw = pico8_jo_kj_dispatch,
    .draw_overlay = pico8_dl_dispatch,
    .hit_test = pico8_jo_hd_dispatch,
    .new_year = pico8_jo_hk_dispatch,
    .refresh = pico8_jo_hb_dispatch,
    .release = pico8_jo_rendre_dispatch,
    .left_press = pico8_jo_ldown_dispatch,
    .accept_drop = pico8_jo_gv_dispatch,
    .drag = pico8_bv_dispatch,
    .drop = pico8_d__dispatch,
    .attach_slipway = pico8_jo_kf_dispatch,
    .offer_trade = pico8_jo_cn_dispatch,
    .states =
        {
            [ENTITY_STATE_VANISHING] = pico8_jo_ll_dispatch,
            [ENTITY_STATE_SETTLED] = pico8_nl_ly_dispatch,
        },
    .tags = TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING),
    .lineage = LINEAGE(LINEAGE_PLANET) | LINEAGE(LINEAGE_STRUCTURE) | LINEAGE(LINEAGE_TRADING_HUB),
};

static pico8_jo_t *pico8_nl_struct_new(const planet_keys_t *keys) {
    pico8_jo_t *self = (pico8_jo_t *)allocator_alloc(pico8_jo_zone_construction(), sizeof *self);

    assert(g_family_up && "pico8_planet_family_init() must run first");
    pico8_qh_struct_init(self, &g_nl_struct_class, &pico8_nl_def);
    self->qw = self->def->qw;
    if (keys != NULL) {
        assert(keys->map_pos == NULL && keys->pos == NULL &&
               "nl: the only construction key is id (ie:ldown); another key has no member "
               "-- stop, do not add one");
        if (keys->planet_id != NULL) {
            self->id = keys->planet_id;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("trading_hub");
#endif
    pico8_jo_pq(self);
    entity_construct(&self->header);
    return self;
}

static const entity_class_t g_os_struct_class = {
    .draw = pico8_jo_kj_dispatch,
    .draw_overlay = pico8_dl_dispatch,
    .hit_test = pico8_jo_hd_dispatch,
    .new_year = pico8_jo_hk_dispatch,
    .refresh = pico8_jo_hb_dispatch,
    .release = pico8_jo_rendre_dispatch,
    .left_press = pico8_jo_ldown_dispatch,
    .accept_drop = pico8_jo_gv_dispatch,
    .drag = pico8_bv_dispatch,
    .drop = pico8_d__dispatch,
    .attach_slipway = pico8_jo_kf_dispatch,
    .offer_trade = pico8_jo_cn_dispatch,
    .states =
        {
            [ENTITY_STATE_VANISHING] = pico8_jo_ll_dispatch,
            [ENTITY_STATE_SETTLED] = pico8_qh_ly_dispatch,
        },
    .tags = TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING),
    .lineage = LINEAGE(LINEAGE_PLANET) | LINEAGE(LINEAGE_STRUCTURE),
};

static pico8_jo_t *pico8_os_struct_new(const planet_keys_t *keys) {
    pico8_jo_t *self = (pico8_jo_t *)allocator_alloc(pico8_jo_zone_construction(), sizeof *self);

    assert(g_family_up && "pico8_planet_family_init() must run first");
    pico8_qh_struct_init(self, &g_os_struct_class, &pico8_os_def);
    if (keys != NULL) {
        assert(keys->map_pos == NULL && keys->pos == NULL &&
               "os: the only construction key is id (ie:ldown); another key has no member "
               "-- stop, do not add one");
        if (keys->planet_id != NULL) {
            self->id = keys->planet_id;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("processor");
#endif
    pico8_jo_pq(self);
    entity_construct(&self->header);
    return self;
}

void pico8_hj_kj(p8_graphics_t *gfx, p8_memory_t *mem, pico8_jo_t *self, vec_t p) {
    if (self == NULL) {
        return;
    }
    if (self->has_mi && self->mi >= p8_fixed_from_int(2)) {
        pico8_ei(gfx, mem, (entity_t *)self);
    }
    pico8_jo_kj(gfx, mem, self, p);
}

void pico8_hj_kj_dispatch(entity_t *self, const vec_t *ra) {
    vec_t p = ra != NULL ? *ra : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    pico8_hj_kj(dispatch_context_gfx(), dispatch_context_mem(), (pico8_jo_t *)self, p);
}

void pico8_hj_rendre_dispatch(entity_t *self) {
    pico8_jo_rendre_rangee(self);
    pico8_qy_rendre(self->emitter);
    pico8_jo_prise_rendue((const pico8_jo_t *)self);
}

static const entity_class_t g_hj_struct_class = {
    .draw = pico8_hj_kj_dispatch,
    .draw_overlay = pico8_dl_dispatch,
    .hit_test = pico8_jo_hd_dispatch,
    .new_year = pico8_jo_hk_dispatch,
    .refresh = pico8_jo_hb_dispatch,
    .release = pico8_hj_rendre_dispatch,
    .left_press = pico8_jo_ldown_dispatch,
    .accept_drop = pico8_jo_gv_dispatch,
    .drag = pico8_bv_dispatch,
    .drop = pico8_d__dispatch,
    .attach_slipway = pico8_jo_kf_dispatch,
    .offer_trade = pico8_jo_cn_dispatch,
    .spawn_particle = pico8_g__mm,
    .states =
        {
            [ENTITY_STATE_VANISHING] = pico8_jo_ll_dispatch,
            [ENTITY_STATE_SETTLED] = pico8_qh_ly_dispatch,
        },
    .tags = TAG(TAG_ASCENSION) | TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING),
    .lineage = LINEAGE(LINEAGE_PLANET) | LINEAGE(LINEAGE_STRUCTURE),
};

void pico8_hj_struct_init(pico8_jo_t *self, const entity_class_t *cls, const pico8_jo_def_t *def) {
    pico8_qh_struct_init(self, cls, def);
    self->header.emitter = &self->emitter_own;
    self->emitter = &self->emitter_own;
    self->emitter->def = &self->def->emitter;
    self->emitter->py = self->def->emitter.py;
    self->emitter->pn = self->def->emitter.pn;
    self->emitter->k_ = self->def->emitter.k_;
    self->emitter->du = self->def->emitter.du;
    self->emitter->qo = self->def->emitter.qo;
    self->emitter->ro = self->def->emitter.ro;
    self->emitter->qg = self->def->emitter.qg;
    self->emitter->q_ = self->def->emitter.q_;
    self->emitter->qy = NULL;
}

static pico8_jo_t *pico8_hj_struct_new(const planet_keys_t *keys) {
    pico8_jo_t *self = (pico8_jo_t *)allocator_alloc(pico8_jo_zone_construction(), sizeof *self);

    assert(g_family_up && "pico8_planet_family_init() must run first");
    pico8_hj_struct_init(self, &g_hj_struct_class, &pico8_hj_def);
    if (keys != NULL) {
        assert(keys->map_pos == NULL && keys->pos == NULL &&
               "hj: the only construction key is id (ie:ldown); another key has no member "
               "-- stop, do not add one");
        if (keys->planet_id != NULL) {
            self->id = keys->planet_id;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("ascension");
#endif
    pico8_f_((entity_t *)self);
    pico8_jo_pq(self);
    entity_construct(&self->header);
    return self;
}

void pico8_pm_pq(pico8_jo_t *self) {
    vec_t spawn_ra;
    pico8_ga_keys_t keys;
    pico8_jo_t *spawned;
    const vec_t *ra;

    if (self == NULL) {
        return;
    }

    ra = self->ra;
    spawn_ra = vec_add(ra != NULL ? *ra : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0)),
                             self->def->ng);

    keys.jo.planet_id = NULL;
    keys.jo.map_pos = NULL;
    keys.jo.pos = NULL;
    keys.ra_copie = &spawn_ra;
    keys.pq = &pico8_pm_ga_pq_keys;

    spawned = pico8_ga_new(&keys);
    (void)spawned;
}

static const entity_class_t g_pm_struct_class = {
    .draw = pico8_hj_kj_dispatch,
    .draw_overlay = pico8_dl_dispatch,
    .hit_test = pico8_jo_hd_dispatch,
    .new_year = pico8_jo_hk_dispatch,
    .refresh = pico8_jo_hb_dispatch,
    .release = pico8_hj_rendre_dispatch,
    .left_press = pico8_jo_ldown_dispatch,
    .accept_drop = pico8_jo_gv_dispatch,
    .drag = pico8_bv_dispatch,
    .drop = pico8_d__dispatch,
    .attach_slipway = pico8_jo_kf_dispatch,
    .offer_trade = pico8_jo_cn_dispatch,
    .spawn_particle = pico8_g__mm,
    .states =
        {
            [ENTITY_STATE_VANISHING] = pico8_jo_ll_dispatch,
            [ENTITY_STATE_SETTLED] = pico8_qh_ly_dispatch,
        },
    .tags = TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING),
    .lineage = LINEAGE(LINEAGE_PLANET) | LINEAGE(LINEAGE_STRUCTURE),
};

static pico8_jo_t *pico8_pm_struct_new(const planet_keys_t *keys) {
    pico8_jo_t *self = (pico8_jo_t *)allocator_alloc(pico8_jo_zone_construction(), sizeof *self);

    assert(g_family_up && "pico8_planet_family_init() must run first");
    pico8_hj_struct_init(self, &g_pm_struct_class, &pico8_pm_def);
    if (keys != NULL) {
        if (keys->map_pos != NULL) {
            self->p = keys->map_pos;
        }
        if (keys->pos != NULL) {
            self->ra = keys->pos;
        }
        if (keys->planet_id != NULL) {
            self->id = keys->planet_id;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("protostar");
#endif
    pico8_pm_pq(self);
    pico8_f_((entity_t *)self);
    pico8_jo_pq(self);
    entity_construct(&self->header);
    return self;
}

void pico8_ga_pq(pico8_jo_t *self) {
    pico8_qb_t *mo;

    if (self == NULL) {
        return;
    }
    pico8_f_((entity_t *)self);
    pico8_ez((entity_t *)self, p8_fixed_from_int(140));

    if (main_camera() != NULL) {
        mo = pico8_mo_qb();
        assert(mo != NULL && "mo: the placeholder has no writer; a write before jl() stops the run");
        mo->ma = p8_fixed_from_int(3);
    }
}

pico8_qy_t *pico8_ga_mm(entity_t *self_ptr) {
    const pico8_jo_t *self = (const pico8_jo_t *)self_ptr;
    p8_fixed_t qd;
    p8_fixed_t ms;
    p8_fixed_t iu;
    p8_fixed_t py;
    vec_t p;
    vec_t v;
    vec_t a;
    pico8_qy_t *particle;

    qd = p8_fixed_add(p8_fixed_rnd(P8_FIXED_LIT(0x00018000, 1.5)), P8_FIXED_LIT(0x00008000, 0.5));
    if (p8_fixed_rnd_default() < P8_FIXED_LIT(0x00009999, 0.6)) {
        qd = p8_fixed_mul(p8_fixed_flr(p8_fixed_div(qd, P8_FIXED_LIT(0x00008000, 0.5))),
                              P8_FIXED_LIT(0x00008000, 0.5));
    }
    p = vec_from_polar(qd, p8_fixed_rnd_default());

    ms = self->ms;
    iu = self->iu;
    py = self->emitter->py;

    v = vec_mul(p, ms);
    a = vec_mul(p, iu);

    particle = pico8_particule_prendre();
    particle->p = p;
    particle->v = v;
    particle->has_a = 1;
    particle->a = a;
    particle->l = p8_fixed_add(p8_fixed_rnd(py), py);

    return particle;
}

void pico8_ga_kj(p8_graphics_t *gfx, p8_memory_t *mem, pico8_jo_t *self, vec_t p) {
    const pointer_list_t *qy = NULL;
    int third_taken = 0;
    int i;
    int px;
    int py;

    if (self == NULL) {
        return;
    }

    qy = self->emitter->qy;

    px = p8_fixed_flr_int(p.x);
    py = p8_fixed_flr_int(p.y);

    for (i = 1; i <= 3; i++) {
        const pico8_qy_t *pt = NULL;

        if (qy != NULL && i <= qy->count) {
            pt = qy->items[i - 1];
        }

        if (pt != NULL) {
            int idx;
            int color = 0;
            int k_entry;

            idx = p8_fixed_flr_int(p8_fixed_flr(p8_fixed_div(pt->l, p8_fixed_from_int(2))));
            /* TROU-21 */
            if (pico8_int_seq_at(pico8_ga_k__seq.items, pico8_ga_k__seq.count, idx, &k_entry)) {
                color = k_entry;
            }
            p8_circ(gfx, mem, px, py, p8_fixed_flr_int(vec_length(pt->p)), color);
        }

        /* Lua's `pt` outlives the loop, so `p_` follows the THIRD slot
         * alone: the burst dies when fewer than 3 particles remain, not
         * when none do. A flag, not `pt`: `ei` may give that particle
         * back before `p_` is written. */
        third_taken = pt != NULL;
    }

    pico8_ei(gfx, mem, (entity_t *)self);
    self->p_ = !third_taken;
}

static void pico8_ga_kj_dispatch(entity_t *self, const vec_t *ra) {
    vec_t p = ra != NULL ? *ra : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    pico8_ga_kj(dispatch_context_gfx(), dispatch_context_mem(), (pico8_jo_t *)self, p);
}

/* A burst's place is the zone of its `oe` (pico8_ga_struct_new): given
 * back last, the struct and `oe` with it. */
static void pico8_ga_rendre_dispatch(entity_t *self) {
    const allocator_t *place = ((pico8_jo_t *)self)->oe->zone;

    pico8_hj_rendre_dispatch(self);
    arena_release(place);
}

static const entity_class_t g_ga_struct_class = {
    .draw = pico8_ga_kj_dispatch,
    .hit_test = pico8_jo_hd_dispatch,
    .new_year = pico8_jo_hk_dispatch,
    .refresh = pico8_jo_hb_dispatch,
    .release = pico8_ga_rendre_dispatch,
    .left_press = pico8_jo_ldown_dispatch,
    .accept_drop = pico8_jo_gv_dispatch,
    .attach_slipway = pico8_jo_kf_dispatch,
    .offer_trade = pico8_jo_cn_dispatch,
    .spawn_particle = pico8_ga_mm,
    .states = {[ENTITY_STATE_VANISHING] = pico8_jo_ll_dispatch},
    /* ga's cart list of tags is empty and replaces jo's: never jo's. */
    .tags = 0,
    .lineage = LINEAGE(LINEAGE_PLANET),
};

static pico8_jo_t *pico8_ga_struct_new(const pico8_ga_keys_t *keys) {
    const allocator_t *place = pico8_ga_place_prendre();
    pico8_jo_t *self = (pico8_jo_t *)allocator_alloc(place, sizeof *self);

    assert(g_family_up && "pico8_planet_family_init() must run first");
    pico8_jo_struct_init(self, &g_ga_struct_class, &pico8_ga_def);
    self->header.emitter = &self->emitter_own;
    self->emitter = &self->emitter_own;
    self->emitter->def = &self->def->emitter;
    self->emitter->qo = self->def->emitter.qo;
    self->emitter->ro = self->def->emitter.ro;
    self->emitter->qg = self->def->emitter.qg;
    self->emitter->q_ = self->def->emitter.q_;
    self->emitter->k_ = self->def->emitter.k_;
    self->iu = self->def->iu;
    self->emitter->py = self->def->emitter.py;
    self->ms = self->def->ms;
    self->emitter->qy = NULL;
    self->by = &g_ga_by_shared;
    if (keys != NULL) {
        if (keys->jo.map_pos != NULL) {
            self->p = keys->jo.map_pos;
        }
        if (keys->jo.pos != NULL) {
            self->ra = keys->jo.pos;
        }
        if (keys->ra_copie != NULL) {
            vec_t *ra = (vec_t *)allocator_alloc(place, sizeof *ra);

            assert(keys->jo.pos == NULL && "ga: a burst borrows its ra or copies it, not both");
            *ra = *keys->ra_copie;
            self->ra = ra;
        }
        if (keys->jo.planet_id != NULL) {
            self->id = keys->jo.planet_id;
        }
        if (keys->pq != NULL) {
            self->emitter->qo = keys->pq->qo;
            self->emitter->ro = keys->pq->ro;
            self->emitter->qg = keys->pq->qg;
            self->emitter->q_ = keys->pq->q_;
            self->emitter->k_ = keys->pq->k_;
            self->iu = keys->pq->iu;
            self->emitter->py = keys->pq->py;
            self->ms = keys->pq->ms;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("burst");
#endif
    pico8_ga_pq(self);
    pico8_jo_pq_dans(self, place);
    entity_construct(&self->header);
    return self;
}

void pico8_planet_family_init(void) {
    assert(pico8_jo_ready() && "pico8_jo_init() must run first");
    pico8_planet_family_shutdown();
    pico8_pi_init();
    g_family_up = 1;
    entity_list_init(&g_ga_by_shared, game_zone_load_zone());
}

void pico8_planet_family_shutdown(void) {
    g_family_up = 0;
    entity_list_free(&g_ga_by_shared);
}

int pico8_qh_ready(void) {
    return g_family_up;
}

static entity_t *pico8_qh_kind_new(const planet_keys_t *keys) {
    return &pico8_qh_struct_new(keys)->header;
}

static const entity_kind_t g_qh_kind = {pico8_qh_kind_new, &pico8_qh_kind_rec};

const entity_kind_t *pico8_qh_kind(void) {
    return &g_qh_kind;
}

const entity_class_t *pico8_qh_struct_class(void) {
    return &g_qh_struct_class;
}

pico8_jo_t *pico8_qh_new(const planet_keys_t *keys) {
    return pico8_qh_struct_new(keys);
}

int pico8_nl_ready(void) {
    return g_family_up;
}

static entity_t *pico8_nl_kind_new(const planet_keys_t *keys) {
    return &pico8_nl_struct_new(keys)->header;
}

static const entity_kind_t g_nl_kind = {pico8_nl_kind_new, &pico8_nl_kind_rec};

const entity_kind_t *pico8_nl_kind(void) {
    return &g_nl_kind;
}

const entity_class_t *pico8_nl_struct_class(void) {
    return &g_nl_struct_class;
}

pico8_jo_t *pico8_nl_new(const planet_keys_t *keys) {
    return pico8_nl_struct_new(keys);
}

int pico8_os_ready(void) {
    return g_family_up;
}

static entity_t *pico8_os_kind_new(const planet_keys_t *keys) {
    return &pico8_os_struct_new(keys)->header;
}

static const entity_kind_t g_os_kind = {pico8_os_kind_new, &pico8_os_kind_rec};

const entity_kind_t *pico8_os_kind(void) {
    return &g_os_kind;
}

const entity_class_t *pico8_os_struct_class(void) {
    return &g_os_struct_class;
}

pico8_jo_t *pico8_os_new(const planet_keys_t *keys) {
    return pico8_os_struct_new(keys);
}

int pico8_hj_ready(void) {
    return g_family_up;
}

static entity_t *pico8_hj_kind_new(const planet_keys_t *keys) {
    return &pico8_hj_struct_new(keys)->header;
}

static const entity_kind_t g_hj_kind = {pico8_hj_kind_new, &pico8_hj_kind_rec};

const entity_kind_t *pico8_hj_kind(void) {
    return &g_hj_kind;
}

const entity_class_t *pico8_hj_struct_class(void) {
    return &g_hj_struct_class;
}

pico8_jo_t *pico8_hj_new(const planet_keys_t *keys) {
    return pico8_hj_struct_new(keys);
}

int pico8_pm_ready(void) {
    return g_family_up;
}

static entity_t *pico8_pm_kind_new(const planet_keys_t *keys) {
    return &pico8_pm_struct_new(keys)->header;
}

static const entity_kind_t g_pm_kind = {pico8_pm_kind_new, &pico8_pm_kind_rec};

const entity_kind_t *pico8_pm_kind(void) {
    return &g_pm_kind;
}

const entity_class_t *pico8_pm_struct_class(void) {
    return &g_pm_struct_class;
}

pico8_jo_t *pico8_pm_new(const planet_keys_t *keys) {
    return pico8_pm_struct_new(keys);
}

int pico8_ga_ready(void) {
    return g_family_up;
}

/* `kv["ga"]` (jo:hu): jo's keys, no `pq`. */
static entity_t *pico8_ga_kind_new(const planet_keys_t *keys) {
    pico8_ga_keys_t ga_keys;

    if (keys == NULL) {
        return &pico8_ga_struct_new(NULL)->header;
    }
    ga_keys.jo = *keys;
    ga_keys.ra_copie = NULL;
    ga_keys.pq = NULL;
    return &pico8_ga_struct_new(&ga_keys)->header;
}

static const entity_kind_t g_ga_kind = {pico8_ga_kind_new, &pico8_ga_kind_rec};

const entity_kind_t *pico8_ga_kind(void) {
    return &g_ga_kind;
}

const entity_class_t *pico8_ga_struct_class(void) {
    return &g_ga_struct_class;
}

pico8_jo_t *pico8_ga_new(const pico8_ga_keys_t *keys) {
    return pico8_ga_struct_new(keys);
}

/* `kv`, indexed by `fz`: `pm` and `ga` are the only kinds whose spec sets
 * one. */
static const struct {
    const char *fz;
    const entity_kind_t *(*kind)(void);
} g_kv[] = {
    {"protostar", pico8_pm_kind},
    {"burst", pico8_ga_kind},
};

const entity_kind_t *pico8_jo_kv(const char *fz) {
    size_t i;

    for (i = 0; i < sizeof g_kv / sizeof g_kv[0]; i++) {
        if (strcmp(g_kv[i].fz, fz) == 0) {
            return g_kv[i].kind();
        }
    }
    return NULL;
}
