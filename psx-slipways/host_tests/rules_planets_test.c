#include "t.h"
#include "../fixed.h"
#include "../rules_planets.h"

#include <string.h>

static void test_earthlike_type_matches_traced_values(void) {
    const planet_type_t *t = rules_planets_type(PLANET_TYPE_EARTHLIKE);

    T_CHECK(t != NULL);
    T_CHECK(t->id == PLANET_TYPE_EARTHLIKE);
    T_CHECK(strcmp(t->name, "earth-like") == 0);
    T_CHECK(t->rarity == (fixed_t)(4.5 * FX_ONE));
}

static void test_forgeworld_type_matches_traced_values(void) {
    const planet_type_t *t = rules_planets_type(PLANET_TYPE_FORGEWORLD);

    T_CHECK(t != NULL);
    T_CHECK(t->id == PLANET_TYPE_FORGEWORLD);
    T_CHECK(strcmp(t->name, "forgeworld") == 0);
    T_CHECK(t->rarity == (fixed_t)(6 * FX_ONE));
}

static void test_mineral_type_matches_traced_values(void) {
    const planet_type_t *t = rules_planets_type(PLANET_TYPE_MINERAL);

    T_CHECK(t != NULL);
    T_CHECK(t->id == PLANET_TYPE_MINERAL);
    T_CHECK(strcmp(t->name, "mineral") == 0);
    T_CHECK(t->rarity == (fixed_t)(4 * FX_ONE));
}

/* Chunk 2 (docs/prd/triage/psx-full-planets-and-tech.md): the 7 new planet
 * types, traced to df (pico8/lua.lua:1377-1389, unstash Annexe A). */
static void test_ocean_type_matches_traced_values(void) {
    const planet_type_t *t = rules_planets_type(PLANET_TYPE_OCEAN);

    T_CHECK(t != NULL);
    T_CHECK(t->id == PLANET_TYPE_OCEAN);
    T_CHECK(strcmp(t->name, "ocean") == 0);
    T_CHECK(t->rarity == (fixed_t)(3 * FX_ONE));
}

static void test_remnant_type_matches_traced_values(void) {
    const planet_type_t *t = rules_planets_type(PLANET_TYPE_REMNANT);

    T_CHECK(t != NULL);
    T_CHECK(t->id == PLANET_TYPE_REMNANT);
    T_CHECK(strcmp(t->name, "remnant") == 0);
    T_CHECK(t->rarity == (fixed_t)(2.5 * FX_ONE));
}

static void test_xeno_type_matches_traced_values(void) {
    const planet_type_t *t = rules_planets_type(PLANET_TYPE_XENO);

    T_CHECK(t != NULL);
    T_CHECK(t->id == PLANET_TYPE_XENO);
    T_CHECK(strcmp(t->name, "xeno") == 0);
    T_CHECK(t->rarity == (fixed_t)(3 * FX_ONE));
}

static void test_jungle_type_matches_traced_values(void) {
    const planet_type_t *t = rules_planets_type(PLANET_TYPE_JUNGLE);

    T_CHECK(t != NULL);
    T_CHECK(t->id == PLANET_TYPE_JUNGLE);
    T_CHECK(strcmp(t->name, "jungle") == 0);
    T_CHECK(t->rarity == (fixed_t)(2 * FX_ONE));
}

static void test_iceball_type_matches_traced_values(void) {
    const planet_type_t *t = rules_planets_type(PLANET_TYPE_ICEBALL);

    T_CHECK(t != NULL);
    T_CHECK(t->id == PLANET_TYPE_ICEBALL);
    T_CHECK(strcmp(t->name, "iceball") == 0);
    T_CHECK(t->rarity == (fixed_t)(2 * FX_ONE));
}

static void test_barren_type_matches_traced_values(void) {
    const planet_type_t *t = rules_planets_type(PLANET_TYPE_BARREN);

    T_CHECK(t != NULL);
    T_CHECK(t->id == PLANET_TYPE_BARREN);
    T_CHECK(strcmp(t->name, "barren") == 0);
    T_CHECK(t->rarity == (fixed_t)(2 * FX_ONE));
}

static void test_gasgiant_type_matches_traced_values(void) {
    const planet_type_t *t = rules_planets_type(PLANET_TYPE_GASGIANT);

    T_CHECK(t != NULL);
    T_CHECK(t->id == PLANET_TYPE_GASGIANT);
    T_CHECK(strcmp(t->name, "gas giant") == 0);
    T_CHECK(t->rarity == (fixed_t)(4 * FX_ONE));
}

/* Chunk 2's own sanity guard: PLANET_TYPE_COUNT must be exactly 10 after
 * this chunk's 7-type extension (3 pre-existing + 7 new), matching the PRD's
 * chunk 10 "PLANET_TYPE_COUNT == 10" full-roster guard applied early, at the
 * point where this module's own enum actually grows to that size. */
static void test_planet_type_count_is_10(void) {
    T_CHECK(PLANET_TYPE_COUNT == 10);
}

static void test_mine_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_MINE);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_types_count == 1);
    T_CHECK(j->planet_types[0] == PLANET_TYPE_MINERAL);
    T_CHECK(j->cost == 5);
    T_CHECK(j->needs_count == 1);
    T_CHECK(j->needs[0] == RESOURCE_PEOPLE);
    T_CHECK(j->produces == RESOURCE_ORE);
    T_CHECK(j->curve == CURVE_MINUS);
    T_CHECK(j->tech_gate == NULL);
    T_CHECK(j->default_need == RESOURCE_NONE);
    T_CHECK(j->required_tech == NULL);
}

static void test_nanotech_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_NANOTECH);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_types_count == 1);
    T_CHECK(j->planet_types[0] == PLANET_TYPE_FORGEWORLD);
    T_CHECK(j->cost == 8);
    T_CHECK(j->needs_count == 1);
    T_CHECK(j->needs[0] == RESOURCE_ORE);
    T_CHECK(j->produces == RESOURCE_TECH);
    T_CHECK(j->curve == CURVE_MINUS);
    T_CHECK(j->tech_gate == NULL);
    T_CHECK(j->default_need == RESOURCE_NONE);
    T_CHECK(j->required_tech == NULL);
}

static void test_bots_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_BOTS);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_types_count == 1);
    T_CHECK(j->planet_types[0] == PLANET_TYPE_FORGEWORLD);
    T_CHECK(j->cost == 8);
    T_CHECK(j->needs_count == 2);
    T_CHECK(j->needs[0] == RESOURCE_ORE);
    T_CHECK(j->needs[1] == RESOURCE_TECH);
    T_CHECK(j->produces == RESOURCE_BOTS);
    T_CHECK(j->curve == CURVE_PLUS);
    T_CHECK(j->tech_gate == NULL);
    T_CHECK(j->default_need == RESOURCE_NONE);
    T_CHECK(j->required_tech == NULL);
}

static void test_agroworld_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_AGROWORLD);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_types_count == 1);
    T_CHECK(j->planet_types[0] == PLANET_TYPE_EARTHLIKE);
    T_CHECK(j->cost == 5);
    T_CHECK(j->needs_count == 1);
    T_CHECK(j->needs[0] == RESOURCE_BOTS);
    T_CHECK(j->produces == RESOURCE_FOOD);
    T_CHECK(j->curve == CURVE_MINUS);
    T_CHECK(j->tech_gate != NULL);
    T_CHECK(strcmp(j->tech_gate, "xenofoods") == 0);
    T_CHECK(j->default_need == RESOURCE_ORGANICS);
    T_CHECK(j->required_tech == NULL);
}

/* Chunk 3 (docs/prd/triage/psx-full-planets-and-tech.md): each of the 5
 * existing jobs is available on exactly its one real planet type, and on
 * none of the other 9 -- walked generically over PLANET_TYPE_COUNT (now 10,
 * per chunk 2) rather than hardcoded per-type checks, mirroring this file's
 * own closed-loop tests' style of exercising the public accessor across the
 * whole enum range instead of a few hand-picked values. Chunk 4 appends the
 * 3 new forgeworld-batch jobs' cases to this same generic table rather than
 * writing separate tests, since the property under test (available on
 * exactly the listed type(s), false everywhere else) is identical. */
static void test_job_available_on_matches_exactly_its_one_real_type(void) {
    static const struct {
        colonization_job_id_t job;
        planet_type_id_t type;
    } cases[] = {
        { JOB_HIVEWORLD, PLANET_TYPE_EARTHLIKE },
        { JOB_AGROWORLD, PLANET_TYPE_EARTHLIKE },
        { JOB_NANOTECH, PLANET_TYPE_FORGEWORLD },
        { JOB_BOTS, PLANET_TYPE_FORGEWORLD },
        { JOB_MINE, PLANET_TYPE_MINERAL },
        /* Chunk 4: forgeworld batch, all single-type. */
        { JOB_GOODS, PLANET_TYPE_FORGEWORLD },
        { JOB_GADGETS, PLANET_TYPE_FORGEWORLD },
        { JOB_UPLIFT, PLANET_TYPE_FORGEWORLD },
        /* Chunk 5: ocean batch, all single-type. */
        { JOB_COLONY_OCEAN, PLANET_TYPE_OCEAN },
        { JOB_ALGAE, PLANET_TYPE_OCEAN },
        { JOB_BREED_OCEAN, PLANET_TYPE_OCEAN },
        /* Chunk 6: remnant batch, all single-type. */
        { JOB_STRIP, PLANET_TYPE_REMNANT },
        { JOB_DISMANTLE, PLANET_TYPE_REMNANT },
        { JOB_SCAVENGE, PLANET_TYPE_REMNANT },
        /* Chunk 7: xeno + jungle batch, all single-type. */
        { JOB_COLONY_XENO, PLANET_TYPE_XENO },
        { JOB_BREED_XENO, PLANET_TYPE_XENO },
        { JOB_HUNT, PLANET_TYPE_JUNGLE },
        { JOB_TOURISM, PLANET_TYPE_JUNGLE },
        /* Chunk 8: luxury, earthlike batch, single-type. */
        { JOB_LUXURY, PLANET_TYPE_EARTHLIKE },
        /* Chunk 9: excavate_mineral is this chunk's one single-real-type
         * job, so it fits this generic table directly; the 4 genuinely
         * multi-type jobs (habitat, genefarm, excavate_iceball_gasgiant,
         * farm) are covered separately by
         * test_multi_type_job_available_on_matches_exactly_its_two_real_types,
         * since this table's row shape assumes exactly one real type. */
        { JOB_EXCAVATE_MINERAL, PLANET_TYPE_MINERAL },
    };

    for (size_t c = 0; c < sizeof(cases) / sizeof(cases[0]); c++) {
        for (int t = 0; t < PLANET_TYPE_COUNT; t++) {
            bool expected = ((planet_type_id_t)t == cases[c].type);
            T_CHECK(rules_planets_job_available_on(cases[c].job, (planet_type_id_t)t) == expected);
        }
    }
}

static void test_curve_minus_levels_1_to_4_match_traced_values(void) {
    T_CHECK(rules_planets_curve_value(CURVE_MINUS, 1, false) == 1);
    T_CHECK(rules_planets_curve_value(CURVE_MINUS, 2, false) == 2);
    T_CHECK(rules_planets_curve_value(CURVE_MINUS, 3, false) == 3);
    T_CHECK(rules_planets_curve_value(CURVE_MINUS, 4, false) == 3);
}

static void test_curve_plus_levels_1_to_4_match_traced_values(void) {
    T_CHECK(rules_planets_curve_value(CURVE_PLUS, 1, false) == 2);
    T_CHECK(rules_planets_curve_value(CURVE_PLUS, 2, false) == 3);
    T_CHECK(rules_planets_curve_value(CURVE_PLUS, 3, false) == 4);
    T_CHECK(rules_planets_curve_value(CURVE_PLUS, 4, false) == 4);
}

static void test_curve_value_level_below_1_returns_0(void) {
    T_CHECK(rules_planets_curve_value(CURVE_MINUS, 0, false) == 0);
    T_CHECK(rules_planets_curve_value(CURVE_MINUS, -1, false) == 0);
    T_CHECK(rules_planets_curve_value(CURVE_PLUS, 0, false) == 0);
    T_CHECK(rules_planets_curve_value(CURVE_PLUS, -5, false) == 0);
}

static void test_curve_minus_upgraded_returns_curve_plus_values(void) {
    T_CHECK(rules_planets_curve_value(CURVE_MINUS, 1, true) == 2);
    T_CHECK(rules_planets_curve_value(CURVE_MINUS, 2, true) == 3);
    T_CHECK(rules_planets_curve_value(CURVE_MINUS, 3, true) == 4);
    T_CHECK(rules_planets_curve_value(CURVE_MINUS, 4, true) == 4);
}

static void test_curve_plus_upgraded_is_a_no_op(void) {
    T_CHECK(rules_planets_curve_value(CURVE_PLUS, 1, true) == 2);
    T_CHECK(rules_planets_curve_value(CURVE_PLUS, 2, true) == 3);
    T_CHECK(rules_planets_curve_value(CURVE_PLUS, 3, true) == 4);
    T_CHECK(rules_planets_curve_value(CURVE_PLUS, 4, true) == 4);
}

static void test_curve_value_level_above_4_clamps_to_level_4(void) {
    T_CHECK(rules_planets_curve_value(CURVE_MINUS, 5, false) == 3);
    T_CHECK(rules_planets_curve_value(CURVE_MINUS, 100, false) == 3);
    T_CHECK(rules_planets_curve_value(CURVE_PLUS, 5, false) == 4);
    T_CHECK(rules_planets_curve_value(CURVE_MINUS, 5, true) == 4);
}

/* Chunk 1 (docs/prd/triage/psx-full-planets-and-tech.md): CURVE_F_MINUS,
 * traced to hc (unstash Annexe A), f- = [0,1,2,3] for levels 1-4. */
static void test_curve_f_minus_levels_1_to_4_match_traced_values(void) {
    T_CHECK(rules_planets_curve_value(CURVE_F_MINUS, 1, false) == 0);
    T_CHECK(rules_planets_curve_value(CURVE_F_MINUS, 2, false) == 1);
    T_CHECK(rules_planets_curve_value(CURVE_F_MINUS, 3, false) == 2);
    T_CHECK(rules_planets_curve_value(CURVE_F_MINUS, 4, false) == 3);
}

static void test_curve_f_minus_level_below_1_returns_0(void) {
    T_CHECK(rules_planets_curve_value(CURVE_F_MINUS, 0, false) == 0);
    T_CHECK(rules_planets_curve_value(CURVE_F_MINUS, -1, false) == 0);
}

/* CURVE_F_MINUS had no reachable upgrade target when this test was first
 * written (neither tourism nor luxury carries an il field). Chunk 2
 * (docs/prd/triage/psx-structures-and-transforms.md) wires a genuine
 * CURVE_F_MINUS->CURVE_F_PLUS upgrade-remap branch into
 * rules_planets_curve_value (Food processor's real il="xenofoods" field is
 * what makes it reachable) -- so upgraded=true is no longer a no-op at the
 * curve-value level in general; it now returns CURVE_F_PLUS's own values.
 * tourism/luxury (both structurally tech_gate == NULL) are unaffected by
 * this change, since rules_planets_job_production never passes upgraded=true
 * for them regardless of this branch existing -- see
 * test_tourism_job_production_matches_curve_f_minus_regardless_of_tech_researched/
 * test_luxury_job_production_matches_curve_f_minus_regardless_of_tech_researched
 * below, both re-asserted unchanged by this chunk. */
static void test_curve_f_minus_upgraded_returns_curve_f_plus_values(void) {
    T_CHECK(rules_planets_curve_value(CURVE_F_MINUS, 1, true) == 0);
    T_CHECK(rules_planets_curve_value(CURVE_F_MINUS, 2, true) == 2);
    T_CHECK(rules_planets_curve_value(CURVE_F_MINUS, 3, true) == 3);
    T_CHECK(rules_planets_curve_value(CURVE_F_MINUS, 4, true) == 4);
    T_CHECK(rules_planets_curve_value(CURVE_F_MINUS, 1, true) ==
            rules_planets_curve_value(CURVE_F_PLUS, 1, false));
    T_CHECK(rules_planets_curve_value(CURVE_F_MINUS, 4, true) ==
            rules_planets_curve_value(CURVE_F_PLUS, 4, false));
}

/* Chunk 1 (docs/prd/triage/psx-structures-and-transforms.md): CURVE_F_PLUS/
 * CURVE_QH/CURVE_QH_PLUS, traced to hc (unstash Annexe A). No
 * structure/transform data exists yet -- these are pure curve-table
 * additions, exercised synthetically here; chunk 2 (Laboratory/Food
 * processor) is the first real, non-synthetic caller. */
static void test_curve_f_plus_levels_1_to_4_match_traced_values(void) {
    T_CHECK(rules_planets_curve_value(CURVE_F_PLUS, 1, false) == 0);
    T_CHECK(rules_planets_curve_value(CURVE_F_PLUS, 2, false) == 2);
    T_CHECK(rules_planets_curve_value(CURVE_F_PLUS, 3, false) == 3);
    T_CHECK(rules_planets_curve_value(CURVE_F_PLUS, 4, false) == 4);
}

/* CURVE_F_PLUS has no upgrade target of its own (hc, Annexe A) -- upgraded
 * must be a documented no-op, mirroring CURVE_PLUS's/CURVE_QH_PLUS's own
 * precedent. */
static void test_curve_f_plus_upgraded_is_a_no_op(void) {
    T_CHECK(rules_planets_curve_value(CURVE_F_PLUS, 1, true) == 0);
    T_CHECK(rules_planets_curve_value(CURVE_F_PLUS, 2, true) == 2);
    T_CHECK(rules_planets_curve_value(CURVE_F_PLUS, 3, true) == 3);
    T_CHECK(rules_planets_curve_value(CURVE_F_PLUS, 4, true) == 4);
}

static void test_curve_qh_levels_1_to_4_match_traced_values(void) {
    T_CHECK(rules_planets_curve_value(CURVE_QH, 1, false) == 0);
    T_CHECK(rules_planets_curve_value(CURVE_QH, 2, false) == 1);
    T_CHECK(rules_planets_curve_value(CURVE_QH, 3, false) == 3);
    T_CHECK(rules_planets_curve_value(CURVE_QH, 4, false) == 5);
}

static void test_curve_qh_plus_levels_1_to_4_match_traced_values(void) {
    T_CHECK(rules_planets_curve_value(CURVE_QH_PLUS, 1, false) == 0);
    T_CHECK(rules_planets_curve_value(CURVE_QH_PLUS, 2, false) == 2);
    T_CHECK(rules_planets_curve_value(CURVE_QH_PLUS, 3, false) == 5);
    T_CHECK(rules_planets_curve_value(CURVE_QH_PLUS, 4, false) == 8);
}

/* CURVE_QH has a real mc upgrade target (qh+) -- wired into
 * rules_planets_curve_value in this same chunk as pure curve-lookup
 * infrastructure, mirroring CURVE_MINUS->CURVE_PLUS's own precedent. This is
 * still a synthetic exercise (no Laboratory data exists yet); chunk 2 is the
 * first real, non-synthetic caller. */
static void test_curve_qh_upgraded_returns_curve_qh_plus_values(void) {
    T_CHECK(rules_planets_curve_value(CURVE_QH, 1, true) == 0);
    T_CHECK(rules_planets_curve_value(CURVE_QH, 2, true) == 2);
    T_CHECK(rules_planets_curve_value(CURVE_QH, 3, true) == 5);
    T_CHECK(rules_planets_curve_value(CURVE_QH, 4, true) == 8);
}

/* CURVE_QH_PLUS has no upgrade target of its own -- upgraded must be a
 * documented no-op, mirroring CURVE_PLUS's own precedent. */
static void test_curve_qh_plus_upgraded_is_a_no_op(void) {
    T_CHECK(rules_planets_curve_value(CURVE_QH_PLUS, 1, true) == 0);
    T_CHECK(rules_planets_curve_value(CURVE_QH_PLUS, 2, true) == 2);
    T_CHECK(rules_planets_curve_value(CURVE_QH_PLUS, 3, true) == 5);
    T_CHECK(rules_planets_curve_value(CURVE_QH_PLUS, 4, true) == 8);
}

/* CURVE_NONE has no per-level table at all (Slipgate/Infragate's genuine "no
 * production" case, this chunk's own new infrastructure) -- it must return 0
 * unconditionally, for every level 0-5 (the full range, not just the 1-4
 * traced window) and both upgraded states, mirroring RESOURCE_NONE's own
 * "real sentinel" precedent. */
static void test_curve_none_returns_0_for_every_level_and_upgraded_state(void) {
    for (int level = 0; level <= 5; level++) {
        T_CHECK(rules_planets_curve_value(CURVE_NONE, level, false) == 0);
        T_CHECK(rules_planets_curve_value(CURVE_NONE, level, true) == 0);
    }
}

/* resource_id_t: no accessor uses RESOURCE_SCIENCE/RESOURCE_ENERGY/
 * RESOURCE_JOY/RESOURCE_JOKER in this chunk (no job/structure data exists
 * yet) -- just confirm the enum compiles and every new value is genuinely
 * distinct from every other, including the pre-existing RESOURCE_LUXURY. */
