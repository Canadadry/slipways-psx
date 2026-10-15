---
title: "Backend side: per-unit cost of every draw family, the 9x print and the meter (plan 20, phase 2, step 8)"
description: "The report knows how much each draw family weighs in total, but not what one call, glyph or primitive costs inside the backend and the meter, nor what the two known backend-side levers (9x outlined print, shipped draw meter) are worth."
status: needs-triage
---

## Plan step (verbatim)

> 8. Backend side: for each draw family, per-unit cost (hblanks per call,
>    glyph or primitive) from the breakdown boot, and the ways to do less
>    inside the backend and the meter. Two experiments run here: the 9× print
>    bounded by a one-print throwaway, and the meter removed from the measured
>    replay. Verified when each family has a per-unit cost and each
>    experiment a measured delta.

Plan file: `docs/plan-20.md`. Rules in force (section 2): the principle is
to **do less**; micro-optimizations rank as a supplement; the measuring loop
is the short runs judged before vs after on themselves, with
`PSX_SHORT_RUN_REFERENCE=` empty; throwaway experiments run in a temporary
worktree or branch, are measured and discarded, and **nothing is kept from an
experiment except its numbers in the report**; the only code the plan commits
is measuring tooling; asserts are off in the measured replays. Guardrails:
nothing in the game or in the shipped drawing path changes; `make e2e`, every
`*.sha256` and `psx-check` stay as they are; new tooling is host-tested where
the repo host-tests its tools and stays out of `make test` and `make e2e`;
never launch the raylib app. "Already known": `hud_print_outlined` calls
`p8_print` nine times per text (8 outline offsets, then the text); the draw
meter is installed in the shipped game only to size `psx_gpu_frame_end`.

Step notes in force:

- **1.2**: `make psx-short-runs PSX_SHORT_RUN_ASSERTS=off` builds and boots
  `psx-gpu-replay` twice and `psx-gpu-replay-breakdown` once per run;
  `<run>/analysis/summary.txt` prints per family the breakdown boot's
  window hblanks, calls and share of draw, plus `in-calls`,
  `outside-calls` and `overhead` (breakdown draw minus plain draw).
- **1.3**: `make psx-probe-baseline` / `make psx-probe PATCH=<file>
  [PSX_SHORT_RUNS=] [PSX_PROBE_LABEL=]`: probe build of the GPU replay,
  asserts off, GPU frame buffer doubled; per run window draw, update,
  submit deltas and `game-state same|differs`; noise floor 0; a part's
  resolution floor is 2 hblanks over the window.
- **1.4**: `make frame-profile` writes per frame the PC calls and time per
  draw family; PC and PSX family calls are equal on every window frame of
  `draw-5564` and `draw-6000` (`frame-profile-breakdown-check`). Any
  committed change under `src/` makes the probe baseline stale.
- **2.6**: hand-off to step 7 (`draw_layers#1` everywhere, `draw_layers#2`
  on `draw-5564`); step 7 is still running while this PRD is written (see
  the H-7 hypotheses).

## Problem Statement

The report says `print` holds 42% of `draw-5564`'s draw and `spr` 22%, and
that 70–79% of draw is spent inside backend calls. That is a split of a
total. It does not say what **one** call costs in each family, nor what one
glyph or one primitive costs, so nobody can turn "this function draws 40
fewer sprites a frame" (step 7's candidates) into hblanks, and step 12 has
no unit price to apply to any lever that removes draw calls.

It also does not say what happens **inside** a call. Every backend call in
the measured replay goes through a stack of wrappers: the breakdown timer
(breakdown boot only), the draw meter (which runs the cost model on every
call), the draw filter (which runs the cost model a second time, and is not
in the shipped game), then the GPU backend itself (placement, clipping, CLUT
and pattern caches, packet writes). Which of those layers can do less is
unknown.

Finally, the two levers the plan already names have no figure: the outlined
text drawn nine times (likely the bulk of `print`) and the shipped draw
meter. Without a measured bound for each, they cannot be ranked against the
draw-tree candidates.

## Solution

From a commit at or after step 7, with no change to the game or the shipped
drawing path, and one committed tooling change:

1. **Units counted per family on PC.** The PC frame profile's per-family
   wrapper also counts, per frame and family, the cost model's content
   primitives ("units": glyphs for `print`, sprites for `spr`, pixels for
   `pset`, spans for the fills, and so on) and its texture-window switches.
   PC and PSX family calls are already proven equal frame by frame, and the
   game is the same on both, so these counts are the PSX's.
2. **A `unit-cost` report** joins each run's breakdown boot (PSX hblanks and
   calls per family per window frame) with the PC units over the same
   window, checks the calls match, and prints per family: calls, units,
   hblanks, **hblanks per call** and **hblanks per unit** (per glyph for
   `print`), per run and pooled over the runs.
3. **Fresh measurements** after the tooling commit: the probe baseline
   rebuilt, and one asserts-off `make psx-short-runs` of all six runs for
   the breakdown boots.
4. **Two throwaway experiments**, with the probe tool on all six runs:
   - **one-print**: `hud_print_outlined` draws only its last pass (the text
     itself, offset 0,0) instead of nine; the picture loses its outline, the
     game state must be unchanged;
   - **no-meter**: the measured replay does not install the draw meter and
     hands `psx_gpu_frame_end` the frame buffer's size, so its overrun check
     is vacuous and the backend's own packet overflow check stays.
   Each gives a window draw, update and submit delta per run. Each also gets
   a predicted delta from the per-unit costs, so the experiment doubles as a
   check of the per-unit table.
5. **"Do less" inside the backend and the meter**, from the code plus the
   per-unit costs, typed and bounded like step 7's candidates.
6. **A "Backend: per-unit costs" section** in `docs/psx-perf-levers.md`
   holds all of it, with a verification table: every family has a per-unit
   cost, each experiment a measured delta.

## User Stories

