#ifndef PLATFORM_PSX_TILES_H
#define PLATFORM_PSX_TILES_H

#include "p8/gfx.h"

/* A GPU TILE in draw-area coordinates, inside the 160x120 screen, w and h at least 1. */
typedef struct {
    int x;
    int y;
    int w;
    int h;
} psx_tile_t;

enum { PSX_TILES_RECT_MAX = 4 };

/* pset's pixel after the camera, as a 1x1 tile. Returns 0 (tile untouched) when off screen, else 1. */
int psx_tiles_pset(const p8_graphics_t *gfx, int x, int y, psx_tile_t *tile);

/* rect's edges after the camera, clipped to the screen and never overlapping. Returns how many
 * tiles were written (0 when no edge shows). */
int psx_tiles_rect(const p8_graphics_t *gfx, int x1, int y1, int x2, int y2, psx_tile_t tiles[PSX_TILES_RECT_MAX]);

/* rectfill's box after the camera, clipped to the screen. Returns 0 (tile untouched) when off
 * screen, else 1. */
int psx_tiles_rectfill(const p8_graphics_t *gfx, int x1, int y1, int x2, int y2, psx_tile_t *tile);

#endif
