---
title: "Plan 19 step 3.1: pset and circ as 2x2 TILEs in the display buffer"
description: "After step 2 the PSX frame is drawn straight into the 320x240 display buffer, but pset and circ still send 1x1 TILE_1s at 160x120 coordinates, so their pixels land at the wrong place and at a quarter of their size."
status: needs-triage
---

## Plan context (verbatim from docs/plan-19.md, § 3)

Step 3's common text:

> **Primitives at x2, one family per step.** Each family comes with its cost-model change, its unit tests, and its capture labels passing the same conditions. It also re-sizes `GPU_FRAME_BUFFER_BYTES` (and the CLUT and pattern slots) by the existing rule in its own commit, with `make state` passing and `ram-room` staying positive. Step 2 does the same for its family. In this order:

This sub-step's bullet:

> - `pset` and `circ`: a 2x2 `TILE` instead of `TILE_1`.

## Hypotheses on step 2's outcome

Step 2 ("Draw into the display buffer") is not done when this PRD is written, and its PRD is not in the committed tree, so this PRD reads step 2 only from the plan's text. Every assumption about what step 2 leaves behind is listed here. The implementer checks each one against the code before starting; when one fails, the "if it fails" line says what changes in this PRD.

- **H-2-1: step 2 doubles tiles in one place.** Step 2 turns `cls`/`rectfill`/`rect`/`circfill` spans into `TILE`s with doubled coordinates through a single doubling point: one pure, host-tested function (or one emitter in the GPU backend) that maps a screen-space tile `{x, y, w, h}` on the 160x120 screen to `{2x, 2y, 2w, 2h}` in the display buffer.
  *If it fails* (step 2 doubled inline in each draw function, or inside each `psx_tiles` function): this step adds that pure doubling function itself, next to the screen-space tile functions, unit-tests it, and uses it for `pset` and `circ` only. It does not move step 2's call sites onto it: that is a refactor of step 2's family, not this step's.
