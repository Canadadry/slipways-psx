#include "t.h"
#include "../cursor.h"
#include "../screen.h"

static void test_cursor_init_produces_documented_initial_state(void) {
	cursor_init();

	cursor_state_t s = cursor_get_state();

	T_CHECK(s.mouse_mode == 0);
	T_CHECK(s.x == SCREEN_XRES / 2);
	T_CHECK(s.y == SCREEN_YRES / 2);
}

#define TEST_PAD_CROSS  (1 << 14)
#define TEST_PAD_CIRCLE (1 << 13)
#define TEST_PAD_UP     (1 << 4)
#define TEST_PAD_RIGHT  (1 << 5)
#define TEST_PAD_DOWN   (1 << 6)
#define TEST_PAD_LEFT   (1 << 7)

static void test_cross_press_edge_flips_mouse_mode(void) {
	cursor_state_t prev = { 0, SCREEN_XRES / 2, SCREEN_YRES / 2 };
	uint16_t prev_buttons = 0xffff;
	uint16_t buttons = (uint16_t) ~TEST_PAD_CROSS;

	cursor_state_t next = cursor_step(prev, prev_buttons, buttons);

	T_CHECK(next.mouse_mode == 1);
}

static void test_holding_cross_does_not_reflip(void) {
	cursor_state_t state = { 0, SCREEN_XRES / 2, SCREEN_YRES / 2 };
	uint16_t prev_buttons = 0xffff;
	uint16_t held = (uint16_t) ~TEST_PAD_CROSS;

	state = cursor_step(state, prev_buttons, held);
	T_CHECK(state.mouse_mode == 1);

	state = cursor_step(state, held, held);
	T_CHECK(state.mouse_mode == 1);
}

static void test_releasing_cross_does_not_flip(void) {
	cursor_state_t state = { 0, SCREEN_XRES / 2, SCREEN_YRES / 2 };
	uint16_t held = (uint16_t) ~TEST_PAD_CROSS;

	state = cursor_step(state, 0xffff, held);
	T_CHECK(state.mouse_mode == 1);

	state = cursor_step(state, held, 0xffff);
	T_CHECK(state.mouse_mode == 1);
}

static void test_unrelated_button_does_not_flip(void) {
	cursor_state_t prev = { 0, SCREEN_XRES / 2, SCREEN_YRES / 2 };
	uint16_t prev_buttons = 0xffff;
	uint16_t buttons = (uint16_t) ~TEST_PAD_UP;

	cursor_state_t next = cursor_step(prev, prev_buttons, buttons);

	T_CHECK(next.mouse_mode == 0);
}

static void test_right_held_while_active_moves_x_right(void) {
	cursor_state_t prev = { 1, SCREEN_XRES / 2, SCREEN_YRES / 2 };
	uint16_t buttons = (uint16_t) ~TEST_PAD_RIGHT;

	cursor_state_t next = cursor_step(prev, 0xffff, buttons);

	T_CHECK(next.x == SCREEN_XRES / 2 + CURSOR_STEP);
	T_CHECK(next.y == SCREEN_YRES / 2);
}

static void test_left_held_while_active_moves_x_left(void) {
	cursor_state_t prev = { 1, SCREEN_XRES / 2, SCREEN_YRES / 2 };
	uint16_t buttons = (uint16_t) ~TEST_PAD_LEFT;

	cursor_state_t next = cursor_step(prev, 0xffff, buttons);

	T_CHECK(next.x == SCREEN_XRES / 2 - CURSOR_STEP);
	T_CHECK(next.y == SCREEN_YRES / 2);
}

static void test_up_held_while_active_moves_y_up(void) {
	cursor_state_t prev = { 1, SCREEN_XRES / 2, SCREEN_YRES / 2 };
	uint16_t buttons = (uint16_t) ~TEST_PAD_UP;

	cursor_state_t next = cursor_step(prev, 0xffff, buttons);

	T_CHECK(next.x == SCREEN_XRES / 2);
	T_CHECK(next.y == SCREEN_YRES / 2 - CURSOR_STEP);
}

