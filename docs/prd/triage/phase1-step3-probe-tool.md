---
title: "The probe tool: measure a throwaway patch on the short runs (plan 20, phase 1, step 3)"
description: "Phases 2–4 of plan 20 must measure dozens of throwaway patches (call-twice, fixed delay, experiments) against the asserts-off baseline, and today each one would be a hand-made build, boot and frame-line diff with no guard against GPU buffer overruns or gameplay drift."
status: needs-triage
---

## Plan step (verbatim)

> 3. A probe tool takes a throwaway patch (call-twice, fixed delay or
>    experiment), applies it in a temporary worktree, runs the chosen short
>    runs, and prints each run's window draw, update and submit delta against
>    the baseline. It also reports whether the game-state lines match the
>    baseline's. Probe builds get room for the extra primitives a probe sends
>    (the GPU frame buffer is sized at the metered peak + 1/8, and an overrun is
>    `p8_fatal`), and the meter's accounting matches; a probe boot that still
>    ends in `p8_fatal` is reported as invalid, never as a delta. Host-tested on
>    hand-written boot outputs. Verified by two sanity probes on the real build,
>    neither ending in `p8_fatal`:
>    - calling `cart_draw` twice roughly doubles draw (within 10%) and leaves
>      the game-state lines identical (`visual_random` may drift);
>    - a fixed delay at the entry of a function called a known number of times
>      per frame gives delta ÷ delay equal to that count.
>    - a null patch (changes the code layout, does no work at run time) gives
>      the noise floor, recorded per run in the report. It must sit clearly
>      below the 2% threshold, or the threshold or the probe method is
>      revisited before Phase 2.

Plan file: `docs/plan-20.md`. Context from its Rules: the measuring loop is
`make psx-short-runs` judged before vs after on itself, with
`PSX_SHORT_RUN_REFERENCE=` empty; drawing functions are probed by calling them
twice, update functions by a fixed delay at entry; throwaway experiments run in
a temporary worktree and nothing is kept from them but their numbers; all gains
are measured against the asserts-off baseline; nothing in the game or in the
shipped drawing path changes, `make e2e`, every `*.sha256` and `psx-check` stay
exactly as they are; new tooling is host-tested where the repo host-tests its
PSX scripts (`psx-tools-test`) and stays out of `make test` and `make e2e`.

## Hypotheses on step 2

Step 2 (an asserts-off option of `make psx-short-runs` and a baseline recorded
in `docs/psx-perf-levers.md`) is being implemented in parallel with this PRD.
Everything below that depends on its outcome rests on these hypotheses. The
implementer checks each one against the committed step 2 before starting and,
where one is false, adapts the named point to what step 2 actually did, without
changing this PRD's behavior.

- **H-2-1 — the asserts-off switch is a CMake-level setting of `psx/CMakeLists.txt`.**
  Step 2 turns asserts off for the measured replays by a configure-time
  setting of the PSX CMake project (a cache variable or option that drops the
  `-UNDEBUG` override for `psx-gpu-replay` and `psx-gpu-replay-breakdown`),
  driven by a make variable of `psx-short-runs`. The probe configures its
  builds with that same setting, always on (asserts off). If step 2 used
  another mechanism (a flag in the Makefile's source/flag lists, a separate
  target), the probe uses that mechanism instead. Whatever the mechanism, the
  probe's analysis checks the boot's own `psx-replay: asserts off` line (H-2-4),
  so a wrong guess shows up as an invalid run, never as a wrong delta.
- **H-2-2 — the step 2 baseline is a human-readable record, not a tool input.**
  Step 2 records, in `docs/psx-perf-levers.md`, per short run the window sums
  of draw, update and submit of one asserts-off run. This PRD does **not** parse
  that file: the probe computes its deltas against a probe baseline it boots
  itself (see Implementation Decisions, "Two baselines"). Its format, section
  name and figures therefore do not matter to the code; they only serve the
  report's side-by-side.
- **H-2-3 — `docs/psx-perf-levers.md` exists after step 2 and is the plan's report.**
  This step adds a section to it (probe baseline, sanity probes, noise floor).
  If the file does not exist when this step runs, this step creates it with
  that section only.
