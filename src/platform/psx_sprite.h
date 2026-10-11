#ifndef PLATFORM_PSX_SPRITE_H
#define PLATFORM_PSX_SPRITE_H

#include "p8/gfx.h"

/* A GPU SPRT: (x, y) and w x h in draw-area coordinates, inside the 160x120 screen; (u, v) its
 * top-left texel in its texture (the sprite sheet, or the font for print). */
typedef struct {
    int x;
    int y;
    int u;
    int v;
    int w;
    int h;
} psx_sprite_t;

/* spr's sprite after the camera, clipped to the screen. Returns 0 (sprite untouched) when nothing
 * of it shows, else 1. */
int psx_sprite_place(const p8_graphics_t *gfx, int n, int x, int y, int w, int h, psx_sprite_t *sprite);

/* Clips sprite, given in draw-area coordinates, to the screen, (u, v) following its top-left
 * corner. Returns 0 when nothing of it shows (sprite then unspecified), else 1. */
int psx_sprite_clip(psx_sprite_t *sprite);

#endif