static void test_down_held_while_active_moves_y_down(void) {
	cursor_state_t prev = { 1, SCREEN_XRES / 2, SCREEN_YRES / 2 };
	uint16_t buttons = (uint16_t) ~TEST_PAD_DOWN;

	cursor_state_t next = cursor_step(prev, 0xffff, buttons);

	T_CHECK(next.x == SCREEN_XRES / 2);
	T_CHECK(next.y == SCREEN_YRES / 2 + CURSOR_STEP);
}

static void test_no_movement_while_inactive(void) {
	cursor_state_t prev = { 0, SCREEN_XRES / 2, SCREEN_YRES / 2 };
	uint16_t buttons = (uint16_t) ~(TEST_PAD_RIGHT | TEST_PAD_DOWN);

	cursor_state_t next = cursor_step(prev, 0xffff, buttons);

	T_CHECK(next.x == SCREEN_XRES / 2);
	T_CHECK(next.y == SCREEN_YRES / 2);
	T_CHECK(next.mouse_mode == 0);
}

static void test_clamps_at_x_zero_without_overshoot(void) {
	cursor_state_t prev = { 1, 0, SCREEN_YRES / 2 };
	uint16_t buttons = (uint16_t) ~TEST_PAD_LEFT;

	cursor_state_t next = cursor_step(prev, 0xffff, buttons);

	T_CHECK(next.x == 0);
}

static void test_clamps_at_x_max_without_overshoot(void) {
	cursor_state_t prev = { 1, SCREEN_XRES - 1, SCREEN_YRES / 2 };
	uint16_t buttons = (uint16_t) ~TEST_PAD_RIGHT;

	cursor_state_t next = cursor_step(prev, 0xffff, buttons);

	T_CHECK(next.x == SCREEN_XRES - 1);
}

static void test_clamps_at_y_zero_without_overshoot(void) {
	cursor_state_t prev = { 1, SCREEN_XRES / 2, 0 };
	uint16_t buttons = (uint16_t) ~TEST_PAD_UP;

	cursor_state_t next = cursor_step(prev, 0xffff, buttons);

	T_CHECK(next.y == 0);
}

static void test_clamps_at_y_max_without_overshoot(void) {
	cursor_state_t prev = { 1, SCREEN_XRES / 2, SCREEN_YRES - 1 };
	uint16_t buttons = (uint16_t) ~TEST_PAD_DOWN;

	cursor_state_t next = cursor_step(prev, 0xffff, buttons);

	T_CHECK(next.y == SCREEN_YRES - 1);
}

static void test_diagonal_moves_both_axes_in_one_step(void) {
	cursor_state_t prev = { 1, SCREEN_XRES / 2, SCREEN_YRES / 2 };
	uint16_t buttons = (uint16_t) ~(TEST_PAD_UP | TEST_PAD_RIGHT);

	cursor_state_t next = cursor_step(prev, 0xffff, buttons);

	T_CHECK(next.x == SCREEN_XRES / 2 + CURSOR_STEP);
	T_CHECK(next.y == SCREEN_YRES / 2 - CURSOR_STEP);
}

static void test_diagonal_corner_clamps_both_axes_summed_then_clamped(void) {
	cursor_state_t prev = { 1, SCREEN_XRES - 1, 0 };
	uint16_t buttons = (uint16_t) ~(TEST_PAD_UP | TEST_PAD_RIGHT);

	cursor_state_t next = cursor_step(prev, 0xffff, buttons);

	T_CHECK(next.x == SCREEN_XRES - 1);
	T_CHECK(next.y == 0);
}

