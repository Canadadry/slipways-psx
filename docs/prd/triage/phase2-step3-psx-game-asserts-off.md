---
title: "Asserts off in the shipped psx-game"
description: "The game as it ships on PSX still compiles every assert in with -UNDEBUG, and the measuring harness still defaults to asserts on, although plan 21 locks asserts off for psx-game and every measured replay."
status: needs-triage
---

## Problem Statement

Plan 21 locks "asserts off in `psx-game` and in the measured GPU replays, on
in the PC build and the tests". Today only half of that holds:

- the measured GPU replays (`psx-gpu-replay`, `psx-gpu-replay-breakdown`)
  drop `-UNDEBUG` when the CMake option `PSX_MEASURED_REPLAY_ASSERTS` is
  `OFF`, so the SDK's `NDEBUG` applies;
- `psx-game`, the program on the CD image, always gets `-UNDEBUG`: every
  assert of the core, the GPU backend and the pad code runs on the console.
  Plan 20 measured asserts off as worth +3.7% of the frame (update +3.2%,
  draw +3.9%, submit +1.2%) and about 70 KB of text: the shipped game pays
  a cost the judge does not (lever G1);
- the measuring harness asks for asserts on unless told otherwise:
  `PSX_FULL_GAME_ASSERTS` and `PSX_SHORT_RUN_ASSERTS` both default to `on`.
  Every lever note of the plan must be taken asserts off, so every later
  step has to remember a variable, and a forgotten one gives an invalid
  figure. (`psx-probe-baseline` and `psx-probe` already build asserts off,
  unconditionally.)

Nothing proves which asserts mode the shipped binary was built in: the
replays print `psx-replay: asserts on|off`, `psx-game` prints nothing.

## Solution

Every `assert` stays in the source. `psx-game`'s asserts go behind a CMake
option, `PSX_GAME_ASSERTS`, default `OFF` (Makefile `PSX_BUILD_GAME_ASSERTS
?= OFF`, passed by `psx-build`): `OFF` builds it without `-UNDEBUG`, with the
same demoted unused-* warnings as the asserts-off measured replays; `ON`
keeps `-UNDEBUG`, so asserts come back with one variable. It prints
`psx-game: asserts on|off` at boot, and `tools/psx_game_check.sh` gains a
sixth game condition, `game-asserts`, that requires the line to match the
mode `psx-build` asked for (`off` by default): `psx-check`
(thirty-two conditions) and `psx-game-cd-check` prove on every run that the
CD's game is asserts off, and their existing `game-state-game-lines`,
`game-pad-log` and `game-pad-states` prove the scripted pad boot still plays
what the PC plays.

The harness's defaults switch to asserts off: `PSX_FULL_GAME_ASSERTS` and
`PSX_SHORT_RUN_ASSERTS` default to `off` (`on` stays accepted for a
deliberate asserts-on measurement). The probe needs no change. `psx-check`'s
CPU-path replay, GPU capture replays and `psx-largest-slots` keep their
asserts, as do the PC build and the tests.

## User Stories

