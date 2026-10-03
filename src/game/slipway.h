#ifndef GAME_SLIPWAY_H
#define GAME_SLIPWAY_H

#include "p8/fixed.h"
#include "game/game_data.h"
#include "engine/entity.h"
#include "game/targeting.h"
#include "engine/pointer_list.h"
#include "engine/entity_class.h"

/* Throughout this module a NULL struct argument is a safe no-op (NULL or 0
 * for the value-returning functions), and an absent or wrong-kind member,
 * or an absent or wrong-kind field of another entity, reads as absent
 * rather than crashing. */

enum { SLIPWAY_SERVED_PLANETS_CAP = 90 };

/* Transit dots reserved in the grab arena: past them the list grows in it,
 * and a full arena is fatal (p8_fatal). */
enum { SLIPWAY_TRANSIT_DOTS_RESERVE = 64 };

/* The planet_id of each planet a slipway serves, each once; the strings
 * are borrowed from the planets. */
typedef struct {
    int count;
    const char *ids[SLIPWAY_SERVED_PLANETS_CAP];
} slipway_served_planets_t;

/* Adds the identifier once: it must not be NULL, and at most
 * SLIPWAY_SERVED_PLANETS_CAP are ever added (asserted). */
void slipway_served_planets_add(slipway_served_planets_t *hg, const char *id);

/* 1 for an identifier already added, 0 otherwise, NULL included. */
int slipway_served_planets_has(const slipway_served_planets_t *hg, const char *id);

/* The point slipway_drag hangs the end on over nothing: the slipway's own,
 * rewritten in place at each call; `pos` points at the slipway's
 * `point_pos`, a copy of the caller's mouse position (NULL for a NULL one),
 * `map_pos` at its `point_map_pos`. */
typedef struct {
    entity_t header;
    const vec_t *pos;
    const vec_t *map_pos;
} slipway_mouse_point_t;

/* `transit_dots` is this slipway's own, made by slipway_construct, which
 * also empties `served_planets_own` and points `served_planets` at it (NULL
 * before). The cost and the target (NULL when absent) live in `targeting`,
 * which points at `targeting_own`; `point.pos` and `point.map_pos` point at
 * `point_pos` and `point_map_pos`: never copy the struct. Built with its
 * transit dots in a grab arena (planet_grab_arena_open), which a refused
 * slipway's release method puts back on the free grabs. */
typedef struct {
    entity_t header;
    const slipway_archetype_t *archetype;
    p8_fixed_t layer;
    p8_fixed_t fade_in_step;
    entity_t *start; /* borrowed, never freed */
    pointer_list_t *transit_dots;
    slipway_served_planets_t *served_planets;
    slipway_served_planets_t served_planets_own;
    entity_t *end; /* borrowed, never freed */
    p8_fixed_t infraspace;
    int has_infraspace;                /* 0: no infraspace, `infraspace` is then never read */
    entity_t *targeted_by; /* borrowed, never freed */
    int is_dead;
    targeting_t *targeting;
    targeting_t targeting_own;
    slipway_mouse_point_t point;
    vec_t point_pos;
    vec_t point_map_pos;
} slipway_t;

/* The one static "-too far-" struct slipway_obstruction returns:
 * `targeted_by` is the last slipway that linked itself to it, borrowed. */
typedef struct {
    entity_t header;
    const char *status_text;
    slipway_t *targeted_by;
} slipway_too_far_t;

void slipway_init(void);

/* Never frees any slipway. Idempotent, and safe before the first
 * slipway_init(). */
void slipway_shutdown(void);

/* 1 from slipway_init() to slipway_shutdown(), 0 otherwise. */
int slipway_ready(void);

/* Map and screen position of an end of a slipway (a planet, or the mouse
 * point slipway_drag hangs the end on over nothing), read through its link:
 * NULL, an end without the link, or an absent vector reads as the zero
 * vector. */
vec_t slipway_end_map_pos(const entity_t *end);
vec_t slipway_end_pos(const entity_t *end);

/* The start of a new slipway: NULL for none, else a planet or a structure,
 * which the type does not enforce (slipway_create asserts it). */
typedef struct {
    entity_t *start;
} slipway_keys_t;

/* `keys` is read, not kept (NULL: none). The slipway registers itself
 * (entity_register); it lives in its grab arena, kept until the game zone is
 * given back once placed, recycled after its release method if refused. */
slipway_t *slipway_new(const slipway_keys_t *keys);

/* Allocates the transit dots and served planets every other function here
 * assumes are already present, the transit dots in the construction zone
 * with room for SLIPWAY_TRANSIT_DOTS_RESERVE. */
void slipway_construct(slipway_t *self);

/* Rebinds `end` every frame: the hovered entity when it has accept_drop,
 * otherwise `point`, rewritten in place. Also rebinds the cost, the
 * infraspace, the target and the layer, and points the `targeted_by` of
 * whatever slipway_obstruction found -- a planet, another slipway or the
 * static too-far struct -- back at this slipway. */
void slipway_drag(slipway_t *self, vec_t *mp);

/* A NULL end, or a targeting_confirm that refuses, only sets `is_dead`;
 * on success both endpoints' attach_slipway run. */
void slipway_drop(slipway_t *self, entity_t *end_planet);

/* Returns the existing slipway to reuse, the first planet or slipway in
 * the way, the one static "-too far-" struct, or NULL. */
entity_t *slipway_obstruction(slipway_t *self);

const entity_class_t *slipway_too_far_class(void);

const entity_class_t *slipway_mouse_point_class(void);

/* The other end of the slipway seen from `planet_instance` (the start when
 * the end is NULL); NULL for a NULL slipway. */
entity_t *slipway_other_end(const slipway_t *self, const entity_t *planet_instance);

void slipway_draw_overlay(slipway_t *self);

void slipway_draw_method(entity_t *self, const vec_t *screen_pos);

#endif
