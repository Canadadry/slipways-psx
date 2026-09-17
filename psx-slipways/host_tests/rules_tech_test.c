#include "t.h"
#include "../rules_tech.h"

#include <string.h>

static void test_space_folding_matches_traced_values(void) {
    const tech_t *t = rules_tech_get(TECH_SPACE_FOLDING);

    T_CHECK(t != NULL);
    T_CHECK(strcmp(t->name, "space folding") == 0);
    T_CHECK(t->base_cost == 4);
}

static void test_geneseeds_matches_traced_values(void) {
    const tech_t *t = rules_tech_get(TECH_GENESEEDS);

    T_CHECK(t != NULL);
    T_CHECK(strcmp(t->name, "geneseeds") == 0);
    T_CHECK(t->base_cost == 5);
}

static void test_nanomaterials_matches_traced_values(void) {
    const tech_t *t = rules_tech_get(TECH_NANOMATERIALS);

    T_CHECK(t != NULL);
    T_CHECK(strcmp(t->name, "nanomaterials") == 0);
    T_CHECK(t->base_cost == 6);
}

static void test_drillbots_matches_traced_values(void) {
    const tech_t *t = rules_tech_get(TECH_DRILLBOTS);

    T_CHECK(t != NULL);
    T_CHECK(strcmp(t->name, "drillbots") == 0);
    T_CHECK(t->base_cost == 7);
}

static void test_select_press_edge_opens_closed_menu(void) {
    uint16_t prev_buttons = 0xffff;
    uint16_t buttons = (uint16_t) ~RULES_TECH_PAD_SELECT;

    int open = rules_tech_step(0, prev_buttons, buttons);

    T_CHECK(open == 1);
}

static void test_select_press_edge_closes_open_menu(void) {
    uint16_t prev_buttons = 0xffff;
    uint16_t buttons = (uint16_t) ~RULES_TECH_PAD_SELECT;

    int open = rules_tech_step(1, prev_buttons, buttons);

    T_CHECK(open == 0);
}

static void test_holding_select_toggles_only_once(void) {
    uint16_t prev_buttons = 0xffff;
    uint16_t held = (uint16_t) ~RULES_TECH_PAD_SELECT;

    /* Frame 1: press edge -- toggles closed(0) -> open(1). */
    int open = rules_tech_step(0, prev_buttons, held);
    T_CHECK(open == 1);

    /* Frame 2: still held, no new edge -- stays open. */
    open = rules_tech_step(open, held, held);
    T_CHECK(open == 1);

    /* Frame 3: still held, no new edge -- stays open. */
    open = rules_tech_step(open, held, held);
    T_CHECK(open == 1);
}

static void test_no_edge_leaves_open_unchanged(void) {
    uint16_t buttons = 0xffff;

    int open = rules_tech_step(0, buttons, buttons);
    T_CHECK(open == 0);

    open = rules_tech_step(1, buttons, buttons);
    T_CHECK(open == 1);
}

static void test_release_edge_does_not_toggle(void) {
    uint16_t held = (uint16_t) ~RULES_TECH_PAD_SELECT;
    uint16_t released = 0xffff;

    /* Menu is open; Select transitions from pressed to released this
     * frame -- must not toggle (only the press edge matters). */
    int open = rules_tech_step(1, held, released);
    T_CHECK(open == 1);

    /* Same check starting from closed. */
    open = rules_tech_step(0, held, released);
    T_CHECK(open == 0);
}

static void test_update_get_state_round_trip_matches_step(void) {
    rules_tech_init();

    T_CHECK(rules_tech_get_state().open == 0);

    /* Frame 1: Select press edge -- expect closed(0) -> open(1), matching
     * rules_tech_step() called directly with the same inputs. */
    uint16_t prev_buttons = 0xffff;
    uint16_t buttons = (uint16_t) ~RULES_TECH_PAD_SELECT;
    int expected = rules_tech_step(0, prev_buttons, buttons);

    rules_tech_update(buttons);
    T_CHECK(rules_tech_get_state().open == expected);
    T_CHECK(expected == 1);

    /* Frame 2: Select held, no new edge -- stays open. */
    uint16_t buttons2 = (uint16_t) ~RULES_TECH_PAD_SELECT;
    expected = rules_tech_step(expected, buttons, buttons2);

    rules_tech_update(buttons2);
    T_CHECK(rules_tech_get_state().open == expected);
    T_CHECK(expected == 1);

    /* Frame 3: Select released -- release edge, no toggle -- stays open. */
    uint16_t buttons3 = 0xffff;
    expected = rules_tech_step(expected, buttons2, buttons3);

    rules_tech_update(buttons3);
    T_CHECK(rules_tech_get_state().open == expected);
    T_CHECK(expected == 1);

    /* Frame 4: fresh Select press edge -- open(1) -> closed(0). */
    uint16_t buttons4 = (uint16_t) ~RULES_TECH_PAD_SELECT;
    expected = rules_tech_step(expected, buttons3, buttons4);

    rules_tech_update(buttons4);
    T_CHECK(rules_tech_get_state().open == expected);
    T_CHECK(expected == 0);
}

int main(void) {
    t_run("space_folding_matches_traced_values", test_space_folding_matches_traced_values);
    t_run("geneseeds_matches_traced_values", test_geneseeds_matches_traced_values);
    t_run("nanomaterials_matches_traced_values", test_nanomaterials_matches_traced_values);
    t_run("drillbots_matches_traced_values", test_drillbots_matches_traced_values);
    t_run("select_press_edge_opens_closed_menu", test_select_press_edge_opens_closed_menu);
    t_run("select_press_edge_closes_open_menu", test_select_press_edge_closes_open_menu);
    t_run("holding_select_toggles_only_once", test_holding_select_toggles_only_once);
    t_run("no_edge_leaves_open_unchanged", test_no_edge_leaves_open_unchanged);
    t_run("release_edge_does_not_toggle", test_release_edge_does_not_toggle);
    t_run("update_get_state_round_trip_matches_step", test_update_get_state_round_trip_matches_step);
    return t_report();
}
