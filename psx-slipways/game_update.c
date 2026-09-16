#include "game.h"
#include "cursor.h"
#include "input.h"
#include "liaison.h"
#include "world.h"

void game_init(void) {
	world_init();
	cursor_init();
	liaison_init();
}

void game_update(void) {
	input_update();
	cursor_update(input_get_state().buttons);
	liaison_update(cursor_get_state());
}
