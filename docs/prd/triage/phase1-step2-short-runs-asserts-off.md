---
title: "psx-short-runs: an asserts-off option and the short-run baseline"
description: "The PSX measuring loop only builds the replays with asserts on and never prints a window's update and submit sums, so the asserts-off baseline every later lever is measured against cannot be taken."
status: needs-triage
---

## Problem Statement

Plan 20 measures every lever against an **asserts-off** baseline, because asserts are already decided to go off in the shipped PSX game and in the measured GPU replay. Today `make psx-short-runs` can only build the measured replays (`psx-gpu-replay` and `psx-gpu-replay-breakdown`) with asserts on: the PSX CMake project forces `-UNDEBUG` on every target, after the SDK's release configuration defines `NDEBUG`.

Two more things are missing:

- The per-run **window sums of update, draw and submit** are not printed when the run is used the way the plan uses it (`PSX_SHORT_RUN_REFERENCE=` empty). Window sums of draw and update only appear inside the `representative` verdict, which needs a reference table, and submit never appears.
- Nothing proves which assert mode a boot actually ran. The replays already print `psx-replay: asserts on|off`, but the analysis ignores that line, so a stale build directory or a wrong flag could produce an asserts-on baseline that is recorded as asserts-off.

There is also a trap: the core does not build with `NDEBUG` under `-Werror`. A host compile of the core with `-DNDEBUG -Wall -Wextra -Werror` fails with about fifteen unused-variable, unused-but-set-variable, unused-function and unused-parameter errors (helpers and locals that only exist for an `assert`, in `game/`, `engine/` and `devtools/`). The plan forbids changing game or shipped-path code, so this cannot be fixed in the sources.

## Solution

