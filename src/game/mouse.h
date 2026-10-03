#ifndef GAME_MOUSE_H
#define GAME_MOUSE_H

#include "game/game_data.h"
#include "p8/input.h"
#include "engine/entity.h"
#include "game/mouse_button.h"
#include "engine/entity_class.h"
#include "engine/vec.h"

/* mouse_update returns early on a NULL struct or input. */

/* The left and right button states are not members: every mouse reads,
 * and mouse_button_update mutates, the same two states, shared by the
 * class. The position starts on a (0,0) every mouse shares; the first
 * mouse_update points it at `pos_own`, rewritten in place from then on.
 * Never copy the struct. */
typedef struct {
    entity_t header;
    const mouse_archetype_t *archetype;
    vec_t *pos;
    vec_t pos_own;
} mouse_t;

/* Calls draw_functions_init(), and requires entity_init() to have run --
 * asserted, not repaired. Starts the shared button states and the shared
 * position afresh. */
void mouse_init(void);

/* Only lowers the ready flag: the button states are static, never freed,
 * and mouse_left_button()/mouse_right_button() return NULL from then on. */
void mouse_shutdown(void);

/* The shared button states, NULL outside mouse_init()..mouse_shutdown(). */
const mouse_button_t *mouse_left_button(void);
const mouse_button_t *mouse_right_button(void);

/* 1 from mouse_init() to mouse_shutdown(), 0 otherwise. */
int mouse_ready(void);
const entity_class_t *mouse_class(void);

/* The struct enables devkit mouse reporting, registers itself through
 * entity_register; it lives in the current game-zone generation. */
mouse_t *mouse_new(void);

/* Rewrites `pos_own` with the mouse position and points the struct's
 * position at it, then runs mouse_button_update over the shared button
 * states, which are mutated in place and never rebound. */
void mouse_update(const p8_input_t *input, mouse_t *self);

#endif
