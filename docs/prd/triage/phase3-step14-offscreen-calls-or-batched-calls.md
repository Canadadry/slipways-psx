---
title: "Off-screen calls rejected early (B2), or slipway lines and particles batched (D11, D15)"
description: "Draw calls that land off screen, and the per-call overhead of the slipways' four-line bundles and the emitters' particles, may still cost the PSX frame enough to matter; the step measures what each alternative can still remove after the meter, the filter and the map culling are gone, implements the cheaper one if it clears 2% of the mean frame, and otherwise skips it with the figures."
status: needs-triage
---

## Problem Statement

Plan 21 step 14 asks for B2 ("reject off-screen calls at the first test"),
or D11 + D12 + D15 instead "if those are cheaper for the same calls". The
report (`docs/psx-perf-levers.md`) treats them as alternatives (group
`offscreen`): D11, D12 and D15 hold most of B2's off-screen `pset`, `line`
and `rectfill` calls.

The report priced them at O2, with the draw meter and the draw filter
compiled in, and with the map's off-screen cells still issued:

- **B2** (backend, step 8 candidate 2): an off-screen call went "all the way
  down": the meter's cost model, the filter's cost model, then the
  backend's placement and clip, to emit nothing. Off-screen calls outside
  the map, per 15-frame window: `pset` 394, `line` 68, `rectfill` 308,
  `spr` 135 on `draw-5564`; 1,916, 444, 660, 225 on `draw-6000`. Gain
  ≤ 373 (25 a frame) / ≤ 1,129 (75 a frame), "less the one test kept".
- **D11**: a slipway's four lines in one primitive. Gain 539 (36) / 756
  (50): the whole cost of the slipway lines at `line` 0.400 a call. The
  report itself rules the thick-line form out (it changes pixels); only a
  batched call that emits the same four lines keeps them.
- **D12**: one 2×2 sprite per transit dot, in place of a `rectfill` and a
  `pset`. Gain 351 (23) / 907 (60), less the sprite.
- **D15**: one runtime call per emitter's particles (a point list) in place
  of one `p8_pset` per particle. Gain 534 (36) / ≤ 724 (48).

Since then the ground moved under all four:

- F1 (step 1) compiled the filter out of the measured replays, B1 (step 4)
  dropped the meter from `psx-game` and the measured replays. In what the
  judge now runs, an off-screen call goes runtime → backend table → the
  backend's own clip (`psx_tiles_*`, `psx_sprite_place`,
  `psx_image_copy_place`), which rejects it at once. **The backend's clip
  is now the first test.** What B2 can still remove is the runtime's
  dispatch into the backend and that rejection, and it must pay its own
  test, on every call of its families, to do so.
- B3 (step 6) and D5 (step 8) removed the map's off-screen `spr` calls (two
  thirds of all off-screen `spr`).
- G2 (`-O3`) and the removals cut the per-call costs to 40–70% of the
  report's (step 3.8, re-measured in step 3.9): `pset` 0.129, `line`
  0.157, `rect` 0.314, `rectfill` 0.190, `spr` 0.448 a call.
- D11 and D15 as the report priced them counted whole calls. A batched
  call keeps every `LINE_F2` and `TILE_1` and every per-element clip; it
  removes only the per-call dispatch.
- Step 3.11 found that a copy or sprite writes each texel raw, bit 15
  included, and that an opaque sprite texel always carries bit 15 (black
  must, to stay opaque). An untextured `TILE` or `TILE_1` writes bit 15
  clear. D12's 2×2 sprite would therefore change the raw VRAM of every
  transit dot's pixels: invisible, but it moves raw capture references,
  and accepting references is the human's command only.

So the gains in the report no longer say whether any of these is worth
doing. At step 3.9's unit costs, loose upper bounds (each off-screen call
priced as an average call of its family, which also pays the primitive an
off-screen call never emits):

- B2: (394 × 0.129 + 68 × 0.157 + 308 × 0.190 + 135 × 0.448) / 15 =
  **12.0** a frame on `draw-5564`; (1,916 × 0.129 + 444 × 0.157 + 660 ×
  0.190 + 225 × 0.448) / 15 = **36.2** on `draw-6000`.
- D11, whole lines (1,348 and 1,890 lines a window, from the report's
  figures at 0.400): 14.1 / 19.8 a frame; of which a batch can remove only
  the dispatch of three lines in four.
