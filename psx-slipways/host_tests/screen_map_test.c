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

/* screen_map_unproject round-trip tolerance. Pixel coordinates are
 * integers, so screen_map_planet's pixel = MARGIN + (normalized*usable)/
 * FX_ONE step quantizes world-space to one pixel's worth of world units
 * per axis; screen_map_unproject's own reverse computation truncates
 * again on the way back. One pixel of X spans SPAN/(SCREEN_XRES-2*MARGIN)
 * = (2.75*65536)/304 =~ 592.8 fixed-point units; one pixel of Y spans
 * SPAN/(SCREEN_YRES-2*MARGIN) = (2.75*65536)/224 =~ 804.6. The Y axis has
 * the coarser step (fewer usable pixels for the same world span), so it
 * sets the bound. An exhaustive sweep over every representable fixed_t
 * coordinate in the placement domain (see
 * psx-slipways/build-host/roundtrip_check.c, a throwaway verification
 * program, not part of the test suite) found the true worst-case
 * round-trip error is 597 units on X and 809 units on Y -- i.e. almost
 * exactly one axis-pixel's worth of world units, as expected, plus a
 * small amount of extra slop from the second truncating division on the
 * way back. SCREEN_MAP_UNPROJECT_TOLERANCE is set just above that
 * exhaustively-verified Y-axis bound (the coarser, binding one), not a
 * round guess. */
#define SCREEN_MAP_UNPROJECT_TOLERANCE ((fixed_t)850)

static fixed_t fixed_abs_diff(fixed_t a, fixed_t b) {
    fixed_t d = fixed_sub(a, b);
    return d < 0 ? -d : d;
}

static void test_unproject_round_trips_screen_map_planet_for_center(void) {
    fixed_t half_span = PLANETGEN_PLACEMENT_SPAN / 2;
    planet_pos_t original = pos(fixed_add(PLANETGEN_PLACEMENT_MIN, half_span),
                                 fixed_add(PLANETGEN_PLACEMENT_MIN, half_span));

    int x, y;
    screen_map_planet(original, &x, &y);
    planet_pos_t back = screen_map_unproject(x, y);

    T_CHECK(fixed_abs_diff(back.x, original.x) <= SCREEN_MAP_UNPROJECT_TOLERANCE);
    T_CHECK(fixed_abs_diff(back.y, original.y) <= SCREEN_MAP_UNPROJECT_TOLERANCE);
}

static void test_unproject_round_trips_screen_map_planet_across_a_spread_of_positions(void) {
    fixed_t low = PLANETGEN_PLACEMENT_MIN;
    fixed_t high = fixed_add(PLANETGEN_PLACEMENT_MIN, PLANETGEN_PLACEMENT_SPAN);
    fixed_t quarter = fixed_add(PLANETGEN_PLACEMENT_MIN, PLANETGEN_PLACEMENT_SPAN / 4);
    fixed_t three_quarter = fixed_add(PLANETGEN_PLACEMENT_MIN, (PLANETGEN_PLACEMENT_SPAN * 3) / 4);

    /* Corners, center, and a couple of interior points, mixed per axis so
     * X and Y don't always move together. */
    planet_pos_t positions[] = {
        pos(low, low),
        pos(low, high),
        pos(high, low),
        pos(high, high),
        pos(fixed_add(low, PLANETGEN_PLACEMENT_SPAN / 2), fixed_add(low, PLANETGEN_PLACEMENT_SPAN / 2)),
        pos(quarter, three_quarter),
        pos(three_quarter, quarter),
    };
    int n = (int)(sizeof(positions) / sizeof(positions[0]));

    for (int i = 0; i < n; i++) {
        int x, y;
        screen_map_planet(positions[i], &x, &y);
        planet_pos_t back = screen_map_unproject(x, y);

        T_CHECK(fixed_abs_diff(back.x, positions[i].x) <= SCREEN_MAP_UNPROJECT_TOLERANCE);
        T_CHECK(fixed_abs_diff(back.y, positions[i].y) <= SCREEN_MAP_UNPROJECT_TOLERANCE);
    }
}

int main(void) {
    t_run("low_corner_maps_to_margin", test_low_corner_maps_to_margin);
    t_run("high_corner_maps_to_resolution_minus_margin", test_high_corner_maps_to_resolution_minus_margin);
    t_run("mixed_corners_map_independently_per_axis", test_mixed_corners_map_independently_per_axis);
    t_run("mapping_is_monotonic_per_axis", test_mapping_is_monotonic_per_axis);
    t_run("mapped_coordinates_stay_in_bounds_no_overflow", test_mapped_coordinates_stay_in_bounds_no_overflow);
    t_run("unproject_round_trips_screen_map_planet_for_center", test_unproject_round_trips_screen_map_planet_for_center);
    t_run("unproject_round_trips_screen_map_planet_across_a_spread_of_positions", test_unproject_round_trips_screen_map_planet_across_a_spread_of_positions);
    return t_report();
}
