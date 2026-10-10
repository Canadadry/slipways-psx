---
title: "Captions and label"
description: "The attract mode plays the compiled demo in the core, but nothing on screen says what the demo teaches or that it is a demo, so a passer-by sees an unexplained game playing itself and may take it for their own."
status: needs-triage
---

## Problem Statement

Plan 18, phase 2, step 11, verbatim:

> 11. **Captions and label.** Draw the current caption on its band and the `demo` label, in the core's draw. Choose the band's place (not over the top HUD or the build bar) and write it in the step note. Verified by a unit test reading the framebuffer for a caption frame and a captionless frame, and every existing `.sha256` unchanged.

After step 10 the title starts the compiled demo by itself and plays it in the core, but nothing is drawn on top of it:

- The seven captions of plan section 2 ("build a trading empire / across the stars", …, "slipways / press any button") are compiled into the core (`demo_data_captions`, step 8), each with a `{from, to}` range of demo frames, but no code reads them.
- Nothing tells the viewer the game is playing itself. A player who walks back to the screen during the demo sees a galaxy that is not theirs, with credits and colonies they never had, and may think their save was replaced.
- The captions' text joins its lines with `'\n'` (step 6), and `p8_print` cannot draw a newline: it would draw the placeholder block in its place.

## Solution

From the viewer's side:

1. While the demo plays, a dark band runs across the whole screen just above the bottom HUD, and the current caption is written on it in white, each line centred. The band is in the same place, with the same height, for every caption, including the closing "slipways / press any button".
2. A small red `demo` tag hangs under the top HUD in the top-right corner for the whole playback, from the demo's first frame to its last.
3. The top HUD, the bottom HUD (menu button, credits and science readouts, build icons) and the tech panel stay fully visible: the band sits between the map and the bottom HUD, the tag below the top HUD.
4. Neither the band nor the tag is ever drawn on the title, in a game the player plays, on the frame that interrupts the demo, while the interrupting press is swallowed, or after the demo's last frame. The title after the demo is pixel-for-pixel the title before it.

From the code's side: the captions are drawn into the PICO-8 framebuffer with `p8_print` and `p8_rectfill`, at the end of `cart_draw`, after both draw layers. So the headless dump, the e2e captures and the PSX all see them, with no raylib overlay. The core's draw takes no argument, so `input_step_run` gives the cart the index of the demo frame it just played (or "none") on every frame. The caption is looked up from that index alone. It does not depend on the visual random stream, so both playbacks of one process show the same captions on the same demo frames, even though the second playback draws its map differently (step 4, item 1). Nothing is drawn unless the attract mode is playing, and no existing replay ever reaches that phase, so every existing `*.sha256`, `.end-state` and `worst-case/` file stays unchanged.

## User Stories

