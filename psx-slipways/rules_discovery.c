#include "rules_discovery.h"

#include "world.h"

/* Ports ij(p,j_) (pico8/lua.lua:351-358), mz:hq()'s minimum-squared-distance
 * helper -- walks founded_positions, tracks the running minimum squared
 * distance to target, then compares against WORLD_NEIGHBOR_RANGE_SQ.
 *
 * Q16.16 throughout, int64_t intermediates -- same overflow-safety idiom as
 * liaison.c's liaison_point_blocks_segment/liaison_check (dx/dy stay within
 * PLANETGEN_PLACEMENT_SPAN's magnitude, so their squared sum comfortably
 * fits int64_t). */
bool rules_discovery_probe_in_range(planet_pos_t target, const planet_pos_t *founded_positions, int founded_count) {
    if (founded_count == 0) {
        return false;
    }

    int64_t min_dist_sq = 0;

    for (int i = 0; i < founded_count; i++) {
        int64_t dx = (int64_t)fixed_sub(target.x, founded_positions[i].x);
        int64_t dy = (int64_t)fixed_sub(target.y, founded_positions[i].y);
        int64_t dist_sq = dx * dx + dy * dy;

        if (i == 0 || dist_sq < min_dist_sq) {
            min_dist_sq = dist_sq;
        }
    }

    /* Source's own comparison is "nq > threshold" -> too far; in-range is
     * therefore "<=", matching liaison.c's identical boundary convention
     * for its own range check (LIAISON_RANGE_SQ). */
    return min_dist_sq <= WORLD_NEIGHBOR_RANGE_SQ;
}

/* n_=0.8 baked into this instantiation (jo.ir=jf(0.8,0.5),
 * pico8/lua.lua:750-751), in Q16.16 -- named following rules_planets.c's
 * own convention for inline fixed-point literal constants (e.g.
 * RULES_PLANETS_TECH_GATE_MULTIPLIER). */
#define RULES_DISCOVERY_EMPTY_ROLL_N ((fixed_t)(0.8 * (double)FX_ONE))

/* jo:hh()'s "<0.45" test (pico8/lua.lua:408), in Q16.16. */
#define RULES_DISCOVERY_EMPTY_ROLL_THRESHOLD ((fixed_t)(FX_ONE * 45 / 100))

static fixed_t rules_discovery_fixed_abs(fixed_t v) {
    return v < 0 ? -v : v;
}

static fixed_t rules_discovery_fixed_max0(fixed_t v) {
    return v > 0 ? v : 0;
}

void rules_discovery_empty_roll_init(rules_discovery_empty_roll_t *state) {
    state->bias = FX_ONE / 2;
}

bool rules_discovery_roll_empty(rules_discovery_empty_roll_t *state, rng_t *rng) {
    fixed_t span = fixed_sub(FX_ONE, rules_discovery_fixed_abs(state->bias));
    fixed_t drawn = (fixed_t)rng_next_range(rng, 0u, (uint32_t)span);
    fixed_t v = fixed_add(drawn, rules_discovery_fixed_max0(state->bias));

    state->bias = fixed_add(state->bias,
                             fixed_mul(fixed_sub(FX_ONE / 2, v), RULES_DISCOVERY_EMPTY_ROLL_N));

    return v < RULES_DISCOVERY_EMPTY_ROLL_THRESHOLD;
}
