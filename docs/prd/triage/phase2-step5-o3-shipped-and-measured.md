---
title: "-O3 for psx-game and the measured replays"
description: "The shipped psx-game and the measured GPU replays compile at the SDK's -O2, while plan 20 measured -O3 (lever G2) at -5.9% of the pooled short-run total."
status: needs-triage
---

## Problem Statement

The PSn00bSDK's interface puts `-O2` at the end of every compile line, after
CMake's release `-O3`, and GCC takes the last `-O`. So every PSX target
compiles at `-O2`: `psx-game` (the program on the CD), the measured GPU
replays (`psx-gpu-replay`, `psx-gpu-replay-breakdown` as built by
`psx-full-game-check`, `psx-short-runs` and the probe), and `psx-check`'s
capture replays.

Plan 20 measured `-O3` on the measured replays (lever G2). It used the CMake
cache variable `PSX_MEASURED_REPLAY_OPT=O3`, which links an interface library
that adds `-O3` after the SDK's `-O2`. The measured per-window gains were:

| Run | Gain |
| --- | --- |
| `start` | −1382 |
| `update-4737` | −43 |
| `update-5500` | −165 |
| `draw-5564` | −683 (−45.5 a frame) |
| `update-5719` | −39 |
| `draw-6000` | −544 (−36.3 a frame) |

Pooled, that is update −13.8%, draw −3.4%, submit −3.1%, total −5.9%.

Nothing ships at `-O3` today. `psx-build` hard-codes
`-DPSX_MEASURED_REPLAY_OPT=O2`. `psx-game` has no opt switch at all.
`psx-short-runs` defaults to `O2`. The probe build never passes the
variable, so it gets whatever its CMake cache holds (`O2` by default).

From this step on, the plan says a lever figure not taken asserts off at
O3 is invalid. The harness must therefore measure at O3 by default, and
must show that it did.

## Solution

`psx-game` and the GPU replays are built at `-O3` everywhere they are
shipped or measured. They use the same mechanism as
`PSX_MEASURED_REPLAY_OPT`: an interface library linked after the SDK's, so
its `-O3` comes after the trailing `-O2`.

- **CMake:**
  - A new cache variable `PSX_GAME_OPT` (`O2|O3|Os`, default `O3`) sets
    `psx-game`'s level.
  - `PSX_MEASURED_REPLAY_OPT`'s default becomes `O3`.
  - Each level other than `O2` gets one interface library, shared by every
    target built at that level.
- **`psx-build`:**
  - It passes a new Makefile variable `PSX_BUILD_OPT` (`O2|O3|Os`, default
    `O3`) to both cache variables. This replaces the hard-coded `O2`.
  - After building, it reads `tools/psx_opt_level.sh` on `psx-game`,
    `psx-gpu-replay` and `psx-gpu-replay-breakdown`. It prints
    `psx-build: opt psx-game <l> psx-gpu-replay <l> psx-gpu-replay-breakdown <l>`
    and fails when one of them is not `PSX_BUILD_OPT`.
  - Every target built through `psx-build` therefore states and enforces its
    level: `psx-check`, `psx-game-check`, `psx-game-cd-check`,
    `psx-gpu-capture-report` and `psx-full-game-check`.
- **The measuring harness:**
  - `psx-full-game-check` passes `PSX_BUILD_OPT=O3` explicitly.
  - `PSX_SHORT_RUN_OPT` defaults to `O3`.
  - The probe builds pass `-DPSX_MEASURED_REPLAY_OPT=O3` explicitly. They
    check the level with `tools/psx_opt_level.sh`, and the level is added to
    the probe baseline's fingerprint.

The CPU-path `psx-replay`, `psx-pad-probe` and `psx-largest-slots` stay at
`O2`. They are neither shipped nor measured.

## User Stories

