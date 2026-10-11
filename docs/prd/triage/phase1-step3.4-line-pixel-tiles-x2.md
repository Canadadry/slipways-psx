---
title: "Plan 19 step 3.4: line drawn at x2 as gfx_software's pixel list, one 2x2 TILE per pixel"
description: "After step 3.3, line is the last family still drawn at 1x: one LINE_F2 into the top-left quarter, stepped by the GPU rather than by PICO-8, so circfill 263, line 276 and the three all labels fail and make psx-check cannot exit green."
status: needs-triage
---

## Plan context (verbatim from docs/plan-19.md, § 3)

Step 3's common text:

> **Primitives at x2, one family per step.** Each family comes with its cost-model change, its unit tests, and its capture labels passing the same conditions. It also re-sizes `GPU_FRAME_BUFFER_BYTES` (and the CLUT and pattern slots) by the existing rule in its own commit, with `make state` passing and `ram-room` staying positive. Step 2 does the same for its family. In this order:

This sub-step's bullet:

> - `line`: a `LINE_F2` is 1 pixel thick whatever its coordinates. Draw it either from `gfx_software.c`'s pixel list as 2x2 tiles (exact, more bytes) or from two offset `LINE_F2`s if the capture proves them exact. Decide from the `line` capture's `differ` and from the frame-buffer bytes and RAM room each option costs. The two-`LINE_F2` option is admissible only if the step 1 reproducer exists and the `line` labels pass under it; otherwise take the pixel list. If the pixel list does not fit in RAM, merge its pixel runs into wider `TILE`s, which is still exact; if that does not fit either, record the blocker and hand it to step 6.

The Rule this step is the first to meet (§ 2):

> Until the `line` step, `make psx-check` fails by design. [...] From the `line` step on, `make psx-check` exits green.

## What steps 1 to 3.2 settled (from the plan's step notes, committed)

- **Step 1:** no headless reproducer exists. "Step 3's two-`LINE_F2` option for `line` is not admissible (`line` takes the pixel list, or merged `TILE` runs)."
- **Step 2:** the frame draws straight into its 320x240 display buffer. Line options measured on `make state` (`worst-case/full_game.json` holds every peak), with `pset`/`circ` priced at 16-byte `TILE`s and no page-switch cost: single `LINE_F2` 67316 (buffer 75776); two `LINE_F2`s 69140 (not admissible); **pixel list as 2x2 `TILE`s 187252 (buffer 210944, ram-room 50212 left); merged runs 96564 (buffer 109568, ram-room 252964 left)**. All fit.
- **Step 3.1:** `pset` and `circ` send one 2x2 `TILE` per on-screen pixel through `psx_tiles_double`, from an emitter (`psx_tiles_circ(gfx, ..., emit, context)`) that walks a copy of `gfx_software.c`'s loop.
- **Step 3.2:** `spr`/`print` are doubled `SPRT`s; any change of texture source sends the `DR_TPAGE` + `DR_TWIN` pair in the backend and the meter. `make state` peak 67940, `GPU_FRAME_BUFFER_BYTES` 76800, CLUT 5/6, pattern 3/4, `ram-room` 316820, `game-ram-room` 324936, last `gpu vsyncs` 878 (ceiling 1655), `game vsyncs` 343 (ceiling 545).
- At the commit that closed 3.2, `line` is one `LINE_F2` between its endpoints after the camera, unclipped, sent when its box meets the screen, metered as `P8_GPU_LINE_F2_BYTES` when that box meets the screen. `psx/gpu_capture_ceilings.txt` holds `line 276 2`; every other label holds 0. All 14 references are pending (`-`).

## Hypotheses on step 3.3's outcome

Step 3.3 (patterned fills at x2, spans split at U = 256) is not done when this PRD is written. Its PRD is in `docs/prd/triage/`, its code is not committed. This PRD reads the code at the commit that closed step 3.2 and reads step 3.3 only from its PRD. Before starting, the implementer checks each hypothesis against the code and the plan's step notes. When one fails, its "if it fails" line says what changes here.

- **H-3.3-1: step 3.3 is committed first.** Its behaviour commit, its re-size commit if a size changed, and its "Step 3.3" step note are on the branch.
  *If it fails* (3.3 missing or partial): this step does not start and folds nothing of 3.3 in, because the plan sets the order. The implementer reports that 3.3 is missing and stops.
