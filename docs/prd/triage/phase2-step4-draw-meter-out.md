---
title: "The draw meter leaves psx-game and the measured replays"
description: "The shipped psx-game and the measured GPU replays run the cost model on every draw call to meter GPU bytes that the PC replay already meters, which costs about 211 hblanks a frame on draw-5564 (lever B1)."
status: needs-triage
---

## Problem Statement

`psx-game` (the program on the CD) and the measured GPU replays
(`psx-gpu-replay` and `psx-gpu-replay-breakdown` as built by
`psx-full-game-check`, `psx-short-runs` and the probe) install the draw meter
(`p8_draw_meter_install`) around the GPU backend. On every draw call the meter
runs the cost model (string walks for `print`, a circle's midpoint loop, the
CLUT table compare on every `spr`, off screen included). It is there for two
reasons:

- `psx_gpu_frame_end(p8_draw_meter_last_frame().bytes)`: the backend fails
  with `p8_fatal` when a frame sent more bytes than metered. This proves on
  MIPS that the cost model bounds the real buffer;
- the `zone gpu_frame`, `cap gpu_cluts` and `cap gpu_patterns` lines of the
  PSX state come from the meter, and three checks read them:
  - `psx-check`'s `gpu-cost`: the GPU capture replay's meter lines equal the
    PC's;
  - `psx-check`'s `game-gpu-cost`: the CD game's meter lines equal those of
    the PC replay of its pad log;
  - `psx-full-game-check`'s `zone-sizes` and `cap-sizes` judge the GPU
    boot's state, `gpu_frame` included, and its `measurements.txt` records
    the GPU replay's meter lines behind `gpu `.

Plan 20 measured what the meter costs in the measured replay: `draw-5564`
−3170 a window (−211.3 a frame), `draw-6000` −2977 (−198.5), `start` −124,
`update-4737` −165, `update-5500` −164, `update-5719` −199. Neither the
console nor the judge needs it to draw. The PC replay meters the same draw
calls under `STATE=` (`make state`), and the backend has its own overflow
check (`packet`).

## Solution

The draw meter goes behind a compile-time switch in the GPU replay, as the
draw filter did in step 1. It is on in `psx-check`'s capture replays and off
in every measured build. `psx-game` loses it outright.

- **`psx-game`**: no meter. `psx_gpu_frame_end` gets `GPU_FRAME_BUFFER_BYTES`.
  The backend now keeps its own per-frame peaks: bytes sent, CLUT slots and
  pattern slots used. At the end of a checked boot, the game prints them as
  `psx-game: gpu-peak bytes <b> cluts <c> patterns <p>`.
