#include "game/input_step.h"

#include "engine/dispatch_context.h"

#include <assert.h>
#include <string.h>

#define BTN_O 4
#define BTN_X 5

/* Mouse mode reads the raw frame, not the played one: the keyboard_research replay depends on it. */
static int run_cheat(cheat_t *cheat, p8_input_mode_t mode, const p8_input_frame_t *raw) {
    p8_input_frame_t view = *raw;
    if (mode == P8_INPUT_PAD) {
        memset(&view, 0, sizeof view);
        view.dpad[BTN_O] = (raw->pad_buttons & P8_PAD_CROSS) != 0;
        view.dpad[BTN_X] = (raw->pad_buttons & P8_PAD_CIRCLE) != 0;
    }
    p8_input_t input = { .frames = &view, .frame_count = 1, .current_frame = 0 };
    return cheat_step(cheat, &input);
}

void input_step_init(input_step_t *step, p8_input_mode_t mode, int keyboard_cursor) {
    assert(!(mode == P8_INPUT_PAD && keyboard_cursor) && "input_step_init: keyboard_cursor is a mouse-mode flag, a pad session cannot run button_cursor");
    memset(step, 0, sizeof *step);
    step->mode = mode;
    step->keyboard_cursor = keyboard_cursor;
    button_cursor_init(&step->cursor);
    cheat_init(&step->cheat);
}

int input_step_run(input_step_t *step, const p8_input_frame_t *raw) {
    if (step->mode == P8_INPUT_PAD) {
        memset(&step->played_frame, 0, sizeof step->played_frame);
        step->played_frame.pad_buttons = raw->pad_buttons;
    } else if (step->keyboard_cursor) {
        button_cursor_step(&step->cursor, raw, &step->played_frame);
    } else {
        step->played_frame = *raw;
    }
    /* Set every frame, not at init: the struct may have been copied since. */
    step->played.frames = &step->played_frame;
    step->played.frame_count = 1;
    step->played.current_frame = 0;
    /* TODO step 1.4: pad navigation runs here, between the mode and the cheat. */
    int fired = run_cheat(&step->cheat, step->mode, raw);
    dispatch_context_set_input(&step->played);
    return fired;
}

const p8_input_frame_t *input_step_played_frame(const input_step_t *step) {
    return &step->played_frame;
}
