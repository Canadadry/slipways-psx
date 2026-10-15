---
title: "Outlined text in one primitive per glyph (D1/B4)"
description: "Every outlined string is printed nine times, eight black passes and one colour pass, so outlined text costs nine print calls and nine SPRTs per glyph where one would draw the same pixels."
status: needs-triage
---

## Problem Statement

`hud_print_outlined` is the game's only way of drawing outlined text. It
calls `p8_print` nine times: eight passes in colour 0 at the eight
neighbour offsets, then one pass in the text colour. Every label, button
row, tooltip row, score readout, message box line and probe label goes
through it. On the PSX GPU path each pass walks the string and emits one
3×5 SPRT per glyph, so an outlined string costs nine print calls and nine
SPRTs per glyph.

The report (`docs/psx-perf-levers.md`, D1/B4) measured the upper bound of
removing eight of those nine passes ("one-print"). It found −542.3 hblanks
a frame on `draw-5564` and −102.1 on `draw-6000`, and −64 to −287 on the
other short runs. Those figures were taken at O2 with the meter and the
filter compiled in. Since then:

- F1, G1, B1, G2 and B3 have been taken.
- The per-call costs were re-measured at O3, asserts off, no meter, no
  filter (commit `be39841`). `print` costs 1.514 hblanks a call pooled
  (report: 1.892, ratio 0.80) and 0.267 a glyph.
- 2% of the judge's mean frame is 10.6 hblanks (step 2.7's note: mean
  530.2).

At today's per-call cost the print family is still the largest single
draw family on `draw-5564`. The report's figure scaled by 0.80 gives
roughly 430 hblanks a frame there and 80 on `draw-6000`. Both are far
above 10.6. This step's opening probe measures the real figure.

The plan also names an alternative. D2 (button rows and tooltip), D3 (the
open message box) and D4 (score panel readouts) cache whole elements
that are mostly outlined text. The plan takes D1/B4 "instead of D2+D3+D4
if it covers them". Nothing settles yet whether D1/B4 leaves enough of
those three to be worth caching.

## Solution

1. **A new runtime primitive, `p8_print_outlined`.** It draws a string in
   a colour with a one-pixel colour-0 outline in all eight directions. Its
   pixels are exactly those of the eight colour-0 prints at the eight
   neighbour offsets followed by the colour print. Every draw backend
   implements it:
   - **Software backend (PC, the PSX software replay):** literally those
     nine passes, so the PC picture is the old one by construction.
   - **PSX GPU backend:** one 5×7 SPRT per glyph, from an outlined font.
     The outlined font is pre-drawn into the free lines of the existing
     font texture, with texel 1 for the glyph, texel 2 for its outline and
     0 elsewhere. Each existing font CLUT gains entry 2, the GPU's black,
     so no new CLUT and no new texture page are needed.
2. **`hud_print_outlined` calls it once**, with the same aligned position
   and the colour's low nibble. The nine-pass table leaves the game.
3. **Measured as the plan requires:**
   - a throwaway gross-target probe first;
   - the lever's own probe against a fresh baseline;
   - the unit costs re-taken after the switch;
   - throwaway probes of what is left of D2, D3 and D4, so the step can
     say which of them D1/B4 covers.

The picture is the same on every frame of both platforms. Every e2e hash
and every `psx-check` capture stays unchanged at 0 tolerance. Draw time
goes down wherever outlined text is on screen.

## User Stories

