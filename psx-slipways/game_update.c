#include "game.h"
#include "cursor.h"
#include "input.h"
#include "world.h"

void game_init(void) {
	world_init();
	cursor_init();
}

void game_update(void) {
	input_update();
	cursor_update(input_get_state().buttons);
}
