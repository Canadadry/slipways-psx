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
/* The outgoing routes every burst shares; each init empties them. */
static entity_list_t g_burst_outgoing_routes;

void structure_update_settled_method(entity_t *self, p8_fixed_t t) {
    planet_bob(self, t);
}

static const entity_class_t structure_class_def = {
    .draw = planet_draw_method,
    .draw_overlay = targeting_tooltip_method,
    .hit_test = planet_hit_test_method,
    .new_year = planet_new_year_method,
    .refresh = planet_refresh_method,
    .release = planet_release_method,
    .left_press = planet_left_press_method,
    .accept_drop = planet_accept_drop_method,
    .drag = targeting_track_method,
    .drop = targeting_confirm_method,
    .attach_slipway = planet_attach_slipway_method,
    .offer_trade = planet_offer_trade_method,
    .states =
        {
            [ENTITY_STATE_VANISHING] = planet_update_vanishing_method,
            [ENTITY_STATE_SETTLED] = structure_update_settled_method,
        },
    .tags = TAG(TAG_STRUCTURE) | TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING),
    .lineage = LINEAGE(LINEAGE_PLANET) | LINEAGE(LINEAGE_STRUCTURE),
};

void structure_init_instance(planet_t *self, const entity_class_t *cls, const planet_archetype_t *archetype) {
    planet_init_instance(self, cls, archetype);
    self->header.targeting = &self->targeting_own;
    self->targeting = &self->targeting_own;
    self->targeting->archetype = &self->archetype->targeting;
    self->targeting->cost = self->archetype->targeting.cost;
    self->targeting->has_cost = self->archetype->targeting.has_cost;
    self->sprite = self->archetype->sprite;
    self->targeting->target = NULL;
}

static planet_t *structure_create(const planet_keys_t *keys) {
    planet_t *self = (planet_t *)allocator_alloc(planet_zone_construction(), sizeof *self);

    assert(g_family_up && "planet_family_init() must run first");
    structure_init_instance(self, &structure_class_def, &structure_archetype);
    if (keys != NULL) {
        assert(keys->map_pos == NULL && keys->pos == NULL &&
               "structure_create: the only construction key is planet_id (structure_icon_left_press); another key has no member "
               "-- stop, do not add one");
        if (keys->planet_id != NULL) {
            self->planet_id = keys->planet_id;
        }
    }
#ifdef CLASS_COVERAGE_BUILD
    class_coverage_record("structure");
#endif
    planet_construct(self);
    entity_construct(&self->header);
    return self;
}

void trading_hub_update_settled(planet_t *self, p8_fixed_t t) {
    if (self == NULL) {
        return;
    }
    planet_bob((entity_t *)self, t);

    assert(self->has_development_tier && "trading_hub_update_settled: a settled trading hub has a development tier; only ascension_draw checks for none");
    if (self->has_development_tier && self->development_tier >= p8_fixed_from_int(2)) {
        self->palette_block =
            p8_fixed_add(p8_fixed_from_int(8),
                            p8_fixed_div(p8_fixed_mod(t, p8_fixed_from_int(42)), p8_fixed_from_int(6)));
    }
}

static void trading_hub_update_settled_method(entity_t *self, p8_fixed_t t) {
    trading_hub_update_settled((planet_t *)self, t);
}

static const entity_class_t trading_hub_class_def = {
    .draw = planet_draw_method,
    .draw_overlay = targeting_tooltip_method,
    .hit_test = planet_hit_test_method,
    .new_year = planet_new_year_method,
    .refresh = planet_refresh_method,
    .release = planet_release_method,
    .left_press = planet_left_press_method,
    .accept_drop = planet_accept_drop_method,
    .drag = targeting_track_method,
    .drop = targeting_confirm_method,
    .attach_slipway = planet_attach_slipway_method,
    .offer_trade = planet_offer_trade_method,
    .states =
        {
            [ENTITY_STATE_VANISHING] = planet_update_vanishing_method,
            [ENTITY_STATE_SETTLED] = trading_hub_update_settled_method,
        },
    .tags = TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING),
    .lineage = LINEAGE(LINEAGE_PLANET) | LINEAGE(LINEAGE_STRUCTURE) | LINEAGE(LINEAGE_TRADING_HUB),
};

