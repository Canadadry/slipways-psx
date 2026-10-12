---
title: "Short GPU-path runs and the draw breakdown (plan 11, step 4.1)"
description: "Every frame-time question costs a 20-minute full-game emulator run, and the dominant part (draw) is one opaque number, so no optimization can be compared or aimed yet."
status: needs-triage
---

## Problem Statement

Step 2 of plan 11 measured the full game on the GPU path: 6,047 of 6,059 frames are over one vsync (263 hblanks), draw holds 87% of the over-budget time, and update overruns the budget in 171 frames (the start spike at frame 12 among them). Step 4 has to optimize what that measurement points to, but today:

- the only way to see a frame time is `make psx-full-game-check`, about 20 minutes per run; an optimization tried, kept or reverted cannot be judged at that cost;
- emulator save states hold the old program's code, so they cannot measure a changed program;
- draw is a single figure per frame: nobody can say whether the time goes into the GPU backend (building primitives) or into the game's own drawing code in `cart_draw`, nor which draw family (cls, pset, line, rect, rectfill, circ, circfill, spr, print, pattern) costs the most;
- nobody knows the best case: if one side of draw cost nothing, would the worst frames fit the budget at all?

Sub-step 4.1 builds the measuring harness and the draw breakdown, and changes nothing in the game or in the shipped drawing path. No optimization happens here.

## Solution

A dedicated target, `make psx-short-runs`, outside `make test-all`, that:

1. Cuts five short recordings out of `worst-case/full_game.json`, one per worst window that step 2 found, as `initial_save` replays like `tests/e2e/pad_resume_at_start.json`: the stored record the game wrote shortly before the window, the title navigation that resumes it, then `full_game.json`'s own frames up to the end of the window. The start spike (frame 12) gets a plain prefix of the first 100 frames. The cuts are generated from the committed full game into `tmp/`, never into `tests/e2e/`.
2. Proves on the PC, in seconds, that each cut reproduces the original game: the resumed run ends in the same game state as `full_game.json` at the matching frame.
3. Boots each cut's GPU replay PS-EXE twice (determinism: identical `psx-frame:` lines) and once in a breakdown build that times every backend call per draw family.
4. Judges each run: done, deterministic, representative (draw and update within ±10% of step 2's full run over the window's frames), breakdown well formed; and prints per window the draw split (inside backend calls per family / game code outside calls) and the lowest frame time possible if the backend, the game's draw code, or the whole draw cost nothing.

The person running step 4.2 then has a few-minute loop with a known-good baseline and a breakdown that says where draw time goes.

## User Stories

