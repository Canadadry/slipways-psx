#ifndef PLATFORM_PSX_PATTERN_H
#define PLATFORM_PSX_PATTERN_H

#include "p8/gfx.h"
#include "platform/psx_sprite.h"
#include "platform/psx_tiles.h"

/* The GPU texture window (setTexWindow's RECT: x = offset_x, y = offset_y, w = mask_x, h = mask_y),
 * in 8-texel units of the texture page: it repeats one pattern slot's 16x16 x2 tile over every texel. */
typedef struct {
    int mask_x;
    int mask_y;
    int offset_x;
    int offset_y;
} psx_pattern_window_t;

/* slot < GPU_PATTERN_SLOTS. */
psx_pattern_window_t psx_pattern_window(int slot);

/* A patterned fill's box or span (from psx_tiles_rectfill or psx_tiles_circfill) as a SPRT whose
 * (u, v) is its top-left pixel's world phase: under psx_pattern_window, each pixel reads the tile's
 * texel for its world (x, y), as fillp does. */
psx_sprite_t psx_pattern_sprite(const p8_graphics_t *gfx, psx_tile_t tile);

enum { PSX_PATTERN_PIECES_MAX = 2 };

/* psx_pattern_sprite doubled by psx_sprite_double, cut where its U would cross 256 (the second
 * piece then starts at U 0, the same phase under the window). Returns how many pieces it wrote. */
int psx_pattern_sprites_x2(const p8_graphics_t *gfx, psx_tile_t tile, psx_sprite_t pieces[PSX_PATTERN_PIECES_MAX]);

#endif
