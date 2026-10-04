#include "engine/button_cursor.h"

#include "p8/vram.h"

#include <assert.h>
#include <stddef.h>

enum { DIRECTION_LEFT, DIRECTION_RIGHT, DIRECTION_UP, DIRECTION_DOWN, DIRECTION_COUNT, BUTTON_O = 4, BUTTON_X = 5, BUTTON_COUNT = 6 };

#define BUTTON_CURSOR_SLOW_SPEED 1
#define BUTTON_CURSOR_FAST_SPEED 2
#define BUTTON_CURSOR_SLOW_FRAMES 10
#define BUTTON_CURSOR_MAX_X (P8_SCREEN_WIDTH - 1)
#define BUTTON_CURSOR_MAX_Y (P8_SCREEN_HEIGHT - 1)

void button_cursor_init(button_cursor_t *state) {
    assert(state != NULL);
    state->owner = BUTTON_CURSOR_OWNER_MOUSE;
    state->cursor_x = 0;
    state->cursor_y = 0;
    for (int i = 0; i < DIRECTION_COUNT; i++) {
        state->held_frames[i] = 0;
    }
    state->has_previous = 0;
}

static int any_button_rises(const p8_input_frame_t *previous, const p8_input_frame_t *raw) {
    for (int i = 0; i < BUTTON_COUNT; i++) {
        if (raw->dpad[i] && !previous->dpad[i]) {
            return 1;
        }
    }
    return 0;
}

static int mouse_changes(const p8_input_frame_t *previous, const p8_input_frame_t *raw) {
    return raw->mouse_x != previous->mouse_x || raw->mouse_y != previous->mouse_y ||
           raw->mouse_buttons != previous->mouse_buttons;
}

static int direction_speed(const button_cursor_t *state, int direction) {
    int held = state->held_frames[direction];
    if (held == 0) {
        return 0;
    }
    return held <= BUTTON_CURSOR_SLOW_FRAMES ? BUTTON_CURSOR_SLOW_SPEED : BUTTON_CURSOR_FAST_SPEED;
}

static int clamp(int value, int high) {
    if (value < 0) {
        return 0;
    }
    return value > high ? high : value;
}

static int cursor_on_edge(const button_cursor_t *state, int direction) {
    switch (direction) {
    case DIRECTION_LEFT:
        return state->cursor_x == 0;
    case DIRECTION_RIGHT:
        return state->cursor_x == BUTTON_CURSOR_MAX_X;
    case DIRECTION_UP:
        return state->cursor_y == 0;
    default:
        return state->cursor_y == BUTTON_CURSOR_MAX_Y;
    }
}

static int owner_holds_a_button(const button_cursor_t *state, const p8_input_frame_t *raw) {
    if (state->owner == BUTTON_CURSOR_OWNER_MOUSE) {
        return raw->mouse_buttons != 0;
    }
    return raw->dpad[BUTTON_O] || raw->dpad[BUTTON_X];
}

static void update_owner(button_cursor_t *state, const p8_input_frame_t *raw) {
    if (owner_holds_a_button(state, raw)) {
        return;
    }
    if (mouse_changes(&state->previous, raw)) {
        state->owner = BUTTON_CURSOR_OWNER_MOUSE;
    } else if (any_button_rises(&state->previous, raw)) {
        state->owner = BUTTON_CURSOR_OWNER_BUTTONS;
    }
}

static void update_held_frames(button_cursor_t *state, const p8_input_frame_t *raw) {
    for (int i = 0; i < DIRECTION_COUNT; i++) {
        if (!raw->dpad[i]) {
            state->held_frames[i] = 0;
        } else if (state->held_frames[i] <= BUTTON_CURSOR_SLOW_FRAMES) {
            state->held_frames[i]++;
        }
    }
}

void button_cursor_step(button_cursor_t *state, const p8_input_frame_t *raw, p8_input_frame_t *played) {
    assert(state != NULL && raw != NULL && played != NULL);
    if (!state->has_previous) {
        state->previous = *raw;
        state->has_previous = 1;
        state->owner = BUTTON_CURSOR_OWNER_MOUSE;
    }
    update_owner(state, raw);
    update_held_frames(state, raw);
    *played = *raw;
    if (state->owner == BUTTON_CURSOR_OWNER_MOUSE) {
        state->cursor_x = raw->mouse_x;
        state->cursor_y = raw->mouse_y;
    } else {
        for (int i = 0; i < DIRECTION_COUNT; i++) {
            played->dpad[i] = cursor_on_edge(state, i) ? raw->dpad[i] : 0;
        }
        state->cursor_x += direction_speed(state, DIRECTION_RIGHT) - direction_speed(state, DIRECTION_LEFT);
        state->cursor_x = clamp(state->cursor_x, BUTTON_CURSOR_MAX_X);
        state->cursor_y += direction_speed(state, DIRECTION_DOWN) - direction_speed(state, DIRECTION_UP);
        state->cursor_y = clamp(state->cursor_y, BUTTON_CURSOR_MAX_Y);
        played->mouse_x = state->cursor_x;
        played->mouse_y = state->cursor_y;
        played->mouse_buttons = (raw->dpad[BUTTON_O] ? P8_MOUSE_LEFT : 0) | (raw->dpad[BUTTON_X] ? P8_MOUSE_RIGHT : 0);
    }
    state->previous = *raw;
}
