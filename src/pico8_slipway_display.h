#ifndef PICO8_SLIPWAY_DISPLAY_H
#define PICO8_SLIPWAY_DISPLAY_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"

/* Throughout this module a NULL instance argument is a safe no-op and an
 * absent or wrong-kind field reads as absent rather than crashing, but
 * `gfx` and `mem` must be real: the drawing primitives dereference them. */

/* TROU-7 */
/* Appends one particle table to `self.ts`, which owns it from then on. */
void pico8_jc_ch(pico8_instance_t *self, const pico8_instance_t *f, const pico8_instance_t *t);

/* The nested loops only ascend: `s` must be positive. */
void pico8_jc_lg(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self, pico8_fixed_t l,
                  pico8_fixed_t h, pico8_fixed_t s, int c);

/* TROU-7 */
/* Draws, and mutates `self`: `nh` climbs back to 20, every live particle's
 * `ra` is rebound to a fresh vector and its `l` decremented, and `fo` is
 * cleared even when the visibility gate drew nothing. */
void pico8_jc_kj(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self);

#endif
