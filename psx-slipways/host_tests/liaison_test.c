#include <stdarg.h>

#include "t.h"
#include "../fixed.h"
#include "../liaison.h"
#include "../planetgen.h"
#include "../screen_map.h"

static planet_pos_t pos(double x, double y) {
    planet_pos_t p;
    p.x = fixed_from_double(x);
    p.y = fixed_from_double(y);
    return p;
}

static void test_point_at_segment_midpoint_within_tolerance_blocks(void) {
    planet_pos_t a = pos(0.0, 0.0);
    planet_pos_t b = pos(1.0, 0.0);
    planet_pos_t c = pos(0.5, 0.0);

    int blocks = liaison_point_blocks_segment(a, b, c);

    T_CHECK(blocks != 0);
}

static void test_point_far_from_segment_does_not_block(void) {
    planet_pos_t a = pos(0.0, 0.0);
    planet_pos_t b = pos(1.0, 0.0);
    planet_pos_t c = pos(0.5, 1.0);

    int blocks = liaison_point_blocks_segment(a, b, c);

    T_CHECK(blocks == 0);
}

static void test_point_near_endpoint_outside_margin_does_not_block(void) {
    planet_pos_t a = pos(0.0, 0.0);
    planet_pos_t b = pos(1.0, 0.0);
    planet_pos_t c = pos(0.005, 0.0);

    int blocks = liaison_point_blocks_segment(a, b, c);

    T_CHECK(blocks == 0);
}

static void test_segments_that_clearly_cross_report_crossing(void) {
    planet_pos_t p1 = pos(0.0, 0.0);
    planet_pos_t p2 = pos(1.0, 1.0);
    planet_pos_t q1 = pos(0.0, 1.0);
    planet_pos_t q2 = pos(1.0, 0.0);

    int crosses = liaison_segments_cross(p1, p2, q1, q2);

    T_CHECK(crosses != 0);
}

static void test_segments_that_clearly_do_not_cross_report_no_crossing(void) {
    planet_pos_t p1 = pos(0.0, 0.0);
    planet_pos_t p2 = pos(1.0, 0.0);
    planet_pos_t q1 = pos(0.0, 1.0);
    planet_pos_t q2 = pos(1.0, 1.0);

    int crosses = liaison_segments_cross(p1, p2, q1, q2);

    T_CHECK(crosses == 0);
}

static void test_segments_sharing_an_endpoint_do_not_cross(void) {
    planet_pos_t p1 = pos(0.0, 0.0);
    planet_pos_t p2 = pos(1.0, 1.0);
    planet_pos_t q1 = pos(1.0, 1.0);
    planet_pos_t q2 = pos(2.0, 0.0);

    int crosses = liaison_segments_cross(p1, p2, q1, q2);

    T_CHECK(crosses == 0);
}

static galaxy_t make_galaxy(int count, ...) {
    galaxy_t g;
    g.count = count;
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++) {
        double x = va_arg(args, double);
        double y = va_arg(args, double);
        g.positions[i] = pos(x, y);
    }
    va_end(args);
    return g;
}

static void test_too_far_short_circuits_even_when_blocked_and_crossed(void) {
    /* from=(0,0), to=(2,0): distance^2 = 4.0, far beyond LIAISON_RANGE_SQ
     * (0.5). Planet 2 sits exactly at the segment's midpoint (would block
     * if evaluated) and route {3,4} crosses the segment at (1,0) (would
     * cross if evaluated) -- neither must be reached because the range
     * check must short-circuit first. */
    galaxy_t g = make_galaxy(5,
        0.0, 0.0,
        2.0, 0.0,
        1.0, 0.0,
        1.0, -1.0,
        1.0, 1.0);
    liaison_route_t routes[] = { { 3, 4 } };

    liaison_result_t r = liaison_check(0, g.positions[0], 1, g.positions[1], &g, routes, 1);

    T_CHECK(r.reason == LIAISON_TOO_FAR);
    T_CHECK(r.blocking_planet == -1);
    T_CHECK(r.blocking_route == -1);
}

static void test_already_connected_pair_reports_already_connected(void) {
    galaxy_t g = make_galaxy(2,
        0.0, 0.0,
        0.3, 0.0);
    liaison_route_t routes[] = { { 1, 0 } }; /* reverse order: symmetric lookup */

    liaison_result_t r = liaison_check(0, g.positions[0], 1, g.positions[1], &g, routes, 1);

    T_CHECK(r.reason == LIAISON_ALREADY_CONNECTED);
}

static void test_blocked_by_planet_reports_the_blocking_planet(void) {
    galaxy_t g = make_galaxy(3,
        0.0, 0.0,
        0.6, 0.0,
        0.3, 0.0);
    liaison_route_t routes[1] = { { 0, 0 } };

    liaison_result_t r = liaison_check(0, g.positions[0], 1, g.positions[1], &g, routes, 0);

    T_CHECK(r.reason == LIAISON_BLOCKED_BY_PLANET);
    T_CHECK(r.blocking_planet == 2);
}

