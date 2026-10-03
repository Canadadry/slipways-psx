#include "game/slipway.h"

#include <assert.h>
#include <string.h>

#include "p8/audio.h"
#include "engine/dispatch_context.h"
#include "game/economy.h"
#include "game/economy_formulas.h"
#include "engine/entity_registry.h"
#include "game/main_economy.h"
#include "engine/geom.h"
#include "p8/input.h"
#include "engine/entity.h"
#include "game/planet.h"
#include "game/targeting.h"
#include "game/tech_gate.h"
#include "game/ui.h"
#include "engine/vec.h"
#include "engine/frame_zone.h"
#include "game/particle_zone.h"

static int g_slipway_up = 0;

static const entity_class_t g_slipway_too_far_class = {0};

/* What slipway_obstruction returns beyond range: never allocated, never
 * freed; the status text is fixed, only `targeted_by` is written, and the
 * next drag overwrites it. */
static slipway_too_far_t g_slipway_too_far = {{.class = &g_slipway_too_far_class}, "-too far-", NULL};

static const entity_class_t g_slipway_mouse_point_class = {0};

vec_t slipway_end_map_pos(const entity_t *end) {
    const vec_t *p = end != NULL && end->map_pos != NULL ? *end->map_pos : NULL;

    return p != NULL ? *p : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
}

vec_t slipway_end_pos(const entity_t *end) {
    const vec_t *ra = end != NULL && end->pos != NULL ? *end->pos : NULL;

    return ra != NULL ? *ra : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
}

static int slipway_infraspace_of(const entity_t *e, p8_fixed_t *out) {
    if (e == NULL || e->infraspace == NULL) {
        return 0;
    }
    *out = *e->infraspace;
    return 1;
}

void slipway_served_planets_add(slipway_served_planets_t *hg, const char *id) {
    assert(id != NULL &&
           "slipway_served_planets_add: a NULL identifier does not fit the member's type -- stop, do not widen the member");
    if (slipway_served_planets_has(hg, id)) {
        return;
    }
    assert(hg->count < SLIPWAY_SERVED_PLANETS_CAP && "slipway_served_planets_add: more planets than the galaxy holds (90) -- stop, do not widen the member");
    hg->ids[hg->count] = id;
    hg->count++;
}

int slipway_served_planets_has(const slipway_served_planets_t *hg, const char *id) {
    int i;

    if (hg == NULL || id == NULL) {
        return 0;
    }
    for (i = 0; i < hg->count; i++) {
        if (strcmp(hg->ids[i], id) == 0) {
            return 1;
        }
    }
    return 0;
}

void slipway_construct(slipway_t *self) {
    const allocator_t *zone;
    pointer_list_t *ts;

    if (self == NULL) {
        return;
    }

    p8_play_sfx(p8_fixed_from_int(50));

    zone = planet_zone_construction();
    ts = (pointer_list_t *)allocator_alloc(zone, sizeof *ts);
    pointer_list_init_reserve(ts, zone, SLIPWAY_TRANSIT_DOTS_RESERVE);
    self->transit_dots = ts;

    self->served_planets_own.count = 0;
    self->served_planets = &self->served_planets_own;
}

void slipway_drag(slipway_t *self, vec_t *mp) {
    ui_t *ok;
    entity_t *hovered;

    if (self == NULL) {
        return;
    }

    ok = main_ui();
    hovered = ok != NULL ? ok->hovered : NULL;

    if (hovered != NULL && hovered->class->accept_drop != NULL) {
        self->end = hovered;
    } else {
        self->point_pos = mp != NULL ? *mp : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
        self->point.pos = mp != NULL ? &self->point_pos : NULL;
        self->point_map_pos = vec_div(self->point_pos, p8_fixed_from_int(128));
        self->end = &self->point.header;
    }

    self->targeting->cost = economy_formulas_route_base_cost(slipway_end_map_pos(self->start), slipway_end_map_pos(self->end));
    self->targeting->has_cost = 1;

    /* The start's infraspace when it has one, else the end's; 0 counts as one. */
    self->infraspace = p8_fixed_from_int(0);
    self->has_infraspace = slipway_infraspace_of(self->start, &self->infraspace) || slipway_infraspace_of(self->end, &self->infraspace);
    self->header.infraspace = self->has_infraspace ? &self->infraspace : NULL;

    self->targeting->target = slipway_obstruction(self);
    if (self->targeting->target != NULL) {
        probe_link_target(self->targeting->target, &self->header);
    }

    self->layer = self->has_infraspace ? p8_fixed_from_int(2) : p8_fixed_from_int(4);
}

