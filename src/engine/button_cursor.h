#ifndef ENGINE_BUTTON_CURSOR_H
#define ENGINE_BUTTON_CURSOR_H

#include "p8/input.h"

typedef enum {
    BUTTON_CURSOR_OWNER_MOUSE,
    BUTTON_CURSOR_OWNER_BUTTONS
} button_cursor_owner_t;

/* Owned by the caller; read through button_cursor_step only. */
typedef struct {
    button_cursor_owner_t owner;
    int cursor_x;
    int cursor_y;
    int held_frames[4];
    p8_input_frame_t previous;
    int has_previous;
} button_cursor_t;

void button_cursor_init(button_cursor_t *state);

/* Call once per frame on that frame's raw input; writes the frame the game
 * reads into `played`. While the mouse owns the cursor `played` is `raw`;
 * `played`'s O and X always equal `raw`'s, so the cheat reads either. */
void button_cursor_step(button_cursor_t *state, const p8_input_frame_t *raw, p8_input_frame_t *played);

#endif
