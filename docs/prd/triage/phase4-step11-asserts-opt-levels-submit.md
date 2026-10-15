---
title: "Global levers: asserts on vs off, -O2 / -O3 / -Os, and what submit spends its time on, on every short run (plan 20, phase 4, step 11)"
description: "Nobody has measured what asserts or the compiler level cost on the short runs, the measuring loop cannot build anything but the SDK's -O2, a boot that hits its vsync ceiling still counts as done, and nobody knows what the submit part of a frame spends its time on."
status: needs-triage
---

## Plan step (verbatim)

> 11. Measure on every run of `psx/short_runs.txt`: asserts off vs on, and
>     `-O2` / `-O3` / `-Os` (`-Os` can win on the R3000's 4 KB instruction
>     cache), with the boot ceiling raised enough for the slowest variant
>     (`draw-6000`'s breakdown boot already exits 2 vsyncs over the default
>     540). Also identify what the submit part spends its time on. Verified
>     when each has a measured delta in the report, every boot exited
>     before its ceiling with its state dump complete, and each level
>     variant's effective optimisation level (the last `-O` reaching the
>     compiler for the measured targets) is shown to be the one requested.

Plan file: `docs/plan-20.md`. Rules in force (section 2):

- 60 fps is strict: every frame (update, draw and submit) fits in 263
  hblanks. The judge is pcsx-redux at 263 hblanks.
- **Asserts go off** in `psx-game` and in the measured GPU replay (decided);
  **all gains are measured against the asserts-off baseline, except the
  assert lever itself**.
- Micro-optimizations (compiler level, asserts) are measured, but rank as a
  supplement to "do less".
- **The measuring loop is `make psx-short-runs`, judged before vs after on
  itself**, with `PSX_SHORT_RUN_REFERENCE=` empty. No full run in this step.
- Throwaway experiments run in a temporary worktree, are measured and
  discarded: **the only code the plan commits is measuring tooling**.
- Guardrails: nothing in the game or the shipped drawing path changes.
  `make e2e`, every `*.sha256` and `psx-check` stay as they are. New tooling
  is host-tested where the repo host-tests its PSX scripts
  (`psx-tools-test`) and stays out of `make test` and `make e2e`. Never
  launch the raylib app.

Step notes in force:

- **1.2**: `make psx-short-runs PSX_SHORT_RUN_ASSERTS=on|off` (default `on`);
  `off` sets the CMake option `PSX_MEASURED_REPLAY_ASSERTS=OFF`, which drops
  `-UNDEBUG` from `psx-gpu-replay` and `psx-gpu-replay-breakdown` only, and
  demotes four unused-* warning kinds from errors in that build only (game
  sources untouched). `tools/psx_short_run.sh` has an `asserts` verdict and
  a `window plain update U draw D submit S` summary line.
- **1.3**: `make psx-probe-baseline` / `make psx-probe PATCH=<file>
  [PSX_SHORT_RUNS=] [PSX_PROBE_LABEL=]` build the GPU replay in a temporary
  worktree of HEAD with `-DPSX_PROBE=ON` (doubled GPU frame buffer, fine
  clock, `psx_probe_delay`) and asserts off, ceiling `PSX_PROBE_CEILING`
  1080, and report window deltas and `game-state same|differs`. Noise floor
  0, but "the emulator does not model code-layout effects".
- **1.4**: the manifest has **six** runs: `start`, `update-4737`,
  `update-5500`, `draw-5564`, `update-5719`, `draw-6000`.
- **1.5**: the asserts-off full run measured a mean frame of **857**
  hblanks against **891** asserts on (−3.8%).
- **2.8**: `draw-6000`'s breakdown boot exited at **542** vsyncs against the
  **540** default ceiling, cutting its state dump; re-run at 720 with an
  identical summary.
- **3.10** (step 10, committed, `bad0a38`): "Ordinary update" is the
  report's last section; only frame-profile tooling changed.
- **3.9**: step 9's span patch (`u-calibrate.patch`, fine-clock spans,
  per-frame `probe-update:` records after the last frame) is the prior art
  for this step's submit spans.

## Problem Statement

Three global levers have no figure yet, and the report cannot rank them:

