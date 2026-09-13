#include "t.h"
#include "../fixed.h"
#include "../planetgen.h"
#include "../screen.h"
#include "../screen_map.h"

#define SCREEN_MAP_MARGIN 8

static planet_pos_t pos(fixed_t x, fixed_t y) {
    planet_pos_t p;
    p.x = x;
    p.y = y;
    return p;
}

static void test_low_corner_maps_to_margin(void) {
    int x, y;
    screen_map_planet(pos(PLANETGEN_PLACEMENT_MIN, PLANETGEN_PLACEMENT_MIN), &x, &y);

    T_CHECK(x == SCREEN_MAP_MARGIN);
    T_CHECK(y == SCREEN_MAP_MARGIN);
}

static void test_high_corner_maps_to_resolution_minus_margin(void) {
    fixed_t high = fixed_add(PLANETGEN_PLACEMENT_MIN, PLANETGEN_PLACEMENT_SPAN);

    int x, y;
    screen_map_planet(pos(high, high), &x, &y);

    T_CHECK(x == SCREEN_XRES - SCREEN_MAP_MARGIN);
    T_CHECK(y == SCREEN_YRES - SCREEN_MAP_MARGIN);
}

static void test_mixed_corners_map_independently_per_axis(void) {
    fixed_t low = PLANETGEN_PLACEMENT_MIN;
    fixed_t high = fixed_add(PLANETGEN_PLACEMENT_MIN, PLANETGEN_PLACEMENT_SPAN);

    int x, y;

    screen_map_planet(pos(low, high), &x, &y);
    T_CHECK(x == SCREEN_MAP_MARGIN);
    T_CHECK(y == SCREEN_YRES - SCREEN_MAP_MARGIN);

    screen_map_planet(pos(high, low), &x, &y);
    T_CHECK(x == SCREEN_XRES - SCREEN_MAP_MARGIN);
    T_CHECK(y == SCREEN_MAP_MARGIN);
}

static void test_mapping_is_monotonic_per_axis(void) {
    int prev_x = -1;
    int prev_y = -1;

    for (int step = 0; step <= 8; step++) {
        fixed_t offset = (fixed_t)(((int64_t)PLANETGEN_PLACEMENT_SPAN * step) / 8);
        fixed_t coord = fixed_add(PLANETGEN_PLACEMENT_MIN, offset);

        int x, y;
        screen_map_planet(pos(coord, coord), &x, &y);

        T_CHECK(x >= prev_x);
        T_CHECK(y >= prev_y);
        prev_x = x;
        prev_y = y;
    }
}

static void test_mapped_coordinates_stay_in_bounds_no_overflow(void) {
    fixed_t low = PLANETGEN_PLACEMENT_MIN;
    fixed_t high = fixed_add(PLANETGEN_PLACEMENT_MIN, PLANETGEN_PLACEMENT_SPAN);

    int x, y;

    screen_map_planet(pos(low, low), &x, &y);
    T_CHECK(x >= 0 && x < SCREEN_XRES);
    T_CHECK(y >= 0 && y < SCREEN_YRES);

    screen_map_planet(pos(high, high), &x, &y);
    T_CHECK(x >= 0 && x < SCREEN_XRES);
    T_CHECK(y >= 0 && y < SCREEN_YRES);

    fixed_t mid = fixed_add(PLANETGEN_PLACEMENT_MIN, PLANETGEN_PLACEMENT_SPAN / 2);
    screen_map_planet(pos(mid, mid), &x, &y);
    T_CHECK(x >= 0 && x < SCREEN_XRES);
    T_CHECK(y >= 0 && y < SCREEN_YRES);
}

int main(void) {
    t_run("low_corner_maps_to_margin", test_low_corner_maps_to_margin);
    t_run("high_corner_maps_to_resolution_minus_margin", test_high_corner_maps_to_resolution_minus_margin);
    t_run("mixed_corners_map_independently_per_axis", test_mixed_corners_map_independently_per_axis);
    t_run("mapping_is_monotonic_per_axis", test_mapping_is_monotonic_per_axis);
    t_run("mapped_coordinates_stay_in_bounds_no_overflow", test_mapped_coordinates_stay_in_bounds_no_overflow);
    return t_report();
}
