#ifndef GAME_INPUT_STEP_H
#define GAME_INPUT_STEP_H

#include "game/cheat.h"
#include "game/pad_nav.h"
#include "game/save_state.h"
#include "p8/input.h"

typedef struct {
    cheat_t cheat;
    pad_nav_t pad_nav;
    p8_input_frame_t played_frame;
    p8_input_t played;
    bindings_t session_bindings;
} input_step_t;

typedef enum { INPUT_STEP_BINDINGS_SESSION, INPUT_STEP_BINDINGS_DEFAULT } input_step_bindings_t;

void input_step_init(input_step_t *step, const bindings_t *bindings);

void input_step_switch_bindings(input_step_t *step, input_step_bindings_t which);

/* At the top of a frame only, before its input_step_run: right after boot's cart_init, or after
   at least one cart_update60 since the last return (else flip_zone_flip asserts). step was
   initialised by input_step_init. Switches to the default table, then cart_resume_game(state). */
void input_step_start_record(input_step_t *step, const save_state_t *state);

/* At the top of a frame only: cart_return_to_title, then back to the session's table. */
void input_step_return_to_title(input_step_t *step);

int input_step_run(input_step_t *step, const p8_input_frame_t *raw);

const p8_input_frame_t *input_step_played_frame(const input_step_t *step);

#endif
