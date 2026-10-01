#include "engine/geom.h"
#include "devtools/fixed_test_double.h"

#include <assert.h>
#include <stdio.h>

static void test_ee_positive_side(void) {
    pico8_vec_t a = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    pico8_vec_t b = pico8_vec_v(pico8_fixed_from_int(1), pico8_fixed_from_int(0));
    pico8_vec_t c = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(-1));
    assert(pico8_geom_ee(a, b, c) == PICO8_EE_POSITIVE);
}

static void test_ee_negative_side(void) {
    pico8_vec_t a = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    pico8_vec_t b = pico8_vec_v(pico8_fixed_from_int(1), pico8_fixed_from_int(0));
    pico8_vec_t c = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(1));
    assert(pico8_geom_ee(a, b, c) == PICO8_EE_NEGATIVE);
}

static void test_ee_on_line_is_degenerate_collinear(void) {
    pico8_vec_t a = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    pico8_vec_t b = pico8_vec_v(pico8_fixed_from_int(1), pico8_fixed_from_int(0));
    pico8_vec_t c = pico8_vec_v(pico8_fixed_from_int(2), pico8_fixed_from_int(0));
    assert(pico8_geom_ee(a, b, c) == PICO8_EE_ON_LINE);
}

static void test_cl_clear_crossing_case(void) {
    pico8_vec_t p1 = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    pico8_vec_t p2 = pico8_vec_v(pico8_fixed_from_int(2), pico8_fixed_from_int(2));
    pico8_vec_t q1 = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(2));
    pico8_vec_t q2 = pico8_vec_v(pico8_fixed_from_int(2), pico8_fixed_from_int(0));
    assert(pico8_geom_cl(p1, p2, q1, q2));
}

static void test_cl_clear_non_crossing_case(void) {
    pico8_vec_t p1 = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    pico8_vec_t p2 = pico8_vec_v(pico8_fixed_from_int(1), pico8_fixed_from_int(0));
    pico8_vec_t q1 = pico8_vec_v(pico8_fixed_from_int(5), pico8_fixed_from_int(5));
    pico8_vec_t q2 = pico8_vec_v(pico8_fixed_from_int(6), pico8_fixed_from_int(6));
    assert(!pico8_geom_cl(p1, p2, q1, q2));
}

static void test_be_within_margin_and_within_distance_is_true(void) {
    pico8_vec_t a = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    pico8_vec_t b = pico8_vec_v(pico8_fixed_from_int(10), pico8_fixed_from_int(0));
    pico8_vec_t c = pico8_vec_v(pico8_fixed_from_double(0.5), pico8_fixed_from_int(0));
    pico8_fixed_t fh = pico8_fixed_from_double(1.0);
    assert(pico8_geom_be(a, b, c, fh));
}

static void test_be_outside_margin_near_start_is_false(void) {
    pico8_vec_t a = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    pico8_vec_t b = pico8_vec_v(pico8_fixed_from_int(10), pico8_fixed_from_int(0));
    pico8_vec_t c = pico8_vec_v(pico8_fixed_from_double(0.05), pico8_fixed_from_int(0));
    pico8_fixed_t fh = pico8_fixed_from_double(1.0);
    assert(!pico8_geom_be(a, b, c, fh));
}

static void test_be_outside_margin_near_end_is_false(void) {
    pico8_vec_t a = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    pico8_vec_t b = pico8_vec_v(pico8_fixed_from_int(10), pico8_fixed_from_int(0));
    pico8_vec_t c = pico8_vec_v(pico8_fixed_from_double(9.95), pico8_fixed_from_int(0));
    pico8_fixed_t fh = pico8_fixed_from_double(1.0);
    assert(!pico8_geom_be(a, b, c, fh));
}

static void test_be_within_margin_but_outside_distance_is_false(void) {
    pico8_vec_t a = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    pico8_vec_t b = pico8_vec_v(pico8_fixed_from_int(10), pico8_fixed_from_int(0));
    pico8_vec_t c = pico8_vec_v(pico8_fixed_from_int(5), pico8_fixed_from_int(3));
    pico8_fixed_t fh = pico8_fixed_from_double(4.0);
    assert(!pico8_geom_be(a, b, c, fh));
}

int main(void) {
    test_ee_positive_side();
    test_ee_negative_side();
    test_ee_on_line_is_degenerate_collinear();
    test_cl_clear_crossing_case();
    test_cl_clear_non_crossing_case();
    test_be_within_margin_and_within_distance_is_true();
    test_be_outside_margin_near_start_is_false();
    test_be_outside_margin_near_end_is_false();
    test_be_within_margin_but_outside_distance_is_false();

    printf("test_pico8_geom: all tests passed\n");
    return 0;
}