- **H-3.3-2: only `line` still draws at 1x.** Patterned `rectfill` boxes and `circfill` spans are doubled `SPRT`s from x2 pattern slots in the font's page. `gpu_line` is the only backend entry that sends anything at 1x, and the PSX game doc's "still draw at 1x" bullet names only `line`.
  *If it fails* (another family still at 1x): this step does not convert it. It converts `line`, then stops before the gate, records which family is left in the step note and hands it back through the plan's failure path, since `make psx-check` cannot be green with a family at 1x.
- **H-3.3-3: 3.3 did not touch `line`.** `p8_draw_cost_line` still returns one `P8_GPU_LINE_F2_BYTES` when the line's box meets the screen; `psx_tiles_line`, `psx_line_t`, `gpu_line` and its long-line assert, `P8_GPU_LINE_F2_BYTES` and the `LINE_F2` size check are as at the commit that closed 3.2; `test_draw_cost`'s two line tests and `test_psx_tiles`'s line test are unchanged.
  *If it fails*: the decisions below apply to whatever replaced those names; the end state is the same (no `LINE_F2` left, line metered as the exact `TILE` count).
- **H-3.3-4: the 1x pixel emitter and `add_tile` are unchanged.** `psx_tiles_emit_t` still takes a 1x1 `psx_tile_t` and a context; the backend's `add_pixel` still forwards to `add_tile(pixel, col)`, which doubles through `psx_tiles_double`.
  *If it fails* (3.3 renamed or moved them): `line` uses whatever `pset`/`circ` use to send one doubled 2x2 `TILE` per pixel; the contract does not change.
- **H-3.3-5: the sizes 3.3 leaves.** `GPU_FRAME_BUFFER_BYTES` is 76800, or one rule step above it if the U = 256 split grew the peak; `GPU_CLUT_SLOTS` 6, `GPU_PATTERN_SLOTS` 4; `ram-room` and `game-ram-room` positive, about 768 bytes below 3.2's (the x2 pattern caches). The projections under "Line form" use 3.2's figures minus those 768 bytes.
  *If it fails* (other figures): recompute the projection as `ram-room(3.3) - 2 x (new buffer - buffer(3.3))`, for `game-ram-room` likewise. The decision rule does not change: the pixel list unless that projection, or the measured value at the gate, is not positive.
- **H-3.3-6: the split SPRTs barely move the line peak.** The pixel-list peak is held by `worst-case/full_game.json` at about 187876 bytes (step 2's 187252 + 3.2's page switches); 3.3's split adds at most one `SPRT` per patterned span wider than 128 pixels on that frame.
  *If it fails* (the peak moves to another replay or grows by more than a few KiB): no decision changes; the re-size uses the measured peak and the step note names the replay that holds it.
- **H-3.3-7: the gate state 3.3 leaves.** In `make psx-check`'s `conditions.txt`, every condition is `ok` except `gpu-capture-clean` and `gpu-capture-ceilings`, and those two fail only on `circfill 263`, `line 276`, `all 100`, `all 188` and `all 265`. `cls 1`, `rectfill 11`, `rect 300`, `pset 131`, `circ 314`, `spr 234`, `print 275`, `pattern 190` and `pattern 257` pass with `differ 0`.
  *If it fails* (another condition or label red): this step does not fix it. It still converts `line` and keeps every green label green, then records the inherited red and its cause in the step note and hands it back through the plan's failure path, because `make psx-check` cannot exit green.
- **H-3.3-8: ceilings and references untouched.** `line 276` still holds 2, every other ceiling 0; all 14 references are `-`.
  *If it fails*: this step changes no ceiling and accepts no capture; it records what it found in the step note.
- **H-3.3-9: 3.3's step note has its figures.** The "Step 3.3" note records the `make state` peak, `GPU_FRAME_BUFFER_BYTES`, the slots, `ram-room`, `game-ram-room` and the vsyncs.
  *If it fails*: before changing anything, the implementer runs `make state` once on 3.3's last commit and writes that baseline in this step's note, so this step's figures are told apart from 3.3's.
- **H-3.3-10: 3.3 edited only its lines of the PSX game doc.** The "still draw at 1x" bullet keeps the `LINE_F2` text for `line`; the drawing-backend section still says the cost model is "an upper bound; a call whose screen box misses 160x120 costs 0".
  *If it fails*: the doc edits below apply to whatever text now describes the same facts.

## Line form: the decision

