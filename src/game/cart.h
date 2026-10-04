#ifndef GAME_CART_H
#define GAME_CART_H

#include "engine/entity_registry.h"
#include "game/ui_button.h"

void cart_load(void);

void cart_flush_exit_queue(void);

void cart_shutdown(void);

void cart_init(void);

void cart_init_method(button_t *self);

extern const entity_registry_reserve_t cart_game_registry_reserve;

void cart_start_game(button_t *chosen_option);

void cart_update60(void);

void cart_draw(void);

#endif
