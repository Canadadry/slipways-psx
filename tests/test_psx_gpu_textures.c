#include "platform/psx_gpu_textures.h"

#include "p8/gfx.h"
#include "p8/vram.h"
#include "platform/psx_colour.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void test_a_transparent_entry_is_zero_and_black_stays_opaque(void) {
    const unsigned char palette[PSX_GPU_TEXTURES_CLUT_ENTRIES] = { 0x80, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 0 };
    uint16_t clut[PSX_GPU_TEXTURES_CLUT_ENTRIES];
    psx_gpu_textures_clut(palette, clut);
    assert(clut[0] == 0x0000);
    assert(clut[15] == 0x8000 && "black remapped onto colour 15 is drawn, not skipped");
    for (int i = 1; i < 15; i++) {
        assert(clut[i] == psx_colour_rgb555(i));
    }
}

static void test_each_entry_follows_its_draw_palette_byte(void) {
    const unsigned char palette[PSX_GPU_TEXTURES_CLUT_ENTRIES] = { 0x00, 0x87, 0x3C, 7, 7, 0x8F, 1, 0, 0xFF, 9, 2, 2, 5, 0x40, 14, 3 };
    uint16_t clut[PSX_GPU_TEXTURES_CLUT_ENTRIES];
    psx_gpu_textures_clut(palette, clut);
    for (int i = 0; i < PSX_GPU_TEXTURES_CLUT_ENTRIES; i++) {
        assert(clut[i] == psx_colour_clut_entry(palette[i]));
    }
    assert(clut[2] == psx_colour_rgb555(12) && "remap uses the low nibble");
    assert(clut[1] == 0 && clut[5] == 0 && clut[8] == 0);
}

static uint16_t font[PSX_GPU_TEXTURES_FONT_UNITS];
static unsigned char expected_texels[PSX_GPU_TEXTURES_FONT_SIZE][PSX_GPU_TEXTURES_FONT_SIZE];

static int font_texel(int u, int v) {
    int unit = font[v * (PSX_GPU_TEXTURES_FONT_SIZE / 4) + u / 4];
    return (unit >> (4 * (u % 4))) & 0x0F;
}

static int glyph_lit_at(const unsigned char *rows, int column, int row) {
    if (column < 0 || column >= P8_FONT_GLYPH_WIDTH || row < 0 || row >= P8_FONT_GLYPH_HEIGHT) {
        return 0;
    }
    return (rows[row] >> (P8_FONT_GLYPH_WIDTH - 1 - column)) & 1;
}

/* The outlined cell, one texel wider on every side: 1 lit, 2 an unlit texel with a lit 8-neighbour. */
static void expect_outlined_cells(void) {
    for (int code = 0; code < 256; code++) {
        psx_gpu_textures_texel_t cell = psx_gpu_textures_outlined_font_cell((char)code);
        assert(cell.u >= 0 && cell.u + P8_FONT_GLYPH_WIDTH + 2 <= PSX_GPU_TEXTURES_FONT_SIZE);
        assert(cell.v >= 0 && cell.v + P8_FONT_GLYPH_HEIGHT + 2 <= PSX_GPU_TEXTURES_FONT_SIZE);
        const unsigned char *rows = p8_font_glyph_rows((char)code);
        for (int row = -1; row <= P8_FONT_GLYPH_HEIGHT; row++) {
            for (int column = -1; column <= P8_FONT_GLYPH_WIDTH; column++) {
                int texel = glyph_lit_at(rows, column, row);
                for (int dy = -1; dy <= 1 && texel == 0; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (glyph_lit_at(rows, column + dx, row + dy)) {
                            texel = 2;
                        }
                    }
                }
                unsigned char *expected = &expected_texels[cell.v + 1 + row][cell.u + 1 + column];
                assert(*expected == 0 && "two cells overlap");
                *expected = (unsigned char)texel;
            }
        }
    }
}

static void test_each_character_cell_holds_its_glyph_and_nothing_else_is_lit(void) {
    memset(font, 0xFF, sizeof font);
    psx_gpu_textures_font(font);
    memset(expected_texels, 0, sizeof expected_texels);
    for (int code = 0; code < 256; code++) {
        psx_gpu_textures_texel_t cell = psx_gpu_textures_font_cell((char)code);
        assert(cell.u >= 0 && cell.u + P8_FONT_CELL_WIDTH <= PSX_GPU_TEXTURES_FONT_SIZE);
        assert(cell.v >= 0 && cell.v + P8_FONT_GLYPH_HEIGHT + 1 <= PSX_GPU_TEXTURES_FONT_SIZE);
        const unsigned char *rows = p8_font_glyph_rows((char)code);
        for (int row = 0; row < P8_FONT_GLYPH_HEIGHT; row++) {
            for (int column = 0; column < P8_FONT_GLYPH_WIDTH; column++) {
                assert(expected_texels[cell.v + row][cell.u + column] == 0 && "two cells overlap");
                expected_texels[cell.v + row][cell.u + column] = (rows[row] >> (P8_FONT_GLYPH_WIDTH - 1 - column)) & 1;
            }
        }
    }
    expect_outlined_cells();
    for (int v = 0; v < PSX_GPU_TEXTURES_FONT_SIZE; v++) {
        for (int u = 0; u < PSX_GPU_TEXTURES_FONT_SIZE; u++) {
            assert(font_texel(u, v) == expected_texels[v][u]);
        }
    }
}

