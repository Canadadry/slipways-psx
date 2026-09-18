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
    return t_report();
}