- **GPU replay**: the meter is compiled in only under
  `PSX_GPU_REPLAY_DRAW_METER`, a CMake option that defaults to `OFF`. The
  Makefile adds `PSX_BUILD_DRAW_METER ?= ON`, which `psx-build` passes.
  `psx-full-game-check`, `psx-short-runs` and the probe build pass `OFF`. The
  replay prints `psx-replay: draw-meter on|off` after its draw-filter line,
  and `psx-replay: gpu-peak ...` (the backend's peaks) before its done line.
  With the meter on, it still hands the metered bytes to `psx_gpu_frame_end`.
  `psx-check` therefore keeps proving, frame by frame on `pad_slipway`, that
  the cost model bounds the real buffer. With the meter off, it hands
  `GPU_FRAME_BUFFER_BYTES`.
- **Another source for the zone and cap lines**: the PC replay's meter.
  - `psx-check`'s `gpu-cost` does not change, because the capture replay
    keeps the meter. Its `gpu` measurement lines keep the same figures.
  - `game-gpu-cost` moves into `tools/psx_game_check.sh` as the seventh game
    condition. It requires the game's backend peaks to be no larger than the
    PC replay's metered peaks for its pad log: bytes against `zone gpu_frame`,
    CLUTs against `cap gpu_cluts`, patterns against `cap gpu_patterns`.
  - `psx-full-game-check` judges `zone-sizes`/`cap-sizes` on the GPU boot's
    state, with its `gpu_*` lines replaced by the PC state's. Its
    `measurements.txt` records those PC lines behind `gpu `, which gives the
    same figures as step 2's run since the meter is deterministic. It also
    records the backend's peaks.
  - A new fourteenth condition, `gpu-draw-meter`, requires the GPU boot to
    print `psx-replay: draw-meter off`.

The PC replay, `make state`, the CPU-path PSX replay, `psx-largest-slots` and
the host frame profiler keep the meter as they are.

## User Stories

1. As a player, I want the shipped `psx-game` to stop running the cost model on every draw call, so that each frame spends its draw time on drawing.
2. As the plan pilot, I want the measured GPU replays built without the meter, so that the judge and the short runs measure what the shipped game runs.
3. As the plan pilot, I want the meter in the GPU replay behind one compile-time switch (`PSX_GPU_REPLAY_DRAW_METER`, CMake default `OFF`), in the style of `PSX_GPU_REPLAY_DRAW_FILTER`, so that one variable decides it and no source edit is needed.
4. As the plan pilot, I want `psx-build` to pass `PSX_BUILD_DRAW_METER` explicitly (default `ON`), so that a cached CMake option never leaves a build directory in the wrong mode.
5. As the plan pilot, I want `psx-full-game-check` to pass `PSX_BUILD_DRAW_METER=OFF`, so that the judge is unmetered without a remembered variable.
6. As the plan pilot, I want `psx-short-runs` and the probe build to pass `-DPSX_GPU_REPLAY_DRAW_METER=OFF` explicitly, so that every lever figure from now on is taken unmetered.
7. As the plan pilot, I want the GPU replay to print `psx-replay: draw-meter on|off` right after its draw-filter line, so that each boot says which mode it was built in.
8. As the plan pilot, I want a `gpu-draw-meter` condition in `psx-full-game-check` that fails unless the GPU boot printed `psx-replay: draw-meter off`, so that a metered judge cannot pass unnoticed.
9. As the plan pilot, I want `psx-full-game-check` to report fourteen conditions, so that its count matches the condition added.
10. As the plan pilot, I want `psx-full-game-check`'s `zone-sizes` and `cap-sizes` to judge `gpu_frame`, `gpu_cluts` and `gpu_patterns` from the PC state of `full_game.json`, so that the primitive buffer and the slot caps are still judged on the worst-case game.
11. As the plan pilot, I want the other zone, budget and cap lines still judged from the GPU boot's MIPS state, so that the PSX's own zone figures stay measured on MIPS.
12. As the plan pilot, I want the PSX state's zero `gpu_*` lines (no meter installed) never judged and never recorded as figures, so that a zero cannot pass for a measurement.
13. As the plan pilot, I want `psx-full-game-check`'s `measurements.txt` to keep its `gpu zone gpu_frame` and `gpu cap gpu_*` lines with the PC's figures, so that the step note can compare them with step 2's (`gpu_frame` 65,028 of 73,728, margin 0/8).
14. As the plan pilot, I want `psx-full-game-check`'s `measurements.txt` to gain the GPU replay's backend peaks behind `gpu gpu-peak `, so that the real worst-case buffer use sits beside the metered upper bound.
15. As the plan pilot, I want `psx_gpu_frame_end`'s argument to be a byte limit, with `GPU_FRAME_BUFFER_BYTES` passed by every unmetered caller, so that the backend keeps one overflow contract whatever the caller.
16. As the plan pilot, I want the backend's own `packet` overflow check to stay, so that a frame too large for the buffer still ends in `p8_fatal` naming the frame.
17. As the plan pilot, I want the metered capture replay to keep passing the metered bytes, so that `psx-check` still proves frame by frame on MIPS that the cost model bounds the buffer, which is what lets the PC's `zone gpu_frame` judge the PSX buffer.
18. As the plan pilot, I want the GPU backend to keep each frame's bytes sent, CLUT slots and pattern slots, and their peaks since `psx_gpu_init`, so that an unmetered program can still report its GPU use.
19. As the plan pilot, I want `psx-game` to print `psx-game: gpu-peak bytes <b> cluts <c> patterns <p>` before its done line, so that the shipped game reports its real GPU use under check.
20. As the plan pilot, I want `game-gpu-cost` to require the game's backend peaks to be no larger than the metered peaks of the PC replay of its pad log, so that the cost model's bound on the shipped game is still checked without a meter on the console.
21. As the plan pilot, I want `game-gpu-cost` to fail naming both figures when a backend peak is above the PC's metered peak, so that the failure is diagnosable from one line.
22. As the plan pilot, I want `game-gpu-cost` to fail with `(no line)` when the TTY has no `psx-game: gpu-peak` line, and to be skipped as failed when the PC replay did not run, so that a missing source is never a pass.
23. As the plan pilot, I want `game-gpu-cost` checked by `tools/psx_game_check.sh`, so that `psx-check`, `psx-game-check` and `psx-game-cd-check` get it from one place, under unit test.
24. As the plan pilot, I want `psx-check` to keep its thirty-two conditions, with `game-gpu-cost` read from `psx_game_check.sh`'s output instead of its own meter comparison, so that the count does not move.
25. As the plan pilot, I want `psx-check`'s `gpu-cost` unchanged and green, so that the capture replay's meter still matches the PC's on MIPS.
26. As the plan pilot, I want `psx-check`'s `gpu` measurement lines (`zone gpu_frame`, `cap gpu_cluts`, `cap gpu_patterns`) unchanged in value, so that the step's "same figures" holds visibly.
27. As the plan pilot, I want every `psx-check` capture unchanged at 0 tolerance, so that the 0-pixel rule holds.
28. As the plan pilot, I want `game-state-game-lines`, `game-pad-log`, `game-pad-states`, `game-done`, `game-title-left` and `game-asserts` still green, so that the unmetered game still plays what the PC plays.
29. As the plan pilot, I want `game-ram-room` and `ram-room`'s bytes left recorded before and after, so that the note shows what dropping the meter's state did (the meter code stays linked, because the shared state writer reads it).
30. As the plan pilot, I want a short-run baseline taken before any edit and a short run after, both with no variable (asserts off, O2), so that B1's measured delta per part is in the note against the report's −211.3 / −198.5 a frame.
31. As the plan pilot, I want the note to record each run's window update, draw and submit sums before and after, and the per-frame draw delta on `draw-5564` and `draw-6000`, so that the lever's actual figure is kept.
32. As the plan pilot, I want the short runs' `representative` verdict recorded and not gating, so that the expected >10% draw drop against the reference table does not void the figures.
33. As the plan pilot, I want this step's `psx-full-game-check` run to have every condition but `frame-budget` green, with its summary (frames over, worst, mean) in the note, so that B1's judge-wide effect is seen.
34. As the plan pilot, I want that run's frame table to replace `psx/perf/full-run-frames.txt`, so that the next lever is measured against the run just before it.
35. As the plan pilot, I want the probe's sanity patch `psx/probes/call-twice-cart-draw.patch` refreshed to the new replay loop, so that `make psx-probe` keeps a working sanity fixture.
36. As a developer, I want `make test-all` green with every `tests/e2e/*.sha256` unchanged, so that the PC game is provably untouched.
37. As a developer, I want `make test` and `make e2e` no slower, so that the fast loop stays fast.
38. As a developer, I want `tests/test_psx_game_check.c` to cover `game-gpu-cost` (pass within the PC peaks, fail above, fail on a missing line, failed when the PC replay is skipped), so that its contract is pinned without an emulator.
39. As a developer, I want `tests/test_psx_full_game_check.c` to cover `gpu-draw-meter` and the PC-sourced `gpu_*` lines, so that the new condition and the re-sourcing are pinned without an emulator.
40. As a reader of `docs/psx-game.md`, I want the GPU replay, game EXE, draw-filter, `psx-check` condition, `psx-full-game-check` condition and `measurements.txt` passages to say where the meter is and where the `gpu_*` figures come from, so that the doc describes the build as it is.
41. As a reader of the Makefile legend, I want `psx-build`, `psx-check`, `psx-game-check`, `psx-game-cd-check`, `psx-full-game-check`, `psx-short-runs` and the probe described with the meter switch, the new condition and the new counts, so that `make` help stays true.
42. As a reader of `docs/psx-perf-levers.md`, I want B1's Cost line, which says the `gpu_*` lines "come from the meter", to say they were re-sourced in plan 21 step 4, so that the report does not state something false until step 22 deletes it.

## Hypotheses on step 3

Step 3 is not committed when this PRD is written. Its plan of record is the
PRD "Asserts off in the shipped psx-game"
(`docs/prd/triage/phase2-step3-psx-game-asserts-off.md`). Each assumption
about its outcome is listed below with what this step does if it turns out
false.

- **H-3-1.** Step 3 is committed before this step starts, and its
  `frame-budget` reading changed nothing that would end the plan (step 3 does
  not run the judge). If step 3 is not committed, this step waits for it.
- **H-3-2.** `tools/psx_game_check.sh` has six game conditions in the order
  `game-done`, `game-pad-log`, `game-pad-states`, `game-state-game-lines`,
  `game-title-left`, `game-asserts`. It takes the expected asserts mode from
  `PSX_GAME_ASSERTS` (default `off`) and writes `pc_state.txt` and
  `game-state/` into its out dir. This step appends `game-gpu-cost` as the
  seventh and makes the header say seven. If the order or count differs,
  `game-gpu-cost` still goes last and the counts follow what is there.
- **H-3-3.** `tools/psx_check.sh` reports thirty-two conditions. Its loop
  over the game conditions lists the six names above, and it still calls
  `check_meter game-gpu-cost ...` on the CD game. This step removes that
  call and adds `game-gpu-cost` to the loop, so the count stays thirty-two.
  If step 3 left a different count, the count stays whatever step 3 left.
- **H-3-4.** `psx-game` prints `psx-game: asserts on|off` as its first TTY
  line, and its header comment lists its TTY lines. This step adds the
  `gpu-peak` line to that list. If the asserts line is missing, nothing here
  depends on it.
- **H-3-5.** `psx/CMakeLists.txt` has a `PSX_GAME_ASSERTS` option, and the
  Makefile's `psx-build` passes `-DPSX_GAME_ASSERTS=$(PSX_BUILD_GAME_ASSERTS)`
  (default `OFF`) beside `-DPSX_MEASURED_REPLAY_ASSERTS` and
  `-DPSX_GPU_REPLAY_DRAW_FILTER`. This step adds
  `-DPSX_GPU_REPLAY_DRAW_METER=$(PSX_BUILD_DRAW_METER)` in the same place. If
  the line differs, the new option goes next to the draw-filter one.
- **H-3-6.** `PSX_FULL_GAME_ASSERTS` and `PSX_SHORT_RUN_ASSERTS` default to
  `off`, so `make psx-short-runs` and `make psx-full-game-check` with no
  variable measure asserts off, at O2. The short runs report
  `asserts ok off in 3 boots`. If a default is still `on`, every run of this
  step passes `=off` explicitly and the note says so.
- **H-3-7.** Step 3 changed none of `src/cmd/psx_gpu_replay.c`,
  `src/platform/psx_gpu.c`/`.h`, `src/p8/draw_meter.c`,
  `src/devtools/state_format.c`, `tools/psx_full_game_check.sh` or
  `tools/psx_frame_budget.sh`. The measured code is therefore as at
  `758f2e6`, and step 3's own short run gives the same window figures as
  this step's "before" run. If one changed, this step applies its decisions
  to the new text. The "before" baseline is taken fresh anyway (Chunk 0).
- **H-3-8.** `tests/test_psx_game_check.c`'s crafted TTY helper writes
  `psx-game: asserts off` before `psx-game: ready`, and every passing case
  expects six `ok` lines. This step adds the `gpu-peak` line to the helper
  and expects seven.
- **H-3-9.** Step 3 edited `docs/psx-game.md` only in the build paragraph,
  the game condition list and counts, the `PSX_*_ASSERTS` paragraphs and the
  `psx-game` protocol lines. It edited the Makefile legend of `psx-check`,
  `psx-game-cd-check`, `psx-full-game-check` and `psx-short-runs`. This step
  edits neighbouring passages, and if step 3 rewrote one of them, this step
  edits the new text to the same effect.
- **H-3-10.** `tmp/psx/full-game/run/frame-budget/frames.txt` is still
  step 2's table (equal to `psx/perf/full-run-frames.txt`), so the short
  runs' `representative` compares against step 2. If it is missing, the
  short runs pass `PSX_SHORT_RUN_REFERENCE=psx/perf/full-run-frames.txt`,
  and the note says so.
- **H-3-11.** The PSn00bSDK toolchain and pcsx-redux are installed in
  `third_party/`. If not, `make psn00bsdk-toolchain-setup` and
  `make psx-emulator-setup` run first.

## Implementation Decisions

- **Where the meter stays and where it goes.** It goes from `psx-game`
  (unconditionally) and from every measured GPU replay build. It stays in:
  - `psx-check`'s GPU capture replays (`psx-build`'s default);
  - the PC replay under `STATE=` (`make state`);
  - the CPU-path PSX replay `psx-replay`, which is neither shipped nor
    measured;
  - the host frame profiler.

  "Measured replays" means the GPU replay as built by `psx-full-game-check`,
  `psx-short-runs`, `psx-probe-baseline` and `psx-probe`. The capture
  replays keep the meter because `gpu-cost` and the per-frame metered-bytes
  check are what prove, on MIPS, that the PC's `zone gpu_frame` is an upper
  bound of the PSX buffer. That proof is what lets the PC's figure stand as
  the "other source" everywhere else.
