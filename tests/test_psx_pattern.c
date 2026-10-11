#include "platform/psx_pattern.h"

#include "p8/draw_cost.h"
#include "p8/gfx.h"
#include "p8/gpu_sizes.h"
#include "p8/vram.h"
#include "platform/psx_colour.h"
#include "platform/psx_gpu_textures.h"
#include "platform/psx_tiles.h"
#include "platform/psx_vram_layout.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum {
    UNTOUCHED = 0xEE,
    PAGE_TEXELS = 256,
    DISPLAY_WIDTH = P8_SCREEN_WIDTH * PSX_TILES_SCALE,
    DISPLAY_HEIGHT = P8_SCREEN_HEIGHT * PSX_TILES_SCALE,
    WINDOW_PAIR_BYTES = P8_GPU_DR_TPAGE_BYTES + P8_GPU_DR_TWIN_BYTES,
};

static p8_vram_t software;
static unsigned char painted[DISPLAY_HEIGHT][DISPLAY_WIDTH];
static unsigned char page[PAGE_TEXELS][PAGE_TEXELS];
static uint16_t clut[PSX_GPU_TEXTURES_CLUT_ENTRIES];
static int clut_index[PSX_GPU_TEXTURES_CLUT_ENTRIES];

static int index_of(uint16_t colour) {
    for (int i = 0; i < 16; i++) {
        if (psx_colour_rgb555(i) == colour) {
            return i;
        }
    }
    assert(0 && "a CLUT entry that is no palette colour");
    return -1;
}

/* The slot's x2 tile and CLUT as the GPU holds them, the rest of the page a texel the tile never has. */
static void store_tile(int slot, const uint16_t *tile) {
    psx_vram_rect_t origin = psx_vram_layout_texture_page(PSX_VRAM_PATTERNS);
    psx_vram_rect_t texture = psx_vram_layout_pattern_slot(slot);
    for (int v = 0; v < PSX_GPU_TEXTURES_PATTERN_X2_SIZE; v++) {
        for (int u = 0; u < PSX_GPU_TEXTURES_PATTERN_X2_SIZE; u++) {
            int unit = tile == NULL ? 0xFFFF : tile[v * (PSX_GPU_TEXTURES_PATTERN_X2_SIZE / 4) + u / 4];
            page[texture.y - origin.y + v][(texture.x - origin.x) * 4 + u] = (unsigned char)((unit >> (4 * (u % 4))) & 0x0F);
        }
    }
}

static void load_slot(int slot, const p8_graphics_t *gfx, int col) {
    static int loaded = -1;
    if (loaded < 0) {
        memset(page, 0x0F, sizeof page);
    } else {
        store_tile(loaded, NULL);
    }
    uint16_t tile[PSX_GPU_TEXTURES_PATTERN_X2_UNITS];
    psx_gpu_textures_pattern_x2(gfx->fillp_pattern, tile);
    store_tile(slot, tile);
    loaded = slot;
    psx_gpu_textures_pattern_clut(col, gfx->fillp_transparent, clut);
    for (int i = 0; i < PSX_GPU_TEXTURES_CLUT_ENTRIES; i++) {
        clut_index[i] = clut[i] == 0x0000 ? -1 : index_of(clut[i]);
    }
}

static int windowed(int texcoord, int mask, int offset) {
    return (texcoord & ~(mask * 8)) | ((offset & mask) * 8);
}

/* The GPU's SPRT under the texture window: each texel through the CLUT, 0x0000 skipped. */
static void paint(psx_sprite_t sprite, psx_pattern_window_t window) {
    assert(sprite.w >= 1 && sprite.h >= 1);
    assert(sprite.x >= 0 && sprite.x + sprite.w <= DISPLAY_WIDTH);
    assert(sprite.y >= 0 && sprite.y + sprite.h <= DISPLAY_HEIGHT);
    assert(sprite.u >= 0 && sprite.u + sprite.w <= PAGE_TEXELS && sprite.v >= 0 && sprite.v + sprite.h <= PAGE_TEXELS
           && "no SPRT relies on the 8-bit UV wrap");
    for (int j = 0; j < sprite.h; j++) {
        for (int i = 0; i < sprite.w; i++) {
            int u = windowed(sprite.u + i, window.mask_x, window.offset_x);
            int v = windowed(sprite.v + j, window.mask_y, window.offset_y);
            int index = clut_index[page[v][u]];
            if (index >= 0) {
                painted[sprite.y + j][sprite.x + i] = (unsigned char)index;
            }
        }
    }
}

