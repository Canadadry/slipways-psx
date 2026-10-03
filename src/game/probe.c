#include "game/probe.h"

#include <assert.h>
#include <string.h>

#include "engine/dispatch_context.h"
#include "game/economy.h"
#include "engine/entity_registry.h"
#include "engine/entity_geom.h"
#include "p8/fixed.h"
#include "game/main_economy.h"
#include "engine/geom.h"
#include "p8/gfx.h"
#include "game/hud.h"
#include "p8/audio.h"
#include "engine/entity.h"
#include "game/particle.h"
#include "game/planet.h"
#include "game/slipway.h"
#include "game/ui.h"
#include "game/wormhole.h"
#include "p8/str.h"
#include "engine/frame_zone.h"

static int g_probe_up = 0;
static entity_t *g_main_economy = NULL;

static const entity_class_t main_economy_placeholder_class_def = {0};

static main_economy_placeholder_t g_main_economy_placeholder = {
    {.class = &main_economy_placeholder_class_def}, 0, 0, 0, 0};

static int targeting_track_target_is_holder_class(const entity_class_t *cls) {
    return cls->drag == targeting_track_method || cls == wormhole_class();
}

static const entity_class_t targeting_track_target_class_def = {0};

/* The target targeting_track sets over the void, off the panel: never
 * allocated nor freed; only targeted_by is written, overwritten next time. */
static targeting_track_target_t g_targeting_track_target = {{.class = &targeting_track_target_class_def}, NULL};

static int probe_to_int(p8_fixed_t x) {
    return p8_fixed_flr_int(x);
}

typedef struct {
    vec_t self_map_pos;
} targeting_track_ctx_t;

static int targeting_track_slipway_predicate(entity_t *e, void *ctx_ptr) {
    const targeting_track_ctx_t *ctx = (const targeting_track_ctx_t *)ctx_ptr;
    const slipway_t *sw;
    vec_t start_p;
    vec_t end_p;

    if (e == NULL) {
        return 0;
    }
    assert(e->class == slipway_class() && "the slipway tag holds slipways only");
    sw = (const slipway_t *)e;

    start_p = slipway_end_map_pos(sw->start);
    end_p = slipway_end_map_pos(sw->end);

    return geom_near_segment(start_p, end_p, ctx->self_map_pos, P8_FIXED_LIT(0x000000d1, 0.0032));
}

/* The panel box is read off the UI archetype, not off main_ui(); while the
 * UI module is down there is none: the point reads as outside it. */
static int probe_panel_box_contains(vec_t rp) {
    const box_t *hw = ui_panel_box();

    if (hw == NULL) {
        return 0;
    }
    return box_contains(hw, rp);
}

/* pos and map_pos, which a host of targeting_track holds for itself: the
 * probe line, the planet family or a wormhole. */
static void probe_host_set_positions(entity_t *self, vec_t *ra, vec_t *p) {
    probe_t *g_;
    planet_t *jo;
    wormhole_t *mz;

    if (self->class->lineage & LINEAGE(LINEAGE_PROBE)) {
        g_ = (probe_t *)self;
        g_->pos = ra;
        g_->map_pos = p;
        return;
    }
    if (planet_is_family(self)) {
        jo = (planet_t *)self;
        jo->pos = ra;
        jo->map_pos = p;
        return;
    }
    assert(self->class == wormhole_class() &&
           "targeting_track: a host neither a probe, of the planet family nor a wormhole, none of whose drag runs it -- stop");
    mz = (wormhole_t *)self;
    mz->pos = ra;
    mz->map_pos = p;
}

/* What targeting_track aims at: the node tag (planets, and the structure
 * classes: exactly those with a settled state), a slipway, or the throwaway. */
static int probe_is_target_class(const entity_class_t *cls) {
    return cls->states[ENTITY_STATE_SETTLED] != NULL || cls == planet_class() ||
           cls == slipway_class() || cls == targeting_track_target_class();
}

