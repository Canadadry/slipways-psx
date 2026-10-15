---
title: "New judge baseline with the draw filter off"
description: "The committed judge frame table still includes the draw filter's cost, so every later lever of plan 21 would be measured against a frame the shipped game never runs."
status: needs-triage
---

## Problem Statement

`psx/perf/full-run-frames.txt` is the judge's frame table: every frame of
`worst-case/full_game.json` on the GPU path in pcsx-redux, in hblanks. Plan 20
recorded it (asserts off, 6,059 frames: 6,047 over 263, worst frame 12 at
9,451, mean 857) with `psx-gpu-replay` installing the draw family filter, which
runs the draw cost model a second time on every draw call. Step 1 compiles the
filter only under a define, off in the judge. Until the judge runs again
without it, the committed table, the "mean frame" that step 15's 2% threshold
reads and the reference of the short runs' `representative` verdict all carry
a cost the shipped game never pays. Plan 21 says each `psx-full-game-check`
run replaces the table "so the next lever is measured against the run just
before it": phase 2 needs a run just before it that measures what ships.

The committed table is also an input of plan 20's report tools:
`make lever-estimate` "regenerates the estimate the report quotes", and
`make lever-estimate-check` is a known-answer check of the report's step 9
results on that exact table. Both require every spike frame's update in
`psx/perf/spike-frames.txt` to equal the table's. Overwriting the table in
place would make the report's commands lie and turn that check red.

## Solution

One `make psx-full-game-check PSX_FULL_GAME_ASSERTS=off` on the commit after
step 1, with the draw filter off (step 1's default for this target). Every
condition but `frame-budget` holds; `frame-budget` stays red as expected. Its
`run/frame-budget/frames.txt` becomes `psx/perf/full-run-frames.txt`, byte for
byte. Plan 20's table is kept, renamed `psx/perf/report-judge-frames.txt`,
as the input of the two report tools, so the report stays true and its check
stays green until step 22 deletes the report. The commit message gives the
new frames over, worst frame and mean frame against plan 20's 6,047 / 9,451 /
857, with the draw and update totals, for the plan's step note.
`docs/psx-game.md`'s stale "Still to do" figures (plan 11's asserts-on run)
are replaced with the new baseline.

## User Stories

