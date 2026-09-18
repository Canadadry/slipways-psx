#ifndef RULES_PLANETS_H
#define RULES_PLANETS_H

#include "fixed.h"

#include <stdbool.h>

/* Only the 7 resources this step's 5-job subset actually touches, plus a
 * RESOURCE_NONE sentinel (used for "no default need"). See
 * docs/prd/triage/psx-rules-planets.md, Implementation Decisions / Data
 * model -- the other 4 named resources (luxury, science, energy, joy) and
 * the joker resource belong to jobs/types this step deliberately defers. */
typedef enum {
    RESOURCE_NONE,
    RESOURCE_PEOPLE,
    RESOURCE_ORE,
    RESOURCE_TECH,
    RESOURCE_BOTS,
    RESOURCE_FOOD,
    RESOURCE_GOODS,
    RESOURCE_ORGANICS,
    /* r.w=12 per Annexe A, "luxury": produced by tourism/luxury (both added
     * in a later chunk of docs/prd/triage/psx-full-planets-and-tech.md),
     * needed by no in-scope job -- a genuine terminal sink in this module,
     * converging with docs/prd/psx-rules-economy.md's own independent r.w /
     * "luxury" trace for db()'s external-trade bonus and bf()'s happiness
     * formula (both outside this module). */
    RESOURCE_LUXURY,
} resource_id_t;

/* All 10 documented planet types, traced to df (pico8/lua.lua:1377-1389).
 * The first 3 (EARTHLIKE, FORGEWORLD, MINERAL) were ported by
 * docs/prd/psx-rules-planets.md; the remaining 7 were added by chunk 2 of
 * docs/prd/triage/psx-full-planets-and-tech.md. The special ll
 * ("unknown/empty") df entry (r=0, pico8/lua.lua:1378) is rules_discovery.c's
 * empty-planet-roll sentinel, not one of these 10, and is out of scope
 * here. */
typedef enum {
    PLANET_TYPE_EARTHLIKE,
    PLANET_TYPE_FORGEWORLD,
    PLANET_TYPE_MINERAL,
    /* df code "o", pico8/lua.lua:1377-1389, unstash Annexe A: rarity r=3. */
    PLANET_TYPE_OCEAN,
    /* df code "r", pico8/lua.lua:1377-1389, unstash Annexe A: rarity r=2.5. */
    PLANET_TYPE_REMNANT,
    /* df code "x", pico8/lua.lua:1377-1389, unstash Annexe A: rarity r=3. */
    PLANET_TYPE_XENO,
    /* df code "j", pico8/lua.lua:1377-1389, unstash Annexe A: rarity r=2. */
    PLANET_TYPE_JUNGLE,
    /* df code "i", pico8/lua.lua:1377-1389, unstash Annexe A: rarity r=2. */
    PLANET_TYPE_ICEBALL,
    /* df code "s", pico8/lua.lua:1377-1389, unstash Annexe A: rarity r=2. */
    PLANET_TYPE_BARREN,
    /* df code "g", pico8/lua.lua:1377-1389, unstash Annexe A: rarity r=4. */
    PLANET_TYPE_GASGIANT,
    PLANET_TYPE_COUNT,
} planet_type_id_t;

/* The 5 (of about 24 documented) colonization jobs this step ports --
 * chosen because they form one closed resource loop (see the PRD's
 * Implementation Decisions). */
typedef enum {
    JOB_HIVEWORLD,
    JOB_AGROWORLD,
    JOB_NANOTECH,
    JOB_BOTS,
    JOB_MINE,
    JOB_COUNT,
} colonization_job_id_t;

/* Production-by-level curve shapes, traced to hc (pico8/lua.lua, unstash
 * 8383). Only the two shapes this step's 5-job subset uses are declared
 * here -- the lookup function itself (rules_planets_curve_value) is chunk
 * 2's job, not this chunk's. */
typedef enum {
    CURVE_MINUS,
    CURVE_PLUS,
    /* hc (unstash Annexe A): f- = [0,1,2,3] for levels 1-4, used by
     * tourism/luxury (both added in a later chunk of
     * docs/prd/triage/psx-full-planets-and-tech.md). f-'s mc upgrade target
     * (f+) is documented in hc but never reachable from any in-scope job --
     * neither tourism nor luxury carries an il field (per that PRD's own
     * finding), so no upgrade-table entry is added for CURVE_F_MINUS here:
     * rules_planets_curve_value(CURVE_F_MINUS, level, upgraded=true) is a
     * documented no-op, returning the same non-upgraded CURVE_F_MINUS
     * values, matching CURVE_PLUS's own existing precedent of having no
     * upgrade target. */
    CURVE_F_MINUS,
} curve_id_t;

