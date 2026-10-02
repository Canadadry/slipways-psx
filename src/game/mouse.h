#ifndef GAME_MOUSE_H
#define GAME_MOUSE_H

#include "game/game_data.h"
#include "p8/input.h"
#include "engine/entity.h"
#include "game/mouse_button.h"
#include "engine/entity_class.h"
#include "engine/vec.h"

/* Throughout this module a NULL struct, `mem` or `input` argument is a
 * safe no-op. */

/* `l` and `r` are not members: every nf reads and bu() mutates the same
 * two button tables, the class's shared state. `ra` starts on a (0,0)
 * every nf shares; the first `ph` points it at `ra_own`, rewritten in place
 * from then on. Never copy the struct. */
typedef struct {
    entity_t header;
    const mouse_archetype_t *archetype;
    vec_t *pos;
    vec_t pos_own;
} mouse_t;

/* Calls pico8_pi_init(), and requires pico8_ke_init() to have run --
 * asserted, not repaired. Starts the shared `l`/`r` tables afresh, as
 * rebuilding the class did in the cart. */
void mouse_init(void);

/* Frees the shared `l`/`r` tables. */
void mouse_shutdown(void);

/* The shared `l`/`r` tables, NULL outside pico8_nf_init()..pico8_nf_shutdown(). */
const mouse_button_t *mouse_left_button(void);
const mouse_button_t *mouse_right_button(void);

/* 1 from pico8_nf_init() to pico8_nf_shutdown(), 0 otherwise. */
int mouse_ready(void);
const entity_class_t *mouse_class(void);

/* The struct enables devkit mouse reporting, registers itself through `lt`;
 * it lives in the current game-zone generation. */
mouse_t *mouse_new(void);

/* Rewrites `self.ra` in place with the mouse position, then runs bu()
 * over the shared `l`/`r` tables, which are mutated in place and never
 * rebound. */
void mouse_update(const p8_input_t *input, mouse_t *self);

#endif