1. As a player, I want the shipped `psx-game` compiled at `-O3`, so that every frame spends less time in the game's update and draw.
2. As the plan pilot, I want the measured GPU replays compiled at `-O3`, so that the judge and the short runs measure the same code generation as the shipped game.
3. As the plan pilot, I want `-O3` to reach the compiler after the SDK's trailing `-O2`, through an interface library linked after the SDK's as `PSX_MEASURED_REPLAY_OPT` already does, so that the level asked for is the level used.
4. As the plan pilot, I want `psx-game`'s level behind its own CMake cache variable `PSX_GAME_OPT` (`O2`, `O3` or `Os`, default `O3`), validated like `PSX_MEASURED_REPLAY_OPT`, so that one variable decides it and no source edit is needed.
5. As the plan pilot, I want `PSX_MEASURED_REPLAY_OPT`'s CMake default to become `O3`, so that a configure that forgets the variable still measures at the plan's level.
6. As the plan pilot, I want a configure with any value other than `O2`, `O3` or `Os` in either cache variable to fail naming the variable and the value, so that a typo never builds silently at the SDK's level.
7. As the plan pilot, I want one interface library per level, shared by every target built at that level, so that the game and the replays use exactly the same mechanism.
8. As the plan pilot, I want `O2` to add nothing to the compile lines, as today, so that an `O2` build is byte-for-byte the build before this step and stays a valid "before" for a delta.
9. As the plan pilot, I want `psx-build` to take `PSX_BUILD_OPT` (`O2`, `O3` or `Os`, default `O3`) and pass it as both `-DPSX_GAME_OPT` and `-DPSX_MEASURED_REPLAY_OPT` on every configure, so that a cached value never leaves a build directory at the wrong level.
10. As the plan pilot, I want `PSX_BUILD_OPT` checked at parse time, like `PSX_BUILD_GAME_ASSERTS`, so that a bad value fails before anything builds.
11. As the plan pilot, I want `psx-build` to read the effective level of `psx-game`, `psx-gpu-replay` and `psx-gpu-replay-breakdown` with `tools/psx_opt_level.sh` after building, so that "O3 on every shipped and measured target" is checked, not assumed.
12. As the plan pilot, I want `psx-build` to print `psx-build: opt psx-game <level> psx-gpu-replay <level> psx-gpu-replay-breakdown <level>`, so that every build log (`psx-check`'s and the judge's included) says the level it measured.
13. As the plan pilot, I want `psx-build` to fail naming the target, the level read and the level asked for when they differ, or when the flags file is unreadable, so that a build at the wrong level never reaches a boot.
14. As the plan pilot, I want `psx-full-game-check` to pass `PSX_BUILD_OPT=O3` explicitly, as it passes `PSX_BUILD_DRAW_FILTER=OFF`, so that the judge is always at the plan's level whatever the caller's environment.
15. As the plan pilot, I want `PSX_SHORT_RUN_OPT` to default to `O3` (`O2` and `Os` still accepted), so that `make psx-short-runs` with no variable measures asserts off at O3.
16. As the plan pilot, I want the short runs' existing `opt <level>` line and per-target level check unchanged in form, so that the note can quote `opt O3` from them.
17. As the plan pilot, I want the probe builds (`psx-probe-baseline`, `psx-probe`) to pass `-DPSX_MEASURED_REPLAY_OPT=O3` explicitly, so that a probe build directory cached at `O2` cannot measure at the wrong level.
18. As the plan pilot, I want each probe build checked with `tools/psx_opt_level.sh`, with a run `invalid` when `psx-gpu-replay` does not read `O3`, so that a probe figure at the wrong level is never a delta.
19. As the plan pilot, I want the probe baseline's fingerprint to include the level, so that a baseline built at another level is stale.
20. As the plan pilot, I want `psx-check` green at O3 with every capture unchanged at 0 tolerance, so that the `-O3` code is proven to draw what the PC draws.
21. As the plan pilot, I want `psx-check`'s `ram-room` (GPU ELF at O3, asserts on, meter and filter in: the largest replay build) to still hold, with its bytes left recorded before and after, so that the +40 KB of text is shown to fit.
22. As the plan pilot, I want `game-ram-room` recorded before and after, so that the shipped game's growth at O3 is in the note.
23. As the plan pilot, I want `psx-check`'s game conditions (`game-done`, `game-pad-log`, `game-pad-states`, `game-state-game-lines`, `game-title-left`, `game-asserts`, `game-gpu-cost`, `game-cue`, `game-cd-exe`) green on the O3 CD game, so that the shipped game at O3 still plays what the PC plays.
24. As the plan pilot, I want the full game's GPU replay built at O3 once, without booting, and its RAM room computed, so that `psx-full-game-check`'s `gpu-ram-room` is known to fit before step 7's judge run.
25. As the plan pilot, I want a short-run baseline before any edit (O2, asserts off) and a short run after (no variable: O3, asserts off), so that G2's measured delta per part is in the note against the report's −5.9%.
26. As the plan pilot, I want the note to give per run and pooled the window update, draw and submit sums before and after, with each part's and the total's delta in hblanks and percent, so that each figure can be compared with the report's (update −13.8%, draw −3.4%, submit −3.1%, total −5.9%).
27. As the plan pilot, I want the note to record the asserts mode and opt level the runs reported (`asserts ok off in 3 boots`, `opt O3`), so that the figures are valid under the plan's rule.
28. As the plan pilot, I want the short runs' `representative` verdicts recorded and not gating, so that a window that moved by more than 10% since the last judge run does not void the figures.
29. As the plan pilot, I want the lever reverted (defaults back to `O2`) when the pooled window total is not a gain, while `PSX_GAME_OPT`, `PSX_BUILD_OPT` and the level checks stay, so that the plan's "not a gain" rule is honoured and the facility is kept.
30. As the plan pilot, I want a probe baseline taken at O3 after this step, so that the next probe compares against a baseline at the plan's level, and the probe build's level check is proven on a real build.
31. As the plan pilot, I want `psx-short-runs`, `psx-probe-baseline` and the game-check family never slower than now beyond the O3 compile itself, so that the harness stays usable.
32. As the plan pilot, I want any warning that `-O3` raises handled explicitly (a true positive fixed, a false positive demoted by kind only), so that the shipped and measured builds keep `-Werror` for everything else.
33. As the plan pilot, I want a behaviour change that appears only at O3 (a capture, a state line or a game condition that differs) treated as a bug with a proven cause, so that undefined behaviour in the source is fixed rather than masked by `-O2`.
34. As a developer, I want `make test-all` green with every `tests/e2e/*.sha256` unchanged, so that the PC game is provably untouched.
35. As a developer, I want `make test` and `make e2e` no slower, so that the fast loop stays fast.
36. As a developer, I want `tools/psx_opt_level.sh` and its host test unchanged, so that the one tool that reads the level is already proven on the SDK's real flag shape, O3 after O2 included.
37. As a reader of `docs/psx-game.md`, I want the build paragraph to say that `psx-game` and the GPU replays are built at `-O3` (`PSX_GAME_OPT`, `PSX_MEASURED_REPLAY_OPT`, `PSX_BUILD_OPT`) while the other targets keep the SDK's `-O2`, so that the doc describes the build as it is.
38. As a reader of `docs/psx-game.md`, I want the short-runs, probe and `psx-full-game-check` passages to say which level each builds at and where it is checked, so that a measured figure can be traced to its level.
39. As a reader of `docs/psx-game.md`, I want the doc to say that O3's gain rests on pcsx-redux's timing model (no instruction-cache miss or data wait), so that the Miyoo is not promised the judge's figure.
40. As a reader of the Makefile legend, I want `psx-build`, `psx-short-runs`, `psx-full-game-check`, `psx-probe-baseline` and `psx-probe` to say their level and the new `opt` line, so that `make` help stays true.