/* Maximum number of entries in a colonization_job_t's needs array. Source
 * jobs need 1 or 2 resources (this subset's max is bots' 2: ORE and TECH) --
 * sized to the real traced maximum, not padded for speculative headroom. */
#define RULES_PLANETS_MAX_NEEDS 2

/* One entry of the traced df table (pico8/lua.lua:1377-1389): a planet
 * type's display name and rarity weight. rarity is fixed_t (Q16.16), never
 * float/double, since 4.5 is not integral -- per the project's established
 * R3000A-has-no-FPU convention (see fixed.h). */
typedef struct {
    planet_type_id_t id;
    const char *name;
    fixed_t rarity;
} planet_type_t;

/* One entry of the traced fd table (pico8/lua.lua, unstash 8494): a
 * colonization job's planet type, base cost, resource needs, production,
 * curve, optional tech gate, and optional default need. Field order
 * cross-checked against hp.mr's positional-field-name list
 * (pico8/lua.lua:1400). tech_gate is NULL when the job has none;
 * default_need is RESOURCE_NONE when the job has none. */
typedef struct {
    planet_type_id_t planet_type;
    int cost;
    resource_id_t needs[RULES_PLANETS_MAX_NEEDS];
    int needs_count;
    resource_id_t produces;
    curve_id_t curve;
    const char *tech_gate;
    resource_id_t default_need;
} colonization_job_t;

/* Plain accessors into the static planet-type/job tables. id must be a
 * valid enum value in range (less than PLANET_TYPE_COUNT or JOB_COUNT) --
 * mirrors world_get_galaxy()'s simplicity, no elaborate error handling. */
const planet_type_t *rules_planets_type(planet_type_id_t id);
const colonization_job_t *rules_planets_job(colonization_job_id_t id);

/* Generic production-by-level curve lookup, traced to hc (pico8/lua.lua,
 * unstash 8383) and hp:po()'s curve-selection logic. level < 1 returns 0
 * (an unfounded colony produces nothing, per design doc SS9 point 1); level
 * is otherwise clamped to [1,4], the only traced levels. When upgraded is
 * true and curve is CURVE_MINUS, looks up CURVE_PLUS's table instead (the
 * mc upgrade mapping) -- CURVE_PLUS has no upgrade target in this subset,
 * so upgraded has no effect when curve is already CURVE_PLUS. */
int rules_planets_curve_value(curve_id_t curve, int level, bool upgraded);

/* CURVE_F_MINUS values, traced to hc (unstash Annexe A): f- = [0,1,2,3] for
 * levels 1-4. It has no upgrade target in this module's scope (see
 * curve_id_t's own doc comment), so upgraded has no effect on it, matching
 * CURVE_PLUS's own precedent. */

/* Job cost formula, ports hp:od() exactly (pico8/lua.lua:1426-1433). Starts
 * from the job's base cost; if the job has a tech_gate AND
 * job_tech_researched is true, multiplies by 1.5; if
 * nanomaterials_researched is true, multiplies by 0.85 (unconditionally).
 * Computed in Q16.16 fixed-point (chained fixed_mul calls), floored to an
 * integer ONCE at the very end -- not after each multiplication step --
 * matching PICO-8's own internal Q16.16 number representation. */
int rules_planets_job_cost(colonization_job_id_t id, bool job_tech_researched, bool nanomaterials_researched);

/* Per-job production wrapper, ports hp:po()'s curve-selection logic
 * (pico8/lua.lua:1422-1425) exactly: resolves the job's curve_id_t and
 * whether it's upgraded (its tech_gate is present AND job_tech_researched is
 * true -- the exact same condition rules_planets_job_cost uses for its
 * surcharge, since source gates both off self.il and oj(self.il)), then
 * calls rules_planets_curve_value. level follows that function's clamping
 * (0 below level 1, clamped at level 4). */
int rules_planets_job_production(colonization_job_id_t id, int level, bool job_tech_researched);

#endif
