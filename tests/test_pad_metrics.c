#include "devtools/pad_metrics.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static pad_metrics_flags_t game_with_nothing_open(void) {
    pad_metrics_flags_t flags;
    memset(&flags, 0, sizeof flags);
    flags.has_economy = 1;
    return flags;
}

static pad_metrics_flags_t all_flags(void) {
    pad_metrics_flags_t flags = {1, 1, 1, 1, 1, 1, 1, 1, 1};
    return flags;
}

static void test_no_economy_is_the_title_whatever_else_is_set(void) {
    pad_metrics_flags_t flags = all_flags();
    flags.has_economy = 0;
    assert(pad_metrics_classify(&flags) == PAD_METRICS_MODE_TITLE);
}

static void test_a_game_with_nothing_open_is_jump(void) {
    pad_metrics_flags_t flags = game_with_nothing_open();
    assert(pad_metrics_classify(&flags) == PAD_METRICS_MODE_JUMP);
}

enum { RUNGS = 8 };

static const pad_metrics_mode_t RUNG_MODES[RUNGS] = {
    PAD_METRICS_MODE_PAUSE, PAD_METRICS_MODE_PLACEMENT, PAD_METRICS_MODE_BAR, PAD_METRICS_MODE_TECH,
    PAD_METRICS_MODE_SCORE, PAD_METRICS_MODE_MENU,      PAD_METRICS_MODE_AIM, PAD_METRICS_MODE_SLIPWAY,
};

static int *rung_flag(pad_metrics_flags_t *flags, int rung) {
    int *order[RUNGS] = {&flags->pause_shown, &flags->placing,    &flags->build_bar_open, &flags->tech_open,
                         &flags->score_open,  &flags->panel_open, &flags->aim_mode,       &flags->slipway_started};
    return order[rung];
}

static void test_each_rung_alone_gives_its_mode(void) {
    for (int rung = 0; rung < RUNGS; rung++) {
        pad_metrics_flags_t flags = game_with_nothing_open();
        *rung_flag(&flags, rung) = 1;
        assert(pad_metrics_classify(&flags) == RUNG_MODES[rung]);
    }
}

/* With every flag set, clearing them one by one from the top walks the modes in order. */
static void test_the_first_matching_rung_wins(void) {
    pad_metrics_flags_t flags = all_flags();
    for (int rung = 0; rung < RUNGS; rung++) {
        assert(pad_metrics_classify(&flags) == RUNG_MODES[rung]);
        *rung_flag(&flags, rung) = 0;
    }
    assert(pad_metrics_classify(&flags) == PAD_METRICS_MODE_JUMP);
}

static void test_mode_names_keep_the_mode_log_spelling(void) {
    const char *names[PAD_METRICS_MODE_COUNT] = {
        "title", "pause", "placement", "bar", "tech", "score", "menu", "jump", "aim", "slipway",
    };
    for (int m = 0; m < PAD_METRICS_MODE_COUNT; m++) {
        assert(strcmp(pad_metrics_mode_name((pad_metrics_mode_t)m), names[m]) == 0);
    }
}

static pad_metrics_view_t still_view(void) {
    pad_metrics_view_t view;
    memset(&view, 0, sizeof view);
    view.mode = PAD_METRICS_MODE_JUMP;
    view.hovered = (entity_id_t){3, 1};
    view.camera_x = 40;
    view.camera_y = -8;
    view.credits = p8_fixed_from_int(12);
    view.year = p8_fixed_from_int(2);
    view.nodes = 30;
    return view;
}

static void test_a_view_against_itself_changes_nothing(void) {
    pad_metrics_view_t a = still_view(), b = still_view();
    assert(!pad_metrics_navigation_changed(&a, &b));
    assert(!pad_metrics_view_changed(&a, &b));
}

static void test_each_navigation_field_moves_both_change_tests(void) {
    for (int field = 0; field < 12; field++) {
        pad_metrics_view_t a = still_view(), b = still_view();
        switch (field) {
        case 0: b.mode = PAD_METRICS_MODE_AIM; break;
        case 1: b.hovered.generation++; break;
        case 2: b.slipway_start = (entity_id_t){5, 1}; break;
        case 3: b.slipway_target = (entity_id_t){6, 1}; break;
        case 4: b.build_icon = (entity_id_t){7, 1}; break;
        case 5: b.placing = (entity_id_t){8, 1}; break;
        case 6: b.focus = (entity_id_t){9, 1}; break;
        case 7: b.camera_x++; break;
        case 8: b.camera_y--; break;
        case 9: b.panel_open = 1; break;
        case 10: b.box_open = 1; break;
        default: b.message_box = 1; break;
        }
        assert(pad_metrics_navigation_changed(&a, &b));
        assert(pad_metrics_view_changed(&a, &b));
    }
}