1. As the plan pilot, I want D1/B4's gross target measured on the short runs before any code, so that whether the levers already taken cover it is decided on today's cost, as the plan's rules require.
2. As the plan pilot, I want the gross target set against the re-measured unit cost (1.514 a print call, 0.267 a glyph) and against the report's −542.3 / −102.1, so that the note cites both figures.
3. As the plan pilot, I want a written rule for "covered" that matches step 8's: under 10.6 hblanks a window frame on both `draw-5564` and `draw-6000`, so that the decision is mechanical.
4. As the plan pilot, I want to know before the switch whether a `psx-check` capture shows an outline on a non-black pixel, so that pixel identity of the outline is actually proven at 0 tolerance and not hidden by a black background.
5. As a game developer, I want one runtime call that prints outlined text, so that the game no longer issues nine prints for one label.
6. As a game developer, I want `hud_print_outlined`'s signature and alignment unchanged, so that none of its four callers (probe label, message box, layout rows, draw-function readouts) change.
7. As a game developer, I want the new call's contract on its declaration (outline colour 0, eight directions, the colour's low nibble), so that nobody has to read a backend to know what it draws.
8. As a player on PC, I want outlined text to look exactly as before, so that nothing about the game changes.
9. As a player on PSX, I want outlined text to look exactly as on PC, at the screen edges and under any camera, so that both platforms stay the same game.
10. As a player, I want neighbouring glyphs' outlines to meet in the gap column exactly as before, so that strings keep their spacing and look.
11. As a player, I want text drawn later to cover text drawn earlier exactly as before, so that overlapping labels keep their order.
12. As the maintainer of the e2e replays, I want every `*.sha256` unchanged, so that the change is proven not to touch the game.
13. As the maintainer of `psx-check`, I want every capture unchanged at 0 tolerance, masked ones included, so that the GPU outlined font is proven pixel-identical.
14. As the maintainer of the draw filter, I want the new call counted and masked under the `print` family, so that the capture list, the masks and the references stay as they are.
15. As the maintainer of the cost model, I want the outlined call costed as one SPRT per non-space glyph when its outlined box meets the screen, so that the meter still bounds what the GPU emits.
16. As the maintainer of the draw meter, I want `make state`'s `zone gpu_frame` peak to drop and be recorded, so that the buffer's margin is known for later steps.
17. As the maintainer of the breakdown boot and the PC call profile, I want the outlined call timed and counted under `print` on both sides, so that the unit-cost join's PC-against-PSX call check still accepts every frame.
18. As the PSX backend maintainer, I want the outlined font in the font region's unused lines on the same 4-bit page, so that no textured call ever changes the page and no new VRAM is taken.
19. As the PSX backend maintainer, I want the outline's black from the same colour the plain font uses for colour 0, so that an outline is the exact pixel a colour-0 print would write.
20. As the PSX backend maintainer, I want the plain font to keep its pixels when the font CLUTs gain entry 2, so that plain prints are untouched.
21. As the PSX backend maintainer, I want the outlined glyph placement as a pure helper tested on the PC against the software backend, so that clipping and camera bugs are caught without an emulator.
22. As the PSX RAM budget's owner, I want the cost recorded (no new texture upload, no new CLUT, a few hundred bytes of code), with `ram-room`, `game-ram-room` and `gpu-ram-room` holding, so that the cost is visible.
23. As the plan pilot, I want the lever's measured draw delta per run against the gross target, the unit-cost estimate and the report's figure, with asserts mode and opt level, so that the note follows the plan's rules.
24. As the plan pilot, I want D1/B4 kept or reverted on its pooled draw delta of the two draw runs, so that a lever that is not a gain does not stay.
25. As the plan pilot, I want the unit costs re-taken after the switch, so that later steps judge their levers on the print family as it now stands.
26. As the plan pilot, I want what is left of D2, D3 and D4 measured after D1/B4, each with a verdict against 10.6, so that steps 13 and 15 can skip a covered lever by citing this step's note.
27. As the plan pilot, I want every assumption about step 8's outcome written as a hypothesis, so that a different outcome of step 8 changes a known decision and nothing silently.
28. As a developer running `make test` and `make e2e`, I want them no slower, so that the fast loop stays fast.
29. As a maintainer, I want unit tests that froze the nine calls, if any, updated rather than kept, so that the e2e replays stay the source of truth.

## Hypotheses on step 8

Step 8 (`docs/prd/triage/phase3-step8-background-image-cache.md`) is not
done when this PRD is written. Its chunks 1 and 2 are committed (`da39d71`
and `be39841`). Its GPU spike was running. Each hypothesis below is
checked when this step starts, against step 8's note in
`docs/plan-21.md` and the code at HEAD. When one does not hold, the
decision it names is replaced as stated.

- **H-8-1.** Step 8 is finished (committed, its note written) before
  this step starts. D1/B4 uses no image cache, so nothing in this step
  waits on step 8's outcome except H-8-2, H-8-5 and H-8-6.
