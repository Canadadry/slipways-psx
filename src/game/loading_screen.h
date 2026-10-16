#ifndef GAME_LOADING_SCREEN_H
#define GAME_LOADING_SCREEN_H

#include "p8/gfx.h"

/* The PSX displays each frame one frame late: with 1, it would show the title during the generation stall. */
#define LOADING_SCREEN_FRAMES 2

/* Draws with the camera at 0,0, no fill pattern and the default palette; restores what it found. */
void loading_screen_draw(p8_graphics_t *gfx, p8_vram_t *mem);

#endif
