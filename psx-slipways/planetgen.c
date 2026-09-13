#include "planetgen.h"

#include <stddef.h>
#include <string.h>

#define PLANETGEN_PLACEMENT_MIN ((fixed_t)(0.125 * FX_ONE))
#define PLANETGEN_PLACEMENT_SPAN ((fixed_t)(2.75 * FX_ONE))
#define PLANETGEN_MAX_ATTEMPTS_PER_POINT 10000

static int planetgen_fits(const galaxy_t *g, fixed_t x, fixed_t y, int64_t min_dist_sq) {
    for (int i = 0; i < g->count; i++) {
        int64_t dx = (int64_t)fixed_sub(x, g->positions[i].x);
        int64_t dy = (int64_t)fixed_sub(y, g->positions[i].y);
        int64_t dist_sq = dx * dx + dy * dy;
        if (dist_sq <= min_dist_sq) {
            return 0;
        }
    }
    return 1;
}

int planetgen_generate(galaxy_t *out, int count, int64_t min_dist_sq, rng_t *rng) {
    if (out == NULL || rng == NULL) {
        return PLANETGEN_FAIL;
    }
    if (count < 0 || count > PLANETGEN_MAX_PLANETS) {
        out->count = 0;
        return PLANETGEN_FAIL;
    }

    out->count = 0;
    for (int i = 0; i < count; i++) {
        int placed = 0;
        for (int attempt = 0; attempt < PLANETGEN_MAX_ATTEMPTS_PER_POINT; attempt++) {
            fixed_t x = fixed_add(PLANETGEN_PLACEMENT_MIN, (fixed_t)rng_next_range(rng, 0u, (uint32_t)PLANETGEN_PLACEMENT_SPAN));
            fixed_t y = fixed_add(PLANETGEN_PLACEMENT_MIN, (fixed_t)rng_next_range(rng, 0u, (uint32_t)PLANETGEN_PLACEMENT_SPAN));

            if (planetgen_fits(out, x, y, min_dist_sq)) {
                out->positions[out->count].x = x;
                out->positions[out->count].y = y;
                out->count++;
                placed = 1;
                break;
            }
        }
        if (!placed) {
            out->count = 0;
            return PLANETGEN_FAIL;
        }
    }

    return PLANETGEN_OK;
}

void planetgen_build_neighbor_graph(neighbor_graph_t *out, const galaxy_t *g, int64_t range_sq) {
    memset(out->adjacent, 0, sizeof(out->adjacent));

    for (int i = 0; i < g->count; i++) {
        for (int j = i + 1; j < g->count; j++) {
            int64_t dx = (int64_t)fixed_sub(g->positions[i].x, g->positions[j].x);
            int64_t dy = (int64_t)fixed_sub(g->positions[i].y, g->positions[j].y);
            int64_t dist_sq = dx * dx + dy * dy;

            if (dist_sq <= range_sq) {
                out->adjacent[i][j] = true;
                out->adjacent[j][i] = true;
            }
        }
    }
}

bool planetgen_is_connected(const neighbor_graph_t *g, int count) {
    if (count <= 1) {
        return true;
    }

    bool visited[PLANETGEN_MAX_PLANETS];
    int queue[PLANETGEN_MAX_PLANETS];
    int queue_head = 0;
    int queue_tail = 0;
    int visited_count = 0;

    memset(visited, 0, sizeof(visited));

    visited[0] = true;
    visited_count++;
    queue[queue_tail++] = 0;

    while (queue_head < queue_tail) {
        int node = queue[queue_head++];
        for (int neighbor = 0; neighbor < count; neighbor++) {
            if (g->adjacent[node][neighbor] && !visited[neighbor]) {
                visited[neighbor] = true;
                visited_count++;
                queue[queue_tail++] = neighbor;
            }
        }
    }

    return visited_count == count;
}
