#include "pico8_cheat.h"

#include "pico8_fixed.h"
#include "pico8_probe.h"
#include "pico8_value.h"

#include <stddef.h>

#define BTN_O 4
#define BTN_X 5

static const int sequence[] = {BTN_O, BTN_O, BTN_X, BTN_X, BTN_O, BTN_O, BTN_X, BTN_X};
#define SEQUENCE_LENGTH ((int)(sizeof sequence / sizeof sequence[0]))

void pico8_cheat_init(pico8_cheat_t *cheat) {
    cheat->prev_o = 0;
    cheat->prev_x = 0;
    cheat->matched = 0;
}

/* Before `eu()`, `fm` is still the provisional, and the code is typed on
 * the title screen too: the cheat writes the provisional there. */
static void apply(void) {
    pico8_struct_header_t *fm = pico8_fm();
    pico8_hy_t *hy;
    if (fm == NULL) {
        return;
    }
    if (((const pico8_struct_header_t *)fm)->cls == pico8_fm_placeholder_struct_class()) {
        pico8_fm_placeholder_t *provisional = (pico8_fm_placeholder_t *)fm;
        provisional->jk = pico8_value_fixed(pico8_fixed_from_int(99));
        provisional->nz = pico8_value_fixed(pico8_fixed_from_int(200));
        return;
    }
    hy = pico8_fm_hy();
    hy->jk = pico8_value_fixed(pico8_fixed_from_int(99));
    hy->nz = pico8_value_fixed(pico8_fixed_from_int(200));
}

static int feed(pico8_cheat_t *cheat, int button) {
    if (sequence[cheat->matched] != button) {
        cheat->matched = 0;
        return 0;
    }
    cheat->matched++;
    if (cheat->matched < SEQUENCE_LENGTH) {
        return 0;
    }
    cheat->matched = 0;
    apply();
    return 1;
}

int pico8_cheat_step(pico8_cheat_t *cheat, const pico8_input_t *input) {
    int o = pico8_btn(input, BTN_O);
    int x = pico8_btn(input, BTN_X);
    int o_edge = o && !cheat->prev_o;
    int x_edge = x && !cheat->prev_x;
    int fired = 0;

    cheat->prev_o = o;
    cheat->prev_x = x;

    if (o_edge && x_edge) {
        cheat->matched = 0;
    } else if (o_edge) {
        fired = feed(cheat, BTN_O);
    } else if (x_edge) {
        fired = feed(cheat, BTN_X);
    }
    return fired;
}