## Hypotheses on step 4

Step 4 is not committed when this PRD is written. Its plan of record is the
PRD "The draw meter leaves psx-game and the measured replays"
(`docs/prd/triage/phase2-step4-draw-meter-out.md`). Each assumption about
its outcome is listed below with what this step does if it turns out false.

- **H-4-1.** Step 4 is fully committed before this step starts: its three
  chunks, its judge run, and the new `psx/perf/full-run-frames.txt`. Its
  `frame-budget` is still red, so the plan goes on to step 5. If step 4 is
  not committed, this step waits for it. If `frame-budget` turned green,
  the plan skips to its last step and this PRD is not implemented.
- **H-4-2.** `psx-build`'s `cmake` line still passes
  `-DPSX_MEASURED_REPLAY_OPT=O2` literally. It now also passes
  `-DPSX_GPU_REPLAY_DRAW_METER=$(PSX_BUILD_DRAW_METER)` beside the draw
  filter. This step replaces the literal `O2` by `$(PSX_BUILD_OPT)` and adds
  `-DPSX_GAME_OPT=$(PSX_BUILD_OPT)` next to it. If the line was rewritten,
  the two flags go next to `-DPSX_MEASURED_REPLAY_ASSERTS`.
- **H-4-3.** `psx-full-game-check` calls `psx-build` with
  `PSX_BUILD_DRAW_FILTER=OFF PSX_BUILD_DRAW_METER=OFF` and the asserts
  mapping. This step appends `PSX_BUILD_OPT=O3` to that call.
