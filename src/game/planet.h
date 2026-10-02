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

/* Throughout this module a NULL instance argument is a safe no-op (Lua nil
 * for the value-returning functions), and an absent or wrong-kind field
 * reads as absent rather than crashing. */

/* A planet of any of the ten kinds, which differ only by class table and
 * archetype: its archetype (`kw` points at the one kw every bare planet
 * shares, written in place by `bb` and `mx:kf`, until `lz` points it at the
 * planet's own `kw_own`), its construction keys `id`/`p`/`ra`, then every
 * field jo's functions write on it, then its kinds' fields. A kind's field is
 * indeterminate on a planet of another kind; `probe`, `emitter` are NULL
 * there, else they point at `probe_own`, `emitter_own`: never copy the
 * struct. The struct lives in its game generation (a burst's in its
 * particle place) and goes back with it. */
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
    const allocator_t *layout_row_slot; /* where `lq` lives; NULL when `lq` is */
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
    p8_fixed_t sprite;           /* qh's lineage */
    p8_fixed_t palette_block;          /* nl */
    p8_fixed_t acceleration_factor, speed;      /* ga */
    const int_seq_t *synthesizable_resources; /* mx */
    targeting_t *targeting;      /* qh's lineage */
    emitter_t *emitter;  /* hj, pm, mx, ga */
    targeting_t probe_own;
    emitter_t emitter_own;
} planet_t;

/* TROU-8 */
/* Calls `pico8_pi_init()` first. */
void planet_init(void);

/* Sets the `oc`/`ir` generators back to nil without reading them: their
 * zone may already be reset. Frees no `jo` instance. Idempotent, and safe
 * before the first `pico8_jo_init()`. */
void planet_shutdown(void);

/* 1 from `pico8_jo_init()` to `pico8_jo_shutdown()`, 0 otherwise. */
int planet_ready(void);

const entity_class_t *planet_class(void);

/* `jo.dz` (PICO8_JO_DZ_COUNT rows), `jo.cq`, `jo.ka`, `jo.em`: jo's
 * generated data, never NULL, init or not. */
const data_row_t *planet_mood_rows(void);
const int_seq_t *planet_made_resources(void);
const int_seq_t *planet_ring_colors(void);
const data_row_t *planet_spacer(void);

/* The `kw` every bare planet points at; each `pico8_jo_init()` resets it to
 * `{mu=0}`. */
production_t *planet_shared_production(void);

/* `keys` (NULL means none) is read, not kept. The new planet registers
 * itself into `hv`/`cv`/`cg` through `lt`, and lives in the construction
 * zone until that zone is given back. Requires `pico8_jo_init()`. */
planet_t *planet_new(const planet_keys_t *keys);

/* Sets up an allocated `self` as a bare planet whose descriptor is `cls`:
 * `def` kept, the members it holds copied from it, the others nil. Neither
 * runs `jo:pq` nor `lt`: a daughter's constructor does, after its own
 * defaults. Requires `pico8_jo_init()`. */
void planet_init_instance(planet_t *self, const entity_class_t *instance_class, const planet_archetype_t *archetype);

/* Bytes of the arena a held icon, a drawn slipway or a wormhole builds its
 * object in. */
#define PLANET_GRAB_ARENA_BYTES 2048u

/* Where the planet constructors, `jo:pq` and `jo:lz` allocate: the grab
 * arena between `pico8_jo_prise_ouvrir` and `_fermer`, else the current
 * game generation. */
const allocator_t *planet_zone_construction(void);

/* Opens a grab arena, taken from the free grabs (emptied) or else from the
 * current generation; a placed structure keeps it until the generation is
 * given back. */
void planet_grab_arena_open(void);
void planet_grab_arena_close(void);
/* Called last by the `rendre` of an object built in `zone`, whose state is
 * `me`: a grab arena (NULL or another zone: nothing) whose object is still
 * "kz" goes on the free grabs, for the next grab to empty and reuse. */
void planet_grab_arena_zone_released(const allocator_t *zone, const char *me);
/* The same whatever the state: for an object whose memory nothing borrows
 * after its death. */
void planet_grab_arena_arena_released(const allocator_t *zone);
/* `pico8_jo_prise_zone_rendue` with the zone of the planet's `oe`. */
void planet_grab_arena_released(const planet_t *self);
/* Sets to nil the `fo` of every `cg.np` entity that points at `e`, which is
 * being given back: `jo:kj` leaves `fo` set on a planet off screen or
 * fading, and a recycled grab arena would put a new structure behind it. */
void planet_forget_targeted_by(const entity_t *e);
/* Drops the free grabs, without reading them: their arenas belong to a
 * generation about to be given back or reset. */
void planet_forget_grab_arenas(void);

/* `oe` taken from, and grown in, the construction zone. */
void planet_construct(planet_t *self);
/* `oe` taken from, and grown in, `zone`, which must outlive it. */
void planet_construct_in(planet_t *self, const allocator_t *zone);

/* The dispatchers jo's descriptor holds, for a daughter's `static const`
 * descriptor to name. */
int planet_hit_test_method(entity_t *self, const vec_t *qa, const vec_t *ra);
void planet_refresh_method(entity_t *self);
/* Gives back the place of `lq` and sets `lq` to nil; the planet stays. */
void planet_release_row(entity_t *self);
/* `pico8_jo_rendre_rangee`, then `pico8_jo_prise_rendue`. */
void planet_release_method(entity_t *self);
down_t planet_accept_drop_method(entity_t *self, const entity_t *s);
void planet_new_year_method(entity_t *self, p8_fixed_t yr);
void planet_draw_method(entity_t *self, const vec_t *ra);
down_t planet_left_press_method(entity_t *self, vec_t *qa, vec_t *ra);
void planet_attach_slipway_method(entity_t *self, entity_t *sw);

/* `e` is one of the ten classes of jo's tree: the only ones whose state
 * `ll` is jo's. */
int planet_is_family(const entity_t *e);
void planet_offer_trade_method(entity_t *self, struct trade_offer *lw);
void planet_update_vanishing_method(entity_t *self, p8_fixed_t t);

void planet_install_project(planet_t *self, research_project_t *research_project_instance);

/* TROU-3 */
/* 1 when `hf` is true and `me` is "ly", 0 otherwise, a NULL `self` included. */
int planet_is_colonized(const planet_t *self);

void planet_restart_shake(planet_t *self);

/* No textual `:ll(` call site exists: `dg()` reaches this by state name
 * (`qc[qc.me]`), through the `LL` entry of a planet's descriptor. */
void planet_update_vanishing(planet_t *self, p8_fixed_t t);

void planet_roll_type(planet_t *self);

void planet_new_year(const planet_t *self);

/* Returns a freshly built `jc` instance in `held`, Lua true as `stop`
 * alone, or nil; `ui:bh` holds only the instance. The `jc` lives in its
 * grab arena, which it gives back if refused. */
down_t planet_left_press(planet_t *self);

/* The state passed must live until the next call; the previous one is not
 * freed. NULL is the valid way back to "none". Both start NULL -- only
 * `bw()` sets them. */
void planet_set_type_picker(weighted_picker_t *oc);
void planet_set_vanish_roll(random_walk_t *ir);

/* Mutates the pico8_vec_t `self.ra` points at, in place: every other holder
 * of that vector -- the `iv` toast `jo:hk` builds from `self.ra` -- sees the
 * bob. Rebinding the field to a fresh vector, as `qb:kj` does, would not. */
void planet_bob(entity_t *self, p8_fixed_t t);

/* NULL when no kind has `fz`. */
const entity_kind_t *planet_kind_named(const char *fz);

#endif
