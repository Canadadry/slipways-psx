#ifndef PICO8_PROBE_H
#define PICO8_PROBE_H

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
    const pico8_g__def_t *def;
    pico8_emitter_t *emitter;
    pico8_probe_t *probe;
    const vec_t *ra;
    const vec_t *p;
    int p_;
    int m_;
    const allocator_t *prise;
    pico8_emitter_t emitter_own;
    pico8_probe_t probe_own;
} pico8_g__t;

/* TROU-8 */
/* Puts the provisional `fm` back in the slot, its `jk`/`nz` nil. Shuts the
 * module down first, so it is idempotent. */
void pico8_probe_init(void);

/* Empties the `fm` slot without reading it, frees no probe. Idempotent,
 * and safe before the first `pico8_probe_init()`. */
void pico8_probe_shutdown(void);

/* 1 from pico8_probe_init() to pico8_probe_shutdown(), 0 otherwise. */
int pico8_g__ready(void);

/* Initializes an allocated pico8_g__t under `cls`: ke's part, g_'s 4
 * written members nil, `prise` NULL, `def` kept, `ra`, the emitter and the probe copied
 * from it. Reads no key, runs neither `pq` nor `lt`, records no coverage. */
void pico8_g__struct_init(pico8_g__t *self, const entity_class_t *cls, const pico8_g__def_t *def);

/* The keys of `pico8_g__new`, read, not kept. `t` is `header.t`, whose default
 * is 0: no flag. `pn`, `du`, `py` have non-zero defaults, so a key at 0
 * needs its `has_` flag. */
typedef struct {
    p8_fixed_t t;
    int has_pn;
    p8_fixed_t pn;
    int has_du;
    p8_fixed_t du;
    int has_py;
    p8_fixed_t py;
} pico8_g__keys_t;

/* `keys` NULL means none. Built in a grab arena (`pico8_jo_prise_ouvrir`),
 * so no other grab may be open. The new probe registers itself into
 * `hv`/`cv`/`cg` through `lt` (and gets its particle list from `f_`, which
 * its `rendre` gives back). Asserts when called before `pico8_probe_init()`. */
pico8_g__t *pico8_g__new(const pico8_g__keys_t *keys);

/* A particle (`has_a` 0) from pico8_particule_prendre, which `ez` appends
 * to `qy`. */
pico8_qy_t *pico8_g__mm(entity_t *self);

/* `rendre` of `g_` and `fv`: gives back their `qy`, and the grab arena of
 * a `g_` not placed (`me` other than "ly"); a placed one keeps it until the
 * generation is given back. */
void pico8_g__rendre_dispatch(entity_t *self);

#endif
