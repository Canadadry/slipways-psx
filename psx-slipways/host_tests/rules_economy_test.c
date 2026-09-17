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

int main(void) {
    t_run("trade_level_bonus_level_1_matches_traced_value", test_trade_level_bonus_level_1_matches_traced_value);
    t_run("trade_level_bonus_level_2_matches_traced_value_exactly", test_trade_level_bonus_level_2_matches_traced_value_exactly);
    t_run("trade_level_bonus_level_3_matches_traced_value", test_trade_level_bonus_level_3_matches_traced_value);
    t_run("trade_level_bonus_level_4_matches_traced_value", test_trade_level_bonus_level_4_matches_traced_value);
    t_run("trade_level_bonus_below_1_clamps_to_level_1", test_trade_level_bonus_below_1_clamps_to_level_1);
    t_run("trade_level_bonus_above_4_clamps_to_level_4", test_trade_level_bonus_above_4_clamps_to_level_4);
    return t_report();
}
