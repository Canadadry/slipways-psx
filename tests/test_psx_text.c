#include "platform/psx_text.h"

#include "p8/draw_cost.h"
#include "p8/gfx.h"
#include "p8/vram.h"
#include "platform/psx_gpu_textures.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

enum { UNTOUCHED = 0xEE };

static p8_vram_t software;
static unsigned char painted[P8_SCREEN_HEIGHT][P8_SCREEN_WIDTH];
static uint16_t font[PSX_GPU_TEXTURES_FONT_UNITS];

typedef struct {
    int colour;
    int highest_texel;
    int sprites;
} paint_t;

/* The GPU's SPRT through a font CLUT: texel 1 in the colour, texel 2 in colour 0, texel 0 skipped
 * as transparent. */
static void paint(psx_sprite_t glyph, void *context) {
    paint_t *paint = context;
    assert(glyph.w >= 1 && glyph.h >= 1);
    assert(glyph.x >= 0 && glyph.x + glyph.w <= P8_SCREEN_WIDTH);
    assert(glyph.y >= 0 && glyph.y + glyph.h <= P8_SCREEN_HEIGHT);
    assert(glyph.u >= 0 && glyph.u + glyph.w <= PSX_GPU_TEXTURES_FONT_SIZE);
    assert(glyph.v >= 0 && glyph.v + glyph.h <= PSX_GPU_TEXTURES_FONT_SIZE);
    for (int j = 0; j < glyph.h; j++) {
        for (int i = 0; i < glyph.w; i++) {
            int u = glyph.u + i;
            int texel = (font[(glyph.v + j) * (PSX_GPU_TEXTURES_FONT_SIZE / 4) + u / 4] >> (4 * (u % 4))) & 0x0F;
            assert(texel <= paint->highest_texel);
            if (texel == 1) {
                painted[glyph.y + j][glyph.x + i] = (unsigned char)paint->colour;
            } else if (texel == 2) {
                painted[glyph.y + j][glyph.x + i] = 0;
            }
        }
    }
    paint->sprites++;
}

static int text_matches_software(const p8_graphics_t *gfx, const char *str, int x, int y, int col) {
    memset(software.screen, UNTOUCHED, sizeof software.screen);
    memset(painted, UNTOUCHED, sizeof painted);
    p8_print(gfx, &software, str, x, y, col);
    paint_t context = { col, 1, 0 };
    psx_text_place(gfx, str, x, y, paint, &context);
    assert(memcmp(painted, software.screen, sizeof painted) == 0);
    p8_draw_cost_window_t window;
    p8_draw_cost_frame_begin(&window);
    assert((unsigned long)context.sprites * P8_GPU_SPRT_BYTES <= p8_draw_cost_print(&window, gfx, str, x, y).bytes);
    return context.sprites;
}

static int outlined_text_matches_software(const p8_graphics_t *gfx, const char *str, int x, int y, int col) {
    memset(software.screen, UNTOUCHED, sizeof software.screen);
    memset(painted, UNTOUCHED, sizeof painted);
    p8_print_outlined(gfx, &software, str, x, y, col);
    paint_t context = { col, 2, 0 };
    psx_text_place_outlined(gfx, str, x, y, paint, &context);
    assert(memcmp(painted, software.screen, sizeof painted) == 0);
    p8_draw_cost_window_t window;
    p8_draw_cost_frame_begin(&window);
    assert((unsigned long)context.sprites * P8_GPU_SPRT_BYTES <= p8_draw_cost_print_outlined(&window, gfx, str, x, y).bytes);
    return context.sprites;
}

static void setup(void) {
    p8_vram_init(&software);
    psx_gpu_textures_font(font);
}

static void test_one_sprite_per_glyph_with_a_lit_row(void) {
    setup();
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    assert(text_matches_software(&gfx, "-too far-", 40, 60, 8) == 8);
    assert(text_matches_software(&gfx, "a b", 0, 0, 7) == 2);
    assert(text_matches_software(&gfx, "   ", 10, 10, 7) == 0);
    assert(text_matches_software(&gfx, "", 10, 10, 7) == 0);
}

static void test_every_character_code_draws_its_software_glyph(void) {
    setup();
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    char line[33];
    for (int first = 1; first < 256; first += 32) {
        for (int i = 0; i < 32; i++) {
            int code = first + i;
            line[i] = (char)(code < 256 ? code : '?');
        }
        line[32] = '\0';
        assert(text_matches_software(&gfx, line, 2, 50, 12) > 0);
    }
}

static void test_text_across_an_edge_is_clipped_like_the_software(void) {
    setup();
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    assert(text_matches_software(&gfx, "SCORE 123", -10, -2, 9) > 0);
    assert(text_matches_software(&gfx, "SCORE 123", 140, 117, 0) > 0);
    assert(text_matches_software(&gfx, "X", -3, 50, 7) == 0);
    assert(text_matches_software(&gfx, "X", 160, 50, 7) == 0);
    assert(text_matches_software(&gfx, "X", 50, -5, 7) == 0);
    assert(text_matches_software(&gfx, "X", 50, 120, 7) == 0);
    assert(text_matches_software(&gfx, "X", -2, 50, 7) == 1);
}

