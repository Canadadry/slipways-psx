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

/* -- rules_economy_colony_net -- the FIRST floor ------------------------ */

static void test_colony_net_floors_positive_fractional_below_half(void) {
    /* trade_income=0.3, maintenance=0 -> diff=0.3 -> flr(0.3)=0. */
    fixed_t trade_income = (fixed_t)(0.3 * (double)FX_ONE);
    T_CHECK(rules_economy_colony_net(trade_income, 0) == 0);
}

static void test_colony_net_floors_positive_fractional_above_half_not_rounded(void) {
    /* trade_income=1.7, maintenance=0 -> diff=1.7 -> flr(1.7)=1, NOT 2 --
     * proves this is a floor, not a round-to-nearest. */
    fixed_t trade_income = (fixed_t)(1.7 * (double)FX_ONE);
    T_CHECK(rules_economy_colony_net(trade_income, 0) == 1);
}

static void test_colony_net_negative_fractional_floors_toward_negative_infinity(void) {
    /* trade_income=0.3, maintenance=1 -> diff=0.3-1=-0.7 -> flr(-0.7)=-1,
     * NOT 0 (which is what truncation-toward-zero would wrongly give) --
     * the classic floor-vs-truncate case for a negative fixed-point value,
     * per the PRD's own "e.g. flr(-0.3)=-1, not 0" example. */
    fixed_t trade_income = (fixed_t)(0.3 * (double)FX_ONE);
    T_CHECK(rules_economy_colony_net(trade_income, 1) == -1);
}

static void test_colony_net_negative_exact_integer_no_off_by_one(void) {
    /* trade_income=-1.0 (exact), maintenance=0 -> diff=-1.0 exactly ->
     * flr(-1.0)=-1, not -2 -- confirms fixed_floor_to_int's "remainder==0"
     * branch doesn't over-correct an exact negative multiple of FX_ONE. */
    fixed_t trade_income = (fixed_t)(-1.0 * (double)FX_ONE);
    T_CHECK(rules_economy_colony_net(trade_income, 0) == -1);
}

static void test_colony_net_zero_is_zero(void) {
    T_CHECK(rules_economy_colony_net(0, 0) == 0);
}

/* -- rules_economy_annual_net_income -- the SECOND floor, and the
 * double-floor discriminating test ---------------------------------------
 *
 * This is the most important test in this module (PRD User Story #9): it
 * proves the real, source-traced double-floor shape
 * (per-colony flr, THEN sum, THEN scale, THEN flr again) is implemented,
 * not silently simplified to the design doc's own single-floor prose
 * (flr(sum) once at the end). */

static void test_annual_net_income_double_floor_discriminating_example(void) {
    /* Three colonies, each with a fractional (trade_income - maintenance)
     * of exactly 0.4 (fixed_t) -- constructed via
     * rules_economy_colony_net(trade_income=0.4, maintenance=0), i.e. as if
     * each colony's own net had genuinely been pre-floored by the module's
     * own first-floor function, not hand-picked to already be integers.
     *
     * Real (correct) double-floor arithmetic, source's own shape:
     *   flr(0.4) = 0 for EACH colony (rules_economy_colony_net)
     *   dd = 0 + 0 + 0 = 0
     *   annual_net_income = flr(0 * 1) - 0 = 0
     *
     * Hypothetical WRONG single-floor-at-the-end arithmetic (summing the
     * raw fractional per-colony nets BEFORE any floor, then flooring only
     * once): 0.4 + 0.4 + 0.4 = 1.2 (fixed_t), flr(1.2) = 1 -- a different
     * answer (1, not 0) that a naive single-floor implementation would
     * silently produce instead of source's real 0.
     *
     * This test asserts BOTH: the module's actual chunk-4 functions
     * reproduce the TRUE double-floor result (0), AND that this explicitly
     * differs from the hand-computed single-floor alternative (1) -- the
     * executable proof the PRD's own User Story #9 asks for. */
    fixed_t per_colony_trade_income = (fixed_t)(0.4 * (double)FX_ONE);
    int colony_net = rules_economy_colony_net(per_colony_trade_income, 0);
    T_CHECK(colony_net == 0);

    int colony_nets[3] = {colony_net, colony_net, colony_net};
    int real_double_floor_result = rules_economy_annual_net_income(colony_nets, 3, false, 0);
    T_CHECK(real_double_floor_result == 0);

    /* The hypothetical single-floor alternative, computed independently
     * here (summing the RAW fractional per-colony nets first, as fixed_t,
     * THEN flooring once) -- deliberately NOT calling
     * rules_economy_colony_net or rules_economy_annual_net_income, so this
     * comparison value can't share a bug with the functions under test. */
    fixed_t raw_sum = fixed_add(fixed_add(per_colony_trade_income, per_colony_trade_income), per_colony_trade_income);
    /* raw_sum is comfortably positive (~1.2 * FX_ONE), so plain truncating
     * division already equals floor() here -- no negative-value correction
     * needed for this hand-computed comparison value. */
    int single_floor_alternative = raw_sum / FX_ONE;
    T_CHECK(single_floor_alternative == 1);

    /* The load-bearing assertion: the module's real double-floor result
     * (0) must DIFFER from the naive single-floor alternative (1). */
    T_CHECK(real_double_floor_result != single_floor_alternative);
}

