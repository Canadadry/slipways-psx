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

/* Floor a fixed_t value to an int, correct for BOTH positive and negative
 * inputs -- unlike rules_planets_job_cost's own `(int)(cost >> 16)` idiom
 * (rules_planets.c:126), which is only ever exercised there with
 * non-negative job costs. This module's own db()/da() trace explicitly
 * allows negative results: trade income minus maintenance
 * (rules_economy_colony_net) and the empire's annual net income
 * (rules_economy_annual_net_income) can both go negative -- a colony can
 * cost more in maintenance than it earns in trade, and the whole empire can
 * run a deficit, which is exactly what RULES_ECONOMY_BANKRUPTCY_TREASURY_
 * THRESHOLD exists to detect in a later chunk -- so a floor idiom that is
 * only correct for x>=0 would silently corrupt this module's two most
 * important numbers.
 *
 * A plain arithmetic right shift (`x >> 16`) DOES floor correctly for a
 * negative two's-complement fixed_t on every compiler this project actually
 * uses (host gcc, and the PSn00bSDK/mipsel cross compiler -- both GCC, and
 * GCC documents right-shifting a negative signed integer as a
 * sign-propagating arithmetic shift). But the C standard itself (C11
 * 6.5.7p5) leaves the result of right-shifting a negative signed value
 * *implementation-defined*, not guaranteed -- so rather than rely on a
 * documented-but-not-ISO-guaranteed GCC extension for this module's two
 * safety-critical floors, this helper is instead built from plain integer
 * division and modulo, both of which the C standard DOES guarantee
 * truncate toward zero for any sign (C11 6.5.5p6) -- portably correct on
 * any standard-conforming compiler, not just this project's current two.
 *
 * Truncating division already equals floor() for x>=0 and for any exact
 * multiple of FX_ONE (remainder 0, either sign). The only case needing a
 * correction is a negative x with a nonzero fractional part, where
 * truncation-toward-zero stops one step short of floor (e.g. -0.3
 * truncates to 0, but flr(-0.3) is -1) -- corrected by subtracting 1 in
 * exactly that case.
 */
static int fixed_floor_to_int(fixed_t x) {
    int whole = x / FX_ONE;
    int remainder = x % FX_ONE;

    if (remainder != 0 && x < 0) {
        whole -= 1;
    }

    return whole;
}

int rules_economy_colony_net(fixed_t trade_income, int maintenance) {
    /* flr(db(p) - da(p)), pico8/lua.lua:1230-1231 -- the FIRST of this
     * module's two floors. maintenance (already an int) is widened to
     * fixed_t via the same "int * FX_ONE, cast to fixed_t" idiom
     * rules_economy_trade_edge_income already uses for its own int
     * trade_difficulty parameter, before subtracting. */
    fixed_t maintenance_fx = (fixed_t)(maintenance * FX_ONE);
    fixed_t net = fixed_sub(trade_income, maintenance_fx);

    return fixed_floor_to_int(net);
}

/* ov("skill implants",1,1.15)'s bonus factor, pico8/lua.lua:1233. 1.15 is
 * not exactly representable in Q16.16, folded from the traced double
 * literal via the same idiom g_trade_level_bonus[] (above) already uses.
 * Kept file-local: no other consumer, per this file's own established
 * "keep unexposed constants static/#define, not in the header" precedent
 * (see g_colony_maintenance_cost/RULES_ECONOMY_LUXURY_TRADE_MULTIPLIER
 * above). */
#define RULES_ECONOMY_SKILL_IMPLANTS_MULTIPLIER ((fixed_t)(1.15 * (double)FX_ONE))

int rules_economy_annual_net_income(const int *colony_nets, int colony_count, bool skill_implants_researched, int overhead) {
    /* dd = sum of already-floored p.kr values, pico8/lua.lua:1227-1231. */
    int sum = 0;
    for (int i = 0; i < colony_count; i++) {
        sum += colony_nets[i];
    }

    /* ov("skill implants",1,1.15): base 1.0 unless researched, matching
     * rules_planets_job_cost's own caller-supplied-boolean pattern for
     * "no live tech-researched state exists" (rules_planets.c). Widen the
     * already-integer sum to fixed_t (same idiom as rules_economy_colony_net
     * above), multiply by the factor, THEN floor -- the SECOND floor,
     * applied to the scaled sum, not to the un-scaled sum. */
    fixed_t multiplier = skill_implants_researched ? RULES_ECONOMY_SKILL_IMPLANTS_MULTIPLIER : FX_ONE;
    fixed_t scaled = fixed_mul((fixed_t)(sum * FX_ONE), multiplier);
    int floored = fixed_floor_to_int(scaled);

    /* - cz(): overhead subtracts AFTER the second floor, exactly matching
     * source's `flr(dd*ov(...)) - cz()` placement (the subtraction is
     * outside the flr(...) call). */
    return floored - overhead;
}

