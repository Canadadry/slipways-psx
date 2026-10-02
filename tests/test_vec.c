#include "engine/vec.h"
#include "devtools/fixed_test_double.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

static void test_vec_make_constructs_from_components(void) {
    vec_t result = vec_make(pico8_fixed_from_double(1.5), pico8_fixed_from_double(-2.25));
    assert(result.x == pico8_fixed_from_double(1.5));
    assert(result.y == pico8_fixed_from_double(-2.25));
}

static void test_add_component_wise(void) {
    vec_t a = vec_make(pico8_fixed_from_double(1.0), pico8_fixed_from_double(2.0));
    vec_t b = vec_make(pico8_fixed_from_double(3.5), pico8_fixed_from_double(-1.0));
    vec_t result = vec_add(a, b);
    assert(result.x == pico8_fixed_from_double(4.5));
    assert(result.y == pico8_fixed_from_double(1.0));
}

static void test_sub_component_wise(void) {
    vec_t a = vec_make(pico8_fixed_from_double(5.0), pico8_fixed_from_double(2.0));
    vec_t b = vec_make(pico8_fixed_from_double(2.5), pico8_fixed_from_double(3.0));
    vec_t result = vec_sub(a, b);
    assert(result.x == pico8_fixed_from_double(2.5));
    assert(result.y == pico8_fixed_from_double(-1.0));
}

static void test_mul_scalar(void) {
    vec_t v = vec_make(pico8_fixed_from_double(2.0), pico8_fixed_from_double(-3.0));
    vec_t result = vec_mul(v, pico8_fixed_from_double(2.5));
    assert(result.x == pico8_fixed_from_double(5.0));
    assert(result.y == pico8_fixed_from_double(-7.5));
}

static void test_div_scalar(void) {
    vec_t v = vec_make(pico8_fixed_from_double(9.0), pico8_fixed_from_double(-6.0));
    vec_t result = vec_div(v, pico8_fixed_from_double(3.0));
    assert(result.x == pico8_fixed_from_double(3.0));
    assert(result.y == pico8_fixed_from_double(-2.0));
}

static void test_neg_negates_both_components(void) {
    vec_t v = vec_make(pico8_fixed_from_double(4.0), pico8_fixed_from_double(-1.5));
    vec_t result = vec_neg(v);
    assert(result.x == pico8_fixed_from_double(-4.0));
    assert(result.y == pico8_fixed_from_double(1.5));
}

static void test_dot_dot_product(void) {
    vec_t a = vec_make(pico8_fixed_from_double(3.0), pico8_fixed_from_double(4.0));
    vec_t b = vec_make(pico8_fixed_from_double(2.0), pico8_fixed_from_double(-1.0));
    p8_fixed_t result = vec_dot(a, b);
    /* 3*2 + 4*(-1) = 2 */
    assert(result == pico8_fixed_from_double(2.0));
}

static void test_length_squared_equals_dot_with_self(void) {
    vec_t v = vec_make(pico8_fixed_from_double(3.0), pico8_fixed_from_double(4.0));
    p8_fixed_t result = vec_length_squared(v);
    /* 3*3 + 4*4 = 25 */
    assert(result == pico8_fixed_from_double(25.0));
    assert(result == vec_dot(v, v));
}

static void test_length_length_of_3_4_5_triangle(void) {
    vec_t v = vec_make(pico8_fixed_from_double(3.0), pico8_fixed_from_double(4.0));
    p8_fixed_t result = vec_length(v);
    assert(result == pico8_fixed_from_double(5.0));
}

static void test_length_and_length_squared_agree_within_fixed_point_rounding(void) {
    /* qx(v)^2 ~= __len(v) (== rg(v,v)), modulo fixed-point rounding: qx
     * truncates rather than rounds (see pico8_fixed_sqrt), so squaring it
     * back can land slightly below the exact squared length. */
    vec_t v = vec_make(pico8_fixed_from_double(7.0), pico8_fixed_from_double(-2.0));
    p8_fixed_t length = vec_length(v);
    p8_fixed_t length_squared = p8_fixed_mul(length, length);
    p8_fixed_t squared_length = vec_length_squared(v);
    p8_fixed_t diff = p8_fixed_abs(p8_fixed_sub(squared_length, length_squared));
    /* Within one unit of length (7^2 + (-2)^2 = 53), a truncated sqrt's
     * square undershoots the exact value by less than 2*sqrt(53)+1 raw
     * units of length -- a generous tolerance of 1.0 in fixed-point terms
     * comfortably covers the actual rounding error here. */
    assert(diff <= pico8_fixed_from_double(1.0));
}

