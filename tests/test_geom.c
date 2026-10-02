#include "engine/geom.h"
#include "devtools/fixed_test_double.h"

#include <assert.h>
#include <stdio.h>

static void test_side_positive_side(void) {
    vec_t a = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    vec_t b = vec_make(p8_fixed_from_int(1), p8_fixed_from_int(0));
    vec_t c = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(-1));
    assert(geom_side_of_line(a, b, c) == SIDE_POSITIVE);
}

static void test_side_negative_side(void) {
    vec_t a = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    vec_t b = vec_make(p8_fixed_from_int(1), p8_fixed_from_int(0));
    vec_t c = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(1));
    assert(geom_side_of_line(a, b, c) == SIDE_NEGATIVE);
}

static void test_side_on_line_is_degenerate_collinear(void) {
    vec_t a = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    vec_t b = vec_make(p8_fixed_from_int(1), p8_fixed_from_int(0));
    vec_t c = vec_make(p8_fixed_from_int(2), p8_fixed_from_int(0));
    assert(geom_side_of_line(a, b, c) == SIDE_ON_LINE);
}

static void test_segments_cross_clear_crossing_case(void) {
    vec_t p1 = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    vec_t p2 = vec_make(p8_fixed_from_int(2), p8_fixed_from_int(2));
    vec_t q1 = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(2));
    vec_t q2 = vec_make(p8_fixed_from_int(2), p8_fixed_from_int(0));
    assert(geom_segments_cross(p1, p2, q1, q2));
}

static void test_segments_cross_clear_non_crossing_case(void) {
    vec_t p1 = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    vec_t p2 = vec_make(p8_fixed_from_int(1), p8_fixed_from_int(0));
    vec_t q1 = vec_make(p8_fixed_from_int(5), p8_fixed_from_int(5));
    vec_t q2 = vec_make(p8_fixed_from_int(6), p8_fixed_from_int(6));
    assert(!geom_segments_cross(p1, p2, q1, q2));
}

static void test_near_segment_within_margin_and_within_distance_is_true(void) {
    vec_t a = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    vec_t b = vec_make(p8_fixed_from_int(10), p8_fixed_from_int(0));
    vec_t c = vec_make(fixed_test_double_from_double(0.5), p8_fixed_from_int(0));
    p8_fixed_t fh = fixed_test_double_from_double(1.0);
    assert(geom_near_segment(a, b, c, fh));
}

static void test_near_segment_outside_margin_near_start_is_false(void) {
    vec_t a = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    vec_t b = vec_make(p8_fixed_from_int(10), p8_fixed_from_int(0));
    vec_t c = vec_make(fixed_test_double_from_double(0.05), p8_fixed_from_int(0));
    p8_fixed_t fh = fixed_test_double_from_double(1.0);
    assert(!geom_near_segment(a, b, c, fh));
}

static void test_near_segment_outside_margin_near_end_is_false(void) {
    vec_t a = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    vec_t b = vec_make(p8_fixed_from_int(10), p8_fixed_from_int(0));
    vec_t c = vec_make(fixed_test_double_from_double(9.95), p8_fixed_from_int(0));
    p8_fixed_t fh = fixed_test_double_from_double(1.0);
    assert(!geom_near_segment(a, b, c, fh));
}

static void test_near_segment_within_margin_but_outside_distance_is_false(void) {
    vec_t a = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    vec_t b = vec_make(p8_fixed_from_int(10), p8_fixed_from_int(0));
    vec_t c = vec_make(p8_fixed_from_int(5), p8_fixed_from_int(3));
    p8_fixed_t fh = fixed_test_double_from_double(4.0);
    assert(!geom_near_segment(a, b, c, fh));
}

int main(void) {
    test_side_positive_side();
    test_side_negative_side();
    test_side_on_line_is_degenerate_collinear();
    test_segments_cross_clear_crossing_case();
    test_segments_cross_clear_non_crossing_case();
    test_near_segment_within_margin_and_within_distance_is_true();
    test_near_segment_outside_margin_near_start_is_false();
    test_near_segment_outside_margin_near_end_is_false();
    test_near_segment_within_margin_but_outside_distance_is_false();

    printf("test_geom: all tests passed\n");
    return 0;
}
