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
    /* Chunk 1 (docs/prd/triage/psx-structures-and-transforms.md): the exact 4
     * resources this step's 6 structure records and 4 transform records
     * touch, confirmed by exhaustive scan -- science (Laboratory's produce),
     * energy (Protostar's produce), joy (Ascension Gate's produce), and the
     * joker (Laboratory's/Trading Hub's flexible need slots, Synthesizer's
     * dynamic produce). No structure/transform data is added by this chunk
     * itself -- these are pure enum appends, exercised by later chunks. */
    RESOURCE_SCIENCE,
    RESOURCE_ENERGY,
    RESOURCE_JOY,
    RESOURCE_JOKER,
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

/* The original 5 (of about 24 documented) colonization jobs
 * docs/prd/psx-rules-planets.md ported -- chosen because they form one
 * closed resource loop (see that PRD's Implementation Decisions). Chunk 4
 * (docs/prd/triage/psx-full-planets-and-tech.md) appends the first 3 of the
 * full roster's remaining jobs: JOB_GOODS/JOB_GADGETS/JOB_UPLIFT, all 3
 * single-type forgeworld jobs -- the simplest possible "add a new job"
 * tracer bullet. These 3 are intentionally NOT part of the original 5's
 * closed loop (see rules_planets_test.c's own closed-loop test scope note);
 * later chunks append the rest. */
