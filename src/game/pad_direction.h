#ifndef GAME_PAD_DIRECTION_H
#define GAME_PAD_DIRECTION_H

/* Not button bits: pad_jump_direction_button gives the button that plays a direction. */
typedef enum {
    PAD_DIRECTION_NONE,
    PAD_DIRECTION_UP,
    PAD_DIRECTION_DOWN,
    PAD_DIRECTION_LEFT,
    PAD_DIRECTION_RIGHT
} pad_direction_t;

#endif
