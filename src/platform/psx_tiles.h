#ifndef PLATFORM_PSX_TILES_H
#define PLATFORM_PSX_TILES_H

#include "p8/gfx.h"

/* A tile in screen coordinates, inside the 160x120 screen, w and h at least 1. */
typedef struct {
    int x;
    int y;
    int w;
    int h;
} psx_tile_t;

/* A GPU LINE_F2 in draw-area coordinates; endpoints may lie off screen. */
typedef struct {
    int x1;
    int y1;
    int x2;
    int y2;
} psx_line_t;

enum { PSX_TILES_RECT_MAX = 4, PSX_TILES_CIRCFILL_MAX = P8_SCREEN_HEIGHT, PSX_TILES_SCALE = 2 };

typedef void (*psx_tiles_emit_t)(psx_tile_t tile, void *context);

/* pset's pixel after the camera, as a 1x1 tile. Returns 0 (tile untouched) when off screen, else 1. */
int psx_tiles_pset(const p8_graphics_t *gfx, int x, int y, psx_tile_t *tile);

/* line's endpoints after the camera, not clipped: the GPU clips to the draw area. Returns 0 (line
 * untouched) when the line's box misses the screen, as p8_draw_cost_line does, else 1. */
int psx_tiles_line(const p8_graphics_t *gfx, int x1, int y1, int x2, int y2, psx_line_t *line);

/* rect's edges after the camera, clipped to the screen and never overlapping. Returns how many
 * tiles were written (0 when no edge shows). */
int psx_tiles_rect(const p8_graphics_t *gfx, int x1, int y1, int x2, int y2, psx_tile_t tiles[PSX_TILES_RECT_MAX]);

/* rectfill's box after the camera, clipped to the screen. Returns 0 (tile untouched) when off
 * screen, else 1. */
int psx_tiles_rectfill(const p8_graphics_t *gfx, int x1, int y1, int x2, int y2, psx_tile_t *tile);

/* circ's pixels after the camera, from gfx_software.c's midpoint loop, each emitted once as a 1x1
 * tile when on screen. Returns how many were emitted, at most 8 per iteration of the loop. */
int psx_tiles_circ(const p8_graphics_t *gfx, int cx, int cy, int r, psx_tiles_emit_t emit, void *context);

/* circfill's disc after the camera, from gfx_software.c's midpoint loop, as one clipped 1-pixel-high
 * span per screen row, top to bottom. Returns how many spans were written, at most 4 per iteration
 * of the loop. */
int psx_tiles_circfill(const p8_graphics_t *gfx, int cx, int cy, int r, psx_tile_t spans[PSX_TILES_CIRCFILL_MAX]);

/* tile with x, y, w and h multiplied by PSX_TILES_SCALE, in display-buffer coordinates: inside
 * 320x240 whenever tile lies inside the 160x120 screen. */
psx_tile_t psx_tiles_double(psx_tile_t tile);

#endif
