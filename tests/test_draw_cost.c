#include "p8/draw_cost.h"
#include "p8/gfx.h"
#include "p8/vram.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static p8_graphics_t gfx;
static p8_draw_cost_window_t window;

static void reset(void) {
    p8_gfx_init(&gfx);
    (void)p8_draw_cost_frame_begin(&window);
}

enum { WINDOW_PAIR_BYTES = P8_GPU_DR_TPAGE_BYTES + P8_GPU_DR_TWIN_BYTES };

static void assert_cost(p8_draw_cost_t cost, unsigned long bytes, int primitives) {
    assert(cost.bytes == bytes);
    assert(cost.primitives == primitives);
}

static void test_cls_is_one_tile(void) {
    reset();
    assert_cost(p8_draw_cost_cls(), P8_GPU_TILE_BYTES, 1);
}

static void test_pset_is_one_tile_and_off_screen_is_free(void) {
    reset();
    assert_cost(p8_draw_cost_pset(&gfx, 0, 0), P8_GPU_TILE_BYTES, 1);
    assert_cost(p8_draw_cost_pset(&gfx, 159, 119), P8_GPU_TILE_BYTES, 1);
    assert_cost(p8_draw_cost_pset(&gfx, 160, 0), 0, 0);
    assert_cost(p8_draw_cost_pset(&gfx, 0, -1), 0, 0);
}


static void test_culling_is_judged_after_the_camera(void) {
    reset();
    p8_camera(&gfx, 100, 50);
    assert_cost(p8_draw_cost_pset(&gfx, 10, 10), 0, 0);
    assert_cost(p8_draw_cost_pset(&gfx, 100, 50), P8_GPU_TILE_BYTES, 1);
    assert_cost(p8_draw_cost_pset(&gfx, 259, 169), P8_GPU_TILE_BYTES, 1);
    assert_cost(p8_draw_cost_pset(&gfx, 260, 169), 0, 0);
}

static unsigned char walked[P8_SCREEN_HEIGHT][P8_SCREEN_WIDTH];

static void mark_walked(int x, int y, void *context) {
    assert(context == walked);
    assert(x >= 0 && x < P8_SCREEN_WIDTH && y >= 0 && y < P8_SCREEN_HEIGHT);
    assert(walked[y][x] == 0 && "a pixel is emitted once");
    walked[y][x] = 1;
}

/* The pixels the walk emits are those the software line lights; returns their number. */
static int line_pixels_match_software(int x1, int y1, int x2, int y2) {
    static p8_vram_t software;
    p8_vram_init(&software);
    p8_line(&gfx, &software, x1, y1, x2, y2, 7);
    memset(walked, 0, sizeof walked);
    int count = p8_draw_cost_line_pixels(&gfx, x1, y1, x2, y2, mark_walked, walked);
    int lit = 0;
    for (int y = 0; y < P8_SCREEN_HEIGHT; y++) {
        for (int x = 0; x < P8_SCREEN_WIDTH; x++) {
            assert(walked[y][x] == (software.screen[y][x] != 0));
            lit += walked[y][x];
        }
    }
    assert(count == lit);
    assert(p8_draw_cost_line_pixels(&gfx, x1, y1, x2, y2, NULL, NULL) == count && "counting needs no emitter");
    assert_cost(p8_draw_cost_line(&gfx, x1, y1, x2, y2), (unsigned long)count * P8_GPU_TILE_BYTES, count);
    return count;
}

