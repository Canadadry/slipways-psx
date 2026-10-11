---
title: "Plan 19 step 3.3: patterned fills at x2 through an x2 texture window and x2 pattern tiles"
description: "After step 3.2, every family but patterned fills and line draws at x2. A patterned rectfill or circfill span is still a 1x SPRT read through a 1x texture window from 8x8 tiles in the 1x page, so the patterned HUD panels and the background grid land in the top-left quarter at a quarter of their size."
status: needs-triage
---

## Plan context (verbatim from docs/plan-19.md, § 3)

Step 3's common text:

> **Primitives at x2, one family per step.** Each family comes with its cost-model change, its unit tests, and its capture labels passing the same conditions. It also re-sizes `GPU_FRAME_BUFFER_BYTES` (and the CLUT and pattern slots) by the existing rule in its own commit, with `make state` passing and `ram-room` staying positive. Step 2 does the same for its family. In this order:

This sub-step's bullet:

> - Patterned fills: the texture window at x2, together with uploading the fill-pattern tiles at x2.

## What steps 2 and 3.1 settled (from the plan's step notes, committed)

- The frame draws straight into its 320x240 display buffer. `cls`, `rect`, `rectfill` and `circfill` spans without a pattern are `TILE`s doubled by `psx_tiles_double`. `pset` and `circ` are 2x2 `TILE`s (step 3.1).
- Frame overhead: `DR_ENV` + `DR_TPAGE` + `DR_TWIN` (52 bytes).
- The x2 VRAM layout, in 16-bit units: the x2 sheet fills the 4-bit page at (512, 256). The x2 font (8x12-texel cells, 64x96) and **the x2 pattern slots (16x16 texels) share the page at (576, 256)**. The CLUTs stay at (640, 0). The 1x textures stay in the page at (512, 0) until the step that replaces them. That makes **two texture pages** once this step is done, and a change of (page, window) costs the `DR_TPAGE` + `DR_TWIN` pair.
- Step 3.1's figures: `make state` peak 67316 (`worst-case/full_game.json`), `GPU_FRAME_BUFFER_BYTES` 75776, `GPU_CLUT_SLOTS` 6 (5 measured), `GPU_PATTERN_SLOTS` 4 (3 measured), `ram-room` 320976, `game-ram-room` 329092.
- The doc's planned row for this step: x2 pattern slots at (576, 352), 4x16 units a slot, v 96..255 of page (576, 256): 16x16 texels on 16-texel bounds for the texture window, 16 a row, up to 160 slots.

## Hypotheses on step 3.2's outcome

Step 3.2 ("`spr`/`print`: `SPRT` with doubled position, size and UV, together with uploading the packed sprite sheet and the font's glyph cells upscaled x2") is not done when this PRD is written. Its PRD exists in `docs/prd/triage/`, but its code is not committed. So this PRD reads the code at the commit that closed step 3.1 (with the step 3.1 note) and reads step 3.2 only from its PRD. Every assumption about what step 3.2 leaves behind is listed here. Before starting, the implementer checks each one against the code and the plan's step notes. When one fails, its "if it fails" line says what changes in this PRD.

- **H-3.2-1: step 3.2 is committed first.** Its behaviour commit (and its re-size commit, if the rule changed a size) is on the branch. `gpu_spr` and `gpu_print` send doubled `SPRT`s from the x2 sheet at (512, 256) and the x2 font at (576, 256).
  *If it fails* (3.2 not committed, or only partly): this step does not start and does not fold any of 3.2 in, because the plan sets the order. The implementer reports that 3.2 is missing and stops.
- **H-3.2-2: the cost model's texture state tells the sources apart.** `p8_draw_cost_window_t` holds a texture (sheet, font or pattern) and a `pattern_key`. Any change of that pair costs `P8_GPU_DR_TPAGE_BYTES + P8_GPU_DR_TWIN_BYTES` (two primitives). `p8_draw_cost_frame_begin` resets it to (sheet, 0). A patterned span switches to (pattern, key) and costs one `SPRT` per span.
  *If it fails* (3.2 kept `patterned`, or keyed the state by page alone): this step must still meter a pair for every change between font, sheet and a pattern slot, because the backend sends one (see "Backend texture state" below). If 3.2's state already prices every such change, nothing changes here. If it misses one (e.g. it collapses font and patterns because they will share a page), this step adds the missing case to the cost model and its tests in chunk 1. That is this family's cost-model change.
