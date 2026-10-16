---
title: "Planet disc and ring drawn from baked images (D8)"
description: "Every on-screen planet draws a filled circle (its black disc) and every colony an outline circle (its tier ring) each frame, the two dearest draw primitives left at about 3.1 and 3.5 hblanks a call; they should be copied from images baked once."
status: needs-triage
---

## Problem Statement

`planet_draw` draws, for every planet on screen, every frame:

1. `p8_circfill` radius 7, colour 0: the black disc that hides the
   background behind the planet;
2. the planet sprite (palette block, `palt`), a 2×2 `p8_spr`;
3. for a colony only: `p8_circ` radius 6 in its development tier's colour
   (`2, 3, 9, 12`), then the tier marker, a 1×1 `p8_spr`;
4. for the planet under the pad: `p8_circ` radius 9, colour 7.

Circles are now the dearest calls in the frame. Step 3.9's re-measured unit
costs (O3, asserts off, no meter, no filter): **circfill 3.137 a call, circ
3.454 a call**, against spr 0.448, rectfill 0.190, pset 0.129. They are the
families that kept most of their cost through phase 2.

The report (`docs/psx-perf-levers.md`, D8, "estimated") puts the disc and
ring at 924 hblanks a window on `draw-5564` (62 a frame) and 1,230 on
`draw-6000` (82 a frame), group `discs`, 4.74% / 8.79% of the runs' draw,
at the report's costs (circfill 3.139, circ 4.096). Both figures are far
above 2% of the mean frame (10.6 hblanks, step 2.7).

The shapes never change: the disc is always the same 15×15 pixels, a ring
is one of four 13×13 outlines. Only their position moves.

## Solution

The disc and the four rings are drawn once into step 8's off-screen image
cache, and each frame `planet_draw` copies them where it drew the circles:

- one 16×16 image slot holds the disc, built as `circfill(7, 7, 7, 0)`,
  copied at (x − 7, y − 7);
- four 16×16 slots hold the rings, one per entry of the ring colour table,
  built as `circ(6, 6, 6, colour)`, copied at (x − 6, y − 6);
- the planet sprite, the tier marker and the pad circle stay as they are,
  in the same order, so every pixel lands as it does today.

All five images are built together the first time a planet needs one, and
never again. When a fill pattern is in force at the disc or the ring, or the
ring colour is not in the table, `planet_draw` draws the primitive as
today, so the result is pixel-identical by construction.

The step opens with a throwaway probe that measures what the two calls cost
today, decides coverage on it, and is judged by a measured short-run delta,
unchanged e2e hashes and unchanged `psx-check` captures.

## User Stories