/* co, pico8/lua.lua:1364: {-5,0,1,1} for levels 1-4. All-integer (like
 * g_colony_maintenance_cost above, unlike g_trade_level_bonus), so each
 * entry is folded to Q16.16 via a plain "int * FX_ONE" literal -- no
 * fractional double-literal folding concern, matching the PRD's own note
 * that these are "exact integers." Index 0..3 represents levels 1..4,
 * matching every other per-level table's indexing convention in this file. */
static const fixed_t g_happiness_level_adjustment[4] = {
    -5 * FX_ONE,
    0 * FX_ONE,
    1 * FX_ONE,
    1 * FX_ONE,
};

fixed_t rules_economy_happiness(fixed_t joy_production, fixed_t luxury_production, const int *colony_levels, int colony_count) {
    /* 100 + 2*(fm.po[r.h]+fm.po[r.w]), pico8/lua.lua:1367. */
    fixed_t h = fixed_add((fixed_t)(100 * FX_ONE), fixed_mul((fixed_t)(2 * FX_ONE), fixed_add(joy_production, luxury_production)));

    /* for pt in all(cg.jo) do if pt:gg() then h += co[pt.mi] end end,
     * pico8/lua.lua:1368-1370 -- every colony_levels entry stands in for a
     * colony already known to satisfy pt:gg(), per this module's established
     * "caller already filtered" convention (see rules_economy.h's doc
     * comment on this function). */
    for (int i = 0; i < colony_count; i++) {
        int level = colony_levels[i];

        if (level < 1) {
            level = 1;
        }
        if (level > 4) {
            level = 4;
        }

        h = fixed_add(h, g_happiness_level_adjustment[level - 1]);
    }

    /* Not floored, not clamped: see rules_economy.h's doc comment on this
     * function for why -- source never does either to `h`. */
    return h;
}

/* b_, pico8/lua.lua:1339: {0,100,200,400} for levels 1-4. All-integer, kept
 * as a plain int table (not fixed_t) since colonies_score/population_score/
 * tech_score/total are all plain ints in source too (di/gl/el/total are never
 * fixed-point in ky() -- only the final `*0.01*fm.happiness` step needs real
 * fixed-point arithmetic, handled directly in rules_economy_compute_score
 * below). */
static const int g_colonies_score[4] = {0, 100, 200, 400};

/* (#fm.invented*1.2)^2's "1.2" multiplier, pico8/lua.lua:1361. Not exactly
 * representable in Q16.16, folded from the traced double literal via the
 * same idiom RULES_ECONOMY_LUXURY_TRADE_MULTIPLIER/RULES_ECONOMY_SKILL_
 * IMPLANTS_MULTIPLIER already use above. Kept file-local: no other consumer,
 * per this file's own established "keep unexposed constants static/#define,
 * not in the header" precedent. */
#define RULES_ECONOMY_SCORE_TECH_MULTIPLIER ((fixed_t)(1.2 * (double)FX_ONE))