1. As the step-4 implementer, I want short GPU-path runs that hold the worst frames of the full game, so that I can measure an optimization in minutes instead of 20.
2. As the step-4 implementer, I want the short runs built from the program under test (not from emulator save states), so that a changed program is what gets measured.
3. As the step-4 implementer, I want each short run to start from a record the game itself wrote, so that the resumed game is the real game at that point and not a hand-made fixture.
4. As the step-4 implementer, I want one short run per worst window named in the plan (frames 0–99 for the start spike at 12; the update spikes 4737, 5500, 5719; the draw cluster 5564–5578), so that every flagged part has a run that shows it.
5. As the step-4 implementer, I want the list of windows in one committed manifest, so that adding or moving a window is a one-line change reviewed in a diff.
6. As the step-4 implementer, I want the cut chosen automatically (the last save written at least 9 frames before the window whose next frame has an idle pad), so that nobody picks frames by hand and the choice is reproducible.
7. As the step-4 implementer, I want the cut refused with a message naming the window when no eligible save exists or the run would exceed the breakdown table, so that a bad window is caught at cut time and not after a boot.
8. As the step-4 implementer, I want the PC to prove each resumed run ends in the same game state as the full game at the matching frame, so that a divergence (an input edge lost at the resume, a crosshair in the wrong place) is caught before any emulator time is spent.
9. As the step-4 implementer, I want a fixed, documented mapping between a short run's frames and `full_game.json`'s frames, so that every comparison talks about the same frames.
10. As the step-4 implementer, I want the resume frame itself excluded from every comparison, so that the cost of rebuilding the world does not count as a window frame.
11. As the step-4 implementer, I want each short run booted twice with identical `psx-frame:` lines required, so that I know a before/after difference is the change and not noise.
12. As the step-4 implementer, I want each short run's draw and update over its window frames compared with step 2's full run within ±10%, so that I know the run is representative before I trust it.
13. As the step-4 implementer, I want the per-frame figures of the short run and the full run printed side by side for the window frames, so that a representativeness failure can be read at once.
14. As the step-4 implementer, I want the full run's frame table given to the check explicitly (defaulting to step 2's `frames.txt`), with a clear failure when it is missing, so that the check never compares against nothing.
15. As the step-4 implementer, I want the representativeness condition to run only when a reference frame table is given, so that later sub-steps can use the short runs after the program changed without a stale reference turning the target red.
16. As the step-4 implementer, I want the draw time split into time spent inside backend calls and time spent in game code inside `cart_draw` outside those calls, so that I know which side to optimize.
17. As the step-4 implementer, I want the backend time split per draw family, with the families of the draw filter, so that I know which kind of call costs the most.
18. As the step-4 implementer, I want call counts beside each family's time, so that I can tell an expensive call from many cheap ones.
19. As the step-4 implementer, I want a patterned fill's time counted under its shape and also under a separate "pattern" figure that is not added to the total, so that the pattern cost is visible without counting it twice.
20. As the step-4 implementer, I want call timing on a counter much finer than the hblank, so that calls shorter than one hblank are not rounded to zero.
21. As the step-4 implementer, I want the fine counter calibrated against hblanks at boot and printed, so that the breakdown is reported in hblanks like every other frame figure.
22. As the step-4 implementer, I want a calibration outside the expected range to fail the breakdown, so that a wrong counter source is never read as data.
23. As the step-4 implementer, I want a call long enough to wrap the fine counter detected and reported, so that a lost wrap never shows up as a cheap call.
24. As the step-4 implementer, I want the breakdown in a separate PS-EXE from the plain GPU replay, so that the timing overhead never enters the figures used for determinism, representativeness and later before/after comparisons.
25. As the step-4 implementer, I want the breakdown's own overhead reported per window (its draw minus the plain run's draw), so that I know how far to trust the split.
26. As the step-4 implementer, I want per window the lowest frame time possible if backend calls cost nothing, if game draw code cost nothing, and if draw cost nothing at all, with how many window frames would still be over budget in each case, so that the step-4.1 note can state the best case the breakdown allows.
27. As the step-4 implementer, I want those best cases computed from the plain run's frame parts with the breakdown's split applied, so that the timing overhead does not inflate them.
28. As the plan pilot, I want short runs never to judge zones or caps, so that the full-game check stays the only judge of sizes.
29. As the plan pilot, I want the target outside `make test-all` and `make psx-check`, so that neither the fast loop nor the PSX check gets slower.
30. As the plan pilot, I want every emulator boot of the target to have a ceiling and a timeout, so that a run that hangs ends on its own and counts as evidence.
31. As the plan pilot, I want the target to print one verdict line per condition per run and a final pass/fail, so that the verifier can decide DONE or FAILED from its output.
32. As the plan pilot, I want the target's outputs in one directory per run under `tmp/psx/short-runs/`, so that a later sub-step can compare new runs with these.
33. As the plan pilot, I want the step note to record per run: the cut frame, the run length, the determinism and representativeness verdicts with their figures, the draw split per family, the overhead, and the best cases, so that 4.2's plan-review reads facts, not guesses.
34. As a maintainer, I want the shipped game (`psx-game`, the CD image) and the plain GPU replay unchanged, so that no figure of steps 2 and 3 moves.
35. As a maintainer, I want every `*.sha256` in `tests/e2e/` unchanged and `make test-all` green on every commit, so that the game is untouched.
36. As a maintainer, I want the timing wrapper host-tested with a fake clock and a fake backend, so that its accounting is proven without an emulator.
37. As a maintainer, I want the cut planner host-tested on synthetic saves and pads, so that its choice rules are proven without replaying the full game.
38. As a maintainer, I want the short-run analysis script host-tested on synthetic TTYs, like the frame-budget script, so that every verdict and failure message is proven without a boot.
39. As a maintainer, I want the new target and its outputs described in the PSX documentation and the Makefile legend, so that the next person can run it without reading the code.

