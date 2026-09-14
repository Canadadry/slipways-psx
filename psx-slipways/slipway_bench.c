#include "slipway_bench.h"

/* Row-major scan order (i ascending, then j > i ascending) is the simplest
 * deterministic choice available -- explicitly arbitrary, NOT a "best
 * edges" selection rule (shortest/most-connected/etc.), which would smell
 * like inventing game content. See slipway_bench.h's header comment for
 * this module's full "pure geometry, not real slipway logic" scope. */
int slipway_bench_select_edges(const neighbor_graph_t *g, int planet_count, int max_edges, slipway_edge_t *out) {
    int count = 0;
    int i, j;

    for (i = 0; i < planet_count && count < max_edges; i++) {
        for (j = i + 1; j < planet_count && count < max_edges; j++) {
            if (g->adjacent[i][j]) {
                out[count].a = i;
                out[count].b = j;
                count++;
            }
        }
    }

    return count;
}
