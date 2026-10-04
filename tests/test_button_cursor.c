#include "engine/button_cursor.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

enum { LEFT, RIGHT, UP, DOWN, O, X };

static p8_input_frame_t frame_at(int x, int y) {
    p8_input_frame_t frame;
    memset(&frame, 0, sizeof frame);
    frame.mouse_x = x;
    frame.mouse_y = y;
    return frame;
}

static p8_input_frame_t step(button_cursor_t *state, p8_input_frame_t raw) {
    p8_input_frame_t played;
    memset(&played, 0x55, sizeof played);
    button_cursor_step(state, &raw, &played);
    return played;
}

static void assert_frames_equal(p8_input_frame_t a, p8_input_frame_t b) {
    assert(a.mouse_x == b.mouse_x);
    assert(a.mouse_y == b.mouse_y);
    assert(a.mouse_buttons == b.mouse_buttons);
    for (int i = 0; i < 6; i++) {
        assert(a.dpad[i] == b.dpad[i]);
    }
}

static void test_first_frame_plays_raw(void) {
    button_cursor_t state;
    button_cursor_init(&state);
    p8_input_frame_t raw = frame_at(40, 50);
    raw.mouse_buttons = P8_MOUSE_LEFT;
    raw.dpad[RIGHT] = 1;
    raw.dpad[O] = 1;
    assert_frames_equal(step(&state, raw), raw);
}

static void test_mouse_owner_plays_raw_with_held_directions(void) {
    button_cursor_t state;
    button_cursor_init(&state);
    step(&state, frame_at(40, 50));
    p8_input_frame_t raw = frame_at(42, 51);
    raw.dpad[LEFT] = 1;
    raw.dpad[DOWN] = 1;
    assert_frames_equal(step(&state, raw), raw);
    raw = frame_at(45, 53);
    raw.dpad[LEFT] = 1;
    raw.dpad[DOWN] = 1;
    raw.mouse_buttons = P8_MOUSE_RIGHT;
    assert_frames_equal(step(&state, raw), raw);
}

static void test_direction_edge_takes_cursor_and_moves_one_pixel_from_it(void) {
    button_cursor_t state;
    button_cursor_init(&state);
    step(&state, frame_at(40, 50));
    p8_input_frame_t raw = frame_at(40, 50);
    raw.dpad[RIGHT] = 1;
    p8_input_frame_t played = step(&state, raw);
    assert(played.mouse_x == 41);
    assert(played.mouse_y == 50);
    assert(played.dpad[RIGHT] == 0);
}

static p8_input_frame_t hold(int x, int y, int button) {
    p8_input_frame_t frame = frame_at(x, y);
    frame.dpad[button] = 1;
    return frame;
}

static void test_ten_frames_at_one_pixel_then_two_and_release_restarts_slow(void) {
    button_cursor_t state;
    button_cursor_init(&state);
    step(&state, frame_at(20, 60));
    p8_input_frame_t played;
    for (int i = 1; i <= 10; i++) {
        played = step(&state, hold(20, 60, RIGHT));
        assert(played.mouse_x == 20 + i);
    }
    played = step(&state, hold(20, 60, RIGHT));
    assert(played.mouse_x == 32);
    played = step(&state, hold(20, 60, RIGHT));
    assert(played.mouse_x == 34);
    played = step(&state, frame_at(20, 60));
    assert(played.mouse_x == 34);
    played = step(&state, hold(20, 60, RIGHT));
    assert(played.mouse_x == 35);
}

static void test_two_directions_move_both_axes_and_opposites_cancel(void) {
    button_cursor_t state;
    button_cursor_init(&state);
    step(&state, frame_at(80, 60));
    p8_input_frame_t raw = hold(80, 60, LEFT);
    raw.dpad[DOWN] = 1;
    p8_input_frame_t played = step(&state, raw);
    assert(played.mouse_x == 79);
    assert(played.mouse_y == 61);
    raw.dpad[RIGHT] = 1;
    raw.dpad[DOWN] = 0;
    raw.dpad[UP] = 1;
    played = step(&state, raw);
    assert(played.mouse_x == 79);
    assert(played.mouse_y == 60);
}

static void test_cursor_clamps_to_screen(void) {
    button_cursor_t state;
    button_cursor_init(&state);
    step(&state, frame_at(1, 1));
    p8_input_frame_t raw = hold(1, 1, LEFT);
    raw.dpad[UP] = 1;
    p8_input_frame_t played;
    for (int i = 0; i < 15; i++) {
        played = step(&state, raw);
    }
    assert(played.mouse_x == 0);
    assert(played.mouse_y == 0);

    button_cursor_init(&state);
    step(&state, frame_at(158, 118));
    raw = hold(158, 118, RIGHT);
    raw.dpad[DOWN] = 1;
    for (int i = 0; i < 15; i++) {
        played = step(&state, raw);
    }
    assert(played.mouse_x == 159);
    assert(played.mouse_y == 119);
}

