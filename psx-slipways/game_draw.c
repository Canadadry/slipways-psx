#include "game.h"

#include <psxgpu.h>
#include <stdbool.h>

#include "cursor.h"
#include "liaison.h"
#include "rules_tech.h"
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

/* Tech menu placeholder tile size/spacing/margin -- v1 UX/rendering
 * choices, not traced art (same framing as GAME_DRAW_CURSOR_TILE_SIZE
 * above; see docs/prd/triage/psx-rules-tech.md's "Rendering" decision).
 * Deliberately larger than the planet/cursor tiles above since this is a
 * top-level UI element, not a world-space marker. GAME_DRAW_TECH_MARGIN
 * is a small offset from the screen's (0,0) corner so the tiles aren't
 * drawn flush against the very corner (per the PRD's own "small margin"
 * framing) while still reading as anchored at the top-left, per the
 * source pitch text's "top-left corner" and ix[2]'s own ra=v(0,0)
 * position. */
#define GAME_DRAW_TECH_TILE_SIZE 8
#define GAME_DRAW_TECH_TILE_SPACING 4
#define GAME_DRAW_TECH_MARGIN 8

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

	/* Chunk 5 (psx-liaison-validity): live slipway-validity drag preview,
	 * gated on liaison_get_state().dragging so nothing is drawn when no
	 * drag is in progress (no leftover/stale line). Source point is the
	 * drag's source planet (screen_map_planet, same as every other
	 * primitive above); target point is the hovered planet's screen
	 * position when one is hovered, or the cursor's own raw x/y directly
	 * otherwise -- no unprojection needed here, only the live validity
	 * math itself needs screen_map_unproject (see docs/prd/triage/
	 * psx-liaison-validity.md, "Rendering"). Mirrors the chunk 4
	 * slipway-line LINE_F2 block above exactly. */
	{
		liaison_drag_state_t liaison = liaison_get_state();
		if (liaison.dragging) {
			int ax, ay, bx, by;
			screen_map_planet(galaxy->positions[liaison.from_index], &ax, &ay);

			if (liaison.hover_index != -1) {
				screen_map_planet(galaxy->positions[liaison.hover_index], &bx, &by);
			} else {
				cursor_state_t cursor = cursor_get_state();
				bx = cursor.x;
				by = cursor.y;
			}

			LINE_F2 line;
			setLineF2(&line);
			setXY2(&line, ax, ay, bx, by);
			/* Green when the live result is valid or already-connected,
			 * red otherwise (too far / blocked by planet / crosses an
			 * existing route) -- a v1 UX color choice, not traced art
			 * (same framing as the click-flash yellow above), distinct
			 * from every existing placeholder color in this file. */
			if (liaison.live.reason == LIAISON_VALID || liaison.live.reason == LIAISON_ALREADY_CONNECTED) {
				setRGB0(&line, 0, 255, 0);
			} else {
				setRGB0(&line, 255, 0, 0);
			}

			DrawPrim(&line);
		}
	}

	/* Chunk 4 (psx-rules-tech): tech menu placeholder, gated on
	 * rules_tech_get_state().open so nothing is drawn (no leftover/stale
	 * primitive) when the menu is closed -- mirrors the mouse-mode
	 * cursor's .mouse_mode gate and the liaison drag preview's .dragging
	 * gate above exactly. 4 TILEs, one per Era-1 tech in tech_id_t's own
	 * enum order (TECH_SPACE_FOLDING, TECH_GENESEEDS, TECH_NANOMATERIALS,
	 * TECH_DRILLBOTS), laid out as a single horizontal row anchored at
	 * the screen's top-left corner (v1 layout choice among equally valid
	 * options like a 2x2 grid -- a single row reads cleanest for 4
	 * same-size tiles and needs no row-wrap logic). See
	 * GAME_DRAW_TECH_TILE_SIZE/_SPACING/_MARGIN above for the size/
	 * spacing/margin rationale. */
	{
		rules_tech_state_t tech = rules_tech_get_state();
		if (tech.open) {
			/* Caller-local placeholder "researched" state (PRD's own
			 * explicit instruction): no live science economy exists yet
			 * to populate this -- Phase 3 step 13 / Phase 4 step 14.
			 * Deliberately not owned by rules_tech.c itself (which has
			 * no way to ever set an entry true yet); this keeps the
			 * render call site reading real, if currently constant,
			 * parameters, the same shape as the liaison preview's
			 * live.reason-driven color above. */
			const bool tech_researched[TECH_COUNT] = { false, false, false, false };

			int t;
			for (t = 0; t < TECH_COUNT; t++) {
				int x = GAME_DRAW_TECH_MARGIN + t * (GAME_DRAW_TECH_TILE_SIZE + GAME_DRAW_TECH_TILE_SPACING);
				int y = GAME_DRAW_TECH_MARGIN;

				TILE tile;
				setTile(&tile);
				setXY0(&tile, x, y);
				setWH(&tile, GAME_DRAW_TECH_TILE_SIZE, GAME_DRAW_TECH_TILE_SIZE);
				/* Cyan for researched (never true today -- the logic is
				 * real and correct if the array ever had a true), gray
				 * for unresearched -- both distinct from every existing
				 * placeholder color in this file (planet green, wormhole
				 * orange, slipway blue, cursor white/click-flash yellow,
				 * liaison drag green/red). */
				if (tech_researched[t]) {
					setRGB0(&tile, 0, 255, 255);
				} else {
					setRGB0(&tile, 128, 128, 128);
				}

				DrawPrim(&tile);
			}
		}
	}
}
