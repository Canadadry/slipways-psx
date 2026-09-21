/* Phase 5, Etape 1, Chunk 3 (docs/prd/triage/phase5-etape1-raylib-
 * interactive-backend.md): unit tests for the one pure, raylib-free
 * function this chunk's own Testing Decisions call out explicitly --
 * known window-pixel inputs at a known fixed scale mapping to known
 * 128x128 outputs, including the clamping boundary. Links only against
 * pico8_raylib_backend_coords.c, never raylib itself, so it can run
 * headlessly as part of the existing `unit`/`test` targets with none of
 * that fast day-to-day suite's window-free/raylib-free guarantee lost. */

#include "../src/pico8_raylib_backend_coords.h"

#include <assert.h>
#include <stdio.h>

static void test_scale_six_origin_maps_to_origin(void) {
    int x;
    int y;
    pico8_raylib_backend_window_to_pico8(0, 0, 6, &x, &y);
    assert(x == 0);
    assert(y == 0);
}

static void test_scale_six_known_pixel_maps_to_known_coordinate(void) {
    int x;
    int y;
    pico8_raylib_backend_window_to_pico8(60, 300, 6, &x, &y);
    assert(x == 10);
    assert(y == 50);
}

static void test_scale_six_max_in_window_pixel_maps_to_max_coordinate(void) {
    int x;
    int y;
    pico8_raylib_backend_window_to_pico8(767, 767, 6, &x, &y);
    assert(x == 127);
    assert(y == 127);
}

static void test_scale_six_beyond_window_clamps_to_127(void) {
    int x;
    int y;
    pico8_raylib_backend_window_to_pico8(1000, 5000, 6, &x, &y);
    assert(x == 127);
    assert(y == 127);
}

static void test_scale_six_negative_pixel_clamps_to_zero(void) {
    int x;
    int y;
    pico8_raylib_backend_window_to_pico8(-30, -1, 6, &x, &y);
    assert(x == 0);
    assert(y == 0);
}

static void test_different_scale_factor_divides_accordingly(void) {
    int x;
    int y;
    pico8_raylib_backend_window_to_pico8(40, 40, 4, &x, &y);
    assert(x == 10);
    assert(y == 10);
}

int main(void) {
    test_scale_six_origin_maps_to_origin();
    test_scale_six_known_pixel_maps_to_known_coordinate();
    test_scale_six_max_in_window_pixel_maps_to_max_coordinate();
    test_scale_six_beyond_window_clamps_to_127();
    test_scale_six_negative_pixel_clamps_to_zero();
    test_different_scale_factor_divides_accordingly();

    printf("test_pico8_raylib_backend_coords: all tests passed\n");
    return 0;
}