static planet_t *trading_hub_create(const planet_keys_t *keys) {
    planet_t *self = (planet_t *)allocator_alloc(planet_zone_construction(), sizeof *self);

    assert(g_family_up && "planet_family_init() must run first");
    structure_init_instance(self, &trading_hub_class_def, &trading_hub_archetype);
    self->palette_block = self->archetype->palette_block;
    if (keys != NULL) {
        assert(keys->map_pos == NULL && keys->pos == NULL &&
               "trading_hub_create: the only construction key is planet_id (structure_icon_left_press); another key has no member "
               "-- stop, do not add one");
        if (keys->planet_id != NULL) {
            self->planet_id = keys->planet_id;
        }
    }
#ifdef CLASS_COVERAGE_BUILD
    class_coverage_record("trading_hub");
#endif
    planet_construct(self);
    entity_construct(&self->header);
    return self;
}

static const entity_class_t processor_class_def = {
    .draw = planet_draw_method,
    .draw_overlay = targeting_tooltip_method,
    .hit_test = planet_hit_test_method,
    .new_year = planet_new_year_method,
    .refresh = planet_refresh_method,
    .release = planet_release_method,
    .left_press = planet_left_press_method,
    .accept_drop = planet_accept_drop_method,
    .drag = targeting_track_method,
    .drop = targeting_confirm_method,
    .attach_slipway = planet_attach_slipway_method,
    .offer_trade = planet_offer_trade_method,
    .states =
        {
            [ENTITY_STATE_VANISHING] = planet_update_vanishing_method,
            [ENTITY_STATE_SETTLED] = structure_update_settled_method,
        },
    .tags = TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING),
    .lineage = LINEAGE(LINEAGE_PLANET) | LINEAGE(LINEAGE_STRUCTURE),
};

static planet_t *processor_create(const planet_keys_t *keys) {
    planet_t *self = (planet_t *)allocator_alloc(planet_zone_construction(), sizeof *self);

    assert(g_family_up && "planet_family_init() must run first");
    structure_init_instance(self, &processor_class_def, &processor_archetype);
    if (keys != NULL) {
        assert(keys->map_pos == NULL && keys->pos == NULL &&
               "processor_create: the only construction key is planet_id (structure_icon_left_press); another key has no member "
               "-- stop, do not add one");
        if (keys->planet_id != NULL) {
            self->planet_id = keys->planet_id;
        }
    }
#ifdef CLASS_COVERAGE_BUILD
    class_coverage_record("processor");
#endif
    planet_construct(self);
    entity_construct(&self->header);
    return self;
}

void ascension_draw(p8_graphics_t *gfx, p8_memory_t *mem, planet_t *self, vec_t p) {
    if (self == NULL) {
        return;
    }
    if (self->has_development_tier && self->development_tier >= p8_fixed_from_int(2)) {
        emitter_update(gfx, mem, (entity_t *)self);
    }
    planet_draw(gfx, mem, self, p);
}

void ascension_draw_method(entity_t *self, const vec_t *screen_pos) {
    vec_t p = screen_pos != NULL ? *screen_pos : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    ascension_draw(dispatch_context_gfx(), dispatch_context_mem(), (planet_t *)self, p);
}

void ascension_release_method(entity_t *self) {
    planet_release_row(self);
    emitter_release_particles(self->emitter);
    planet_grab_arena_released((const planet_t *)self);
}

