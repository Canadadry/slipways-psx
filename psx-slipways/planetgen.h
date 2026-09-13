#ifndef PLANETGEN_H
#define PLANETGEN_H

#include <stdbool.h>
#include <stdint.h>

#include "fixed.h"
#include "rng.h"

#define PLANETGEN_MAX_PLANETS 90

#define PLANETGEN_OK 0
#define PLANETGEN_FAIL (-1)

typedef struct {
    fixed_t x;
    fixed_t y;
} planet_pos_t;

typedef struct {
    planet_pos_t positions[PLANETGEN_MAX_PLANETS];
    int count;
} galaxy_t;

int planetgen_generate(galaxy_t *out, int count, int64_t min_dist_sq, rng_t *rng);

/* Dense boolean adjacency matrix over a galaxy's planets. adjacent[i][j] is
 * true iff planets i and j are within range_sq of each other (squared
 * distance, same Q32.32 convention as planetgen_generate's min_dist_sq).
 * Symmetric by construction; the diagonal (adjacent[i][i]) is always false
 * -- a planet is never its own neighbor. Only entries with i < g->count and
 * j < g->count are meaningful; the rest of the matrix is zeroed. */
typedef struct {
    bool adjacent[PLANETGEN_MAX_PLANETS][PLANETGEN_MAX_PLANETS];
} neighbor_graph_t;

void planetgen_build_neighbor_graph(neighbor_graph_t *out, const galaxy_t *g, int64_t range_sq);

#endif