void slipway_drop(slipway_t *self, entity_t *end_planet) {
    int refused;

    if (self == NULL) {
        return;
    }

    /* targeting_confirm charges the player and advances the clock, so it
     * must stay behind the NULL end short-circuit. */
    refused = (end_planet == NULL) || !targeting_confirm((entity_t *)self);
    if (refused) {
        /* p8_stat has no case for 19, so it reads 0 and this sfx never
         * fires. It dereferences `input` without checking it, so the NULL
         * guard below is load-bearing. */
        const p8_input_t *input = dispatch_context_input();
        if (input != NULL && p8_stat(input, 19) == 50) {
            p8_play_sfx(p8_fixed_from_int(-1));
        }
        self->is_dead = 1;
        return;
    }

    self->fade_in_step = p8_fixed_from_int(1);
    self->end = end_planet;

    {
        entity_t *qu = self->start;
        int start_registered = qu != NULL && qu->class->attach_slipway != NULL;
        int end_registered;
        if (start_registered) {
            qu->class->attach_slipway(qu, &self->header);
        }
        end_registered = end_planet != NULL && end_planet->class->attach_slipway != NULL;
        if (end_registered) {
            end_planet->class->attach_slipway(end_planet, &self->header);
        }
        assert(start_registered && end_registered &&
               "slipway_drop: both endpoints must have attach_slipway, or the slipway never joins their slipways lists");
        (void)start_registered;
        (void)end_registered;
        economy_advance_clock(main_economy_typed(), p8_fixed_from_int(1));
    }
}

typedef struct {
    vec_t start_point;
    vec_t end_point;
} slipway_obstruction_node_ctx_t;

/* geom_near_segment leaves out the segment's two ends (fraction bounds 0.01
 * and 0.99), so the start and end nodes never match themselves here. */
static int slipway_obstruction_node_predicate(entity_t *e, void *ctx_ptr) {
    const slipway_obstruction_node_ctx_t *ctx = (const slipway_obstruction_node_ctx_t *)ctx_ptr;

    if (e == NULL) {
        return 0;
    }
    return geom_near_segment(ctx->start_point, ctx->end_point, slipway_end_map_pos(e), P8_FIXED_LIT(0x000000d1, 0.0032));
}

typedef struct {
    vec_t start_point;
    vec_t end_point;
    int self_has_infraspace;
    p8_fixed_t self_infraspace;
} slipway_obstruction_slipway_ctx_t;

/* `self` is already under the slipway tag when this runs; a segment never
 * crosses itself (geom_segments_cross), so no self-exclusion is needed. */
static int slipway_obstruction_slipway_predicate(entity_t *e, void *ctx_ptr) {
    const slipway_obstruction_slipway_ctx_t *ctx = (const slipway_obstruction_slipway_ctx_t *)ctx_ptr;
    const slipway_t *sw;

    if (e == NULL) {
        return 0;
    }
    assert(e->class == slipway_class() && "the slipway tag holds slipways only");
    sw = (const slipway_t *)e;
    if (sw->has_infraspace != ctx->self_has_infraspace || (sw->has_infraspace && sw->infraspace != ctx->self_infraspace)) {
        return 0;
    }
    return geom_segments_cross(ctx->start_point, ctx->end_point, slipway_end_map_pos(sw->start), slipway_end_map_pos(sw->end));
}

entity_t *slipway_obstruction(slipway_t *self) {
    entity_t *start_node;
    entity_t *end_node;
    vec_t start_point;
    vec_t end_point;
    p8_fixed_t threshold;

    if (self == NULL) {
        return NULL;
    }

    start_node = self->start;
    end_node = self->end;
    start_point = slipway_end_map_pos(start_node);
    end_point = slipway_end_map_pos(end_node);

    threshold = tech_gate_if_unlocked("space folding", P8_FIXED_LIT(0x00008000, 0.5), P8_FIXED_LIT(0x0001147a, 1.08));

    if (vec_length_squared(vec_sub(start_point, end_point)) > threshold) {
        return &g_slipway_too_far.header;
    }

    {
        const entity_list_t *oe = NULL;

        if (start_node != NULL) {
            assert(planet_is_family(start_node) && "a slipway's start is a planet: slipway_create asserts it");
            oe = ((const planet_t *)start_node)->slipways;
        }
        if (oe != NULL) {
            entity_list_iter_t it = entity_list_all(oe);
            entity_t *w;
            while (entity_list_iter_next(&it, &w)) {
                if (w == NULL) {
                    continue;
                }
                assert(w->class == slipway_class() &&
                       "the start's slipways list holds slipways: only planet_attach_slipway adds to it, and only slipways");
                if (slipway_other_end((const slipway_t *)w, start_node) == end_node) {
                    return w;
                }
            }
        }
    }

    {
        slipway_obstruction_node_ctx_t node_ctx;
        slipway_obstruction_slipway_ctx_t slipway_ctx;
        entity_list_t part1;
        entity_list_t part2;
        entity_list_t ds;
        entity_t *result = NULL;

        node_ctx.start_point = start_point;
        node_ctx.end_point = end_point;
        slipway_ctx.start_point = start_point;
        slipway_ctx.end_point = end_point;
        slipway_ctx.self_has_infraspace = self->has_infraspace;
        slipway_ctx.self_infraspace = self->infraspace;

        part1 = entity_list_filter(frame_zone(), entities_by_tag("node"), slipway_obstruction_node_predicate, &node_ctx);
        part2 = entity_list_filter(frame_zone(), entities_by_tag("slipway"), slipway_obstruction_slipway_predicate, &slipway_ctx);
        ds = entity_list_concat(frame_zone(), &part1, &part2);

        if (ds.count > 0) {
            result = ds.items[0];
        }
        return result;
    }
}

