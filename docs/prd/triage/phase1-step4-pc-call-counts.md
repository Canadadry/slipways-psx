---
title: "The PC frame profile: call counts and PC time per cart_draw / cart_update60 child and per draw family, and the draw-window coverage check (plan 20, phase 1, step 4)"
description: "Phases 2–3 of plan 20 need, for any recording and in seconds, how often each top-level game function and each draw family runs per frame and how much PC time it takes, and the short runs' draw windows must be shown to cover every draw child that matters anywhere in the full game."
status: needs-triage
---

## Plan step (verbatim)

> 4. A PC tool prints per-frame call counts and PC time per `cart_draw` and
>    `cart_update60` child, and per draw family, for any recording. It runs in
>    seconds and its own test sits outside `make test`. Verified on
>    `draw-5564`'s window: its per-family call counts equal the breakdown
>    boot's `calls` column. Then a coverage check: over all of
>    `full_game.json`, compare each `cart_draw` child's and family's share of
>    draw with its share in draw-5564's window. A child or family above 2% of
>    draw on a significant share of full-run frames but under 2% at 5564 gets
>    an extra draw window in `psx/short_runs.txt`, probed in steps 6–7 like
>    draw-5564. Verified when the comparison is in the report and every child
>    above 2% somewhere in the full run is covered by a probed window.

Plan file: `docs/plan-20.md`. Context from its Rules: hot paths are found
top-down from `cart_draw` and `cart_update60`, going down only into branches
above 2% of the run's mean window frame; the update spikes (frames 12, 4737,
5500, 5719) are identified on PC first, from PC timing or call counts per
`cart_update60` child; update costs come from PC timing proportions scaled to
the PSX update total; nothing in the game or in the shipped drawing path
changes, `make e2e`, every `*.sha256` and `psx-check` stay exactly as they
are; new tooling stays out of `make test` and `make e2e`, which must not get
slower; never launch the raylib app.

## Hypotheses on step 3

Step 3 (the probe tool, PRD `docs/prd/triage/phase1-step3-probe-tool.md`) is
being implemented in parallel with this PRD. Everything below that depends on
its outcome rests on these hypotheses. The implementer checks each one against
the committed step 3 before starting and, where one is false, adapts the named
point to what step 3 actually did, without changing this PRD's behavior.

- **H-3-1 — step 3 changes nothing that a PC build compiles differently.** Its
  probe mode (bigger GPU frame buffer, delay helper, `psx-probe:` lines) is
  compiled only under the `PSX_PROBE` option of the PSX CMake project. Without
  it, `src/p8/gpu_sizes.h`, the draw filter, the draw meter, the draw timing
  module, `cart_draw`, `cart_update60` and the GPU replay's non-probe loop are
  as at `e786b93`. This PRD's PC tool never defines `PSX_PROBE`. If step 3 did
  change a shared source, the PC tool is built against that version and the
  breakdown comparison (chunk 4) proves whether the counts still agree.
- **H-3-2 — step 3 keeps the short-run pipeline and its outputs.** The
  manifest format of `psx/short_runs.txt`, `make psx-short-runs-cut` and its
  per-run `run.json`, `map.txt` (`cut <W> first-game-frame <f> source-first
  <s> window <a> <b>`) and `run.end-state`, `make psx-short-runs` and its
  `breakdown.tty.txt` with one `psx-draw: frame <n> <family> <ticks> <calls>
  ... wrapped <0|1>` line per frame (families in the order cls, pset, line,
  rect, rectfill, circ, circfill, spr, print, pattern) and the
  `analysis/summary.txt` lines `family <f> <h> hblanks <calls> calls <pct>`
  are unchanged. The breakdown check reads the per-frame `psx-draw:` lines,
  not the summary, so a summary reformat would not matter.
- **H-3-3 — the probe tool picks its runs by name from `psx/short_runs.txt`.**
  `make psx-probe PSX_SHORT_RUNS=<names>` reads the same manifest and has no
  per-run table of its own, so a draw window added to the manifest by this
  step is probe-able in steps 6–7 without changing the probe tool. If step 3
  keeps a list of its own, this step adds the new window to it as well.
- **H-3-4 — the probe baseline goes stale on this step's commit, by design.**
  Step 3 fingerprints the git trees of `src/` and `psx/`; this step adds files
  under `src/` and may add lines to `psx/short_runs.txt`, so the next probe
  (step 6) starts with `make psx-probe-baseline`. This step runs no probe and
  does not refresh that baseline (it lives in `tmp/`, not in the repo).
