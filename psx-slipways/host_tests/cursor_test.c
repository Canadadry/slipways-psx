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

#define TEST_PAD_CROSS (1 << 14)
#define TEST_PAD_UP    (1 << 4)

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

int main(void) {
	t_run("cursor_init_produces_documented_initial_state", test_cursor_init_produces_documented_initial_state);
	t_run("cross_press_edge_flips_mouse_mode", test_cross_press_edge_flips_mouse_mode);
	t_run("holding_cross_does_not_reflip", test_holding_cross_does_not_reflip);
	t_run("releasing_cross_does_not_flip", test_releasing_cross_does_not_flip);
	t_run("unrelated_button_does_not_flip", test_unrelated_button_does_not_flip);
	return t_report();
}