- **H-2-4 — step 2 keeps the GPU replay's TTY protocol.** The asserts line
  (`psx-replay: asserts on|off`, already printed by `psx-gpu-replay` and
  turning to `off` under `NDEBUG`), the `psx-frame: frame <n> update <u> draw
  <d> submit <s> total <t> vsyncs <v>` lines (13 fields), the
  `psx-replay: done <name> <frames> vsyncs <v>` line, the `psx-state: ` lines
  and the `psx-state: stopped p8_fatal ...` line keep their current form.
- **H-2-5 — step 2 keeps the short-run cut and its outputs.** The manifest
  `psx/short_runs.txt` (run names `start`, `update-4737`, `update-5500`,
  `draw-5564`, `update-5719`), the `make psx-short-runs-cut` target, the
  variables `PSX_SHORT_RUNS` and `PSX_SHORT_RUNS_DIR`, and per run
  `run.embedded.c`, `map.txt` (`cut <n> first-game-frame <f> source-first <s>
  window <a> <b>`) and `run.log` (`replay: <frames> frame(s) run`) are unchanged.
- **H-2-6 — step 2 changes nothing in `src/` that the probe touches.** No change
  to `src/p8/gpu_sizes.h`, `src/cmd/psx_gpu_replay.c`'s replay loop, or the
  fine clock and frame timer interfaces. If step 2 did touch them, the probe's
  additions are merged onto its version.

**Checked against the committed step 2 (`e786b93`): all six hold.** H-2-1:
the switch is the CMake option `PSX_MEASURED_REPLAY_ASSERTS` (default `ON`),
set by `make psx-short-runs PSX_SHORT_RUN_ASSERTS=off`; `OFF` drops `-UNDEBUG`
from the two measured replays and demotes four warning kinds
(`unused-variable`, `unused-but-set-variable`, `unused-function`,
`unused-parameter`) from errors, which the asserts-off build needs. The probe
configures with `-DPSX_MEASURED_REPLAY_ASSERTS=OFF`. H-2-3: the file exists with
a "Short-run baseline (asserts off)" section. H-2-4, H-2-5, H-2-6: unchanged
(`psx_short_run.sh` gained a 9th argument and a `window plain` summary line, which the
probe does not depend on).

## Problem Statement

Plan 20 finds the levers that would make a PSX frame do less work. Phases 2–4
will run dozens of measurements: each `cart_draw` child called twice, update
functions slowed by a fixed delay, throwaway experiments such as drawing the
outlined text once instead of nine times. Each measurement is a code change
that must never be committed, built into the measured GPU replay with asserts
off, booted on the short runs, and compared with the unchanged build.

Today, the person running those phases would have to do all of that by hand
for every probe:

- edit the source, remember to revert it, and hope nothing of it leaks into a
  commit (the plan forbids keeping anything from an experiment but its
  numbers);
- rebuild each short run's GPU replay from the edited tree, with asserts off;
- diff the `psx-frame:` lines of two boots and sum the window frames by hand,
  per part (draw, update, submit);
- notice by themselves when a probe changed the game (a drawing function that
  secretly mutates gameplay state makes call-twice meaningless);
- live with the fact that calling `cart_draw` twice sends twice the GPU
  primitives into a buffer sized at the metered peak + 1/8: the boot dies in
  `p8_fatal` partway, and a half run's frame lines could be read as a delta;
- turn a fixed delay into a call count with a delay whose length in hblanks is
  not known precisely;
- guess how much of a delta is the probe and how much is code-layout noise
  (instruction cache, alignment) that any recompiled build carries.

Without a tool that does all of this the same way every time, Phase 2's 2%
threshold cannot be trusted and the report's numbers cannot be reproduced.

## Solution

Two make targets, outside `make test-all` and `psx-check`, plus a small
probe-only build mode of the GPU replay:

1. **`make psx-probe-baseline [PSX_SHORT_RUNS=<names>]`** builds the *probe
   build* (asserts off, GPU frame buffer room, delay helper available) of each
   chosen short run from `HEAD`, unpatched, boots it once, and stores its
   boot outputs as the probe baseline, stamped with a fingerprint of the
   committed inputs.
