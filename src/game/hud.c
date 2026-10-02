#include "game/hud.h"

#include <assert.h>
#include <string.h>

/* `er`: the 8 outline offsets in black, then the center in color 15. */
typedef struct {
    int offset_x;
    int offset_y;
    int color;
} print_outlined_offset_t;

static const print_outlined_offset_t g_outline_offsets[9] = {
    {-1, -1, 0}, {0, -1, 0}, {1, -1, 0}, {-1, 0, 0}, {1, 0, 0}, {-1, 1, 0}, {0, 1, 0}, {1, 1, 0}, {0, 0, 15},
};

static int to_int(p8_fixed_t x) {
    return p8_fixed_flr_int(x);
}

/* `c==3 and 0x80` is Lua's false, not a number, for every other color --
 * see the header on that coercion. */
void hud_build_palette_table(p8_memory_t *mem) {
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

void hud_select_palette_block(p8_memory_t *mem, p8_fixed_t no) {
    p8_fixed_t offset = p8_fixed_shl(p8_fixed_flr(no), 4);
    int src = 0x5000 + to_int(offset);
    p8_memcpy(mem, 0x5f00, src, 16);
}

void hud_select_palette_block_default(p8_memory_t *mem) {
    hud_select_palette_block(mem, p8_fixed_from_int(0));
}

void hud_draw_bar(p8_graphics_t *gfx, p8_memory_t *mem, p8_fixed_t y1, p8_fixed_t y2, int col) {
    assert(gfx != NULL && "rk: pico8_rectfill dereferences gfx, so a NULL one is not a no-op");
    if (gfx != NULL) {
        gfx->pen_color = col;
    }
    p8_rectfill(gfx, mem, 0, to_int(y1), 127, to_int(y2), col);
}

void hud_draw_bar_y1(p8_graphics_t *gfx, p8_memory_t *mem, p8_fixed_t y1, int col) {
    hud_draw_bar(gfx, mem, y1, y1, col);
}

/* 6 is PICO-8's pen color at power-on, before any explicit col; the cart
 * always sets one first. */
void hud_draw_bar_default(p8_graphics_t *gfx, p8_memory_t *mem, p8_fixed_t y1, p8_fixed_t y2) {
    int col = (gfx != NULL) ? gfx->pen_color : 6;
    hud_draw_bar(gfx, mem, y1, y2, col);
}

void hud_draw_help_frame(const p8_graphics_t *gfx, p8_memory_t *mem, p8_fixed_t x1, p8_fixed_t y1, p8_fixed_t x2) {
    int galaxy_menu_button1 = to_int(x1);
    int top_pixel = to_int(y1);
    int galaxy_menu_button2 = to_int(x2);

    p8_rect(gfx, mem, galaxy_menu_button1, top_pixel, galaxy_menu_button2 + 1, top_pixel + 12, 0);
    p8_rectfill(gfx, mem, galaxy_menu_button1 + 1, top_pixel + 1, galaxy_menu_button2, top_pixel + 11, 5);
    p8_spr(gfx, mem, 70, galaxy_menu_button1, top_pixel, 1, 2);
    p8_spr(gfx, mem, 71, galaxy_menu_button2 - 2, top_pixel, 1, 2);
}

void hud_print_outlined(const p8_graphics_t *gfx, p8_memory_t *mem, const char *t, p8_fixed_t x, p8_fixed_t y, int c,
              const p8_fixed_t *a) {
    int ix = to_int(x);
    int iy = to_int(y);
    int i;

    if (a != NULL) {
        p8_fixed_t shift =
            p8_fixed_mul(p8_fixed_mul(*a, p8_fixed_from_int(4)), p8_fixed_from_int((int)strlen(t)));
        ix -= to_int(shift);
    }

    for (i = 0; i < 9; i++) {
        p8_print(gfx, mem, t, ix + g_outline_offsets[i].offset_x, iy + g_outline_offsets[i].offset_y, g_outline_offsets[i].color & c);
    }
}

void hud_print_outlined_default(const p8_graphics_t *gfx, p8_memory_t *mem, const char *t, p8_fixed_t x, p8_fixed_t y,
                       int c) {
    hud_print_outlined(gfx, mem, t, x, y, c, NULL);
}
