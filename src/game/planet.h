#ifndef GAME_PLANET_H
#define GAME_PLANET_H

#include "engine/entity_list.h"
#include "p8/fixed.h"
#include "game/game_data.h"
#include "game/gen.h"
#include "game/layout.h"
#include "game/particle.h"
#include "game/targeting.h"
#include "engine/pointer_list.h"
#include "game/research.h"
#include "engine/entity_class.h"

/* Throughout this module a NULL instance argument is a safe no-op (NULL or
 * 0 for the value-returning functions), and an absent or wrong-kind field
 * reads as absent rather than crashing. */

/* A planet of any of the ten kinds of the planet lineage, which differ only
 * by class and archetype: its archetype (`production` points at the one
 * production every bare planet shares, written in place by
 * update_tiers_visit and synthesizer_attach_slipway, until
 * planet_install_project points it at the planet's own `production_own`),
 * its construction keys `planet_id`/`map_pos`/`pos`, then every field the
 * planet functions write on it, then its kinds' fields. A kind's field is
 * indeterminate on a planet of another kind; `targeting`, `emitter` are
 * NULL there, else they point at `targeting_own`, `emitter_own`: never copy the
 * struct. The struct lives in the game zone (a burst's in its particle
 * slot) and goes back with it. */
typedef struct {
    entity_t header;
    const planet_archetype_t *archetype;
    p8_fixed_t shake;
    production_t *production;
    const char *planet_id;
    vec_t *map_pos;
    vec_t *pos;
    entity_list_t *slipways;
    research_project_t *project;
    const char *description;
    pointer_list_t *needs;
    const planet_types_entry_t *planet_type;
    p8_fixed_t fold_anchor;
    layout_row_t *layout_row;
    const allocator_t *layout_row_slot; /* where the layout row lives; NULL when it is */
    entity_t *targeted_by;
    p8_fixed_t production_bonus;
    int has_production_bonus;
    int is_dead;
    p8_fixed_t net_income;
    int has_net_income;
    entity_list_t *incoming_routes;
    entity_list_t *outgoing_routes;
    p8_fixed_t development_tier;
    int has_development_tier;
    int is_hovered;
    production_t production_own;
    p8_fixed_t sprite;           /* structure lineage */
    p8_fixed_t palette_block;          /* trading hub */
    p8_fixed_t acceleration_factor, speed;      /* burst */
    const int_seq_t *synthesizable_resources; /* synthesizer */
    targeting_t *targeting;      /* structure lineage */
    emitter_t *emitter;  /* ascension, protostar, synthesizer, burst */
    targeting_t targeting_own;
    emitter_t emitter_own;
} planet_t;

/* TROU-8 */
/* Calls draw_functions_init(). */
void planet_init(void);

/* Sets the type picker and the vanish roll back to NULL without reading
 * them: their zone may already be reset. Frees no planet. Idempotent, and
 * safe before the first planet_init(). */
void planet_shutdown(void);

/* 1 from planet_init() to planet_shutdown(), 0 otherwise. */
int planet_ready(void);

const entity_class_t *planet_class(void);

/* The planet's generated data (PLANET_MOOD_ROWS_COUNT mood rows, made
 * resources, ring colors, spacer row): never NULL, init or not. */
const data_row_t *planet_mood_rows(void);
const int_seq_t *planet_made_resources(void);
const int_seq_t *planet_ring_colors(void);
const data_row_t *planet_spacer(void);

/* The production every bare planet points at; each planet_init() resets
 * it to zero. */
production_t *planet_shared_production(void);

/* `keys` (NULL means none) is read, not kept. The new planet registers
 * itself (entity_register), and lives in the construction zone until that
 * zone is given back. Requires planet_init(). */
planet_t *planet_new(const planet_keys_t *keys);

/* Sets up an allocated `self` as a bare planet of class `instance_class`:
 * the archetype kept, the members it holds copied from it, the others NULL
 * or 0. Runs neither planet_construct nor entity_construct: a subclass's
 * constructor does, after its own defaults. Requires planet_init(). */
void planet_init_instance(planet_t *self, const entity_class_t *instance_class, const planet_archetype_t *archetype);

/* Bytes of the arena a held icon, a drawn slipway or a wormhole builds its
 * object in. */
#define PLANET_GRAB_ARENA_BYTES 2048u

