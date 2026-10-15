---
title: "Off-screen image cache, and the background map drawn from it (D5)"
description: "Every frame re-issues one spr per visible map cell to redraw a background that never changes, and the draw levers of phase 3 have no way to draw something once and copy it afterwards."
status: needs-triage
---

## Problem Statement

`background_draw` calls `p8_map` over the whole 32×32 map every frame.
Since step 6 (B3) only the cells that meet the screen are issued, but that
still leaves about 174 `spr` calls a frame on `draw-5564` (2,606 of 7,541
over the window) and about 192 on `draw-6000` (2,877 of 7,707). Each one
goes through the runtime, the backend's clip and CLUT lookup, and a SPRT
packet, to draw pixels that are the same as the frame before. Only the
camera moves. The report (`docs/psx-perf-levers.md`, D5) puts the map at
221 hblanks a frame on `draw-5564` and 224 on `draw-6000` (241 on
`start`). It measured that at O2 with the meter and filter compiled in.
Step 6 showed that per-call figures taken that way read about four times
too high at O3 without meter and filter, so what is left today is far
smaller. It is still well above 2% of the judge's mean frame (see
"Coverage" below).

The plan also needs a reusable tool that does not exist yet. D6 (step 12),
D4 (step 13), and D2 and D3 (step 15) all mean drawing an element once
into an off-screen image and copying that image each frame. Neither the
runtime nor either backend can draw into anything other than the screen.
Plan 21 gives that job to this step: build the image cache in the runtime
and both backends, prove it pixel-identical on D5, and leave it for the
later levers.

## Solution

1. **An image cache in the PICO-8 runtime.** The runtime has a small
   fixed set of image slots. A caller *builds* a slot by handing over a
   draw function. The runtime runs that function once per screen-sized
   window of the image, with each draw call going into the image instead
   of the screen. The caller then *draws* the slot at a world position:
   one copy, clipped to the screen, transparent wherever the build drew
   nothing. Both backends implement it.
   - **Software backend (PC, and the PSX software replay).** The image
     holds colour indices in RAM.
   - **PSX GPU backend.** The image is a 15-bit area of VRAM. The build
     draws into it with the GPU, through the same primitives as on the
     screen, in a separate build buffer spliced into the frame's packet
     chain. The copy is one textured sprite from that area.
2. **Proven first on the GPU.** A throwaway spike builds the map into
   VRAM and copies it, hard-wired, and compares the result at 0 tolerance
   against a capture before any of the facility is wired. Two items
   `warn-review` flagged in the code this step extends get fixed first:
   the GPU peak bookkeeping gets a unit test, and the check scripts'
   condition counts are derived instead of hand-copied.
3. **D5 on top.** `background_draw` builds the map into a 256×256 image
   the first time it is needed, and again whenever the map cells or the
   draw palette it was built with change. Every frame it copies that image
   at the map's world position, `(0, 0)`, under the current camera. The
   grid lines and the frame rectangle drawn after the map stay as they
   are.

The picture is the same on every frame of every platform. The e2e hashes
and every `psx-check` capture stay unchanged. The draw time of in-game
frames goes down by the map's remaining per-call cost, less one copy and a
cheap cache-key check.

## User Stories

