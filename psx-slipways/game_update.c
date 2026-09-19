#include "game.h"
#include "cursor.h"
#include "input.h"
#include "liaison.h"
#include "rules_tech.h"
#include "sprite_atlas.h"
#include "world.h"

void game_init(void) {
	/* First: uploads the sprite atlas texture/CLUT to VRAM (chunk 4,
	 * docs/prd/triage/psx-sprites-audio-extraction.md), which game_draw.c's
	 * cursor block (and future chunks' planet/liaison-marker blocks) reads
	 * from every frame -- positioned ahead of the other boot-time inits
	 * below since it's the one other draw code depends on, not because
	 * those inits depend on it themselves. */
	sprite_atlas_init();
	world_init();
	cursor_init();
	liaison_init();
	rules_tech_init();
}

void game_update(void) {
	input_update();
	cursor_update(input_get_state().buttons);
	liaison_update(cursor_get_state());
	rules_tech_update(input_get_state().buttons);
}
