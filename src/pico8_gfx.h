#ifndef PICO8_GFX_H
#define PICO8_GFX_H

#include "pico8_mem.h"

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
    unsigned char transparent[16];
    unsigned int fillp_pattern;
} pico8_gfx_t;

void pico8_gfx_init(pico8_gfx_t *gfx);

void pico8_cls(pico8_mem_t *mem);

void pico8_pset(const pico8_gfx_t *gfx, pico8_mem_t *mem, int x, int y, int col);

void pico8_camera(pico8_gfx_t *gfx, int x, int y);

void pico8_camera_reset(pico8_gfx_t *gfx);

void pico8_palt(pico8_gfx_t *gfx, int col, int transparent);

int pico8_gfx_is_transparent(const pico8_gfx_t *gfx, int col);

void pico8_line(const pico8_gfx_t *gfx, pico8_mem_t *mem, int x1, int y1, int x2, int y2, int col);

void pico8_rect(const pico8_gfx_t *gfx, pico8_mem_t *mem, int x1, int y1, int x2, int y2, int col);

void pico8_rectfill(const pico8_gfx_t *gfx, pico8_mem_t *mem, int x1, int y1, int x2, int y2, int col);

void pico8_circ(const pico8_gfx_t *gfx, pico8_mem_t *mem, int cx, int cy, int r, int col);

void pico8_circfill(const pico8_gfx_t *gfx, pico8_mem_t *mem, int cx, int cy, int r, int col);

void pico8_fillp(pico8_gfx_t *gfx, unsigned int pattern);

void pico8_fillp_reset(pico8_gfx_t *gfx);

unsigned char pico8_sget(const pico8_mem_t *mem, int x, int y);

void pico8_spr(const pico8_gfx_t *gfx, pico8_mem_t *mem, int n, int x, int y, int w, int h);

void pico8_spr_default(const pico8_gfx_t *gfx, pico8_mem_t *mem, int n, int x, int y);

void pico8_mset(pico8_mem_t *mem, int x, int y, int val);

void pico8_map(const pico8_gfx_t *gfx, pico8_mem_t *mem, int cel_x, int cel_y, int sx, int sy, int cel_w, int cel_h);

#endif