void probe_link_target(entity_t *et, entity_t *self) {
    if (planet_is_family(et)) {
        assert((self->class->drag == targeting_track_method || self->class == slipway_class()) &&
               "probe_link_target: a planet's targeted_by takes NULL, a host whose drag is targeting_track_method "
               "or a slipway, nothing else -- stop, do not widen the member");
        ((planet_t *)et)->targeted_by = self;
        return;
    }
    if (et->class == slipway_class()) {
        assert((self->class->drag == targeting_track_method || self->class == slipway_class() ||
                self->class == wormhole_class()) &&
               "probe_link_target: a slipway's targeted_by takes NULL, a host whose drag is targeting_track_method, "
               "a wormhole or another slipway, nothing else -- stop");
        ((slipway_t *)et)->targeted_by = self;
        return;
    }
    if (et->class == slipway_too_far_class()) {
        assert(self->class == slipway_class() &&
               "probe_link_target: the too-far slipway struct's targeted_by takes NULL or a slipway, nothing else "
               "-- stop, do not widen the member");
        ((slipway_too_far_t *)et)->targeted_by = (slipway_t *)self;
        return;
    }
    assert(et == &g_targeting_track_target.header && "probe_link_target: the target is a planet, a slipway or the static throwaway struct, nothing else");
    assert(targeting_track_target_is_holder_class(self->class) &&
           "probe_link_target: the throwaway target's targeted_by takes NULL, a host whose drag is "
           "targeting_track_method or a wormhole, nothing else -- stop, do not widen the member");
    g_targeting_track_target.targeted_by = self;
}

void targeting_track(entity_t *self, vec_t *mouse_world, vec_t *mouse_screen) {
    targeting_t *targeting;
    vec_t mouse_world_vec;
    vec_t p_vec;
    p8_fixed_t distance;
    entity_t *candidate;
    entity_t *target;

    if (self == NULL) {
        return;
    }
    assert(self->targeting != NULL && "targeting_track: a class that carries no targeting is no receiver -- stop");

    targeting = self->targeting;
    mouse_world_vec = mouse_world != NULL ? *mouse_world : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    p_vec = vec_div(mouse_world_vec, p8_fixed_from_int(128));
    targeting->track_pos = mouse_world_vec;
    targeting->track_map_pos = p_vec;
    probe_host_set_positions(self, mouse_world != NULL ? &targeting->track_pos : NULL, &targeting->track_map_pos);

    candidate = NULL;
    distance = entity_geom_nearest(p_vec, entities_by_tag("node"), &candidate);

    target = NULL;

    if (distance < targeting->archetype->attach_distance && candidate != NULL) {
        target = candidate;
    } else {
        targeting_track_ctx_t ctx;
        entity_list_t filtered;
        int have_first = 0;
        entity_t *first = NULL;

        ctx.self_map_pos = p_vec;
        filtered = entity_list_filter(frame_zone(), entities_by_tag("slipway"), targeting_track_slipway_predicate, &ctx);
        if (filtered.count > 0) {
            first = filtered.items[0];
            have_first = 1;
        }

        if (have_first && first != NULL) {
            target = first;
        } else {
            vec_t mouse_screen_vec = mouse_screen != NULL ? *mouse_screen : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
            if (!probe_panel_box_contains(mouse_screen_vec)) {
                target = &g_targeting_track_target.header;
            } else {
                target = NULL;
            }
        }
    }

    assert((target == NULL ||
            probe_is_target_class(target->class)) &&
           "targeting_track: the target is NULL or what it aims at (a planet, a class with a settled state, a slipway, "
           "or the static throwaway struct), nothing else");
    targeting->target = target;

    if (targeting->target != NULL) {
        probe_link_target(targeting->target, self);
    }
}

void targeting_track_method(entity_t *self, vec_t *mouse_world, vec_t *mouse_screen) {
    targeting_track(self, mouse_world, mouse_screen);
}

/* The status_text of the tooltip's target: the wormhole's and the slipway's
 * too-far structs have one; any other target reads as absent, NULL. */
static const char *probe_target_status_text(const entity_t *et) {
    if (et->class == wormhole_too_far_target_class()) {
        return ((const wormhole_too_far_target_t *)et)->status_text;
    }
    if (et->class == slipway_too_far_class()) {
        return ((const slipway_too_far_t *)et)->status_text;
    }
    return NULL;
}

void targeting_tooltip(const p8_graphics_t *gfx, p8_memory_t *mem, entity_t *self) {
    if (self == NULL) {
        return;
    }
    assert(self->targeting != NULL && "targeting_tooltip: a class that carries no targeting is no receiver -- stop");

    targeting_tooltip_show(gfx, mem, self->state, self->targeting);
}

void targeting_tooltip_show(const p8_graphics_t *gfx, p8_memory_t *mem, const char *state, const targeting_t *pb) {
    vec_t mp;
    const mouse_t *je;
    const char *display = "";

    if (state == NULL || strcmp(state, "placing") != 0) {
        return;
    }

    je = main_mouse_typed();
    mp = je != NULL && je->pos != NULL ? *je->pos : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));

    if (pb->target != NULL) {
        const char *qs = probe_target_status_text(pb->target);
        if (qs != NULL) {
            display = qs;
        } else {
            if (gfx != NULL && mem != NULL) {
                p8_spr_default(gfx, mem, 31, probe_to_int(p8_fixed_sub(mp.x, p8_fixed_from_int(3))),
                                   probe_to_int(p8_fixed_sub(mp.y, p8_fixed_from_int(8))));
            }
        }
    } else if (pb->has_cost) {
        display = p8_concat2(frame_zone(), p8_tostr_fixed(frame_zone(), pb->cost), "$");
    }

    if (gfx != NULL && mem != NULL) {
        p8_fixed_t a = P8_FIXED_LIT(0x00008000, 0.5);
        hud_print_outlined(gfx, mem, display, mp.x, p8_fixed_sub(mp.y, p8_fixed_from_int(8)), 8, &a);
    }
}

