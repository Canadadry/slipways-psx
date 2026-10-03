#include "p8/input.h"

#include <assert.h>
#include <stddef.h>

void p8_input_init(p8_input_t *input) {
    input->frames = NULL;
    input->frame_count = 0;
    input->current_frame = 0;
}

void p8_input_step(p8_input_t *input) {
    assert(input->current_frame + 1 < input->frame_count);
    input->current_frame++;
}

int p8_btn(const p8_input_t *input, int b) {
    if (b < 0 || b > 5) {
        return 0;
    }
    if (input->frame_count == 0 || input->current_frame >= input->frame_count) {
        return 0;
    }
    return input->frames[input->current_frame].dpad[b];
}

int p8_stat(const p8_input_t *input, int n) {
    if (input->frame_count == 0 || input->current_frame >= input->frame_count) {
        return 0;
    }
    const p8_input_frame_t *f = &input->frames[input->current_frame];
    switch (n) {
        case 32:
            return f->mouse_x;
        case 33:
            return f->mouse_y;
        case 34:
            return f->mouse_buttons;
        default:
            return 0;
    }
}