- **H-3-5 — a new draw window fits step 3's probe limits.** A window added
  here has the shape of `draw-5564` (a resume run of at most 15 window frames,
  fewer than 100 run frames), so it stays under `PSX_SHORT_RUN_CEILING` (540
  vsyncs) and under the probe's default ceiling (1,080 vsyncs, twice that).
- **H-3-6 — `docs/psx-perf-levers.md` holds step 2's baseline section and
  step 3's probe section.** This step appends its own section after them (after
  the baseline section if step 3's is absent) and adds any new window's row to
  step 2's baseline table, flagged as measured by this step.
- **H-3-7 — step 3's sanity fixtures patch `src/cmd/psx_gpu_replay.c` and
  `src/game/cart.c`.** This step modifies neither file (nor any file of
  `src/game/`, `src/engine/`, `src/p8/` or `src/platform/`), so the fixtures
  still apply after its commit.
- **H-3-8 — step 3 keeps the plan's 2% threshold.** Step 3 measures the noise
  floor and may revisit the threshold in its step note. The coverage check
  takes the threshold as a parameter (default 2%); if step 3's note changed
  it, chunk 4 runs with the revised value and the report says so.
- **H-3-9 — no name collision.** Step 3 adds no make target, script or test
  named `frame-profile*`, `frame_profile*` or `call_profile*`.

**Checked against the committed step 3 (`be10b55`): all nine hold.** H-3-1:
`src/p8/gpu_sizes.h` and `src/cmd/psx_gpu_replay.c` changed, but only under
`PSX_PROBE`; without it `GPU_FRAME_BUFFER_BYTES` is still 73728 and the loop
is unchanged (non-probe PSX binaries byte-identical). The new
`src/platform/psx_probe.c` is PSX-only. H-3-3: `psx-probe` reads
`psx/short_runs.txt` like `psx-short-runs`. H-3-5: probe ceiling
`PSX_PROBE_CEILING` = 1080. H-3-7: the fixtures patch `src/cmd/psx_gpu_replay.c`
and `src/game/cart.c`. H-3-8: the threshold stays 2% (noise floor 0.00%).
Note from step 3: `help_screen_draw` ticks the RNG per call; it does not
affect this PC tool, which only counts and times.

## Problem Statement

Plan 20 finds the levers that would make a PSX frame do less work. The PSX
side can only time whole parts of a frame (update, draw, submit) and, in the
breakdown boot, each backend call per draw family, on five short runs. Several
later steps need to know, per frame and over the whole 6,059-frame
`full_game.json`, which top-level game functions run, how often, and how much
time they take:

- Step 6 probes each direct child of `cart_draw` on `start` and `draw-5564`,
  and needs to know each child's calls per frame beforehand.
- Step 9 must identify the four update spikes (frames 12, 4737, 5500, 5719) on
  PC, from call counts or PC time per `cart_update60` child.
- Step 10 scales PC time proportions of update's children to the PSX update
  total.
- The draw probes only see the short runs' windows. Nobody knows today whether
  `draw-5564` is representative of draw over the whole game: a child or family
  that is heavy on hundreds of other frames but light at 5564 would never be
  explored, and its lever would be missing from the report.

Today the only per-function figures come from a PSX boot (minutes, five short
runs, families only), and the only PC replay (`tmp/replay`) times nothing.
There is no way to count or time `cart_draw`'s and `cart_update60`'s children
without editing game code, which the plan forbids.

## Solution

A PC binary, `tmp/frame_profile`, and three make targets, all outside `make
test`, `make e2e`, `make test-all` and `psx-check`:

1. **`make frame-profile [EVENTS=<events>] [REFERENCE=<end-state>]`** replays
   the recording (default `worst-case/full_game.json`) headless, exactly as
   `tmp/replay` does, with the game, engine and PICO-8 runtime compiled with
   function-entry/exit instrumentation. It writes, in
   `tmp/frame-profile/<name>/`:
   - `frames.txt`: per frame, the PC time of `cart_update60` and `cart_draw`;
     per direct child of each (a child is a function and the call site it was
     called from), its calls, PC time and PC time net of the instrumentation;
     per draw family (the ten of the breakdown, `pattern` included), its calls
     and PC time;
   - `summary.txt`: the same figures summed over the run, with each child's
     and family's share of its root;
   - the last frame's state, checked against the recording's committed
     end-state (`<events>.end-state`, or `REFERENCE`) with
     `tools/end_state.sh`, printed as `same-game yes|no|unchecked`.
   It runs the full game in seconds.