1. As the plan pilot, I want the gross cost of the disc and the ring measured on the six short runs before any code, so that coverage is decided on today's cost as the plan's rules require.
2. As the plan pilot, I want the disc's share and the ring's share measured apart, so that the note can attribute the gain and compare each with its unit-cost estimate.
3. As the plan pilot, I want the gross figures cited next to the report's figures (62 / 82 a frame) and next to an estimate from step 3.9's unit costs (circfill 3.137, circ 3.454, copy at spr's 0.448), so that the note follows the "cite both figures" rule.
4. As the plan pilot, I want the probe to leave the picture and the game state untouched, so that its runs are valid.
5. As the plan pilot, I want a written rule for "covered" (gross under 10.6 hblanks a window frame on both draw runs), so that the decision is mechanical and matches steps 8, 9 and 10.
6. As the plan pilot, I want it checked, before any code, that a `psx-check` capture shows a planet disc and a colony ring at 0 tolerance, so that the plan's "capture showing the cached element" rule holds before the copy replaces the circles.
7. As the plan pilot, I want every assumption about step 10's outcome written as a hypothesis `H-10-k`, so that a different outcome changes a known decision and nothing silently.
8. As a player on PC, I want planets, their discs, their rings and their tier markers to look exactly as before, so that the change is invisible.
9. As a player on PSX, I want the same picture as on PC, pixel for pixel, so that the platforms stay identical.
10. As a player, I want a planet half off the screen edge to be cut exactly as before, so that edges show no seam or missing pixel.
11. As a player, I want a planet under the pad to keep its white radius-9 circle on top of the ring, so that the pad cursor reads as before.
12. As a player, I want a vanishing planet's flicker to behave exactly as before, so that the visual random stream is consumed as today.
13. As a player, I want a planet drawn while a fill pattern is in force to look as it does today, so that no corner case changes a pixel.
14. As a game developer, I want `planet_draw` to keep its signature and its call order (disc, sprite, ring, marker, pad circle, row, targeting), so that every caller (draw pass, ascension) is untouched.
15. As a game developer, I want the five images built by one function the first time a planet needs them, so that the one-off cost lands on one frame and is easy to find.
16. As a game developer, I want the images never rebuilt once built, so that no frame after the first pays a build.
17. As a game developer, I want the build to read no game state and draw no visual random number, so that it obeys the image cache's draw contract.
18. As a game developer, I want a ring whose tier has no entry in the ring colour table to draw the primitive as today (colour 0), so that an out-of-table tier keeps today's output.
19. As a game developer, I want one game-side list of image slot numbers, the background's among them, so that two modules can never claim the same slot.
20. As a game developer, I want a compile-time check that the game's slot list and the runtime's slot count agree, so that adding a slot in one place and not the other fails the build.
21. As a maintainer of the runtime, I want the new slots added through the existing slot list, each 16 wide (the PSX FILL), so that the image cache needs no new API.
22. As a maintainer of the PSX backend, I want the new slots placed in the free (704, 256) half of the image region, each on a 16-unit bound, on one 15-bit texture page, so that they overlap nothing and the copy code works unchanged.
23. As a maintainer of the PSX backend, I want the layout tests to prove the new slots lie inside the image region, overlap no other slot, and sit on their page, so that a later slot cannot collide with them.
24. As a maintainer of the PSX backend, I want the copy placement tested for a slot on the second page, so that its texture coordinates are known right without an emulator.
25. As a maintainer of the PSX frame budget, I want `make state`'s `zone gpu_build` peak checked after the change, and `GPU_BUILD_BUFFER_BYTES` raised by its own sizing rule only if the peak needs it, so that a first frame building the map and the planet images together cannot overflow.
26. As a maintainer of `psx-check`'s masked captures, I want the ring image to hold only circ and the disc image only circfill, so that the draw filter passes or drops each copy exactly where it passed or dropped the circle it replaces.
27. As a maintainer of the e2e replays, I want every `*.sha256` and `make end-state` unchanged, so that the game is proven untouched.
28. As the maintainer of `psx-check`, I want every capture unchanged at 0 tolerance, so that PSX pixels are proven identical.
29. As the plan pilot, I want the lever's delta per part (update, draw, submit) on all six short runs, with asserts mode and opt level as the runs report them, so that the note has the measured gain against the estimate.
30. As the plan pilot, I want the lever judged on the draw runs without their resume frame when that frame carries the builds, so that the plan's one-off rule is applied.
31. As the plan pilot, I want the one-off build cost and the judge frame it lands on recorded, so that phase 4's spike list has it.
32. As the PSX RAM budget's owner, I want the static cost (five 256-byte software slots, about 1.3 KB in every ELF) recorded with `ram-room`, `game-ram-room` and `gpu-ram-room`, so that the cost is visible.
33. As a developer running `make test` and `make e2e`, I want them no slower, so that the fast loop stays fast.
34. As a maintainer, I want a unit test that draws planets with every ring tier, at the centre and across each screen edge, both ways (images and primitives) and compares the framebuffers, so that pixel identity on the software backend is proven without a replay.

## Hypotheses on step 10

Step 10 (`docs/prd/triage/phase3-step10-kept-layer-lists.md`, L1) is not
done while this PRD is written. Each hypothesis is checked when this step
starts, against step 10's note in `docs/plan-21.md` and the code at HEAD.
When one does not hold, the decision it names changes as stated.

- **H-10-1.** Step 10 is finished (committed, its note written) before
  this step starts, whether L1 was kept, reverted or skipped as covered.
  D8 shares no code with it. Nothing here waits on its outcome except the
  baseline: every probe of this step takes its baseline at the HEAD it
  runs on.
- **H-10-2.** Step 10 left every e2e hash, every end state and every
  `psx-check` capture unchanged. "Unchanged" here means unchanged against
  the HEAD this step starts from. If step 10 re-recorded anything (the plan
  forbids it before step 20), stop at chunk 0 and go to `fix-bug`.
- **H-10-3.** Step 10 did not touch `planet_draw`, the image cache
  (runtime, software and PSX backends, layout, draw filter, call profile),
  `hud_select_palette_block` or the ring colour table. Its layer setter
  touches the planet constructors only. If it touched one of these, chunk 0
  re-reads that code before the probe and rebuilds the probe patch from
  HEAD.
- **H-10-4.** Step 10 kept the draw order: planets are still dispatched by
  `draw_layers`, each through `planet_draw`, in the same order, with the
  same camera and the same graphics state at entry. This step relies on
  that only through the e2e hashes and captures, which check it anyway.
- **H-10-5.** If step 10 kept L1, its asserts forbid a draw method from
  registering, unregistering or re-layering an entity. The image build
  inside `planet_draw` does none of these, so it trips nothing. If an
  assert does fire, the build has done something it must not, and that is
  a bug of this step.
- **H-10-6.** The probe tooling (`make psx-probe-baseline`,
  `make psx-probe PATCH=… PSX_PROBE_LABEL=…`, `PSX_SHORT_RUNS`) and
  `make frame-profile-unit-cost` work as in steps 8–10: asserts off, O3, no
  meter, no filter.
- **H-10-7.** The latest unit-cost table is step 3.9's (circ 3.454,
  circfill 3.137, spr 0.448). Step 10 removes no draw call, so it has no
  reason to re-take it. If its note gives a newer table, that table
  replaces step 3.9's wherever this PRD cites one.
