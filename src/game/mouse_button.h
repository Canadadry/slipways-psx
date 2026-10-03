#ifndef GAME_MOUSE_BUTTON_H
#define GAME_MOUSE_BUTTON_H

#include "p8/input.h"

/* One button's state, left or right: until the first mouse_button_update,
 * `has_held` is 0 and `held`/`pressed`/`released` are absent. */
typedef struct {
    int has_held;
    int held;
    int pressed;
    int released;
} mouse_button_t;

/* The button state is a plain record, not an instance: its members are
 * read and written directly. The input and the button state are both
 * dereferenced, so neither may be NULL, and nothing here is allocated or
 * freed. The mask is a raw stat(34) bitmask (1 left, 6 right), applied with
 * an ordinary integer `&` since neither side is fractional. */
void mouse_button_update(const p8_input_t *input, mouse_button_t *qn, int oa);

#endif