- **CMake.** A new option, `PSX_GPU_REPLAY_DRAW_METER` (default `OFF`), adds
  the `PSX_GPU_REPLAY_DRAW_METER` definition to `psx-gpu-replay` and
  `psx-gpu-replay-breakdown`. Its block mirrors `PSX_GPU_REPLAY_DRAW_FILTER`,
  and the two switches stay independent: the filter can be built without the
  meter and the meter without the filter.
- **Makefile.**
  - `PSX_BUILD_DRAW_METER ?= ON`, passed by `psx-build` as
    `-DPSX_GPU_REPLAY_DRAW_METER=...` (CMake caches options per build
    directory).
  - `psx-full-game-check` passes `PSX_BUILD_DRAW_METER=OFF`.
  - The `cmake` invocations of `psx-short-runs` and of
    `psx-probe-baseline`/`psx-probe` add `-DPSX_GPU_REPLAY_DRAW_METER=OFF`
    explicitly, beside their `-DPSX_GPU_REPLAY_DRAW_FILTER=OFF`.
  - There is no user-facing measuring variable for the meter.
- **GPU replay** (`src/cmd/psx_gpu_replay.c`):
  - The meter's install, frame end and byte limit sit behind
    `#ifdef PSX_GPU_REPLAY_DRAW_METER`, as three small static helpers in the
    same style as the draw-filter helpers (`meter_install`,
    `meter_frame_end`, `meter_frame_limit`). Off, they do nothing and return
    `GPU_FRAME_BUFFER_BYTES`.
  - The install order is unchanged: filter first, meter outermost, so the
    meter still sees masked calls.
  - New TTY line `psx-replay: draw-meter on|off`, right after
    `psx-replay: draw-filter on|off`.
  - New TTY line `psx-replay: gpu-peak bytes <b> cluts <c> patterns <p>`,
    after the probe lines and before the done line. It is not printed on a
    `p8_fatal` path.
  - The header comment's protocol list gains both lines.