**Decision: the pixel list. One 2x2 `TILE` per on-screen pixel of `gfx_software.c`'s line, no merging.**

- The two-`LINE_F2` option is out: "admissible only if the step 1 reproducer exists", and step 1's note says none does.
- The bullet then reads: "otherwise take the pixel list. If the pixel list does not fit in RAM, merge its pixel runs into wider `TILE`s". Merging is a fallback with one trigger, the pixel list not fitting in RAM. The sentence "Decide from the `line` capture's `differ` and from the frame-buffer bytes and RAM room each option costs" frames the choice between `LINE_F2`s and the pixel list; the following sentences give its rule. So the text does not let this step pick merged runs because they are cheaper.
- The pixel list fits. Projected from step 2's measurement plus 3.2's page switches: peak about 187876, so `GPU_FRAME_BUFFER_BYTES` 76800 -> 211968 (peak x 9 / 8 rounded up to 1 KiB). The two primitive buffers grow by 2 x 135168 = 270336 bytes. That leaves `ram-room` about 316820 - 768 - 270336 = 45716 and `game-ram-room` about 53832, both positive. Merged runs would have left about 250 KB, but that is not the trigger.
- "Fit in RAM" means what the Rules check: `make state` passes with the re-sized buffer, and `ram-room` and `game-ram-room` are positive on `make psx-check`.
- **Fallback, only on that trigger.** If the projection (H-3.3-5) or the gate's measured `ram-room` or `game-ram-room` is not positive, the step switches to merged runs as the plan says: consecutive pixels of the list that share a row, or that share a column, become one clipped `TILE`, counted exactly by the same walk. Same chunks and tests, with "pixel" read as "run". If merged runs do not fit either, the step records the blocker (figures and the replay holding the peak) in the step note and hands it to step 6, as the bullet says.
- Not a trigger: the vsync count. If the GPU replay or the game crosses a boot ceiling (`PSX_GPU_CEILING`, `PSX_GAME_CEILING`, `PSX_GAME_CD_CEILING`) or its timeout, the line form does not change and the ceiling is not raised here, because step 5 re-sizes the boot ceilings. The step records the measured vsyncs and hands the red gate back through the plan's failure path, naming merged runs as the likely fix. Last margins: GPU replay 878 of 1655, game 343 of 545, CD boot 769 of 1140; the GPU replay's frames peak at 28132 bytes, far below `full_game`'s, so this is not expected.

## Problem Statement

A player boots the PSX disc after step 3.3. Every shape, sprite, letter and patterned fill is now drawn at display resolution, but each `line` (the slipways between planets, the aim lines, the HUD strokes) is still one `LINE_F2` at its 1x coordinates. It is 1 pixel thick on a 320x240 screen where every other PICO-8 pixel is a 2x2 block, it lands in the top-left quarter apart from everything drawn around it, and its stepping is the GPU's rather than PICO-8's. On `make psx-check`, `line 276`, `circfill 263` (its discs only show where they cut a slipway) and the three `all` captures fail `nonuniform`/`differ`, so `make psx-check` cannot exit green, which the plan requires from this step on.

The cost model also prices a line as one 16-byte `LINE_F2` whenever its bounding box meets the screen, which says nothing about what an exact line costs.

## Solution

`line` is drawn exactly as the PC draws it. The PC's line walk (`gfx_software.c`'s Bresenham loop, after the camera, clipped to the screen) gives the list of pixels the PC lights. Each one becomes one 2x2 `TILE` at doubled coordinates, the same way `pset` and `circ` already are. So a PSX line is the PC line upscaled x2, pixel for pixel, whatever the renderer, and no `LINE_F2` is left in a frame.

The pixel walk lives in the cost model, so the meter counts exactly the `TILE`s the backend sends: a line costs 16 bytes per on-screen pixel, and 0 when none of its pixels shows. `GPU_FRAME_BUFFER_BYTES` grows by the existing rule to about 207 KiB, with `make state` green and the RAM room still positive. `line 276`, `circfill 263`, `all 100`, `all 188` and `all 265` then pass `offpalette 0`, `nonuniform 0` and `differ 0`, and `make psx-check` exits 0. The PC game, its replays and its timing do not change.

## User Stories