- **H-8-2.** The draw-backend table holds step 8's four image entries
  (`image_begin`, `image_window`, `image_end`, `image_draw`).
  - `print_outlined` is added as one more entry, after `print`.
  - Each wrapper forwards it through the same helpers as `print`. The
    behaviour inside a build then follows step 8's rules with nothing
    added:
    - the meter puts its bytes in the build count;
    - the filter passes it and records `print` for the slot;
    - the GPU's `packet()` writes it into the build buffer;
    - the software `plot` writes it into the image.
  - A unit test checks that an outlined print inside a build lands in the
    image like a plain print does.
  - **If step 8 reverted its cache code** (the "cannot be made
    pixel-identical" case), there are no image entries. The entry is added
    after `print` in the nine-entry table, and that test is not written.
- **H-8-3.** The images region, if step 8 added it, is x 704–959. The
  outlined font sits inside the font region (x 512–543, y 128–255), so
  it overlaps nothing either way. The layout's no-overlap test covers
  whatever regions exist.
- **H-8-4.** Step 8's D5 switch was kept or reverted. This affects only
  the baseline: every probe of this step takes its baseline at the HEAD
  it runs on.
- **H-8-5.** The cache was proven pixel-identical, and steps 12, 13 and
  15 will use it. Then the residual probes of D2, D3 and D4 run (chunk
  4).
  - **If step 8's note says the cache cannot be made pixel-identical,**
    D2, D3 and D4 are skipped in their own steps anyway, citing step 8.
  - Chunk 4 then runs no probe. The note says "D2, D3, D4: no cache
    (step 8), not measured".
- **H-8-6.** The probe tooling (`make psx-probe-baseline`,
  `make psx-probe`) and `make frame-profile-unit-cost` work as in step 8,
  at asserts off, O3, no meter, no filter. If step 8 added a `build`
  field to the `gpu-peak` line or a `zone gpu_build` line, they are read
  as they are. This step changes neither.
- **H-8-7.** The unit-cost table at HEAD is still `be39841`'s, since
  step 8 changed no per-call cost of `print`. If step 8's note gives a
  newer table, its `print` figures replace 1.514 and 0.267 throughout.
- **H-8-8.** The 2% threshold stays 10.6 hblanks (step 2.7's mean frame).
  Step 8 runs no judge, and no phase ends between steps 8 and 9.

## Implementation Decisions

### Coverage, both ways

- **D1/B4 against the levers already taken** (F1, G1, B1, G2, B3, and D5
  if kept). None of them removes print calls, so D1/B4 is expected not
  covered. The measurement decides:
  - **Gross target:** the draw delta per window frame of the throwaway
    `one-print` probe, a patch that keeps only the colour pass of
    `hud_print_outlined`. It changes pixels, which does not matter: only
    time is read.
  - **Run with** `make psx-probe PATCH=<patch> PSX_PROBE_LABEL=one-print`
    on all six short runs, against `make psx-probe-baseline` at the HEAD
    the step starts from. Asserts off, O3.
  - **Rule:** covered if the gross target is under 10.6 hblanks a window
    frame on both `draw-5564` and `draw-6000`. This is step 8's rule,
    recorded here as a decision.
  - **If covered:** nothing is built. The note records the figure and the
    step ends.
- **D1/B4 against D2 + D3 + D4.** The plan's "instead of D2+D3+D4 if it
  covers them" is read as follows, recorded as a decision:
  - D1/B4 is taken first, whatever the alternative's size. It needs no
    image cache, reaches every outlined string in the game, not just three
    elements, and comes first in the plan's order.
  - The report's rule picked the larger side, D2 + D3 + D4, from O2
    figures. That rule decided which side the *estimate* counts. It does
    not forbid taking both, since caching D2, D3 or D4 after D1/B4 still
    saves whatever D1/B4 left.
  - After the switch, chunk 4 measures what each of D2, D3 and D4 still
    costs, with the same rule: covered if under 10.6 on both draw runs.
    `draw-6000` has no message box, so D3's figure there is expected at
    0.
  - A covered lever is skipped in its own step (D4 in step 13, D2 and D3
    in step 15), citing this step's note. A lever that is not covered
    stays for its step, which opens with its own probe as usual.
- **Residual probes** (throwaway, not committed), each against a
  `make psx-probe-baseline` taken at the HEAD after the switch, on
  `PSX_SHORT_RUNS="draw-5564 draw-6000"`:
  - **D2 (`no-buttons`):** `button_draw` skips its row drawing at the two
    sites the report names, and `tooltip_draw_at` skips its rows.
  - **D3 (`no-message-box`):** `message_box_draw_overlay` draws nothing.
  - **D4 (`no-readouts`):** `print_outlined_entry` returns at once, which
    removes both the formatting and the print.
- Every figure is cited next to the unit-cost estimate and the report's
  figure:
  - D2: 313 / 49 a frame;
  - D3: 268 / 0;
  - D4: 80 / 79.

  All of these are at O2 with meter and filter, before D1/B4.

### Runtime: `p8_print_outlined` (the PICO-8 runtime's graphics module)

- **Interface:** `p8_print_outlined(gfx, mem, str, x, y, col)`, declared
  next to `p8_print`, forwarding to the active backend's new
  `print_outlined` entry. It is a runtime extension, like `p8_map`'s
  culling and step 8's image calls.
- **Contract on the declaration:** draws the same pixels as eight
  `p8_print` calls in colour 0 at the eight neighbour offsets, in the
  order (−1,−1), (0,−1), (1,−1), (−1,0), (1,0), (−1,1), (0,1), (1,1),
  followed by one `p8_print` in `col`.
  - `col` is stored as given, like `print`'s. The caller passes the low
    nibble.
  - The outline colour is always 0. A parameter for it was rejected: the
    game has no other outline colour, and the GPU path bakes outline = 0
    into the CLUT.
- **Draw-backend table:** one new entry, `print_outlined`, with `print`'s
  signature. The backend contract ("reads gfx and mem's draw state and
  never writes it") holds.
- **Software backend:** runs the nine passes through its own `print`, so
  the PC cost and pixels are unchanged. A faster direct rasterisation is
  allowed later, as long as the contract test holds. It is not done here.

### Game: `hud_print_outlined`

- It keeps its signature and its alignment computation (`a × 4 ×
  strlen`). It then makes one call, `p8_print_outlined(gfx, mem, t, ix,
  iy, c & 0x0F)`.
- The `outline_offsets` table leaves the game module. Its contents become
  the runtime contract above.
- Its callers (`probe.c`, `message_box.c`, `layout.c`,
  `draw_functions.c`) are unchanged.

### Pixel identity argument (why one sprite per glyph is exact)

- **Within a glyph.** The nine passes leave the colour on every lit pixel
  (the colour pass is last). They leave 0 on every unlit pixel with a lit
  8-neighbour, and nothing elsewhere. That is exactly the outlined cell:
  texel 1 lit, texel 2 outline, texel 0 transparent.
- **Between glyphs of one string.** Glyphs are 3 wide on a 4-pixel
  advance. A glyph's outline reaches one column past its cell, into the
  next glyph's gap column (its column −1). That column is never lit, so
  it can only hold outline (0) from either glyph.
  - The nine passes draw all outlines before any fill.
  - Per-glyph sprites interleave outline and fill.
  - The only pixels two glyphs share are 0 under both orders, so the
    order makes no difference.
- **Between strings.** Each `p8_print_outlined` call completes before the
  next, as each nine-pass block did. The draw order between strings is
  unchanged.
- **Clipping and the camera.** Each glyph sprite is placed at
  (cx − 1, y − 1), 5×7, then clipped like a plain glyph. A pixel off
  screen is off screen in both. The space, the only glyph with no lit
  row, emits nothing in either.
- **Colour.** On the GPU an outline texel reads font CLUT entry 2 =
  `psx_colour_rgb555(0)`. That is the value a colour-0 plain print writes
  through CLUT 0's entry 1, with the mask bit set, so black is opaque.

### PSX GPU backend

- **Font texture:** `psx_gpu_textures_font` also writes the outlined
  font into lines 48–127 of the 128×128 font texture. The plain font
  uses lines 0–47 (8 rows of 6-line cells).
  - Outlined cells are 5×7, 25 to a row, 11 rows (77 lines, v 48–124).
    Every character code has one, as in the plain font, and a
    compile-time check proves it fits.
  - `psx_gpu_textures_outlined_font_cell(character)` gives a cell's
    top-left texel.
  - Texel 1 is lit and texel 2 is an outline (an unlit texel with a lit
    8-neighbour).
  - The upload is unchanged: `upload_font` already sends the whole
    128×128 region once at boot. So there is no new VRAM, no new upload
    and no boot-time change.
- **Font CLUTs:** `psx_gpu_textures_font_clut(colour)` sets entry 2 to
  `psx_colour_rgb555(0)` in all 16. The plain font has no texel 2, so its
  pixels are unchanged. The test that froze entries 2–15 at 0x0000 is
  updated to expect entry 2 black and 3–15 transparent.
- **Placement helper:**
  - **Interface:** `psx_text_place_outlined(gfx, str, x, y, emit,
    context)`, next to `psx_text_place`.
  - It walks the string as `psx_text_place` does: same glyph-lit skip,
    same 4-pixel advance.
  - For each glyph it emits a 5×7 sprite at (cx − 1 − camera_x,
    y − 1 − camera_y) with the outlined cell's u and v, clipped with
    `psx_sprite_clip`.
  - It is pure and PC-tested.
- **`gpu_print_outlined`:** the font CLUT of `col & 0x0F`, then the helper
  emitting through the same textured-sprite path as `gpu_print`. That
  means texture window none, the font region and the one 4-bit page.
  - No `DR_TPAGE` change.
  - CLUT and pattern caches are unaffected (font CLUTs are fixed).
- **VRAM layout:** unchanged. The font region already holds both fonts.
  The layout test gains nothing. The texture test checks the outlined
  cells stay inside the region's 128 lines.

### Cost model, meter, filter, timing, call profile

- **Cost model:** `p8_draw_cost_print_outlined(window, gfx, str, x, y)`.
  - It costs one SPRT per non-space glyph, plus the window reset
    `unpatterned_window` charges, as `print` does.
  - The cost is 0 when there is no non-space glyph, or when the outlined
    box (x − 1 … x + (length − 1) × 4 + 3, y − 1 … y + 5) misses the
    screen.
  - That is an upper bound of what the GPU emits, since each emitted
    sprite is one non-space glyph whose clipped box is not empty.
- **Meter:** adds that cost and forwards. `zone gpu_frame` falls by about
  eight glyph SPRTs per outlined glyph on the heaviest frame. The new peak
  is recorded. `GPU_FRAME_BUFFER_BYTES` is not resized in this step.
- **Filter:** notes the call under `DRAW_FILTER_PRINT` with the outlined
  cost and forwards it when `print` is in the mask.
  - The nine prints were all under `print`, so every masked capture shows
    the same pixels.
  - No new family bit, no change to the capture list format or the
    references.
  - The `print` call counts fall (one call where there were nine) on PC
    and PSX alike.
- **Draw timing (breakdown boot) and the PC call profile:** both time and
  count the call under `print`, with units = the cost's SPRT count. This
  is the same rule on both sides, so the unit-cost join's call check
  holds. The family count does not change, so no table width changes.
- **Test doubles** that implement the backend table (`test_draw_backend`,
  `test_draw_meter`, `test_draw_filter`, `test_draw_timing`,
  `test_call_profile`, `test_state_format`) gain the entry.

### Measuring and verdict

- **Before (chunk 0):**
  - the `one-print` gross target, against 10.6, the unit-cost estimate
    and the report's figure;
  - the capture check.
- **Unit-cost estimate:** the print calls the switch removes per window
  frame × 1.514. The removed calls come from `make
  frame-profile-unit-cost` before and after: the print family's call
  count, before minus after. That count is 8 × the outlined prints that
  reach the screen. It is cited with the report's −542.3 / −102.1.
- **The lever's delta (chunk 3):**
  - Measured with `make psx-probe`, with chunk 3's diff as the patch,
    against `make psx-probe-baseline` at chunk 2's HEAD, on all six short
    runs. Measuring before the commit avoids step 1's reversed
    measurement.
  - The note gives each part's delta per run, draw per window frame on
    the two draw runs, and the asserts mode and opt level the runs
    reported.
- **No resume-frame rule needed:** the lever adds no one-off cost (no
  build, no upload). The note says so; there is nothing to add to phase
  4's spike list.
- **Keep or revert (plan rule):**
  - kept when the pooled draw delta of `draw-5564` and `draw-6000` is a
    gain;
  - otherwise the switch (chunk 3) is reverted and noted. The primitive
    and the GPU font (chunks 1–2) stay as a facility, as the plan keeps a
    step's facilities.
- **Pixel failure:** a differing capture or e2e hash goes to `fix-bug`.
  Only a difference shown to be inherent to one sprite per glyph ends the
  attempt. In that case chunk 3 is reverted, the note says D1/B4 cannot be
  made pixel-identical, and it cites the difference. The argument above
  says this cannot happen.
- **After the switch:** `make psx-short-runs` then `make
  frame-profile-unit-cost`, at the harness defaults (O3, asserts off, no
  meter, no filter). The new `print` per-call and per-glyph costs go in
  the note next to `be39841`'s. `representative` failures are recorded
  and do not void a figure. The other short-run conditions must hold.

## Implementation Chunks

0. **Gross target and capture check.** No source change.
   - Check H-8-1 to H-8-8 against step 8's note and HEAD, and record which
     hold.
   - `make psx-probe-baseline`, then the `one-print` probe on the six
     short runs. Record the gross target per run against 10.6, the
     unit-cost estimate (`be39841`'s print call counts × 8/9 × 1.514) and
     the report's figure. If covered, the step ends here, with the note.
   - **Capture check:** a throwaway PC patch draws the outline passes in
     colour 8 instead of 0. Run the PC frames of `psx/gpu_captures.txt`
     with and without it, and count the pixels that differ per capture.
     A capture with differing pixels shows a visible outline.
     - Expected: the `all` captures (100, 188, 265) show outlined labels
       over non-black pixels.
     - If none does, add a 0-tolerance capture on a `pad_slipway` frame
       whose PC frame shows an outline over a non-black pixel, with its
       reference, in its own commit. `psx-check` stays green.
   - Figures go in `progress.md`'s commit message, as step 8's chunk 2 did.
1. **Runtime primitive (PC).**
   - `p8_print_outlined`, the backend entry, the software backend's
     nine-pass implementation, the cost function, and forwarding in the
     meter, filter, timing and call profile, with every test double.
   - No game change. Unit tests through the public interfaces.
   - `make test-all` green, e2e hashes unchanged by construction.
2. **PSX outlined font.**
   - The outlined cells in the font texture, font CLUT entry 2, the
     placement helper, and `gpu_print_outlined`. PC unit tests for the
     texture, the CLUT and the helper.
   - `docs/psx-game.md`'s font paragraph says what is now true: two fonts
     in one texture, CLUT entry 2 black.
   - `psx-build` and `psx-check` green with every capture unchanged
     (nothing calls the primitive yet).
3. **The switch.**
   - `hud_print_outlined` calls `p8_print_outlined` once. `test_hud`
     gains its one-call check.
   - Before committing: `make psx-probe-baseline` at chunk 2's HEAD, and
     the lever probe with this chunk's diff.
   - `make test-all` (hashes unchanged), `psx-check` (every capture
     unchanged at 0 tolerance, `game-gpu-cost` within the PC's peaks),
     and `make state` for the new `zone gpu_frame` peak.
   - Keep or revert per the verdict, then commit with the figures in the
     message.
   - Then `make psx-short-runs` and `make frame-profile-unit-cost` for the
     new print unit costs.
4. **Residual D2, D3, D4.** No source change.
   - Under H-8-5: `make psx-probe-baseline` at HEAD on the two draw runs,
     then the three residual probes. Record each figure, the 10.6 verdict
     and the report's figure.
   - Figures go in `progress.md`'s commit message.

## Testing Decisions

- **A good test** drives a public interface (`p8_print_outlined`,
  `hud_print_outlined`, the cost function, the PSX pure helpers) and
  checks what comes out: screen pixels, metered counts, the calls a
  recording backend receives. It never inspects a backend's private
  state.
- **Runtime** (`test_gfx.c` or `test_draw_backend.c`, the existing
  style):
  - `p8_print_outlined` gives the same screen as the eight colour-0
    prints and the colour print, over:
    - every character code;
    - strings with spaces and placeholder glyphs;
    - each screen edge;
    - a camera sweep;
    - colour 0 text (outline and fill the same colour).
  - Text drawn over a non-black background shows the outline, which
    proves the test is not vacuous.
  - A recording backend receives one `print_outlined` call with the
    arguments given.
  - Under H-8-2: inside an image build the outlined print lands in the
    image.
- **Game** (`test_hud.c`):
  - `hud_print_outlined` issues exactly one `print_outlined` call, at the
    aligned position, in `c & 0x0F`.
  - The existing alignment tests keep passing unchanged.
- **Cost model** (`test_draw_cost.c`):
  - one SPRT per non-space glyph;
  - 0 for empty or all-space strings;
  - the outlined box reaches one pixel further than `print`'s on each
    side (a string at x = −7 that `print` costs 0 is charged when its
    outline column is on screen);
  - the pattern-window reset as for `print`.
- **Wrappers:** meter, filter, timing and call profile each gain a case:
  - bytes metered;
  - passed only when `print` is in the mask;
  - counted under `print` with SPRT units.

  Prior art: `test_draw_meter.c`, `test_draw_filter.c`,
  `test_draw_timing.c`, `test_call_profile.c`.
- **PSX pure helpers:**
  - **`test_psx_gpu_textures.c`:**
    - every code's outlined cell matches the software glyph rows (1 lit,
      2 outline, 0 elsewhere);
    - outlined cells do not overlap each other or the plain cells, and
      stay inside 128 lines;
    - font CLUT entry 2 is `psx_colour_rgb555(0)` (0x8000), entries 3–15
      are transparent;
    - the plain cells are unchanged.
  - **`test_psx_text.c`:** the paint double reads texel 2 as colour 0. The
    outlined placement then matches the software `p8_print_outlined`
    pixel for pixel, with the same cases as the plain-text tests (every
    code, the edges, the camera, 200 random strings and cameras). Its
    sprite count is bounded by the outlined cost.
- **The GPU path itself** cannot run on the PC. Its proof is `psx-check`'s
  captures at 0 tolerance after chunk 3, with chunk 0's check that a
  capture shows a visible outline.
- **`make test` and `make e2e` stay as fast:** the software backend does
  the same nine passes, and the new tests are of the size of the existing
  text tests.
- **No warn-review prerequisite.** Step 2.7's open warnings are about
  `asserts_enabled()`, the O3 default, the GPU replay's `#ifdef` stubs and
  the Makefile's size. This step does not extend any of that code, so
  under the plan's rule it adds no prerequisite test.

## Out of Scope

- Baking outlined strings into images (that is D2, D3 and D4, through step
  8's cache, in steps 13 and 15).
- B5, the lit-row table in `psx_text.c`: not separable, and not this
  lever.
- A faster software rasterisation of outlined text.
- Resizing `GPU_FRAME_BUFFER_BYTES` after the peak drops.
- An outline colour other than 0.
- Any change to the e2e hashes, the capture references, the capture list
  format or the filter's family bits.
- A judge run (step 16 ends phase 3), and running the raylib app.

## Further Notes

- **Step note contents:**
  - which H-8 hypotheses held;
  - D1/B4's gross target per run, against 10.6, the unit-cost estimate and
    the report's −542.3 / −102.1, with the coverage verdict;
  - the capture check (which capture shows a visible outline, or the
    capture added);
  - the lever's measured deltas per part and per run, draw per window
    frame on the two draw runs, against the gross target, the unit-cost
    estimate and the report;
  - the asserts mode and opt level read from the runs;
  - the `representative` outcome;
  - keep or revert;
  - the new `print` per-call and per-glyph costs;
  - `zone gpu_frame`'s new `make state` peak;
  - `ram-room`, `game-ram-room` and `gpu-ram-room` (no new VRAM);
  - D2, D3 and D4's residual figures with their verdicts, or "not
    measured" under H-8-5's failure branch;
  - "no one-off cost".
- **Expected outcome:** a draw gain of several hundred hblanks a frame on
  `draw-5564` and several tens on `draw-6000`, close to the gross target,
  since the colour pass the probe keeps costs about what the outlined
  sprites cost.
  - D4 is expected near the threshold. The report's 76-a-frame redraw is
    almost all outlined print, and after D1/B4 at 0.80 it falls to about
    7, plus formatting of about 3.
  - D2 and D3 are expected to stay above it, since they also hold
    rectangles, sprites and the message box's per-article shake.
- **Timing-model caveat** (report): a 5×7 sprite fills more texels than a
  3×5 one. Nine 3×5 sprites per glyph become one 5×7, so the GPU fill
  drops on real hardware too, but the judge charges none of it.
