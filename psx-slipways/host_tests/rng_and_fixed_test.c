#include "t.h"
#include "../fixed.h"
#include "../rng.h"

#include <math.h>

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
    t_run("rng_deterministic_for_same_seed", test_rng_deterministic_for_same_seed);
    t_run("rng_next_range_stays_within_bounds", test_rng_next_range_stays_within_bounds);
    return t_report();
}