- **H-10-8.** The 2% threshold stays 10.6 hblanks (step 2.7's mean of
  530.2). No phase ends between steps 10 and 11.
- **H-10-9.** The short runs' `representative` condition still fails
  (draw moved more than 10% since step 7's judge run). As in earlier
  notes, the failure is recorded and does not void a figure.
- **H-10-10.** The latest `warn-review` is still step 2.7's (none runs
  before step 16). This step extends the image cache (step 8's code, not
  flagged) and `planet_display.c` (not flagged). It does not touch
  `psx_gpu_replay.c`'s `#ifdef` blocks, the condition counts or the
  Makefile's structure. So there is no prerequisite chunk (see "Flagged
  code"). If step 10's note records a newer `warn-review`, chunk 0 checks
  its list against the files this step touches.

## Implementation Decisions

### Coverage

- **D8 against the levers already taken:** F1, G1, B1, G2, B3, D5, D1/B4,
  and L1 if step 10 kept it (H-10-1). None removes or cheapens
  `planet_draw`'s circles: D5 is the map, D1/B4 the outlined text, L1 the
  dispatch walk, G2 made the circles cheaper but left them. D9 (step 15)
  culls planets that already return before the disc. The report pairs D8
  with no alternative. So D8 is expected not to be covered, and the
  measurement decides.
- **Gross target probes** (throwaway, uncommitted):
  - `disc-twice`: in `planet_draw`, the disc's `p8_circfill` is issued
    twice in a row. The second paints the same pixels over the first,
    before the sprite, so the picture and the game state are unchanged.
    The delta is the disc's cost.
  - `ring-twice`: the same for the ring's `p8_circ`, twice in a row before
    the marker. The delta is the ring's cost.
  - Each runs with `make psx-probe` on all six short runs, against one
    `make psx-probe-baseline` at the HEAD the step starts from.
  - Prior art: `psx/probes/call-twice-cart-draw.patch`. The frame buffer
    has room: the probe build doubles it, and these probes add far less.
