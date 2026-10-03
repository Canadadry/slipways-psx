#ifndef GAME_TARGETING_H
#define GAME_TARGETING_H

#include "p8/fixed.h"
#include "game/game_data.h"
#include "p8/gfx.h"
#include "p8/mem.h"
#include "engine/entity_class.h"
#include "engine/vec.h"

/* What targeting_track, targeting_tooltip and targeting_confirm read and
 * write of their host, the targeting's own fields, stored in the host
 * (targeting_own) and never copied; the rest is read on the archetype, the
 * host's archetype's targeting (targeting_archetype_none for the structure
 * icon's tooltip). The target is borrowed. */
typedef struct targeting {
    const targeting_archetype_t *archetype;
    entity_t *target;
    p8_fixed_t cost;
    int has_cost; /* 0: no cost, and cost is then never read */
    vec_t track_pos;
    vec_t track_map_pos;
} targeting_t;

/* The host's pos and map_pos point at its targeting's track_pos (NULL for a
 * NULL mouse position) and track_map_pos, rewritten in place: a copy of the
 * mouse position, which stays the caller's. The target ends in one of three
 * states -- a real target, the static throwaway struct (over the void, off
 * the panel) or NULL (over the panel) -- of which only NULL lets
 * targeting_confirm spend; collapsing the throwaway to NULL would change it. */
void targeting_track(entity_t *self, vec_t *mp, vec_t *rp);

void targeting_track_method(entity_t *self, vec_t *mouse_world, vec_t *mouse_screen);

/* Points the target's targeted_by back at `self`, which it borrows: the
 * target is a planet of the planet family, a slipway, the slipway's too-far
 * struct or targeting_track's throwaway target (asserted, as is the class of
 * `self` each accepts). */
void probe_link_target(entity_t *et, entity_t *self);

/* The target targeting_track sets over the void: one static struct, never
 * allocated nor freed; only targeted_by is written, by probe_link_target(),
 * and the next targeting_track overwrites it. */
typedef struct {
    entity_t header;
    entity_t *targeted_by;
} targeting_track_target_t;

const entity_class_t *targeting_track_target_class(void);

/* The tooltip of a targeting host: `self` must carry a targeting (asserted),
 * and is read, never retained. */
void targeting_tooltip(const p8_graphics_t *gfx, p8_memory_t *mem, entity_t *self);

/* The same, for a caller that is not a host (the structure icon): a state
 * other than ENTITY_STATE_PLACING draws nothing, the targeting is read
 * during the call only. */
void targeting_tooltip_show(const p8_graphics_t *gfx, p8_memory_t *mem, state_t state, const targeting_t *pb);

void targeting_tooltip_method(entity_t *self, const vec_t *screen_pos);

/* Returns 1 on the success branch, 0 on the other; slipway_drop and
 * wormhole_drop consume it. */
int targeting_confirm(entity_t *self);

void targeting_confirm_method(entity_t *self, entity_t *dropped);

#endif
