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
} resource_id_t;

/* The 3 (of 10 documented) planet types this step ports. See
 * docs/prd/triage/psx-rules-planets.md, Out of Scope for the other 7. */
typedef enum {
    PLANET_TYPE_EARTHLIKE,
    PLANET_TYPE_FORGEWORLD,
    PLANET_TYPE_MINERAL,
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

#endif
