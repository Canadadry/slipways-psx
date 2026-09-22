#ifndef PICO8_PARTICLE_H
#define PICO8_PARTICLE_H

#include "pico8_class.h"
#include "pico8_gfx.h"
#include "pico8_list.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_value.h"

/* Throughout this module a NULL instance, gfx or mem argument is a safe
 * no-op, and an absent or wrong-kind field reads as its zero default.
 *
 * The C shape of a particle record -- what every `mm` must return, and what
 * `qy` therefore holds -- is this module's own contract, carried by no
 * name: a REF to a heap `pico8_ob_table_t` with "p" and "v" (REFs to heap
 * `pico8_vec_t`), an optional "a" (same shape; absent is a real, checked
 * case) and "l" (fixed). */

/* TROU-7 */
/* `pico8_class_pq_fn`-shaped, so `g_.pq=f_`/`hj.pq=f_` port as a direct
 * `pico8_class_set_pq(cls, pico8_f_)` with no adapter. The fresh `qy` list
 * is heap-allocated and owned by the instance, which never frees it. */
void pico8_f_(pico8_instance_t *self);

/* Appends `self:mm()` records to `self.qy` while `rnd() < n`, subtracting 1
 * from `n` each turn: a fractional `n` genuinely has a chance of spawning
 * one more particle than `flr(n)`. A `self` whose class chain has no `mm`
 * adds nothing. */
void pico8_ez(pico8_instance_t *self, pico8_fixed_t n);

/* Takes gfx/mem explicitly, for the classes whose own `kj` calls it as a
 * plain function; `pico8_ei_dispatch` below is the same work reached
 * through a `kj` field, reading both from the dispatch context instead. */
void pico8_ei(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self);

/* `argc`/`arg0`/`arg1` are never read: `ei` declares only `self`, so Lua
 * drops the `entity.ra` argument `dc()` passes. */
pico8_value_t pico8_ei_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);

#endif