static void test_each_game_field_moves_only_the_view_change_test(void) {
    for (int field = 0; field < 6; field++) {
        pad_metrics_view_t a = still_view(), b = still_view();
        switch (field) {
        case 0: b.credits -= p8_fixed_from_int(1); break;
        case 1: b.year += p8_fixed_from_int(1); break;
        case 2: b.year_ticks = 1; break;
        case 3: b.nodes++; break;
        case 4: b.probes = 1; break;
        default: b.techs = 1; break;
        }
        assert(!pad_metrics_navigation_changed(&a, &b));
        assert(pad_metrics_view_changed(&a, &b));
    }
}

/* Feeds the same pad frames to the counter in mode and to pad_jump; returns how many frames of frame_count both
 * report a repeat on, and asserts they never disagree past a press frame. */
static int repeats_against_pad_jump(const int *buttons, int frame_count, pad_metrics_mode_t mode) {
    pad_metrics_hold_t hold;
    memset(&hold, 0, sizeof hold);
    pad_jump_t jump;
    pad_jump_init(&jump);
    int previous = 0;
    int repeats = 0;
    for (int f = 0; f < frame_count; f++) {
        int pressed = buttons[f] & ~previous;
        int counted = pad_metrics_hold_step(&hold, mode, buttons[f]);
        int jumped = pad_jump_repeat_step(&jump, buttons[f], pressed);
        if (pressed != 0) {
            assert(counted == 0);
        } else {
            assert(counted == jumped);
        }
        repeats += counted != 0;
        previous = buttons[f];
    }
    return repeats;
}

static void test_a_held_direction_repeats_on_the_frames_pad_jump_does(void) {
    enum { FRAMES = 40 };
    int buttons[FRAMES];
    for (int f = 0; f < FRAMES; f++) buttons[f] = P8_PAD_LEFT;
    pad_metrics_hold_t hold;
    memset(&hold, 0, sizeof hold);
    for (int f = 0; f < FRAMES; f++) {
        int fired = pad_metrics_hold_step(&hold, PAD_METRICS_MODE_JUMP, buttons[f]);
        assert(fired == ((f == 18 || f == 26 || f == 34) ? P8_PAD_LEFT : 0));
    }
    assert(repeats_against_pad_jump(buttons, FRAMES, PAD_METRICS_MODE_JUMP) == 3);
}

static void test_a_fresh_press_restarts_the_count(void) {
    enum { FRAMES = 60 };
    int buttons[FRAMES];
    for (int f = 0; f < FRAMES; f++) buttons[f] = f == 22 ? 0 : P8_PAD_DOWN;
    assert(repeats_against_pad_jump(buttons, FRAMES, PAD_METRICS_MODE_JUMP) == 4);
    pad_metrics_hold_t hold;
    memset(&hold, 0, sizeof hold);
    for (int f = 0; f < FRAMES; f++) {
        int fired = pad_metrics_hold_step(&hold, PAD_METRICS_MODE_JUMP, buttons[f]);
        assert(fired == ((f == 18 || f == 23 + 18 || f == 23 + 26 || f == 23 + 34) ? P8_PAD_DOWN : 0));
    }
}

static void test_holds_repeat_in_slipway_mode_and_nowhere_else(void) {
    enum { FRAMES = 30 };
    int buttons[FRAMES];
    for (int f = 0; f < FRAMES; f++) buttons[f] = P8_PAD_RIGHT;
    assert(repeats_against_pad_jump(buttons, FRAMES, PAD_METRICS_MODE_SLIPWAY) == 2);
    for (int m = 0; m < PAD_METRICS_MODE_COUNT; m++) {
        if (m == PAD_METRICS_MODE_JUMP || m == PAD_METRICS_MODE_SLIPWAY) continue;
        pad_metrics_hold_t hold;
        memset(&hold, 0, sizeof hold);
        for (int f = 0; f < FRAMES; f++) {
            assert(pad_metrics_hold_step(&hold, (pad_metrics_mode_t)m, buttons[f]) == 0);
        }
    }
}

int main(void) {
    test_no_economy_is_the_title_whatever_else_is_set();
    test_a_game_with_nothing_open_is_jump();
    test_each_rung_alone_gives_its_mode();
    test_the_first_matching_rung_wins();
    test_mode_names_keep_the_mode_log_spelling();
    test_a_view_against_itself_changes_nothing();
    test_each_navigation_field_moves_both_change_tests();
    test_each_game_field_moves_only_the_view_change_test();
    test_a_held_direction_repeats_on_the_frames_pad_jump_does();
    test_a_fresh_press_restarts_the_count();
    test_holds_repeat_in_slipway_mode_and_nowhere_else();
    printf("test_pad_metrics: ok\n");
    return 0;
}