static void test_direction_pushing_against_edge_scrolls_from_the_next_frame(void) {
    button_cursor_t state;
    button_cursor_init(&state);
    step(&state, frame_at(1, 60));
    p8_input_frame_t played = step(&state, hold(1, 60, LEFT));
    assert(played.mouse_x == 0);
    assert(played.dpad[LEFT] == 0);
    played = step(&state, hold(1, 60, LEFT));
    assert(played.mouse_x == 0);
    assert(played.dpad[LEFT] == 1);

    p8_input_frame_t raw = hold(1, 60, LEFT);
    raw.dpad[DOWN] = 1;
    played = step(&state, raw);
    assert(played.dpad[LEFT] == 1);
    assert(played.dpad[DOWN] == 0);
    assert(played.mouse_y == 61);
}

static void test_each_edge_scrolls_its_own_direction(void) {
    int edges[4][3] = {{0, 60, LEFT}, {159, 60, RIGHT}, {80, 0, UP}, {80, 119, DOWN}};
    for (int e = 0; e < 4; e++) {
        button_cursor_t state;
        button_cursor_init(&state);
        step(&state, frame_at(edges[e][0], edges[e][1]));
        p8_input_frame_t played = step(&state, hold(edges[e][0], edges[e][1], edges[e][2]));
        assert(played.mouse_x == edges[e][0]);
        assert(played.mouse_y == edges[e][1]);
        for (int d = 0; d < 4; d++) {
            assert(played.dpad[d] == (d == edges[e][2]));
        }
    }
}

static void test_o_reads_left_x_reads_right_both_read_both(void) {
    button_cursor_t state;
    button_cursor_init(&state);
    step(&state, frame_at(80, 60));
    p8_input_frame_t played = step(&state, hold(80, 60, O));
    assert(played.mouse_buttons == P8_MOUSE_LEFT);
    assert(played.mouse_x == 80);
    p8_input_frame_t raw = hold(80, 60, O);
    raw.dpad[X] = 1;
    played = step(&state, raw);
    assert(played.mouse_buttons == (P8_MOUSE_LEFT | P8_MOUSE_RIGHT));
    played = step(&state, hold(80, 60, X));
    assert(played.mouse_buttons == P8_MOUSE_RIGHT);
    played = step(&state, frame_at(80, 60));
    assert(played.mouse_buttons == 0);
}

static void test_mouse_move_takes_cursor_back_and_cursor_jumps_to_it(void) {
    button_cursor_t state;
    button_cursor_init(&state);
    step(&state, frame_at(80, 60));
    step(&state, hold(80, 60, RIGHT));
    p8_input_frame_t raw = frame_at(10, 20);
    assert_frames_equal(step(&state, raw), raw);
}

static void test_mouse_button_change_takes_cursor_back(void) {
    button_cursor_t state;
    button_cursor_init(&state);
    step(&state, frame_at(80, 60));
    step(&state, hold(80, 60, RIGHT));
    p8_input_frame_t raw = frame_at(80, 60);
    raw.mouse_buttons = P8_MOUSE_LEFT;
    assert_frames_equal(step(&state, raw), raw);
}

static void test_mouse_move_beats_button_edge_on_the_same_frame(void) {
    button_cursor_t state;
    button_cursor_init(&state);
    step(&state, frame_at(80, 60));
    p8_input_frame_t raw = hold(81, 60, UP);
    assert_frames_equal(step(&state, raw), raw);
}

static void test_button_held_across_mouse_takeover_needs_a_new_press(void) {
    button_cursor_t state;
    button_cursor_init(&state);
    step(&state, frame_at(80, 60));
    step(&state, hold(80, 60, RIGHT));
    p8_input_frame_t raw = hold(70, 50, RIGHT);
    assert_frames_equal(step(&state, raw), raw);
    assert_frames_equal(step(&state, raw), raw);
    assert_frames_equal(step(&state, raw), raw);
    step(&state, frame_at(70, 50));
    p8_input_frame_t played = step(&state, hold(70, 50, RIGHT));
    assert(played.mouse_x == 71);
}