typedef enum {
    JOB_HIVEWORLD,
    JOB_AGROWORLD,
    JOB_NANOTECH,
    JOB_BOTS,
    JOB_MINE,
    JOB_GOODS,
    JOB_GADGETS,
    JOB_UPLIFT,
    /* Chunk 5 (docs/prd/triage/psx-full-planets-and-tech.md): ocean batch,
     * the first chunk to combine a new planet type (PLANET_TYPE_OCEAN, added
     * by chunk 2 but unused by any job until now) with new jobs on it.
     * colony(ocean) and algae both carry a real tech_gate (il) AND a real
     * default_need (dm) -- exercised the same way agroworld's own existing
     * il/dm pair already is. breed(ocean) has neither, matching bots'/
     * goods'/gadgets' own CURVE_PLUS precedent. */
    JOB_COLONY_OCEAN,
    JOB_ALGAE,
    JOB_BREED_OCEAN,
    /* Chunk 6 (docs/prd/triage/psx-full-planets-and-tech.md): remnant batch,
     * the first jobs on PLANET_TYPE_REMNANT (added by chunk 2 but unused by
     * any job until now). Deliberately simple: none of the 3 carries a
     * tech_gate (il), a required_tech (rr), or a default_need (dm) -- placed
     * here to keep pace varied and each chunk reviewable in isolation. */
    JOB_STRIP,
    JOB_DISMANTLE,
    JOB_SCAVENGE,
    /* Chunk 7 (docs/prd/triage/psx-full-planets-and-tech.md): xeno + jungle
     * batch, the first chunk to combine TWO new planet types
     * (PLANET_TYPE_XENO, PLANET_TYPE_JUNGLE -- both added by chunk 2 but
     * unused by any job until now) in one chunk. colony(xeno) is tech-gated
     * exactly like colony(ocean) from chunk 5 (same tech_gate = "gene
     * rewriting", same base cost 10, same default_need = RESOURCE_GOODS).
     * breed(xeno) and hunt have no tech_gate, no default_need, no
     * required_tech -- the simple no-gate pattern chunks 4/5/6 already
     * established. tourism is the first real job to use CURVE_F_MINUS
     * (added in chunk 1, previously only exercised by a synthetic unit test
     * on rules_planets_curve_value directly) -- proving the curve lookup is
     * genuinely job-agnostic for real data, not just synthetic test data. */
    JOB_COLONY_XENO,
    JOB_BREED_XENO,
    JOB_HUNT,
    JOB_TOURISM,
    /* Chunk 8 (docs/prd/triage/psx-full-planets-and-tech.md): "Earthlike
     * remaining" -- a single new job, luxury, on the already-included
     * PLANET_TYPE_EARTHLIKE type. It's the second real job to use
     * CURVE_F_MINUS (after chunk 7's tourism), deliberately chosen to prove
     * the curve lookup is genuinely job-agnostic across independent jobs,
     * not just correct for one specific job. No tech_gate, no default_need,
     * no required_tech -- the same no-gate shape as tourism itself. */
    JOB_LUXURY,
    /* Chunk 9 (docs/prd/triage/psx-full-planets-and-tech.md): barren/iceball/
     * gas-giant batch -- the first chunk to genuinely exercise
     * planet_types_count == 2 (habitat/genefarm: barren+iceball;
     * excavate_iceball_gasgiant: iceball+gas giant; farm: xeno+jungle, both
     * already added by chunk 7). excavate_mineral is a NEW job on the
     * already-existing PLANET_TYPE_MINERAL, deliberately placed next to
     * excavate_iceball_gasgiant despite sharing identical cost/needs/produces
     * (12/{bots}/ore) -- their curves differ (CURVE_PLUS vs. CURVE_MINUS) and
     * their planet_types differ, proving they're genuinely distinct entries,
     * not a copy-paste-confused merge (see this PRD's own explicit risk
     * flag). None of the 5 carries a tech_gate (il); habitat, genefarm,
     * excavate_mineral, and excavate_iceball_gasgiant DO carry a
     * required_tech (rr), captured as data only, mirroring uplift's own
     * chunk-4 precedent. */
    JOB_HABITAT,
    JOB_GENEFARM,
    JOB_EXCAVATE_MINERAL,
    JOB_EXCAVATE_ICEBALL_GASGIANT,
    JOB_FARM,
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
     * docs/prd/triage/psx-full-planets-and-tech.md) and, from chunk 2
     * (docs/prd/triage/psx-structures-and-transforms.md), Food processor.
     * f-'s mc upgrade target (f+) is documented in hc but was never reachable
     * from any in-scope job when this enum value was first added -- neither
     * tourism nor luxury carries an il field, so their production is
     * unaffected by upgraded regardless. Food processor's real
     * il="xenofoods" field is the first genuinely reachable case: chunk 2
     * wires a CURVE_F_MINUS->CURVE_F_PLUS upgrade-remap branch into
     * rules_planets_curve_value (mirroring CURVE_MINUS->CURVE_PLUS's/
     * CURVE_QH->CURVE_QH_PLUS's own precedent), so
     * rules_planets_curve_value(CURVE_F_MINUS, level, upgraded=true) now
     * returns CURVE_F_PLUS's values -- no longer a no-op in general, though
     * it remains one for any caller (tourism/luxury) whose own upgraded flag
     * is structurally always false. */
    CURVE_F_MINUS,
    /* Chunk 1 (docs/prd/triage/psx-structures-and-transforms.md): the 3 real
     * curve shapes this step's structures need (hc, Annexe A) plus one new
     * sentinel. f+ = [0,2,3,4] for levels 1-4, no mc upgrade target of its
     * own (Ascension Gate; also Food processor's il-gated upgrade target
     * from f-). qh = [0,1,3,5], with a real mc upgrade target to qh+
     * (Laboratory) -- the upgrade-remap branch is wired into
     * rules_planets_curve_value in this same chunk (mirroring
     * CURVE_MINUS->CURVE_PLUS's own existing precedent) since it is pure
     * curve-lookup infrastructure, not structure/transform data; chunk 2
     * (Laboratory) is the first REAL, non-synthetic caller to exercise it.
     * qh+ = [0,2,5,8], no upgrade target of its own. */
    CURVE_F_PLUS,
    CURVE_QH,
    CURVE_QH_PLUS,
    /* CURVE_NONE has NO per-level table at all -- Slipgate/Infragate's
     * genuine "no production" case (this step's own new infrastructure, not
     * a direct hc table). rules_planets_curve_value(CURVE_NONE, level,
     * upgraded) returns 0 unconditionally, for any level (including
     * out-of-range) and any upgraded value -- a real sentinel, mirroring
     * RESOURCE_NONE's own precedent, not a magic-number special case buried
     * elsewhere. */
    CURVE_NONE,
} curve_id_t;

/* Maximum number of entries in a colonization_job_t's needs array. Source
 * jobs need 1 or 2 resources (this subset's max is bots' 2: ORE and TECH) --
 * sized to the real traced maximum, not padded for speculative headroom. */
#define RULES_PLANETS_MAX_NEEDS 2

/* Maximum number of entries in a structure_t's resolvable[] joker-resolution
 * set. Laboratory's/Trading Hub's own ba field is the real traced maximum (6
 * resources) -- see docs/prd/triage/psx-structures-and-transforms.md,
 * "Data model changes needed". */
#define RULES_PLANETS_MAX_RESOLVABLE 6

/* Maximum number of planet types a single colonization_job_t can be valid
 * on. Chunk 3 (docs/prd/triage/psx-full-planets-and-tech.md): source's own
 * `lr` field is a string of 1 or 2 planet-type codes -- no job in the full
 * 24-job roster spans 3+ types (confirmed by that PRD's own exhaustive `fd`
 * re-decode), so this is sized to the real traced maximum, not padded for
 * speculative headroom, mirroring RULES_PLANETS_MAX_NEEDS's own precedent. */
