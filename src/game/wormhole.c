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

static int g_wormhole_up = 0;

static const entity_class_t wormhole_too_far_target_class_def = {0};

/* The only one: never allocated, never freed; no writer, so a write
 * reaching it stops the run. */
static wormhole_too_far_target_t g_wormhole_too_far_target_wrapper = {
    {.class = &wormhole_too_far_target_class_def},
    wormhole_too_far_target_status_text,
};

static int wormhole_anchor(const entity_t *p) {
    return p != NULL && p->fold_anchor != NULL && *p->fold_anchor != 0;
}

void wormhole_construct(wormhole_t *self) {
    const allocator_t *zone = planet_zone_construction();
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
        n += wormhole_anchor(p);
    }
    ox = (entity_list_t *)allocator_alloc(zone, sizeof *ox);
    entity_list_init_reserve(ox, zone, n);
    it = entity_list_all(live_entities());
    while (entity_list_iter_next(&it, &p)) {
        if (wormhole_anchor(p)) {
            entity_list_add(ox, p);
        }
    }
    assert(ox->count == n && ox->capacity == n);
    self->anchors = ox;
}

void wormhole_drag(wormhole_t *self, vec_t *mp, vec_t *rp) {
    vec_t p_vec;
    entity_t *nearest = NULL;
    p8_fixed_t nq;
    p8_fixed_t threshold;

    targeting_track((entity_t *)self, mp, rp);
    if (self == NULL) {
        return;
    }

    p_vec = self->map_pos != NULL ? *self->map_pos : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    nq = entity_geom_nearest(p_vec, self->anchors, &nearest);
    self->anchor = nearest;

    threshold = tech_gate_if_unlocked("space folding", P8_FIXED_LIT(0x000030a3, 0.19), P8_FIXED_LIT(0x0000451e, 0.27));

    if (nq > threshold) {
        self->targeting->target = (entity_t *)wormhole_too_far_target_wrapper();
    }
}

static void wormhole_drag_method(entity_t *self, vec_t *qa, vec_t *ra) {
    wormhole_drag((wormhole_t *)self, qa, ra);
}

void wormhole_update_placing(wormhole_t *self, p8_fixed_t t) {
    p8_fixed_t threshold;
    p8_fixed_t one;
    p8_fixed_t reach;
    p8_fixed_t min_val;
    p8_fixed_t sin_term;
    p8_fixed_t result;

    if (self == NULL) {
        return;
    }

    threshold = tech_gate_if_unlocked("space folding", p8_fixed_from_int(29), p8_fixed_from_int(35));

    one = p8_fixed_from_int(1);
    reach = p8_fixed_div(t, p8_fixed_from_int(15));
    min_val = (reach < one) ? reach : one;
    sin_term =
        p8_fixed_mul(p8_fixed_sin(p8_fixed_div(t, p8_fixed_from_int(40))), P8_FIXED_LIT(0x00001999, 0.1));

    result = p8_fixed_mul(threshold, p8_fixed_add(min_val, sin_term));
    self->radius = result;
}

static void wormhole_update_placing_method(entity_t *self, p8_fixed_t t) {
    wormhole_update_placing((wormhole_t *)self, t);
}

void wormhole_drop(wormhole_t *self) {
    p8_fixed_t t;
    int result;

    if (self == NULL) {
        return;
    }

    t = self->header.state_time;

    /* targeting_confirm's side effects must not run before tick 15, so
     * this call cannot be hoisted out of the else branch. */
    if (t < p8_fixed_from_int(15)) {
        result = 1;
    } else {
        result = !targeting_confirm((entity_t *)self);
    }

    self->is_dead = result;
}

static void wormhole_drop_method(entity_t *self, entity_t *kv) {
    (void)kv;
    wormhole_drop((wormhole_t *)self);
}

