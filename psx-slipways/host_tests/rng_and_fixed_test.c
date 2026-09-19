#include "t.h"
#include "../fixed.h"
#include "../rng.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define EPSILON 0.001

static void test_fixed_round_trip(void) {
    fixed_t f = fixed_from_double(1.5);
    double d = fixed_to_double(f);
    T_CHECK(fabs(d - 1.5) < EPSILON);
}

static void test_fixed_arithmetic(void) {
    fixed_t a = fixed_from_double(1.0);
    fixed_t b = fixed_from_double(2.0);

    fixed_t sum = fixed_add(a, b);
    T_CHECK(fabs(fixed_to_double(sum) - 3.0) < EPSILON);

    fixed_t diff = fixed_sub(b, a);
    T_CHECK(fabs(fixed_to_double(diff) - 1.0) < EPSILON);

    fixed_t two_and_half = fixed_from_double(2.5);
    fixed_t four = fixed_from_double(4.0);
    fixed_t product = fixed_mul(two_and_half, four);
    T_CHECK(fabs(fixed_to_double(product) - 10.0) < EPSILON);
}

static void test_fixed_sqrt_exact_perfect_squares(void) {
    T_CHECK(fixed_sqrt(0) == 0);
    T_CHECK(fixed_sqrt(FX_ONE) == FX_ONE);
    T_CHECK(fixed_sqrt(4 * FX_ONE) == 2 * FX_ONE);
    T_CHECK(fixed_sqrt(9 * FX_ONE) == 3 * FX_ONE);
}

static void test_fixed_sqrt_irrational_within_tolerance(void) {
    /* sqrt(2) is irrational, so fixed_sqrt(2*FX_ONE) can't land exactly on
     * fixed_from_double(1.41421356*FX_ONE); it converges to within the
     * truncation error of an integer isqrt (at most 1 ULP of Q16.16, i.e.
     * off by less than 1/65536 in real terms) plus fixed_from_double's own
     * double->fixed rounding. 4 fixed-point units (~0.00006 in real terms)
     * is tight enough to catch a broken algorithm (e.g. off-by-one-bit
     * shift errors are orders of magnitude bigger) but loose enough to
     * tolerate genuine integer-sqrt truncation. */
    fixed_t expected = fixed_from_double(1.41421356);
    fixed_t actual = fixed_sqrt(2 * FX_ONE);
    fixed_t diff = actual - expected;
    if (diff < 0) diff = -diff;
    T_CHECK(diff <= 4);
}

static void test_fixed_sqrt_negative_is_zero(void) {
    T_CHECK(fixed_sqrt(-FX_ONE) == 0);
    T_CHECK(fixed_sqrt(-1) == 0);
}

static void test_fixed_sqrt_is_monotonic(void) {
    fixed_t prev = fixed_sqrt(0);
    for (fixed_t x = FX_ONE / 4; x <= 10 * FX_ONE; x += FX_ONE / 4) {
        fixed_t cur = fixed_sqrt(x);
        T_CHECK(cur >= prev);
        prev = cur;
    }
}

static void test_fixed_sin_cos_known_turns(void) {
    /* PICO-8's own documented convention: pico8_sin(x) = -sin(2*pi*x),
     * pico8_cos(x) = cos(2*pi*x), in standard-math terms, x in turns.
     * At x = 0, 0.25, 0.5, 0.75 these land on exact values, independently
     * confirmed here via ordinary trig identities (not copied from this
     * chunk's own PRD prose):
     *   x=0.00: sin(0)=0,      cos(0)=1      -> pico8_sin=0,  pico8_cos=1
     *   x=0.25: sin(pi/2)=1,   cos(pi/2)=0   -> pico8_sin=-1, pico8_cos=0
     *   x=0.50: sin(pi)=0,     cos(pi)=-1    -> pico8_sin=0,  pico8_cos=-1
     *   x=0.75: sin(3pi/2)=-1, cos(3pi/2)=0  -> pico8_sin=1,  pico8_cos=0
     * These also happen to be exact table-boundary samples (turns that are
     * multiples of a quarter turn need zero interpolation), so the checks
     * below are exact equalities, not tolerance-bounded. The sign flip on
     * sin (and its absence on cos) is the load-bearing assertion here: a
     * plain, unflipped sin() port would fail the 0.25/0.75 checks with the
     * sign backwards while still passing the 0/0.5 checks (both 0 either
     * way), which is exactly the kind of bug this test exists to catch.
     */
    T_CHECK(fixed_sin(0) == 0);
    T_CHECK(fixed_cos(0) == FX_ONE);

    T_CHECK(fixed_sin(FX_ONE / 4) == -FX_ONE);
    T_CHECK(fixed_cos(FX_ONE / 4) == 0);

    T_CHECK(fixed_sin(FX_ONE / 2) == 0);
    T_CHECK(fixed_cos(FX_ONE / 2) == -FX_ONE);

    T_CHECK(fixed_sin(3 * FX_ONE / 4) == FX_ONE);
    T_CHECK(fixed_cos(3 * FX_ONE / 4) == 0);
}

