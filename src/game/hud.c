#include "game/hud.h"

#include <assert.h>
#include <string.h>

/* `er`: the 8 outline offsets in black, then the center in color 15. */
typedef struct {
    int dx;
    int dy;
    int c;
} pico8_io_offset_t;

static const pico8_io_offset_t g_er[9] = {
    {-1, -1, 0}, {0, -1, 0}, {1, -1, 0}, {-1, 0, 0}, {1, 0, 0}, {-1, 1, 0}, {0, 1, 0}, {1, 1, 0}, {0, 0, 15},
};

static int pico8_hud_to_int(p8_fixed_t x) {
    return p8_fixed_flr_int(x);
}

/* `c==3 and 0x80` is Lua's false, not a number, for every other color --
 * see the header on that coercion. */
void pico8_cu(p8_memory_t *mem) {
    int a = 0x5000;
    int p, c;

    for (p = 0; p <= 15; p++) {
        for (c = 0; c <= 15; c++) {
            unsigned char sg = p8_sget(mem, p, c);
            unsigned char val = (unsigned char)(sg | (c == 3 ? 0x80 : 0));
            p8_poke(mem, a, val);
            a += 1;
        }
    }
}

void pico8_dv(p8_memory_t *mem, p8_fixed_t no) {
    p8_fixed_t offset = p8_fixed_shl(p8_fixed_flr(no), 4);
    int src = 0x5000 + pico8_hud_to_int(offset);
    p8_memcpy(mem, 0x5f00, src, 16);
}

void pico8_dv_default(p8_memory_t *mem) {
    pico8_dv(mem, p8_fixed_from_int(0));
}

void pico8_rk(p8_graphics_t *gfx, p8_memory_t *mem, p8_fixed_t y1, p8_fixed_t y2, int col) {
    assert(gfx != NULL && "rk: pico8_rectfill dereferences gfx, so a NULL one is not a no-op");
    if (gfx != NULL) {
        gfx->pen_color = col;
    }
    p8_rectfill(gfx, mem, 0, pico8_hud_to_int(y1), 127, pico8_hud_to_int(y2), col);
}

void pico8_rk_y1(p8_graphics_t *gfx, p8_memory_t *mem, p8_fixed_t y1, int col) {
    pico8_rk(gfx, mem, y1, y1, col);
}

/* 6 is PICO-8's pen color at power-on, before any explicit col; the cart
 * always sets one first. */
void pico8_rk_default(p8_graphics_t *gfx, p8_memory_t *mem, p8_fixed_t y1, p8_fixed_t y2) {
    int col = (gfx != NULL) ? gfx->pen_color : 6;
    pico8_rk(gfx, mem, y1, y2, col);
}

void pico8_eq(const p8_graphics_t *gfx, p8_memory_t *mem, p8_fixed_t x1, p8_fixed_t y1, p8_fixed_t x2) {
    int ix1 = pico8_hud_to_int(x1);
    int iy1 = pico8_hud_to_int(y1);
    int ix2 = pico8_hud_to_int(x2);

    p8_rect(gfx, mem, ix1, iy1, ix2 + 1, iy1 + 12, 0);
    p8_rectfill(gfx, mem, ix1 + 1, iy1 + 1, ix2, iy1 + 11, 5);
    p8_spr(gfx, mem, 70, ix1, iy1, 1, 2);
    p8_spr(gfx, mem, 71, ix2 - 2, iy1, 1, 2);
}

void pico8_io(const p8_graphics_t *gfx, p8_memory_t *mem, const char *t, p8_fixed_t x, p8_fixed_t y, int c,
              const p8_fixed_t *a) {
    int ix = pico8_hud_to_int(x);
    int iy = pico8_hud_to_int(y);
    int i;

    if (a != NULL) {
        p8_fixed_t shift =
            p8_fixed_mul(p8_fixed_mul(*a, p8_fixed_from_int(4)), p8_fixed_from_int((int)strlen(t)));
        ix -= pico8_hud_to_int(shift);
    }

    for (i = 0; i < 9; i++) {
        p8_print(gfx, mem, t, ix + g_er[i].dx, iy + g_er[i].dy, g_er[i].c & c);
    }
}

void pico8_io_default(const p8_graphics_t *gfx, p8_memory_t *mem, const char *t, p8_fixed_t x, p8_fixed_t y,
                       int c) {
    pico8_io(gfx, mem, t, x, y, c, NULL);
}
