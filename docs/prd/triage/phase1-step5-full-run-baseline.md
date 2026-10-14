---
title: "Asserts-off full-run baseline: one make psx-full-game-check PSX_FULL_GAME_ASSERTS=off, its frame table recorded as the judge's baseline (plan 20, phase 1, step 5)"
description: "Plan 20 measures every lever against an asserts-off baseline, but the judge (psx-full-game-check) can only build its GPU replay with asserts on, so the judge's own frames have no asserts-off baseline for step 12's verdict."
status: needs-triage
---

## Plan step (verbatim)

> 5. Milestone: one asserts-off `make psx-full-game-check`, the only full run
>    of this plan besides step 9's fallback. Verified when its frame table is
>    recorded in the report as the judge's baseline, next to the step 2 short-run
>    baseline, so step 12's verdict can be stated against both.

Plan file: `docs/plan-20.md`. Context from its Rules: the judge is
`frame-budget` in `make psx-full-game-check` (pcsx-redux, GPU path, 263
hblanks a frame, no margin); asserts go off in the GPU replay that is
measured and stay on in the PC build and the tests; all gains are measured
against the asserts-off baseline; `psx-full-game-check` (about 20 min) runs
only at milestones; nothing in the game or in the shipped drawing path
changes, `make e2e`, every `*.sha256` and `psx-check` stay exactly as they
are; new tooling stays out of `make test` and `make e2e`.

Step notes in force: step 1.2 added `make psx-short-runs
PSX_SHORT_RUN_ASSERTS=on|off`, which sets the CMake option
`PSX_MEASURED_REPLAY_ASSERTS` (default `ON`; `OFF` drops `-UNDEBUG` from
`psx-gpu-replay` and `psx-gpu-replay-breakdown` only and demotes the four
unused-* warning kinds from errors in those two targets), and recorded the
asserts-off short-run baseline in `docs/psx-perf-levers.md` at `473bdcc`.
Step 1.3 added the probe tool, whose baseline equals step 2's within one
hblank per run.

## Problem Statement

Plan 20's step 12 must state, for every frame the judge measures, how far the
levers bring it against 263 hblanks. The judge's only per-frame table today
comes from an asserts-**on** GPU replay (the plan's Goal figures: 6,047 of
6,059 frames over, mean 891 hblanks), while every lever is measured against an
asserts-**off** baseline. The two cannot be mixed: the assert lever is
measured separately (step 11), and the other levers' relative gains must be
applied to an asserts-off frame.

`make psx-full-game-check` cannot produce that table. It builds through
`make psx-build`, which never passes `PSX_MEASURED_REPLAY_ASSERTS` to CMake, so
the full-game GPU replay is always built with asserts, and the option's value
in its CMake cache directory (`tmp/psx/full-game/build`) is whatever an earlier
configure left there. There is also no record of the judge's frames that
outlives `tmp/`: the next sessions of plan 20 (and the implementation plan
written from its report) would have nothing to read.

## Solution

