#include "t.h"
#include "../fixed.h"
#include "../rules_economy.h"

/* -0.333/0.25/0.5 are not exactly representable in Q16.16, so exercise them
 * within a tight documented tolerance rather than requiring exact equality
 * (level 2's traced value, 0, IS compared exactly below since 0 is exactly
 * representable) -- matching host_tests/rng_and_fixed_test.c's own
 * EPSILON=0.001 tolerance convention for fixed_sqrt's irrational-rounding
 * test. */
#define TOLERANCE ((fixed_t)(0.001 * (double)FX_ONE))

static int fixed_approx_equal(fixed_t a, fixed_t b) {
    fixed_t diff = (a > b) ? (a - b) : (b - a);
    return diff <= TOLERANCE;
}

static void test_trade_level_bonus_level_1_matches_traced_value(void) {
    T_CHECK(fixed_approx_equal(rules_economy_trade_level_bonus(1), (fixed_t)(-0.333 * (double)FX_ONE)));
}

static void test_trade_level_bonus_level_2_matches_traced_value_exactly(void) {
    T_CHECK(rules_economy_trade_level_bonus(2) == 0);
}

static void test_trade_level_bonus_level_3_matches_traced_value(void) {
    T_CHECK(fixed_approx_equal(rules_economy_trade_level_bonus(3), (fixed_t)(0.25 * (double)FX_ONE)));
}

static void test_trade_level_bonus_level_4_matches_traced_value(void) {
    T_CHECK(fixed_approx_equal(rules_economy_trade_level_bonus(4), (fixed_t)(0.5 * (double)FX_ONE)));
}

static void test_trade_level_bonus_below_1_clamps_to_level_1(void) {
    T_CHECK(rules_economy_trade_level_bonus(0) == rules_economy_trade_level_bonus(1));
    T_CHECK(rules_economy_trade_level_bonus(-1) == rules_economy_trade_level_bonus(1));
}

static void test_trade_level_bonus_above_4_clamps_to_level_4(void) {
    T_CHECK(rules_economy_trade_level_bonus(5) == rules_economy_trade_level_bonus(4));
    T_CHECK(rules_economy_trade_level_bonus(100) == rules_economy_trade_level_bonus(4));
}

/* -- rules_economy_colony_maintenance --------------------------------- */

static void test_colony_maintenance_laboratory_branch_ignores_level(void) {
    /* Laboratory branch returns laboratory_count regardless of level --
     * exercise a couple of different level values with the same
     * laboratory_count and confirm level is genuinely ignored. */
    T_CHECK(rules_economy_colony_maintenance(true, 7, false, 1) == 7);
    T_CHECK(rules_economy_colony_maintenance(true, 7, false, 4) == 7);
    T_CHECK(rules_economy_colony_maintenance(true, 7, true, 2) == 7);
    T_CHECK(rules_economy_colony_maintenance(true, 0, false, 3) == 0);
}

static void test_colony_maintenance_colonized_branch_matches_es_table(void) {
    /* es = {1,0,1,2} for levels 1-4. */
    T_CHECK(rules_economy_colony_maintenance(false, 0, true, 1) == 1);
    T_CHECK(rules_economy_colony_maintenance(false, 0, true, 2) == 0);
    T_CHECK(rules_economy_colony_maintenance(false, 0, true, 3) == 1);
    T_CHECK(rules_economy_colony_maintenance(false, 0, true, 4) == 2);
}

static void test_colony_maintenance_colonized_branch_clamps_level(void) {
    T_CHECK(rules_economy_colony_maintenance(false, 0, true, 0) == rules_economy_colony_maintenance(false, 0, true, 1));
    T_CHECK(rules_economy_colony_maintenance(false, 0, true, -1) == rules_economy_colony_maintenance(false, 0, true, 1));
    T_CHECK(rules_economy_colony_maintenance(false, 0, true, 5) == rules_economy_colony_maintenance(false, 0, true, 4));
    T_CHECK(rules_economy_colony_maintenance(false, 0, true, 100) == rules_economy_colony_maintenance(false, 0, true, 4));
}

static void test_colony_maintenance_neither_condition_returns_zero(void) {
    T_CHECK(rules_economy_colony_maintenance(false, 5, false, 1) == 0);
    T_CHECK(rules_economy_colony_maintenance(false, 5, false, 2) == 0);
    T_CHECK(rules_economy_colony_maintenance(false, 5, false, 3) == 0);
    T_CHECK(rules_economy_colony_maintenance(false, 5, false, 4) == 0);
}