1. As the plan pilot, I want D5's target cost measured on the short runs before the lever, so that whether B3 already covers D5 is decided on the cost as it stands now, as the plan's rules require, and the figure goes in the note.
2. As the plan pilot, I want a written rule for "covered", so that the decision is mechanical: the gross target under 10.6 hblanks (2% of step 7's mean frame, 530.2) on both 15-frame draw runs.
3. As the plan pilot, I want to know before the lever whether a `psx-check` capture shows the background map, so that the Phase 3 rule ("add one at 0 tolerance first") is met before anything draws through the cache.
4. As a game developer, I want to build an image once from ordinary draw calls (`spr`, `print`, `rectfill`, `line`, …), so that later levers can cache button rows, readouts and icons without new drawing code.
5. As a game developer, I want to draw a built image at a world position under the current camera, so that the image moves with the scene like the calls it replaces.
6. As a game developer, I want pixels the build never touched to stay transparent when the image is copied, so that a cached element can sit on top of other drawing.
7. As a game developer, I want images bigger than the screen (up to 256×256), so that the whole 32×32 map fits in one image.
8. As a game developer, I want the build to run my draw function in screen-sized windows with the camera set for me, so that the existing clipping and the B3 cell culling work unchanged inside an image.
9. As a game developer, I want the build to leave the camera as it found it, so that the draws after it are unaffected.
10. As a game developer, I want a clear, enforced contract (no `cls`, no nested build, no copy during a build, a draw function that touches no game state), so that misuse fails loudly instead of drawing garbage.
11. As a game developer, I want to decide myself when an image is stale, so that the runtime stays generic and each lever keys its image on what its content depends on.
12. As a game developer, I want a cheap way to tell that the map changed, so that the background's cache check costs almost nothing per frame.
13. As a player on PC, I want the background to look exactly as before at every camera position, parallax included, so that nothing about the game changes.
14. As a player on PSX, I want the background to look exactly as on PC, so that the two platforms stay the same game.
15. As a player, I want a new galaxy to show its own map at once, so that a stale image never shows.
16. As the maintainer of the e2e replays, I want every `*.sha256` unchanged, so that the change is proven not to touch the game.
17. As the maintainer of `psx-check`, I want every capture and its reference unchanged at 0 tolerance, masked captures included, so that the GPU image path is proven pixel-identical.
18. As the maintainer of `psx-check`, I want a masked capture to show a cached image exactly when its mask holds every family drawn into it, so that the family captures keep their meaning and their references.
19. As the maintainer of the draw meter, I want build calls metered apart from the frame's bytes, against their own buffer, so that `zone gpu_frame` keeps measuring the frame buffer and the build buffer has a bound of its own.
20. As the maintainer of the cost model, I want the copy's packets and the build's window packets in the model as upper bounds, so that the meter still bounds what the GPU backend emits.
21. As the maintainer of the breakdown boot and the PC frame profile, I want image calls attributed the same way on PC and PSX, so that the unit-cost join still accepts every frame.
22. As the PSX backend maintainer, I want the image VRAM in its own region of the layout, on its own 15-bit texture page, with no overlap with the display, draw area, sheet, font, patterns or CLUTs, so that a build cannot corrupt anything else.
23. As the PSX backend maintainer, I want GPU state the build changes (draw area, offset, mask bit) restored inside the chain, so that the frame's own draws see the state they expect.
24. As the PSX backend maintainer, I want a black drawn by an untextured primitive inside an image to stay opaque, so that later levers caching rectangles or text outlines keep their black pixels.
25. As the PSX backend maintainer, I want the build buffer double-buffered like the frame buffer, so that a build in frame N never overwrites packets frame N−1's list is still reading.
26. As the plan pilot, I want the build's one-off cost recorded separately (the first-frame delta on each short run, and the judge frames it lands on), so that phase 4's spike list includes it.
27. As the plan pilot, I want D5's measured short-run draw delta against the report's estimate and against the coverage figure, with asserts mode and opt level, so that the note follows the plan's rules.
28. As the plan pilot, I want D5 kept or reverted on its draw delta without the resume frame, and the cache kept either way, so that a one-off rebuild that only resumed runs pay does not decide the lever, and the facility steps 12, 13 and 15 rely on survives.
29. As the plan pilot, I want the step to say plainly if the cache cannot be made pixel-identical, so that steps 12, 13 and 15 can skip their cached levers by citing the note.
30. As the PSX RAM budget's owner, I want the RAM and VRAM the cache costs recorded, with `ram-room`, `game-ram-room` and `gpu-ram-room` holding, so that the cost is visible.
31. As a developer running `make test` and `make e2e`, I want them no slower, so that the fast loop stays fast.
32. As the plan pilot, I want phase 3's per-call costs re-measured at O3, asserts off, no meter, no filter, with the table in the note, so that every "covers" and "under 2%" choice in phase 3 rests on today's costs and not on the report's O2 figures.
33. As the plan pilot, I want a throwaway GPU spike that builds and copies the map, hard-wired, compared at 0 tolerance, before any PC-side wiring, so that a GPU behaviour that breaks pixel identity is found before the facility is built around it.
34. As a maintainer of the GPU backend, I want the peak bookkeeping this step extends covered by a PC unit test first, so that adding the build field cannot silently break the existing peaks.
35. As a maintainer of the check scripts, I want the "all N conditions hold" counts derived from the conditions each run wrote, so that adding checks never leaves a stale hand-copied number.

## Hypotheses on step 7

Step 7 (`docs/prd/triage/phase2-step7-end-of-phase2-judge-run.md`) was a
judge run that had not happened when this PRD was first written. It is now
committed (`eb677e8`). Each hypothesis is checked against step 2.7's note
in `docs/plan-21.md`:

- **H-7-1, held.** Step 7 is committed (table `6d00bf1`, note `eb677e8`)
  and changed no code under `src/`.
- **H-7-2, held.** `frame-budget` is red: 6,046 frames of 6,059 over 263.
  The worst is frame 12 at 7,853 (update 7,616, draw 231).
- **H-7-3, held.** The run's table is `psx/perf/full-run-frames.txt` and
  `tmp/psx/full-game/run/frame-budget/frames.txt`, the short runs' default
  `representative` reference. If the tmp copy is missing (new session), the
  short runs are given
  `PSX_SHORT_RUN_REFERENCE=psx/perf/full-run-frames.txt`.
- **H-7-4, held.** Every condition but `frame-budget` held, at asserts off,
  O3, no filter, no meter. Zones and caps are unchanged (`gpu_frame` 65,028
  of 73,728).
- **H-7-5, held, corrected to the measured figure.** The mean frame is
  **530.2**, so 2% of the mean frame is **10.6 hblanks**. Every threshold
  below uses 10.6. The step-7 table also gives frame 12's draw as 231: the
  frame the map build will land on (see "Measuring and verdict").
- **H-7-6, held.** `gpu-ram-room` is 315,792. Step 2.7 did not restate
  `ram-room` and `game-ram-room`, so step 6's figures stand (`ram-room`
  325,948; `game-ram-room` about 469,652 at step 5). The cache's roughly
  110 KB (a 64 KiB software store plus two build buffers of about 24 KiB)
  keeps all three far above the 64 KiB floor.
