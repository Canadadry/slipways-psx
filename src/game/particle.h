#ifndef GAME_PARTICLE_H
#define GAME_PARTICLE_H

#include "engine/entity_class.h"
#include "game/game_data.h"
#include "p8/gfx.h"
#include "p8/mem.h"
#include "engine/pointer_list.h"
#include "engine/vec.h"
#include "game/particle_zone.h"

/* A NULL `self` is a no-op for emitter_init, emitter_init_in, emitter_spawn
 * and emitter_update; emitter_release_particles needs a non-NULL emitter,
 * and emitter_update draws each surviving particle through p8_pset, which
 * needs non-NULL graphics and memory. emitter_init_in and emitter_update
 * stop on an entity whose emitter link is NULL (asserted); emitter_spawn
 * adds nothing there. */

/* An element of an emitter's particle list: every spawn_particle method
 * takes one from particle_take, emitter_update updates it in place and gives
 * it back when it takes it out of the list, or emitter_release_particles
 * with its list. has_acceleration is 0 for a probe's particles, whose
 * acceleration is zero and never read. */
typedef struct particle {
    vec_t pos;
    vec_t velocity;
    int has_acceleration;
    vec_t acceleration;
    p8_fixed_t life;
} particle_t;

/* What emitter_update, emitter_spawn, emitter_init and probe_spawn_particle
 * read of their host, the particle emitter's own fields, stored in the host
 * (emitter_own) and never copied; the rest is read on the archetype, the
 * host's archetype's emitter. particles is NULL until emitter_init, then in
 * its own arena of the kind particles_slot_kind, owned by the host, whose
 * release method gives it back by emitter_release_particles. angle_slices
 * and base_radius are read by probe_spawn_particle only: a burst leaves them
 * unwritten. */
typedef struct emitter {
    const emitter_archetype_t *archetype;
    p8_fixed_t angle_slices;
    p8_fixed_t base_radius;
    p8_fixed_t speed_factor;
    const int_seq_t *trail_colors; /* NULL: absent, every particle drawn in colour 0 */
    p8_fixed_t matrix_xx;
    p8_fixed_t matrix_xy;
    p8_fixed_t matrix_yx;
    p8_fixed_t matrix_yy;
    pointer_list_t *particles;
    particle_slot_kind_t particles_slot_kind; /* meaningless while particles is NULL */
} emitter_t;

/* Gives back the list `self` already has (emitter_release_particles), then
 * takes a fresh one, reserved to its capacity: the help screen takes a help
 * screen list through emitter_init_in, every other host a particle list
 * through emitter_init. */
void emitter_init(entity_t *self);
void emitter_init_in(entity_t *self, particle_slot_kind_t slot_kind);

/* An uninitialised slot of the particle kind; given back by emitter_update
 * or emitter_release_particles once in a particle list. */
particle_t *particle_take(void);

/* A fresh slot of the burst kind, as an arena: the burst puts its struct,
 * its slipways list and its pos box there, and its release method gives it
 * back by arena_release. */
const allocator_t *particle_burst_slot_take(void);

/* Gives back each particle still in emitter->particles to the particle
 * handle, then the list's arena, and leaves the list NULL; nothing when it
 * already is. `emitter` must not be NULL. */
void emitter_release_particles(emitter_t *emitter);

/* Appends what the class's spawn_particle returns to the particle list
 * while rnd() is below the count, subtracting 1 from the count each turn: a
 * fractional count genuinely has a chance of spawning one more particle
 * than its floor. A class without spawn_particle, or a host without a list,
 * adds nothing but still draws from rnd() each turn. */
void emitter_spawn(entity_t *self, p8_fixed_t n);

/* Takes graphics and memory explicitly, for the classes whose own draw
 * method calls it as a plain function; emitter_update_method below is the
 * same work reached through a draw entry, reading both from the dispatch
 * context instead. */
void emitter_update(const p8_graphics_t *graphics, p8_memory_t *memory, entity_t *self);

/* The position argument is never read: emitter_update reads the host's
 * own position. */
void emitter_update_method(entity_t *self, const vec_t *ra);

#endif
