---
title: "Draw the PSX frame straight into its display buffer"
description: "The PSX GPU backend renders a 160x120 frame and stretches it with one textured quad, which hardware renderers sample unevenly; the frame must be drawn at x2 straight into the display buffer, starting with cls, rect, rectfill and circle spans."
status: needs-triage
---

## Plan step (verbatim, plan 19 § 3, step 2)

> 2. **Draw into the display buffer.** First, lay out the 1024x512 VRAM at x2 on paper: the two display buffers, the x2 sprite sheet, the x2 font, the x2 pattern slots and the CLUTs. Write into this step's note how many texture pages the textured calls use and whether the cost model must count a page switch. Also record the frame-buffer peak and RAM room each `line` option would cost on `make state` (PC only). The x2 sheet alone fills a whole 4-bit page, so the single-page rule of today's `frame_begin` does not hold. Then the frame draws straight into its display buffer and clears it once, at the start. Remove the 160x120 draw area (and its place in `psx_vram_layout`), the end-of-frame black `FILL`, the `POLY_FT4` and the 0x0000-transparency workaround they needed. In the same step, `cls`/`rectfill`/`rect`/circle spans become `TILE` with doubled coordinates. In that step's commit, set all 14 entries of `psx/gpu_capture_references.txt` to pending (`-`), so that `psx-check` can still gate each step. Proof: the capture labels of this family pass `offpalette 0`, `nonuniform 0` and their `differ` ceiling. A label is proven at the step that converts the last family in its capture mask (`psx/gpu_captures.txt`): `circfill 263` also draws `line`, so it is proven at the `line` step, and the `all` labels at step 4. Each step names the labels it must turn green. Any other label may still fail.

## Problem Statement

A player who boots the PSX disc in an emulator with a hardware renderer (DuckStation, Metal, internal resolution above 1x) sees a damaged title screen: glyph outlines of uneven widths, a broken-up logo, particles turned into big squares. The cause is the backend's render-to-texture step: every frame is drawn at 160x120 into a separate draw area of VRAM, then one `POLY_FT4` reads that area as a 15-bit texture and stretches it x2 onto the 320x240 display buffer. Software GPUs (PCSX-Redux's, and a real console by design) sample that quad exactly; a hardware renderer at a higher internal resolution does not, and some texels come out 1 pixel wide and others 3.

The developer cannot remove the stretch in one move: each draw family needs its own x2 primitive, cost-model change and proof. This step removes the stretch itself and converts the first family, the untextured rectangles (`cls`, `rect`, `rectfill`, plain `circfill` spans). It also settles, on paper, the x2 VRAM layout the later textured families need, and measures the `line` options the `line` step will choose between.

## Solution

The frame is drawn straight into the display buffer it will be shown from, at display resolution. `psx_gpu_frame_begin` points the draw area and draw offset at that frame's display buffer (320x240), sets the texture page and an empty texture window as today, and nothing else. The frame's first call, `cart_draw`'s `cls`, clears the whole 320x240 buffer with one black `TILE`. `psx_gpu_frame_end` no longer appends anything: no second draw target, no black `FILL`, no upscale quad. The 160x120 draw area and the 15-bit upscale page disappear from the VRAM layout.

`cls`, `rect`, plain `rectfill` and plain `circfill` spans are `TILE`s whose position and size are the 160x120 tile's, doubled, so each PICO-8 pixel is a uniform 2x2 block, as the `psx-check` captures demand. The cost model drops the quad and the second draw environment from the per-frame overhead and meters `cls` as a `TILE`; `GPU_FRAME_BUFFER_BYTES` is re-sized from the new `make state` peak.

The families not converted yet (`pset`/`circ`, `spr`/`print`, patterned fills, `line`) keep their 1x primitives and coordinates. They land in the top-left 160x120 quarter of the display buffer, at the wrong size, until their own step converts them; they cannot crash or overflow (see Implementation Decisions).