static const entity_class_t ascension_class_def = {
    .draw = ascension_draw_method,
    .draw_overlay = targeting_tooltip_method,
    .hit_test = planet_hit_test_method,
    .new_year = planet_new_year_method,
    .refresh = planet_refresh_method,
    .release = ascension_release_method,
    .left_press = planet_left_press_method,
    .accept_drop = planet_accept_drop_method,
    .drag = targeting_track_method,
    .drop = targeting_confirm_method,
    .attach_slipway = planet_attach_slipway_method,
    .offer_trade = planet_offer_trade_method,
    .spawn_particle = probe_spawn_particle,
    .states =
        {
            [ENTITY_STATE_VANISHING] = planet_update_vanishing_method,
            [ENTITY_STATE_SETTLED] = structure_update_settled_method,
        },
    .tags = TAG(TAG_ASCENSION) | TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING),
    .lineage = LINEAGE(LINEAGE_PLANET) | LINEAGE(LINEAGE_STRUCTURE),
};

void ascension_init_instance(planet_t *self, const entity_class_t *cls, const planet_archetype_t *archetype) {
    structure_init_instance(self, cls, archetype);
    self->header.emitter = &self->emitter_own;
    self->emitter = &self->emitter_own;
    self->emitter->archetype = &self->archetype->emitter;
    self->emitter->speed_factor = self->archetype->emitter.speed_factor;
    self->emitter->angle_slices = self->archetype->emitter.angle_slices;
    self->emitter->trail_colors = self->archetype->emitter.trail_colors;
    self->emitter->base_radius = self->archetype->emitter.base_radius;
    self->emitter->matrix_xx = self->archetype->emitter.matrix_xx;
    self->emitter->matrix_xy = self->archetype->emitter.matrix_xy;
    self->emitter->matrix_yx = self->archetype->emitter.matrix_yx;
    self->emitter->matrix_yy = self->archetype->emitter.matrix_yy;
    self->emitter->particles = NULL;
}

static planet_t *ascension_create(const planet_keys_t *keys) {
    planet_t *self = (planet_t *)allocator_alloc(planet_zone_construction(), sizeof *self);

    assert(g_family_up && "planet_family_init() must run first");
    ascension_init_instance(self, &ascension_class_def, &ascension_archetype);
    if (keys != NULL) {
        assert(keys->map_pos == NULL && keys->pos == NULL &&
               "ascension_create: the only construction key is planet_id (structure_icon_left_press); another key has no member "
               "-- stop, do not add one");
        if (keys->planet_id != NULL) {
            self->planet_id = keys->planet_id;
        }
    }
#ifdef CLASS_COVERAGE_BUILD
    class_coverage_record("ascension");
#endif
    emitter_init((entity_t *)self);
    planet_construct(self);
    entity_construct(&self->header);
    return self;
}

void protostar_construct(planet_t *self) {
    vec_t spawn_pos;
    burst_keys_t keys;
    planet_t *spawned;
    const vec_t *ra;

    if (self == NULL) {
        return;
    }

    ra = self->pos;
    spawn_pos = vec_add(ra != NULL ? *ra : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0)),
                             self->archetype->spawn_offset);

    keys.planet.planet_id = NULL;
    keys.planet.map_pos = NULL;
    keys.planet.pos = NULL;
    keys.pos_copy = &spawn_pos;
    keys.construct = &protostar_burst_construct_keys;

    spawned = burst_new(&keys);
    (void)spawned;
}

static const entity_class_t protostar_class_def = {
    .draw = ascension_draw_method,
    .draw_overlay = targeting_tooltip_method,
    .hit_test = planet_hit_test_method,
    .new_year = planet_new_year_method,
    .refresh = planet_refresh_method,
    .release = ascension_release_method,
    .left_press = planet_left_press_method,
    .accept_drop = planet_accept_drop_method,
    .drag = targeting_track_method,
    .drop = targeting_confirm_method,
    .attach_slipway = planet_attach_slipway_method,
    .offer_trade = planet_offer_trade_method,
    .spawn_particle = probe_spawn_particle,
    .states =
        {
            [ENTITY_STATE_VANISHING] = planet_update_vanishing_method,
            [ENTITY_STATE_SETTLED] = structure_update_settled_method,
        },
    .tags = TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING),
    .lineage = LINEAGE(LINEAGE_PLANET) | LINEAGE(LINEAGE_STRUCTURE),
};