1. As a player on the PSX disc, I want each slipway drawn at its PC position on the 320x240 screen, so that it joins the planets it links.
2. As a player, I want a line to be 2 display pixels thick, so that it has the weight of every other PICO-8 pixel.
3. As a player, I want a line's staircase to step where PICO-8's does, so that diagonal slipways look the same as on PC.
4. As a player, I want horizontal and vertical lines drawn as solid 2-pixel bars, so that the HUD strokes are clean.
5. As a player, I want a line with one or both ends off screen to show exactly the part the PC shows, so that nothing spills into the other display buffer.
6. As a player, I want a line entirely off screen to draw nothing, so that the PSX frame matches the PC frame.
7. As a player, I want a single-point line (both ends equal) drawn as one 2x2 block, so that it matches `pset`.
8. As a player, I want a line drawn in either direction (x2 < x1, y2 < y1) to light the same pixels as on PC, so that the order of the arguments never shows.
9. As a player, I want a line to keep its colour exactly, so that slipway colours match the palette.
10. As a player, I want lines and the discs drawn over them (the `circfill 263` frame) to cut each other as on PC, so that planets sit cleanly on their slipways.
11. As a player, I want the title screen and a whole game to look the same under any renderer, so that the disc does not depend on DuckStation's settings.
12. As a player, I want a frame full of slipways to never stop on a frame-buffer overflow, so that a long game still runs to the end.
13. As a maintainer, I want one pixel walk that the meter and the backend both use, so that the bytes sent and the bytes metered cannot drift.
14. As a maintainer, I want that walk proven pixel for pixel against the software backend's line, so that the PSX lights exactly what the PC lights.
15. As a maintainer, I want the software backend's line left untouched, so that every e2e `*.sha256` stays identical and it remains the oracle.
16. As a maintainer, I want the walk's count returned without an emitter, so that the cost model counts pixels without storing them.
17. As a maintainer, I want a line's cost to be exactly 16 bytes per on-screen pixel, so that the meter is equal to what the backend sends for lines, not only an upper bound.
18. As a maintainer, I want a line whose box meets the screen but whose pixels all miss it to cost 0 and send nothing, so that the meter and the backend agree on that edge case.
19. As a maintainer, I want a line to leave the texture state untouched in the meter and the backend, so that it never costs a `DR_TPAGE` + `DR_TWIN` pair.
20. As a maintainer, I want a line under `fillp` to stay a solid line, so that the PSX follows the software backend, which ignores patterns for lines.
21. As a maintainer, I want `psx_tiles_line` to emit 1x1 tiles through the same emitter type as `psx_tiles_circ`, so that the backend sends lines through its existing `add_pixel`/`add_tile` path.
22. As a maintainer, I want a host test that paints a line's tiles at 1x and doubled into 320x240 and compares them with the software screen and the software screen upscaled x2, so that the whole host-side line path is proven exact at x2.
23. As a maintainer, I want that test to also check that the tile count equals the metered primitives and the bytes equal 16 per tile, so that the meter is proven exact.
24. As a maintainer, I want the test cases to cover points, horizontal, vertical, 45-degree, shallow and steep lines in both directions, lines clipped by every edge and corner, lines under a camera, and fully off-screen lines, so that every branch of the walk and of the clipping is pinned.
25. As a maintainer, I want a seeded sweep of random lines with endpoints well off screen, so that an untested case fails `make test`.
26. As a maintainer, I want `LINE_F2`, `psx_line_t`, `P8_GPU_LINE_F2_BYTES` and the long-line assert removed, so that no dead GPU path is left.
27. As a maintainer, I want the frame-overhead and packet-size compile-time checks kept for every primitive still sent, so that the cost model's sizes stay checked against the SDK.
28. As a maintainer, I want the backend's "sent more than metered" fatal to stay, so that a future mismatch still stops the replay at the frame.
29. As a maintainer, I want the PC and PSX `zone gpu_frame` meter lines to stay equal (`gpu-cost`, `game-gpu-cost`), so that the meter is proven to describe the PSX build.
30. As a maintainer, I want the family filter to count a `line` call only when it lights a pixel, so that `counts.txt` follows the exact cost model.
31. As a maintainer, I want `GPU_FRAME_BUFFER_BYTES` re-sized by the existing rule (measured `make state` peak + 1/8, rounded up to 1 KiB) in its own commit, so that a bisect tells the size apart from the behaviour.
32. As a maintainer, I want `GPU_CLUT_SLOTS` and `GPU_PATTERN_SLOTS` re-checked by the same rule, so that the plan's re-size clause is met even though lines use neither.
33. As a maintainer, I want `make state` to pass and `ram-room`/`game-ram-room` to stay positive after the larger buffers, so that the replay and the game still fit in 2 MiB.
34. As a maintainer, I want the fallback to merged runs written down with its single trigger (RAM), so that the implementer never switches form for another reason.
35. As a maintainer, I want `line 276` and `circfill 263` to pass with `differ 0`, so that `line` is proven exact at x2.
36. As a maintainer, I want `all 100`, `all 188` and `all 265` to pass with `differ 0`, since `line` is the last family of their mask, so that every family is proven together.
37. As a maintainer, I want every label green before this step to stay green, so that the step regresses nothing.
38. As a maintainer, I want `make psx-check` to exit 0 from this step on, as the plan's Rules require.
39. As a maintainer, I want `line 276`'s ceiling left at 2 and no reference accepted, so that ceilings are never raised and human acceptance stays one pass at step 6.
40. As a maintainer, I want every `*.sha256` in `tests/e2e/` unchanged and `make test`/`make e2e` no slower, so that the PC game is proven untouched and the loop stays fast.
41. As a maintainer, I want the PSX game doc to describe lines as 2x2 tiles from the PC's walk, and to say no family draws at 1x any more, so that `docs/` says what is true.
42. As a maintainer, I want plan 19's step notes to record the line form chosen, the peak, the buffer size, the slots, the RAM room, the vsyncs and every label's figure, so that steps 4 and 5 start from known figures.
43. As step 4's implementer, I want every family converted and `make psx-check` green, so that step 4 only has to prove that no stretched texture is left.
44. As a reviewer, I want the PR to touch only the line walk, the line cost, the line tiles, the backend's `line`, their tests, the sizes and the doc, so that I can check it against the plan bullet in one read.