- **H-4-4.** The `cmake` lines of `psx-short-runs` and of
  `psx-probe-baseline`/`psx-probe` pass `-DPSX_GPU_REPLAY_DRAW_METER=OFF`
  and `-DPSX_GPU_REPLAY_DRAW_FILTER=OFF`. The short runs still pass
  `-DPSX_MEASURED_REPLAY_OPT=$(PSX_SHORT_RUN_OPT)`, and the probe passes no
  opt flag. If the probe already passes one, this step only makes it `O3`
  and adds the check.
- **H-4-5.** `psx/CMakeLists.txt`'s opt block is as at `1fcfabe`:
  - `PSX_MEASURED_REPLAY_OPT`, default `O2`;
  - one `psx-measured-replay-opt` interface library linked to the two GPU
    replays when the level is not `O2`, with `-Wno-error`.

  Step 4 added only a `PSX_GPU_REPLAY_DRAW_METER` option and its definition
  block. If step 4 touched the opt block, this step applies its decisions
  to the new text.
- **H-4-6.** `psx-check` reports thirty-two conditions, with `ram-room` and
  `game-ram-room` printing their bytes left. `psx-full-game-check` reports
  fourteen, `gpu-ram-room` included. This step adds no condition and
  changes no count. If the counts differ, the docs this step edits keep
  whatever counts step 4 left.
- **H-4-7.** Step 4's lever was kept: the measured replays and `psx-game`
  are unmetered. If step 4 reverted B1 as "not a gain", the meter switch
  still exists, because the facility stays. This step changes nothing in
  that case: it measures the replays as step 4 left them, and the note says
  so.
- **H-4-8.** `tmp/psx/full-game/run/frame-budget/frames.txt` is step 4's
  judge table: unmetered, O2, asserts off. The short runs' `representative`
  compares against it. If it is missing, both short runs pass
  `PSX_SHORT_RUN_REFERENCE=psx/perf/full-run-frames.txt`, and the note says
  so.
- **H-4-9.** Step 4's commit messages record `psx-check`'s `ram-room`, its
  `game-ram-room` bytes left, and `psx-game`'s ELF size after B1. This
  step's "before" figures for those are taken fresh anyway (Chunk 0), so
  the hypothesis only serves as a cross-check.
- **H-4-10.** Step 4 did not touch:
  - `docs/psx-game.md`'s build paragraph sentence "built `GPREL`, Release
    (`-O2`)";
  - the `PSX_SHORT_RUN_OPT` paragraph of the short-runs section;
  - the probe paragraph's configuration sentence.

  It did edit the probe paragraph's "one constant for the backend's overrun
  check, the meter and the `gpu_frame` zone". If step 4 rewrote one of the
  passages above, this step edits the new text to the same effect.
