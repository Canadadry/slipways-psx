#ifndef GAME_PARTICLE_H
#define GAME_PARTICLE_H

#include "engine/entity_class.h"
#include "game/game_data.h"
#include "p8/gfx.h"
#include "p8/mem.h"
#include "engine/pointer_list.h"
#include "engine/vec.h"
#include "game/particle_zone.h"

/* Throughout this module a NULL instance, gfx or mem argument is a safe
 * no-op and a non-NULL `self` must be a struct (asserted). `f_` and `ei`
 * stop on an entity whose `emitter` link is NULL; `ez` adds nothing there. */

/* An element of `qy`, `{p,v,a,l}` in the cart: every `mm` takes one from
 * pico8_particule_prendre, `ei` updates it in place and gives it back when
 * it takes it out of `qy`, or pico8_qy_rendre with its list. `has_a` is 0 for
 * `g_`'s particles, whose `a` is absent and never read. */
typedef struct particle {
    vec_t pos;
    vec_t velocity;
    int has_acceleration;
    vec_t acceleration;
    p8_fixed_t life;
} particle_t;

/* What `ei`, `ez`, `f_` and `g_:mm` read of their host, the particle
 * emitter's own fields, stored in the host (`emitter_own`) and never copied;
 * the rest is read on `def`, the host's archetype's emitter. `qy` is NULL
 * until `f_`, then in its own arena of the sort `qy_sorte`, owned by the
 * host, whose `rendre` gives it back by pico8_qy_rendre. `pn`, `du`
 * are read by `g_:mm` only: a `ga` leaves them unwritten. */
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
    particle_slot_kind_t particles_slot_kind; /* meaningless while `qy` is NULL */
} emitter_t;

/* Gives back the list `self` already has (pico8_qy_rendre), then takes a
 * fresh one, reserved to its capacity: `fv` takes `qy_fv`, every other
 * host `qy` by pico8_f_. */
void emitter_init(entity_t *self);
void emitter_init_in(entity_t *self, particle_slot_kind_t slot_kind);

/* An uninitialised place of the `particule` sort; given back by `ei` or
 * pico8_qy_rendre once in a `qy`. */
particle_t *particle_take(void);

/* A fresh place of the `ga` sort, as an arena: the burst puts its struct,
 * its `oe` and its `ra` box there, and its `rendre` gives it back by
 * pico8_zone_arene_rendre. */
const allocator_t *particle_burst_slot_take(void);

/* Gives back each particle still in `emitter->qy` to the `particule`
 * handle, then the list's arena, and leaves `emitter->qy` NULL; nothing
 * when it already is. */
void emitter_release_particles(emitter_t *emitter);

/* Appends `self:mm()` records to `self.qy` while `rnd() < n`, subtracting 1
 * from `n` each turn: a fractional `n` genuinely has a chance of spawning
 * one more particle than `flr(n)`. A `self` whose class chain has no `mm`
 * adds nothing. */
void emitter_spawn(entity_t *self, p8_fixed_t n);

/* Takes gfx/mem explicitly, for the classes whose own `kj` calls it as a
 * plain function; `pico8_ei_dispatch` below is the same work reached
 * through a `kj` field, reading both from the dispatch context instead. */
void emitter_update(const p8_graphics_t *gfx, p8_memory_t *mem, entity_t *self);

/* `ra` is never read: `ei` declares only `self`, so Lua drops the
 * `entity.ra` argument `dc()` passes. */
void emitter_update_method(entity_t *self, const vec_t *ra);

#endif