static void test_a_line_costs_one_tile_per_pixel_the_software_lights(void) {
    reset();
    assert(line_pixels_match_software(5, 5, 5, 5) == 1);
    assert(line_pixels_match_software(-5, 3, 4, 3) == 5);
    assert(line_pixels_match_software(7, 100, 7, 130) == 20);
    assert(line_pixels_match_software(0, 0, 5, 5) == 6);
    assert(line_pixels_match_software(10, 20, 90, 37) == 81);
    assert(line_pixels_match_software(90, 37, 10, 20) == 81);
    assert(line_pixels_match_software(30, 10, 41, 100) == 91);
    assert(line_pixels_match_software(41, 100, 30, 10) == 91);
    assert(line_pixels_match_software(-10, 5, 5, -10) == 0 && "its box meets the screen, its pixels miss it");
    assert(line_pixels_match_software(170, -10, 160, 50) == 0);
    p8_camera(&gfx, 100, -30);
    assert(line_pixels_match_software(100, -30, 120, -10) == 21);
    assert(line_pixels_match_software(0, 0, 99, 50) == 0);
}

static void test_random_lines_cost_what_the_software_lights(void) {
    unsigned seed = 4242;
    for (int i = 0; i < 300; i++) {
        int values[6];
        for (int v = 0; v < 6; v++) {
            seed = seed * 1103515245u + 12345u;
            values[v] = (int)((seed >> 8) % 560) - 200;
        }
        reset();
        p8_camera(&gfx, values[4] / 8, values[5] / 8);
        line_pixels_match_software(values[0], values[1] * 520 / 560, values[2], values[3] * 520 / 560);
    }
}

static void test_rect_is_four_tiles_and_straddling_costs_in_full(void) {
    reset();
    assert_cost(p8_draw_cost_rect(&gfx, 150, 110, 170, 130), 4 * P8_GPU_TILE_BYTES, 4);
    assert_cost(p8_draw_cost_rect(&gfx, -1, -5, -20, 30), 0, 0);
}


static void test_plain_rectfill_is_one_tile(void) {
    reset();
    assert_cost(p8_draw_cost_rectfill(&window, &gfx, 0, 0, 159, 119, 7), P8_GPU_TILE_BYTES, 1);
    assert_cost(p8_draw_cost_rectfill(&window, &gfx, 0, 120, 10, 130, 7), 0, 0);
}

static void test_circles_cost_per_midpoint_iteration(void) {
    static const int RADII[] = {0, 1, 7, 9};
    static const int ITERATIONS[] = {1, 1, 5, 6};
    for (int i = 0; i < 4; i++) {
        reset();
        int n = ITERATIONS[i];
        assert_cost(p8_draw_cost_circ(&gfx, 80, 60, RADII[i]), (unsigned long)(8 * n) * P8_GPU_TILE_BYTES, 8 * n);
        assert_cost(p8_draw_cost_circfill(&window, &gfx, 80, 60, RADII[i], 7), (unsigned long)(4 * n) * P8_GPU_TILE_BYTES, 4 * n);
    }
    reset();
    assert_cost(p8_draw_cost_circ(&gfx, 80, 60, -1), 0, 0);
    assert_cost(p8_draw_cost_circfill(&window, &gfx, 80, 60, -1, 7), 0, 0);
    assert_cost(p8_draw_cost_circ(&gfx, -9, 60, 9), 8 * 6 * P8_GPU_TILE_BYTES, 8 * 6);
    assert_cost(p8_draw_cost_circ(&gfx, -10, 60, 9), 0, 0);
}

static void test_spr_is_one_sprt_whatever_its_cells(void) {
    reset();
    assert_cost(p8_draw_cost_spr(&window, &gfx, 3, 10, 10, 1, 1), P8_GPU_SPRT_BYTES, 1);
    assert_cost(p8_draw_cost_spr(&window, &gfx, 3, 10, 10, 4, 2), P8_GPU_SPRT_BYTES, 1);
    assert_cost(p8_draw_cost_spr(&window, &gfx, 3, -8, 10, 1, 1), 0, 0);
    assert_cost(p8_draw_cost_spr(&window, &gfx, 3, -16, 10, 3, 1), P8_GPU_SPRT_BYTES, 1);
    assert_cost(p8_draw_cost_spr(&window, &gfx, 3, 10, 10, 0, 1), 0, 0);
}