void targeting_tooltip_method(entity_t *self, const vec_t *screen_pos) {
    (void)screen_pos;
    targeting_tooltip(dispatch_context_gfx(), dispatch_context_mem(), self);
}

/* Raises is_dead, which a host of targeting_confirm holds for itself: the
 * probe line, the planet family, a slipway or a wormhole. */
static void probe_host_raise_is_dead(entity_t *self) {
    assert(self->is_dead != NULL && "targeting_confirm: a host that carries no is_dead, none of whose drop runs it -- stop");
    *self->is_dead = 1;
}

int targeting_confirm(entity_t *self) {
    const targeting_t *targeting;
    int has_target;
    economy_t *fm;
    int spend_ok = 0;

    if (self == NULL) {
        return 0;
    }
    assert(self->targeting != NULL && "targeting_confirm: a class that carries no targeting is no receiver -- stop");

    targeting = self->targeting;
    has_target = targeting->target != NULL;

    fm = main_economy_typed();
    if (!has_target) {
        spend_ok = economy_spend(fm, targeting->has_cost ? targeting->cost : p8_fixed_from_int(0));
    }

    assert((!has_target || !spend_ok) && "targeting_confirm: economy_spend must stay short-circuited when a target is set");

    if (!has_target && spend_ok) {
        entity_set_state(self, targeting->archetype->confirmed_state != NULL ? targeting->archetype->confirmed_state : "normal");

        economy_advance_clock(main_economy_typed(), targeting->archetype->clock_ticks);

        p8_play_sfx(targeting->archetype->has_status_sprite ? targeting->archetype->status_sprite : p8_fixed_from_int(54));

        return 1;
    }

    p8_play_sfx(p8_fixed_from_int(57));
    probe_host_raise_is_dead(self);
    return 0;
}

void targeting_confirm_method(entity_t *self, entity_t *dropped) {
    (void)dropped;
    (void)targeting_confirm(self);
}

particle_t *probe_spawn_particle(entity_t *self) {
    const emitter_t *emitter = NULL;
    p8_fixed_t angle_slices = 0, state_time = 0, base_radius = 0, speed_factor = 0, lifetime = 0;
    int pk = 0;
    p8_fixed_t a, rnd_angle_slices, rnd_half;
    vec_t p, v;
    particle_t *particle;

    if (self != NULL) {
        assert(self->emitter != NULL && "probe_spawn_particle: a class that carries no particle emitter spawns nothing -- stop");
        emitter = self->emitter;
        angle_slices = emitter->angle_slices;
        state_time = self->state_time;
        base_radius = emitter->base_radius;
        speed_factor = emitter->speed_factor;
        lifetime = emitter->archetype->lifetime;
        pk = emitter->archetype->quarter_turn != 0;
    }

    /* Two statements, not two arguments: C leaves argument order unspecified
     * (gcc drew rnd(0.5) first), and rnd(angle_slices) must come first. */
    rnd_angle_slices = p8_fixed_rnd(angle_slices);
    rnd_half = p8_fixed_rnd(P8_FIXED_LIT(0x00008000, 0.5));
    a = p8_fixed_add(
        p8_fixed_div(p8_fixed_add(p8_fixed_flr(rnd_angle_slices), rnd_half), angle_slices),
        p8_fixed_mul(state_time, P8_FIXED_LIT(0x00000041, 0.001)));

    p = vec_from_polar(p8_fixed_add(base_radius, p8_fixed_rnd_default()), a);
    v = vec_mul(p, speed_factor);

    if (pk) {
        v = vec_perpendicular(v);
    }

    particle = particle_take();
    particle->pos = p;
    particle->velocity = v;
    particle->has_acceleration = 0;
    particle->acceleration = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    particle->life = lifetime;

    return particle;
}

void probe_release_method(entity_t *self) {
    const probe_t *g_ = (const probe_t *)self;

    emitter_release_particles(self->emitter);
    if (g_->grab_arena != NULL && (self->state == NULL || strcmp(self->state, "settled") != 0)) {
        planet_grab_arena_arena_released(g_->grab_arena);
    }
}