static planet_t *protostar_create(const planet_keys_t *keys) {
    planet_t *self = (planet_t *)allocator_alloc(planet_zone_construction(), sizeof *self);

    assert(g_family_up && "planet_family_init() must run first");
    ascension_init_instance(self, &protostar_class_def, &protostar_archetype);
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
#ifdef CLASS_COVERAGE_BUILD
    class_coverage_record("protostar");
#endif
    protostar_construct(self);
    emitter_init((entity_t *)self);
    planet_construct(self);
    entity_construct(&self->header);
    return self;
}

void burst_construct(planet_t *self) {
    camera_t *mo;

    if (self == NULL) {
        return;
    }
    emitter_init((entity_t *)self);
    emitter_spawn((entity_t *)self, p8_fixed_from_int(140));

    if (main_camera() != NULL) {
        mo = main_camera_typed();
        assert(mo != NULL && "burst_construct: the camera placeholder has no shake; a burst before reset_world() stops the run");
        mo->shake = p8_fixed_from_int(3);
    }
}

particle_t *burst_spawn_particle(entity_t *self_ptr) {
    const planet_t *self = (const planet_t *)self_ptr;
    p8_fixed_t qd;
    p8_fixed_t ms;
    p8_fixed_t iu;
    p8_fixed_t py;
    vec_t p;
    vec_t v;
    vec_t a;
    particle_t *particle;

    qd = p8_fixed_add(p8_fixed_rnd(P8_FIXED_LIT(0x00018000, 1.5)), P8_FIXED_LIT(0x00008000, 0.5));
    if (p8_fixed_rnd_default() < P8_FIXED_LIT(0x00009999, 0.6)) {
        qd = p8_fixed_mul(p8_fixed_flr(p8_fixed_div(qd, P8_FIXED_LIT(0x00008000, 0.5))),
                              P8_FIXED_LIT(0x00008000, 0.5));
    }
    p = vec_from_polar(qd, p8_fixed_rnd_default());

    ms = self->speed;
    iu = self->acceleration_factor;
    py = self->emitter->speed_factor;

    v = vec_mul(p, ms);
    a = vec_mul(p, iu);

    particle = particle_take();
    particle->pos = p;
    particle->velocity = v;
    particle->has_acceleration = 1;
    particle->acceleration = a;
    particle->life = p8_fixed_add(p8_fixed_rnd(py), py);

    return particle;
}

void burst_draw(p8_graphics_t *graphics, p8_memory_t *memory, planet_t *self, vec_t screen_pos) {
    const pointer_list_t *particles = NULL;
    int third_taken = 0;
    int index;
    int screen_x;
    int screen_y;

    if (self == NULL) {
        return;
    }

    particles = self->emitter->particles;

    screen_x = p8_fixed_flr_int(screen_pos.x);
    screen_y = p8_fixed_flr_int(screen_pos.y);

    for (index = 1; index <= 3; index++) {
        const particle_t *pt = NULL;

        if (particles != NULL && index <= particles->count) {
            pt = particles->items[index - 1];
        }

        if (pt != NULL) {
            int idx;
            int color = 0;
            int k_entry;

            idx = p8_fixed_flr_int(p8_fixed_flr(p8_fixed_div(pt->life, p8_fixed_from_int(2))));
            /* TROU-21 */
            if (int_seq_at(burst_trail_colors_seq.items, burst_trail_colors_seq.count, idx, &k_entry)) {
                color = k_entry;
            }
            p8_circ(graphics, memory, screen_x, screen_y, p8_fixed_flr_int(vec_length(pt->pos)), color);
        }

        /* Only the THIRD slot decides is_dead: the burst dies when fewer
         * than 3 particles remain, not when none do. A flag, not the
         * particle: emitter_update may give it back before is_dead is
         * written. */
        third_taken = pt != NULL;
    }

    emitter_update(graphics, memory, (entity_t *)self);
    self->is_dead = !third_taken;
}

static void burst_draw_method(entity_t *self, const vec_t *screen_pos) {
    vec_t p = screen_pos != NULL ? *screen_pos : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    burst_draw(dispatch_context_gfx(), dispatch_context_mem(), (planet_t *)self, p);
}

