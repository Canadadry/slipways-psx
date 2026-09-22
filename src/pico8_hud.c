#include "pico8_hud.h"

#include <assert.h>
#include <string.h>

/* `er`: the 8 outline offsets in black, then the center in color 15.
 * Constant data with no table bookkeeping to preserve, so a plain C array
 * rather than a pico8_ob() spec. */
typedef struct {
    int dx;
    int dy;
    int c;
} pico8_io_offset_t;

static const pico8_io_offset_t g_er[9] = {
    {-1, -1, 0}, {0, -1, 0}, {1, -1, 0}, {-1, 0, 0}, {1, 0, 0}, {-1, 1, 0}, {0, 1, 0}, {1, 1, 0}, {0, 0, 15},
};

static int pico8_hud_to_int(pico8_fixed_t x) {
    return (int)pico8_fixed_to_double(x);
}

/* `c==3 and 0x80` is Lua's false, not a number, for every other color --
 * see the header on that coercion. */
void pico8_cu(pico8_mem_t *mem) {
    int a = 0x5000;
    int p, c;

    for (p = 0; p <= 15; p++) {
        for (c = 0; c <= 15; c++) {
            unsigned char sg = pico8_sget(mem, p, c);
            unsigned char val = (unsigned char)(sg | (c == 3 ? 0x80 : 0));
            pico8_poke(mem, a, val);
            a += 1;
        }
    }
}

void pico8_dv(pico8_mem_t *mem, pico8_fixed_t no) {
    pico8_fixed_t offset = pico8_fixed_shl(pico8_fixed_flr(no), 4);
    int src = 0x5000 + pico8_hud_to_int(offset);
    pico8_memcpy(mem, 0x5f00, src, 16);
}

void pico8_dv_default(pico8_mem_t *mem) {
    pico8_dv(mem, pico8_fixed_from_int(0));
}

void pico8_rk(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_fixed_t y1, pico8_fixed_t y2, int col) {
    assert(gfx != NULL && "rk: pico8_rectfill dereferences gfx, so a NULL one is not a no-op");
    if (gfx != NULL) {
        gfx->pen_color = col;
    }
    pico8_rectfill(gfx, mem, 0, pico8_hud_to_int(y1), 127, pico8_hud_to_int(y2), col);
}

void pico8_rk_y1(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_fixed_t y1, int col) {
    pico8_rk(gfx, mem, y1, y1, col);
}

/* 6 is PICO-8's pen color at power-on, before any explicit col; the cart
 * always sets one first. */
void pico8_rk_default(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_fixed_t y1, pico8_fixed_t y2) {
    int col = (gfx != NULL) ? gfx->pen_color : 6;
    pico8_rk(gfx, mem, y1, y2, col);
}

void pico8_eq(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_fixed_t x1, pico8_fixed_t y1, pico8_fixed_t x2) {
    int ix1 = pico8_hud_to_int(x1);
    int iy1 = pico8_hud_to_int(y1);
    int ix2 = pico8_hud_to_int(x2);

    pico8_rect(gfx, mem, ix1, iy1, ix2 + 1, iy1 + 12, 0);
    pico8_rectfill(gfx, mem, ix1 + 1, iy1 + 1, ix2, iy1 + 11, 5);
    pico8_spr(gfx, mem, 70, ix1, iy1, 1, 2);
    pico8_spr(gfx, mem, 71, ix2 - 2, iy1, 1, 2);
}

void pico8_io(const pico8_gfx_t *gfx, pico8_mem_t *mem, const char *t, pico8_fixed_t x, pico8_fixed_t y, int c,
              const pico8_fixed_t *a) {
    int ix = pico8_hud_to_int(x);
    int iy = pico8_hud_to_int(y);
    int i;

    if (a != NULL) {
        pico8_fixed_t shift =
            pico8_fixed_mul(pico8_fixed_mul(*a, pico8_fixed_from_int(4)), pico8_fixed_from_int((int)strlen(t)));
        ix -= pico8_hud_to_int(shift);
    }

    for (i = 0; i < 9; i++) {
        pico8_print(gfx, mem, t, ix + g_er[i].dx, iy + g_er[i].dy, g_er[i].c & c);
    }
}

void pico8_io_default(const pico8_gfx_t *gfx, pico8_mem_t *mem, const char *t, pico8_fixed_t x, pico8_fixed_t y,
                       int c) {
    pico8_io(gfx, mem, t, x, y, c, NULL);
}
