#include "rules_planets.h"

#include <stddef.h>

static const planet_type_t g_planet_types[PLANET_TYPE_COUNT] = {
    [PLANET_TYPE_EARTHLIKE] = {
        .id = PLANET_TYPE_EARTHLIKE,
        .name = "earth-like",
        .rarity = (fixed_t)(4.5 * FX_ONE),
    },
    [PLANET_TYPE_FORGEWORLD] = {
        .id = PLANET_TYPE_FORGEWORLD,
        .name = "forgeworld",
        .rarity = (fixed_t)(6 * FX_ONE),
    },
    [PLANET_TYPE_MINERAL] = {
        .id = PLANET_TYPE_MINERAL,
        .name = "mineral",
        .rarity = (fixed_t)(4 * FX_ONE),
    },
};

static const colonization_job_t g_jobs[JOB_COUNT] = {
    [JOB_MINE] = {
        .planet_type = PLANET_TYPE_MINERAL,
        .cost = 5,
        .needs = { RESOURCE_PEOPLE },
        .needs_count = 1,
        .produces = RESOURCE_ORE,
        .curve = CURVE_MINUS,
        .tech_gate = NULL,
        .default_need = RESOURCE_NONE,
    },
    [JOB_NANOTECH] = {
        .planet_type = PLANET_TYPE_FORGEWORLD,
        .cost = 8,
        .needs = { RESOURCE_ORE },
        .needs_count = 1,
        .produces = RESOURCE_TECH,
        .curve = CURVE_MINUS,
        .tech_gate = NULL,
        .default_need = RESOURCE_NONE,
    },
    [JOB_BOTS] = {
        .planet_type = PLANET_TYPE_FORGEWORLD,
        .cost = 8,
        .needs = { RESOURCE_ORE, RESOURCE_TECH },
        .needs_count = 2,
        .produces = RESOURCE_BOTS,
        .curve = CURVE_PLUS,
        .tech_gate = NULL,
        .default_need = RESOURCE_NONE,
    },
    [JOB_AGROWORLD] = {
        .planet_type = PLANET_TYPE_EARTHLIKE,
        .cost = 5,
        .needs = { RESOURCE_BOTS },
        .needs_count = 1,
        .produces = RESOURCE_FOOD,
        .curve = CURVE_MINUS,
        .tech_gate = "xenofoods",
        .default_need = RESOURCE_ORGANICS,
    },
    [JOB_HIVEWORLD] = {
        .planet_type = PLANET_TYPE_EARTHLIKE,
        .cost = 5,
        .needs = { RESOURCE_FOOD },
        .needs_count = 1,
        .produces = RESOURCE_PEOPLE,
        .curve = CURVE_PLUS,
        .tech_gate = NULL,
        .default_need = RESOURCE_GOODS,
    },
};

const planet_type_t *rules_planets_type(planet_type_id_t id) {
    return &g_planet_types[id];
}

const colonization_job_t *rules_planets_job(colonization_job_id_t id) {
    return &g_jobs[id];
}

/* Per-level production tables traced to hc (pico8/lua.lua, unstash 8383).
 * Index 0..3 represents levels 1..4. */
static const int g_curve_minus[4] = { 1, 2, 3, 3 };
static const int g_curve_plus[4] = { 2, 3, 4, 4 };

int rules_planets_curve_value(curve_id_t curve, int level, bool upgraded) {
    if (level < 1) {
        return 0;
    }
    if (level > 4) {
        level = 4;
    }

    curve_id_t effective_curve = curve;
    if (upgraded && curve == CURVE_MINUS) {
        effective_curve = CURVE_PLUS;
    }

    const int *table = (effective_curve == CURVE_MINUS) ? g_curve_minus : g_curve_plus;
    return table[level - 1];
}

/* hp:od()'s two multipliers, traced to pico8/lua.lua:1426-1433. Q16.16
 * fixed-point constants, never float/double at runtime. */
#define RULES_PLANETS_TECH_GATE_MULTIPLIER ((fixed_t)(1.5 * FX_ONE))
#define RULES_PLANETS_NANOMATERIALS_MULTIPLIER ((fixed_t)(0.85 * FX_ONE))

int rules_planets_job_cost(colonization_job_id_t id, bool job_tech_researched, bool nanomaterials_researched) {
    const colonization_job_t *job = rules_planets_job(id);

    fixed_t cost = (fixed_t)(job->cost * FX_ONE);

    if (job->tech_gate != NULL && job_tech_researched) {
        cost = fixed_mul(cost, RULES_PLANETS_TECH_GATE_MULTIPLIER);
    }
    if (nanomaterials_researched) {
        cost = fixed_mul(cost, RULES_PLANETS_NANOMATERIALS_MULTIPLIER);
    }

    /* Floor to an integer ONCE at the very end -- not after each
     * multiplication step -- matching PICO-8's own internal Q16.16 number
     * representation. */
    return (int)(cost >> 16);
}