2. **`make frame-profile-breakdown-check [PSX_SHORT_RUNS=<names>]`** (default
   `draw-5564`) profiles each named short run's `run.json` and compares, frame
   by frame over the run's window, the PC per-family call counts with the
   `psx-draw:` lines of that run's last breakdown boot (left in
   `tmp/psx/short-runs/<name>/` by `make psx-short-runs`; never boots). It
   prints `calls-match ok` with the window totals per family side by side, or
   the first mismatch.
3. **`make frame-profile-coverage`** cuts the short runs on PC (`make
   psx-short-runs-cut`), profiles `full_game.json` and every draw window
   (`start` and every `draw-*` run of the manifest), and prints, per
   `cart_draw` child and per draw family: on how many full-run frames its
   share of draw is above the threshold, its mean and maximum share, its share
   in each draw window, and a verdict (`covered`, `uncovered` with a proposed
   15-frame window, `below`, or `sporadic`). It fails when anything is
   `uncovered`.

The step then runs the breakdown check on `draw-5564`, runs the coverage
check, adds a draw window to `psx/short_runs.txt` for each `uncovered` item
(none if none), boots each new window once with `make psx-short-runs` asserts
off, and records everything in `docs/psx-perf-levers.md`.

## User Stories

1. As the Phase 2 implementer, I want each direct child of `cart_draw` listed with its calls per frame, so that I know what a call-twice probe of it doubles before I write the probe.
2. As the Phase 2 implementer, I want the two `draw_layers` calls of `cart_draw` (draw and draw-overlay) reported as two separate children, so that each gets its own probe and its own cost.
3. As the Phase 2 implementer, I want every child of `cart_draw` that ran at least once in the recording listed on every frame, with zero calls where it did not run, so that the columns are stable and a script can read them.
4. As the Phase 2 implementer, I want the per-family call counts of any frame, so that I can turn a family's per-call PSX cost into a per-frame cost on any frame.
5. As the Phase 2 implementer, I want the per-family call counts counted exactly as the breakdown boot counts them (every backend call reaching the top of the stack, and `pattern` for a patterned rectfill or circfill), so that PC counts and PSX breakdown figures can be multiplied together.
6. As the Phase 2 implementer, I want that equality proven on `draw-5564`'s window frame by frame, so that I can trust the PC counts on frames no PSX boot ever measured.
7. As the Phase 3 implementer, I want each direct child of `cart_update60` with its calls and PC time per frame, so that I can see which child holds the update spikes at frames 12, 4737, 5500 and 5719.
8. As the Phase 3 implementer, I want PC time per update child on every frame of the full game, so that I can scale PC proportions to the PSX update total (step 10).
9. As any phase implementer, I want the tool to accept any recording the replay accepts (title start, resume start, fixture bindings), so that I can profile a short run's `run.json`, an e2e or a worst-case game alike.
10. As any phase implementer, I want the tool to play exactly the same game as `tmp/replay`, so that its frame numbers are the replay's and the PSX replays' frame numbers.
11. As any phase implementer, I want the tool to say whether its last-frame state matches the recording's committed end-state, so that an optimized, asserts-off build that drifted from the reference game is caught.
12. As any phase implementer, I want the tool built like the measured PSX replay (optimized, asserts off), so that its PC time proportions are as close as a PC can get to what the PSX runs.
13. As any phase implementer, I want the PC time net of the instrumentation's own cost next to the raw time, so that a child making thousands of small calls is not inflated by the profiler.
14. As any phase implementer, I want the instrumentation's per-call cost measured and printed at the head of the output, so that I can judge how much of a raw time is the profiler.
15. As any phase implementer, I want the full 6,059-frame game profiled in seconds, so that I can rerun it after every question without waiting.
16. As any phase implementer, I want the outputs kept per recording under `tmp/frame-profile/`, so that I can reread or diff them without rerunning.
17. As any phase implementer, I want unknown functions named by address rather than dropped, so that a child is never silently missing from the totals.
18. As any phase implementer, I want each root's children to add up, with the root's own time outside its children shown as `self`, so that I see how much of `cart_draw` is not in any child.
19. As the plan pilot, I want the coverage check to compare each `cart_draw` child's and family's share of draw over the full game with its share in each draw window, so that the probed windows are known to stand for the whole game.
20. As the plan pilot, I want "above 2% on a significant share of frames" to be a fixed, printed rule, so that the coverage verdict is mechanical and reproducible.
21. As the plan pilot, I want an item above 2% only on a handful of frames reported as `sporadic` with its frames, so that PC timer noise on single frames does not create windows, yet nothing is hidden from the report.
22. As the plan pilot, I want, for each uncovered item, a proposed 15-frame window of the full game where its share is highest, so that adding the window to the manifest is a copy, not a search.
23. As the plan pilot, I want one proposed window to cover several uncovered items when one span serves them all, so that the short runs do not grow more than needed.
24. As the plan pilot, I want the coverage target to fail while any item is uncovered, so that the step's verification is a green command.
25. As the plan pilot, I want the coverage threshold to be a parameter with 2% as default, so that a threshold revised after step 3's noise floor is applied without a code change.
26. As the plan pilot, I want the coverage check to use the windows of `start` and every `draw-*` run, so that a window added by this step counts as soon as it is in the manifest.
27. As the plan pilot, I want each new draw window booted once with `make psx-short-runs` asserts off, `done`, `deterministic` and `breakdown` ok, so that it is a usable window for the probes of steps 6–7.
28. As the plan pilot, I want the new window's breakdown calls checked against the PC counts too, so that every draw window has the same proof as `draw-5564`.
29. As the plan pilot, I want the tool's tests in a target of their own, outside `make test`, `make test-all` and `psx-tools-test`, so that neither the fast loop nor `psx-check` gets slower.
30. As the plan pilot, I want the tool's counting rule tested against the breakdown's own draw timing module on the same calls, so that the two can never drift apart unnoticed.
31. As the plan pilot, I want the comparison and coverage scripts host-tested on hand-written profiles, TTYs and maps, so that their arithmetic and their verdicts are proven without an emulator or a long replay.
32. As the plan pilot, I want no file of the game, the engine, the PICO-8 runtime or the platform changed, so that `make e2e`, every `*.sha256`, `psx-check` and step 3's fixtures stay as they are.
33. As the plan pilot, I want `make test`'s static checks (float-grep partition and the other guards over `src/`) to accept the new host-tool files, so that `make test` stays green without getting slower.
34. As the human reading the report, I want the breakdown comparison's result, the coverage table and any window added, so that I know what the draw probes stand for.
35. As the human reading the report, I want PC shares labelled as PC time, not PSX cost, so that I do not read a PC software-raster share as a PSX share.
36. As the human reading the report, I want the new targets documented with the other short-run tooling, so that I know how the figures were produced.

