#ifndef GAME_CHEAT_H
#define GAME_CHEAT_H

#include "p8/input.h"

#define CHEAT_CHORDS 4

typedef struct {
    int previous_cross;
    int previous_circle;
    int chords;
} cheat_t;

void cheat_init(cheat_t *cheat);

/* Fed the raw pad buttons every frame; returns 1 on the frame that completes CHEAT_CHORDS × + ○ chords (both going
 * down on the same frame) with no lone × or ○ press between them. */
int cheat_step(cheat_t *cheat, int pad_buttons);

#endif
