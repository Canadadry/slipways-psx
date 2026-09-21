#include "pico8_hud.h"

#include <string.h>

/* er=ob([[
 * o(x=-1,y=-1,c=0),
 * o(x=0,y=-1,c=0),
 * o(x=1,y=-1,c=0),
 * o(x=-1,y=0,c=0),
 * o(x=1,y=0,c=0),
 * o(x=-1,y=1,c=0),
 * o(x=0,y=1,c=0),
 * o(x=1,y=1,c=0),
 * o(x=0,y=0,c=15),
 * ]])
 * See pico8_hud.h's own comment on pico8_io for why this is a plain C
 * array rather than an actual pico8_ob() call. */
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

/* function cu()local a=0x5000
 * for p=0,15 do
 * for c=0,15 do
 * poke(a,bor(sget(p,c),c==3 and 0x80))
 * a+=1
 * end
 * end
 * end
 * See pico8_hud.h's own top comment for the `c==3 and 0x80`-is-`false`-
 * when-false-coerces-to-0 assumption this line relies on. */
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

/* function dv(no)memcpy(0x5f00,0x5000+shl(flr(no),4),16)end
 * See pico8_hud.h's own comment on this declaration. */
void pico8_dv(pico8_mem_t *mem, pico8_fixed_t no) {
    pico8_fixed_t offset = pico8_fixed_shl(pico8_fixed_flr(no), 4);
    int src = 0x5000 + pico8_hud_to_int(offset);
    pico8_memcpy(mem, 0x5f00, src, 16);
}

void pico8_dv_default(pico8_mem_t *mem) {
    pico8_dv(mem, pico8_fixed_from_int(0));
}

/* function rk(y1,y2,...)rectfill(0,y1,127,y2 or y1,...)end
 * See pico8_hud.h's own comment on this declaration, including the
 * Chunk 18 `pen_color`-recording resolution. */
void pico8_rk(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_fixed_t y1, pico8_fixed_t y2, int col) {
    if (gfx != NULL) {
        gfx->pen_color = col;
    }
    pico8_rectfill(gfx, mem, 0, pico8_hud_to_int(y1), 127, pico8_hud_to_int(y2), col);
}

void pico8_rk_y1(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_fixed_t y1, int col) {
    pico8_rk(gfx, mem, y1, y1, col);
}

void pico8_rk_default(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_fixed_t y1, pico8_fixed_t y2) {
    int col = (gfx != NULL) ? gfx->pen_color : 6;
    pico8_rk(gfx, mem, y1, y2, col);
}

/* function eq(x1,y1,x2)rect(x1,y1,x2+1,y1+12,0)rectfill(x1+1,y1+1,x2,y1+11,5)spr(70,x1,y1,1,2)spr(71,x2-2,y1,1,2)end
 * See pico8_hud.h's own comment on this declaration. */
void pico8_eq(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_fixed_t x1, pico8_fixed_t y1, pico8_fixed_t x2) {
    int ix1 = pico8_hud_to_int(x1);
    int iy1 = pico8_hud_to_int(y1);
    int ix2 = pico8_hud_to_int(x2);

    pico8_rect(gfx, mem, ix1, iy1, ix2 + 1, iy1 + 12, 0);
    pico8_rectfill(gfx, mem, ix1 + 1, iy1 + 1, ix2, iy1 + 11, 5);
    pico8_spr(gfx, mem, 70, ix1, iy1, 1, 2);
    pico8_spr(gfx, mem, 71, ix2 - 2, iy1, 1, 2);
}

/* function io(t,x,y,c,a)
 * if (a)x-=a*4*#t
 * for _,d in pairs(er)do
 * print(t,x+d.x,y+d.y,band(d.c,c))end
 * end
 * See pico8_hud.h's own comment on this declaration. */
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
