#include "game.h"

#include <psxgpu.h>

#include "screen_map.h"
#include "world.h"

/* Arbitrary size, not traced art -- chunk 1's only goal is proving the
 * TILE/DrawPrim primitive path end-to-end (see docs/prd/triage/
 * psx-galaxy-rendering.md). */
#define GAME_DRAW_PLANET_TILE_SIZE 4

void game_draw(void) {
	const galaxy_t *galaxy = world_get_galaxy();

	/* Chunk 2 scope: draw every generated planet (positions[0..count))
	 * as a TILE primitive, all with the same placeholder color. No
	 * wormhole/slipways yet -- those are chunk 3/4's job. */
	int i;
	for (i = 0; i < galaxy->count; i++) {
		int x, y;
		screen_map_planet(galaxy->positions[i], &x, &y);

		TILE tile;
		setTile(&tile);
		setXY0(&tile, x - GAME_DRAW_PLANET_TILE_SIZE / 2, y - GAME_DRAW_PLANET_TILE_SIZE / 2);
		setWH(&tile, GAME_DRAW_PLANET_TILE_SIZE, GAME_DRAW_PLANET_TILE_SIZE);
		setRGB0(&tile, 0, 255, 128); /* arbitrary placeholder color */

		DrawPrim(&tile);
	}
}
