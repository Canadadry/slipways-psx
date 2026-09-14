#include "world.h"

#include "rng.h"

/* Literal fixed seed, not hardware entropy: a test-harness/reproducibility
 * choice (fully deterministic visuals + later FPS measurement), not a
 * game-design constant. Real seeding is deferred, per
 * docs/prd/psx-planet-generation.md, to whichever future step implements
 * an actual "new game" flow. */
#define WORLD_RNG_SEED 12345u

#define WORLD_PLANET_COUNT 90

/* Minimum squared placement distance, traced to pico8/lua.lua:748 -- the
 * same threshold host_tests/planetgen_test.c's SQ_UNITS(0.05) test-fixture
 * macro already exercises for the real 90-planet configuration. world.c is
 * the first production (non-test) code to need this value, so it is named
 * here rather than duplicated inline. */
#define WORLD_MIN_DIST_SQ ((int64_t)(0.05 * (double)FX_ONE * (double)FX_ONE))

static galaxy_t g_galaxy;
static neighbor_graph_t g_neighbor_graph;
static slipway_edge_t g_slipway_edges[MAX_SYNTHETIC_SLIPWAYS];
static int g_slipway_edge_count;

void world_init(void) {
    rng_t rng;
    rng_seed(&rng, WORLD_RNG_SEED);

    planetgen_generate(&g_galaxy, WORLD_PLANET_COUNT, WORLD_MIN_DIST_SQ, &rng);
    planetgen_build_neighbor_graph(&g_neighbor_graph, &g_galaxy, WORLD_NEIGHBOR_RANGE_SQ);
    g_slipway_edge_count = slipway_bench_select_edges(&g_neighbor_graph, g_galaxy.count,
                                                        MAX_SYNTHETIC_SLIPWAYS, g_slipway_edges);
}

const galaxy_t *world_get_galaxy(void) {
    return &g_galaxy;
}

int world_get_wormhole_index(void) {
    return WORLD_WORMHOLE_INDEX;
}

void world_get_slipway_edges(const slipway_edge_t **out_edges, int *out_count) {
    *out_edges = g_slipway_edges;
    *out_count = g_slipway_edge_count;
}
