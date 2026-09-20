#ifndef PICO8_GFX_H
#define PICO8_GFX_H

#include "pico8_mem.h"

#define PICO8_SCREEN_WIDTH 128
#define PICO8_SCREEN_HEIGHT 128

typedef struct {
    int camera_x;
    int camera_y;
    unsigned char transparent[16];
} pico8_gfx_t;

void pico8_gfx_init(pico8_gfx_t *gfx);

void pico8_cls(pico8_mem_t *mem);

void pico8_pset(const pico8_gfx_t *gfx, pico8_mem_t *mem, int x, int y, int col);

void pico8_camera(pico8_gfx_t *gfx, int x, int y);

void pico8_camera_reset(pico8_gfx_t *gfx);

void pico8_palt(pico8_gfx_t *gfx, int col, int transparent);

int pico8_gfx_is_transparent(const pico8_gfx_t *gfx, int col);

#endif
