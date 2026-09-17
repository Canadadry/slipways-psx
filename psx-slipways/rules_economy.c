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
