#include "game.h"

#include <psxgpu.h>
#include <stdbool.h>

#include "cursor.h"
#include "liaison.h"
#include "rules_tech.h"
#include "screen_map.h"
#include "sprite_atlas.h"
#include "world.h"

/* Arbitrary half-width/half-height in pixels of the wormhole placeholder's
 * POLY_F4 quad -- chosen only to be small and visually distinct from the
 * planet dots, not traced art (see docs/prd/triage/
 * psx-galaxy-rendering.md). The planet dots themselves are no longer a
 * placeholder tile size -- they now draw SPRITE_PLANET_UNKNOWN from the
 * real sprite atlas, see docs/prd/triage/psx-sprites-audio-extraction.md. */
#define GAME_DRAW_WORMHOLE_HALF_SIZE 6

/* Tech menu placeholder tile size/spacing/margin -- v1 UX/rendering
 * choices, not traced art (same framing the wormhole half-size above
 * already established; see
 * docs/prd/triage/psx-rules-tech.md's "Rendering" decision). The cursor
 * is no longer a placeholder tile size -- it now draws SPRITE_CURSOR_DEFAULT
 * from the real sprite atlas, see docs/prd/triage/psx-sprites-audio-extraction.md.
 * Deliberately sized as a top-level UI element, not a world-space
 * marker, independent of the planet/wormhole/cursor sprite sizes above.
 * GAME_DRAW_TECH_MARGIN
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

	/* Chunk 5 (psx-sprites-audio-extraction): draw every generated planet
	 * (positions[0..count)), EXCEPT the wormhole marker's index -- that
	 * one is drawn once, below, as a distinct POLY_F4 instead of being
	 * double-drawn as a plain sprite too -- as the real "unknown planet"
	 * sprite (SPRITE_PLANET_UNKNOWN, a 16x16 2x2-cel sprite in the atlas
	 * sprite_atlas_init() uploaded from game_init()), replacing chunk 3's
	 * flat placeholder TILE.
	 *
	 * Same SPRT/DR_TPAGE mechanism the mouse cursor block below already
	 * established (see that block's own comment for the full rationale),
	 * with the DR_TPAGE hoisted out of the loop and sent once: every
	 * planet uses the same texture page/CLUT, and up to 90 of them are
	 * drawn per frame, so there is no reason to re-emit an identical
	 * DR_TPAGE primitive 90 times.
	 *
	 * Untinted (plain white, no color modulation): confirmed by this
	 * PRD's own Implementation Decisions item 1 trace of `jo:kj`
	 * (pico8/lua.lua:541-551) -- `dv(self.qw)`/`dv()` brackets the
	 * planet sprite draw, but no planet type ever has a `qw` field, so
	 * `self.qw` is always nil and the bracket is a no-op; "planets
	 * always draw with the identity/default palette," i.e. untinted. */
	{
		DR_TPAGE tpage_prim;
		setDrawTPage(&tpage_prim, 0, 1, sprite_atlas_get_tpage());
		DrawPrim(&tpage_prim);
	}

	int i;
	for (i = 0; i < galaxy->count; i++) {
		if (i == wormhole_index) {
			continue;
		}

		int x, y;
		screen_map_planet(galaxy->positions[i], &x, &y);

		int sx, sy, sw, sh;
		sprite_atlas_cel_rect(SPRITE_PLANET_UNKNOWN, 2, 2, &sx, &sy, &sw, &sh);

		SPRT sprite;
		setSprt(&sprite);
		setXY0(&sprite, x - sw / 2, y - sh / 2);
		setUV0(&sprite, sx, sy);
		setWH(&sprite, sw, sh);
		sprite.clut = sprite_atlas_get_clut();
		setRGB0(&sprite, 255, 255, 255); /* untinted -- see comment above */

		DrawPrim(&sprite);
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
		setRGB0(&poly, 255, 64, 0); /* arbitrary placeholder color, distinct from the planet sprite */

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
		setRGB0(&line, 64, 128, 255); /* arbitrary placeholder color, distinct from the planet sprite and wormhole */

		DrawPrim(&line);
	}

	/* Chunk 4 (psx-sprites-audio-extraction): mouse-mode cursor, gated on
	 * cursor_get_state().mouse_mode exactly as before -- the flat
	 * placeholder TILE is replaced by the real cursor sprite
	 * (SPRITE_CURSOR_DEFAULT, an 8x8 1x1-cel sprite in the atlas
	 * sprite_atlas_init() uploaded from game_init()).
	 *
	 * SPRT (not POLY_FT4): PSn00bSDK's dedicated single axis-aligned
	 * UV-mapped rectangle primitive (psxgpu.h's SPRT struct: x0/y0/u0/v0/
	 * w/h -- one position, one size, no per-corner xy/uv quadruple to fill
	 * in by hand) -- the more direct, idiomatic fit for one plain square
	 * sprite than POLY_FT4's 4 explicit corners. Unlike POLY_FT4, SPRT
	 * carries no tpage field of its own, so the active texture page is set
	 * once via a DR_TPAGE primitive immediately before it (matching the
	 * PRD's own "DR_TPAGE set once, not per-primitive" framing -- there's
	 * only one sprite drawn in this block, but the mechanism is the same
	 * one the planet-tile loop will reuse).
	 *
	 * setSprt's own fixed code byte (0x64) already has its raw-texture bit
	 * clear, meaning this primitive is modulated (not raw) by default --
	 * no extra flag needed to keep it that way -- so setRGB0 below keeps
	 * tinting the sprite exactly like it tinted the flat TILE before it;
	 * real PS1 hardware still color-multiplies a modulated textured
	 * primitive by its r0/g0/b0, so the click-flash yellow/white logic is
	 * carried over completely unchanged. */
	{
		cursor_state_t cursor = cursor_get_state();
		if (cursor.mouse_mode) {
			int sx, sy, sw, sh;
			sprite_atlas_cel_rect(SPRITE_CURSOR_DEFAULT, 1, 1, &sx, &sy, &sw, &sh);

			DR_TPAGE tpage_prim;
			setDrawTPage(&tpage_prim, 0, 1, sprite_atlas_get_tpage());
			DrawPrim(&tpage_prim);

			SPRT sprite;
			setSprt(&sprite);
			setXY0(&sprite, cursor.x - sw / 2, cursor.y - sh / 2);
			setUV0(&sprite, sx, sy);
			setWH(&sprite, sw, sh);
			sprite.clut = sprite_atlas_get_clut();
			/* Click-acknowledgment flash (psx-simulated-click.md): yellow
			 * while click_flash_frames is counting down, matching
			 * psx-hello/main.c's exact click_flash color (255, 255, 0);
			 * plain white otherwise -- identical logic to the flat TILE
			 * this block replaced. */
			if (cursor.click_flash_frames > 0) {
				setRGB0(&sprite, 255, 255, 0);
			} else {
				setRGB0(&sprite, 255, 255, 255);
			}

			DrawPrim(&sprite);
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

			/* Chunk 6 (psx-sprites-audio-extraction): invalid-target
			 * marker, layered on top of the line above (drawn after it,
			 * same primitive list). Ports dl()'s spr(31,mp.x-3,mp.y-8)
			 * call (pico8/lua.lua:562-563, reachable via jc:ev whenever
			 * jc:ce()'s candidate blocker self.et is set but has no .qs
			 * text field -- see this PRD's Implementation Decisions item
			 * 4 for the full trace). That maps onto liaison_reason_t as:
			 * already-connected (an existing route object, no .qs),
			 * blocked-by-planet or crosses-route (the first blocker/
			 * crossing ds[1], no .qs) all show the marker; too-far (a
			 * real {qs="-too far-"} text reason, unrendered in this
			 * port -- just the red line alone) and valid (nil, real cost
			 * text instead, also unrendered -- just the green line
			 * alone) do not. Deliberately NOT the same condition as the
			 * red/green line color above: LIAISON_ALREADY_CONNECTED
			 * draws a GREEN line WITH the marker on top (matching
			 * source's own behavior exactly, not a bug). Drawn at the
			 * same bx,by the line's own target endpoint already
			 * computed (the hovered planet's screen position, or the
			 * cursor's raw position), centered like every other sprite
			 * in this file (x - sw/2, y - sh/2) rather than replicating
			 * PICO-8's raw mp.x-3,mp.y-8 offset literally -- same
			 * half-size-centering convention chunk 5's cursor/planet
			 * sprites already established.
			 *
			 * DR_TPAGE kept inline (per-draw), not hoisted: this marker
			 * draws at most once per frame (only while dragging, and
			 * only for 3 of the 5 live.reason values), matching the
			 * mouse cursor block's own single/rare-draw precedent, not
			 * the 90-planet loop's hoisted-for-many-draws precedent. */
			if (liaison.live.reason == LIAISON_ALREADY_CONNECTED ||
				liaison.live.reason == LIAISON_BLOCKED_BY_PLANET ||
				liaison.live.reason == LIAISON_CROSSES_ROUTE) {
				int sx, sy, sw, sh;
				sprite_atlas_cel_rect(SPRITE_LIAISON_INVALID_TARGET, 1, 1, &sx, &sy, &sw, &sh);

				DR_TPAGE tpage_prim;
				setDrawTPage(&tpage_prim, 0, 1, sprite_atlas_get_tpage());
				DrawPrim(&tpage_prim);

				SPRT marker;
				setSprt(&marker);
				setXY0(&marker, bx - sw / 2, by - sh / 2);
				setUV0(&marker, sx, sy);
				setWH(&marker, sw, sh);
				marker.clut = sprite_atlas_get_clut();
				/* Untinted -- dl()'s spr(31,...) call is bare, no
				 * dv()/qw palette bracket around it at all (unlike the
				 * cursor's click-flash tint), per this PRD's own
				 * "Transparency" section. */
				setRGB0(&marker, 255, 255, 255);

				DrawPrim(&marker);
			}
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
			const bool tech_researched[RULES_TECH_ERA1_TILE_COUNT] = { false, false, false, false };

			/* Chunk 8 (psx-sprites-audio-extraction): DR_TPAGE hoisted out
			 * of the loop and sent once, same reasoning as the planet
			 * loop above -- every tile uses the same texture page/CLUT,
			 * so there is no reason to re-emit an identical DR_TPAGE
			 * primitive RULES_TECH_ERA1_TILE_COUNT times. */
			DR_TPAGE tpage_prim;
			setDrawTPage(&tpage_prim, 0, 1, sprite_atlas_get_tpage());
			DrawPrim(&tpage_prim);

			int t;
			for (t = 0; t < RULES_TECH_ERA1_TILE_COUNT; t++) {
				int x = GAME_DRAW_TECH_MARGIN + t * (GAME_DRAW_TECH_TILE_SIZE + GAME_DRAW_TECH_TILE_SPACING);
				int y = GAME_DRAW_TECH_MARGIN;

				TILE tile;
				setTile(&tile);
				setXY0(&tile, x, y);
				setWH(&tile, GAME_DRAW_TECH_TILE_SIZE, GAME_DRAW_TECH_TILE_SIZE);
				/* Cyan for researched (never true today -- the logic is
				 * real and correct if the array ever had a true), gray
				 * for unresearched -- both distinct from every existing
				 * placeholder color in this file (wormhole orange, slipway
				 * blue, cursor white/click-flash yellow, liaison drag
				 * green/red; the planet sprite itself is untinted, not a
				 * placeholder color at all -- see the planet loop's own
				 * comment above). */
				if (tech_researched[t]) {
					setRGB0(&tile, 0, 255, 255);
				} else {
					setRGB0(&tile, 128, 128, 128);
				}

				DrawPrim(&tile);

				/* Chunk 8 (psx-sprites-audio-extraction): the tech's
				 * resource icon, layered on top of the flat TILE above
				 * (drawn after it, same primitive list) -- an addition,
				 * not a swap, per this PRD's own "the existing TILE stays
				 * exactly as-is" instruction. Ports js:kj() ->
				 * self.lq:ol() -> lq:mn()'s bare-number-label closure
				 * (`spr(f,p.x,p.y)`, pico8/lua.lua:300-310), where f is
				 * t.nk after nb's own load-time resource-code resolution
				 * -- rules_tech_get(t)->icon already carries that resolved
				 * value (rules_tech.h's new `icon` field, Implementation
				 * Chunks chunk 8). t here is also a valid tech_id_t: the
				 * loop index matches tech_id_t's own enum order exactly
				 * (TECH_SPACE_FOLDING=0..TECH_DRILLBOTS=3), same
				 * correspondence RULES_TECH_ERA1_TILE_COUNT's own doc
				 * comment already relies on.
				 *
				 * SPRT/DR_TPAGE/setUV0+setWH (sprite_atlas_cel_rect's
				 * 1x1-cel rectangle)/setClut, the same mechanism the
				 * cursor/planet/liaison-marker blocks above already
				 * established -- not drawn at the tile's own x,y directly
				 * (unlike lq:mn()'s literal spr(f,p.x,p.y)) but centered
				 * on the tile like every other sprite in this file (x -
				 * sw/2, y - sh/2 from the tile's own center), since the
				 * tile size here (GAME_DRAW_TECH_TILE_SIZE=8) already
				 * matches the icon's own 8x8 cel exactly, so centering and
				 * top-left-aligning coincide. Untinted (setRGB0(255,255,
				 * 255)), matching lq:mn()'s own bare spr(f,p.x,p.y) call
				 * (no dv()/qw palette bracket around it) -- same
				 * "Transparency"/"Palette-swap mechanism" reasoning as the
				 * liaison-marker block above. */
				{
					int sx, sy, sw, sh;
					sprite_atlas_cel_rect(rules_tech_get((tech_id_t) t)->icon, 1, 1, &sx, &sy, &sw, &sh);

					SPRT icon;
					setSprt(&icon);
					setXY0(&icon, x + GAME_DRAW_TECH_TILE_SIZE / 2 - sw / 2, y + GAME_DRAW_TECH_TILE_SIZE / 2 - sh / 2);
					setUV0(&icon, sx, sy);
					setWH(&icon, sw, sh);
					icon.clut = sprite_atlas_get_clut();
					setRGB0(&icon, 255, 255, 255); /* untinted -- see comment above */

					DrawPrim(&icon);
				}
			}
		}
	}
}