static void test_crosses_route_reports_first_crossing_route_in_list_order(void) {
    /* from=0 (0,0), to=1 (0.6,0). route0={4,5} is far away, no crossing.
     * route1={2,3} crosses the candidate segment at (0.3,0). route2={1,6}
     * shares an endpoint with the candidate segment (the "to" planet) and
     * must not itself be treated as a crossing. Expect the scan to skip
     * route0, find route1 as the first true crossing, and never need to
     * evaluate route2. */
    galaxy_t g = make_galaxy(7,
        0.0, 0.0,
        0.6, 0.0,
        0.3, -1.0,
        0.3, 1.0,
        2.0, -1.0,
        2.0, 1.0,
        1.0, 1.0);
    liaison_route_t routes[] = {
        { 4, 5 },
        { 2, 3 },
        { 1, 6 },
    };

    liaison_result_t r = liaison_check(0, g.positions[0], 1, g.positions[1], &g, routes, 3);

    T_CHECK(r.reason == LIAISON_CROSSES_ROUTE);
    T_CHECK(r.blocking_route == 1);
}

static void test_no_obstruction_reports_valid(void) {
    galaxy_t g = make_galaxy(2,
        0.0, 0.0,
        0.4, 0.4);

    liaison_result_t r = liaison_check(0, g.positions[0], 1, g.positions[1], &g, NULL, 0);

    T_CHECK(r.reason == LIAISON_VALID);
    T_CHECK(r.blocking_planet == -1);
    T_CHECK(r.blocking_route == -1);
}

static void test_pick_planet_within_radius_returns_its_index(void) {
    /* Planet 0 sits at the placement domain's low corner, which
     * screen_map_planet maps exactly to (SCREEN_MAP_MARGIN,
     * SCREEN_MAP_MARGIN) -- see screen_map_test.c's own
     * low_corner_maps_to_margin case. Query right on top of it. */
    galaxy_t g = make_galaxy(1, 0.125, 0.125);
    int x, y;
    screen_map_planet(g.positions[0], &x, &y);

    int picked = liaison_pick_planet(x, y, &g, 8);

    T_CHECK(picked == 0);
}

static void test_pick_planet_outside_radius_returns_negative_one(void) {
    galaxy_t g = make_galaxy(1, 0.125, 0.125);
    int x, y;
    screen_map_planet(g.positions[0], &x, &y);

    /* 50px away in both axes, well outside an 8px pick radius. */
    int picked = liaison_pick_planet(x + 50, y + 50, &g, 8);

    T_CHECK(picked == -1);
}

static void test_pick_planet_nearest_wins_when_multiple_in_radius(void) {
    /* Two planets, both within a generous pick radius of the query point.
     * Planet 0 (galaxy index 0) is farther from the query point than
     * planet 1 (galaxy index 1) -- deliberately placing the FARTHER
     * planet EARLIER in index order, so a buggy "first planet found
     * in-range" implementation (scanning index order without tracking a
     * running minimum) would wrongly return index 0 instead of the
     * genuinely nearer index 1. */
    galaxy_t g = make_galaxy(2,
        0.3, 0.125,   /* index 0: farther from the query point below */
        0.125, 0.125  /* index 1: exactly at the query point */
    );
    int qx, qy;
    screen_map_planet(g.positions[1], &qx, &qy);

    int picked = liaison_pick_planet(qx, qy, &g, 100);

    T_CHECK(picked == 1);
}

int main(void) {
    t_run("point_at_segment_midpoint_within_tolerance_blocks", test_point_at_segment_midpoint_within_tolerance_blocks);
    t_run("point_far_from_segment_does_not_block", test_point_far_from_segment_does_not_block);
    t_run("point_near_endpoint_outside_margin_does_not_block", test_point_near_endpoint_outside_margin_does_not_block);
    t_run("segments_that_clearly_cross_report_crossing", test_segments_that_clearly_cross_report_crossing);
    t_run("segments_that_clearly_do_not_cross_report_no_crossing", test_segments_that_clearly_do_not_cross_report_no_crossing);
    t_run("segments_sharing_an_endpoint_do_not_cross", test_segments_sharing_an_endpoint_do_not_cross);
    t_run("too_far_short_circuits_even_when_blocked_and_crossed", test_too_far_short_circuits_even_when_blocked_and_crossed);
    t_run("already_connected_pair_reports_already_connected", test_already_connected_pair_reports_already_connected);
    t_run("blocked_by_planet_reports_the_blocking_planet", test_blocked_by_planet_reports_the_blocking_planet);
    t_run("crosses_route_reports_first_crossing_route_in_list_order", test_crosses_route_reports_first_crossing_route_in_list_order);
    t_run("no_obstruction_reports_valid", test_no_obstruction_reports_valid);
    t_run("pick_planet_within_radius_returns_its_index", test_pick_planet_within_radius_returns_its_index);
    t_run("pick_planet_outside_radius_returns_negative_one", test_pick_planet_outside_radius_returns_negative_one);
    t_run("pick_planet_nearest_wins_when_multiple_in_radius", test_pick_planet_nearest_wins_when_multiple_in_radius);
    return t_report();
}
