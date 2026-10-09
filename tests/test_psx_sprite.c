#include "platform/psx_sprite.h"

#include "p8/draw_cost.h"
#include "p8/gfx.h"
#include "p8/vram.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

enum { UNTOUCHED = 0xEE };

static p8_vram_t software;
static unsigned char painted[P8_SCREEN_HEIGHT][P8_SCREEN_WIDTH];

static void setup_sheet_and_palette(void) {
    p8_vram_init(&software);
    for (int y = 0; y < P8_SPRITE_SHEET_HEIGHT; y++) {
        for (int x = 0; x < P8_SPRITE_SHEET_WIDTH; x++) {
            software.sprite_sheet[y][x] = (unsigned char)((x * 7 + y * 3 + (x ^ y)) & 0x0F);
        }
    }
    software.draw_palette[5] = 0x80 | 5;
    software.draw_palette[9] = 0;
    software.draw_palette[3] = 12;
}

/* The GPU's SPRT: each texel through the CLUT rule, a transparent entry skipped. */
static void paint(const psx_sprite_t *sprite) {
    assert(sprite->w >= 1 && sprite->h >= 1);
    assert(sprite->x >= 0 && sprite->x + sprite->w <= P8_SCREEN_WIDTH);
    assert(sprite->y >= 0 && sprite->y + sprite->h <= P8_SCREEN_HEIGHT);
    assert(sprite->u >= 0 && sprite->u + sprite->w <= P8_SPRITE_SHEET_WIDTH);
    assert(sprite->v >= 0 && sprite->v + sprite->h <= P8_SPRITE_SHEET_HEIGHT);
    for (int j = 0; j < sprite->h; j++) {
        for (int i = 0; i < sprite->w; i++) {
            unsigned char entry = software.draw_palette[software.sprite_sheet[sprite->v + j][sprite->u + i]];
            if ((entry & P8_DRAW_PALETTE_TRANSPARENT_BIT) == 0) {
                painted[sprite->y + j][sprite->x + i] = (unsigned char)(entry & 0x0F);
            }
        }
    }
}

static int sprite_matches_software(const p8_graphics_t *gfx, int n, int x, int y, int w, int h) {
    memset(software.screen, UNTOUCHED, sizeof software.screen);
    memset(painted, UNTOUCHED, sizeof painted);
    p8_spr(gfx, &software, n, x, y, w, h);
    psx_sprite_t sprite;
    int placed = psx_sprite_place(gfx, n, x, y, w, h, &sprite);
    assert(placed == 0 || placed == 1);
    if (placed) {
        paint(&sprite);
    }
    assert(memcmp(painted, software.screen, sizeof painted) == 0);
    p8_draw_cost_window_t window;
    p8_draw_cost_frame_begin(&window);
    assert(!placed || p8_draw_cost_spr(&window, gfx, n, x, y, w, h).bytes > 0);
    return placed;
}

static void test_a_sprite_inside_the_screen_is_the_software_one(void) {
    setup_sheet_and_palette();
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    assert(sprite_matches_software(&gfx, 0, 10, 20, 1, 1) == 1);
    assert(sprite_matches_software(&gfx, 37, 60, 40, 2, 2) == 1);
    assert(sprite_matches_software(&gfx, 70, 3, 100, 1, 2) == 1);
    assert(sprite_matches_software(&gfx, 255, 152, 112, 1, 1) == 1);
}

static void test_a_sprite_across_an_edge_is_clipped_like_the_software_one(void) {
    setup_sheet_and_palette();
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    assert(sprite_matches_software(&gfx, 17, -5, -3, 2, 2) == 1);
    assert(sprite_matches_software(&gfx, 17, 155, 115, 2, 2) == 1);
    assert(sprite_matches_software(&gfx, 17, -15, 50, 2, 2) == 1);
    assert(sprite_matches_software(&gfx, 17, -16, 50, 2, 2) == 0);
    assert(sprite_matches_software(&gfx, 17, 160, 50, 1, 1) == 0);
    assert(sprite_matches_software(&gfx, 17, 50, 120, 1, 1) == 0);
}

static void test_the_camera_moves_the_sprite_not_its_texture(void) {
    setup_sheet_and_palette();
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    p8_camera(&gfx, 100, -40);
    assert(sprite_matches_software(&gfx, 33, 95, -42, 2, 2) == 1);
    assert(sprite_matches_software(&gfx, 33, 250, 70, 2, 2) == 1);
    assert(sprite_matches_software(&gfx, 33, 0, 0, 2, 2) == 0);
}

static void test_an_empty_size_draws_nothing(void) {
    setup_sheet_and_palette();
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    assert(sprite_matches_software(&gfx, 3, 10, 10, 0, 1) == 0);
    assert(sprite_matches_software(&gfx, 3, 10, 10, 1, -1) == 0);
}

static void test_random_sprites_and_cameras_match_the_software(void) {
    setup_sheet_and_palette();
    unsigned state = 12345u;
    int placed = 0;
    for (int i = 0; i < 300; i++) {
        state = state * 1103515245u + 12345u;
        int w = 1 + (int)(state >> 16) % 2;
        int h = 1 + (int)(state >> 20) % 2;
        int column = (int)(state >> 8) % (17 - w);
        int row = (int)(state >> 12) % (17 - h);
        state = state * 1103515245u + 12345u;
        p8_graphics_t gfx;
        p8_gfx_init(&gfx);
        p8_camera(&gfx, (int)(state >> 4) % 61 - 30, (int)(state >> 12) % 61 - 30);
        int x = (int)(state >> 18) % 220 - 30;
        state = state * 1103515245u + 12345u;
        int y = (int)(state >> 16) % 180 - 30;
        placed += sprite_matches_software(&gfx, row * 16 + column, x, y, w, h);
    }
    assert(placed > 100 && placed < 300);
}

int main(void) {
    test_a_sprite_inside_the_screen_is_the_software_one();
    test_a_sprite_across_an_edge_is_clipped_like_the_software_one();
    test_the_camera_moves_the_sprite_not_its_texture();
    test_an_empty_size_draws_nothing();
    test_random_sprites_and_cameras_match_the_software();
    printf("test_psx_sprite: ok\n");
    return 0;
}