- **H-4-11.** The short runs still print
  `psx-short-runs: <name>: opt <level> psx-gpu-replay <t>/<d>/<b> psx-gpu-replay-breakdown <t>/<d>/<b>`
  and `asserts ok off in 3 boots`. The note reads its level, sizes and
  asserts mode from them.
- **H-4-12.** `psx/probes/call-twice-cart-draw.patch` was refreshed by step
  4 and applies on `HEAD`. This step changes no source the probe patches
  touch, so they stay valid.
- **H-4-13.** The PSn00bSDK toolchain and pcsx-redux are still installed in
  `third_party/`. If not, `make psn00bsdk-toolchain-setup` and
  `make psx-emulator-setup` run first.

## Implementation Decisions

- **No covering lever.** G2 is in none of the plan's alternative pairs, and
  no lever already taken changes the compiler level. The note says so.
- **Which targets get O3:**
  - **O3:** `psx-game`, plus `psx-gpu-replay` and
    `psx-gpu-replay-breakdown` in every build: `psx-build`'s (`psx-check`,
    the game checks, the capture report, the judge), the short runs' and
    the probe's.
  - **Stay at the SDK's `-O2`:** `psx-replay`, `psx-pad-probe` and
    `psx-largest-slots`. They are neither shipped nor measured. The plan
    asks for O3 on "every shipped and measured target" only.
  - **Why `psx-check`'s capture replays are O3 too**, although they are not
    measured: the captures then prove the O3 code generation
    pixel-identical. Their `ram-room` is the step's verification. With
    asserts on and both the meter and the filter in, this build is the
    largest replay, so it is the worst case for RAM. An O2 capture build
    would prove nothing about the binaries that are measured.
- **CMake** (`psx/CMakeLists.txt`):
  - `PSX_GAME_OPT`:
    - a new `CACHE STRING`, default `O3`;
    - `STRINGS O2 O3 Os`;
    - validated like `PSX_MEASURED_REPLAY_OPT`: a `FATAL_ERROR` names the
      variable and the value.
  - `PSX_MEASURED_REPLAY_OPT`'s default becomes `O3`. Its docstring stays
    the same apart from the default.
  - The single `psx-measured-replay-opt` interface library becomes one
    interface library per level, `psx-opt-<level>`. It is created once,
    adds `-<level>` and is linked `PRIVATE`:
    - to `psx-game` when `PSX_GAME_OPT` is not `O2`;
    - to the two GPU replays when `PSX_MEASURED_REPLAY_OPT` is not `O2`.

    A small CMake function does it, so both call sites read the same.
  - `O2` links nothing, as today. An `O2` build is the build before this
    step.
  - The existing comment, which explains why a target's own options lose to
    the SDK's trailing `-O2`, is kept and moved onto the function. It now
    names `psx-build` as well as `psx-short-runs` as the places that check
    the level.
- **No blanket `-Wno-error`.** The interface library drops the
  `-Wno-error` it adds today. O3 becomes the level of the shipped game, so
  it keeps `-Werror` like every other build. If an O3 build raises a
  diagnostic:
  - a true positive is fixed through `fix-bug`;
  - a false positive (typically `-Wmaybe-uninitialized`,
    `-Warray-bounds` or `-Wstringop-*` from inlining) is demoted by kind
    only, with `-Wno-error=<kind>` on that level's interface library, and
    the commit message names the kind and the line.

  The report says `psx-game` raised no warning of its own at O3. The
  measured replays were built with `-Wno-error` in plan 20, so their
  warnings are unknown.
- **Makefile, `psx-build`:**
  - `PSX_BUILD_OPT ?= O3`, validated at parse time to be one word among
    `O2 O3 Os`, with the same `$(error ...)` form as
    `PSX_BUILD_GAME_ASSERTS`.
  - It is passed on every configure as
    `-DPSX_GAME_OPT=$(PSX_BUILD_OPT) -DPSX_MEASURED_REPLAY_OPT=$(PSX_BUILD_OPT)`.
    This replaces the literal `O2`.
  - One variable sets both, because no build of this plan wants the game
    and the replays at different levels. The CMake variables stay separate
    so that the short runs and the probe can set the replays alone.