/* A burst's slot is the zone of its slipways list (burst_create): given
 * back last, the struct and the list with it. */
static void burst_release_method(entity_t *self) {
    const allocator_t *slot = ((planet_t *)self)->slipways->zone;

    ascension_release_method(self);
    arena_release(slot);
}

static const entity_class_t burst_class_def = {
    .draw = burst_draw_method,
    .hit_test = planet_hit_test_method,
    .new_year = planet_new_year_method,
    .refresh = planet_refresh_method,
    .release = burst_release_method,
    .left_press = planet_left_press_method,
    .accept_drop = planet_accept_drop_method,
    .attach_slipway = planet_attach_slipway_method,
    .offer_trade = planet_offer_trade_method,
    .spawn_particle = burst_spawn_particle,
    .states = {[ENTITY_STATE_VANISHING] = planet_update_vanishing_method},
    /* A burst carries no tag, not even a planet's. */
    .tags = 0,
    .lineage = LINEAGE(LINEAGE_PLANET),
};

static planet_t *burst_create(const burst_keys_t *keys) {
    const allocator_t *slot = particle_burst_slot_take();
    planet_t *self = (planet_t *)allocator_alloc(slot, sizeof *self);

    assert(g_family_up && "planet_family_init() must run first");
    planet_init_instance(self, &burst_class_def, &burst_archetype);
    self->header.emitter = &self->emitter_own;
    self->emitter = &self->emitter_own;
    self->emitter->archetype = &self->archetype->emitter;
    self->emitter->matrix_xx = self->archetype->emitter.matrix_xx;
    self->emitter->matrix_xy = self->archetype->emitter.matrix_xy;
    self->emitter->matrix_yx = self->archetype->emitter.matrix_yx;
    self->emitter->matrix_yy = self->archetype->emitter.matrix_yy;
    self->emitter->trail_colors = self->archetype->emitter.trail_colors;
    self->acceleration_factor = self->archetype->acceleration_factor;
    self->emitter->speed_factor = self->archetype->emitter.speed_factor;
    self->speed = self->archetype->speed;
    self->emitter->particles = NULL;
    self->outgoing_routes = &g_burst_outgoing_routes;
    if (keys != NULL) {
        if (keys->planet.map_pos != NULL) {
            self->map_pos = keys->planet.map_pos;
        }
        if (keys->planet.pos != NULL) {
            self->pos = keys->planet.pos;
        }
        if (keys->pos_copy != NULL) {
            vec_t *ra = (vec_t *)allocator_alloc(slot, sizeof *ra);

            assert(keys->planet.pos == NULL && "burst_create: a burst borrows its position or copies it, not both");
            *ra = *keys->pos_copy;
            self->pos = ra;
        }
        if (keys->planet.planet_id != NULL) {
            self->planet_id = keys->planet.planet_id;
        }
        if (keys->construct != NULL) {
            self->emitter->matrix_xx = keys->construct->matrix_xx;
            self->emitter->matrix_xy = keys->construct->matrix_xy;
            self->emitter->matrix_yx = keys->construct->matrix_yx;
            self->emitter->matrix_yy = keys->construct->matrix_yy;
            self->emitter->trail_colors = keys->construct->trail_colors;
            self->acceleration_factor = keys->construct->acceleration_factor;
            self->emitter->speed_factor = keys->construct->speed_factor;
            self->speed = keys->construct->speed;
        }
    }
#ifdef CLASS_COVERAGE_BUILD
    class_coverage_record("burst");
#endif
    burst_construct(self);
    planet_construct_in(self, slot);
    entity_construct(&self->header);
    return self;
}

void planet_family_init(void) {
    assert(planet_ready() && "planet_init() must run first");
    planet_family_shutdown();
    draw_functions_init();
    g_family_up = 1;
    entity_list_init(&g_burst_outgoing_routes, game_zone_load_zone());
}

void planet_family_shutdown(void) {
    g_family_up = 0;
    entity_list_free(&g_burst_outgoing_routes);
}

int structure_ready(void) {
    return g_family_up;
}

