#ifndef PICO8_SLIPWAY_DISPLAY_H
#define PICO8_SLIPWAY_DISPLAY_H

#include "engine/entity_class.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "p8/mem.h"
#include "pico8_slipway.h"
#include "engine/vec.h"

/* Throughout this module a NULL struct or entity argument is a safe no-op
 * and a nil or wrong-kind member or field reads as absent rather than
 * crashing, but `gfx` and `mem` must be real: the drawing primitives
 * dereference them. */

/* A particle of `jc.ts`: `ra` moves by `v` and `l` counts down once a frame. */
typedef struct {
    pico8_vec_t ra;
    pico8_vec_t v;
    pico8_fixed_t l;
} pico8_ts_t;

/* Appends one particle to `self.ts`, in a place of the particles zone's
 * `ts` sort, given back by `kj` when it dies or by the slipway's `rendre`. */
void pico8_jc_ch(pico8_jc_t *self, const pico8_struct_header_t *f, const pico8_struct_header_t *t);

/* The nested loops only ascend: `s` must be positive. */
void pico8_jc_lg(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_jc_t *self, pico8_fixed_t l,
                  pico8_fixed_t h, pico8_fixed_t s, int c);

/* Draws, and mutates `self`: `nh` climbs back to 20, every live particle's
 * `ra` moves by its `v` in place and its `l` is decremented (a dead one gives
 * its place back), and `fo` is cleared even when the visibility gate drew
 * nothing. */
void pico8_jc_kj(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_jc_t *self);

#endif