static void test_new_resource_ids_are_distinct(void) {
    T_CHECK(RESOURCE_SCIENCE != RESOURCE_ENERGY);
    T_CHECK(RESOURCE_SCIENCE != RESOURCE_JOY);
    T_CHECK(RESOURCE_SCIENCE != RESOURCE_JOKER);
    T_CHECK(RESOURCE_ENERGY != RESOURCE_JOY);
    T_CHECK(RESOURCE_ENERGY != RESOURCE_JOKER);
    T_CHECK(RESOURCE_JOY != RESOURCE_JOKER);
    T_CHECK(RESOURCE_SCIENCE != RESOURCE_LUXURY);
    T_CHECK(RESOURCE_ENERGY != RESOURCE_LUXURY);
    T_CHECK(RESOURCE_JOY != RESOURCE_LUXURY);
    T_CHECK(RESOURCE_JOKER != RESOURCE_LUXURY);
}

/* Chunk 2 (docs/prd/triage/psx-structures-and-transforms.md): STRUCTURE_COUNT
 * is already 8 (the full enum) even though only Laboratory/Food processor
 * have real data yet -- the other 6 slots are zero-initialized-by-omission,
 * left for chunks 3-5 to populate and assert. */
static void test_structure_count_is_8(void) {
    T_CHECK(STRUCTURE_COUNT == 8);
}

/* Laboratory: od=15 (pico8/lua.lua:910), no rr field on its own definition,
 * needs = people + joker, produces = science, curve = qh (upgraded via
 * il="superhuman ai" to qh+), resolvable = the 6-resource "any resource" set
 * (ba=o(28,18,20,21,22,48), pico8/lua.lua:914). */
static void test_laboratory_structure_matches_traced_values(void) {
    const structure_t *s = rules_planets_structure(STRUCTURE_LABORATORY);

    T_CHECK(s != NULL);
    T_CHECK(strcmp(s->name, "Laboratory") == 0);
    T_CHECK(s->purchasable == true);
    T_CHECK(s->cost == 15);
    T_CHECK(s->required_tech == NULL);
    T_CHECK(s->needs_count == 2);
    T_CHECK(s->needs[0] == RESOURCE_PEOPLE);
    T_CHECK(s->needs[1] == RESOURCE_JOKER);
    T_CHECK(s->produces == RESOURCE_SCIENCE);
    T_CHECK(s->curve == CURVE_QH);
    T_CHECK(s->tech_gate != NULL);
    T_CHECK(strcmp(s->tech_gate, "superhuman ai") == 0);
    T_CHECK(s->default_need == RESOURCE_NONE);
    T_CHECK(s->resolvable_count == 6);
    T_CHECK(s->resolvable[0] == RESOURCE_FOOD);
    T_CHECK(s->resolvable[1] == RESOURCE_BOTS);
    T_CHECK(s->resolvable[2] == RESOURCE_ORGANICS);
    T_CHECK(s->resolvable[3] == RESOURCE_ORE);
    T_CHECK(s->resolvable[4] == RESOURCE_TECH);
    T_CHECK(s->resolvable[5] == RESOURCE_GOODS);
    T_CHECK(s->max_connections == 0);
    T_CHECK(s->crosses_other_routes == false);
}

/* Food processor: od=15 (pico8/lua.lua:1099), no rr field on its own
 * definition, needs = organics, produces = food, curve = f- (upgraded via
 * il="xenofoods" to f+). No joker need/produce, unlike Laboratory/
 * Synthesizer/Trading Hub -- resolvable_count == 0. */
static void test_food_processor_structure_matches_traced_values(void) {
    const structure_t *s = rules_planets_structure(STRUCTURE_FOOD_PROCESSOR);

    T_CHECK(s != NULL);
    T_CHECK(strcmp(s->name, "Food processor") == 0);
    T_CHECK(s->purchasable == true);
    T_CHECK(s->cost == 15);
    T_CHECK(s->required_tech == NULL);
    T_CHECK(s->needs_count == 1);
    T_CHECK(s->needs[0] == RESOURCE_ORGANICS);
    T_CHECK(s->produces == RESOURCE_FOOD);
    T_CHECK(s->curve == CURVE_F_MINUS);
    T_CHECK(s->tech_gate != NULL);
    T_CHECK(strcmp(s->tech_gate, "xenofoods") == 0);
    T_CHECK(s->default_need == RESOURCE_NONE);
    T_CHECK(s->resolvable_count == 0);
    T_CHECK(s->max_connections == 0);
    T_CHECK(s->crosses_other_routes == false);
}

/* Laboratory's rules_planets_structure_production: unresearched matches qh's
 * [0,1,3,5]; researched matches qh+'s [0,2,5,8] -- the first real,
 * non-synthetic exercise of CURVE_QH's upgrade path (chunk 1 only tested it
 * synthetically on rules_planets_curve_value directly). */
static void test_laboratory_structure_production_matches_curve_qh_unresearched(void) {
    T_CHECK(rules_planets_structure_production(STRUCTURE_LABORATORY, 1, false) == 0);
    T_CHECK(rules_planets_structure_production(STRUCTURE_LABORATORY, 2, false) == 1);
    T_CHECK(rules_planets_structure_production(STRUCTURE_LABORATORY, 3, false) == 3);
    T_CHECK(rules_planets_structure_production(STRUCTURE_LABORATORY, 4, false) == 5);
}

static void test_laboratory_structure_production_switches_to_curve_qh_plus_when_researched(void) {
    T_CHECK(rules_planets_structure_production(STRUCTURE_LABORATORY, 1, true) == 0);
    T_CHECK(rules_planets_structure_production(STRUCTURE_LABORATORY, 2, true) == 2);
    T_CHECK(rules_planets_structure_production(STRUCTURE_LABORATORY, 3, true) == 5);
    T_CHECK(rules_planets_structure_production(STRUCTURE_LABORATORY, 4, true) == 8);
}

/* Food processor's rules_planets_structure_production: unresearched matches
 * f-'s [0,1,2,3]; researched matches f+'s [0,2,3,4] -- the first real,
 * non-synthetic exercise of the NEW CURVE_F_MINUS->CURVE_F_PLUS branch added
 * in this same chunk. */
static void test_food_processor_structure_production_matches_curve_f_minus_unresearched(void) {
    T_CHECK(rules_planets_structure_production(STRUCTURE_FOOD_PROCESSOR, 1, false) == 0);
    T_CHECK(rules_planets_structure_production(STRUCTURE_FOOD_PROCESSOR, 2, false) == 1);
    T_CHECK(rules_planets_structure_production(STRUCTURE_FOOD_PROCESSOR, 3, false) == 2);
    T_CHECK(rules_planets_structure_production(STRUCTURE_FOOD_PROCESSOR, 4, false) == 3);
}

static void test_food_processor_structure_production_switches_to_curve_f_plus_when_researched(void) {
    T_CHECK(rules_planets_structure_production(STRUCTURE_FOOD_PROCESSOR, 1, true) == 0);
    T_CHECK(rules_planets_structure_production(STRUCTURE_FOOD_PROCESSOR, 2, true) == 2);
    T_CHECK(rules_planets_structure_production(STRUCTURE_FOOD_PROCESSOR, 3, true) == 3);
    T_CHECK(rules_planets_structure_production(STRUCTURE_FOOD_PROCESSOR, 4, true) == 4);
}

/* Chunk 3 (docs/prd/triage/psx-structures-and-transforms.md): Ascension
 * Gate. od=60 (pico8/lua.lua:1003), rr="ascension" (a purchase gate,
 * pico8/lua.lua:1000) -- captured as data only, not applied (mirrors
 * colonization_job_t.required_tech's own precedent; no live
 * colonization-choice/structure-purchase menu exists anywhere in this port).
 * needs = people, produces = joy, curve = f+, no il (curve-upgrade gate) --
 * f+ has no mc upgrade target of its own, so tech_gate == NULL here is not a
 * simplification, it is the traced fact. No joker need/produce, so
 * resolvable_count == 0. */
static void test_ascension_gate_structure_matches_traced_values(void) {
    const structure_t *s = rules_planets_structure(STRUCTURE_ASCENSION_GATE);

    T_CHECK(s != NULL);
    T_CHECK(strcmp(s->name, "Ascension Gate") == 0);
    T_CHECK(s->purchasable == true);
    T_CHECK(s->cost == 60);
    T_CHECK(s->required_tech != NULL);
    T_CHECK(strcmp(s->required_tech, "ascension") == 0);
    T_CHECK(s->needs_count == 1);
    T_CHECK(s->needs[0] == RESOURCE_PEOPLE);
    T_CHECK(s->produces == RESOURCE_JOY);
    T_CHECK(s->curve == CURVE_F_PLUS);
    T_CHECK(s->tech_gate == NULL);
    T_CHECK(s->default_need == RESOURCE_NONE);
    T_CHECK(s->resolvable_count == 0);
    T_CHECK(s->max_connections == 0);
    T_CHECK(s->crosses_other_routes == false);
}

/* Ascension Gate's rules_planets_structure_production: tech_gate == NULL, so
 * rules_planets_structure_production's own "upgraded = tech_gate != NULL &&
 * curve_upgrade_researched" guard forces upgraded to always be false here --
 * both researched states must therefore match f+'s own [0,2,3,4] identically
 * (mirroring tourism's/luxury's own CURVE_F_MINUS no-tech-gate precedent,
 * just for CURVE_F_PLUS now). This is a structural no-op (proven by the guard
 * itself, not a coincidence of f+ having no upgrade table -- f+ genuinely has
 * none, per curve_id_t's own doc comment, so both reasons independently hold). */
static void test_ascension_gate_structure_production_matches_curve_f_plus_unresearched(void) {
    T_CHECK(rules_planets_structure_production(STRUCTURE_ASCENSION_GATE, 1, false) == 0);
    T_CHECK(rules_planets_structure_production(STRUCTURE_ASCENSION_GATE, 2, false) == 2);
    T_CHECK(rules_planets_structure_production(STRUCTURE_ASCENSION_GATE, 3, false) == 3);
    T_CHECK(rules_planets_structure_production(STRUCTURE_ASCENSION_GATE, 4, false) == 4);
}

static void test_ascension_gate_structure_production_identical_when_researched(void) {
    T_CHECK(rules_planets_structure_production(STRUCTURE_ASCENSION_GATE, 1, true) == 0);
    T_CHECK(rules_planets_structure_production(STRUCTURE_ASCENSION_GATE, 2, true) == 2);
    T_CHECK(rules_planets_structure_production(STRUCTURE_ASCENSION_GATE, 3, true) == 3);
    T_CHECK(rules_planets_structure_production(STRUCTURE_ASCENSION_GATE, 4, true) == 4);
}

/* Protostar: non-purchasable, created only by the collapse transform action
 * (chunk 7's job) -- never bought directly. needs_count == 0 ("none" per the
 * traced table); produces == energy; curve == CURVE_MINUS, the first
 * non-colonization-job caller of that curve (every prior CURVE_MINUS user
 * was a real colonization job -- mine, nanotech, uplift, etc.). cost is
 * deliberately NOT asserted here: the PRD documents it as meaningless when
 * purchasable == false (Protostar has no od field at all in source), so this
 * test only confirms the struct is readable without crashing, never pinning
 * a particular literal. */
static void test_protostar_structure_matches_traced_values(void) {
    const structure_t *s = rules_planets_structure(STRUCTURE_PROTOSTAR);

    T_CHECK(s != NULL);
    T_CHECK(strcmp(s->name, "Protostar") == 0);
    T_CHECK(s->purchasable == false);
    T_CHECK(s->required_tech == NULL);
    T_CHECK(s->needs_count == 0);
    T_CHECK(s->produces == RESOURCE_ENERGY);
    T_CHECK(s->curve == CURVE_MINUS);
    T_CHECK(s->tech_gate == NULL);
    T_CHECK(s->default_need == RESOURCE_NONE);
    T_CHECK(s->resolvable_count == 0);
    T_CHECK(s->max_connections == 0);
    T_CHECK(s->crosses_other_routes == false);
}

/* Protostar's rules_planets_structure_production: tech_gate == NULL, so both
 * researched states are identical here too (same structural guard as
 * Ascension Gate above) -- and since CURVE_MINUS has no upgrade target
 * wired for a NULL tech_gate to reach anyway, both match CURVE_MINUS's
 * already-tested [1,2,3,3] identically. This proves
 * rules_planets_curve_value/rules_planets_structure_production are genuinely
 * caller-agnostic -- Protostar is a structure_t, not a colonization_job_t,
 * yet the exact same CURVE_MINUS values already asserted for real jobs
 * (e.g. test_mine_job_production_matches_curve_minus_unresearched) come back
 * unchanged through the structure-side call path. */
static void test_protostar_structure_production_matches_curve_minus_unresearched(void) {
    T_CHECK(rules_planets_structure_production(STRUCTURE_PROTOSTAR, 1, false) == 1);
    T_CHECK(rules_planets_structure_production(STRUCTURE_PROTOSTAR, 2, false) == 2);
    T_CHECK(rules_planets_structure_production(STRUCTURE_PROTOSTAR, 3, false) == 3);
    T_CHECK(rules_planets_structure_production(STRUCTURE_PROTOSTAR, 4, false) == 3);
}

static void test_protostar_structure_production_identical_when_researched(void) {
    T_CHECK(rules_planets_structure_production(STRUCTURE_PROTOSTAR, 1, true) == 1);
    T_CHECK(rules_planets_structure_production(STRUCTURE_PROTOSTAR, 2, true) == 2);
    T_CHECK(rules_planets_structure_production(STRUCTURE_PROTOSTAR, 3, true) == 3);
    T_CHECK(rules_planets_structure_production(STRUCTURE_PROTOSTAR, 4, true) == 3);
}

/* Chunk 4 (docs/prd/triage/psx-structures-and-transforms.md): Synthesizer.
 * od=40 (pico8/lua.lua:1040), rr="void synthesis" (pico8/lua.lua:1038) -- a
 * purchase gate, captured as data only (mirrors Ascension Gate's own
 * required_tech precedent from chunk 3). needs = none; produces = joker (a
 * dynamic produce -- captured as the joker sentinel, not resolved to a
 * concrete resource, per this PRD's Out of Scope). curve = f-, no il
 * (curve-upgrade gate). resolvable = ht=o(28,20,21,22) (pico8/lua.lua:1041)
 * = food, organics, ore, tech (4 resources) -- design-doc correction #2: the
 * design doc's own §6 prose says this set is "food/organics/bots/tech," but
 * the raw ht field never contains code 18 (bots); the real 4th resource is
 * code 21 (ore). The explicit "does NOT contain RESOURCE_BOTS" check below
 * guards against a future silent revert to the wrong design-doc value. */
static void test_synthesizer_structure_matches_traced_values(void) {
    const structure_t *s = rules_planets_structure(STRUCTURE_SYNTHESIZER);

    T_CHECK(s != NULL);
    T_CHECK(strcmp(s->name, "Synthesizer") == 0);
    T_CHECK(s->purchasable == true);
    T_CHECK(s->cost == 40);
    T_CHECK(s->required_tech != NULL);
    T_CHECK(strcmp(s->required_tech, "void synthesis") == 0);
    T_CHECK(s->needs_count == 0);
    T_CHECK(s->produces == RESOURCE_JOKER);
    T_CHECK(s->curve == CURVE_F_MINUS);
    T_CHECK(s->tech_gate == NULL);
    T_CHECK(s->default_need == RESOURCE_NONE);
    T_CHECK(s->resolvable_count == 4);
    T_CHECK(s->resolvable[0] == RESOURCE_FOOD);
    T_CHECK(s->resolvable[1] == RESOURCE_ORGANICS);
    T_CHECK(s->resolvable[2] == RESOURCE_ORE);
    T_CHECK(s->resolvable[3] == RESOURCE_TECH);
    T_CHECK(s->max_connections == 0);
    T_CHECK(s->crosses_other_routes == false);

    /* Design-doc correction #2 guard: RESOURCE_BOTS must NOT appear anywhere
     * in Synthesizer's resolvable[] set -- guards against a future silent
     * revert to the design doc's wrong "food/organics/bots/tech" prose. */
    bool has_bots = false;
    for (int i = 0; i < s->resolvable_count; i++) {
        if (s->resolvable[i] == RESOURCE_BOTS) {
            has_bots = true;
            break;
        }
    }
    T_CHECK(!has_bots);
}

/* Synthesizer's rules_planets_structure_production: tech_gate == NULL, so
 * both researched states must match f-'s own [0,1,2,3] identically -- the
 * same structural no-op the Ascension Gate/Protostar tests already
 * established in chunk 3, exercised here for CURVE_F_MINUS's base (not
 * upgraded) table instead. */
static void test_synthesizer_structure_production_matches_curve_f_minus_unresearched(void) {
    T_CHECK(rules_planets_structure_production(STRUCTURE_SYNTHESIZER, 1, false) == 0);
    T_CHECK(rules_planets_structure_production(STRUCTURE_SYNTHESIZER, 2, false) == 1);
    T_CHECK(rules_planets_structure_production(STRUCTURE_SYNTHESIZER, 3, false) == 2);
    T_CHECK(rules_planets_structure_production(STRUCTURE_SYNTHESIZER, 4, false) == 3);
}

static void test_synthesizer_structure_production_identical_when_researched(void) {
    T_CHECK(rules_planets_structure_production(STRUCTURE_SYNTHESIZER, 1, true) == 0);
    T_CHECK(rules_planets_structure_production(STRUCTURE_SYNTHESIZER, 2, true) == 1);
    T_CHECK(rules_planets_structure_production(STRUCTURE_SYNTHESIZER, 3, true) == 2);
    T_CHECK(rules_planets_structure_production(STRUCTURE_SYNTHESIZER, 4, true) == 3);
}

/* Trading Hub. od=20 (pico8/lua.lua:1061), rr="trade league"
 * (pico8/lua.lua:1059) -- a purchase gate, captured as data only. needs =
 * joker, joker; produces = luxury -- design-doc correction #1: the design
 * doc's own §6 row guesses "joy/énergie" (marked uncertain by the doc
 * itself), but the raw fd record's produce field is unambiguously w =
 * luxury (r.w=12). The explicit produces == RESOURCE_LUXURY check below
 * (plus the != RESOURCE_JOY / != RESOURCE_ENERGY checks) guards this
 * correction. curve = f-, no il. default_need = joker (dm="?", the joker
 * itself). resolvable = ba=o(28,48,18,20,21,22) (pico8/lua.lua:1062) = food,
 * goods, bots, organics, ore, tech (6 resources). */
static void test_trading_hub_structure_matches_traced_values(void) {
    const structure_t *s = rules_planets_structure(STRUCTURE_TRADING_HUB);

    T_CHECK(s != NULL);
    T_CHECK(strcmp(s->name, "Trading Hub") == 0);
    T_CHECK(s->purchasable == true);
    T_CHECK(s->cost == 20);
    T_CHECK(s->required_tech != NULL);
    T_CHECK(strcmp(s->required_tech, "trade league") == 0);
    T_CHECK(s->needs_count == 2);
    T_CHECK(s->needs[0] == RESOURCE_JOKER);
    T_CHECK(s->needs[1] == RESOURCE_JOKER);

    /* Design-doc correction #1 guard: Trading Hub must produce luxury, not
     * joy or energy (the design doc's own already-flagged-uncertain guess). */
    T_CHECK(s->produces == RESOURCE_LUXURY);
    T_CHECK(s->produces != RESOURCE_JOY);
    T_CHECK(s->produces != RESOURCE_ENERGY);

    T_CHECK(s->curve == CURVE_F_MINUS);
    T_CHECK(s->tech_gate == NULL);
    T_CHECK(s->default_need == RESOURCE_JOKER);
    T_CHECK(s->resolvable_count == 6);
    T_CHECK(s->resolvable[0] == RESOURCE_FOOD);
    T_CHECK(s->resolvable[1] == RESOURCE_GOODS);
    T_CHECK(s->resolvable[2] == RESOURCE_BOTS);
    T_CHECK(s->resolvable[3] == RESOURCE_ORGANICS);
    T_CHECK(s->resolvable[4] == RESOURCE_ORE);
    T_CHECK(s->resolvable[5] == RESOURCE_TECH);
    T_CHECK(s->max_connections == 0);
    T_CHECK(s->crosses_other_routes == false);
}

/* Trading Hub's rules_planets_structure_production: tech_gate == NULL, so
 * both researched states must match f-'s own [0,1,2,3] identically, mirroring
 * Synthesizer's own no-op above and the Ascension Gate/Protostar precedent
 * from chunk 3. */
static void test_trading_hub_structure_production_matches_curve_f_minus_unresearched(void) {
    T_CHECK(rules_planets_structure_production(STRUCTURE_TRADING_HUB, 1, false) == 0);
    T_CHECK(rules_planets_structure_production(STRUCTURE_TRADING_HUB, 2, false) == 1);
    T_CHECK(rules_planets_structure_production(STRUCTURE_TRADING_HUB, 3, false) == 2);
    T_CHECK(rules_planets_structure_production(STRUCTURE_TRADING_HUB, 4, false) == 3);
}

