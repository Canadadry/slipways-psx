---
title: "Draw filter compiled only under a define in the GPU replay"
description: "The judge's GPU replay runs the draw family filter on every call although the shipped game has none, so it reads about 16% of draw high."
status: needs-triage
---

## Problem Statement

The PSX judge (`make psx-full-game-check`, the frame budget of 263 hblanks) and
the short runs time `psx-gpu-replay`, which wraps the GPU backend in the draw
family filter. The filter runs the draw cost model a second time on every draw
call (plan 20's report, lever F1: 26.67% / 16.26% / 21.28% of draw on the
measured rows). The shipped `psx-game` never installs it: its only job is the
family masks of `psx-check`'s GPU captures. So every judge figure carries a
cost the player never pays, and every lever later in plan 21 would be measured
against a draw that is too high.

## Solution

The filter becomes a compile-time option of the GPU replay. Without it, the
replay installs only the GPU backend and the meter, as before minus the
filter; with it, the replay is exactly today's. The option is off by default:
the judge, the short runs and the probe builds measure without it. `psx-check`
(and `psx-gpu-capture-report`, which shares its build) turns it on, so every
capture keeps its family mask and its 0-pixel reference. The GPU replay says
on its TTY which way it was built, and the judge refuses a boot built with the
filter. One measurement with the probe tool gives F1's actual figure.

## User Stories

1. As the plan pilot, I want the judge's GPU replay built without the draw filter, so that its draw figures match what the shipped game costs.
2. As the plan pilot, I want the short runs built without the filter by default, so that every lever of plan 21 is measured against the shipped draw.
3. As the plan pilot, I want the probe builds (`psx-probe`, `psx-probe-baseline`) built without the filter, so that their deltas are deltas of shipped code.
4. As the plan pilot, I want `psx-check`'s GPU replay built with the filter, so that its masked captures still draw only their families.
5. As the plan pilot, I want every `psx-check` capture unchanged, so that the 0-pixel references in `psx/gpu_capture_references.txt` still hold without re-acceptance.
6. As the plan pilot, I want `psx-check` to hold all its thirty-one conditions with the filter build, so that the step changes nothing it checks.
7. As the plan pilot, I want `psx-gpu-capture-report` to keep working, so that a capture can still be inspected alone.
8. As the plan pilot, I want the GPU replay to print whether it was built with the filter, so that a TTY log says what was measured.
9. As the plan pilot, I want `psx-full-game-check` to fail a condition when its GPU boot reports the filter on, so that a judge figure can never silently include it again.
10. As the plan pilot, I want a filter-less replay asked for a masked capture to stop with a clear fatal naming the option, so that a misconfigured capture never yields an unmasked picture that passes for a masked one.
11. As the plan pilot, I want a filter-less replay to still serve an "all" capture, so that the frame can be captured without the option when no mask is needed.
12. As the plan pilot, I want the breakdown replay (`psx-gpu-replay-breakdown`) to follow the same option as the plain one, so that the breakdown boot of a short run times the same code as its plain boots.
13. As the plan pilot, I want the shipped `psx-game` untouched, so that nothing the player runs changes.
14. As the plan pilot, I want every e2e hash unchanged, so that the PC game is provably the same.
15. As the plan pilot, I want the measured draw delta of removing the filter on `draw-5564` and `draw-6000`, so that F1's actual figure is recorded against the report's upper bound.
16. As the plan pilot, I want the delta taken asserts off, with its opt level stated, so that it follows the plan's measuring rules.
17. As the next step (step 2), I want `psx-full-game-check` to build the GPU replay without the filter with no extra variable, so that the new judge baseline is the shipped draw.
18. As a reader of `docs/psx-game.md`, I want the GPU replay section to say the filter is a build option, on only for `psx-check`, so that the doc stays true.
19. As a reader of the Makefile legend, I want `psx-build`, `psx-check`, `psx-full-game-check` and `psx-short-runs` to say which way they build the filter, so that the targets explain themselves.
20. As a developer, I want the PC `tmp/replay` capture path unchanged, so that the PC frames of the captures still come from the filter.

## Implementation Decisions