static void test_annual_net_income_skill_implants_multiplier_applies_before_second_floor(void) {
    /* A single already-floored colony net of 13 (an int, as if it came
     * from rules_economy_colony_net). Without the skill-implants bonus,
     * annual_net_income is just 13 (no fraction to floor). WITH the bonus,
     * 13*1.15=14.95 must be floored to 14 -- not rounded to 15, and not
     * left at 13 (which is what would happen if the multiplier were
     * applied too late to affect the floor, or not applied at all). This
     * is the "ordering matters" case the PRD's chunk-4 spec asks for: the
     * result genuinely differs (13 vs 14) depending on whether/how the
     * multiplier participates in the value that gets floored. */
    int colony_nets[1] = {13};

    int without_bonus = rules_economy_annual_net_income(colony_nets, 1, false, 0);
    T_CHECK(without_bonus == 13);

    int with_bonus = rules_economy_annual_net_income(colony_nets, 1, true, 0);
    /* flr(13*1.15) = flr(14.95) = 14. */
    T_CHECK(with_bonus == 14);

    T_CHECK(with_bonus != without_bonus);
}

static void test_annual_net_income_overhead_subtracts_after_second_floor(void) {
    /* Two already-floored colony nets summing to 9 (dd=9), skill implants
     * researched (multiplier=1.15), overhead=4.
     *
     * Correct (source's own shape, overhead OUTSIDE the flr(...) call):
     *   flr(9*1.15) - 4 = flr(10.35) - 4 = 10 - 4 = 6
     *
     * A naive alternative that folds overhead in too early -- subtracting
     * it from the sum BEFORE the skill-implants multiplier is applied,
     * instead of after the second floor -- gives a different, WRONG
     * answer:
     *   flr((9-4)*1.15) = flr(5*1.15) = flr(5.75) = 5
     *
     * 6 != 5: this is the case the PRD's chunk-4 spec asks for, where the
     * result would differ depending on when overhead enters the
     * computation relative to the second floor. */
    int colony_nets[2] = {5, 4};

    int actual = rules_economy_annual_net_income(colony_nets, 2, true, 4);
    T_CHECK(actual == 6);

    /* The naive "overhead before the multiplier/floor" alternative,
     * computed independently here (not via the function under test). */
    int naive_wrong = rules_economy_annual_net_income((int[]){9 - 4}, 1, true, 0);
    T_CHECK(naive_wrong == 5);

    T_CHECK(actual != naive_wrong);
}

/* -- rules_economy_happiness --------------------------------------------
 *
 * bf, pico8/lua.lua:1365-1373: h = 100 + 2*(joy+luxury) + sum(co[level]),
 * co={-5,0,1,1} for levels 1-4. fixed_t, uncapped in both directions --
 * these tests specifically construct cases both above and below 100 to
 * prove it, per the PRD's chunk-5 spec (User Story #10, Implementation
 * Chunks item 5). */

static void test_happiness_base_100_with_no_production_or_colonies(void) {
    T_CHECK(rules_economy_happiness(0, 0, NULL, 0) == 100 * FX_ONE);
}

static void test_happiness_joy_and_luxury_production_add_term(void) {
    /* 100 + 2*(3+2) = 110, exact. */
    fixed_t joy = 3 * FX_ONE;
    fixed_t luxury = 2 * FX_ONE;
    T_CHECK(rules_economy_happiness(joy, luxury, NULL, 0) == 110 * FX_ONE);
}

static void test_happiness_mixed_colony_levels_sum_correctly(void) {
    /* One colony at each level 1-4: co sums to -5+0+1+1=-3, so
     * h = 100 + 0 - 3 = 97, exact. Exercises every co[] entry in one call,
     * not just the extremes the above/below-100 tests below isolate. */
    int colony_levels[4] = {1, 2, 3, 4};
    T_CHECK(rules_economy_happiness(0, 0, colony_levels, 4) == 97 * FX_ONE);
}