All 14 accepted capture references become pending (`-`). `make psx-check` then fails by design, and the step is gated on its `conditions.txt`: every condition `ok` except `gpu-capture-clean` and `gpu-capture-ceilings`, and those two fail only on labels this step does not owe. This step owes three labels: `cls 1`, `rectfill 11`, `rect 300`.

The step also delivers, for its plan note: the x2 VRAM layout, the number of texture pages the textured calls will use (two) and the fact that the cost model must count a page switch from the `spr`/`print` step on, and the frame-buffer peak and RAM room of each `line` option measured on `make state`.

## User Stories

1. As a player using a hardware renderer, I want the frame drawn without stretching any texture, so that the PSX image does not depend on how a renderer samples a scaled quad.
2. As a player, I want every PICO-8 pixel of a cleared screen, a rectangle outline, a filled box and a plain filled disc drawn as an exact 2x2 block, so that those shapes look the same under every renderer.
3. As a player, I want each frame to start from a black screen, so that no part of an older frame shows through.
4. As a player, I want the displayed frame never to be the one being drawn, so that I see no half-drawn frame.
5. As a developer, I want the frame to draw into the display buffer it will be shown from, so that VRAM no longer needs a separate 160x120 draw area.
6. As a developer, I want the end-of-frame black `FILL`, the upscale `POLY_FT4` and the 0x0000-transparency comment that justified them gone, so that no code is left for a stretch that no longer happens.
7. As a developer, I want `cls` to be one black `TILE` over the 320x240 buffer, so that the clear goes through the same draw offset and clip as every other primitive.
8. As a developer, I want `rect`'s up to four edges to be doubled `TILE`s that still never overlap, so that the outline is exact at x2.
9. As a developer, I want a plain `rectfill` to be one doubled `TILE`, so that a filled box costs what it cost before.
10. As a developer, I want each plain `circfill` span to be one doubled `TILE`, so that discs stay exact pixel for pixel at x2.
11. As a developer, I want the doubling done by one small pure function over the 160x120 tile, so that the existing pixel-for-pixel tests of `psx_tiles` stay the proof of the shape and only the scale is new.
12. As a developer, I want that function host-tested against the software backend's screen upscaled x2, so that a wrong offset or size fails `make test` instead of a 110-second emulator run.
13. As a developer, I want the x2 scale defined once, so that the later family steps reuse it rather than writing their own 2.
14. As a developer, I want the families not converted yet to keep their primitives, byte sizes and 1x coordinates, so that the frame they send never exceeds what the meter counted and no `p8_fatal` fires.
15. As a developer, I want the draw area to clip to the 320x240 display buffer, so that no primitive, converted or not, writes into the textures or CLUTs.
16. As a developer, I want textured calls to keep reading the one 1x texture page `frame_begin` sets, so that `spr`, `print` and patterned fills keep working until their own step.
17. As a developer, I want the cost model's per-frame overhead to be one draw environment plus the texture page and window pair, so that the meter matches what `frame_begin` now sends.
18. As a developer, I want `cls` metered as a `TILE`, so that the cost model names the primitive the backend sends.
19. As a developer, I want the compile-time size checks to cover the new frame overhead and drop the `FILL` and `POLY_FT4` checks, so that the cost model still bounds the packets.
20. As a developer, I want `GPU_FRAME_BUFFER_BYTES` re-sized at the new `make state` peak + 1/8, rounded up to 1 KiB, so that the reserve stays tight and `ram-room` stays positive.
21. As a developer, I want the VRAM layout module and its test to lose the draw area and the upscale page, so that the layout lists only regions still used.
22. As a developer, I want all 14 capture references set to pending in the same commit, so that `gpu-capture-references` keeps passing while every capture changes.
23. As a developer, I want `cls 1`, `rectfill 11` and `rect 300` to pass `offpalette 0`, `nonuniform 0` and their `differ` ceiling, so that this step proves its family.
24. As a developer, I want every other `psx-check` condition to stay `ok`, so that only the not-yet-due labels explain a red run.
25. As a developer, I want the `gpu-capture-clean` and `gpu-capture-ceilings` failures to name no label this step owes, so that the gate is mechanical.
26. As a developer, I want the e2e hashes and `make test-all` untouched and no slower, so that the game is proven unchanged.
27. As a developer, I want the x2 VRAM layout written down before the spr/print and pattern steps, so that those steps upload their textures where nothing else lives.
28. As a developer, I want that layout to keep every 1x texture region valid until the step that replaces it, so that the half-converted builds between steps keep drawing.
29. As a developer, I want to know how many texture pages the x2 textured calls need, so that the spr/print step knows whether `frame_begin`'s single page still holds.
30. As a developer, I want to know whether the cost model must count a page switch, so that the spr/print step changes the meter in the same commit as the primitives.
31. As a developer deciding the `line` step, I want the frame-buffer peak and RAM room of the two-`LINE_F2` option, the 2x2-pixel-list option and the merged-runs option measured on `make state`, so that the choice rests on numbers.
32. As a developer, I want those measurements taken with a throwaway local change, so that no measurement code stays in the tree.
33. As the plan pilot, I want the layout, page count, page-switch answer and line figures in the commit message under a "Step note" heading, so that I can copy them into the plan's step note.
34. As a reader of `docs/psx-game.md`, I want the GPU backend description and the VRAM layout table to say what the code now does, so that the doc stays true.
35. As a reader of `docs/psx-game.md`, I want the planned x2 texture layout marked as still to do, so that the doc doesn't claim it is in place.
36. As the human doing final acceptance, I want the references re-accepted once at the end, so that I review 14 captures once rather than after every step.