- **Backend** (`src/platform/psx_gpu.h`/`.c`):
  - `psx_gpu_frame_end`'s argument is renamed to a byte limit. Its contract:
    `p8_fatal` when the frame sent more than the limit. The message says
    `limit` instead of `metered`.
  - New `psx_gpu_peaks()` returns a small struct `{ unsigned long bytes; int
    cluts; int patterns; }`. Each field is its own peak over the frames ended
    since `psx_gpu_init`: the bytes the frame put in its primitive buffer
    (upscale and clear included, as metered), the CLUT cache count and the
    pattern cache count. Each is taken in `psx_gpu_frame_end` before
    `current` flips.
  - `packet`'s overflow check and the frame-overhead size check are
    unchanged.
- **`psx-game`** (`src/cmd/psx_game.c`):
  - The meter include, install and frame end go.
  - Each frame ends with `psx_gpu_frame_end(GPU_FRAME_BUFFER_BYTES)`.
  - After the loop, before the done line, it prints
    `psx-game: gpu-peak bytes <b> cluts <c> patterns <p>`.
  - Its state block still prints `zone gpu_frame 0 0 <limit>` and zero
    `cap gpu_*` peaks. This is the existing behaviour of an unmetered state
    (as `psx-largest-slots` already prints today), and no check reads those
    lines from it.
  - `pad_log` ignores `psx-game:` lines that are not `pad` lines, so the pad
    log is unaffected.