2. **`make psx-probe PATCH=<file> [PSX_SHORT_RUNS=<names>] [PSX_PROBE_LABEL=<label>]`**
   creates a temporary git worktree at `HEAD`, applies the patch there,
   builds the probe build of each chosen short run from that worktree, boots
   it once, removes the worktree, and prints per run:
   - the window sums of draw, update, submit and total, baseline → probe, with
     the delta in hblanks and as a percentage of the baseline window total
     (the same as a percentage of the run's mean window frame, the plan's 2%
     yardstick);
   - whether the game-state lines (every `psx-state:` line but zone, budget
     and cap) match the baseline's, with the first difference when not;
   - when the patch used the delay helper: the delay in hblanks, the calls the
     helper counted, and delta ÷ delay over the whole run and per window frame;
   - or, instead of all the above, `invalid` with the reason: the patch did
     not apply, the build failed, the boot stopped in `p8_fatal` (with its
     message), the run did not finish its frames, asserts were on, or the
     baseline is missing or stale.
3. **The probe build** is the plain GPU replay compiled with a `PSX_PROBE`
   definition: its GPU frame buffer is twice the shipped size (room for
   calling even `cart_draw` twice), the meter and the state's `gpu_frame`
   zone use that same size, and a delay helper spins for an exact number of
   hblanks on the fine clock and counts its calls.

Three sanity probes on the real build verify the tool, and their figures go
into `docs/psx-perf-levers.md`: `cart_draw` called twice, a fixed delay at the
entry of `cart_update60` and of `cart_draw` (each called once a frame), and a
null patch whose per-run delta is the noise floor.

## User Stories

