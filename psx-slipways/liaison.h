#ifndef SLIPWAYS_LIAISON_H
#define SLIPWAYS_LIAISON_H

#include "fixed.h"
#include "planetgen.h"

/* Pure section-5 slipway-validity geometry, ported from pico8/lua.lua's
 * jc:ce() (line 641-653) and its be()/cl()/ee() helpers. See
 * docs/prd/triage/psx-liaison-validity.md for the full trace. This chunk
 * is geometry only: no state, no PSX headers, no caller yet.
 *
 * PICO-8's # (length) operator on a vector returns squared magnitude, not
 * magnitude, so every threshold traced from a #(...) comparison in source
 * is a squared-distance quantity -- same convention as world.h's
 * WORLD_NEIGHBOR_RANGE_SQ. */

/* Squared max slipway range, jc:ce() line 641-644 (base value; the "space
 * folding" tech extension to 1.08 is deferred, same treatment
 * WORLD_NEIGHBOR_RANGE_SQ gave the 0.19/0.27 probe-range pair). Same
 * int64_t squared-Q16.16 convention as WORLD_NEIGHBOR_RANGE_SQ. */
#define LIAISON_RANGE_SQ ((int64_t)(0.5 * (double)FX_ONE * (double)FX_ONE))

/* Squared "blocked by a planet" tolerance, be()'s call site in jc:ce()
 * line 648-649. Same int64_t squared-Q16.16 convention as above. */
#define LIAISON_BLOCK_TOLERANCE_SQ ((int64_t)(0.0032 * (double)FX_ONE * (double)FX_ONE))

/* Segment-projection-parameter margins, be() line 1488-1494: the
 * projection parameter must be strictly between these. Unlike the two
 * constants above this is a plain fraction of the segment (0=source,
 * 1=target), so it is an ordinary Q16.16 fixed_t. Neither constant is
 * named in pico8/lua.lua itself; named locally here per this codebase's
 * established traced-local-constant convention. */
#define LIAISON_T_MARGIN_LOW ((fixed_t)(0.01 * (double)FX_ONE))
#define LIAISON_T_MARGIN_HIGH ((fixed_t)(0.99 * (double)FX_ONE))

typedef enum {
    LIAISON_VALID = 0,
    LIAISON_ALREADY_CONNECTED,
    LIAISON_TOO_FAR,
    LIAISON_BLOCKED_BY_PLANET,
    LIAISON_CROSSES_ROUTE,
} liaison_reason_t;

typedef struct {
    liaison_reason_t reason;
    int blocking_planet; /* galaxy index, or -1 */
    int blocking_route;  /* index into routes[], or -1 */
} liaison_result_t;

/* A candidate/existing route as a pair of galaxy planet indices.
 * Deliberately separate from slipway_bench.h's slipway_edge_t (identical
 * shape, different meaning): that module's own header says it must never
 * grow into real logic, so real logic never depends on its type. */
typedef struct {
    int a;
    int b;
} liaison_route_t;

/* Ports be(a,b,c,fh), pico8/lua.lua line 1488-1494: computes the segment-
 * projection parameter t = dot(d, c-a) / dot(d,d) for d = b-a, requires t
 * strictly inside (LIAISON_T_MARGIN_LOW, LIAISON_T_MARGIN_HIGH), and
 * checks the squared distance from the closest point (a + d*t) to c is
 * under LIAISON_BLOCK_TOLERANCE_SQ. Nonzero iff c blocks segment a-b. */
int liaison_point_blocks_segment(planet_pos_t a, planet_pos_t b, planet_pos_t c);

/* Ports cl()/ee(), pico8/lua.lua line 1475-1486: standard four-orientation
 * strict segment-intersection test (pure cross-product sign comparisons,
 * no square roots). A collinear/zero-cross case, including an endpoint
 * exactly touching the other segment, is not a crossing. Nonzero iff
 * segment p1-p2 strictly crosses segment q1-q2. */
int liaison_segments_cross(planet_pos_t p1, planet_pos_t p2, planet_pos_t q1, planet_pos_t q2);

/* Ports jc:ce(), pico8/lua.lua line 641-653, in source's own order:
 *   1. Range check first (LIAISON_TOO_FAR if squared distance exceeds
 *      LIAISON_RANGE_SQ); short-circuits everything below.
 *   2. Already-connected: only meaningful when to_index != -1. Walks
 *      routes[] for an existing route between from_index/to_index.
 *   3. Blocked-by-planet: scans every planet in galaxy; first blocker in
 *      galaxy index order wins.
 *   4. Crosses-route: only evaluated if step 3 found nothing. Scans
 *      routes[]; first crossing route in list order wins.
 *   5. Otherwise LIAISON_VALID.
 * galaxy may be NULL only if route_count is 0; routes may be NULL iff
 * route_count is 0. */
liaison_result_t liaison_check(int from_index, planet_pos_t from_pos,
                                int to_index, planet_pos_t to_pos,
                                const galaxy_t *galaxy,
                                const liaison_route_t *routes, int route_count);

#endif
