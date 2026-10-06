#include "game/input_step.h"

#include "engine/dispatch_context.h"

#include <string.h>

void input_step_init(input_step_t *step) {
    memset(step, 0, sizeof *step);
    cheat_init(&step->cheat);
    pad_nav_init(&step->pad_nav);
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