1. As the plan pilot, I want the PC frame profile to count, per frame and per draw family, the units the cost model charges, so that every family's work is known in glyphs, sprites or primitives, not only in calls.
2. As the plan pilot, I want `print`'s unit to be the glyph (every non-space character of a call whose box is on screen), so that the 9× print and any text lever are priced per glyph.
3. As the plan pilot, I want texture-window switches counted apart from content primitives, so that a family's per-unit cost is not muddled by the state changes another family causes.
4. As the plan pilot, I want the units counted with the cost model the meter already uses, so that the counting adds no second model of the backend.
5. As the plan pilot, I want the units written as extra columns at the end of the existing family lines, so that the breakdown and coverage modes, and step 7's subtree mode, keep parsing the profile unchanged.
6. As the plan pilot, I want a `unit-cost` mode that joins a run's breakdown boot with the PC units of the same window, so that the per-unit table is computed by a tested tool, never by hand.
7. As the plan pilot, I want the `unit-cost` mode to refuse a window where PC and PSX calls differ on any frame, naming the first mismatch, so that units are never divided by the wrong calls.
8. As the plan pilot, I want the per-unit table per run and pooled over the runs, so that both a run's own price and a stable overall price exist.
9. As the plan pilot, I want one make target that runs the profile on each short run and writes the unit-cost table, so that the whole table is one command.
10. As the plan pilot, I want a missing breakdown TTY to fail naming the `make psx-short-runs` to run, so that a stale or absent boot is never read.
11. As a developer, I want the new tooling tested in `make frame-profile-test`, so that it is checked beside the profile tool and outside `make test`.
12. As a developer, I want `make test` and `make e2e` untouched in content and duration, so that the fast loop stays fast.
13. As the plan pilot, I want the probe baseline rebuilt after the tooling commit, so that no experiment is refused as stale.
14. As the plan pilot, I want fresh asserts-off breakdown boots of all six runs on the commit measured, so that the per-unit costs come from the same code as the experiments.
15. As the human reading the report, I want each family's hblanks per call and per unit, so that I can price any lever that removes calls or primitives.
16. As the human reading the report, I want `print` priced per call and per glyph, so that a lever removing whole calls (one pass instead of nine) and a lever removing glyphs are both priced.
17. As the human reading the report, I want `pattern` priced inside its shapes and never added to them, so that patterned fills are not counted twice.
18. As the human reading the report, I want families with very few window calls (`cls`, `circ`, `rect` on some runs) marked as low-count, so that I do not over-read a ratio of tiny numbers.
19. As the human reading the report, I want the breakdown timer's own bias on per-call figures stated, so that I know the per-call costs read high by the timer's per-call overhead.
20. As the human reading the report, I want the per-call and per-unit figures stated as two divisions of one total, never added, so that nobody double-counts them.
21. As the plan pilot, I want the one-print experiment applied as a throwaway patch through `make psx-probe`, so that it is measured exactly like every other probe and never committed.
22. As the plan pilot, I want the one-print patch to keep the text pass at offset 0,0 with its colour, so that the remaining draw is the closest stand-in for one primitive per glyph from a pre-drawn outlined font.
23. As the plan pilot, I want the one-print experiment's game state checked, so that a print that touched game state would be caught.
24. As the human reading the report, I want the one-print delta per run with the print calls and glyphs it removed, so that the 9× print lever has a measured bound.
25. As the human reading the report, I want the one-print delta compared with the delta the per-unit costs predict, so that I know how far the per-unit table can be trusted for other levers.
26. As the human reading the report, I want the one-print bound described as an upper bound of the outlined-font lever's gain, so that the lever's remaining cost (one glyph per glyph, larger glyphs) is not forgotten.
27. As the plan pilot, I want the no-meter experiment to remove only the meter's install and its byte check from the measured replay, so that the delta is the meter's cost and nothing else.
28. As the plan pilot, I want the backend's packet overflow check kept in the no-meter build, so that an overrun still ends in `p8_fatal` and an invalid probe is never read as a delta.
29. As the human reading the report, I want the no-meter delta per run and per backend call, so that the meter lever is priced and can be compared across families.
30. As the human reading the report, I want the no-meter delta compared with the meter's expected share (cost-model calls per window), so that a surprise is noticed.
31. As the human reading the report, I want the overlap between the two experiments named (fewer prints also means less meter work), so that step 12 does not add both gains in full.
32. As the plan pilot, I want the removed print calls and glyphs counted by the PC profile on the patched tree in the throwaway worktree, so that the prediction uses counts, not guesses.
33. As the human reading the report, I want the backend's per-call work listed per family (wrappers, cost model, placement, caches, packets), so that the "do less inside the backend" candidates have a map.
34. As the human reading the report, I want each backend or meter candidate typed as repeated calls, identical redraws or recomputation, with its location, evidence, upper-bound gain and pixel risk, so that it can be ranked like step 7's candidates.
35. As the human reading the report, I want the draw filter named as a wrapper the judge's replay has and the shipped game lacks, with its cost-model call per backend call, so that this measurement bias is known to step 12 and the human.
36. As the human reading the report, I want candidates whose gain is estimated (not measured) labelled "estimated", so that I can tell them apart from the two experiments.
37. As the plan pilot, I want every experiment boot `valid` before its delta is recorded, an invalid one fixed and re-run, so that the report holds only valid measurements.
38. As the plan pilot, I want the experiment patches described in one line each in the report and kept only in `tmp/`, so that they can be regenerated and are never committed.
39. As the human reading the report, I want a verification table listing every family with its per-unit cost and each experiment with its measured delta, so that the step's verification is checkable line by line.
40. As the plan pilot, I want every assumption about step 7 written as a hypothesis with a check and a fallback, so that this PRD holds whatever step 7 finds.
41. As the plan pilot, I want no other experiment and no draw-tree descent run here, so that the step stays its own size.
42. As a developer, I want no change to the game, the engine, the runtime, the PSX platform code, `psx/` or the e2e files, so that every `*.sha256` and `psx-check` are untouched.

## Implementation Decisions

### Tooling (the only committed code)

- **Decision: units are counted on PC, not on the PSX.** The breakdown boot
  already gives PSX time and calls per family per frame; adding counts there
  would change the measured-run tooling (`psx-short-runs`' TTY format and
  verdicts) and the PSX boots are slow to iterate. The PC per-family calls
  are proven equal to the PSX's on every window frame (step 1.4), and the
  game is identical, so the cost model gives the same units on both. No
  guideline call: the plan allows committing measuring tooling and this
  choice changes no measurement.