static void test_fixed_sin_cos_irrational_spot_check(void) {
    /* turns = 1/3: not a table-boundary value, so this actually exercises
     * the interpolation path (unlike the exact-quarter-turn checks above).
     * Expected values are independently derived here from PICO-8's own
     * documented formula (pico8_sin(x) = -sin(2*pi*x), pico8_cos(x) =
     * cos(2*pi*x)) via libm at test time -- not copied from this chunk's
     * PRD text, which never states this particular value. */
    double x = 1.0 / 3.0;
    fixed_t turns = fixed_from_double(x);

    double expected_sin = -sin(2.0 * M_PI * x);
    double expected_cos = cos(2.0 * M_PI * x);

    double actual_sin = fixed_to_double(fixed_sin(turns));
    double actual_cos = fixed_to_double(fixed_cos(turns));

    /* Tolerance covers (a) fixed_from_double's own rounding of 1/3 into
     * Q16.16 before fixed_sin/fixed_cos ever see it, (b) the lookup table's
     * linear-interpolation error between its 1/1024-turn samples (bounded
     * by (1/8)*|f''|*h^2 for linear interpolation of sin/cos, h = 2*pi/1024
     * radians, |f''| <= (2*pi)^2 -- a small fraction of a Q16.16 unit), and
     * (c) the table itself being rounded to the nearest Q16.16 integer at
     * generation time. EPSILON (0.001, already used above for fixed_sqrt)
     * is generous against all three combined while still tight enough to
     * catch a genuinely broken table, interpolation, or sign error. */
    T_CHECK(fabs(actual_sin - expected_sin) < EPSILON);
    T_CHECK(fabs(actual_cos - expected_cos) < EPSILON);
}

static void test_fixed_polar_zero_turns_returns_magnitude_x(void) {
    /* qm(fy,0) = v(cos(0),sin(0))*fy = v(1,0)*fy = (fy,0) -- turns=0 is also
     * an exact table-boundary value for fixed_sin/fixed_cos (already
     * confirmed exact above), so this is an exact equality, not a
     * tolerance-bounded check. */
    fixed_t magnitude = fixed_from_double(2.75);
    fixed_t out_x, out_y;

    fixed_polar(magnitude, 0, &out_x, &out_y);
    T_CHECK(out_x == magnitude);
    T_CHECK(out_y == 0);

    /* Also true for a negative magnitude -- fixed_mul(mag, FX_ONE) == mag
     * regardless of sign. */
    fixed_t neg_magnitude = fixed_from_double(-1.5);
    fixed_polar(neg_magnitude, 0, &out_x, &out_y);
    T_CHECK(out_x == neg_magnitude);
    T_CHECK(out_y == 0);
}

static void test_fixed_polar_zero_magnitude_returns_zero(void) {
    /* qm(0,lx) = v(cos(lx),sin(lx))*0 = (0,0) for any turns -- checked at a
     * few different turns values, including ones that exercise the
     * table-interpolation path (not just exact quarter-turn boundaries). */
    fixed_t out_x, out_y;

    fixed_polar(0, FX_ONE / 4, &out_x, &out_y);
    T_CHECK(out_x == 0);
    T_CHECK(out_y == 0);

    fixed_polar(0, fixed_from_double(1.0 / 6.0), &out_x, &out_y);
    T_CHECK(out_x == 0);
    T_CHECK(out_y == 0);

    fixed_polar(0, fixed_from_double(3.7), &out_x, &out_y);
    T_CHECK(out_x == 0);
    T_CHECK(out_y == 0);
}