1. As the plan pilot, I want one asserts-off judge run with the draw filter off, so that the baseline measures the draw the shipped game pays.
2. As the plan pilot, I want the run to use `make psx-full-game-check` with no variable but `PSX_FULL_GAME_ASSERTS=off`, so that the baseline is reproducible by the one command the plan names.
3. As the plan pilot, I want the GPU boot's TTY checked for `psx-replay: asserts off` and `psx-replay: draw-filter off`, so that the baseline provably measured the intended build.
4. As the plan pilot, I want every condition of the judge but `frame-budget` to hold, so that the run obeys the plan's rule for end-of-phase runs.
5. As the plan pilot, I want `frame-budget`'s verdict recorded even if, against all odds, it is green, so that the plan can jump to its last step as its rules say.
6. As the plan pilot, I want the new frame table to replace `psx/perf/full-run-frames.txt` byte for byte, so that the committed table is exactly what the judge wrote.
7. As the plan pilot, I want the new table checked for its header and 6,059 frame lines numbered 0 to 6,058, so that a truncated or malformed run is never committed.
8. As the plan pilot, I want the frames over, the worst frame and the mean frame set against plan 20's 6,047 / 9,451 / 857, so that the step note answers the step's verification.
9. As the plan pilot, I want the draw total of all frames compared with plan 20's 4,550,826, so that the filter's removal is seen judge-wide.
10. As the plan pilot, I want a draw total that is not lower than plan 20's to fail the step, so that a wiring error in step 1 is caught before any lever is measured against it.
11. As the plan pilot, I want the update total and the largest per-frame update difference against plan 20 recorded, so that a reader can see the filter left update alone apart from timer jitter.
12. As the plan pilot, I want the judge-wide draw drop compared with step 1's short-run F1 figure, so that the two measures of the same lever can be read side by side.
13. As the plan pilot, I want the summary's overrun mean, overrun distribution and dominant part recorded, so that later end-of-phase notes compare like with like.
14. As the plan pilot, I want the zone, cap and `gpu-ram-room` figures recorded, so that phase 2's meter removal (step 4) can show "the same zone and cap figures" against this run.
15. As the plan pilot, I want the ELF text size of the GPU replay recorded, so that later code-size levers have a filter-off starting point.
16. As the plan pilot, I want the ten worst frames and the four known update spikes (12, 4737, 5500, 5719) tabulated with plan 20's beside them, so that a moved spike is visible.
17. As the plan pilot, I want the full-run window sums of each short run (`psx/short_runs.txt`) on the new table, so that later `representative` verdicts can be read against it.
18. As the plan pilot, I want the wall time of the run recorded, so that later runs can be planned.
19. As the plan pilot, I want the run's `tmp/psx/full-game/run/frame-budget/frames.txt` left in place, so that later short runs' `representative` verdict compares against the filter-off judge by default.
20. As the plan pilot, I want plan 20's table kept as `psx/perf/report-judge-frames.txt`, so that `make lever-estimate` still regenerates the figures the report quotes.
21. As the plan pilot, I want `make lever-estimate-check` green after the step, so that the report's known-answer check is not broken by a baseline it was never about.
22. As the plan pilot, I want the frame-profile targets (`frame-profile-spikes`, `-spread`, `-ordinary`) to keep reading the live judge table, so that a later profile of spike frames reads the latest judge.
23. As a reader of `docs/psx-perf-levers.md`, I want its companion-file mentions and its window-sum command to name `psx/perf/report-judge-frames.txt`, so that the report's commands reproduce its figures.
24. As a reader of `psx/perf/gains.txt`, I want its header to name the table it was computed on, so that the gain table's context stays true.
25. As a reader of the Makefile legend, I want `lever-estimate` and `lever-estimate-check` described on plan 20's table and the frame-profile targets on the latest judge table, so that the targets explain themselves.
26. As a reader of `docs/psx-game.md`, I want the "Still to do" bullet on `frame-budget` to give this run's figures, asserts off and filter off, so that the doc describes what is true now.
27. As a developer, I want no source, CMake or check-script change in this step, so that the baseline measures exactly step 1's code.
28. As a developer, I want `make test-all` green with every `tests/e2e/*.sha256` unchanged, so that the step provably changes nothing on PC.
29. As the next step (step 3), I want this baseline taken asserts off, so that G1's "asserts off in psx-game" is measured against an asserts-off judge as the plan requires.

## Hypotheses on step 1

Step 1 is not committed when this PRD is written; its plan of record is the
PRD "Draw filter compiled only under a define in the GPU replay". Each
assumption about its outcome is listed with what this step does if it turns
out false.

- **H-1-1.** `make psx-full-game-check` builds `psx-gpu-replay` without the
  draw filter with no extra variable, whatever `PSX_FULL_GAME_ASSERTS` is
  (step 1 passes `PSX_BUILD_DRAW_FILTER=OFF` to `psx-build`). If false: the
  step stops before the run; a judge run with the filter on is not a baseline
  for this plan, and the fix belongs to step 1.
- **H-1-2.** The GPU replay prints `psx-replay: draw-filter off` (or `on`) on
  its TTY, and the judge has thirteen conditions, `gpu-draw-filter` after
  `gpu-state-game-lines`. If the line exists but the condition does not, this
  step checks the line by hand in `gpu.tty.txt`; if neither exists, H-1-1's
  rule applies (no proof the filter is off, no baseline).
- **H-1-3.** `PSX_FULL_GAME_ASSERTS` still defaults to `on` and accepts `off`
  (step 3 changes the default, not step 1). The run passes `off` explicitly
  either way.
- **H-1-4.** `tools/psx_frame_budget.sh` is untouched: `frames.txt` keeps the
  header `frame update draw submit total vsyncs` and `summary.txt` its line
  format. If the format changed, the committed table is still the judge's
  file byte for byte, and the report tools' fixed copy is unaffected.
