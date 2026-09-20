#include "../src/pico8_gfx.h"
#include "../src/pico8_mem.h"

#include <assert.h>
#include <stdio.h>

static void test_cls_clears_whole_screen_to_zero(void) {
    pico8_mem_t mem;
    pico8_mem_init(&mem);

    for (int i = 0; i < PICO8_SCREEN_WIDTH * PICO8_SCREEN_HEIGHT; i++) {
        pico8_poke(&mem, PICO8_ADDR_SCREEN + i, 9);
    }

    pico8_cls(&mem);

    for (int i = 0; i < PICO8_SCREEN_WIDTH * PICO8_SCREEN_HEIGHT; i++) {
        assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + i) == 0);
    }
}

static void test_pset_plots_pixel_at_coordinate(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);

    pico8_pset(&gfx, &mem, 10, 20, 7);

    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 20 * PICO8_SCREEN_WIDTH + 10) == 7);
    /* Neighboring pixels are untouched. */
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 20 * PICO8_SCREEN_WIDTH + 11) == 0);
}

static void test_pset_off_screen_is_clipped_silently(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);

    pico8_pset(&gfx, &mem, -1, 0, 7);
    pico8_pset(&gfx, &mem, 0, -1, 7);
    pico8_pset(&gfx, &mem, PICO8_SCREEN_WIDTH, 0, 7);
    pico8_pset(&gfx, &mem, 0, PICO8_SCREEN_HEIGHT, 7);

    for (int i = 0; i < PICO8_SCREEN_WIDTH * PICO8_SCREEN_HEIGHT; i++) {
        assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + i) == 0);
    }
}

static void test_camera_offset_shifts_subsequent_pset(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);

    pico8_camera(&gfx, 10, 5);
    pico8_pset(&gfx, &mem, 15, 15, 7);

    /* camera(10,5) shifts the world left/up by (10,5): world (15,15)
     * lands at screen (5,10). */
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 10 * PICO8_SCREEN_WIDTH + 5) == 7);
    /* It did NOT land at the unshifted coordinate. */
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 15 * PICO8_SCREEN_WIDTH + 15) == 0);
}

static void test_camera_reset_restores_zero_offset(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);

    pico8_camera(&gfx, 10, 5);
    pico8_camera_reset(&gfx);
    pico8_pset(&gfx, &mem, 3, 4, 6);

    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 4 * PICO8_SCREEN_WIDTH + 3) == 6);
}

static void test_gfx_init_default_transparency_is_color_0_only(void) {
    pico8_gfx_t gfx;
    pico8_gfx_init(&gfx);

    assert(pico8_gfx_is_transparent(&gfx, 0));
    for (int c = 1; c < 16; c++) {
        assert(!pico8_gfx_is_transparent(&gfx, c));
    }
}

static void test_palt_marks_color_transparent_and_opaque(void) {
    pico8_gfx_t gfx;
    pico8_gfx_init(&gfx);

    pico8_palt(&gfx, 3, 1);
    assert(pico8_gfx_is_transparent(&gfx, 3));

    pico8_palt(&gfx, 0, 0);
    assert(!pico8_gfx_is_transparent(&gfx, 0));

    pico8_palt(&gfx, 3, 0);
    assert(!pico8_gfx_is_transparent(&gfx, 3));
}

static void test_palt_out_of_range_col_is_noop_and_queries_as_opaque(void) {
    pico8_gfx_t gfx;
    pico8_gfx_init(&gfx);

    pico8_palt(&gfx, -1, 1);
    pico8_palt(&gfx, 16, 1);

    assert(!pico8_gfx_is_transparent(&gfx, -1));
    assert(!pico8_gfx_is_transparent(&gfx, 16));
}

static void test_palt_affects_a_subsequent_transparent_color_draw(void) {
    /* pset itself never consults palt (matches real PICO-8: transparency
     * only affects sprite-drawing primitives, added in a later chunk).
     * This exercises the "does this pixel draw" query a later chunk's
     * spr() will call before plotting each source pixel. */
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);

    pico8_palt(&gfx, 3, 1);

    int col = 3;
    if (!pico8_gfx_is_transparent(&gfx, col)) {
        pico8_pset(&gfx, &mem, 5, 5, col);
    }
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 5 * PICO8_SCREEN_WIDTH + 5) == 0);

    pico8_palt(&gfx, 3, 0);
    if (!pico8_gfx_is_transparent(&gfx, col)) {
        pico8_pset(&gfx, &mem, 5, 5, col);
    }
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 5 * PICO8_SCREEN_WIDTH + 5) == 3);
}

static void test_line_draws_diagonal_pixels(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);

    pico8_line(&gfx, &mem, 2, 2, 5, 5, 8);

    for (int i = 2; i <= 5; i++) {
        assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + i * PICO8_SCREEN_WIDTH + i) == 8);
    }
    /* Off the diagonal is untouched. */
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 2 * PICO8_SCREEN_WIDTH + 3) == 0);
}

static void test_line_zero_length_draws_single_pixel(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);

    pico8_line(&gfx, &mem, 7, 9, 7, 9, 11);

    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 9 * PICO8_SCREEN_WIDTH + 7) == 11);
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 9 * PICO8_SCREEN_WIDTH + 8) == 0);
}