static void test_a_cell_is_found_by_its_character_code_signed_or_not(void) {
    psx_gpu_textures_texel_t a = psx_gpu_textures_font_cell('A');
    psx_gpu_textures_texel_t high = psx_gpu_textures_font_cell((char)0xE9);
    psx_gpu_textures_texel_t other = psx_gpu_textures_font_cell((char)0x69);
    assert(a.u != psx_gpu_textures_font_cell('B').u || a.v != psx_gpu_textures_font_cell('B').v);
    assert(high.u != other.u || high.v != other.v);
}

static void test_a_font_clut_lights_texel_one_in_its_colour_and_outlines_texel_two_in_black(void) {
    for (int colour = 0; colour < 16; colour++) {
        uint16_t clut[PSX_GPU_TEXTURES_CLUT_ENTRIES];
        memset(clut, 0xFF, sizeof clut);
        psx_gpu_textures_font_clut(colour, clut);
        assert(clut[0] == 0x0000 && "a clear texel is transparent");
        assert(clut[1] == psx_colour_rgb555(colour));
        assert(clut[2] == psx_colour_rgb555(0) && "an outline is the pixel a colour-0 print writes");
        for (int i = 3; i < PSX_GPU_TEXTURES_CLUT_ENTRIES; i++) {
            assert(clut[i] == 0x0000);
        }
    }
    uint16_t black[PSX_GPU_TEXTURES_CLUT_ENTRIES];
    psx_gpu_textures_font_clut(0, black);
    assert(black[1] == 0x8000 && "black text is drawn, not skipped");
}

static int pattern_texel(const uint16_t tile[PSX_GPU_TEXTURES_PATTERN_UNITS], int u, int v) {
    int unit = tile[v * (PSX_GPU_TEXTURES_PATTERN_SIZE / 4) + u / 4];
    return (unit >> (4 * (u % 4))) & 0x0F;
}

/* fillp's high-nibble colour marks a set bit: the software's own reading of the pattern. */
static int software_bit_set(unsigned int pattern, int world_x, int world_y) {
    static p8_vram_t software;
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    p8_fillp(&gfx, pattern);
    p8_camera(&gfx, world_x, world_y);
    p8_vram_init(&software);
    p8_rectfill(&gfx, &software, world_x, world_y, world_x, world_y, 0x21);
    assert(software.screen[0][0] == 1 || software.screen[0][0] == 2);
    return software.screen[0][0] == 2;
}

static void test_a_pattern_tile_holds_the_software_bit_at_each_world_phase(void) {
    const unsigned int patterns[] = { 0xA5A5, 0x0F0F, 0xAAAA, 0x8001 };
    for (unsigned p = 0; p < sizeof patterns / sizeof patterns[0]; p++) {
        uint16_t tile[PSX_GPU_TEXTURES_PATTERN_UNITS];
        memset(tile, 0xFF, sizeof tile);
        psx_gpu_textures_pattern(patterns[p], tile);
        for (int v = 0; v < PSX_GPU_TEXTURES_PATTERN_SIZE; v++) {
            for (int u = 0; u < PSX_GPU_TEXTURES_PATTERN_SIZE; u++) {
                int expected = software_bit_set(patterns[p], u, v);
                assert(pattern_texel(tile, u, v) == expected);
                assert(software_bit_set(patterns[p], u - 40, v + 16) == expected && "world phase repeats every 8");
            }
        }
    }
}

static void test_a_pattern_clut_maps_clear_bits_to_the_low_nibble_and_set_ones_to_the_high(void) {
    uint16_t clut[PSX_GPU_TEXTURES_CLUT_ENTRIES];
    memset(clut, 0xFF, sizeof clut);
    psx_gpu_textures_pattern_clut(0x5C, 0, clut);
    assert(clut[0] == psx_colour_rgb555(12));
    assert(clut[1] == psx_colour_rgb555(5));
    for (int i = 2; i < PSX_GPU_TEXTURES_CLUT_ENTRIES; i++) {
        assert(clut[i] == 0x0000);
    }
    psx_gpu_textures_pattern_clut(0x10, 0, clut);
    assert(clut[0] == 0x8000 && "black is drawn, not skipped");
    psx_gpu_textures_pattern_clut(0x07, 0, clut);
    assert(clut[1] == 0x8000);
}

static void test_a_transparent_pattern_clut_skips_set_bits(void) {
    uint16_t clut[PSX_GPU_TEXTURES_CLUT_ENTRIES];
    psx_gpu_textures_pattern_clut(0x5C, 1, clut);
    assert(clut[0] == psx_colour_rgb555(12));
    assert(clut[1] == 0x0000);
    psx_gpu_textures_pattern_clut(0xF0, 1, clut);
    assert(clut[0] == 0x8000);
}

int main(void) {
    test_a_transparent_entry_is_zero_and_black_stays_opaque();
    test_each_entry_follows_its_draw_palette_byte();
    test_each_character_cell_holds_its_glyph_and_nothing_else_is_lit();
    test_a_cell_is_found_by_its_character_code_signed_or_not();
    test_a_font_clut_lights_texel_one_in_its_colour_and_outlines_texel_two_in_black();
    test_a_pattern_tile_holds_the_software_bit_at_each_world_phase();
    test_a_pattern_clut_maps_clear_bits_to_the_low_nibble_and_set_ones_to_the_high();
    test_a_transparent_pattern_clut_skips_set_bits();
    printf("test_psx_gpu_textures: ok\n");
    return 0;
}
