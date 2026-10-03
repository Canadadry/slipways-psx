#ifndef GAME_PROBE_H
#define GAME_PROBE_H

#include "game/game_data.h"
#include "game/particle.h"
#include "game/targeting.h"
#include "engine/entity_class.h"
#include "engine/vec.h"
#include "engine/allocator.h"

/* targeting_track, targeting_tooltip and targeting_confirm return at once
 * on a NULL `self` (targeting_confirm returns 0); probe_release_method and
 * probe_init_instance do not check it, and probe_spawn_particle still takes
 * a particle, its draws at zero. A non-NULL `self` must carry the component
 * the function reads (asserted): its targeting for the three targeting_*
 * functions, its emitter for probe_spawn_particle. NULL graphics or memory
 * skips the draw calls while the rest of the body still runs. */

/* The emitter's trail_colors points at probe_trail_colors_seq
 * (help_screen_trail_colors_seq for the help screen); pos and map_pos point
 * at the targeting's track_pos and track_map_pos once targeting_track ran.
 * emitter and targeting point at emitter_own and targeting_own: never copy the
 * struct. is_hovered is the help screen's: a probe never links it and leaves
 * it indeterminate. grab_arena is the grab arena a probe lives in, NULL for
 * the help screen. */
typedef struct {
    entity_t header;
    const probe_archetype_t *archetype;
    emitter_t *emitter;
    targeting_t *targeting;
    const vec_t *pos;
    const vec_t *map_pos;
    int is_dead;
    int is_hovered;
    const allocator_t *grab_arena;
    emitter_t emitter_own;
    targeting_t targeting_own;
} probe_t;

/* GAP-8 */
/* Puts the placeholder main economy back, its credits and research points
 * absent. Shuts the module down first, so it is idempotent. */
void probe_init(void);

/* Empties the main economy slot without reading it, frees no probe.
 * Idempotent, and safe before the first probe_init(). */
void probe_shutdown(void);

/* 1 from probe_init() to probe_shutdown(), 0 otherwise. */
int probe_ready(void);

/* Initializes an allocated probe_t under the given class: the entity part,
 * the particle list, target, map_pos, is_dead and grab_arena cleared, the
 * archetype kept, pos, the emitter and the targeting copied from it. Reads no key, runs neither
 * the construct method nor entity_construct. */
void probe_init_instance(probe_t *self, const entity_class_t *cls, const probe_archetype_t *archetype);

/* The keys of probe_new, read, not kept. state_time is the header's, whose
 * default is 0: no flag. angle_slices, base_radius, speed_factor have
 * non-zero defaults, so a key at 0 needs its has_ flag. */
typedef struct {
    p8_fixed_t state_time;
    int has_angle_slices;
    p8_fixed_t angle_slices;
    int has_base_radius;
    p8_fixed_t base_radius;
    int has_speed_factor;
    p8_fixed_t speed_factor;
} probe_keys_t;

/* `keys` NULL means none. Built in a grab arena (planet_grab_arena_open),
 * so no other grab may be open. The new probe registers itself among the
 * live entities, draw layers and tags through entity_construct (and gets
 * its particle list from emitter_init, which its release method gives
 * back). Asserts when called before probe_init(). */
probe_t *probe_new(const probe_keys_t *keys);

/* A particle (has_acceleration 0) from particle_take, which emitter_spawn
 * appends to the particle list. */
particle_t *probe_spawn_particle(entity_t *self);

/* Release method of the probe and the help screen: gives back their
 * particle list, and the grab arena of a probe not placed (state other than
 * settled); a placed one keeps it until the generation is given back. */
void probe_release_method(entity_t *self);

#endif
