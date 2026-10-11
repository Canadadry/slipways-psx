#include "platform/psx_gpu_textures.h"

#include "p8/gfx.h"
#include "p8/sprite_sheet.h"
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

static void test_a_font_clut_lights_texel_one_in_its_colour(void) {
    for (int colour = 0; colour < 16; colour++) {
        uint16_t clut[PSX_GPU_TEXTURES_CLUT_ENTRIES];
        memset(clut, 0xFF, sizeof clut);
        psx_gpu_textures_font_clut(colour, clut);
        assert(clut[0] == 0x0000 && "a clear texel is transparent");
        assert(clut[1] == psx_colour_rgb555(colour));
        for (int i = 2; i < PSX_GPU_TEXTURES_CLUT_ENTRIES; i++) {
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

static int pattern_x2_texel(const uint16_t tile[PSX_GPU_TEXTURES_PATTERN_X2_UNITS], int u, int v) {
    int unit = tile[v * (PSX_GPU_TEXTURES_PATTERN_X2_SIZE / 4) + u / 4];
    return (unit >> (4 * (u % 4))) & 0x0F;
}

static void test_the_x2_pattern_tile_is_the_tile_at_half_its_coordinates_with_a_16_texel_period(void) {
    assert(PSX_GPU_TEXTURES_PATTERN_X2_SIZE == 16);
    for (unsigned int pattern = 1; pattern <= 0xFFFFu; pattern += 251) {
        uint16_t tile[PSX_GPU_TEXTURES_PATTERN_UNITS];
        uint16_t tile_x2[PSX_GPU_TEXTURES_PATTERN_X2_UNITS];
        memset(tile_x2, 0xFF, sizeof tile_x2);
        psx_gpu_textures_pattern(pattern, tile);
        psx_gpu_textures_pattern_x2(pattern, tile_x2);
        for (int v = 0; v < PSX_GPU_TEXTURES_PATTERN_X2_SIZE; v++) {
            for (int u = 0; u < PSX_GPU_TEXTURES_PATTERN_X2_SIZE; u++) {
                assert(pattern_x2_texel(tile_x2, u, v) == pattern_texel(tile, u / 2, v / 2));
                assert(pattern_x2_texel(tile_x2, u, v) == pattern_x2_texel(tile_x2, u % 8, v % 8)
                       && "the doubled 4x4 pattern, twice each way");
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

enum { SHEET_X2_SIZE = 2 * P8_SPRITE_SHEET_WIDTH, SHEET_X2_BYTES = SHEET_X2_SIZE * SHEET_X2_SIZE / 2 };

static unsigned char sheet_x2[SHEET_X2_BYTES];

static int packed_texel(const unsigned char *texture, int width, int u, int v) {
    unsigned char byte = texture[v * (width / 2) + u / 2];
    return u % 2 == 0 ? byte & 0x0F : byte >> 4;
}

static void test_the_x2_sheet_texel_is_the_packed_texel_at_half_its_coordinates(void) {
    memset(sheet_x2, 0xEE, sizeof sheet_x2);
    psx_gpu_textures_upscale(p8_sprite_sheet_packed(), P8_SPRITE_SHEET_WIDTH, 0, SHEET_X2_SIZE, sheet_x2);
    for (int v = 0; v < SHEET_X2_SIZE; v++) {
        for (int u = 0; u < SHEET_X2_SIZE; u++) {
            assert(packed_texel(sheet_x2, SHEET_X2_SIZE, u, v) == packed_texel(p8_sprite_sheet_packed(), P8_SPRITE_SHEET_WIDTH, u / 2, v / 2));
        }
    }
}

static unsigned char chunked[SHEET_X2_BYTES];

static void upscale_sheet_in_chunks(int first_line, int chunk_lines) {
    memset(chunked, 0xEE, sizeof chunked);
    for (int line = first_line; line < SHEET_X2_SIZE; line += chunk_lines) {
        int lines = SHEET_X2_SIZE - line < chunk_lines ? SHEET_X2_SIZE - line : chunk_lines;
        psx_gpu_textures_upscale(p8_sprite_sheet_packed(), P8_SPRITE_SHEET_WIDTH, line, lines,
                                 chunked + line * P8_SPRITE_SHEET_WIDTH);
    }
}

static void test_a_chunked_upscale_equals_the_single_pass(void) {
    psx_gpu_textures_upscale(p8_sprite_sheet_packed(), P8_SPRITE_SHEET_WIDTH, 0, SHEET_X2_SIZE, sheet_x2);
    upscale_sheet_in_chunks(0, 16);
    assert(memcmp(chunked, sheet_x2, sizeof chunked) == 0);
    upscale_sheet_in_chunks(0, 7);
    assert(memcmp(chunked, sheet_x2, sizeof chunked) == 0);
    upscale_sheet_in_chunks(33, 5);
    int skipped = 33 * P8_SPRITE_SHEET_WIDTH;
    assert(memcmp(chunked + skipped, sheet_x2 + skipped, sizeof chunked - (size_t)skipped) == 0 && "an odd first line");
}

enum { FONT_X2_SIZE = 2 * PSX_GPU_TEXTURES_FONT_SIZE, FONT_X2_LINES = 2 * PSX_GPU_TEXTURES_FONT_LINES };

static unsigned char font_x2[FONT_X2_SIZE * FONT_X2_LINES / 2];

static void test_the_font_bytes_are_its_texels_low_nibble_first(void) {
    psx_gpu_textures_font(font);
    const unsigned char *bytes = (const unsigned char *)font;
    for (int v = 0; v < PSX_GPU_TEXTURES_FONT_SIZE; v++) {
        for (int u = 0; u < PSX_GPU_TEXTURES_FONT_SIZE; u++) {
            assert(packed_texel(bytes, PSX_GPU_TEXTURES_FONT_SIZE, u, v) == font_texel(u, v)
                   && "the upscaler reads the font's uint16_t units as bytes: only a little-endian host and PSX agree");
        }
    }
}

static void test_the_x2_font_texel_is_the_font_texel_at_half_its_coordinates(void) {
    psx_gpu_textures_font(font);
    memset(font_x2, 0xEE, sizeof font_x2);
    psx_gpu_textures_upscale((const unsigned char *)font, PSX_GPU_TEXTURES_FONT_SIZE, 0, FONT_X2_LINES, font_x2);
    for (int v = 0; v < FONT_X2_LINES; v++) {
        for (int u = 0; u < FONT_X2_SIZE; u++) {
            assert(packed_texel(font_x2, FONT_X2_SIZE, u, v) == font_texel(u / 2, v / 2));
        }
    }
    for (int v = FONT_X2_LINES; v < 2 * PSX_GPU_TEXTURES_FONT_SIZE; v++) {
        for (int u = 0; u < FONT_X2_SIZE; u++) {
            assert(font_texel(u / 2, v / 2) == 0 && "the font extent leaves out a lit texel");
        }
    }
}

static void test_every_x2_cell_fits_the_x2_font(void) {
    for (int code = 0; code < 256; code++) {
        psx_gpu_textures_texel_t cell = psx_gpu_textures_font_cell((char)code);
        assert(2 * cell.u >= 0 && 2 * cell.u + 2 * P8_FONT_CELL_WIDTH <= FONT_X2_SIZE);
        assert(2 * cell.v >= 0 && 2 * cell.v + 2 * (P8_FONT_GLYPH_HEIGHT + 1) <= FONT_X2_LINES);
    }
}

int main(void) {
    test_a_transparent_entry_is_zero_and_black_stays_opaque();
    test_each_entry_follows_its_draw_palette_byte();
    test_each_character_cell_holds_its_glyph_and_nothing_else_is_lit();
    test_a_cell_is_found_by_its_character_code_signed_or_not();
    test_a_font_clut_lights_texel_one_in_its_colour();
    test_a_pattern_tile_holds_the_software_bit_at_each_world_phase();
    test_the_x2_pattern_tile_is_the_tile_at_half_its_coordinates_with_a_16_texel_period();
    test_a_pattern_clut_maps_clear_bits_to_the_low_nibble_and_set_ones_to_the_high();
    test_a_transparent_pattern_clut_skips_set_bits();
    test_the_x2_sheet_texel_is_the_packed_texel_at_half_its_coordinates();
    test_a_chunked_upscale_equals_the_single_pass();
    test_the_font_bytes_are_its_texels_low_nibble_first();
    test_the_x2_font_texel_is_the_font_texel_at_half_its_coordinates();
    test_every_x2_cell_fits_the_x2_font();
    printf("test_psx_gpu_textures: ok\n");
    return 0;
}
