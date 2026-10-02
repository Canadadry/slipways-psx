#include "game/planet.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "engine/collect.h"
#include "engine/entity_registry.h"
#include "engine/entity_geom.h"
#include "game/gen.h"
#include "engine/entity.h"
#include "game/message_box.h"
#include "game/draw_functions.h"
#include "game/research.h"
#include "engine/frame_zone.h"
#include "engine/game_zone.h"

static int g_planet_up = 0;

static weighted_picker_t *g_type_picker = NULL;
static random_walk_t *g_planet_vanish_roll = NULL;

/* Shared by every bare planet: written in place by `bb` and `mx:kf` until
 * `lz` gives the planet its own. */
static production_t g_planet_production_shared;

/* Nom des arenes de prise : son adresse les reconnait (pico8_jo_prise_arene_rendue). */
static const char g_planet_name_grab_arena[] = "prise";
#define GRAB_ARENAS_FREE_MAX 8

/* Non NULL seulement entre pico8_jo_prise_ouvrir et _fermer. */
static const allocator_t *g_planet_grab_arena = NULL;
static const allocator_t *g_planet_grab_arenas_free[GRAB_ARENAS_FREE_MAX];
static int g_planet_grab_arenas_free_n = 0;

int planet_hit_test_method(entity_t *self, const vec_t *qa, const vec_t *ra) {
    return entity_geom_contains_cursor(self, qa, ra);
}

void planet_new_year_method(entity_t *self, p8_fixed_t yr) {
    (void)yr;
    planet_new_year((const planet_t *)self);
}

int planet_is_family(const entity_t *e) {
    return (e->class->lineage & LINEAGE(LINEAGE_PLANET)) != 0;
}

void planet_update_vanishing_method(entity_t *self, p8_fixed_t t) {
    planet_update_vanishing((planet_t *)self, t);
}

static const entity_class_t g_planet_class = {
    .draw = planet_draw_method,
    .hit_test = planet_hit_test_method,
    .new_year = planet_new_year_method,
    .refresh = planet_refresh_method,
    .release = planet_release_method,
    .left_press = planet_left_press_method,
    .accept_drop = planet_accept_drop_method,
    .attach_slipway = planet_attach_slipway_method,
    .offer_trade = planet_offer_trade_method,
    .states = {[ENTITY_STATE_VANISHING] = planet_update_vanishing_method},
    .tags = TAG(TAG_PLANET) | TAG(TAG_NODE) | TAG(TAG_TARGETING),
    .lineage = LINEAGE(LINEAGE_PLANET),
};

void planet_init_instance(planet_t *self, const entity_class_t *cls, const planet_archetype_t *archetype) {
    assert(g_planet_up && "pico8_jo_init() must run first");
    entity_init_instance(&self->header, cls, archetype->state, archetype->state_time);
    self->archetype = archetype;
    self->header.layer = &self->archetype->layer;
    /* Read-only: the planet modules write through their members. */
    self->header.pos = (const vec_t *const *)&self->pos;
    self->header.map_pos = (const vec_t *const *)&self->map_pos;
    self->header.hit_box = &self->archetype->hit_box;
    self->header.shake = &self->shake;
    self->header.fold_anchor = &self->fold_anchor;
    self->header.is_dead = &self->is_dead;
    self->shake = self->archetype->shake;
    self->production = &g_planet_production_shared;
    self->planet_id = NULL;
    self->map_pos = NULL;
    self->pos = NULL;
    self->slipways = NULL;
    self->project = NULL;
    self->description = NULL;
    self->needs = NULL;
    self->planet_type = NULL;
    self->fold_anchor = 0;
    self->layout_row = NULL;
    self->layout_row_slot = NULL;
    self->targeted_by = NULL;
    self->production_bonus = 0;
    self->has_production_bonus = 0;
    self->is_dead = 0;
    self->net_income = 0;
    self->has_net_income = 0;
    self->incoming_routes = NULL;
    self->outgoing_routes = NULL;
    self->development_tier = self->archetype->development_tier;
    self->has_development_tier = self->archetype->has_development_tier;
    self->header.is_hovered = &self->is_hovered;
    self->is_hovered = 0;
    self->production_own = (production_t){0};
    self->targeting = NULL;
    self->emitter = NULL;
}

static planet_t *planet_create(const planet_keys_t *keys) {
    planet_t *self = (planet_t *)allocator_alloc(planet_zone_construction(), sizeof *self);

    planet_init_instance(self, &g_planet_class, &planet_archetype);
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
    pico8_class_coverage_record("planet");
#endif
    planet_construct(self);
    entity_construct(&self->header);
    return self;
}