- **Rule** (steps 8–10's, recorded as a decision): D8 is covered when
  disc + ring gross is under 10.6 hblanks a window frame on both
  `draw-5564` and `draw-6000`. If covered, the step ends at chunk 0 with
  its note. The disc and the ring are one lever: they are not judged
  apart, and a part whose gross is small still goes in with the other.
- **Figures cited next to the gross target:**
  - the report's: 924 / 1,230 a window, 62 / 82 a frame (`draw-5564` /
    `draw-6000`), at circfill 3.139, circ 4.096;
  - the unit-cost estimate: gross = 3.137 × discs + 3.454 × rings, net =
    gross − 0.448 × (discs + rings), the copy costing a spr call (the call
    profile files `image_draw` under spr, one unit and one page switch).
    The disc and ring counts per run come from the probes' own call
    counts, or from `make frame-profile` on the same windows. The note
    cites the table by step (3.9, or step 10's under H-10-7).

### Capture check (before any code)

- The plan requires a `psx-check` capture showing the cached element at 0
  tolerance before a lever draws through the image cache.
- Expected to hold already: `tests/e2e/pad_slipway.txt` puts colony p47
  under the screen centre at f98–f100, so captures `all 100`, `all 188` and
  `all 265` (mask `all`) should show at least one disc and one tier-0 ring
  (colour 2) with its marker. The `circ 314` (mask `cls,circ`) and
  `circfill 263` (mask `cls,line,circfill`) captures show the filtered
  views.
- Chunk 0 confirms it from the PC side: the PC dump of frame 100 (and 188,
  265) of `pad_slipway` shows ring pixels at radius 6 around a colony and
  black disc pixels over the map around a planet. The note names the
  captures that show them.
- If none shows a ring, a capture at a `pad_slipway` frame that shows a
  colony is added to `psx/gpu_captures.txt` at 0 tolerance (mask `all`),
  with its reference recorded, in its own commit, before chunk 1.
- No capture of a tier 1–3 ring is added. The four rings differ only in
  their build colour and go through the same build and copy path. The
  e2e replays cover the higher tiers on PC, and PC and PSX share the
  images' code. Recorded as a decision.

### What is baked, and what is not

- **Two images, not one.** The disc goes under the planet sprite and the
  ring over it. So the disc is one image and the ring another, with the
  sprite drawn between them as today.
- **The full bake was rejected.** The report's cost line ("pre-rendered
  planet sprites with disc and ring, per size and colour") would bake the
  planet sprite as well. Rejected because:
  - the trading hub's palette block animates (blocks 8–14 cycling, 15
    otherwise) and the slipgate's sprite follows its slipway count, so the
    variants are many and change at run time;
  - the image cache draws a whole slot, so an atlas would need a new
    sub-rectangle copy on every backend and wrapper;
  - the extra gain is one spr call and two palette selects a planet, about
    0.5 hblank, against an image per (sprite, palette block, tier).
- **The marker stays a sprite.** Baking it into the ring image would give
  that image the spr family. The `circ 314` capture's mask (`cls,circ`)
  would then drop the whole copy, ring included, where today it shows the
  ring: a capture would change. Kept separate, the ring image holds circ
  only and the disc image circfill only, so the draw filter passes or
  drops each copy exactly where it passed or dropped the circle.
- **The pad circle (radius 9, colour 7) stays a primitive.** It is drawn
  for one planet at most, at most 3.5 hblanks a frame, under 10.6.
- **The targeting circle** (`p8_circfill` with a transparent fill
  pattern and a variable radius) stays a primitive. It is not D8's.

### Why the images are pixel-identical

- `circ` and `circfill` ignore the draw palette on both backends: the
  software backend writes the colour as given, and the PSX backend matches
  it (every capture proves this today). So the images do not depend on the
  palette in force, and nothing else in `planet_draw`'s state reaches
  them.