static void test_trading_hub_structure_production_identical_when_researched(void) {
    T_CHECK(rules_planets_structure_production(STRUCTURE_TRADING_HUB, 1, true) == 0);
    T_CHECK(rules_planets_structure_production(STRUCTURE_TRADING_HUB, 2, true) == 1);
    T_CHECK(rules_planets_structure_production(STRUCTURE_TRADING_HUB, 3, true) == 2);
    T_CHECK(rules_planets_structure_production(STRUCTURE_TRADING_HUB, 4, true) == 3);
}

/* Chunk 5 (docs/prd/triage/psx-structures-and-transforms.md): Slipgate.
 * Genuinely "no production" -- neither nr nor fr's own entity definition
 * (pico8/lua.lua:1101-1127) sets an hp= field at all, so there is no fd
 * record and thus no needs/produces/curve data for either Slipgate or
 * Infragate: needs_count == 0, produces == RESOURCE_NONE, curve ==
 * CURVE_NONE, the first real (non-synthetic) structure data to exercise
 * CURVE_NONE (added in chunk 1). od=10 (pico8/lua.lua:1103), rr="slipgates"
 * (pico8/lua.lua:1108) -- a purchase gate, captured as data only.
 * max_connections == 3 (design doc SS5, #self.oe<3) -- shared with Infragate.
 * crosses_other_routes == false: Slipgate does NOT carry mt=1, unlike
 * Infragate below -- the one field that actually distinguishes the two
 * structures (see test_slipgate_and_infragate_crosses_other_routes_differ). */
static void test_slipgate_structure_matches_traced_values(void) {
    const structure_t *s = rules_planets_structure(STRUCTURE_SLIPGATE);

    T_CHECK(s != NULL);
    T_CHECK(strcmp(s->name, "Slipgate") == 0);
    T_CHECK(s->purchasable == true);
    T_CHECK(s->cost == 10);
    T_CHECK(s->required_tech != NULL);
    T_CHECK(strcmp(s->required_tech, "slipgates") == 0);
    T_CHECK(s->needs_count == 0);
    T_CHECK(s->produces == RESOURCE_NONE);
    T_CHECK(s->curve == CURVE_NONE);
    T_CHECK(s->tech_gate == NULL);
    T_CHECK(s->default_need == RESOURCE_NONE);
    T_CHECK(s->resolvable_count == 0);
    T_CHECK(s->max_connections == 3);
    T_CHECK(s->crosses_other_routes == false);
}

/* Slipgate's rules_planets_structure_production: CURVE_NONE is a real
 * sentinel (chunk 1) -- must return 0 for every level 0-5 (the full range,
 * not just the traced 1-4 window) and both upgraded states, the first real,
 * non-synthetic exercise of CURVE_NONE's no-op (chunk 1 only tested it
 * synthetically on rules_planets_curve_value directly). */
static void test_slipgate_structure_production_returns_0_for_every_level(void) {
    for (int level = 0; level <= 5; level++) {
        T_CHECK(rules_planets_structure_production(STRUCTURE_SLIPGATE, level, false) == 0);
        T_CHECK(rules_planets_structure_production(STRUCTURE_SLIPGATE, level, true) == 0);
    }
}

/* Infragate: the same "no production" status as Slipgate above -- fr's own
 * entity definition (pico8/lua.lua:1109-1127) also sets no hp= field, so
 * needs_count == 0, produces == RESOURCE_NONE, curve == CURVE_NONE
 * identically. od=25 (pico8/lua.lua:1126), rr="infraspace"
 * (pico8/lua.lua:1125) -- a purchase gate, captured as data only.
 * max_connections == 3, same #self.oe<3 rule as Slipgate. crosses_other_routes
 * == true: Infragate's mt=1 -- a route that crosses another route is only
 * valid if the crossing structure is an Infragate, the one field that
 * actually distinguishes it from Slipgate. */
static void test_infragate_structure_matches_traced_values(void) {
    const structure_t *s = rules_planets_structure(STRUCTURE_INFRAGATE);

    T_CHECK(s != NULL);
    T_CHECK(strcmp(s->name, "Infragate") == 0);
    T_CHECK(s->purchasable == true);
    T_CHECK(s->cost == 25);
    T_CHECK(s->required_tech != NULL);
    T_CHECK(strcmp(s->required_tech, "infraspace") == 0);
    T_CHECK(s->needs_count == 0);
    T_CHECK(s->produces == RESOURCE_NONE);
    T_CHECK(s->curve == CURVE_NONE);
    T_CHECK(s->tech_gate == NULL);
    T_CHECK(s->default_need == RESOURCE_NONE);
    T_CHECK(s->resolvable_count == 0);
    T_CHECK(s->max_connections == 3);
    T_CHECK(s->crosses_other_routes == true);
}

/* Infragate's rules_planets_structure_production: same CURVE_NONE no-op
 * property as Slipgate's own test above, exercised independently here so a
 * future reader can't mistake it for a shared-fixture coincidence. */
static void test_infragate_structure_production_returns_0_for_every_level(void) {
    for (int level = 0; level <= 5; level++) {
        T_CHECK(rules_planets_structure_production(STRUCTURE_INFRAGATE, level, false) == 0);
        T_CHECK(rules_planets_structure_production(STRUCTURE_INFRAGATE, level, true) == 0);
    }
}

/* The discriminating test: crosses_other_routes is the ONE field that
 * actually distinguishes Slipgate from Infragate (every other field --
 * needs/produces/curve/max_connections -- is identical between the two, per
 * both structures' own tests above). Asserted both directions here (not just
 * a single positive check on one structure) so this genuinely proves the
 * two structures differ, not merely that one of them happens to be true. */
static void test_slipgate_and_infragate_crosses_other_routes_differ(void) {
    const structure_t *slipgate = rules_planets_structure(STRUCTURE_SLIPGATE);
    const structure_t *infragate = rules_planets_structure(STRUCTURE_INFRAGATE);

    T_CHECK(slipgate->crosses_other_routes == false);
    T_CHECK(infragate->crosses_other_routes == true);
    T_CHECK(slipgate->crosses_other_routes != infragate->crosses_other_routes);

    /* max_connections is identical between the two (both == 3) -- confirming
     * crosses_other_routes, not max_connections, is the discriminating
     * field. */
    T_CHECK(slipgate->max_connections == infragate->max_connections);
}

static void test_hiveworld_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_HIVEWORLD);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_types_count == 1);
    T_CHECK(j->planet_types[0] == PLANET_TYPE_EARTHLIKE);
    T_CHECK(j->cost == 5);
    T_CHECK(j->needs_count == 1);
    T_CHECK(j->needs[0] == RESOURCE_FOOD);
    T_CHECK(j->produces == RESOURCE_PEOPLE);
    T_CHECK(j->curve == CURVE_PLUS);
    T_CHECK(j->tech_gate == NULL);
    T_CHECK(j->default_need == RESOURCE_GOODS);
    T_CHECK(j->required_tech == NULL);
}

/* Chunk 4 (docs/prd/triage/psx-full-planets-and-tech.md): forgeworld batch
 * (goods, gadgets, uplift), the first jobs added beyond the original 5. */
static void test_goods_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_GOODS);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_types_count == 1);
    T_CHECK(j->planet_types[0] == PLANET_TYPE_FORGEWORLD);
    T_CHECK(j->cost == 10);
    T_CHECK(j->needs_count == 2);
    T_CHECK(j->needs[0] == RESOURCE_ORE);
    T_CHECK(j->needs[1] == RESOURCE_BOTS);
    T_CHECK(j->produces == RESOURCE_GOODS);
    T_CHECK(j->curve == CURVE_PLUS);
    T_CHECK(j->tech_gate == NULL);
    T_CHECK(j->default_need == RESOURCE_NONE);
    T_CHECK(j->required_tech == NULL);
}

static void test_gadgets_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_GADGETS);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_types_count == 1);
    T_CHECK(j->planet_types[0] == PLANET_TYPE_FORGEWORLD);
    T_CHECK(j->cost == 10);
    T_CHECK(j->needs_count == 2);
    T_CHECK(j->needs[0] == RESOURCE_ORE);
    T_CHECK(j->needs[1] == RESOURCE_TECH);
    T_CHECK(j->produces == RESOURCE_GOODS);
    T_CHECK(j->curve == CURVE_PLUS);
    T_CHECK(j->tech_gate == NULL);
    T_CHECK(j->default_need == RESOURCE_NONE);
    T_CHECK(j->required_tech == NULL);
}

/* uplift's required_tech = "gene rewriting" is the rr availability-gate
 * field, captured here as data only -- not applied by any function (see
 * colonization_job_t's own doc comment). */
static void test_uplift_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_UPLIFT);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_types_count == 1);
    T_CHECK(j->planet_types[0] == PLANET_TYPE_FORGEWORLD);
    T_CHECK(j->cost == 20);
    T_CHECK(j->needs_count == 1);
    T_CHECK(j->needs[0] == RESOURCE_ORGANICS);
    T_CHECK(j->produces == RESOURCE_PEOPLE);
    T_CHECK(j->curve == CURVE_MINUS);
    T_CHECK(j->tech_gate == NULL);
    T_CHECK(j->default_need == RESOURCE_NONE);
    T_CHECK(j->required_tech != NULL);
    T_CHECK(strcmp(j->required_tech, "gene rewriting") == 0);
}

/* Chunk 5 (docs/prd/triage/psx-full-planets-and-tech.md): ocean batch.
 * breed(ocean) has no tech_gate and no default_need, matching bots'/goods'/
 * gadgets' own CURVE_PLUS precedent. */
static void test_breed_ocean_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_BREED_OCEAN);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_types_count == 1);
    T_CHECK(j->planet_types[0] == PLANET_TYPE_OCEAN);
    T_CHECK(j->cost == 5);
    T_CHECK(j->needs_count == 1);
    T_CHECK(j->needs[0] == RESOURCE_PEOPLE);
    T_CHECK(j->produces == RESOURCE_ORGANICS);
    T_CHECK(j->curve == CURVE_PLUS);
    T_CHECK(j->tech_gate == NULL);
    T_CHECK(j->default_need == RESOURCE_NONE);
    T_CHECK(j->required_tech == NULL);
}

/* colony(ocean)'s tech_gate (il) happens to be the same string
 * ("gene rewriting") as uplift's required_tech (rr) -- a coincidence of the
 * raw fd source data, not a shared field: colony(ocean) has no required_tech
 * of its own, and uplift has no tech_gate of its own. */
static void test_colony_ocean_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_COLONY_OCEAN);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_types_count == 1);
    T_CHECK(j->planet_types[0] == PLANET_TYPE_OCEAN);
    T_CHECK(j->cost == 10);
    T_CHECK(j->needs_count == 2);
    T_CHECK(j->needs[0] == RESOURCE_ORE);
    T_CHECK(j->needs[1] == RESOURCE_FOOD);
    T_CHECK(j->produces == RESOURCE_PEOPLE);
    T_CHECK(j->curve == CURVE_MINUS);
    T_CHECK(j->tech_gate != NULL);
    T_CHECK(strcmp(j->tech_gate, "gene rewriting") == 0);
    T_CHECK(j->default_need == RESOURCE_GOODS);
    T_CHECK(j->required_tech == NULL);
}

static void test_algae_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_ALGAE);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_types_count == 1);
    T_CHECK(j->planet_types[0] == PLANET_TYPE_OCEAN);
    T_CHECK(j->cost == 8);
    T_CHECK(j->needs_count == 1);
    T_CHECK(j->needs[0] == RESOURCE_BOTS);
    T_CHECK(j->produces == RESOURCE_FOOD);
    T_CHECK(j->curve == CURVE_MINUS);
    T_CHECK(j->tech_gate != NULL);
    T_CHECK(strcmp(j->tech_gate, "xenofoods") == 0);
    T_CHECK(j->default_need == RESOURCE_ORGANICS);
    T_CHECK(j->required_tech == NULL);
}

/* Chunk 6 (docs/prd/triage/psx-full-planets-and-tech.md): remnant batch
 * (strip, dismantle, scavenge). Deliberately simple: no tech_gate (il), no
 * required_tech (rr), no default_need (dm) -- the PRD's own "simpler chunk
 * placed here to keep pace varied" framing. */
static void test_strip_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_STRIP);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_types_count == 1);
    T_CHECK(j->planet_types[0] == PLANET_TYPE_REMNANT);
    T_CHECK(j->cost == 5);
    T_CHECK(j->needs_count == 2);
    T_CHECK(j->needs[0] == RESOURCE_PEOPLE);
    T_CHECK(j->needs[1] == RESOURCE_FOOD);
    T_CHECK(j->produces == RESOURCE_ORE);
    T_CHECK(j->curve == CURVE_MINUS);
    T_CHECK(j->tech_gate == NULL);
    T_CHECK(j->default_need == RESOURCE_NONE);
    T_CHECK(j->required_tech == NULL);
}

static void test_dismantle_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_DISMANTLE);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_types_count == 1);
    T_CHECK(j->planet_types[0] == PLANET_TYPE_REMNANT);
    T_CHECK(j->cost == 5);
    T_CHECK(j->needs_count == 2);
    T_CHECK(j->needs[0] == RESOURCE_PEOPLE);
    T_CHECK(j->needs[1] == RESOURCE_FOOD);
    T_CHECK(j->produces == RESOURCE_TECH);
    T_CHECK(j->curve == CURVE_MINUS);
    T_CHECK(j->tech_gate == NULL);
    T_CHECK(j->default_need == RESOURCE_NONE);
    T_CHECK(j->required_tech == NULL);
}

static void test_scavenge_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_SCAVENGE);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_types_count == 1);
    T_CHECK(j->planet_types[0] == PLANET_TYPE_REMNANT);
    T_CHECK(j->cost == 5);
    T_CHECK(j->needs_count == 2);
    T_CHECK(j->needs[0] == RESOURCE_PEOPLE);
    T_CHECK(j->needs[1] == RESOURCE_FOOD);
    T_CHECK(j->produces == RESOURCE_BOTS);
    T_CHECK(j->curve == CURVE_MINUS);
    T_CHECK(j->tech_gate == NULL);
    T_CHECK(j->default_need == RESOURCE_NONE);
    T_CHECK(j->required_tech == NULL);
}

/* Chunk 7 (docs/prd/triage/psx-full-planets-and-tech.md): xeno + jungle
 * batch. colony(xeno) is tech-gated exactly like colony(ocean) from chunk 5
 * -- same tech_gate string ("gene rewriting"), same default_need
 * (RESOURCE_GOODS). */
static void test_colony_xeno_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_COLONY_XENO);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_types_count == 1);
    T_CHECK(j->planet_types[0] == PLANET_TYPE_XENO);
    T_CHECK(j->cost == 10);
    T_CHECK(j->needs_count == 2);
    T_CHECK(j->needs[0] == RESOURCE_TECH);
    T_CHECK(j->needs[1] == RESOURCE_FOOD);
    T_CHECK(j->produces == RESOURCE_PEOPLE);
    T_CHECK(j->curve == CURVE_MINUS);
    T_CHECK(j->tech_gate != NULL);
    T_CHECK(strcmp(j->tech_gate, "gene rewriting") == 0);
    T_CHECK(j->default_need == RESOURCE_GOODS);
    T_CHECK(j->required_tech == NULL);
}

static void test_breed_xeno_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_BREED_XENO);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_types_count == 1);
    T_CHECK(j->planet_types[0] == PLANET_TYPE_XENO);
    T_CHECK(j->cost == 5);
    T_CHECK(j->needs_count == 1);
    T_CHECK(j->needs[0] == RESOURCE_BOTS);
    T_CHECK(j->produces == RESOURCE_ORGANICS);
    T_CHECK(j->curve == CURVE_MINUS);
    T_CHECK(j->tech_gate == NULL);
    T_CHECK(j->default_need == RESOURCE_NONE);
    T_CHECK(j->required_tech == NULL);
}

static void test_hunt_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_HUNT);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_types_count == 1);
    T_CHECK(j->planet_types[0] == PLANET_TYPE_JUNGLE);
    T_CHECK(j->cost == 5);
    T_CHECK(j->needs_count == 1);
    T_CHECK(j->needs[0] == RESOURCE_PEOPLE);
    T_CHECK(j->produces == RESOURCE_ORGANICS);
    T_CHECK(j->curve == CURVE_MINUS);
    T_CHECK(j->tech_gate == NULL);
    T_CHECK(j->default_need == RESOURCE_NONE);
    T_CHECK(j->required_tech == NULL);
}

/* tourism: the first real job to use CURVE_F_MINUS (added in chunk 1, only
 * exercised by a synthetic unit test on rules_planets_curve_value directly
 * until now). */
static void test_tourism_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_TOURISM);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_types_count == 1);
    T_CHECK(j->planet_types[0] == PLANET_TYPE_JUNGLE);
    T_CHECK(j->cost == 10);
    T_CHECK(j->needs_count == 2);
    T_CHECK(j->needs[0] == RESOURCE_PEOPLE);
    T_CHECK(j->needs[1] == RESOURCE_GOODS);
    T_CHECK(j->produces == RESOURCE_LUXURY);
    T_CHECK(j->curve == CURVE_F_MINUS);
    T_CHECK(j->tech_gate == NULL);
    T_CHECK(j->default_need == RESOURCE_NONE);
    T_CHECK(j->required_tech == NULL);
}

/* luxury: the second real job to use CURVE_F_MINUS (after tourism, chunk 7)
 * -- deliberately chosen to prove the curve lookup is genuinely job-agnostic
 * across independent jobs, not just correct for one specific job. Same
 * no-gate shape as tourism: no tech_gate, no default_need, no
 * required_tech. */
static void test_luxury_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_LUXURY);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_types_count == 1);
    T_CHECK(j->planet_types[0] == PLANET_TYPE_EARTHLIKE);
    T_CHECK(j->cost == 10);
    T_CHECK(j->needs_count == 2);
    T_CHECK(j->needs[0] == RESOURCE_TECH);
    T_CHECK(j->needs[1] == RESOURCE_GOODS);
    T_CHECK(j->produces == RESOURCE_LUXURY);
    T_CHECK(j->curve == CURVE_F_MINUS);
    T_CHECK(j->tech_gate == NULL);
    T_CHECK(j->default_need == RESOURCE_NONE);
    T_CHECK(j->required_tech == NULL);
}

/* Chunk 9 (docs/prd/triage/psx-full-planets-and-tech.md): barren/iceball/
 * gas-giant batch, the first chunk to genuinely exercise
 * planet_types_count == 2. habitat is valid on both barren and iceball. */
static void test_habitat_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_HABITAT);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_types_count == 2);
    T_CHECK(j->planet_types[0] == PLANET_TYPE_BARREN);
    T_CHECK(j->planet_types[1] == PLANET_TYPE_ICEBALL);
    T_CHECK(j->cost == 12);
    T_CHECK(j->needs_count == 2);
    T_CHECK(j->needs[0] == RESOURCE_ORGANICS);
    T_CHECK(j->needs[1] == RESOURCE_FOOD);
    T_CHECK(j->produces == RESOURCE_PEOPLE);
    T_CHECK(j->curve == CURVE_MINUS);
    T_CHECK(j->tech_gate == NULL);
    T_CHECK(j->default_need == RESOURCE_GOODS);
    T_CHECK(j->required_tech != NULL);
    T_CHECK(strcmp(j->required_tech, "geneseeds") == 0);
}

static void test_genefarm_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_GENEFARM);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_types_count == 2);
    T_CHECK(j->planet_types[0] == PLANET_TYPE_BARREN);
    T_CHECK(j->planet_types[1] == PLANET_TYPE_ICEBALL);
    T_CHECK(j->cost == 12);
    T_CHECK(j->needs_count == 2);
    T_CHECK(j->needs[0] == RESOURCE_ORGANICS);
    T_CHECK(j->needs[1] == RESOURCE_BOTS);
    T_CHECK(j->produces == RESOURCE_FOOD);
    T_CHECK(j->curve == CURVE_MINUS);
    T_CHECK(j->tech_gate == NULL);
    T_CHECK(j->default_need == RESOURCE_NONE);
    T_CHECK(j->required_tech != NULL);
    T_CHECK(strcmp(j->required_tech, "geneseeds") == 0);
}

/* excavate_mineral: single-type (mineral only), CURVE_PLUS -- see the
 * dedicated test_excavate_mineral_and_iceball_gasgiant_are_genuinely_distinct
 * test below for the direct side-by-side comparison against
 * excavate_iceball_gasgiant that this chunk's PRD explicitly calls for. */
static void test_excavate_mineral_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_EXCAVATE_MINERAL);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_types_count == 1);
    T_CHECK(j->planet_types[0] == PLANET_TYPE_MINERAL);
    T_CHECK(j->cost == 12);
    T_CHECK(j->needs_count == 1);
    T_CHECK(j->needs[0] == RESOURCE_BOTS);
    T_CHECK(j->produces == RESOURCE_ORE);
    T_CHECK(j->curve == CURVE_PLUS);
    T_CHECK(j->tech_gate == NULL);
    T_CHECK(j->default_need == RESOURCE_NONE);
    T_CHECK(j->required_tech != NULL);
    T_CHECK(strcmp(j->required_tech, "drillbots") == 0);
}