- D15, whole `pset` calls (about 1,050 and 1,420 a window): 9.0 / 12.2 a
  frame; of which a point list can remove only the per-particle dispatch.

The threshold is 2% of the mean frame, **10.6 hblanks** (step 2.7). The
realistic figures are below the upper bounds, possibly under 10.6. Only a
measurement on today's build settles it.

## Solution

The step opens with three throwaway, pixel-safe probes that measure, on
the six short runs, exactly what each lever can remove at most:

- **`offscreen-twice`** (B2's target): inside the PSX backend, every call
  the backend rejects whole is re-entered once through the backend table.
  The delta is the dispatch plus the rejection of every off-screen call:
  B2's ceiling.
- **`slipway-line-extra`** (D11's target): each slipway line is preceded by
  one extra `p8_line` that lies wholly off screen. The delta is one
  dispatched-and-rejected line call per slipway line; three quarters of it
  is D11's ceiling.
- **`particle-extra`** (D15's target): each particle `p8_pset` is preceded
  by one extra `p8_pset` off screen. The delta is one dispatched-and-
  rejected call per particle: D15's ceiling.

D12 is skipped without a probe: no sprite form of it keeps the raw VRAM
(bit 15) of the dot's pixels, and an untextured form is not D12 (two
primitives stay two).

Then:

- A lever whose ceiling is under 10.6 on both draw runs is **covered**: B2
  by B1 + F1 + B3 (the meter, the filter and the map's off-screen cells
  were what made an off-screen call expensive), D11 and D15 as under 2% of
  the mean frame against what a batch can remove.
- Of the alternatives still standing, B2 on one side and the surviving
  members of {D11, D15} on the other, the one with the larger pooled
  ceiling is **cheaper** (it leaves the cheaper frame) and is implemented.
- It is kept if the measured short-run delta on the draw runs is a gain,
  reverted otherwise, in which case the other alternative, if it stood, is
  tried next.
- If nothing stands, the step ends after the probes with its note, and no
  source changes.

Whatever is implemented emits exactly the primitives the game emits today,
in the same order (B2 emits nothing for calls that drew nothing): pixel
identical and raw-VRAM identical by construction, proven by unit tests,
unchanged e2e hashes, unchanged `make end-state`, unchanged `make state`
zones, and every `psx-check` capture reference unchanged.

## User Stories

1. As the plan pilot, I want B2's remaining target measured as it stands after F1, B1, B3 and D5, so that coverage is decided on today's cost and not on the report's O2-with-meter figure.
2. As the plan pilot, I want D11's and D15's targets measured as what a batch can remove (the per-call dispatch), not as the whole calls, so that a lever is not credited with work it keeps.
3. As the plan pilot, I want each probe to leave the picture and the game state untouched, so that its runs are valid.
4. As the plan pilot, I want each probe's figure cited next to the report's and next to the estimate from step 3.9's unit costs, so that the note follows the "cite both figures" rule.
5. As the plan pilot, I want a written, mechanical rule for "covered" (ceiling under 10.6 a window frame on both draw runs), so that the decision matches steps 8 to 12.
6. As the plan pilot, I want a written, mechanical rule for "cheaper" (larger pooled ceiling on the two draw runs), so that the choice between B2 and D11 + D15 needs no human.
7. As the plan pilot, I want D12's skip recorded with its reason (bit 15 on sprite texels, step 3.11), so that nobody retries it without the human accepting references.
8. As the plan pilot, I want every assumption about step 12's outcome stated as a hypothesis and checked against its note and HEAD before chunk 0, so that no decision rests on an outcome that did not happen.
9. As the plan pilot, I want the per-family off-screen call counts and the slipway-line, particle and emitter counts read from the PC replay of the same windows, so that the ceilings can be turned into per-call figures.
10. As the plan pilot, I want the step to end after the probes, with no source change, when nothing clears 10.6, so that no effort goes into a lever below the plan's threshold.
11. As the plan pilot, I want the chosen lever measured with `make psx-probe` on its own diff, on all six short runs, so that the keep-or-revert decision follows the plan's rule.
12. As the plan pilot, I want the other alternative tried if the chosen one measures no gain, so that the step does not end on a revert when another lever stood.
13. As a player on PC, I want slipways, transit dots and particles to look exactly as before, so that the change is invisible.
14. As a player on PSX, I want the same pixels as on PC, raw bit for raw bit, so that the platforms stay identical and the capture references hold.
15. As a player, I want a slipway or a particle half off the screen edge cut exactly as before, so that edges show no seam.
16. As a game developer (B2), I want an off-screen call rejected in the runtime before the backend table, by a test that can never reject a call that draws a pixel, so that correctness never depends on the backend.
17. As a game developer (B2), I want calls with a degenerate size passed through to the backend as today, so that the runtime test only ever rejects provably empty calls.
18. As a game developer (D11), I want one runtime call that draws a list of segments in one colour, in order, so that a slipway's four lines cost one dispatch.
19. As a game developer (D11), I want `slipway_draw_line` to issue that call with the same four segments in the same order, so that its pixels are those of today's loop.
20. As a game developer (D15), I want one runtime call that draws a list of coloured points, in order, so that an emitter's particles cost one dispatch per batch.
21. As a game developer (D15), I want `emitter_update` to collect its points in a fixed local buffer, flushed when full and after the loop, so that no zone memory is needed and nothing is drawn between points out of order.
22. As a maintainer of the runtime, I want each new backend entry documented with its contract (order, colour, camera, what a NULL or zero count does), so that a third backend can implement it.
23. As a maintainer of the meter, I want a batch metered as the sum of its elements' costs, so that `make state`'s byte zones and caps are unchanged.
24. As a maintainer of `psx-check`'s masked captures, I want a batch filed under its element's family (`line` or `pset`) by the draw filter, so that `line 276 cls,line` and `pset 131 cls,pset` pass or drop exactly the pixels they did.
25. As a maintainer of the frame profile, I want the call profile (PC) and the draw timing (PSX) to count a batch as one call per element of its family, so that the per-call tables stay comparable and the PC/PSX join still agrees.
26. As a maintainer of the image cache, I want the image guard to forward every new entry, so that a batch inside a build works like its single calls.
27. As a maintainer of the e2e replays, I want every `*.sha256` and `make end-state` unchanged, so that the game is proven untouched.
28. As the human who alone accepts capture references, I want every capture reference unchanged at the raw hash, and the lever reverted if one moves, so that this step never needs a reference I have not accepted.
29. As a developer running `make test` and `make e2e`, I want them no slower, so that the fast loop stays fast.
30. As a maintainer, I want a unit test sweeping calls across every screen edge and camera that compares the runtime's off-screen test with the software backend's framebuffer and with the PSX placement functions, so that B2's exactness is proven without an emulator.
31. As a maintainer, I want unit tests that draw a batch and the same elements one by one on the software backend and compare framebuffers, so that D11's and D15's pixel identity is proven on PC.
32. As a maintainer, I want unit tests on the PSX placement side (compiled on PC) showing a batch emits the same primitives as the single calls, so that the PSX path is proven before `psx-check`.
33. As a maintainer, I want every wrapper backend (meter, filter, timing, call profile, image guard) tested to forward or count the new entries, so that no table holds a NULL entry.
34. As the PSX RAM budget's owner, I want `ram-room`, `game-ram-room` and `gpu-ram-room` recorded after the step, so that a text growth is visible.
35. As the plan pilot, I want the note to record, for every lever not implemented, which rule skipped it and with what figure, so that step 16 and the human see why the `offscreen` group delivered what it did.

## Step 12's outcome (hypotheses, checked before chunk 0)

Step 12 (D6, planet row icons through the image cache) is being
implemented when this PRD is written; step 13 (D4) is expected to be
skipped, covered by step 9 (step 3.9's note). Each assumption is a
hypothesis; chunk 0 checks every one against `docs/plan-21.md` and HEAD
and records its outcome in the note. If one fails, the decision it names
is adapted as said, and nothing else changes.

- **H-12-1.** Steps 12 and 13 are finished and their notes (`### Step
  3.12`, `### Step 3.13`) are written at the HEAD this step starts from.
  Step 13 changed no source. All probes take their baseline at that HEAD
  (`make psx-probe-baseline`; the fingerprint refuses a stale one).
- **H-12-2.** D6 was kept or reverted; either way this PRD holds. If kept,
  planet row sprites became `image_draw` calls (filed under `spr` by the
  call profile and the timing): `offscreen-twice` re-enters rejected copies
  too, and B2's runtime test covers `p8_image_draw` with the built size
  of the slot. If reverted, nothing changes here.
- **H-12-3.** Step 12 gives no newer per-family unit-cost table: step 3.9's
  (`pset` 0.129, `line` 0.157, `rect` 0.314, `rectfill` 0.190, `spr`
  0.448) is the one cited. If step 3.12 gives one, it is cited instead.
- **H-12-4.** No `psx-full-game-check` ran since step 7: the threshold
  stays **10.6 hblanks**.
- **H-12-5.** The draw backend table still has its 14 entries (`cls` …
  `print_outlined`, then the four image entries), step 12's PRD adding
  none. If an entry was added, D11's and D15's entries go after it, and
  every wrapper table lists every entry.
- **H-12-6.** E2e hashes, `make end-state` and every capture reference at
  HEAD are those step 12 kept unchanged (its hard requirement): the
  baselines this step compares against are valid.
- **H-12-7.** `slipway_draw_line`, `slipway_draw`, `emitter_update`, the
  runtime's family calls, `psx_tiles_*`, `psx_sprite_place` and the PSX
  backend's family functions are unchanged by step 12 (it touches
  `planet_draw`, a row-image module, the image slots, the PSX VRAM
  layout and the background's slot). If one changed, chunk 0 re-reads it
  before writing a probe.
- **H-12-8.** No `warn-review` ran since step 2.7's: no prerequisite chunk.
  This step adds entries to the wrapper backends and to the PSX backend;
  none of these is step 2.7's flagged code (the replay's `#ifdef` stub
  blocks are not touched).
- **H-12-9.** The probe tooling (`make psx-probe`, `make
  psx-probe-baseline`, `psx/probes/`) works as in steps 8 to 12;
  `representative` still fails on every short run, recorded, not a
  blocker.

## Implementation Decisions

### What each lever's "target" is today

Recorded as a decision because the report's figures meant something else:

- **B2's target** is the cost of every call the backend rejects whole:
  the runtime's dispatch (`p8_<family>` → backend table) and the
  backend's own rejection. The meter's and the filter's work on those
  calls, which the report counted, is gone (B1, F1); the map's off-screen
  cells are gone (B3, D5).
- **D11's target** is the dispatch of three of every four slipway lines.
  The four `LINE_F2` and their clips stay: the report's single-primitive
  form (a thick line) changes pixels and is ruled out by the report itself.
- **D15's target** is the dispatch of every particle `pset` but one per
  batch. The `TILE_1` and the clip of each point stay.
- **D12's target** is not measured: the lever is skipped on references
  (below).

### Probes (chunk 0, throwaway, uncommitted)

Each probe is a patch run with `make psx-probe` on all six short runs,
against one `make psx-probe-baseline` at the HEAD the step starts from.
None draws a pixel, moves the game state or draws a visual random number.
Prior art: `psx/probes/call-twice-cart-draw.patch`.

- **`offscreen-twice`.** In the PSX backend, each of `gpu_pset`,
  `gpu_line`, `gpu_rect`, `gpu_rectfill`, `gpu_spr` and `gpu_image_draw`,
  when its placement or clip rejects the whole call, calls its own entry
  once more through `psx_gpu_backend()`, guarded by a static re-entry flag
  so the second call does not recurse. The delta is B2's ceiling (dispatch
  from the backend table plus the rejection; the runtime wrapper's own
  call, a few instructions, is not counted: an accepted underestimate).
  `circ`, `circfill` and `print` are left out, as in the report (their
  rejection is per point, span or glyph, not whole-call).
- **`slipway-line-extra`.** In `slipway_draw_line`, before each `p8_line`,
  one extra `p8_line` whose both ends lie 64 pixels left of the camera
  (`camera_x − 64`), so the backend rejects it. The delta is one off-screen
  `line` call per slipway line; **D11's ceiling is 3/4 of it**.
- **`particle-extra`.** In `emitter_update`, before each particle's
  `p8_pset`, one extra `p8_pset` at `camera_x − 64`. The delta is one
  off-screen `pset` call per particle; **D15's ceiling is that delta times
  (1 − emitters ÷ particles)**, the counts from the PC replay (below); if
  the count is not at hand, the whole delta is used (an upper bound).
- **Counts** (host-only, PC replay of the same windows, nothing tracked
  written): per family, calls and units (calls − units = off-screen calls)
  from the call profile, as the report counted them; slipway lines drawn,
  particles drawn and emitters drawing, per window. Used to give each
  ceiling a per-call figure and to apply D15's factor.

### Rules: covered, cheaper, keep

- **Covered:** a lever whose ceiling is under 10.6 hblanks a window frame
  on both `draw-5564` and `draw-6000` is skipped (steps 8 to 12's rule).
  For B2 the note says "covered by B1 + F1 + B3"; for D11 and D15,
  "under 2% of the mean frame against what a batch removes". The
  one-frame update runs are recorded, not used.
- **D12: skipped, not probed.** An opaque sprite texel carries bit 15
  (`psx_colour_rgb555` sets it on every non-transparent CLUT entry, and
  black needs it to stay opaque), while the `rectfill` and `pset` it
  replaces write bit 15 clear. Any capture showing a transit dot would
  change its raw reference, and accepting references is the human's
  command only (step 3.11). An untextured one-call form still emits two
  primitives (a 2×2 `TILE` and a `TILE_1`), so it is not D12 and not in
  the report: out of scope by the plan's rules.
- **Cheaper:** of the two alternatives still standing, B2 on one side, the
  sum of the standing members of {D11, D15} on the other, the one with the
  larger ceiling, summed over `draw-5564` and `draw-6000` (a window frame
  each), is implemented. Ties within 10% go to B2 (one change in the
  runtime, no new backend entry).
- **Keep:** measured with `make psx-probe` on the lever's diff against a
  baseline at the HEAD before it, all six runs. Kept when the pooled
  `draw-5564` + `draw-6000` draw delta a frame is a gain (no one-off
  cost: no resume-frame exclusion). Otherwise reverted, and the other
  alternative, if it stood, is implemented and judged the same way.
  D11 and D15 are separate levers: each is measured on its own diff and
  kept or reverted on its own.
- **References:** any change of a `psx-check` capture's raw reference
  reverts the lever whatever its gain. This step never runs `make
  psx-gpu-capture-accept` and never stages a reference file.

### B2 (if chosen): the runtime rejects provably empty calls

- `p8_pset`, `p8_line`, `p8_rect`, `p8_rectfill`, `p8_spr` and
  `p8_image_draw` test the call's integer bounding box after the camera
  against the screen (0..127 on both axes) before reaching the backend
  table, and return when the box lies wholly outside. The box is the one
  both backends clip: the point; the two end points' min and max; the
  corners; `x .. x + 8w − 1` by `y .. y + 8h − 1`; the slot's built
  size.
- **Only provably empty calls are rejected.** A call with a zero or
  negative size, or anything the test cannot classify, goes to the
  backend as today. The test lives once in the runtime as a small static
  function per box shape; the backends keep their own clipping (they still
  need it for partial calls).
- **During an image build** the runtime test is unchanged: a build draws
  through screen-sized windows at a moved camera (`image_window`'s
  contract), so a call off the screen-sized window is empty in the build
  too.
- **What the wrappers see:** a rejected call reaches no wrapper. The meter
  charged it 0 bytes (`make state` unchanged); the filter passed or
  dropped nothing visible (captures unchanged); the call profile and the
  draw timing stop counting it, on PC and PSX alike, so the PC/PSX join of
  `make frame-profile-unit-cost` still agrees. The note says the per-call
  table's definition moved (off-screen calls no longer counted). A unit
  test that froze "an off-screen call is 1 call and 0 units" through the
  runtime is updated to call the wrapper directly or to expect no call, by
  CLAUDE.md's rule.
- `circ`, `circfill` and `print` keep today's path (as in the report).

### D11 (if chosen and standing): one call for a slipway's line bundle

- **Runtime:** a new call drawing a list of segments in one colour, in list
  order, each exactly as `p8_line` draws it (world coordinates, camera
  applied by the backend). A zero count draws nothing. The segment type is
  a small struct of four ints.
- **Backend entry:** `lines`, appended to the table (after the image
  entries, or after any entry step 12 added). Software backend: its line
  rasteriser per segment, in order. PSX backend: per segment
  `psx_tiles_line` and one `LINE_F2`, the colour's RGB looked up once, the
  length assert kept per segment.
- **Game:** `slipway_draw_line` fills a local buffer of four segments from
  its two loops, in the same `dx`-outer, `dy`-inner order, flushes it with
  the new call when full, and once more after the loops if anything is
  left. Its two callers give 4 segments each (border, then line colour), so
  each issues one call; any other `l`, `h`, `s` still works.
- **Wrappers:** the meter sums `p8_draw_cost_line` over the segments
  (bytes identical); the filter treats the batch as the `line` family,
  passing or dropping it whole (each segment would have had the same
  verdict); the draw timing times the batch once and adds one `line` call
  per segment; the call profile adds one `line` call and its units per
  segment; the image guard forwards it.

### D15 (if chosen and standing): one call per emitter's particle batch

- **Runtime:** a new call drawing a list of points, each with its own
  colour, in list order, each exactly as `p8_pset` draws it. A zero count
  draws nothing. The point type is three ints.
- **Backend entry:** `psets`, after `lines` if D11 is in, otherwise after
  the image entries. Software backend: its `pset` per point. PSX backend:
  per point `psx_tiles_pset` and one `TILE_1`.
- **Game:** `emitter_update` appends each particle's point (when
  `g_pixels_hidden` is clear, exactly where it calls `p8_pset` today) to a
  local buffer of **32** points, flushes it with the new call when full,
  and once after the particle loop, before `emitter_spawn`. Nothing is
  drawn between two particles of one emitter (the loop only moves, ages
  and releases particles), so the order of pixels is today's. No zone
  memory (rejected: the report's "point list in the frame zone"; a fixed
  stack buffer bounds memory and needs no zone sizing). 32 is a decision:
  large enough that most emitters flush once, small enough for the stack
  (384 bytes).
- **Wrappers:** as D11, with the `pset` family and `p8_draw_cost_pset` per
  point.
- Transit dots' `pset` (in `slipway_draw`) stay single calls: they
  alternate with a `rectfill` per dot, and they are D12's, which is
  skipped.

### What is not changed

- No other caller of `p8_line` or `p8_pset` is moved to the batch calls.
- The cost model, the GPU frame and build buffers, the VRAM layout and the
  image cache are unchanged.
- Line numbers in the report (`slipway_display.c:83`, `particle.c:190`)
  are the report's; the functions named here are what counts.

## Implementation Chunks

0. **Probes, counts, decision** (no committed source change).
   - Check H-12-1 to H-12-9 and record their outcomes.
   - `make psx-probe-baseline` at the starting HEAD; `offscreen-twice`,
     `slipway-line-extra`, `particle-extra` on the six short runs.
   - The PC counts.
   - The figures per run against 10.6, the report's and the unit-cost
     estimates above; D12's skip; the covered / cheaper verdict.
   - If nothing stands, the step goes to chunk 4 directly.
   - Figures go in the progress commit's message, as steps 8 to 12 did.
1. **B2** (only if chosen).
   - The runtime test for the six calls; the unit tests below; any unit
     test that froze the old counting updated.
   - `make test-all`, `make end-state`, `make state` (zones unchanged) and
     `psx-check` green, every hash and capture reference unchanged.
   - `make psx-probe` on this diff; keep or revert.
2. **D11** (only if chosen and standing, or as the fallback after a B2
   revert).
   - Runtime call, `lines` entry in both backends and in every wrapper,
     `slipway_draw_line` switched; the tests below.
   - Same gates and measurement as chunk 1.
3. **D15** (same conditions as chunk 2, independent of it).
   - Runtime call, `psets` entry in both backends and in every wrapper,
     `emitter_update` switched; the tests below.
   - Same gates and measurement as chunk 1.
4. **Close.**
   - If a lever was kept: `make psx-short-runs` at the harness defaults,
     `ram-room`, `game-ram-room` and `gpu-ram-room`.
   - The step note in `docs/plan-21.md`.

## Testing Decisions

- **What makes a good test here:** the output is pixels and, on PSX, the
  primitives emitted. Tests draw through the public runtime calls on the
  software backend and compare framebuffers; on the PSX side they call the
  placement functions compiled on PC (`psx_tiles_*`, `psx_sprite_place`,
  `psx_image_copy_place`), as `tests/test_psx_tiles.c` and
  `tests/test_psx_image.c` do. Calls reaching the backend are observed by a
  recording backend installed with `p8_draw_backend_set`, as
  `tests/test_draw_backend.c` does. No test reads a wrapper's internals.
- **B2 (`tests/test_draw_backend.c`, `tests/test_gfx.c`):**
  - A sweep over positions and cameras around every screen edge and corner,
    for each of the six calls: a call the runtime rejects reaches no
    backend entry, leaves the software framebuffer unchanged, and is
    rejected by the matching PSX placement function; a call it passes
    reaches the backend once with its arguments. Prior art: step 2.6's
    camera sweep of `p8_map` against the cost model
    (`test_map_gives_the_old_loops_on_screen_calls_at_every_edge`).
  - Degenerate sizes (`spr` with `w` or `h` ≤ 0) reach the backend.
  - An image build's windowed calls behave as before (prior art:
    `test_a_build_frames_its_windowed_calls_and_a_copy_reaches_its_entry`).