## Implementation Decisions

### How children are counted without touching the game

- The tool's game, engine and PICO-8 runtime sources (`src/game/`,
  `src/engine/`, `src/p8/`) are compiled with the compiler's function
  entry/exit instrumentation (`-finstrument-functions`, supported by GCC and
  Clang). Everything else in the binary (devtools, the tool's entry point, the
  generated default-bindings unit) is compiled without it, so the hooks never
  instrument themselves or the replay harness. No game source is edited: the
  plan forbids it, and a hook point in `cart.c` would also break step 3's
  fixtures (H-3-7). Rejected: copying `cart_draw`'s body into the tool (it
  would drift from the game silently), and editing `cart.c` behind a
  definition.
- Instrumentation also fires for functions the optimizer inlined, so the
  children of `cart_draw` stay visible at `-O2`.
- **Roots** are `cart_update60` and `cart_draw`, identified by their own
  addresses. A **child** is a function entered while the innermost active
  instrumented function is a root, keyed by (function, call site): the two
  `draw_layers` calls of `cart_draw` are two children. A function reached
  from one call site is named by its symbol; one reached from several sites
  of the same root is named `<symbol>#<k>`, k = 1, 2, … in the order the
  sites were first entered in the run (`draw_layers#1` is the draw pass,
  `draw_layers#2` the overlay pass). Names are final at the end of the run,
  when the outputs are written.