- **H-7-7, open.** The toolchain and pcsx-redux are assumed still
  installed. If they are not, `make psn00bsdk-toolchain-setup` and
  `make psx-emulator-setup` run first.

## Implementation Decisions

### Phase 3's per-call costs (plan rule)

- Phase 3 opens by re-measuring the per-call costs on today's setup, and
  this step does it (chunk 2):
  1. `make psx-probe-baseline`.
  2. `make psx-short-runs` at O3, asserts off, no meter, no filter (the
     harness defaults since steps 3–5). If the tmp reference is missing,
     pass `PSX_SHORT_RUN_REFERENCE` as in H-7-3.
  3. `make frame-profile-unit-cost`.
- The per-family table (calls, units, hblanks, per call, per unit, for
  each run and pooled) goes in the step note, next to the report's
  O2/meter/filter table ("Backend: per-unit costs").
- If the breakdown boot of `psx-short-runs` cannot run without the meter
  or the filter, the note says what it ran with. The figures are still
  used, labelled as such.
- Every choice in this step cites both the re-measured figure and the
  report's figure:
  - the coverage verdict;
  - the "measured against estimate" lines.

### Coverage (D5 against B3)

- **Expected: D5 is not covered by B3.** This is an estimate from today's
  costs. Chunk 2's measurement decides.
  - B3 removed only the off-screen calls, about 65% of `p8_map`'s calls.
  - D5 removes all the on-screen ones that remain: about 174 a frame on
    `draw-5564` and 192 on `draw-6000`.
  - Step 6 measured an off-screen call at about 0.12 hblank at O3 without
    meter or filter. An on-screen call also pays the CLUT lookup, the
    texture-window check and a SPRT packet, so it costs more than that.
    Even at the off-screen price, the remaining calls cost about 21–23
    hblanks a frame. A realistic on-screen price (0.3–0.4) gives about
    50–77.
  - 2% of the mean frame is 10.6 hblanks (H-7-5).
  - The report's figure for the same target is 221 / 224 a frame
    (O2, meter, filter, B3 not taken).
- **Rule for "covered"** (the plan gives no threshold, and this is
  recorded as a decision): D5 is covered if its measured gross target is
  under 10.6 hblanks per window frame on **both** `draw-5564` and
  `draw-6000`. This is the plan's only numeric threshold (step 15's),
  applied the same way.
- **Measured** with the step's throwaway gross-target probe (plan rule),
  not committed:
  - The patch removes the `p8_map` call from `background_draw`. It
    changes pixels, which does not matter: only time is read.
  - Run with `make psx-probe PATCH=<patch> PSX_PROBE_LABEL=no-map`,
    against the baseline of the per-call measurement, asserts off, O3.
  - The draw delta per window frame on each run is D5's gross target.
  - The note records the gross target, the on-screen `spr` per-call cost
    it implies (gross target ÷ the run's on-screen map calls from the
    unit-cost table), the report's figure, the 10.6 threshold and the
    verdict.
- **If covered:** D5 itself is skipped and the cache is still built.
  - The pixel proof is made with D5's switch applied for one `psx-check`
    run only. The note records the result; the switch is not committed.
  - Steps 12, 13 and 15 then add their own captures, as the Phase 3 rule
    requires.