static void test_excavate_iceball_gasgiant_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_EXCAVATE_ICEBALL_GASGIANT);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_types_count == 2);
    T_CHECK(j->planet_types[0] == PLANET_TYPE_ICEBALL);
    T_CHECK(j->planet_types[1] == PLANET_TYPE_GASGIANT);
    T_CHECK(j->cost == 12);
    T_CHECK(j->needs_count == 1);
    T_CHECK(j->needs[0] == RESOURCE_BOTS);
    T_CHECK(j->produces == RESOURCE_ORE);
    T_CHECK(j->curve == CURVE_MINUS);
    T_CHECK(j->tech_gate == NULL);
    T_CHECK(j->default_need == RESOURCE_NONE);
    T_CHECK(j->required_tech != NULL);
    T_CHECK(strcmp(j->required_tech, "drillbots") == 0);
}

/* The PRD's own explicitly-flagged risk: excavate_mineral and
 * excavate_iceball_gasgiant share identical cost/needs/produces
 * (12/{bots}/ore) -- this test proves they remain two genuinely distinct
 * colonization_job_id_t entries, not accidentally merged or copy-paste
 * confused, by checking the two fields that actually differ (curve,
 * planet_types/available_on) directly against each other. */
static void test_excavate_mineral_and_iceball_gasgiant_are_genuinely_distinct(void) {
    T_CHECK(JOB_EXCAVATE_MINERAL != JOB_EXCAVATE_ICEBALL_GASGIANT);

    const colonization_job_t *mineral = rules_planets_job(JOB_EXCAVATE_MINERAL);
    const colonization_job_t *iceball_gasgiant = rules_planets_job(JOB_EXCAVATE_ICEBALL_GASGIANT);

    /* Same cost/needs/produces -- the coincidence this test guards against. */
    T_CHECK(mineral->cost == iceball_gasgiant->cost);
    T_CHECK(mineral->needs_count == iceball_gasgiant->needs_count);
    T_CHECK(mineral->needs[0] == iceball_gasgiant->needs[0]);
    T_CHECK(mineral->produces == iceball_gasgiant->produces);

    /* Genuinely different curves. */
    T_CHECK(mineral->curve == CURVE_PLUS);
    T_CHECK(iceball_gasgiant->curve == CURVE_MINUS);
    T_CHECK(mineral->curve != iceball_gasgiant->curve);

    /* Genuinely different availability: mineral is mineral-only; iceball's
     * gas-giant variant is iceball+gasgiant, never mineral. */
    T_CHECK(rules_planets_job_available_on(JOB_EXCAVATE_MINERAL, PLANET_TYPE_MINERAL) == true);
    T_CHECK(rules_planets_job_available_on(JOB_EXCAVATE_MINERAL, PLANET_TYPE_ICEBALL) == false);
    T_CHECK(rules_planets_job_available_on(JOB_EXCAVATE_MINERAL, PLANET_TYPE_GASGIANT) == false);

    T_CHECK(rules_planets_job_available_on(JOB_EXCAVATE_ICEBALL_GASGIANT, PLANET_TYPE_ICEBALL) == true);
    T_CHECK(rules_planets_job_available_on(JOB_EXCAVATE_ICEBALL_GASGIANT, PLANET_TYPE_GASGIANT) == true);
    T_CHECK(rules_planets_job_available_on(JOB_EXCAVATE_ICEBALL_GASGIANT, PLANET_TYPE_MINERAL) == false);
}

static void test_farm_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_FARM);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_types_count == 2);
    T_CHECK(j->planet_types[0] == PLANET_TYPE_XENO);
    T_CHECK(j->planet_types[1] == PLANET_TYPE_JUNGLE);
    T_CHECK(j->cost == 10);
    T_CHECK(j->needs_count == 1);
    T_CHECK(j->needs[0] == RESOURCE_BOTS);
    T_CHECK(j->produces == RESOURCE_FOOD);
    T_CHECK(j->curve == CURVE_MINUS);
    T_CHECK(j->tech_gate == NULL);
    T_CHECK(j->default_need == RESOURCE_ORGANICS);
    T_CHECK(j->required_tech != NULL);
    T_CHECK(strcmp(j->required_tech, "xenofoods") == 0);
}

/* Chunk 9's central new property: rules_planets_job_available_on returns
 * true for BOTH real types of each of this chunk's 4 multi-type jobs, and
 * false for every other type -- walked generically over PLANET_TYPE_COUNT,
 * mirroring test_job_available_on_matches_exactly_its_one_real_type's own
 * style but extended to a 2-type expectation, since that existing table's
 * shape assumes exactly one real type per case. A separate loop (rather than
 * widening the existing table's row shape) keeps both tests simple and keeps
 * this chunk's new property isolated and easy to review on its own. */
static void test_multi_type_job_available_on_matches_exactly_its_two_real_types(void) {
    static const struct {
        colonization_job_id_t job;
        planet_type_id_t type_a;
        planet_type_id_t type_b;
    } cases[] = {
        { JOB_HABITAT, PLANET_TYPE_BARREN, PLANET_TYPE_ICEBALL },
        { JOB_GENEFARM, PLANET_TYPE_BARREN, PLANET_TYPE_ICEBALL },
        { JOB_EXCAVATE_ICEBALL_GASGIANT, PLANET_TYPE_ICEBALL, PLANET_TYPE_GASGIANT },
        { JOB_FARM, PLANET_TYPE_XENO, PLANET_TYPE_JUNGLE },
    };

    for (size_t c = 0; c < sizeof(cases) / sizeof(cases[0]); c++) {
        for (int t = 0; t < PLANET_TYPE_COUNT; t++) {
            bool expected = ((planet_type_id_t)t == cases[c].type_a) || ((planet_type_id_t)t == cases[c].type_b);
            T_CHECK(rules_planets_job_available_on(cases[c].job, (planet_type_id_t)t) == expected);
        }
    }
}

static void test_mine_job_cost_unaffected_by_tech_researched(void) {
    T_CHECK(rules_planets_job_cost(JOB_MINE, false, false) == 5);
    T_CHECK(rules_planets_job_cost(JOB_MINE, true, false) == 5);
}

static void test_agroworld_job_cost_base_case(void) {
    T_CHECK(rules_planets_job_cost(JOB_AGROWORLD, false, false) == 5);
}

static void test_agroworld_job_cost_tech_researched_only(void) {
    /* 5 * 1.5 = 7.5, floored -> 7. */
    T_CHECK(rules_planets_job_cost(JOB_AGROWORLD, true, false) == 7);
}

static void test_agroworld_job_cost_nanomaterials_researched_only(void) {
    /* 5 * 0.85 = 4.25, floored -> 4. */
    T_CHECK(rules_planets_job_cost(JOB_AGROWORLD, false, true) == 4);
}

static void test_agroworld_job_cost_both_flags_true_floors_once_at_end(void) {
    /* 5 * 1.5 * 0.85 = 6.375, floored ONCE at the end -> 6. This is the
     * load-bearing case: flooring after each step instead would give
     * floor(floor(5*1.5)*0.85) = floor(7*0.85) = floor(5.95) = 5, a
     * DIFFERENT (wrong) result. This module must compute the whole chain in
     * fixed-point and truncate only once at the very end, per hp:od(). */
    T_CHECK(rules_planets_job_cost(JOB_AGROWORLD, true, true) == 6);
}

static void test_mine_job_production_matches_curve_minus_unresearched(void) {
    T_CHECK(rules_planets_job_production(JOB_MINE, 0, false) == 0);
    T_CHECK(rules_planets_job_production(JOB_MINE, 1, false) == 1);
    T_CHECK(rules_planets_job_production(JOB_MINE, 2, false) == 2);
    T_CHECK(rules_planets_job_production(JOB_MINE, 3, false) == 3);
    T_CHECK(rules_planets_job_production(JOB_MINE, 4, false) == 3);
}

static void test_nanotech_job_production_matches_curve_minus_unresearched(void) {
    T_CHECK(rules_planets_job_production(JOB_NANOTECH, 0, false) == 0);
    T_CHECK(rules_planets_job_production(JOB_NANOTECH, 1, false) == 1);
    T_CHECK(rules_planets_job_production(JOB_NANOTECH, 2, false) == 2);
    T_CHECK(rules_planets_job_production(JOB_NANOTECH, 3, false) == 3);
    T_CHECK(rules_planets_job_production(JOB_NANOTECH, 4, false) == 3);
}

static void test_bots_job_production_matches_curve_plus_unresearched(void) {
    T_CHECK(rules_planets_job_production(JOB_BOTS, 0, false) == 0);
    T_CHECK(rules_planets_job_production(JOB_BOTS, 1, false) == 2);
    T_CHECK(rules_planets_job_production(JOB_BOTS, 2, false) == 3);
    T_CHECK(rules_planets_job_production(JOB_BOTS, 3, false) == 4);
    T_CHECK(rules_planets_job_production(JOB_BOTS, 4, false) == 4);
}

static void test_agroworld_job_production_matches_curve_minus_unresearched(void) {
    T_CHECK(rules_planets_job_production(JOB_AGROWORLD, 0, false) == 0);
    T_CHECK(rules_planets_job_production(JOB_AGROWORLD, 1, false) == 1);
    T_CHECK(rules_planets_job_production(JOB_AGROWORLD, 2, false) == 2);
    T_CHECK(rules_planets_job_production(JOB_AGROWORLD, 3, false) == 3);
    T_CHECK(rules_planets_job_production(JOB_AGROWORLD, 4, false) == 3);
}

static void test_hiveworld_job_production_matches_curve_plus_unresearched(void) {
    T_CHECK(rules_planets_job_production(JOB_HIVEWORLD, 0, false) == 0);
    T_CHECK(rules_planets_job_production(JOB_HIVEWORLD, 1, false) == 2);
    T_CHECK(rules_planets_job_production(JOB_HIVEWORLD, 2, false) == 3);
    T_CHECK(rules_planets_job_production(JOB_HIVEWORLD, 3, false) == 4);
    T_CHECK(rules_planets_job_production(JOB_HIVEWORLD, 4, false) == 4);
}

static void test_agroworld_job_production_switches_to_curve_plus_when_tech_researched(void) {
    /* The anchor property this chunk exists to prove: agroworld's tech_gate
     * ("xenofoods") plus job_tech_researched=true must flip its curve from
     * CURVE_MINUS ({1,2,3,3}) to CURVE_PLUS ({2,3,4,4}) -- the exact same
     * condition that raises its cost in rules_planets_job_cost, proving the
     * shared cost/curve gating trigger is actually wired here, not just
     * documented. */
    T_CHECK(rules_planets_job_production(JOB_AGROWORLD, 1, true) == 2);
    T_CHECK(rules_planets_job_production(JOB_AGROWORLD, 2, true) == 3);
    T_CHECK(rules_planets_job_production(JOB_AGROWORLD, 3, true) == 4);
    T_CHECK(rules_planets_job_production(JOB_AGROWORLD, 4, true) == 4);
}

/* Closed-loop structural regression test, from docs/prd/psx-rules-planets.md
 * (that PRD's own chunk 5). Walks the g_jobs table generically (via the
 * public rules_planets_job() accessor, not hardcoded pairwise "mine needs
 * PEOPLE, hiveworld produces PEOPLE" checks) and asserts the property that
 * PRD's own 5-job selection was deliberately built to have: every resource
 * any of those 5 jobs needs is produced by another of those same 5 jobs, and
 * every resource any of them produces is needed by another of them.
 *
 * This is a property of that original 5-job subset by deliberate design
 * choice, NOT a property of the full ~24-job source roster (see
 * docs/prd/triage/psx-full-planets-and-tech.md's Further Notes: the full
 * roster genuinely isn't closed -- luxury alone is a pure sink). Chunk 4 of
 * that PRD is this module's first chunk to grow JOB_COUNT past 5 (to 8, via
 * goods/gadgets/uplift below), and neither of those 3 new jobs is part of
 * this closed loop yet (uplift needs organics, produced by no job before a
 * later chunk's breed(ocean); goods/gadgets produce goods, needed by no job
 * before a later chunk's luxury/tourism) -- so these two loops are pinned to
 * RULES_PLANETS_ORIGINAL_CLOSED_LOOP_JOB_COUNT, the original 5-job count,
 * rather than JOB_COUNT, to keep this test's scope exactly what it always
 * was instead of silently widening as the table grows. Chunk 10 of that PRD
 * adds a separately-named, explicit consolidation of this same scoping
 * confirmation; this comment documents it starting here, where the scope
 * question first becomes real. */
#define RULES_PLANETS_ORIGINAL_CLOSED_LOOP_JOB_COUNT 5

static void test_every_need_is_produced_by_another_job(void) {
    for (int consumer = 0; consumer < RULES_PLANETS_ORIGINAL_CLOSED_LOOP_JOB_COUNT; consumer++) {
        const colonization_job_t *c = rules_planets_job((colonization_job_id_t)consumer);

        for (int n = 0; n < c->needs_count; n++) {
            resource_id_t needed = c->needs[n];
            bool produced_elsewhere = false;

            for (int producer = 0; producer < RULES_PLANETS_ORIGINAL_CLOSED_LOOP_JOB_COUNT; producer++) {
                if (producer == consumer) {
                    continue;
                }
                const colonization_job_t *p = rules_planets_job((colonization_job_id_t)producer);
                if (p->produces == needed) {
                    produced_elsewhere = true;
                    break;
                }
            }

            T_CHECK(produced_elsewhere);
        }
    }
}

static void test_every_produce_is_needed_by_another_job(void) {
    for (int producer = 0; producer < RULES_PLANETS_ORIGINAL_CLOSED_LOOP_JOB_COUNT; producer++) {
        const colonization_job_t *p = rules_planets_job((colonization_job_id_t)producer);
        resource_id_t produced = p->produces;
        bool needed_elsewhere = false;

        for (int consumer = 0; consumer < RULES_PLANETS_ORIGINAL_CLOSED_LOOP_JOB_COUNT; consumer++) {
            if (consumer == producer) {
                continue;
            }
            const colonization_job_t *c = rules_planets_job((colonization_job_id_t)consumer);
            for (int n = 0; n < c->needs_count; n++) {
                if (c->needs[n] == produced) {
                    needed_elsewhere = true;
                    break;
                }
            }
            if (needed_elsewhere) {
                break;
            }
        }

        T_CHECK(needed_elsewhere);
    }
}

/* Chunk 4: cost/production exercised for all 3 new jobs at levels 1-4 with
 * both booleans, confirming rules_planets_job_cost/_production's existing
 * bodies genuinely cover new jobs with zero code change -- neither function
 * was touched by this chunk, only g_jobs grew. None of the 3 has a
 * tech_gate, so job_tech_researched never changes cost or curve for them
 * (matching mine's own existing precedent above); nanomaterials_researched
 * still applies its unconditional 0.85 multiplier. */
static void test_goods_job_cost_unaffected_by_tech_researched(void) {
    T_CHECK(rules_planets_job_cost(JOB_GOODS, false, false) == 10);
    T_CHECK(rules_planets_job_cost(JOB_GOODS, true, false) == 10);
}

static void test_goods_job_cost_nanomaterials_researched(void) {
    /* 10 * 0.85 = 8.5, floored -> 8. */
    T_CHECK(rules_planets_job_cost(JOB_GOODS, false, true) == 8);
    T_CHECK(rules_planets_job_cost(JOB_GOODS, true, true) == 8);
}

static void test_goods_job_production_matches_curve_plus(void) {
    for (int researched = 0; researched <= 1; researched++) {
        T_CHECK(rules_planets_job_production(JOB_GOODS, 0, researched) == 0);
        T_CHECK(rules_planets_job_production(JOB_GOODS, 1, researched) == 2);
        T_CHECK(rules_planets_job_production(JOB_GOODS, 2, researched) == 3);
        T_CHECK(rules_planets_job_production(JOB_GOODS, 3, researched) == 4);
        T_CHECK(rules_planets_job_production(JOB_GOODS, 4, researched) == 4);
    }
}

static void test_gadgets_job_cost_unaffected_by_tech_researched(void) {
    T_CHECK(rules_planets_job_cost(JOB_GADGETS, false, false) == 10);
    T_CHECK(rules_planets_job_cost(JOB_GADGETS, true, false) == 10);
}

static void test_gadgets_job_cost_nanomaterials_researched(void) {
    /* 10 * 0.85 = 8.5, floored -> 8. */
    T_CHECK(rules_planets_job_cost(JOB_GADGETS, false, true) == 8);
    T_CHECK(rules_planets_job_cost(JOB_GADGETS, true, true) == 8);
}

static void test_gadgets_job_production_matches_curve_plus(void) {
    for (int researched = 0; researched <= 1; researched++) {
        T_CHECK(rules_planets_job_production(JOB_GADGETS, 0, researched) == 0);
        T_CHECK(rules_planets_job_production(JOB_GADGETS, 1, researched) == 2);
        T_CHECK(rules_planets_job_production(JOB_GADGETS, 2, researched) == 3);
        T_CHECK(rules_planets_job_production(JOB_GADGETS, 3, researched) == 4);
        T_CHECK(rules_planets_job_production(JOB_GADGETS, 4, researched) == 4);
    }
}

static void test_uplift_job_cost_unaffected_by_tech_researched(void) {
    T_CHECK(rules_planets_job_cost(JOB_UPLIFT, false, false) == 20);
    T_CHECK(rules_planets_job_cost(JOB_UPLIFT, true, false) == 20);
}

static void test_uplift_job_cost_nanomaterials_researched(void) {
    /* 20 * 0.85 would be 17.0 exactly in real arithmetic, but
     * RULES_PLANETS_NANOMATERIALS_MULTIPLIER is (fixed_t)(0.85 * FX_ONE),
     * which truncates 0.85's Q16.16 representation slightly below 0.85
     * (55705/65536 == 0.8499908447265625, not 0.85 exactly) -- so
     * 20 * 0.8499908... == 16.999816..., floored to 16, not 17. Confirmed by
     * hand-computing the exact fixed_mul intermediate, not assumed. */
    T_CHECK(rules_planets_job_cost(JOB_UPLIFT, false, true) == 16);
    T_CHECK(rules_planets_job_cost(JOB_UPLIFT, true, true) == 16);
}

static void test_uplift_job_production_matches_curve_minus(void) {
    for (int researched = 0; researched <= 1; researched++) {
        T_CHECK(rules_planets_job_production(JOB_UPLIFT, 0, researched) == 0);
        T_CHECK(rules_planets_job_production(JOB_UPLIFT, 1, researched) == 1);
        T_CHECK(rules_planets_job_production(JOB_UPLIFT, 2, researched) == 2);
        T_CHECK(rules_planets_job_production(JOB_UPLIFT, 3, researched) == 3);
        T_CHECK(rules_planets_job_production(JOB_UPLIFT, 4, researched) == 3);
    }
}

/* Chunk 5 (docs/prd/triage/psx-full-planets-and-tech.md): ocean batch.
 * colony(ocean) and algae are the second and third real tech_gate cases in
 * this module (after agroworld) -- exercised with the exact same
 * cost-surcharge/curve-upgrade test shape agroworld's own tests above use,
 * proving rules_planets_job_cost/_production's existing, unmodified bodies
 * genuinely generalize to new tech-gated data, not just the original
 * agroworld case. */
static void test_colony_ocean_job_cost_base_case(void) {
    T_CHECK(rules_planets_job_cost(JOB_COLONY_OCEAN, false, false) == 10);
}

static void test_colony_ocean_job_cost_tech_researched_only(void) {
    /* 10 * 1.5 = 15, exact -- no flooring surprise here (unlike agroworld's
     * 5 * 1.5 = 7.5 case), but still routed through the same fixed-point
     * chain and floored once at the end. */
    T_CHECK(rules_planets_job_cost(JOB_COLONY_OCEAN, true, false) == 15);
}

static void test_colony_ocean_job_cost_nanomaterials_researched_only(void) {
    /* 10 * 0.85 = 8.5, floored -> 8. */
    T_CHECK(rules_planets_job_cost(JOB_COLONY_OCEAN, false, true) == 8);
}

static void test_colony_ocean_job_cost_both_flags_true_floors_once_at_end(void) {
    /* 10 * 1.5 * 0.85 = 12.75, floored ONCE at the end -> 12. */
    T_CHECK(rules_planets_job_cost(JOB_COLONY_OCEAN, true, true) == 12);
}

static void test_colony_ocean_job_production_matches_curve_minus_unresearched(void) {
    T_CHECK(rules_planets_job_production(JOB_COLONY_OCEAN, 0, false) == 0);
    T_CHECK(rules_planets_job_production(JOB_COLONY_OCEAN, 1, false) == 1);
    T_CHECK(rules_planets_job_production(JOB_COLONY_OCEAN, 2, false) == 2);
    T_CHECK(rules_planets_job_production(JOB_COLONY_OCEAN, 3, false) == 3);
    T_CHECK(rules_planets_job_production(JOB_COLONY_OCEAN, 4, false) == 3);
}

