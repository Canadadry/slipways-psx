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

static void test_mine_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_MINE);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_type == PLANET_TYPE_MINERAL);
    T_CHECK(j->cost == 5);
    T_CHECK(j->needs_count == 1);
    T_CHECK(j->needs[0] == RESOURCE_PEOPLE);
    T_CHECK(j->produces == RESOURCE_ORE);
    T_CHECK(j->curve == CURVE_MINUS);
    T_CHECK(j->tech_gate == NULL);
    T_CHECK(j->default_need == RESOURCE_NONE);
}

static void test_nanotech_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_NANOTECH);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_type == PLANET_TYPE_FORGEWORLD);
    T_CHECK(j->cost == 8);
    T_CHECK(j->needs_count == 1);
    T_CHECK(j->needs[0] == RESOURCE_ORE);
    T_CHECK(j->produces == RESOURCE_TECH);
    T_CHECK(j->curve == CURVE_MINUS);
    T_CHECK(j->tech_gate == NULL);
    T_CHECK(j->default_need == RESOURCE_NONE);
}

static void test_bots_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_BOTS);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_type == PLANET_TYPE_FORGEWORLD);
    T_CHECK(j->cost == 8);
    T_CHECK(j->needs_count == 2);
    T_CHECK(j->needs[0] == RESOURCE_ORE);
    T_CHECK(j->needs[1] == RESOURCE_TECH);
    T_CHECK(j->produces == RESOURCE_BOTS);
    T_CHECK(j->curve == CURVE_PLUS);
    T_CHECK(j->tech_gate == NULL);
    T_CHECK(j->default_need == RESOURCE_NONE);
}

static void test_agroworld_job_matches_traced_values(void) {
    const colonization_job_t *j = rules_planets_job(JOB_AGROWORLD);

    T_CHECK(j != NULL);
    T_CHECK(j->planet_type == PLANET_TYPE_EARTHLIKE);
    T_CHECK(j->cost == 5);
    T_CHECK(j->needs_count == 1);
    T_CHECK(j->needs[0] == RESOURCE_BOTS);
    T_CHECK(j->produces == RESOURCE_FOOD);
    T_CHECK(j->curve == CURVE_MINUS);
    T_CHECK(j->tech_gate != NULL);
    T_CHECK(strcmp(j->tech_gate, "xenofoods") == 0);
    T_CHECK(j->default_need == RESOURCE_ORGANICS);
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
    T_CHECK(j->planet_type == PLANET_TYPE_EARTHLIKE);
    T_CHECK(j->cost == 5);
    T_CHECK(j->needs_count == 1);
    T_CHECK(j->needs[0] == RESOURCE_FOOD);
    T_CHECK(j->produces == RESOURCE_PEOPLE);
    T_CHECK(j->curve == CURVE_PLUS);
    T_CHECK(j->tech_gate == NULL);
    T_CHECK(j->default_need == RESOURCE_GOODS);
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

/* Chunk 5: closed-loop structural regression test. Walks the JOB_COUNT-sized
 * g_jobs table generically (via the public rules_planets_job() accessor, not
 * hardcoded pairwise "mine needs PEOPLE, hiveworld produces PEOPLE" checks)
 * and asserts the property the whole PRD's scoping argument depends on: every
 * resource any job needs is produced by some OTHER job in the table, and
 * every resource any job produces is needed by some OTHER job in the table.
 * A future edit that breaks this closure (e.g. changing a job's needs[]
 * without checking downstream effects) must fail here. */
static void test_every_need_is_produced_by_another_job(void) {
    for (int consumer = 0; consumer < JOB_COUNT; consumer++) {
        const colonization_job_t *c = rules_planets_job((colonization_job_id_t)consumer);

        for (int n = 0; n < c->needs_count; n++) {
            resource_id_t needed = c->needs[n];
            bool produced_elsewhere = false;

            for (int producer = 0; producer < JOB_COUNT; producer++) {
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
    for (int producer = 0; producer < JOB_COUNT; producer++) {
        const colonization_job_t *p = rules_planets_job((colonization_job_id_t)producer);
        resource_id_t produced = p->produces;
        bool needed_elsewhere = false;

        for (int consumer = 0; consumer < JOB_COUNT; consumer++) {
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

int main(void) {
    t_run("earthlike_type_matches_traced_values", test_earthlike_type_matches_traced_values);
    t_run("forgeworld_type_matches_traced_values", test_forgeworld_type_matches_traced_values);
    t_run("mineral_type_matches_traced_values", test_mineral_type_matches_traced_values);
    t_run("mine_job_matches_traced_values", test_mine_job_matches_traced_values);
    t_run("nanotech_job_matches_traced_values", test_nanotech_job_matches_traced_values);
    t_run("bots_job_matches_traced_values", test_bots_job_matches_traced_values);
    t_run("agroworld_job_matches_traced_values", test_agroworld_job_matches_traced_values);
    t_run("hiveworld_job_matches_traced_values", test_hiveworld_job_matches_traced_values);
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
    return t_report();
}
