---
title: "Lever N3: PSX circle outline drawn as runs"
description: "On PSX a circle outline costs about 20 hblanks a call because every pixel is its own TILE_1 packet, behind a 28-comparison duplicate check and a clip per pixel. Wormhole rings make that a large part of the frames over 263 in map-move. The outline should go out as one tile per run of pixels, with the same pixels, the same picture and every e2e hash unchanged."
status: needs-triage
---

## Plan step (verbatim)

Plan 22, phase 3, step 7 (`docs/plan-22.md`, section "3. Phases and steps",
as committed at `55eb2dc`):

> 7. Each lever of step 5's ranked list, in its order, one step note each:
>    measured gain against its estimate, pixel-identical captures, unchanged
>    e2e hashes. After each lever, its measured gain replaces its estimate
>    in `docs/psx-levers.md` and, once two levers of the same estimation
>    basis (`calls` or `share`) are measured, the median of their
>    measured-to-estimated ratios scales the remaining estimates of that
>    basis before re-ranking (the ratio and the levers it rests on are in
>    the note); the projected steady frames still over are
>    recomputed (step 5's calculation) and the remaining levers re-ranked
>    if the order changes. Whenever a `psx-full-game-check` replaces
>    `psx/perf/full-run-frames.txt`, the projection is rebased on that
>    table: every lever already in the measured code is marked `in-base` in
>    the lever table (kept for the record, no gain applied), and the note
>    gives the measured steady frames over next to the last projection,
>    before and after the rebase. When the projection reaches zero steady frames
>    over, `psx-full-game-check` confirms `real-game-budget` green before the
>    phase stops early. (plan-pilot: one PRD per lever, or per group of
>    small levers that touch the same code.) Each lever's PRD lists the
>    open warn-review warnings (phases 1 and 2) that fall in the files it
>    changes, and decides each one: acted on in that PRD, or left with its
>    reason. The step note records which. A lever measured on
>    `psx-full-game-check` is first probed (`psx-probe`) on a run that
>    draws its target function — a representative run of another situation
>    if one does, else its best-try run. If that probe shows no gain on the
>    target function, the lever is reverted with no full-game check.
>    Consecutive levers of this kind in rank order then share one
>    `psx-full-game-check`, at most three per check, the last group sharing
>    step 8's. Each situation's mean update and draw are compared with the
>    previous full-game table (`make situation-report` on both). A lever's
>    measured gain is that per-situation delta, split between the group's
>    levers in proportion to their probe gains. If a group shows no gain in
>    the situation its levers target, it is split and re-measured one lever
>    at a time.

This PRD covers **only lever N3**, "PSX circle outline drawn as runs" in
`docs/psx-levers.md` (rank 3 at `55eb2dc`, after N1 and N2). D3 and N1 are
done (step notes `### Step 3.7 — D3` and `### Step 3.7 — N1`). N2 is being
implemented while this PRD is written. Every assumption about N2's outcome
is a hypothesis `H-N2-k`, listed in "Hypotheses on N2's outcome" and checked
before N3 starts. G4, N5, N6 and N7 get their own PRDs.

N3 is the **second `calls`-basis lever to be measured**. D3 is the first,
at a ratio of 0.42; N1 and N2 are `share`. So, per step 7, once N3 is
measured, the median of D3's and N3's measured-to-estimated ratios scales
the remaining `calls` estimates before the re-rank. This PRD writes that
first `scale calls` line.

## Problem Statement

On PSX, `circ` (a circle outline) goes through `gpu_circ` →
`psx_tiles_circ`. That function runs the software backend's midpoint loop.
At each step it builds the 8 symmetric offsets and, for each one:

- checks it against every earlier offset of the same step (up to 28
  comparisons a step);
- clips it as a one-pixel box;
- calls the emit callback, which builds one 12-byte `TILE_1` packet.

A radius-24 ring is 17 steps and 128 pixels, so 128 packets and 128 trips
through the callback and the packet allocator.

The steady profiles measure this at **20.158 hblanks a call** on
`map-move-1057`, whose window draws a wormhole's reveal rings every frame
(4.7 `circ` a frame). `wormhole_draw_method` is 95.8 h a window frame there,
94.1 h of it inside `circ`: 27% of the run's frame. In the full game, 62 of
map-move's 388 frames over 263 draw a circle (294 calls in all). Map-rest,
menu and score-panel draw a few more: the pad cursor's ring (radius 9),
planet rings drawn without their image, and particle trail rings.
`docs/psx-levers.md` sizes N3 on the full game's circle count, not on the
window: map-move 9.2 h (4.1% of draw), map-rest 0.8%, menu 0.5%,
score-panel 1.8%. That removes about 50 steady frames over 263 by itself,
48 of them in map-move.

For the player, the map stutters while a wormhole opens and the camera
moves, and a big part of that frame is the console sending circle pixels
one at a time.

## Solution

`psx_tiles_circ` emits the same pixels as tiles, one per run instead of
one per pixel:

- **Vertical runs:** each column a circle crosses in its steep parts (the
  octants where `|dx| >= |dy|`) gets one 1×h tile per run of pixels.
- **Horizontal runs:** each row in its flat parts (`|dx| < |dy|`) gets one
  w×1 tile per run.

Runs that meet on an axis merge into one tile. Every pixel falls in
exactly one class, so no duplicate check is left. Each run is clipped once.

`gpu_circ` sends a lone pixel as a `TILE_1`, as today, and a longer run as
a `TILE`. The pixel set, the colour and the mask-bit behaviour do not
change, so the picture is identical. Only the number of packets and the
CPU work per circle go down: a radius-24 ring goes from 128 packets to 64,
a radius-9 ring from 44 to 28.

Before the change, `psx-check` gains a capture of a circle clipped by the
screen edge, accepted on today's code. The change is then measured on the
short runs with `make psx-probe`, against a probe baseline taken just
before it, on `map-move-1057`:

- its measured gain replaces the estimate in `docs/psx-levers.md`;
- the median of D3's and N3's ratios becomes the `scale calls` line;
- the projection is recomputed, and the remaining levers are re-ranked if
  the order changes.

If N3 does not save hblanks, it is reverted and noted.

The player sees the same game, the same pixels, and fewer dropped frames
while wormholes open.

## User Stories

1. As a player moving the camera while a wormhole opens, I want its rings drawn faster, so that map-move frames fit 263 hblanks more often.
2. As a player watching a wormhole's reveal at rest, I want the rings to cost less, so that the map at rest stays at 60 fps.
3. As a player with the pad cursor on a planet, I want its ring cheap, so that the cursor ring does not push a frame over.
4. As a player whose planets show their development ring, I want the ring's image built from the same pixels as before, so that rings look the same.
5. As a player, I want every circle's pixels exactly as today, including where a circle crosses the screen edge, so that nothing visible changes in pass 1.
6. As a player on PC, I want the PC build untouched, so that both platforms stay identical.
7. As a player, I want the game itself unchanged, so that my game plays out exactly as before.
8. As the plan pilot, I want N3 checked against "covered" and "under 2%" on its remaining cost, measured, so that the plan's rule is followed.
9. As the plan pilot, I want that remaining cost measured with a throwaway probe that draws every circle twice, so that the verdict rests on today's code, not on step 4's commit.
10. As the plan pilot, I want the "under 2%" verdict judged by step 5's two rules, so that it uses the same rules as the ranking.
11. As the plan pilot, I want a capture of a circle clipped by the screen edge added and accepted on the code before N3, so that the clipped runs are proven on the GPU, not just argued.
12. As the plan pilot, I want the existing `circ 316` capture and the `all` captures to keep matching their references, so that the whole circle, the wormhole rings and the planet ring images are proven pixel-identical.
13. As the plan pilot, I want a fresh probe baseline at the commit just before N3, so that the measured delta compares against the code N3 starts from.
14. As the plan pilot, I want N3 measured on `map-move-1057`, its "measured by" run, so that the lever table's contract holds.
15. As the plan pilot, I want the measured-to-estimated ratio defined before measuring, against the window's own estimate, so that the window's wormhole bias does not inflate or deflate the ratio.
16. As the plan pilot, I want each situation's gain set to its estimate × N3's ratio, so that the full-game sizing (not the window's) carries into the projection.
17. As the plan pilot, I want `map-rest-5581` and the other runs reported as weaker evidence, so that the projection has a cross-check.
18. As the plan pilot, I want `game-state same` on every probed run, so that a PSX-only change is proven not to touch the game.
19. As the plan pilot, I want N3 reverted when it is not a gain, so that the plan's revert rule holds.
20. As the plan pilot, I want the measured gain set against its estimate in the step note, so that a lever under its estimate is noted and the plan goes on.
21. As the plan pilot, I want N3's line switched to `measured` in the lever table, so that `make lever-estimate` uses the measurement.
22. As the plan pilot, I want the `scale calls` line written from the median of D3's and N3's ratios, so that N7, the last `calls` lever, is projected on measured evidence.
23. As the plan pilot, I want the levers and ratios the scale rests on named in the note, so that the scale can be traced.
24. As the plan pilot, I want the projection recomputed and the remaining levers re-ranked if the order changes, so that the next lever is the right one.
25. As the plan pilot, I want no `psx-full-game-check` for N3, so that the phase's full-game check stays at step 8.
26. As the plan pilot, I want the open warn-review warnings in N3's files listed and decided, so that the amended step 7 is followed.
27. As the plan pilot, I want `psx-check` green with every capture of both lists matching its reference, so that pixel identity on PSX is observed.
28. As the plan pilot, I want every e2e `.sha256` and `.end-state` unchanged, so that the PC picture and game are proven unchanged.
29. As the plan pilot, I want the PSX text size and RAM room reported, so that a size change shows.
30. As a developer, I want `psx_tiles_circ` tested against the software `circ` pixel for pixel over many radii, centres, cameras and screen edges, so that a wrong run boundary is caught on the host.
31. As a developer, I want the tests to fail if two emitted tiles overlap, so that no pixel is sent twice.
32. As a developer, I want the tile counts of a radius-9 and a radius-24 circle pinned, so that a regression to per-pixel emission is caught.
33. As a developer, I want the real packet bytes of a circle still under its metered cost, so that the draw meter's upper bound and the GPU buffer sizes stay valid.
34. As a developer, I want `psx_tiles_circ`'s header contract to describe the runs, so that a caller knows a tile can be wider or taller than one pixel.
35. As a developer, I want `make test` and `make e2e` no slower beyond noise, so that the fast loop stays fast.
36. As a reviewer, I want the step note to say what N3 measured, against what, the scale it sets, and why it was kept or reverted, so that the decision can be traced.

## Implementation Decisions

### Checks before the change (plan rules)

- **Covered?** No lever taken or in progress touches the PSX circle path.
  D3 changed message-box text, N1 slipway line end points, and N2 inlines
  fixed-point and vector helpers (H-N2-6). `psx_tiles_circ` works in
  plain `int`, and its cost is in-primitives time (94.1 of 95.8 h). N3 is
  not covered.
- **Under 2%?** The remaining cost is **measured** with a throwaway probe,
  as D3 and N1 did:
  - the patch makes `gpu_circ` run `psx_tiles_circ` twice with the same
    callback, so every circle is sent twice (same pixels, same picture);
  - it is run with `make psx-probe` on `map-move-1057` and `map-rest-5581`
    against the baseline below;
  - the residual is the window draw delta a window frame, and per call
    divided by the window's 4.7 `circ` a frame (map-move-1057).
  It is judged by step 5's rules:
  - **rule (b):** on `map-move-1057`, the residual is above 2% of that
    run's measured total (352.0 → 7.0 h);
  - **rule (a)**, reported: the per-call residual times the full game's
    circle calls per frame over in each situation (the estimate's own
    basis: map-move 294 / 388, map-rest 246 / 429, menu 10 / 91,
    score-panel 3 / 8), weighted by steady frames as in step 5, against
    T = 4.4 h. It is on the frames-over basis, so it is an upper bound,
    stated as such.
  - Either rule holds → go on. Neither → N3 is not taken ("under 2%"),
    never committed, and noted with its figures.
  - Step 4 measured 94.1 h a window frame in `circ`, so a skip is not
    expected.
- **Fresh probe baseline:** `make psx-probe-baseline` (all runs) after
  chunk 1's commit. Chunk 1 changes `psx/`, whose tree id is in the
  fingerprint, so any earlier baseline is stale. The note says when it was
  taken. Every run must be valid.

### The change (chunk 2)

- **Modules:** the PSX backend's `platform/psx_tiles` (pure, host-tested)
  and its caller in `platform/psx_gpu` only. No `p8/`, `engine/`, `game/`
  or devtools file changes.
- **`psx_tiles_circ` keeps its signature:** `gfx`, centre, radius, a
  `psx_tiles_emit_t` callback and its context.
  - Its contract becomes: it emits, through the callback, non-overlapping
    tiles that are 1 pixel wide or 1 pixel high, clipped to the screen,
    whose union is exactly the software `circ`'s on-screen pixels.
  - It returns how many tiles it emitted.
  - The header comment says this, and also that a 1×1 tile is a lone
    pixel.
- **Partition rule** (a decision of this PRD; it replaces the duplicate
  check). For a midpoint step `(x, y)`, the 8 symmetric pixels split into
  two classes:
  - **Column class:** the pixels with `|dx| >= |dy|`, i.e. `(±x, ±y)`.
    The diagonal pixel of a step with `x == y` is in this class.
  - **Row class:** the pixels with `|dx| < |dy|`, i.e. `(±y, ±x)` with
    `y < x`.
  - Consecutive steps that share `x` form a group, with `y` from `y0` to
    `y1`. For each group:
    - **Column class:** the columns `sx + x` and `sx − x` (only one column
      when `x == 0`, i.e. radius 0) each get the rows `sy + y0 .. sy + y1`
      and `sy − y1 .. sy − y0`. When `y0 == 0`, those two ranges merge into
      one run, `sy − y1 .. sy + y1`.
    - **Row class:** the rows `sy + x` and `sy − x` each get the columns
      `sx + y0 .. sx + y1'` and `sx − y1' .. sx − y0`, where `y1'` is `y1`
      cut below `x`. They merge the same way when `y0 == 0`. A group whose
      row-class range is empty emits nothing in that class.
  - Each run is clipped once, by the existing clip helper. A run that is
    fully off screen emits nothing.
  - The circle's box check before the loop stays.
- **Same loop:** the midpoint loop (`midpoint_start` / `midpoint_advance`)
  is unchanged. Its "change all three together" coupling with
  `gfx_software.c` and `draw_cost.c` still holds, and neither of those
  files changes.
- **`gpu_circ`'s callback:** a tile of 1×1 becomes a `TILE_1`, as today.
  Any other tile becomes a `TILE` of its width and height. Both use the
  colour already resolved for the call. `gpu_pset` and `gpu_psets` keep
  their own one-pixel path.
- **Byte bound:** a `TILE` is 16 bytes and stands for at least 2 pixels,
  which `p8_draw_cost_circ` meters at 12 bytes each. A `TILE_1` is metered
  as itself. So the real bytes stay under the metered cost, and
  `draw_cost.c`, the draw meter, the GPU buffer sizes, the build-buffer
  bound and the `worst-case` peak tables do not change. Tightening the
  meter is out of scope.
- **Draw order:** the order of pixels inside one `circ` call changes. That
  cannot change the picture: one colour, opaque, no semi-transparency, no
  overlap. The mask-bit state (`DR_STP` during image builds) applies to
  `TILE` exactly as to `TILE_1`.
- **Expected packets:**
  - radius 24: 128 → 64; radius 9: 44 → 28; radius 26: 140 → 68;
  - radius 0 to 4: unchanged (every run is one pixel long, so every tile
    is still a `TILE_1`).
  The lever's "about 60% fewer packets" is closer to 50% on the wormhole's
  radii and 36% on the pad ring. N3's ratio will show it.
- **No comment** is added beyond the header contract (comment rules).

### The clipped-circle capture (chunk 1)

What `psx-check` covers today for `circ`, from `pad_slipway`'s calls at
`55eb2dc` (PC replay):

- **`circ 316`** (mask `cls,circ`): the pad ring, radius 9, at (79, 60),
  whole. It has runs of 1, 2 and 3 pixels in both classes.
- **`all 235`:** the wormhole's five reveal rings, radius 24, colour 1, at
  (77, 37), whole.
- **`all 267`:** a pad ring, radius 9, whole.
- **`all 102`, `all 190`, `all 235`, `all 267`:** these copy the planet
  ring image (slot 66, built at f14 by `circ` radius 6 inside an image
  window with the mask bit on). So the image-build path is covered.
- **Not covered:**
  - **A circle clipped by the screen edge.** `pad_slipway` draws one at
    f245–257 and f273–283 (the pad ring cut by the top or bottom edge), but
    no capture lands there. Clipping a run is new code: today each pixel is
    clipped alone.
  - **Radii 0 to 2** (f43–46, f197–200). They emit the same `TILE_1`
    packets as today, so there is no GPU-side change, and the unit tests
    cover them. No capture is added for them.
  - **A circle clipped by the left or right edge.** No frame of the check
    replay draws one. It is the same `TILE` with a smaller width, so it has
    the same GPU semantics as the top-clipped capture below, and the unit
    tests cover its clip arithmetic. No new recording is made for it: a new
    e2e recording is out of proportion for a host-proven clip.

Decision: **add `circ 245 cls,circ` to `psx/gpu_captures.txt`**, between
`spr 236` and `pattern 259`. At f245 the pad ring (radius 9, colour 7) sits
at (52, −3), cut by the top edge: rows 0 to 6 show, so its column runs are
clipped mid-run and its lower row runs are whole.

- f245 is neither a 32nd frame nor the last, so it is not a `DIGEST=`
  sample. It is not next to the `display` entry, and its family's call is
  on screen, which `gpu-capture-list` requires.
- It is accepted **on the code before N3**, as D3 did for `pad_message_box`
  (agent rule, `docs/psx-game.md` § Acceptance):
  1. add the line and run `make psx-check`;
  2. run `make psx-gpu-capture-ceilings LABEL=circ` from that run;
  3. run `make psx-check` again, green;
  4. run `make psx-gpu-capture-accept LABEL=circ`.
  This writes the ceiling and reference lines of both `circ` captures (316
  unchanged).
- The new capture must be at `differ 0`. If it is not, the mismatch
  predates N3: N3 stops, and the mismatch goes to `fix-bug` as its own
  bug.
- If the extra capture pushes the GPU replay boot past `PSX_GPU_CEILING`
  vsyncs or `PSX_GPU_TIMEOUT` seconds, raise that limit by the capture's
  measured cost, and explain it in the commit.
- `docs/psx-game.md`'s "Capture list" bullet gains one sentence: the
  `circ` label has two captures, f316 a whole ring and f245 a ring cut by
  the top edge.
- Chunk 1 is committed on its own, and it **stays even if N3 is reverted**:
  it checks today's clipped circles too.

### Measurement and bookkeeping

- **Order of operations:**
  1. Chunk 1: commit the capture.
  2. Take the probe baseline.
  3. Run the residual probe and record the covered / under-2% verdict.
  4. Write the tests (red on today's per-pixel emission where they pin
     tile counts), then make the change.
  5. Measure N3 **before** committing it: its diff against `HEAD` is the
     `psx-probe` patch (it touches only `src/` and `tests/`).
  6. Commit only if N3 is kept.
- **Probe:** `make psx-probe PATCH=<N3 diff>` on every short run (all 12).
  Every run must be valid, with `game-state same`.
- **Estimate the ratio is set against** (fixed here, before measuring):
  - the lever entry's window figure for `map-move-1057`: 4.7 `circ` a
    frame × 20.158 h × 0.6 = **56.8 h a window frame**;
  - N3's ratio = (− window draw delta a frame on `map-move-1057`) ÷ 56.8.
  - It is a per-call ratio. The window draws more circles than map-move's
    frames over do, but the same rings, so the ratio carries over to the
    full-game sizing.
- **Measured gains:** every N3 `gain` line becomes its estimate × N3's
  ratio: map-move 4.1%, map-rest 0.8%, menu 0.5%, score-panel 1.8% (draw).
  The window delta itself is never booked as map-move's gain: the window
  overstates map-move's circles (`docs/psx-levers.md`, Known biases).
- **Weaker evidence, reported only:**
  - `map-rest-5581`: about 1 `circ` a frame at 5.716 h; its delta against
    0.6 × 5.716 = 3.4 h, and its own ratio. It is under its 3 h noise floor
    at most, so it sets nothing.
  - Every other run's deltas: no circle in the window, expected around 0.
- **Revert rule:** N3 is kept when all of these hold:
  - `map-move-1057`'s window draw delta and total delta are negative;
  - no run's total delta is positive beyond its noise floor (step 4's
    `max(3 h, 1% of the window)` a frame);
  - every run's `game-state` is `same`.
  Otherwise it is reverted (never committed), and every run's deltas go in
  the note.
- **Lever table** (`docs/psx-levers.md`):
  - N3's line becomes `measured`, with its four scaled gains.
  - N3's entry gets: the residual, the measured figures per run, the
    ratio, the packet counts above, the under-2% verdict, the text and RAM
    figures, and the new capture.
  - The "What each function gets" row for `wormhole_draw_method` stays.
- **The `scale calls` line** (the table's first `calls` scale):
  - Its value is the median of D3's 0.42 and N3's ratio, i.e. their mean,
    since there are two levers.
  - A reverted N3's ratio still counts (it was measured), floored at 0. So
    a reverted or losing N3 gives 0.21.
  - It applies only to `estimated` `calls` levers, which is **N7** alone
    (D3 and N3 are measured). N7's entry says "scaled by <ratio> (D3, N3)".
  - `lever_estimate.sh` refuses a second `scale` line for one basis, or a
    ratio ≤ 0. The value here is always ≥ 0.21, so the line is always
    written.
  - The note gives the ratio, D3's and N3's own ratios, and that it rests
    on those two levers.
- **Projection:** run `make lever-estimate` on the committed tables after
  the table edit. Paste its block in place of the current projection,
  together with `order ok`, or the new order with the ranked list
  re-ordered and renumbered to match. Update the closing paragraph's
  figures, and `docs/psx-game.md`'s "Still to do" line on the lever list
  (its projection end). The note gives the steady frames over after N3,
  nominal and pessimistic, against the projection N2 left (H-N2-4).
- **`docs/psx-game.md`, backend paragraph** (if N3 is kept): "`circ` is
  one `TILE_1` per on-screen pixel" becomes "one `TILE` per run of pixels
  on a row or column, a `TILE_1` for a lone pixel". The rest of the
  sentence (from the midpoint loop, host-tested pixel for pixel) stays.
- **No `psx-full-game-check`** in this step: N3 is measured on short runs,
  and step 8 is the phase's full-game check. No rebase, no `in-base` mark
  (H-N2-7). Probe-first and grouping apply only to full-game-measured
  levers.
- **`psx-check`:** green, with every capture of both lists matching its
  reference, the new `circ 245` included. After chunk 1, no reference or
  ceiling changes: the GPU draws the same pixels, so the raw VRAM is the
  same. `gpu-cost` and `game-gpu-cost` real bytes go down and stay under
  the metered cost. `make raylib` still links (it does not compile the PSX
  backend; this is only a sanity check).
- **Fast loop:** `make test` and `make e2e` wall times before and after,
  in the note. The new unit tests are bounded (Testing Decisions), and
  neither target may be slower beyond run-to-run noise.
- **Step note** (`### Step 3.7 — N3` in `docs/plan-22.md`, after N2's,
  H-N2-2), giving:
  - the capture added and its acceptance;
  - the baseline;
  - the residual and the covered / under-2% verdict;
  - the probe deltas per run (draw, update, submit, total, `game-state`,
    `text`);
  - measured vs estimated on `map-move-1057`, and the ratio;
  - kept or reverted, and why;
  - the `scale calls` value and what it rests on;
  - the steady frames over before and after (nominal / pessimistic) and
    any re-rank;
  - `ram-room` / `game-ram-room`;
  - the fast-loop timings;
  - the warn-review decisions below;
  - any deviation.

### Open warn-review warnings in N3's files

N3 changes these files:

- `src/platform/psx_tiles.c`, `src/platform/psx_tiles.h`,
  `src/platform/psx_gpu.c` and `tests/test_psx_tiles.c`;
- `psx/gpu_captures.txt`, `psx/gpu_capture_ceilings.txt` and
  `psx/gpu_capture_references.txt` (possibly the Makefile's
  `PSX_GPU_CEILING` / `PSX_GPU_TIMEOUT`);
- `docs/psx-levers.md`, `docs/plan-22.md` (step note) and
  `docs/psx-game.md` (backend paragraph, Capture list bullet, "Still to do"
  figure).

The open warnings are the end-of-phase notes of `docs/plan-22.md`, plus
plan 21's, which plan 22 still governs:

- **Phase 1 (13):**
  - the table join and the situation lists duplicated across five scripts
    and the Makefile;
  - the inline 263 / 31 / 15 / 3;
  - two variables for one table;
  - 55 untested recipe lines in `steady-profile`;
  - `end_status != NULL` next to its bit in `situation.c`;
  - `pad_author.c` at 1,540 lines.
- **Phase 2 (13):**
  - the display mask as a `0x8000` sentinel in the family bitmask,
    repeated as `8000` in the Lua and inline in two awk scripts;
  - `psx_gpu_replay.c` reading a protocol value from the host parser;
  - `cart.c` holding a whole `button_t` as the difficulty carrier, copying
    its fields one by one, and duplicating `cart_update60`'s prologue and
    epilogue;
  - "one frame late" vs F−2 wording;
  - `loading_screen.c` in `ENGINE_SRCS`;
  - e2e frame numbers inline everywhere;
  - `pad_author.c` at 1,549 lines with no unit test.
- **Plan 21's:**
  - duplicated `asserts_enabled()`;
  - the O3 default in five places;
  - hand-copied condition counts;
  - `#ifdef` stub blocks in `psx_gpu_replay.c`;
  - no unit test for `psx_gpu_peaks`;
  - a 3,000-line Makefile.

Decisions:

- **The "one frame late" vs F−2 wording** (phase 2) is the only warning in
  a file N3 changes (`docs/psx-game.md`). It is **left open**: it sits in
  the "Display capture" bullet and the loading-screen text, and N3 edits
  neither. N3 edits only the backend paragraph's `circ` sentence, one
  sentence of the "Capture list" bullet and the "Still to do" figure. This
  follows N1's precedent, and step 9 carries it to "Still to do".
- **The Makefile warnings** (phase 1's lists, constants and table
  variables, and plan 21's 3,000-line Makefile) are **left open**. N3
  touches the Makefile only if the capture forces a vsync or timeout
  limit up, and that would be one value on an existing line, unrelated to
  any of these warnings.
- **"Hand-copied condition counts"** (plan 21) is **left open**. The added
  capture changes `gpu-captures`' printed count, which nothing copies
  (`test_psx_capture_check` uses its own fixture). It does not change
  `psx-check`'s number of conditions, so no copied count goes stale.
- **Every other warning: left open**, because it is not in a file N3
  changes. `psx_gpu_replay.c`, `psx_gpu_peaks`, `capture_list.c`, the Lua
  and the awk scripts are not touched; `psx/gpu_captures.txt` names masks
  by family, not by the `0x8000` sentinel.

## Implementation Chunks

1. **Clipped-circle capture.** Add `circ 245 cls,circ` to
   `psx/gpu_captures.txt` and accept it at `differ 0` on today's code
   (ceilings, then references, for label `circ`). Add the "Capture list"
   sentence in `docs/psx-game.md`. `psx-check` must be green. Commit. It
   stays even if N3 is reverted.
2. **N3 itself.** Take the probe baseline and run the residual probe, then
   record the verdict. The tests in `test_psx_tiles` come first, then:
   - `psx_tiles_circ` emits runs by the partition rule;
   - its header contract is updated;
   - `gpu_circ`'s callback picks `TILE_1` or `TILE`.
   Measure with `psx-probe PATCH=`. Keep and commit, or revert. Then run
   `psx-check` and `make test-all`.
3. **Bookkeeping.** Update `docs/psx-levers.md`: the N3 entry and line,
   `scale calls`, N7's entry, and the projection and re-rank. Update
   `docs/psx-game.md`: the backend sentence if kept, and the "Still to do"
   figure. Add the step note to `docs/plan-22.md`. If N3 is reverted, this
   chunk still runs: N3's line becomes `measured` with zero gains, the
   scale is written, and the note records the revert.

## Testing Decisions

- **What a good test does:** it calls `psx_tiles_circ` through its public
  interface and compares what it emits with the software backend's `circ`
  on a `p8_vram_t`, pixel for pixel. It never depends on the order of
  tiles or on how runs are built inside.
- **`test_psx_tiles`, adapted:**
  - The collecting callback accepts tiles that are 1 wide or 1 high (no
    longer only 1×1).
  - `paint` keeps asserting that no two tiles overlap and that each tile
    is on screen.
  - The pixel set must equal the software screen.
  - The byte check counts each 1×1 tile at `P8_GPU_TILE_1_BYTES` and each
    other tile at `P8_GPU_TILE_BYTES`, and must stay ≤
    `p8_draw_cost_circ(...).bytes`.
  - The existing cases (radius 0 → 1 tile, radius 1 → 4, radius −1 → 0,
    the corner clip, the arcs-only case, the box miss) keep their values:
    these radii have no run longer than one pixel, or are checked only as
    "> 0".
  - The test's name drops "one pixel each".
- **`test_psx_tiles`, new cases:**
  - **Pinned tile counts** for whole circles at (80, 60): radius 9 → 28,
    radius 24 → 64, radius 26 → 68, and radii 2, 3, 4 equal to their pixel
    counts (8, 12, 16 tiles). These fail on today's per-pixel emission
    (red first) and pin the lever's packet cut.
  - **Random circles**, the same shape as
    `test_random_boxes_draw_as_the_software_does`:
    - 200 seeded cases;
    - centre from −60 to 220 by −60 to 180, radius 0 to 70, and a random
      camera;
    - each compared pixel for pixel with the software `circ`, no overlap,
      bytes under the meter.
    This covers clipping by each of the four edges and the corners, runs
    clipped mid-run, and runs fully off screen.
  - Bounded at 200 cases, so `make test` stays within noise. The note
    gives its wall time.
- **Not unit-tested:** `gpu_circ`'s `TILE_1` / `TILE` choice. It is
  PSX-only code that the host does not compile. It is proven by
  `psx-check`'s captures (`circ 316`, the new `circ 245`, the `all`
  captures), by `gpu-cost` / `game-gpu-cost` (real bytes under the meter),
  and by the probe's `game-state same`.
- **Prior art:** `test_a_circ_is_the_software_midpoint_circle_one_pixel_each`,
  `test_a_circfill_is_one_span_per_row_of_the_software_disc`, and
  `test_random_boxes_draw_as_the_software_does` in `test_psx_tiles`.
- **The real proofs:** every e2e `.sha256` and `.end-state` unchanged
  (`make e2e`; nothing on PC changes); `psx-check` with every capture of
  both lists matching its reference; `game-state same` on every probed run.

## Out of Scope

- Every other lever: N2 (its own PRD), G4, N5, N6, N7.
- `circfill` (already one span per row), `pset`, `rect`, `rectfill` and
  `line`.
- Changing the midpoint loop, `gfx_software.c`'s `circ`, or `draw_cost.c`'s
  metering. The meter stays an upper bound, and the worst-case peak tables
  are not touched.
- Batching several circles of one call site into one packet chain, or
  drawing wormhole rings as images: those would be other levers, and P4
  (fewer rings) is a picture change kept for the close.
- A new e2e recording to capture a circle clipped by the left or right
  edge.
- A `psx-full-game-check` (that is step 8's).
- Any warn-review warning (each is decided above, and all are left open).
- Any picture change, e2e re-record, or change to the situation rule or
  tables.

## Hypotheses on N2's outcome

N2 (`docs/prd/triage/phase3-step7-lever-n2-fixed-point-and-vector-helpers-inlined.md`)
is being implemented while this PRD is written, and nothing of it is
committed at `55eb2dc`. Check each hypothesis below against what N2
committed before N3 starts. A false one is resolved in favour of what N2
committed, and the difference goes in N3's step note.

- **H-N2-1:** N2 ends either kept (committed, measured on its three
  representative runs) or reverted (never committed, noted). Nothing in
  this PRD depends on which, except H-N2-3, H-N2-4 and H-N2-5.
- **H-N2-2:** N2's step note is `### Step 3.7 — N2`, placed after
  `### Step 3.7 — N1`. N3's goes after it, as `### Step 3.7 — N3`.
- **H-N2-3:** N2 wrote the table's first `scale share` line: the median of
  N1's 0.81 and N2's own ratio, floored at 0, or no line if both are 0. It
  applies only to `share` levers (G4, N5, N6). There is still **no
  `scale calls` line**, so N3 writes the first one, from D3's 0.42 and N3's
  ratio. If a `scale calls` line somehow exists, N3 replaces it with the
  median over D3 and N3 (the tool refuses two), and the note says so.
- **H-N2-4:** N2's re-rank keeps N3 next. N3 is `calls`-based and is not
  touched by the `share` scale. Only G4 (alone about 36 frames against
  N3's 50) could pass it, and only with a `share` scale near 2. The
  projection N3's note compares against is the one N2 left. If N2's
  re-rank put another lever before N3, the pilot runs that lever first;
  this PRD is unchanged and compares against the projection of whichever
  lever comes just before it. The `scale calls` line is still N3's to
  write, unless that lever was N7 (the only other `calls` lever). In that
  case the scale is written by whichever of N7 and N3 is measured second,
  over the levers measured by then.
- **H-N2-5:** if N2 was kept, its commit changed `src/`. Chunk 1 of this
  PRD changes `psx/` either way. So the probe baseline is retaken after
  chunk 1's commit, whatever N2's outcome: N3 checks the fingerprint
  rather than assuming.
- **H-N2-6:** N2 changed only `src/p8/fixed.{c,h}`, `src/engine/vec.{c,h}`,
  `tests/test_fixed.c` and docs, none of N3's code files. `psx_tiles.c`
  and `psx_gpu.c` use no fixed-point helper on the circle path, so the
  circle's per-call cost is N2-independent. The residual probe measures it
  anyway.
- **H-N2-7:** N2 ran no `psx-full-game-check`.
  `psx/perf/full-run-frames.txt` is still step 6's table, so no rebase or
  `in-base` mark is due before N3.
- **H-N2-8:** `psx-check` is green at N2's end, with all 39 conditions and
  both capture lists matching their references. Chunk 1 starts from that.
- **H-N2-9:** N2 acted on no warn-review warning, and no warn-review has
  run since phase 2's end. So the open list above is the plan's
  end-of-phase notes as committed at `55eb2dc`, plus plan 21's.
- **H-N2-10:** N2 updated `docs/psx-levers.md`'s projection and
  `docs/psx-game.md`'s "Still to do" figure (563 / 741 at `55eb2dc`).
  Whatever N2 left there, N3 writes its own projection end.
- **H-N2-11:** N2's line in the lever table is `measured` (kept) or
  `measured` with its observed gains (reverted), and its `after N1`
  stays. Neither affects N3's line, which has no `after`.

## Further Notes

- **Pixel identity holds by construction:** the same pixel set, as
  non-overlapping, opaque, single-colour rectangles. The proofs are the
  PC unit tests against the software `circ`, `psx-check`'s captures
  (whole, clipped, wormhole rings, ring images) and the probe's
  `game-state`.
- **Where the gain comes from:** fewer packets (−50% on the wormhole's
  radii), fewer callback calls, and no 28-comparison duplicate check per
  step. Each step's work moves to one clip and one emit per run, and the
  runs are fewer than the pixels.
- **Why the ratio may land under 1:** the estimate assumed 60% of the
  per-call cost; the packet count drops by about 50%. Some of the 20 h
  per call may be GPU-side fill time that the run form does not reduce. The
  ratio measures all of this, and the `scale calls` line carries it to N7.
- **The trace used for coverage:** `pad_slipway`'s `circ` calls were read
  from the PC replay at `55eb2dc`, with a print in a throwaway copy of the
  tree. The capture frames above come from it. `counts.txt` from
  `psx-check` gives the per-family counts for the same frames, and the
  implementer re-reads it after chunk 1.