static void test_colony_ocean_job_production_switches_to_curve_plus_when_tech_researched(void) {
    /* Same anchor property agroworld's own equivalent test above proves,
     * now for colony(ocean)'s tech_gate ("gene rewriting"): CURVE_MINUS
     * ({1,2,3,3}) flips to CURVE_PLUS ({2,3,4,4}). */
    T_CHECK(rules_planets_job_production(JOB_COLONY_OCEAN, 1, true) == 2);
    T_CHECK(rules_planets_job_production(JOB_COLONY_OCEAN, 2, true) == 3);
    T_CHECK(rules_planets_job_production(JOB_COLONY_OCEAN, 3, true) == 4);
    T_CHECK(rules_planets_job_production(JOB_COLONY_OCEAN, 4, true) == 4);
}

static void test_algae_job_cost_base_case(void) {
    T_CHECK(rules_planets_job_cost(JOB_ALGAE, false, false) == 8);
}

static void test_algae_job_cost_tech_researched_only(void) {
    /* 8 * 1.5 = 12, exact. */
    T_CHECK(rules_planets_job_cost(JOB_ALGAE, true, false) == 12);
}

static void test_algae_job_cost_nanomaterials_researched_only(void) {
    /* 8 * 0.85 = 6.8, floored -> 6. */
    T_CHECK(rules_planets_job_cost(JOB_ALGAE, false, true) == 6);
}

static void test_algae_job_cost_both_flags_true_floors_once_at_end(void) {
    /* 8 * 1.5 * 0.85 = 10.2, floored ONCE at the end -> 10. */
    T_CHECK(rules_planets_job_cost(JOB_ALGAE, true, true) == 10);
}

static void test_algae_job_production_matches_curve_minus_unresearched(void) {
    T_CHECK(rules_planets_job_production(JOB_ALGAE, 0, false) == 0);
    T_CHECK(rules_planets_job_production(JOB_ALGAE, 1, false) == 1);
    T_CHECK(rules_planets_job_production(JOB_ALGAE, 2, false) == 2);
    T_CHECK(rules_planets_job_production(JOB_ALGAE, 3, false) == 3);
    T_CHECK(rules_planets_job_production(JOB_ALGAE, 4, false) == 3);
}

static void test_algae_job_production_switches_to_curve_plus_when_tech_researched(void) {
    /* Same anchor property, now for algae's tech_gate ("xenofoods") --
     * the exact same gate string agroworld itself already uses, confirming
     * the gating is keyed off "has a tech_gate", not off any particular
     * string value. */
    T_CHECK(rules_planets_job_production(JOB_ALGAE, 1, true) == 2);
    T_CHECK(rules_planets_job_production(JOB_ALGAE, 2, true) == 3);
    T_CHECK(rules_planets_job_production(JOB_ALGAE, 3, true) == 4);
    T_CHECK(rules_planets_job_production(JOB_ALGAE, 4, true) == 4);
}

/* breed(ocean) has no tech_gate: same simpler shape as bots'/goods'/gadgets'
 * own CURVE_PLUS tests above -- job_tech_researched never changes its cost
 * or curve, and nanomaterials_researched still applies unconditionally. */
static void test_breed_ocean_job_cost_unaffected_by_tech_researched(void) {
    T_CHECK(rules_planets_job_cost(JOB_BREED_OCEAN, false, false) == 5);
    T_CHECK(rules_planets_job_cost(JOB_BREED_OCEAN, true, false) == 5);
}

static void test_breed_ocean_job_cost_nanomaterials_researched(void) {
    /* 5 * 0.85 = 4.25, floored -> 4. */
    T_CHECK(rules_planets_job_cost(JOB_BREED_OCEAN, false, true) == 4);
    T_CHECK(rules_planets_job_cost(JOB_BREED_OCEAN, true, true) == 4);
}

static void test_breed_ocean_job_production_matches_curve_plus(void) {
    for (int researched = 0; researched <= 1; researched++) {
        T_CHECK(rules_planets_job_production(JOB_BREED_OCEAN, 0, researched) == 0);
        T_CHECK(rules_planets_job_production(JOB_BREED_OCEAN, 1, researched) == 2);
        T_CHECK(rules_planets_job_production(JOB_BREED_OCEAN, 2, researched) == 3);
        T_CHECK(rules_planets_job_production(JOB_BREED_OCEAN, 3, researched) == 4);
        T_CHECK(rules_planets_job_production(JOB_BREED_OCEAN, 4, researched) == 4);
    }
}

/* Chunk 6 (docs/prd/triage/psx-full-planets-and-tech.md): remnant batch.
 * None of strip/dismantle/scavenge has a tech_gate, so job_tech_researched
 * never changes cost or curve for any of them (matching mine's/bots'/goods'/
 * gadgets'/breed(ocean)'s own no-tech-gate precedent above);
 * nanomaterials_researched still applies its unconditional 0.85 multiplier.
 * floor(5*0.85)=4 reuses breed(ocean)'s own exact precedent -- same base cost
 * (5), same math. */
static void test_strip_job_cost_unaffected_by_tech_researched(void) {
    T_CHECK(rules_planets_job_cost(JOB_STRIP, false, false) == 5);
    T_CHECK(rules_planets_job_cost(JOB_STRIP, true, false) == 5);
}

static void test_strip_job_cost_nanomaterials_researched(void) {
    /* 5 * 0.85 = 4.25, floored -> 4. */
    T_CHECK(rules_planets_job_cost(JOB_STRIP, false, true) == 4);
    T_CHECK(rules_planets_job_cost(JOB_STRIP, true, true) == 4);
}

static void test_strip_job_production_matches_curve_minus(void) {
    for (int researched = 0; researched <= 1; researched++) {
        T_CHECK(rules_planets_job_production(JOB_STRIP, 0, researched) == 0);
        T_CHECK(rules_planets_job_production(JOB_STRIP, 1, researched) == 1);
        T_CHECK(rules_planets_job_production(JOB_STRIP, 2, researched) == 2);
        T_CHECK(rules_planets_job_production(JOB_STRIP, 3, researched) == 3);
        T_CHECK(rules_planets_job_production(JOB_STRIP, 4, researched) == 3);
    }
}

static void test_dismantle_job_cost_unaffected_by_tech_researched(void) {
    T_CHECK(rules_planets_job_cost(JOB_DISMANTLE, false, false) == 5);
    T_CHECK(rules_planets_job_cost(JOB_DISMANTLE, true, false) == 5);
}

static void test_dismantle_job_cost_nanomaterials_researched(void) {
    /* 5 * 0.85 = 4.25, floored -> 4. */
    T_CHECK(rules_planets_job_cost(JOB_DISMANTLE, false, true) == 4);
    T_CHECK(rules_planets_job_cost(JOB_DISMANTLE, true, true) == 4);
}

static void test_dismantle_job_production_matches_curve_minus(void) {
    for (int researched = 0; researched <= 1; researched++) {
        T_CHECK(rules_planets_job_production(JOB_DISMANTLE, 0, researched) == 0);
        T_CHECK(rules_planets_job_production(JOB_DISMANTLE, 1, researched) == 1);
        T_CHECK(rules_planets_job_production(JOB_DISMANTLE, 2, researched) == 2);
        T_CHECK(rules_planets_job_production(JOB_DISMANTLE, 3, researched) == 3);
        T_CHECK(rules_planets_job_production(JOB_DISMANTLE, 4, researched) == 3);
    }
}

static void test_scavenge_job_cost_unaffected_by_tech_researched(void) {
    T_CHECK(rules_planets_job_cost(JOB_SCAVENGE, false, false) == 5);
    T_CHECK(rules_planets_job_cost(JOB_SCAVENGE, true, false) == 5);
}

static void test_scavenge_job_cost_nanomaterials_researched(void) {
    /* 5 * 0.85 = 4.25, floored -> 4. */
    T_CHECK(rules_planets_job_cost(JOB_SCAVENGE, false, true) == 4);
    T_CHECK(rules_planets_job_cost(JOB_SCAVENGE, true, true) == 4);
}

static void test_scavenge_job_production_matches_curve_minus(void) {
    for (int researched = 0; researched <= 1; researched++) {
        T_CHECK(rules_planets_job_production(JOB_SCAVENGE, 0, researched) == 0);
        T_CHECK(rules_planets_job_production(JOB_SCAVENGE, 1, researched) == 1);
        T_CHECK(rules_planets_job_production(JOB_SCAVENGE, 2, researched) == 2);
        T_CHECK(rules_planets_job_production(JOB_SCAVENGE, 3, researched) == 3);
        T_CHECK(rules_planets_job_production(JOB_SCAVENGE, 4, researched) == 3);
    }
}

/* Chunk 7 (docs/prd/triage/psx-full-planets-and-tech.md): xeno + jungle
 * batch. colony(xeno) is tech-gated exactly like colony(ocean) from chunk 5
 * -- same base cost (10) and same multipliers, so the same numbers apply:
 * base 10, tech-only -> 15, nano-only -> 8, both -> 12. Independently
 * derived here (not copied blindly): 10*1.5=15 exact; 10*0.85=8.5, floored
 * -> 8; 10*1.5*0.85=12.75, floored ONCE at the end -> 12. */
static void test_colony_xeno_job_cost_base_case(void) {
    T_CHECK(rules_planets_job_cost(JOB_COLONY_XENO, false, false) == 10);
}

static void test_colony_xeno_job_cost_tech_researched_only(void) {
    /* 10 * 1.5 = 15, exact. */
    T_CHECK(rules_planets_job_cost(JOB_COLONY_XENO, true, false) == 15);
}

static void test_colony_xeno_job_cost_nanomaterials_researched_only(void) {
    /* 10 * 0.85 = 8.5, floored -> 8. */
    T_CHECK(rules_planets_job_cost(JOB_COLONY_XENO, false, true) == 8);
}

static void test_colony_xeno_job_cost_both_flags_true_floors_once_at_end(void) {
    /* 10 * 1.5 * 0.85 = 12.75, floored ONCE at the end -> 12. */
    T_CHECK(rules_planets_job_cost(JOB_COLONY_XENO, true, true) == 12);
}

static void test_colony_xeno_job_production_matches_curve_minus_unresearched(void) {
    T_CHECK(rules_planets_job_production(JOB_COLONY_XENO, 0, false) == 0);
    T_CHECK(rules_planets_job_production(JOB_COLONY_XENO, 1, false) == 1);
    T_CHECK(rules_planets_job_production(JOB_COLONY_XENO, 2, false) == 2);
    T_CHECK(rules_planets_job_production(JOB_COLONY_XENO, 3, false) == 3);
    T_CHECK(rules_planets_job_production(JOB_COLONY_XENO, 4, false) == 3);
}

static void test_colony_xeno_job_production_switches_to_curve_plus_when_tech_researched(void) {
    /* Same anchor property agroworld's/colony(ocean)'s own equivalent tests
     * above prove, now for colony(xeno)'s tech_gate ("gene rewriting"):
     * CURVE_MINUS ({1,2,3,3}) flips to CURVE_PLUS ({2,3,4,4}). */
    T_CHECK(rules_planets_job_production(JOB_COLONY_XENO, 1, true) == 2);
    T_CHECK(rules_planets_job_production(JOB_COLONY_XENO, 2, true) == 3);
    T_CHECK(rules_planets_job_production(JOB_COLONY_XENO, 3, true) == 4);
    T_CHECK(rules_planets_job_production(JOB_COLONY_XENO, 4, true) == 4);
}

/* breed(xeno) and hunt have no tech_gate: same simpler shape as bots'/
 * goods'/gadgets'/breed(ocean)'s own no-gate tests above -- job_tech_researched
 * never changes cost or curve, and nanomaterials_researched still applies
 * its unconditional 0.85 multiplier. */
static void test_breed_xeno_job_cost_unaffected_by_tech_researched(void) {
    T_CHECK(rules_planets_job_cost(JOB_BREED_XENO, false, false) == 5);
    T_CHECK(rules_planets_job_cost(JOB_BREED_XENO, true, false) == 5);
}

static void test_breed_xeno_job_cost_nanomaterials_researched(void) {
    /* 5 * 0.85 = 4.25, floored -> 4. */
    T_CHECK(rules_planets_job_cost(JOB_BREED_XENO, false, true) == 4);
    T_CHECK(rules_planets_job_cost(JOB_BREED_XENO, true, true) == 4);
}

static void test_breed_xeno_job_production_matches_curve_minus(void) {
    for (int researched = 0; researched <= 1; researched++) {
        T_CHECK(rules_planets_job_production(JOB_BREED_XENO, 0, researched) == 0);
        T_CHECK(rules_planets_job_production(JOB_BREED_XENO, 1, researched) == 1);
        T_CHECK(rules_planets_job_production(JOB_BREED_XENO, 2, researched) == 2);
        T_CHECK(rules_planets_job_production(JOB_BREED_XENO, 3, researched) == 3);
        T_CHECK(rules_planets_job_production(JOB_BREED_XENO, 4, researched) == 3);
    }
}

static void test_hunt_job_cost_unaffected_by_tech_researched(void) {
    T_CHECK(rules_planets_job_cost(JOB_HUNT, false, false) == 5);
    T_CHECK(rules_planets_job_cost(JOB_HUNT, true, false) == 5);
}

static void test_hunt_job_cost_nanomaterials_researched(void) {
    /* 5 * 0.85 = 4.25, floored -> 4. */
    T_CHECK(rules_planets_job_cost(JOB_HUNT, false, true) == 4);
    T_CHECK(rules_planets_job_cost(JOB_HUNT, true, true) == 4);
}

static void test_hunt_job_production_matches_curve_minus(void) {
    for (int researched = 0; researched <= 1; researched++) {
        T_CHECK(rules_planets_job_production(JOB_HUNT, 0, researched) == 0);
        T_CHECK(rules_planets_job_production(JOB_HUNT, 1, researched) == 1);
        T_CHECK(rules_planets_job_production(JOB_HUNT, 2, researched) == 2);
        T_CHECK(rules_planets_job_production(JOB_HUNT, 3, researched) == 3);
        T_CHECK(rules_planets_job_production(JOB_HUNT, 4, researched) == 3);
    }
}

/* tourism: the first real job to use CURVE_F_MINUS (added in chunk 1).
 * tech_gate == NULL, so job_tech_researched has zero cost effect (same
 * no-gate pattern as breed(xeno)/hunt above) -- base 10, nano-only
 * floor(10*0.85)=8, same nanomaterials math as colony(ocean)'s own base-10
 * case. Production values {0,1,2,3} at levels 1-4 are asserted identical for
 * job_tech_researched=true and false, since CURVE_F_MINUS has no upgrade
 * table at all (confirmed directly from rules_planets_curve_value's source:
 * the "upgraded && curve == CURVE_MINUS" branch never matches
 * CURVE_F_MINUS) -- this is the first real (non-synthetic) exercise of that
 * no-op behavior end to end through rules_planets_job_production. */
static void test_tourism_job_cost_unaffected_by_tech_researched(void) {
    T_CHECK(rules_planets_job_cost(JOB_TOURISM, false, false) == 10);
    T_CHECK(rules_planets_job_cost(JOB_TOURISM, true, false) == 10);
}

static void test_tourism_job_cost_nanomaterials_researched(void) {
    /* 10 * 0.85 = 8.5, floored -> 8. */
    T_CHECK(rules_planets_job_cost(JOB_TOURISM, false, true) == 8);
    T_CHECK(rules_planets_job_cost(JOB_TOURISM, true, true) == 8);
}

static void test_tourism_job_production_matches_curve_f_minus_regardless_of_tech_researched(void) {
    for (int researched = 0; researched <= 1; researched++) {
        T_CHECK(rules_planets_job_production(JOB_TOURISM, 0, researched) == 0);
        T_CHECK(rules_planets_job_production(JOB_TOURISM, 1, researched) == 0);
        T_CHECK(rules_planets_job_production(JOB_TOURISM, 2, researched) == 1);
        T_CHECK(rules_planets_job_production(JOB_TOURISM, 3, researched) == 2);
        T_CHECK(rules_planets_job_production(JOB_TOURISM, 4, researched) == 3);
    }
}

/* luxury: same no-gate cost/production shape as tourism (chunk 7) -- base
 * cost 10, tech_gate == NULL so job_tech_researched never changes cost,
 * nanomaterials-only floor(10*0.85)=8 (same math as tourism's own base-10
 * case). Production values {0,1,2,3} at levels 1-4 identical for
 * job_tech_researched=true/false, since CURVE_F_MINUS has no upgrade table
 * regardless of which job selects it -- this is the second independent,
 * real (non-synthetic) exercise of that no-op behavior end to end through
 * rules_planets_job_production, confirming the curve lookup genuinely
 * generalizes across jobs rather than having been coincidentally correct
 * for tourism alone. */
static void test_luxury_job_cost_unaffected_by_tech_researched(void) {
    T_CHECK(rules_planets_job_cost(JOB_LUXURY, false, false) == 10);
    T_CHECK(rules_planets_job_cost(JOB_LUXURY, true, false) == 10);
}

static void test_luxury_job_cost_nanomaterials_researched(void) {
    /* 10 * 0.85 = 8.5, floored -> 8. */
    T_CHECK(rules_planets_job_cost(JOB_LUXURY, false, true) == 8);
    T_CHECK(rules_planets_job_cost(JOB_LUXURY, true, true) == 8);
}

static void test_luxury_job_production_matches_curve_f_minus_regardless_of_tech_researched(void) {
    for (int researched = 0; researched <= 1; researched++) {
        T_CHECK(rules_planets_job_production(JOB_LUXURY, 0, researched) == 0);
        T_CHECK(rules_planets_job_production(JOB_LUXURY, 1, researched) == 0);
        T_CHECK(rules_planets_job_production(JOB_LUXURY, 2, researched) == 1);
        T_CHECK(rules_planets_job_production(JOB_LUXURY, 3, researched) == 2);
        T_CHECK(rules_planets_job_production(JOB_LUXURY, 4, researched) == 3);
    }
}

/* Chunk 9 (docs/prd/triage/psx-full-planets-and-tech.md): barren/iceball/
 * gas-giant batch cost/production. None of the 5 has a tech_gate, so
 * job_tech_researched has zero cost/curve effect for any of them (same
 * no-gate pattern as breed(xeno)/hunt/tourism/luxury above);
 * nanomaterials_researched still applies its unconditional 0.85 multiplier.
 * Base-12 jobs (habitat, genefarm, excavate_mineral,
 * excavate_iceball_gasgiant): 12*0.85=10.2, floored -> 10 (independently
 * computed here, not assumed from any earlier chunk's base-10/base-5/base-8
 * values, since no prior chunk has a base-12 job). farm's base-10 nano-only
 * value floor(10*0.85)=8 reuses colony(xeno)'s/tourism's/luxury's own
 * already-verified base-10 math directly. */
static void test_habitat_job_cost_unaffected_by_tech_researched(void) {
    T_CHECK(rules_planets_job_cost(JOB_HABITAT, false, false) == 12);
    T_CHECK(rules_planets_job_cost(JOB_HABITAT, true, false) == 12);
}

static void test_habitat_job_cost_nanomaterials_researched(void) {
    /* 12 * 0.85 = 10.2, floored -> 10. */
    T_CHECK(rules_planets_job_cost(JOB_HABITAT, false, true) == 10);
    T_CHECK(rules_planets_job_cost(JOB_HABITAT, true, true) == 10);
}

static void test_habitat_job_production_matches_curve_minus(void) {
    for (int researched = 0; researched <= 1; researched++) {
        T_CHECK(rules_planets_job_production(JOB_HABITAT, 0, researched) == 0);
        T_CHECK(rules_planets_job_production(JOB_HABITAT, 1, researched) == 1);
        T_CHECK(rules_planets_job_production(JOB_HABITAT, 2, researched) == 2);
        T_CHECK(rules_planets_job_production(JOB_HABITAT, 3, researched) == 3);
        T_CHECK(rules_planets_job_production(JOB_HABITAT, 4, researched) == 3);
    }
}

static void test_genefarm_job_cost_unaffected_by_tech_researched(void) {
    T_CHECK(rules_planets_job_cost(JOB_GENEFARM, false, false) == 12);
    T_CHECK(rules_planets_job_cost(JOB_GENEFARM, true, false) == 12);
}

static void test_genefarm_job_cost_nanomaterials_researched(void) {
    /* 12 * 0.85 = 10.2, floored -> 10. */
    T_CHECK(rules_planets_job_cost(JOB_GENEFARM, false, true) == 10);
    T_CHECK(rules_planets_job_cost(JOB_GENEFARM, true, true) == 10);
}

static void test_genefarm_job_production_matches_curve_minus(void) {
    for (int researched = 0; researched <= 1; researched++) {
        T_CHECK(rules_planets_job_production(JOB_GENEFARM, 0, researched) == 0);
        T_CHECK(rules_planets_job_production(JOB_GENEFARM, 1, researched) == 1);
        T_CHECK(rules_planets_job_production(JOB_GENEFARM, 2, researched) == 2);
        T_CHECK(rules_planets_job_production(JOB_GENEFARM, 3, researched) == 3);
        T_CHECK(rules_planets_job_production(JOB_GENEFARM, 4, researched) == 3);
    }
}

