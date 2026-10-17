#include "devtools/situation.h"

#include <assert.h>
#include <string.h>

static situation_snapshot_t title_screen(void) {
    situation_snapshot_t snapshot;
    memset(&snapshot, 0, sizeof snapshot);
    snapshot.mode = PAD_METRICS_MODE_TITLE;
    return snapshot;
}

static void test_the_first_frame_of_a_title_start_is_title(void) {
    situation_snapshot_t start = title_screen();
    situation_snapshot_t after = title_screen();
    situation_t first = situation_classify(&start, &after, situation_start(&start));
    assert(first == SITUATION_TITLE);
    assert(strcmp(situation_name(first), "title") == 0);
    assert(situation_family(first) == SITUATION_STEADY);
}

static situation_snapshot_t on_screen(pad_metrics_mode_t mode) {
    situation_snapshot_t snapshot = title_screen();
    snapshot.mode = mode;
    snapshot.has_economy = mode != PAD_METRICS_MODE_TITLE;
    snapshot.year = 1;
    return snapshot;
}

static situation_t settled_frame_on(const situation_snapshot_t *snapshot) {
    return situation_classify(snapshot, snapshot, SITUATION_MAP_REST);
}

static void test_a_frame_with_nothing_happening_is_named_by_its_screen(void) {
    static const struct {
        pad_metrics_mode_t mode;
        situation_t expected;
        const char *name;
    } CASES[] = {
        { PAD_METRICS_MODE_TITLE, SITUATION_TITLE, "title" },
        { PAD_METRICS_MODE_PAUSE, SITUATION_PAUSE, "pause" },
        { PAD_METRICS_MODE_PLACEMENT, SITUATION_BUILD, "build" },
        { PAD_METRICS_MODE_BAR, SITUATION_BUILD, "build" },
        { PAD_METRICS_MODE_TECH, SITUATION_TECH_PANEL, "tech-panel" },
        { PAD_METRICS_MODE_SCORE, SITUATION_SCORE_PANEL, "score-panel" },
        { PAD_METRICS_MODE_MENU, SITUATION_MENU, "menu" },
        { PAD_METRICS_MODE_SLIPWAY, SITUATION_SLIPWAY_AIM, "slipway-aim" },
        { PAD_METRICS_MODE_JUMP, SITUATION_MAP_REST, "map-rest" },
        { PAD_METRICS_MODE_AIM, SITUATION_MAP_REST, "map-rest" },
    };
    for (size_t i = 0; i < sizeof CASES / sizeof CASES[0]; i++) {
        situation_snapshot_t snapshot = on_screen(CASES[i].mode);
        situation_t label = settled_frame_on(&snapshot);
        assert(label == CASES[i].expected);
        assert(strcmp(situation_name(label), CASES[i].name) == 0);
        assert(situation_family(label) == SITUATION_STEADY);
        assert(strcmp(situation_family_name(situation_family(label)), "steady") == 0);
    }
}

static void test_the_attract_demo_is_attract_whatever_its_screen(void) {
    situation_snapshot_t snapshot = on_screen(PAD_METRICS_MODE_JUMP);
    snapshot.attract = 1;
    assert(settled_frame_on(&snapshot) == SITUATION_ATTRACT);
    snapshot = title_screen();
    snapshot.attract = 1;
    assert(settled_frame_on(&snapshot) == SITUATION_ATTRACT);
}

static const entity_id_t PLANET_A = { 3, 1 };
static const entity_id_t PLANET_B = { 4, 1 };

