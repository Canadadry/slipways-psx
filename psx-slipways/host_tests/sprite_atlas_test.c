#include "t.h"
#include "../sprite_atlas.h"

#include <stdint.h>

static const uint8_t g_sprite43_pixels[16][16] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 1, 5, 5, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 5, 13, 13, 5, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 5, 13, 5, 5, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 5, 5, 5, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

static const uint8_t g_sprite58_pixels[8][8] = {
    { 0, 0, 3, 3, 3, 3, 3, 3 },
    { 0, 6, 0, 3, 3, 3, 3, 3 },
    { 0, 7, 6, 0, 3, 3, 3, 3 },
    { 0, 7, 7, 6, 0, 3, 3, 3 },
    { 0, 7, 7, 7, 6, 0, 3, 3 },
    { 0, 0, 0, 7, 0, 0, 3, 3 },
    { 3, 3, 0, 0, 6, 0, 3, 3 },
    { 3, 3, 3, 0, 0, 0, 3, 3 },
};

static const uint8_t g_sprite31_pixels[8][8] = {
    { 3, 3, 0, 0, 0, 3, 3, 3 },
    { 3, 0, 8, 8, 8, 0, 3, 3 },
    { 0, 8, 0, 0, 8, 2, 0, 3 },
    { 0, 8, 0, 8, 0, 2, 0, 3 },
    { 0, 8, 8, 0, 0, 2, 0, 3 },
    { 3, 0, 2, 2, 2, 0, 3, 3 },
    { 3, 3, 0, 0, 0, 3, 3, 3 },
    { 3, 3, 3, 3, 3, 3, 3, 3 },
};

static uint8_t unpack_4bpp_pixel(const uint8_t *atlas, int x, int y) {
    uint8_t byte = atlas[y * (128 / 2) + (x / 2)];
    if (x % 2 == 0) {
        return byte & 0x0F;
    }
    return (byte >> 4) & 0x0F;
}

static void test_sprite_planet_unknown_pixels_match_gfx_json(void) {
    int rx, ry, rw, rh;
    sprite_atlas_cel_rect(SPRITE_PLANET_UNKNOWN, 2, 2, &rx, &ry, &rw, &rh);

    T_CHECK(rw == 16);
    T_CHECK(rh == 16);
    for (int y = 0; y < rh; y++) {
        for (int x = 0; x < rw; x++) {
            uint8_t decoded = unpack_4bpp_pixel(g_sprite_atlas_4bpp, rx + x, ry + y);
            T_CHECK(decoded == g_sprite43_pixels[y][x]);
        }
    }
}

static void test_sprite_cursor_default_pixels_match_gfx_json(void) {
    int rx, ry, rw, rh;
    sprite_atlas_cel_rect(SPRITE_CURSOR_DEFAULT, 1, 1, &rx, &ry, &rw, &rh);

    T_CHECK(rw == 8);
    T_CHECK(rh == 8);
    for (int y = 0; y < rh; y++) {
        for (int x = 0; x < rw; x++) {
            uint8_t decoded = unpack_4bpp_pixel(g_sprite_atlas_4bpp, rx + x, ry + y);
            T_CHECK(decoded == g_sprite58_pixels[y][x]);
        }
    }
}

static void test_sprite_liaison_invalid_target_pixels_match_gfx_json(void) {
    int rx, ry, rw, rh;
    sprite_atlas_cel_rect(SPRITE_LIAISON_INVALID_TARGET, 1, 1, &rx, &ry, &rw, &rh);

    T_CHECK(rw == 8);
    T_CHECK(rh == 8);
    for (int y = 0; y < rh; y++) {
        for (int x = 0; x < rw; x++) {
            uint8_t decoded = unpack_4bpp_pixel(g_sprite_atlas_4bpp, rx + x, ry + y);
            T_CHECK(decoded == g_sprite31_pixels[y][x]);
        }
    }
}

