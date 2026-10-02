#ifndef GAME_SLIPWAY_DISPLAY_H
#define GAME_SLIPWAY_DISPLAY_H

#include "engine/entity_class.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "p8/mem.h"
#include "game/slipway.h"
#include "engine/vec.h"

/* Throughout this module a NULL struct or entity argument is a safe no-op
 * and a nil or wrong-kind member or field reads as absent rather than
 * crashing, but `gfx` and `mem` must be real: the drawing primitives
 * dereference them. */

/* A particle of `jc.ts`: `ra` moves by `v` and `l` counts down once a frame. */
typedef struct {
    vec_t pos;
    vec_t velocity;
    p8_fixed_t life;
} transit_dot_t;

/* Appends one particle to `self.ts`, in a place of the particles zone's
 * `ts` sort, given back by `kj` when it dies or by the slipway's `rendre`. */
void slipway_spawn_transit_dot(slipway_t *self, const entity_t *f, const entity_t *t);

/* The nested loops only ascend: `s` must be positive. */
void slipway_draw_line(const p8_graphics_t *gfx, p8_memory_t *mem, const slipway_t *self, p8_fixed_t l,
                  p8_fixed_t h, p8_fixed_t s, int c);

/* Draws, and mutates `self`: `nh` climbs back to 20, every live particle's
 * `ra` moves by its `v` in place and its `l` is decremented (a dead one gives
 * its place back), and `fo` is cleared even when the visibility gate drew
 * nothing. */
void slipway_draw(const p8_graphics_t *gfx, p8_memory_t *mem, slipway_t *self);

#endif
