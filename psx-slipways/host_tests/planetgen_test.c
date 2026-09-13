#include "t.h"
#include "../fixed.h"
#include "../planetgen.h"
#include "../rng.h"

#define SQ_UNITS(d) ((int64_t)((d) * (double)FX_ONE * (double)FX_ONE))

static void test_generates_exact_count(void) {
    rng_t rng;
    rng_seed(&rng, 1u);

    galaxy_t g;
    int status = planetgen_generate(&g, 10, SQ_UNITS(0.01), &rng);

    T_CHECK(status == PLANETGEN_OK);
    T_CHECK(g.count == 10);
}

static int64_t squared_distance(planet_pos_t a, planet_pos_t b) {
    int64_t dx = (int64_t)fixed_sub(a.x, b.x);
    int64_t dy = (int64_t)fixed_sub(a.y, b.y);
    return dx * dx + dy * dy;
}

static void test_no_pair_violates_minimum_distance(void) {
    rng_t rng;
    rng_seed(&rng, 7u);

    int64_t min_dist_sq = SQ_UNITS(0.05);

    galaxy_t g;
    int status = planetgen_generate(&g, 20, min_dist_sq, &rng);

    T_CHECK(status == PLANETGEN_OK);
    T_CHECK(g.count == 20);

    for (int i = 0; i < g.count; i++) {
        for (int j = i + 1; j < g.count; j++) {
            T_CHECK(squared_distance(g.positions[i], g.positions[j]) > min_dist_sq);
        }
    }
}

static void test_same_seed_is_deterministic(void) {
    rng_t rng_a, rng_b;
    rng_seed(&rng_a, 99u);
    rng_seed(&rng_b, 99u);

    galaxy_t ga, gb;
    int status_a = planetgen_generate(&ga, 15, SQ_UNITS(0.05), &rng_a);
    int status_b = planetgen_generate(&gb, 15, SQ_UNITS(0.05), &rng_b);

    T_CHECK(status_a == PLANETGEN_OK);
    T_CHECK(status_b == PLANETGEN_OK);
    T_CHECK(ga.count == gb.count);

    for (int i = 0; i < ga.count && i < gb.count; i++) {
        T_CHECK(ga.positions[i].x == gb.positions[i].x);
        T_CHECK(ga.positions[i].y == gb.positions[i].y);
    }
}

static void test_impossible_configuration_reports_failure(void) {
    rng_t rng;
    rng_seed(&rng, 3u);

    /* The placement domain spans [0.125, 2.875) per axis, so no two points
     * in it can ever be more than 2.75 apart on either axis -- a squared-
     * distance threshold of 20.0 can never be satisfied by a second point,
     * regardless of retry budget. Genuinely impossible, not just slow. */
    int64_t impossible_min_dist_sq = SQ_UNITS(20.0);

    galaxy_t g;
    g.count = 123;
    int status = planetgen_generate(&g, 5, impossible_min_dist_sq, &rng);

    T_CHECK(status == PLANETGEN_FAIL);
    T_CHECK(g.count == 0);
}

int main(void) {
    t_run("generates_exact_count", test_generates_exact_count);
    t_run("no_pair_violates_minimum_distance", test_no_pair_violates_minimum_distance);
    t_run("same_seed_is_deterministic", test_same_seed_is_deterministic);
    t_run("impossible_configuration_reports_failure", test_impossible_configuration_reports_failure);
    return t_report();
}