- **Define.** One preprocessor define, `PSX_GPU_REPLAY_DRAW_FILTER`, named after the existing `PSX_GPU_REPLAY_BREAKDOWN`. In the GPU replay's main file, the filter's install, its per-frame mask set and its frame end are compiled only under it. Without it, the backend chain is GPU backend wrapped by the meter only.
- **Masked capture without the filter.** Without the define, a capture request whose mask is not `DRAW_FILTER_ALL` is a `p8_fatal` whose message names the define. A request with `DRAW_FILTER_ALL`, or no request (the empty capture script's `PSX_CAPTURE_NO_FRAME`), proceeds. The capture handshake itself stays compiled in both ways: the judge and the short runs drive it with an empty script.
- **TTY line.** The GPU replay prints `psx-replay: draw-filter on` or `psx-replay: draw-filter off` right after `psx-replay: backend gpu`. Existing parsers read only lines they name (`asserts`, `format`, `done`, `capture`), so an extra `psx-replay:` line breaks none of them.
- **Draw filter source stays linked.** `draw_filter.c` stays in the GPU replay's source lists: the replay needs its header for `DRAW_FILTER_ALL` and the mask type, and its unused text costs nothing per frame. Only the calls are under the define. The ELF text delta is reported by the probe, not gated.
- **CMake option.** `psx/CMakeLists.txt` gains `option(PSX_GPU_REPLAY_DRAW_FILTER ... OFF)`; when on, it adds the compile definition to both `psx-gpu-replay` and `psx-gpu-replay-breakdown`. `psx-game`, `psx-replay`, the pad probe and `psx-largest-slots` never get it.
- **Makefile wiring.** CMake caches options per build directory, so every caller passes the option explicitly, as `PSX_MEASURED_REPLAY_ASSERTS` is today:
  - `psx-build` passes `-DPSX_GPU_REPLAY_DRAW_FILTER=$(PSX_BUILD_DRAW_FILTER)`, a variable defaulting to `ON`: `psx-build` is `psx-check`'s and `psx-gpu-capture-report`'s build.
  - `psx-full-game-check` calls `psx-build` with `PSX_BUILD_DRAW_FILTER=OFF`, whatever `PSX_FULL_GAME_ASSERTS` is.
  - `psx-short-runs` and `psx-probe-baseline`/`psx-probe` pass `-DPSX_GPU_REPLAY_DRAW_FILTER=OFF`. (The probe baseline built from a commit before this step ignores the unknown variable with a CMake warning and installs the filter unconditionally, which is exactly the "before" of the measurement below.)
  - No user-facing variable to turn the filter on in the judge or the short runs: the plan's locked decision is "off in the measured replay".
- **Judge condition.** `tools/psx_full_game_check.sh` gains a thirteenth condition, `gpu-draw-filter`: ok when the GPU boot's TTY has `psx-replay: draw-filter off`; failed, quoting the line or "(no line)", otherwise. It sits after `gpu-state-game-lines` in `conditions.txt`. Its usage text, final line ("all thirteen conditions hold") and the Makefile legend follow.
- **`psx-check` gets no new condition** (QUESTION settled here, per the plan's rules: no human, smallest change that guards the decision). Its seven `gpu-capture-*` conditions already prove the filter is in: a filter-less build would hit the masked-capture fatal on the first family capture and fail `gpu-replay-done`. It stays at thirty-one conditions.
- **Short runs get no new verdict** for the same reason of scope: they always pass `OFF` explicitly, and their `asserts` verdict is the prior art should a check ever be wanted.
- **Measurement (F1's actual figure).** Before committing, with the toolchain and emulator installed:
  1. `make psx-probe-baseline PSX_SHORT_RUNS="draw-5564 draw-6000"` on HEAD (filter installed unconditionally, asserts off, O2).
  2. The step's uncommitted diff of `src/` and `psx/` written to a patch in the scratchpad, then `make psx-probe PATCH=<that patch> PSX_SHORT_RUNS="draw-5564 draw-6000" PSX_PROBE_LABEL=f1-filter-off`.
  3. Both runs must be valid and report game-state same. The window draw delta per frame (total over 15 frames, divided by 15) must be negative on both runs. The report's upper bound is −211.3 a frame on `draw-5564` and −198.5 on `draw-6000` (no-meter delta, "of the order of, and below").
  4. The figures go in the commit message, for the plan's step note: draw, update, submit and total deltas per run, per frame and over the window, the ELF text delta, "asserts off, O2" (step 1 precedes the O3 rule of step 5), and the bound each is compared with.
  5. A delta that is not a gain fails the step (the filter's removal cannot add cost: look for a wiring error). A gain larger than the bound is not a failure: the plan does not revert the filter decision, which is locked; the commit message flags it as above the report's bound, with the per-run figures, and the step goes on.
- **Docs.** `docs/psx-game.md`'s "GPU replay" section says the filter is installed only in the `PSX_GPU_REPLAY_DRAW_FILTER` build (on in `psx-check`'s build, off in the judge, short runs and probe), that a masked capture without it is fatal, and gives the new TTY line; its submit paragraph says the filter's frame end only exists in that build. The judge's conditions list gains `gpu-draw-filter`.

## Implementation Chunks

1. **Judge condition (host, TDD).** `gpu-draw-filter` in `tools/psx_full_game_check.sh`, driven red-green from `tests/test_psx_full_game_check.c`: the fake GPU TTYs of the existing tests gain `psx-replay: draw-filter off`; new tests for a boot saying `on` and a boot with no line, each failing only that condition. Thirteen conditions everywhere the count is written.
2. **Replay and build wiring.** The define in the GPU replay (guarded calls, masked-capture fatal, TTY line), the CMake option, the Makefile variables and explicit `-D` in `psx-build`, `psx-full-game-check`, `psx-short-runs`, `psx-probe-baseline`/`psx-probe`, the Makefile legend and `docs/psx-game.md`. Checked by `make test-all` (unchanged e2e hashes) and `make psx-build` compiling every target both ways (`PSX_BUILD_DRAW_FILTER=ON` and `OFF`), warnings as errors.
3. **PSX verification and measurement.** `make psx-check` green with every capture unchanged (no ceiling or reference file touched), then the probe measurement above, figures into the commit message.

## Testing Decisions

- A good test drives the check script through its command line with fake TTY files and asserts on its printed verdicts and `conditions.txt`, never on its internals, as `tests/test_psx_full_game_check.c` already does.
- Tested by host unit tests: `tools/psx_full_game_check.sh`'s new condition (prior art: the twelve-condition tests and the `frame-budget failed` test in `tests/test_psx_full_game_check.c`; the asserts-mode checks of `tests/test_psx_short_run.c` and `tests/test_psx_probe.c` for reading a `psx-replay:` mode line).
- Not unit-tested: the GPU replay's guarded calls and the CMake/Makefile wiring. They compile only with PSn00bSDK and run only in pcsx-redux; `psx-check` (filter on, masked captures at 0 pixels) and the probe run (filter off, valid, game-state same) are their tests. No PSX check enters `make test`, which must not get slower.
- `make test-all` must stay green with every `tests/e2e/*.sha256` unchanged: the PC side is not touched.

## Out of Scope

- The new judge baseline (`psx-full-game-check` with the filter off, replacing `psx/perf/full-run-frames.txt`): step 2.
- Removing the draw meter (B1), asserts off in `psx-game` (G1), `-O3` (G2): phase 2.
- The PC `tmp/replay` capture path and `src/devtools/draw_filter.c` itself: unchanged.
- Any change to `psx/gpu_captures.txt`, `psx/gpu_capture_ceilings.txt` or `psx/gpu_capture_references.txt`.
- A `psx-check` condition or short-run verdict on the filter mode (decided against above).

## Further Notes

- The PSn00bSDK toolchain and pcsx-redux were being installed into `third_party/` while this PRD was written; chunks 2's PSX build and 3 need them in place (`make psn00bsdk-toolchain-setup`, `make psx-emulator-setup` report a complete install).
- The report's F1 entry also counts the filter's share of submit (its frame end: about 0.12 hblanks a frame); the probe's submit delta is recorded beside the draw delta but carries no condition.
- The `representative` short-run condition is not involved: the measurement uses the probe tool, which compares against its own baseline boot.
