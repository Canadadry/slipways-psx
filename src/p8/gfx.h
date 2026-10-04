#ifndef P8_GFX_H
#define P8_GFX_H

#include "p8/vram.h"

#define P8_SPRITE_SIZE 8

typedef struct {
    int camera_x;
    int camera_y;
    unsigned int fillp_pattern;
    int fillp_transparent;
    int pen_color;
} p8_graphics_t;

void p8_gfx_init(p8_graphics_t *gfx);

void p8_cls(p8_vram_t *mem);

void p8_pset(const p8_graphics_t *gfx, p8_vram_t *mem, int x, int y, int col);

void p8_camera(p8_graphics_t *gfx, int x, int y);

void p8_camera_reset(p8_graphics_t *gfx);

void p8_palt(p8_vram_t *mem, int col, int transparent);

int p8_gfx_is_transparent(const p8_vram_t *mem, int col);

void p8_line(const p8_graphics_t *gfx, p8_vram_t *mem, int x1, int y1, int x2, int y2, int col);

void p8_rect(const p8_graphics_t *gfx, p8_vram_t *mem, int x1, int y1, int x2, int y2, int col);

void p8_rectfill(const p8_graphics_t *gfx, p8_vram_t *mem, int x1, int y1, int x2, int y2, int col);

void p8_circ(const p8_graphics_t *gfx, p8_vram_t *mem, int cx, int cy, int r, int col);

void p8_circfill(const p8_graphics_t *gfx, p8_vram_t *mem, int cx, int cy, int r, int col);

void p8_fillp(p8_graphics_t *gfx, unsigned int pattern);

void p8_fillp_transparent(p8_graphics_t *gfx, unsigned int pattern);

void p8_fillp_reset(p8_graphics_t *gfx);

unsigned char p8_sget(const p8_vram_t *mem, int x, int y);

void p8_spr(const p8_graphics_t *gfx, p8_vram_t *mem, int n, int x, int y, int w, int h);

void p8_spr_default(const p8_graphics_t *gfx, p8_vram_t *mem, int n, int x, int y);

void p8_mset(p8_vram_t *mem, int x, int y, int val);

void p8_map(const p8_graphics_t *gfx, p8_vram_t *mem, int cel_x, int cel_y, int sx, int sy, int cel_w, int cel_h);

#define P8_FONT_GLYPH_WIDTH 3
#define P8_FONT_GLYPH_HEIGHT 5
#define P8_FONT_CELL_WIDTH 4

void p8_print(const p8_graphics_t *gfx, p8_vram_t *mem, const char *str, int x, int y, int col);

#endif