static void test_the_map_moves_when_the_camera_or_the_selection_moves(void) {
    situation_snapshot_t before = on_screen(PAD_METRICS_MODE_JUMP);
    before.hovered = PLANET_A;
    before.selected = PLANET_A;
    situation_snapshot_t after = before;
    after.camera_x = 1;
    assert(situation_classify(&before, &after, SITUATION_MAP_REST) == SITUATION_MAP_MOVE);
    after = before;
    after.camera_y = -1;
    assert(situation_classify(&before, &after, SITUATION_MAP_REST) == SITUATION_MAP_MOVE);
    after = before;
    after.hovered = PLANET_B;
    assert(situation_classify(&before, &after, SITUATION_MAP_REST) == SITUATION_MAP_MOVE);
    after = before;
    after.selected = PLANET_B;
    assert(situation_classify(&before, &after, SITUATION_MAP_REST) == SITUATION_MAP_MOVE);
    before.mode = after.mode = PAD_METRICS_MODE_AIM;
    assert(situation_classify(&before, &after, SITUATION_MAP_REST) == SITUATION_MAP_MOVE);
}

static void test_a_moving_camera_off_the_map_keeps_the_screen_label(void) {
    situation_snapshot_t before = on_screen(PAD_METRICS_MODE_MENU);
    situation_snapshot_t after = before;
    after.camera_x = 5;
    after.hovered = PLANET_B;
    assert(situation_classify(&before, &after, SITUATION_MENU) == SITUATION_MENU);
}

static situation_snapshot_t on_map(void) {
    situation_snapshot_t snapshot = on_screen(PAD_METRICS_MODE_JUMP);
    snapshot.colonised = 2;
    snapshot.slipways = 1;
    snapshot.structures = 1;
    snapshot.techs = 1;
    snapshot.probes = 3;
    return snapshot;
}

static void assert_one_off(const situation_snapshot_t *before, const situation_snapshot_t *after,
                           situation_t expected, const char *name) {
    situation_t label = situation_classify(before, after, SITUATION_MAP_REST);
    assert(label == expected);
    assert(strcmp(situation_name(label), name) == 0);
    assert(situation_family(label) == SITUATION_ONE_OFF);
    assert(strcmp(situation_family_name(situation_family(label)), "one-off") == 0);
}

static void test_each_event_alone_labels_its_frame(void) {
    situation_snapshot_t title = title_screen();
    situation_snapshot_t map = on_map();
    assert_one_off(&title, &map, SITUATION_GALAXY_GENERATION, "galaxy-generation");
    assert_one_off(&map, &title, SITUATION_RETURN_TO_TITLE, "return-to-title");
    situation_snapshot_t after = map;
    after.game_ended = 1;
    assert_one_off(&map, &after, SITUATION_GAME_END, "game-end");
    after = map;
    after.year++;
    assert_one_off(&map, &after, SITUATION_NEW_YEAR, "new-year");
    after = map;
    after.techs++;
    assert_one_off(&map, &after, SITUATION_RESEARCH, "research");
    after = map;
    after.colonised++;
    assert_one_off(&map, &after, SITUATION_COLONISE, "colonise");
    after = map;
    after.structures++;
    assert_one_off(&map, &after, SITUATION_STRUCTURE_BUILT, "structure-built");
    after = map;
    after.slipways++;
    assert_one_off(&map, &after, SITUATION_SLIPWAY_BUILT, "slipway-built");
    after = map;
    after.probes++;
    assert_one_off(&map, &after, SITUATION_PROBE, "probe");
    after = map;
    after.probes--;
    assert_one_off(&map, &after, SITUATION_PROBE, "probe");
    after = map;
    after.mode = PAD_METRICS_MODE_MENU;
    assert_one_off(&map, &after, SITUATION_SCREEN_CHANGE, "screen-change");
    assert_one_off(&after, &map, SITUATION_SCREEN_CHANGE, "screen-change");
    after = map;
    after.attract = 1;
    assert_one_off(&map, &after, SITUATION_SCREEN_CHANGE, "screen-change");
    assert_one_off(&after, &map, SITUATION_SCREEN_CHANGE, "screen-change");
}

static void test_counts_that_fall_or_an_end_that_stays_are_no_event(void) {
    situation_snapshot_t before = on_map();
    before.game_ended = 1;
    situation_snapshot_t after = before;
    after.year--;
    after.techs--;
    after.colonised--;
    after.structures--;
    after.slipways--;
    assert(situation_classify(&before, &after, SITUATION_MAP_REST) == SITUATION_MAP_REST);
    after = before;
    after.game_ended = 0;
    assert(situation_classify(&before, &after, SITUATION_MAP_REST) == SITUATION_MAP_REST);
}