static entity_t *structure_kind_new(const planet_keys_t *keys) {
    return &structure_create(keys)->header;
}

static const entity_kind_t structure_kind_def = {structure_kind_new, &structure_kind_record};

const entity_kind_t *structure_kind(void) {
    return &structure_kind_def;
}

const entity_class_t *structure_class(void) {
    return &structure_class_def;
}

planet_t *structure_new(const planet_keys_t *keys) {
    return structure_create(keys);
}

int trading_hub_ready(void) {
    return g_family_up;
}

static entity_t *trading_hub_kind_new(const planet_keys_t *keys) {
    return &trading_hub_create(keys)->header;
}

static const entity_kind_t trading_hub_kind_def = {trading_hub_kind_new, &trading_hub_kind_record};

const entity_kind_t *trading_hub_kind(void) {
    return &trading_hub_kind_def;
}

const entity_class_t *trading_hub_class(void) {
    return &trading_hub_class_def;
}

planet_t *trading_hub_new(const planet_keys_t *keys) {
    return trading_hub_create(keys);
}

int processor_ready(void) {
    return g_family_up;
}

static entity_t *processor_kind_new(const planet_keys_t *keys) {
    return &processor_create(keys)->header;
}

static const entity_kind_t processor_kind_def = {processor_kind_new, &processor_kind_record};

const entity_kind_t *processor_kind(void) {
    return &processor_kind_def;
}

const entity_class_t *processor_class(void) {
    return &processor_class_def;
}

planet_t *processor_new(const planet_keys_t *keys) {
    return processor_create(keys);
}

int ascension_ready(void) {
    return g_family_up;
}

static entity_t *ascension_kind_new(const planet_keys_t *keys) {
    return &ascension_create(keys)->header;
}

static const entity_kind_t ascension_kind_def = {ascension_kind_new, &ascension_kind_record};

const entity_kind_t *ascension_kind(void) {
    return &ascension_kind_def;
}

const entity_class_t *ascension_class(void) {
    return &ascension_class_def;
}

planet_t *ascension_new(const planet_keys_t *keys) {
    return ascension_create(keys);
}

int protostar_ready(void) {
    return g_family_up;
}

static entity_t *protostar_kind_new(const planet_keys_t *keys) {
    return &protostar_create(keys)->header;
}

static const entity_kind_t protostar_kind_def = {protostar_kind_new, &protostar_kind_record};

const entity_kind_t *protostar_kind(void) {
    return &protostar_kind_def;
}

const entity_class_t *protostar_class(void) {
    return &protostar_class_def;
}

planet_t *protostar_new(const planet_keys_t *keys) {
    return protostar_create(keys);
}

int burst_ready(void) {
    return g_family_up;
}

/* planet_kind_named("burst"): a planet's keys, no construction group. */
static entity_t *burst_kind_new(const planet_keys_t *keys) {
    burst_keys_t burst_keys;

    if (keys == NULL) {
        return &burst_create(NULL)->header;
    }
    burst_keys.planet = *keys;
    burst_keys.pos_copy = NULL;
    burst_keys.construct = NULL;
    return &burst_create(&burst_keys)->header;
}

static const entity_kind_t burst_kind_def = {burst_kind_new, &burst_kind_record};

const entity_kind_t *burst_kind(void) {
    return &burst_kind_def;
}

const entity_class_t *burst_class(void) {
    return &burst_class_def;
}

planet_t *burst_new(const burst_keys_t *keys) {
    return burst_create(keys);
}

/* The table planet_kind_named reads, by class name: protostar and burst
 * are the only kinds a research project turns a planet into. */
static const struct {
    const char *class_name;
    const entity_kind_t *(*kind)(void);
} planet_kinds[] = {
    {"protostar", protostar_kind},
    {"burst", burst_kind},
};

const entity_kind_t *planet_kind_named(const char *fz) {
    size_t i;

    for (i = 0; i < sizeof planet_kinds / sizeof planet_kinds[0]; i++) {
        if (strcmp(planet_kinds[i].class_name, fz) == 0) {
            return planet_kinds[i].kind();
        }
    }
    return NULL;
}