void wormhole_update_unfolding(wormhole_t *self, p8_fixed_t t) {
    p8_fixed_t sr;
    vec_t self_p;
    const entity_list_t *planet_list;
    int i;

    if (self == NULL) {
        return;
    }

    sr = p8_fixed_add(p8_fixed_div(wormhole_reveal_radius(self, t), p8_fixed_from_int(128)),
                          P8_FIXED_LIT(0x000007ef, 0.031));

    self_p = self->map_pos != NULL ? *self->map_pos : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    planet_list = entities_by_tag("planet");
    if (planet_list != NULL) {
        for (i = 0; i < planet_list->count; i++) {
            entity_t *planet = planet_list->items[i];
            planet_t *pt;

            if (planet == NULL) {
                continue;
            }
            assert(planet_is_family(planet) && "wormhole_update_unfolding: the planet tag holds planets only");
            pt = (planet_t *)planet;

            if (pt->header.state != NULL && strcmp(pt->header.state, "unknown") == 0) {
                vec_t planet_map_pos =
                    pt->map_pos != NULL ? *pt->map_pos : vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
                vec_t diff = vec_sub(planet_map_pos, self_p);
                if (vec_length(diff) < sr) {
                    planet_roll_type(pt);
                }
            }
        }
    }

    self->is_dead = t > p8_fixed_from_int(25);
}

static void wormhole_update_unfolding_method(entity_t *self, p8_fixed_t t) {
    wormhole_update_unfolding((wormhole_t *)self, t);
}

p8_fixed_t wormhole_reveal_radius(wormhole_t *self, p8_fixed_t t) {
    p8_fixed_t sq = p8_fixed_sqrt(p8_fixed_div(t, p8_fixed_from_int(15)));
    p8_fixed_t one = p8_fixed_from_int(1);
    p8_fixed_t min_val = (sq < one) ? sq : one;
    p8_fixed_t oi = p8_fixed_from_int(0);

    if (self != NULL) {
        oi = self->radius;
    }
    return p8_fixed_mul(min_val, oi);
}

static void wormhole_release_method(entity_t *self) {
    planet_grab_arena_arena_released(((wormhole_t *)self)->anchors->zone);
}

/* The drag entry is wormhole_drag, which calls targeting_track itself and
 * then searches the nearest anchor. Putting targeting_track_method there,
 * as in the other targeting classes, would look uniform and would drop the
 * nearest-anchor search and the too-far target. */
static const entity_class_t wormhole_class_def = {
    .draw = wormhole_draw_method,
    .draw_overlay = targeting_tooltip_method,
    .drag = wormhole_drag_method,
    .drop = wormhole_drop_method,
    .release = wormhole_release_method,
    .states =
        {
            [ENTITY_STATE_PLACING] = wormhole_update_placing_method,
            [ENTITY_STATE_UNFOLDING] = wormhole_update_unfolding_method,
        },
    .tags = TAG(TAG_WORMHOLE),
};

static wormhole_t *wormhole_create(void) {
    wormhole_t *self;

    planet_grab_arena_open();
    self = (wormhole_t *)allocator_alloc(planet_zone_construction(), sizeof *self);

    entity_init_instance(&self->header, &wormhole_class_def, wormhole_archetype.state, wormhole_archetype.state_time);
    self->archetype = &wormhole_archetype;
    self->header.layer = &self->archetype->layer;
    self->header.pos = &self->pos;
    self->header.map_pos = &self->map_pos;
    self->header.targeting = &self->targeting_own;
    self->targeting = &self->targeting_own;
    self->targeting->archetype = &self->archetype->targeting;
    self->radius = self->archetype->radius;
    self->targeting->cost = self->archetype->targeting.cost;
    self->targeting->has_cost = self->archetype->targeting.has_cost;
    self->too_far_target = &g_wormhole_too_far_target_wrapper;
    self->anchors = NULL;
    self->pos = NULL;
    self->map_pos = NULL;
    self->targeting->target = NULL;
    self->anchor = NULL;
    self->header.is_dead = &self->is_dead;
    self->is_dead = 0;
#ifdef CLASS_COVERAGE_BUILD
    class_coverage_record("wormhole");
#endif
    wormhole_construct(self);
    planet_grab_arena_close();
    entity_construct(&self->header);
    return self;
}

void wormhole_init(void) {
    wormhole_shutdown();
    g_wormhole_up = 1;
}

void wormhole_shutdown(void) {
    g_wormhole_up = 0;
}

int wormhole_ready(void) {
    return g_wormhole_up;
}

const entity_class_t *wormhole_class(void) {
    return &wormhole_class_def;
}

wormhole_t *wormhole_new(void) {
    return wormhole_create();
}

wormhole_too_far_target_t *wormhole_too_far_target_wrapper(void) {
    return &g_wormhole_too_far_target_wrapper;
}

const entity_class_t *wormhole_too_far_target_class(void) {
    return &wormhole_too_far_target_class_def;
}
