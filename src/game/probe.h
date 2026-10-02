#ifndef GAME_PROBE_H
#define GAME_PROBE_H

#include "game/game_data.h"
#include "game/particle.h"
#include "game/targeting.h"
#include "engine/entity_class.h"
#include "engine/vec.h"
#include "engine/allocator.h"

/* Throughout this module a NULL instance argument is a safe no-op (Lua nil
 * for the value-returning functions), a non-NULL `self` must be a struct
 * (asserted) whose class carries the component the function reads -- a
 * probe for `bv`, `dl`, `d_`, an emitter for `g_:mm` -- and a NULL
 * `gfx`/`mem` skips the draw calls while the rest of the body still runs. */

/* The emitter's `k_` points at
 * `pico8_g__k__seq` (`pico8_fv_k__seq` for `fv`); `ra` and `p` point at
 * the probe's `bv_ra` and `bv_p` once `bv` ran. `emitter` and `probe` point at `emitter_own` and
 * `probe_own`: never copy the struct. `m_` is `fv`'s: a `g_` has no `m_`
 * link and leaves it indeterminate. `prise` is the grab arena a `g_` lives
 * in, NULL for `fv`. */
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
    targeting_t probe_own;
} probe_t;

/* TROU-8 */
/* Puts the provisional `fm` back in the slot, its `jk`/`nz` nil. Shuts the
 * module down first, so it is idempotent. */
void probe_init(void);

/* Empties the `fm` slot without reading it, frees no probe. Idempotent,
 * and safe before the first `pico8_probe_init()`. */
void probe_shutdown(void);

/* 1 from pico8_probe_init() to pico8_probe_shutdown(), 0 otherwise. */
int probe_ready(void);

/* Initializes an allocated pico8_g__t under `cls`: ke's part, g_'s 4
 * written members nil, `prise` NULL, `def` kept, `ra`, the emitter and the probe copied
 * from it. Reads no key, runs neither `pq` nor `lt`, records no coverage. */
void probe_init_instance(probe_t *self, const entity_class_t *cls, const probe_archetype_t *archetype);

/* The keys of `pico8_g__new`, read, not kept. `t` is `header.t`, whose default
 * is 0: no flag. `pn`, `du`, `py` have non-zero defaults, so a key at 0
 * needs its `has_` flag. */
typedef struct {
    p8_fixed_t state_time;
    int has_angle_slices;
    p8_fixed_t angle_slices;
    int has_base_radius;
    p8_fixed_t base_radius;
    int has_speed_factor;
    p8_fixed_t speed_factor;
} probe_keys_t;

/* `keys` NULL means none. Built in a grab arena (`pico8_jo_prise_ouvrir`),
 * so no other grab may be open. The new probe registers itself into
 * `hv`/`cv`/`cg` through `lt` (and gets its particle list from `f_`, which
 * its `rendre` gives back). Asserts when called before `pico8_probe_init()`. */
probe_t *probe_new(const probe_keys_t *keys);

/* A particle (`has_a` 0) from pico8_particule_prendre, which `ez` appends
 * to `qy`. */
particle_t *probe_spawn_particle(entity_t *self);

/* `rendre` of `g_` and `fv`: gives back their `qy`, and the grab arena of
 * a `g_` not placed (`me` other than "ly"); a placed one keeps it until the
 * generation is given back. */
void probe_release_method(entity_t *self);

#endif
