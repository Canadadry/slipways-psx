#ifndef SLIPWAYS_SLIPWAY_BENCH_H
#define SLIPWAYS_SLIPWAY_BENCH_H

#include "planetgen.h"

/* This module is pure geometry for a GPU-primitive-submission / rendering-
 * performance benchmark -- it is NOT real slipway logic and must never grow
 * into any. No cost is computed, no validity rule (range/crossing/planet-
 * on-path, docs/design/slipways-mechanics.md Sec.5) is evaluated, no
 * player action creates or removes an edge, and nothing produced here is
 * ever read back by any other game system. It exists only to give the
 * FPS-feasibility harness a plausible-shaped, plausible-count, fully
 * deterministic load to submit to the GPU before the real subsystem
 * exists. Real slipway logic (liaison.c) is Phase 3 step 9's job; see
 * docs/prd/triage/psx-galaxy-rendering.md, "Synthetic slipway lines --
 * reusing real data, with an explicit justification". */

typedef struct {
    int a;
    int b;
} slipway_edge_t;

/* Scans g->adjacent[i][j] for i < j in a fixed row-major order and appends
 * the first max_edges edges found into out, returning the actual number
 * written (<= max_edges, and less than max_edges only if the graph itself
 * has fewer edges than that). Row-major scan order is the simplest
 * possible deterministic choice and is explicitly arbitrary: it does NOT
 * pick "the best"/shortest/most-connected edges by any game-relevant
 * measure -- a rule like that would smell like inventing a selection rule
 * (content). It just picks *some* fixed, reproducible max_edges of them. */
int slipway_bench_select_edges(const neighbor_graph_t *g, int planet_count, int max_edges, slipway_edge_t *out);

#endif
