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

    printf("test_pico8_gfx: all tests passed\n");
    return 0;
}
