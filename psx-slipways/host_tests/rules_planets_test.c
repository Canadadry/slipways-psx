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
    t_run("mine_job_cost_unaffected_by_tech_researched", test_mine_job_cost_unaffected_by_tech_researched);
    t_run("agroworld_job_cost_base_case", test_agroworld_job_cost_base_case);
    t_run("agroworld_job_cost_tech_researched_only", test_agroworld_job_cost_tech_researched_only);
    t_run("agroworld_job_cost_nanomaterials_researched_only", test_agroworld_job_cost_nanomaterials_researched_only);
    t_run("agroworld_job_cost_both_flags_true_floors_once_at_end", test_agroworld_job_cost_both_flags_true_floors_once_at_end);
    return t_report();
}
