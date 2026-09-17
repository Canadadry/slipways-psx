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
    return t_report();
}