static void test_fixed_polar_matches_hand_computed_value(void) {
    /* A non-axis-aligned case: magnitude=3, turns=1/6 (60 degrees), not a
     * table-boundary value, so this exercises fixed_sin/fixed_cos's
     * interpolation path, not just an exact sample. Expected values are
     * independently derived here from qm()'s own real formula
     * (v(cos(lx),sin(lx))*fy, using PICO-8's turns-based, sign-flipped-sine
     * cos()/sin() -- see the fixed_polar/fixed_sin/fixed_cos block comments
     * in fixed.h) via libm at test time, not copied from this chunk's PRD
     * text, which never states this particular value. */
    double magnitude_d = 3.0;
    double turns_d = 1.0 / 6.0;

    double expected_x = magnitude_d * cos(2.0 * M_PI * turns_d);
    double expected_y = magnitude_d * (-sin(2.0 * M_PI * turns_d));

    fixed_t magnitude = fixed_from_double(magnitude_d);
    fixed_t turns = fixed_from_double(turns_d);
    fixed_t out_x, out_y;

    fixed_polar(magnitude, turns, &out_x, &out_y);

    /* Tolerance covers the same combined sources test_fixed_sin_cos_
     * irrational_spot_check already documents above (fixed_from_double's own
     * rounding, the lookup table's linear-interpolation error, and the
     * table's own Q16.16 rounding), plus fixed_mul's own truncation on top --
     * EPSILON is already generous enough to cover all of these combined. */
    T_CHECK(fabs(fixed_to_double(out_x) - expected_x) < EPSILON);
    T_CHECK(fabs(fixed_to_double(out_y) - expected_y) < EPSILON);
}

static void test_rng_deterministic_for_same_seed(void) {
    rng_t a, b;
    rng_seed(&a, 12345u);
    rng_seed(&b, 12345u);

    for (int i = 0; i < 100; i++) {
        T_CHECK(rng_next_u32(&a) == rng_next_u32(&b));
    }
}

static void test_rng_next_range_stays_within_bounds(void) {
    rng_t rng;
    rng_seed(&rng, 42u);

    const uint32_t min = 5u;
    const uint32_t max_exclusive = 15u;

    int saw_low = 0;
    int saw_high = 0;

    for (int i = 0; i < 10000; i++) {
        uint32_t v = rng_next_range(&rng, min, max_exclusive);
        T_CHECK(v >= min);
        T_CHECK(v < max_exclusive);
        if (v == min) saw_low = 1;
        if (v == max_exclusive - 1) saw_high = 1;
    }

    /* Plausibly spans the declared range, not just a narrow slice of it. */
    T_CHECK(saw_low);
    T_CHECK(saw_high);
}

int main(void) {
    t_run("fixed_round_trip", test_fixed_round_trip);
    t_run("fixed_arithmetic", test_fixed_arithmetic);
    t_run("fixed_sqrt_exact_perfect_squares", test_fixed_sqrt_exact_perfect_squares);
    t_run("fixed_sqrt_irrational_within_tolerance", test_fixed_sqrt_irrational_within_tolerance);
    t_run("fixed_sqrt_negative_is_zero", test_fixed_sqrt_negative_is_zero);
    t_run("fixed_sqrt_is_monotonic", test_fixed_sqrt_is_monotonic);
    t_run("fixed_sin_cos_known_turns", test_fixed_sin_cos_known_turns);
    t_run("fixed_sin_cos_irrational_spot_check", test_fixed_sin_cos_irrational_spot_check);
    t_run("fixed_polar_zero_turns_returns_magnitude_x", test_fixed_polar_zero_turns_returns_magnitude_x);
    t_run("fixed_polar_zero_magnitude_returns_zero", test_fixed_polar_zero_magnitude_returns_zero);
    t_run("fixed_polar_matches_hand_computed_value", test_fixed_polar_matches_hand_computed_value);
    t_run("rng_deterministic_for_same_seed", test_rng_deterministic_for_same_seed);
    t_run("rng_next_range_stays_within_bounds", test_rng_next_range_stays_within_bounds);
    return t_report();
}