static void test_the_camera_moves_the_text(void) {
    setup();
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    p8_camera(&gfx, 300, -40);
    assert(text_matches_software(&gfx, "p61 (2/3)", 320, -10, 11) > 0);
    assert(text_matches_software(&gfx, "far away", 0, 0, 11) == 0);
}

static void test_random_text_and_cameras_match_the_software(void) {
    setup();
    unsigned state = 2024u;
    int drawn = 0;
    for (int i = 0; i < 200; i++) {
        char text[12];
        state = state * 1103515245u + 12345u;
        int length = (int)(state >> 16) % 11;
        for (int c = 0; c < length; c++) {
            state = state * 1103515245u + 12345u;
            text[c] = (char)(32 + (int)(state >> 16) % 96);
        }
        text[length] = '\0';
        p8_graphics_t gfx;
        p8_gfx_init(&gfx);
        state = state * 1103515245u + 12345u;
        p8_camera(&gfx, (int)(state >> 4) % 61 - 30, (int)(state >> 12) % 61 - 30);
        int x = (int)(state >> 18) % 220 - 50;
        state = state * 1103515245u + 12345u;
        int y = (int)(state >> 16) % 160 - 30;
        drawn += text_matches_software(&gfx, text, x, y, (int)(state >> 8) % 16) > 0;
    }
    assert(drawn > 50 && drawn < 200);
}

static void test_an_outlined_glyph_is_one_sprite(void) {
    setup();
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    assert(outlined_text_matches_software(&gfx, "-too far-", 40, 60, 8) == 8);
    assert(outlined_text_matches_software(&gfx, "a b", 0, 0, 7) == 2);
    assert(outlined_text_matches_software(&gfx, "   ", 10, 10, 7) == 0);
    assert(outlined_text_matches_software(&gfx, "", 10, 10, 7) == 0);
    assert(outlined_text_matches_software(&gfx, "WMW#", 30, 30, 0) == 4);
}

static void test_every_outlined_character_code_draws_its_software_glyph(void) {
    setup();
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    char line[33];
    for (int first = 1; first < 256; first += 32) {
        for (int i = 0; i < 32; i++) {
            int code = first + i;
            line[i] = (char)(code < 256 ? code : '?');
        }
        line[32] = '\0';
        assert(outlined_text_matches_software(&gfx, line, 1, 50, 12) > 0);
    }
}

static void test_outlined_text_across_an_edge_is_clipped_like_the_software(void) {
    setup();
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    assert(outlined_text_matches_software(&gfx, "SCORE 123", -10, -2, 9) > 0);
    assert(outlined_text_matches_software(&gfx, "SCORE 123", 140, 118, 3) > 0);
    assert(outlined_text_matches_software(&gfx, "X", -4, 50, 7) == 0);
    assert(outlined_text_matches_software(&gfx, "X", -3, 50, 7) == 1 && "only the outline shows");
    assert(outlined_text_matches_software(&gfx, "X", 160, 50, 7) == 1);
    assert(outlined_text_matches_software(&gfx, "X", 161, 50, 7) == 0);
    assert(outlined_text_matches_software(&gfx, "X", 50, -6, 7) == 0);
    assert(outlined_text_matches_software(&gfx, "X", 50, -5, 7) == 1);
    assert(outlined_text_matches_software(&gfx, "X", 50, 120, 7) == 1);
    assert(outlined_text_matches_software(&gfx, "X", 50, 121, 7) == 0);
}

static void test_the_camera_moves_the_outlined_text(void) {
    setup();
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    p8_camera(&gfx, 300, -40);
    assert(outlined_text_matches_software(&gfx, "p61 (2/3)", 320, -10, 11) > 0);
    assert(outlined_text_matches_software(&gfx, "far away", 0, 0, 11) == 0);
}

static void test_random_outlined_text_and_cameras_match_the_software(void) {
    setup();
    unsigned state = 1789u;
    int drawn = 0;
    for (int i = 0; i < 200; i++) {
        char text[12];
        state = state * 1103515245u + 12345u;
        int length = (int)(state >> 16) % 11;
        for (int c = 0; c < length; c++) {
            state = state * 1103515245u + 12345u;
            text[c] = (char)(32 + (int)(state >> 16) % 96);
        }
        text[length] = '\0';
        p8_graphics_t gfx;
        p8_gfx_init(&gfx);
        state = state * 1103515245u + 12345u;
        p8_camera(&gfx, (int)(state >> 4) % 61 - 30, (int)(state >> 12) % 61 - 30);
        int x = (int)(state >> 18) % 220 - 50;
        state = state * 1103515245u + 12345u;
        int y = (int)(state >> 16) % 160 - 30;
        drawn += outlined_text_matches_software(&gfx, text, x, y, (int)(state >> 8) % 16) > 0;
    }
    assert(drawn > 50 && drawn < 200);
}

int main(void) {
    test_one_sprite_per_glyph_with_a_lit_row();
    test_every_character_code_draws_its_software_glyph();
    test_text_across_an_edge_is_clipped_like_the_software();
    test_the_camera_moves_the_text();
    test_random_text_and_cameras_match_the_software();
    test_an_outlined_glyph_is_one_sprite();
    test_every_outlined_character_code_draws_its_software_glyph();
    test_outlined_text_across_an_edge_is_clipped_like_the_software();
    test_the_camera_moves_the_outlined_text();
    test_random_outlined_text_and_cameras_match_the_software();
    printf("test_psx_text: ok\n");
    return 0;
}