- Each root also gets a `self` line: the root's time minus its children's.
- Descendants below the children are not reported (that is steps 7, 9 and
  10's descent); they are only counted, per child, as `below` (instrumented
  calls under the child), which the net time uses.
- **Symbol names**: after linking, the build writes the binary's symbol table
  (`nm` default format) next to it; the tool reads it at start, and maps a
  hooked address to a name with the load offset computed from `cart_draw`'s
  known address against its table entry (works with PIE, and on Mach-O after
  stripping the leading underscore). An address with no symbol is printed as
  `fn@<hex offset>`; a name shared by several static functions is suffixed
  with its offset. A missing or unreadable symbol table is a usage error.
- **Time**: a monotonic nanosecond clock, read only at a child's entry and
  exit, at each root's entry and exit, and around each backend call. The hook
  of every other function only pushes and pops an address on a fixed-depth
  stack (depth over its cap is a fatal naming the cap).
- **Instrumentation cost**: at start, the tool times a fixed number of
  enter/exit pairs through the same bookkeeping on a dummy address and prints
  `hook-pair-ns <x>`. A child's `net` time is its raw time minus `below` ×
  that cost (floored at 0). The root totals are raw.
- **Families**: a backend wrapper installed on top of the same stack as the
  GPU replay's breakdown boot (software backend, then the draw filter with
  every family passing, then the draw meter, then the profile wrapper), so the
  calls it sees are exactly the calls the breakdown's timing wrapper sees.
  Counting rule copied from the breakdown: every call counts under its family;
  a rectfill or circfill with a non-zero fill pattern also counts under
  `pattern`, whose time stays inside its shape's (not added). Rejected: reusing
  the breakdown's draw timing module itself (16-bit clocks, 1,024-frame table,
  16-bit call counts: all sized for the PSX). The equality of the two rules is
  pinned by a test (Testing Decisions).
- `in-call`: per frame, the sum of the family times but `pattern`;
  `outside-calls` is `cart_draw`'s time minus it, as the breakdown's summary
  names them.

### The binary and its build

- A pure bookkeeping module under `src/devtools/` (name free, e.g.
  `call_profile`): a clock injected by the caller, enter/exit entry points
  taking (function, call site), root registration by address, frame end,
  per-frame records in static tables (frame cap at least `full_game.json`'s
  6,059 frames plus 1/8, children per root capped at 16, both overruns are a
  `p8_fatal` naming the cap), and the family wrapper. No allocation, no libc
  clock: the entry point owns those.
- An entry point under `src/cmd/` (e.g. `frame_profile.c`): the compiler's
  two hook symbols forwarding to the module (marked as never instrumented),
  the monotonic clock, the symbol table, and a replay loop with the same
  semantics as `tmp/replay` (recording opening, bindings, resume start, the
  `input_step_run` → `cart_update60` → `cart_draw` order), writing `STATE`
  through the existing state module at the last frame. It does not reuse
  `tmp/replay`'s binary because that binary is `-O0`, asserts on and not
  instrumented.
- Build: optimized (`-O2`) with asserts off (`-DNDEBUG`), mirroring the
  measured PSX replay of the plan's asserts-off baseline. Two new object trees
  under `tmp/` (instrumented and plain), following the Makefile's rule of one
  object tree per set of flags. The unused-* warning kinds that assert-only
  code raises under `NDEBUG` are demoted from errors in these two trees only,
  as step 1.2 did for the PSX measured replay; any other warning stays an
  error.
- Usage: `frame_profile <events> <out-dir> <symbols>`; with `STATE=<file>`
  in the environment it also writes the last frame's state. Exit 0 on a
  complete run, 1 on a usage or input error, and the replay's own failure
  status when the game stops in `p8_fatal` (a partial `frames.txt` is then
  not written).
- Runtime target: `full_game.json` profiled in at most 10 s on the
  development container. The real figure is recorded in the step note.

### Output contract (pinned by the tests)

`frames.txt`:

```
profile <events> frames <n> hook-pair-ns <x.y> clock monotonic-ns
frame <i> update <ns> draw <ns>
frame <i> child update <name> <calls> <ns> <net-ns> <below>
frame <i> child draw <name> <calls> <ns> <net-ns> <below>
frame <i> self update <ns>
frame <i> self draw <ns>
frame <i> family <family> <calls> <ns>
frame <i> in-call <ns>
```

