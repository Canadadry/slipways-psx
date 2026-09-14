#include "t.h"
#include "../planetgen.h"
#include "../world.h"

static void test_world_init_produces_90_planets(void) {
    world_init();

    const galaxy_t *g = world_get_galaxy();

    T_CHECK(g != NULL);
    T_CHECK(g->count == PLANETGEN_MAX_PLANETS);
}

static void test_world_init_is_deterministic_across_calls(void) {
    world_init();
    galaxy_t first = *world_get_galaxy();

    world_init();
    const galaxy_t *second = world_get_galaxy();

    T_CHECK(first.count == second->count);
    for (int i = 0; i < first.count && i < second->count; i++) {
        T_CHECK(first.positions[i].x == second->positions[i].x);
        T_CHECK(first.positions[i].y == second->positions[i].y);
    }
}

static void test_world_wormhole_index_is_within_range(void) {
    world_init();
    const galaxy_t *g = world_get_galaxy();

    int wormhole_index = world_get_wormhole_index();

    T_CHECK(wormhole_index == WORLD_WORMHOLE_INDEX);
    T_CHECK(wormhole_index >= 0);
    T_CHECK(wormhole_index < g->count);
}

/* Chunk 4: world_init() must expose exactly MAX_SYNTHETIC_SLIPWAYS (50)
 * synthetic slipway edges for the real fixed-seed/90-planet configuration
 * -- the concrete count both game_draw.c and (later) the FPS harness
 * depend on. */
static void test_world_init_produces_max_synthetic_slipway_edges(void) {
    world_init();

    const slipway_edge_t *edges;
    int count;
    world_get_slipway_edges(&edges, &count);

    T_CHECK(edges != NULL);
    T_CHECK(count == MAX_SYNTHETIC_SLIPWAYS);

    for (int i = 0; i < count; i++) {
        T_CHECK(edges[i].a >= 0 && edges[i].a < PLANETGEN_MAX_PLANETS);
        T_CHECK(edges[i].b >= 0 && edges[i].b < PLANETGEN_MAX_PLANETS);
        T_CHECK(edges[i].a != edges[i].b);
    }
}

/* Chunk 4 integration check (Testing Decisions): confirms that, for the
 * real 90-planet/fixed-WORLD_RNG_SEED configuration at the real, traced
 * WORLD_NEIGHBOR_RANGE_SQ threshold, the generation-time neighbor graph
 * genuinely has MORE than MAX_SYNTHETIC_SLIPWAYS (50) edges -- i.e. the cap
 * in slipway_bench_select_edges is the thing that determines the rendered
 * edge count for the real scenario, not incidental graph sparseness. Reuses
 * world_init()'s already-generated real galaxy rather than re-deriving the
 * seed/count, and rebuilds the neighbor graph directly (world.c doesn't
 * expose its own pre-cap graph) using the same real, shared
 * WORLD_NEIGHBOR_RANGE_SQ constant world_init() itself uses -- no threshold
 * is reinvented here. */
static void test_real_configuration_has_more_edges_than_the_cap(void) {
    world_init();
    const galaxy_t *g = world_get_galaxy();

    neighbor_graph_t graph;
    planetgen_build_neighbor_graph(&graph, g, WORLD_NEIGHBOR_RANGE_SQ);

    int edge_count = 0;
    for (int i = 0; i < g->count; i++) {
        for (int j = i + 1; j < g->count; j++) {
            if (graph.adjacent[i][j]) {
                edge_count++;
            }
        }
    }

    T_CHECK(edge_count > MAX_SYNTHETIC_SLIPWAYS);
}

int main(void) {
    t_run("world_init_produces_90_planets", test_world_init_produces_90_planets);
    t_run("world_init_is_deterministic_across_calls", test_world_init_is_deterministic_across_calls);
    t_run("world_wormhole_index_is_within_range", test_world_wormhole_index_is_within_range);
    t_run("world_init_produces_max_synthetic_slipway_edges", test_world_init_produces_max_synthetic_slipway_edges);
    t_run("real_configuration_has_more_edges_than_the_cap", test_real_configuration_has_more_edges_than_the_cap);
    return t_report();
}