## Implementation Decisions

**Frame structure (GPU backend).**
- `psx_gpu_frame_begin` emits, in order: draw area and draw offset on the current display buffer (`PSX_VRAM_DISPLAY_0` or `_1`, the one `frame_end` will submit), the texture page of the current 1x textures (unchanged), and the empty texture window. It emits no clear.
- The frame's one clear is `cls`: `cart_draw` opens every frame with `p8_cls`, in the game and in both replays, and a captured frame's mask always holds `cls`. `cls` is one black `TILE` of the full screen, doubled to 320x240. The display buffer holds frame N−2 until that clear; a frame with no `cls` would show it. That can't happen today and needs no guard. This reads the plan's "clears it once, at the start" as the opening `cls`, not as a second clear in `frame_begin`, which would cost 16 bytes a frame to clear twice.
- `psx_gpu_frame_end` keeps its metering check, `DrawSync`, slot uploads, `PutDispEnv` of the other buffer and `DrawOTag`, in that order, and appends no primitive. `PutDispEnv` still comes before `DrawOTag`, so the GPU never draws into the buffer on screen. `psx_gpu_frame_show_last` is unchanged.
- Removed: `add_black_fill`, `add_upscale`, the second `add_draw_target` and `add_no_texture_window` of `frame_end`, the 15-bit texture constant, and the comment about 0x0000 texels showing the previous frame. The spr CLUT's black-with-mask-bit (0x8000) and transparent 0x0000 stay: they are PICO-8's `palt`, not the upscale workaround.
- The header comment of `psx_gpu_frame_end` stops mentioning the clear and the upscale.

**Doubling.**
- `psx_tiles` gains the x2 scale, `PSX_TILES_SCALE = 2`, and one pure function, `psx_tiles_double` (returns the tile with x, y, w and h multiplied by `PSX_TILES_SCALE`, in display-buffer coordinates). Its contract is that the result lies inside 320x240 whenever the input lies inside 160x120. The existing `psx_tiles_*` functions keep returning 160x120 tiles. Their pixel-for-pixel tests stay, and the patterned path (still 1x) keeps feeding them to `psx_pattern_sprite`.
- In the backend, the `TILE` emitter used by `cls`, `rect`, plain `rectfill` and plain `circfill` doubles its tile. The `TILE_1` emitter of `pset`/`circ`, the `LINE_F2` of `line`, and the `SPRT`s of `spr`, `print` and patterned fills are untouched.
- A compile-time check in the backend ties the 320x240 display to `P8_SCREEN_WIDTH`/`HEIGHT` times `PSX_TILES_SCALE`.

