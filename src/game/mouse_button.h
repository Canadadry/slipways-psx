#ifndef GAME_MOUSE_BUTTON_H
#define GAME_MOUSE_BUTTON_H

/* One button's state, left or right: until the first mouse_button_update,
 * `has_held` is 0 and `held`/`pressed`/`released` are absent. */
typedef struct {
    int has_held;
    int held;
    int pressed;
    int released;
} mouse_button_t;

/* The button state is a plain record, not an instance: its members are
 * read and written directly. It is dereferenced, so it may not be NULL,
 * and nothing here is allocated or freed. `down` is 0 or 1. */
void mouse_button_update(mouse_button_t *button, int down);

#endif