static void test_sprite_planet_unknown_specific_positions(void) {
    int rx, ry, rw, rh;
    sprite_atlas_cel_rect(SPRITE_PLANET_UNKNOWN, 2, 2, &rx, &ry, &rw, &rh);

    T_CHECK(rx == 88);
    T_CHECK(ry == 16);
    T_CHECK(rw == 16);
    T_CHECK(rh == 16);

    T_CHECK(unpack_4bpp_pixel(g_sprite_atlas_4bpp, rx + 0, ry + 0) == 0);
    T_CHECK(unpack_4bpp_pixel(g_sprite_atlas_4bpp, rx + 8, ry + 8) == 0);
    T_CHECK(unpack_4bpp_pixel(g_sprite_atlas_4bpp, rx + 15, ry + 15) == 0);
    T_CHECK(unpack_4bpp_pixel(g_sprite_atlas_4bpp, rx + 3, ry + 3) == 13);
}

static void test_sprite_cursor_default_specific_positions(void) {
    int rx, ry, rw, rh;
    sprite_atlas_cel_rect(SPRITE_CURSOR_DEFAULT, 1, 1, &rx, &ry, &rw, &rh);

    T_CHECK(rx == 80);
    T_CHECK(ry == 24);
    T_CHECK(rw == 8);
    T_CHECK(rh == 8);

    T_CHECK(unpack_4bpp_pixel(g_sprite_atlas_4bpp, rx + 0, ry + 0) == 0);
    T_CHECK(unpack_4bpp_pixel(g_sprite_atlas_4bpp, rx + 4, ry + 4) == 6);
    T_CHECK(unpack_4bpp_pixel(g_sprite_atlas_4bpp, rx + 7, ry + 7) == 3);
}

static void test_sprite_liaison_invalid_target_specific_positions(void) {
    int rx, ry, rw, rh;
    sprite_atlas_cel_rect(SPRITE_LIAISON_INVALID_TARGET, 1, 1, &rx, &ry, &rw, &rh);

    T_CHECK(rx == 120);
    T_CHECK(ry == 8);
    T_CHECK(rw == 8);
    T_CHECK(rh == 8);

    T_CHECK(unpack_4bpp_pixel(g_sprite_atlas_4bpp, rx + 0, ry + 0) == 3);
    T_CHECK(unpack_4bpp_pixel(g_sprite_atlas_4bpp, rx + 4, ry + 4) == 0);
    T_CHECK(unpack_4bpp_pixel(g_sprite_atlas_4bpp, rx + 7, ry + 7) == 3);
    T_CHECK(unpack_4bpp_pixel(g_sprite_atlas_4bpp, rx + 3, ry + 3) == 8);
}

/* Independently-derived expected CLUT values (chunk 2,
 * docs/prd/triage/psx-sprites-audio-extraction.md, Implementation Chunks
 * item 2). Hand-typed here from PICO-8's real, well-known, fixed default
 * 16-color system palette -- deliberately NOT copied from
 * tools/gen_psx_sprite_atlas.py's PICO8_PALETTE_HEX table -- so that this
 * test independently checks the tool's generated output rather than
 * tautologically re-testing the tool against itself. Standard PICO-8
 * color names/hex, in PICO-8's fixed index order 0-15: black, dark-blue,
 * dark-purple, dark-green, brown, dark-grey, light-grey, white, red,
 * orange, yellow, green, blue, lavender, pink, light-peach. */
static const uint32_t g_expected_pico8_palette_rgb888[16] = {
    0x000000, /* 0  black */
    0x1D2B53, /* 1  dark-blue */
    0x7E2553, /* 2  dark-purple */
    0x008751, /* 3  dark-green */
    0xAB5236, /* 4  brown */
    0x5F574F, /* 5  dark-grey */
    0xC2C3C7, /* 6  light-grey */
    0xFFF1E8, /* 7  white */
    0xFF004D, /* 8  red */
    0xFFA300, /* 9  orange */
    0xFFEC27, /* 10 yellow */
    0x00E436, /* 11 green */
    0x29ADFF, /* 12 blue */
    0x83769C, /* 13 lavender */
    0xFF77A8, /* 14 pink */
    0xFFCCAA, /* 15 light-peach */
};