#define RULES_PLANETS_MAX_TYPES 2

/* Maximum number of entries in a transform_action_t's applicable_types[]
 * array. Chunk 7 (docs/prd/triage/psx-structures-and-transforms.md): the
 * PRD's own "Data model changes needed" section states this as 7 ("mechanize's
 * `lr`, the real traced maximum"), but that is a transcription error in the
 * PRD itself, caught by re-verifying directly against the PRD's own raw `fd`
 * dump rather than trusting its stated constant: harvest's own `lr` string,
 * "efmorxjisg" (o("harvest","efmorxjisg",-20,ii=23,cw="ga",rn=46), the PRD's
 * "Transform/destroy actions" section), is 10 distinct single-character
 * planet-type codes (e,f,m,o,r,x,j,i,s,g -- all 10 of PLANET_TYPE_COUNT),
 * strictly larger than mechanize's own 7-character "mrsoijx". harvest is
 * genuinely applicable to every planet type (per the PRD's own transform
 * table row: "all 10 (`efmorxjisg`)"), so 7 is not the real traced maximum --
 * 10 is. This mirrors this same PRD's own precedent of correcting two
 * design-doc errors (Trading Hub's produce, Synthesizer's resolvable set)
 * from source rather than copying them: here the source being corrected
 * against is the PRD's own stated constant, re-verified against the PRD's own
 * cited raw data. */
#define RULES_PLANETS_MAX_TRANSFORM_TYPES 10

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
 * colonization job's planet type(s), base cost, resource needs, production,
 * curve, optional cost/curve tech gate, optional default need, and optional
 * required tech. Field order cross-checked against hp.mr's
 * positional-field-name list (pico8/lua.lua:1400). tech_gate is NULL when
 * the job has none; default_need is RESOURCE_NONE when the job has none.
 *
 * planet_types/planet_types_count (chunk 3,
 * docs/prd/triage/psx-full-planets-and-tech.md) replace a single
 * planet_type_id_t field: source's own `lr` field is a string of 1 or 2
 * planet-type codes (4 of the full 24-job roster's jobs are valid on two
 * types from a single source record), which a single-value field cannot
 * represent faithfully. Mirrors needs[]/needs_count's existing shape
 * exactly.
 *
 * required_tech (chunk 3) captures the raw fd array's second, distinct
 * tech-gating mechanism -- `rr`, a job-availability gate (whether the job is
 * offered as a choice at all), consumed by a small stateful loop at
 * pico8/lua.lua:1434-1440 -- as opposed to tech_gate (`il`), a cost
 * surcharge + curve upgrade on an already-available job, already wired via
 * rules_planets_job_cost/rules_planets_job_production. required_tech is
 * captured as data only, mirroring default_need's own precedent: no
 * function applies it (see this PRD's Out of Scope -- no live
 * colonization-choice menu exists anywhere in this port to apply it to).
 * NULL when the job has no rr gate; none of this chunk's 5 existing jobs do. */
typedef struct {
    planet_type_id_t planet_types[RULES_PLANETS_MAX_TYPES];
    int planet_types_count;
    int cost;
    resource_id_t needs[RULES_PLANETS_MAX_NEEDS];
    int needs_count;
    resource_id_t produces;
    curve_id_t curve;
    const char *tech_gate;
    resource_id_t default_need;
    const char *required_tech;
} colonization_job_t;

/* The 7 purchasable structures (Laboratory, Food processor, Slipgate,
 * Infragate, Trading Hub, Synthesizer, Ascension Gate) plus the
 * non-purchasable Protostar (STRUCTURE_PROTOSTAR, created only by the
 * collapse transform action, chunk 7's job -- never bought directly), traced
 * to fd's 6 structure-shaped records (unstash 8494) plus dr's 7 tooltip
 * entries (unstash 9807). Ordered to match
 * docs/design/slipways-mechanics.md SS6's own table, with Protostar appended
 * last and clearly marked non-purchasable. Chunk 2
 * (docs/prd/triage/psx-structures-and-transforms.md) declares the full
 * 8-member enum but only populates g_structures[]'s
 * STRUCTURE_LABORATORY/STRUCTURE_FOOD_PROCESSOR entries -- the other 6 stay
 * zero-initialized (uninitialized-by-omission via C's designated-initializer
 * semantics) until chunks 3-5 fill them in. */
