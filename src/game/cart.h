#ifndef GAME_CART_H
#define GAME_CART_H

#include "engine/entity_registry.h"
#include "game/save_state.h"
#include "game/ui_button.h"

void cart_load(void);

void cart_flush_exit_queue(void);

void cart_shutdown(void);

void cart_init(void);

/* Between frames only, never from a handler inside ui_update or update_entities: every live entity
   is released before cart_init builds the title. The next cart_resume_game needs one cart_update60
   first, or flip_zone_flip's assert stops it. */
void cart_return_to_title(void);

/* 1 from cart_init until a game starts or cart_load runs, and only while no box is open. */
int cart_title_shown(void);

/* cart_update60 then neither writes nor erases the stored save, until cart_return_to_title or
   cart_load; cart_init alone keeps the cut. */
void cart_cut_save(void);

void cart_init_method(button_t *self);

extern const entity_registry_reserve_t cart_game_registry_reserve;

void cart_start_game(button_t *chosen_option);

/* Contract: cart_load() has run and state came out of a save_state_decode
   that returned 1. Works from the title or a running game, like
   cart_start_game. */
void cart_resume_game(const save_state_t *state);

void cart_update60(void);

void cart_draw(void);

#endif