1. As the Phase 2–4 implementer, I want to measure a code change by handing a patch file to one command, so that a probe costs a command, not a manual build-boot-diff session.
2. As the Phase 2–4 implementer, I want the patch applied in a temporary worktree, never in my working tree, so that nothing of an experiment can leak into a commit.
3. As the Phase 2–4 implementer, I want the temporary worktree removed when the probe ends, whether it succeeded or failed, so that probes do not pile up worktrees.
4. As the Phase 2–4 implementer, I want a stale probe worktree left by an interrupted probe pruned at the next probe, so that one killed run does not block the next.
5. As the Phase 2–4 implementer, I want the patch to be an ordinary unified diff against `HEAD` (what `git diff` prints), so that I write probes with the tools I already use.
6. As the Phase 2–4 implementer, I want a patch that does not apply reported as invalid with git's message, so that a probe written against an older tree is not silently measured as a no-op.
7. As the Phase 2–4 implementer, I want to choose which short runs a probe boots with the same `PSX_SHORT_RUNS` variable as `psx-short-runs`, so that a draw probe can boot only `start` and `draw-5564`.
8. As the Phase 2–4 implementer, I want every probe build compiled with asserts off, so that every delta is against the asserts-off baseline the plan requires.
9. As the Phase 2–4 implementer, I want a boot that reports asserts on to be invalid, so that a build-flag mistake can never be read as a gain.
10. As the Phase 2–4 implementer, I want per run the window sums of draw, update, submit and total, baseline and probe side by side, so that I see which part a probe moved.
11. As the Phase 2–4 implementer, I want each delta also as a percentage of the baseline window total, so that I compare it directly with the plan's 2% threshold.
12. As the Phase 2–4 implementer, I want the window to be exactly the run's window frames of `map.txt`, as `psx-short-runs` uses, so that the probe and the short-run analysis speak about the same frames.
13. As the Phase 2–4 implementer, I want the probe to say whether the game-state lines match the baseline's, so that I know whether a call-twice probe mutated gameplay state and must fall back to a fixed delay.
14. As the Phase 2–4 implementer, I want the first differing game-state line printed when they differ, so that I see what the probed function changed.
15. As the Phase 2–4 implementer, I want zone, budget and cap lines left out of the game-state comparison, so that the probe build's bigger buffer and a probe's extra primitives do not count as a gameplay change.
16. As the Phase 2–4 implementer, I want probe builds to have room for at least twice the shipped peak of GPU primitives, so that calling any drawing function twice, `cart_draw` included, does not overrun the frame buffer.
17. As the Phase 2–4 implementer, I want the meter's accounting to use the probe build's buffer size, so that the meter, the backend's overrun check and the state's `gpu_frame` zone agree on the limit.
18. As the Phase 2–4 implementer, I want a probe boot that still ends in `p8_fatal` reported as invalid with the fatal message, never as a delta, so that a half run is never read as a measurement.
19. As the Phase 2–4 implementer, I want a boot that hits its ceiling or timeout, or finishes with the wrong frame count, reported as invalid, so that a hang is evidence, not a number.
20. As the Phase 2–4 implementer, I want the probe's boot ceiling and timeout to allow a run up to twice as slow as the short run, so that a call-twice of `cart_draw` finishes.
21. As the Phase 2–4 implementer, I want a fixed-delay helper that spins for an exact number of hblanks, so that delta ÷ delay is a call count and not an estimate skewed by the delay's own error.
22. As the Phase 2–4 implementer, I want the delay helper to count its calls and the boot to print the delay and the count, so that every delay probe checks itself: delta ÷ delay must agree with the counted calls.
23. As the Phase 2–4 implementer, I want delta ÷ delay printed over the whole run and per window frame, so that I read a function's calls per frame directly.
24. As the Phase 2–4 implementer, I want one delay value per probe build (a second value is a `p8_fatal`), so that delta ÷ delay is never a mix of two lengths.
25. As the Phase 2–4 implementer, I want the delay helper available only in probe builds, so that it can never reach the shipped game or the measured replays of `psx-short-runs`.
26. As the Phase 2–4 implementer, I want the probe baseline built with the same probe build settings as the probes, so that the bigger buffer and the fine clock never show up as a probe's delta.
27. As the Phase 2–4 implementer, I want the probe to refuse a baseline taken from different committed sources, so that a probe is never compared with a build of another tree.
28. As the Phase 2–4 implementer, I want the probe baseline to be rebuilt by one command, so that refreshing it after a tooling commit costs one run.
29. As the Phase 2–4 implementer, I want each probe's outputs (build log, TTY, analysis) kept under a label in `tmp/`, so that I can reread a probe after the fact without rerunning it.
30. As the Phase 2–4 implementer, I want the label to default to the patch's file name, so that I do not have to name every probe.
31. As the Phase 2–4 implementer, I want the probe to print the text size of the probe ELF against the baseline's, so that I can confirm a null patch moved the code layout.
32. As the Phase 2–4 implementer, I want the noise floor of a null patch recorded per run in the report, so that I know which deltas are signal.
33. As the Phase 2–4 implementer, I want the probe to exit non-zero when a run is invalid but zero when runs are valid and only the game-state lines differ, so that a differing state is a finding and not a tool failure.
34. As the plan pilot, I want the analysis script host-tested on hand-written boot outputs inside `psx-tools-test`, so that its arithmetic and its invalid cases are proven without an emulator.
35. As the plan pilot, I want the probe targets outside `make test`, `make e2e`, `make test-all` and `psx-check`, so that the fast loop never gets slower.
36. As the plan pilot, I want the shipped game, the PC build and every non-probe PSX build compiled exactly as before, so that `make e2e`, every `*.sha256` and `psx-check` stay unchanged.
37. As the plan pilot, I want two sanity probes and a null patch run on the real build before Phase 2, so that the tool is proven on the hardware model it will be used on.
38. As the plan pilot, I want the sanity patches committed as the tool's fixtures, so that anyone can rerun the sanity probes and the noise floor after a tooling change.
39. As the human reading the report, I want the probe baseline's window sums next to step 2's baseline, so that I see how far the probe build sits from the measured replay.
40. As the human reading the report, I want the probe targets documented with the other PSX targets, so that I know how the report's numbers were produced.

## Implementation Decisions

### Two baselines

- Deltas are computed against a **probe baseline**: the probe build of
  unpatched `HEAD`, booted once per run by `make psx-probe-baseline`. Not
  against step 2's recorded baseline: the probe build differs from the
  measured replay (bigger GPU buffer, fine clock started, delay helper
  linked), and a delta must compare like with like. Step 2's baseline stays
  the absolute reference of the report (H-2-2); the probe baseline's window
  sums are recorded next to it, and their gap is stated.