- **Makefile, the level check in `psx-build`.** It runs after
  `cmake --build` and before the `size` listing. For each of `psx-game`,
  `psx-gpu-replay` and `psx-gpu-replay-breakdown`, it runs
  `sh tools/psx_opt_level.sh $(PSX_BUILD_DIR)/CMakeFiles/<target>.dir/flags.make`,
  in the same form as the short runs' loop:
  - an unreadable file counts as level `unreadable`;
  - a level other than `PSX_BUILD_OPT` prints
    `psx-build: <target> effective level <l>, not <asked> as requested`.

  After the loop, it prints one line,
  `psx-build: opt psx-game <l> psx-gpu-replay <l> psx-gpu-replay-breakdown <l>`,
  then exits 1 if any target failed. `tools/psx_opt_level.sh` becomes a
  prerequisite of `psx-build`.
- **`psx-full-game-check`** appends `PSX_BUILD_OPT=O3` to its `psx-build`
  call. There is no user-facing opt variable for the judge: the plan
  declares any figure that is not O3 invalid.
- **`psx-short-runs`:**
  - `PSX_SHORT_RUN_OPT ?= O3`. `O2` and `Os` stay accepted, for a
    before/after comparison like this step's.
  - Nothing else changes: it already passes the level, checks it on both
    targets and prints `opt <level>`.
- **`psx-probe-baseline` / `psx-probe`:**
  - They add `-DPSX_MEASURED_REPLAY_OPT=O3` beside
    `-DPSX_MEASURED_REPLAY_ASSERTS=OFF`. The probe build directories are
    kept between calls, and CMake caches the variable, so an explicit value
    is required.
  - After a successful build, `tools/psx_opt_level.sh` reads
    `psx-gpu-replay`'s `flags.make`. A level other than `O3` makes the run
    `invalid opt <level>, not O3`, and the run is not booted.
  - The fingerprint gains a last field, `opt O3`, so a baseline stamped
    before this step is stale. The `psx/` tree id changes in this step
    anyway, so this field only guards a future change made in the Makefile
    alone.
  - `tools/psx_probe.sh` is unchanged.
- **No TTY line for the level.** GCC exposes no macro for `-O3` as such
  (`__OPTIMIZE__` is set from `-O1` up), and the plan's verification names
  `tools/psx_opt_level.sh`. The level is read from the build, and the build
  fails when it is wrong, before anything boots.
- **Behaviour that differs at O3.** If a `psx-check` condition that holds at
  O2 fails at O3 (a capture, a state line, a game condition), it is a
  latent bug, for example undefined behaviour that `-O2` happened to
  tolerate. It goes through `fix-bug` in its own commit, provided its cause
  is proven and every `tests/e2e/*.sha256` stays unchanged. If its cause
  cannot be proven, the lever is reverted (both defaults back to `O2`, the
  facility kept), and the note names the failing condition.
