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
    planet_bob(self, t);
}

static const entity_class_t g_qh_struct_class = {
    .draw = planet_draw_method,
    .draw_overlay = pico8_dl_dispatch,
    .hit_test = planet_hit_test_method,
    .new_year = planet_new_year_method,
    .refresh = planet_refresh_method,
    .release = planet_release_method,
    .left_press = planet_left_press_method,
    .accept_drop = planet_accept_drop_method,
    .drag = pico8_bv_dispatch,
    .drop = pico8_d__dispatch,
    .attach_slipway = planet_attach_slipway_method,
    .offer_trade = planet_offer_trade_method,
    .states =
        {
            [ENTITY_STATE_VANISHING] = planet_update_vanishing_method,
            [ENTITY_STATE_SETTLED] = pico8_qh_ly_dispatch,
        },
    .tags = TAG(TAG_STRUCTURE) | TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING),
    .lineage = LINEAGE(LINEAGE_PLANET) | LINEAGE(LINEAGE_STRUCTURE),
};

void pico8_qh_struct_init(planet_t *self, const entity_class_t *cls, const planet_archetype_t *def) {
    planet_init_instance(self, cls, def);
    self->header.targeting = &self->probe_own;
    self->targeting = &self->probe_own;
    self->targeting->def = &self->archetype->targeting;
    self->targeting->od = self->archetype->targeting.cost;
    self->targeting->has_od = self->archetype->targeting.has_cost;
    self->sprite = self->archetype->sprite;
    self->targeting->et = NULL;
}