static void test_perpendicular_perpendicular(void) {
    vec_t v = vec_make(pico8_fixed_from_double(3.0), pico8_fixed_from_double(4.0));
    vec_t result = vec_perpendicular(v);
    assert(result.x == pico8_fixed_from_double(-4.0));
    assert(result.y == pico8_fixed_from_double(3.0));
}

static void test_normalize_normalizes_to_unit_length(void) {
    vec_t v = vec_make(pico8_fixed_from_double(3.0), pico8_fixed_from_double(4.0));
    vec_t result = vec_normalize(v);
    /* qx(v) == 5, so nn(v) == v/5 == (0.6, 0.8), modulo the fixed-point
     * division's own truncation (pico8_fixed_div truncates toward zero,
     * so e.g. 3/5's raw result can land one 1/65536th below the nearest
     * rounded 0.6) -- compared with a small tolerance rather than exact
     * equality for that reason. */
    assert(fabs(pico8_fixed_to_double(result.x) - 0.6) < 0.0001);
    assert(fabs(pico8_fixed_to_double(result.y) - 0.8) < 0.0001);
}

static void test_normalize_zero_length_vector_saturates_via_fixed_div_by_zero(void) {
    /* qx((0,0)) == 0, so nn((0,0)) == (0,0)/0, which -- per pico8_fixed_div
     * -- saturates each component to +0x7fff.ffff rather than crashing or
     * invoking C's own undefined behavior for integer division by zero.
     * This is the original Lua's actual behavior too: nn() has no
     * zero-length special case, so it inherits whatever v/0 does. */
    vec_t zero = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
    vec_t result = vec_normalize(zero);
    assert(result.x == P8_FIXED_DIV0_POS);
    assert(result.y == P8_FIXED_DIV0_POS);
}

static void test_from_polar_polar_to_cartesian_zero_turns(void) {
    /* qm(fy,lx) = v(cos(lx),sin(lx))*fy. At lx=0 turns, cos(0)=1, sin(0)=0
     * (see pico8_fixed_cos/pico8_fixed_sin), so qm(fy,0) == (fy, 0). */
    p8_fixed_t magnitude = pico8_fixed_from_double(10.0);
    vec_t result = vec_from_polar(magnitude, p8_fixed_from_int(0));
    assert(fabs(pico8_fixed_to_double(result.x) - 10.0) < 0.01);
    assert(fabs(pico8_fixed_to_double(result.y) - 0.0) < 0.01);
}

static void test_from_polar_polar_to_cartesian_quarter_turn(void) {
    /* At lx=0.25 turns, cos=0, sin's PICO-8 sign-inverted value is -1, so
     * qm(fy,0.25) == (0, -fy). */
    p8_fixed_t magnitude = pico8_fixed_from_double(4.0);
    vec_t result = vec_from_polar(magnitude, pico8_fixed_from_double(0.25));
    assert(fabs(pico8_fixed_to_double(result.x) - 0.0) < 0.01);
    assert(fabs(pico8_fixed_to_double(result.y) - (-4.0)) < 0.01);
}

int main(void) {
    test_vec_make_constructs_from_components();
    test_add_component_wise();
    test_sub_component_wise();
    test_mul_scalar();
    test_div_scalar();
    test_neg_negates_both_components();
    test_dot_dot_product();
    test_length_squared_equals_dot_with_self();
    test_length_length_of_3_4_5_triangle();
    test_length_and_length_squared_agree_within_fixed_point_rounding();
    test_perpendicular_perpendicular();
    test_normalize_normalizes_to_unit_length();
    test_normalize_zero_length_vector_saturates_via_fixed_div_by_zero();
    test_from_polar_polar_to_cartesian_zero_turns();
    test_from_polar_polar_to_cartesian_quarter_turn();

    printf("test_vec: all tests passed\n");
    return 0;
}
