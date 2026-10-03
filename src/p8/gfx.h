#ifndef P8_GFX_H
#define P8_GFX_H

#include "p8/vram.h"

#define P8_SPRITE_SIZE 8

typedef struct {
    int camera_x;
    int camera_y;
    unsigned int fillp_pattern;
    /* PICO-8's fill-pattern transparency flag, the `.1` suffix of a
     * fillp() literal: where a pattern bit is set, the second color is not
     * drawn at all. Set only by p8_fillp_transparent(). */
    int fillp_transparent;
    /* PICO-8's current pen color, what a draw call uses when `col` is
     * omitted. Only hud_draw_bar() writes it and hud_draw_bar_default()
     * reads it, for the one call site that omits the color; the other
     * primitives here take `col` and leave this field alone. */
    int pen_color;
} p8_graphics_t;

void p8_gfx_init(p8_graphics_t *gfx);

void p8_cls(p8_vram_t *mem);

void p8_pset(const p8_graphics_t *gfx, p8_vram_t *mem, int x, int y, int col);

void p8_camera(p8_graphics_t *gfx, int x, int y);

void p8_camera_reset(p8_graphics_t *gfx);

/* Flips the transparency bit of draw_palette[col] -- the single store
 * hud_select_palette_block() writes too, hence the memory and not the
 * graphics state, and hence that block overriding a p8_palt()
 * that came before it. A second flag array in the graphics state once
 * shadowed this byte: every icon lost its black outline. */
void p8_palt(p8_vram_t *mem, int col, int transparent);

int p8_gfx_is_transparent(const p8_vram_t *mem, int col);

void p8_line(const p8_graphics_t *gfx, p8_vram_t *mem, int x1, int y1, int x2, int y2, int col);

void p8_rect(const p8_graphics_t *gfx, p8_vram_t *mem, int x1, int y1, int x2, int y2, int col);

void p8_rectfill(const p8_graphics_t *gfx, p8_vram_t *mem, int x1, int y1, int x2, int y2, int col);

void p8_circ(const p8_graphics_t *gfx, p8_vram_t *mem, int cx, int cy, int r, int col);

void p8_circfill(const p8_graphics_t *gfx, p8_vram_t *mem, int cx, int cy, int r, int col);

void p8_fillp(p8_graphics_t *gfx, unsigned int pattern);

/* fillp(0b<pattern>.1), the transparency-flag form, used at one call site
 * (planet_draw); the other two patterns in use are plain two-color ones and
 * go through p8_fillp above. */
void p8_fillp_transparent(p8_graphics_t *gfx, unsigned int pattern);

void p8_fillp_reset(p8_graphics_t *gfx);

unsigned char p8_sget(const p8_vram_t *mem, int x, int y);

void p8_spr(const p8_graphics_t *gfx, p8_vram_t *mem, int n, int x, int y, int w, int h);

void p8_spr_default(const p8_graphics_t *gfx, p8_vram_t *mem, int n, int x, int y);

void p8_mset(p8_vram_t *mem, int x, int y, int val);

void p8_map(const p8_graphics_t *gfx, p8_vram_t *mem, int cel_x, int cel_y, int sx, int sy, int cel_w, int cel_h);

/* A glyph is a 3x5 bitmap left-aligned in a 4px advance cell, matching
 * PICO-8's 4px character pitch. p8/gfx.c holds the covered character set
 * and the placeholder drawn outside it. */
#define P8_FONT_GLYPH_WIDTH 3
#define P8_FONT_GLYPH_HEIGHT 5
#define P8_FONT_CELL_WIDTH 4

void p8_print(const p8_graphics_t *gfx, p8_vram_t *mem, const char *str, int x, int y, int col);

#endif
