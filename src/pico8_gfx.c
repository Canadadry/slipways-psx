#include "pico8_gfx.h"

#include <stdlib.h>

void pico8_gfx_init(pico8_gfx_t *gfx) {
    gfx->camera_x = 0;
    gfx->camera_y = 0;
    for (int i = 0; i < 16; i++) {
        gfx->transparent[i] = 0;
    }
    gfx->transparent[0] = 1;
    gfx->fillp_pattern = 0;
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

void pico8_line(const pico8_gfx_t *gfx, pico8_mem_t *mem, int x1, int y1, int x2, int y2, int col) {
    int dx = abs(x2 - x1);
    int sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1);
    int sy = y1 < y2 ? 1 : -1;
    int err = dx + dy;
    int x = x1;
    int y = y1;

    for (;;) {
        pico8_pset(gfx, mem, x, y, col);
        if (x == x2 && y == y2) {
            break;
        }
        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y += sy;
        }
    }
}

/* fillp's 16-bit pattern is a 4x4 grid of bits, read most-significant-bit
 * first, row-major from top-left: bit 15 is (row 0, col 0), bit 0 is
 * (row 3, col 3). A set bit means "skip this pixel"; fillp(0) (the
 * default/reset value) has every bit clear, so every pixel is drawn.
 * Only *fill* operations (rectfill, circfill) consult this: every
 * fillp(...)-bracketed call site found in pico8/lua.lua surrounds a fill
 * call and explicitly resets the pattern with fillp() before any
 * unfilled line/rect/circ call, so those never need to respect it. */
static int pico8_fillp_skips(const pico8_gfx_t *gfx, int x, int y) {
    int row = ((y % 4) + 4) % 4;
    int col = ((x % 4) + 4) % 4;
    int bit_index = 15 - (row * 4 + col);
    return (gfx->fillp_pattern >> bit_index) & 1;
}

static int pico8_min(int a, int b) {
    return a < b ? a : b;
}

static int pico8_max(int a, int b) {
    return a < b ? b : a;
}

void pico8_rect(const pico8_gfx_t *gfx, pico8_mem_t *mem, int x1, int y1, int x2, int y2, int col) {
    int xmin = pico8_min(x1, x2);
    int xmax = pico8_max(x1, x2);
    int ymin = pico8_min(y1, y2);
    int ymax = pico8_max(y1, y2);

    for (int x = xmin; x <= xmax; x++) {
        pico8_pset(gfx, mem, x, ymin, col);
        pico8_pset(gfx, mem, x, ymax, col);
    }
    for (int y = ymin; y <= ymax; y++) {
        pico8_pset(gfx, mem, xmin, y, col);
        pico8_pset(gfx, mem, xmax, y, col);
    }
}

static void pico8_circ_plot8(const pico8_gfx_t *gfx, pico8_mem_t *mem, int cx, int cy, int dx, int dy, int col) {
    pico8_pset(gfx, mem, cx + dx, cy + dy, col);
    pico8_pset(gfx, mem, cx - dx, cy + dy, col);
    pico8_pset(gfx, mem, cx + dx, cy - dy, col);
    pico8_pset(gfx, mem, cx - dx, cy - dy, col);
    pico8_pset(gfx, mem, cx + dy, cy + dx, col);
    pico8_pset(gfx, mem, cx - dy, cy + dx, col);
    pico8_pset(gfx, mem, cx + dy, cy - dx, col);
    pico8_pset(gfx, mem, cx - dy, cy - dx, col);
}

void pico8_circ(const pico8_gfx_t *gfx, pico8_mem_t *mem, int cx, int cy, int r, int col) {
    if (r < 0) {
        return;
    }
    int x = r;
    int y = 0;
    int err = 0;

    while (x >= y) {
        pico8_circ_plot8(gfx, mem, cx, cy, x, y, col);
        y++;
        if (err <= 0) {
            err += 2 * y + 1;
        }
        if (err > 0) {
            x--;
            err -= 2 * x + 1;
        }
    }
}

static void pico8_circfill_span(const pico8_gfx_t *gfx, pico8_mem_t *mem, int x1, int x2, int y, int col) {
    int xmin = pico8_min(x1, x2);
    int xmax = pico8_max(x1, x2);

    for (int x = xmin; x <= xmax; x++) {
        if (!pico8_fillp_skips(gfx, x, y)) {
            pico8_pset(gfx, mem, x, y, col);
        }
    }
}

void pico8_circfill(const pico8_gfx_t *gfx, pico8_mem_t *mem, int cx, int cy, int r, int col) {
    if (r < 0) {
        return;
    }
    int x = r;
    int y = 0;
    int err = 0;

    while (x >= y) {
        pico8_circfill_span(gfx, mem, cx - x, cx + x, cy + y, col);
        pico8_circfill_span(gfx, mem, cx - x, cx + x, cy - y, col);
        pico8_circfill_span(gfx, mem, cx - y, cx + y, cy + x, col);
        pico8_circfill_span(gfx, mem, cx - y, cx + y, cy - x, col);
        y++;
        if (err <= 0) {
            err += 2 * y + 1;
        }
        if (err > 0) {
            x--;
            err -= 2 * x + 1;
        }
    }
}