- **Asserts.** Turning them off is decided, and every lever is measured on
  an asserts-off baseline, but the assert lever itself has only the full
  run's mean (891 → 857). Its gain per short run, per part (update, draw,
  submit), has never been measured, so step 12 cannot carry it over the way
  it carries every other lever.
- **Compiler level.** Every PSX build is compiled at the PSn00bSDK's release
  level. Nobody knows whether `-O3` (more inlining and unrolling) or `-Os`
  (smaller code, which on the real R3000 fits its 4 KB instruction cache
  better) is faster on the judge. Worse, the measuring loop cannot even
  build another level: the SDK's interface flags put `-O2` **last** on every
  compile line of our targets (after the `-O3 -DNDEBUG` of CMake's release
  flags, which therefore never applies), so a target option `-O3` would be
  silently overridden.
- **Submit.** The submit part (from the end of `cart_draw` to the start of
  the next frame: the meter's frame end, the draw filter's frame end, the
  breakdown's frame end, `psx_gpu_frame_end` with its `DrawSync`, slot
  uploads, `PutDispEnv` and `DrawOTag`, then `psx_gpu_frame_begin`) holds 7
  or 8 hblanks a frame. Nobody knows which of those pieces costs what, which
  are shipped and which exist only in the measured replay, or whether
  `DrawSync` waits on the GPU at all on pcsx-redux.

