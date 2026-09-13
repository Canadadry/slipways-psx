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
    t_run("rng_deterministic_for_same_seed", test_rng_deterministic_for_same_seed);
    t_run("rng_next_range_stays_within_bounds", test_rng_next_range_stays_within_bounds);
    return t_report();
}