static void test_print_is_one_sprt_per_glyph_but_spaces(void) {
    reset();
    assert_cost(p8_draw_cost_print(&window, &gfx, "a b!", 0, 0), 3 * P8_GPU_SPRT_BYTES + WINDOW_PAIR_BYTES, 3 + 2);
    assert_cost(p8_draw_cost_print(&window, &gfx, "   ", 0, 0), 0, 0);
    assert_cost(p8_draw_cost_print(&window, &gfx, "", 0, 0), 0, 0);
    assert_cost(p8_draw_cost_print(&window, &gfx, "ab", -6, 0), 2 * P8_GPU_SPRT_BYTES, 2);
    assert_cost(p8_draw_cost_print(&window, &gfx, "ab", -7, 0), 0, 0);
    assert_cost(p8_draw_cost_print(&window, &gfx, "ab", 0, -5), 0, 0);
}


static void test_patterned_fill_is_a_sprt_with_its_window_pair_once(void) {
    reset();
    p8_fillp(&gfx, 0xA5A5);
    assert_cost(p8_draw_cost_rectfill(&window, &gfx, 0, 0, 10, 10, 0x15), P8_GPU_SPRT_BYTES + WINDOW_PAIR_BYTES, 3);
    assert_cost(p8_draw_cost_rectfill(&window, &gfx, 20, 0, 30, 10, 0x15), P8_GPU_SPRT_BYTES, 1);
    assert_cost(p8_draw_cost_circfill(&window, &gfx, 80, 60, 7, 0x15), 4 * 5 * P8_GPU_SPRT_BYTES, 4 * 5);
    assert_cost(p8_draw_cost_rectfill(&window, &gfx, 0, 0, 10, 10, 0x16), P8_GPU_SPRT_BYTES + WINDOW_PAIR_BYTES, 3);
    p8_fillp_transparent(&gfx, 0xA5A5);
    assert_cost(p8_draw_cost_rectfill(&window, &gfx, 0, 0, 10, 10, 0x16), P8_GPU_SPRT_BYTES + WINDOW_PAIR_BYTES, 3);
    p8_fillp(&gfx, 0x0F0F);
    assert_cost(p8_draw_cost_circfill(&window, &gfx, 80, 60, 0, 0x16), 4 * P8_GPU_SPRT_BYTES + WINDOW_PAIR_BYTES, 4 + 2);
}

static void test_spr_and_print_reset_the_window_after_a_pattern(void) {
    reset();
    assert_cost(p8_draw_cost_spr(&window, &gfx, 1, 0, 0, 1, 1), P8_GPU_SPRT_BYTES, 1);
    p8_fillp(&gfx, 0xA5A5);
    assert_cost(p8_draw_cost_rectfill(&window, &gfx, 0, 0, 10, 10, 1), P8_GPU_SPRT_BYTES + WINDOW_PAIR_BYTES, 3);
    assert_cost(p8_draw_cost_print(&window, &gfx, "a", 0, 0), P8_GPU_SPRT_BYTES + WINDOW_PAIR_BYTES, 3);
    assert_cost(p8_draw_cost_print(&window, &gfx, "a", 0, 0), P8_GPU_SPRT_BYTES, 1);
    assert_cost(p8_draw_cost_rectfill(&window, &gfx, 0, 0, 10, 10, 1), P8_GPU_SPRT_BYTES + WINDOW_PAIR_BYTES, 3);
    assert_cost(p8_draw_cost_spr(&window, &gfx, 1, 0, 0, 1, 1), P8_GPU_SPRT_BYTES + WINDOW_PAIR_BYTES, 3);
    assert_cost(p8_draw_cost_spr(&window, &gfx, 1, 0, 0, 1, 1), P8_GPU_SPRT_BYTES, 1);
}

