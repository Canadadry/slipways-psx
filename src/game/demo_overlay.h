#ifndef GAME_DEMO_OVERLAY_H
#define GAME_DEMO_OVERLAY_H

#include "game/demo_data.h"
#include "p8/gfx.h"

/* The index into captions of the caption shown on demo frame `frame` (from <= frame < to), or -1. */
int demo_overlay_find_caption(const demo_data_caption_t *captions, int count, int frame);

/* Draws the demo tag and, when a compiled caption covers demo frame `frame`, its band and lines; nothing when
 * frame < 0. Draws with a copy of *gfx whose camera and fill pattern are reset: *gfx is left unchanged. */
void demo_overlay_draw(const p8_graphics_t *gfx, p8_vram_t *mem, int frame);

#endif
