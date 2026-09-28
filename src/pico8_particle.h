#ifndef PICO8_PARTICLE_H
#define PICO8_PARTICLE_H

#include "pico8_struct.h"
#include "pico8_gfx.h"
#include "pico8_list.h"
#include "pico8_mem.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* Throughout this module a NULL instance, gfx or mem argument is a safe
 * no-op, a non-NULL `self` must be a struct (asserted), and an absent or
 * wrong-kind field reads as its zero default. */

/* An element of `qy`, `{p,v,a,l}` in the cart: every `mm` returns a REF to
 * one on the heap, `ei` updates it in place, nothing frees it. `has_a` is 0
 * for `g_`'s particles, whose `a` is absent and never read. */
typedef struct {
    pico8_vec_t p;
    pico8_vec_t v;
    int has_a;
    pico8_vec_t a;
    pico8_fixed_t l;
} pico8_qy_t;

/* TROU-7 */
/* The fresh `qy` list is heap-allocated and owned by `self`, which never
 * frees it. */
void pico8_f_(pico8_struct_header_t *self);

/* Appends `self:mm()` records to `self.qy` while `rnd() < n`, subtracting 1
 * from `n` each turn: a fractional `n` genuinely has a chance of spawning
 * one more particle than `flr(n)`. A `self` whose class chain has no `mm`
 * adds nothing. */
void pico8_ez(pico8_struct_header_t *self, pico8_fixed_t n);

/* Takes gfx/mem explicitly, for the classes whose own `kj` calls it as a
 * plain function; `pico8_ei_dispatch` below is the same work reached
 * through a `kj` field, reading both from the dispatch context instead. */
void pico8_ei(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_struct_header_t *self);

/* `argc`/`arg0`/`arg1` are never read: `ei` declares only `self`, so Lua
 * drops the `entity.ra` argument `dc()` passes. */
pico8_value_t pico8_ei_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);

#endif