static void test_a_pattern_after_print_costs_the_pair_although_they_share_a_page(void) {
    reset();
    assert_cost(p8_draw_cost_print(&window, &gfx, "a", 0, 0), P8_GPU_SPRT_BYTES + WINDOW_PAIR_BYTES, 3);
    p8_fillp(&gfx, 0xA5A5);
    assert_cost(p8_draw_cost_rectfill(&window, &gfx, 0, 0, 10, 10, 1), P8_GPU_SPRT_BYTES + WINDOW_PAIR_BYTES, 3);
    assert_cost(p8_draw_cost_print(&window, &gfx, "a", 0, 0), P8_GPU_SPRT_BYTES + WINDOW_PAIR_BYTES, 3);
}

static void assert_patterned_rectfill_sprts(int x1, int x2, int sprts) {
    assert_cost(p8_draw_cost_rectfill(&window, &gfx, x1, 0, x2, 10, 0x15), (unsigned long)sprts * P8_GPU_SPRT_BYTES, sprts);
}

/* At x2, a span's U runs from twice its world phase mod 8 over twice its width: past 256 it is
 * cut in two SPRTs. */
static void test_a_patterned_span_costs_one_sprt_per_piece_under_u_256(void) {
    reset();
    p8_fillp(&gfx, 0xA5A5);
    (void)p8_draw_cost_rectfill(&window, &gfx, 0, 0, 0, 0, 0x15);
    assert_patterned_rectfill_sprts(0, 127, 1);
    assert_patterned_rectfill_sprts(0, 128, 2);
    assert_patterned_rectfill_sprts(7, 127, 1);
    assert_patterned_rectfill_sprts(7, 128, 2);
    assert_patterned_rectfill_sprts(8, 135, 1);
    assert_patterned_rectfill_sprts(0, 159, 2);
    assert_patterned_rectfill_sprts(-50, 300, 2);
    assert_patterned_rectfill_sprts(40, 159, 1);
    p8_camera(&gfx, 3, 0);
    assert_patterned_rectfill_sprts(3, 130, 2);
    assert_patterned_rectfill_sprts(5, 127, 1);
    assert_patterned_rectfill_sprts(5, 128, 2);
    p8_camera(&gfx, -2, 0);
    assert_patterned_rectfill_sprts(-50, 119, 1);
    assert_patterned_rectfill_sprts(-50, 120, 2);
}

static void test_a_patterned_disc_costs_one_more_sprt_per_row_under_u_256(void) {
    reset();
    int plain_r60 = p8_draw_cost_circfill(&window, &gfx, 84, 60, 60, 0x15).primitives;
    int plain_r200 = p8_draw_cost_circfill(&window, &gfx, 80, 60, 200, 0x15).primitives;
    p8_fillp(&gfx, 0xA5A5);
    (void)p8_draw_cost_rectfill(&window, &gfx, 0, 0, 0, 0, 0x15);
    for (int cx = 60; cx < 100; cx++) {
        assert(p8_draw_cost_circfill(&window, &gfx, cx, 60, 60, 0x15).primitives == plain_r60 && "121 wide never crosses 256");
    }
    assert(p8_draw_cost_circfill(&window, &gfx, 80, 60, 200, 0x15).primitives == plain_r200 + P8_SCREEN_HEIGHT);
    p8_camera(&gfx, 0, 100);
    assert(p8_draw_cost_circfill(&window, &gfx, 80, 60, 200, 0x15).primitives == plain_r200 + P8_SCREEN_HEIGHT);
}

/* The rows of the software's disc whose span crosses U 256 at x2. */
static int rows_to_split(int cx, int cy, int r) {
    static p8_vram_t software;
    p8_graphics_t plain = gfx;
    p8_fillp(&plain, 0);
    p8_vram_init(&software);
    p8_circfill(&plain, &software, cx, cy, r, 7);
    int rows = 0;
    for (int y = 0; y < P8_SCREEN_HEIGHT; y++) {
        int first = -1;
        int last = -1;
        for (int x = 0; x < P8_SCREEN_WIDTH; x++) {
            if (software.screen[y][x] == 7) {
                first = first < 0 ? x : first;
                last = x;
            }
        }
        int phase = ((first + gfx.camera_x) % 8 + 8) % 8;
        rows += first >= 0 && phase + (last - first + 1) > 128;
    }
    return rows;
}