/* Returns how many SPRTs the span sent. */
static int paint_span(const p8_graphics_t *gfx, psx_tile_t span, int slot) {
    psx_sprite_t pieces[PSX_PATTERN_PIECES_MAX];
    int count = psx_pattern_sprites_x2(gfx, span, pieces);
    assert(count >= 1 && count <= PSX_PATTERN_PIECES_MAX);
    for (int p = 0; p < count; p++) {
        paint(pieces[p], psx_pattern_window(slot));
    }
    return count;
}

static void begin(void) {
    p8_vram_init(&software);
    memset(software.screen, UNTOUCHED, sizeof software.screen);
    memset(painted, UNTOUCHED, sizeof painted);
}

static void assert_painted_is_the_software_screen_x2(void) {
    for (int y = 0; y < DISPLAY_HEIGHT; y++) {
        for (int x = 0; x < DISPLAY_WIDTH; x++) {
            assert(painted[y][x] == software.screen[y / PSX_TILES_SCALE][x / PSX_TILES_SCALE]);
        }
    }
}

static int rectfill_matches_software(const p8_graphics_t *gfx, int slot, int x1, int y1, int x2, int y2, int col) {
    begin();
    load_slot(slot, gfx, col);
    p8_rectfill(gfx, &software, x1, y1, x2, y2, col);
    psx_tile_t tile;
    int sent = psx_tiles_rectfill(gfx, x1, y1, x2, y2, &tile);
    int sprts = sent ? paint_span(gfx, tile, slot) : 0;
    assert_painted_is_the_software_screen_x2();
    p8_draw_cost_window_t window;
    p8_draw_cost_frame_begin(&window);
    p8_draw_cost_t cost = p8_draw_cost_rectfill(&window, gfx, x1, y1, x2, y2, col);
    assert(cost.bytes == (sent ? (unsigned long)sprts * P8_GPU_SPRT_BYTES + WINDOW_PAIR_BYTES : 0)
           && "the meter counts a box's pieces exactly");
    return sprts;
}

static int circfill_matches_software(const p8_graphics_t *gfx, int slot, int cx, int cy, int r, int col) {
    begin();
    load_slot(slot, gfx, col);
    p8_circfill(gfx, &software, cx, cy, r, col);
    psx_tile_t spans[PSX_TILES_CIRCFILL_MAX];
    int count = psx_tiles_circfill(gfx, cx, cy, r, spans);
    int sprts = 0;
    for (int s = 0; s < count; s++) {
        sprts += paint_span(gfx, spans[s], slot);
    }
    assert_painted_is_the_software_screen_x2();
    p8_draw_cost_window_t window;
    p8_draw_cost_frame_begin(&window);
    assert((unsigned long)sprts * P8_GPU_SPRT_BYTES + WINDOW_PAIR_BYTES <= p8_draw_cost_circfill(&window, gfx, cx, cy, r, col).bytes
           || count == 0);
    return count;
}

static void test_every_slot_window_repeats_16_texels_from_even_offsets(void) {
    for (int slot = 0; slot < GPU_PATTERN_SLOTS; slot++) {
        psx_pattern_window_t window = psx_pattern_window(slot);
        assert(window.mask_x == 0x1E && window.mask_y == 0x1E);
        assert(window.offset_x % 2 == 0 && window.offset_y % 2 == 0);
    }
}

static void test_a_patterned_box_is_the_software_one_x2_in_each_slot(void) {
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    p8_fillp(&gfx, 0xA5A5);
    for (int slot = 0; slot < GPU_PATTERN_SLOTS; slot++) {
        assert(rectfill_matches_software(&gfx, slot, 10, 20, 40, 30, 0x12) == 1);
        assert(rectfill_matches_software(&gfx, slot, 13, 21, 13, 21, 0x7C) == 1);
    }
    p8_fillp(&gfx, 0x0F0F);
    assert(rectfill_matches_software(&gfx, 2, 0, 111, 159, 119, 0x05) == 2 && "the menu bar");
    assert(rectfill_matches_software(&gfx, 1, 100, 50, 3, 7, 0x50) == 1 && "corners in any order");
}

static void test_a_patterned_box_keeps_its_world_phase_under_any_camera(void) {
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    p8_fillp(&gfx, 0x8421);
    const int cameras[][2] = { { 1, 0 }, { 0, 3 }, { -5, -7 }, { 13, 250 }, { -301, 99 } };
    for (unsigned c = 0; c < sizeof cameras / sizeof cameras[0]; c++) {
        p8_camera(&gfx, cameras[c][0], cameras[c][1]);
        int x = cameras[c][0];
        int y = cameras[c][1];
        assert(rectfill_matches_software(&gfx, 3, x + 9, y + 2, x + 70, y + 41, 0x9A) == 1);
        assert(rectfill_matches_software(&gfx, 0, x - 20, y - 9, x + 6, y + 4, 0x31) == 1 && "clipped at the top left");
        assert(rectfill_matches_software(&gfx, 1, x + 150, y + 100, x + 200, y + 300, 0x31) == 1);
        assert(rectfill_matches_software(&gfx, 1, x + 160, y, x + 200, y + 30, 0x31) == 0);
    }
}