**What keeps the unconverted families safe (a fact about the code, not a hypothesis).**
- Their primitive types, counts and byte sizes do not change, and neither do their cost-model entries, so `used <= metered_bytes` still holds and neither the overflow `p8_fatal` nor the "sent more than metered" `p8_fatal` can newly fire.
- They keep 1x coordinates relative to the draw offset, now the display buffer's origin. They land in its top-left 160x120 at half size, which is wrong until their step but harmless.
- The draw area clip is the 320x240 display buffer, so no primitive writes outside it. The textures (x ≥ 512) and CLUTs (x 640) are beyond it, and the other display buffer is outside the clip.
- `line` stays one unclipped `LINE_F2`. The GPU now clips it to 320x240 instead of 160x120, so a 1x line whose endpoints pass 159/119 shows its overrun to the right of or below the quarter. Its endpoint assert (|coordinate| < 1024) is unchanged and still holds.
- `spr`, `print` and patterned fills keep reading the one 1x 4-bit page `frame_begin` sets, through the uploads `psx_gpu_init` and `upload_slots` already do. Nothing moves in VRAM for them in this step.

**Cost model.**
- `p8_draw_cost_frame_begin` returns one `P8_GPU_DR_ENV_BYTES` (area + offset) plus the `DR_TPAGE` + `DR_TWIN` pair: 52 bytes, 3 primitives. Today it is 128 bytes, 5 primitives (two environments, the pair, the quad). Its header comment drops "x2 upscale quad".
- `p8_draw_cost_cls` returns one `P8_GPU_TILE_BYTES`. That is the same 16 bytes as before, but it names the primitive sent.
- `P8_GPU_FILL_BYTES` and `P8_GPU_POLY_FT4_BYTES` are removed with their `SIZE_CHECK`s. The frame-overhead `SIZE_CHECK` becomes: `DR_AREA` + `DR_OFFSET` + `DR_TPAGE` + `DR_TWIN` ≤ `DR_ENV` + `DR_TPAGE` + `DR_TWIN` bytes.
- The other entries are unchanged: `rect` 4 `TILE`s, `rectfill`/`circfill` spans 1 `TILE` each. Doubling changes no byte count.
- The cost header's "a call whose screen box misses 0..159 x 0..119 costs 0" stays true: costs are judged on API coordinates.

**Sizes.** `GPU_FRAME_BUFFER_BYTES` = `make state`'s new `zone gpu_frame` peak + 1/8, rounded up to 1 KiB, by the existing rule. It still divides by 4. `GPU_CLUT_SLOTS` and `GPU_PATTERN_SLOTS` don't change: this step changes no CLUT or pattern counting. `make state` must pass (margin) and `ram-room`/`game-ram-room` must stay positive in `psx-check`.

**VRAM layout module.** `PSX_VRAM_DRAW_AREA` and `psx_vram_layout_upscale_page` are removed. The other regions don't move in this step: the display buffers at (0,0)/(0,240), the 1x sheet, font and patterns in the 4-bit page at (512,0), and the CLUTs at (640,0).

**The x2 VRAM layout on paper** (16-bit units; decided here, built by steps 3.2 and 3.3, recorded in the step note and in `docs/psx-game.md` as still to do):

| region | x, y | size (units) | notes |
|---|---|---|---|
| display buffers | (0,0), (0,240) | 320x240 each | unchanged |
| 1x sheet / font / patterns | page (512,0) | as today | kept until steps 3.2 / 3.3 replace them, then free |
| x2 sprite sheet | page (512,256) | 64x256 | 256x256 texels 4-bit: the whole page |
| x2 font | page (576,256), v 0..95 | 64x96 | 256 codes, 8x12-texel cells (4x6 doubled), 32 a row, 8 rows |
| x2 pattern slots | page (576,256), v 96..255 | 4x16 a slot | 16x16 texels (8x8 doubled), on 16-texel bounds for the texture window, 16 a row, up to 160 slots |
| CLUTs | (640,0) | 16x64 | unchanged; 64 lines hold `GPU_CLUT_SLOTS` + 16 font + `GPU_PATTERN_SLOTS` |

