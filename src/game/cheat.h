#ifndef GAME_CHEAT_H
#define GAME_CHEAT_H

#include "p8/input.h"

#define CHEAT_CHORDS 4

typedef struct {
    int previous_first;
    int previous_second;
    int chords;
} cheat_t;

void cheat_init(cheat_t *cheat);

/* The pad mask of the chord's two buttons, fixed below the bindings. */
int cheat_chord_buttons(void);

/* Fed the raw pad buttons every frame; returns 1 on the frame that completes CHEAT_CHORDS chords of
 * cheat_chord_buttons() (both going down on the same frame) with no lone press of either between them. */
int cheat_step(cheat_t *cheat, int pad_buttons);

#endif