static void test_excavate_mineral_job_cost_unaffected_by_tech_researched(void) {
    T_CHECK(rules_planets_job_cost(JOB_EXCAVATE_MINERAL, false, false) == 12);
    T_CHECK(rules_planets_job_cost(JOB_EXCAVATE_MINERAL, true, false) == 12);
}

static void test_excavate_mineral_job_cost_nanomaterials_researched(void) {
    /* 12 * 0.85 = 10.2, floored -> 10. */
    T_CHECK(rules_planets_job_cost(JOB_EXCAVATE_MINERAL, false, true) == 10);
    T_CHECK(rules_planets_job_cost(JOB_EXCAVATE_MINERAL, true, true) == 10);
}

/* CURVE_PLUS, distinct from excavate_iceball_gasgiant's CURVE_MINUS below
 * despite the identical base cost/needs/produces -- see
 * test_excavate_mineral_and_iceball_gasgiant_are_genuinely_distinct for the
 * direct side-by-side comparison. */
static void test_excavate_mineral_job_production_matches_curve_plus(void) {
    for (int researched = 0; researched <= 1; researched++) {
        T_CHECK(rules_planets_job_production(JOB_EXCAVATE_MINERAL, 0, researched) == 0);
        T_CHECK(rules_planets_job_production(JOB_EXCAVATE_MINERAL, 1, researched) == 2);
        T_CHECK(rules_planets_job_production(JOB_EXCAVATE_MINERAL, 2, researched) == 3);
        T_CHECK(rules_planets_job_production(JOB_EXCAVATE_MINERAL, 3, researched) == 4);
        T_CHECK(rules_planets_job_production(JOB_EXCAVATE_MINERAL, 4, researched) == 4);
    }
}

static void test_excavate_iceball_gasgiant_job_cost_unaffected_by_tech_researched(void) {
    T_CHECK(rules_planets_job_cost(JOB_EXCAVATE_ICEBALL_GASGIANT, false, false) == 12);
    T_CHECK(rules_planets_job_cost(JOB_EXCAVATE_ICEBALL_GASGIANT, true, false) == 12);
}

static void test_excavate_iceball_gasgiant_job_cost_nanomaterials_researched(void) {
    /* 12 * 0.85 = 10.2, floored -> 10. */
    T_CHECK(rules_planets_job_cost(JOB_EXCAVATE_ICEBALL_GASGIANT, false, true) == 10);
    T_CHECK(rules_planets_job_cost(JOB_EXCAVATE_ICEBALL_GASGIANT, true, true) == 10);
}

/* CURVE_MINUS, distinct from excavate_mineral's CURVE_PLUS above. */
static void test_excavate_iceball_gasgiant_job_production_matches_curve_minus(void) {
    for (int researched = 0; researched <= 1; researched++) {
        T_CHECK(rules_planets_job_production(JOB_EXCAVATE_ICEBALL_GASGIANT, 0, researched) == 0);
        T_CHECK(rules_planets_job_production(JOB_EXCAVATE_ICEBALL_GASGIANT, 1, researched) == 1);
        T_CHECK(rules_planets_job_production(JOB_EXCAVATE_ICEBALL_GASGIANT, 2, researched) == 2);
        T_CHECK(rules_planets_job_production(JOB_EXCAVATE_ICEBALL_GASGIANT, 3, researched) == 3);
        T_CHECK(rules_planets_job_production(JOB_EXCAVATE_ICEBALL_GASGIANT, 4, researched) == 3);
    }
}

static void test_farm_job_cost_unaffected_by_tech_researched(void) {
    T_CHECK(rules_planets_job_cost(JOB_FARM, false, false) == 10);
    T_CHECK(rules_planets_job_cost(JOB_FARM, true, false) == 10);
}

static void test_farm_job_cost_nanomaterials_researched(void) {
    /* 10 * 0.85 = 8.5, floored -> 8 -- reuses colony(xeno)'s/tourism's/
     * luxury's own already-verified base-10 nanomaterials math directly. */
    T_CHECK(rules_planets_job_cost(JOB_FARM, false, true) == 8);
    T_CHECK(rules_planets_job_cost(JOB_FARM, true, true) == 8);
}

static void test_farm_job_production_matches_curve_minus(void) {
    for (int researched = 0; researched <= 1; researched++) {
        T_CHECK(rules_planets_job_production(JOB_FARM, 0, researched) == 0);
        T_CHECK(rules_planets_job_production(JOB_FARM, 1, researched) == 1);
        T_CHECK(rules_planets_job_production(JOB_FARM, 2, researched) == 2);
        T_CHECK(rules_planets_job_production(JOB_FARM, 3, researched) == 3);
        T_CHECK(rules_planets_job_production(JOB_FARM, 4, researched) == 3);
    }
}

/* Chunk 9's own sanity guard: JOB_COUNT must be exactly 24 after this
 * chunk's 5-job extension -- the full roster, matching the PRD's chunk 10
 * "JOB_COUNT == 24" full-roster guard applied early, at the point where this
 * module's own enum actually grows to that size. */
static void test_job_count_is_24(void) {
    T_CHECK(JOB_COUNT == 24);
}

/* ========================================================================
 * Chunk 10 (docs/prd/triage/psx-full-planets-and-tech.md): "Full-roster
 * regression + closed-loop scope confirmation". No new jobs or planet types
 * are added here -- chunks 1-9 already landed all 10 planet types and all 24
 * jobs (confirmed by direct re-read of rules_planets.h/.c immediately above
 * writing this section). This is purely a consolidation/regression-proofing
 * chunk, in three parts.
 * ======================================================================== */

/* Part 1: a consolidated sanity guard, asserting PLANET_TYPE_COUNT == 10 and
 * JOB_COUNT == 24 together, in one place, as the PRD's own chunk 10 wording
 * asks for ("A consolidated test asserting PLANET_TYPE_COUNT == 10 and
 * JOB_COUNT == 24"). test_planet_type_count_is_10 (chunk 2, above) and
 * test_job_count_is_24 (chunk 9, above) already assert these individually --
 * deliberately left untouched rather than deleted or merged into this one:
 * each chunk's own tests are meant to stand alone as that chunk's regression
 * proof (per this chunk's own task framing), so consolidating by deleting
 * either earlier test would weaken that guarantee for no real benefit. This
 * new, separately-named test is chunk 10's own belt-and-suspenders duplicate
 * of the same guard, not a replacement for either. */
static void test_full_roster_sanity_guard(void) {
    T_CHECK(PLANET_TYPE_COUNT == 10);
    T_CHECK(JOB_COUNT == 24);
}

/* Part 2: every one of the 24 jobs' full field set, re-asserted in ONE place
 * as a data table. This table is independently re-derived directly from the
 * PRD's own "24 real colonization jobs" table
 * (docs/prd/triage/psx-full-planets-and-tech.md, Implementation Decisions),
 * re-typed by hand from that table's rows -- deliberately NOT copied or
 * factored out of any of this file's own earlier per-job tests above (e.g.
 * test_agroworld_job_matches_traced_values), since this chunk's whole
 * purpose is to catch any per-chunk transcription slip a prior chunk might
 * have introduced; using an earlier chunk's own test as "ground truth" could
 * never catch that earlier chunk's own mistake, by construction. */
typedef struct {
    colonization_job_id_t id;
    planet_type_id_t planet_types[RULES_PLANETS_MAX_TYPES];
    int planet_types_count;
    int cost;
    resource_id_t needs[RULES_PLANETS_MAX_NEEDS];
    int needs_count;
    resource_id_t produces;
    curve_id_t curve;
    const char *tech_gate;      /* il: cost surcharge + curve upgrade gate. */
    const char *required_tech;  /* rr: job-availability gate (data only). */
    resource_id_t default_need; /* dm. */
} prd_job_row_t;

static const prd_job_row_t g_prd_job_table[] = {
    /* earth-like */
    { JOB_HIVEWORLD, { PLANET_TYPE_EARTHLIKE }, 1, 5, { RESOURCE_FOOD }, 1, RESOURCE_PEOPLE, CURVE_PLUS, NULL, NULL, RESOURCE_GOODS },
    { JOB_AGROWORLD, { PLANET_TYPE_EARTHLIKE }, 1, 5, { RESOURCE_BOTS }, 1, RESOURCE_FOOD, CURVE_MINUS, "xenofoods", NULL, RESOURCE_ORGANICS },
    { JOB_LUXURY, { PLANET_TYPE_EARTHLIKE }, 1, 10, { RESOURCE_TECH, RESOURCE_GOODS }, 2, RESOURCE_LUXURY, CURVE_F_MINUS, NULL, NULL, RESOURCE_NONE },
    /* forgeworld */
    { JOB_NANOTECH, { PLANET_TYPE_FORGEWORLD }, 1, 8, { RESOURCE_ORE }, 1, RESOURCE_TECH, CURVE_MINUS, NULL, NULL, RESOURCE_NONE },
    { JOB_BOTS, { PLANET_TYPE_FORGEWORLD }, 1, 8, { RESOURCE_ORE, RESOURCE_TECH }, 2, RESOURCE_BOTS, CURVE_PLUS, NULL, NULL, RESOURCE_NONE },
    { JOB_GOODS, { PLANET_TYPE_FORGEWORLD }, 1, 10, { RESOURCE_ORE, RESOURCE_BOTS }, 2, RESOURCE_GOODS, CURVE_PLUS, NULL, NULL, RESOURCE_NONE },
    { JOB_GADGETS, { PLANET_TYPE_FORGEWORLD }, 1, 10, { RESOURCE_ORE, RESOURCE_TECH }, 2, RESOURCE_GOODS, CURVE_PLUS, NULL, NULL, RESOURCE_NONE },
    { JOB_UPLIFT, { PLANET_TYPE_FORGEWORLD }, 1, 20, { RESOURCE_ORGANICS }, 1, RESOURCE_PEOPLE, CURVE_MINUS, NULL, "gene rewriting", RESOURCE_NONE },
    /* mineral */
    { JOB_MINE, { PLANET_TYPE_MINERAL }, 1, 5, { RESOURCE_PEOPLE }, 1, RESOURCE_ORE, CURVE_MINUS, NULL, NULL, RESOURCE_NONE },
    { JOB_EXCAVATE_MINERAL, { PLANET_TYPE_MINERAL }, 1, 12, { RESOURCE_BOTS }, 1, RESOURCE_ORE, CURVE_PLUS, NULL, "drillbots", RESOURCE_NONE },
    /* ocean */
    { JOB_COLONY_OCEAN, { PLANET_TYPE_OCEAN }, 1, 10, { RESOURCE_ORE, RESOURCE_FOOD }, 2, RESOURCE_PEOPLE, CURVE_MINUS, "gene rewriting", NULL, RESOURCE_GOODS },
    { JOB_ALGAE, { PLANET_TYPE_OCEAN }, 1, 8, { RESOURCE_BOTS }, 1, RESOURCE_FOOD, CURVE_MINUS, "xenofoods", NULL, RESOURCE_ORGANICS },
    { JOB_BREED_OCEAN, { PLANET_TYPE_OCEAN }, 1, 5, { RESOURCE_PEOPLE }, 1, RESOURCE_ORGANICS, CURVE_PLUS, NULL, NULL, RESOURCE_NONE },
    /* remnant */
    { JOB_STRIP, { PLANET_TYPE_REMNANT }, 1, 5, { RESOURCE_PEOPLE, RESOURCE_FOOD }, 2, RESOURCE_ORE, CURVE_MINUS, NULL, NULL, RESOURCE_NONE },
    { JOB_DISMANTLE, { PLANET_TYPE_REMNANT }, 1, 5, { RESOURCE_PEOPLE, RESOURCE_FOOD }, 2, RESOURCE_TECH, CURVE_MINUS, NULL, NULL, RESOURCE_NONE },
    { JOB_SCAVENGE, { PLANET_TYPE_REMNANT }, 1, 5, { RESOURCE_PEOPLE, RESOURCE_FOOD }, 2, RESOURCE_BOTS, CURVE_MINUS, NULL, NULL, RESOURCE_NONE },
    /* xeno */
    { JOB_COLONY_XENO, { PLANET_TYPE_XENO }, 1, 10, { RESOURCE_TECH, RESOURCE_FOOD }, 2, RESOURCE_PEOPLE, CURVE_MINUS, "gene rewriting", NULL, RESOURCE_GOODS },
    { JOB_BREED_XENO, { PLANET_TYPE_XENO }, 1, 5, { RESOURCE_BOTS }, 1, RESOURCE_ORGANICS, CURVE_MINUS, NULL, NULL, RESOURCE_NONE },
    /* jungle */
    { JOB_HUNT, { PLANET_TYPE_JUNGLE }, 1, 5, { RESOURCE_PEOPLE }, 1, RESOURCE_ORGANICS, CURVE_MINUS, NULL, NULL, RESOURCE_NONE },
    { JOB_TOURISM, { PLANET_TYPE_JUNGLE }, 1, 10, { RESOURCE_PEOPLE, RESOURCE_GOODS }, 2, RESOURCE_LUXURY, CURVE_F_MINUS, NULL, NULL, RESOURCE_NONE },
    /* xeno + jungle (multi-type) */
    { JOB_FARM, { PLANET_TYPE_XENO, PLANET_TYPE_JUNGLE }, 2, 10, { RESOURCE_BOTS }, 1, RESOURCE_FOOD, CURVE_MINUS, NULL, "xenofoods", RESOURCE_ORGANICS },
    /* barren + iceball (multi-type) */
    { JOB_HABITAT, { PLANET_TYPE_BARREN, PLANET_TYPE_ICEBALL }, 2, 12, { RESOURCE_ORGANICS, RESOURCE_FOOD }, 2, RESOURCE_PEOPLE, CURVE_MINUS, NULL, "geneseeds", RESOURCE_GOODS },
    { JOB_GENEFARM, { PLANET_TYPE_BARREN, PLANET_TYPE_ICEBALL }, 2, 12, { RESOURCE_ORGANICS, RESOURCE_BOTS }, 2, RESOURCE_FOOD, CURVE_MINUS, NULL, "geneseeds", RESOURCE_NONE },
    /* iceball + gas giant (multi-type) */
    { JOB_EXCAVATE_ICEBALL_GASGIANT, { PLANET_TYPE_ICEBALL, PLANET_TYPE_GASGIANT }, 2, 12, { RESOURCE_BOTS }, 1, RESOURCE_ORE, CURVE_MINUS, NULL, "drillbots", RESOURCE_NONE },
};

static void test_full_roster_matches_prd_table(void) {
    size_t row_count = sizeof(g_prd_job_table) / sizeof(g_prd_job_table[0]);
    /* The table itself must have exactly 24 rows -- a missing or duplicated
     * row here would otherwise silently under-cover the roster. */
    T_CHECK(row_count == (size_t)JOB_COUNT);

    bool seen[JOB_COUNT];
    for (int i = 0; i < JOB_COUNT; i++) {
        seen[i] = false;
    }

    for (size_t r = 0; r < row_count; r++) {
        const prd_job_row_t *row = &g_prd_job_table[r];

        /* Every row's id must be in range and appear exactly once -- catches
         * both an omitted job id and an accidentally-duplicated one. */
        T_CHECK(row->id >= 0 && row->id < JOB_COUNT);
        T_CHECK(!seen[row->id]);
        seen[row->id] = true;

        const colonization_job_t *j = rules_planets_job(row->id);
        T_CHECK(j != NULL);

        T_CHECK(j->planet_types_count == row->planet_types_count);
        for (int t = 0; t < row->planet_types_count; t++) {
            T_CHECK(j->planet_types[t] == row->planet_types[t]);
        }

        T_CHECK(j->cost == row->cost);

        T_CHECK(j->needs_count == row->needs_count);
        for (int n = 0; n < row->needs_count; n++) {
            T_CHECK(j->needs[n] == row->needs[n]);
        }

        T_CHECK(j->produces == row->produces);
        T_CHECK(j->curve == row->curve);

        T_CHECK((j->tech_gate == NULL) == (row->tech_gate == NULL));
        if (j->tech_gate != NULL && row->tech_gate != NULL) {
            T_CHECK(strcmp(j->tech_gate, row->tech_gate) == 0);
        }

        T_CHECK((j->required_tech == NULL) == (row->required_tech == NULL));
        if (j->required_tech != NULL && row->required_tech != NULL) {
            T_CHECK(strcmp(j->required_tech, row->required_tech) == 0);
        }

        T_CHECK(j->default_need == row->default_need);
    }

    /* Every one of the 24 job ids was covered by exactly one row above. */
    for (int i = 0; i < JOB_COUNT; i++) {
        T_CHECK(seen[i]);
    }
}

/* Part 3: an explicit, separately-named test confirming the EXISTING
 * closed-loop structural tests (test_every_need_is_produced_by_another_job/
 * test_every_produce_is_needed_by_another_job, from
 * docs/prd/psx-rules-planets.md's own original, already-graduated PRD, above)
 * still pass unchanged in scope -- both are already pinned to
 * RULES_PLANETS_ORIGINAL_CLOSED_LOOP_JOB_COUNT (5), not JOB_COUNT, and this
 * chunk does not touch either test or that constant.
 *
 * This step does not attempt to extend "closed loop" to the full 24-job
 * roster, because that's not a real property of the source game itself.
 * The original 5-job subset's closed-loop property (every need has an
 * in-subset producer, every product has an in-subset consumer) was a
 * deliberate, self-imposed design choice of that original PRD's own 5-job
 * selection, not a property of the source game as a whole -- the full
 * 24-job roster genuinely isn't closed: luxury, produced by tourism/luxury,
 * is consumed by no job among the full 24-job roster's needs; it feeds
 * docs/prd/psx-rules-economy.md's db()/bf() formulas directly, outside this
 * module entirely. This test states that explicitly and demonstrates it
 * directly (not just by assertion) so a future reader doesn't either
 * (a) assume the extended roster is still closed and get confused when a
 * hypothetical future "closed-loop, full roster" test would necessarily
 * fail, or (b) assume the existing 5-job test above was weakened by this
 * step (it wasn't touched at all). */
static void test_full_roster_is_not_closed_loop(void) {
    /* First confirm RESOURCE_LUXURY genuinely has a producer in the full
     * roster (tourism, luxury) -- otherwise "no consumer" below would be a
     * vacuous finding about a resource nothing produces, not a real sink. */
    bool luxury_has_producer = false;
    for (int i = 0; i < JOB_COUNT; i++) {
        const colonization_job_t *j = rules_planets_job((colonization_job_id_t)i);
        if (j->produces == RESOURCE_LUXURY) {
            luxury_has_producer = true;
            break;
        }
    }
    T_CHECK(luxury_has_producer);

    /* The actual negative property this test exists to prove: no job among
     * the full 24-job roster needs RESOURCE_LUXURY -- luxury is a genuine
     * terminal sink, not a dangling oversight. */
    bool luxury_has_consumer = false;
    for (int i = 0; i < JOB_COUNT; i++) {
        const colonization_job_t *j = rules_planets_job((colonization_job_id_t)i);
        for (int n = 0; n < j->needs_count; n++) {
            if (j->needs[n] == RESOURCE_LUXURY) {
                luxury_has_consumer = true;
                break;
            }
        }
        if (luxury_has_consumer) {
            break;
        }
    }
    T_CHECK(!luxury_has_consumer);
}

/* ========================================================================
 * Chunk 6 (docs/prd/triage/psx-structures-and-transforms.md): "Full-
 * structure-roster regression". Mirrors
 * docs/prd/psx-full-planets-and-tech.md's own chunk-10 precedent
 * (test_full_roster_matches_prd_table above, "Part 2"): a single consolidated
 * test walking all 8 structures' full field set, independently re-derived
 * directly from THIS PRD's own structure table
 * (docs/prd/triage/psx-structures-and-transforms.md, Implementation
 * Decisions -- the 8-row #/hp/Structure/Needs/Produces/Curve/il/dm table,
 * plus the cost/required_tech/resolvable[]/max_connections/
 * crosses_other_routes values cited in that PRD's "Cost model"/"Purchase-gate
 * rr"/"Resource-code resolution"/"Live-wiring verdict" prose paragraphs) --
 * deliberately NOT copied from this file's own chunks 2-5 tests above
 * (test_laboratory_structure_matches_traced_values,
 * test_food_processor_structure_matches_traced_values,
 * test_ascension_gate_structure_matches_traced_values,
 * test_protostar_structure_matches_traced_values,
 * test_synthesizer_structure_matches_traced_values,
 * test_trading_hub_structure_matches_traced_values,
 * test_slipgate_structure_matches_traced_values,
 * test_infragate_structure_matches_traced_values), since the whole point of
 * this consolidation (like the predecessor PRD's chunk 10) is to catch any
 * per-chunk transcription slip one of those earlier tests' own hand-typed
 * expectations might itself have introduced -- using an earlier chunk's own
 * test as "ground truth" here could never catch that earlier chunk's own
 * mistake, by construction.
 *
 * STRUCTURE_COUNT == 8 is already asserted by test_structure_count_is_8
 * (chunk 2, above) -- this chunk's own task explicitly does NOT duplicate
 * that guard; its job is the field-set consolidation below only.
 * ======================================================================== */