/* -- rules_economy_overhead --------------------------------------------- */

static void test_overhead_matches_hand_computed_floor(void) {
    /* floor(0.18*n^2) hand-computed for each n:
     *   n=0:  0.18*0   = 0     -> 0
     *   n=1:  0.18*1   = 0.18  -> 0
     *   n=5:  0.18*25  = 4.5   -> 4
     *   n=10: 0.18*100 = 18    -> 18 (exact)
     *   n=23: 0.18*529 = 95.22 -> 95
     */
    T_CHECK(rules_economy_overhead(0) == 0);
    T_CHECK(rules_economy_overhead(1) == 0);
    T_CHECK(rules_economy_overhead(5) == 4);
    T_CHECK(rules_economy_overhead(10) == 18);
    T_CHECK(rules_economy_overhead(23) == 95);
}

/* -- rules_economy_trade_edge_income ------------------------------------ */

/* Hand-derived expected values, straight from the traced `ct` table
 * ({-0.333, 0, 0.25, 0.5} for levels 1-4, pico8/lua.lua:1323-1324) and the
 * formula `trade_difficulty * sensitivity * (1 + ct[self]+ct[partner])`
 * (pico8/lua.lua:1333) -- computed independently of
 * rules_economy_trade_level_bonus itself, so a shared bug in both the
 * lookup and this test wouldn't accidentally cancel out. Uses
 * trade_difficulty=6 (RULES_ECONOMY_TRADE_REASONABLE), sensitivity=1.0
 * (FX_ONE, the default `hs` for a regular colony). */
static void test_trade_edge_income_level_1_1_hand_derived(void) {
    /* 6 * 1 * (1 + (-0.333) + (-0.333)) = 6 * 0.334 = 2.004 */
    fixed_t expected = (fixed_t)(2.004 * (double)FX_ONE);
    fixed_t actual = rules_economy_trade_edge_income(RULES_ECONOMY_TRADE_REASONABLE, FX_ONE, 1, 1);
    T_CHECK(fixed_approx_equal(actual, expected));
}

static void test_trade_edge_income_level_1_4_hand_derived(void) {
    /* 6 * 1 * (1 + (-0.333) + 0.5) = 6 * 1.167 = 7.002 */
    fixed_t expected = (fixed_t)(7.002 * (double)FX_ONE);
    fixed_t actual = rules_economy_trade_edge_income(RULES_ECONOMY_TRADE_REASONABLE, FX_ONE, 1, 4);
    T_CHECK(fixed_approx_equal(actual, expected));
}

static void test_trade_edge_income_level_4_4_hand_derived(void) {
    /* 6 * 1 * (1 + 0.5 + 0.5) = 6 * 2 = 12 (exact) */
    fixed_t expected = 12 * FX_ONE;
    fixed_t actual = rules_economy_trade_edge_income(RULES_ECONOMY_TRADE_REASONABLE, FX_ONE, 4, 4);
    T_CHECK(fixed_approx_equal(actual, expected));
}

static void test_trade_edge_income_level_2_3_hand_derived(void) {
    /* 6 * 1 * (1 + 0 + 0.25) = 6 * 1.25 = 7.5 (exact) */
    fixed_t expected = (fixed_t)(7.5 * (double)FX_ONE);
    fixed_t actual = rules_economy_trade_edge_income(RULES_ECONOMY_TRADE_REASONABLE, FX_ONE, 2, 3);
    T_CHECK(fixed_approx_equal(actual, expected));
}

/* Full [1,4]x[1,4] sweep, all 16 combinations, checked against the same
 * traced `ct` table values ({-0.333,0,0.25,0.5}) written out directly here
 * (NOT re-derived via rules_economy_trade_level_bonus, so this loop can't
 * trivially pass just because both the table and the lookup share the same
 * bug), for two different (trade_difficulty, sensitivity) pairs to also
 * exercise a non-1.0 sensitivity and a different trade_difficulty value. */