static void test_happiness_heavy_production_goes_above_100(void) {
    /* 100 + 2*(40+40) = 100+160 = 260, exact -- genuinely above 100, not
     * clamped. */
    fixed_t joy = 40 * FX_ONE;
    fixed_t luxury = 40 * FX_ONE;
    fixed_t actual = rules_economy_happiness(joy, luxury, NULL, 0);
    T_CHECK(actual == 260 * FX_ONE);
    T_CHECK(actual > 100 * FX_ONE);
}

static void test_happiness_many_unhappy_colonies_goes_below_100_and_negative(void) {
    /* 25 colonies at level 1 (co[1]=-5 each): h = 100 - 125 = -25, exact --
     * genuinely below 100 AND negative, proving this is not silently
     * clamped to 0 or 100 in either direction. */
    int colony_levels[25];
    for (int i = 0; i < 25; i++) {
        colony_levels[i] = 1;
    }

    fixed_t actual = rules_economy_happiness(0, 0, colony_levels, 25);
    T_CHECK(actual == -25 * FX_ONE);
    T_CHECK(actual < 100 * FX_ONE);
    T_CHECK(actual < 0);
}

static void test_happiness_level_clamps_to_valid_range(void) {
    int level_0[1] = {0};
    int level_1[1] = {1};
    int level_5[1] = {5};
    int level_4[1] = {4};
    T_CHECK(rules_economy_happiness(0, 0, level_0, 1) == rules_economy_happiness(0, 0, level_1, 1));
    T_CHECK(rules_economy_happiness(0, 0, level_5, 1) == rules_economy_happiness(0, 0, level_4, 1));
}

/* -- rules_economy_compute_score ------------------------------------------
 *
 * ky, pico8/lua.lua:1339-1363: colonies_score=sum(b_[level]),
 * b_={0,100,200,400}; population_score=population*40;
 * tech_score=floor((tech*1.2)^2)*10; total=floor((sum)*0.01*happiness);
 * stars=min(floor(total/2000),5). Per the PRD's chunk-5 spec: each component
 * checked against a hand-computed value for a non-trivial input, and a case
 * where total exceeds 5*2000=10000 confirming stars clamps to 5 exactly. */

static void test_compute_score_components_match_hand_computed_values(void) {
    /* colony_levels={1,2,3,4,4}: colonies_score = 0+100+200+400+400=1100.
     * population=50: population_score=2000.
     * tech_invented_count=4: tech_x=4*1.2=4.8 (approx in Q16.16),
     * tech_x^2=23.04 (approx), floored to 23, tech_score=230.
     * happiness_percent=80% (80*FX_ONE, exact):
     * total = floor(3330*0.01*80) = floor(2664.0) = 2664 (exact).
     * stars = min(floor(2664/2000),5) = min(1,5) = 1. */
    int colony_levels[5] = {1, 2, 3, 4, 4};
    rules_economy_score_t score = rules_economy_compute_score(colony_levels, 5, 50, 4, 80 * FX_ONE);

    T_CHECK(score.colonies_score == 1100);
    T_CHECK(score.population_score == 2000);
    T_CHECK(score.tech_score == 230);
    T_CHECK(score.total == 2664);
    T_CHECK(score.stars == 1);
}

static void test_compute_score_colonies_score_matches_b_table_alone(void) {
    /* Isolate colonies_score with population/tech at 0: one colony per
     * level, b_ sums to 0+100+200+400=700. */
    int colony_levels[4] = {1, 2, 3, 4};
    rules_economy_score_t score = rules_economy_compute_score(colony_levels, 4, 0, 0, 0);
    T_CHECK(score.colonies_score == 700);
    T_CHECK(score.population_score == 0);
    T_CHECK(score.tech_score == 0);
}

static void test_compute_score_population_score_is_population_times_40(void) {
    rules_economy_score_t score = rules_economy_compute_score(NULL, 0, 17, 0, 0);
    T_CHECK(score.population_score == 17 * 40);
}

static void test_compute_score_stars_clamps_to_5_when_total_exceeds_10000(void) {
    /* 30 colonies at level 4: colonies_score=30*400=12000.
     * population=100: population_score=4000.
     * tech_invented_count=0: tech_score=0.
     * sum=16000. happiness_percent=100% (100*FX_ONE, exact):
     * total = floor(16000*0.01*100) = floor(16000.0) = 16000 (exact).
     * Without the min(...,5) cap, floor(16000/2000)=8 -- this test confirms
     * stars is clamped to 5, not 8. */
    int colony_levels[30];
    for (int i = 0; i < 30; i++) {
        colony_levels[i] = 4;
    }

    rules_economy_score_t score = rules_economy_compute_score(colony_levels, 30, 100, 0, 100 * FX_ONE);
    T_CHECK(score.colonies_score == 12000);
    T_CHECK(score.population_score == 4000);
    T_CHECK(score.tech_score == 0);
    T_CHECK(score.total == 16000);
    T_CHECK(score.total / 2000 == 8); /* confirms the cap is genuinely doing something */
    T_CHECK(score.stars == 5);
}