static planet_t *pico8_qh_struct_new(const planet_keys_t *keys) {
    planet_t *self = (planet_t *)allocator_alloc(planet_zone_construction(), sizeof *self);

    assert(g_family_up && "pico8_planet_family_init() must run first");
    pico8_qh_struct_init(self, &g_qh_struct_class, &structure_archetype);
    if (keys != NULL) {
        assert(keys->map_pos == NULL && keys->pos == NULL &&
               "qh: the only construction key is id (ie:ldown); another key has no member "
               "-- stop, do not add one");
        if (keys->planet_id != NULL) {
            self->planet_id = keys->planet_id;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("structure");
#endif
    planet_construct(self);
    entity_construct(&self->header);
    return self;
}

void pico8_nl_ly(planet_t *self, p8_fixed_t t) {
    if (self == NULL) {
        return;
    }
    planet_bob((entity_t *)self, t);

    assert(self->has_development_tier && "nl:ly reads self.mi unguarded, unlike hj:kj");
    if (self->has_development_tier && self->development_tier >= p8_fixed_from_int(2)) {
        self->palette_block =
            p8_fixed_add(p8_fixed_from_int(8),
                            p8_fixed_div(p8_fixed_mod(t, p8_fixed_from_int(42)), p8_fixed_from_int(6)));
    }
}

static void pico8_nl_ly_dispatch(entity_t *self, p8_fixed_t t) {
    pico8_nl_ly((planet_t *)self, t);
}

static const entity_class_t g_nl_struct_class = {
    .draw = planet_draw_method,
    .draw_overlay = pico8_dl_dispatch,
    .hit_test = planet_hit_test_method,
    .new_year = planet_new_year_method,
    .refresh = planet_refresh_method,
    .release = planet_release_method,
    .left_press = planet_left_press_method,
    .accept_drop = planet_accept_drop_method,
    .drag = pico8_bv_dispatch,
    .drop = pico8_d__dispatch,
    .attach_slipway = planet_attach_slipway_method,
    .offer_trade = planet_offer_trade_method,
    .states =
        {
            [ENTITY_STATE_VANISHING] = planet_update_vanishing_method,
            [ENTITY_STATE_SETTLED] = pico8_nl_ly_dispatch,
        },
    .tags = TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING),
    .lineage = LINEAGE(LINEAGE_PLANET) | LINEAGE(LINEAGE_STRUCTURE) | LINEAGE(LINEAGE_TRADING_HUB),
};

static planet_t *pico8_nl_struct_new(const planet_keys_t *keys) {
    planet_t *self = (planet_t *)allocator_alloc(planet_zone_construction(), sizeof *self);

    assert(g_family_up && "pico8_planet_family_init() must run first");
    pico8_qh_struct_init(self, &g_nl_struct_class, &trading_hub_archetype);
    self->palette_block = self->archetype->palette_block;
    if (keys != NULL) {
        assert(keys->map_pos == NULL && keys->pos == NULL &&
               "nl: the only construction key is id (ie:ldown); another key has no member "
               "-- stop, do not add one");
        if (keys->planet_id != NULL) {
            self->planet_id = keys->planet_id;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("trading_hub");
#endif
    planet_construct(self);
    entity_construct(&self->header);
    return self;
}

static const entity_class_t g_os_struct_class = {
    .draw = planet_draw_method,
    .draw_overlay = pico8_dl_dispatch,
    .hit_test = planet_hit_test_method,
    .new_year = planet_new_year_method,
    .refresh = planet_refresh_method,
    .release = planet_release_method,
    .left_press = planet_left_press_method,
    .accept_drop = planet_accept_drop_method,
    .drag = pico8_bv_dispatch,
    .drop = pico8_d__dispatch,
    .attach_slipway = planet_attach_slipway_method,
    .offer_trade = planet_offer_trade_method,
    .states =
        {
            [ENTITY_STATE_VANISHING] = planet_update_vanishing_method,
            [ENTITY_STATE_SETTLED] = pico8_qh_ly_dispatch,
        },
    .tags = TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING),
    .lineage = LINEAGE(LINEAGE_PLANET) | LINEAGE(LINEAGE_STRUCTURE),
};

static planet_t *pico8_os_struct_new(const planet_keys_t *keys) {
    planet_t *self = (planet_t *)allocator_alloc(planet_zone_construction(), sizeof *self);

    assert(g_family_up && "pico8_planet_family_init() must run first");
    pico8_qh_struct_init(self, &g_os_struct_class, &processor_archetype);
    if (keys != NULL) {
        assert(keys->map_pos == NULL && keys->pos == NULL &&
               "os: the only construction key is id (ie:ldown); another key has no member "
               "-- stop, do not add one");
        if (keys->planet_id != NULL) {
            self->planet_id = keys->planet_id;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("processor");
#endif
    planet_construct(self);
    entity_construct(&self->header);
    return self;
}

void pico8_hj_kj(p8_graphics_t *gfx, p8_memory_t *mem, planet_t *self, vec_t p) {
    if (self == NULL) {
        return;
    }
    if (self->has_development_tier && self->development_tier >= p8_fixed_from_int(2)) {
        pico8_ei(gfx, mem, (entity_t *)self);
    }
    planet_draw(gfx, mem, self, p);
}

void pico8_hj_kj_dispatch(entity_t *self, const vec_t *ra) {
    vec_t p = ra != NULL ? *ra : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    pico8_hj_kj(dispatch_context_gfx(), dispatch_context_mem(), (planet_t *)self, p);
}

void pico8_hj_rendre_dispatch(entity_t *self) {
    planet_release_row(self);
    pico8_qy_rendre(self->emitter);
    planet_grab_arena_released((const planet_t *)self);
}

static const entity_class_t g_hj_struct_class = {
    .draw = pico8_hj_kj_dispatch,
    .draw_overlay = pico8_dl_dispatch,
    .hit_test = planet_hit_test_method,
    .new_year = planet_new_year_method,
    .refresh = planet_refresh_method,
    .release = pico8_hj_rendre_dispatch,
    .left_press = planet_left_press_method,
    .accept_drop = planet_accept_drop_method,
    .drag = pico8_bv_dispatch,
    .drop = pico8_d__dispatch,
    .attach_slipway = planet_attach_slipway_method,
    .offer_trade = planet_offer_trade_method,
    .spawn_particle = pico8_g__mm,
    .states =
        {
            [ENTITY_STATE_VANISHING] = planet_update_vanishing_method,
            [ENTITY_STATE_SETTLED] = pico8_qh_ly_dispatch,
        },
    .tags = TAG(TAG_ASCENSION) | TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING),
    .lineage = LINEAGE(LINEAGE_PLANET) | LINEAGE(LINEAGE_STRUCTURE),
};

void pico8_hj_struct_init(planet_t *self, const entity_class_t *cls, const planet_archetype_t *def) {
    pico8_qh_struct_init(self, cls, def);
    self->header.emitter = &self->emitter_own;
    self->emitter = &self->emitter_own;
    self->emitter->def = &self->archetype->emitter;
    self->emitter->py = self->archetype->emitter.speed_factor;
    self->emitter->pn = self->archetype->emitter.angle_slices;
    self->emitter->k_ = self->archetype->emitter.trail_colors;
    self->emitter->du = self->archetype->emitter.base_radius;
    self->emitter->qo = self->archetype->emitter.matrix_xx;
    self->emitter->ro = self->archetype->emitter.matrix_xy;
    self->emitter->qg = self->archetype->emitter.matrix_yx;
    self->emitter->q_ = self->archetype->emitter.matrix_yy;
    self->emitter->qy = NULL;
}

static planet_t *pico8_hj_struct_new(const planet_keys_t *keys) {
    planet_t *self = (planet_t *)allocator_alloc(planet_zone_construction(), sizeof *self);

    assert(g_family_up && "pico8_planet_family_init() must run first");
    pico8_hj_struct_init(self, &g_hj_struct_class, &ascension_archetype);
    if (keys != NULL) {
        assert(keys->map_pos == NULL && keys->pos == NULL &&
               "hj: the only construction key is id (ie:ldown); another key has no member "
               "-- stop, do not add one");
        if (keys->planet_id != NULL) {
            self->planet_id = keys->planet_id;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("ascension");
#endif
    pico8_f_((entity_t *)self);
    planet_construct(self);
    entity_construct(&self->header);
    return self;
}

void pico8_pm_pq(planet_t *self) {
    vec_t spawn_ra;
    pico8_ga_keys_t keys;
    planet_t *spawned;
    const vec_t *ra;

    if (self == NULL) {
        return;
    }

    ra = self->pos;
    spawn_ra = vec_add(ra != NULL ? *ra : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0)),
                             self->archetype->spawn_offset);

    keys.jo.planet_id = NULL;
    keys.jo.map_pos = NULL;
    keys.jo.pos = NULL;
    keys.ra_copie = &spawn_ra;
    keys.pq = &protostar_burst_construct_keys;

    spawned = pico8_ga_new(&keys);
    (void)spawned;
}

static const entity_class_t g_pm_struct_class = {
    .draw = pico8_hj_kj_dispatch,
    .draw_overlay = pico8_dl_dispatch,
    .hit_test = planet_hit_test_method,
    .new_year = planet_new_year_method,
    .refresh = planet_refresh_method,
    .release = pico8_hj_rendre_dispatch,
    .left_press = planet_left_press_method,
    .accept_drop = planet_accept_drop_method,
    .drag = pico8_bv_dispatch,
    .drop = pico8_d__dispatch,
    .attach_slipway = planet_attach_slipway_method,
    .offer_trade = planet_offer_trade_method,
    .spawn_particle = pico8_g__mm,
    .states =
        {
            [ENTITY_STATE_VANISHING] = planet_update_vanishing_method,
            [ENTITY_STATE_SETTLED] = pico8_qh_ly_dispatch,
        },
    .tags = TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING),
    .lineage = LINEAGE(LINEAGE_PLANET) | LINEAGE(LINEAGE_STRUCTURE),
};

static planet_t *pico8_pm_struct_new(const planet_keys_t *keys) {
    planet_t *self = (planet_t *)allocator_alloc(planet_zone_construction(), sizeof *self);

    assert(g_family_up && "pico8_planet_family_init() must run first");
    pico8_hj_struct_init(self, &g_pm_struct_class, &protostar_archetype);
    if (keys != NULL) {
        if (keys->map_pos != NULL) {
            self->map_pos = keys->map_pos;
        }
        if (keys->pos != NULL) {
            self->pos = keys->pos;
        }
        if (keys->planet_id != NULL) {
            self->planet_id = keys->planet_id;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("protostar");
#endif
    pico8_pm_pq(self);
    pico8_f_((entity_t *)self);
    planet_construct(self);
    entity_construct(&self->header);
    return self;
}

void pico8_ga_pq(planet_t *self) {
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
    const planet_t *self = (const planet_t *)self_ptr;
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

    ms = self->speed;
    iu = self->acceleration_factor;
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

void pico8_ga_kj(p8_graphics_t *gfx, p8_memory_t *mem, planet_t *self, vec_t p) {
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
            if (int_seq_at(burst_trail_colors_seq.items, burst_trail_colors_seq.count, idx, &k_entry)) {
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
    self->is_dead = !third_taken;
}

static void pico8_ga_kj_dispatch(entity_t *self, const vec_t *ra) {
    vec_t p = ra != NULL ? *ra : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    pico8_ga_kj(dispatch_context_gfx(), dispatch_context_mem(), (planet_t *)self, p);
}

/* A burst's place is the zone of its `oe` (pico8_ga_struct_new): given
 * back last, the struct and `oe` with it. */
static void pico8_ga_rendre_dispatch(entity_t *self) {
    const allocator_t *place = ((planet_t *)self)->slipways->zone;

    pico8_hj_rendre_dispatch(self);
    arena_release(place);
}

static const entity_class_t g_ga_struct_class = {
    .draw = pico8_ga_kj_dispatch,
    .hit_test = planet_hit_test_method,
    .new_year = planet_new_year_method,
    .refresh = planet_refresh_method,
    .release = pico8_ga_rendre_dispatch,
    .left_press = planet_left_press_method,
    .accept_drop = planet_accept_drop_method,
    .attach_slipway = planet_attach_slipway_method,
    .offer_trade = planet_offer_trade_method,
    .spawn_particle = pico8_ga_mm,
    .states = {[ENTITY_STATE_VANISHING] = planet_update_vanishing_method},
    /* ga's cart list of tags is empty and replaces jo's: never jo's. */
    .tags = 0,
    .lineage = LINEAGE(LINEAGE_PLANET),
};

static planet_t *pico8_ga_struct_new(const pico8_ga_keys_t *keys) {
    const allocator_t *place = pico8_ga_place_prendre();
    planet_t *self = (planet_t *)allocator_alloc(place, sizeof *self);

    assert(g_family_up && "pico8_planet_family_init() must run first");
    planet_init_instance(self, &g_ga_struct_class, &burst_archetype);
    self->header.emitter = &self->emitter_own;
    self->emitter = &self->emitter_own;
    self->emitter->def = &self->archetype->emitter;
    self->emitter->qo = self->archetype->emitter.matrix_xx;
    self->emitter->ro = self->archetype->emitter.matrix_xy;
    self->emitter->qg = self->archetype->emitter.matrix_yx;
    self->emitter->q_ = self->archetype->emitter.matrix_yy;
    self->emitter->k_ = self->archetype->emitter.trail_colors;
    self->acceleration_factor = self->archetype->acceleration_factor;
    self->emitter->py = self->archetype->emitter.speed_factor;
    self->speed = self->archetype->speed;
    self->emitter->qy = NULL;
    self->outgoing_routes = &g_ga_by_shared;
    if (keys != NULL) {
        if (keys->jo.map_pos != NULL) {
            self->map_pos = keys->jo.map_pos;
        }
        if (keys->jo.pos != NULL) {
            self->pos = keys->jo.pos;
        }
        if (keys->ra_copie != NULL) {
            vec_t *ra = (vec_t *)allocator_alloc(place, sizeof *ra);

            assert(keys->jo.pos == NULL && "ga: a burst borrows its ra or copies it, not both");
            *ra = *keys->ra_copie;
            self->pos = ra;
        }
        if (keys->jo.planet_id != NULL) {
            self->planet_id = keys->jo.planet_id;
        }
        if (keys->pq != NULL) {
            self->emitter->qo = keys->pq->matrix_xx;
            self->emitter->ro = keys->pq->matrix_xy;
            self->emitter->qg = keys->pq->matrix_yx;
            self->emitter->q_ = keys->pq->matrix_yy;
            self->emitter->k_ = keys->pq->trail_colors;
            self->acceleration_factor = keys->pq->acceleration_factor;
            self->emitter->py = keys->pq->speed_factor;
            self->speed = keys->pq->speed;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("burst");
#endif
    pico8_ga_pq(self);
    planet_construct_in(self, place);
    entity_construct(&self->header);
    return self;
}

void pico8_planet_family_init(void) {
    assert(planet_ready() && "pico8_jo_init() must run first");
    pico8_planet_family_shutdown();
    draw_functions_init();
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

static const entity_kind_t g_qh_kind = {pico8_qh_kind_new, &structure_kind_record};

const entity_kind_t *pico8_qh_kind(void) {
    return &g_qh_kind;
}

const entity_class_t *pico8_qh_struct_class(void) {
    return &g_qh_struct_class;
}

planet_t *pico8_qh_new(const planet_keys_t *keys) {
    return pico8_qh_struct_new(keys);
}

int pico8_nl_ready(void) {
    return g_family_up;
}

static entity_t *pico8_nl_kind_new(const planet_keys_t *keys) {
    return &pico8_nl_struct_new(keys)->header;
}

static const entity_kind_t g_nl_kind = {pico8_nl_kind_new, &trading_hub_kind_record};

const entity_kind_t *pico8_nl_kind(void) {
    return &g_nl_kind;
}

const entity_class_t *pico8_nl_struct_class(void) {
    return &g_nl_struct_class;
}

planet_t *pico8_nl_new(const planet_keys_t *keys) {
    return pico8_nl_struct_new(keys);
}

int pico8_os_ready(void) {
    return g_family_up;
}

static entity_t *pico8_os_kind_new(const planet_keys_t *keys) {
    return &pico8_os_struct_new(keys)->header;
}

static const entity_kind_t g_os_kind = {pico8_os_kind_new, &processor_kind_record};

const entity_kind_t *pico8_os_kind(void) {
    return &g_os_kind;
}

const entity_class_t *pico8_os_struct_class(void) {
    return &g_os_struct_class;
}

planet_t *pico8_os_new(const planet_keys_t *keys) {
    return pico8_os_struct_new(keys);
}

int pico8_hj_ready(void) {
    return g_family_up;
}

static entity_t *pico8_hj_kind_new(const planet_keys_t *keys) {
    return &pico8_hj_struct_new(keys)->header;
}

static const entity_kind_t g_hj_kind = {pico8_hj_kind_new, &ascension_kind_record};

const entity_kind_t *pico8_hj_kind(void) {
    return &g_hj_kind;
}

const entity_class_t *pico8_hj_struct_class(void) {
    return &g_hj_struct_class;
}

planet_t *pico8_hj_new(const planet_keys_t *keys) {
    return pico8_hj_struct_new(keys);
}

int pico8_pm_ready(void) {
    return g_family_up;
}

static entity_t *pico8_pm_kind_new(const planet_keys_t *keys) {
    return &pico8_pm_struct_new(keys)->header;
}

static const entity_kind_t g_pm_kind = {pico8_pm_kind_new, &protostar_kind_record};

const entity_kind_t *pico8_pm_kind(void) {
    return &g_pm_kind;
}

const entity_class_t *pico8_pm_struct_class(void) {
    return &g_pm_struct_class;
}

planet_t *pico8_pm_new(const planet_keys_t *keys) {
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

static const entity_kind_t g_ga_kind = {pico8_ga_kind_new, &burst_kind_record};

const entity_kind_t *pico8_ga_kind(void) {
    return &g_ga_kind;
}

const entity_class_t *pico8_ga_struct_class(void) {
    return &g_ga_struct_class;
}

planet_t *pico8_ga_new(const pico8_ga_keys_t *keys) {
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
