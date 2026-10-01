#ifndef PICO8_PARTICLE_H
#define PICO8_PARTICLE_H

#include "pico8_struct.h"
#include "pico8_game_data.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_ptrs.h"
#include "pico8_vec.h"
#include "pico8_zone_particules.h"

/* Throughout this module a NULL instance, gfx or mem argument is a safe
 * no-op and a non-NULL `self` must be a struct (asserted). `f_` and `ei`
 * stop on an entity whose `emitter` link is NULL; `ez` adds nothing there. */

/* An element of `qy`, `{p,v,a,l}` in the cart: every `mm` takes one from
 * pico8_particule_prendre, `ei` updates it in place and gives it back when
 * it takes it out of `qy`, or pico8_qy_rendre with its list. `has_a` is 0 for
 * `g_`'s particles, whose `a` is absent and never read. */
typedef struct pico8_qy {
    pico8_vec_t p;
    pico8_vec_t v;
    int has_a;
    pico8_vec_t a;
    pico8_fixed_t l;
} pico8_qy_t;

/* What `ei`, `ez`, `f_` and `g_:mm` read of their host, the particle
 * emitter's own fields, stored in the host (`emitter_own`) and never copied;
 * the rest is read on `def`, the host's archetype's emitter. `qy` is NULL
 * until `f_`, then in its own arena of the sort `qy_sorte`, owned by the
 * host, whose `rendre` gives it back by pico8_qy_rendre. `pn`, `du`
 * are read by `g_:mm` only: a `ga` leaves them unwritten. */
typedef struct pico8_emitter {
    const pico8_emitter_def_t *def;
    pico8_fixed_t pn;
    pico8_fixed_t du;
    pico8_fixed_t py;
    const pico8_int_seq_t *k_; /* NULL: absent, every particle drawn in colour 0 */
    pico8_fixed_t qo;
    pico8_fixed_t ro;
    pico8_fixed_t qg;
    pico8_fixed_t q_;
    pico8_ptrs_t *qy;
    pico8_particules_sorte_t qy_sorte; /* meaningless while `qy` is NULL */
} pico8_emitter_t;

/* Gives back the list `self` already has (pico8_qy_rendre), then takes a
 * fresh one, reserved to its capacity: `fv` takes `qy_fv`, every other
 * host `qy` by pico8_f_. */
void pico8_f_(pico8_struct_header_t *self);
void pico8_f__dans(pico8_struct_header_t *self, pico8_particules_sorte_t sorte);

/* An uninitialised place of the `particule` sort; given back by `ei` or
 * pico8_qy_rendre once in a `qy`. */
pico8_qy_t *pico8_particule_prendre(void);

/* Gives back each particle still in `emitter->qy` to the `particule`
 * handle, then the list's arena, and leaves `emitter->qy` NULL; nothing
 * when it already is. */
void pico8_qy_rendre(pico8_emitter_t *emitter);

/* Appends `self:mm()` records to `self.qy` while `rnd() < n`, subtracting 1
 * from `n` each turn: a fractional `n` genuinely has a chance of spawning
 * one more particle than `flr(n)`. A `self` whose class chain has no `mm`
 * adds nothing. */
void pico8_ez(pico8_struct_header_t *self, pico8_fixed_t n);

/* Takes gfx/mem explicitly, for the classes whose own `kj` calls it as a
 * plain function; `pico8_ei_dispatch` below is the same work reached
 * through a `kj` field, reading both from the dispatch context instead. */
void pico8_ei(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_struct_header_t *self);

/* `ra` is never read: `ei` declares only `self`, so Lua drops the
 * `entity.ra` argument `dc()` passes. */
void pico8_ei_dispatch(pico8_struct_header_t *self, const pico8_vec_t *ra);

#endif
