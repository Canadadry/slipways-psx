#ifndef SLIPWAYS_WORLD_H
#define SLIPWAYS_WORLD_H

#include "planetgen.h"
#include "slipway_bench.h"

void world_init(void);
const galaxy_t *world_get_galaxy(void);

/* Index into world_get_galaxy()'s positions[] designated, by world_init(),
 * as the wormhole marker's screen position. This is a placeholder for the
 * sole purpose of exercising the POLY_F4 GPU-primitive path (see
 * docs/prd/triage/psx-galaxy-rendering.md, "The wormhole placeholder --
 * deliberately not the real mechanic") -- it is not a placement rule, and
 * index 0 has no significance beyond being a real generated planet position
 * that lands inside the galaxy's visual footprint. The real wormhole is
 * placed by the player's first click, Phase 4 step 15 -- this is not that. */
#define WORLD_WORMHOLE_INDEX 0

int world_get_wormhole_index(void);

/* Squared neighbor-probe-range threshold, traced to pico8/lua.lua:767-770
 * (mz:hq's base probe range -- 0.19 squared units -- before any "space
 * folding" tech that would extend it to 0.27; no tech system exists yet,
 * so only the base value is modeled). Same SQ_UNITS convention as
 * host_tests/planetgen_test.c's pre-existing test-local use of this same
 * traced value; world.c is the first production (non-test) code to need
 * it, so it is named here, as a real shared symbol, rather than left
 * inlined in test code. */
#define WORLD_NEIGHBOR_RANGE_SQ ((int64_t)(0.19 * (double)FX_ONE * (double)FX_ONE))

/* Middle of docs/plan.md's stated 40-60-simultaneous-slipway feasibility
 * band, chosen once and reused by both the rendering code
 * (world_get_slipway_edges(), game_draw.c) and, later, chunk 5's FPS
 * harness, so "how many lines are on screen" and "how many lines the
 * benchmark measured against" can never drift apart. Benchmark-scaffolding
 * constant, not traced to docs/design/slipways-mechanics.md -- there is no
 * "50 slipways" in the source game. */
#define MAX_SYNTHETIC_SLIPWAYS 50

/* Exposes world_init()'s capped, deterministic synthetic slipway-line
 * geometry (see slipway_bench.h for what these edges are and are not).
 * *out_edges points at internal static storage of exactly
 * MAX_SYNTHETIC_SLIPWAYS slots (no dynamic allocation); *out_count is the
 * actual number populated (<= MAX_SYNTHETIC_SLIPWAYS). */
void world_get_slipway_edges(const slipway_edge_t **out_edges, int *out_count);

#endif