- The baseline directory holds a fingerprint of the committed inputs: the git
  tree ids of `src/` and `psx/` and the blob id of `worst-case/full_game.json`
  at `HEAD` when it was taken. `make psx-probe` computes the same fingerprint
  for its `HEAD` and marks every run invalid ("baseline stale: run make
  psx-probe-baseline") when they differ. A tooling-only commit outside those
  paths does not invalidate the baseline.
- A run asked of `psx-probe` that has no baseline boot is invalid ("no
  baseline for <run>"); the other runs are still measured.

### The probe build

- A `PSX_PROBE` configure-time option of the PSX CMake project (off by
  default), which adds the `PSX_PROBE` compile definition and the delay
  helper's source to `psx-gpu-replay` only. Probe builds configure with it on
  and with the asserts-off setting of step 2 (H-2-1). Only the
  `psx-gpu-replay` target is built per run; the breakdown is not part of a
  probe.
- **Room.** Under `PSX_PROBE`, the GPU frame buffer constant is twice its
  shipped value. One definition serves the backend's buffers and overrun
  check, the meter, and the state's `gpu_frame` zone limit, so the meter's
  accounting matches by construction. The shipped value and the 1/8-margin
  rule stay as they are; without `PSX_PROBE` every build compiles exactly as
  before. Twice is enough for the largest probe the plan names (calling
  `cart_draw` twice sends at most twice the metered peak, and the shipped
  buffer is already the peak + 1/8). It costs about 144 KiB of RAM more in the probe build only;
  RAM room is not checked by the probe (an out-of-RAM probe build fails at
  boot and is reported invalid). CLUT and pattern slots are not enlarged:
  calling the same drawing twice adds no distinct palette or pattern, and an
  experiment that overruns them ends in `p8_fatal`, reported invalid.
- **Delay helper** (PSX platform, probe builds only): a start function, called
  once by the GPU replay under `PSX_PROBE` right after the frame timer starts,
  starts the fine clock and calibrates it against the frame timer (the
  existing fine-clock calibration); `delay(hblanks)` converts hblanks to fine
  ticks with that calibration and busy-waits on the fine clock until that many
  ticks elapsed; it records the delay value on the first call and the number
  of calls. A call with another value, or a delay that would wrap the 16-bit
  fine clock (above 200 hblanks, the breakdown's wrap limit), is a `p8_fatal`
  naming both values. The fine clock is preferred to a fixed iteration count
  (its length in hblanks would itself need calibrating) and to the hblank
  frame timer (an edge-aligned wait carries up to one hblank of error per
  call, which a call count multiplies).
- Under `PSX_PROBE`, the GPU replay prints, before its done line:
  `psx-probe: clk8-per-64-hblanks <n>` and, when the helper was called,
  `psx-probe: delay <hblanks> calls <count>`. The calibration line is printed
  in every probe build, baseline included.
- Probe patches use the helper by including its header and calling it at the
  function's entry. The helper's call counter is a cross-check; the plan's
  method (delta ÷ delay) stays the measurement.

### The probe target

- `make psx-probe PATCH=<file>`: prerequisites `psx-tools-test` and
  `psx-short-runs-cut` (the cut recordings are inputs; the patch never touches
  them). A missing `PATCH` or patch file fails at once with usage.
- Worktree: `git worktree add --detach` of `HEAD` under the probe's output
  directory in `tmp/psx/probe/`; `git worktree prune` and removal of a leftover
  worktree first; removal (forced) on every exit path. Uncommitted changes of
  the main tree are deliberately not in the worktree: a probe measures
  `HEAD` + patch.
- `git apply` of the patch in the worktree; a failure makes every chosen run
  invalid with git's message, and nothing is built.
- Per run, the same CMake configuration as `psx-short-runs` (H-2-5), with
  three differences: the source directory, `SLIPWAYS_SRC_DIR` and every
  source path under `src/` point into the worktree (generated units such as
  the default bindings and the demo, and the run's embedded recording, stay
  the main tree's `tmp/` files); the `PSX_PROBE` option on; asserts off.
  A patch that would need a generated unit regenerated is out of scope.
- One boot per run (determinism is established by step 2's `deterministic`
  condition; the null patch measures what differs between builds), with an
  empty capture script, ceiling `PSX_PROBE_CEILING` (default 1080 vsyncs,
  twice `PSX_SHORT_RUN_CEILING`) and timeout `PSX_PROBE_TIMEOUT` (default
  20 s).
- `make psx-probe-baseline` is the same pipeline with no worktree patch
  (still built from a worktree of `HEAD`, so the baseline and the probes
  build from the same kind of tree), writing to `tmp/psx/probe/baseline/` and
  the fingerprint.
- Outputs per probe: `tmp/psx/probe/<label>/<run>/` (build log, TTY,
  analysis), `<label>` = `PSX_PROBE_LABEL`, default the patch's base name
  without extension; `baseline` is a reserved label.
- The text size of each run's probe ELF and baseline ELF (through the SDK's
  size tool, as `psx-check` does) goes into the run's analysis, as
  `text <baseline> -> <probe> (<delta> bytes)`.
- Every run is booted and analyzed even when an earlier one is invalid; the
  target exits 1 at the end if any run was invalid, 0 otherwise (differing
  game-state lines do not fail it).

### The analysis script

A new script under `tools/`, pure (no emulator, no git): it reads a baseline
TTY, a probe TTY, the run's `map.txt` and frame count, and an output
directory; writes `<out>/probe.txt` and prints it. It reuses the window
arithmetic of the short-run analysis (run frames from `map.txt`) and its
`psx-frame:` parsing (H-2-4).

Output, one block per run (the line order is the contract the host test pins):

```
<run> valid window run frames <a>..<b> (<n> frames)
<run> draw <base> -> <probe> <+delta> hblanks <+pct>% of window total
<run> update <base> -> <probe> <+delta> hblanks <+pct>%
<run> submit <base> -> <probe> <+delta> hblanks <+pct>%
<run> total <base> -> <probe> <+delta> hblanks <+pct>%
<run> game-state same|differs first difference: <baseline line> | <probe line>
<run> delay <d> hblanks calls <c> delta/delay run <x> (<±e>% vs calls) window per frame <y>
```

(the delay line only when the probe TTY has a `psx-probe: delay` line; the
percentages are of the baseline's window total, so "of the run's mean window
frame" in the plan's sense), or a single line `<run> invalid <reason>`.

Invalid reasons, checked on both TTYs (a bad baseline TTY is reported as
"baseline ..."): missing TTY; a `psx-state: stopped` line (the reason quotes
it, `p8_fatal` message included); no done line or a frame count other than
the expected one; no `psx-replay: asserts off` line; a window frame without its
`psx-frame:` line or with a malformed one; a delay line without a calibration
line, or a calibration outside 15500..19000 (the breakdown's accepted range).
Exit 0 when the run is valid, 2 when invalid, 1 on a usage error.

Game-state lines: the `psx-state:` lines of each TTY without the prefix,
minus those starting with `zone `, `budget ` or `cap ` (the rule of the
existing state comparison), compared in order; a missing line on either side
is a difference.

### Sanity fixtures

Committed under `psx/probes/` as the tool's fixtures, applied by `make
psx-probe PATCH=psx/probes/<name>.patch`:

- `call-twice-cart-draw.patch`: the GPU replay's loop calls `cart_draw` twice
  in a row (inside the same draw-filter frame and meter frame). Passes when, on
  every run, the draw delta is within ±10% of the baseline window draw and the
  game-state lines are the same.
- `delay-update-and-draw.patch`: the delay helper (20 hblanks) at the entry of
  `cart_update60` and of `cart_draw`, each called once a frame by the replay
  loop, so the known count is 2 per frame. Passes when the helper's counted
  calls equal twice the run's frame count, delta ÷ delay over the run is within
  2% of that count, and the window update and draw deltas are each within 2% of
  20 hblanks × window frames (which also checks that the delay lands in the
  right part).
- `null.patch`: adds code that is linked but never executed (for example a
  function marked used and kept by the linker, next to `cart_draw` in its
  translation unit), shifting the addresses of the code after it. Passes as a
  valid run on every run with a changed text size; its total delta per run is
  the noise floor. "Clearly below 2%" is decided as: |total delta| ≤ 0.5% of
  the window total on every run (a quarter of the threshold). Above that on
  any run, the step note says so and the threshold or the probe method is
  revisited before Phase 2, as the plan says; this PRD does not pick the
  remedy.

All three must end without `p8_fatal` on all five runs.

The fixtures are diffs against the tree; when a later commit moves their
context, they are refreshed with that commit (they are tooling, the only kind
of code this plan commits).

### Report and docs

- `docs/psx-perf-levers.md` (H-2-3) gains a section on the probe tool: the
  probe baseline's window sums per run next to step 2's, the three sanity
  probes' results per run, and the noise floor per run with the verdict
  against the 0.5% rule.
- `docs/psx-game.md` gains the two targets in its target table and a short
  paragraph in its "Short runs" section: the probe build, the patch contract,
  the delay helper, the two baselines and the invalid cases.
- The `Makefile` legend gains both targets, in the style of `psx-short-runs`.

## Implementation Chunks

1. **Analysis script and its host test.** The pure script and
   `tests/test_psx_probe.c` (name free), added to the `psx-tools-test` list.
   No PSX build involved.
2. **Probe build mode.** The `PSX_PROBE` option, the doubled buffer constant
   under it, the delay helper, and the GPU replay's start call and
   `psx-probe:` lines under it. Proven by `make test-all` and `psx-check`
   unchanged and by a probe-build configure and compile of one run (the
   delay helper has no host test: it is two clock reads in a loop, and the
   sanity probe is its test).
3. **Targets, fixtures, real-build verification.** `psx-probe-baseline` and
   `psx-probe`, the three fixtures, the Makefile legend and docs, then the
   baseline, the three sanity probes on all five short runs, and the report
   section.

## Testing Decisions

- A good test drives the analysis script as a user would: hand-written TTYs,
  map and frame count in, `probe.txt` and exit status out. It checks the
  printed lines, never the script's internals.
- Tested module: the analysis script only, in `psx-tools-test` (outside
  `make test` and `make e2e`). Cases at least:
  - a valid run: exact window sums and deltas per part, percentages, window
    frames from a resume map and from a prefix map;
  - frames outside the window ignored (a big delta on the resume frame does
    not enter the window sums);
  - game-state same; differs (first difference printed); zone, budget and cap
    lines differing do not count; one side shorter;
  - the delay line: delta ÷ delay over the run and per window frame, and the
    disagreement percentage against the counted calls;
  - invalid: probe stopped in `p8_fatal` (message quoted, no delta printed);
    baseline stopped; missing TTY; no done line; wrong frame count; asserts
    on; missing window frame line; malformed frame line; delay without
    calibration; calibration out of range;
  - usage errors exit 1; CR line ends (the emulator's TTY) are accepted.
- Not host-tested: the worktree and build orchestration (it is git, CMake and
  the emulator glued together; the sanity probes prove it on the real build)
  and the delay helper (proven by the delay sanity probe).
- Prior art: `tests/test_psx_short_run.c` (hand-written TTYs and map for
  `tools/psx_short_run.sh`, run through `sh`, outputs compared line by line),
  and the other `test_psx_*` host tests of `psx-tools-test`.
- Unchanged and must stay green: `make test-all`, every `tests/e2e/*.sha256`,
  `psx-check`.

## Out of Scope

- Any probe of Phase 2–4 (level-1 draw children, the draw tree, backend
  experiments, update probes, global levers): this step only builds and proves
  the tool.
- The asserts-off option of `psx-short-runs` and step 2's baseline (step 2).
- The PC call-count tool and the coverage check (step 4), the asserts-off full
  run (step 5).
- Choosing a remedy if the noise floor is too high (the plan revisits it
  before Phase 2).
- Probing the breakdown build, `psx-game`, or the core replay; patches that
  need generated units rebuilt; RAM-room checks of probe builds.
- Running more than one boot per probe or judging determinism of probes.
- Any change to the game, the shipped drawing path, the PC build, or the
  e2e references.

## Further Notes

- Decisions taken without a human (no QUESTION was asked): probe baseline
  distinct from step 2's, fingerprint on `src/`, `psx/` and `full_game.json`;
  2× buffer room; fine-clock delay with a self-counting helper; one boot per
  probe; 0.5% noise-floor rule; 2% and ±10% sanity tolerances; fixtures
  committed under `psx/probes/`; default ceiling 1080 vsyncs and timeout 20 s.
  Each follows the plan's Rules (relative comparison on deterministic runs,
  nothing kept from experiments but the tool, no slower fast loop).
- The 10% tolerance of the call-twice sanity probe is the plan's own. The draw
  part is measured around `cart_draw` alone (the meter's and the filter's
  frame ends come after its mark), so the meter's and the filter's per-call
  costs double with it; a ratio away from 2 points at per-frame work inside
  `cart_draw` that a second call does differently (a cache, a camera reset).
  The report states the measured ratio per run.
- If a sanity probe fails, the step note records it and the tool is fixed in
  this step; Phase 2 does not start on an unproven probe.
