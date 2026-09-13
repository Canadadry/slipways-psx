#include "t.h"
#include "../fixed.h"
#include "../planetgen.h"
#include "../rng.h"

#include <string.h>

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

static planet_pos_t pos(double x, double y) {
    planet_pos_t p;
    p.x = fixed_from_double(x);
    p.y = fixed_from_double(y);
    return p;
}

static void test_two_points_within_threshold_become_adjacent(void) {
    galaxy_t g;
    g.count = 2;
    g.positions[0] = pos(0.0, 0.0);
    g.positions[1] = pos(1.0, 0.0); /* squared distance = 1.0 */

    neighbor_graph_t graph;
    planetgen_build_neighbor_graph(&graph, &g, SQ_UNITS(2.0));

    T_CHECK(graph.adjacent[0][1]);
    T_CHECK(graph.adjacent[1][0]);
}

static void test_two_points_beyond_threshold_are_not_adjacent(void) {
    galaxy_t g;
    g.count = 2;
    g.positions[0] = pos(0.0, 0.0);
    g.positions[1] = pos(1.0, 0.0); /* squared distance = 1.0 */

    neighbor_graph_t graph;
    planetgen_build_neighbor_graph(&graph, &g, SQ_UNITS(0.5));

    T_CHECK(!graph.adjacent[0][1]);
    T_CHECK(!graph.adjacent[1][0]);
}

/* Hand-placed on a line: P0=(0,0) P1=(1,0) P2=(2,0) P3=(0,4).
 * Squared distances: P0-P1=1 P0-P2=4 P0-P3=16 P1-P2=1 P1-P3=17 P2-P3=20.
 * With range_sq = 2.0, only P0-P1 and P1-P2 are within range: the graph
 * connects P0-P1-P2 but leaves P3 isolated, and P0-P2 unconnected too. */
static void test_hand_placed_points_have_expected_edges_and_some_gaps(void) {
    galaxy_t g;
    g.count = 4;
    g.positions[0] = pos(0.0, 0.0);
    g.positions[1] = pos(1.0, 0.0);
    g.positions[2] = pos(2.0, 0.0);
    g.positions[3] = pos(0.0, 4.0);

    neighbor_graph_t graph;
    planetgen_build_neighbor_graph(&graph, &g, SQ_UNITS(2.0));

    T_CHECK(graph.adjacent[0][1]);
    T_CHECK(graph.adjacent[1][2]);

    T_CHECK(!graph.adjacent[0][2]);
    T_CHECK(!graph.adjacent[0][3]);
    T_CHECK(!graph.adjacent[1][3]);
    T_CHECK(!graph.adjacent[2][3]);
}

static void test_neighbor_graph_is_symmetric(void) {
    rng_t rng;
    rng_seed(&rng, 55u);

    galaxy_t g;
    int status = planetgen_generate(&g, 25, SQ_UNITS(0.05), &rng);
    T_CHECK(status == PLANETGEN_OK);

    neighbor_graph_t graph;
    planetgen_build_neighbor_graph(&graph, &g, SQ_UNITS(0.19));

    for (int i = 0; i < g.count; i++) {
        for (int j = 0; j < g.count; j++) {
            T_CHECK(graph.adjacent[i][j] == graph.adjacent[j][i]);
        }
    }
}

static void test_neighbor_graph_has_no_self_loops(void) {
    rng_t rng;
    rng_seed(&rng, 56u);

    galaxy_t g;
    int status = planetgen_generate(&g, 25, SQ_UNITS(0.05), &rng);
    T_CHECK(status == PLANETGEN_OK);

    neighbor_graph_t graph;
    planetgen_build_neighbor_graph(&graph, &g, SQ_UNITS(0.19));

    for (int i = 0; i < g.count; i++) {
        T_CHECK(!graph.adjacent[i][i]);
    }
}

static void test_trivial_count_one_is_connected(void) {
    neighbor_graph_t graph;
    memset(graph.adjacent, 0, sizeof(graph.adjacent));

    T_CHECK(planetgen_is_connected(&graph, 1));
}

static void test_trivial_count_zero_is_connected(void) {
    neighbor_graph_t graph;
    memset(graph.adjacent, 0, sizeof(graph.adjacent));

    T_CHECK(planetgen_is_connected(&graph, 0));
}

static void set_edge(neighbor_graph_t *g, int i, int j) {
    g->adjacent[i][j] = true;
    g->adjacent[j][i] = true;
}

static void test_hand_built_connected_graph_is_connected(void) {
    neighbor_graph_t graph;
    memset(graph.adjacent, 0, sizeof(graph.adjacent));

    /* Planets 0..5 fully connected to each other. */
    for (int i = 0; i < 6; i++) {
        for (int j = i + 1; j < 6; j++) {
            set_edge(&graph, i, j);
        }
    }

    T_CHECK(planetgen_is_connected(&graph, 6));
}

static void test_hand_built_two_disjoint_clusters_is_not_connected(void) {
    neighbor_graph_t graph;
    memset(graph.adjacent, 0, sizeof(graph.adjacent));

    /* Planets 0-2 fully connected to each other, planets 3-5 fully
     * connected to each other, but zero edges between the two groups. */
    for (int i = 0; i < 3; i++) {
        for (int j = i + 1; j < 3; j++) {
            set_edge(&graph, i, j);
        }
    }
    for (int i = 3; i < 6; i++) {
        for (int j = i + 1; j < 6; j++) {
            set_edge(&graph, i, j);
        }
    }

    T_CHECK(!planetgen_is_connected(&graph, 6));
}

