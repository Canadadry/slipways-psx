#include "rules_economy.h"

/* ct, pico8/lua.lua:1323-1324: {-0.333, 0, 0.25, 0.5} for levels 1-4.
 * Initialized from traced double literals via the same
 * "(fixed_t)(<literal> * (double)FX_ONE)" idiom rules_planets.c's own
 * g_planet_types[]/g_jobs[] tables use for their fixed_t fields (e.g.
 * PLANET_TYPE_EARTHLIKE's rarity, "(fixed_t)(4.5 * FX_ONE)") -- fixed_from_
 * double() is not usable here since it is not a compile-time constant
 * expression, and a static const array initializer must be one. Index 0..3
 * represents levels 1..4, mirroring rules_planets.c's g_curve_minus/
 * g_curve_plus tables' own indexing convention. */
static const fixed_t g_trade_level_bonus[4] = {
    (fixed_t)(-0.333 * (double)FX_ONE),
    (fixed_t)(0 * (double)FX_ONE),
    (fixed_t)(0.25 * (double)FX_ONE),
    (fixed_t)(0.5 * (double)FX_ONE),
};

fixed_t rules_economy_trade_level_bonus(int level) {
    /* Clamp to [1,4], mirroring rules_planets_curve_value's own clamping
     * idiom (rules_planets.c) exactly -- two separate "if (level < N) level
     * = N;" guards rather than a single min/max expression. Unlike that
     * function, there is no "level < 1 returns 0" special case: ct has no
     * natural zero entry, so both ends clamp to the nearest in-range
     * boundary value instead of a sentinel. */
    if (level < 1) {
        level = 1;
    }
    if (level > 4) {
        level = 4;
    }

    return g_trade_level_bonus[level - 1];
}

/* es, pico8/lua.lua:1311: {1,0,1,2} for levels 1-4. Kept private to this
 * file, not declared in rules_economy.h -- per chunk 1's own precedent
 * (g_trade_level_bonus above) of keeping unexposed source tables static and
 * file-local, and since chunk 1's own report explicitly deferred `es`
 * specifically to this chunk. Index 0..3 represents levels 1..4, matching
 * g_trade_level_bonus's own indexing convention. All-integer, unlike `ct`,
 * so no fixed_t/double literal folding is needed here. */
static const int g_colony_maintenance_cost[4] = {1, 0, 1, 2};

int rules_economy_colony_maintenance(bool is_laboratory, int laboratory_count, bool colonized, int level) {
    if (is_laboratory) {
        /* pt.qh branch: costs the current total Laboratory count,
         * regardless of level. */
        return laboratory_count;
    }

    if (colonized) {
        /* pt:gg() branch: costs es[level]. Clamp to [1,4], mirroring
         * rules_economy_trade_level_bonus's own clamping idiom exactly. */
        if (level < 1) {
            level = 1;
        }
        if (level > 4) {
            level = 4;
        }

        return g_colony_maintenance_cost[level - 1];
    }

    return 0;
}

/* db()'s external-trade ("luxury") bonus multiplier, pico8/lua.lua:1336:
 * `total += pt.kw.mu * dk.trade * 1.5`. 1.5 is exactly representable in
 * Q16.16 (1.5*65536=98304, no rounding), so this is a plain double-literal
 * fold, matching rules_planets_job_cost's own
 * RULES_PLANETS_TECH_GATE_MULTIPLIER idiom exactly (rules_planets.c). Kept
 * file-local: no other consumer, per rules_economy.c's own established
 * "keep unexposed constants static/#define, not in the header" precedent
 * (see g_colony_maintenance_cost above). */
#define RULES_ECONOMY_LUXURY_TRADE_MULTIPLIER ((fixed_t)(1.5 * FX_ONE))