The measuring loop also has a hole the slower variants will hit: a short-run
boot that reaches its vsync ceiling after the `done` line still passes
`done`, with its state dump cut (step 2.8's snag), because `psx-short-runs`
only echoes a failed boot and `psx_short_run.sh` checks the `done` line, not
the state block after it. The default ceiling, 540, is already 2 vsyncs too
low for `draw-6000`'s breakdown boot with asserts off at `-O2`; asserts on
or a slower level makes it worse.

## Solution

At `HEAD` (after `bad0a38`), with no change to the game:

1. **The loop refuses a cut boot.** A short-run boot that fails (ceiling,
   timeout, non-zero exit) fails its run, and `done` requires each boot's
   state block to be complete. The default ceiling and timeout are raised
   to the probe tool's (1080 vsyncs, 20 s).
2. **The loop can build another level.** `make psx-short-runs
   PSX_SHORT_RUN_OPT=O2|O3|Os` (default `O2`, today's build unchanged)
   compiles the two measured replays at that level, and the run fails if
   the level that actually reaches the compiler (the last `-O` on the
   targets' compile lines) is not the one asked for. Each run prints that
   effective level and the two executables' text, data and bss sizes: this
   is the plan's proof that "each level variant's effective optimisation
   level (the last `-O` reaching the compiler for the measured targets) is
   shown to be the one requested".
3. **Four variants on all six runs**, each measured with `make
   psx-short-runs PSX_SHORT_RUN_REFERENCE=`:
   - `base`: asserts off, `-O2` (the baseline every gain is against);
   - `asserts-on`: asserts on, `-O2`;
   - `o3`: asserts off, `-O3`;
   - `os`: asserts off, `-Os`.
   Each variant's per-run window update, draw, submit and total are set
   against `base`, with the game state checked against the PC's.
4. **Submit, split by a throwaway span patch** on the probe build, all six
   runs: each piece's hblanks over the window and per window frame, labelled
   shipped or replay-only.
5. A new report section **"Global levers"** in `docs/psx-perf-levers.md`
   with the four-variant tables, the submit split, the lever entries and a
   verification table.

## User Stories

1. As the human reading the report, I want the cost of asserts measured on every short run, so that the decided "asserts off" has a gain like every other lever.
2. As step 12, I want the assert lever's delta per run and per part (update, draw, submit), so that I can carry it over as a relative gain on each part.
3. As the human reading the report, I want the short-run assert delta set beside the full run's 891 → 857, so that I can see whether the short runs read the lever like the judge.
4. As the human reading the report, I want `-O3` measured against the `-O2` baseline on every short run, so that I know whether more inlining pays on the judge.
5. As the human reading the report, I want `-Os` measured against the `-O2` baseline on every short run, so that I know whether smaller code pays on the judge.
6. As the human reading the report, I want to be told plainly whether the judge models the R3000's instruction cache, so that I do not read an `-Os` figure as a statement about the Miyoo.
7. As the plan pilot, I want every level variant measured with asserts off, so that the levels are measured against the baseline the plan prescribes.
8. As the plan pilot, I want the `base` variant measured fresh in the same session as the three others, so that every delta compares builds of one commit.
9. As the plan pilot, I want `base` set against step 2's recorded baseline, so that a drift since step 2 is caught before any delta is read.
10. As a developer, I want `make psx-short-runs PSX_SHORT_RUN_OPT=O2|O3|Os`, so that a level is one variable away, like asserts.
11. As a developer, I want any other `PSX_SHORT_RUN_OPT` value refused at parse time, so that a typo never measures the wrong build.
12. As a developer, I want the default `O2` to produce exactly today's measured build, so that every earlier figure stays comparable.
13. As a developer, I want the level applied to `psx-gpu-replay` and `psx-gpu-replay-breakdown` only, so that `psx-game`, `psx-check` and every other target are untouched.
14. As the human reading the report, I want each level variant's effective optimisation level, read from the last `-O` on the measured targets' compile flags, shown in the report next to the level requested, so that the plan's third "verified when" item is met by evidence, not by assumption.
15. As the plan pilot, I want the run to fail when the last `-O` on the measured targets' compile flags is not the level asked for, so that the SDK's trailing `-O2` can never silently win.
16. As a developer, I want the effective-level reader host-tested on hand-written flag files, including one that holds `-O3` early and `-O2` last, so that the trap is pinned.
17. As the human reading the report, I want each variant's text, data and bss sizes of both executables, so that an `-O3` growth against the PSX's 2 MB is visible.
18. As the human reading the report, I want the warnings each level raises counted by kind, so that the implementation plan knows what a level change costs in warnings.
19. As the plan pilot, I want a non-default level built without warnings as errors in the measured targets only, so that a warning `-O3` raises in a source that must not change cannot block the measurement.
20. As the plan pilot, I want a short-run boot that fails (ceiling, timeout, non-zero exit) to fail its run, so that a cut boot is never read as a figure.
21. As the plan pilot, I want `done` to require each boot's state block complete after its done line, so that a dump cut by the ceiling is caught even when the done line was written.
22. As the plan pilot, I want the default short-run ceiling raised to 1080 vsyncs and the timeout to 20 s, so that the slowest variant fits with room to spare, as the probe tool already does.
23. As a developer, I want the raised ceiling to cost nothing on a passing run, so that `psx-short-runs` stays as fast as today (boots exit when done).
24. As the human reading the report, I want each boot's exit vsyncs against its ceiling for every variant and run, so that "every boot exited before its ceiling" is checkable.
25. As the plan pilot, I want each variant's game state compared with the PC's on every run, so that a level that changes game behaviour is caught, not ranked.
26. As the plan pilot, I want every variant `deterministic` on every run, so that a delta is a property of the build, not of a boot.
27. As the plan pilot, I want each variant's outputs copied aside before the next variant overwrites the shared run directories, so that all four stay readable.
28. As the human reading the report, I want per run and part the variant's window sum, the delta against `base` in hblanks and per window frame, and the relative change, so that the levers read like the other probes.
29. As step 12, I want the six runs pooled per part for each variant, so that I have one relative gain per part to carry over.
30. As the human reading the report, I want the breakdown boot's draw split per family for each variant on `draw-5564` and `draw-6000`, so that I see whether a level helps the game or the backend.
31. As the human reading the report, I want the submit part split into its pieces, so that I know what it spends its time on.
32. As the human reading the report, I want each submit piece labelled shipped (in `psx-game`'s loop) or replay-only, so that I do not count a replay-only cost as a lever.
33. As the human reading the report, I want `DrawSync`'s wait measured on its own, so that I know whether the GPU ever makes the CPU wait on the judge.
34. As the plan pilot, I want the submit spans' sum checked against each frame's submit and the patch's own overhead recorded, so that a missing or doubled span is caught.
35. As the plan pilot, I want every span checked against the fine clock's wrap, so that a span longer than the clock's range is never misread.
36. As the human reading the report, I want a lever entry per measured lever with location, gain, cost and risk, so that step 12 can rank them with the others.
37. As the human reading the report, I want a do-less note for any submit piece above 2% of a run's mean window frame, or a line saying none is, so that submit gets the same treatment as draw and update.
38. As the human reading the report, I want a verification table answering the step's "verified when" item by item, so that step 12 can rely on it.
39. As the plan pilot, I want this step's figures independent of step 10's ordinary-update figures, so that its levers are measured on their own baseline.
40. As a developer, I want `make test` and `make e2e` untouched in content and duration, and no game, engine, runtime or e2e change, so that every `*.sha256` stays.
41. As a developer, I want `psx-check` green and unchanged after the CMake change, so that the default build is proven untouched.
42. As the plan pilot, I want the submit span patch discarded after its numbers are recorded, so that nothing but tooling is committed.

## Implementation Decisions

### Step 10, as committed (checked on `bad0a38`)

The drafting hypotheses H-10-1 to H-10-6 were checked against `git log` and
`git diff 3fe4b8b bad0a38`. All held; they are now facts, and no fallback
remains:

- **H-10-1, report section order (held).** "Ordinary update" is the
  report's last `##` section, and the opening paragraph names it.
  "Global levers" goes after it, with one clause added to the opening
  paragraph.
- **H-10-2, nothing this step builds moved (held).** Since `3fe4b8b`, phase
  3 changed only `Makefile` (the `frame-profile-*` targets and their legend),
  `tools/frame_profile_check.sh`, `tests/test_frame_profile_check.c`,
  `tests/test_frame_profile.c` and `src/cmd/frame_profile.c` (a PC-only
  binary). Nothing under `psx/`, nothing the PSX build compiles, and none of
  `tools/psx_short_run.sh`, `tools/psx_probe.sh`, `tools/psx_boot.sh` or the
  `psx-build`, `psx-short-runs` and `psx-probe*` recipes. The probe
  baseline's fingerprint (tree ids of `src/` and `psx/`) still changes with
  `src/cmd/frame_profile.c`, so the probe tool may ask for `make
  psx-probe-baseline` before the submit probe; that rebuild is routine.
- **H-10-3, the manifest (held).** `psx/short_runs.txt` has the six runs
  `start`, `update-4737`, `update-5500`, `draw-5564`, `update-5719`,
  `draw-6000`.
- **H-10-4, no Makefile conflict (held).** Step 10's hunks are all in the
  frame-profile targets and legend; this step edits `psx-build`,
  `psx-short-runs` and their legend entries.
- **H-10-5, no shared scratch (held).** Step 10 used `tmp/plan20-ordinary/`
  and `tmp/frame-profile/ordinary/`; this step writes `tmp/plan20-global/`.
- **H-10-6, lever numbering (held).** The report's candidates use the
  letters `U` and `O`; `G` is free, so this step's levers are `G1, G2, …`.

### Tooling 1: a cut boot fails its run (`psx-short-runs`, `psx_short_run.sh`)

- `psx-short-runs` records each boot's `tools/psx_boot.sh` result in
  `boots.txt`: `exit ok` or `exit failed`, beside the wall time and vsyncs
  already there. A failed boot **fails the run** (it is added to the failed
  list, the loop goes on to the next run), in place of today's bare echo.
- `tools/psx_short_run.sh`'s `done` condition gains a check per boot: after
  its `psx-replay: done` line, the TTY holds the state block through its
  last line, `psx-state: cheat activations <n>` (the replay writes it last:
  `state_format` then `state_format_cheat_activations`). A boot without it
  fails `done` with `<boot> state dump incomplete (no cheat activations line
  after the done line)`. The `done ok …` text is unchanged. A `psx-state:
  stopped` boot keeps its existing reason.
- Defaults raised: `PSX_SHORT_RUN_CEILING` 540 → **1080**,
  `PSX_SHORT_RUN_TIMEOUT` 10 → **20**, the probe tool's values. Rationale: a
  ceiling only caps a boot that never exits, so a passing run is not slower;
  1080 is twice the slowest boot seen so far (542), which covers asserts on
  (+4% on the full run) and any level a few tens of percent slower. If a
  variant still hits 1080, that variant is re-run with
  `PSX_SHORT_RUN_CEILING=2160 PSX_SHORT_RUN_TIMEOUT=40` on the command line
  and the report says so; the default is not raised again.

### Tooling 2: an optimization level for the measured replays

- **Make variable** `PSX_SHORT_RUN_OPT=O2|O3|Os`, default `O2`, checked at
  parse time like `PSX_SHORT_RUN_ASSERTS` (one word, one of the three, else
  `$(error …)`). It is passed to the configure as the CMake option
  `PSX_MEASURED_REPLAY_OPT`. `psx-build` passes `PSX_MEASURED_REPLAY_OPT=O2`
  explicitly on every configure, as step 1.5 made it do for asserts, so a
  cached value never leaks into the build `psx-check` uses.
- **CMake option** `PSX_MEASURED_REPLAY_OPT` (string, default `O2`, one of
  `O2`, `O3`, `Os`, anything else a `FATAL_ERROR`), applied to
  `psx-gpu-replay` and `psx-gpu-replay-breakdown` only:
  - `O2` adds nothing: the compile lines stay byte-for-byte today's, and the
    SDK's `-O2` stays the effective level;
  - `O3` / `Os` put `-O3` / `-Os` **after** the SDK's interface flags on
    every compile line of those two targets, plus `-Wno-error` (warnings
    still printed in `build.log`).
  - **Trap (the reason for the check below):** a target's own compile
    options come *before* the options it inherits from the SDK's
    `psn00bsdk` interface library, which ends with `-O2`; GCC takes the last
    `-O`. A plain `target_compile_options(… -O3)` is silently overridden.
    The mechanism must land after the interface flags (for example an
    interface library linked after the SDK's, or a per-target property that
    CMake emits later); whichever is chosen, the check proves it.
  - The prebuilt libpsn00b libraries (GPU, libc helpers) stay at the level
    the SDK was built with; only our sources change level. The report says
    so.
- **Effective-level reader** `tools/psx_opt_level.sh <flags-file>`: prints
  the effective level of a CMake-generated per-target flags file (the
  `C_FLAGS = …` line of `flags.make`): the **last** `-O<x>` token, as
  `O0|O1|O2|O3|Os|Og|Oz|Ofast`. Exit 1 on a usage error, 2 when the file has
  no `C_FLAGS` line or no `-O` token.
- `psx-short-runs`, after building a run, reads both measured targets'
  flags files and fails the run (before booting) when either level is not
  `PSX_SHORT_RUN_OPT`, naming the target and both levels. It prints
  `psx-short-runs: <name>: opt <level> psx-gpu-replay <text>/<data>/<bss>
  psx-gpu-replay-breakdown <text>/<data>/<bss>` (sizes from `$(PSX_SIZE)`).
- The level is not added to the TTY or to `psx_short_run.sh`: the binary
  cannot tell `-O2` from `-O3`, and the flags check is the proof.
- **This check is the plan's third "verified when" item.** For every level
  variant (`base` at `O2`, `o3`, `os`; `asserts-on` at `O2` too) and every
  run, the report records the opt line: the level requested, and the
  effective level `tools/psx_opt_level.sh` read from each measured target's
  flags file (`psx-gpu-replay`, `psx-gpu-replay-breakdown`). A variant whose
  check failed has no figure.
- Legend entries in the Makefile are updated for the new variable, the new
  defaults and the failing boot.

### Measurement: four variants on every run

- Runs: every run of `psx/short_runs.txt` at `HEAD` (six, H-10-3).
- Commands, in this order, each followed by a copy of every run's
  `analysis/`, `boots.txt`, `build.log` and the three TTYs into
  `tmp/plan20-global/<variant>/<run>/` (the runs share
  `tmp/psx/short-runs/<run>/`, so the next variant would overwrite them):
  - `base`: `make psx-short-runs PSX_SHORT_RUN_ASSERTS=off
    PSX_SHORT_RUN_OPT=O2 PSX_SHORT_RUN_REFERENCE=`
  - `asserts-on`: `… PSX_SHORT_RUN_ASSERTS=on PSX_SHORT_RUN_OPT=O2 …`
  - `o3`: `… PSX_SHORT_RUN_ASSERTS=off PSX_SHORT_RUN_OPT=O3 …`
  - `os`: `… PSX_SHORT_RUN_ASSERTS=off PSX_SHORT_RUN_OPT=Os …`
- **Valid variant run:** the run passes (`done`, now with complete state
  blocks, `deterministic`, `breakdown`, `asserts`; `representative`
  skipped), every boot `exit ok` below its ceiling, the opt check passes,
  and the plain-1 TTY's game lines equal the PC's (`run.state.txt`, with
  `tools/psx_state_compare.sh`, as step 2 did). A level or assert mode that
  changes game state is a **finding** (undefined behaviour or an
  assert with a side effect): the variant gets no delta on that run, the
  first differing line is recorded, and the cause is investigated with
  `help-me-debug` before the step ends; it is never "fixed" in game code by
  this step.
- **Drift check:** `base` equals step 2's baseline table (and step 4's
  `draw-6000` line) on every run and part. A difference is explained (a
  commit that changed measured code) before any delta is read.
- **Deltas:** per run and part (update, draw, submit, total), variant minus
  `base`, over the window and per window frame (one decimal), and as a
  percentage of `base`'s part (one decimal). For asserts the sign is "on
  minus off", read as the gain asserts off already takes. Pooled per part:
  the six runs' window sums added, then the percentage. The step 1.5
  full-run ratio (857 / 891, −3.8%) is printed beside the assert pool, not
  gated.
- **Draw split:** from each variant's breakdown boot on `draw-5564` and
  `draw-6000` (`analysis/summary.txt`'s draw split), per family, the
  variant's time and its change against `base`.
- **Sizes and warnings:** per variant, both executables' text/data/bss
  (from the opt line), and the warnings in `build.log` counted by
  `-W<kind>`.
- Resolution: the plain boots are deterministic and the noise floor is 0
  (step 1.3); a part's resolution is 1 hblank over the window. A delta
  inside ±2 hblanks over the window is reported as "no change".

### Measurement: what submit spends its time on (throwaway)

- **Patch** `tmp/plan20-global/submit-spans.patch` (git-ignored, base
  commit recorded), on the probe build only (`PSX_PROBE`): fine-clock spans
  (step 9's mechanism: `psx_fine_clock_read` pairs, hundredths of an hblank
  through the probe's calibration) around each piece between `marks[2]` and
  `marks[3]` of the GPU replay's frame loop:
  1. `p8_draw_meter_frame_end` (shipped);
  2. `draw_filter_frame_end` (replay-only);
  3. `breakdown_frame_end` (replay-only; empty in this build);
  4. inside `psx_gpu_frame_end`: the closing packets and the overflow check
     (draw target, texture window, black fill, upscale) (shipped);
  5. `DrawSync(0)` (shipped; the GPU wait);
  6. `upload_slots` (shipped);
  7. `PutDispEnv` and `DrawOTag` (shipped);
  8. `psx_gpu_frame_begin` (shipped);
  9. the capture branch and the remaining timer reads (replay-only; the
     capture is never taken with an empty capture script).
  Per frame records, printed after the last frame as `probe-submit: frame
  <n> meter <h> filter <h> breakdown <h> close <h> drawsync <h> upload <h>
  otag <h> begin <h>` (1024 frames at most, as step 9's records).
  "Shipped" is checked against `psx-game`'s frame loop at `HEAD`.
- **Run:** `make psx-probe PATCH=tmp/plan20-global/submit-spans.patch
  PSX_PROBE_LABEL=submit-spans` (all six runs; the probe baseline rebuilt
  first if its fingerprint is stale).
- **Validity:** every boot `valid`, `game-state same`; the spans' overhead
  (window submit delta and total delta against the probe baseline)
  recorded; on every window frame the spans sum to at most the frame's
  submit and the remainder is at most 2 hblanks a frame on average; no span
  above 200 hblanks (the fine clock's 16 bits wrap at about 243 hblanks: a
  span is cross-checked against the frame timer's submit). A failure is
  investigated and re-run, never read.
- **Recorded:** per run and piece, hblanks over the window and per window
  frame, share of submit, shipped or replay-only, and whether the piece is
  above 2% of the run's mean window frame (thresholds as the report already
  uses: `start` 189.0, `draw-5564` 27.0, `draw-6000` 19.6; for the three
  one-frame update runs, 2% of their `base` window total). Expected: none is
  above (submit is under 1% of every window); then the report says so in one
  line and gives each shipped piece a "do less" note only when one is
  obvious from the code, labelled estimated.
- **`DrawSync`:** its wait is reported on its own, with one sentence on what
  it means (near zero: the GPU finishes within the CPU's draw on the judge,
  so GPU-side cost is invisible to the frame budget; large: the GPU is a
  bottleneck the CPU levers will not move).
- The measured meter and filter pieces are cross-referenced to step 8's
  no-meter experiment and its draw-filter finding, so their gains are not
  counted twice.

### What is recorded in `docs/psx-perf-levers.md`

A new section **"Global levers"** after "Ordinary update", the last section (H-10-1), and one
clause in the opening paragraph naming it. It holds:

- the commit measured, the commands, and the tooling in a few lines (opt
  variable and its check, raised ceiling, complete-state check);
- **the variant table:** per variant and run, verdicts, boot exit vsyncs
  against the ceiling, game state against the PC, and the level requested
  next to the effective level of both measured targets (the opt line);
- **the drift line:** `base` against step 2's table;
- **the delta tables:** one per variant (`asserts-on`, `o3`, `os`), per run
  and part, then pooled; the full-run assert ratio beside the assert pool;
- **the draw split** per variant on the two draw runs;
- **sizes and warnings** per variant;
- **the submit table** per run and piece, shipped or replay-only, and the
  `DrawSync` sentence;
- **lever entries**, numbered `G1, …` (H-10-6): asserts off (already
  decided; its gain is the "on minus off" delta), `-O3`, `-Os`, and any
  submit piece with a do-less note. Each: location (the CMake option and
  the targets it would apply to, or the function and file:line), measured
  gain per run and pooled relative gain per part, cost to implement (for a
  level: applying it to `psx-game`, the warnings to clear, the RAM growth),
  risk (game state, e2e hashes, PC/PSX identity, RAM). A level slower than
  `-O2` is still an entry, with a negative gain;
- **the caveat on `-Os`:** whether pcsx-redux models the R3000's
  instruction cache (from its source or documentation at the version in
  `third_party/`, cited), and that the Miyoo is never measured, so `-Os`'s
  cache argument is unverified either way;
- **the verification table**, following the step's "verified when", one
  row per item: (1) each of asserts, `-O2`/`-O3`/`-Os` and submit has a
  measured delta; (2) every boot of every variant exited before its ceiling
  with its state dump complete; (3) each level variant's effective
  optimisation level (the last `-O` reaching the compiler for the measured
  targets) is shown to be the one requested, citing the opt lines;
- the throwaway patch (one line) and the note that it was discarded.

### Not decided by a guideline call

Every question was settled from the plan, the committed report and the code:

- the four variants, not the full 2 × 3 grid: the plan measures levels
  against the asserts-off baseline and asserts against `-O2`;
- the third "verified when" item is met by the flags reader, run on the
  real build of every variant, not by a manual look at one compile line;
- the level applies to the measured replays only, as asserts did in step 2:
  the shipped game changes only in the implementation plan;
- the effective level is proven from the build's flags, not from the
  binary, because the SDK's trailing `-O2` makes a silent no-op the likely
  failure;
- a non-default level builds without warnings as errors in the measured
  targets only, so no source changes, and the report counts what it raised;
- the raised default ceiling and the boot-failure rule fix step 2.8's snag
  in the tool, so the verification's "exited before its ceiling with its
  state dump complete" is a tool verdict, not a manual reading;
- submit is split on the probe build, like step 9's update spans, because
  the fine clock only exists there; its submit equals the short runs'
  within 1 hblank (step 1.3).

No `guideline` agent was needed.

## Implementation Chunks

1. **A cut boot fails its run.** `psx_short_run.sh`'s complete-state check
   in `done`, the `exit ok|failed` record and the failing boot in
   `psx-short-runs`, the raised defaults, legend. Tests in
   `test_psx_short_run.c`. Commit.
2. **Optimization level.** `tools/psx_opt_level.sh` and its tests, the
   CMake option, `PSX_SHORT_RUN_OPT` with its parse-time check, the opt
   check and the size line in `psx-short-runs`, `psx-build` passing `O2`,
   legend. Run `psx-check` once: green and unchanged. Then one build-only
   check (no boot needed, but the run's normal path is fine): the opt line
   reads `O3` and `Os` on both measured targets when asked, and `O2` by
   default. Commit.
3. **Four variants.** Run the four commands, copy each variant's outputs
   (opt lines included), compare game states, write the section head, the
   variant table with each variant's requested and effective level, drift, delta,
   draw-split, size and warning tables. Commit the report.
4. **Submit.** Write the span patch, run the probe (after `make
   psx-probe-baseline` if the fingerprint is stale), write the submit table,
   the `DrawSync` sentence, the `-Os` caveat, the lever entries, the
   three-row verification table and the patch line. Commit the report.

## Testing Decisions

- A good test checks a tool's observable contract (exit status, the lines it
  writes, the verdicts it gives from given inputs), never its internals.
- **Tested, in `psx-tools-test` (outside `make test`, `make test-all` and
  `make e2e`):**
  - `tools/psx_short_run.sh` in `tests/test_psx_short_run.c`: a boot whose
    state block stops before its `cheat activations` line fails `done`
    naming the boot; a boot with the line passes; the `done ok` text is
    unchanged; a `stopped` boot keeps its reason. Hand-written TTY fixtures
    that lack the closing line gain it (they model a complete boot).
  - `tools/psx_opt_level.sh` in a new `tests/test_psx_opt_level.c`: the last
    `-O` wins when several appear (the SDK's real shape: `-O3 -DNDEBUG …
    -O2 -G8`), `-Os` and `-O3` read as such, no `-O` token and no `C_FLAGS`
    line exit 2, a missing argument exits 1. Prior art: `test_psx_probe.c`,
    `test_psx_short_run.c`.
- **Not tested:** the Makefile recipes and the CMake option (prior art:
  `PSX_SHORT_RUN_ASSERTS`), the throwaway span patch, the measurements and
  the report. Their proof is the tools' verdicts on the real build (opt
  check, `done`, `deterministic`, probe `valid`, game state), the span-sum
  check, `psx-check` green and unchanged, and the verification table.
- `make test-all` stays green and is not slower. No `*.sha256` moves.

## Out of Scope

- Applying asserts off or any level to `psx-game` or `psx-check`'s builds:
  the implementation plan's job.
- Other compiler flags (`-flto`, `-fno-inline`, `-G` sizes, per-file
  levels), and rebuilding libpsn00b at another level.
- Asserts on at `-O3` / `-Os`, or any variant beyond the four.
- A full run, a new short run, or fixing the short runs' `representative`
  failure.
- Step 10's ordinary update (its figures are not used here) and step 12's
  ranking, overlaps and per-frame verdict.
- Implementing any submit lever, or removing the meter (step 8's
  experiment).
- Anything on the Miyoo or PCSX-ReARMed, audio, launching the raylib app.

## Further Notes

- Today's measured compile line for `draw-6000` reads `C_FLAGS = -O3 -DNDEBUG
  -std=c99 … -O2 -G8 …`: the release `-O3` is CMake's default release flags,
  the trailing `-O2` the SDK's interface. A naive grep for `-O3` on that line
  would wrongly pass an `o3` build; the reader takes the last token.
- The judge's emulator may not charge instruction-cache misses at all (step
  1.3 already found it insensitive to code layout). If so, `-Os` can only
  look as fast as its instruction count allows on the judge, and its gain on
  a real R3000 (or the Miyoo's emulator) stays an open question the report
  states rather than resolves.
- Submit is about 7 hblanks a frame on every run (step 2): the expected
  outcome is a split that names no lever above 2%. The point of measuring it
  is to close the part, and to learn whether `DrawSync` hides GPU time.
- Asserts on the full run cost 34 hblanks a mean frame (891 vs 857); the
  short runs are expected near −4%, but update-heavy runs may differ since
  assert density varies by code.
