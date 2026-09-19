#include "t.h"
#include "../rules_tech.h"

#include <string.h>

/* Chunk 11 (psx-full-planets-and-tech): the 4 existing Era-1 entries'
 * name/base_cost must stay byte-identical after the table grows to 19 --
 * these are the same tests as before, now also asserting .era == 1 (a
 * regression check, not just a new-data check). Chunk 8
 * (docs/prd/triage/psx-sprites-audio-extraction.md, Implementation
 * Chunks item 8) extends each with a .icon assertion against the same
 * traced values (sprite_atlas.h's SPRITE_RESOURCE_TECH/ORGANICS/ORE/BOTS
 * constants). */
static void test_space_folding_matches_traced_values(void) {
    const tech_t *t = rules_tech_get(TECH_SPACE_FOLDING);

    T_CHECK(t != NULL);
    T_CHECK(strcmp(t->name, "space folding") == 0);
    T_CHECK(t->base_cost == 4);
    T_CHECK(t->era == 1);
    T_CHECK(t->icon == 22);
}

static void test_geneseeds_matches_traced_values(void) {
    const tech_t *t = rules_tech_get(TECH_GENESEEDS);

    T_CHECK(t != NULL);
    T_CHECK(strcmp(t->name, "geneseeds") == 0);
    T_CHECK(t->base_cost == 5);
    T_CHECK(t->era == 1);
    T_CHECK(t->icon == 20);
}

static void test_nanomaterials_matches_traced_values(void) {
    const tech_t *t = rules_tech_get(TECH_NANOMATERIALS);

    T_CHECK(t != NULL);
    T_CHECK(strcmp(t->name, "nanomaterials") == 0);
    T_CHECK(t->base_cost == 6);
    T_CHECK(t->era == 1);
    T_CHECK(t->icon == 21);
}

static void test_drillbots_matches_traced_values(void) {
    const tech_t *t = rules_tech_get(TECH_DRILLBOTS);

    T_CHECK(t != NULL);
    T_CHECK(strcmp(t->name, "drillbots") == 0);
    T_CHECK(t->base_cost == 7);
    T_CHECK(t->era == 1);
    T_CHECK(t->icon == 18);
}

/* Era 2 (chunk 11 new entries). */
static void test_xenofoods_matches_traced_values(void) {
    const tech_t *t = rules_tech_get(TECH_XENOFOODS);

    T_CHECK(t != NULL);
    T_CHECK(strcmp(t->name, "xenofoods") == 0);
    T_CHECK(t->base_cost == 10);
    T_CHECK(t->era == 2);
}

static void test_slipgates_matches_traced_values(void) {
    const tech_t *t = rules_tech_get(TECH_SLIPGATES);

    T_CHECK(t != NULL);
    T_CHECK(strcmp(t->name, "slipgates") == 0);
    T_CHECK(t->base_cost == 12);
    T_CHECK(t->era == 2);
}

static void test_geoharvesting_matches_traced_values(void) {
    const tech_t *t = rules_tech_get(TECH_GEOHARVESTING);

    T_CHECK(t != NULL);
    T_CHECK(strcmp(t->name, "geoharvesting") == 0);
    T_CHECK(t->base_cost == 14);
    T_CHECK(t->era == 2);
}

static void test_trade_league_matches_traced_values(void) {
    const tech_t *t = rules_tech_get(TECH_TRADE_LEAGUE);

    T_CHECK(t != NULL);
    T_CHECK(strcmp(t->name, "trade league") == 0);
    T_CHECK(t->base_cost == 16);
    T_CHECK(t->era == 2);
}

static void test_replication_matches_traced_values(void) {
    const tech_t *t = rules_tech_get(TECH_REPLICATION);

    T_CHECK(t != NULL);
    T_CHECK(strcmp(t->name, "replication") == 0);
    T_CHECK(t->base_cost == 18);
    T_CHECK(t->era == 2);
}

/* Era 3 (chunk 11 new entries). */
static void test_autoassemblers_matches_traced_values(void) {
    const tech_t *t = rules_tech_get(TECH_AUTOASSEMBLERS);

    T_CHECK(t != NULL);
    T_CHECK(strcmp(t->name, "autoassemblers") == 0);
    T_CHECK(t->base_cost == 19);
    T_CHECK(t->era == 3);
}

static void test_skill_implants_matches_traced_values(void) {
    const tech_t *t = rules_tech_get(TECH_SKILL_IMPLANTS);

    T_CHECK(t != NULL);
    T_CHECK(strcmp(t->name, "skill implants") == 0);
    T_CHECK(t->base_cost == 22);
    T_CHECK(t->era == 3);
}

static void test_starbirth_matches_traced_values(void) {
    const tech_t *t = rules_tech_get(TECH_STARBIRTH);

    T_CHECK(t != NULL);
    T_CHECK(strcmp(t->name, "starbirth") == 0);
    T_CHECK(t->base_cost == 25);
    T_CHECK(t->era == 3);
}