void pico8_fillp(pico8_gfx_t *gfx, unsigned int pattern) {
    gfx->fillp_pattern = pattern & 0xFFFFu;
}

void pico8_fillp_reset(pico8_gfx_t *gfx) {
    pico8_fillp(gfx, 0);
}

unsigned char pico8_sget(const pico8_mem_t *mem, int x, int y) {
    if (x < 0 || x >= PICO8_SPRITE_SHEET_WIDTH || y < 0 || y >= PICO8_SPRITE_SHEET_HEIGHT) {
        return 0;
    }
    return pico8_peek(mem, PICO8_ADDR_SPRITE_SHEET + y * PICO8_SPRITE_SHEET_WIDTH + x);
}

/* Sprite index n's top-left pixel sits at sheet coordinate
 * ((n % 16) * 8, (n / 16) * 8): the sheet is 128x128 pixels, laid out as
 * 16x16 blocks of 8x8-pixel sprites (128/8 = 16 per row). w/h are in
 * sprite-size (8px) multiples, so a wide/tall sprite just reads a larger
 * contiguous pixel rectangle from the sheet, spilling into neighboring
 * sprite slots exactly like real PICO-8's multi-cell sprites do. */
void pico8_spr(const pico8_gfx_t *gfx, pico8_mem_t *mem, int n, int x, int y, int w, int h) {
    const int sprites_per_row = PICO8_SPRITE_SHEET_WIDTH / PICO8_SPRITE_SIZE;
    int sheet_x = (n % sprites_per_row) * PICO8_SPRITE_SIZE;
    int sheet_y = (n / sprites_per_row) * PICO8_SPRITE_SIZE;
    int pixel_w = w * PICO8_SPRITE_SIZE;
    int pixel_h = h * PICO8_SPRITE_SIZE;

    for (int dy = 0; dy < pixel_h; dy++) {
        for (int dx = 0; dx < pixel_w; dx++) {
            unsigned char col = pico8_sget(mem, sheet_x + dx, sheet_y + dy);
            if (pico8_gfx_is_transparent(gfx, col)) {
                continue;
            }
            pico8_pset(gfx, mem, x + dx, y + dy, col);
        }
    }
}

void pico8_spr_default(const pico8_gfx_t *gfx, pico8_mem_t *mem, int n, int x, int y) {
    pico8_spr(gfx, mem, n, x, y, 1, 1);
}

/* Map cell addressing mirrors pico8_assets.c's map_cell_sink: cel rows
 * 0-31 live in the base map region, rows 32-63 spill into the shared
 * sprite-sheet/map region (pico8/map.json's cart data is 128x64 cells,
 * larger than the base 128x32 map region). Kept in sync by hand rather
 * than shared code, since pico8_assets.c's sink is private to its own
 * flat-JSON-array loading loop. */
#define PICO8_MAP_BASE_ROWS 32

static int pico8_map_cell_addr(int x, int y) {
    if (x < 0 || x >= PICO8_MAP_WIDTH || y < 0 || y >= PICO8_MAP_HEIGHT) {
        return -1;
    }
    if (y < PICO8_MAP_BASE_ROWS) {
        return PICO8_ADDR_MAP + y * PICO8_MAP_WIDTH + x;
    }
    return PICO8_ADDR_SPRITE_MAP_SHARED + (y - PICO8_MAP_BASE_ROWS) * PICO8_MAP_WIDTH + x;
}

void pico8_mset(pico8_mem_t *mem, int x, int y, int val) {
    int addr = pico8_map_cell_addr(x, y);
    if (addr < 0) {
        return;
    }
    pico8_poke(mem, addr, (unsigned char)val);
}

static unsigned char pico8_map_cell_get(const pico8_mem_t *mem, int x, int y) {
    int addr = pico8_map_cell_addr(x, y);
    if (addr < 0) {
        return 0;
    }
    return pico8_peek(mem, addr);
}

void pico8_map(const pico8_gfx_t *gfx, pico8_mem_t *mem, int cel_x, int cel_y, int sx, int sy, int cel_w, int cel_h) {
    for (int r = 0; r < cel_h; r++) {
        for (int c = 0; c < cel_w; c++) {
            unsigned char sprite_n = pico8_map_cell_get(mem, cel_x + c, cel_y + r);
            pico8_spr(gfx, mem, sprite_n, sx + c * PICO8_SPRITE_SIZE, sy + r * PICO8_SPRITE_SIZE, 1, 1);
        }
    }
}

void pico8_rectfill(const pico8_gfx_t *gfx, pico8_mem_t *mem, int x1, int y1, int x2, int y2, int col) {
    int xmin = pico8_min(x1, x2);
    int xmax = pico8_max(x1, x2);
    int ymin = pico8_min(y1, y2);
    int ymax = pico8_max(y1, y2);

    for (int y = ymin; y <= ymax; y++) {
        for (int x = xmin; x <= xmax; x++) {
            if (!pico8_fillp_skips(gfx, x, y)) {
                pico8_pset(gfx, mem, x, y, col);
            }
        }
    }
}