Frame index i is the replay's loop index (0-based, the PSX replays' `g_frame`,
the breakdown's `psx-draw: frame <i>`). Children appear in first-entered
order, every child of the run on every frame; families in the breakdown's
order, all ten on every frame.

`summary.txt`: the same lines without the frame prefix, summed over the run,
each child, `self` and family line followed by its share of its root's total
(`<pct>%`, one decimal), then `same-game yes|no|unchecked`.

### The check script

One pure POSIX shell script under `tools/` (e.g. `frame_profile_check.sh`),
two subcommands, prior art `tools/psx_short_run.sh`:

- `breakdown <frames.txt> <breakdown.tty.txt> <map.txt>`: window run frames
  from `map.txt` (run frame = first-game-frame + full frame − source-first,
  as `psx_short_run.sh` computes them); for every window frame and every
  family, the PC calls must equal the `psx-draw:` line's calls. Prints
  `calls-match ok <n> frames` and one line per family `family <f> pc <c> psx
  <c>` (window totals), or `calls-match differs frame <i> family <f> pc <c>
  psx <c>` for the first mismatch. Exit 0 match, 2 mismatch or missing frame
  line, 1 usage error. CR line ends (the emulator's TTY) accepted.
- `coverage <threshold> <full frames.txt> <name>=<frames.txt>:<map.txt> ...`:
  for each `cart_draw` child and each family other than `pattern` (its time
  is inside its shapes), the share of `cart_draw`'s PC time per full-run frame,
  and per window the share over the window's frames (window time over window
  draw time). Verdict per item:
  - `covered`: above the threshold on at least 15 full-run frames, and above
    it in at least one window;
  - `uncovered`: above the threshold on at least 15 full-run frames, below it
    in every window; followed by `propose resume <a> <b>`, the 15-frame span
    of the full game with the highest mean share for that item;
  - `sporadic`: above the threshold on 1 to 14 full-run frames; the frames are
    listed; no window is proposed;
  - `below`: never above the threshold.
  Each item's line carries the frame count above the threshold, the full-run
  mean and maximum shares, and each window's share. Items that are uncovered
  are then covered greedily by the fewest proposed spans (a span covers an
  item when the item's mean share over it is above the threshold; ties go to
  the earliest span), printed as `window draw-<a> resume <a> <b>` lines, ready
  for the manifest. Exit 0 when nothing is uncovered, 3 when something is, 2 on
  malformed input, 1 on a usage error.
- **"Significant share of full-run frames"** is decided as: at least 15
  frames, the length of `draw-5564`'s window (a quarter second of play).
  Rationale: the plan wants every child that matters anywhere covered (its
  verification says "above 2% somewhere"), and a lower count would turn PC
  timer noise on single short frames into windows. Items above the threshold
  on fewer frames are not hidden: they are `sporadic` and listed with their
  frames in the report, and the plan's verification is read as "above 2% on
  at least 15 frames".
- **What "covered by a probed window" means**: above the threshold in at least
  one draw window, so step 7's 2% rule descends into it there. Every child
  that runs in a draw window is probed at level 1 by step 6 anyway.
- The shares are **PC time** shares (PC software raster, not the PSX GPU).
  That is what the plan asks for and the only full-game per-function source
  before the probes; the report says so, and step 10's calibration measures
  the PC/PSX gap later.
- The draw windows compared are the short runs' own `run.json` windows (what
  the probes measure), not frames 5564–5578 of the full profile: a resumed run
  can differ in camera, selection and crosshair from the full game.

### Make targets

- `frame-profile`: prerequisite the binary; defaults `EVENTS =
  worst-case/full_game.json`, `REFERENCE = <EVENTS>.end-state`; a missing
  reference prints `same-game unchecked`; `same-game no` fails the target.
- `frame-profile-breakdown-check`: prerequisites the binary, the script and
  `psx-short-runs-cut`; per run of `PSX_SHORT_RUNS` (default `draw-5564`),
  profiles `run.json` with `REFERENCE` the run's `run.end-state`, then the
  `breakdown` subcommand on that run's existing `breakdown.tty.txt`. A missing
  TTY fails naming the `make psx-short-runs` command to run first. Never boots.