static void test_movement_resumes_after_toggling_off_then_back_on(void) {
	/* Start active, move right once. */
	cursor_state_t state = { 1, SCREEN_XRES / 2, SCREEN_YRES / 2 };
	uint16_t prev_buttons = 0xffff;
	uint16_t buttons = (uint16_t) ~TEST_PAD_RIGHT;

	state = cursor_step(state, prev_buttons, buttons);
	T_CHECK(state.mouse_mode == 1);
	T_CHECK(state.x == SCREEN_XRES / 2 + CURSOR_STEP);
	int moved_x = state.x;

	/* Toggle off (Cross press-edge): position must not be disturbed. */
	prev_buttons = buttons; /* Right still held */
	buttons = (uint16_t) ~(TEST_PAD_RIGHT | TEST_PAD_CROSS);
	state = cursor_step(state, prev_buttons, buttons);
	T_CHECK(state.mouse_mode == 0);
	T_CHECK(state.x == moved_x);

	/* Still holding Right while off: no movement. */
	prev_buttons = buttons;
	buttons = (uint16_t) ~TEST_PAD_RIGHT;
	state = cursor_step(state, prev_buttons, buttons);
	T_CHECK(state.mouse_mode == 0);
	T_CHECK(state.x == moved_x);

	/* Release Right, then toggle back on (Cross press-edge) with Right
	 * released: mode flips on, no movement this same frame. */
	prev_buttons = buttons;
	buttons = 0xffff;
	state = cursor_step(state, prev_buttons, buttons);
	T_CHECK(state.mouse_mode == 0);

	prev_buttons = buttons;
	buttons = (uint16_t) ~TEST_PAD_CROSS;
	state = cursor_step(state, prev_buttons, buttons);
	T_CHECK(state.mouse_mode == 1);
	T_CHECK(state.x == moved_x);

	/* Movement works again once re-toggled on. */
	prev_buttons = buttons;
	buttons = (uint16_t) ~TEST_PAD_RIGHT;
	state = cursor_step(state, prev_buttons, buttons);
	T_CHECK(state.mouse_mode == 1);
	T_CHECK(state.x == moved_x + CURSOR_STEP);
}

static void test_circle_press_edge_fires_click_pressed(void) {
	cursor_state_t prev = { 1, SCREEN_XRES / 2, SCREEN_YRES / 2 };
	uint16_t prev_buttons = 0xffff;
	uint16_t buttons = (uint16_t) ~TEST_PAD_CIRCLE;

	cursor_state_t next = cursor_step(prev, prev_buttons, buttons);

	T_CHECK(next.click_pressed == 1);
}

static void test_holding_circle_keeps_click_held_without_refiring_pressed(void) {
	cursor_state_t state = { 1, SCREEN_XRES / 2, SCREEN_YRES / 2 };
	uint16_t prev_buttons = 0xffff;
	uint16_t held = (uint16_t) ~TEST_PAD_CIRCLE;

	state = cursor_step(state, prev_buttons, held);
	T_CHECK(state.click_pressed == 1);
	T_CHECK(state.click_held == 1);

	state = cursor_step(state, held, held);
	T_CHECK(state.click_pressed == 0);
	T_CHECK(state.click_held == 1);
}

static void test_releasing_circle_fires_click_released_once(void) {
	cursor_state_t state = { 1, SCREEN_XRES / 2, SCREEN_YRES / 2 };
	uint16_t held = (uint16_t) ~TEST_PAD_CIRCLE;

	state = cursor_step(state, 0xffff, held);
	T_CHECK(state.click_released == 0);

	state = cursor_step(state, held, 0xffff);
	T_CHECK(state.click_released == 1);
	T_CHECK(state.click_held == 0);

	state = cursor_step(state, 0xffff, 0xffff);
	T_CHECK(state.click_released == 0);
}

static void test_no_click_state_while_mode_off_never_pressed(void) {
	cursor_state_t prev = { 0, SCREEN_XRES / 2, SCREEN_YRES / 2 };
	uint16_t prev_buttons = 0xffff;
	uint16_t buttons = (uint16_t) ~TEST_PAD_CIRCLE;

	cursor_state_t next = cursor_step(prev, prev_buttons, buttons);

	T_CHECK(next.click_pressed == 0);
	T_CHECK(next.click_held == 0);
	T_CHECK(next.click_released == 0);
}

static void test_no_click_state_while_mode_off_held_throughout(void) {
	cursor_state_t state = { 0, SCREEN_XRES / 2, SCREEN_YRES / 2 };
	uint16_t held = (uint16_t) ~TEST_PAD_CIRCLE;

	state = cursor_step(state, 0xffff, held);
	T_CHECK(state.click_pressed == 0);
	T_CHECK(state.click_held == 0);
	T_CHECK(state.click_released == 0);

	state = cursor_step(state, held, held);
	T_CHECK(state.click_pressed == 0);
	T_CHECK(state.click_held == 0);
	T_CHECK(state.click_released == 0);

	state = cursor_step(state, held, 0xffff);
	T_CHECK(state.click_pressed == 0);
	T_CHECK(state.click_held == 0);
	T_CHECK(state.click_released == 0);
}