No x2 region overlaps a 1x region, so every intermediate build keeps both alive. The pages at y 256 sit right of the display buffers (x ≥ 320).

- **Texture pages:** the textured calls use **two** 4-bit pages, the sheet's (512,256) and the font + patterns' (576,256). The x2 sheet fills its page, so no single-page rule can hold.
- **Page switch: yes, the cost model must count it, from step 3.2 on.** Today's window state (`patterned`, `pattern_key`) treats `spr` and `print` as the same state, so a `spr`↔`print` alternation costs nothing. At x2 it is a `DR_TPAGE` change. Step 3.2 must add the page to the window state, so that any change of (page, window) costs the existing `DR_TPAGE` + `DR_TWIN` pair, and the backend must then really send the `DR_TPAGE` where the page changes. Font ↔ pattern changes stay window-only on one page, still covered by the pair.

**`line` options measurement** (PC only, throwaway, nothing committed but the figures):
- Starting from this step's committed cost model, with `pset` and `circ` priced at `P8_GPU_TILE_BYTES` a pixel (their step 3.1 cost, known now), swap `p8_draw_cost_line` locally for each option, run `make state`, and read the `zone gpu_frame` peak:
  - (a) two `LINE_F2`s: 2 × `P8_GPU_LINE_F2_BYTES` a line;
  - (b) pixel list as 2x2 `TILE`s: (max(|dx|, |dy|) + 1) × `P8_GPU_TILE_BYTES`, the software Bresenham's pixel count, unclipped (an upper bound);
  - (c) merged runs: (min(|dx|, |dy|) + 1) × `P8_GPU_TILE_BYTES`;
  - plus the unchanged one-`LINE_F2` line, as a baseline.
- For each, record the peak, the buffer it implies (peak + 1/8, rounded up to 1 KiB), and the RAM room. The RAM room is this step's `psx-check` `gpu ram-room` and `game ram-room` "left" figures minus 2 × (option buffer − this step's `GPU_FRAME_BUFFER_BYTES`). Then revert the local change.
- The figures exclude the page-switch cost step 3.2 adds (at most 16 bytes per switch); step 3.4 re-measures.

**Step note hand-off.** The plan pilot writes the plan's `### Step 2` note. The implementer puts the layout table, the page count, the page-switch answer, the line figures, the new `GPU_FRAME_BUFFER_BYTES` and `make state` peak, and the three labels proven into the converting commit's message, under a "Step note" heading. The pilot copies it from there.

**References.** In the converting commit, all 14 lines of `psx/gpu_capture_references.txt` become `<label> <frame> -`, keeping order. `psx/gpu_capture_ceilings.txt` is untouched: `cls`, `rectfill` and `rect` must reach their ceiling of 0. No ceiling is raised for a label not due.

**Labels this step must turn green:** `cls 1` (mask `cls`), `rectfill 11` (`cls,rectfill`), `rect 300` (`cls,rect`). Each must pass `offpalette 0`, `nonuniform 0`, `differ` ≤ ceiling (0). Not due: `pset 131` and `circ 314` (step 3.1); `spr 234` and `print 275` (3.2); `pattern 190` and `257` (3.3); `line 276` and `circfill 263` (3.4, whose mask holds `line`: plain `circfill` spans are converted here but proven there); `all 100`, `188` and `265` (step 4).

**Gate.** `make test-all` green and no slower, every `*.sha256` in `tests/e2e/` unchanged, `make state` green, and `make psx-check`'s `conditions.txt` with every condition `ok` except `gpu-capture-clean` and `gpu-capture-ceilings`. Their failure lines must name none of `cls f1`, `rectfill f11`, `rect f300`. `gpu-capture-references` is `ok`, with 14 "pending human acceptance" lines. If `gpu-replay-done` or the game boot fails on its vsync ceiling, which is not expected since the quad and the `FILL` wrote 153,600 pixels a frame that are gone, raise that ceiling by its existing rule in this step and say so in the step note. Step 5 re-measures.