static void test_a_wide_patterned_disc_costs_one_more_sprt_per_row_to_split(void) {
    unsigned seed = 77;
    for (int i = 0; i < 60; i++) {
        int values[5];
        for (int v = 0; v < 5; v++) {
            seed = seed * 1103515245u + 12345u;
            values[v] = (int)((seed >> 8) % 300) - 70;
        }
        reset();
        p8_camera(&gfx, values[3] / 2, values[4] / 2);
        int r = 55 + (values[2] + 70) % 120;
        int plain = p8_draw_cost_circfill(&window, &gfx, values[0], values[1], r, 0x15).primitives;
        p8_fillp(&gfx, 0x5A5A);
        (void)p8_draw_cost_rectfill(&window, &gfx, gfx.camera_x, gfx.camera_y, gfx.camera_x, gfx.camera_y, 0x15);
        int patterned = p8_draw_cost_circfill(&window, &gfx, values[0], values[1], r, 0x15).primitives;
        assert(patterned == plain + rows_to_split(values[0], values[1], r));
    }
}

static void test_alternating_spr_and_print_costs_a_pair_each_switch(void) {
    reset();
    assert_cost(p8_draw_cost_spr(&window, &gfx, 1, 0, 0, 1, 1), P8_GPU_SPRT_BYTES, 1);
    assert_cost(p8_draw_cost_print(&window, &gfx, "ab", 0, 0), 2 * P8_GPU_SPRT_BYTES + WINDOW_PAIR_BYTES, 4);
    assert_cost(p8_draw_cost_print(&window, &gfx, "c", 0, 0), P8_GPU_SPRT_BYTES, 1);
    assert_cost(p8_draw_cost_spr(&window, &gfx, 1, 0, 0, 1, 1), P8_GPU_SPRT_BYTES + WINDOW_PAIR_BYTES, 3);
    assert_cost(p8_draw_cost_spr(&window, &gfx, 2, 8, 0, 1, 1), P8_GPU_SPRT_BYTES, 1);
    assert_cost(p8_draw_cost_print(&window, &gfx, "a", 0, 0), P8_GPU_SPRT_BYTES + WINDOW_PAIR_BYTES, 3);
}

static void test_culled_spr_and_print_keep_the_texture(void) {
    reset();
    assert_cost(p8_draw_cost_print(&window, &gfx, "a", 0, -50), 0, 0);
    assert_cost(p8_draw_cost_print(&window, &gfx, " ", 0, 0), 0, 0);
    assert_cost(p8_draw_cost_spr(&window, &gfx, 1, 0, 0, 1, 1), P8_GPU_SPRT_BYTES, 1);
    assert_cost(p8_draw_cost_print(&window, &gfx, "a", 0, 0), P8_GPU_SPRT_BYTES + WINDOW_PAIR_BYTES, 3);
    assert_cost(p8_draw_cost_spr(&window, &gfx, 1, -50, 0, 1, 1), 0, 0);
    assert_cost(p8_draw_cost_spr(&window, &gfx, 1, 0, 0, 0, 1), 0, 0);
    assert_cost(p8_draw_cost_print(&window, &gfx, "a", 0, 0), P8_GPU_SPRT_BYTES, 1);
}

static void test_untextured_and_culled_calls_keep_the_window(void) {
    reset();
    p8_fillp(&gfx, 0xA5A5);
    assert_cost(p8_draw_cost_rectfill(&window, &gfx, 0, 0, 10, 10, 1), P8_GPU_SPRT_BYTES + WINDOW_PAIR_BYTES, 3);
    assert_cost(p8_draw_cost_line(&gfx, 0, 0, 5, 5), 6 * P8_GPU_TILE_BYTES, 6);
    assert_cost(p8_draw_cost_spr(&window, &gfx, 1, -50, 0, 1, 1), 0, 0);
    assert_cost(p8_draw_cost_rectfill(&window, &gfx, 0, 0, 10, 10, 1), P8_GPU_SPRT_BYTES, 1);
}

