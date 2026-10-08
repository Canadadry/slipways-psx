#include "game/cheat.h"

#include "p8/fixed.h"
#include "game/main_economy.h"

#include <stddef.h>

void cheat_init(cheat_t *cheat) {
    cheat->previous_cross = 0;
    cheat->previous_circle = 0;
    cheat->chords = 0;
}

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

int cheat_step(cheat_t *cheat, int pad_buttons) {
    int cross = (pad_buttons & P8_PAD_CROSS) != 0;
    int circle = (pad_buttons & P8_PAD_CIRCLE) != 0;
    int cross_edge = cross && !cheat->previous_cross;
    int circle_edge = circle && !cheat->previous_circle;

    cheat->previous_cross = cross;
    cheat->previous_circle = circle;

    if (!cross_edge && !circle_edge) {
        return 0;
    }
    if (!(cross_edge && circle_edge)) {
        cheat->chords = 0;
        return 0;
    }
    cheat->chords++;
    if (cheat->chords < CHEAT_CHORDS) {
        return 0;
    }
    cheat->chords = 0;
    apply();
    return 1;
}
