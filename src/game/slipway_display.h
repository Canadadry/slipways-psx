#ifndef GAME_SLIPWAY_DISPLAY_H
#define GAME_SLIPWAY_DISPLAY_H

#include "engine/entity_class.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "p8/vram.h"
#include "game/slipway.h"
#include "engine/vec.h"

/* Throughout this module a NULL struct or entity argument is a safe no-op
 * and an absent or wrong-kind member or field reads as absent rather than
 * crashing, but the graphics state and the memory must be real: the
 * drawing primitives dereference them. */

/* A particle of a slipway's transit dots: `pos` moves by `velocity` and
 * `life` counts down once a frame. */
typedef struct {
    vec_t pos;
    vec_t velocity;
    p8_fixed_t life;
} transit_dot_t;

/* Appends one particle to the slipway's transit dots, in a slot of the
 * particle zone's transit dot sort, given back by slipway_draw when it dies
 * or by the slipway's release method. */
void slipway_spawn_transit_dot(slipway_t *self, const entity_t *f, const entity_t *t);

/* The nested loops only ascend: the step must be positive. */
void slipway_draw_line(const p8_graphics_t *gfx, p8_vram_t *mem, const slipway_t *self, p8_fixed_t l,
                  p8_fixed_t h, p8_fixed_t s, int c);

/* Draws, and mutates `self`: `fade_in_step` climbs back to 20, every live
 * particle's `pos` moves by its `velocity` in place and its `life` is
 * decremented (a dead one gives its slot back), and `targeted_by` is
 * cleared even when the visibility gate drew nothing. */
void slipway_draw(const p8_graphics_t *graphics, p8_vram_t *memory, slipway_t *self);

#endif