static void test_frame_begin_returns_the_overhead_and_resets_the_window(void) {
    reset();
    p8_fillp(&gfx, 0xA5A5);
    assert_cost(p8_draw_cost_rectfill(&window, &gfx, 0, 0, 10, 10, 1), P8_GPU_SPRT_BYTES + WINDOW_PAIR_BYTES, 3);
    assert_cost(p8_draw_cost_frame_begin(&window),
                P8_GPU_DR_ENV_BYTES + P8_GPU_DR_TPAGE_BYTES + P8_GPU_DR_TWIN_BYTES, 3);
    assert_cost(p8_draw_cost_rectfill(&window, &gfx, 0, 0, 10, 10, 1), P8_GPU_SPRT_BYTES + WINDOW_PAIR_BYTES, 3);
}

static void test_frame_begins_on_the_sprite_sheet(void) {
    reset();
    assert_cost(p8_draw_cost_print(&window, &gfx, "a", 0, 0), P8_GPU_SPRT_BYTES + WINDOW_PAIR_BYTES, 3);
    (void)p8_draw_cost_frame_begin(&window);
    assert_cost(p8_draw_cost_spr(&window, &gfx, 1, 0, 0, 1, 1), P8_GPU_SPRT_BYTES, 1);
    (void)p8_draw_cost_frame_begin(&window);
    assert_cost(p8_draw_cost_print(&window, &gfx, "a", 0, 0), P8_GPU_SPRT_BYTES + WINDOW_PAIR_BYTES, 3);
}

static void test_pattern_key_tells_pattern_colour_and_transparency_apart(void) {
    reset();
    p8_fillp(&gfx, 0xA5A5);
    unsigned long key = p8_draw_cost_pattern_key(&gfx, 0x15);
    assert(p8_draw_cost_pattern_key(&gfx, 0x115) == key && "only the colour byte counts");
    assert(p8_draw_cost_pattern_key(&gfx, 0x16) != key);
    p8_fillp_transparent(&gfx, 0xA5A5);
    assert(p8_draw_cost_pattern_key(&gfx, 0x15) != key);
    p8_fillp(&gfx, 0x0F0F);
    assert(p8_draw_cost_pattern_key(&gfx, 0x15) != key);
}

int main(void) {
    test_cls_is_one_tile();
    test_pset_is_one_tile_and_off_screen_is_free();
    test_culling_is_judged_after_the_camera();
    test_a_line_costs_one_tile_per_pixel_the_software_lights();
    test_random_lines_cost_what_the_software_lights();
    test_rect_is_four_tiles_and_straddling_costs_in_full();
    test_plain_rectfill_is_one_tile();
    test_circles_cost_per_midpoint_iteration();
    test_spr_is_one_sprt_whatever_its_cells();
    test_print_is_one_sprt_per_glyph_but_spaces();
    test_patterned_fill_is_a_sprt_with_its_window_pair_once();
    test_spr_and_print_reset_the_window_after_a_pattern();
    test_a_pattern_after_print_costs_the_pair_although_they_share_a_page();
    test_a_patterned_span_costs_one_sprt_per_piece_under_u_256();
    test_a_patterned_disc_costs_one_more_sprt_per_row_under_u_256();
    test_a_wide_patterned_disc_costs_one_more_sprt_per_row_to_split();
    test_alternating_spr_and_print_costs_a_pair_each_switch();
    test_culled_spr_and_print_keep_the_texture();
    test_untextured_and_culled_calls_keep_the_window();
    test_frame_begin_returns_the_overhead_and_resets_the_window();
    test_frame_begins_on_the_sprite_sheet();
    test_pattern_key_tells_pattern_colour_and_transparency_apart();
    printf("test_draw_cost: ok\n");
    return 0;
}