- **H-3.2-3: the backend has one texture-state function.** It holds (texture region, pattern slot or no window). When either part changes, it sends a `DR_TPAGE` for the page holding the region, then a `DR_TWIN`. `psx_gpu_frame_begin` forces (sheet, none). `add_patterned_spans` sets (patterns region, slot).
  *If it fails* (3.2 still sends only a `DR_TWIN` on a window change, with a page fixed per frame): this step cannot work, since patterns and sheet live on different pages from now on. The implementer stops and reports that 3.2 did not deliver its texture-state contract. This step does not add it, because that would fold 3.2 in.
- **H-3.2-4: the layout gives a region's page.** `psx_vram_layout_texture_page(region)` returns the 64x256 4-bit page that holds a region and asserts the region does not straddle one. `PSX_VRAM_SPRITE_SHEET` is (512, 256) 64x256, `PSX_VRAM_FONT` is (576, 256) 64x96. `PSX_VRAM_PATTERNS` is still (544, 0) 32x256, slots 2x8 units, and `psx_pattern_window` uses the page of `PSX_VRAM_PATTERNS`. The 1x sheet and the 1x font regions are gone.
  *If it fails* (another signature, or the font region has another height): the decisions below keep their contract. The pattern region is the rest of the font's page below the font, starting on a 16-line bound. If the font region ends past v 96 of its page, the pattern region starts at the next 16-line bound after it, and the slot capacity in the layout test is computed, not fixed at 160.