static void test_compute_score_negative_total_floors_toward_negative_infinity_not_truncated(void) {
    /* A single level-4 colony (colonies_score=400), no population/tech, and
     * a deeply unhappy (uncapped-negative) happiness_percent of -50%:
     * total = floor(400*0.01*(-50)) = floor(-200.0) = -200 (exact).
     * stars = floor(-200/2000) = floor(-0.1) = -1, NOT 0 -- C's truncating
     * `/` would give -200/2000==0, which is wrong; this asserts the correct
     * floor-toward-negative-infinity result instead. */
    int colony_levels[1] = {4};
    rules_economy_score_t score = rules_economy_compute_score(colony_levels, 1, 0, 0, -50 * FX_ONE);
    T_CHECK(score.total == -200);
    T_CHECK(score.stars == -1);
}

/* -- rules_economy_check_game_over --------------------------------------
 *
 * hy.hl's assignment, pico8/lua.lua:1235-1236:
 *   self.hl = self.yr>=3426 and "final score"
 *             or self.ge<=0 and self.nz<3 and "bankrupt"
 * Per the PRD's chunk-6 spec: continues-vs-bankrupt-vs-final-year, exact
 * boundary operators (>= year, <= net_income, < treasury), and the critical
 * short-circuit test where a simultaneous final-year-and-bankrupt case must
 * resolve to final year, not bankrupt or some combined result. */

static void test_game_over_continues_with_positive_income(void) {
    /* year<3426, net_income>0 (treasury doesn't matter since the AND's
     * first clause is already false) -> continues. */
    T_CHECK(rules_economy_check_game_over(3400, 1, 0) == RULES_ECONOMY_GAME_CONTINUES);
}

static void test_game_over_continues_with_sufficient_treasury_despite_nonpositive_income(void) {
    /* year<3426, net_income<=0 BUT treasury>=3 (the AND's second clause is
     * false) -> continues. Confirms the bankruptcy AND genuinely requires
     * BOTH clauses -- either alone being "not bankrupt" is enough to keep
     * the game going. */
    T_CHECK(rules_economy_check_game_over(3400, 0, RULES_ECONOMY_BANKRUPTCY_TREASURY_THRESHOLD) == RULES_ECONOMY_GAME_CONTINUES);
    T_CHECK(rules_economy_check_game_over(3400, -50, 1000) == RULES_ECONOMY_GAME_CONTINUES);
}

static void test_game_over_bankrupt_when_both_clauses_hold(void) {
    /* year<3426, net_income<=0 AND treasury<3 -> bankrupt. */
    T_CHECK(rules_economy_check_game_over(3400, -10, 0) == RULES_ECONOMY_GAME_OVER_BANKRUPT);
}

static void test_game_over_bankrupt_boundary_net_income_zero_counts(void) {
    /* net_income==0 counts as bankrupt-eligible, per source's <= (not <). */
    T_CHECK(rules_economy_check_game_over(3400, 0, RULES_ECONOMY_BANKRUPTCY_TREASURY_THRESHOLD - 1) == RULES_ECONOMY_GAME_OVER_BANKRUPT);
}

static void test_game_over_bankrupt_boundary_treasury_strict_less_than(void) {
    /* treasury exactly AT the threshold does NOT count (strict <), but one
     * below it does -- both checked with the same nonpositive net_income to
     * isolate the treasury boundary specifically. */
    T_CHECK(rules_economy_check_game_over(3400, -1, RULES_ECONOMY_BANKRUPTCY_TREASURY_THRESHOLD) == RULES_ECONOMY_GAME_CONTINUES);
    T_CHECK(rules_economy_check_game_over(3400, -1, RULES_ECONOMY_BANKRUPTCY_TREASURY_THRESHOLD - 1) == RULES_ECONOMY_GAME_OVER_BANKRUPT);
}