1. As a player, I want the shipped `psx-game` built without asserts, so that each frame does not pay for checks the PC build already makes.
2. As the developer, I want `psx-game`'s asserts behind one switch (`PSX_GAME_ASSERTS`, default `OFF`), so that I can turn them back on for debugging without editing any source, while the default build ships without them.
3. As the plan pilot, I want `psx-game` to print `psx-game: asserts off` on its TTY, so that the shipped binary itself says which mode it was built in.
4. As the plan pilot, I want a `game-asserts` condition that fails when the TTY's mode differs from the one `psx-build` asked for (`off` by default) or the line is missing, so that a default build that ships asserts turns `psx-check` red.
5. As the plan pilot, I want `game-asserts` checked by `tools/psx_game_check.sh`, so that `psx-check` and `psx-game-cd-check` both get it from one place.
6. As the plan pilot, I want `psx-check` to report thirty-two conditions, so that its count matches the condition added.
7. As the plan pilot, I want `game-state-game-lines`, `game-pad-log`, `game-pad-states`, `game-done` and `game-title-left` to stay green, so that the scripted pad boot provably plays what the PC plays with asserts off.
8. As the plan pilot, I want `game-gpu-cost` to stay green, so that the backend still receives the PC's exact draw calls from the asserts-off game.
9. As the plan pilot, I want `game-ram-room` to stay green and its bytes left recorded before and after, so that the text saved by dropping asserts is seen.
10. As the plan pilot, I want the `psx-game` ELF text/data/bss before and after recorded, so that the note can set the text drop against the report's −70 KB.
11. As the plan pilot, I want `game-cd-exe` to stay green, so that the disc carries the asserts-off EXE that was checked.
12. As the plan pilot, I want every `psx-check` capture unchanged, so that the step keeps the 0-pixel rule.
13. As the plan pilot, I want `psx-check`'s `asserts-on` condition to keep requiring asserts on in the CPU-path replay, so that one PSX boot still runs the core with its asserts.
14. As the plan pilot, I want `psx-largest-slots` to keep its asserts and its `largest-slots` condition unchanged, so that the slot peaks are still checked with asserts on.
15. As the plan pilot, I want `psx-check`'s GPU capture replays to keep asserts on, so that the capture boots stay the asserts-on check of the GPU path.
16. As the plan pilot, I want the unused-* warnings that `NDEBUG` exposes in `psx-game`'s sources to stay warnings, as for the measured replays, so that the build passes without changing any source to please `NDEBUG`.
17. As the plan pilot, I want the sources only `psx-game` compiles (beyond the GPU replay's) reviewed for an assert with a side effect, so that `NDEBUG` cannot silently drop work the game needs.
18. As the plan pilot, I want `make psx-full-game-check` with no variable to build the measured GPU replays asserts off, so that step 7's end-of-phase run cannot be taken asserts on by mistake.
19. As the plan pilot, I want `make psx-short-runs` with no variable to build asserts off and its `asserts` verdict to read `asserts ok off in 3 boots`, so that every later lever note is valid without a remembered variable.
20. As the plan pilot, I want `PSX_FULL_GAME_ASSERTS=on` and `PSX_SHORT_RUN_ASSERTS=on` still accepted, so that an asserts-on measurement stays one variable away.
21. As the plan pilot, I want any other value of those two variables still refused before anything builds, so that a typo never measures an unknown mode.
22. As the plan pilot, I want `psx-probe-baseline` and `psx-probe` left as they are, so that the probe, already asserts off, keeps its fingerprints and baselines valid.
23. As the plan pilot, I want the hint printed by `frame-profile-breakdown-check` when a breakdown TTY is missing to drop the now-default `PSX_SHORT_RUN_ASSERTS=off`, so that the command it names is the shortest correct one.
24. As the plan pilot, I want one `make psx-short-runs` with no variable after the change, every boot ok and every condition holding, so that the default switch is proven on the real harness.
25. As the plan pilot, I want the short runs' `representative` verdict against step 2's judge table recorded, so that the note shows the default run agrees with the baseline it is compared to.
26. As the plan pilot, I want the note to say G1's judge gain is zero by construction, so that nobody looks for a delta the judge already included.
27. As a developer, I want `make test-all` green with every `tests/e2e/*.sha256` unchanged, so that the PC game is provably untouched.
28. As a developer, I want `tests/test_psx_game_check.c` to cover `game-asserts` (default expectation: off passes, on fails, missing fails; expectation `on`: on passes, off fails), so that the condition's contract is pinned without an emulator.
29. As a reader of `docs/psx-game.md`, I want the build section to say `psx-game` is built without asserts while the other PSX targets keep `-UNDEBUG` unless asked, so that the doc describes the build as it is.
30. As a reader of `docs/psx-game.md`, I want `game-asserts` listed with the other game conditions and the condition counts updated, so that the check's description stays exact.
31. As a reader of `docs/psx-game.md`, I want `PSX_FULL_GAME_ASSERTS` and `PSX_SHORT_RUN_ASSERTS` described with default `off`, so that the harness doc matches the Makefile.
32. As a reader of the Makefile legend, I want `psx-check`, `psx-game-cd-check`, `psx-full-game-check` and `psx-short-runs` described with the new count, condition and defaults, so that `make` help stays true.
33. As a reader of `src/cmd/psx_game.c`, I want its TTY protocol comment to list the asserts line, so that the program's output contract is complete.
34. As a reader of `docs/psx-perf-levers.md`, I want G1's two "it keeps `-UNDEBUG` today" mentions to say `psx-game` dropped it in plan 21 step 3, so that the report does not state something false until step 22 deletes it.

## Hypotheses on step 2

Step 2 is not committed when this PRD is written; its plan of record is the
PRD "New judge baseline with the draw filter off". Each assumption about its
outcome is listed with what this step does if it turns out false.

- **H-2-1.** Step 2 changes no source, CMake file or check script: `psx/CMakeLists.txt`, `src/cmd/psx_game.c`, `tools/psx_game_check.sh` and `tools/psx_check.sh` are as described here. If one changed, this step applies its decisions to the new text; the decisions do not depend on the old lines.
- **H-2-2.** Step 2's Makefile edits are limited to a variable for the report's judge table and the legend of `lever-estimate`, `lever-estimate-check` and the frame-profile targets. They do not touch `PSX_FULL_GAME_ASSERTS`, `PSX_SHORT_RUN_ASSERTS`, `psx-build`, `psx-check` or the probe. If they did, this step re-reads those rules before editing.
- **H-2-3.** Step 2's baseline was taken with `PSX_FULL_GAME_ASSERTS=off` explicitly, so the default switch here changes no judge figure: step 7's run with no variable measures the same mode as step 2's. If step 2's run was asserts on, the note says step 2's table is not comparable for the asserts mode and step 7's run is the first asserts-off-by-default baseline.
- **H-2-4.** Step 2 left `tmp/psx/full-game/run/frame-budget/frames.txt` in place, equal to the committed `psx/perf/full-run-frames.txt`, so the short runs' default reference is step 2's asserts-off, filter-off judge. If the file is missing (a cleaned `tmp/`), the short run of this step passes `PSX_SHORT_RUN_REFERENCE=psx/perf/full-run-frames.txt`, the same table, and says so in the note.
- **H-2-5.** Step 2's `frame-budget` stayed red. If it was green, the plan skips to its last step and this PRD is not implemented.
- **H-2-6.** `psx-full-game-check` still has thirteen conditions and its GPU replay prints `psx-replay: asserts off` when built asserts off. This step does not change its conditions.
- **H-2-7.** Step 2 edited `docs/psx-game.md` only in its "Still to do" bullet on `frame-budget`. This step edits other passages of that file; if step 2 rewrote one of them, this step edits the new text to the same effect.
- **H-2-8.** The PSn00bSDK toolchain and pcsx-redux are installed in `third_party/` (steps 1 and 2 used them). If not, `make psn00bsdk-toolchain-setup` and `make psx-emulator-setup` run first.

## Implementation Decisions

- **`psx-game`'s asserts sit behind a switch, default off** (human
  decision, overriding this PRD's first draft of "unconditionally, no
  option"). Every `assert` stays in the source; none is removed or
  rewritten. New CMake option `PSX_GAME_ASSERTS` (default `OFF`), in the
  style of `PSX_MEASURED_REPLAY_ASSERTS`; the Makefile gets
  `PSX_BUILD_GAME_ASSERTS ?= OFF`, passed explicitly by `psx-build` (and so
  by `psx-check`, `psx-game-cd` and `psx-full-game-check`; CMake caches
  options per build directory). In `psx/CMakeLists.txt`'s per-target
  flags, the branch that drops `-UNDEBUG` and demotes `unused-variable`,
  `unused-but-set-variable`, `unused-function` and `unused-parameter` to
  warnings applies to `psx-game` when `PSX_GAME_ASSERTS` is `OFF`, and to
  the two measured replays when `PSX_MEASURED_REPLAY_ASSERTS` is `OFF`.
  Every other target keeps `-Wall -Wextra -Werror -UNDEBUG`.
  `tools/psx_game_check.sh` takes the expected mode (default `off`) from
  its caller, so `make psx-check PSX_BUILD_GAME_ASSERTS=ON` stays green.
  The plan's lock (asserts off in the shipped game) is the default. `-Werror` stays for every other warning.
  The `_assert_abort` link wrap stays on every target (it is already
  harmless on the asserts-off replays). The comment above the branch is
  rewritten to say which targets drop `-UNDEBUG` (a trap: the SDK's release
  config defines `NDEBUG`, so the `-U` is what keeps asserts).
- **The binary says its mode.** `psx-game` prints
  `psx-game: asserts on|off` as its first TTY line, before `psx-game: ready`,
  computed the way `psx_gpu_replay.c` does (an assert that sets a flag). Its
  header comment lists the line. `pad_log` ignores `psx-game:` lines that
  are not `pad` lines, so the pad log and `tmp/psx_pad_log` are unaffected.
- **`game-asserts` condition** in `tools/psx_game_check.sh`, checked last
  (after `game-title-left`), so existing outputs keep their order: `ok, off`
  when the first `psx-game: asserts` line says `off`; failure
  `psx-game: asserts on` or `psx-game: asserts (no line)` otherwise. The
  header comment of the script names six conditions. `tools/psx_check.sh`
  adds `game-asserts` to its loop over the game conditions, its header and
  final line say thirty-two. `psx-game-cd-check` gets the condition through
  the same script with no change of its own.
- **Side-effect review.** Before the build, the implementer reads every
  `assert` in the sources `psx-game` compiles that the GPU replay does not
  (`src/cmd/psx_game.c`, `src/platform/psx_pad.c`,
  `src/platform/psx_pad_map.c`, and any other unit of `PSX_GAME_SRCS` not in
  `PSX_GPU_REPLAY_SRCS`). An assert whose expression has a side effect is
  rewritten as the existing pattern (`int ok = call(); assert(ok); (void)ok;`).
  The shared core has run asserts off with the PC's game state since plan 20,
  so it is not reviewed again. The note lists what the review found
  (expected: nothing; `save_storage_erase` already follows the pattern).
- **Harness defaults.** `PSX_FULL_GAME_ASSERTS ?= off` and
  `PSX_SHORT_RUN_ASSERTS ?= off`; their parse-time refusals of anything but
  `on`/`off` stay. `PSX_BUILD_MEASURED_REPLAY_ASSERTS ?= ON` stays: it is
  `psx-check`'s capture replays' mode, not a measuring default.
  `psx-probe-baseline`/`psx-probe` are already `-DPSX_MEASURED_REPLAY_ASSERTS=OFF`
  and are not touched, so their fingerprints stay valid (the fingerprint
  covers `psx/` and `src/`, which this step changes: a later probe re-runs
  its baseline anyway, as the plan's lever rule already does).
  `frame-profile-breakdown-check`'s missing-TTY hint drops
  `PSX_SHORT_RUN_ASSERTS=off`.
- **No measured lever delta.** The judge and the short runs have measured
  the GPU replays asserts off since plan 20, and `psx-game` has no frame
  timer: G1's delta on the judge is zero by construction. The note says so,
  quotes the report's +3.7% as the gain the shipped game now gets, and
  records instead the `psx-game` ELF text/data/bss and `game-ram-room` bytes
  left before and after (expected text about −70 KB). No `psx-probe` and no
  `psx-full-game-check` run in this step (step 7 runs the latter, now asserts
  off by default).
- **Proof runs** (all must hold before the commit):
  - `make test-all`, every `tests/e2e/*.sha256` unchanged;
  - `make psx-check`: all thirty-two conditions hold, `game-asserts` reads
    `ok, off`, `asserts-on` still `ok`, every capture unchanged;
  - `make psx-short-runs` with no variable: every boot ok; `opt O2`;
    `asserts ok off in 3 boots` on every run; `done`, `deterministic` and
    `breakdown` hold; `representative` against step 2's table (H-2-4) is
    recorded, and per the plan's rule a failure of it alone does not fail
    the step (none is expected: the measured code is unchanged);
  - the `psx-game` target's `flags.make` in the `psx-check` build has no
    `-UNDEBUG` (read by hand, quoted in the note).
- **Docs.** `docs/psx-game.md`: the build paragraph (which today says every
  target is built with `-UNDEBUG`), the condition list (`game-asserts` among
  the game conditions, "five" → "six"), the table rows and counts
  (thirty-one → thirty-two), the `PSX_FULL_GAME_ASSERTS` and
  `PSX_SHORT_RUN_ASSERTS` paragraphs (default `off`), and the protocol lines
  of `psx-game` if listed. The Makefile legend of `psx-check`,
  `psx-game-cd-check`, `psx-full-game-check` and `psx-short-runs`.
  `docs/psx-perf-levers.md`: only G1's two "keeps `-UNDEBUG` today" phrases.
  No figure of the report changes.
- **Figures for the step note** go in the commit message: `psx-game` ELF
  sizes and `game-ram-room` before/after, `psx-check`'s condition count and
  `game-asserts` line, each short run's asserts and representative verdicts
  and window update/draw/submit sums (beside step 2's table's window sums),
  the side-effect review's result, "asserts off, O2".

## Implementation Chunks

1. **The shipped game asserts off, proven by a condition.** `game-asserts`
   in `tools/psx_game_check.sh` driven by its unit test, the asserts line in
   `psx-game`, the CMake branch, the `psx_check.sh` wiring and count, the
   side-effect review; then `make test-all` and `make psx-check`
   (thirty-two conditions green, captures unchanged). Commit.
2. **The harness defaults to asserts off.** The two Makefile defaults, the
   breakdown-check hint, the legends and docs (`docs/psx-game.md`,
   `docs/psx-perf-levers.md`'s G1 phrases); then `make test-all` and one
   `make psx-short-runs` with no variable. Commit with the step-note figures.

## Testing Decisions

- A good test drives a tool through its command line on crafted inputs and
  checks its printed verdicts and exit status, never its internals.
- `tests/test_psx_game_check.c` (prior art: its own crafted TTYs built from
  `tests/e2e/pad_title.json`) gains: the TTY helper writes
  `psx-game: asserts off` before `psx-game: ready`; a passing boot prints
  `psx-game-check: game-asserts: ok, off`; a TTY with `asserts on` fails
  with `psx-game-check: game-asserts: psx-game: asserts on`; a TTY without
  the line fails naming `(no line)`; the five existing conditions keep their
  verdicts in each case.
- No unit test for the CMake change or the Makefile defaults: they are proven
  by `psx-check`'s `game-asserts` on the real boot and by the short runs'
  `asserts` verdict, which `tests/test_psx_short_run.c` already covers for
  both modes.
- `make test-all` stays green with every e2e hash unchanged; nothing enters
  `make test` that boots an emulator, and `make test`/`make e2e` get no
  slower: the new cases live in `test_psx_game_check`, one of
  `PSX_TOOLS_TESTS`, which `psx-tools-test` runs (a prerequisite of
  `psx-check`, `psx-full-game-check` and `psx-short-runs`), not `make test`.

## Out of Scope

- Asserts off in `psx-check`'s GPU capture replays, the CPU-path replay or
  `psx-largest-slots`.
- The draw meter (step 4), `-O3` (step 5), map culling (step 6), the end of
  phase judge run (step 7).
- Clearing the unused-* diagnostics in the sources: they stay demoted.
- Any change to the probe, its fingerprint or its baselines.
- Any change to captures, ceilings, references, e2e replays or game code.
- Measuring the shipped game's frame time (it has no timer) or anything on
  the Miyoo.

## Further Notes

- The judge has always measured the GPU replay, not `psx-game`; the two
  share the frame code, so after this step the judge's asserts mode and the
  shipped game's agree for the first time.
- A fault an assert would have caught on the PSX now goes silent there; it
  is still caught by the PC build, the unit tests, the e2e replays and
  `psx-check`'s asserts-on boots (CPU-path replay, GPU capture replays,
  largest slots).
