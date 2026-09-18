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
    /* Chunk 8 (docs/prd/triage/psx-full-planets-and-tech.md): "Earthlike
     * remaining", luxury -- the second real job to use CURVE_F_MINUS (after
     * tourism above), deliberately chosen to prove the curve lookup is
     * genuinely job-agnostic across independent jobs, not just correct for
     * one specific job. Same no-gate shape as tourism: no tech_gate, no
     * default_need, no required_tech. */
    [JOB_LUXURY] = {
        .planet_types = { PLANET_TYPE_EARTHLIKE },
        .planet_types_count = 1,
        .cost = 10,
        .needs = { RESOURCE_TECH, RESOURCE_GOODS },
        .needs_count = 2,
        .produces = RESOURCE_LUXURY,
        .curve = CURVE_F_MINUS,
        .tech_gate = NULL,
        .default_need = RESOURCE_NONE,
        .required_tech = NULL,
    },
    /* Chunk 9 (docs/prd/triage/psx-full-planets-and-tech.md): barren/
     * iceball/gas-giant batch, traced to fd's full 24-job re-decode
     * (Implementation Decisions table). habitat and genefarm are both valid
     * on barren+iceball (planet_types_count == 2) -- the first real
     * exercise of RULES_PLANETS_MAX_TYPES == 2 in this table. Neither has a
     * tech_gate; both share required_tech = "geneseeds". */
    [JOB_HABITAT] = {
        .planet_types = { PLANET_TYPE_BARREN, PLANET_TYPE_ICEBALL },
        .planet_types_count = 2,
        .cost = 12,
        .needs = { RESOURCE_ORGANICS, RESOURCE_FOOD },
        .needs_count = 2,
        .produces = RESOURCE_PEOPLE,
        .curve = CURVE_MINUS,
        .tech_gate = NULL,
        .default_need = RESOURCE_GOODS,
        .required_tech = "geneseeds",
    },
    [JOB_GENEFARM] = {
        .planet_types = { PLANET_TYPE_BARREN, PLANET_TYPE_ICEBALL },
        .planet_types_count = 2,
        .cost = 12,
        .needs = { RESOURCE_ORGANICS, RESOURCE_BOTS },
        .needs_count = 2,
        .produces = RESOURCE_FOOD,
        .curve = CURVE_MINUS,
        .tech_gate = NULL,
        .default_need = RESOURCE_NONE,
        .required_tech = "geneseeds",
    },
    /* excavate_mineral and excavate_iceball_gasgiant deliberately share
     * identical cost/needs/produces (12/{bots}/ore) but differ in both
     * planet_types (mineral-only vs. iceball+gasgiant) and curve (CURVE_PLUS
     * vs. CURVE_MINUS) -- two genuinely distinct colonization_job_id_t
     * entries, not a copy-paste-confused merge (see this PRD's own explicit
     * risk flag, re-verified by rules_planets_test.c's dedicated
     * distinguishing test). */
    [JOB_EXCAVATE_MINERAL] = {
        .planet_types = { PLANET_TYPE_MINERAL },
        .planet_types_count = 1,
        .cost = 12,
        .needs = { RESOURCE_BOTS },
        .needs_count = 1,
        .produces = RESOURCE_ORE,
        .curve = CURVE_PLUS,
        .tech_gate = NULL,
        .default_need = RESOURCE_NONE,
        .required_tech = "drillbots",
    },
    [JOB_EXCAVATE_ICEBALL_GASGIANT] = {
        .planet_types = { PLANET_TYPE_ICEBALL, PLANET_TYPE_GASGIANT },
        .planet_types_count = 2,
        .cost = 12,
        .needs = { RESOURCE_BOTS },
        .needs_count = 1,
        .produces = RESOURCE_ORE,
        .curve = CURVE_MINUS,
        .tech_gate = NULL,
        .default_need = RESOURCE_NONE,
        .required_tech = "drillbots",
    },
    /* farm is valid on xeno+jungle, both already added by chunk 7 -- the
     * third of this chunk's 4 genuinely multi-type jobs. */
    [JOB_FARM] = {
        .planet_types = { PLANET_TYPE_XENO, PLANET_TYPE_JUNGLE },
        .planet_types_count = 2,
        .cost = 10,
        .needs = { RESOURCE_BOTS },
        .needs_count = 1,
        .produces = RESOURCE_FOOD,
        .curve = CURVE_MINUS,
        .tech_gate = NULL,
        .default_need = RESOURCE_ORGANICS,
        .required_tech = "xenofoods",
    },
};