- **Measurement** follows the plan's lever rule.
  - **Before** (Chunk 0, at the step-4 commit, clean tree): one
    `make psx-short-runs` with no variable, which is asserts off, O2,
    unmetered. Each run's `analysis/` and printed lines are copied to
    `tmp/psx/plan21-step5/before/`.
  - **After** (end of Chunk 2): `make psx-short-runs` with no variable,
    which is now asserts off at O3.
  - **The note records:**
    - per run and pooled over the six runs: the window update, draw and
      submit sums before and after, and the delta per part and on the total,
      in hblanks and in percent of the before figure;
    - the per-frame draw delta on `draw-5564` and `draw-6000`;
    - the comparison with the report: per run `start` −1382, `update-4737`
      −43, `update-5500` −165, `draw-5564` −683, `update-5719` −39,
      `draw-6000` −544; pooled update −13.8%, draw −3.4%, submit −3.1%,
      total −5.9%;
    - both runs' `opt` and `asserts` lines;
    - the `representative` verdicts, which are not gating;
    - the text/data/bss of both replays, before and after.
  - **Expected deviation from the report.** Plan 20 measured G2 with the
    draw filter and the meter compiled in. Steps 1 and 4 removed both, so
    the draw base is smaller and part of plan 20's draw gain was the meter
    and the filter running faster. The draw delta may come out smaller in
    hblanks than the report's, while the total percentage may come out
    larger, since the update gain does not depend on them and the base
    shrank. The step does not gate on matching −5.9%. A shortfall or
    excess is recorded, and the plan goes on.
  - **Not a gain.** If the pooled window total after is not below the
    pooled total before, the lever is reverted: `PSX_BUILD_OPT`,
    `PSX_SHORT_RUN_OPT` and both CMake defaults go back to `O2`. The probe
    flag and the judge's flag go back to `O2` too. The variables and the
    level checks stay. A part that is not a gain while the total is a gain
    is recorded, not reverted, because G2's rows cover all three parts as
    one `scale` lever.
