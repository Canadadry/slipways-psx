#include "game.h"

#include <psxgpu.h>

#include "cursor.h"
#include "screen_map.h"
#include "world.h"

/* Arbitrary size, not traced art -- chunk 1's only goal is proving the
 * TILE/DrawPrim primitive path end-to-end (see docs/prd/triage/
 * psx-galaxy-rendering.md). */
#define GAME_DRAW_PLANET_TILE_SIZE 4

/* Arbitrary half-width/half-height in pixels of the wormhole placeholder's
 * POLY_F4 quad -- chosen only to be small and visually distinct from the
 * GAME_DRAW_PLANET_TILE_SIZE tile dots, not traced art (see docs/prd/
 * triage/psx-galaxy-rendering.md). */
#define GAME_DRAW_WORMHOLE_HALF_SIZE 6

/* Arbitrary size for the mouse-mode cursor marker -- a v1 UX/rendering
 * choice, not traced art (see docs/prd/triage/psx-mouse-mode-cursor.md's
 * "Rendering the cursor" decision). Only needs to be small and visually
 * distinct from GAME_DRAW_PLANET_TILE_SIZE; doesn't need to match it. */
#define GAME_DRAW_CURSOR_TILE_SIZE 6

void game_draw(void) {
	const slipway_edge_t *slipway_edges;
	int slipway_edge_count;
	const galaxy_t *galaxy = world_get_galaxy();
	int wormhole_index = world_get_wormhole_index();

	/* Chunk 3 scope: draw every generated planet (positions[0..count))
	 * as a TILE primitive, EXCEPT the wormhole marker's index -- that
	 * one is drawn once, below, as a distinct POLY_F4 instead of being
	 * double-drawn as a plain tile too. No slipways yet -- that's
	 * chunk 4's job. */
	int i;
	for (i = 0; i < galaxy->count; i++) {
		if (i == wormhole_index) {
			continue;
		}

		int x, y;
		screen_map_planet(galaxy->positions[i], &x, &y);

		TILE tile;
		setTile(&tile);
		setXY0(&tile, x - GAME_DRAW_PLANET_TILE_SIZE / 2, y - GAME_DRAW_PLANET_TILE_SIZE / 2);
		setWH(&tile, GAME_DRAW_PLANET_TILE_SIZE, GAME_DRAW_PLANET_TILE_SIZE);
		setRGB0(&tile, 0, 255, 128); /* arbitrary placeholder color */

		DrawPrim(&tile);
	}

	/* placeholder position for the wormhole GPU-primitive proof; the real
	 * wormhole is placed by the player's first click, Phase 4 step 15 --
	 * this is not that. */
	{
		int wx, wy;
		screen_map_planet(galaxy->positions[wormhole_index], &wx, &wy);

		POLY_F4 poly;
		setPolyF4(&poly);
		/* Corner order matches setXYWH's convention: top-left,
		 * top-right, bottom-left, bottom-right. */
		setXY4(&poly,
			wx - GAME_DRAW_WORMHOLE_HALF_SIZE, wy - GAME_DRAW_WORMHOLE_HALF_SIZE,
			wx + GAME_DRAW_WORMHOLE_HALF_SIZE, wy - GAME_DRAW_WORMHOLE_HALF_SIZE,
			wx - GAME_DRAW_WORMHOLE_HALF_SIZE, wy + GAME_DRAW_WORMHOLE_HALF_SIZE,
			wx + GAME_DRAW_WORMHOLE_HALF_SIZE, wy + GAME_DRAW_WORMHOLE_HALF_SIZE);
		setRGB0(&poly, 255, 64, 0); /* arbitrary placeholder color, distinct from planet tiles */

		DrawPrim(&poly);
	}

	/* Chunk 4: draw world.c's capped, deterministic synthetic slipway-line
	 * geometry (see slipway_bench.h) as LINE_F2 primitives. Pure geometry
	 * standing in for "slipways" for GPU-primitive-path/FPS-feasibility
	 * purposes only -- no cost, no validity, no game meaning; the real
	 * slipway logic is Phase 3 step 9's job (see docs/prd/triage/
	 * psx-galaxy-rendering.md). */
	world_get_slipway_edges(&slipway_edges, &slipway_edge_count);

	int e;
	for (e = 0; e < slipway_edge_count; e++) {
		int ax, ay, bx, by;
		screen_map_planet(galaxy->positions[slipway_edges[e].a], &ax, &ay);
		screen_map_planet(galaxy->positions[slipway_edges[e].b], &bx, &by);

		LINE_F2 line;
		setLineF2(&line);
		setXY2(&line, ax, ay, bx, by);
		setRGB0(&line, 64, 128, 255); /* arbitrary placeholder color, distinct from planet tiles and wormhole */

		DrawPrim(&line);
	}

	/* Chunk 3 (psx-mouse-mode-cursor): mouse-mode cursor marker, gated on
	 * cursor_get_state().mouse_mode so it's invisible until the player
	 * toggles it on with Cross. Mirrors the planet TILE code above exactly
	 * (setTile/setXY0/setWH/setRGB0/DrawPrim, centered on the coordinate
	 * via a half-size offset), per the PRD's "Rendering the cursor"
	 * decision. Plain white, distinct from the planet (green), wormhole
	 * (orange), and slipway (blue) placeholder colors above. */
	{
		cursor_state_t cursor = cursor_get_state();
		if (cursor.mouse_mode) {
			TILE tile;
			setTile(&tile);
			setXY0(&tile, cursor.x - GAME_DRAW_CURSOR_TILE_SIZE / 2, cursor.y - GAME_DRAW_CURSOR_TILE_SIZE / 2);
			setWH(&tile, GAME_DRAW_CURSOR_TILE_SIZE, GAME_DRAW_CURSOR_TILE_SIZE);
			/* Click-acknowledgment flash (psx-simulated-click.md): yellow
			 * while click_flash_frames is counting down, matching
			 * psx-hello/main.c's exact click_flash color (255, 255, 0);
			 * plain white otherwise, distinct from planet/wormhole/slipway
			 * colors above. */
			if (cursor.click_flash_frames > 0) {
				setRGB0(&tile, 255, 255, 0);
			} else {
				setRGB0(&tile, 255, 255, 255);
			}

			DrawPrim(&tile);
		}
	}
}