/* Chunk 2 (docs/prd/triage/psx-structures-and-transforms.md): structure_t's
 * struct shape + the first 2 of 8 structures (Laboratory, Food processor),
 * chosen because neither has a required_tech purchase-gate -- proving the
 * struct shape and the flat-cost model (cost read directly, no formula
 * function exists to call, unlike colonization_job_t's cost) before any
 * tech-gated structure. The other 6 slots stay zero-initialized
 * (uninitialized-by-omission via C's designated-initializer semantics) until
 * chunks 3-5 populate them -- STRUCTURE_COUNT is already 8 (the full enum),
 * but only these 2 array entries have real data yet. */
static const structure_t g_structures[STRUCTURE_COUNT] = {
    /* od=15, pico8/lua.lua:910; no rr field on Laboratory's own definition;
     * needs = people, joker; produces = science; curve = qh, upgraded via
     * il="superhuman ai" (pico8/lua.lua:907-914) to qh+ (chunk 1's remap,
     * exercised for real here for the first time); resolvable =
     * ba=o(28,18,20,21,22,48) (pico8/lua.lua:914) = food, bots, organics,
     * ore, tech, goods (6 resources). */
    [STRUCTURE_LABORATORY] = {
        .name = "Laboratory",
        .purchasable = true,
        .cost = 15,
        .required_tech = NULL,
        .needs = { RESOURCE_PEOPLE, RESOURCE_JOKER },
        .needs_count = 2,
        .produces = RESOURCE_SCIENCE,
        .curve = CURVE_QH,
        .tech_gate = "superhuman ai",
        .default_need = RESOURCE_NONE,
        .resolvable = { RESOURCE_FOOD, RESOURCE_BOTS, RESOURCE_ORGANICS, RESOURCE_ORE, RESOURCE_TECH, RESOURCE_GOODS },
        .resolvable_count = 6,
        .max_connections = 0,
        .crosses_other_routes = false,
    },
    /* od=15, pico8/lua.lua:1099; no rr field on Food processor's own
     * definition; needs = organics; produces = food; curve = f-, upgraded
     * via il="xenofoods" (pico8/lua.lua:1099) to f+ -- the first point in the
     * port where CURVE_F_MINUS's f-->f+ upgrade path becomes genuinely
     * reachable (see the new remap branch in rules_planets_curve_value
     * below). No joker need/produce, unlike Laboratory/Synthesizer/Trading
     * Hub -- resolvable_count = 0. */
    [STRUCTURE_FOOD_PROCESSOR] = {
        .name = "Food processor",
        .purchasable = true,
        .cost = 15,
        .required_tech = NULL,
        .needs = { RESOURCE_ORGANICS },
        .needs_count = 1,
        .produces = RESOURCE_FOOD,
        .curve = CURVE_F_MINUS,
        .tech_gate = "xenofoods",
        .default_need = RESOURCE_NONE,
        .resolvable_count = 0,
        .max_connections = 0,
        .crosses_other_routes = false,
    },
    /* Chunk 3 (docs/prd/triage/psx-structures-and-transforms.md): od=60,
     * pico8/lua.lua:1003; rr="ascension" (pico8/lua.lua:1000) -- a purchase
     * gate, captured as data only (see structure_t's own doc comment); needs
     * = people; produces = joy; curve = f+, no il (curve-upgrade gate) -- f+
     * has no mc upgrade target of its own (hc, Annexe A), so tech_gate ==
     * NULL is the traced fact, not a simplification. No joker need/produce
     * -- resolvable_count = 0. */
    [STRUCTURE_ASCENSION_GATE] = {
        .name = "Ascension Gate",
        .purchasable = true,
        .cost = 60,
        .required_tech = "ascension",
        .needs = { RESOURCE_PEOPLE },
        .needs_count = 1,
        .produces = RESOURCE_JOY,
        .curve = CURVE_F_PLUS,
        .tech_gate = NULL,
        .default_need = RESOURCE_NONE,
        .resolvable_count = 0,
        .max_connections = 0,
        .crosses_other_routes = false,
    },
    /* Chunk 3: Protostar (`pm`), non-purchasable -- created only by the
     * collapse transform action (chunk 7's job), never bought directly. No
     * `od` field exists on its own source definition at all, so `cost` here
     * is a placeholder literal with no traced meaning (see structure_t's own
     * doc comment: "meaningless when purchasable == false") -- never
     * asserted to any particular value by any test. needs = none; produces =
     * energy; curve = `-` (CURVE_MINUS) -- the first non-colonization-job
     * caller of that curve. */
    [STRUCTURE_PROTOSTAR] = {
        .name = "Protostar",
        .purchasable = false,
        .cost = 0,
        .required_tech = NULL,
        .needs_count = 0,
        .produces = RESOURCE_ENERGY,
        .curve = CURVE_MINUS,
        .tech_gate = NULL,
        .default_need = RESOURCE_NONE,
        .resolvable_count = 0,
        .max_connections = 0,
        .crosses_other_routes = false,
    },
};