- `frame-profile-coverage`: prerequisites the binary, the script and
  `psx-short-runs-cut`; profiles `full_game.json` and each draw window run
  (`start` and the manifest's `draw-*` runs), then the `coverage` subcommand
  with `FRAME_PROFILE_THRESHOLD` (default 2), output also written to
  `tmp/frame-profile/coverage.txt`.
- `frame-profile-test`: the tool's tests (below). Outside `make test`, `make
  test-all`, `make e2e` and `psx-tools-test` (the user asked for the test
  outside `make test`; keeping it out of `psx-tools-test` keeps `psx-check`'s
  prerequisites as they are).
- All four in the Makefile legend, in the style of `psx-short-runs`.
- The new `src/` files are host tools: listed in the float-grep exclusion list
  with their justification, and passing the other `src/` guards of `make
  test`, which only scan them (no new build in `make test`).

### Manifest, real runs and report

- For each `window draw-<a> resume <a> <b>` the coverage check prints, the
  line is appended to `psx/short_runs.txt` (if the cut refuses it — no
  eligible save — the span is shifted to the nearest accepted one that still
  covers its items, and the step note says so). Then `make psx-short-runs
  PSX_SHORT_RUNS=<new> PSX_SHORT_RUN_ASSERTS=off PSX_SHORT_RUN_REFERENCE=`
  must give `done`, `deterministic`, `asserts` and `breakdown` ok;
  `frame-profile-breakdown-check` on it must match; the coverage check is
  rerun until it exits 0.
- `docs/psx-perf-levers.md` (H-3-6) gains a section "PC frame profile and
  draw coverage": the tool's command line and commit, the full-game runtime
  and `same-game`, the `draw-5564` breakdown check (per-family PC and PSX
  window calls side by side), the coverage table (every `cart_draw` child and
  family: frames above the threshold, mean and maximum share, share in each
  draw window, verdict), the sporadic items with their frames, and any window
  added with why. A new window's asserts-off window sums join step 2's
  baseline table as a new row, marked as measured by this step with its
  commit. The PC `cart_update60` child figures at frames 12, 4737, 5500 and
  5719 are not analyzed here (step 9).
- `docs/psx-game.md`'s "Short runs" section gains a short paragraph on the PC
  frame profile and its three targets, and its manifest paragraph lists any
  new run (the "five runs" wording updated).

## Implementation Chunks

1. **Bookkeeping module and its unit test.** The pure module (roots,
   children by call site, `#k` naming, `self`, `below`, net time, family
   wrapper and counting rule, frame tables and caps) driven through its
   public entry points with a fake clock and fake addresses; the
   `frame-profile-test` target with this test only.
2. **The binary.** Entry point, hooks, clock, symbol table, replay loop,
   `STATE`, the two object trees, the symbol-table step after linking,
   `make frame-profile`, the float-grep exclusions, and the binary's test (in
   `frame-profile-test`). Proven also by `make test-all` unchanged and by
   profiling `full_game.json` with `same-game yes`.
3. **The check script.** Both subcommands, their host test (in
   `frame-profile-test`), the targets `frame-profile-breakdown-check` and
   `frame-profile-coverage`, the Makefile legend.
4. **Real-build verification and report.** `make psx-short-runs
   PSX_SHORT_RUNS=draw-5564 PSX_SHORT_RUN_ASSERTS=off PSX_SHORT_RUN_REFERENCE=`
   (if its breakdown TTY is not already from the current `HEAD`), the
   breakdown check on `draw-5564`, the coverage check on the full game, any
   window added and booted, the report section and the docs. The step note
   records the full-game runtime, the windows added (or none) and any
   hypothesis that turned out false.

## Testing Decisions

- A good test drives a public interface the way its user does: the module
  through its entry points, the binary through its command line and output
  files, the script through `sh` with hand-written inputs; it checks outputs,
  never internals. Timings are not deterministic on PC: tests pin call counts,
  names, line shapes and arithmetic, and only check that times are present
  and consistent (child time ≤ root time, `self` = root − children, net ≤
  raw), except where the fake clock makes them exact.
- **Module test** (`tests/test_call_profile.c`, name follows the module):
  - children keyed by call site: one function from two sites gives `#1` and
    `#2` in first-entered order; from one site, its bare name; a child seen
    late in the run still appears with zeros on earlier frames;
  - only direct children of a root are reported; deeper calls feed `below`;
    calls outside any root are ignored;
  - exact times with the fake clock: child, `self`, root, net time with a
    given pair cost, floored at 0;
  - families: the same scripted sequence of backend calls (every family, with
    and without a fill pattern on rectfill and circfill) sent through this
    module's wrapper and through the breakdown's draw timing wrapper over a
    null backend gives identical per-family call counts — the counting rule
    can never drift from the breakdown's;
  - frame cap, children cap and stack-depth cap overruns are `p8_fatal` with
    their message (checked in a child process, as `test_draw_timing` checks
    its table-full fatal).
