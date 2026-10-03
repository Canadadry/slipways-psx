#include "game/hud.h"

#include <assert.h>
#include <string.h>

/* The 8 outline offsets in black, then the center in color 15. */
typedef struct {
    int offset_x;
    int offset_y;
    int color;
} print_outlined_offset_t;

static const print_outlined_offset_t outline_offsets[9] = {
    {-1, -1, 0}, {0, -1, 0}, {1, -1, 0}, {-1, 0, 0}, {1, 0, 0}, {-1, 1, 0}, {0, 1, 0}, {1, 1, 0}, {0, 0, 15},
};

static int to_int(p8_fixed_t x) {
    return p8_fixed_flr_int(x);
}

/* Row = block, column = source color; color 3 carries the transparency
 * bit in every block. tests/test_hud.c checks it against the cart's
 * sprite-sheet pixels (block n, color c at pixel (n, c)). */
static const unsigned char palette_blocks[16][16] = {
    {0, 1, 2, 131, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
    {0, 1, 2, 131, 2, 5, 6, 7, 8, 4, 9, 11, 12, 5, 14, 9},
    {0, 1, 1, 131, 2, 1, 13, 6, 2, 2, 4, 3, 13, 5, 8, 4},
    {0, 0, 1, 129, 1, 1, 5, 13, 2, 2, 2, 3, 5, 1, 2, 2},
    {0, 0, 0, 129, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {0, 0, 0, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 2, 2, 130, 2, 2, 8, 8, 2, 2, 8, 8, 2, 2, 8, 8},
    {0, 1, 1, 129, 1, 1, 5, 13, 1, 1, 5, 5, 1, 1, 5, 5},
    {0, 1, 2, 131, 4, 5, 6, 7, 10, 9, 0, 4, 2, 13, 0, 0},
    {0, 1, 2, 131, 4, 5, 6, 7, 0, 10, 9, 0, 4, 13, 4, 0},
    {0, 1, 2, 131, 4, 5, 6, 7, 0, 0, 10, 4, 0, 13, 9, 4},
    {0, 1, 2, 131, 4, 5, 6, 7, 4, 0, 0, 9, 4, 13, 0, 9},
    {0, 1, 2, 131, 4, 5, 6, 7, 9, 4, 0, 0, 9, 13, 9, 0},
    {0, 1, 2, 131, 4, 5, 6, 7, 0, 9, 4, 0, 0, 13, 10, 9},
    {0, 1, 2, 131, 4, 5, 6, 7, 9, 0, 9, 2, 0, 13, 0, 10},
    {0, 1, 2, 131, 4, 5, 6, 6, 0, 0, 0, 0, 0, 13, 0, 0},
};

void hud_select_palette_block(p8_memory_t *mem, p8_fixed_t no) {
    int block = p8_fixed_flr_int(no);
    assert(block >= 0 && block < 16 && "hud_select_palette_block: only blocks 0..15 exist; the cart would read past its table");
    for (int c = 0; c < 16; c++) {
        p8_poke(mem, P8_ADDR_DRAW_PALETTE + c, palette_blocks[block][c]);
    }
}

void hud_select_palette_block_default(p8_memory_t *mem) {
    hud_select_palette_block(mem, p8_fixed_from_int(0));
}

void hud_draw_bar(p8_graphics_t *gfx, p8_memory_t *mem, p8_fixed_t y1, p8_fixed_t y2, int col) {
    assert(gfx != NULL && "hud_draw_bar: p8_rectfill dereferences the graphics state, so a NULL one is not a no-op");
    if (gfx != NULL) {
        gfx->pen_color = col;
    }
    p8_rectfill(gfx, mem, 0, to_int(y1), 127, to_int(y2), col);
}

void hud_draw_bar_y1(p8_graphics_t *gfx, p8_memory_t *mem, p8_fixed_t y1, int col) {
    hud_draw_bar(gfx, mem, y1, y1, col);
}

/* 6 is PICO-8's pen color at power-on, before any explicit color; its one
 * caller always draws a colored bar first. */
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
        p8_print(gfx, mem, t, ix + outline_offsets[i].offset_x, iy + outline_offsets[i].offset_y, outline_offsets[i].color & c);
    }
}

void hud_print_outlined_default(const p8_graphics_t *gfx, p8_memory_t *mem, const char *t, p8_fixed_t x, p8_fixed_t y,
                       int c) {
    hud_print_outlined(gfx, mem, t, x, y, c, NULL);
}
