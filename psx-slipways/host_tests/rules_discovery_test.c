#include "t.h"
#include "../rules_discovery.h"
#include "../world.h"

static void test_probe_cost_credits_matches_traced_value(void) {
    T_CHECK(RULES_DISCOVERY_PROBE_COST_CREDITS == 3);
}

/* This asserts 1, NOT the 3 that docs/plan.md's Phase 3 step 10 one-liner
 * and docs/design/slipways-mechanics.md SS4 both state in prose ("3 mois"/
 * "3 months"). The source data (mz's fp=1, pico8/lua.lua:758, flowing
 * through d_()'s fm:nx(self.fp or 0) at line 571 and hy:nx(hx)'s months
 * semantics at lines 1204-1209 -- see rules_discovery.h's comment on this
 * constant for the full trail) contradicts both docs, and source wins.
 * This test exists specifically so that correction is asserted in code,
 * not just narrated in the PRD. */
static void test_probe_cost_months_is_1_not_3(void) {
    T_CHECK(RULES_DISCOVERY_PROBE_COST_MONTHS == 1);
}

/* WORLD_NEIGHBOR_RANGE_SQ (chunk 2) is 816043786 for this build (see
 * world.h: (int64_t)(0.19 * (double)FX_ONE * (double)FX_ONE)). This exact
 * integer is NOT expressible as a sum of two squares of integers: its prime
 * factorization is 2 x 67 x 73 x 83423, and 67 (~= 3 mod 4) appears to an
 * ODD power, which by Fermat's two-square theorem rules out ANY integer
 * (dx, dy) with dx*dx + dy*dy == 816043786 -- exhaustively confirmed by
 * checking every dx in [0, sqrt(target)] against an exact integer
 * square-root of the remainder (no floating-point rounding involved). So a
 * real planet_pos_t pair whose Euclidean squared distance is bit-for-bit
 * equal to WORLD_NEIGHBOR_RANGE_SQ cannot exist for this threshold value --
 * this is a property of the number itself, not a limitation of this test.
 * The boundary tests below therefore use the closest INTEGER-EXACT squared
 * distances the same exhaustive search found on each side of the threshold
 * (816043780, 6 below; 816043792, 6 above -- both globally-nearest, not
 * merely "close enough" guesses), which is the tightest fixture that can
 * actually be constructed from real fixed-point coordinates while still
 * being exactly, provably on one side of the "<=" comparison. */
static void test_founded_planet_at_closest_achievable_threshold_is_in_range(void) {
    planet_pos_t target = { 0, 0 };
    /* dx=696, dy=28558 -> dx*dx + dy*dy == 816043780 == WORLD_NEIGHBOR_RANGE_SQ - 6,
     * the largest integer-exact squared distance not exceeding the threshold. */
    planet_pos_t founded[] = { { (fixed_t)696, (fixed_t)28558 } };

    T_CHECK((int64_t)696 * 696 + (int64_t)28558 * 28558 == WORLD_NEIGHBOR_RANGE_SQ - 6);
    T_CHECK(rules_discovery_probe_in_range(target, founded, 1) == true);
}

static void test_founded_planet_just_beyond_threshold_is_out_of_range(void) {
    planet_pos_t target = { 0, 0 };
    /* dx=6384, dy=27844 -> dx*dx + dy*dy == 816043792 == WORLD_NEIGHBOR_RANGE_SQ + 6,
     * the smallest integer-exact squared distance exceeding the threshold. */
    planet_pos_t founded[] = { { (fixed_t)6384, (fixed_t)27844 } };

    T_CHECK((int64_t)6384 * 6384 + (int64_t)27844 * 27844 == WORLD_NEIGHBOR_RANGE_SQ + 6);
    T_CHECK(rules_discovery_probe_in_range(target, founded, 1) == false);
}

/* Proves the function takes the TRUE minimum across the whole list, not
 * e.g. the first or last entry: the far planet alone would report
 * "too far", but a nearer, in-range planet elsewhere in the same list must
 * still make the overall call report in-range. */
static void test_nearest_founded_planet_governs_result(void) {
    planet_pos_t target = { 0, 0 };
    planet_pos_t far_only[] = { { FX_ONE, 0 } };
    planet_pos_t near_only[] = { { (fixed_t)1000, (fixed_t)1000 } };
    planet_pos_t far_then_near[] = {
        { FX_ONE, 0 },
        { (fixed_t)1000, (fixed_t)1000 },
    };

    /* Sanity: the far planet is genuinely out of range on its own, and the
     * near planet is genuinely in range on its own -- otherwise this test
     * wouldn't actually exercise the "nearest governs" property. */
    T_CHECK(rules_discovery_probe_in_range(target, far_only, 1) == false);
    T_CHECK(rules_discovery_probe_in_range(target, near_only, 1) == true);

    T_CHECK(rules_discovery_probe_in_range(target, far_then_near, 2) == true);
}

/* founded_count == 0 always returns false, even when target coincides
 * exactly with a position that simply isn't part of the (empty) founded
 * list passed in -- founded_positions itself points at a real planet at
 * that exact same position, but founded_count says there are 0 entries to
 * consider, so it must never be read. */
static void test_founded_count_zero_returns_false(void) {
    planet_pos_t target = { (fixed_t)12345, (fixed_t)6789 };
    planet_pos_t coincident_but_not_counted[] = { { (fixed_t)12345, (fixed_t)6789 } };

    T_CHECK(rules_discovery_probe_in_range(target, coincident_but_not_counted, 0) == false);
}

int main(void) {
    t_run("probe_cost_credits_matches_traced_value", test_probe_cost_credits_matches_traced_value);
    t_run("probe_cost_months_is_1_not_3", test_probe_cost_months_is_1_not_3);
    t_run("founded_planet_at_closest_achievable_threshold_is_in_range",
          test_founded_planet_at_closest_achievable_threshold_is_in_range);
    t_run("founded_planet_just_beyond_threshold_is_out_of_range",
          test_founded_planet_just_beyond_threshold_is_out_of_range);
    t_run("nearest_founded_planet_governs_result", test_nearest_founded_planet_governs_result);
    t_run("founded_count_zero_returns_false", test_founded_count_zero_returns_false);
    return t_report();
}