### The runtime image cache (the PICO-8 runtime's graphics module)

- **New runtime module** for images, prefixed `p8_`, with a deep, small
  interface:
  - **Building:** `p8_image_build(gfx, mem, slot, w, h, draw, context)`.
    It clears the slot, then for each window of at most 160×120 covering
    `w×h`, in row-major order, it:
    - tells the backend which window comes next;
    - sets the camera to the window's origin, so image pixel `(x, y)` is
      world `(x, y)`;
    - calls `draw(gfx, mem, context)`.

    It then restores the caller's camera and returns the backend to the
    screen.
  - **Drawing:** `p8_image_draw(gfx, mem, slot, x, y)` copies the slot's
    last build with its top-left at world `(x, y)`, after the camera,
    clipped to the screen. Pixels the build never wrote are transparent.
  - **Querying:** `p8_image_size(slot)` gives the slot's capacity.
- **Slots** are a fixed table in the runtime, with a capacity
  (width × height) for each. Slot 0 is 256×256. Later steps add slots
  (each at most 256×256), each capacity a multiple of 16 wide, as the
  PSX layout requires. The runtime knows nothing about games. The game
  names its slots (the background owns slot 0).
- **Contract, enforced with a `p8_fatal` naming the slot:**
  - `w` and `h` within the slot's capacity;
  - no `cls` during a build;
  - no build inside a build;
  - no `p8_image_draw` during a build;
  - drawing a slot that was never built.
- **Contract the name does not state** (written on the declaration):
  - The draw function is called once per window, so it must be
    deterministic and must not touch game state or `visual_random`.
  - The image bakes in the draw palette, the sprite sheet and the fill
    pattern phase in force during the build. Patterns are phased on image
    coordinates, which are world coordinates only when the image is drawn
    at `(0, 0)`.
  - The runtime never rebuilds by itself: knowing when an image is stale
    is the caller's job.
- **Draw-backend table:** four new entries, which every backend
  implements:
  - `image_begin(slot, w, h)`: clear the slot, then route draws to it;
  - `image_window(slot, x, y)`: the next calls draw at image origin
    `(x, y)`, clipped to the screen size and then to the image;
  - `image_end(slot)`: back to the screen;
  - `image_draw(gfx, mem, slot, x, y)`: the copy.

  The backend contract ("reads gfx and mem's draw state and never writes
  it") still holds. A wrapper forwards all four.
- **Map version.** `p8_mset` stamps a process-wide, ever-increasing
  counter into the vram (a new field, set by `p8_vram_init` too). Any map
  write, on any vram, then gives a version never seen before. Nothing
  else writes map cells. `p8_vram_t` is not serialized whole anywhere: the
  vram digest hashes named arrays only.

### Software backend

- `plot` writes either to the screen or to the image window in progress.
  In an image it keeps the 160×120 clip and then clips to the image
  bounds. The windows tile the image exactly, so no pixel is written
  twice from two windows.
- **Image store:** static in the software backend, sized by the slot
  table (64 KiB for slot 0). Each pixel is a colour index, or an "empty"
  marker that no primitive can store.
  - The PSX ELFs link this store (the runtime's default backend refers to
    the software backend) without using it. That is accepted. The note
    records `ram-room`, `game-ram-room` and `gpu-ram-room`, which must
    hold.
  - Lazy allocation was rejected: the PSX software replay would then take
    heap.
- **Copy:** for each pixel of the clipped rectangle that is not empty,
  store its index on the screen.

### PSX GPU backend

- **VRAM layout:** a new images region, x 704–959, y 0–511 (256 units ×
  512 lines), free today. The layout gives each slot its rectangle and its
  15-bit texture page.
  - Slot 0 sits at the page origin (704, 0), so u and v run 0–255.
  - Every slot's rectangle is 16-aligned in x and a multiple of 16 wide,
    for the FILL that clears it.
  - Later slots are cut from (704, 256)–(959, 511).
  - The layout's tests check there is no overlap with any other region
    (the 15-bit upscale page reads 320–575 only).
- **Build buffer:** two, one per frame parity, like the frame buffers.
  `packet()` writes to the build buffer while a build is open and to the
  frame buffer otherwise. The chain stays one linked list in call order,
  so the build's primitives sit in frame N's list ahead of the copy and
  run after the CLUT and pattern uploads of frame N's end.
  - No mid-frame `DrawSync` and no separate submission. The frame's
    caches serve the build's `spr` and patterned calls, as they would
    serve the direct calls.
  - A full build buffer is a `p8_fatal` naming the frame and slot, like
    the frame buffer's. The overrun check in `packet()` stays.
  - Its size, a new constant next to `GPU_FRAME_BUFFER_BYTES`, follows
    the same rule: the `make state` peak of the new build zone + 1/8,
    rounded up to 1 KiB. That is measured over the e2e replays and
    `worst-case/full_game.json`. Expect about 21 KB, so about 24 KiB each.
  - Rejected alternative, building in the frame buffer itself: a map
    build on a heavy frame (every resumed short run and e2e resume builds
    on its first frame) adds about 20 KB on top of a frame peak of 65 KB
    in a 72 KiB buffer.
  - Rejected alternative, a synchronous build with its own submission and
    CLUT uploads: it needs a mid-frame `DrawSync`, its own CLUT and
    pattern slots, and an aliasing argument.
- **`image_begin`:** a FILL of 0x0000 over the slot. 0x0000 is "nothing
  drawn", which the textured copy skips as transparent.
- **`image_window`:** draw area set to window ∩ image, draw offset set to
  image origin + window origin, and the GP0(E6h) "set mask bit while
  drawing" on (PSn00bSDK's primitive for it if it has one, else a
  one-word packet). Untextured black (TILE, LINE) is then written as
  0x8000 and stays opaque on copy. Textured draws already carry the bit
  through `psx_colour_rgb555`, so D5's `spr`-only image does not depend
  on it; the first later lever caching untextured black is where it gets
  proven.