static void test_the_highest_priority_event_wins(void) {
    situation_snapshot_t map = on_map();
    situation_snapshot_t all = map;
    all.mode = PAD_METRICS_MODE_MENU;
    all.probes++;
    assert(situation_classify(&map, &all, SITUATION_MAP_REST) == SITUATION_PROBE);
    all.slipways++;
    assert(situation_classify(&map, &all, SITUATION_MAP_REST) == SITUATION_SLIPWAY_BUILT);
    all.structures++;
    assert(situation_classify(&map, &all, SITUATION_MAP_REST) == SITUATION_STRUCTURE_BUILT);
    all.colonised++;
    assert(situation_classify(&map, &all, SITUATION_MAP_REST) == SITUATION_COLONISE);
    all.techs++;
    assert(situation_classify(&map, &all, SITUATION_MAP_REST) == SITUATION_RESEARCH);
    all.year++;
    assert(situation_classify(&map, &all, SITUATION_MAP_REST) == SITUATION_NEW_YEAR);
    all.game_ended = 1;
    assert(situation_classify(&map, &all, SITUATION_MAP_REST) == SITUATION_GAME_END);
    situation_snapshot_t title = title_screen();
    assert(situation_classify(&title, &all, SITUATION_TITLE) == SITUATION_GALAXY_GENERATION);
    title.attract = 1;
    assert(situation_classify(&all, &title, SITUATION_GAME_END) == SITUATION_RETURN_TO_TITLE);
}

static void test_a_new_year_with_a_screen_change_is_a_new_year(void) {
    situation_snapshot_t before = on_map();
    situation_snapshot_t after = before;
    after.year++;
    after.mode = PAD_METRICS_MODE_PAUSE;
    assert(situation_classify(&before, &after, SITUATION_MAP_REST) == SITUATION_NEW_YEAR);
}

static situation_snapshot_t menu_with_item_at(int x) {
    situation_snapshot_t snapshot = on_map();
    snapshot.mode = PAD_METRICS_MODE_MENU;
    snapshot.layout.count = 2;
    snapshot.layout.items[0] = PLANET_A;
    snapshot.layout.items[1] = PLANET_B;
    snapshot.layout.x[0] = x;
    snapshot.layout.x[1] = x + 10;
    snapshot.layout.y[1] = 8;
    return snapshot;
}

static void test_an_event_continues_while_its_panel_layout_moves(void) {
    situation_snapshot_t map = on_map();
    situation_snapshot_t frames[] = { menu_with_item_at(40), menu_with_item_at(30), menu_with_item_at(20),
                                      menu_with_item_at(20) };
    situation_t label = situation_classify(&map, &frames[0], SITUATION_MAP_REST);
    assert(label == SITUATION_SCREEN_CHANGE);
    label = situation_classify(&frames[0], &frames[1], label);
    assert(label == SITUATION_SCREEN_CHANGE);
    label = situation_classify(&frames[1], &frames[2], label);
    assert(label == SITUATION_SCREEN_CHANGE);
    label = situation_classify(&frames[2], &frames[3], label);
    assert(label == SITUATION_MENU);
}

static void test_any_layout_difference_keeps_the_continuation(void) {
    situation_snapshot_t before = menu_with_item_at(20);
    situation_snapshot_t after = before;
    after.layout.y[1]++;
    assert(situation_classify(&before, &after, SITUATION_RESEARCH) == SITUATION_RESEARCH);
    after = before;
    after.layout.items[1] = PLANET_A;
    assert(situation_classify(&before, &after, SITUATION_RESEARCH) == SITUATION_RESEARCH);
    after = before;
    after.layout.count = 1;
    assert(situation_classify(&before, &after, SITUATION_RESEARCH) == SITUATION_RESEARCH);
    after = before;
    after.layout.x[1 + 1] = 99;
    assert(situation_classify(&before, &after, SITUATION_RESEARCH) == SITUATION_MENU);
}