**Docs.** `docs/psx-game.md`'s GPU backend section changes in the same commit:
- frame structure: no draw area, no end-of-frame `FILL` or quad;
- `cls` is a doubled `TILE`; `rect`/`rectfill`/`circfill` spans are doubled `TILE`s;
- the families still at 1x are named as such;
- the VRAM layout table loses the draw-area row;
- the planned x2 layout is added, marked as still to do (steps 3.2/3.3);
- the paragraph on the `DR_TPAGE` + `DR_TWIN` pair keeps saying a single page for now.

## Implementation Chunks

1. **The x2 tile.** `PSX_TILES_SCALE` and `psx_tiles_double`, with their host tests in `test_psx_tiles`. Pure addition; nothing calls it yet. `make test` green.
2. **Draw into the display buffer.** One commit, because the cost model, the backend and the size checks must agree for the PSX build to boot:
   - the frame structure;
   - `cls`/`rect`/`rectfill`/`circfill` through the doubled `TILE`;
   - the removals in the backend, the cost model and the VRAM layout module, with their tests updated (`test_draw_cost`, `test_draw_meter`, `test_psx_vram_layout`);
   - `GPU_FRAME_BUFFER_BYTES` re-sized from `make state`;
   - the 14 references set pending;
   - `docs/psx-game.md` (with the x2 layout as still to do).

   Gate as above, then the "Step note" commit message, without the line figures yet.
3. **Line options measurement.** The throwaway `make state` runs of the four line variants, then the revert (`git status` clean afterwards). The figures join the "Step note" text: amend chunk 2's message if it is not pushed yet, otherwise make an empty `docs:` commit (`--allow-empty`) whose message carries them. No code changes in this chunk.

## Testing Decisions

- A good test drives a module through its public interface and checks what a user of it observes: pixels painted, bytes metered, regions returned. Never private state.
- **`test_psx_tiles`** (prior art: its own `paint` / `assert_painted_is_the_software_screen` helpers):
  - a doubled-tile painter on a 320x240 grid, compared with the software screen upscaled x2 (each pixel a 2x2 block), for the cases its `rect`, `rectfill` and `circfill` tests already cover: clipped, camera-shifted, degenerate, full screen;
  - the doubled tiles never overlap and stay inside 320x240;
  - the full-screen tile doubles to exactly (0, 0, 320, 240).
- **`test_draw_cost`**: the frame overhead is `DR_ENV` + `DR_TPAGE` + `DR_TWIN` bytes, 3 primitives, and still resets the window; `cls` costs one `TILE`. Every other existing case stays.
- **`test_draw_meter`**: the expectations built on `P8_GPU_FILL_BYTES` switch to `P8_GPU_TILE_BYTES`; the base overhead comes from `p8_draw_cost_frame_begin` as today.
- **`test_psx_vram_layout`**: the upscale-page test goes. The non-overlap, single-texture-page, pattern-slot and CLUT tests stay as they are: the layout of the 1x textures doesn't change in this step.
- **`test_state`, `test_state_format`, `test_state_peaks`** read `GPU_FRAME_BUFFER_BYTES` symbolically and need no change. A failure there means the re-size broke the rule.
- **The GPU backend itself** (PSn00bSDK, MIPS only) has no host test. It is proven by `make psx-check`'s captures: `cls 1`, `rectfill 11` and `rect 300` at `offpalette 0`, `nonuniform 0`, `differ 0`; `gpu-cost` and `game-gpu-cost` (PSX meter lines equal the PC's); `gpu-replay-done` (no metering `p8_fatal`); `ram-room`.
- The e2e replays are the source of truth: every `*.sha256` and `*.end-state` stays identical. The end-state ignores `zone`/`cap` lines, so the meter change can't move them.
- No new target is added to `make test` or `make e2e`. The doubled-tile tests are a few `psx_tiles` cases at host speed.

## Hypotheses on step 1 (not done when this PRD was written)

