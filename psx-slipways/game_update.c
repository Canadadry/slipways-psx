#include "game.h"
#include "input.h"
#include "world.h"

void game_init(void) {
	world_init();
}

void game_update(void) {
	input_update();
}