1. As a passer-by, I want a caption saying what the game is about while it plays itself, so that I learn the game without reading a manual.
2. As a passer-by, I want each caption to stay on screen while its beat plays (probe, colonize, slipway, end of year, tech), so that the words match what I see.
3. As a passer-by, I want the captions on a dark band, so that the text is readable over stars, planets and slipways.
4. As a passer-by, I want the caption centred, so that it reads like a title card and not like a game message.
5. As a passer-by, I want a two-line caption to show as two lines, so that "build a trading empire / across the stars" never shows a block where the line breaks.
6. As a passer-by, I want the band in the same place and at the same height for every caption, so that my eye does not have to look for it.
7. As a passer-by, I want the one-line tech caption centred vertically on the band, so that the band does not look half empty.
8. As a passer-by, I want the closing "slipways / press any button" on the same band, so that the end of the demo reads like the rest of it.
9. As a player coming back to the screen, I want a `demo` tag visible during the whole playback, so that I never take the demo for my own game.
10. As a player coming back to the screen, I want the tag in a corner and small, so that it does not hide the action.
11. As a player, I want the tag and the band gone as soon as I press a button, so that the title I come back to is clean.
12. As a player, I want the title after the demo to look exactly like the title before it, so that nothing from the demo stays on screen.
13. As a player in my own game, I want no band and no tag ever, so that the attract mode never shows up in my game.
14. As a player, I want the top HUD (buttons at y≈1) never covered, so that the demo shows the real interface.
15. As a player, I want the bottom HUD and build icons (y≈112–119) never covered, so that the demo shows where building happens.
16. As a player, I want the tech panel never covered during the tech beat, so that the purchase the caption talks about is visible.
17. As a maintainer, I want the captions drawn inside the core's draw, so that `game`, `replay`, the dump and the e2e captures show the same pixels.
18. As a PSX porter, I want the captions drawn with the PICO-8 primitives into the framebuffer, so that the console gets them with no raylib code.
19. As a maintainer, I want the caption chosen from the demo frame index alone, so that the second playback (whose visual stream differs) shows the same captions at the same frames.
20. As a maintainer, I want the overlay drawn with its own copy of the graphics state (camera and fill pattern reset), so that a camera left set by a message box cannot move the band, and the next frame's draw sees the shared state unchanged.
21. As a maintainer, I want the lookup and the drawing in one small module tested without a cart, so that every pixel rule is pinned in milliseconds.
22. As a maintainer, I want an integration test that reads the framebuffer on a caption frame and on captionless frames through `input_step_run`, `cart_update60` and `cart_draw`, so that the hook is proven where the drivers call it.
23. As a maintainer, I want the full-demo feed of `make demo-check` to check the closing caption on the last frame and no overlay on the return frame, so that the end of the demo is covered without slowing `make test`.
24. As a maintainer, I want every existing `*.sha256`, `.end-state` and `worst-case/` file unchanged, and `pico8/demo.json` regenerated byte for byte, so that the step is shown to change no existing replay.
25. As a maintainer, I want `make test` and `make e2e` not slower than step 1's baseline, so that the loop stays fast (`CLAUDE.md`).
26. As a maintainer, I want the pointer, float, button, random, data and p8 guards to keep their counts, so that the overlay adds no pointer field, float, button name or random draw to the core.
27. As the step 12 implementer, I want the band's rows, colours and the tag's box written in the step note, so that `tests/e2e/attract.json`'s `.txt` can name the frame where the first caption shows.
28. As the step 13 implementer, I want the captions to be a pure function of the demo frame, so that the full-cycle hash is stable across playbacks wherever the map is.
29. As a reviewer, I want the measurements behind the band's place (HUD rows, tech panel rows, tooltip rows) in the step note, so that the choice is evidence, not taste.

## Implementation Decisions

### Where the band goes (measured)

Measured headlessly on a capture of `pico8/demo.json` replayed from the resume (all 2736 frames, `tmp/replay` with a capture path), and on `worst-case/title_idle.json` for the title. The screen is 160×120.

