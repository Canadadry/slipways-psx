#include "cursor.h"

#include "screen.h"

#define CURSOR_PAD_CROSS  (1 << 14)
#define CURSOR_PAD_CIRCLE (1 << 13)
#define CURSOR_PAD_UP     (1 << 4)
#define CURSOR_PAD_RIGHT  (1 << 5)
#define CURSOR_PAD_DOWN   (1 << 6)
#define CURSOR_PAD_LEFT   (1 << 7)

int g_cursor_mouse_mode = 0;
int g_cursor_x = SCREEN_XRES / 2;
int g_cursor_y = SCREEN_YRES / 2;
int g_cursor_click_pressed = 0;
int g_cursor_click_held = 0;
int g_cursor_click_released = 0;
int g_cursor_click_flash_frames = 0;
uint16_t g_cursor_prev_buttons = 0xffff;

cursor_state_t cursor_step(cursor_state_t prev_state,
                            uint16_t prev_buttons,
                            uint16_t buttons) {
	cursor_state_t next = prev_state;

	uint16_t pressed = (uint16_t) (~buttons) & prev_buttons;

	if (pressed & CURSOR_PAD_CROSS) {
		next.mouse_mode = !next.mouse_mode;
	}

	if (next.mouse_mode) {
		int dx = 0;
		int dy = 0;

		if (!(buttons & CURSOR_PAD_UP))    dy -= CURSOR_STEP;
		if (!(buttons & CURSOR_PAD_DOWN))  dy += CURSOR_STEP;
		if (!(buttons & CURSOR_PAD_LEFT))  dx -= CURSOR_STEP;
		if (!(buttons & CURSOR_PAD_RIGHT)) dx += CURSOR_STEP;

		next.x += dx;
		next.y += dy;

		if (next.x < 0) next.x = 0;
		if (next.y < 0) next.y = 0;
		if (next.x > SCREEN_XRES - 1) next.x = SCREEN_XRES - 1;
		if (next.y > SCREEN_YRES - 1) next.y = SCREEN_YRES - 1;

		uint16_t pressed_circle = (uint16_t) (~buttons) & prev_buttons & CURSOR_PAD_CIRCLE;
		next.click_pressed = pressed_circle != 0;
		next.click_held = !(buttons & CURSOR_PAD_CIRCLE);

		uint16_t released_circle = buttons & (uint16_t) (~prev_buttons) & CURSOR_PAD_CIRCLE;
		next.click_released = released_circle != 0;

		if (next.click_pressed) {
			next.click_flash_frames = CLICK_FLASH_FRAMES;
		} else if (next.click_flash_frames > 0) {
			next.click_flash_frames--;
		}
	} else {
		next.click_pressed = 0;
		next.click_held = 0;
		next.click_released = 0;
		next.click_flash_frames = 0;
	}

	return next;
}

void cursor_init(void) {
	g_cursor_mouse_mode = 0;
	g_cursor_x = SCREEN_XRES / 2;
	g_cursor_y = SCREEN_YRES / 2;
	g_cursor_click_pressed = 0;
	g_cursor_click_held = 0;
	g_cursor_click_released = 0;
	g_cursor_click_flash_frames = 0;
	g_cursor_prev_buttons = 0xffff;
}

void cursor_update(uint16_t buttons) {
	cursor_state_t prev;
	prev.mouse_mode = g_cursor_mouse_mode;
	prev.x = g_cursor_x;
	prev.y = g_cursor_y;
	prev.click_pressed = g_cursor_click_pressed;
	prev.click_held = g_cursor_click_held;
	prev.click_released = g_cursor_click_released;
	prev.click_flash_frames = g_cursor_click_flash_frames;

	cursor_state_t next = cursor_step(prev, g_cursor_prev_buttons, buttons);

	g_cursor_mouse_mode = next.mouse_mode;
	g_cursor_x = next.x;
	g_cursor_y = next.y;
	g_cursor_click_pressed = next.click_pressed;
	g_cursor_click_held = next.click_held;
	g_cursor_click_released = next.click_released;
	g_cursor_click_flash_frames = next.click_flash_frames;
	g_cursor_prev_buttons = buttons;
}

cursor_state_t cursor_get_state(void) {
	cursor_state_t state;
	state.mouse_mode = g_cursor_mouse_mode;
	state.x = g_cursor_x;
	state.y = g_cursor_y;
	state.click_pressed = g_cursor_click_pressed;
	state.click_held = g_cursor_click_held;
	state.click_released = g_cursor_click_released;
	state.click_flash_frames = g_cursor_click_flash_frames;
	return state;
}