- **Units are the cost model's** (`p8_draw_cost_*`), the meter's model:
  content primitives per call, the texture-window pair (`DR_TPAGE` +
  `DR_TWIN`) counted separately as a "switch". This is an upper bound of
  what the GPU backend emits (the backend clips per glyph, per span and per
  rect edge; the model counts whole calls on screen), so per-unit costs are
  slight lower bounds; the report says so. A second model of the backend's
  exact emission (running the platform placement code on PC) is rejected:
  it would duplicate `psx_gpu.c`'s logic in a tool.
- **Per family, a unit means:** `cls` 1 fill; `pset` 1 pixel; `line` 1
  line; `rect` 4 tiles; `rectfill` 1 tile or patterned sprite; `circ` 8 ×
  midpoint iterations; `circfill` 4 × midpoint iterations spans; `spr` 1
  sprite; `print` 1 glyph (non-space characters, call on screen);
  `pattern` the spans of patterned fills (inside `rectfill`/`circfill`,
  never added). A call off screen is 0 units but still 1 call, as the
  filter and the profile already count calls.
- **The profile's family wrapper** keeps its own cost-model window state
  (as the filter does) and accumulates units and switches per family per
  frame beside calls and time. Its interface gains the two fields in its
  per-family figures; nothing else about the bookkeeping changes.
- **Output:** each `frame <n> family <f> <calls> <ns>` line of `frames.txt`
  and each family line of `summary.txt` gains two trailing fields, `units
  <u> switches <s>`. Existing readers parse by leading position; the
  breakdown and coverage tests must stay green unmodified (the coverage
  mode's `NF == 6` filter on family lines is the known trap: it must be
  widened to accept the new field count, and that is the only change to the
  existing modes). Step 7's subtree mode is handled per H-7-2.
- **New mode `unit-cost` of `tools/frame_profile_check.sh`**: inputs a
  run's breakdown TTY, its PC `frames.txt` and the run's `map.txt` (window
  frames, as `breakdown` reads it); converts fine-clock ticks to hblanks with
  the TTY's calibration line, as `psx_short_run.sh` does; refuses (exit 1,
  first mismatch named) when calls differ on a window frame; prints per
  family a line
  `family <f> calls <c> units <u> switches <s> hblanks <h> per-call <x> per-unit <y>`
  (one decimal for hblanks, three for ratios; `per-unit -` when units are
  0), then `in-calls <h>` and `window-frames <n>`. A pooled mode reads
  several runs' outputs and prints the same lines summed.
- **Make target `frame-profile-unit-cost [PSX_SHORT_RUNS=<names>]`**
  (default: all six runs of the manifest): runs `psx-short-runs-cut`, then
  per run the profile of its `run.json` into
  `tmp/frame-profile/unit-cost/<run>/`, checks same-game as the other
  profile targets do, runs the `unit-cost` mode against
  `tmp/psx/short-runs/<run>/breakdown.tty.txt`, and writes the pooled table
  to `tmp/frame-profile/unit-cost/pooled.txt`. A missing breakdown TTY
  fails naming `make psx-short-runs PSX_SHORT_RUN_ASSERTS=off
  PSX_SHORT_RUN_REFERENCE=`. Never boots. Makefile legend entry added.
  Outside `make test`, `make test-all`, `make e2e`.

### Measurements

- **Order:** tooling committed → `make psx-probe-baseline` (all six runs) →
  `make psx-short-runs PSX_SHORT_RUN_ASSERTS=off PSX_SHORT_RUN_REFERENCE=`
  (all six runs, every verdict but `representative` ok) →
  `make frame-profile-unit-cost` → the two experiments. The baseline and
  the short runs are compared with step 2.6's (and step 7's, H-7-5) figures:
  the tooling touches no file the PSX build compiles, so window sums must be
  equal within 2 hblanks per part, otherwise investigate before any delta.
- **Runs:** all six. The two 15-frame draw windows (`draw-5564`,
  `draw-6000`) carry the headline per-unit costs; the one-frame windows add
  to the pooled figures. A family with fewer than 30 window calls on a run
  is marked "low count" there.