static void test_trade_edge_income_full_level_sweep_matches_hand_written_formula(void) {
    static const double ct[4] = {-0.333, 0.0, 0.25, 0.5};

    for (int self_level = 1; self_level <= 4; self_level++) {
        for (int partner_level = 1; partner_level <= 4; partner_level++) {
            double bonus = 1.0 + ct[self_level - 1] + ct[partner_level - 1];

            /* Pair A: trade_difficulty=7 (FORGIVING), sensitivity=1.0. */
            {
                double expected_d = 7.0 * 1.0 * bonus;
                fixed_t expected = (fixed_t)(expected_d * (double)FX_ONE);
                fixed_t actual = rules_economy_trade_edge_income(RULES_ECONOMY_TRADE_FORGIVING, FX_ONE, self_level, partner_level);
                T_CHECK(fixed_approx_equal(actual, expected));
            }

            /* Pair B: trade_difficulty=4 (TOUGH), sensitivity=0.5 (e.g. a
             * Food-processor-like entity's `hs`, pico8/lua.lua:1096). */
            {
                fixed_t half = FX_ONE / 2;
                double expected_d = 4.0 * 0.5 * bonus;
                fixed_t expected = (fixed_t)(expected_d * (double)FX_ONE);
                fixed_t actual = rules_economy_trade_edge_income(RULES_ECONOMY_TRADE_TOUGH, half, self_level, partner_level);
                T_CHECK(fixed_approx_equal(actual, expected));
            }
        }
    }
}

/* -- rules_economy_colony_trade_income ----------------------------------- */

static void test_colony_trade_income_no_partners_no_luxury_is_zero(void) {
    fixed_t actual = rules_economy_colony_trade_income(RULES_ECONOMY_TRADE_REASONABLE, FX_ONE, 2, NULL, 0, false, 0);
    T_CHECK(actual == 0);
}

static void test_colony_trade_income_multiple_edges_sum_correctly(void) {
    /* 3 partners at levels 1, 2, 4; self_level=3; trade_difficulty=5
     * (CHALLENGING); sensitivity=1.0 (FX_ONE). Confirm the total equals the
     * sum of the individual per-edge calls (not re-derived from the raw
     * formula -- this specifically checks the summing behavior). */
    int partner_levels[3] = {1, 2, 4};
    fixed_t expected = fixed_add(
        fixed_add(rules_economy_trade_edge_income(RULES_ECONOMY_TRADE_CHALLENGING, FX_ONE, 3, 1),
                  rules_economy_trade_edge_income(RULES_ECONOMY_TRADE_CHALLENGING, FX_ONE, 3, 2)),
        rules_economy_trade_edge_income(RULES_ECONOMY_TRADE_CHALLENGING, FX_ONE, 3, 4));

    fixed_t actual = rules_economy_colony_trade_income(RULES_ECONOMY_TRADE_CHALLENGING, FX_ONE, 3, partner_levels, 3, false, 0);
    T_CHECK(fixed_approx_equal(actual, expected));
}

static void test_colony_trade_income_luxury_bonus_absent_when_flag_false(void) {
    int partner_levels[2] = {2, 3};
    fixed_t without_luxury = rules_economy_colony_trade_income(RULES_ECONOMY_TRADE_TOUGH, FX_ONE, 2, partner_levels, 2, false, 5 * FX_ONE);
    fixed_t edges_only = fixed_add(rules_economy_trade_edge_income(RULES_ECONOMY_TRADE_TOUGH, FX_ONE, 2, 2),
                                    rules_economy_trade_edge_income(RULES_ECONOMY_TRADE_TOUGH, FX_ONE, 2, 3));
    T_CHECK(fixed_approx_equal(without_luxury, edges_only));
}

static void test_colony_trade_income_luxury_bonus_scales_linearly_with_production(void) {
    /* luxury bonus = luxury_production * trade_difficulty * 1.5. No trade
     * edges here (partner_count=0), so the whole result IS the bonus term,
     * isolating it cleanly. trade_difficulty=7 (FORGIVING). */
    fixed_t bonus_at_2 = rules_economy_colony_trade_income(RULES_ECONOMY_TRADE_FORGIVING, FX_ONE, 1, NULL, 0, true, 2 * FX_ONE);
    fixed_t bonus_at_4 = rules_economy_colony_trade_income(RULES_ECONOMY_TRADE_FORGIVING, FX_ONE, 1, NULL, 0, true, 4 * FX_ONE);

    /* Hand-derived: production=2 -> 2*7*1.5=21; production=4 -> 4*7*1.5=42. */
    T_CHECK(fixed_approx_equal(bonus_at_2, 21 * FX_ONE));
    T_CHECK(fixed_approx_equal(bonus_at_4, 42 * FX_ONE));
    /* Doubling production doubles the bonus (linear scaling). */
    T_CHECK(fixed_approx_equal(fixed_mul(bonus_at_2, 2 * FX_ONE), bonus_at_4));
}