static const entity_class_t probe_class_def = {
    .draw = emitter_update_method,
    .draw_overlay = targeting_tooltip_method,
    .drag = targeting_track_method,
    .drop = targeting_confirm_method,
    .spawn_particle = probe_spawn_particle,
    .release = probe_release_method,
    .tags = TAG(TAG_PROBE) | TAG(TAG_TARGETING),
    .lineage = LINEAGE(LINEAGE_PROBE),
};

void probe_init_instance(probe_t *self, const entity_class_t *cls, const probe_archetype_t *archetype) {
    entity_init_instance(&self->header, cls, archetype->state, archetype->state_time);
    self->archetype = archetype;
    self->header.layer = &self->archetype->layer;
    self->header.pos = &self->pos;
    self->header.map_pos = &self->map_pos;
    self->header.fold_anchor = &self->archetype->fold_anchor;
    self->header.is_dead = &self->is_dead;
    self->header.emitter = &self->emitter_own;
    self->header.targeting = &self->targeting_own;
    self->emitter = &self->emitter_own;
    self->emitter->archetype = &self->archetype->emitter;
    self->emitter->particles = NULL;
    self->pos = self->archetype->has_pos ? &self->archetype->pos : NULL;
    self->map_pos = NULL;
    self->targeting = &self->targeting_own;
    self->targeting->archetype = &self->archetype->targeting;
    self->targeting->target = NULL;
    self->is_dead = 0;
    self->grab_arena = NULL;
    self->targeting->cost = self->archetype->targeting.cost;
    self->targeting->has_cost = self->archetype->targeting.has_cost;
    self->emitter->speed_factor = self->archetype->emitter.speed_factor;
    self->emitter->angle_slices = self->archetype->emitter.angle_slices;
    self->emitter->trail_colors = self->archetype->emitter.trail_colors;
    self->emitter->matrix_xx = self->archetype->emitter.matrix_xx;
    self->emitter->matrix_xy = self->archetype->emitter.matrix_xy;
    self->emitter->matrix_yx = self->archetype->emitter.matrix_yx;
    self->emitter->matrix_yy = self->archetype->emitter.matrix_yy;
    self->emitter->base_radius = self->archetype->emitter.base_radius;
}

static probe_t *probe_create(const probe_keys_t *keys) {
    probe_t *self;

    assert(g_probe_up && "probe_init() must run first");
    planet_grab_arena_open();
    self = (probe_t *)allocator_alloc(planet_zone_construction(), sizeof *self);
    probe_init_instance(self, &probe_class_def, &probe_archetype);
    self->grab_arena = planet_zone_construction();
    planet_grab_arena_close();
    if (keys != NULL) {
        self->header.state_time = keys->state_time;
        if (keys->has_angle_slices) {
            self->emitter->angle_slices = keys->angle_slices;
        }
        if (keys->has_base_radius) {
            self->emitter->base_radius = keys->base_radius;
        }
        if (keys->has_speed_factor) {
            self->emitter->speed_factor = keys->speed_factor;
        }
    }
    emitter_init((entity_t *)self);
    entity_construct(&self->header);
    return self;
}

void probe_init(void) {
    probe_shutdown();

    g_probe_up = 1;

    g_main_economy_placeholder.research_points = 0;
    g_main_economy_placeholder.has_research_points = 0;
    g_main_economy_placeholder.credits = 0;
    g_main_economy_placeholder.has_credits = 0;
    g_main_economy = (entity_t *)&g_main_economy_placeholder;
}

void probe_shutdown(void) {
    g_probe_up = 0;
    g_main_economy = NULL;
}

int probe_ready(void) {
    return g_probe_up;
}

const entity_class_t *probe_class(void) {
    return &probe_class_def;
}

probe_t *probe_new(const probe_keys_t *keys) {
    return probe_create(keys);
}

const entity_class_t *main_economy_placeholder_class(void) {
    return &main_economy_placeholder_class_def;
}

entity_t *main_economy(void) {
    return g_main_economy;
}

economy_t *main_economy_typed(void) {
    if (g_main_economy == NULL || g_main_economy == &g_main_economy_placeholder.header) {
        return NULL;
    }
    assert(g_main_economy->class == economy_class() && "main_economy_typed: only the placeholder or an economy is ever stored");
    return (economy_t *)g_main_economy;
}

const main_economy_placeholder_t *main_economy_placeholder(void) {
    return g_main_economy == &g_main_economy_placeholder.header ? &g_main_economy_placeholder : NULL;
}

void probe_set_main_economy(entity_t *instance) {
    g_main_economy = instance;
}

const entity_class_t *targeting_track_target_class(void) {
    return &targeting_track_target_class_def;
}
