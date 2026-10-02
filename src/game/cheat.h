#ifndef GAME_CHEAT_H
#define GAME_CHEAT_H

#include "p8/input.h"

/* The A A B B A A B B code (A = O, button 4; B = X, button 5), read from
 * the input stream alone so that a replay re-applies it on the same frame. */
typedef struct {
    int prev_o;
    int prev_x;
    int matched;
} cheat_t;

void cheat_init(cheat_t *cheat);

/* Call once per frame, before cart_update60(), on that frame's input.
 * Counts rising edges of O and X only; one out of sequence, or both on the
 * same frame, restarts the count from zero, arrows and mouse leave it
 * alone. On the last press it sets the main economy's research_points to
 * 99 and credits to 200 and returns 1, else returns 0. */
int cheat_step(cheat_t *cheat, const p8_input_t *input);

#endif
