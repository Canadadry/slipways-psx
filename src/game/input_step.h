#ifndef GAME_INPUT_STEP_H
#define GAME_INPUT_STEP_H

#include "game/cheat.h"
#include "game/pad_nav.h"
#include "p8/input.h"

typedef struct {
    cheat_t cheat;
    pad_nav_t pad_nav;
    p8_input_frame_t played_frame;
    p8_input_t played;
} input_step_t;

void input_step_init(input_step_t *step, const bindings_t *bindings);

int input_step_run(input_step_t *step, const p8_input_frame_t *raw);

const p8_input_frame_t *input_step_played_frame(const input_step_t *step);

#endif