static void test_biome_hacking_matches_traced_values(void) {
    const tech_t *t = rules_tech_get(TECH_BIOME_HACKING);

    T_CHECK(t != NULL);
    T_CHECK(strcmp(t->name, "biome hacking") == 0);
    T_CHECK(t->base_cost == 28);
    T_CHECK(t->era == 3);
}

static void test_infraspace_matches_traced_values(void) {
    const tech_t *t = rules_tech_get(TECH_INFRASPACE);

    T_CHECK(t != NULL);
    T_CHECK(strcmp(t->name, "infraspace") == 0);
    T_CHECK(t->base_cost == 31);
    T_CHECK(t->era == 3);
}

/* Era 4 (chunk 11 new entries). */
static void test_superhuman_ai_matches_traced_values(void) {
    const tech_t *t = rules_tech_get(TECH_SUPERHUMAN_AI);

    T_CHECK(t != NULL);
    T_CHECK(strcmp(t->name, "superhuman ai") == 0);
    T_CHECK(t->base_cost == 34);
    T_CHECK(t->era == 4);
}

static void test_void_synthesis_matches_traced_values(void) {
    const tech_t *t = rules_tech_get(TECH_VOID_SYNTHESIS);

    T_CHECK(t != NULL);
    T_CHECK(strcmp(t->name, "void synthesis") == 0);
    T_CHECK(t->base_cost == 38);
    T_CHECK(t->era == 4);
}

static void test_gene_rewriting_matches_traced_values(void) {
    const tech_t *t = rules_tech_get(TECH_GENE_REWRITING);

    T_CHECK(t != NULL);
    T_CHECK(strcmp(t->name, "gene rewriting") == 0);
    T_CHECK(t->base_cost == 42);
    T_CHECK(t->era == 4);
}

static void test_time_compression_matches_traced_values(void) {
    const tech_t *t = rules_tech_get(TECH_TIME_COMPRESSION);

    T_CHECK(t != NULL);
    T_CHECK(strcmp(t->name, "time compression") == 0);
    T_CHECK(t->base_cost == 46);
    T_CHECK(t->era == 4);
}

static void test_ascension_matches_traced_values(void) {
    const tech_t *t = rules_tech_get(TECH_ASCENSION);

    T_CHECK(t != NULL);
    T_CHECK(strcmp(t->name, "ascension") == 0);
    T_CHECK(t->base_cost == 50);
    T_CHECK(t->era == 4);
}

/* Full-roster sanity guard against silent enum drift (chunk 11, mirroring
 * chunk 10's PLANET_TYPE_COUNT/JOB_COUNT guard in rules_planets_test.c). */
static void test_tech_count_is_19(void) {
    T_CHECK(TECH_COUNT == 19);
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
    t_run("xenofoods_matches_traced_values", test_xenofoods_matches_traced_values);
    t_run("slipgates_matches_traced_values", test_slipgates_matches_traced_values);
    t_run("geoharvesting_matches_traced_values", test_geoharvesting_matches_traced_values);
    t_run("trade_league_matches_traced_values", test_trade_league_matches_traced_values);
    t_run("replication_matches_traced_values", test_replication_matches_traced_values);
    t_run("autoassemblers_matches_traced_values", test_autoassemblers_matches_traced_values);
    t_run("skill_implants_matches_traced_values", test_skill_implants_matches_traced_values);
    t_run("starbirth_matches_traced_values", test_starbirth_matches_traced_values);
    t_run("biome_hacking_matches_traced_values", test_biome_hacking_matches_traced_values);
    t_run("infraspace_matches_traced_values", test_infraspace_matches_traced_values);
    t_run("superhuman_ai_matches_traced_values", test_superhuman_ai_matches_traced_values);
    t_run("void_synthesis_matches_traced_values", test_void_synthesis_matches_traced_values);
    t_run("gene_rewriting_matches_traced_values", test_gene_rewriting_matches_traced_values);
    t_run("time_compression_matches_traced_values", test_time_compression_matches_traced_values);
    t_run("ascension_matches_traced_values", test_ascension_matches_traced_values);
    t_run("tech_count_is_19", test_tech_count_is_19);
    t_run("select_press_edge_opens_closed_menu", test_select_press_edge_opens_closed_menu);
    t_run("select_press_edge_closes_open_menu", test_select_press_edge_closes_open_menu);
    t_run("holding_select_toggles_only_once", test_holding_select_toggles_only_once);
    t_run("no_edge_leaves_open_unchanged", test_no_edge_leaves_open_unchanged);
    t_run("release_edge_does_not_toggle", test_release_edge_does_not_toggle);
    t_run("update_get_state_round_trip_matches_step", test_update_get_state_round_trip_matches_step);
    return t_report();
}