- **`state_format` is not changed.** Unmetered PSX states print zero
  `gpu_*` lines, and every consumer that judges or records `gpu_*` figures
  takes them from the PC state instead. Omitting the lines when the meter is
  not installed was considered and rejected: it would change a shared PC/PSX
  writer and its PC tests to save a filter in two scripts.
- **`game-gpu-cost`** (in `tools/psx_game_check.sh`, checked last, after
  `game-asserts`):
  - **Inputs:** the first `psx-game: gpu-peak bytes B cluts C patterns P`
    line of the TTY, and the PC state `pc_state.txt` that
    `game-state-game-lines` writes.
  - **Pass:** when `B > 0`, `B` ≤ the PC's `zone gpu_frame` peak (field 4),
    `C` ≤ the `cap gpu_cluts` peak and `P` ≤ the `cap gpu_patterns` peak. It
    prints
    `game-gpu-cost: ok, bytes B of M metered, cluts C of Mc, patterns P of Mp`.
  - **Fail with `psx-game: gpu-peak (no line)`** when the TTY has no
    `gpu-peak` line, or the line has a zero byte count.
  - **Fail with `skipped, game-state-game-lines failed`** when the PC state
    is missing.
  - **Fail with `PC state has no <line>`** when the PC state lacks one of
    the three lines.
  - **Fail naming the backend figure and the PC figure** when a peak is
    above its bound, e.g. `bytes 900 above the PC's metered 800`.
  - Equality is not required. The backend skips off-screen `spr` and fills
    before touching its caches, while the meter counts every call, and the
    cost model is an upper bound, so the backend peaks are expected at or
    below the PC's.
  - The header comment of the script names seven conditions.