- **Proof runs** (all must hold before the final commit):
  - `make test-all`, with every `tests/e2e/*.sha256` unchanged.
  - `make psx-check`, all conditions green, every capture unchanged.
    `psx-build` prints `opt psx-game O3 psx-gpu-replay O3
    psx-gpu-replay-breakdown O3`. The note gives `ram-room` and
    `game-ram-room` bytes left before and after, and the ELF sizes of
    `psx-game` and `psx-gpu-replay`.
  - **The full game's GPU replay, built only.** One `make psx-build
    PSX_EVENTS=worst-case/full_game.json
    PSX_BUILD_DIR=tmp/psx/plan21-step5/full-game-build
    PSX_BUILD_DRAW_FILTER=OFF PSX_BUILD_DRAW_METER=OFF
    PSX_BUILD_MEASURED_REPLAY_ASSERTS=OFF`, with the default O3. It is
    never booted. The note gives the GPU ELF's text/data/bss and its RAM
    room, computed as `ram-room` does (2,097,152 − 65,536 − text − data −
    bss − `PSX_STACK_RESERVE`). The figure is compared with step 1.2's
    `gpu-ram-room`, 351,872 left. A negative room is a failure of this step.
  - `make psx-short-runs` with no variable. On every run: every boot ok,
    `opt O3`, `asserts ok off in 3 boots`, and `done`, `deterministic` and
    `breakdown` hold.
  - `make psx-probe-baseline` with no variable. Every run is valid, and
    each build's level reads `O3`. This stamps the O3 baseline for the next
    probe.
  - **No `psx-full-game-check`** in this step. Step 7 runs it at the end of
    the phase, at O3 by construction.
- **Docs:**
  - `docs/psx-game.md`:
    - the build paragraph: `psx-game` and the GPU replays at `-O3` through
      `PSX_GAME_OPT`/`PSX_MEASURED_REPLAY_OPT`, set by `PSX_BUILD_OPT`, with
      the other targets at the SDK's `-O2`; why a target's own `-O3` would
      lose to the SDK's; `psx-build`'s `opt` line and its failure; one
      sentence on the timing-model caveat;
    - the short-runs `PSX_SHORT_RUN_OPT` paragraph: default `O3`, and
      `psx-build` no longer always `O2`;
    - the probe paragraph: the probe build is at `O3`, checked, and its
      level is in the fingerprint;
    - the `psx-full-game-check` paragraph: always `O3`.
  - The Makefile legend, per user story 40.
  - **`docs/psx-perf-levers.md` is not changed.** G2's "Where" still
    describes the mechanism truthfully, and step 22 deletes the report.
- **Step note figures** go in the commit message of Chunk 2: the
  measurement above, the ELF sizes and the RAM rooms, the `opt` lines, the
  demoted warning kinds (if any), and `psx-check`'s count.

## Implementation Chunks

0. **Baseline, no commit.** At the step-4 commit with a clean tree:
   - run `make psx-short-runs` with no variable and save the outputs to
     `tmp/psx/plan21-step5/before/`;
   - run `make psx-check`, then record `ram-room`, `game-ram-room` and the
     ELF sizes of `psx-game` and `psx-gpu-replay`.
1. **O3 in every `psx-build`.**
   - Write `psx-build`'s level check first. Expecting `PSX_BUILD_OPT=O3`,
     it fails on `psx-game` and on both replays, because `psx-build` still
     passes `O2` and `psx-game` has no switch. This is the red.
   - Then add the CMake variable, the per-level interface library, the
     dropped `-Wno-error` and `PSX_BUILD_OPT`.
   - Handle any O3 diagnostic per the decision above.
   - Then run `make test-all` and `make psx-check`: every condition green,
     captures unchanged, `opt ... O3` printed.
   - Build the full game's GPU replay once and record its RAM room.
   - Commit.
2. **The measuring harness at O3.**
   - `psx-full-game-check` passes `PSX_BUILD_OPT=O3`. `PSX_SHORT_RUN_OPT`
     defaults to `O3`. The probe flag, its level check and the fingerprint
     field.
   - The docs and the Makefile legend.
   - Then `make test-all`, `make psx-short-runs` (the "after") and
     `make psx-probe-baseline`.
   - Apply the "not a gain" rule.
   - Commit with the step-note figures.

## Testing Decisions

- A good test drives a tool through its command line and checks what it
  prints and its exit status, never its internals. No test boots an
  emulator.
- `tools/psx_opt_level.sh` is the one tool that judges the level. Its host
  test, `test_psx_opt_level` in `psx-tools-test`, already covers the SDK's
  real flag shape, CMake's release `-O3` followed by the SDK's `-O2`, and
  covers an `-O3` appended after it. It is unchanged, and no host test is
  added.
- The CMake variables, the interface libraries, the Makefile flags and
  `psx-build`'s check only exist against the PSX SDK. The real builds prove
  them:
  - the red/green of Chunk 1 on `psx-build`'s own check;
  - `psx-check`'s conditions and captures on the O3 binaries;
  - the short runs' existing level check;
  - the probe build's new level check on `psx-probe-baseline`.

  This follows step 1's and step 3's precedent for build switches.
- `make test` and `make e2e` get no new case and no slower step. The PC
  build is untouched.

## Out of Scope

- `-O3` for `psx-replay`, `psx-pad-probe` or `psx-largest-slots`.
- `-Os` (G3, measured negative), LTO, `-march` tuning, or any flag other
  than the `-O` level.
- Rebuilding libpsn00b at another level. The prebuilt SDK libraries keep
  their level.
- A TTY line or a `psx-check`/`psx-full-game-check` condition for the
  level. `psx-build`'s check fails the build instead.
- Changing `tools/psx_opt_level.sh`, `tools/psx_short_run.sh`,
  `tools/psx_probe.sh` or any check script.
- The judge run, which belongs to step 7, and map culling, which belongs to
  step 6.
- Measuring on the Miyoo, or any claim that the judge's gain holds there.
- Any change to captures, ceilings, references, e2e replays or game code,
  except a `fix-bug` for a proven O3-only defect.

## Further Notes

- The report labels G2 a timing-model lever. pcsx-redux charges a flat 2
  cycles per instruction, with no instruction-cache miss and no data wait.
  O3's +40 KB of text could cost cache misses on real hardware, which the
  judge cannot see. The plan accepts this, and `docs/psx-game.md` states it
  in one sentence.
- After this step, every "before" figure of a lever comes from a default
  run at O3, asserts off. An older short-run output or probe baseline at O2
  is not comparable. The probe fingerprint makes that mechanical for the
  probe. For the short runs, the printed `opt` line is the record.
- The report puts `psx-gpu-replay` at 1.53 → 1.57 MB of 2 MB with O3. That
  was the asserts-off full-game build with the meter and filter in. The
  full-game build-only proof above replaces that estimate with the actual
  figure ahead of step 7.