- **Top HUD: rows 0–10.** The buttons and readouts are on rows 0–9 and the separator line on row 10, across the whole width.
- **Bottom HUD: rows 109–119.** The galaxy menu button starts at row 109 (`P8_SCREEN_HEIGHT - 11` in the menu's layout row). The readout panel is on rows 111–119 (`P8_SCREEN_HEIGHT - 10` frame, readouts at `- 7`). The build icons are on rows ≈112–119 (rest cursor at `P8_SCREEN_HEIGHT - 8`). The plan's "y≈116" falls inside.
- **Tech panel (tech beat, frames 2120–2556): rows 11–87** on the left half, with its confirm row at 77–87. A band under the top HUD would cover its first rows for 436 frames.
- **Year banner: rows ≈36–41** (frames 1863–1905). The commands' targets sit around rows 38–45.
- **Tooltips over the tech panel: rows 101–109**, on 3 frames of the whole demo (2271, 2282–2283).

**Decision: the band covers rows 96–108, full width (x 0–159)**, directly on top of the bottom HUD (row 109). It is 13 rows, fixed for every caption: 1 row padding, line 1 (5 rows), 1 row gap, line 2 (5 rows), 1 row padding.

- Two-line caption: line 1 at y = 97, line 2 at y = 103.
- One-line caption (only the tech caption): at y = 100 (centred: 4 rows above, 4 below).
- Accepted cost: the band hides the lowest 13 map rows during the demo, and the 3 tooltip frames. A band under the top HUD would hide the tech panel instead (on average 70 non-background pixels per frame in rows 11–23, against 48 in rows 96–108, and 550+ on the tech frames).
- The constants are written relative to `P8_SCREEN_HEIGHT` (band bottom = `P8_SCREEN_HEIGHT - 12`, top = bottom − 12), so they read as "just above the bottom HUD".

### Colours and text

- **Band fill: colour 1 (dark blue), solid, no fill pattern.** It separates from the black map and grid dots and matches the cart's own dark message stripes.
- **Caption text: colour 7 (white).** It is the highest contrast on colour 1, and no other text on the band can be confused with it.
- **Centring:** a line of `n` characters is `n × P8_FONT_CELL_WIDTH − 1` pixels wide (the last cell's gap is not ink). It is drawn at `x = (P8_SCREEN_WIDTH − (n × 4 − 1)) / 2`, with integer division. The compiled captions give x = 36/48, 52/42, 46/20, 26/46, 54/22, 30, 64/48. A 32-character line gives x = 16..142.
- **Colours go straight into `p8_print`/`p8_rectfill`.** The draw palette only applies to sprites (`p8_spr`), so the palette blocks the entities select cannot remap the band or the text.

### Splitting the caption's lines

- `demo_data_caption_t.text` joins at most 2 lines of at most 32 characters with `'\n'` (steps 6 and 8). The overlay splits it at the `'\n'` into a local buffer of 33 bytes per line and prints each line with its own `p8_print` call. It never passes a `'\n'` to `p8_print`, which would draw the placeholder block.
- Trap guarded with `assert`s with messages: more than one `'\n'`, or a line over 32 characters ("the generator refuses it; a hand-made unit would overflow the line buffer"). `demo_source` (step 8) already refuses both, so these never fire on a generated unit.
- No `v` check here: step 6's generator and step 8's `demo_source` refuse it. The overlay draws what it is given.

### The `demo` tag

- **Corner: top right, under the top HUD.** All four screen corners hold HUD elements (top: buttons at x 0–9 and 150–159; bottom: menu button at x 0–9, build icons at x ≈135–159). The tag therefore hangs from the HUD's separator into the map's top-right corner, which no beat's UI covers (the tech panel is on the left; targets sit lower).
- **Box: x 143–159, y 11–17, colour 8 (red)**, with `demo` (lowercase like the cart's texts, 4 characters, 15 pixels) in colour 7 at (144, 12). That leaves 1 pixel of padding on each side.
- Red is the usual "this is a recording" marker, and no HUD text uses it (the HUD's texts use 9, 2, 13 and 6). White on red reads at this size. The tag is not animated: a blink would add a clock and make no pixel rule simpler.
- Drawn on **every frame the core plays a demo frame**, from demo frame 0 to the last. It is drawn whether or not a caption covers that frame (the compiled demo's captions tile it, so in practice both always show).

### Closing card

- **Drawn the same way**: same band, same rows, same colours, same tag. The plan calls it a "card", but in the data it is just the last caption (frames 2556–2736). One code path, and step 7 has already paced it at 180 frames. A full-screen card would hide the final state (the new tech) that the closing frames still show.

### How the draw learns the current demo frame

- `cart_draw()` takes no argument, and its many callers (drivers, tests) must not change. The attract state lives in `input_step_t` (step 10), which the drivers own. So **`input_step_run` publishes the frame to the cart on every frame**:
  - New in `cart`: `void cart_set_demo_frame(int frame)` and `#define CART_NO_DEMO_FRAME (-1)`. It sets a file-scope `int` read by `cart_draw`. The header states the contract: the index into `demo_data_pad_buttons` of the demo frame the core played this frame, or `CART_NO_DEMO_FRAME`.
  - `input_step_run`, after the attract decision and before it returns, calls `cart_set_demo_frame(phase == ATTRACT_PHASE_PLAYING ? demo_frame - 1 : CART_NO_DEMO_FRAME)`, reading its own attract state through the step 10 accessors (H-10-2). `demo_frame` is the *next* index, so the frame played this frame is `demo_frame − 1`. The START frame publishes 0, the last demo frame publishes `demo_data_frame_count − 1`, and the RETURN frame (IDLE or SWALLOWING) publishes none.
  - `cart_load` and `cart_return_to_title` reset it to `CART_NO_DEMO_FRAME`. A new session in the same process, or a tool's return (`record_start_return`), can then never draw a stale caption on the title.
- **Not** a new driver call after `cart_draw`: every driver and test would have to add it, and one that forgot would capture different pixels (plan: "inside the core's draw"). **Not** in the engine's dispatch context: `engine/` knows no game concept (`CLAUDE.md` layout).
- The published value is one `int`: no pointer, so `make pointer-guard` keeps its counts.

### The module: `src/game/demo_overlay.{c,h}`

A small module with no cart, no entity and no input. It includes `demo_data.h` and `p8/gfx.h`.

```c
/* Index into captions of the caption shown on demo frame `frame` (from <= frame < to), or -1. */
int demo_overlay_find_caption(const demo_data_caption_t *captions, int count, int frame);

/* Draws the band, the caption of the compiled demo's frame `frame` and the demo tag; does nothing
   when frame < 0. Draws with a copy of *gfx whose camera and fill pattern are reset: *gfx is not changed. */
void demo_overlay_draw(const p8_graphics_t *gfx, p8_vram_t *mem, int frame);
```

- `demo_overlay_find_caption` takes the table as a parameter, so a test can pass a fixture with a gap. `demo_overlay_draw` uses `demo_data_captions` / `demo_data_caption_count`. The search is linear over 7 entries.
- In `demo_overlay_draw`: if frame < 0, return. Otherwise draw the tag. If a caption covers the frame, fill the band and print its lines.
- **Own graphics state**: it copies `*gfx` into a local, applies `p8_camera_reset` and `p8_fillp_reset` to the copy, and draws with it. Message boxes leave the camera shifted after some draws (`message_box.c` sets it per article), and `pen_color` persists between frames (`hud_draw_bar_default` reads it). Drawing with a copy means the shared state the next frame's draw starts from is the same with or without the overlay.
- **`cart_draw`** ends with `demo_overlay_draw(dispatch_context_gfx(), mem, g_demo_frame)` after `draw_layers(ENTITY_MESSAGE_DRAW_OVERLAY)`. It is drawn over everything, message boxes and tooltips included.
- **Makefile**: `src/game/demo_overlay.c` joins `ENGINE_SRCS` and `CART_ONLY_SRCS` (it reads the demo unit, which step 10 puts in `CART_ONLY_SRCS`, so `DATA_ENGINE_SRCS` binaries keep not depending on the demo generator). It reaches `GAME_SRCS` through `ENGINE_SRCS` only, never listed twice. It is in `CORE_SRCS` (portable, integer only), so `float-grep`, `float-guard`, `alloc-guard` and `constants-check` read it. `POINTER_GUARD_SRCS` picks it up by its wildcard.

### Why no existing hash moves

- `cart_draw` draws the overlay only when the published frame is ≥ 0, which only `input_step_run` in the PLAYING phase sets. No existing replay or unit test reaches PLAYING: step 10 measured the longest empty title run at 59 frames in `tests/e2e/`, 300 in `worst-case/title_idle.json`, and 301 in `worst-case/` overall (in a game), against a threshold of 1200, and no file declares `"attract_after"` (H-10-8).
- `"start": "resume"` files (`pad_author` from-record outputs, `pico8/demo.json`, `REPLAY_STARTS=2`) start through `record_start`, not the attract machine: they never publish a demo frame, so `make demo-check`'s replays and `make state` dumps of `demo.json` are unchanged too.
- The draw changes no game state (it only writes `mem->screen`), so every `.end-state`, `pad_author`'s frames and `pico8/demo.json` are unchanged.

### Unchanged

- `attract.{c,h}`'s state machine and transition table (step 10), the drivers' loops, the session recorder, `pad_author`, `demo_generate`, `pico8/demo.json`, `p8_print` and the font (the `v` glyph stays out of scope).

### Decisions taken without a human (guideline, `.claude/skills/guideline/SKILL.md`)

- **Band at rows 96–108**, not under the top HUD: measured to cover neither HUD and not the tech panel the tech caption talks about (plan: "not over the top HUD or the build bar; its exact place is fixed in the step that draws it").
- **Fixed band height for one- and two-line captions**: a band that jumps between beats is a visual change for no gain. It is the smaller rule.
- **Closing card drawn like every caption**: one path, no new layout. The plan's pacing already gives it 180 frames.
- **Tag at top right under the HUD, red box, white text**: every true corner is HUD, and this is the nearest free corner.
- **Published demo frame through `cart_set_demo_frame` from `input_step_run`**, rather than a driver call or an engine field. It is the one place every driver already goes through, and `input_step` already calls `cart` since step 10 (H-10-6).
- **A separate `demo_overlay` module**, not drawing code in `attract.c`: step 10 keeps `attract.c` a pure state machine, testable without graphics.

## Implementation Chunks

1. **The overlay module.** `src/game/demo_overlay.{c,h}` and `tests/test_demo_overlay.c` (pure: links the module, `src/p8/gfx.c`, `src/p8/vram.c` and the demo unit, with no cart run), plus the Makefile lists above. Not called yet. Done when `make test` passes with the guards' counts unchanged.
2. **The hook.** `cart_set_demo_frame` / `CART_NO_DEMO_FRAME` and the reset in `cart_load` and `cart_return_to_title`. `cart_draw` calls `demo_overlay_draw`, and `input_step_run` publishes. Then the integration cases in `tests/test_cart.c`, the closing-caption and return checks in the demo-check binary, and the guardrail runs. Done when `make test-all` passes with every `.sha256` unchanged, followed by the runs listed under Testing Decisions.

## Testing Decisions

- A good test drives the public interface and reads results only: the returned caption index, and the pixels of `mem->screen`. Expected pixels are built in the test from the rule (band rectangle, `p8_print` of each line at the computed x and y, tag box and text), not by calling `demo_overlay_draw` itself. One exception: the integration tests compare the live framebuffer's band and tag regions with a reference that `demo_overlay_draw` draws on a scratch `p8_vram_t`. The pure test has already pinned that reference pixel by pixel. Those regions are fully overwritten by the band and the box, so they do not depend on the game under them.
- No test reads `attract_t` or the cart's published frame directly.

### `tests/test_demo_overlay.c` (new, in `UNIT_TESTS`, a few milliseconds)

- **Lookup on the compiled table**: for each caption `i`, frames `from` and `to − 1` give `i`. Frame −1 and frame `demo_data_frame_count` give −1. On a fixture table with a gap (`{0,10}`, `{20,30}`), frames 10 and 19 give −1, 9 gives 0 and 20 gives 1.
- **Caption frame, two lines** (demo frame 0): starting from a screen filled with a sentinel colour (3), rows 96–108 × x 0–159 equal the expected band (colour 1, with "build a trading empire" printed in 7 at (36, 97) and "across the stars" at (48, 103)). The tag box x 143–159 × y 11–17 equals colour 8 with `demo` in 7 at (144, 12). Every other pixel is still 3, so nothing is drawn outside the band and the tag. The exact match also proves that no placeholder block is drawn for the `'\n'`.
- **One-line caption** (a frame in 2120–2555): "spend science on new tech" at (30, 100). Rows 96–99 and 106–108 are all colour 1.
- **Closing caption** (frame `demo_data_frame_count − 1`): "slipways" at (64, 97), "press any button" at (48, 103).
- **Captionless frame** (frame −1): the sentinel screen is untouched, everywhere.
- **Graphics state**: called with a `p8_graphics_t` whose camera is (5, 7) and whose fill pattern is set, the pixels are the same as above, and the caller's struct is byte-identical after the call.
- Prior art: `tests/test_gfx.c` and `tests/test_demo.c` (pixels after `p8_print` / `p8_cls`), `tests/test_demo_data.c` (reads the compiled captions).

### Integration in `make test` (extends step 10's attract cases in `tests/test_cart.c`, H-10-4)

Over a player's save, with threshold 3 set through `input_step_set_attract_after`, each frame driven as the drivers drive it (`input_step_run`, `cart_update60`, `cart_draw`):

- **Captionless frames on the title**: on frames 0–2 (before the start) the overlay is not shown. Predicate: the band rows are not all in {1, 7}, or the tag box does not equal the reference tag. The title has colour 0 on row 96 and in the top-right corner, measured on `worst-case/title_idle.json`.
- **Caption frame**: on frame 3 (demo frame 0) the band rows and the tag box equal the scratch rendering of demo frame 0.
- **Caption change**: in step 10's segment that reaches the fold (≈520 frames, already played), demo frame 299 shows caption 0 and demo frame 300 shows caption 1. This adds no frames.
- **Interruption**: the interrupting frame, the swallowed frames and the first idle frames show no overlay. The returned title equals the boot title pixel for pixel (`titles_equal`, step 4's helper), so nothing of the band or tag is left.
- **Session without attract**: in step 4's two-start harness (`record_start`-style start and return, no attract), no frame shows the overlay.
- Budget: no new segment beyond step 10's, only framebuffer reads. `make test` is timed against step 1's minimum.

### The demo-check binary (step 10's full feed, H-10-5; outside `make test`)

- At the last frame of each of the two complete playbacks (`demo_data_frame_count − 1`, read through the accessors), the band shows the closing caption and the tag is present. On the return frame that follows, no overlay.
- At each caption's `from` frame, the band and tag regions of the second playback equal the first's. The captions do not depend on the visual stream, even though the map under them does (step 4, item 1).

### No e2e is added here

`tests/e2e/attract.json` is step 12. This step only shows that no existing replay changes.

### Guardrail runs (plan section 2; this step changes the draw and `input_step`)

- `make test-all`, then, never alongside it, `make demo-check`. Then `make end-state`, `make full-game`, `make pad-author` and `make pad-author-proof`, with no diff in `worst-case/`, `pico8/demo.json` or `tests/e2e/`. Every `tests/e2e/*.sha256` stays unchanged.
- `make raylib` (link only, never run) and the check that `$(DEMO_UNIT)` appears exactly once in `GAME_SRCS`'s expansion, with `src/game/demo_overlay.c` also once.
- `make sanitize-net` and an ASan/UBSan build of `test_demo_overlay` and `test_cart`: clean.
- Timing: `make test` with nothing to rebuild and `make e2e` after `rm -rf tmp/frames`, three runs each with step 1's method, compared with 8.2 s and 1.39 s.
- The step note records: the band's rows (96–108), its colours (fill 1, text 7), the line positions (y 97/103, or 100 for one line) and the centring rule; the tag's box (x 143–159, y 11–17, colour 8, text 7 at (144, 12)); the measurements above; the guardrail runs and timings; and any deviation.

## Out of Scope

- `tests/e2e/attract.json` (step 12), the full-cycle hash (step 13), the README and the Makefile legend (step 14).
- Fixing the font's `v` glyph (plan, out of scope), and any new glyph.
- A blinking or animated tag, a fade-in or fade-out of captions, or a full-screen closing card.
- Captions in `pad_author`'s previews or any tool output beyond what the core draws.
- Moving or changing the game's HUD, tooltips or message boxes to make room for the band.
- Any change to the attract state machine, its thresholds or the drivers' loops.

## Further Notes

- Measurements were taken on HEAD 247c2c8's `tmp/replay` (the demo replayed from the resume with a capture; the title from `worst-case/title_idle.json`). The draw does not change before the overlay, so they hold after step 10.
- Caption timing on screen: the band shows caption `i` on exactly the frames where the core plays demo frames `from_i … to_i − 1`. With `"attract_after": N`, frame N of a file is demo frame 0, so step 12's `"attract_after": 3` shows the first caption from file frame 3.
- Captions in the dump: `make replay` / `make state` dumps of a title-start file that reaches the demo show the band, like `game` does. That is the point of drawing in the framebuffer.

### Hypotheses

Step 10 (`docs/prd/triage/phase2-step10-attract-state-machine.md`) is being implemented while this PRD is written. This PRD's reading of the code is HEAD 247c2c8. Below is what it assumes about step 10's outcome. If step 10's note says otherwise, the note wins, and only the named part of this PRD follows it.

- **H-10-1 (hook):** `input_step_t` holds the attract state by value. `input_step_run` calls the attract machine at its very top, and runs START (default table, `cart_resume_game` of the compiled record, save cut) and RETURN (`cart_return_to_title`, session table) there, before the pad navigation. This step adds one call at the end of `input_step_run`.
- **H-10-2 (accessors and meaning):** `input_step_attract_phase(const input_step_t *)` returns `ATTRACT_PHASE_IDLE | PLAYING | SWALLOWING`. `input_step_attract_demo_frame(const input_step_t *)` returns the *next* index into `demo_data_pad_buttons` (1 after the START frame, `demo_data_frame_count` after the last demo frame). If step 10 defines it as the index just played, the published value is that index, unchanged: the rule is "the demo frame the core played this frame".
- **H-10-3 (phases per frame):** PLAYING holds, after `input_step_run`, on exactly the frames that played demo frames 0 … `demo_data_frame_count − 1`. The interrupting frame ends in SWALLOWING, and the frame after the last demo frame ends in IDLE (the return). Neither publishes a demo frame.
- **H-10-4 (integration prior art):** `tests/test_cart.c` holds step 10's attract cases: threshold 3 through `input_step_set_attract_after` over a player's save, a play-frame helper running `input_step_run` + `cart_update60` + `cart_draw`, a ≈150-frame segment then an interruption, a segment reaching the fold (≈520 frames), and step 4's `titles_equal` and boot title snapshot.
- **H-10-5 (demo-check binary):** step 10 adds a test binary run by `make demo-check` (not `make test`). It feeds the whole compiled demo twice through the attract module and stops at each playback's last demo frame through the accessors. This step adds framebuffer checks to it. If step 10 put that feed elsewhere, the checks go where the feed is.
- **H-10-6 (dependencies):** `input_step.c` may call `cart` functions (step 10's start and return already do), and both are in `CART_ONLY_SRCS`.
- **H-10-7 (demo unit in the core):** `$(DEMO_UNIT)` is in `ENGINE_SRCS` and `CART_ONLY_SRCS`, filtered out of `CORE_SRCS`, and appears once in `GAME_SRCS`'s expansion. `tools/float_grep_exclusions.txt` no longer lists `src/game/demo_data.h`, so `demo_overlay.c` including it changes no float-grep result.
- **H-10-8 (no existing replay plays the demo):** with the default threshold of 1200, no file in `tests/e2e/` or `worst-case/` and no existing unit test reaches PLAYING (step 10's measurement: longest empty title run 300 frames).
- **H-10-9 (draw untouched):** step 10 does not change `cart_draw`, `cart_load`'s or `cart_return_to_title`'s order, nor what any draw layer draws.