- They depend on the fill pattern. The image cache bakes the pattern phase
  in image coordinates, which would differ from screen coordinates. So the
  images are built with no pattern, and used only when
  `gfx->fillp_pattern` is 0 at the disc or at the ring. Otherwise that
  call draws the primitive. This is the fallback that keeps identity by
  construction. Expected never taken in the replays; it is tested.
- The copy is clipped to the screen like the primitives' pixels, and drawn
  under the same camera. A disc or ring half off screen keeps exactly the
  on-screen pixels. Step 8 proved this for both backends; the unit test
  below proves it for these images on the software backend.
- Pixels the build never wrote stay transparent. The disc's black is
  written, so it stays opaque (the PSX mask bit, step 8).
- Ring colours are taken from the ring colour table by tier, as today.
  When the lookup fails (a tier outside the table), today's code draws
  colour 0, and so does the fallback: that case draws the primitive.

### Image cache: slots (runtime and PSX layout)

- The runtime's slot list gains five `SLOT(16, 16)` entries after the
  map's `SLOT(256, 256)`. No API changes: `p8_image_build` and
  `p8_image_draw` already serve any slot. The disc is built 15×15 and the
  rings 13×13 inside them.
- The software store grows by 5 × 256 bytes (about 1.3 KB in every ELF
  that links the software backend, the PSX ones included).
- PSX layout: the five slots go in the free (704, 256) half, as the
  layout's own comment plans: (704, 256), (720, 256), (736, 256),
  (752, 256), (768, 256), each 16×16. They share the 15-bit page
  (704, 256). The copy code (`psx_image_copy_place`, `gpu_image_draw`)
  is unchanged; each copy switches to the slot's page and back, as the
  map's does.
- The cost model, the meter, the draw filter, the call profile and the
  timing wrapper are already per slot and need no change beyond the
  larger count.

### Game: slot numbers

- One game-side enum lists every image slot: the background map's (0,
  today a `#define` in `background.c`), the disc, and the four rings. A
  compile-time check ties its count to `P8_IMAGE_SLOT_COUNT`.
- The ring slots are indexed by the ring colour table's index. A
  compile-time check ties their number to the table's length (4); the
  table is a `static const` array in `game_data.c`, so its length is
  exposed as a constant next to it if it is not already.

### Game: `planet_draw`

- A module-local function builds all five images, the first time any of
  them is needed, and records that they are built. It calls
  `p8_image_build` five times: the disc with `circfill(7, 7, 7, 0)`, each
  ring with `circ(6, 6, 6, colour)` for its table colour. Its draw
  callbacks read only the colour passed as context. It is called from
  `planet_draw` only, never from inside a build.
- At the disc: if `gfx->fillp_pattern` is 0, ensure the images are built
  and copy the disc slot at (x − 7, y − 7); otherwise `p8_circfill` as
  today.
- At the ring: if the tier's table lookup succeeds and the fill pattern is
  0, copy that tier's ring slot at (x − 6, y − 6); otherwise `p8_circ` as
  today, with today's colour.
- Everything else (the vanishing roll, the palette block, `palt`, the
  sprite, the marker, the pad circle, the row, the targeting circle) is
  unchanged and in the same order.
- **No rebuild key.** The background keys its image on the map version
  and the draw palette because both change its pixels. Nothing changes
  these images' pixels, so a built flag is enough. The flag is
  process-wide like the runtime's own `built[]` per slot. A unit test that
  needs a fresh state does not need one: the images are the same in every
  state.