- **H-3.2-5: one pure x2 upscaler for 4-bit textures.** `psx_gpu_textures` has a function that takes a 4-bit source of any width in texels (byte layout of `p8_sprite_sheet_packed`: texel u in byte u/2, low nibble for an even u), a first x2 line and a line count, and writes those x2 lines. Its test states the little-endian trap (a `uint16_t` texture's bytes are that layout).
  *If it fails* (no such function, or its width is fixed to the sheet's 128): `psx_gpu_textures_pattern_x2` below is written as its own loop, x2 texel (u, v) = 1x tile texel (u/2, v/2). Its test against the 1x tile does not change.
- **H-3.2-6: one pure sprite doubling.** `psx_sprite` has `psx_sprite_double`, which multiplies x, y, u, v, w and h by `PSX_TILES_SCALE`.
  *If it fails* (the doubling lives elsewhere, or doubles only some fields): `psx_pattern_sprite_x2` below doubles the six fields itself through `PSX_TILES_SCALE`. Its contract and its test do not change.
- **H-3.2-7: patterned `SPRT`s bypass the textured-sprite path.** `add_patterned_spans` still sends its `SPRT`s with `add_sprt` directly, at 1x. The `u + w <= 256` / `v + h <= 256` asserts 3.2 added sit in `add_textured_sprite` only.
  *If it fails* (patterned spans now go through the asserting path): this step routes them around those asserts. A patterned span at x2 is up to 320 texels wide, and its UV wraps at 256 on purpose (see "No span split").
- **H-3.2-8: the sizes step 3.2 set are recorded.** Plan 19's step notes have a "Step 3.2" entry with the `make state` peak, the `GPU_FRAME_BUFFER_BYTES` it set (75776, or one step above if page switches grew the peak), `GPU_CLUT_SLOTS` 6, `GPU_PATTERN_SLOTS` 4, and positive `ram-room` and `game-ram-room`.
  *If it fails* (no entry, or no figure): before changing anything, the implementer runs `make state` once on 3.2's last commit and writes that baseline in this step's note, so that this step's figures are told apart from 3.2's.
- **H-3.2-9: the gate state step 3.2 leaves.** In `make psx-check`'s `conditions.txt`, every condition is `ok` except `gpu-capture-clean` and `gpu-capture-ceilings`. Those two fail only on `pattern 190`, `pattern 257`, `circfill 263`, `line 276`, `all 100`, `all 188` and `all 265`. `cls 1`, `rectfill 11`, `rect 300`, `pset 131`, `circ 314`, `spr 234` and `print 275` are green.
  *If it fails* (another condition or a label due by 3.2 is red): this step does not fix it, because that would fold steps together. The step still makes `pattern 190` and `pattern 257` green and keeps every label that was green still green. It stops before the re-size commit, records the inherited red condition or label and its reason in the step note, and hands it back through the plan's normal failure path.
- **H-3.2-10: references pending, ceilings untouched.** All 14 entries of `psx/gpu_capture_references.txt` are `-`. In `psx/gpu_capture_ceilings.txt`, `pattern 190` and `pattern 257` still hold 0.
  *If it fails*: this step accepts no capture. It sets any remaining entry to `-` in its own commit and records that in the step note. It does not raise a ceiling: patterned fills must be exact.
- **H-3.2-11: step 3.2 edited only its own lines of the PSX game doc.** The VRAM table has the x2 sheet and x2 font rows and a pattern row that says it stays 1x in page (512, 0) until step 3.3. The "Still to do" table under it keeps only the x2 pattern slots row, labelled step 3.3. The fill paragraph says any change of texture source (sheet, font, pattern slot) sends the `DR_TPAGE` + `DR_TWIN` pair.
  *If it fails*: the doc edits below apply to whatever text now describes the same facts.
- **H-3.2-12: the pattern tests are as step 3.1 left them.** `test_psx_pattern` paints 1x `SPRT`s into a 160x120 buffer from the 1x page, through the page of `PSX_VRAM_PATTERNS`. `test_psx_pattern_cache` compares cached tiles with `psx_gpu_textures_pattern`. 3.2 changed at most the page call.
  *If it fails*: the new expectations below apply to whatever those tests became.

## Problem Statement

A player boots the PSX disc after step 3.2. Sprites, text, dots and solid shapes are now drawn at display resolution, but every `rectfill` and `circfill` under a fill pattern (`fillp`) is still a 1x `SPRT` at its 160x120 position, read through a texture window that repeats an 8x8 tile of the 1x page. So the patterned HUD panels of aim mode and the background grid of slipway mode sit in the top-left quarter of the screen at a quarter of their size, apart from the frames and icons drawn around them. On `make psx-check`, the `pattern 190` and `pattern 257` captures fail their `offpalette`/`nonuniform`/`differ` conditions.

The 1x pattern slots are also the last thing in the 1x page at (512, 0). As long as they stay, the frame reads three texture pages instead of the two step 2 planned, and VRAM keeps a page that serves one 8x8 tile per slot.

## Solution

Each frame's pattern tiles are built at x2: the 1x 8x8 tile, every texel doubled in both directions, gives a 16x16-texel tile (the 4x4 pattern doubled, tiled twice each way). The tiles are uploaded at the end of the frame, as today, into x2 pattern slots of 4x16 units on 16-texel bounds, below the x2 font in the font's page (576, 256). The texture window repeats 16 texels instead of 8. Each patterned span stays placed at 1x, with its world phase (mod 8), exactly as today. It is then doubled (position, size, UV) by the same pure function as `spr` and `print`, and sent as one `SPRT` that reads the x2 tile under the x2 window. So every PICO-8 pixel of a patterned fill is a uniform 2x2 block of a palette colour, with the same world phase as on PC, and no texture is ever scaled.

The 1x pattern region leaves the layout, and with it the last reader of the page at (512, 0). The frame then reads exactly two pages: the sheet's, and the one the font and the patterns share. The cost model meters no new bytes: a span stays one `SPRT`, and a change of texture source already costs the `DR_TPAGE` + `DR_TWIN` pair that the backend sends. `GPU_FRAME_BUFFER_BYTES` and the CLUT and pattern slots are re-checked by the existing rule. `pattern 190` and `pattern 257` then pass `offpalette 0`, `nonuniform 0` and `differ` 0. The PC game, its replays and its timing do not change.

## User Stories

1. As a player on the PSX disc, I want the patterned HUD panels in aim mode drawn at their PC position on the 320x240 screen, so that they sit under the frames and icons drawn around them.
2. As a player, I want the slipway-mode background grid drawn at x2 over the whole screen, so that the grid matches the PC.
3. As a player, I want every patterned pixel to be a 2x2 block, so that a pattern has the same size as every other PICO-8 pixel.
4. As a player, I want each pattern to keep its world phase (the pattern bit for the pixel's world x and y, mod 4), so that adjacent patterned shapes line up as on PC.
5. As a player, I want a patterned fill to keep its phase when the camera moves, so that the grid scrolls with the world, not with the screen.
6. As a player, I want a transparent pattern (`fillp` with the transparent flag) to leave its set bits untouched at x2, so that what is under it shows through as on PC.
7. As a player, I want the two colours of a pattern's colour byte drawn exactly, so that the panels keep their two-tone look.
8. As a player, I want a patterned `circfill` drawn at x2 with its spans' phase kept, so that patterned discs match the PC.
9. As a player, I want a patterned fill that is partly off screen clipped exactly as on PC, so that nothing spills into the other display buffer.
10. As a player, I want a patterned fill that is entirely off screen to draw nothing, so that the PSX frame matches the PC frame.
11. As a player, I want a full-width patterned span to read its tile correctly across the whole 320 pixels, so that the right side of the grid matches the left.
12. As a player, I want a frame that alternates patterned fills, sprites and text to never stop on a frame-buffer overflow, so that a long game still runs to the end.
13. As a player, I want sprites and text to keep working while patterns move next to the font, so that this step does not break step 3.2's family.
14. As a maintainer, I want the x2 tile built from the 1x tile by the one x2 upscaler 3.2 added, so that "upscaled x2" keeps one definition and one test.
15. As a maintainer, I want the 1x pattern tile to stay as it is, so that it remains the input of the x2 tile and the oracle of its test.
16. As a maintainer, I want a unit test that checks every texel of the x2 tile against the 1x tile at (u/2, v/2) for many patterns, so that a wrong nibble or a missed line fails `make test`.
17. As a maintainer, I want the x2 tile to repeat with a 16-texel period whose first 8 texels are the 4x4 pattern doubled, so that the tile is proven to repeat seamlessly under the window.
18. As a maintainer, I want the pattern cache to hold x2 tiles, so that the frame's upload sends exactly what the GPU must read.
19. As a maintainer, I want the cache's keys, slot order and capacity unchanged, so that `p8_draw_cost_pattern_key` and `cap gpu_patterns` still describe the PSX build.
20. As a maintainer, I want the texture window computed at x2 (a 16-texel repeat on 16-texel bounds), so that a slot's tile repeats over the whole span with no seam.
21. As a maintainer, I want the window function to assert that a slot lies on 16-texel bounds of its page, so that a moved slot that the window cannot reach fails where it happens.
22. As a maintainer, I want one pure function that turns a 1x patterned span into its x2 `SPRT` (doubled position, size and phase UV), so that the backend has no doubling of its own.
23. As a maintainer, I want `psx_pattern_sprite` to keep its 1x contract, so that its existing pixel-for-pixel tests against the software backend stay valid.
24. As a maintainer, I want a host test that paints x2 patterned boxes and discs from the x2 tile under the x2 window into a 320x240 buffer and compares them with the software screen upscaled x2, in each slot, under a camera, transparent, and for random fills, so that the whole host-side patterned path is proven exact at x2.
25. As a maintainer, I want wide patterned spans split so that no SPRT crosses U = 256, so that no renderer-dependent UV wrap is left in a frame.
26. As a maintainer, I want the x2 pattern slots laid out below the x2 font in the font's page, so that the frame reads only the two pages step 2 planned.
27. As a maintainer, I want the 1x pattern region removed from the layout, so that the layout lists only regions something reads and the page at (512, 0) is free.
28. As a maintainer, I want a layout test that checks the pattern region and every slot lie in the font's page, on 16-texel bounds, do not overlap the font or each other, and that the region holds at least `GPU_PATTERN_SLOTS` slots, so that a moved region fails `make test`.
29. As a maintainer, I want a layout test that checks the textured regions use exactly two pages, so that a region left in the 1x page fails `make test`.
30. As a maintainer, I want the cost model to keep pricing a patterned span as one `SPRT` and a source change as the `DR_TPAGE` + `DR_TWIN` pair, so that the meter stays an upper bound without any new byte.
31. As a maintainer, I want a cost-model test that a pattern after `print` (same page) still costs the pair, so that the meter stays equal to what the backend sends.
32. As a maintainer, I want the backend's "sent more than metered" fatal to stay unreachable, so that the meter remains an upper bound for every frame.
33. As a maintainer, I want the PC and PSX `zone gpu_frame` meter lines to stay equal (`gpu-cost`, `game-gpu-cost`), so that the meter is proven to describe the PSX build.
34. As a maintainer, I want the end-of-frame upload to load each used slot's x2 tile and its CLUT, word-aligned, so that the DMA never reads half a word.
35. As a maintainer, I want `GPU_FRAME_BUFFER_BYTES` re-checked by the existing rule (measured `make state` peak + 1/8, rounded up to 1 KiB), in its own commit when it changes, so that a bisect tells the size apart from the behaviour.
36. As a maintainer, I want `GPU_CLUT_SLOTS` and `GPU_PATTERN_SLOTS` re-checked by the same rule, so that the plan's re-size clause is met.
37. As a maintainer, I want `make state` to pass and `ram-room`/`game-ram-room` to stay positive with the larger pattern caches, so that the replay and the game still fit in 2 MiB.
38. As a maintainer, I want `pattern 190` and `pattern 257` to pass `offpalette 0`, `nonuniform 0` and `differ` 0, so that this family is proven exact at x2.
39. As a maintainer, I want every label that was green before this step to stay green, so that the step regresses nothing.
40. As a maintainer, I want `circfill 263` left red, because its mask also holds `line`, so that the mask rule of `psx/gpu_captures.txt` is applied as written.
41. As a maintainer, I want no capture reference accepted and no ceiling changed, so that human acceptance stays one pass at step 6.
42. As a maintainer, I want every `*.sha256` in `tests/e2e/` unchanged and `make test`/`make e2e` no slower, so that the PC game is proven untouched and the loop stays fast.
43. As a maintainer, I want the PSX game doc to describe the x2 pattern slots, the x2 window and the two pages, so that `docs/` says what is true.
44. As a maintainer, I want plan 19's step notes to record the peak, the buffer size, the slots, the RAM room, the two labels' figures and the remaining red labels, so that the `line` step starts from known figures.
45. As the `line` step's implementer, I want only `line` left at 1x, so that its step touches one family.
46. As a reviewer, I want the PR to touch only the patterned path, its tiles, its window, the layout, their tests, the sizes and the doc, so that I can check it against the plan bullet in one read.

## Implementation Decisions

- **x2 pattern tile (`psx_gpu_textures`), pure.** New constants: the x2 tile size (16 texels) and its units (16 x 16 / 4 = 64). They are checked at compile time to be `PSX_TILES_SCALE` times the 1x size. New function `psx_gpu_textures_pattern_x2(pattern, tile)` fills a 64-unit tile: it builds the 1x tile with `psx_gpu_textures_pattern`, then upscales its 8 lines into 16 x2 lines with 3.2's upscaler (H-3.2-5), reading the 1x tile through its byte view. The 1x function and constants stay: they are the input and the test oracle. Why 16x16 and not the 4x4 pattern doubled in 8x8: step 2's layout settled 16x16 slots, and "the 1x tile upscaled x2" reuses the one upscaler with no new definition.
- **Pattern cache (`psx_pattern_cache`).** The tiles array holds x2 tiles (64 units a slot). A new slot is built by `psx_gpu_textures_pattern_x2`. Keys, CLUTs, slot order, capacity (`GPU_PATTERN_SLOTS`), the `-1` result and the one-cache-per-primitive-buffer rule do not change. The tile accessor's contract (owned by the cache, word-aligned when the cache is) does not change. RAM: two caches x 4 slots x 96 more bytes = 768 bytes of BSS.
- **VRAM layout (`psx_vram_layout`).** `PSX_VRAM_PATTERNS` becomes (576, 352), 64x160: the rest of the font's page below the x2 font, starting at v 96 of the page, a 16-line bound. `psx_vram_layout_pattern_slot(slot)` returns 4x16 units, 16 a row, row by row. A compile-time check proves `GPU_PATTERN_SLOTS` fits the region (160 slots). The 1x pattern region (544, 0) leaves the table. Nothing reads the page at (512, 0) any more, so no region and no `DR_TPAGE` names it; the layout does not list free VRAM. The CLUT region and the pattern CLUT lines do not change.
- **x2 texture window (`psx_pattern`).** `psx_pattern_window(slot)` takes the slot's offset in `psx_vram_layout_texture_page(PSX_VRAM_PATTERNS)` (H-3.2-4), in texels. It asserts that both offsets are multiples of 16 and below 256 ("the texture window only reaches 16-texel bounds at x2"). It returns mask 0x1E on both axes (clears texcoord bits 4..7: a 16-texel repeat) and offsets `u / 8`, `v / 8` (even, in the GPU's 8-texel units). The window unit check against the tile size becomes a check that the x2 tile is 16 texels, which is twice the GPU's 8-texel window unit.
- **x2 patterned sprite (`psx_pattern`), pure.** `psx_pattern_sprite` keeps its 1x contract: placed span, UV = world phase mod 8. A new `psx_pattern_sprite_x2(gfx, tile)` returns `psx_sprite_double(psx_pattern_sprite(gfx, tile))` (H-3.2-6): x2 position and size, UV = 2 x phase, in 0..14. Under the x2 window, x2 texel 2p and 2p+1 both hold 1x texel p, so each doubled pixel reads its world bit, as `fillp` does. It is applied after placement and clipping at 1x, so a doubled span always lies inside 320x240.
- **Span split at the U wrap (pilot decision, replaces "No span split").** A doubled span is up to 320 texels wide, but no `SPRT` may rely on its 8-bit U wrapping past 256: how an upscaled hardware renderer handles that wrap is exactly the renderer-dependent behaviour this plan removes, and step 1 left no reproducer to prove it safe. So a doubled span is cut into `SPRT`s whose `u + w <= 256`; with a 16-texel window each piece restarts at the same phase. The cost model counts the pieces exactly (at most 2 per span at 320 wide), with a test; the re-size rule then applies as usual.
- **Backend (`psx_gpu`).** `add_patterned_spans` sets the texture state (patterns region, slot) through 3.2's function (H-3.2-3), then sends one `add_sprt` per span from `psx_pattern_sprite_x2`, with the slot's pattern CLUT. Patterned spans keep bypassing the textured-sprite UV asserts (H-3.2-7), and get their own assert instead: UV phase below 16, w and h at least 2. The `DR_TWIN` for a slot is built from `psx_pattern_window` as today. The backend keeps sending the `DR_TPAGE` + `DR_TWIN` pair on every change of source, even font to pattern, which share a page: dropping the `DR_TPAGE` there would save 8 bytes, but the backend and the meter would no longer match byte for byte, and the meter would need the page in its state. The end-of-frame `upload_slots` loads each used slot's x2 tile (4x16 units) and its CLUT, queued as today behind the previous `DrawSync`.
- **Cost model (`draw_cost`, `draw_meter`): no byte change, by design.** A patterned span is one `SPRT` at 1x and at x2 (no split, above). A switch to or from a pattern slot already costs the pair (H-3.2-2), and the backend sends exactly that pair. The pattern key and `cap gpu_patterns` do not change. The upload is a `LoadImage`, not part of the primitive buffer, so it is not metered. This family's cost-model change is a new test that pins this (see Testing Decisions). If H-3.2-2 fails, the missing case is added here.
- **Why the bound still holds.** Unchanged from 3.2: the backend only switches state when it emits a `SPRT` for a call whose box the meter counted, and both start from (sheet, none). A subsequence of the meter's switch sequence never has more adjacent changes. The per-frame "sent > metered" fatal stays.
- **Re-size (`gpu_sizes`), own commit when a size changes.** After the behaviour commit, run `make state` and read the `zone gpu_frame` peak and `cap gpu_cluts` / `cap gpu_patterns` from `tmp/state.txt`. Set `GPU_FRAME_BUFFER_BYTES` to `peak x 9 / 8`, rounded up to a multiple of 1024, and the slots to their peak + 1/8, rounded up to a whole slot. No byte changes in this step, so all three are expected to stay as 3.2 left them (H-3.2-8): then no commit, and the figures go in the step note. `make state` must exit 0. `ram-room` and `game-ram-room` must stay positive after the 768 bytes of cache growth. If either goes negative, the step stops before committing and records the blocker.
- **Gate.** `make psx-check`'s `conditions.txt` must have every condition `ok` except `gpu-capture-clean` and `gpu-capture-ceilings`. Those two must fail only on labels not yet due: `circfill 263` (its mask `cls,line,circfill` includes `line`), `line 276`, `all 100`, `all 188` and `all 265`. By the mask rule in `psx/gpu_captures.txt`, a label is proven at the step that converts the last family in its mask. `pattern 190` and `pattern 257` (mask `cls,rectfill,circfill,pattern`; `cls`, `rectfill` and `circfill` are converted since step 2) are due now, each with `offpalette 0`, `nonuniform 0` and `differ` <= 0. `cls 1`, `rectfill 11`, `rect 300`, `pset 131`, `circ 314`, `spr 234` and `print 275` stay green (H-3.2-9). `make psx-check` itself still exits 1 by design, until the `line` step.
- **Ceilings and references untouched** (H-3.2-10).
- **Documentation (PSX game doc).** The VRAM table: the pattern row becomes the x2 pattern slots, (576, 352), 64x160, 4x16 a slot, same page as the font, v 96..255, 16x16 texels on 16-texel bounds. The "Still to do" table under it goes, and so does its sentence: every planned region is now in place. A line says the page at (512, 0) is free. The fill paragraph: a patterned span is a doubled `SPRT` of a 16x16 x2 tile (the 8x8 tile upscaled x2) repeated by a 16-texel texture window, UV twice the world phase mod 8. The "Still to do" section: only `line` still draws at 1x.
- **Step note.** Plan 19's step notes get a "Step 3.3" entry. It records the `make state` peak and the replay that holds it, `GPU_FRAME_BUFFER_BYTES` (old and new, or unchanged), the CLUT/pattern slot figures, `ram-room` and `game-ram-room`, the `pattern 190`/`pattern 257` figures, the remaining red labels, the pages in use, and which H-3.2-k held.

## Implementation Chunks

1. **x2 tile and cache (host only).** Add the x2 tile constants and `psx_gpu_textures_pattern_x2` with its test. Switch the pattern cache to x2 tiles and update `test_psx_pattern_cache`. Add the cost-model test that pins the patterned costs at x2 (and, if H-3.2-2 failed, the missing switch case). The PSX backend still uploads 2x8-unit slots from a cache whose tiles are now 4x16, so this chunk does not build for the PSX alone. It is committed together with chunk 2 as one behaviour commit, and `make test` stays green after each chunk.
2. **Layout, window, x2 sprite and backend (one behaviour commit with chunk 1, because the layout, the window and the backend must agree to build).** Move `PSX_VRAM_PATTERNS` and the slot geometry, compute the x2 window, add `psx_pattern_sprite_x2`. Rewrite `test_psx_vram_layout`'s pattern tests and `test_psx_pattern` at x2. In the backend, send doubled patterned `SPRT`s with their assert and upload x2 tiles. Edit the doc. `make test` and `make e2e` are green, and `make psx-build` compiles and links.
3. **Re-size (own commit, only if a size changes).** Run `make state`, apply the rule, and run `make state` again until it is green.
4. **Gate.** Run `make psx-check` once per attempt (the emulator is shared). Read `conditions.txt` and the capture reports. Check the two labels, the green set and the remaining red set, then write the step note.

## Testing Decisions

- Good tests here check what a caller sees: the texels an x2 tile holds, the pixels a placed, doubled and windowed span paints compared with the software backend upscaled x2, the cost a call reports given the previous calls, where a slot lies. They do not inspect packet bytes or the backend's statics.
- **x2 tile (`test_psx_gpu_textures`).** For the existing pattern set (and every 16-bit pattern in a stride that covers each bit), check all 16x16 x2 texels against the 1x tile at (u/2, v/2), and that texel (u, v) equals texel (u mod 8, v mod 8) (the 16-texel period holds the doubled 4x4 pattern twice). Prior art: the existing 1x pattern test and its `pattern_texel` helper in the same file.
- **Cache (`test_psx_pattern_cache`).** `assert_slot_holds` compares with `psx_gpu_textures_pattern_x2`. The slot-allocation tests are unchanged.
- **Window and x2 paint (`test_psx_pattern`).** The page buffer is filled from `psx_vram_layout_texture_page(PSX_VRAM_PATTERNS)` with the x2 tile of each slot, the rest of the page a texel no tile has. `paint` draws into a 320x240 buffer, applies the 8-bit UV wrap then the window formula (as today), and the oracle is the software screen upscaled x2. Every existing case moves to x2: a box in each slot, the world phase under any camera, a transparent pattern, a patterned disc, random fills. New cases: a full-width span (x 0..159, 320 texels wide at x2) under each camera phase, to prove the wrap at 256, and the window of every slot up to `GPU_PATTERN_SLOTS` has mask 0x1E and even offsets. The existing cost check in those tests (sent `SPRT`s plus the pair <= the metered cost) stays.
- **Layout (`test_psx_vram_layout`).** The pattern slot test: each slot is 4x16 units, inside the pattern region, inside the font's page, on 16-texel bounds of that page (x offset % 4 units, y offset % 16 lines), and no two slots overlap. New: the pattern region and the font share one page, the region holds at least `GPU_PATTERN_SLOTS` slots, and the sheet, the font and the patterns use exactly two distinct pages. The no-overlap and the CLUT tests are unchanged.
- **Cost model (`test_draw_cost`).** One new test: a patterned `rectfill` and a patterned `circfill` cost one `SPRT` per span (the same counts at x2), a pattern after `print` costs the pair although they share a page, and `print` after a pattern costs it too. Prior art: `assert_cost` and `WINDOW_PAIR_BYTES` in the same file. `test_draw_meter` needs no change.
- **Not unit-tested.** `psx_gpu.c` builds only for the PSX. Its behaviour is proven by `pattern 190` and `pattern 257` in `make psx-check` (the emulator's display buffer against the PC capture x2), and by the equal meter lines.
- **Speed.** No new test file, no new e2e file, no slow check. The paints move from 160x120 to 320x240 buffers and the pattern sweep stays at a few hundred thousand texel comparisons, well within `make test`'s budget.

## Out of Scope

- `line` (the `line` step), including making `circfill 263` green.
- `spr`, `print`, the x2 sheet and font, the texture-state design and the page-switch cost (step 3.2). `cls`, `rect`, `rectfill`, `circfill` without a pattern, `pset` and `circ` (steps 2 and 3.1).
- Saving the `DR_TPAGE` on a font-to-pattern change, or adding the page to the meter's state.
- Accepting or editing capture references, or changing capture ceilings.
- Making `make psx-check` exit green (from the `line` step on), the "no stretch left" check (step 4), and the boot ceilings (step 5).
- Any headless reproducer (step 1 found none) and any DuckStation check (step 6).
- Audio, the PC raylib backend, and the e2e replays.

## Further Notes

- After this step, two pages are in use, as step 2's note planned: the x2 sheet (512, 256), and the x2 font with the x2 pattern slots (576, 256). The page at (512, 0) is free VRAM.
- `GPU_PATTERN_SLOTS` is 4 (3 measured). The region holds up to 160 slots, so a future raise of the slot count needs no layout change.
- The per-frame upload grows from 32 to 128 bytes of tile per used slot, at most 512 bytes a frame. That is `LoadImage` traffic outside the primitive buffer and the meter.
- PSX targets run only in chunk 4's gate, once per attempt.