- **`psx_check.sh`**: the `check_meter game-gpu-cost` call goes, and
  `game-gpu-cost` joins the loop over `psx_game_check.sh`'s conditions.
  `check_meter gpu-cost` (capture replay against the PC) stays as it is. The
  count stays at H-3-3's figure.
- **`psx_full_game_check.sh`**:
  - **`gpu-draw-meter` condition** after `gpu-draw-filter`: ok on
    `psx-replay: draw-meter off`. Otherwise it fails with
    `expected psx-replay: draw-meter off, got: <line or (no line)>`.
  - **Judged state:** it writes `gpu-state/judged_state.txt`. The file is the
    GPU boot's MIPS state without its `zone gpu_frame`/`cap gpu_*` lines,
    followed by those lines taken from the PC state. It is passed to
    `tools/psx_frame_budget.sh` in place of `gpu-state/psx_state.txt`, and
    `psx_frame_budget.sh` itself does not change.
  - **`measurements.txt`:** the `gpu zone gpu_frame`/`gpu cap gpu_*` lines
    come from the PC state. A new line, `gpu gpu-peak bytes <b> cluts <c>
    patterns <p>`, comes from the GPU TTY's `psx-replay: gpu-peak` line when
    there is one.
  - **Messages:** the header comment and the final line say fourteen.
  - **Why the PC state:** it is the plan's named source ("the PC replay
    keeps the meter for `make state`"). It always exists when the script
    runs, because the Makefile stops if the PC replay fails, and `gpu-cost`
    proves it equal to the MIPS meter. The CPU-path PSX replay's meter lines
    were considered and rejected as the source: that boot can fail
    independently, and a failed boot would silently drop the lines.
- **Probe sanity patch.** `psx/probes/call-twice-cart-draw.patch` is
  regenerated against the new loop, still a second `cart_draw()` after the
  first. It is checked with `git apply --check` on a clean tree, without
  booting. Its context already went stale in step 1, which renamed the
  filter calls.
- **No change** to:
  - `state_format`, `draw_meter.c`/`.h`;
  - the PC replay, `frame_profile`, `psx_replay.c`, `psx_largest_slots.c`;
  - `psx_frame_budget.sh`, `psx_state_compare.sh`, `psx_short_run.sh`;
  - captures, ceilings, references, e2e replays, game code.

  The meter's code stays linked in `psx-game` and the measured replays,
  because `state_format` reads its getters. Only its per-call work and its
  frame end go. The text drop is therefore smaller than the report's
  −2152 bytes, and the note records the actual figure.
- **Measurement.** It follows the plan's lever rule, asserts off at O2, both
  read from the short runs' output.
  - **Before:** at the step-3 commit, before any edit, one
    `make psx-short-runs` with no variable (all six runs). Each run's
    `analysis/` and printed lines are copied to `tmp/psx/plan21-step4/before/`.
  - **After:** the same command after Chunk 2.
  - **Note:** the per-run window update/draw/submit sums before and after;
    the draw delta per run and per frame on `draw-5564` and `draw-6000`
    against the report's −211.3 / −198.5 a frame; the `representative`
    verdicts (expected to fail on the draw runs, not gating); the ELF
    text/data/bss of `psx-gpu-replay` before and after.
  - **Expected differences from plan 20's experiment:** the delta can come
    out slightly different from plan 20's. That experiment kept the meter's
    frame end and the draw filter (step 1 has since removed the filter), so
    this step also gains about 0.13 a frame of submit.
  - **Not a gain:** if the draw delta on its target runs is not a gain, the
    lever is reverted and noted, per the plan's rule. The backend peaks and
    the meter switch are a reusable facility and stay.