static void test_rect_draws_unfilled_outline(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);

    pico8_rect(&gfx, &mem, 2, 2, 5, 4, 8);

    /* Corners and edges are drawn. */
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 2 * PICO8_SCREEN_WIDTH + 2) == 8);
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 2 * PICO8_SCREEN_WIDTH + 5) == 8);
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 4 * PICO8_SCREEN_WIDTH + 2) == 8);
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 4 * PICO8_SCREEN_WIDTH + 5) == 8);
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 2 * PICO8_SCREEN_WIDTH + 3) == 8);
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 3 * PICO8_SCREEN_WIDTH + 2) == 8);
    /* Interior is untouched (unfilled). */
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 3 * PICO8_SCREEN_WIDTH + 3) == 0);
}

static void test_rect_single_pixel_draws_one_point(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);

    pico8_rect(&gfx, &mem, 6, 6, 6, 6, 12);

    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 6 * PICO8_SCREEN_WIDTH + 6) == 12);
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 6 * PICO8_SCREEN_WIDTH + 7) == 0);
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 7 * PICO8_SCREEN_WIDTH + 6) == 0);
}

static void test_rectfill_fills_every_pixel_in_bounds(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);

    pico8_rectfill(&gfx, &mem, 2, 2, 4, 3, 9);

    for (int y = 2; y <= 3; y++) {
        for (int x = 2; x <= 4; x++) {
            assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + y * PICO8_SCREEN_WIDTH + x) == 9);
        }
    }
    /* Just outside the rectangle is untouched. */
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 4 * PICO8_SCREEN_WIDTH + 2) == 0);
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 2 * PICO8_SCREEN_WIDTH + 5) == 0);
}

static void test_circ_draws_unfilled_outline(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);

    pico8_circ(&gfx, &mem, 10, 10, 3, 8);

    /* Cardinal points of the outline. */
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 10 * PICO8_SCREEN_WIDTH + 13) == 8);
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 10 * PICO8_SCREEN_WIDTH + 7) == 8);
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 13 * PICO8_SCREEN_WIDTH + 10) == 8);
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 7 * PICO8_SCREEN_WIDTH + 10) == 8);
    /* Center is untouched (unfilled). */
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 10 * PICO8_SCREEN_WIDTH + 10) == 0);
}

static void test_circ_zero_radius_draws_single_pixel(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);

    pico8_circ(&gfx, &mem, 5, 5, 0, 6);

    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 5 * PICO8_SCREEN_WIDTH + 5) == 6);
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 5 * PICO8_SCREEN_WIDTH + 6) == 0);
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 4 * PICO8_SCREEN_WIDTH + 5) == 0);
}

static void test_circfill_fills_the_disc(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);

    pico8_circfill(&gfx, &mem, 10, 10, 3, 8);

    /* Center and outline are both filled. */
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 10 * PICO8_SCREEN_WIDTH + 10) == 8);
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 10 * PICO8_SCREEN_WIDTH + 13) == 8);
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 10 * PICO8_SCREEN_WIDTH + 7) == 8);
    /* Well outside the disc is untouched. */
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 10 * PICO8_SCREEN_WIDTH + 14) == 0);
}

static void test_circfill_zero_radius_draws_single_pixel(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);

    pico8_circfill(&gfx, &mem, 5, 5, 0, 6);

    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 5 * PICO8_SCREEN_WIDTH + 5) == 6);
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 5 * PICO8_SCREEN_WIDTH + 6) == 0);
    assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 4 * PICO8_SCREEN_WIDTH + 5) == 0);
}

static void test_fillp_pattern_alters_rectfill_output(void) {
    pico8_mem_t mem;
    pico8_gfx_t gfx;
    pico8_mem_init(&mem);
    pico8_gfx_init(&gfx);

    /* Rows alternate solid/skip in groups of 4: row%4==0..2 draws (bits
     * clear), row%4==3 skips (bits set) -- 0b0000000000001111 marks only
     * the pattern's last row as "skip". */
    pico8_fillp(&gfx, 0x000F);
    pico8_rectfill(&gfx, &mem, 0, 0, 3, 3, 8);

    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 4; x++) {
            assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + y * PICO8_SCREEN_WIDTH + x) == 8);
        }
    }
    for (int x = 0; x < 4; x++) {
        assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + 3 * PICO8_SCREEN_WIDTH + x) == 0);
    }

    /* fillp() with no pattern resets to solid (always draw). */
    pico8_fillp_reset(&gfx);
    pico8_rectfill(&gfx, &mem, 0, 0, 3, 3, 8);
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            assert(pico8_peek(&mem, PICO8_ADDR_SCREEN + y * PICO8_SCREEN_WIDTH + x) == 8);
        }
    }
}

int main(void) {
    test_cls_clears_whole_screen_to_zero();
    test_pset_plots_pixel_at_coordinate();
    test_pset_off_screen_is_clipped_silently();
    test_camera_offset_shifts_subsequent_pset();
    test_camera_reset_restores_zero_offset();
    test_gfx_init_default_transparency_is_color_0_only();
    test_palt_marks_color_transparent_and_opaque();
    test_palt_out_of_range_col_is_noop_and_queries_as_opaque();
    test_palt_affects_a_subsequent_transparent_color_draw();
    test_line_draws_diagonal_pixels();
    test_line_zero_length_draws_single_pixel();
    test_rect_draws_unfilled_outline();
    test_rect_single_pixel_draws_one_point();
    test_rectfill_fills_every_pixel_in_bounds();
    test_circ_draws_unfilled_outline();
    test_circ_zero_radius_draws_single_pixel();
    test_circfill_fills_the_disc();
    test_circfill_zero_radius_draws_single_pixel();
    test_fillp_pattern_alters_rectfill_output();

    printf("test_pico8_gfx: all tests passed\n");
    return 0;
}