- **H-1-1:** Step 1 produced no headless reproducer: both DuckStation's regression runner with a hardware renderer and PCSX-Redux's OpenGL GPU on Mesa llvmpipe were exhausted, so step 2's proof is the PCSX-Redux software captures of `make psx-check` only.
  *If it fails* (a reproducer exists): the plan says each family's labels must also pass under it from step 2 on. Add to the gate that the step 1 target, run on `cls 1`, `rectfill 11` and `rect 300`, passes `offpalette 0`/`nonuniform 0` at blocks of 2 × its internal scale. Add a user story for it, and record its result in the step note. The target stays outside `make test-all`.
- **H-1-2:** If a reproducer exists (H-1-1 fails), it is a make target, outside `make test-all` and outside `psx-check`'s conditions, that captures the frames of `psx/gpu_captures.txt` by label and reports per label, so that step 2 can name its three labels to it.
  *If it fails* (for example, it only captures the title screen, or judges whole images): step 2 runs it as it is, and only its verdict on the frames this family draws counts. If it can't single out those labels, step 2 records that its labels cannot be judged under the reproducer before step 4, and the gate falls back to H-1-1's.
- **H-1-3:** Step 1 committed no change to the GPU backend, the cost model, `src/p8/gpu_sizes.h`, the VRAM layout module, `psx/gpu_captures.txt`, `psx/gpu_capture_ceilings.txt` or `psx/gpu_capture_references.txt`. The HEAD state this PRD reads is step 2's starting point, with 14 labels and the ceilings above.
  *If it fails:* re-read those files before chunk 2. If step 1 added capture labels (a title-screen frame, say), the "14 entries" become all entries, and each new label is placed by the mask rule. A title label with mask `all` is due at step 4, not here. Any figure this PRD quotes (128-byte overhead, 73,728-byte buffer) is re-derived from the new HEAD.
- **H-1-4:** Step 1 added no condition to `make psx-check` and left it green ("thirty conditions"), so the step 2 gate is exactly "every condition `ok` except `gpu-capture-clean` and `gpu-capture-ceilings`".
  *If it fails:* a condition step 1 added joins the gate: it must be `ok`, or fail only on labels not due under the mask rule. If the baseline `psx-check` is no longer green before chunk 2, stop and record the blocker; don't start converting.
- **H-1-5:** Nothing in step 1's note contradicts the plan's premise that a `TILE` with doubled coordinates is exact under a hardware renderer, i.e. uniform blocks of 2 × the internal scale.
  *If it fails* (step 1's reproducer shows doubled `TILE`s, or `FILL`, misplaced at its scale): step 2 still removes the quad. Its labels can't pass under that renderer, so the step note records the blocker for step 6, as the plan's step 3 `line` rule does. The software-capture gate stays.

## Out of Scope

- Converting `pset`/`circ` (step 3.1), `spr`/`print` and the x2 sheet and font uploads (3.2), patterned fills and the x2 pattern tiles (3.3), and `line` (3.4). Until then those families draw at 1x in the top-left quarter of the display buffer, and their labels may fail.
- Moving or uploading any texture, changing the CLUT or pattern slot counts, and adding the page to the cost model's window state (3.2).
- Choosing the `line` option: this step only measures the options.
- The final check that no `POLY_FT4` or scaled texture remains (step 4), the boot-ceiling re-size (step 5, unless a ceiling blocks this step's gate), and human re-acceptance of the references (step 6).
- `make psx-full-game-check` (plan 11). Audio. Any PC-side drawing change.

## Further Notes

- `make psx-check` exits 1 by design from this step until step 3.4. The proof is the conditions file, not the exit code.
- The meter's per-frame overhead drops by 76 bytes (128 → 52), so the `make state` peak and the re-sized buffer go down. `ram-room` can only grow.
- PCSX-Redux's capture reads the displayed 320x240 buffer. The comparator masks bit 15, so black written by a `TILE` (0x0000) and by a `SPRT` through a CLUT (0x8000) both read as colour 0. Inside one primitive's 2x2 block both are uniform.