- **Contract comment** on the build function: it draws no visual random
  number and reads no game state (the image cache's rule).

### GPU build buffer

- The first frame that draws a planet builds the five images. If it is
  also the frame that builds the map (galaxy generation, or a resumed
  short run's first frame), the builds share that frame's build buffer
  (step 8: map peak 8,800 of 10,240 bytes).
- After chunk 2, `make state`'s `zone gpu_build` peak is read. If the
  sizing rule in `gpu_sizes.h` (the peak + 1/8, rounded up to 1 KiB) gives
  more than 10,240, `GPU_BUILD_BUFFER_BYTES` is raised to that figure in
  the same chunk, and the RAM cost (twice the increase, one buffer per
  frame parity) goes in the note. Otherwise it stays.
- The images are not built earlier (for example at boot) to keep them off
  that frame: the frame they land on is recorded instead, as the plan's
  one-off rule asks.

### Flagged code

- The latest `warn-review` (step 2.7) flagged nothing this step extends
  (H-10-10). Step 8 already gave `psx_gpu_peaks` its test and derived the
  condition counts. So, by the plan's rule, there is no prerequisite test
  or derived figure.

### Measuring and verdict

- **Before (chunk 0):** the H-10 check, the capture check, the two gross
  probes, the coverage verdict.
- **The lever's delta (chunk 2):**
  - Measured with `make psx-probe`, using chunk 2's diff as the patch,
    against `make psx-probe-baseline` at chunk 1's HEAD, on all six short
    runs, before chunk 2 is committed (avoids step 1's reversed
    measurement).
  - Chunk 1 adds slots nobody draws; its delta is not separated.
- **Resume frame:** a resumed short run draws its first planets on its
  first window frame, so that frame carries the five builds. By the
  plan's rule, the lever is kept or reverted on the window without that
  frame. The one-off cost is that frame's draw delta minus the steady
  per-frame delta, recorded separately with the judge frame it lands on
  (the first frame that draws a planet in `full_game.json`: frame 12,
  galaxy generation, unless the title draws planets first; the note
  says which).
- **Keep or revert:** kept when the pooled `draw-5564` + `draw-6000` draw
  delta, without the resume frame, is a gain (the plan's rule on the
  target part). Otherwise chunk 2 is reverted, and chunk 1 with it: five
  slots nobody draws are no reusable facility, only RAM. The note says so.
- **Note contents:**
  - the H-10 outcomes;
  - the captures that show the disc and the ring;
  - the gross targets (disc, ring, total) per run against 10.6, the
    report's figures and the unit-cost estimate;
  - the measured delta per part per run, with asserts mode and opt level
    as reported, against the estimate;
  - the one-off build cost and its judge frame;
  - `representative` failures;
  - `make state`'s `zone gpu_build` and `zone gpu_frame` peaks before and
    after, and `GPU_BUILD_BUFFER_BYTES` if it moved;
  - `ram-room`, `game-ram-room` and `gpu-ram-room`;
  - e2e, `make end-state` and `psx-check` status.

## Implementation Chunks

0. **Gross target and capture check** (no committed source change).
   - Check H-10-1 to H-10-10 and record which hold.
   - Confirm from the PC dump that a capture shows a disc and a ring; if
     none shows a ring, add one (its own commit) as decided above.
   - `make psx-probe-baseline`, then `disc-twice` and `ring-twice` on the
     six short runs.
   - Record the figures against 10.6, the report and the unit-cost
     estimate. If covered, the step ends here with its note.
   - Figures go in the progress commit's message, as steps 8 and 9 did.
1. **Five small image slots** (no behaviour change).
   - Runtime slot list, PSX layout entries, the game-side slot enum (the
     background's slot moves into it) with its compile-time checks.
   - Tests: runtime (a 16×16 slot builds and copies like the primitives,
     clipped at every screen edge), PSX layout (inside the region, no
     overlap, 16-unit bound, page (704, 256)), PSX copy placement on the
     second page.
   - `make test-all` green; e2e hashes unchanged by construction.
2. **`planet_draw` copies the disc and the ring.**
   - The build-once function, the two copies with their fallbacks.
   - The pixel-identity unit test below.
   - `make test-all`, `make end-state` and `psx-check` green, every hash
     and capture unchanged; `make state` peaks read, build buffer resized
     only if its rule says so.
   - The `psx-probe` measurement of this diff, then keep or revert.
3. **Close.**
   - `make psx-short-runs` at the harness defaults, the RAM figures.
   - The step note in `docs/plan-21.md`.

## Testing Decisions

- **What makes a good test here:** the output is pixels. Tests draw
  through public calls (`planet_draw`, `p8_image_build`, `p8_image_draw`,
  `p8_circ`, `p8_circfill`) on the software backend and compare
  framebuffers. They do not read the module's built flag or the slot
  store.
- **Runtime (`tests/test_image.c`):** a 16×16 slot holding a circle, built
  and copied at the screen centre and across each of the four edges (and
  a corner), equals the same circle drawn directly on the same background.
  Prior art: `test_a_built_map_copies_like_the_map_drawn_directly`,
  `test_a_partial_image_clips_at_its_edge_and_leaves_the_rest_transparent`.
- **PSX layout and placement (`tests/test_psx_vram_layout.c`,
  `tests/test_psx_image.c`):** the new slots lie inside the image region,
  on 16-unit bounds, overlap neither each other nor the map slot, and
  their page is (704, 256); a copy from a second-page slot gets the right
  texture coordinates and clips at the screen edge. Run on PC, as today.
- **Game (`tests/test_planet.c`, or a new `tests/test_planet_draw.c` if
  `test_planet.c` lacks the drawing setup):**
  - **Pixel identity:** for a plain planet and a colony at each ring tier
    0–3, and for the pad-circled planet, `planet_draw` on a framebuffer
    holding a non-black background, at the centre and across each screen
    edge, gives the same framebuffer as a reference that draws today's
    sequence of primitives. The reference is written in the test from
    today's code (disc, palette, sprite, ring, marker, pad circle), so it
    stays a fixed oracle.
  - **Fallback:** with a fill pattern in force at entry, the output
    equals the reference with the same pattern (the primitives were
    used).
  - **No game state:** the game and visual random states are unchanged by
    the first (building) draw of a settled planet. Prior art:
    `test_vanishing_flicker_draws_from_the_visual_stream`.
- **Whole-game proof:** every `tests/e2e/*.sha256` unchanged, `make
  end-state` unchanged, `psx-check` green with every capture at 0
  tolerance (the PSX copy is proven only there).
- Any existing unit test that froze the old call sequence (for example a
  test counting `circfill` calls of a planet draw) is updated, since the
  e2e replays are the truth.

## Out of Scope

- Baking the planet sprite, the palette block or the tier marker
  (rejected above).
- The pad circle and the targeting circle.
- A sub-rectangle copy or an atlas in the image cache.
- D6 (planet row icons, step 12) and D9 (culling before dispatch, step 15),
  though both touch `planet_draw`.
- Other circles in the game (ascension, particles, UI).
- Running the judge (`psx-full-game-check`): step 16 does it.
- Any measurement on the Miyoo or PCSX-ReARMed; audio.

## Further Notes

- **Timing-model caveat** (the report's label on D8): the copy moves work
  from the CPU to the GPU (a 15×15 textured sprite in place of a
  rasterised circle). pcsx-redux charges no GPU-side time and no cache
  miss, so the gain is real on the judge. The plan accepts that.
- **Per-copy overhead on PSX:** each copy emits a texture page switch to
  the image page, the sprite, and a switch back (step 8's
  `gpu_image_draw`). That overhead is inside the measured delta; no
  batching of switches across planets is attempted here.
- **Code read at HEAD when this PRD was written:** the disc is
  `planet_draw`'s first draw call, after the vanishing roll; the ring and
  marker follow the sprite under `planet_is_colonized`; the ring colour
  table is `{2, 3, 9, 12}`; `circ`/`circfill` ignore the draw palette on
  the software backend; the PSX image region's lower half is empty; the
  map's slot is `0`, defined in `background.c`. Chunk 1 re-reads these at
  HEAD in case step 10 moved any (H-10-3).