static void test_toggling_mode_off_mid_hold_zeroes_click_state(void) {
	/* Turn mode on, press Circle: pressed+held. */
	cursor_state_t state = { 0, SCREEN_XRES / 2, SCREEN_YRES / 2 };
	uint16_t prev_buttons = 0xffff;
	uint16_t buttons = (uint16_t) ~TEST_PAD_CROSS;
	state = cursor_step(state, prev_buttons, buttons);
	T_CHECK(state.mouse_mode == 1);

	prev_buttons = buttons; /* Cross released this frame, Circle newly pressed */
	buttons = (uint16_t) ~TEST_PAD_CIRCLE;
	state = cursor_step(state, prev_buttons, buttons);
	T_CHECK(state.click_pressed == 1);
	T_CHECK(state.click_held == 1);

	/* Toggle mode off (Cross press-edge) while Circle is still physically
	 * held: all click fields must be forced to 0 this same frame, no
	 * click_released synthesized. */
	prev_buttons = buttons; /* Circle held */
	buttons = (uint16_t) ~(TEST_PAD_CIRCLE | TEST_PAD_CROSS);
	state = cursor_step(state, prev_buttons, buttons);
	T_CHECK(state.mouse_mode == 0);
	T_CHECK(state.click_pressed == 0);
	T_CHECK(state.click_held == 0);
	T_CHECK(state.click_released == 0);

	/* Toggle back on (Cross press-edge) with Circle still physically held:
	 * no retroactive click_pressed -- there's no edge in prev_buttons ->
	 * buttons for Circle on this frame, only for Cross. */
	prev_buttons = buttons; /* Circle still held, Cross released */
	buttons = (uint16_t) ~TEST_PAD_CIRCLE;
	state = cursor_step(state, prev_buttons, buttons);
	T_CHECK(state.mouse_mode == 0);

	prev_buttons = buttons;
	buttons = (uint16_t) ~(TEST_PAD_CIRCLE | TEST_PAD_CROSS);
	state = cursor_step(state, prev_buttons, buttons);
	T_CHECK(state.mouse_mode == 1);
	T_CHECK(state.click_pressed == 0);
	T_CHECK(state.click_held == 1);
	T_CHECK(state.click_released == 0);
}

static void test_click_fields_independent_of_dpad_and_cross(void) {
	/* Circle press alongside D-pad + Cross held simultaneously: click
	 * fields fire correctly, and cursor movement/mouse_mode are unaffected
	 * by Circle. */
	cursor_state_t prev = { 1, SCREEN_XRES / 2, SCREEN_YRES / 2 };
	uint16_t prev_buttons = 0xffff;
	uint16_t buttons = (uint16_t) ~(TEST_PAD_CIRCLE | TEST_PAD_RIGHT | TEST_PAD_UP);

	cursor_state_t next = cursor_step(prev, prev_buttons, buttons);

	T_CHECK(next.click_pressed == 1);
	T_CHECK(next.click_held == 1);
	T_CHECK(next.x == SCREEN_XRES / 2 + CURSOR_STEP);
	T_CHECK(next.y == SCREEN_YRES / 2 - CURSOR_STEP);
	T_CHECK(next.mouse_mode == 1);
}

static void test_cross_toggle_does_not_affect_click_fields_when_circle_idle(void) {
	/* A Cross press-edge (toggling mode on) with Circle untouched must not
	 * spuriously set click_pressed/click_held/click_released. */
	cursor_state_t prev = { 0, SCREEN_XRES / 2, SCREEN_YRES / 2 };
	uint16_t prev_buttons = 0xffff;
	uint16_t buttons = (uint16_t) ~TEST_PAD_CROSS;

	cursor_state_t next = cursor_step(prev, prev_buttons, buttons);

	T_CHECK(next.mouse_mode == 1);
	T_CHECK(next.click_pressed == 0);
	T_CHECK(next.click_held == 0);
	T_CHECK(next.click_released == 0);
}

