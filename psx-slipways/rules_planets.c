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
    [PLANET_TYPE_OCEAN] = {
        .id = PLANET_TYPE_OCEAN,
        .name = "ocean",
        .rarity = (fixed_t)(3 * FX_ONE),
    },
    [PLANET_TYPE_REMNANT] = {
        .id = PLANET_TYPE_REMNANT,
        .name = "remnant",
        .rarity = (fixed_t)(2.5 * FX_ONE),
    },
    [PLANET_TYPE_XENO] = {
        .id = PLANET_TYPE_XENO,
        .name = "xeno",
        .rarity = (fixed_t)(3 * FX_ONE),
    },
    [PLANET_TYPE_JUNGLE] = {
        .id = PLANET_TYPE_JUNGLE,
        .name = "jungle",
        .rarity = (fixed_t)(2 * FX_ONE),
    },
    [PLANET_TYPE_ICEBALL] = {
        .id = PLANET_TYPE_ICEBALL,
        .name = "iceball",
        .rarity = (fixed_t)(2 * FX_ONE),
    },
    [PLANET_TYPE_BARREN] = {
        .id = PLANET_TYPE_BARREN,
        .name = "barren",
        .rarity = (fixed_t)(2 * FX_ONE),
    },
    [PLANET_TYPE_GASGIANT] = {
        .id = PLANET_TYPE_GASGIANT,
        .name = "gas giant",
        .rarity = (fixed_t)(4 * FX_ONE),
    },
};