## Implementation Decisions

- **No optimization, no game change.** The game, the shipped PS-EXE, the CD image and the plain GPU replay's measured loop stay as they are. Step 2's thresholds (`PSX_FRAME_BUDGET_HBLANKS` 263) are not touched.

- **Manifest.** One committed text file in `psx/` lists the short runs, one per line: name, kind (`prefix` or `resume`), window first frame, window last frame, and for `prefix` the last frame of the run. Initial content (frames are `full_game.json`'s):
  - `start prefix 12 12 99`
  - `update-4737 resume 4737 4737`
  - `update-5500 resume 5500 5500`
  - `draw-5564 resume 5564 5578`
  - `update-5719 resume 5719 5719`

- **Cut planner (new devtools module, pure).** Given `full_game.json`'s pad frames, the list of frames at which the game wrote a save with each record's bytes, and one manifest entry, it returns the short run's frames, its `initial_save`, the cut frame W and the frame mapping, or a refusal with a reason.
  - `prefix`: the first `end + 1` frames, no `initial_save` (storage erased, as the full replay does); mapping is the identity.
  - `resume`: W is the last frame ≤ window first − 9 at which a save was written and whose next frame (W+1) has an empty pad (an idle W+1 keeps the button edges the same as in the original after the resume's cross press). The run's frames are the 13-frame title prefix of `tests/e2e/pad_resume_at_start.json` (idle f0–f9, Up f10, idle f11, cross f12, copied by value), then `full_game.json` frames W+1 … window last. Mapping: run frame 13+k ↔ full frame W+1+k; run frames 0–12 (the resume frame included) are never compared.
  - Refusals: no eligible save; a run longer than the breakdown table (1,024 frames).
- **Cut command (new host binary).** Replays `full_game.json` headless, observes each storage write (prior art: the save fuzz test and the memory save storage), feeds the planner, and writes per run into the output directory: the short-run recording (`"input": "pad"`, `initial_save` when `resume`), a reference recording made of `full_game.json`'s frames 0 … matching last frame, and a one-line map (`cut W first-game-frame 13 source-first W+1 window F L`, or the identity for `prefix`). The recordings must pass `embed_replay` (title start, default bindings).
- **PC game-match guard.** For each run, `tmp/replay` with `STATE=` on the short run and on its reference recording; both states normalized by `tools/end_state.sh` must be equal. Failure stops the target before any boot and names the run.

- **Draw timing wrapper (new devtools module, host-testable).** A draw backend wrapper like the draw filter and the meter: it wraps the backend active at install and records, per frame and per family (`draw_filter_family_t`), the ticks spent inside wrapped calls and the call count. It takes an injected clock: a fine 16-bit counter read and a coarse 16-bit counter read with a wrap limit. Per call: fine delta modulo 65,536 added to a 32-bit per-family sum; if the coarse delta reaches the limit, the frame is flagged wrapped. A patterned rectfill/circfill (fill pattern non-zero) adds its time to its shape and also to `DRAW_FILTER_PATTERN`, which is excluded from the in-call total. `cls` counts under `DRAW_FILTER_CLS`. A frame-end call stores the frame's record in a static table (capacity 1,024 frames; recording past it is a `p8_fatal`), and the table is written after the loop through a state sink, never during the measured frames. It is installed outermost (after the meter), so "inside backend calls" means everything under the p8 draw API on the GPU path: meter, filter and GPU backend.
- **PSX fine clock (new platform module).** Root counter 2, source system clock / 8 (~269 ticks an hblank), explicit mode as `psx_frame_timer` does for counter 1; the coarse clock is the existing counter-1 hblank read, wrap limit 200 hblanks. Calibration: fine ticks over 64 hblanks, printed once as `psx-draw: clk8-per-64-hblanks <n>`.
- **Breakdown PS-EXE.** A second CMake executable, `psx-gpu-replay-breakdown`, built from the GPU replay's sources plus the timing wrapper and the fine clock, with one compile definition that installs the wrapper, ends its frame next to the meter's, and writes the calibration line and `psx-draw: frame <n> <family> <ticks> <calls> … wrapped <0|1>` lines after the `psx-frame:` lines. Without the definition the GPU replay compiles to the same program as today. `make psx-build` builds it too (so it is compiled in `psx-check`, never booted there).
- **Analysis script (new tool).** Inputs: the two plain TTYs, the breakdown TTY, the run's map, the budget, an optional reference frame table, an output directory. Conditions, one line each, `ok` or `failed <reason>`:
  - `done`: each of the three boots printed its `done` line with the run's frame count;
  - `deterministic`: the two plain boots' `psx-frame:` lines are byte-identical (calibration line included);
  - `representative`: only with a reference; over the window frames (mapped), the sum of draw and the sum of update each within ±10% of the reference's sums; the per-frame figures of both runs are written side by side to the output;
  - `breakdown`: calibration in 15,500–19,000; one `psx-draw:` record per frame; no wrapped frame; in-call time (converted) never above the breakdown boot's own draw part of that frame.
  - Printed summary per run (also written to a file), over the window frames: draw split in hblanks and shares — each family, pattern apart, in-call total, outside calls; breakdown overhead (breakdown draw − plain draw); and, per frame then as the window's maximum, `update + submit + draw × (1 − s)` (backend free), `update + submit + draw × s` (game draw code free) and `update + submit` (draw free), with s the frame's in-call share in the breakdown boot and the parts from the first plain boot, plus the count of window frames still over budget in each case.
- **Target `make psx-short-runs`.** Outside `make test-all` and `make psx-check`; depends on `psx-tools-test`, `tmp/replay` and the cut command. Per manifest entry: cut, PC guard, configure the PSX CMake project in `tmp/psx/short-runs/<name>/build` with that run embedded and build only the two GPU replay targets, boot plain ×2 and breakdown ×1 with an empty capture script through `psx_boot.sh --until-exit`, then the analysis. Variables: `PSX_SHORT_RUN_REFERENCE` (default step 2's `tmp/psx/full-game/run/frame-budget/frames.txt`; empty skips `representative`; a set path that does not exist fails naming `make psx-full-game-check`), `PSX_SHORT_RUN_CEILING` 7,000 vsyncs and `PSX_SHORT_RUN_TIMEOUT` 180 s (re-set from the first measured boots under plan 11's two-thirds habit, recorded in the note), `PSX_SHORT_RUNS` to select manifest names. Fails if any condition fails, after running every run. Never runs a setup.
- **Plan rules honoured.** Short runs never judge zones or caps. Boots that outlast an agent's command limit run in the background and are waited on; only a run that ended on its own is evidence.
- **Step note content (for plan-pilot).** Per run: W, length, determinism, representativeness figures, per-family split, overhead, the three best cases and the frames still over budget; and the statement step 4 asks for if even the best case leaves frames over budget.

## Implementation Chunks

1. **Draw timing wrapper** — the devtools module with its injected clock and its unit test (host, in `make unit`).
2. **Cut planner and cut command** — the planner and its unit test, the host command, the manifest, the PC game-match guard as a host-only make target step (`make psx-short-runs-cut`, outside the suite) that writes and checks all five runs into `tmp/psx/short-runs/`.
3. **Breakdown PS-EXE** — the fine clock, the compile-time variant of the GPU replay, the CMake target, the `psx-build` source list; verified by `make psx-build` compiling and `make psx-check` staying green (the plain GPU replay unchanged).
4. **Analysis script** — the script and its host test in `psx-tools-test`.
5. **Target, docs and first measurement** — `make psx-short-runs` wiring, the Makefile legend and the PSX doc section, then one full run of the target in the background with the reference, its figures for the step note.

## Testing Decisions

- Good tests here check behaviour through the public interface: what the wrapper records for a given sequence of calls and clock readings, what the planner returns for given saves and pads, what verdict lines the script prints for given TTYs. No test reads private state.
- **Draw timing wrapper** (unit, host, fast): fake fine/coarse clocks advanced by a fake backend; checks per-family ticks and counts, 16-bit fine wrap handled by modulo, wrap flag from the coarse limit, the pattern double entry excluded from the in-call total, frame-end reset, the table capacity fatal, arguments passed unchanged to the wrapped backend, reinstalling the returned backend removes the wrapper. Prior art: the draw filter's and the meter's tests.
- **Cut planner** (unit, host, fast, synthetic data): picks the last save ≤ first − 9 with an idle next frame; skips a save whose next frame is not idle; refuses with no eligible save and when the run exceeds 1,024 frames; the 13-frame title prefix equals `pad_resume_at_start.json`'s first 13 frames; mapping and prefix-kind identity.
- **Analysis script** (host, in `psx-tools-test`, outside `make test`): each condition's ok and failed paths (missing done line, one differing frame line, representativeness just inside and just outside ±10%, missing reference skips the condition, bad calibration, wrapped frame, missing breakdown frame), and the best-case arithmetic on a hand-computed example. Prior art: the frame-budget and full-game check script tests.
- **Not unit-tested:** the PSX fine clock (hardware register access; covered by the `breakdown` calibration condition on a real boot) and the cut command's replay loop (covered by the PC game-match guard on the five real runs).
- `make test` stays as fast as it is: only two small unit binaries join it; the script test joins `psx-tools-test`.

## Out of Scope

- Any optimization of draw, update or submit (sub-steps 4.2 and 4.3), and any change to the game, to the shipped PS-EXE or to the plain GPU replay's loop.
- Changing step 2's budget, the full-game check's conditions, or any zone or cap size.
- Judging zones or caps on short runs.
- Splitting update or submit further (4.3 does its own split for update).
- Running `make psx-full-game-check` again: the reference is step 2's current frame table.
- Committing the generated short-run recordings, or adding them to `tests/e2e/`.
- Audio.

## Further Notes

- Why ±10% on window sums and not per frame: the window's light part can be tiny (update ≈ 49 hblanks in the draw cluster), where particles restarting after a resume move it by more than 10% while having no weight; sums over the window keep the plan's ±10% meaningful. The per-frame table is still written for reading.
- Why W+1 must be idle: the resume ends on a cross press at run frame 12; in the original the game saw frame W's pad before W+1. An idle W+1 makes the edges seen at W+1 and W+2 the same in both runs. The PC game-match guard is the proof; the rule only makes it pass by construction in the common case.
- Expected outcome stated by the plan: on step 2's data the median frame needs draw 4.4× faster, so the best-case figures are likely to leave frames over budget; the note says so plainly if they do. That is a finding of 4.1, not a failure of it: 4.1 is done when the short runs exist, pass their checks and the breakdown is printed.
- If `deterministic` or `representative` fails on a real run, the sub-step fails (verifier reports `FAILED:`) with the figures; the cut rules or the windows are not loosened inside this PRD.