static void test_a_full_width_span_is_cut_under_u_256_at_every_phase(void) {
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    p8_fillp(&gfx, 0x1E87);
    for (int phase = 0; phase < 8; phase++) {
        p8_camera(&gfx, phase - 64, 3 * phase);
        int slot = phase % GPU_PATTERN_SLOTS;
        int x = gfx.camera_x;
        int y = gfx.camera_y;
        assert(rectfill_matches_software(&gfx, slot, x, y + 10, x + 159, y + 10, 0x4B) == 2);
        assert(rectfill_matches_software(&gfx, slot, x - 30, y, x + 200, y + 119, 0x4B) == 2);
        assert(rectfill_matches_software(&gfx, slot, x, y + 50, x + 127 - phase, y + 60, 0x4B) == 1);
        assert(rectfill_matches_software(&gfx, slot, x, y + 50, x + 128 - phase, y + 60, 0x4B) == 2);
    }
}

static void test_a_transparent_pattern_leaves_its_set_bits_untouched(void) {
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    p8_fillp_transparent(&gfx, 0xAAAA);
    assert(rectfill_matches_software(&gfx, 0, 4, 4, 30, 30, 0x0C) == 1);
    p8_camera(&gfx, -3, 5);
    assert(circfill_matches_software(&gfx, 2, 40, 40, 9, 0x07) == 19);
    p8_fillp_transparent(&gfx, 0x5A5A);
    assert(rectfill_matches_software(&gfx, 3, 4, 4, 30, 30, 0xF0) == 1 && "black stays drawn");
}

static void test_a_patterned_disc_is_the_software_one_x2(void) {
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    p8_fillp(&gfx, 0xA5A5);
    assert(circfill_matches_software(&gfx, 0, 80, 60, 0, 0x21) == 1);
    assert(circfill_matches_software(&gfx, 1, 80, 60, 9, 0x21) == 19);
    assert(circfill_matches_software(&gfx, 3, 80, 60, 61, 0x10) == P8_SCREEN_HEIGHT);
    assert(circfill_matches_software(&gfx, 3, 83, 60, 100, 0x10) == P8_SCREEN_HEIGHT);
    p8_camera(&gfx, 37, -211);
    assert(circfill_matches_software(&gfx, 2, 40, -200, 20, 0xE3) > 0);
    assert(circfill_matches_software(&gfx, 2, 300, -200, 20, 0xE3) == 0);
}

static void test_random_patterned_fills_are_the_software_ones_x2(void) {
    unsigned seed = 4242;
    p8_graphics_t gfx;
    for (int i = 0; i < 80; i++) {
        int values[9];
        for (int v = 0; v < 9; v++) {
            seed = seed * 1103515245u + 12345u;
            values[v] = (int)((seed >> 8) % 400) - 120;
        }
        p8_gfx_init(&gfx);
        unsigned int pattern = (unsigned)(values[6] * 7919 + values[7]) & 0xFFFFu;
        if (values[8] & 1) {
            p8_fillp_transparent(&gfx, pattern == 0 ? 1 : pattern);
        } else {
            p8_fillp(&gfx, pattern == 0 ? 1 : pattern);
        }
        p8_camera(&gfx, values[4] / 2, values[5] / 2);
        int col = (values[6] + 1000) & 0xFF;
        int slot = (values[7] + 1000) % GPU_PATTERN_SLOTS;
        rectfill_matches_software(&gfx, slot, values[0], values[1], values[2], values[3], col);
        circfill_matches_software(&gfx, slot, values[0], values[1], (values[2] + 120) / 4, col);
    }
}

int main(void) {
    test_every_slot_window_repeats_16_texels_from_even_offsets();
    test_a_patterned_box_is_the_software_one_x2_in_each_slot();
    test_a_patterned_box_keeps_its_world_phase_under_any_camera();
    test_a_full_width_span_is_cut_under_u_256_at_every_phase();
    test_a_transparent_pattern_leaves_its_set_bits_untouched();
    test_a_patterned_disc_is_the_software_one_x2();
    test_random_patterned_fills_are_the_software_ones_x2();
    printf("test_psx_pattern: ok\n");
    return 0;
}