- **Breakdown bias:** the breakdown boot's in-call time includes the timer
  wrapper's own per-call work (one coarse read and the extra dispatch). The
  `cls` per-call figure (one fill packet, the cheapest call) is reported as
  the floor reference; no correction is applied. The breakdown-vs-plain
  overhead from `summary.txt` is quoted beside the table.

### Experiments (throwaway, probe tool, all six runs)

- **one-print** (`PSX_PROBE_LABEL=b-one-print`): in `hud_print_outlined`,
  the loop over the nine outline offsets runs only its last entry (offset
  0,0, colour `15 & c`). Patch on `src/game/hud.c`, base commit recorded.
  Records per run: window draw/update/submit deltas, game state (must be
  `same`; `differs` is recorded as a finding and the delta still reported,
  labelled).
- **no-meter** (`PSX_PROBE_LABEL=b-no-meter`): in the GPU replay's
  `main`, the `p8_draw_meter_install` call is removed and
  `psx_gpu_frame_end` receives `GPU_FRAME_BUFFER_BYTES` instead of the
  meter's last frame bytes. Patch on `src/cmd/psx_gpu_replay.c`. The draw
  filter and everything else stay. The measured replay is the plan's stand-in
  for `psx-game` (same wrapper, same backend); the report says the lever is
  "drop the meter from `psx-game`", where it sits at `src/cmd/psx_game.c`.