- **`image_end`:** puts back the draw area, the draw offset (the frame's
  draw area) and the mask setting (off), inside the chain.
  `window_slot`'s texture-window tracking carries on unchanged, since the
  list stays in call order.
- **`image_draw`:**
  - texture window none;
  - DR_TPAGE for the slot's 15-bit page;
  - one SPRT, shade 128, from the slot, placed and clipped like `spr`
    (u and v follow the clip);
  - DR_TPAGE back to the frame's 4-bit page.

  The SPRT goes in the frame buffer.
- **Pure placement helpers**, so the PC can unit-test them, in the style
  of the existing PSX sprite and tile helpers:
  - the copy's SPRT (screen position, size, u and v) from world position,
    camera, image size and the slot's rectangle;
  - the window's draw area and offset.
- **Peaks (warn-review item, done first).** Today `psx_gpu_peaks` and
  `record_peaks` have no unit test. They cannot get one where they are:
  the GPU backend includes PSn00bSDK's `psxgpu.h` and builds only for the
  PSX.
  - Chunk 1 moves the peak bookkeeping, and nothing else, into a pure
    helper in the style of the CLUT and pattern caches: the peaks
    structure, reset, and "record this frame's bytes, CLUTs, patterns".
    The GPU backend calls it.
  - A PC unit test drives the helper: each field keeps its own maximum;
    reset zeroes them all.
  - This step then extends the helper with the build buffer's bytes. The
    GPU replay's `gpu-peak` TTY line gains a `build <b>` field, read by
    every parser of that line (`game-gpu-cost` in the game check, the
    full-game measurements).
  - `psx-check`'s `game-gpu-cost` checks the build peak against the PC's
    metered build peak, as it does for frame bytes.
- **Fed by the GPU spike (chunk 3).** The spike settles, on the real GPU
  path, the following:
  - the images region and page;
  - the FILL clear;
  - the draw area and offset of a window;
  - the 15-bit copy with page switch and restore.

  Chunk 5 implements what the spike proved. Any spike finding that
  contradicts a decision above (for instance a page, alignment, mask-bit
  or FILL behaviour) replaces that decision. The commit message of
  chunk 5 says which.

### Cost model, meter, filter, timing, profile

- **Cost model:**
  - The copy is DR_TWIN + 2 DR_TPAGE + SPRT, or 0 when its box misses
    the screen.
  - The window switches (open: area, offset, mask; close: the same) and
    the FILL are build bytes.
  - The build's ordinary calls are costed as they are now: the window
    camera makes the 160×120 test match the backend's clip.
- **Meter:** while a build is open, bytes go to a separate build count
  with its own last-frame value and peak. CLUTs and patterns count in the
  frame's caps, as on the GPU. `make state` gains a
  `zone gpu_build <last> <peak> <size>` line. Every check that reads zone
  lines (`psx-check`, `psx-full-game-check`, state compare) accepts and
  checks it like the others: `check_meter`'s line pattern in the
  `psx-check` script, and the PC-sourced GPU lines and `zone-sizes`
  verdict of the full-game check. `zone gpu_frame` keeps its meaning.
  `GPU_FRAME_BUFFER_BYTES` is not resized in this step.
