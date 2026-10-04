#ifndef GAME_SLIPWAY_DISPLAY_H
#define GAME_SLIPWAY_DISPLAY_H

#include "engine/entity_class.h"
#include "p8/fixed.h"
#include "p8/gfx.h"
#include "p8/vram.h"
#include "game/slipway.h"
#include "engine/vec.h"

typedef struct {
    vec_t pos;
    vec_t velocity;
    p8_fixed_t life;
} transit_dot_t;

void slipway_spawn_transit_dot(slipway_t *self, const entity_t *f, const entity_t *t);

void slipway_draw_line(const p8_graphics_t *gfx, p8_vram_t *mem, const slipway_t *self, p8_fixed_t l,
                  p8_fixed_t h, p8_fixed_t s, int c);

void slipway_draw(const p8_graphics_t *graphics, p8_vram_t *memory, slipway_t *self);

#endif
