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

static void test_clut_index_zero_is_forced_transparent_black(void) {
    T_CHECK(g_pico8_clut[0] == 0x0000);
}

/* color 1: #1D2B53 -> r=0x1D>>3=3, g=0x2B>>3=5, b=0x53>>3=10 ->
 * (10<<10)|(5<<5)|3 = 0x28a3
 * color 6: #C2C3C7 -> r=0xC2>>3=24, g=0xC3>>3=24, b=0xC7>>3=24 ->
 * (24<<10)|(24<<5)|24 = 0x6318
 * color 7: #FFF1E8 -> r=0xFF>>3=31, g=0xF1>>3=30, b=0xE8>>3=29 ->
 * (29<<10)|(30<<5)|31 = 0x77df
 * color 8: #FF004D -> r=0xFF>>3=31, g=0x00>>3=0, b=0x4D>>3=9 ->
 * (9<<10)|(0<<5)|31 = 0x241f */
static void test_clut_matches_traced_pico8_default_palette(void) {
    T_CHECK(g_pico8_clut[1] == 0x28a3);
    T_CHECK(g_pico8_clut[6] == 0x6318);
    T_CHECK(g_pico8_clut[7] == 0x77df);
    T_CHECK(g_pico8_clut[8] == 0x241f);
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
    t_run("clut_matches_traced_pico8_default_palette", test_clut_matches_traced_pico8_default_palette);
    t_run("cel_rect_1x1_matches_id_to_pixel_formula", test_cel_rect_1x1_matches_id_to_pixel_formula);
    t_run("cel_rect_2x2_matches_id_to_pixel_formula", test_cel_rect_2x2_matches_id_to_pixel_formula);
    return t_report();
}