- **Draw filter:**
  - During a build every call passes. An image always holds its full
    content.
  - The filter records which families, the pattern flag included, went
    into each slot.
  - A copy passes only when the mask holds every one of them. The
    `spr 234` capture then still shows the map exactly as before, and the
    masks without `spr` still hide it.
  - No new family bit: the capture list format, the mask bits and the
    references stay as they are.
  - In the counts, a copy that passes counts once under each family its
    image holds. Build calls are not counted (they add no frame bytes).
- **Timing (breakdown boot) and the PC call profile:** build calls are
  timed and counted under their own families. The copy goes under `spr`,
  since it is one textured sprite on the GPU. This rule is the same on
  both sides, so the unit-cost join's PC-vs-PSX call check still holds.
  The family count does not change, so no tool's table width changes.

### D5 in the game (background module)

- `background_draw` keeps a cache key:
  - the vram pointer;
  - the map version;
  - the 16 draw-palette bytes in force at the call;
  - a "built" flag.

  It rebuilds slot 0 (256×256) when the key differs. The draw function
  is `p8_map(gfx, mem, 0, 0, 0, 0, 32, 32)`, unchanged, so B3 culls per
  window.
- Every frame it then copies slot 0 at `(0, 0)` and draws the grid and
  the frame rectangle as today.
- The sprite sheet is assumed fixed after boot. It is only written by the
  asset and sheet loaders, before the first draw. That assumption is the
  image contract's, written there.
- The key check costs a pointer compare, an integer compare and a
  16-byte compare. A memcmp of the 1,024 map cells was rejected: about 5
  hblanks a frame on the judge.
- The background is the only entity on layer 1 and is drawn right after
  `cls` and the default palette block. The image's transparent pixels
  therefore show black, as the skipped cells do today.
- The grid and frame rectangle are not baked. They are not part of D5's
  report entry, and the plan takes no lever outside the report.
- The build costs roughly one frame of the map drawn six times. It lands
  on the first draw after each galaxy generation: frame 12 on the judge,
  and the first frame of every resumed short run and e2e resume. That is
  accepted and recorded as a spike for phase 4.

### Measuring and verdict

- **Before:**
  - the per-call table (chunk 2);
  - the no-map probe giving D5's gross target, which is the coverage
    figure (chunk 2).
- **After:** short runs at asserts off, O3, before and after the D5 switch
  (chunk 6), by the same method as step 6, with
  `make psx-probe-baseline` for the baseline. The note gives:
  - each part's delta per run;
  - draw per window frame on the two draw runs **without the resume
    frame**, set against the report's 221 / 224, against the coverage
    figure and against the per-call table's on-screen `spr` cost × the
    calls removed. Both figures are cited, as the plan requires.
  - The asserts mode and opt level the runs reported.
- **Without the resume frame:** each resumed run rebuilds the map on its
  first window frame, because the boot starts with no image.
  - The delta without that frame is the window's draw sum less the first
    frame's draw, before and after. It is taken from the per-frame lines
    of the measured boot (the lines `frame-budget` reads).
  - If the short-run analysis gives only window sums, the per-frame lines
    of the same boot are read directly, and the note says so.