typedef enum {
    STRUCTURE_LABORATORY,
    STRUCTURE_FOOD_PROCESSOR,
    STRUCTURE_SLIPGATE,
    STRUCTURE_INFRAGATE,
    STRUCTURE_TRADING_HUB,
    STRUCTURE_SYNTHESIZER,
    STRUCTURE_ASCENSION_GATE,
    /* Non-purchasable: created only by the collapse transform action (chunk
     * 7's job). Deliberately last -- see this enum's own doc comment. */
    STRUCTURE_PROTOSTAR,
    STRUCTURE_COUNT,
} structure_id_t;

/* One entry of the traced fd structure records (unstash 8494) plus each
 * structure entity's own literal fields (pico8/lua.lua:905-1127). Unlike
 * colonization_job_t, cost is a flat literal (od) -- never run through
 * rules_planets_job_cost's tech-multiplier formula (see this PRD's Cost
 * model section) -- and required_tech is a purchase gate (entity-literal rr),
 * structurally distinct from tech_gate (il, a curve-upgrade gate applied via
 * rules_planets_structure_production, same mechanism as
 * colonization_job_t.tech_gate). resolvable[]/resolvable_count captures the
 * joker (?) need/produce resolution set (ba/ht); dynamic resolution at
 * runtime is out of scope (see PRD Out of Scope). max_connections/
 * crosses_other_routes are Slipgate/Infragate-only (chunk 5's job); 0/false
 * for every other structure. */
typedef struct {
    const char *name;
    bool purchasable;                 /* false only for STRUCTURE_PROTOSTAR */
    int cost;                          /* od; meaningless when purchasable == false */
    const char *required_tech;         /* entity-literal rr; NULL for Laboratory/Food processor/Protostar */
    resource_id_t needs[RULES_PLANETS_MAX_NEEDS];
    int needs_count;
    resource_id_t produces;            /* RESOURCE_NONE for Slipgate/Infragate */
    curve_id_t curve;                  /* CURVE_NONE for Slipgate/Infragate */
    const char *tech_gate;             /* il; curve-upgrade gate, NULL if none */
    resource_id_t default_need;        /* dm; RESOURCE_NONE if none */
    resource_id_t resolvable[RULES_PLANETS_MAX_RESOLVABLE]; /* ba/ht: joker-resolution set */
    int resolvable_count;
    int max_connections;               /* Slipgate/Infragate only; 0 otherwise */
    bool crosses_other_routes;          /* Infragate's mt=1; false for every other structure */
} structure_t;

/* The 4 planet-transform/destroy actions (mechanize, terraform, collapse,
 * harvest), traced to fd's 4 transform-shaped records (unstash 8494), each
 * preceded by exactly one bare required_tech string in fd's own array order
 * (the same bare-string-loop mechanism real jobs' required_tech already
 * uses). See docs/prd/triage/psx-structures-and-transforms.md, "Transform/
 * destroy actions, re-verified against the current, larger fd decode". */
typedef enum {
    TRANSFORM_MECHANIZE,
    TRANSFORM_TERRAFORM,
    TRANSFORM_COLLAPSE,
    TRANSFORM_HARVEST,
    TRANSFORM_COUNT,
} transform_action_id_t;

/* Distinguishes what a transform's target field (go vs. cw in source) names:
 * `go` (mechanize/terraform) names a single planet_type_id_t the planet's
 * type becomes; `cw` (collapse/harvest) names a different *entity* the planet
 * becomes instead of a plain type -- Protostar (collapse) or a plain
 * destroy/particle-effect marker with no further game-mechanical role
 * (harvest). result_planet_type is meaningful only for
 * TRANSFORM_RESULT_PLANET_TYPE. */
typedef enum {
    TRANSFORM_RESULT_PLANET_TYPE,
    TRANSFORM_RESULT_PROTOSTAR,
    TRANSFORM_RESULT_DESTROYED,
} transform_result_kind_t;

/* One entry of the traced fd transform records (unstash 8494). cost is
 * negative for harvest (a genuine refund, ported as-is, never clamped to
 * non-negative). required_tech is the raw fd array's bare-string rr gate
 * (the same job-availability mechanism colonization_job_t.required_tech
 * already uses), captured as data only -- no live colonization/transform-
 * commit menu exists anywhere in this port to apply it to (see this PRD's Out
 * of Scope). result_planet_type is valid iff result_kind ==
 * TRANSFORM_RESULT_PLANET_TYPE; for TRANSFORM_RESULT_PROTOSTAR/
 * TRANSFORM_RESULT_DESTROYED it holds an unasserted placeholder value only. */