## Implementation Decisions

- **The pixel walk lives in the cost model (`draw_cost`, p8).** New export, `p8_draw_cost_line_pixels(gfx, x1, y1, x2, y2, emit, context)`: it walks the line with the same Bresenham loop as `gfx_software.c`'s `software_line` (same `err`, same `e2 >= dy` / `e2 <= dx` steps, from (x1, y1) to (x2, y2)). It calls `emit(x, y, context)` once per pixel that lies on the 160x120 screen after the camera, in screen coordinates, in walk order, and returns how many it emitted. `emit` may be NULL: it then only counts. It first returns 0 when the line's box misses the screen, as today, so an off-screen line costs no walk. A new callback type in the cost model's header carries (x, y, context). Contract in the header: each pixel is emitted at most once (the walk never revisits a pixel), the count is what the backend sends, and the loop must change together with `software_line` (a trap comment, as for the circle loop).
- **Why in the cost model and not a new module or in `gfx_software.c`.** The meter and the backend must agree pixel for pixel; putting the one walk where the meter lives makes that true by construction, and every binary that meters or sends lines already links the cost model, so no link line changes. The software backend keeps its own loop as the oracle the tests compare against: making it call the walk would make the tests compare the walk with itself, and would touch the e2e path.
- **Line cost (`p8_draw_cost_line`), exact.** Same signature. Returns `P8_GPU_TILE_BYTES` x the walk's count, with that count as the primitive count. A line whose box meets the screen but whose pixels all miss it now costs 0 (it cost one `LINE_F2` before). A line never touches the texture state. `P8_GPU_LINE_F2_BYTES` leaves the enum. The draw meter and the family filter call `p8_draw_cost_line` unchanged; the filter now counts a line only when it lights a pixel, which is still "a call the cost model costs above 0".
- **Line tiles (`psx_tiles`).** `psx_tiles_line(gfx, x1, y1, x2, y2, emit, context)` takes the same `psx_tiles_emit_t` as `psx_tiles_circ` and returns how many 1x1 tiles it emitted, each inside the screen, from `p8_draw_cost_line_pixels`. `psx_line_t` leaves the header. The header contract says the count equals `p8_draw_cost_line(...).primitives`.
- **Backend (`psx_gpu`).** `gpu_line` calls `psx_tiles_line(gfx, x1, y1, x2, y2, add_pixel, &col)`, so each pixel goes through `add_tile` and `psx_tiles_double` into one 2x2 `TILE` of `P8_PALETTE_RGB[col & 0x0F]`, as `pset` and `circ` do. The `LINE_F2` size check and the long-line assert go: no `LINE_F2` is sent, and every `TILE` is clipped to the screen. The other size checks, the frame-overhead check and the "sent more than metered" fatal stay. For lines the backend now sends exactly what was metered.
- **No merging, no special cases.** Horizontal and vertical lines are also sent pixel by pixel: a run of pixels as one `TILE` is the merged-runs form, which the plan reserves for the RAM fallback (see "Line form: the decision").
- **Re-size (`gpu_sizes`), own commit.** After the behaviour commit, run `make state` and read the `zone gpu_frame` peak and `cap gpu_cluts`/`cap gpu_patterns` from `tmp/state.txt`. Set `GPU_FRAME_BUFFER_BYTES` to `peak x 9 / 8` rounded up to a multiple of 1024 (expected about 211968), and each slot count to its peak + 1/8 rounded up to a whole slot (expected unchanged, lines use neither). Run `make state` again until it exits 0. Before committing, project `ram-room` and `game-ram-room` (H-3.3-5); if either is not positive, apply the fallback before committing anything.
- **Gate.** `make psx-check` must exit 0: all 30 conditions `ok`, `ram-room` and `game-ram-room` positive, `gpu-cost` and `game-gpu-cost` equal. Per the mask rule of `psx/gpu_captures.txt` (a label is proven at the step that converts the last family in its mask), `line 276` and `circfill 263` (mask `cls,line,circfill`) are due here, and so are `all 100`, `all 188` and `all 265`, because `line` is the last family of the `all` mask once 3.3 is done. Step 2's text puts the `all` labels at step 4. The Rule that `make psx-check` exits green from the `line` step on is only met if they pass here, so this step requires them; step 4 still re-checks every label with its no-stretch audit. Every one of the 14 labels must have `offpalette 0`, `nonuniform 0` and `differ 0`. Only the 14 "pending human acceptance" lines remain.
- **The `line 276` ceiling stays 2.** With the pixel list its `differ` should drop to 0. `tools/psx_capture_check.sh` then prints `below ceiling: line f276 2 -> 0` and `gpu-capture-ceilings` passes. This step does not lower it: the bullet does not ask for it, and lowering a ceiling is a separate decision. The step note records the note line so that step 5 or the human can decide. Whatever the ceiling, this step treats any `differ` above 0 on `line 276` as a defect of this step to fix, not something the ceiling absorbs, because the pixel list is exact by construction.
- **References untouched.** No capture is accepted (step 6).
- **Documentation (PSX game doc).** The x2 bullet adds `line`: one 2x2 `TILE` per on-screen pixel of the software backend's line walk, after the camera and clipped to the screen, host-tested by `test_psx_tiles`. The "other families still draw at 1x" bullet goes: no family draws at 1x. The cost-model paragraph says a line costs one `TILE` per on-screen pixel, from the same walk the backend uses (0 when none shows), so the meter is exact for lines and an upper bound elsewhere.
- **Step note.** Plan 19's step notes get a "Step 3.4" entry: the line form and why (pixel list; fallback not triggered, or triggered with figures), the `make state` peak and the replay holding it, `GPU_FRAME_BUFFER_BYTES` old -> new, the CLUT/pattern figures, `ram-room`, `game-ram-room`, the three vsync counts against their ceilings, each label's `differ`/`offpalette`/`nonuniform`, the `below ceiling` line for `line 276`, `make psx-check`'s exit status, and which H-3.3-k held.

