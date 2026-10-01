#include "p8/input.h"

#include <assert.h>
#include <stddef.h>

void pico8_input_init(pico8_input_t *input) {
    input->frames = NULL;
    input->frame_count = 0;
    input->current_frame = 0;
}

void pico8_input_step(pico8_input_t *input) {
    assert(input->current_frame + 1 < input->frame_count);
    input->current_frame++;
}

int pico8_btn(const pico8_input_t *input, int b) {
    if (b < 0 || b > 5) {
        return 0;
    }
    if (input->frame_count == 0 || input->current_frame >= input->frame_count) {
        return 0;
    }
    return input->frames[input->current_frame].dpad[b];
}

int pico8_stat(const pico8_input_t *input, int n) {
    if (input->frame_count == 0 || input->current_frame >= input->frame_count) {
        return 0;
    }
    const pico8_input_frame_t *f = &input->frames[input->current_frame];
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