void planet_init(void) {
    planet_shutdown();
    draw_functions_init();
    g_planet_up = 1;
    g_planet_production_shared = (production_t){0};
}

void planet_shutdown(void) {
    g_planet_up = 0;
    g_type_picker = NULL;
    g_planet_vanish_roll = NULL;
}

void planet_set_type_picker(weighted_picker_t *oc) {
    g_type_picker = oc;
}

void planet_set_vanish_roll(random_walk_t *ir) {
    g_planet_vanish_roll = ir;
}

int planet_ready(void) {
    return g_planet_up;
}

const data_row_t *planet_mood_rows(void) {
    return planet_mood_rows_rows;
}

const int_seq_t *planet_made_resources(void) {
    return &planet_made_resources_seq;
}

const int_seq_t *planet_ring_colors(void) {
    return &planet_ring_colors_seq;
}

const data_row_t *planet_spacer(void) {
    return &planet_spacer_row;
}

production_t *planet_shared_production(void) {
    return &g_planet_production_shared;
}

const entity_class_t *planet_class(void) {
    return &g_planet_class;
}

planet_t *planet_new(const planet_keys_t *keys) {
    return planet_create(keys);
}

const allocator_t *planet_zone_construction(void) {
    return g_planet_grab_arena != NULL ? g_planet_grab_arena : game_zone();
}

void planet_grab_arena_open(void) {
    assert(g_planet_grab_arena == NULL && "une prise d'icone est deja ouverte");
    if (g_planet_grab_arenas_free_n > 0) {
        g_planet_grab_arena = g_planet_grab_arenas_free[--g_planet_grab_arenas_free_n];
        arena_flush(g_planet_grab_arena);
    } else {
        g_planet_grab_arena = arena_take(game_zone(), g_planet_name_grab_arena, PLANET_GRAB_ARENA_BYTES);
    }
}

void planet_grab_arena_close(void) {
    g_planet_grab_arena = NULL;
}

void planet_grab_arena_released(const planet_t *self) {
    planet_grab_arena_zone_released(self->slipways != NULL ? self->slipways->zone : NULL, self->header.state);
}

void planet_grab_arena_zone_released(const allocator_t *zone, const char *me) {
    if (me == NULL || strcmp(me, "placing") != 0) {
        return;
    }
    planet_grab_arena_arena_released(zone);
}

void planet_grab_arena_arena_released(const allocator_t *zone) {
    if (zone == NULL || zone->bump == NULL || zone->bump->name != g_planet_name_grab_arena) {
        return;
    }
    assert(g_planet_grab_arenas_free_n < GRAB_ARENAS_FREE_MAX &&
           "prises libres pleines : plus de prises refusees en vol que prevu");
    g_planet_grab_arenas_free[g_planet_grab_arenas_free_n++] = zone;
}

/* TROU-26 */
void planet_forget_targeted_by(const entity_t *e) {
    const entity_list_t *np = entities_by_tag("node");

    for (int i = 0; np != NULL && i < np->count; i++) {
        planet_t *p = (planet_t *)np->items[i];

        assert(planet_is_family(&p->header) && "cg.np ne tient que des planetes, qui ont un fo");
        if (p->targeted_by == e) {
            p->targeted_by = NULL;
        }
    }
}

void planet_forget_grab_arenas(void) {
    g_planet_grab_arenas_free_n = 0;
}

void planet_construct(planet_t *self) {
    planet_construct_in(self, planet_zone_construction());
}

void planet_construct_in(planet_t *self, const allocator_t *zone) {
    entity_list_t *oe;

    if (self == NULL) {
        return;
    }

    oe = (entity_list_t *)allocator_alloc(zone, sizeof *oe);
    entity_list_init(oe, zone);
    self->slipways = oe;

    if (self->header.class->lineage & LINEAGE(LINEAGE_STRUCTURE)) {
        pico8_hp_t *target = pico8_gb_at(p8_fixed_flr_int(self->archetype->project_index));

        if (target != NULL) {
            planet_install_project(self, target);
        }
    }
}

/* `mw(i)`: a copy of the want, never shared with `hp.i`. */
static int planet_install_project_copy_step(void *e, void *ctx, void **out) {
    if (e != NULL) {
        pico8_jy_t *copy = (pico8_jy_t *)allocator_alloc((const allocator_t *)ctx, sizeof *copy);
        const pico8_jy_t *i = e;

        *copy = *i;
        *out = copy;
        return 1;
    }
    return 0;
}