1. `make psx-full-game-check` gains `PSX_FULL_GAME_ASSERTS=on|off` (default
   `on`, today's behaviour), checked at parse time like
   `PSX_SHORT_RUN_ASSERTS`. `off` builds the full game's measured GPU replays
   without asserts through the existing CMake option; every other PSX target,
   the CPU-path replay of the same check included, keeps its asserts.
   `make psx-build` always passes the option explicitly (`ON` unless asked
   `OFF`), so a stale cache never carries one mode into the next build.
2. The milestone run, once: `make psx-full-game-check
   PSX_FULL_GAME_ASSERTS=off` (about 20 min). The expected outcome is every
   condition `ok` except `frame-budget`, which is red (the plan's premise).
3. Its frame table is committed verbatim next to the report, and
   `docs/psx-perf-levers.md` gains a "Full-run baseline (asserts off)"
   section right after the short-run baseline: the command, commit, wall
   time, conditions, the frame summary, the four update-spike frames, the
   worst frames, the comparison with the plan's asserts-on Goal figures, and,
   per short run, the full run's window sums next to the step 2 short-run
   sums with their ratio.

## User Stories

1. As the plan pilot, I want `make psx-full-game-check PSX_FULL_GAME_ASSERTS=off`, so that the judge's run is measured on the same asserts-off build as every lever.
2. As the plan pilot, I want the default to stay `on`, so that anyone running the judge without the variable gets today's check unchanged.
3. As the plan pilot, I want any value but `on` or `off` refused before anything builds, so that a typo never costs a 20-minute run in the wrong mode.
4. As the plan pilot, I want the refusal at parse time and only when `psx-full-game-check` is a goal, so that it does not race the build under `-j` and no other target is affected, as `PSX_SHORT_RUN_ASSERTS` does.
5. As the plan pilot, I want `make psx-build` to pass the CMake option explicitly on every configure, so that an `off` run never leaves `OFF` cached for the next default run in the same build directory.
6. As the plan pilot, I want the CPU-path replay of the check (`psx-replay`) to keep asserts, so that `replay-done`, `state-game-lines` and the digests stay checked with asserts on, as the plan's "asserts stay on in the tests" asks.
7. As the plan pilot, I want `psx-check`, `psx-replay-check`, `psx-short-runs` and the probe targets to build exactly as before, so that the change is confined to the judge.
8. As the plan pilot, I want the run's asserts mode confirmed from its own boot output (the GPU boot's `psx-replay: asserts off` TTY line, the CPU boot's `on`), so that the report never records a run in the wrong mode.
9. As the plan pilot, I want the run executed once, in the background, with its log kept, so that the 20 minutes are spent only once and nothing is lost if the session waits on it.
10. As the plan pilot, I want the expected conditions stated before the run (all `ok` but `frame-budget`), so that any other red condition is recognised as a finding, not as noise.
11. As the plan pilot, I want a red `gpu-state-game-lines` or `gpu-replay-done` to stop the step with the evidence, so that a baseline whose game diverges from the PC's is never recorded.
12. As the plan pilot, I want `zone-sizes`, `cap-sizes` and `gpu-ram-room` read from the asserts-off run, so that the report notes whether asserts off moved any MIPS size.
13. As the human reading the report, I want the whole per-frame table (frame, update, draw, submit, total, vsyncs) committed, so that step 12 can put a cumulative estimate against every judged frame.
14. As the human reading the report, I want the table in a companion file referenced from the report rather than 6,000 lines inline, so that the report stays readable and the table stays greppable.
15. As the human reading the report, I want the companion file to be byte-for-byte the run's `frames.txt`, so that nothing is lost or reformatted between the judge and the report.
16. As the human reading the report, I want the frame summary (over count, worst frame, overrun mean and distribution, parts over-budget and over all frames, mean total) in the report, so that the baseline reads at a glance.
17. As the human reading the report, I want the frames 12, 4737, 5500 and 5719 listed with their parts, so that the spike work of Phase 3 starts from the judge's own numbers.
18. As the human reading the report, I want the ten worst frames by total listed, so that I see whether any frame beyond the four known spikes dominates.
19. As the human reading the report, I want the asserts-off figures next to the plan's asserts-on Goal figures (over count, mean, frame 12, draw share), labelled as a cross-commit comparison, so that I get a first idea of the assert lever without mistaking it for step 11's measurement.
20. As the human reading the report, I want, for each short run, the full run's window sums of update, draw and submit next to step 2's short-run sums and their ratio, so that the short runs' offset from the judge is known under asserts off.
21. As the plan pilot at step 12, I want that ratio per part, so that a short-run relative gain can be carried onto the judge's frames with its offset known.
22. As the human reading the report, I want the commit, command and wall time of the run recorded, so that the baseline can be reproduced.
23. As the human reading the report, I want the measured commit's game and measured-replay sources shown unchanged since step 2's baseline commit, so that the short-run and full-run baselines measure the same code.
24. As the human reading the report, I want the frame indexing (0-based, as the judge writes it, so frame 12 is the 13th line after the header) stated, so that a join with another table never slips by one.
25. As the plan pilot, I want the docs of `psx-full-game-check` (Makefile legend and `docs/psx-game.md`) to name the new variable, so that the next full run (step 9's fallback, the implementation plan's verdicts) knows how to ask for asserts off.
26. As the plan pilot, I want the default short-run reference (`psx-full-game-check`'s `frames.txt`) to be noted as now asserts-off, so that a later `representative` verdict is read in the right mode.
27. As the plan pilot, I want every assumption about the parallel step 4 written as a hypothesis with its fallback, so that the step can be implemented before or after step 4 lands.
28. As a developer, I want `make test` and `make e2e` untouched and no slower, so that the fast loop stays fast.

## Implementation Decisions

### Harness change (the only code)

- **New variable on the judge, default unchanged.** `PSX_FULL_GAME_ASSERTS`
  takes `on` (default) or `off`. Rejected: switching the default to `off`
  now. The plan only asks for one asserts-off run; which mode the judge uses
  by default is the implementation plan's call once asserts actually go off.
- **Validation** mirrors `PSX_SHORT_RUN_ASSERTS`: a parse-time `$(error ...)`
  guarded on `psx-full-game-check` being in `MAKECMDGOALS`, one word, `on` or
  `off`, message naming the variable and the value.
- **Plumbing through `psx-build`.** `psx-full-game-check` already reaches
  CMake through a recursive `make psx-build`. `psx-build` gains an internal
  variable carrying the CMake option's value (default `ON`), always passed as
  `-DPSX_MEASURED_REPLAY_ASSERTS=<ON|OFF>` on its configure line;
  `psx-full-game-check` sets it to `OFF` when `PSX_FULL_GAME_ASSERTS=off`.
  Always passing it is the point: CMake caches options per build directory,
  and `psx-build` reconfigures in place.
- **What goes off.** Exactly what the CMake option already turns off:
  `psx-gpu-replay` and `psx-gpu-replay-breakdown`. `psx-full-game-check`
  boots only `psx-gpu-replay` among them. `psx-replay` (the CPU path the check
  also boots), `psx-game`, the pad probe and the largest-slots run keep their
  asserts. No CMake change.
- **No new condition.** The asserts mode is not added as a thirteenth
  condition of `tools/psx_full_game_check.sh`: it would change a host-tested
  script and its test for a single run. The mode is checked once, by hand,
  from the boots' TTY files (`psx-replay: asserts <mode>`, printed by both
  replays) and recorded in the report. Rejected as heavier than the step
  needs; revisit if the judge's default ever becomes `off`.
- **Docs**: the Makefile legend entry of `psx-full-game-check` and its
  section in `docs/psx-game.md` (the paragraph that describes the full-game
  check, and the one-line row of the target table) name the variable and say
  that `off` builds the measured GPU replays without asserts while the
  CPU-path replay keeps them. Nothing else in the docs changes.

### The run

- Command, from a clean tree at the commit that carries the harness change:
  `make psx-full-game-check PSX_FULL_GAME_ASSERTS=off`, in the background
  with stdout and stderr kept in a log under the scratchpad or `tmp/`, waited
  on with a poll, never re-run. The existing ceilings and timeouts stay
  (asserts off only makes the GPU boot faster).
- **Expected verdict**: the make exits non-zero because `frame-budget`
  fails; every other condition (`replay-done`, `state-game-lines`,
  `sprite-sheet`, `screen`, `map`, `gpu-replay-done`, `gpu-state-game-lines`,
  `timer`, `zone-sizes`, `cap-sizes`, `gpu-ram-room`) is `ok` in
  `run/conditions.txt`. That exit status is the expected result, not a
  failure of the step.
- **Stop rule**: if any condition other than `frame-budget` is red, or the
  GPU TTY does not say `asserts off`, or the CPU TTY does not say
  `asserts on`, nothing is recorded as a baseline. The step records the
  failing conditions and their reasons in the report under the same section
  heading marked "not a baseline", and stops for the plan pilot. It does not
  re-run and does not fix the game (the plan forbids game changes). A red
  `timer` from an emulator abort (exit 134 near 32,700 vsyncs) is the known
  listener-stack issue: the implementer checks the emulator install carries
  the patch, as `docs/psx-game.md` describes, before concluding.
- **Asserts mode check**: `tmp/psx/full-game/run/gpu.tty.txt` (or the GPU
  boot's TTY file as the check names it) holds `psx-replay: asserts off`;
  the CPU boot's TTY holds `psx-replay: asserts on`. Both lines are quoted in
  the report.
- **Same code as step 2**: before recording, the implementer shows that no
  file compiled into `psx-gpu-replay` (the game, engine, PICO-8 runtime and
  platform sources of its list, `psx/CMakeLists.txt` apart from the probe
  block) nor `worst-case/full_game.json` changed between `473bdcc` (step 2's
  baseline commit) and the measured commit, except changes behind
  `PSX_PROBE` (step 3) that the non-probe build does not compile. The report
  states the result in one line. If something did change, the report names
  it and the comparison with step 2 is labelled accordingly.

### What is recorded

- **Companion file, verbatim.** The run's `run/frame-budget/frames.txt`
  (header `frame update draw submit total vsyncs`, one line a frame, 0-based)
  is copied unchanged to `docs/psx-perf-levers-full-run-frames.txt`. Decision
  over "inline in the report": the plan says "recorded in the report"; a
  6,060-line block would make the report unreadable for people and too long
  for an agent's single read, so the table is a companion file that is part
  of the report, referenced by name from it, and deleted with it when the
  report is deleted. The report's opening paragraph says so.
- **Report section** "Full-run baseline (asserts off)", inserted right after
  the "Short-run baseline (asserts off)" section (H-4-3), holding:
  - command, measured commit (full hash), wall time, emulator path, and the
    two asserts lines;
  - the conditions table (name, verdict, detail) from `conditions.txt` and
    `verdicts.txt`;
  - the frame summary from `run/frame-budget/summary.txt`, verbatim in a
    code block, then the same figures in a short table (frames, over,
    worst frame and parts, mean total, update / draw / submit sums over all
    frames and over over-budget frames, dominant part, overrun distribution);
  - the zone, budget and cap lines and the GPU ELF size and `ram-room` from
    `measurements.txt`, with one line on whether they moved against the
    asserts-on figures where the repo records them (otherwise "no asserts-on
    figure recorded");
  - the update-spike frames 12, 4737, 5500 and 5719: update, draw, submit,
    total;
  - the ten worst frames by total (ties broken by frame number), with
    parts;
  - a comparison with the plan's Goal figures (asserts on, earlier commit):
    frames over, mean frame, frame 12's total and update, draw's share of
    over-budget time; labelled "cross-commit, not step 11's assert
    measurement";
  - **the comparison with step 2** (the step's verification): for each run
    of `psx/short_runs.txt` at the measured commit, the full-run window (the
    run's window frames as the manifest gives them, summed over the
    companion file's lines), the step 2 short-run window sums from the
    report, and the ratio short ÷ full per part and for the total, as a
    percentage with one decimal. A run whose short-run sums are not in the
    report (a window step 4 may add, H-4-4) gets its full-run sums and "no
    short-run figure" in the short columns.
- The sums are computed with a one-off `awk` over the companion file, whose
  command line is given in the report so anyone can recompute them. No
  script is committed for it.
- Integer hblanks everywhere; percentages with one decimal; share of draw
  over over-budget frames with one decimal, as the Goal states it.

### Hypotheses about step 4 (being implemented in parallel)

Step 4 is the PC call-count/timing tool of
`docs/prd/triage/phase1-step4-pc-call-counts.md`. Nothing of its outcome is
read; each assumption below is checked at implementation time, with the
fallback named.

**Checked against the committed step 4 (`b7d8226`): all six hold.** No
change to `psx/CMakeLists.txt`, `tools/psx_full_game_check.sh`,
`worst-case/`, `src/p8`, `src/game`, `src/engine`, `src/platform`, or the
`psx-build` / `psx-full-game-check` recipes since step 3. H-4-4: step 4 added
one window, `draw-6000 resume 6000 6014` (asserts-off window sums update 593,
draw 13987, submit 107, in the baseline table), so the comparison table has
six rows.

- **H-4-1 — step 4 changes no file built into the measured GPU replay.** Its
  PRD instruments the game on PC through compiler flags in new `tmp/` object
  trees and adds sources under `src/devtools/` and `src/cmd/` only, none of
  them in the PSX source lists. So the full run measures the same code as
  step 2's short-run baseline whether step 4 has landed or not. Check: the
  "same code as step 2" diff above. Fallback: if step 4 touched a source in
  `psx-gpu-replay`'s list, the report names the change and labels the step 2
  comparison as cross-code.
- **H-4-2 — step 4 does not change `psx-build`, `psx-full-game-check`,
  `psx/CMakeLists.txt` or `tools/psx_full_game_check.sh`.** Its PRD adds
  PC-side targets (`frame-profile...`) and may append lines to
  `psx/short_runs.txt`. Check: `git log` on those paths since this PRD's
  base. Fallback: if step 4 edited one of those recipes, chunk 1 applies its
  change on top of step 4's version, same contract.
- **H-4-3 — step 4 leaves the "Short-run baseline (asserts off)" section of
  `docs/psx-perf-levers.md` where it is and adds its own sections
  (breakdown comparison, coverage table, any new window's figures) after the
  existing ones.** This step's section goes right after the short-run
  baseline section, before the probe tool section. Fallback: if step 4
  renamed or moved the baseline section, this step's section goes right
  after wherever step 2's figures now are; if the two edits conflict, the
  report is merged by hand, keeping both.
- **H-4-4 — step 4 may add draw windows to `psx/short_runs.txt`, booted
  once asserts off with their sums in the report.** The step 2 comparison
  covers every run of the manifest at the measured commit; a new window with
  sums in the report gets its ratio, one without gets full-run sums only.
  Fallback if step 4 adds none: the table has the five step 2 runs.
- **H-4-5 — step 4 does not touch `worst-case/full_game.json`, and its
  frame numbering is 0-based like the judge's.** Its PRD writes `frame <i>`
  lines with the same replay semantics as `tmp/replay`. So the companion
  file can be joined with step 4's per-frame profile on the frame number in
  step 12. This step does not join them; it only states the judge's
  indexing. Fallback: if step 4 indexes differently, step 12 converts; this
  step's file is unchanged.
- **H-4-6 — step 4 needs nothing from this step.** Its coverage check runs
  on PC only and its breakdown check reads `psx-short-runs` outputs, not the
  full run's. So the two steps can land in either order. Fallback: if step 4
  turns out to read `psx-full-game-check`'s `frames.txt` (for example as the
  short runs' default reference), the note under "Further Notes" about the
  reference now being asserts-off applies to it.

### Not decided by a guideline call

Every question was settled from the plan's Rules and the code: the default
stays `on` (the plan asks for one run, not a new default), no thirteenth
condition (one run, host-tested script untouched), companion file (the
table must outlive `tmp/` and stay readable). No `guideline` agent was
needed.

## Implementation Chunks

1. **Harness option.** `PSX_FULL_GAME_ASSERTS=on|off` with parse-time
   validation; `psx-build` always passes `PSX_MEASURED_REPLAY_ASSERTS`
   explicitly; the legend and `docs/psx-game.md` updated. Verified without
   booting anything: `make -n psx-full-game-check PSX_FULL_GAME_ASSERTS=off`
   shows `-DPSX_MEASURED_REPLAY_ASSERTS=OFF` on the recursive configure,
   the default shows `=ON`, `make -n psx-check` shows `=ON`,
   `PSX_FULL_GAME_ASSERTS=maybe` and `PSX_FULL_GAME_ASSERTS='on off'` fail
   at parse time with the variable named, and `PSX_FULL_GAME_ASSERTS=maybe
   make -n test` does not fail (the guard only fires for the judge). Then,
   when the toolchain is present, a configure of a scratch build directory
   with `OFF` then with `ON` shows the cached option following the command
   line each time. `make test-all` green. Commit.
2. **The run and the report.** One `make psx-full-game-check
   PSX_FULL_GAME_ASSERTS=off` on the commit of chunk 1 (about 20 min,
   background). Check the conditions, the asserts lines and the
   same-code diff; copy `frames.txt` to the companion file; compute the
   window sums; write the report section. Commit the companion file and the
   report together.

## Testing Decisions

- A good test here checks the observable contract (what `make` passes to
  CMake, what it refuses), never the recipe's text.
- **No new automated test.** The repo has no host test of Makefile variables
  (the `PSX_SHORT_RUN_ASSERTS` validation, prior art for this one, was proven
  by dry runs and one real boot, and has no test); a test would have to sit
  outside `make test` anyway (it calls `make`, and the fast loop must not
  slow down). Chunk 1's dry-run checks are the proof and go in its commit
  message.
- No host-tested script changes, so `psx-tools-test` keeps its tests as
  they are and must stay green (it is a prerequisite of
  `psx-full-game-check`).
- The milestone run is itself the end-to-end proof of the option: the GPU
  boot's `psx-replay: asserts off` line shows the option reached the build;
  the CPU boot's `asserts on` shows it did not leak.
- `make test-all` and `make e2e` stay green and unchanged; no `*.sha256`
  moves; `psx-check` is not re-run by this step (its build line only gains
  an explicit `ON`, the cached default it already had), but the dry run of
  chunk 1 proves its configure is unchanged in effect.

## Out of Scope

- Any other full run (step 9's fallback is a separate step).
- Measuring the assert lever (step 11) or any other lever.
- Changing the judge's default mode, the CPU-path replay's asserts,
  `psx-game`'s asserts, or `psx-short-runs` / `psx-probe`.
- Adding an asserts condition to `tools/psx_full_game_check.sh`.
- Fixing `frame-budget` or any other red condition, or the short runs'
  `representative` failure.
- Joining the full-run table with step 4's PC profile (step 12).
- Running `psx-short-runs` again: the step 2 figures are read from the
  report.
- Audio; anything on the Miyoo or PCSX-ReARMed; launching the raylib app.

## Further Notes

- After this run, `psx-short-runs`' default reference
  (`tmp/psx/full-game/run/frame-budget/frames.txt`) is the asserts-off table
  in this working copy. Plan 20 runs the short runs with
  `PSX_SHORT_RUN_REFERENCE=` empty, so nothing changes for it; the report
  says it in one line for anyone who runs `representative` later.
- The run takes about 20 minutes, most of it the CPU-path replay boot (its
  timeout is 1,197 s) before the GPU boot (525 s). Asserts off shortens only
  the GPU boot.
- The plan's Goal figures (6,047 of 6,059 over, mean 891, frame 12 at 9,558
  with 9,074 update) are asserts on, from plan 11's last run. They are quoted
  as given; the step does not re-run asserts on.
- Hypotheses H-4-1 to H-4-6 above are the only assumptions about step 4.