- **Binary test** (`tests/test_frame_profile.c`): runs `tmp/frame_profile`
  on a short committed e2e recording (one with a resume start and one from the
  title): the frame count equals the recording's; the children of
  `cart_draw` include `p8_cls`, `hud_select_palette_block_default`,
  `draw_layers#1`, `draw_layers#2` and `demo_overlay_draw`, one call each per
  frame; `cls` is called once per frame; the children of `cart_update60`
  include `update_entities` and `ui_update`; two runs give identical call
  counts; `STATE` gives the same normalized end-state as `tmp/replay`'s;
  usage errors exit 1. Runs in a few seconds.
- **Script test** (`tests/test_frame_profile_check.c`): hand-written
  `frames.txt`, `psx-draw:` TTYs and maps: a match over a resume window and a
  prefix window; frames outside the window ignored; first mismatch reported;
  missing frame line; CR line ends; coverage verdicts `covered`, `uncovered`
  with the right proposed span, `sporadic` (14 frames) versus relevant (15
  frames), `below`; the greedy window grouping; a threshold other than 2;
  exit codes 0, 1, 2, 3.
- All three in `frame-profile-test`, outside `make test`, `make test-all`,
  `make e2e` and `psx-tools-test`.
- Prior art: `tests/test_draw_timing.c` (backend wrappers over a recording
  backend), `tests/test_frames_tool.c` (a host binary run on a committed
  recording), `tests/test_psx_short_run.c` (a shell script on hand-written
  TTYs and maps, outputs compared line by line).
- Unchanged and must stay green: `make test-all` (no slower), every
  `tests/e2e/*.sha256`, `psx-check`.

## Out of Scope

- Any probe (steps 6–8, 10), the full asserts-off run (step 5), and any
  analysis of the update spikes (step 9): this step only gives the PC figures
  they will use.
- Reporting descendants below the direct children of the two roots (the
  descent of steps 7, 9 and 10 may extend the tool then).
- A per-child breakdown of families (which child emits which calls).
- Converting PC time to PSX hblanks, or calibrating PC against PSX (step 10).
- Update windows: the coverage check covers draw only, as the plan says.
- Changing the probe tool (step 3), the breakdown boot or `psx_short_run.sh`.
- Any change to the game, the engine, the PICO-8 runtime, the platform, the
  shipped drawing path, the PC builds of `make test`/`make e2e`, or the e2e
  references.
- Running the raylib app (never) or anything on the Miyoo.

## Further Notes

- Decisions taken without a human (no QUESTION was asked, no guideline agent
  needed): compiler instrumentation instead of any game-side hook; children
  keyed by call site with `#k` names; `-O2` and asserts off to mirror the
  measured PSX replay; net time from a measured hook-pair cost; families
  counted by an own wrapper whose rule is test-pinned against the breakdown's;
  "significant share" = at least 15 full-run frames, with `sporadic` items
  listed; "covered" = above the threshold in at least one draw window; windows
  compared on the short runs' own `run.json`; PC time shares, labelled as
  such; tests in a `frame-profile-test` target outside `make test` and
  `psx-tools-test`; threshold as a parameter (H-3-8). Each follows the plan's
  Rules: no game change, fast loop untouched, relative figures from
  deterministic runs, PC first for counts.
- Families' PC time is software rasterization on PC and is not a PSX cost; a
  family's coverage verdict can therefore differ from what a PSX full run
  would say. All nine shape families already run in `draw-5564`'s window, so a
  family verdict only adds a window when its PC share is low at 5564 and high
  elsewhere; the report states the PC/PSX caveat next to the table.
- The breakdown check is exact equality: if it fails, the step note records
  the first mismatch and the tool is fixed in this step (a game difference
  between the `-O2` asserts-off PC build and the PSX one would show up there
  or in `same-game`).
- Adding a run to the manifest lengthens `make psx-short-runs` (about 50 s
  per run); it is outside every suite, so this costs no fast-loop time.