## Implementation Chunks

1. **The pixel walk and the exact line cost (host only, behaviour commit together with chunk 2).** Add `p8_draw_cost_line_pixels` and its callback type, with `test_draw_cost` cases against the software line. Switch `p8_draw_cost_line` to the exact count and replace the two `LINE_F2` expectations in `test_draw_cost`. Not committed alone: a PSX build at this point would send a `LINE_F2` for a line metered 0 and hit the "sent more than metered" fatal. `make test` stays green after the chunk.
2. **Line tiles, backend, doc (one behaviour commit with chunk 1).** Rewrite `psx_tiles_line` as an emitter and its test in `test_psx_tiles` (1x, x2, metered count). Switch `gpu_line` to `add_pixel` and remove `psx_line_t`, `P8_GPU_LINE_F2_BYTES`, the `LINE_F2` size check and the long-line assert. Edit the PSX game doc. `make test` and `make e2e` green with every `*.sha256` unchanged; `make psx-build` compiles and links.
3. **Re-size (own commit).** `make state`, apply the rule, `make state` again until green, project the RAM room; apply the fallback here if the projection is not positive.
4. **Gate.** Run `make psx-check` once per attempt (the emulator is shared). Read `conditions.txt`, `measurements.txt` and the capture reports, check all 14 labels and the exit status, then write the step note.

