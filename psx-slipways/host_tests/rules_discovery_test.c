#include "t.h"
#include "../fixed.h"
#include "../rng.h"
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

/* Chunk 3: the empty-roll generator (jo.ir=jf(0.8,0.5), pico8/lua.lua:333-
 * 338,750-751; jo:hh()'s "<0.45" test, line 408). Mirrors planetgen_test.c's
 * test_same_seed_is_deterministic pattern exactly: same seed and same
 * initial state must reproduce an identical sequence of results AND of the
 * internal bias state, not just matching booleans. */
static void test_same_seed_and_state_is_deterministic(void) {
    rng_t rng_a, rng_b;
    rng_seed(&rng_a, 99u);
    rng_seed(&rng_b, 99u);

    rules_discovery_empty_roll_t state_a, state_b;
    rules_discovery_empty_roll_init(&state_a);
    rules_discovery_empty_roll_init(&state_b);

    for (int i = 0; i < 20; i++) {
        bool result_a = rules_discovery_roll_empty(&state_a, &rng_a);
        bool result_b = rules_discovery_roll_empty(&state_b, &rng_b);

        T_CHECK(result_a == result_b);
        T_CHECK(state_a.bias == state_b.bias);
    }
}

/* Proves this is a genuine stateful, mean-reverting walk -- not a stateless
 * constant-probability stand-in wearing a struct. bias must visibly move
 * away from its FX_ONE/2 (0.5) initial value across a handful of calls. */
static void test_bias_evolves_across_calls(void) {
    rng_t rng;
    rng_seed(&rng, 99u);

    rules_discovery_empty_roll_t state;
    rules_discovery_empty_roll_init(&state);

    T_CHECK(state.bias == FX_ONE / 2);

    for (int i = 0; i < 5; i++) {
        rules_discovery_roll_empty(&state, &rng);
    }

    T_CHECK(state.bias != FX_ONE / 2);
}

/* Hand-computed check, seed=1, first two calls of jf(0.8,0.5) stepped by
 * hand (all values are exact Q16.16 int32 arithmetic, cross-checked against
 * a standalone scratch build of the real rng.c/fixed.h against this exact
 * seed -- not "some plausible value"):
 *
 * rng_seed(1): state=1 (seed!=0, used as-is).
 *
 * Call 1, bias b0 = FX_ONE/2 = 32768 (0.5):
 *   span = FX_ONE - |b0| = 65536 - 32768 = 32768               (1-abs(b)=0.5)
 *   rng_next_u32(state=1): x=1; x^=x<<13 -> 1^8192=8193;
 *     x^=x>>17 -> 8193^0=8193 (8193 < 2^17); x^=x<<5 -> 8193^262176=270369.
 *     new state=270369, returned raw=270369.
 *   drawn = 270369 % 32768 = 8225                               (rnd(0.5))
 *   v0 = drawn + max(b0,0) = 8225 + 32768 = 40993               (0.625504)
 *   v0 (40993) is NOT < threshold (FX_ONE*45/100 = 29491), so the roll is
 *     "not empty" (false).
 *   b1 = b0 + fixed_mul(FX_ONE/2 - v0, n_)
 *      = 32768 + fixed_mul(32768-40993, 52428)
 *      = 32768 + fixed_mul(-8225, 52428)
 *      = 32768 + ((-8225*52428)>>16) = 32768 + (-431221900>>16)
 *      = 32768 + (-6580) = 26188                                (0.399597)
 *     (n_ = (fixed_t)(0.8*FX_ONE) = 52428, matching
 *     RULES_DISCOVERY_EMPTY_ROLL_N.)
 *
 * Call 2, bias b1 = 26188:
 *   span = 65536 - 26188 = 39348                                (0.600403)
 *   rng_next_u32(state=270369): computed result = 67634689 (raw next
 *     xorshift output from that intermediate state -- taken as given from
 *     the algorithm, same as any second call in a deterministic sequence).
 *   drawn = 67634689 % 39348 = 34825                            (0.531387)
 *   v1 = drawn + max(b1,0) = 34825 + 26188 = 61013               (0.930984)
 *   v1 (61013) is NOT < threshold (29491), so the roll is "not empty"
 *     (false) again.
 *
 * These are exact, checkable values, not "some plausible" ones. */
static void test_hand_derived_first_two_rolls_match_seed_1(void) {
    rng_t rng;
    rng_seed(&rng, 1u);

    rules_discovery_empty_roll_t state;
    rules_discovery_empty_roll_init(&state);

    bool empty_1 = rules_discovery_roll_empty(&state, &rng);
    T_CHECK(empty_1 == false);
    T_CHECK(state.bias == 26188);

    bool empty_2 = rules_discovery_roll_empty(&state, &rng);
    T_CHECK(empty_2 == false);
    T_CHECK(state.bias == 3592);
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
    t_run("same_seed_and_state_is_deterministic", test_same_seed_and_state_is_deterministic);
    t_run("bias_evolves_across_calls", test_bias_evolves_across_calls);
    t_run("hand_derived_first_two_rolls_match_seed_1", test_hand_derived_first_two_rolls_match_seed_1);
    return t_report();
}