static void test_colony_trade_income_all_four_difficulties(void) {
    /* Exercise all four named trade_difficulty values at least once, via
     * the luxury-bonus-only isolation trick above (no edges), each checked
     * against the hand-derived bonus = production * difficulty * 1.5, with
     * production fixed at 1.0 (FX_ONE) throughout. */
    T_CHECK(fixed_approx_equal(
        rules_economy_colony_trade_income(RULES_ECONOMY_TRADE_FORGIVING, FX_ONE, 1, NULL, 0, true, FX_ONE),
        (fixed_t)(7.0 * 1.5 * (double)FX_ONE)));
    T_CHECK(fixed_approx_equal(
        rules_economy_colony_trade_income(RULES_ECONOMY_TRADE_REASONABLE, FX_ONE, 1, NULL, 0, true, FX_ONE),
        (fixed_t)(6.0 * 1.5 * (double)FX_ONE)));
    T_CHECK(fixed_approx_equal(
        rules_economy_colony_trade_income(RULES_ECONOMY_TRADE_CHALLENGING, FX_ONE, 1, NULL, 0, true, FX_ONE),
        (fixed_t)(5.0 * 1.5 * (double)FX_ONE)));
    T_CHECK(fixed_approx_equal(
        rules_economy_colony_trade_income(RULES_ECONOMY_TRADE_TOUGH, FX_ONE, 1, NULL, 0, true, FX_ONE),
        (fixed_t)(4.0 * 1.5 * (double)FX_ONE)));
}

int main(void) {
    t_run("trade_level_bonus_level_1_matches_traced_value", test_trade_level_bonus_level_1_matches_traced_value);
    t_run("trade_level_bonus_level_2_matches_traced_value_exactly", test_trade_level_bonus_level_2_matches_traced_value_exactly);
    t_run("trade_level_bonus_level_3_matches_traced_value", test_trade_level_bonus_level_3_matches_traced_value);
    t_run("trade_level_bonus_level_4_matches_traced_value", test_trade_level_bonus_level_4_matches_traced_value);
    t_run("trade_level_bonus_below_1_clamps_to_level_1", test_trade_level_bonus_below_1_clamps_to_level_1);
    t_run("trade_level_bonus_above_4_clamps_to_level_4", test_trade_level_bonus_above_4_clamps_to_level_4);
    t_run("colony_maintenance_laboratory_branch_ignores_level", test_colony_maintenance_laboratory_branch_ignores_level);
    t_run("colony_maintenance_colonized_branch_matches_es_table", test_colony_maintenance_colonized_branch_matches_es_table);
    t_run("colony_maintenance_colonized_branch_clamps_level", test_colony_maintenance_colonized_branch_clamps_level);
    t_run("colony_maintenance_neither_condition_returns_zero", test_colony_maintenance_neither_condition_returns_zero);
    t_run("overhead_matches_hand_computed_floor", test_overhead_matches_hand_computed_floor);
    t_run("trade_edge_income_level_1_1_hand_derived", test_trade_edge_income_level_1_1_hand_derived);
    t_run("trade_edge_income_level_1_4_hand_derived", test_trade_edge_income_level_1_4_hand_derived);
    t_run("trade_edge_income_level_4_4_hand_derived", test_trade_edge_income_level_4_4_hand_derived);
    t_run("trade_edge_income_level_2_3_hand_derived", test_trade_edge_income_level_2_3_hand_derived);
    t_run("trade_edge_income_full_level_sweep_matches_hand_written_formula", test_trade_edge_income_full_level_sweep_matches_hand_written_formula);
    t_run("colony_trade_income_no_partners_no_luxury_is_zero", test_colony_trade_income_no_partners_no_luxury_is_zero);
    t_run("colony_trade_income_multiple_edges_sum_correctly", test_colony_trade_income_multiple_edges_sum_correctly);
    t_run("colony_trade_income_luxury_bonus_absent_when_flag_false", test_colony_trade_income_luxury_bonus_absent_when_flag_false);
    t_run("colony_trade_income_luxury_bonus_scales_linearly_with_production", test_colony_trade_income_luxury_bonus_scales_linearly_with_production);
    t_run("colony_trade_income_all_four_difficulties", test_colony_trade_income_all_four_difficulties);
    return t_report();
}