rules_economy_score_t rules_economy_compute_score(const int *colony_levels, int colony_count, int population,
                                                    int tech_invented_count, fixed_t happiness_percent) {
    rules_economy_score_t score;

    /* di: sum of b_[level] over colony_levels, pico8/lua.lua:1341-1345 --
     * every entry stands in for a colony already known to satisfy pt:gg(),
     * same "caller already filtered" convention as rules_economy_happiness
     * above. level clamped to [1,4], mirroring this file's established
     * clamping idiom exactly. */
    int colonies_score = 0;
    for (int i = 0; i < colony_count; i++) {
        int level = colony_levels[i];

        if (level < 1) {
            level = 1;
        }
        if (level > 4) {
            level = 4;
        }

        colonies_score += g_colonies_score[level - 1];
    }

    /* gl = fm.po[r.p]*40, pico8/lua.lua:1360. Plain int arithmetic: both
     * operands are exact integers, no fixed_t needed. */
    int population_score = population * 40;

    /* el = flr((#fm.invented*1.2)^2)*10, pico8/lua.lua:1361. tech_x is
     * (tech_invented_count*1.2) in Q16.16; tech_x_squared is tech_x^2 in
     * Q16.16, floored to an int via this file's own fixed_floor_to_int
     * (defined above, chunk 4) -- reused, not duplicated -- then multiplied
     * by 10. tech_invented_count is widened to fixed_t via the same
     * "int * FX_ONE, cast to fixed_t" idiom this file already uses
     * repeatedly (e.g. rules_economy_trade_edge_income's trade_difficulty). */
    fixed_t tech_x = fixed_mul((fixed_t)(tech_invented_count * FX_ONE), RULES_ECONOMY_SCORE_TECH_MULTIPLIER);
    fixed_t tech_x_squared = fixed_mul(tech_x, tech_x);
    int tech_score = fixed_floor_to_int(tech_x_squared) * 10;

    /* total = flr((di+gl+el)*0.01*fm.happiness), pico8/lua.lua:1362.
     *
     * Computed with int64_t intermediates throughout, NOT via a fixed_t
     * widening of the raw integer sum, for two independent reasons:
     * (1) overflow -- population_score (population*40) is unbounded by
     * anything this module tracks, so a large enough population pushes
     * sum*FX_ONE past fixed_t's int32 range even though this module's own
     * tests never exercise a sum that large; (2) precision -- even for
     * sums well inside fixed_t's range, multiplying by a Q16.16-truncated
     * "0.01" literal ((fixed_t)(0.01*FX_ONE) is not exact, since 0.01 has
     * no finite binary representation) measurably corrupts the result --
     * verified concretely: for this module's own stars-clamping test
     * (sum=16000, happiness=100%), a naive two-step fixed_mul chain through
     * a truncated 0.01 constant yields total=15991, not the exact 16000,
     * an error large enough to have failed that test's own exact-match
     * assertion even though no step in that particular chain overflows.
     * Instead: numerator = sum * happiness_percent (both widened to
     * int64_t before multiplying, so THIS product cannot overflow for any
     * value either operand could plausibly hold); denominator = 100 *
     * FX_ONE (the fixed-point representation of the literal "100" in
     * "*0.01*..."). numerator/denominator is mathematically exactly
     * sum*happiness_percent_real/100, i.e. sum*0.01*happiness_percent_real
     * -- the same value source computes, without ever materializing a
     * truncated "0.01" constant or an out-of-range fixed_t along the way.
     *
     * Floored toward negative infinity, not truncated toward zero:
     * happiness_percent (rules_economy_happiness's own return value) is
     * uncapped and can be negative, which makes numerator (and so `total`)
     * negative too for a nonzero sum -- the same "truncation != floor for
     * negative values" correction fixed_floor_to_int already establishes,
     * applied here directly to the int64_t quotient/remainder since this
     * value never takes the form of an in-range fixed_t that
     * fixed_floor_to_int itself could be called on. */
    int sum = colonies_score + population_score + tech_score;
    int64_t numerator = (int64_t)sum * (int64_t)happiness_percent;
    int64_t denominator = (int64_t)100 * (int64_t)FX_ONE;
    int64_t total64 = numerator / denominator;
    if (numerator % denominator != 0 && numerator < 0) {
        total64 -= 1;
    }
    int total = (int)total64;

    /* stars = min(flr(total/2000), 5), pico8/lua.lua:1363. `total` is
     * already a plain, already-floored int here -- this is an int/int floor
     * division, not a fixed_t operation -- but still not C's truncating `/`,
     * for the same reason `total` above needed care: `total` can be
     * negative, and truncating division rounds toward zero, not toward
     * negative infinity. No lower clamp: source has none, only the upper
     * min(...,5) bound is ported. */
    int stars = total / 2000;
    if (total % 2000 != 0 && total < 0) {
        stars -= 1;
    }
    if (stars > 5) {
        stars = 5;
    }

    score.colonies_score = colonies_score;
    score.population_score = population_score;
    score.tech_score = tech_score;
    score.total = total;
    score.stars = stars;

    return score;
}

rules_economy_game_over_t rules_economy_check_game_over(int year, int net_income, int treasury) {
    /* self.yr>=3426 and "final score" or ..., pico8/lua.lua:1235. This `if`
     * returning unconditionally, before the bankruptcy check below is even
     * reached, is what reproduces Lua's `or` short-circuit exactly: once
     * this is true, the bankruptcy clause is structurally never evaluated --
     * see rules_economy.h's doc comment on this function for the full
     * short-circuit trace. */
    if (year >= RULES_ECONOMY_FINAL_YEAR) {
        return RULES_ECONOMY_GAME_OVER_FINAL_YEAR;
    }

    /* self.ge<=0 and self.nz<3 and "bankrupt", pico8/lua.lua:1236. Only
     * reached when the final-year check above was false. Both clauses must
     * hold (source's own `and`): net_income<=0 (0 counts) AND
     * treasury<RULES_ECONOMY_BANKRUPTCY_TREASURY_THRESHOLD (strictly below,
     * exactly at the threshold does NOT count). */
    if (net_income <= 0 && treasury < RULES_ECONOMY_BANKRUPTCY_TREASURY_THRESHOLD) {
        return RULES_ECONOMY_GAME_OVER_BANKRUPT;
    }

    return RULES_ECONOMY_GAME_CONTINUES;
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