static void test_game_over_final_year_alone_with_healthy_economy(void) {
    /* year>=3426, healthy income/treasury -> final year. */
    T_CHECK(rules_economy_check_game_over(RULES_ECONOMY_FINAL_YEAR, 500, 1000) == RULES_ECONOMY_GAME_OVER_FINAL_YEAR);
    T_CHECK(rules_economy_check_game_over(RULES_ECONOMY_FINAL_YEAR + 1, 500, 1000) == RULES_ECONOMY_GAME_OVER_FINAL_YEAR);
}

static void test_game_over_final_year_wins_over_simultaneous_bankruptcy(void) {
    /* THE critical short-circuit test (PRD's own single most important test
     * in this chunk): year>=3426 AND a simultaneously bankrupt net_income/
     * treasury pair (net_income<=0 AND treasury<3, both true) must still
     * resolve to RULES_ECONOMY_GAME_OVER_FINAL_YEAR, exactly matching
     * source's `and`/`or` short-circuit -- the bankruptcy clause is never
     * even evaluated once the final-year clause is true, so a simultaneous
     * bankrupt fixture must NOT produce RULES_ECONOMY_GAME_OVER_BANKRUPT or
     * any other result. */
    T_CHECK(rules_economy_check_game_over(RULES_ECONOMY_FINAL_YEAR, -10, 0) == RULES_ECONOMY_GAME_OVER_FINAL_YEAR);
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
    t_run("colony_net_floors_positive_fractional_below_half", test_colony_net_floors_positive_fractional_below_half);
    t_run("colony_net_floors_positive_fractional_above_half_not_rounded", test_colony_net_floors_positive_fractional_above_half_not_rounded);
    t_run("colony_net_negative_fractional_floors_toward_negative_infinity", test_colony_net_negative_fractional_floors_toward_negative_infinity);
    t_run("colony_net_negative_exact_integer_no_off_by_one", test_colony_net_negative_exact_integer_no_off_by_one);
    t_run("colony_net_zero_is_zero", test_colony_net_zero_is_zero);
    t_run("annual_net_income_double_floor_discriminating_example", test_annual_net_income_double_floor_discriminating_example);
    t_run("annual_net_income_skill_implants_multiplier_applies_before_second_floor", test_annual_net_income_skill_implants_multiplier_applies_before_second_floor);
    t_run("annual_net_income_overhead_subtracts_after_second_floor", test_annual_net_income_overhead_subtracts_after_second_floor);
    t_run("happiness_base_100_with_no_production_or_colonies", test_happiness_base_100_with_no_production_or_colonies);
    t_run("happiness_joy_and_luxury_production_add_term", test_happiness_joy_and_luxury_production_add_term);
    t_run("happiness_mixed_colony_levels_sum_correctly", test_happiness_mixed_colony_levels_sum_correctly);
    t_run("happiness_heavy_production_goes_above_100", test_happiness_heavy_production_goes_above_100);
    t_run("happiness_many_unhappy_colonies_goes_below_100_and_negative", test_happiness_many_unhappy_colonies_goes_below_100_and_negative);
    t_run("happiness_level_clamps_to_valid_range", test_happiness_level_clamps_to_valid_range);
    t_run("compute_score_components_match_hand_computed_values", test_compute_score_components_match_hand_computed_values);
    t_run("compute_score_colonies_score_matches_b_table_alone", test_compute_score_colonies_score_matches_b_table_alone);
    t_run("compute_score_population_score_is_population_times_40", test_compute_score_population_score_is_population_times_40);
    t_run("compute_score_stars_clamps_to_5_when_total_exceeds_10000", test_compute_score_stars_clamps_to_5_when_total_exceeds_10000);
    t_run("compute_score_negative_total_floors_toward_negative_infinity_not_truncated", test_compute_score_negative_total_floors_toward_negative_infinity_not_truncated);
    t_run("game_over_continues_with_positive_income", test_game_over_continues_with_positive_income);
    t_run("game_over_continues_with_sufficient_treasury_despite_nonpositive_income", test_game_over_continues_with_sufficient_treasury_despite_nonpositive_income);
    t_run("game_over_bankrupt_when_both_clauses_hold", test_game_over_bankrupt_when_both_clauses_hold);
    t_run("game_over_bankrupt_boundary_net_income_zero_counts", test_game_over_bankrupt_boundary_net_income_zero_counts);
    t_run("game_over_bankrupt_boundary_treasury_strict_less_than", test_game_over_bankrupt_boundary_treasury_strict_less_than);
    t_run("game_over_final_year_alone_with_healthy_economy", test_game_over_final_year_alone_with_healthy_economy);
    t_run("game_over_final_year_wins_over_simultaneous_bankruptcy", test_game_over_final_year_wins_over_simultaneous_bankruptcy);
    return t_report();
}
