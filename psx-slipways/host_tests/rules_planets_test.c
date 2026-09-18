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

/* CURVE_F_MINUS has no upgrade target in this module's scope (neither
 * tourism nor luxury carries an il field) -- upgraded=true must be a
 * documented no-op, returning the exact same values as upgraded=false, not
 * some garbage/uninitialized/wrongly-mapped value. */
static void test_curve_f_minus_upgraded_is_a_no_op(void) {
    for (int level = 1; level <= 4; level++) {
        T_CHECK(rules_planets_curve_value(CURVE_F_MINUS, level, true) ==
                rules_planets_curve_value(CURVE_F_MINUS, level, false));
    }
    T_CHECK(rules_planets_curve_value(CURVE_F_MINUS, 1, true) == 0);
    T_CHECK(rules_planets_curve_value(CURVE_F_MINUS, 2, true) == 1);
    T_CHECK(rules_planets_curve_value(CURVE_F_MINUS, 3, true) == 2);
    T_CHECK(rules_planets_curve_value(CURVE_F_MINUS, 4, true) == 3);
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
    t_run("curve_f_minus_upgraded_is_a_no_op", test_curve_f_minus_upgraded_is_a_no_op);
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
    return t_report();
}
