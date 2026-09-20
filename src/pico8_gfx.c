#include "pico8_gfx.h"

void pico8_gfx_init(pico8_gfx_t *gfx) {
    gfx->camera_x = 0;
    gfx->camera_y = 0;
    for (int i = 0; i < 16; i++) {
        gfx->transparent[i] = 0;
    }
    gfx->transparent[0] = 1;
}

void pico8_cls(pico8_mem_t *mem) {
    for (int i = 0; i < PICO8_SCREEN_WIDTH * PICO8_SCREEN_HEIGHT; i++) {
        pico8_poke(mem, PICO8_ADDR_SCREEN + i, 0);
    }
}

void pico8_pset(const pico8_gfx_t *gfx, pico8_mem_t *mem, int x, int y, int col) {
    int sx = x - gfx->camera_x;
    int sy = y - gfx->camera_y;
    if (sx < 0 || sx >= PICO8_SCREEN_WIDTH || sy < 0 || sy >= PICO8_SCREEN_HEIGHT) {
        return;
    }
    pico8_poke(mem, PICO8_ADDR_SCREEN + sy * PICO8_SCREEN_WIDTH + sx, (unsigned char)col);
}

void pico8_camera(pico8_gfx_t *gfx, int x, int y) {
    gfx->camera_x = x;
    gfx->camera_y = y;
}

void pico8_camera_reset(pico8_gfx_t *gfx) {
    pico8_camera(gfx, 0, 0);
}

void pico8_palt(pico8_gfx_t *gfx, int col, int transparent) {
    if (col < 0 || col > 15) {
        return;
    }
    gfx->transparent[col] = transparent ? 1 : 0;
}

int pico8_gfx_is_transparent(const pico8_gfx_t *gfx, int col) {
    if (col < 0 || col > 15) {
        return 0;
    }
    return gfx->transparent[col];
}