- **D11 / D15 (`tests/test_gfx.c`, `tests/test_draw_backend.c`,
  `tests/test_psx_tiles.c`):**
  - A batch and the same elements drawn one by one give the same
    framebuffer on the software backend, at every edge, with overlapping
    elements and a non-zero camera; a zero count draws nothing.
  - The batch reaches its own entry once (prior art:
    `test_each_draw_call_reaches_its_own_entry_once`).
  - The PSX side: for a batch, the sequence of accepted placements equals
    that of the single calls.
- **Wrappers (`tests/test_draw_meter.c`, `tests/test_draw_filter.c`,
  `tests/test_draw_timing.c`, `tests/test_call_profile.c`,
  `tests/test_image.c`):** the meter's bytes for a batch equal the sum of
  the single calls'; the filter passes a batch under its family's mask and
  drops it otherwise; timing and profile count one family call per
  element; the image guard forwards the batch.
- **Game (`tests/test_slipway.c`; a particle test next to
  `tests/test_particle_zone.c`'s prior art):** a slipway drawn through
  `slipway_draw` and an emitter through `emitter_update` give the same
  framebuffer as today's single calls drawn by hand from the same state,
  and the game and visual random states are unchanged.
- **Whole-game proof:** every `tests/e2e/*.sha256`, `make end-state`
  and `make state`'s zones unchanged; `psx-check` green with every capture
  at 0 tolerance and every raw reference unchanged (the PSX path is proven
  only there; `line 276`, `pset 131` and the `all` captures show slipways
  and particles).
- No test is added if chunk 0 skips every lever.

## Out of Scope

- D12 in any form (skipped on references; an untextured variant is not in
  the report).
- Batching other callers of `p8_line`, `p8_pset` or `p8_rectfill`
  (tooltips, HUD, transit dots, circles).
- B2 for `circ`, `circfill` and `print`.
- D10 (slipway end points computed once), D13 (planet visibility reused in
  `slipway_draw`), D9: step 15.
- Doing both alternatives: the report and the plan treat them as one
  choice.
- Running `psx-full-game-check`: step 16 does it.
- Accepting capture references; measuring on the Miyoo or PCSX-ReARMed;
  audio.

## Further Notes

- **Expected outcome, not a decision:** at step 3.9's unit costs, B2's
  loose upper bound is 12.0 / 36.2 a frame, and an off-screen call costs a
  fraction of an average one (it emits nothing); D11's and D15's ceilings
  are a fraction of 14.1 / 19.8 and 9.0 / 12.2. Several, possibly all,
  may fall under 10.6. The probes decide; a skip with figures is a valid
  end of this step.
- **Why B2 is not "free":** with the meter and the filter gone, the
  backend's clip is the first and only test; B2 moves it up by one call
  and pays it again, in the runtime, on every on-screen call of its
  families. Its measured delta, not its ceiling, decides whether it is
  kept.
- **Timing-model caveat:** these levers remove instructions and calls
  only (the report's unlabelled kind), so the judge's flat 2 cycles an
  instruction applies as to any code change.
- **Code read at HEAD `4938bc5`** when this PRD was written (files equal to
  HEAD in the working tree): `slipway_draw` draws two `slipway_draw_line`
  calls of four lines each (offsets {−1, 2}² in the border colour, then
  {0, 1}² in the line colour), then per transit dot a 2×2 `rectfill` in
  black and a `pset` in the dot colour; `emitter_update` draws one
  `p8_pset` per living particle, nothing drawn in between, and returns
  early for an off-screen emitter below layer 10 only; the runtime's
  `p8_<family>` calls go straight to the backend table; the PSX backend
  rejects a wholly off-screen `pset`, `line`, `rect`, `rectfill`, `spr` or
  image copy in its placement function before emitting anything; the five
  wrapper backends (meter, filter, timing, call profile, image guard) list
  all 14 entries positionally.