`make psx-short-runs` gets an option, `PSX_SHORT_RUN_ASSERTS=on|off` (default `on`, so today's behavior is unchanged). With `off`, the two measured replays are compiled without asserts, the unused-* warnings that only asserts silence stop being errors for them, and nothing else changes: `psx-build`, `psx-check`, `psx-game` and the PC build keep asserts on.

The per-run analysis gains:

- a fifth condition, `asserts`, that checks every boot's `psx-replay: asserts` line against the requested mode;
- one summary line with the window sums of update, draw and submit from the plain boot, printed with or without a reference table.

Then one asserts-off run of all five short runs (`PSX_SHORT_RUN_ASSERTS=off PSX_SHORT_RUN_REFERENCE= make psx-short-runs`) is taken. When every run is `done`, `deterministic` and `asserts ok off`, its window sums are written into a new `docs/psx-perf-levers.md` as the short-run baseline.

## User Stories

1. As the plan-20 investigator, I want `make psx-short-runs` to build the measured replays without asserts on request, so that every lever is measured against the build that will actually ship.
2. As the plan-20 investigator, I want the option to default to asserts on, so that step 11 can measure the assert lever itself by running the same target both ways.
3. As the plan-20 investigator, I want an invalid value of the option refused before anything is built, so that a typo never yields a silently asserts-on baseline.
4. As the plan-20 investigator, I want each run's analysis to fail when a boot's TTY reports a different assert mode from the one requested, so that a stale build cannot pass as a baseline.
5. As the plan-20 investigator, I want the analysis to fail when a boot's TTY has no `psx-replay: asserts` line at all, so that a broken boot is not mistaken for the right mode.
6. As the plan-20 investigator, I want the asserts verdict to name the boot and the mode it reported, so that I can see at a glance which boot was wrong.
7. As the plan-20 investigator, I want the window sums of update, draw and submit printed for every run, so that I can record the baseline without a reference frame table.
8. As the plan-20 investigator, I want those sums taken from the plain boot, not the breakdown boot, so that the baseline carries no timing overhead.
9. As the plan-20 investigator, I want the window-sums line printed even when the breakdown boot is missing or its summary is unavailable, so that a breakdown problem does not hide the plain numbers.
10. As the plan-20 investigator, I want the window-sums line to say plainly when the plain boot lacks a window frame, so that a partial sum is never recorded.
11. As the plan-20 investigator, I want the asserts-off build to keep `-Werror` for every warning except the unused-* family, so that a real warning still stops the build.
12. As the plan-20 investigator, I want the asserts-off option to leave the game sources untouched, so that the plan's guardrail (no game or shipped-path change) holds.
13. As the maintainer, I want `psx-build`, `psx-check` and `psx-full-game-check` to build exactly as before, so that their `asserts-on` conditions and every recorded figure stay valid.
14. As the maintainer, I want `make test` and `make e2e` untouched and no slower, so that the fast loop stays fast.
15. As the maintainer, I want the new analysis behavior host-tested in the existing `psx-tools-test` test of the short-run script, so that it is checked without an emulator.
16. As the plan-20 investigator, I want the target's help text in the `Makefile` legend to name the option and its default, so that the next step's probe tool knows how to request it.
17. As the plan-20 investigator, I want `docs/psx-perf-levers.md` to hold, per short run, the window frames and the window sums of update, draw and submit, so that every later delta has a written reference.
18. As the plan-20 investigator, I want the baseline section to state the commit, the exact command and the assert mode it was taken with, so that it can be reproduced and compared.
19. As the plan-20 investigator, I want the baseline section to quote each run's `done`, `deterministic` and `asserts` verdicts, so that the step's verification is visible in the report.
20. As the plan-20 investigator, I want a one-off check that the asserts-off boots play the same game as the PC, so that an assert with a side effect is caught before it skews every measurement.
21. As the human reading the report, I want the baseline labeled as relative-only short-run figures, so that I do not read them as the judge's full-run numbers (those come at step 5).

## Implementation Decisions

- **Option name and values.** A `Makefile` variable `PSX_SHORT_RUN_ASSERTS`, default `on`, accepted values `on` and `off`. Any other value makes `psx-short-runs` fail with a one-line message before the cut or the build. Default `on` was chosen (instead of flipping the default to `off`) because step 11 measures asserts off vs on with this same target, and an opt-in flag cannot change any existing run by surprise. Step 3's probe tool passes `off` explicitly.
- **CMake side.** The PSX CMake project gets one cache option for the measured replays' assert mode, default on. `psx-short-runs` passes it on every configure, so switching modes in the same `<name>/build` directory reconfigures and rebuilds; no separate build directory is needed. When off, only `psx-gpu-replay` and `psx-gpu-replay-breakdown` lose `-UNDEBUG` (the SDK's release `NDEBUG` then applies) and gain `-Wno-error=` for `unused-variable`, `unused-but-set-variable`, `unused-function` and `unused-parameter`. Every other target, and every target when the option is on, is compiled exactly as today. `psx-build` does not pass the option, so `psx-check`'s `asserts-on` condition is untouched. The link options (the `_assert_abort` wrap) stay as they are; an unreferenced wrap is harmless.
- **Why not fix the warnings in the sources.** The plan's guardrail forbids game and shipped-path changes, and the "Done when" list allows only the phase-1 harness as committed code. Demoting four warning kinds from errors to warnings, for the asserts-off measured build only, keeps the sources untouched; the warnings stay visible in the build log.
- **The `asserts` condition.** The short-run analysis script takes one more required argument, the expected mode (`on` or `off`), and writes a fifth verdict line, after `breakdown`, so the existing order (done, deterministic, representative, breakdown) is kept and only extended. It reads the first `psx-replay: asserts <mode>` line of each of the three TTYs (carriage returns stripped, as for every other line). Verdicts: `asserts ok <mode> in 3 boots`, or `asserts failed <boot> says <mode>` / `asserts failed <boot> has no asserts line` / `<boot> tty missing`, several reasons joined with `; ` like `done`. A bad expected value is a usage error (exit 1). Since every `failed` verdict already fails the run in the `Makefile` loop, nothing else changes there besides passing the argument.
- **The window-sums line.** `summary.txt` gains, right after its first `window run frames ...` line, `window plain update <U> draw <D> submit <S>`: the sums over the run's window frames of the plain-1 boot's `psx-frame:` update, draw and submit fields. It does not depend on the reference table nor on the breakdown boot. If plain-1 lacks a window frame, the line is `window plain unavailable: plain-1 has no frame <r>`. It is printed before the existing `summary unavailable` early exit, so a breakdown failure no longer hides it. Plain-1 is enough: `deterministic` already proves plain-2 identical.
- **Game-state check.** A one-off, non-gating check, done once while taking the baseline: each run's plain-1 TTY `psx-state:` game lines are compared with the PC state the cut already wrote for that run, using the repo's existing PSX-versus-PC state comparison. Its result is recorded in the baseline section. If any run differs, the step stops: an assert has a side effect, and the plan's baseline cannot be taken until a human decides. This check is not made a sixth condition, because step 3's probe tool owns game-state comparison and the plan's verification for this step is `done` + `deterministic`.
- **Reference.** The baseline run uses `PSX_SHORT_RUN_REFERENCE=` empty, as the plan's Rules say; `representative` then reads `skipped`, which is expected and is recorded as such.
- **The report file.** `docs/psx-perf-levers.md` is created by this step. It holds only what is true now: a title, one line on what the file will become (plan 20's ranked lever inventory), and a "Short-run baseline (asserts off)" section with: the commit and command; one row per run (`start`, `update-4737`, `update-5500`, `draw-5564`, `update-5719`) giving the window's full-game frames, frame count, and window sums of update, draw, submit and their total, in hblanks; the `done`, `deterministic`, `representative` (skipped) and `asserts` verdicts; the game-state check's result; and a note that these figures are for before/after comparison on the short runs only. Nothing about levers yet, and no asserts-on figures (that is step 11).
- **Help text.** The `psx-short-runs` entry in the `Makefile` legend names `PSX_SHORT_RUN_ASSERTS=on|off`, its default, the `asserts` condition and the window-sums line, in its existing style.

## Implementation Chunks

1. **Analysis script: window sums and the `asserts` condition.** Add the expected-mode argument, the `asserts` verdict and the `window plain ...` summary line to the short-run analysis script, test-first in its host test. Update the `Makefile` call to pass the mode (`on` for now, since the variable does not exist yet, or the variable with its default if added in the same chunk). `psx-tools-test` green.
2. **Build option.** Add `PSX_SHORT_RUN_ASSERTS` to the `Makefile` (validation, help text, passing the CMake option and the expected mode to the script) and the CMake option for the two measured replays. Verified on the host side by `psx-tools-test` and by `make test-all` staying green and unchanged; on the real toolchain by one `PSX_SHORT_RUNS=start` run with `off` that builds and reports `asserts ok off in 3 boots`.
3. **Baseline.** Run all five short runs with `PSX_SHORT_RUN_ASSERTS=off PSX_SHORT_RUN_REFERENCE=`, do the one-off game-state check, and write `docs/psx-perf-levers.md` from `analysis/verdicts.txt` and `analysis/summary.txt` of each run. This chunk needs the PSn00bSDK toolchain and pcsx-redux (`make psn00bsdk-toolchain-setup`, `make psx-emulator-setup`); it has no test of its own.

## Testing Decisions

- A good test drives the analysis script as a black box, through its command line, on hand-written TTYs, map and reference files, and asserts only on its exit code and the lines of `verdicts.txt` and `summary.txt`. No test looks at the awk internals.
- Tested: the short-run analysis script, in its existing host test (`psx-tools-test`, outside `make test`, so the fast loop is not slowed). New cases:
  - asserts on expected, all three TTYs say on → `asserts ok on in 3 boots`;
  - off expected, all three say off → `asserts ok off in 3 boots`;
  - off expected, one boot says on → `asserts failed <boot> says on`, exit 2;
  - one boot without the line → `asserts failed <boot> has no asserts line`;
  - a missing TTY → named in the `asserts` reason;
  - an expected value other than on/off, or a missing ninth argument → usage error, exit 1;
  - verdict order stays done, deterministic, representative, breakdown, asserts;
  - the window-sums line with the existing fixture's window (run frames 15 and 16: update 48 + 49, draw 1600 + 1500, submit 8 + 9 → `window plain update 97 draw 3100 submit 17`), with an empty reference;
  - the same line still printed when the breakdown TTY is missing (summary otherwise unavailable);
  - plain-1 missing a window frame → `window plain unavailable: plain-1 has no frame 16`.
  The existing fixtures already carry `psx-replay: asserts on`; the existing cases gain the expected-mode argument and must keep passing unchanged otherwise.
- Not unit-tested: the `Makefile` validation and the CMake option. They are thin wiring, and the end-to-end proof is the `asserts` verdict on a real boot (chunk 2's single run, then chunk 3's five runs), which reads the mode the replay itself reports.
- Prior art: the existing short-run analysis host test (hand-written TTYs, `replaced()` fixture edits, `verdict_failed()` helper), and the other `psx-tools-test` script tests such as the state-compare and frame-budget ones.
- `make test-all` must stay green and every `tests/e2e/*.sha256` unchanged; nothing in this step touches what they cover.

## Out of Scope

- Turning asserts off in `psx-game`, `psx-build`, `psx-check` or `psx-full-game-check` (the latter is step 5's milestone, which may reuse this option's CMake side but is not built here).
- Measuring asserts off versus on (step 11).
- The probe tool and its noise floor (step 3).
- Any lever, any game or shipped-path source change, including silencing the unused-* warnings in the sources.
- Fixing the short runs' `representative` failure.
- Running anything on PCSX-ReARMed or the Miyoo; launching the raylib app.

## Further Notes

- Questions settled without a human, in line with the plan's Rules: default `on` (opt-in flag); warnings demoted for the asserts-off measured build only rather than fixed in the sources (guardrail); the assert mode is checked from the replay's own TTY line rather than trusted from the flag; the game-state check is one-off and recorded rather than a new gating condition (step 3 owns state comparison).
- The host compile that revealed the `-Werror` trap used the PC compiler at `-O2 -DNDEBUG`; the PSX GCC may report a slightly different set. If it reports a warning outside the four unused-* kinds, the chunk-2 implementer adds that kind to the demoted list only if it comes from assert-only code, and records it in the commit message.
- The baseline numbers are relative-only: plan 20 accepts that the short runs read about 15% off the full run and lose the update spikes after a resume.