typedef struct {
    const char *name;
    planet_type_id_t applicable_types[RULES_PLANETS_MAX_TRANSFORM_TYPES];
    int applicable_types_count;
    int cost;                          /* negative for harvest (a refund) */
    const char *required_tech;         /* rr, via fd's bare-string mechanism (same as real jobs) */
    transform_result_kind_t result_kind;
    planet_type_id_t result_planet_type; /* valid iff result_kind == TRANSFORM_RESULT_PLANET_TYPE */
} transform_action_t;

/* Plain accessors into the static planet-type/job/structure tables. id must
 * be a valid enum value in range (less than PLANET_TYPE_COUNT, JOB_COUNT, or
 * STRUCTURE_COUNT) -- mirrors world_get_galaxy()'s simplicity, no elaborate
 * error handling. */
const planet_type_t *rules_planets_type(planet_type_id_t id);
const colonization_job_t *rules_planets_job(colonization_job_id_t id);
const structure_t *rules_planets_structure(structure_id_t id);
const transform_action_t *rules_planets_transform(transform_action_id_t id);

/* Mirrors source's own bj() filter (pico8/lua.lua:1391-1398), minus the
 * oj(b.rr) tech-researched check (required_tech stays a caller-supplied
 * concern, exactly like every other tech gate in this module -- see
 * colonization_job_t's own doc comment). Returns true iff type appears
 * anywhere in job id's planet_types[0..planet_types_count-1]. */
bool rules_planets_job_available_on(colonization_job_id_t id, planet_type_id_t type);

/* Mirrors rules_planets_job_available_on exactly, for transform_action_t's
 * applicable_types[] in place of colonization_job_t's planet_types[]. Returns
 * true iff type appears anywhere in id's
 * applicable_types[0..applicable_types_count-1]. */
bool rules_planets_transform_applicable_to(transform_action_id_t id, planet_type_id_t type);

/* Generic production-by-level curve lookup, traced to hc (pico8/lua.lua,
 * unstash 8383) and hp:po()'s curve-selection logic. level < 1 returns 0
 * (an unfounded colony produces nothing, per design doc SS9 point 1); level
 * is otherwise clamped to [1,4], the only traced levels. When upgraded is
 * true, looks up the curve's own mc upgrade target instead: CURVE_MINUS ->
 * CURVE_PLUS, CURVE_QH -> CURVE_QH_PLUS, and (chunk 2,
 * docs/prd/triage/psx-structures-and-transforms.md) CURVE_F_MINUS ->
 * CURVE_F_PLUS. Every other curve (including each of those 3 curves'
 * upgrade targets) has no upgrade target of its own, so upgraded has no
 * effect on it. */
int rules_planets_curve_value(curve_id_t curve, int level, bool upgraded);

/* CURVE_F_PLUS/CURVE_QH/CURVE_QH_PLUS values, traced to hc (unstash Annexe
 * A): f+ = [0,2,3,4], qh = [0,1,3,5], qh+ = [0,2,5,8], all for levels 1-4.
 * When upgraded is true and curve is CURVE_QH, looks up CURVE_QH_PLUS's
 * table instead (the mc upgrade mapping), mirroring CURVE_MINUS's own
 * upgrade-to-CURVE_PLUS precedent exactly. CURVE_F_PLUS and CURVE_QH_PLUS
 * have no upgrade target of their own, so upgraded has no effect on them.
 * CURVE_NONE has no table at all: it returns 0 for any level/upgraded
 * combination (see curve_id_t's own doc comment). */

/* CURVE_F_MINUS values, traced to hc (unstash Annexe A): f- = [0,1,2,3] for
 * levels 1-4. Its mc upgrade target (f+) is genuinely reachable as of chunk 2
 * (docs/prd/triage/psx-structures-and-transforms.md, Food processor's real
 * il="xenofoods" field) -- see curve_id_t's own doc comment and
 * rules_planets_curve_value's. */

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

/* Per-structure production wrapper, mirrors rules_planets_job_production
 * exactly: resolves the structure's curve_id_t and whether it's upgraded
 * (its tech_gate is present AND curve_upgrade_researched is true), then calls
 * rules_planets_curve_value. level follows that function's clamping (0 below
 * level 1, clamped at level 4). Ports hp:po()'s curve-selection logic
 * (pico8/lua.lua:1422-1425), confirmed shared verbatim between jobs and
 * structures (see this PRD's Implementation Decisions, "Curve-upgrade il"). */
int rules_planets_structure_production(structure_id_t id, int level, bool curve_upgrade_researched);

#endif