const planet_type_t *rules_planets_type(planet_type_id_t id) {
    return &g_planet_types[id];
}

const structure_t *rules_planets_structure(structure_id_t id) {
    return &g_structures[id];
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
/* f- = [0,1,2,3] for levels 1-4, traced to hc (unstash Annexe A). Its mc
 * upgrade target (CURVE_F_PLUS) is wired into the remap below by chunk 2
 * (docs/prd/triage/psx-structures-and-transforms.md, Food processor's real
 * il="xenofoods" field -- see curve_id_t's own doc comment). */
static const int g_curve_f_minus[4] = { 0, 1, 2, 3 };
/* Chunk 1 (docs/prd/triage/psx-structures-and-transforms.md): the 3 new
 * per-level tables this step's structures need, traced to hc (unstash
 * Annexe A). No upgrade target is modeled for CURVE_F_PLUS/CURVE_QH_PLUS
 * (see curve_id_t's own doc comment); CURVE_QH's real mc upgrade target
 * (CURVE_QH_PLUS) is handled in the remap below. */
static const int g_curve_f_plus[4] = { 0, 2, 3, 4 };
static const int g_curve_qh[4] = { 0, 1, 3, 5 };
static const int g_curve_qh_plus[4] = { 0, 2, 5, 8 };

int rules_planets_curve_value(curve_id_t curve, int level, bool upgraded) {
    /* CURVE_NONE has no per-level table at all -- a real sentinel (mirroring
     * RESOURCE_NONE's own precedent), returning 0 unconditionally for any
     * level (including out-of-range) and any upgraded value. Checked before
     * the level clamp below since this case is independent of level. */
    if (curve == CURVE_NONE) {
        return 0;
    }
    if (level < 1) {
        return 0;
    }
    if (level > 4) {
        level = 4;
    }

    curve_id_t effective_curve = curve;
    if (upgraded && curve == CURVE_MINUS) {
        effective_curve = CURVE_PLUS;
    } else if (upgraded && curve == CURVE_QH) {
        effective_curve = CURVE_QH_PLUS;
    } else if (upgraded && curve == CURVE_F_MINUS) {
        /* Chunk 2 (docs/prd/triage/psx-structures-and-transforms.md): the
         * f-->f+ upgrade remap, new in this chunk. Food processor
         * (il="xenofoods") is the first structure with a real il field on a
         * CURVE_F_MINUS structure -- tourism/luxury (the predecessor PRD's
         * only other CURVE_F_MINUS users) both have tech_gate == NULL, so
         * rules_planets_job_production's upgraded is always false for them
         * regardless of this branch existing; their production values are
         * unaffected. */
        effective_curve = CURVE_F_PLUS;
    }

    switch (effective_curve) {
        case CURVE_F_MINUS:
            return g_curve_f_minus[level - 1];
        case CURVE_F_PLUS:
            return g_curve_f_plus[level - 1];
        case CURVE_QH:
            return g_curve_qh[level - 1];
        case CURVE_QH_PLUS:
            return g_curve_qh_plus[level - 1];
        case CURVE_MINUS:
            return g_curve_minus[level - 1];
        case CURVE_PLUS:
        default:
            /* CURVE_NONE is handled by the early return above and never
             * reaches here; default exists only to keep this switch total
             * over curve_id_t without a compiler warning. */
            return g_curve_plus[level - 1];
    }
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

int rules_planets_structure_production(structure_id_t id, int level, bool curve_upgrade_researched) {
    const structure_t *structure = rules_planets_structure(id);

    /* Mirrors rules_planets_job_production's "has a tech_gate and it's
     * researched" condition exactly (see this function's own header
     * comment). Written verbatim here rather than factored into a shared
     * helper with rules_planets_job_production, matching that function's own
     * precedent of not sharing with rules_planets_job_cost. */
    bool upgraded = structure->tech_gate != NULL && curve_upgrade_researched;

    return rules_planets_curve_value(structure->curve, level, upgraded);
}