static void test_arrow_during_mouse_drag_keeps_mouse_and_reaches_camera(void) {
    button_cursor_t state;
    button_cursor_init(&state);
    p8_input_frame_t raw = frame_at(80, 60);
    raw.mouse_buttons = P8_MOUSE_LEFT;
    step(&state, raw);
    raw.dpad[RIGHT] = 1;
    assert_frames_equal(step(&state, raw), raw);
    assert_frames_equal(step(&state, raw), raw);
    raw.mouse_buttons = 0;
    assert_frames_equal(step(&state, raw), raw);
    assert_frames_equal(step(&state, raw), raw);
    step(&state, frame_at(80, 60));
    p8_input_frame_t played = step(&state, hold(80, 60, RIGHT));
    assert(played.mouse_x == 81);
    assert(played.dpad[RIGHT] == 0);
}

static void test_mouse_bump_during_o_drag_leaves_cursor_with_buttons(void) {
    button_cursor_t state;
    button_cursor_init(&state);
    step(&state, frame_at(80, 60));
    step(&state, hold(80, 60, O));
    p8_input_frame_t played = step(&state, hold(90, 65, O));
    assert(played.mouse_x == 80);
    assert(played.mouse_y == 60);
    assert(played.mouse_buttons == P8_MOUSE_LEFT);
    played = step(&state, frame_at(91, 65));
    assert(played.mouse_x == 91);
    assert(played.mouse_y == 65);
    assert(played.mouse_buttons == 0);
}

static void test_real_button_during_o_drag_is_ignored_until_it_changes_unlocked(void) {
    button_cursor_t state;
    button_cursor_init(&state);
    step(&state, frame_at(80, 60));
    step(&state, hold(80, 60, O));
    p8_input_frame_t raw = hold(80, 60, O);
    raw.mouse_buttons = P8_MOUSE_RIGHT;
    p8_input_frame_t played = step(&state, raw);
    assert(played.mouse_x == 80);
    assert(played.mouse_buttons == P8_MOUSE_LEFT);
    raw.dpad[O] = 0;
    played = step(&state, raw);
    assert(played.mouse_buttons == 0);
    raw.dpad[RIGHT] = 1;
    played = step(&state, raw);
    assert(played.mouse_x == 81);
    assert(played.mouse_buttons == 0);
    raw = frame_at(80, 60);
    assert_frames_equal(step(&state, raw), raw);
}

static void test_arrow_alone_does_not_lock_the_cursor(void) {
    button_cursor_t state;
    button_cursor_init(&state);
    step(&state, frame_at(80, 60));
    step(&state, hold(80, 60, LEFT));
    p8_input_frame_t raw = hold(30, 40, LEFT);
    assert_frames_equal(step(&state, raw), raw);
}

static void test_played_o_and_x_always_equal_raw(void) {
    button_cursor_t state;
    button_cursor_init(&state);
    int script[][4] = {{80, 60, 0, 0}, {80, 60, 1, 0}, {80, 60, 1, 1}, {85, 60, 0, 1}, {85, 60, 0, 0}, {86, 61, 1, 0}};
    for (int i = 0; i < 6; i++) {
        p8_input_frame_t raw = frame_at(script[i][0], script[i][1]);
        raw.dpad[O] = script[i][2];
        raw.dpad[X] = script[i][3];
        p8_input_frame_t played = step(&state, raw);
        assert(played.dpad[O] == raw.dpad[O]);
        assert(played.dpad[X] == raw.dpad[X]);
    }
}

int main(void) {
    test_first_frame_plays_raw();
    test_mouse_owner_plays_raw_with_held_directions();
    test_direction_edge_takes_cursor_and_moves_one_pixel_from_it();
    test_ten_frames_at_one_pixel_then_two_and_release_restarts_slow();
    test_two_directions_move_both_axes_and_opposites_cancel();
    test_cursor_clamps_to_screen();
    test_direction_pushing_against_edge_scrolls_from_the_next_frame();
    test_each_edge_scrolls_its_own_direction();
    test_o_reads_left_x_reads_right_both_read_both();
    test_mouse_move_takes_cursor_back_and_cursor_jumps_to_it();
    test_mouse_button_change_takes_cursor_back();
    test_mouse_move_beats_button_edge_on_the_same_frame();
    test_button_held_across_mouse_takeover_needs_a_new_press();
    test_arrow_during_mouse_drag_keeps_mouse_and_reaches_camera();
    test_mouse_bump_during_o_drag_leaves_cursor_with_buttons();
    test_real_button_during_o_drag_is_ignored_until_it_changes_unlocked();
    test_arrow_alone_does_not_lock_the_cursor();
    test_played_o_and_x_always_equal_raw();
    printf("test_button_cursor: all passed\n");
    return 0;
}