## Testing Decisions

- Good tests here check what a caller sees: the pixels a walk emits compared with the pixels the software backend lights, the 1x and doubled tiles painted into a screen compared with the software screen and its x2 upscale, and the cost a call reports. They do not inspect packet bytes or the backend's statics.
- **Walk and cost (`test_draw_cost`).** For each case, the pixels `p8_draw_cost_line_pixels` emits are exactly the pixels `p8_line` lights through the software backend on a cleared screen, each once, all on screen, and the return value is their number. With a NULL emitter the count is the same. `p8_draw_cost_line` returns `P8_GPU_TILE_BYTES` x that count and that count as primitives. Fixed cases: a point, a horizontal line clipped at x = 0 (`(-5, 3)` to `(4, 3)` costs 5 tiles), a vertical line, `(0, 0)` to `(5, 5)` (6 tiles), shallow and steep lines both ways, a line under a camera, a line whose box meets the screen but whose pixels miss it (`(-10, 5)` to `(5, -10)`, cost 0), a line whose box misses (0). A seeded sweep of a few hundred random lines with endpoints in -200..360 x -200..320, compared at 1x only, so it stays a few milliseconds. The existing test that an untextured call keeps the window is updated to the line's new cost (6 tiles for `(0, 0)` to `(5, 5)`). Prior art: `assert_cost` in the same file; the circle cases of `test_psx_tiles`.
- **Line tiles (`test_psx_tiles`).** The line test becomes "a line is the software line, one pixel each": collect the emitted 1x1 tiles, check them with the existing `assert_tiles_and_their_doubles_match_software` (no overlap, inside the screen, equal to the software screen at 1x and to its x2 upscale in a 320x240 buffer), and check that the count equals `p8_draw_cost_line(...).primitives` and `count x P8_GPU_TILE_BYTES` equals its bytes (equality, not `<=`). Cases: the fixed set above plus lines clipped by each edge and corner and one whose endpoints are far off screen on both sides. Prior art: `circ_pixels_match_software` and `collect_pixel` in the same file.
- **Unchanged tests.** `test_draw_meter` and `test_draw_filter` call `p8_line` with on-screen arguments and do not test line costs; they need no change. The e2e replays do not run the meter or the filter (only under `STATE=` or `CAPTURE_LIST=`).
- **Not unit-tested.** `psx_gpu.c` builds only for the PSX. Its behaviour is proven by `line 276`, `circfill 263` and the `all` labels in `make psx-check`, and by the equal meter lines.
- **Speed.** No new test file, no new e2e file, no slow check. The added cases paint at most a few dozen 320x240 buffers and walk a few hundred short lines, well within `make test`'s budget.

## Out of Scope

- Two offset `LINE_F2`s (not admissible: no reproducer). Merged runs, unless the RAM trigger fires.
- Lowering `line 276`'s ceiling, or changing any other ceiling. Raising a boot vsync ceiling (step 5).
- Accepting or editing capture references (step 6).
- Patterned fills, the U = 256 split, the x2 pattern slots (step 3.3); `spr`, `print`, texture switches (step 3.2); `cls`, `rect`, `rectfill`, `circfill`, `pset`, `circ` (steps 2 and 3.1).
- The "no stretch left" audit (step 4), the final re-measure (step 5), any headless reproducer (step 1 found none), any DuckStation check (step 6).
- Changing `gfx_software.c`'s line, the PC game, the e2e replays, the raylib backend, audio.

## Further Notes

- After this step no family draws at 1x and no `LINE_F2` or `TILE_1` is left; the only primitives sent are `TILE`, `SPRT`, `DR_TPAGE`, `DR_TWIN` and the draw-target packets.
- The two primitive buffers grow from about 150 KiB to about 414 KiB of the 2 MiB. Merged runs would cost about 214 KiB for both; if a later plan (the full game, plan 11) needs that RAM, switching to merged runs is the exact, cheaper form the plan already names, and needs no other change to the meter's interface.
- A line lights at most 160 pixels on screen (one per column or per row of the walk), so one line costs at most 2560 bytes.
- PSX targets run only in chunk 4's gate, once per attempt.