static const colonization_job_t g_jobs[JOB_COUNT] = {
    [JOB_MINE] = {
        .planet_types = { PLANET_TYPE_MINERAL },
        .planet_types_count = 1,
        .cost = 5,
        .needs = { RESOURCE_PEOPLE },
        .needs_count = 1,
        .produces = RESOURCE_ORE,
        .curve = CURVE_MINUS,
        .tech_gate = NULL,
        .default_need = RESOURCE_NONE,
        .required_tech = NULL,
    },
    [JOB_NANOTECH] = {
        .planet_types = { PLANET_TYPE_FORGEWORLD },
        .planet_types_count = 1,
        .cost = 8,
        .needs = { RESOURCE_ORE },
        .needs_count = 1,
        .produces = RESOURCE_TECH,
        .curve = CURVE_MINUS,
        .tech_gate = NULL,
        .default_need = RESOURCE_NONE,
        .required_tech = NULL,
    },
    [JOB_BOTS] = {
        .planet_types = { PLANET_TYPE_FORGEWORLD },
        .planet_types_count = 1,
        .cost = 8,
        .needs = { RESOURCE_ORE, RESOURCE_TECH },
        .needs_count = 2,
        .produces = RESOURCE_BOTS,
        .curve = CURVE_PLUS,
        .tech_gate = NULL,
        .default_need = RESOURCE_NONE,
        .required_tech = NULL,
    },
    [JOB_AGROWORLD] = {
        .planet_types = { PLANET_TYPE_EARTHLIKE },
        .planet_types_count = 1,
        .cost = 5,
        .needs = { RESOURCE_BOTS },
        .needs_count = 1,
        .produces = RESOURCE_FOOD,
        .curve = CURVE_MINUS,
        .tech_gate = "xenofoods",
        .default_need = RESOURCE_ORGANICS,
        .required_tech = NULL,
    },
    [JOB_HIVEWORLD] = {
        .planet_types = { PLANET_TYPE_EARTHLIKE },
        .planet_types_count = 1,
        .cost = 5,
        .needs = { RESOURCE_FOOD },
        .needs_count = 1,
        .produces = RESOURCE_PEOPLE,
        .curve = CURVE_PLUS,
        .tech_gate = NULL,
        .default_need = RESOURCE_GOODS,
        .required_tech = NULL,
    },
    /* Chunk 4 (docs/prd/triage/psx-full-planets-and-tech.md): forgeworld
     * batch, traced to fd's full 24-job re-decode (Implementation
     * Decisions table). All 3 are single-type (forgeworld only). */
    [JOB_GOODS] = {
        .planet_types = { PLANET_TYPE_FORGEWORLD },
        .planet_types_count = 1,
        .cost = 10,
        .needs = { RESOURCE_ORE, RESOURCE_BOTS },
        .needs_count = 2,
        .produces = RESOURCE_GOODS,
        .curve = CURVE_PLUS,
        .tech_gate = NULL,
        .default_need = RESOURCE_NONE,
        .required_tech = NULL,
    },
    [JOB_GADGETS] = {
        .planet_types = { PLANET_TYPE_FORGEWORLD },
        .planet_types_count = 1,
        .cost = 10,
        .needs = { RESOURCE_ORE, RESOURCE_TECH },
        .needs_count = 2,
        .produces = RESOURCE_GOODS,
        .curve = CURVE_PLUS,
        .tech_gate = NULL,
        .default_need = RESOURCE_NONE,
        .required_tech = NULL,
    },
    /* required_tech = "gene rewriting": the raw fd array's rr
     * (availability-gate) field, captured as data only -- no live
     * colonization-choice menu exists anywhere in this port to apply it to
     * (see colonization_job_t's own doc comment and this PRD's Out of
     * Scope). Not to be confused with tech_gate (il), which this job does
     * not have. */
    [JOB_UPLIFT] = {
        .planet_types = { PLANET_TYPE_FORGEWORLD },
        .planet_types_count = 1,
        .cost = 20,
        .needs = { RESOURCE_ORGANICS },
        .needs_count = 1,
        .produces = RESOURCE_PEOPLE,
        .curve = CURVE_MINUS,
        .tech_gate = NULL,
        .default_need = RESOURCE_NONE,
        .required_tech = "gene rewriting",
    },
    /* Chunk 5 (docs/prd/triage/psx-full-planets-and-tech.md): ocean batch,
     * traced to fd's full 24-job re-decode (Implementation Decisions table).
     * The first chunk combining a new planet type (PLANET_TYPE_OCEAN, added
     * by chunk 2 but unused by any job until now) with new jobs on it.
     * colony(ocean) and algae both carry a real tech_gate (il) AND a real
     * default_need (dm) -- exercised via rules_planets_job_cost/_production
     * exactly like agroworld's own existing il/dm pair, proving those
     * already-landed, unmodified functions genuinely generalize to new
     * tech-gated data. */
    [JOB_COLONY_OCEAN] = {
        .planet_types = { PLANET_TYPE_OCEAN },
        .planet_types_count = 1,
        .cost = 10,
        .needs = { RESOURCE_ORE, RESOURCE_FOOD },
        .needs_count = 2,
        .produces = RESOURCE_PEOPLE,
        .curve = CURVE_MINUS,
        .tech_gate = "gene rewriting",
        .default_need = RESOURCE_GOODS,
        .required_tech = NULL,
    },
    [JOB_ALGAE] = {
        .planet_types = { PLANET_TYPE_OCEAN },
        .planet_types_count = 1,
        .cost = 8,
        .needs = { RESOURCE_BOTS },
        .needs_count = 1,
        .produces = RESOURCE_FOOD,
        .curve = CURVE_MINUS,
        .tech_gate = "xenofoods",
        .default_need = RESOURCE_ORGANICS,
        .required_tech = NULL,
    },
    [JOB_BREED_OCEAN] = {
        .planet_types = { PLANET_TYPE_OCEAN },
        .planet_types_count = 1,
        .cost = 5,
        .needs = { RESOURCE_PEOPLE },
        .needs_count = 1,
        .produces = RESOURCE_ORGANICS,
        .curve = CURVE_PLUS,
        .tech_gate = NULL,
        .default_need = RESOURCE_NONE,
        .required_tech = NULL,
    },
    /* Chunk 6 (docs/prd/triage/psx-full-planets-and-tech.md): remnant batch,
     * traced to fd's full 24-job re-decode (Implementation Decisions table).
     * All 3 are single-type (remnant only), with no tech_gate, no
     * default_need, and no required_tech -- deliberately the simplest
     * "add a new job" case in the full roster. */
    [JOB_STRIP] = {
        .planet_types = { PLANET_TYPE_REMNANT },
        .planet_types_count = 1,
        .cost = 5,
        .needs = { RESOURCE_PEOPLE, RESOURCE_FOOD },
        .needs_count = 2,
        .produces = RESOURCE_ORE,
        .curve = CURVE_MINUS,
        .tech_gate = NULL,
        .default_need = RESOURCE_NONE,
        .required_tech = NULL,
    },
    [JOB_DISMANTLE] = {
        .planet_types = { PLANET_TYPE_REMNANT },
        .planet_types_count = 1,
        .cost = 5,
        .needs = { RESOURCE_PEOPLE, RESOURCE_FOOD },
        .needs_count = 2,
        .produces = RESOURCE_TECH,
        .curve = CURVE_MINUS,
        .tech_gate = NULL,
        .default_need = RESOURCE_NONE,
        .required_tech = NULL,
    },
    [JOB_SCAVENGE] = {
        .planet_types = { PLANET_TYPE_REMNANT },
        .planet_types_count = 1,
        .cost = 5,
        .needs = { RESOURCE_PEOPLE, RESOURCE_FOOD },
        .needs_count = 2,
        .produces = RESOURCE_BOTS,
        .curve = CURVE_MINUS,
        .tech_gate = NULL,
        .default_need = RESOURCE_NONE,
        .required_tech = NULL,
    },
    /* Chunk 7 (docs/prd/triage/psx-full-planets-and-tech.md): xeno + jungle
     * batch, traced to fd's full 24-job re-decode (Implementation Decisions
     * table). colony(xeno) is tech-gated exactly like colony(ocean) from
     * chunk 5 -- same tech_gate ("gene rewriting") and default_need
     * (RESOURCE_GOODS); only needs/planet_types differ. */
    [JOB_COLONY_XENO] = {
        .planet_types = { PLANET_TYPE_XENO },
        .planet_types_count = 1,
        .cost = 10,
        .needs = { RESOURCE_TECH, RESOURCE_FOOD },
        .needs_count = 2,
        .produces = RESOURCE_PEOPLE,
        .curve = CURVE_MINUS,
        .tech_gate = "gene rewriting",
        .default_need = RESOURCE_GOODS,
        .required_tech = NULL,
    },
    [JOB_BREED_XENO] = {
        .planet_types = { PLANET_TYPE_XENO },
        .planet_types_count = 1,
        .cost = 5,
        .needs = { RESOURCE_BOTS },
        .needs_count = 1,
        .produces = RESOURCE_ORGANICS,
        .curve = CURVE_MINUS,
        .tech_gate = NULL,
        .default_need = RESOURCE_NONE,
        .required_tech = NULL,
    },
    [JOB_HUNT] = {
        .planet_types = { PLANET_TYPE_JUNGLE },
        .planet_types_count = 1,
        .cost = 5,
        .needs = { RESOURCE_PEOPLE },
        .needs_count = 1,
        .produces = RESOURCE_ORGANICS,
        .curve = CURVE_MINUS,
        .tech_gate = NULL,
        .default_need = RESOURCE_NONE,
        .required_tech = NULL,
    },
    /* tourism: the first real job to use CURVE_F_MINUS (added in chunk 1).
     * No tech_gate, so job_tech_researched never changes its cost or curve
     * -- and CURVE_F_MINUS itself has no upgrade table (see curve_id_t's own
     * doc comment), so even a hypothetical tech_gate wouldn't change its
     * production values. */
    [JOB_TOURISM] = {
        .planet_types = { PLANET_TYPE_JUNGLE },
        .planet_types_count = 1,
        .cost = 10,
        .needs = { RESOURCE_PEOPLE, RESOURCE_GOODS },
        .needs_count = 2,
        .produces = RESOURCE_LUXURY,
        .curve = CURVE_F_MINUS,
        .tech_gate = NULL,
        .default_need = RESOURCE_NONE,
        .required_tech = NULL,
    },
};