- **Predictions** (per run, written next to each measured delta):
  - one-print: removed `print` calls × per-call cost and removed glyphs ×
    per-glyph cost, from the run's own per-unit row; the removed counts are
    HEAD's minus the patched tree's PC units, from `make frame-profile` run
    in the throwaway worktree on the run's `run.json` (worktree and outputs
    discarded);
  - no-meter: none from the per-unit table (the meter is inside every
    family's time); instead the delta divided by the window's total backend
    calls gives the meter's mean cost per call, reported beside the
    `cls` floor.
- **Gap rule:** prediction vs measurement off by more than 15% is
  explained (timer bias, outside-call work such as `hud_print_outlined`'s
  own loop and fixed-point maths, upper-bound units) or named as open.
- **Overlap:** the one-print delta includes the meter's and the filter's
  share of the removed prints; the report names it, step 12 settles it.
- **Validity:** a boot that is not `valid` is never a figure; the cause is
  read, the patch fixed, the probe re-run.

### "Do less" inside the backend and the meter

- **Sources:** the code of the wrappers and the backend per family (meter,
  filter, GPU backend and its placement, CLUT and pattern caches, text
  placement) read at the measured commit, plus the per-unit costs and the
  experiments.
- **Expected entries to examine** (not a suspect list; each is kept only
  if the code confirms it): the cost model run twice per call in the judge's
  replay (meter, then filter); the cost model's per-call string walk for
  `print` and midpoint loop for `circ`/`circfill`; per-glyph lit-row test and
  font-cell lookup in text placement, repeated nine times per outlined text;
  per-call CLUT lookup by memcmp in the meter and in the backend's cache for
  `spr`; per-sprite texture-page and region lookups; per-call texture-window
  checks.
- **Each candidate:** kind (repeated calls / identical redraw /
  recomputation), location (function, file:line at the measured commit),
  evidence, upper-bound gain per window frame per run (from the per-unit
  costs × the units or calls it removes, labelled "estimated"; the two
  experiments' figures labelled "measured"), pixel risk (identical /
  `visual_random` only / changes pixels — the last kept and marked "ruled out
  by the plan unless reopened"; the one-print stand-in itself changes
  pixels, the outlined-font lever it bounds does not).
- **The draw filter** is recorded as a finding, not a lever: it wraps every
  call in the judge's replay and the short runs, runs the cost model once
  more per call, and is absent from `psx-game`. Its cost is not measured
  here (a third experiment is out of this step's scope); the report names it
  so step 12 and the human know the judge reads high by it.
- **Not ranked here:** ranking is step 12's.

### What is recorded in `docs/psx-perf-levers.md`

A new section **"Backend: per-unit costs"**, after step 7's "Draw tree"
(H-7-6), and one clause in the opening paragraph naming it:

- measured commit, commands, method in a few sentences (units from the cost
  model on PC, time from the breakdown boot, the bias);
- the rebuilt probe baseline and short-run sums beside the previous ones;
- the per-unit table per draw run and pooled (family, calls, units,
  switches, hblanks, per call, per unit), `pattern` marked "inside its
  shapes";
- the two experiments: patch line, per-run deltas (draw, update, submit,
  game state), removed calls and glyphs, prediction, gap;
- the backend/meter candidates and the draw-filter finding;
- the verification table: each family with its per-unit cost (pooled
  value and runs), each experiment with its measured delta;
- the patch list (one line each) and the note that they were discarded.

Integer hblanks for window sums, one decimal for per-frame figures, three
decimals for per-unit ratios.

### Hypotheses about step 7 (not done yet; drafted in parallel)

Step 7's PRD is `docs/prd/triage/phase2-step7-draw-tree.md`. Each assumption
is checked when this step starts.

- **H-7-1 — step 7 is committed, with a "Draw tree" section in the report.**
  Check: the section in `docs/psx-perf-levers.md` at `HEAD` and step 7's
  note in the plan. Fallback: this step's work does not depend on step 7's
  numbers (per-unit costs come from the breakdown boot, the experiments from
  the probe tool), so it runs; its section goes after the last draw section
  present and the cross-references to the tree (H-7-3, H-7-4) are left out
  and named as such.
- **H-7-2 — step 7's committed tooling is the subtree root of the PC
  profile** (an optional root argument, a `root draw <symbol>` header line
  in `frames.txt` and `summary.txt`, a `subtree` mode in
  `tools/frame_profile_check.sh`, a `frame-profile-subtree` target). Check:
  `git log` since `8c841e7 -- src/devtools src/cmd/frame_profile.c tools
  Makefile tests`. Fallback: whatever step 7 committed, this step only
  appends two fields to the family lines and adds a mode and a target; every
  existing mode, step 7's included, must keep its tests green unmodified
  except the field-count filters, which are widened. If step 7 changed the
  family line format itself, the units are appended after its last field.
- **H-7-3 — step 7 located `hud_print_outlined` in the tree with its
  `p8_print` calls per frame per run.** Check: the "Draw tree" section.
  Fallback: the removed calls and glyphs come from the patched-tree PC
  profile in any case (Experiments); step 7's figure is only a cross-check
  and is left out if absent.
- **H-7-4 — step 7's hand-off names the runtime leaves (families) and
  their heaviest parents.** Check: its hand-off paragraph. Fallback: the
  per-unit section lists families from the breakdown boot alone, without
  naming parents.
- **H-7-5 — step 7 changed nothing the PSX build compiles** (no change to
  `src/` outside devtools and `cmd/frame_profile.c`, `psx/`, the probe tool
  or the manifest), and rebuilt the probe baseline. Check: `git log` since
  `8c841e7 -- src psx tools/psx_probe.sh psx/short_runs.txt` and step 7's
  note. Fallback: the comparison with the previous baseline is labelled
  "cross-code"; if the manifest gained or lost runs, this step measures
  exactly the manifest's runs.
- **H-7-6 — step 7's section is the last section of the report.** Check:
  section order. Fallback: this step's section goes right after "Draw tree"
  wherever it is (or after "Draw level 1" if H-7-1 fails); opening-paragraph
  conflicts are merged keeping every clause.
- **H-7-7 — step 7 ran no backend experiment and recorded no per-unit
  cost**, per its own out-of-scope list. Check: its section. Fallback: any
  such figure is quoted beside this step's and not re-measured unless it was
  on a different commit.
- **H-7-8 — step 7 committed no probe patch.** Check: `git ls-files
  psx/probes`. Fallback: committed patches are left as they are; this
  step's patches stay in `tmp/`.

### Not decided by a guideline call

Every question was settled from the plan and the code: units on PC (calls
proven equal, tooling committable), the cost model as unit (the meter's
model, an upper bound stated), the one-print shape (the last pass is the
text), the no-meter shape (byte check made vacuous, packet overflow check
kept), the filter recorded but not measured (the step names exactly two
experiments). No `guideline` agent was needed.

## Implementation Chunks

1. **Units in the PC profile.** The family wrapper counts cost-model units
   and switches per family per frame; `frames.txt` and `summary.txt` family
   lines gain `units <u> switches <s>`; the coverage mode's field-count
   filter widened. Tests in `test_call_profile` (units and switches per
   family on hand-made calls: a print with spaces, an off-screen spr, a
   patterned rectfill counted under its shape and under `pattern`, a window
   switch) and `test_frame_profile` (the built binary on
   `tests/e2e/pad_title.json` writes the new fields; breakdown and coverage
   tests unchanged and green). Commit.
2. **`unit-cost` mode and `frame-profile-unit-cost` target.** Mode tested in
   `test_frame_profile_check` on hand-written breakdown TTYs, profiles and
   map files (ratios, calibration, a calls mismatch refused, zero units,
   pooled sums). Target and legend entry. Commit.
3. **Measurements.** Pre-check hypotheses H-7-1 to H-7-8; probe baseline
   rebuilt; asserts-off short runs of all six runs; `make
   frame-profile-unit-cost`; write the section head, the baselines and the
   per-unit tables. Commit the report.
4. **Experiments and candidates.** The one-print and no-meter probes on all
   six runs, the patched-tree PC counts, predictions and gaps; the
   backend/meter candidates, the filter finding, the verification table and
   the patch list. Commit the report.

## Testing Decisions

- A good test checks the tool's observable contract (exit status, the lines
  it writes, the figures it computes from given inputs), never internals.
- **Tested:** the family wrapper's unit and switch counts
  (`tests/test_call_profile.c`, prior art: its existing family-wrapper tests
  counting calls and pattern under shape); the profile binary's new fields
  end to end (`tests/test_frame_profile.c`, prior art: it already runs the
  built binary on `tests/e2e/pad_title.json`); the `unit-cost` mode
  (`tests/test_frame_profile_check.c`, prior art: the `breakdown` mode's
  tests on hand-written TTYs and profiles). All in `make frame-profile-test`,
  outside `make test`, `make test-all` and `make e2e`.
- **Default behaviour pinned:** the breakdown and coverage tests (and step
  7's subtree tests, H-7-2) stay green unmodified apart from field-count
  filters.
- **Not tested:** the measurements, the experiments and the report. Their
  proof is the tools' own verdicts (`valid`, game state, calls match) and the
  prediction-vs-measurement gaps.
- `make test-all` stays green and is not slower; no `*.sha256` moves;
  `psx-check` is not re-run since no file the PSX build compiles changes;
  `psx-tools-test` stays green as the probe's prerequisite.

## Out of Scope

- Going down the draw tree, or any game-side "do less" candidate (step 7).
- Any experiment other than one-print and no-meter: no outlined-font
  prototype, no filter removal, no backend rewrite to bound a candidate.
- Measuring submit's content, asserts on vs off, `-O` levels (step 11).
- The update side (steps 9–10).
- Ranking, cumulative estimates, carrying gains onto the full run (step 12).
- Any full run; anything on the Miyoo or PCSX-ReARMed; audio; launching the
  raylib app.
- Committing experiment patches; changing the game, the engine, the
  runtime, the PSX platform code, `psx/`, the probe tool or any e2e file.

## Further Notes

- Expected shape (not a target): on `draw-5564`, `print` at about 9,218
  hblanks for 4,437 calls (about 2.1 per call) and `spr` about 4,813 for
  7,541 (about 0.64 per call) in the step-2-era breakdown. If most prints
  come from `hud_print_outlined`, one-print should remove close to 8/9 of
  `print`'s calls and a large part of its time, minus the per-text work
  outside the calls.
- The breakdown boot measures time inside backend calls only: the one-print
  delta also contains the outside-call work of the eight removed passes
  (argument setup, `p8_print`'s own entry), which is why the prediction
  from per-call cost may read below the measured delta.
- The judge's replay stacks timer (breakdown only), meter, filter and GPU
  backend; `psx-game` stacks meter and GPU backend. The no-meter delta is
  therefore also the shipped lever, while the filter's cost is a bias of the
  judge, named for step 12.
- Hypotheses H-7-1 to H-7-8 are the only assumptions about step 7.
