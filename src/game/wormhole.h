#ifndef GAME_WORMHOLE_H
#define GAME_WORMHOLE_H

#include "engine/entity_list.h"
#include "p8/fixed.h"
#include "game/game_data.h"
#include "game/targeting.h"
#include "engine/entity_class.h"
#include "engine/vec.h"

/* Throughout this module a NULL struct argument is a safe no-op, and an
 * absent pos or map_pos reads as the zero vector rather than crashing. */

/* The wormhole's too-far target: no parent, never an entity, and no
 * writer. */
typedef struct {
    entity_t header;
    const char *status_text;
} wormhole_too_far_target_t;

/* too_far_target points at the one wormhole_too_far_target_t, shared by
 * every wormhole and never modified in place. The cost and the target live
 * in the targeting; targeting points at targeting_own: never copy the struct.
 * The struct and its anchors list live in one grab arena, which its release
 * method gives back whatever its state. */
typedef struct {
    entity_t header;
    const wormhole_archetype_t *archetype;
    p8_fixed_t radius;
    wormhole_too_far_target_t *too_far_target;
    entity_list_t *anchors;
    const vec_t *pos;
    const vec_t *map_pos;
    entity_t *anchor; /* borrowed, never freed */
    int is_dead;
    targeting_t *targeting;
    targeting_t targeting_own;
} wormhole_t;

void wormhole_init(void);

/* Gives back no wormhole nor the too-far target. Idempotent, and safe
 * before the first wormhole_init(). */
void wormhole_shutdown(void);

/* 1 from wormhole_init() to wormhole_shutdown(), 0 otherwise. */
int wormhole_ready(void);
const entity_class_t *wormhole_class(void);

/* Built in a grab arena (planet_grab_arena_open), so no other grab may be
 * open; the struct registers itself among the live entities, draw layers
 * and tags through entity_construct. */
wormhole_t *wormhole_new(void);

/* The one too-far target, shared by every wormhole: static, never NULL,
 * never freed, and without a writer, so a write reaching it stops the run. */
wormhole_too_far_target_t *wormhole_too_far_target_wrapper(void);
const entity_class_t *wormhole_too_far_target_class(void);

/* The anchors list taken from the construction zone, reserved to the
 * exact count of live entities with a non-zero fold_anchor: it never
 * grows. */
void wormhole_construct(wormhole_t *self);

/* Calls targeting_track first, which is what sets pos/map_pos and a
 * first target; the "too far" branch here then overwrites the target. */
void wormhole_drag(wormhole_t *self, vec_t *mp, vec_t *rp);

void wormhole_update_placing(wormhole_t *self, p8_fixed_t t);

void wormhole_drop(wormhole_t *self);

/* GAP-3 */
void wormhole_update_unfolding(wormhole_t *self, p8_fixed_t t);

/* Reads the radius only; `self` is non-const for signature uniformity. */
p8_fixed_t wormhole_reveal_radius(wormhole_t *self, p8_fixed_t t);

void wormhole_draw_method(entity_t *self, const vec_t *screen_pos);

#endif