- **H-2-2: the display buffer is the draw target, origin at its corner.** After step 2, `psx_gpu_frame_begin` sets the draw area and draw offset to the current display buffer (320x240), so a `TILE` at (0, 0) lands at the buffer's top-left pixel, and the 160x120 draw area, the end-of-frame black `FILL`, the `POLY_FT4` upscale and the 0x0000-transparency workaround are gone.
  *If it fails* (some offset other than the buffer's corner is still applied): the doubled coordinates of this step are computed the same way as step 2's `TILE`s, whatever that is, and the unit test of the doubling (H-2-1) pins that mapping; the capture proof below is unchanged.
- **H-2-3: `pset` and `circ` are untouched by step 2.** After step 2, `gpu_pset` and `gpu_circ` still send one `TILE_1` per pixel at undoubled coordinates, the screen-space functions `psx_tiles_pset` and `psx_tiles_circ` keep their current contract (screen coordinates after the camera, 1x1 tiles, clipped to 0..159 x 0..119), and the cost model still counts `P8_GPU_TILE_1_BYTES` for both. So the `pset 131` and `circ 314` captures fail `gpu-capture-clean` and/or `gpu-capture-ceilings` after step 2.
  *If it fails* because step 2 already converted `pset` or `circ`: this step shrinks to what is left of the list below (cost model, `TILE_1` removal, re-size, doc, gate); if nothing is left, the step records that in the plan's step notes and ends with the gate run alone. If it fails because step 2 changed the `psx_tiles` contract for these two functions (e.g. they now return display-space tiles), this step follows the new contract and its unit tests assert 2x2 tiles at even coordinates instead of 1x1 tiles.
- **H-2-4: the 14 references are pending.** Step 2's commit set every entry of `psx/gpu_capture_references.txt` to `-`, so `gpu-capture-references` passes with "pending human acceptance" lines and this step edits no reference.
  *If it fails* (some references still hold the pre-plan hashes): `gpu-capture-references` fails on them; this step does not accept captures (Rules: acceptance happens once, at step 6) and instead sets the remaining entries to `-` in its own commit, as the plan says step 2 should have, recording it in the step note.
- **H-2-5: the gate state step 2 leaves.** After step 2, `make psx-check`'s `conditions.txt` has every condition `ok` except `gpu-capture-clean` and `gpu-capture-ceilings`, and those two fail only on labels not yet due at step 2: `pset 131`, `circ 314`, `spr 234`, `print 275`, `pattern 190`, `pattern 257`, `circfill 263`, `line 276` and the three `all` labels. `cls 1`, `rectfill 11` and `rect 300` are green.
  *If it fails* (another condition is red, or a step 2 label is red): this step does not fix it (no folding of steps). It stops before committing the re-size, records the red condition and its reason in the step note, and hands it back through the plan's normal failure path.
- **H-2-6: the cost model's frame overhead and page rule are settled.** Step 2 updated `p8_draw_cost_frame_begin` (no `POLY_FT4`, its new draw-target packets) and decided whether the cost model counts a texture page switch, and re-sized `GPU_FRAME_BUFFER_BYTES` from the resulting `make state` peak. Neither decision touches `pset` or `circ` (untextured primitives that never switch a page or a window).
  *If it fails* (step 2 left the frame overhead unmetered or the buffer unsized): this step's re-size still applies the rule to whatever peak `make state` now measures, and its step note says the figure includes step 2's leftover; it does not change the frame overhead itself.

## Problem Statement

A player booting the PSX disc on any emulator or console sees the frame drawn at display resolution after step 2, except for single pixels: every `pset` (the title's particles, star dots, small markers) and every `circ` outline is drawn as one 1x1 pixel at the 160x120 coordinate, so it shows a quarter of its size, in the top-left quarter of the screen instead of where the PC draws it. The `pset` and `circ` capture labels of `make psx-check` therefore fail their `offpalette`/`nonuniform`/`differ` conditions, and the GPU frame buffer is sized for 12-byte `TILE_1`s that the frame should no longer send.

## Solution

`pset` and `circ` send, per on-screen PICO-8 pixel, one 2x2 `TILE` at doubled coordinates in the display buffer: the same primitive and the same doubling as step 2's fills, so every PICO-8 pixel is a uniform 2x2 block of a palette colour and no 1x1 primitive is left. The cost model counts a `TILE` (16 bytes) for each of those pixels instead of a `TILE_1` (12 bytes); `GPU_FRAME_BUFFER_BYTES` is re-sized from the new `make state` peak by the existing rule; the `pset 131` and `circ 314` captures pass `offpalette 0`, `nonuniform 0` and their `differ` ceiling of 0. The PC game, its replays and its timing are unchanged.

## User Stories

1. As a player on the PSX disc, I want every `pset` dot to appear where the PC draws it, so that the title screen's particles and the game's small markers are in the right place.
2. As a player, I want every `pset` dot to be a 2x2 block on the 320x240 screen, so that it has the same size as every other PICO-8 pixel.
3. As a player, I want every `circ` outline to be drawn at the PC's position and at x2, so that planet rings and selection circles match the PC.
4. As a player using DuckStation's hardware renderer, I want `pset` and `circ` drawn without any stretched texture, so that they look the same as under the Software renderer.
5. As a player, I want a `pset` or `circ` partly off screen to be clipped exactly as on PC, so that nothing spills into the other display buffer or wraps around.
6. As a player, I want a `circ` with a negative radius or entirely off screen to draw nothing, so that the PSX frame matches the PC frame.
7. As a player, I want a `circ` of radius 0 to draw a single 2x2 block, so that degenerate circles match the PC.
8. As a player, I want `pset` and `circ` to keep the colour mapping `P8_PALETTE_RGB[col & 0x0F]`, so that their colours stay palette colours.
9. As a player, I want the game to never stop on a frame-buffer overflow because dots got bigger, so that a long game still runs to the end.
10. As a maintainer, I want `pset` and `circ` to go through the same tile emitter and doubling as step 2's fills, so that there is one way to put a solid block on screen.
11. As a maintainer, I want the screen-space functions for `pset` and `circ` to keep returning 1x1 tiles in 160x120 coordinates, so that their existing pixel-for-pixel tests against the software backend stay meaningful.
12. As a maintainer, I want the doubling to be a pure, host-tested function, so that a wrong factor or offset fails `make test` without booting the emulator.
13. As a maintainer, I want no `TILE_1` left in any frame, so that the Rules' "no 1x1 primitive in a x2 frame" invariant holds by construction.
14. As a maintainer, I want the now-unused `TILE_1` byte constant and its size check removed, so that the cost model names only primitives the backend sends.
15. As a maintainer, I want `p8_draw_cost_pset` to return one `TILE` (16 bytes, 1 primitive) when the pixel is on screen and nothing when it is off, so that the meter matches what the backend sends.
16. As a maintainer, I want `p8_draw_cost_circ` to return `8 x iterations` `TILE`s when the circle's box meets the screen, so that the meter still bounds the real bytes from above.
17. As a maintainer, I want the backend's "sent more than metered" fatal to stay unreachable for `pset` and `circ`, so that the meter remains an upper bound for every frame.
18. As a maintainer, I want the PC and PSX `zone gpu_frame` meter lines to stay equal (`gpu-cost`, `game-gpu-cost`), so that the meter is proven to describe the PSX build.
19. As a maintainer, I want `GPU_FRAME_BUFFER_BYTES` re-sized by the existing rule (measured `make state` peak + 1/8, rounded up to 1 KiB), so that the buffer keeps its margin with bigger primitives.
20. As a maintainer, I want the re-size in its own commit, so that a later bisect separates the size change from the behaviour change.
21. As a maintainer, I want `GPU_CLUT_SLOTS` and `GPU_PATTERN_SLOTS` re-checked by the same rule in that commit, so that the plan's re-size clause is fulfilled even though this family does not use them.
22. As a maintainer, I want `make state` to pass after the re-size, so that no peak eats its 1/8 margin.
23. As a maintainer, I want `ram-room` and `game-ram-room` to stay positive after the bigger buffers, so that the GPU replay and the game still fit in 2 MiB.
24. As a maintainer, I want the `pset 131` capture to pass `offpalette 0`, `nonuniform 0` and `differ` at most its ceiling 0, so that `pset` is proven exact at x2.
25. As a maintainer, I want the `circ 314` capture to pass the same three conditions, so that `circ` is proven exact at x2.
26. As a maintainer, I want `cls 1`, `rectfill 11` and `rect 300`, green since step 2, to stay green, so that this step regresses nothing.
27. As a maintainer, I want every `psx-check` condition other than `gpu-capture-clean` and `gpu-capture-ceilings` to be `ok`, and those two to fail only on labels due at later steps, so that the step is gated as the plan's Rules require.
28. As a maintainer, I want the capture ceilings file left unchanged, so that `pset` and `circ` are held to the exact 0 they had before the plan.
29. As a maintainer, I want no capture reference accepted in this step, so that human acceptance stays a single pass at step 6.
30. As a maintainer, I want every `*.sha256` in `tests/e2e/` unchanged, so that the PC game is proven untouched.
31. As a maintainer, I want `make test` and `make e2e` no slower, so that the fast loop stays fast.
32. As a maintainer, I want the PSX game documentation to say that `pset` and `circ` are 2x2 `TILE`s at doubled coordinates, so that `docs/` describes what is true.
33. As a maintainer, I want the step note of plan 19 to record the new peak, the new buffer size, the RAM room and the gate's remaining red labels, so that the next family step starts from known figures.
34. As the next step's implementer (`spr`/`print`), I want the frame-buffer figure this step sets to be measured with `pset` and `circ` already at 16 bytes, so that my re-size only reflects my family.
35. As a reviewer, I want the PR to touch only `pset`, `circ`, their cost, their tests, the sizes and the doc line, so that I can check it against the plan bullet in one read.

## Implementation Decisions

- **GPU backend (`psx_gpu`).** `gpu_pset` and `gpu_circ` stop building `TILE_1`s. Each on-screen pixel the screen-space functions return is passed through the doubling (H-2-1) and sent as one `TILE` coloured from `P8_PALETTE_RGB[col & 0x0F]`, through the same emitter step 2's solid fills use. The per-pixel `TILE_1` emitter is deleted. `circ` keeps its callback-per-pixel shape: the callback's context carries the colour (or the colour index), the callback doubles and emits.
- **No merging of pixels.** Each PICO-8 pixel stays one `TILE`. Merging adjacent `circ` pixels into wider tiles would save bytes but is a different contract (and the plan reserves run-merging for `line` only, as a fallback). Decided against: the byte cost of 2x2 tiles is covered by the re-size, and per-pixel tiles keep the existing pixel-for-pixel tests valid.
- **Screen-space functions (`psx_tiles`) unchanged.** `psx_tiles_pset` and `psx_tiles_circ` keep returning 1x1 tiles in 160x120 coordinates after the camera, clipped to the screen (H-2-3). Clipping happens before doubling, so a doubled tile always lies inside 0..319 x 0..239 and never touches the other display buffer.
- **Doubling.** One pure function maps a screen tile `{x, y, w, h}` to `{2x, 2y, 2w, 2h}`. If step 2 provides it (H-2-1), this step reuses it unchanged; otherwise this step adds it beside the screen-space tile functions with its own unit test and uses it for `pset` and `circ` only.
- **Cost model (`draw_cost`).** `p8_draw_cost_pset` returns `{P8_GPU_TILE_BYTES, 1}` on screen, `{0, 0}` off screen. `p8_draw_cost_circ` returns `{8 x iterations x P8_GPU_TILE_BYTES, 8 x iterations}` when the circle's box meets the screen and `r >= 0`, else `{0, 0}`. The visibility rules do not change. No window or page cost: these primitives are untextured.
- **`TILE_1` removed.** `P8_GPU_TILE_1_BYTES` and the backend's `TILE_1` size check are deleted, since no primitive the backend sends is a `TILE_1` any more. Every test reference to it moves to `P8_GPU_TILE_BYTES`.
- **Re-size (`gpu_sizes`), own commit.** After the behaviour commit, run `make state`; take the `zone gpu_frame` peak from `tmp/state.txt` and set `GPU_FRAME_BUFFER_BYTES` to `peak x 9 / 8` rounded up to a multiple of 1024 (which also keeps it a multiple of 4, as the backend's word check requires). If that equals the current value, the constant stays and the commit is skipped, with the figures written in the step note. `GPU_CLUT_SLOTS` and `GPU_PATTERN_SLOTS` are recomputed by the same rule (peak + 1/8, rounded up to a whole slot) from the same run; this family should not move them, and if it does, the step note says why. `make state` must exit 0 afterwards.
- **RAM room.** Each extra 1 KiB of `GPU_FRAME_BUFFER_BYTES` costs 2 KiB of BSS (two primitive buffers). `ram-room` and `game-ram-room` from `make psx-check` must stay positive; the figures go in the step note. If either goes negative, the step stops before committing the re-size and records the blocker (no other family's memory is touched to make room).
- **Gate.** The step is done when `make psx-check`'s `conditions.txt` has every condition `ok` except `gpu-capture-clean` and `gpu-capture-ceilings`, and those two fail only on labels not yet due: `spr 234`, `print 275`, `pattern 190`, `pattern 257`, `circfill 263` (its mask includes `line`), `line 276`, `all 100`, `all 188`, `all 265`. The labels this step must turn green, per the mask rule (a label is proven at the step that converts the last family in its mask, `psx/gpu_captures.txt`): `pset 131` (mask `cls,pset`) and `circ 314` (mask `cls,circ`), each with `offpalette 0`, `nonuniform 0` and `differ` <= 0. `cls 1`, `rectfill 11` and `rect 300` stay green. `make psx-check` itself still exits 1 by design (Rules).
- **Ceilings and references untouched.** `psx/gpu_capture_ceilings.txt` keeps 0 for `pset` and `circ`; `psx/gpu_capture_references.txt` stays all pending (H-2-4).
- **Documentation.** The PSX game doc's backend paragraph changes its `pset` and `circ` wording from `TILE_1` to "one 2x2 `TILE` at doubled coordinates per on-screen pixel". Nothing else in the doc is rewritten by this step.
- **Step note.** Plan 19's step notes get a "Step 3.1" entry: the `make state` peak and the replay holding it, the old and new `GPU_FRAME_BUFFER_BYTES`, the CLUT/pattern slot figures, `ram-room` and `game-ram-room`, the `pset 131`/`circ 314` differ/offpalette/nonuniform figures, and the remaining red labels; plus which H-2-k hypotheses held.

## Implementation Chunks

1. **Cost model and `TILE_1` removal (host only).** `p8_draw_cost_pset`/`p8_draw_cost_circ` count `TILE`; delete `P8_GPU_TILE_1_BYTES`; update cost and tile tests. `make test` green, e2e unchanged.
2. **Backend.** `gpu_pset`/`gpu_circ` send doubled 2x2 `TILE`s through the shared emitter; delete the `TILE_1` emitter and size check; add the doubling function and its test only if H-2-1 fails. Doc line. `make psx-build` links. Committed together with chunk 1 if the PSX build would otherwise fail on the deleted constant (the cost model and the backend's size checks must agree in every commit).
3. **Re-size (own commit).** `make state`, apply the rule, `make state` again green.
4. **Gate.** `make psx-check`, read `conditions.txt` and the capture report, check the two labels and the remaining red set, write the step note.

## Testing Decisions

- Good tests here assert what a caller sees: the cost a call reports, the tiles a screen-space function returns compared pixel for pixel with the software backend, the doubled tile a screen tile maps to. They do not inspect packet bytes or the backend's internals.
- **Cost model (`test_draw_cost`).** The `pset` test becomes "one `TILE` on screen, free off screen" with the same coordinates and camera cases as today. The `circ` test keeps its radii table and clipping cases with `P8_GPU_TILE_BYTES` per pixel slot. No new test file.
- **Screen-space tiles (`test_psx_tiles`).** The existing `pset` and `circ` pixel-for-pixel tests stay as they are (H-2-3); their "real bytes fit the metered" assertions switch to `P8_GPU_TILE_BYTES`. If this step adds the doubling function (H-2-1 failed), a test covers it: `{0,0,1,1}` -> `{0,0,2,2}`, `{159,119,1,1}` -> `{318,238,2,2}` (the last block fits the 320x240 buffer exactly), and a wider span doubling both width and height.
- **Meter (`test_draw_meter`).** Unchanged unless it names `TILE_1`, in which case it moves to `TILE`.
- **Not unit-tested.** `psx_gpu.c` builds only for the PSX; its behaviour is proven by the `pset 131` and `circ 314` captures in `make psx-check`, which compare the emulator's VRAM against the PC capture x2.
- **Prior art.** `test_psx_tiles`' software-comparison helpers (`pset_tile_matches_software`, `circ_pixels_match_software`) and `test_draw_cost`'s `assert_cost`.
- **Speed.** No new e2e file, no new slow check; `make test` and `make e2e` keep their run time.

## Out of Scope

- Any other family: `cls`, `rect`, `rectfill`, `circfill` (step 2), `spr`/`print`, patterned fills, `line` (later sub-steps of step 3).
- The frame-begin/frame-end layout, the VRAM layout, the texture-page decision and the frame overhead in the cost model (step 2).
- Merging `circ` pixels into wider tiles.
- Accepting or editing capture references, or changing capture ceilings.
- Making `make psx-check` exit green (from the `line` step on).
- Any headless reproducer (step 1 found none) and any DuckStation check (step 6).
- Audio, the PC raylib backend, the e2e replays.

## Further Notes

- The `circfill 263` label is not due here: its mask includes `line`, so it turns green at the `line` step. The `all` labels are due at step 4.
- The byte growth is bounded: each `pset` grows by 4 bytes and each `circ` by `32 x iterations` bytes. The peak frame of `make state` may hold many particles on the title screen, so the re-size is measured, not estimated.
- PSX targets run only for the gate in chunk 4; the emulator is shared, so the implementer runs `make psx-check` once per attempt, not in a loop.