fixed_t rules_economy_trade_edge_income(int trade_difficulty, fixed_t sensitivity, int self_level, int partner_level) {
    /* dk.trade * pt.hs * (1+ct[pt.mi]+ct[oz.mi]), pico8/lua.lua:1333.
     * trade_difficulty is a plain int (max traced value 7, per
     * RULES_ECONOMY_TRADE_FORGIVING) converted to fixed_t via the same
     * "int * FX_ONE, cast to fixed_t" idiom rules_planets_job_cost uses for
     * its own int `job->cost` (rules_planets.c: "(fixed_t)(job->cost *
     * FX_ONE)") -- 7*FX_ONE (458752) is comfortably within int32 range, no
     * overflow risk from this step.
     *
     * Chained fixed_mul calls, reassigning a running `fixed_t income`
     * variable one factor at a time -- the exact idiom
     * rules_planets_job_cost's own chained-fixed_mul cost computation uses
     * (rules_planets.c: `cost = fixed_mul(cost, MULT1); cost =
     * fixed_mul(cost, MULT2);`), confirmed by reading that function
     * directly. Each fixed_mul's own int64_t-widened intermediate product
     * (fixed.h) makes every individual step overflow-safe; multiplication
     * order (difficulty*sensitivity, then *bonus_sum) doesn't change the
     * mathematical result, only which rounding step happens first -- picked
     * to mirror job_cost's own "base, then successive multiplier" shape. */
    fixed_t bonus_sum = fixed_add(FX_ONE, fixed_add(rules_economy_trade_level_bonus(self_level), rules_economy_trade_level_bonus(partner_level)));

    fixed_t income = (fixed_t)(trade_difficulty * FX_ONE);
    income = fixed_mul(income, sensitivity);
    income = fixed_mul(income, bonus_sum);

    /* Not floored: a single edge's fractional contribution. The per-colony
     * floor is rules_economy_colony_net's job (a later chunk). */
    return income;
}

fixed_t rules_economy_colony_trade_income(int trade_difficulty, fixed_t sensitivity, int self_level,
                                           const int *partner_levels, int partner_count, bool produces_luxury,
                                           fixed_t luxury_production) {
    fixed_t total = 0;

    for (int i = 0; i < partner_count; i++) {
        total = fixed_add(total, rules_economy_trade_edge_income(trade_difficulty, sensitivity, self_level, partner_levels[i]));
    }

    if (produces_luxury) {
        /* pt.kw.mu * dk.trade * 1.5, pico8/lua.lua:1336. Corrected condition
         * (produces_luxury stands in for `pt.kw.qe==r.w`, "this colony's own
         * produced resource is luxury" -- NOT the design doc's own guessed
         * "energy"/"joker", see rules_economy.h's doc comment on this
         * function for the full Annexe A citation trail). Same chained-
         * fixed_mul idiom as rules_economy_trade_edge_income above. */
        fixed_t difficulty_fx = (fixed_t)(trade_difficulty * FX_ONE);
        fixed_t bonus = fixed_mul(luxury_production, difficulty_fx);
        bonus = fixed_mul(bonus, RULES_ECONOMY_LUXURY_TRADE_MULTIPLIER);
        total = fixed_add(total, bonus);
    }

    /* Not floored: matches rules_economy_trade_edge_income's own
     * "not floored here" convention. */
    return total;
}

int rules_economy_overhead(int colonized_count) {
    /* floor(0.18 * colonized_count^2). Computed with plain integer
     * arithmetic (18*n*n/100, using C's truncating integer division, which
     * is exactly floor() for the non-negative operands this formula always
     * has) rather than Q16.16 fixed-point: 0.18 is not exactly representable
     * in binary, so a `(fixed_t)(0.18 * FX_ONE)` literal (this codebase's
     * usual double-literal-folding idiom, e.g. liaison.h's
     * LIAISON_RANGE_SQ/LIAISON_T_MARGIN_LOW) carries its own rounding error
     * before any arithmetic even starts. The plain-integer route has none:
     * 0.18*n^2 is exactly the rational number 18*n^2/100 for any integer n,
     * so integer-dividing 18*n*n by 100 and truncating gives the exact
     * mathematical floor with zero intermediate rounding -- strictly more
     * precise than the fixed-point route for this integer-squared formula,
     * not merely equally correct.
     *
     * Confirmed empirically, not just asserted: a scratch comparison against
     * both a `fixed_mul`-based Q16.16 computation (using the same
     * `(fixed_t)(0.18 * FX_ONE)` literal-folding idiom) and a
     * double-precision `floor(0.18*n*n)` reference, swept over n=0..200,
     * shows the plain-integer result matches the double reference exactly
     * at every n, while the fixed-point route does not: e.g. at n=10 the
     * fixed-point route yields 17 (wrong -- the truncated 0.18 literal
     * rounds down just enough to clip an exact boundary case) against the
     * correct 18 that both the plain-integer route and the double reference
     * agree on, and for n>=182 the fixed-point route overflows fixed_t's
     * 32-bit range entirely (n^2*FX_ONE exceeds INT32_MAX) and returns
     * garbage (even negative) values, while the plain-integer route stays
     * exact throughout. The five values host_tests/rules_economy_test.c
     * checks by hand (n=0,1,5,10,23) were chosen specifically because they
     * include n=10, the smallest n where this divergence is visible --
     * confirming the plain-integer route is not merely simpler but the only
     * one of the two that is actually correct here. */
    return (18 * colonized_count * colonized_count) / 100;
}