void planet_install_project(planet_t *self, pico8_hp_t *research_project_instance) {
    const allocator_t *zone = planet_zone_construction();
    pointer_list_t jy;
    pointer_list_t *needs_own;

    pico8_hp_assert(research_project_instance);
    if (self == NULL) {
        return;
    }

    self->project = research_project_instance;
    self->description = NULL;

    pointer_list_init(&jy, zone);
    if (research_project_instance != NULL && research_project_instance->i != NULL) {
        jy = collect_map_pointer_list(zone, research_project_instance->i, planet_install_project_copy_step, (void *)zone);
    }
    needs_own = (pointer_list_t *)allocator_alloc(zone, sizeof *needs_own);
    *needs_own = jy;
    self->needs = needs_own;

    self->production_own.has_resource = research_project_instance != NULL && research_project_instance->has_o;
    self->production_own.resource = self->production_own.has_resource ? research_project_instance->o : 0;
    self->production_own.amount = p8_fixed_from_int(0);
    self->production = &self->production_own;
}

int planet_is_colonized(const planet_t *self) {
    if (self == NULL) {
        return 0;
    }
    if (!self->archetype->is_planet) {
        return 0;
    }
    return self->header.state != NULL && strcmp(self->header.state, "settled") == 0;
}

void planet_restart_shake(planet_t *self) {
    if (self == NULL) {
        return;
    }
    self->shake = P8_FIXED_LIT(0x00014000, 1.25);
}

void planet_update_vanishing(planet_t *self, p8_fixed_t t) {
    if (self == NULL) {
        return;
    }
    self->is_dead = t > p8_fixed_from_int(60);
}

void planet_roll_type(planet_t *self) {
    const planet_types_entry_t *nk = NULL;
    const char *set_state_arg;

    if (self == NULL) {
        return;
    }

    if (g_planet_vanish_roll != NULL) {
        p8_fixed_t roll = random_walk_next(g_planet_vanish_roll);
        int use_vanishing = 0;

        /* `jo.ir()<0.45 and df.ll or jo.oc()`: a falsy `df.ll` falls through
         * to `jo.oc()` even on a winning roll -- both conditions, never the
         * roll alone. */
        if (roll < P8_FIXED_LIT(0x00007333, 0.45)) {
            use_vanishing = planet_types_lookup("vanishing") != NULL;
        }
        if (use_vanishing) {
            nk = planet_types_lookup("vanishing");
        } else if (g_type_picker != NULL) {
            nk = weighted_picker_next(g_type_picker);
        }
    }
    self->planet_type = nk;
    self->description = nk != NULL ? nk->description : NULL;

    if (nk != NULL && nk->state != NULL) {
        set_state_arg = nk->state;
    } else {
        set_state_arg = "known";
    }
    entity_set_state((entity_t *)self, set_state_arg);
    planet_restart_shake(self);
}

void planet_new_year(const planet_t *self) {
    if (self == NULL) {
        return;
    }
    if (!self->has_net_income || self->net_income == p8_fixed_from_int(0)) {
        return;
    }
    if (!entity_geom_is_on_screen((const entity_t *)self)) {
        return;
    }
    {
        pico8_iv_keys_t keys = {0};
        int color = 0;
        char *text;

        keys.ra = self->pos;
        text = collect_format_signed(frame_zone(), self->net_income, 10, "$", &color);
        /* `ne()` is the last expression of the Lua constructor, so both of its
         * return values land as positional entries 1 and 2, not named fields. */
        keys.at1 = text;
        keys.at2 = p8_fixed_from_int(color);
        pico8_iv_new(&keys);
    }
}

void planet_bob(entity_t *self, p8_fixed_t t) {
    vec_t *pos_vec;

    assert((self == NULL || planet_is_family(self)) && "na: self is a planet (qh:ly, nl:ly) -- stop");
    pos_vec = self != NULL ? ((planet_t *)self)->pos : NULL;
    if (pos_vec != NULL) {
        p8_fixed_t delta = p8_fixed_mul(p8_fixed_sin(p8_fixed_mul(t, P8_FIXED_LIT(0x00000147, 0.005))),
                                               P8_FIXED_LIT(0x00000f5c, 0.06));
        pos_vec->y = p8_fixed_add(pos_vec->y, delta);
    }
}