entity_t *slipway_other_end(const slipway_t *self, const entity_t *planet_instance) {
    if (self == NULL) {
        return NULL;
    }

    /* A NULL end falls through to the start even when `planet_instance` is
     * the start, so this is not a plain ternary. */
    if (planet_instance == self->start && self->end != NULL) {
        return self->end;
    }
    return self->start;
}

void slipway_draw_overlay(slipway_t *self) {
    if (self == NULL) {
        return;
    }

    if (self->start != self->end) {
        targeting_tooltip(dispatch_context_gfx(), dispatch_context_mem(), (entity_t *)self);
    }
}

static void slipway_drag_method(entity_t *self, vec_t *qa, vec_t *ra) {
    (void)ra;
    slipway_drag((slipway_t *)self, qa);
}

static void slipway_drop_method(entity_t *self, entity_t *kv) {
    slipway_drop((slipway_t *)self, kv);
}

static void slipway_draw_overlay_method(entity_t *self, const vec_t *ra) {
    (void)ra;
    slipway_draw_overlay((slipway_t *)self);
}

static void slipway_release_method(entity_t *self) {
    slipway_t *jc = (slipway_t *)self;
    const allocator_t *zone = jc->transit_dots->zone;

    for (int i = 0; i < jc->transit_dots->count; i++)
        allocator_release(particle_zone(PARTICLE_SLOT_TRANSIT_DOT), jc->transit_dots->items[i]);
    jc->transit_dots->count = 0;
    planet_grab_arena_zone_released(zone, jc->header.state);
}

static int slipway_is_endpoint_class(const entity_class_t *cls) {
    return cls->states[ENTITY_STATE_SETTLED] != NULL || cls == planet_class();
}

static const entity_class_t g_slipway_class = {
    .draw = slipway_draw_method,
    .draw_overlay = slipway_draw_overlay_method,
    .drag = slipway_drag_method,
    .drop = slipway_drop_method,
    .release = slipway_release_method,
    .tags = TAG(TAG_SLIPWAY),
};

static slipway_t *slipway_create(const slipway_keys_t *keys) {
    slipway_t *self;

    planet_grab_arena_open();
    self = (slipway_t *)allocator_alloc(planet_zone_construction(), sizeof *self);

    entity_init_instance(&self->header, &g_slipway_class, slipway_archetype.state, slipway_archetype.state_time);
    self->archetype = &slipway_archetype;
    self->layer = self->archetype->layer;
    self->header.layer = &self->layer;
    self->header.targeting = &self->targeting_own;
    self->targeting = &self->targeting_own;
    self->targeting->archetype = &self->archetype->targeting;
    self->fade_in_step = self->archetype->fade_in_step;
    self->start = NULL;
    self->transit_dots = NULL;
    self->served_planets = NULL;
    self->end = NULL;
    self->targeting->cost = self->archetype->targeting.cost;
    self->targeting->has_cost = self->archetype->targeting.has_cost;
    self->infraspace = 0;
    self->has_infraspace = 0;
    self->header.infraspace = NULL;
    self->targeting->target = NULL;
    self->targeted_by = NULL;
    self->header.is_dead = &self->is_dead;
    self->is_dead = 0;
    entity_init_header(&self->point.header, &g_slipway_mouse_point_class);
    self->point.header.pos = &self->point.pos;
    self->point.header.map_pos = &self->point.map_pos;
    self->point.pos = NULL;
    self->point.map_pos = &self->point_map_pos;
    self->point_pos = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    self->point_map_pos = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    if (keys != NULL && keys->start != NULL) {
        assert(slipway_is_endpoint_class(keys->start->class) &&
               "slipway_create: a start key that is neither NULL nor a planet or a structure "
               "does not fit the member's type -- stop, do not widen the member");
        self->start = keys->start;
    }
#ifdef CLASS_COVERAGE_BUILD
    class_coverage_record("slipway");
#endif
    slipway_construct(self);
    planet_grab_arena_close();
    entity_construct(&self->header);
    return self;
}

void slipway_init(void) {
    slipway_shutdown();
    g_slipway_up = 1;
}

void slipway_shutdown(void) {
    g_slipway_up = 0;
}

int slipway_ready(void) {
    return g_slipway_up;
}

const entity_class_t *slipway_class(void) {
    return &g_slipway_class;
}

slipway_t *slipway_new(const slipway_keys_t *keys) {
    return slipway_create(keys);
}

const entity_class_t *slipway_too_far_class(void) {
    return &g_slipway_too_far_class;
}

const entity_class_t *slipway_mouse_point_class(void) {
    return &g_slipway_mouse_point_class;
}
