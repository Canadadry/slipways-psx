#include "game/cheat.h"

#include "p8/fixed.h"
#include "game/main_economy.h"

#include <stddef.h>

#define BTN_O 4
#define BTN_X 5

static const int sequence[] = {BTN_O, BTN_O, BTN_X, BTN_X, BTN_O, BTN_O, BTN_X, BTN_X};
#define SEQUENCE_LENGTH ((int)(sizeof sequence / sizeof sequence[0]))

void cheat_init(cheat_t *cheat) {
    cheat->prev_o = 0;
    cheat->prev_x = 0;
    cheat->matched = 0;
}

/* Before cart_start_game(), the main economy is still the placeholder, and
 * the code is typed on the title screen too: the cheat writes it there. */
static void apply(void) {
    entity_t *fm = main_economy();
    economy_t *hy;
    if (fm == NULL) {
        return;
    }
    if (((const entity_t *)fm)->class == main_economy_placeholder_class()) {
        main_economy_placeholder_t *provisional = (main_economy_placeholder_t *)fm;
        provisional->research_points = p8_fixed_from_int(99);
        provisional->has_research_points = 1;
        provisional->credits = p8_fixed_from_int(200);
        provisional->has_credits = 1;
        return;
    }
    hy = main_economy_typed();
    hy->research_points = p8_fixed_from_int(99);
    hy->credits = p8_fixed_from_int(200);
}

static int feed(cheat_t *cheat, int button) {
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

int cheat_step(cheat_t *cheat, const p8_input_t *input) {
    int o = p8_btn(input, BTN_O);
    int x = p8_btn(input, BTN_X);
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
