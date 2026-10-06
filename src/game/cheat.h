#ifndef GAME_CHEAT_H
#define GAME_CHEAT_H

#include "p8/input.h"

typedef struct {
    int prev_o;
    int prev_x;
    int matched;
} cheat_t;

void cheat_init(cheat_t *cheat);

int cheat_step(cheat_t *cheat, int pad_buttons);

#endif