/* Where the planet constructors, planet_construct and
 * planet_install_project allocate: the grab arena between
 * planet_grab_arena_open and planet_grab_arena_close, else the game zone. */
const allocator_t *planet_zone_construction(void);

/* Opens a grab arena, taken from the free grabs (emptied) or else from the
 * game zone; a placed structure keeps it until the game zone is given back. */
void planet_grab_arena_open(void);
void planet_grab_arena_close(void);
/* Called last by the release method of an object built in `zone`, with the
 * object's state: a grab arena (NULL or another zone: nothing) whose object
 * is still "placing" goes on the free grabs, for the next grab to empty and
 * reuse. */
void planet_grab_arena_zone_released(const allocator_t *zone, const char *me);
/* The same whatever the state: for an object whose memory nothing borrows
 * after its death. */
void planet_grab_arena_arena_released(const allocator_t *zone);
/* planet_grab_arena_zone_released with the zone of the planet's slipways. */
void planet_grab_arena_released(const planet_t *self);
/* Sets to NULL the `targeted_by` of every node entity that points at the
 * entity being given back: planet_draw leaves `targeted_by` set on a planet
 * off screen or fading, and a recycled grab arena would put a new structure
 * behind it. */
void planet_forget_targeted_by(const entity_t *e);
/* Drops the free grabs, without reading them: their arenas belong to a
 * game zone about to be given back or reset. */
void planet_forget_grab_arenas(void);

/* The slipways list taken from, and grown in, the construction zone. */
void planet_construct(planet_t *self);
/* The slipways list taken from, and grown in, `zone`, which must outlive
 * it. */
void planet_construct_in(planet_t *self, const allocator_t *zone);

/* The methods the planet class holds, for a subclass's `static const`
 * class to name. */
int planet_hit_test_method(entity_t *self, const vec_t *qa, const vec_t *ra);
void planet_refresh_method(entity_t *self);
/* Gives back the layout row's slot and sets the row to NULL; the planet
 * stays. */
void planet_release_row(entity_t *self);
/* planet_release_row, then planet_grab_arena_released. */
void planet_release_method(entity_t *self);
down_t planet_accept_drop_method(entity_t *self, const entity_t *s);
void planet_new_year_method(entity_t *self, p8_fixed_t yr);
void planet_draw_method(entity_t *self, const vec_t *ra);
down_t planet_left_press_method(entity_t *self, vec_t *qa, vec_t *ra);
void planet_attach_slipway_method(entity_t *self, entity_t *sw);

/* 1 when the entity's class is one of the ten of the planet lineage. */
int planet_is_family(const entity_t *e);
void planet_offer_trade_method(entity_t *self, struct trade_offer *lw);
void planet_update_vanishing_method(entity_t *self, p8_fixed_t t);

void planet_install_project(planet_t *self, research_project_t *research_project_instance);

/* TROU-3 */
/* 1 when the archetype's is_planet is set and the state is "settled", 0
 * otherwise, a NULL `self` included. */
int planet_is_colonized(const planet_t *self);

void planet_restart_shake(planet_t *self);

/* No direct caller: update_entities reaches it by state, through the
 * ENTITY_STATE_VANISHING entry of the planet class. */
void planet_update_vanishing(planet_t *self, p8_fixed_t t);

void planet_roll_type(planet_t *self);

void planet_new_year(const planet_t *self);

/* Returns a freshly built slipway in `held` for a settled planet, `stop`
 * alone for a known one (after opening its projects panel), or neither. The
 * slipway lives in its grab arena, which it gives back if refused. */
down_t planet_left_press(planet_t *self);

/* The state passed must live until the next call; the previous one is not
 * freed. NULL is the valid way back to none. Both start NULL -- only
 * galaxy_populate sets them. */
void planet_set_type_picker(weighted_picker_t *picker);
void planet_set_vanish_roll(random_walk_t *ir);

/* Mutates the vec_t the planet's `pos` points at, in place: every other
 * holder of that vector -- the toast planet_new_year builds from `pos` --
 * sees the bob. Rebinding the field to a fresh vector, as camera_update
 * does, would not. */
void planet_bob(entity_t *self, p8_fixed_t t);

/* NULL when no kind has that class name. */
const entity_kind_t *planet_kind_named(const char *fz);

#endif
