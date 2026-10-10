#include "game/input_step.h"

#include "engine/dispatch_context.h"

#include <assert.h>
#include <string.h>

void input_step_init(input_step_t *step, const bindings_t *bindings) {
    memset(step, 0, sizeof *step);
    cheat_init(&step->cheat);
    pad_nav_init(&step->pad_nav, bindings);
    step->session_bindings = *bindings;
}

void input_step_switch_bindings(input_step_t *step, input_step_bindings_t which) {
    assert((which == INPUT_STEP_BINDINGS_SESSION || which == INPUT_STEP_BINDINGS_DEFAULT)
           && "input_step_switch_bindings: which is the session or the default table");
    bindings_t session = step->session_bindings;
    input_step_init(step, which == INPUT_STEP_BINDINGS_DEFAULT ? &bindings_default : &session);
    step->session_bindings = session;
}

int input_step_run(input_step_t *step, const p8_input_frame_t *raw) {
    memset(&step->played_frame, 0, sizeof step->played_frame);
    step->played_frame.pad_buttons = raw->pad_buttons;
    pad_nav_step(&step->pad_nav, raw->pad_buttons, &step->played_frame);
    /* Set every frame, not at init: the struct may have been copied since. */
    step->played.frames = &step->played_frame;
    step->played.frame_count = 1;
    step->played.current_frame = 0;
    int fired = cheat_step(&step->cheat, raw->pad_buttons);
    dispatch_context_set_input(&step->played);
    return fired;
}

const p8_input_frame_t *input_step_played_frame(const input_step_t *step) {
    return &step->played_frame;
}