typedef struct {
    structure_id_t id;
    const char *name;
    bool purchasable;
    int cost;                   /* od; not checked when !purchasable (Protostar has no od field at all). */
    const char *required_tech;  /* entity-literal rr: purchase gate (data only). */
    resource_id_t needs[RULES_PLANETS_MAX_NEEDS];
    int needs_count;
    resource_id_t produces;
    curve_id_t curve;
    const char *tech_gate;      /* il: curve-upgrade gate. */
    resource_id_t default_need; /* dm. */
    resource_id_t resolvable[RULES_PLANETS_MAX_RESOLVABLE]; /* ba/ht: joker-resolution set. */
    int resolvable_count;
    int max_connections;        /* Slipgate/Infragate only (#self.oe<3); 0 otherwise. */
    bool crosses_other_routes;  /* Infragate's mt=1; false for every other structure. */
} prd_structure_row_t;

static const prd_structure_row_t g_prd_structure_table[] = {
    /* Ordered per the PRD's own #/hp table (Laboratory=1 .. Trading Hub=6),
     * then Slipgate/Infragate (separately cited, no fd/hp record) -- NOT the
     * enum's/g_structures[]'s own declaration order (Laboratory, Food
     * processor, Slipgate, Infragate, Trading Hub, Synthesizer, Ascension
     * Gate, Protostar) -- deliberately, so this table's row order can't be a
     * disguised copy of the struct's own layout. */

    /* 1: Laboratory. od=15 (pico8/lua.lua:910); no rr on its own definition;
     * needs = people, joker; produces = science; curve = qh, upgraded via
     * il="superhuman ai" to qh+; resolvable = ba=o(28,18,20,21,22,48)
     * (pico8/lua.lua:914) = food, bots, organics, ore, tech, goods (6). */
    { STRUCTURE_LABORATORY, "Laboratory", true, 15, NULL,
      { RESOURCE_PEOPLE, RESOURCE_JOKER }, 2, RESOURCE_SCIENCE, CURVE_QH, "superhuman ai", RESOURCE_NONE,
      { RESOURCE_FOOD, RESOURCE_BOTS, RESOURCE_ORGANICS, RESOURCE_ORE, RESOURCE_TECH, RESOURCE_GOODS }, 6,
      0, false },

    /* 2: Ascension Gate. od=60 (pico8/lua.lua:1003); rr="ascension"
     * (pico8/lua.lua:1000) -- a purchase gate, captured as data only; needs =
     * people; produces = joy; curve = f+, no il (f+ has no mc upgrade target
     * of its own). No joker need/produce -- resolvable_count = 0. */
    { STRUCTURE_ASCENSION_GATE, "Ascension Gate", true, 60, "ascension",
      { RESOURCE_PEOPLE }, 1, RESOURCE_JOY, CURVE_F_PLUS, NULL, RESOURCE_NONE,
      { 0 }, 0,
      0, false },

    /* 3: Protostar, non-purchasable -- created only by collapse (chunk 7's
     * job); no od field at all in source, so cost is not checked below (see
     * structure_t's own doc comment: "meaningless when purchasable ==
     * false"). needs = none; produces = energy; curve = `-` (CURVE_MINUS). */
    { STRUCTURE_PROTOSTAR, "Protostar", false, 0, NULL,
      { 0 }, 0, RESOURCE_ENERGY, CURVE_MINUS, NULL, RESOURCE_NONE,
      { 0 }, 0,
      0, false },

    /* 4: Food processor. od=15 (pico8/lua.lua:1099); no rr on its own
     * definition; needs = organics; produces = food; curve = f-, upgraded via
     * il="xenofoods" to f+. No joker need/produce -- resolvable_count = 0. */
    { STRUCTURE_FOOD_PROCESSOR, "Food processor", true, 15, NULL,
      { RESOURCE_ORGANICS }, 1, RESOURCE_FOOD, CURVE_F_MINUS, "xenofoods", RESOURCE_NONE,
      { 0 }, 0,
      0, false },

    /* 5: Synthesizer. od=40 (pico8/lua.lua:1040); rr="void synthesis"
     * (pico8/lua.lua:1038); needs = none; produces = joker (dynamic, captured
     * as the sentinel, not resolved); curve = f-, no il; resolvable =
     * ht=o(28,20,21,22) (pico8/lua.lua:1041) = food, organics, ore, tech (4)
     * -- design-doc correction #2: the real 4th resource is ore (21), NOT
     * bots (18). */
    { STRUCTURE_SYNTHESIZER, "Synthesizer", true, 40, "void synthesis",
      { 0 }, 0, RESOURCE_JOKER, CURVE_F_MINUS, NULL, RESOURCE_NONE,
      { RESOURCE_FOOD, RESOURCE_ORGANICS, RESOURCE_ORE, RESOURCE_TECH }, 4,
      0, false },

    /* 6: Trading Hub. od=20 (pico8/lua.lua:1061); rr="trade league"
     * (pico8/lua.lua:1059); needs = joker, joker; produces = luxury --
     * design-doc correction #1: the raw fd record's produce field is
     * unambiguously w=luxury (r.w=12), NOT joy/energy; curve = f-, no il;
     * default_need = joker (dm="?"); resolvable = ba=o(28,48,18,20,21,22)
     * (pico8/lua.lua:1062) = food, goods, bots, organics, ore, tech (6). */
    { STRUCTURE_TRADING_HUB, "Trading Hub", true, 20, "trade league",
      { RESOURCE_JOKER, RESOURCE_JOKER }, 2, RESOURCE_LUXURY, CURVE_F_MINUS, NULL, RESOURCE_JOKER,
      { RESOURCE_FOOD, RESOURCE_GOODS, RESOURCE_BOTS, RESOURCE_ORGANICS, RESOURCE_ORE, RESOURCE_TECH }, 6,
      0, false },

    /* Slipgate. od=10 (pico8/lua.lua:1103); rr="slipgates"
     * (pico8/lua.lua:1108); zero production (nr's own entity definition sets
     * no hp= field at all) -- needs = none, produces = none, curve =
     * CURVE_NONE; max_connections = 3 (design doc S5, #self.oe<3), shared
     * with Infragate; crosses_other_routes = false (no mt=1 on nr). */
    { STRUCTURE_SLIPGATE, "Slipgate", true, 10, "slipgates",
      { 0 }, 0, RESOURCE_NONE, CURVE_NONE, NULL, RESOURCE_NONE,
      { 0 }, 0,
      3, false },

    /* Infragate. od=25 (pico8/lua.lua:1126); rr="infraspace"
     * (pico8/lua.lua:1125); same zero-production status as Slipgate (fr also
     * sets no hp= field); max_connections = 3, same #self.oe<3 rule as
     * Slipgate; crosses_other_routes = true (fr's own mt=1) -- the one field
     * that actually distinguishes it from Slipgate. */
    { STRUCTURE_INFRAGATE, "Infragate", true, 25, "infraspace",
      { 0 }, 0, RESOURCE_NONE, CURVE_NONE, NULL, RESOURCE_NONE,
      { 0 }, 0,
      3, true },
};

static void test_full_structure_roster_matches_prd_table(void) {
    size_t row_count = sizeof(g_prd_structure_table) / sizeof(g_prd_structure_table[0]);
    /* The table itself must have exactly 8 rows -- a missing or duplicated
     * row here would otherwise silently under-cover the roster. */
    T_CHECK(row_count == (size_t)STRUCTURE_COUNT);

    bool seen[STRUCTURE_COUNT];
    for (int i = 0; i < STRUCTURE_COUNT; i++) {
        seen[i] = false;
    }

    for (size_t r = 0; r < row_count; r++) {
        const prd_structure_row_t *row = &g_prd_structure_table[r];

        /* Every row's id must be in range and appear exactly once -- catches
         * both an omitted structure id and an accidentally-duplicated one. */
        T_CHECK(row->id >= 0 && row->id < STRUCTURE_COUNT);
        T_CHECK(!seen[row->id]);
        seen[row->id] = true;

        const structure_t *s = rules_planets_structure(row->id);
        T_CHECK(s != NULL);

        T_CHECK(strcmp(s->name, row->name) == 0);
        T_CHECK(s->purchasable == row->purchasable);
        if (row->purchasable) {
            /* Protostar's cost is deliberately never asserted (see the
             * table's own doc comment and structure_t's doc comment). */
            T_CHECK(s->cost == row->cost);
        }

        T_CHECK((s->required_tech == NULL) == (row->required_tech == NULL));
        if (s->required_tech != NULL && row->required_tech != NULL) {
            T_CHECK(strcmp(s->required_tech, row->required_tech) == 0);
        }

        T_CHECK(s->needs_count == row->needs_count);
        for (int n = 0; n < row->needs_count; n++) {
            T_CHECK(s->needs[n] == row->needs[n]);
        }

        T_CHECK(s->produces == row->produces);
        T_CHECK(s->curve == row->curve);

        T_CHECK((s->tech_gate == NULL) == (row->tech_gate == NULL));
        if (s->tech_gate != NULL && row->tech_gate != NULL) {
            T_CHECK(strcmp(s->tech_gate, row->tech_gate) == 0);
        }

        T_CHECK(s->default_need == row->default_need);

        T_CHECK(s->resolvable_count == row->resolvable_count);
        for (int i = 0; i < row->resolvable_count; i++) {
            T_CHECK(s->resolvable[i] == row->resolvable[i]);
        }

        T_CHECK(s->max_connections == row->max_connections);
        T_CHECK(s->crosses_other_routes == row->crosses_other_routes);
    }

    /* Every one of the 8 structure ids was covered by exactly one row
     * above -- the "seen" bitset itself is what actually proves this (not
     * just the row_count == STRUCTURE_COUNT check above, which alone could
     * pass with a duplicate id masking an omitted one). */
    for (int i = 0; i < STRUCTURE_COUNT; i++) {
        T_CHECK(seen[i]);
    }
}

/* ========================================================================
 * Chunk 7 (docs/prd/triage/psx-structures-and-transforms.md, the FINAL
 * chunk of this PRD): transform_action_t/transform_action_id_t, the 4
 * planet-transform/destroy actions (mechanize, terraform, collapse,
 * harvest), traced to fd's 4 transform-shaped records (unstash 8494).
 *
 * RULES_PLANETS_MAX_TRANSFORM_TYPES correction: the PRD's own "Data model
 * changes needed" section states this constant as 7 ("mechanize's lr, the
 * real traced maximum"), but that is wrong -- re-verified directly against
 * the PRD's own raw fd dump for harvest,
 * o("harvest","efmorxjisg",-20,ii=23,cw="ga",rn=46): "efmorxjisg" is 10
 * distinct single-character planet-type codes (e,f,m,o,r,x,j,i,s,g -- all 10
 * of PLANET_TYPE_COUNT), strictly more than mechanize's own 7-character
 * "mrsoijx". harvest is genuinely applicable to all 10 planet types (matching
 * the PRD's own transform table row: "all 10 (efmorxjisg)"), so the real
 * traced maximum is 10, not 7 -- rules_planets.h defines
 * RULES_PLANETS_MAX_TRANSFORM_TYPES as 10 accordingly (see that header's own
 * doc comment for the full correction). test_harvest_transform_matches_traced_values
 * below doubles as the regression guard for this corrected constant.
 * ======================================================================== */

static void test_transform_count_is_4(void) {
    T_CHECK(TRANSFORM_COUNT == 4);
}

/* mechanize: applicable_types = mineral, remnant, barren, ocean, iceball,
 * jungle, xeno (7: lr="mrsoijx"); cost = 20; required_tech = "replication";
 * result_kind = TRANSFORM_RESULT_PLANET_TYPE, result_planet_type =
 * PLANET_TYPE_FORGEWORLD (go="f"). Forgeworld itself is deliberately absent
 * from applicable_types -- a planet already forgeworld can't "become
 * forgeworld" -- confirmed explicitly by
 * test_mechanize_transform_not_applicable_to_forgeworld below. */
static void test_mechanize_transform_matches_traced_values(void) {
    const transform_action_t *tr = rules_planets_transform(TRANSFORM_MECHANIZE);

    T_CHECK(tr != NULL);
    T_CHECK(strcmp(tr->name, "mechanize") == 0);
    T_CHECK(tr->applicable_types_count == 7);
    T_CHECK(tr->applicable_types[0] == PLANET_TYPE_MINERAL);
    T_CHECK(tr->applicable_types[1] == PLANET_TYPE_REMNANT);
    T_CHECK(tr->applicable_types[2] == PLANET_TYPE_BARREN);
    T_CHECK(tr->applicable_types[3] == PLANET_TYPE_OCEAN);
    T_CHECK(tr->applicable_types[4] == PLANET_TYPE_ICEBALL);
    T_CHECK(tr->applicable_types[5] == PLANET_TYPE_JUNGLE);
    T_CHECK(tr->applicable_types[6] == PLANET_TYPE_XENO);
    T_CHECK(tr->cost == 20);
    T_CHECK(tr->required_tech != NULL);
    T_CHECK(strcmp(tr->required_tech, "replication") == 0);
    T_CHECK(tr->result_kind == TRANSFORM_RESULT_PLANET_TYPE);
    T_CHECK(tr->result_planet_type == PLANET_TYPE_FORGEWORLD);
}

static void test_mechanize_transform_applicable_to_matches_exactly_its_7_real_types(void) {
    static const planet_type_id_t applicable[] = {
        PLANET_TYPE_MINERAL, PLANET_TYPE_REMNANT, PLANET_TYPE_BARREN,
        PLANET_TYPE_OCEAN, PLANET_TYPE_ICEBALL, PLANET_TYPE_JUNGLE, PLANET_TYPE_XENO,
    };

    for (int t = 0; t < PLANET_TYPE_COUNT; t++) {
        bool expected = false;
        for (size_t i = 0; i < sizeof(applicable) / sizeof(applicable[0]); i++) {
            if (applicable[i] == (planet_type_id_t)t) {
                expected = true;
                break;
            }
        }
        T_CHECK(rules_planets_transform_applicable_to(TRANSFORM_MECHANIZE, (planet_type_id_t)t) == expected);
    }

    /* This PRD's own explicitly-named test case: mechanize is NOT applicable
     * to forgeworld -- a planet can't "become forgeworld" by transforming a
     * planet that's already forgeworld, since forgeworld is absent from
     * mechanize's own 7-type applicable_types[] (asserted generically above,
     * called out explicitly here too so this specific case can't silently
     * regress). */
    T_CHECK(rules_planets_transform_applicable_to(TRANSFORM_MECHANIZE, PLANET_TYPE_FORGEWORLD) == false);
}

/* terraform: applicable_types = xeno, iceball, barren, ocean, jungle (5:
 * lr="xisoj"); cost = 20; required_tech = "biome hacking"; result_kind =
 * TRANSFORM_RESULT_PLANET_TYPE, result_planet_type = PLANET_TYPE_EARTHLIKE
 * (go="e"). */
static void test_terraform_transform_matches_traced_values(void) {
    const transform_action_t *tr = rules_planets_transform(TRANSFORM_TERRAFORM);

    T_CHECK(tr != NULL);
    T_CHECK(strcmp(tr->name, "terraform") == 0);
    T_CHECK(tr->applicable_types_count == 5);
    T_CHECK(tr->applicable_types[0] == PLANET_TYPE_XENO);
    T_CHECK(tr->applicable_types[1] == PLANET_TYPE_ICEBALL);
    T_CHECK(tr->applicable_types[2] == PLANET_TYPE_BARREN);
    T_CHECK(tr->applicable_types[3] == PLANET_TYPE_OCEAN);
    T_CHECK(tr->applicable_types[4] == PLANET_TYPE_JUNGLE);
    T_CHECK(tr->cost == 20);
    T_CHECK(tr->required_tech != NULL);
    T_CHECK(strcmp(tr->required_tech, "biome hacking") == 0);
    T_CHECK(tr->result_kind == TRANSFORM_RESULT_PLANET_TYPE);
    T_CHECK(tr->result_planet_type == PLANET_TYPE_EARTHLIKE);
}

static void test_terraform_transform_applicable_to_matches_exactly_its_5_real_types(void) {
    static const planet_type_id_t applicable[] = {
        PLANET_TYPE_XENO, PLANET_TYPE_ICEBALL, PLANET_TYPE_BARREN, PLANET_TYPE_OCEAN, PLANET_TYPE_JUNGLE,
    };

    for (int t = 0; t < PLANET_TYPE_COUNT; t++) {
        bool expected = false;
        for (size_t i = 0; i < sizeof(applicable) / sizeof(applicable[0]); i++) {
            if (applicable[i] == (planet_type_id_t)t) {
                expected = true;
                break;
            }
        }
        T_CHECK(rules_planets_transform_applicable_to(TRANSFORM_TERRAFORM, (planet_type_id_t)t) == expected);
    }
}

/* collapse: applicable_types = gas giant, mineral (2: lr="gm"); cost = 30;
 * required_tech = "starbirth"; result_kind = TRANSFORM_RESULT_PROTOSTAR
 * (cw="pm" -- becomes the Protostar structure, chunk 3's own
 * STRUCTURE_PROTOSTAR). result_planet_type is not meaningful for this
 * result_kind -- deliberately NOT asserted below, per structure_t's own
 * "valid iff result_kind == TRANSFORM_RESULT_PLANET_TYPE" doc comment. */
static void test_collapse_transform_matches_traced_values(void) {
    const transform_action_t *tr = rules_planets_transform(TRANSFORM_COLLAPSE);

    T_CHECK(tr != NULL);
    T_CHECK(strcmp(tr->name, "collapse") == 0);
    T_CHECK(tr->applicable_types_count == 2);
    T_CHECK(tr->applicable_types[0] == PLANET_TYPE_GASGIANT);
    T_CHECK(tr->applicable_types[1] == PLANET_TYPE_MINERAL);
    T_CHECK(tr->cost == 30);
    T_CHECK(tr->required_tech != NULL);
    T_CHECK(strcmp(tr->required_tech, "starbirth") == 0);
    T_CHECK(tr->result_kind == TRANSFORM_RESULT_PROTOSTAR);
    /* result_planet_type deliberately not asserted -- not meaningful here. */
}

static void test_collapse_transform_applicable_to_matches_exactly_its_2_real_types(void) {
    static const planet_type_id_t applicable[] = { PLANET_TYPE_GASGIANT, PLANET_TYPE_MINERAL };

    for (int t = 0; t < PLANET_TYPE_COUNT; t++) {
        bool expected = false;
        for (size_t i = 0; i < sizeof(applicable) / sizeof(applicable[0]); i++) {
            if (applicable[i] == (planet_type_id_t)t) {
                expected = true;
                break;
            }
        }
        T_CHECK(rules_planets_transform_applicable_to(TRANSFORM_COLLAPSE, (planet_type_id_t)t) == expected);
    }
}

/* harvest: applicable_types = ALL 10 planet types (lr="efmorxjisg" -- see
 * this section's own header comment for the RULES_PLANETS_MAX_TRANSFORM_TYPES
 * correction this finding drives); cost = -20 (a genuine refund, asserted
 * exactly, not clamped to non-negative); required_tech = "geoharvesting";
 * result_kind = TRANSFORM_RESULT_DESTROYED (cw="ga" -- planet destroyed, no
 * resulting type). result_planet_type similarly not meaningful -- not
 * asserted. applicable_types_count == 10 (not the PRD's stated 7) is this
 * chunk's regression guard for the corrected RULES_PLANETS_MAX_TRANSFORM_TYPES
 * constant. */
static void test_harvest_transform_matches_traced_values(void) {
    const transform_action_t *tr = rules_planets_transform(TRANSFORM_HARVEST);

    T_CHECK(tr != NULL);
    T_CHECK(strcmp(tr->name, "harvest") == 0);
    /* Regression guard for the corrected RULES_PLANETS_MAX_TRANSFORM_TYPES
     * (10, not the PRD's stated 7) -- harvest's own applicable_types_count is
     * the real traced maximum across all 4 transforms. */
    T_CHECK(tr->applicable_types_count == 10);
    T_CHECK(tr->applicable_types[0] == PLANET_TYPE_EARTHLIKE);
    T_CHECK(tr->applicable_types[1] == PLANET_TYPE_FORGEWORLD);
    T_CHECK(tr->applicable_types[2] == PLANET_TYPE_MINERAL);
    T_CHECK(tr->applicable_types[3] == PLANET_TYPE_OCEAN);
    T_CHECK(tr->applicable_types[4] == PLANET_TYPE_REMNANT);
    T_CHECK(tr->applicable_types[5] == PLANET_TYPE_XENO);
    T_CHECK(tr->applicable_types[6] == PLANET_TYPE_JUNGLE);
    T_CHECK(tr->applicable_types[7] == PLANET_TYPE_ICEBALL);
    T_CHECK(tr->applicable_types[8] == PLANET_TYPE_BARREN);
    T_CHECK(tr->applicable_types[9] == PLANET_TYPE_GASGIANT);
    /* cost == -20 captured as-is: a documented refund, never clamped to
     * non-negative. */
    T_CHECK(tr->cost == -20);
    T_CHECK(tr->required_tech != NULL);
    T_CHECK(strcmp(tr->required_tech, "geoharvesting") == 0);
    T_CHECK(tr->result_kind == TRANSFORM_RESULT_DESTROYED);
    /* result_planet_type deliberately not asserted -- not meaningful here. */
}