const planet_type_t *rules_planets_type(planet_type_id_t id) {
    return &g_planet_types[id];
}

const colonization_job_t *rules_planets_job(colonization_job_id_t id) {
    return &g_jobs[id];
}

bool rules_planets_job_available_on(colonization_job_id_t id, planet_type_id_t type) {
    const colonization_job_t *job = rules_planets_job(id);

    for (int i = 0; i < job->planet_types_count; i++) {
        if (job->planet_types[i] == type) {
            return true;
        }
    }
    return false;
}

/* Per-level production tables traced to hc (pico8/lua.lua, unstash 8383).
 * Index 0..3 represents levels 1..4. */
static const int g_curve_minus[4] = { 1, 2, 3, 3 };
static const int g_curve_plus[4] = { 2, 3, 4, 4 };
/* f- = [0,1,2,3] for levels 1-4, traced to hc (unstash Annexe A). No
 * upgrade target is modeled for this curve (see curve_id_t's own doc
 * comment) -- CURVE_F_MINUS is deliberately absent from the "upgraded"
 * mapping below, so upgraded=true is a documented no-op for it. */
static const int g_curve_f_minus[4] = { 0, 1, 2, 3 };

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

    if (effective_curve == CURVE_F_MINUS) {
        return g_curve_f_minus[level - 1];
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

int rules_planets_job_production(colonization_job_id_t id, int level, bool job_tech_researched) {
    const colonization_job_t *job = rules_planets_job(id);

    /* Same "has a tech_gate and it's researched" condition
     * rules_planets_job_cost uses for its surcharge -- source gates both
     * hp:od()'s cost surcharge and hp:po()'s curve upgrade off the exact
     * same self.il and oj(self.il) check. Written verbatim here rather than
     * factored into a shared helper with rules_planets_job_cost, since this
     * chunk must not modify that already-landed function. */
    bool upgraded = job->tech_gate != NULL && job_tech_researched;

    return rules_planets_curve_value(job->curve, level, upgraded);
}