/* Test-local BGR555 conversion, independently re-implemented here rather
 * than shared with tools/gen_psx_sprite_atlas.py's rgb_to_bgr555() -- same
 * reasoning as the palette table above: this must independently verify
 * the tool's conversion, not just re-run it. Same standard,
 * truncating-right-shift-by-3-per-channel encoding:
 * bgr555 = (b5<<10)|(g5<<5)|r5. */
static uint16_t expected_bgr555(uint32_t rgb888) {
    uint8_t r8 = (uint8_t)((rgb888 >> 16) & 0xFF);
    uint8_t g8 = (uint8_t)((rgb888 >> 8) & 0xFF);
    uint8_t b8 = (uint8_t)(rgb888 & 0xFF);
    uint16_t r5 = (uint16_t)(r8 >> 3);
    uint16_t g5 = (uint16_t)(g8 >> 3);
    uint16_t b5 = (uint16_t)(b8 >> 3);
    return (uint16_t)((b5 << 10) | (g5 << 5) | r5);
}

/* Dedicated, standalone guard for PICO-8's own sprite-color-0-is-
 * transparent convention (see sprite_atlas.h's doc comment on
 * g_pico8_clut) -- kept deliberately separate from the general per-index
 * loop below, and asserting the literal 0x0000 value rather than relying
 * on "whatever black's real RGB happens to convert to" (which also
 * independently computes to 0x0000 -- see index 0 in
 * test_clut_matches_independently_derived_pico8_palette below -- but that
 * is a coincidence this test does not depend on): this is the one check
 * that keeps a future edit from silently dropping the deliberate
 * clut[0] = 0x0000 override). */
static void test_clut_index_zero_is_forced_transparent_black(void) {
    T_CHECK(g_pico8_clut[0] == 0x0000);
}

static void test_clut_matches_independently_derived_pico8_palette(void) {
    for (int i = 0; i < 16; i++) {
        uint16_t expected = expected_bgr555(g_expected_pico8_palette_rgb888[i]);
        T_CHECK(g_pico8_clut[i] == expected);
    }
}

static void test_cel_rect_1x1_matches_id_to_pixel_formula(void) {
    int x, y, w, h;
    sprite_atlas_cel_rect(0, 1, 1, &x, &y, &w, &h);
    T_CHECK(x == 0);
    T_CHECK(y == 0);
    T_CHECK(w == 8);
    T_CHECK(h == 8);

    sprite_atlas_cel_rect(17, 1, 1, &x, &y, &w, &h);
    T_CHECK(x == 8);
    T_CHECK(y == 8);
    T_CHECK(w == 8);
    T_CHECK(h == 8);
}

static void test_cel_rect_2x2_matches_id_to_pixel_formula(void) {
    int x, y, w, h;
    sprite_atlas_cel_rect(43, 2, 2, &x, &y, &w, &h);
    T_CHECK(x == 88);
    T_CHECK(y == 16);
    T_CHECK(w == 16);
    T_CHECK(h == 16);
}

int main(void) {
    t_run("sprite_planet_unknown_pixels_match_gfx_json", test_sprite_planet_unknown_pixels_match_gfx_json);
    t_run("sprite_cursor_default_pixels_match_gfx_json", test_sprite_cursor_default_pixels_match_gfx_json);
    t_run("sprite_liaison_invalid_target_pixels_match_gfx_json", test_sprite_liaison_invalid_target_pixels_match_gfx_json);
    t_run("sprite_planet_unknown_specific_positions", test_sprite_planet_unknown_specific_positions);
    t_run("sprite_cursor_default_specific_positions", test_sprite_cursor_default_specific_positions);
    t_run("sprite_liaison_invalid_target_specific_positions", test_sprite_liaison_invalid_target_specific_positions);
    t_run("clut_index_zero_is_forced_transparent_black", test_clut_index_zero_is_forced_transparent_black);
    t_run("clut_matches_independently_derived_pico8_palette", test_clut_matches_independently_derived_pico8_palette);
    t_run("cel_rect_1x1_matches_id_to_pixel_formula", test_cel_rect_1x1_matches_id_to_pixel_formula);
    t_run("cel_rect_2x2_matches_id_to_pixel_formula", test_cel_rect_2x2_matches_id_to_pixel_formula);
    return t_report();
}
