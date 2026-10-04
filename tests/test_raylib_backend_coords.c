/* Links against the coordinate module alone and never raylib, so it stays in
 * the window-free suite. The screen size comes through the module's own
 * header. */

#include "platform/raylib_backend_coords.h"

#include <assert.h>
#include <stdio.h>

static void test_scale_six_origin_maps_to_origin(void) {
    int x;
    int y;
    raylib_backend_window_to_screen(0, 0, 6, &x, &y);
    assert(x == 0);
    assert(y == 0);
}

static void test_scale_six_known_pixel_maps_to_known_coordinate(void) {
    int x;
    int y;
    raylib_backend_window_to_screen(60, 300, 6, &x, &y);
    assert(x == 10);
    assert(y == 50);
}

static void test_scale_six_max_in_window_pixel_maps_to_last_pixel_of_each_axis(void) {
    int x;
    int y;
    raylib_backend_window_to_screen(P8_SCREEN_WIDTH * 6 - 1, P8_SCREEN_HEIGHT * 6 - 1, 6, &x, &y);
    assert(x == P8_SCREEN_WIDTH - 1);
    assert(y == P8_SCREEN_HEIGHT - 1);
}

static void test_scale_six_beyond_window_clamps_each_axis_to_its_last_pixel(void) {
    int x;
    int y;
    raylib_backend_window_to_screen(P8_SCREEN_WIDTH * 6, P8_SCREEN_HEIGHT * 6, 6, &x, &y);
    assert(x == P8_SCREEN_WIDTH - 1);
    assert(y == P8_SCREEN_HEIGHT - 1);
    raylib_backend_window_to_screen(5000, 5000, 6, &x, &y);
    assert(x == P8_SCREEN_WIDTH - 1);
    assert(y == P8_SCREEN_HEIGHT - 1);
}

static void test_scale_six_window_is_960x720_onto_160x120(void) {
    int x;
    int y;
    raylib_backend_window_to_screen(959, 719, 6, &x, &y);
    assert(x == 159);
    assert(y == 119);
    raylib_backend_window_to_screen(960, 720, 6, &x, &y);
    assert(x == 159);
    assert(y == 119);
}

static void test_scale_six_negative_pixel_clamps_to_zero(void) {
    int x;
    int y;
    raylib_backend_window_to_screen(-30, -1, 6, &x, &y);
    assert(x == 0);
    assert(y == 0);
}

static void test_different_scale_factor_divides_accordingly(void) {
    int x;
    int y;
    raylib_backend_window_to_screen(40, 40, 4, &x, &y);
    assert(x == 10);
    assert(y == 10);
}

int main(void) {
    test_scale_six_origin_maps_to_origin();
    test_scale_six_known_pixel_maps_to_known_coordinate();
    test_scale_six_max_in_window_pixel_maps_to_last_pixel_of_each_axis();
    test_scale_six_beyond_window_clamps_each_axis_to_its_last_pixel();
    test_scale_six_window_is_960x720_onto_160x120();
    test_scale_six_negative_pixel_clamps_to_zero();
    test_different_scale_factor_divides_accordingly();

    printf("test_raylib_backend_coords: all tests passed\n");
    return 0;
}
