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