- **H-1-5.** Step 1 leaves `psx/perf/` untouched and plan 20's table is still
  the committed one (its sha256 equals HEAD's at plan 21's start). Checked
  with `git log -- psx/perf` before the rename.
- **H-1-6.** Step 1 changes no game code, no update path and no replay
  recording: the run still has 6,059 frames and the PC side (`make test-all`,
  e2e hashes) is unchanged.
- **H-1-7.** Step 1's commit message records F1's measured short-run draw
  delta per frame on `draw-5564` and `draw-6000` (asserts off, O2). If it is
  missing, user story 12's comparison is noted as unavailable; it gates
  nothing.
- **H-1-8.** The PSn00bSDK toolchain and pcsx-redux are installed in
  `third_party/` (step 1's PSX checks needed them). If not,
  `make psn00bsdk-toolchain-setup` and `make psx-emulator-setup` run first.
- **H-1-9.** Step 1 did not touch the timer marks: the filter's mask set and
  frame end sat between the draw marks and the submit, so removing them moves
  draw (and a fraction of submit, about 0.12 hblanks a frame in the report's
  F1 entry), not update.

## Implementation Decisions

- **The run.** `make psx-full-game-check PSX_FULL_GAME_ASSERTS=off` on the
  committed step-1 tree, with a clean working copy of `src/`, `psx/` and
  `worst-case/` (`git status` shows nothing there), and no other variable.
  It takes about 20 minutes (plan 20: 1,166 s), longer than a foreground
  command may run: it is started in the background and waited on. The make
  exits non-zero because `frame-budget` fails; that exit is expected and is
  not a failure of the step.
- **Validity gates** (all must hold, otherwise nothing is committed and the
  cause is reported):
  - every condition in `run/conditions.txt` is `ok` except `frame-budget`;
  - `gpu.tty.txt` has `psx-replay: asserts off` and
    `psx-replay: draw-filter off`;
  - `run/frame-budget/frames.txt` has the header line and exactly 6,059
    frame lines numbered 0 to 6,058;
  - the draw total over all frames (the summary's `parts all frames` draw) is
    lower than plan 20's 4,550,826. A draw total not lower is a wiring error
    of step 1 (removing the filter cannot add cost) and fails the step.
- **`frame-budget` green** is recorded, not acted on here: the step note says
  so and the plan pilot applies the plan's rule (go to the last step).
- **Table replacement.** Plan 20's table is renamed (history kept) to
  `psx/perf/report-judge-frames.txt`; the run's `frames.txt` is copied to
  `psx/perf/full-run-frames.txt`, and `cmp` confirms byte equality.
  Decided here, per the plan's rules: the plan says the run's table replaces
  `full-run-frames.txt` (the live judge, re-replaced at every end-of-phase
  run), and "docs describe only what is true", which the report's commands
  would break if they silently read the new table. Keeping plan 20's table
  under a second name is the smallest change that honours both. The copy is
  part of the report and goes with it in step 22.
- **Who reads which table.**
  - `lever-estimate` and `lever-estimate-check` read
    `psx/perf/report-judge-frames.txt` through a new Makefile variable for the
    report's judge table. They need spike frames whose update equals the
    table's, and `psx/perf/spike-frames.txt` is plan 20's step 9
    classification: it is not regenerated or edited.
  - `frame-profile-spikes`, `frame-profile-spread` and
    `frame-profile-ordinary` keep `FRAME_PROFILE_JUDGE_FRAMES`, the live
    `psx/perf/full-run-frames.txt`. A `spikes.txt` left in `tmp/` from plan
    20 may no longer match it; `frame-profile-spread` already fails naming the
    mismatch, and rerunning `frame-profile-spikes` regenerates it.
  - `psx-short-runs`' default reference stays the run's
    `tmp/psx/full-game/run/frame-budget/frames.txt`, now this filter-off run.
- **Text that names the table.** `docs/psx-perf-levers.md`'s companion
  mentions and its window-sum `awk` command, and `psx/perf/gains.txt`'s
  header, name `psx/perf/report-judge-frames.txt`. The Makefile legend of
  `lever-estimate`, `lever-estimate-check` and `frame-profile-spikes` says
  which table each reads. No figure of the report changes.
- **`docs/psx-game.md`.** The "Still to do" bullet on `frame-budget` gets this
  run's figures (frames over of 6,059, worst frame with its update, overrun
  mean, mean frame, dominant part), stated as asserts off and draw filter
  off, and keeps "the other conditions hold" with the new count (twelve of
  thirteen under H-1-2). The plan reference in that bullet ("until plan 11
  step 4") is replaced by plan 21. Nothing else in that file changes.
- **Figures for the step note** go in the commit message (the plan pilot
  copies them into the plan's step notes), each against plan 20's value:
  - frames over 263 (plan 20: 6,047 of 6,059), worst frame and its parts
    (frame 12: 9,451 = update 8,980 + draw 464 + submit 7), mean frame
    (857);
  - overrun mean (595), overrun distribution (0 / 0 / 3 / 2,620 / 3,424),
    dominant part and its share (draw, 87.6%);
  - update, draw and submit totals, all frames (597,148 / 4,550,826 /
    45,018), and draw drop in hblanks and percent;
  - update: total difference and the largest per-frame absolute difference
    (expected within timer jitter, a few hblanks; recorded, not gated);
  - the ten worst frames and the four known spikes (12, 4737, 5500, 5719),
    new and old;
  - for each run of `psx/short_runs.txt`, the window sums on the new table
    beside the old (the report's `awk` window-sum command);
  - judge-wide draw drop per frame beside step 1's F1 short-run figure
    (H-1-7);
  - zone peaks, cap margins, GPU ELF text/data/bss and `gpu-ram-room`;
  - the commit hashed, wall time, "asserts off, O2, draw filter off".
- **No source, CMake, check-script or replay change.** Anything the run shows
  wrong in step 1's wiring is reported, not fixed here.

## Implementation Chunks

1. **Run and validate.** The judge run in the background, then the validity
   gates. Nothing in the tree changes in this chunk.
2. **Commit the baseline.** The rename and copy, the Makefile variable and
   legend, the report and `gains.txt` mentions, `docs/psx-game.md`'s bullet;
   then `make test-all`, `make lever-estimate-check` and
   `make lever-estimate ESTIMATE_RUN=draw-6000` (its summary must still show
   the report's 6,047 frames over with no lever), and the commit with the
   step-note figures.

## Testing Decisions

- A good check here exercises the tools through their make targets and
  compares printed results with recorded figures, never internals.
- No new unit test: the step adds no code and no behaviour. The data swap is
  checked by `cmp` against the judge's output and by the validity gates above.
- `make lever-estimate-check` is the regression test for the report tools on
  the renamed table (prior art: its own fixtures in
  `tests/fixtures/lever_estimate/`); `make lever-estimate ESTIMATE_RUN=draw-6000`
  reproducing 6,047 frames over is the check that the report's quoted
  estimate still regenerates.
- `make test-all` stays green with every `tests/e2e/*.sha256` unchanged; no
  PSX check enters `make test`.
- `psx-check` is not rerun: the step changes no PSX code or capture input.

## Out of Scope

- Any lever, including asserts off in `psx-game` (step 3), the draw meter
  (step 4) and `-O3` (step 5).
- Regenerating or editing `psx/perf/spike-frames.txt` or `psx/perf/gains.txt`
  figures; re-running `frame-profile-*` on the new table.
- Re-running `psx-short-runs` or the probe; the short runs' `representative`
  verdict against the new table.
- Fixing anything in step 1's wiring the run exposes: reported only.
- Deleting the report or its companions (step 22).
- Any change to captures, ceilings or references.

## Further Notes

- The new baseline is expected to stay far from green: the filter is about
  16–27% of draw on the measured rows, and draw is about 87% of the
  over-budget time; frames over will likely remain close to 6,047. The figure
  that should move is the mean frame and the draw total.
- Frame 12 (galaxy generation, update 8,980) is expected to stay the worst
  frame; its total changes only by its draw share.
- Plan 20's table was itself asserts off, so this step isolates the filter:
  the two runs differ by step 1's code alone (H-1-6).