static void test_update_get_state_round_trip_matches_step(void) {
	cursor_init();

	cursor_state_t expected = { 0, SCREEN_XRES / 2, SCREEN_YRES / 2 };
	uint16_t prev_buttons = 0xffff;
	uint16_t buttons = (uint16_t) ~TEST_PAD_CROSS;
	expected = cursor_step(expected, prev_buttons, buttons);

	cursor_update(buttons);
	cursor_state_t got = cursor_get_state();

	T_CHECK(got.mouse_mode == expected.mouse_mode);
	T_CHECK(got.x == expected.x);
	T_CHECK(got.y == expected.y);

	/* Second frame: hold Right while active, via the stateful wrapper. */
	uint16_t buttons2 = (uint16_t) ~TEST_PAD_RIGHT;
	expected = cursor_step(expected, buttons, buttons2);

	cursor_update(buttons2);
	got = cursor_get_state();

	T_CHECK(got.mouse_mode == expected.mouse_mode);
	T_CHECK(got.x == expected.x);
	T_CHECK(got.y == expected.y);

	/* Third frame: press Circle while active, via the stateful wrapper --
	 * the new click fields round-trip too. */
	uint16_t buttons3 = (uint16_t) ~TEST_PAD_CIRCLE;
	expected = cursor_step(expected, buttons2, buttons3);

	cursor_update(buttons3);
	got = cursor_get_state();

	T_CHECK(got.click_pressed == expected.click_pressed);
	T_CHECK(got.click_held == expected.click_held);
	T_CHECK(got.click_released == expected.click_released);
	T_CHECK(got.click_pressed == 1);
	T_CHECK(got.click_held == 1);
}

int main(void) {
	t_run("cursor_init_produces_documented_initial_state", test_cursor_init_produces_documented_initial_state);
	t_run("cross_press_edge_flips_mouse_mode", test_cross_press_edge_flips_mouse_mode);
	t_run("holding_cross_does_not_reflip", test_holding_cross_does_not_reflip);
	t_run("releasing_cross_does_not_flip", test_releasing_cross_does_not_flip);
	t_run("unrelated_button_does_not_flip", test_unrelated_button_does_not_flip);
	t_run("right_held_while_active_moves_x_right", test_right_held_while_active_moves_x_right);
	t_run("left_held_while_active_moves_x_left", test_left_held_while_active_moves_x_left);
	t_run("up_held_while_active_moves_y_up", test_up_held_while_active_moves_y_up);
	t_run("down_held_while_active_moves_y_down", test_down_held_while_active_moves_y_down);
	t_run("no_movement_while_inactive", test_no_movement_while_inactive);
	t_run("clamps_at_x_zero_without_overshoot", test_clamps_at_x_zero_without_overshoot);
	t_run("clamps_at_x_max_without_overshoot", test_clamps_at_x_max_without_overshoot);
	t_run("clamps_at_y_zero_without_overshoot", test_clamps_at_y_zero_without_overshoot);
	t_run("clamps_at_y_max_without_overshoot", test_clamps_at_y_max_without_overshoot);
	t_run("diagonal_moves_both_axes_in_one_step", test_diagonal_moves_both_axes_in_one_step);
	t_run("diagonal_corner_clamps_both_axes_summed_then_clamped", test_diagonal_corner_clamps_both_axes_summed_then_clamped);
	t_run("movement_resumes_after_toggling_off_then_back_on", test_movement_resumes_after_toggling_off_then_back_on);
	t_run("circle_press_edge_fires_click_pressed", test_circle_press_edge_fires_click_pressed);
	t_run("holding_circle_keeps_click_held_without_refiring_pressed", test_holding_circle_keeps_click_held_without_refiring_pressed);
	t_run("releasing_circle_fires_click_released_once", test_releasing_circle_fires_click_released_once);
	t_run("no_click_state_while_mode_off_never_pressed", test_no_click_state_while_mode_off_never_pressed);
	t_run("no_click_state_while_mode_off_held_throughout", test_no_click_state_while_mode_off_held_throughout);
	t_run("toggling_mode_off_mid_hold_zeroes_click_state", test_toggling_mode_off_mid_hold_zeroes_click_state);
	t_run("click_fields_independent_of_dpad_and_cross", test_click_fields_independent_of_dpad_and_cross);
	t_run("cross_toggle_does_not_affect_click_fields_when_circle_idle", test_cross_toggle_does_not_affect_click_fields_when_circle_idle);
	t_run("update_get_state_round_trip_matches_step", test_update_get_state_round_trip_matches_step);
	return t_report();
}
