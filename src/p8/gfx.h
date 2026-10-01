#ifndef PICO8_GFX_H
#define PICO8_GFX_H

#include "p8/mem.h"

#define PICO8_SCREEN_WIDTH 128
#define PICO8_SCREEN_HEIGHT 128

#define PICO8_SPRITE_SIZE 8
#define PICO8_SPRITE_SHEET_WIDTH 128
#define PICO8_SPRITE_SHEET_HEIGHT 128
#define PICO8_MAP_WIDTH 128
#define PICO8_MAP_HEIGHT 64

typedef struct {
    int camera_x;
    int camera_y;
    unsigned int fillp_pattern;
    /* PICO-8's fill-pattern transparency flag, the `.1` suffix of a
     * fillp() literal: where a pattern bit is set, the second color is not
     * drawn at all. Set only by pico8_fillp_transparent(). */
    int fillp_transparent;
    /* PICO-8's current pen color, what a draw call uses when `col` is
     * omitted. Only pico8_rk() reads and writes it, for the cart's one
     * col-omitted call site; the other primitives here take `col` and
     * leave this field alone. */
    int pen_color;
} pico8_gfx_t;

void pico8_gfx_init(pico8_gfx_t *gfx);

void pico8_cls(pico8_mem_t *mem);

void pico8_pset(const pico8_gfx_t *gfx, pico8_mem_t *mem, int x, int y, int col);

void pico8_camera(pico8_gfx_t *gfx, int x, int y);

void pico8_camera_reset(pico8_gfx_t *gfx);

/* Flips the 0x80 bit of the draw-palette byte at 0x5f00+c -- the single
 * store pal() and dv() write too, hence `mem` and not `gfx`, and hence
 * dv() overriding a palt() that came before it. A second gfx-side flag
 * array once shadowed this byte: every icon lost its black outline. */
void pico8_palt(pico8_mem_t *mem, int col, int transparent);

int pico8_gfx_is_transparent(const pico8_mem_t *mem, int col);

void pico8_line(const pico8_gfx_t *gfx, pico8_mem_t *mem, int x1, int y1, int x2, int y2, int col);

void pico8_rect(const pico8_gfx_t *gfx, pico8_mem_t *mem, int x1, int y1, int x2, int y2, int col);

void pico8_rectfill(const pico8_gfx_t *gfx, pico8_mem_t *mem, int x1, int y1, int x2, int y2, int col);

void pico8_circ(const pico8_gfx_t *gfx, pico8_mem_t *mem, int cx, int cy, int r, int col);

void pico8_circfill(const pico8_gfx_t *gfx, pico8_mem_t *mem, int cx, int cy, int r, int col);

void pico8_fillp(pico8_gfx_t *gfx, unsigned int pattern);

/* fillp(0b<pattern>.1), the transparency-flag form. The cart writes it at
 * one call site (pico8_jo_kj); its other two patterns are plain
 * two-color ones and go through pico8_fillp above. */
void pico8_fillp_transparent(pico8_gfx_t *gfx, unsigned int pattern);

void pico8_fillp_reset(pico8_gfx_t *gfx);

unsigned char pico8_sget(const pico8_mem_t *mem, int x, int y);

void pico8_spr(const pico8_gfx_t *gfx, pico8_mem_t *mem, int n, int x, int y, int w, int h);

void pico8_spr_default(const pico8_gfx_t *gfx, pico8_mem_t *mem, int n, int x, int y);

void pico8_mset(pico8_mem_t *mem, int x, int y, int val);

void pico8_map(const pico8_gfx_t *gfx, pico8_mem_t *mem, int cel_x, int cel_y, int sx, int sy, int cel_w, int cel_h);

/* A glyph is a 3x5 bitmap left-aligned in a 4px advance cell, matching
 * PICO-8's 4px character pitch. pico8_gfx.c holds the covered character
 * set and the placeholder drawn outside it. */
#define PICO8_FONT_GLYPH_WIDTH 3
#define PICO8_FONT_GLYPH_HEIGHT 5
#define PICO8_FONT_CELL_WIDTH 4

void pico8_print(const pico8_gfx_t *gfx, pico8_mem_t *mem, const char *str, int x, int y, int col);

#endif