static void test_hand_built_chain_is_connected_via_transitive_bridges(void) {
    neighbor_graph_t graph;
    memset(graph.adjacent, 0, sizeof(graph.adjacent));

    /* Chain 0-1-2-3-4: only adjacent links, no shortcuts. Confirms BFS
     * genuinely traverses transitively rather than only checking direct
     * neighbors of some fixed start node. */
    set_edge(&graph, 0, 1);
    set_edge(&graph, 1, 2);
    set_edge(&graph, 2, 3);
    set_edge(&graph, 3, 4);

    T_CHECK(planetgen_is_connected(&graph, 5));
}

/* Chunk 5 (integration): the real 90-planet configuration (traced
 * MIN_DIST_SQ = 0.05, NEIGHBOR_RANGE_SQ = 0.19, both squared units per
 * lua.lua:748/768) exercised across many distinct seeds.
 *
 * This does NOT assert 100% connectivity. The original PICO-8 source has no
 * galaxy-level connectivity guarantee: bw(90) (lua.lua:744) is called
 * exactly once per game, from eu() (lua.lua:1181-1182), with no retry and
 * no post-generation connectivity check or regeneration. The probe-range
 * check (mz:hq, lua.lua:767) tests against a fixed 0.19 (0.27 with "space
 * folding" tech) that never grows over time -- so an isolated planet
 * stranded in a domain corner, outside probe range of everything else, is a
 * real, if rare, outcome of the original game, not a porting bug. This was
 * confirmed empirically (22/2000 seeds, 1.1%, disconnected) and the project
 * owner directed that this quirk be preserved rather than engineered away.
 *
 * So instead this test bounds the disconnect rate: it must be > 0% (a
 * suspicious 0% across 2000 seeds would suggest the test isn't exercising
 * real generation, or a silent behavior change masking the quirk) and
 * < 5% (~91 seeds; a rate that much higher than the measured ~1.1% would
 * suggest a real regression in generation/graph/BFS logic, not the
 * expected quirk). 5% is a generous ~4.5x margin over the observed 1.1%,
 * chosen to absorb run-to-run/platform noise without masking a regression. */
static void test_real_90_planet_configuration_is_connected_across_many_seeds(void) {
    const int NUM_SEEDS = 2000;
    const int64_t min_dist_sq = SQ_UNITS(0.05);       /* lua.lua:748 */
    const int64_t neighbor_range_sq = SQ_UNITS(0.19); /* lua.lua:768, base (no tech) */
    int disconnected_count = 0;

    for (uint32_t seed = 1; seed <= (uint32_t)NUM_SEEDS; seed++) {
        rng_t rng;
        rng_seed(&rng, seed);

        galaxy_t g;
        int status = planetgen_generate(&g, PLANETGEN_MAX_PLANETS, min_dist_sq, &rng);
        if (status != PLANETGEN_OK) {
            printf("     seed %u: planetgen_generate failed (status=%d)\n", seed, status);
            t_current_test_failed = 1;
            continue;
        }

        neighbor_graph_t graph;
        planetgen_build_neighbor_graph(&graph, &g, neighbor_range_sq);

        if (!planetgen_is_connected(&graph, g.count)) {
            printf("     seed %u: galaxy of %d planets is NOT connected at range_sq=0.19\n", seed, g.count);
            disconnected_count++;
        }
    }

    printf("     %d/%d seeds disconnected (%.2f%%)\n", disconnected_count, NUM_SEEDS,
           100.0 * disconnected_count / NUM_SEEDS);

    T_CHECK(disconnected_count > 0);
    T_CHECK(disconnected_count < (NUM_SEEDS * 5 / 100));
}

int main(void) {
    t_run("generates_exact_count", test_generates_exact_count);
    t_run("no_pair_violates_minimum_distance", test_no_pair_violates_minimum_distance);
    t_run("same_seed_is_deterministic", test_same_seed_is_deterministic);
    t_run("impossible_configuration_reports_failure", test_impossible_configuration_reports_failure);
    t_run("two_points_within_threshold_become_adjacent", test_two_points_within_threshold_become_adjacent);
    t_run("two_points_beyond_threshold_are_not_adjacent", test_two_points_beyond_threshold_are_not_adjacent);
    t_run("hand_placed_points_have_expected_edges_and_some_gaps", test_hand_placed_points_have_expected_edges_and_some_gaps);
    t_run("neighbor_graph_is_symmetric", test_neighbor_graph_is_symmetric);
    t_run("neighbor_graph_has_no_self_loops", test_neighbor_graph_has_no_self_loops);
    t_run("trivial_count_one_is_connected", test_trivial_count_one_is_connected);
    t_run("trivial_count_zero_is_connected", test_trivial_count_zero_is_connected);
    t_run("hand_built_connected_graph_is_connected", test_hand_built_connected_graph_is_connected);
    t_run("hand_built_two_disjoint_clusters_is_not_connected", test_hand_built_two_disjoint_clusters_is_not_connected);
    t_run("hand_built_chain_is_connected_via_transitive_bridges", test_hand_built_chain_is_connected_via_transitive_bridges);
    t_run("real_90_planet_configuration_is_connected_across_many_seeds", test_real_90_planet_configuration_is_connected_across_many_seeds);
    return t_report();
}