/* harvest is applicable to literally every planet type -- exercised
 * generically over the whole PLANET_TYPE_COUNT range, expecting true
 * unconditionally (there is no non-applicable type to sample, unlike the
 * other 3 transforms). */
static void test_harvest_transform_applicable_to_every_planet_type(void) {
    for (int t = 0; t < PLANET_TYPE_COUNT; t++) {
        T_CHECK(rules_planets_transform_applicable_to(TRANSFORM_HARVEST, (planet_type_id_t)t) == true);
    }
}

int main(void) {
    t_run("earthlike_type_matches_traced_values", test_earthlike_type_matches_traced_values);
    t_run("forgeworld_type_matches_traced_values", test_forgeworld_type_matches_traced_values);
    t_run("mineral_type_matches_traced_values", test_mineral_type_matches_traced_values);
    t_run("ocean_type_matches_traced_values", test_ocean_type_matches_traced_values);
    t_run("remnant_type_matches_traced_values", test_remnant_type_matches_traced_values);
    t_run("xeno_type_matches_traced_values", test_xeno_type_matches_traced_values);
    t_run("jungle_type_matches_traced_values", test_jungle_type_matches_traced_values);
    t_run("iceball_type_matches_traced_values", test_iceball_type_matches_traced_values);
    t_run("barren_type_matches_traced_values", test_barren_type_matches_traced_values);
    t_run("gasgiant_type_matches_traced_values", test_gasgiant_type_matches_traced_values);
    t_run("planet_type_count_is_10", test_planet_type_count_is_10);
    t_run("mine_job_matches_traced_values", test_mine_job_matches_traced_values);
    t_run("nanotech_job_matches_traced_values", test_nanotech_job_matches_traced_values);
    t_run("bots_job_matches_traced_values", test_bots_job_matches_traced_values);
    t_run("agroworld_job_matches_traced_values", test_agroworld_job_matches_traced_values);
    t_run("hiveworld_job_matches_traced_values", test_hiveworld_job_matches_traced_values);
    t_run("goods_job_matches_traced_values", test_goods_job_matches_traced_values);
    t_run("gadgets_job_matches_traced_values", test_gadgets_job_matches_traced_values);
    t_run("uplift_job_matches_traced_values", test_uplift_job_matches_traced_values);
    t_run("colony_ocean_job_matches_traced_values", test_colony_ocean_job_matches_traced_values);
    t_run("algae_job_matches_traced_values", test_algae_job_matches_traced_values);
    t_run("breed_ocean_job_matches_traced_values", test_breed_ocean_job_matches_traced_values);
    t_run("strip_job_matches_traced_values", test_strip_job_matches_traced_values);
    t_run("dismantle_job_matches_traced_values", test_dismantle_job_matches_traced_values);
    t_run("scavenge_job_matches_traced_values", test_scavenge_job_matches_traced_values);
    t_run("colony_xeno_job_matches_traced_values", test_colony_xeno_job_matches_traced_values);
    t_run("breed_xeno_job_matches_traced_values", test_breed_xeno_job_matches_traced_values);
    t_run("hunt_job_matches_traced_values", test_hunt_job_matches_traced_values);
    t_run("tourism_job_matches_traced_values", test_tourism_job_matches_traced_values);
    t_run("luxury_job_matches_traced_values", test_luxury_job_matches_traced_values);
    t_run("habitat_job_matches_traced_values", test_habitat_job_matches_traced_values);
    t_run("job_available_on_matches_exactly_its_one_real_type", test_job_available_on_matches_exactly_its_one_real_type);
    t_run("curve_minus_levels_1_to_4_match_traced_values", test_curve_minus_levels_1_to_4_match_traced_values);
    t_run("curve_plus_levels_1_to_4_match_traced_values", test_curve_plus_levels_1_to_4_match_traced_values);
    t_run("curve_value_level_below_1_returns_0", test_curve_value_level_below_1_returns_0);
    t_run("curve_minus_upgraded_returns_curve_plus_values", test_curve_minus_upgraded_returns_curve_plus_values);
    t_run("curve_plus_upgraded_is_a_no_op", test_curve_plus_upgraded_is_a_no_op);
    t_run("curve_value_level_above_4_clamps_to_level_4", test_curve_value_level_above_4_clamps_to_level_4);
    t_run("curve_f_minus_levels_1_to_4_match_traced_values", test_curve_f_minus_levels_1_to_4_match_traced_values);
    t_run("curve_f_minus_level_below_1_returns_0", test_curve_f_minus_level_below_1_returns_0);
    t_run("curve_f_minus_upgraded_returns_curve_f_plus_values", test_curve_f_minus_upgraded_returns_curve_f_plus_values);
    t_run("curve_f_plus_levels_1_to_4_match_traced_values", test_curve_f_plus_levels_1_to_4_match_traced_values);
    t_run("curve_f_plus_upgraded_is_a_no_op", test_curve_f_plus_upgraded_is_a_no_op);
    t_run("curve_qh_levels_1_to_4_match_traced_values", test_curve_qh_levels_1_to_4_match_traced_values);
    t_run("curve_qh_plus_levels_1_to_4_match_traced_values", test_curve_qh_plus_levels_1_to_4_match_traced_values);
    t_run("curve_qh_upgraded_returns_curve_qh_plus_values", test_curve_qh_upgraded_returns_curve_qh_plus_values);
    t_run("curve_qh_plus_upgraded_is_a_no_op", test_curve_qh_plus_upgraded_is_a_no_op);
    t_run("curve_none_returns_0_for_every_level_and_upgraded_state", test_curve_none_returns_0_for_every_level_and_upgraded_state);
    t_run("new_resource_ids_are_distinct", test_new_resource_ids_are_distinct);
    t_run("structure_count_is_8", test_structure_count_is_8);
    t_run("laboratory_structure_matches_traced_values", test_laboratory_structure_matches_traced_values);
    t_run("food_processor_structure_matches_traced_values", test_food_processor_structure_matches_traced_values);
    t_run("laboratory_structure_production_matches_curve_qh_unresearched", test_laboratory_structure_production_matches_curve_qh_unresearched);
    t_run("laboratory_structure_production_switches_to_curve_qh_plus_when_researched", test_laboratory_structure_production_switches_to_curve_qh_plus_when_researched);
    t_run("food_processor_structure_production_matches_curve_f_minus_unresearched", test_food_processor_structure_production_matches_curve_f_minus_unresearched);
    t_run("food_processor_structure_production_switches_to_curve_f_plus_when_researched", test_food_processor_structure_production_switches_to_curve_f_plus_when_researched);
    t_run("ascension_gate_structure_matches_traced_values", test_ascension_gate_structure_matches_traced_values);
    t_run("ascension_gate_structure_production_matches_curve_f_plus_unresearched", test_ascension_gate_structure_production_matches_curve_f_plus_unresearched);
    t_run("ascension_gate_structure_production_identical_when_researched", test_ascension_gate_structure_production_identical_when_researched);
    t_run("protostar_structure_matches_traced_values", test_protostar_structure_matches_traced_values);
    t_run("protostar_structure_production_matches_curve_minus_unresearched", test_protostar_structure_production_matches_curve_minus_unresearched);
    t_run("protostar_structure_production_identical_when_researched", test_protostar_structure_production_identical_when_researched);
    t_run("synthesizer_structure_matches_traced_values", test_synthesizer_structure_matches_traced_values);
    t_run("synthesizer_structure_production_matches_curve_f_minus_unresearched", test_synthesizer_structure_production_matches_curve_f_minus_unresearched);
    t_run("synthesizer_structure_production_identical_when_researched", test_synthesizer_structure_production_identical_when_researched);
    t_run("trading_hub_structure_matches_traced_values", test_trading_hub_structure_matches_traced_values);
    t_run("trading_hub_structure_production_matches_curve_f_minus_unresearched", test_trading_hub_structure_production_matches_curve_f_minus_unresearched);
    t_run("trading_hub_structure_production_identical_when_researched", test_trading_hub_structure_production_identical_when_researched);
    t_run("slipgate_structure_matches_traced_values", test_slipgate_structure_matches_traced_values);
    t_run("slipgate_structure_production_returns_0_for_every_level", test_slipgate_structure_production_returns_0_for_every_level);
    t_run("infragate_structure_matches_traced_values", test_infragate_structure_matches_traced_values);
    t_run("infragate_structure_production_returns_0_for_every_level", test_infragate_structure_production_returns_0_for_every_level);
    t_run("slipgate_and_infragate_crosses_other_routes_differ", test_slipgate_and_infragate_crosses_other_routes_differ);
    t_run("mine_job_cost_unaffected_by_tech_researched", test_mine_job_cost_unaffected_by_tech_researched);
    t_run("agroworld_job_cost_base_case", test_agroworld_job_cost_base_case);
    t_run("agroworld_job_cost_tech_researched_only", test_agroworld_job_cost_tech_researched_only);
    t_run("agroworld_job_cost_nanomaterials_researched_only", test_agroworld_job_cost_nanomaterials_researched_only);
    t_run("agroworld_job_cost_both_flags_true_floors_once_at_end", test_agroworld_job_cost_both_flags_true_floors_once_at_end);
    t_run("mine_job_production_matches_curve_minus_unresearched", test_mine_job_production_matches_curve_minus_unresearched);
    t_run("nanotech_job_production_matches_curve_minus_unresearched", test_nanotech_job_production_matches_curve_minus_unresearched);
    t_run("bots_job_production_matches_curve_plus_unresearched", test_bots_job_production_matches_curve_plus_unresearched);
    t_run("agroworld_job_production_matches_curve_minus_unresearched", test_agroworld_job_production_matches_curve_minus_unresearched);
    t_run("hiveworld_job_production_matches_curve_plus_unresearched", test_hiveworld_job_production_matches_curve_plus_unresearched);
    t_run("agroworld_job_production_switches_to_curve_plus_when_tech_researched", test_agroworld_job_production_switches_to_curve_plus_when_tech_researched);
    t_run("every_need_is_produced_by_another_job", test_every_need_is_produced_by_another_job);
    t_run("every_produce_is_needed_by_another_job", test_every_produce_is_needed_by_another_job);
    t_run("goods_job_cost_unaffected_by_tech_researched", test_goods_job_cost_unaffected_by_tech_researched);
    t_run("goods_job_cost_nanomaterials_researched", test_goods_job_cost_nanomaterials_researched);
    t_run("goods_job_production_matches_curve_plus", test_goods_job_production_matches_curve_plus);
    t_run("gadgets_job_cost_unaffected_by_tech_researched", test_gadgets_job_cost_unaffected_by_tech_researched);
    t_run("gadgets_job_cost_nanomaterials_researched", test_gadgets_job_cost_nanomaterials_researched);
    t_run("gadgets_job_production_matches_curve_plus", test_gadgets_job_production_matches_curve_plus);
    t_run("uplift_job_cost_unaffected_by_tech_researched", test_uplift_job_cost_unaffected_by_tech_researched);
    t_run("uplift_job_cost_nanomaterials_researched", test_uplift_job_cost_nanomaterials_researched);
    t_run("uplift_job_production_matches_curve_minus", test_uplift_job_production_matches_curve_minus);
    t_run("colony_ocean_job_cost_base_case", test_colony_ocean_job_cost_base_case);
    t_run("colony_ocean_job_cost_tech_researched_only", test_colony_ocean_job_cost_tech_researched_only);
    t_run("colony_ocean_job_cost_nanomaterials_researched_only", test_colony_ocean_job_cost_nanomaterials_researched_only);
    t_run("colony_ocean_job_cost_both_flags_true_floors_once_at_end", test_colony_ocean_job_cost_both_flags_true_floors_once_at_end);
    t_run("colony_ocean_job_production_matches_curve_minus_unresearched", test_colony_ocean_job_production_matches_curve_minus_unresearched);
    t_run("colony_ocean_job_production_switches_to_curve_plus_when_tech_researched", test_colony_ocean_job_production_switches_to_curve_plus_when_tech_researched);
    t_run("algae_job_cost_base_case", test_algae_job_cost_base_case);
    t_run("algae_job_cost_tech_researched_only", test_algae_job_cost_tech_researched_only);
    t_run("algae_job_cost_nanomaterials_researched_only", test_algae_job_cost_nanomaterials_researched_only);
    t_run("algae_job_cost_both_flags_true_floors_once_at_end", test_algae_job_cost_both_flags_true_floors_once_at_end);
    t_run("algae_job_production_matches_curve_minus_unresearched", test_algae_job_production_matches_curve_minus_unresearched);
    t_run("algae_job_production_switches_to_curve_plus_when_tech_researched", test_algae_job_production_switches_to_curve_plus_when_tech_researched);
    t_run("breed_ocean_job_cost_unaffected_by_tech_researched", test_breed_ocean_job_cost_unaffected_by_tech_researched);
    t_run("breed_ocean_job_cost_nanomaterials_researched", test_breed_ocean_job_cost_nanomaterials_researched);
    t_run("breed_ocean_job_production_matches_curve_plus", test_breed_ocean_job_production_matches_curve_plus);
    t_run("strip_job_cost_unaffected_by_tech_researched", test_strip_job_cost_unaffected_by_tech_researched);
    t_run("strip_job_cost_nanomaterials_researched", test_strip_job_cost_nanomaterials_researched);
    t_run("strip_job_production_matches_curve_minus", test_strip_job_production_matches_curve_minus);
    t_run("dismantle_job_cost_unaffected_by_tech_researched", test_dismantle_job_cost_unaffected_by_tech_researched);
    t_run("dismantle_job_cost_nanomaterials_researched", test_dismantle_job_cost_nanomaterials_researched);
    t_run("dismantle_job_production_matches_curve_minus", test_dismantle_job_production_matches_curve_minus);
    t_run("scavenge_job_cost_unaffected_by_tech_researched", test_scavenge_job_cost_unaffected_by_tech_researched);
    t_run("scavenge_job_cost_nanomaterials_researched", test_scavenge_job_cost_nanomaterials_researched);
    t_run("scavenge_job_production_matches_curve_minus", test_scavenge_job_production_matches_curve_minus);
    t_run("colony_xeno_job_cost_base_case", test_colony_xeno_job_cost_base_case);
    t_run("colony_xeno_job_cost_tech_researched_only", test_colony_xeno_job_cost_tech_researched_only);
    t_run("colony_xeno_job_cost_nanomaterials_researched_only", test_colony_xeno_job_cost_nanomaterials_researched_only);
    t_run("colony_xeno_job_cost_both_flags_true_floors_once_at_end", test_colony_xeno_job_cost_both_flags_true_floors_once_at_end);
    t_run("colony_xeno_job_production_matches_curve_minus_unresearched", test_colony_xeno_job_production_matches_curve_minus_unresearched);
    t_run("colony_xeno_job_production_switches_to_curve_plus_when_tech_researched", test_colony_xeno_job_production_switches_to_curve_plus_when_tech_researched);
    t_run("breed_xeno_job_cost_unaffected_by_tech_researched", test_breed_xeno_job_cost_unaffected_by_tech_researched);
    t_run("breed_xeno_job_cost_nanomaterials_researched", test_breed_xeno_job_cost_nanomaterials_researched);
    t_run("breed_xeno_job_production_matches_curve_minus", test_breed_xeno_job_production_matches_curve_minus);
    t_run("hunt_job_cost_unaffected_by_tech_researched", test_hunt_job_cost_unaffected_by_tech_researched);
    t_run("hunt_job_cost_nanomaterials_researched", test_hunt_job_cost_nanomaterials_researched);
    t_run("hunt_job_production_matches_curve_minus", test_hunt_job_production_matches_curve_minus);
    t_run("tourism_job_cost_unaffected_by_tech_researched", test_tourism_job_cost_unaffected_by_tech_researched);
    t_run("tourism_job_cost_nanomaterials_researched", test_tourism_job_cost_nanomaterials_researched);
    t_run("tourism_job_production_matches_curve_f_minus_regardless_of_tech_researched", test_tourism_job_production_matches_curve_f_minus_regardless_of_tech_researched);
    t_run("luxury_job_cost_unaffected_by_tech_researched", test_luxury_job_cost_unaffected_by_tech_researched);
    t_run("luxury_job_cost_nanomaterials_researched", test_luxury_job_cost_nanomaterials_researched);
    t_run("luxury_job_production_matches_curve_f_minus_regardless_of_tech_researched", test_luxury_job_production_matches_curve_f_minus_regardless_of_tech_researched);
    t_run("genefarm_job_matches_traced_values", test_genefarm_job_matches_traced_values);
    t_run("excavate_mineral_job_matches_traced_values", test_excavate_mineral_job_matches_traced_values);
    t_run("excavate_iceball_gasgiant_job_matches_traced_values", test_excavate_iceball_gasgiant_job_matches_traced_values);
    t_run("excavate_mineral_and_iceball_gasgiant_are_genuinely_distinct", test_excavate_mineral_and_iceball_gasgiant_are_genuinely_distinct);
    t_run("farm_job_matches_traced_values", test_farm_job_matches_traced_values);
    t_run("multi_type_job_available_on_matches_exactly_its_two_real_types", test_multi_type_job_available_on_matches_exactly_its_two_real_types);
    t_run("habitat_job_cost_unaffected_by_tech_researched", test_habitat_job_cost_unaffected_by_tech_researched);
    t_run("habitat_job_cost_nanomaterials_researched", test_habitat_job_cost_nanomaterials_researched);
    t_run("habitat_job_production_matches_curve_minus", test_habitat_job_production_matches_curve_minus);
    t_run("genefarm_job_cost_unaffected_by_tech_researched", test_genefarm_job_cost_unaffected_by_tech_researched);
    t_run("genefarm_job_cost_nanomaterials_researched", test_genefarm_job_cost_nanomaterials_researched);
    t_run("genefarm_job_production_matches_curve_minus", test_genefarm_job_production_matches_curve_minus);
    t_run("excavate_mineral_job_cost_unaffected_by_tech_researched", test_excavate_mineral_job_cost_unaffected_by_tech_researched);
    t_run("excavate_mineral_job_cost_nanomaterials_researched", test_excavate_mineral_job_cost_nanomaterials_researched);
    t_run("excavate_mineral_job_production_matches_curve_plus", test_excavate_mineral_job_production_matches_curve_plus);
    t_run("excavate_iceball_gasgiant_job_cost_unaffected_by_tech_researched", test_excavate_iceball_gasgiant_job_cost_unaffected_by_tech_researched);
    t_run("excavate_iceball_gasgiant_job_cost_nanomaterials_researched", test_excavate_iceball_gasgiant_job_cost_nanomaterials_researched);
    t_run("excavate_iceball_gasgiant_job_production_matches_curve_minus", test_excavate_iceball_gasgiant_job_production_matches_curve_minus);
    t_run("farm_job_cost_unaffected_by_tech_researched", test_farm_job_cost_unaffected_by_tech_researched);
    t_run("farm_job_cost_nanomaterials_researched", test_farm_job_cost_nanomaterials_researched);
    t_run("farm_job_production_matches_curve_minus", test_farm_job_production_matches_curve_minus);
    t_run("job_count_is_24", test_job_count_is_24);
    t_run("full_roster_sanity_guard", test_full_roster_sanity_guard);
    t_run("full_roster_matches_prd_table", test_full_roster_matches_prd_table);
    t_run("full_roster_is_not_closed_loop", test_full_roster_is_not_closed_loop);
    t_run("full_structure_roster_matches_prd_table", test_full_structure_roster_matches_prd_table);
    t_run("transform_count_is_4", test_transform_count_is_4);
    t_run("mechanize_transform_matches_traced_values", test_mechanize_transform_matches_traced_values);
    t_run("mechanize_transform_applicable_to_matches_exactly_its_7_real_types", test_mechanize_transform_applicable_to_matches_exactly_its_7_real_types);
    t_run("terraform_transform_matches_traced_values", test_terraform_transform_matches_traced_values);
    t_run("terraform_transform_applicable_to_matches_exactly_its_5_real_types", test_terraform_transform_applicable_to_matches_exactly_its_5_real_types);
    t_run("collapse_transform_matches_traced_values", test_collapse_transform_matches_traced_values);
    t_run("collapse_transform_applicable_to_matches_exactly_its_2_real_types", test_collapse_transform_applicable_to_matches_exactly_its_2_real_types);
    t_run("harvest_transform_matches_traced_values", test_harvest_transform_matches_traced_values);
    t_run("harvest_transform_applicable_to_every_planet_type", test_harvest_transform_applicable_to_every_planet_type);
    return t_report();
}