static void test_an_event_continues_while_a_shake_runs(void) {
    situation_snapshot_t before = on_map();
    before.shaking = 1;
    situation_snapshot_t after = before;
    assert(situation_classify(&before, &after, SITUATION_COLONISE) == SITUATION_COLONISE);
    after.shaking = 0;
    assert(situation_classify(&before, &after, SITUATION_COLONISE) == SITUATION_MAP_REST);
}

static void test_a_steady_frame_never_continues(void) {
    situation_snapshot_t before = menu_with_item_at(40);
    situation_snapshot_t after = menu_with_item_at(30);
    after.shaking = 1;
    assert(situation_classify(&before, &after, SITUATION_MENU) == SITUATION_MENU);
    before = on_map();
    after = before;
    after.shaking = 1;
    after.camera_x = 2;
    assert(situation_classify(&before, &after, SITUATION_MAP_MOVE) == SITUATION_MAP_MOVE);
}

static void test_a_new_event_during_a_continuation_takes_over(void) {
    situation_snapshot_t before = menu_with_item_at(40);
    situation_snapshot_t after = menu_with_item_at(30);
    after.year++;
    assert(situation_classify(&before, &after, SITUATION_SCREEN_CHANGE) == SITUATION_NEW_YEAR);
}

static void test_the_first_frame_of_a_resume_start_is_judged_against_the_resumed_game(void) {
    situation_snapshot_t start = on_map();
    start.shaking = 1;
    situation_t previous = situation_start(&start);
    assert(previous == SITUATION_MAP_REST);
    situation_snapshot_t after = start;
    assert(situation_classify(&start, &after, previous) == SITUATION_MAP_REST);
    after.camera_x = 4;
    assert(situation_classify(&start, &after, previous) == SITUATION_MAP_MOVE);
    after = start;
    after.colonised++;
    assert(situation_classify(&start, &after, previous) == SITUATION_COLONISE);
}

static void test_a_resume_into_an_open_panel_starts_on_that_panel(void) {
    situation_snapshot_t start = on_screen(PAD_METRICS_MODE_TECH);
    assert(situation_start(&start) == SITUATION_TECH_PANEL);
    situation_snapshot_t after = start;
    after.layout.count = 1;
    assert(situation_classify(&start, &after, situation_start(&start)) == SITUATION_TECH_PANEL);
}

static void test_every_situation_has_its_own_name_and_ten_are_steady(void) {
    int steady = 0;
    for (int i = 0; i < SITUATION_COUNT; i++) {
        assert(situation_name((situation_t)i)[0] != '\0');
        for (int j = 0; j < i; j++) assert(strcmp(situation_name((situation_t)i), situation_name((situation_t)j)) != 0);
        steady += situation_family((situation_t)i) == SITUATION_STEADY;
    }
    assert(steady == 10);
}

int main(void) {
    test_the_first_frame_of_a_title_start_is_title();
    test_a_frame_with_nothing_happening_is_named_by_its_screen();
    test_the_attract_demo_is_attract_whatever_its_screen();
    test_the_map_moves_when_the_camera_or_the_selection_moves();
    test_a_moving_camera_off_the_map_keeps_the_screen_label();
    test_each_event_alone_labels_its_frame();
    test_counts_that_fall_or_an_end_that_stays_are_no_event();
    test_the_highest_priority_event_wins();
    test_a_new_year_with_a_screen_change_is_a_new_year();
    test_an_event_continues_while_its_panel_layout_moves();
    test_any_layout_difference_keeps_the_continuation();
    test_an_event_continues_while_a_shake_runs();
    test_a_steady_frame_never_continues();
    test_a_new_event_during_a_continuation_takes_over();
    test_the_first_frame_of_a_resume_start_is_judged_against_the_resumed_game();
    test_a_resume_into_an_open_panel_starts_on_that_panel();
    test_every_situation_has_its_own_name_and_ten_are_steady();
    return 0;
}