- **Keep or revert (plan rule):**
  - D5 is kept when its pooled draw delta of `draw-5564` and `draw-6000`,
    **without the resume frame**, is a gain. Otherwise D5's switch is
    reverted and noted, and the cache stays (the plan's rule).
  - `representative` failures are recorded and do not void the figure.
    The other short-run conditions must hold.
- **The build's one-off cost**, recorded separately as input to phase 4's
  spike list:
  - **On the short runs:** the draw delta on the first window frame of
    each run. On `start` and the three one-frame update runs it is their
    only frame. They are recorded and play no part in the verdict.
  - **Judge frames it lands on:** every frame of
    `worst-case/full_game.json` whose draw follows a map change. They are
    found on the PC by counting image builds per frame in one throwaway
    replay of `full_game.json` (a counter, not committed).
  - Expected: frame 12 only, the galaxy generation, already the worst
    frame (7,853; draw 231). The note lists each frame with its current
    draw from `psx/perf/full-run-frames.txt` and the estimated build cost
    (the one-off cost measured on `start`, the same frame).

## Implementation Chunks

1. **Warn-review prerequisites**, in the code this step extends only (plan
   rule).
   - **Peaks helper.** Move the GPU peak bookkeeping into a pure helper,
     with its PC unit test (see "Peaks"). There is no behaviour change and
     the `gpu-peak` line stays as it is.
   - **Derived condition counts.** Both check scripts this step adds
     checks to, `psx-check` and `psx-full-game-check`, print a
     hand-copied count: "all thirty-two conditions hold" and "all
     fourteen conditions hold", also in their header comments. Each final
     line instead prints the number of condition lines written to that
     run's `conditions.txt`, as digits. The header comments stop stating a
     number.
     - The tests that assert the line (`test_psx_full_game_check.c`, and
       any psx-check test reading it) compare against the count of
       conditions their fixture run wrote, not a literal.
     - `docs/psx-game.md`'s "thirty-two conditions" row is reworded so it
       does not state a number.
   - `make test-all` green. Nothing else is refactored.
2. **Per-call costs, coverage and capture check.** No source change.
   - Re-measure the per-call costs (short runs, then
     `make frame-profile-unit-cost`) and keep the table for the note.
   - Run the no-map probe of "Coverage". Record D5's gross target against
     10.6 and against the report's figure, and the verdict.
   - Check that a capture of `psx/gpu_captures.txt` shows the background
     map. Expected: the `all` captures and `spr 234`; confirm on the PC
     frames that map tiles lie on screen.
   - If none does, add a 0-tolerance capture on a `pad_slipway` frame
     whose PC frame shows map tiles, with its reference, in its own
     commit, and keep `psx-check` green. Otherwise nothing is committed:
     the figures go in chunk 6's commit message.
3. **GPU spike, throwaway and uncommitted** (plan step 8). It runs before
   any PC-side wiring of the cache.
   - **Hard-wired in the GPU backend and `background_draw`:**
     1. On the first draw after a map change, clear the images region's
        slot 0 rectangle with a FILL.
     2. Draw the 32×32 map into it with the existing `spr` path, by
        windows, through draw area and offset switches.
     3. Restore the draw state.
     4. Every frame, copy the image with the 15-bit page switch, one
        SPRT, and the switch back.
   - **No runtime API.** The software backend still draws `p8_map`
     directly.
   - **Compare at 0 tolerance** against a capture that shows the map (from
     chunk 2), through `psx-check`'s capture path.
   - **Findings go in chunk 5's commit message:** pass or fail, the
     differing pixels if any, and what had to change. A failure the spike
     cannot fix is the "cannot be made pixel-identical" case. The note
     says so, chunks 4–6 are not done, and steps 12, 13 and 15 skip their
     cached levers citing it.
   - The spike's code is discarded.
4. **Runtime cache, software backend, wrappers (PC).**
   - The image module and the map version.
   - The four backend entries on the software backend.
   - Forwarding and the new rules in the meter, cost model, filter,
     timing and call profile.
   - The `zone gpu_build` line, with every test double updated.

   No game change. Unit tests through the public interfaces; `make
   test-all` green, e2e hashes unchanged by construction.
5. **PSX backend**, following the spike's findings.
   - The images region and slot rectangles in the VRAM layout.
   - The pure placement helpers, with PC unit tests.
   - The double build buffer and `packet()`'s target.
   - The four GPU entries, the build-buffer constant, and the peaks
     helper's build field with `game-gpu-cost` reading it.

   `psx-build` and `psx-check` green with every capture unchanged (nothing
   draws through an image yet).
6. **D5.**
   - `background_draw` with its key, build and copy.
   - Size the build buffer from `make state`.
   - Run `make test-all` (hashes unchanged) and `psx-check`: every
     capture unchanged at 0 tolerance. This is the cache's pixel proof
     through the real API.
   - Run the short runs. Give the verdict without the resume frame and the
     build's one-off cost with its judge frames, then keep or revert D5's
     switch.
   - If a capture differs, it goes to `fix-bug`. Only a difference shown
     to be inherent to the GPU path ends the attempt. In that case the
     cache's GPU and runtime code are reverted, and the note says the
     cache cannot be made pixel-identical, citing the difference.

## Testing Decisions

- **A good test** drives the public interface (the runtime's image calls,
  `background_draw`, the cost-model and meter entry points) and checks
  what comes out: screen pixels, metered counts, the calls a recording
  backend receives. It never inspects the store or the backend's private
  state.
- **Runtime image module**, new test file in the existing style
  (`test_gfx.c`, `test_draw_backend.c`):
  - **Matches direct drawing.** A map built into an image and copied
    gives the same screen as `p8_map` drawn directly, over a camera
    sweep: negative, past the map, odd offsets, the half-parallax steps.
  - **Transparency.** Empty cells and transparent sprite colours show
    what was drawn underneath, not black.
  - **Windows.** A 256×256 build calls the draw function six times, each
    time with the window's camera, and restores the caller's camera.
  - **Contract violations.** A partial window clips at the image edge;
    `cls`, a nested build, a copy during a build and an oversize request
    are fatal (through the test fatal hook the suite already uses).
  - **Map version.** `p8_mset` changes the map version, on any vram.
- **Wrappers:** the existing meter, filter, timing and call-profile tests
  gain cases:
  - build bytes go to the build count, not the frame;
  - the copy costs its packets on screen and 0 off screen;
  - the filter passes the copy only when every family of the image is in
    the mask;
  - timing and profile put the copy under `spr`.
  - Prior art: `test_draw_meter.c`, `test_draw_filter.c`,
    `test_draw_timing.c`, `test_call_profile.c`, `test_draw_cost.c`,
    `test_state_format.c`.
- **PSX pure helpers:** `test_psx_vram_layout.c` checks:
  - the images region against every other region;
  - each slot's 16-alignment;
  - each page's origin.

  A new test for the copy's and the window's placement checks clip, u/v
  and partial windows, against the expected rectangles. Prior art:
  `test_psx_sprite.c`, `test_psx_tiles.c`.
- **Background:** `test_background_draw.c` gains:
  - one build on first draw, none on later frames;
  - a rebuild after `p8_mset` and after a palette change;
  - the screen identical to the old direct `p8_map` draw.

  Builds are counted through a recording backend.
- **Peaks helper** (chunk 1): a new PC test drives the extracted helper:
  - each field's own maximum over several frames;
  - reset;
  - from chunk 5 on, the build field.

  Prior art: `test_psx_clut_cache.c`, `test_psx_pattern_cache.c`.
- **Derived condition counts** (chunk 1): the check-script tests assert
  "all <n> conditions hold", with n the number of condition lines the
  fixture run wrote. A failing run still prints no such line. Prior art:
  `test_psx_full_game_check.c`.
- **The GPU path itself** cannot run on the PC. Its proofs are the spike's
  0-tolerance comparison (chunk 3), then `psx-check`'s captures at 0
  tolerance through the real API (chunk 6), and `game-gpu-cost`.
- **`make test` and `make e2e` stay as fast.** On the PC the software
  build runs once per map change (six windows) and the copy replaces
  about 180 sprite blits. No new target joins the fast loop.

## Out of Scope

- Baking the grid lines and the frame rectangle (not in D5's report
  entry).
- Any later cached lever (D6, D4, D2, D3), and any slot beyond slot 0.
- Resizing `GPU_FRAME_BUFFER_BYTES` after the map's SPRTs leave the frame.
- Spreading the build over frames (phase 4's spreading covers spikes).
- Any change to the e2e hashes, the capture references, the capture list
  format or the draw-filter family bits.
- A judge run (step 16 ends phase 3), and running the raylib app.
- The other `warn-review` items of step 2.7's note (duplicated
  `asserts_enabled()`, the O3 default in five places, the `#ifdef` stub
  blocks of the GPU replay, the Makefile's size): this step does not
  extend that code.

## Further Notes

- **Step note contents:**
  - the re-measured per-call table (O3, asserts off, no meter, no filter),
    next to the report's;
  - the coverage figure per draw run and the 2% figure used (10.6, from
    step 2.7's note), each choice citing both cost figures;
  - the GPU spike's result;
  - the capture-check result (which capture shows the map, or the capture
    added);
  - D5's measured draw deltas without the resume frame (the verdict's
    figure), against the report, the per-call estimate and the coverage
    figure, and the other parts' deltas;
  - the asserts mode and opt level read from the runs;
  - the `representative` outcome;
  - the build's one-off cost: the first-frame delta on each short run,
    and the judge frames it lands on with their current draw;
  - the build buffer size chosen and its `make state` peak;
  - `ram-room`, `game-ram-room` and `gpu-ram-room`, and the VRAM taken;
  - keep or revert.
- **Expected outcome:** a measured draw gain of a few tens of hblanks per
  in-game frame. That is well under the report's 221 / 224 (O2, meter and
  filter, B3 not yet taken) and above 2% of the mean frame.
- **Timing-model caveat** (report): the copy's 19,200 texels are GPU-side
  work the judge does not charge. On real hardware, one big 15-bit sprite
  should cost no more than the ~180 small 4-bit ones it replaces.
- **Steps 12, 13 and 15** reuse the cache through new slots in the
  (704, 256) half and their own capture checks. The mask-bit black path
  gets its proof with the first of them that caches untextured black.