- **Proof runs** (all must hold before the final commit):
  - `make test-all`, every `tests/e2e/*.sha256` unchanged.
  - `make psx-check`: every condition holds, with `gpu-cost` ok and
    `game-gpu-cost` reading `ok, bytes ... of ... metered, ...`. Every
    capture is unchanged. The `gpu zone gpu_frame`/`gpu cap gpu_*` lines of
    `measurements.txt` equal the last run's before this step, and the note
    quotes them.
  - `make psx-short-runs`, no variable: every boot ok; `opt O2`;
    `asserts ok off in 3 boots`; `done`, `deterministic`, `breakdown` hold.
  - `make psx-full-game-check`, no variable: every condition but
    `frame-budget` holds, `gpu-draw-meter` included. Its `gpu zone
    gpu_frame` line reads `65028` peak of `73728`, and `gpu_cluts`/
    `gpu_patterns` equal step 2's. The note gives `frame-budget`'s frames
    over, worst and mean against step 2's (6,047 / 9,392 / 765) and the
    backend peak against the metered peak. Its `frames.txt` replaces
    `psx/perf/full-run-frames.txt` in a commit of its own, as step 2 did.
- **Docs.**
  - `docs/psx-game.md`:
    - the GPU replay paragraph (meter only under the switch, the two new
      lines, which byte limit it hands over);
    - the draw-filter build paragraph ("between the GPU backend and the
      meter" when both are built);
    - the CPU-path replay paragraph is unchanged;
    - the Game EXE paragraph (no meter, buffer-size limit, `gpu-peak` line);
    - the `psx-check` condition list (`game-gpu-cost` among the game
      conditions with its new meaning; `gpu-cost` unchanged);
    - the game condition count;
    - the `psx-full-game-check` condition list and count (`gpu-draw-meter`;
      `zone-sizes`/`cap-sizes` take `gpu_*` from the PC state);
    - the `measurements.txt` paragraph (PC-sourced `gpu_*` lines, the
      `gpu gpu-peak` line);
    - the probe paragraph's "one constant for the backend's overrun check,
      the meter and the `gpu_frame` zone".
  - The Makefile legend, per user story 41.
  - `docs/psx-perf-levers.md`: only B1's Cost sentence. No figure changes.
- **Step note figures** go in the commit message of Chunk 2 and of the
  table commit:
  - the before/after short-run sums and deltas;
  - the asserts mode and opt level;
  - the ELF sizes and `ram-room`/`game-ram-room` bytes left, before and
    after;
  - `psx-check`'s count and its `gpu`/`game-gpu-cost` lines;
  - `psx-full-game-check`'s conditions, summary and gpu lines.

## Implementation Chunks

0. **Baseline, no commit.** At the step-3 commit with a clean tree, run
   `make psx-short-runs` with no variable and save the outputs to
   `tmp/psx/plan21-step4/before/`. Record `psx-gpu-replay`'s ELF sizes and
   the last `psx-check` `measurements.txt` (run `make psx-check` if none is
   left from step 3).
1. **The shipped game unmetered, bounded by the PC.**
   - The backend peaks and the byte-limit rename in `psx_gpu`.
   - `psx-game` without the meter, printing `gpu-peak`.
   - `game-gpu-cost` in `tools/psx_game_check.sh`, driven by
     `tests/test_psx_game_check.c`.
   - `psx_check.sh` reads it from the loop instead of `check_meter`.
   - The GPU replay passes its metered bytes under the new limit name, with
     no other change yet.
   - Then `make test-all` and `make psx-check`: every condition green,
     captures unchanged, `gpu` meter lines unchanged. Commit.
2. **The measured replays unmetered.**
   - The CMake option, the Makefile variable and flags, and the GPU replay's
     switch, `draw-meter` and `gpu-peak` lines.
   - `gpu-draw-meter` and the PC-sourced judged state and measurements in
     `tools/psx_full_game_check.sh`, driven by
     `tests/test_psx_full_game_check.c`.
   - The probe patch refresh, the docs and the legend.
   - Then `make test-all`, `make psx-check`, `make psx-short-runs`. Commit
     with the short-run figures.
3. **Judge run.** `make psx-full-game-check` with no variable. Every
   condition but `frame-budget` is green. Commit the new
   `psx/perf/full-run-frames.txt` with the summary and gpu lines in the
   message.

## Testing Decisions

- A good test drives a check script through its command line on crafted
  TTYs and states, and checks its printed verdicts, its files and its exit
  status, never its internals. No test boots an emulator.
- `tests/test_psx_game_check.c` (prior art: its crafted TTYs built from
  `tests/e2e/pad_title.json`, and the real `tmp/replay` writing the PC
  state):
  - the TTY helper writes `psx-game: gpu-peak bytes 100 cluts 0 patterns 0`
    before the done line, a figure below any real PC peak;
  - a passing boot prints `psx-game-check: game-gpu-cost: ok, bytes 100 of
    <n> metered, ...`;
  - a TTY whose byte peak is above the PC's metered peak fails naming both
    figures, and the same holds for CLUTs above `cap gpu_cluts`;
  - a TTY without the line fails with `(no line)`;
  - a broken pad log, which fails `game-pad-log`, leaves `game-gpu-cost`
    failed with `skipped`;
  - the six other conditions keep their verdicts in each case.
- `tests/test_psx_full_game_check.c` (prior art: its stub pcsx-redux and
  `FILTER_OFF` header, and step 1's `gpu-draw-filter` cases):
  - the GPU header gains `psx-replay: draw-meter off`, and the GPU state's
    `gpu_*` lines become zeros (`zone gpu_frame 0 0 200`, zero cap peaks)
    while the PC state keeps real ones;
  - a passing run reports `gpu-draw-meter: ok`, fourteen conditions in
    `conditions.txt` order, and `measurements.txt`'s `gpu zone gpu_frame`/
    `gpu cap gpu_*` lines equal the PC's, not the zeros;
  - `draw-meter on` and a missing line each fail `gpu-draw-meter` alone;
  - a PC state whose `gpu_frame` peak is above its limit fails `zone-sizes`
    naming it, which proves the judge reads the PC's line;
  - a GPU TTY with a `psx-replay: gpu-peak` line puts `gpu gpu-peak ...` in
    `measurements.txt`.
- No host test for `psx_gpu_peaks`, the CMake option, the Makefile flags or
  the replay's lines. `psx_gpu.c` builds only against the PSX SDK. They are
  proven by `game-gpu-cost` and `gpu-cost` on the real `psx-check` boots, by
  `gpu-draw-meter` on the judge run, and by the short runs' draw drop.
- `make test` and `make e2e` get no new case and no slower step. Both
  changed tests are in `PSX_TOOLS_TESTS`, run by `psx-tools-test`, which is
  a prerequisite of the PSX checks and not part of `make test`.

## Out of Scope

- `-O3` (step 5), map culling (step 6), the end-of-phase judge run
  (step 7). This step's judge run is its own verification and does not
  replace step 7.
- Removing the meter from `psx-check`'s capture replays, the CPU-path PSX
  replay, `psx-largest-slots`, the PC replay or the host frame profiler.
- Changing `state_format`, the meter, or what an unmetered state prints.
- Unlinking the meter's code from `psx-game`.
- A `draw-meter` verdict in `tools/psx_short_run.sh`. The short runs and the
  probe hard-code `OFF`, so no variable can be forgotten.
- Changing `lever-estimate`/`frame-profile-*`, the report's figures, or any
  capture, ceiling, reference or e2e hash.

## Further Notes

- Plan 20's B1 entry says the `gpu_*` lines "must come from the backend's
  own counts". The plan's verification asks for "the same zone and cap
  figures". The backend's byte count is lower than the metered one, since
  the cost model is an upper bound. This PRD reconciles the two:
  - the judged and recorded figures come from the PC meter, so they are the
    same as before;
  - the backend's own counts become the PSX-side check on the shipped game
    (`game-gpu-cost`) and a recorded measurement on the judge.
- After this step, the per-frame proof that the cost model bounds the real
  buffer runs only in `psx-check`'s capture replay, on `pad_slipway`. The
  shipped game and the judge get a peak-level bound (`game-gpu-cost`, and
  the recorded `gpu-peak` beside the metered peak). `packet`'s overflow
  check still ends any frame too large for the buffer.
