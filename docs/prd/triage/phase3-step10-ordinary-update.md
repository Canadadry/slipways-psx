---
title: "Ordinary update: cost, call count and do-less candidates of update's heavy functions on the frames outside step 9's spike classes (plan 20, phase 3, step 10)"
description: "Three quarters of the judge's update time lies outside the 166 spike frames, about half of it outside cart_update60, and nobody knows which functions it goes to, what they cost on the PSX, how often they run, or what they could stop doing."
status: needs-triage
---

## Plan step (verbatim)

> 10. Ordinary update: update's share in the frames outside step 9's
>     classes, through the fixed-delay probes and PC proportions corrected by
>     step 9's calibration. The over-budget frames are step 9's classified
>     frames, never recounted here. Verified when update's children above 2%
>     of the mean window frame have a cost, a call count and their do-less
>     candidates in the report.

Plan file: `docs/plan-20.md`. Rules in force (section 2):
- 60 fps is strict: every frame (update, draw, submit) fits in 263
  hblanks. The judge is pcsx-redux at 263 hblanks.
- The principle is to **do less**: repeated calls, identical work,
  recomputation. Micro-optimizations rank as a supplement.
- **Hot paths are found top-down**: start at the update part's roots, then
  their direct children, and go down only into branches that weigh more
  than 2% of the mean window frame.
- **Update functions get a fixed delay** at entry. The delay measures the
  call count, not the cost. The cost comes from PC timing proportions,
  scaled to the PSX update total and corrected by a calibration.
- Throwaway experiments are allowed and discarded: **the only code the plan
  commits is measuring tooling**. No full run besides steps 5 and 9's.
- Guardrails: nothing in the game or the shipped drawing path changes.
  `make e2e`, every `*.sha256` and `psx-check` stay as they are. New
  tooling stays out of `make test` and `make e2e`. Never launch the raylib
  app.

Step notes in force:

- **1.3**: `make psx-probe PATCH=<file> [PSX_SHORT_RUNS=] [PSX_PROBE_LABEL=]`
  builds the GPU replay in a temporary worktree of HEAD with
  `-DPSX_PROBE=ON`, asserts off, one boot per run (ceiling
  `PSX_PROBE_CEILING`, 1080 vsyncs), and reports window update, draw and
  submit deltas against the probe baseline, `game-state same|differs` and
  `delay / calls`. `psx_probe_delay(d)` takes one value `d` in 1..200 per
  build. Noise floor 0; a part's resolution floor is 2 hblanks over the
  window. The step 1.3 delay probe shows one `cart_update60` call a frame.
- **1.4**: `make frame-profile` (game, engine and runtime built `-O2
  -DNDEBUG -finstrument-functions`) writes per frame the calls and PC time
  of each direct child of the update and draw roots, by call site.
  `full_game.json` profiles in about 7.5 s. PC times are noisy: per-frame
  minimum over K profiles.
- **1.5**: the judge table `docs/psx-perf-levers-full-run-frames.txt`
  (asserts off, `7eb512f`): 6,059 frames, 166 with update > 263.
- **2.6–2.8**: mean window frame and 2% threshold: `draw-5564` 1351 / 27.0,
  `draw-6000` 979 / 19.6 (short-run probe baseline). Step 7's method:
  count-only delay probes with a delay small enough for the boot ceiling
  (`d` 1 overshoots calls by about 3.5%), path-flag gating, "do less"
  candidates typed and bounded.
- **3.9** (committed, `1e5433b`; tooling `e8706be`, `e95956a`, `42c5f2c`;
  report section "Update spikes"): see "Step 9, as committed" below.

## Problem Statement

Step 9 explains the 166 frames whose update alone is over 263 hblanks. It
says nothing about update on the other 5,893 frames of the judge's full run,
and those hold most of update's time: from the committed judge table, the
5,893 frames with update ≤ 263 sum **461,457** hblanks of update (77% of the
run's 597,148), a mean of **78.3** hblanks a frame against a mean frame of
**838.0**. Their update runs from 8 to 263 (median 63, 90th percentile 132;
584 of them between 133 and 263).

Step 9 found that about **40 hblanks of every in-game frame's update** (about
4 on title frames) are not spent in `cart_update60` at all, but in
`input_step_run`, which the measured replay calls just before
`cart_update60`, inside the update timer. It is game code
(`src/game/input_step.c`, pad navigation, attract and cheat handling) that
the shipped `psx-game` runs every frame too. On an ordinary frame that is
about half the update, and every PC profile rooted at `cart_update60`
misses it.

Step 9 also found that its two-frame calibration does not hold on every
kind of frame: on frames holding an event and the save, the PC overstates
`ui_update` (ratio 0.76–0.94 by class) and understates the save
(1.17–1.38). Ordinary frames run yet other code (hover, pad navigation,
entity updates), on which no ratio has been measured.

So nobody knows which functions ordinary update goes to on the PSX, how
many times each runs a frame, what each costs per call, or what each could
stop doing. Without this step, step 12 has no update lever for the frames
it must bring under 263, and the report's update side stops at the spikes.

## Solution

On PC, plus two kinds of throwaway PSX probes, at `HEAD` (after `1e5433b`),
with no change to the game:

1. **The population.** The ordinary frames are the 5,893 judge frames with
   update ≤ 263, the complement of step 9's 19 classes. Their PSX update per
   frame is the judge table's. The mean ordinary frame gives the 2%
   threshold. The two short-run windows with no class frame, `draw-5564`
   and `draw-6000`, are reported beside it with their own thresholds.
2. **The update part has two roots.** The judge's update is
   `input_step_run` plus `cart_update60`. Both are profiled as update roots
   (step 9's fifth argument of `frame_profile`), so the input step is
   measured and descended like the rest, never folded into `cart_update60`.
3. **A calibration on ordinary frames.** Step 9's span patch, rebuilt with a
   sixth span around `input_step_run`, times the six top-level pieces of the
   update part on every frame of `start` (whose frames 0–99 other than 12, 58 and 91 are
   ordinary judge frames, played identically), `draw-5564` and `draw-6000`.
   Set against PC shares of the same frames, it gives each piece's ratio on
   ordinary code, and the input step's PSX size directly.
4. **The transfer check against step 9.** The ordinary ratios are set
   against step 9's two-frame ratios and its per-class spread. On ordinary
   frames where the save runs, the `ui_update` and save costs carry step 9's
   event-and-save band as an interval.
5. **Costs from calibrated PC shares**, renormalised to the judge's update
   on each ordinary frame, through a new `ordinary` mode and a
   `frame-profile-ordinary` target that walk both roots top-down until every
   function above the threshold is a leaf or explored.
6. **The update root in the `subtree` mode**, for the PC call counts on the
   short-run recordings.
7. **Fixed-delay probes**: one count-only delay probe per function above
   the threshold, on `draw-5564` and `draw-6000`.
8. **Do-less candidates** for every function above the threshold, and a new
   section **"Ordinary update"** in `docs/psx-perf-levers.md`, after
   "Update spikes", with a verification table.

## User Stories

1. As the human reading the report, I want update on the frames outside step 9's classes split among the functions that run it, so that I know where three quarters of update's time goes.
2. As the plan pilot, I want the ordinary frames taken as the judge frames with update ≤ 263, which are exactly the complement of step 9's class lists, so that no spike frame is recounted here.
3. As the human reading the report, I want the ordinary population's frame count, mean frame, mean update and 2% threshold printed, so that I can check the threshold the descent used.
4. As the human reading the report, I want `draw-5564` and `draw-6000` reported beside the full-run population with their own thresholds, so that the windows the draw steps used are covered too.
5. As the plan pilot, I want the update part treated as two roots, `input_step_run` and `cart_update60`, so that the 40 hblanks step 9 found outside `cart_update60` are measured, not spread over its children.
6. As the human reading the report, I want `input_step_run` given a cost, a call count and candidates like any other function above the threshold, so that a lever in the shipped game's input step is not missed.
7. As the plan pilot, I want `input_step_run` profiled as an update root on PC, so that its PC time and its children are known frame by frame.
8. As the plan pilot, I want the six top-level pieces of update timed on the PSX on ordinary frames, so that PC proportions are corrected on the code those frames run.
9. As the plan pilot, I want the calibration to reuse step 9's span mechanism with one more span around `input_step_run`, so that the method stays the one already validated and the input step is timed, not inferred from a remainder.
10. As the plan pilot, I want `start`'s frames 0–99 other than its three class frames (12, 58, 91) used as calibration frames, so that title and early-game ordinary frames are covered by frames identical to the judge's.
11. As the plan pilot, I want the calibration boots valid with the game state the same, the spans' overhead recorded, and the six spans' sum checked against each frame's update, so that a missing span or a double count is caught.
12. As the human reading the report, I want per piece and run the PSX hblanks a frame, the PSX share, the PC share, the ratio and the gap, so that the ordinary calibration is recorded as step 9's was.
13. As the human reading the report, I want the input step's PSX hblanks a frame on title and in-game frames, so that step 9's "about 40" becomes a measured figure.
14. As the human reading the report, I want the ordinary ratios set against step 9's two-frame ratios and its per-class spread, so that I see whether update code reads alike on PC and PSX across frame kinds.
15. As the plan pilot, I want ordinary frames where the save runs to carry step 9's event-and-save band on `ui_update` and the save, so that the spread step 9 found is not hidden behind a single ratio.
16. As the plan pilot, I want a node's PSX cost taken as its calibrated PC share times the judge's update on each frame, renormalised so the pieces sum to update, so that a ratio never creates or loses hblanks.
17. As the plan pilot, I want a deeper node to carry the ratio of the top-level piece above it, so that every node gets the best measured correction, as step 9 did.
18. As the human reading the report, I want every cost labelled with the confidence of the ratio it used, so that I know how far it holds on the PSX.
19. As the plan pilot, I want the descent to profile every node above the threshold as an update root and stop at leaves or under-threshold nodes, so that the plan's top-down rule is applied mechanically.
20. As the plan pilot, I want a node above the threshold in any of the three populations explored, so that a node heavy in a draw window but light on average is not missed.
21. As the plan pilot, I want the make target to loop until no node is open, with a level cap, so that the whole descent is one command.
22. As the plan pilot, I want a symbol without exactly one text address closed as unresolvable, as step 9's target does, so that the tool never profiles the wrong function.
23. As the plan pilot, I want a node reached through several call sites or parents flagged multi-site, so that I know its children's costs are an upper bound.
24. As the human reading the report, I want each node's mean calls a frame, maximum calls, frames it runs on, mean cost, 90th-percentile cost and share of update, so that I can tell an always-running node from an occasional one.
25. As the human reading the report, I want each explored node's self cost, so that a heavy loop body is not hidden behind light children.
26. As the plan pilot, I want the subtree summary to accept an update root, so that the PC calls a window frame on the short-run recordings are available for the delay probes.
27. As the plan pilot, I want the subtree mode's draw output unchanged, so that step 7's tooling keeps working.
28. As the plan pilot, I want one throwaway count-only delay probe per node above the threshold, run on `draw-5564` and `draw-6000`, so that each call count is measured on the PSX.
29. As the plan pilot, I want each probe's delay chosen so the boot stays under its ceiling, so that no probe is invalid for time.
30. As the human reading the report, I want the PSX call count to win when it differs from the PC's, with the gap explained, so that the call column is the PSX's.
31. As the human reading the report, I want each explored node's cost per call, so that a lever that removes calls can be priced.
32. As the human reading the report, I want do-less candidates for every node above the threshold, typed, located, bounded and with their risk, so that they can be ranked like the draw and spike candidates.
33. As step 12, I want each candidate's gain as hblanks a frame and as a relative gain on update over the ordinary frames, so that I can carry it over to the judge's frames.
34. As the human reading the report, I want a candidate on a function also on a step 9 class path, or the same as one of U1–U8, cross-referenced, so that the two steps' gains are not counted twice.
35. As the human reading the report, I want a verification table answering the step's "verified when" item by item, so that step 12 can rely on it.
36. As a developer, I want the new mode and the update-root subtree tested on hand-written inputs in `make frame-profile-test`, so that the tooling is checked outside `make test`.
37. As a developer, I want `make test` and `make e2e` untouched in content and duration, and no change to the game, the engine, the runtime, the PSX platform code, `psx/` or the e2e files, so that every `*.sha256` and `psx-check` are untouched.
38. As the plan pilot, I want every throwaway patch discarded after its numbers are recorded, so that nothing but tooling is committed.

## Implementation Decisions

### Step 9, as committed (checked on `1e5433b`)

These replace the drafting hypotheses H-9-1 to H-9-9. Each was checked
against the step 3.9 note, the report's "Update spikes" section and the
diffs of `e8706be`, `e95956a`, `42c5f2c`. All held, with the differences
noted.

- **Class membership (H-9-1, held).** 166 frames, 42 over 1000, in 19
  classes (C1–C19), each frame in exactly one, none open. The classified
  frames are exactly the judge frames with update > 263 (the classification
  selects them by that rule, and the per-class lists sum to 166). The
  ordinary frames are therefore selected by `update ≤ 263` on the judge
  table; no list file is needed.
- **K = 8, not 4.** Step 9 deviated to K = 8 because K = 4 left a class
  share at 119.3% from timing noise. This step profiles with
  `FRAME_PROFILE_REPEATS=8` too. The Makefile's default stays 4.
- **Calibration table (H-9-2, held).** On `start` (frame 12) and
  `update-5500` (frame 5500), against the window update: `ui_update` 1.00
  and 0.92, `save_at_frame_end` 1.11 (`update-5500`; "below" on `start`),
  `update_entities`, `frame_zone_reset` and `cart_flush_exit_queue` "below"
  on both. Against `cart_update60` itself (the spans' sum), `ui_update` 1.00
  and 0.95, the save 1.14. Labels: high 0.85–1.15, medium 0.67–1.5, low
  outside, uncalibrated.
- **Which ratio a child used (H-9-3, held, superseded here).** Step 9's
  spread used `ui_update=0.96,save_at_frame_end=1.11`. This step measures
  its own ratios on ordinary frames (below) and uses step 9's only as the
  transfer reference, because:
  - `update_entities` is "below" on both of step 9's runs, so any node under
    it would be uncalibrated, and on ordinary frames it is about a fifth of
    `cart_update60`'s PC time (step 1.4);
  - step 9's per-class ratios over 165 spike frames show the ratio depends
    on the kind of frame (next item), and step 9's report says step 10 can
    use them in place of the two-frame ones.
- **Per-class spread (step 9's addition).** From the C13 fallback run,
  against `cart_update60` itself, class medians: on frames holding an event
  and the save, `ui_update` 0.76–0.94 and the save 1.17–1.38; on save-only
  frames (C5, C7) the save 0.95–0.97 and `ui_update` 0.48–0.54 (a node of
  4%); wormhole (C4) and game start (C6) 1.00–1.02.
- **PSX hblanks per PC µs (H-9-4, held, not used).** 1.127 on `start`, 2.144
  on `update-5500`: 1.9× apart. Step 9 concluded that only shares carry
  over, not times, so this step drops the hblanks-per-µs transfer check the
  draft had and compares ratios instead.
- **Outside `cart_update60`.** The update timer opens before
  `input_step_run` and closes after `cart_update60` (`src/cmd/psx_gpu_replay.c`,
  `marks[0]` and `marks[1]`). Step 9 measured the remainder at 4.06
  hblanks on `start`'s frame 12, about 4 on title frames and about 40 on
  every in-game frame of both calibration runs, and 9.7–31.5 on the three
  C13 frames. `input_step_run` is in `src/game`, so `frame_profile`
  instruments it, and calls it outside both roots (`src/cmd/frame_profile.c`,
  before `cart_update60`). The shipped `psx-game` calls it every frame too
  (`src/cmd/psx_game.c`).
- **Update root (H-9-5, held).** `frame_profile <events.json> <out-dir>
  <symbols> [<draw-root> [<update-root>]]`; header `root update <symbol>`
  after `root draw <symbol>`; an unknown or ambiguous name, `cart_draw` or
  the draw root as update root exits 1. A root is entered wherever it runs
  while no root is active, so `input_step_run` as update root (draw root
  `cart_draw`) is entered once a frame from the replay loop.
- **Step 9's mode code (H-9-6, held in part).** The `spikes` mode reads
  `frame … update`, `child update` and `root update` lines inline in its own
  awk program; there is no shared helper function. Its `<symbol>=unresolvable`
  argument form and its target's symbol check (exactly one `T/t/W/w` address
  in `tmp/frame_profile.syms`) are reused as they are. The `subtree` mode is
  still draw-only: it requires a `root draw` line and takes `$6` (draw time)
  from `frame` lines.
- **Nothing measured moved (H-9-7, held).** Between `3fe4b8b` and `1e5433b`,
  only `src/cmd/frame_profile.c` and the Makefile changed under `src/`,
  `psx/`, `tests/e2e/` and the Makefile; the judge table did not change.
  The probe baseline built at `3fe4b8b` still describes `HEAD`; the probe
  tool's fingerprint check decides, and a stale baseline is rebuilt with
  `make psx-probe-baseline`.
- **Section order (H-9-8, held).** "Update spikes" is the report's last
  section and the opening paragraph names it. "Ordinary update" goes after
  it, with one clause added to the opening paragraph.
- **Span patches (H-9-9, held).** Both of step 9's patches are described in
  its "Patches (update spikes)" table and are git-ignored in
  `tmp/plan20-update/`. `u-calibrate.patch` (probe build, `PSX_PROBE`,
  per-frame records of every frame, 1024 at most) is the base of this
  step's calibration patch. `u-fallback.patch` records only frames whose
  spans sum to 200 hblanks or more, so it holds no ordinary-frame data and
  is not used. If `u-calibrate.patch` is gone or no longer applies to
  `HEAD`, it is rewritten from the report's description.

### The population and the threshold

- **Ordinary frames:** the 5,893 judge frames with update ≤ 263: update sum
  461,457, mean update 78.3, mean total 838.0, threshold 2% = 16.8 hblanks.
  The tool prints these figures and the implementer checks them against
  this line.
- **Windows:** `draw-5564` (5564–5578) and `draw-6000` (6000–6014), the only
  short-run windows with no class frame. Their costs use the judge's frames
  in the window. Their threshold is the lower of 2% of the judge's mean
  window frame (`draw-5564` 31.2, `draw-6000` 18.6) and the short-run
  threshold the report already uses (27.0 and 19.6).
- **Excluded windows:** `start`'s window (frame 12, C6), `update-4737`,
  `update-5500` and `update-5719` (C1). Their window frame is a class frame,
  even where the resumed run lost the spike (`update-4737` 118,
  `update-5719` 46); the report says so in one line. `start`'s other frames
  serve the calibration only.
- **A node is above** when its mean cost a frame reaches the threshold in at
  least one of the three populations. It is explored (its children
  profiled) and gets a cost, a call count and do-less candidates. A node
  above whose children are all under, or that has no child, is a **leaf**.
- The 90th-percentile cost is printed next to the mean for reading, not
  gating.

### The update part: two roots

- The tree's top is the **update part**, with two children:
  `input_step_run` and `cart_update60`. `cart_update60`'s five statements
  are its children, as in step 9. Every per-frame figure below uses `P(f) =
  T(input_step_run, f) + T(cart_update60, f)`, each the per-frame minimum
  over its own K-profile set rooted at it.
- The six **top-level pieces** are `input_step_run`, `frame_zone_reset`,
  `cart_flush_exit_queue`, `update_entities`, `ui_update` (with its
  `dispatch_context_mem` and `main_ui` arguments, as step 9 counted them)
  and `save_at_frame_end`. `cart_update60`'s PC self (between its
  statements, there is no code; the PC self is hook overhead) is kept as a
  seventh piece with ratio 1, so it is visible and never redistributed.

### Calibration on ordinary frames (throwaway)

- **Patch** (`tmp/plan20-ordinary/u-ordinary-calibrate.patch`, base commit
  recorded): step 9's `u-calibrate.patch` (H-9-9) plus a sixth span around
  the `input_step_run` call of the GPU replay's frame loop, between
  `marks[0]` and `cart_update60`. Same clock pair, same 200-hblank rule,
  same per-frame records printed after the last frame, one line
  `probe-update: frame <n> input <h> zone <h> exit-queue <h> entities <h>
  ui <h> save <h>` (hundredths of an hblank).
- **Run:** `make psx-probe PATCH=tmp/plan20-ordinary/u-ordinary-calibrate.patch
  PSX_SHORT_RUNS="start draw-5564 draw-6000" PSX_PROBE_LABEL=u-ordinary-calibrate`.
- **Calibration frames:**
  - `start`: run frames 0–11 (title) and 13–99 (in game) except 58 (C5)
    and 91 (C19), the two other class frames in that range (checked on the
    judge table). `start` is a prefix run, so these frames are the judge's
    own.
  - `draw-5564` and `draw-6000`: every run frame after the resume's first
    game frame (from each run's `map.txt`), window and lead-in, since a
    resumed run's frames are ordinary game frames; `draw-6000`'s resumed game
    diverges from the full one (the report already says so), which does not
    matter to a ratio measured on the same recording on both sides.
- **Validity:** every boot `valid`, `game-state same`; the spans' overhead
  (window update delta against the probe baseline) recorded; on every frame
  the six spans sum to at most the frame's update (the boot's own frame
  line) and the remainder (frame-timer reads) is at most 2 hblanks on
  average. A failure is investigated and the probe re-run, never read.
- **PC side:** each run's `run.json` is profiled K = 8 times rooted at
  `cart_update60` and K = 8 times rooted at `input_step_run` (draw root
  `cart_draw`), same-game checked, into
  `tmp/frame-profile/ordinary-calibration/<run>/<root>/<k>/`. Per frame, a
  piece's PC time is its per-frame minimum, the denominator `P(f)`.
- **Ratios:** per run and piece, pooled over the run's calibration frames,
  `ρ = (Σ PSX hblanks ÷ Σ PSX update) ÷ (Σ PC ns ÷ Σ P)`, and the gap in
  points; per piece, also the three runs pooled together. `start`'s title
  frames are pooled separately (`start-title`), since title and game run
  different code. A piece under 2% on both sides is "below" and gets no
  ratio (1 in use). The commands and the small awk that pools them are
  written in the report; the figures are few, so no tool is committed for
  them, as step 9 did.
- **Recorded:** per run (`start-title`, `start-game`, `draw-5564`,
  `draw-6000`, pooled) and piece: PSX hblanks a frame, PSX share, PC share,
  ρ, gap; the input step's PSX hblanks a frame (mean, min, max) on title and
  in-game frames; the spans' overhead.
- **Ratio in use:** each piece's pooled in-game ratio (title frames use
  `start-title`'s, where it is not "below"). Its **label** comes from the
  spread of its per-run ratios (`start-game`, `draw-5564`, `draw-6000`),
  with step 9's thresholds: all within 0.85–1.15 of 1 is high, within
  0.67–1.5 medium, otherwise low; "below" everywhere is uncalibrated.

### Transfer check against step 9

- **Reference figures** (step 9, against `cart_update60` itself):
  `ui_update` 1.00 and 0.95 on its two frames, 0.76–0.94 on event-and-save
  classes, 0.48–0.54 on save-only classes; save 1.14 on frame 5500,
  1.17–1.38 on event-and-save classes, 0.95–0.97 on save-only classes.
  For comparison, this step's `ui_update` and save ratios are also given
  against `cart_update60` alone (input step left out).
- **Reported:** the ordinary ratios next to those ranges, and whether each
  falls inside, above or below. A difference is a finding, not a failure:
  ordinary frames run different code.
- **Ordinary frames where the save runs.** A frame is a **save frame** when
  `save_at_frame_end` holds at least 2% of `P(f)` on PC. On save frames,
  the calibration windows give no ratio for the save (they hold few or
  none), so:
  - the point estimate uses the ordinary `ui_update` ratio and step 9's save
    ratio 1.14;
  - the cost of `ui_update`, the save and every node under them on those
    frames is also given as an interval, computed with step 9's
    event-and-save band (`ui_update` 0.76–0.94 with the save 1.38 at one end,
    0.94 with 1.17 at the other) and the save-only band (save 0.95–0.97),
    whichever bounds are wider;
  - the report gives the save frames' count and their share of ordinary
    update. When that share is under 5%, the interval is printed but the
    label stays the ordinary one; at 5% or more, the label of `ui_update`,
    the save and their nodes is lowered one step.

### Cost from calibrated PC shares

For an ordinary frame `f`, with `T(n, f)` the per-frame minimum of node `n`'s
PC time over the set rooted at its parent (for a top-level piece, its own
root set or `cart_update60`'s set):

- each top-level piece `c` gets `q(c, f) = T(c, f) / P(f) × ρ(c)`, the
  `cart_update60` self `max(0, T(cart_update60, f) − Σ its children) /
  P(f)` with ratio 1;
- `N(f) = Σ q`; the calibrated share of piece `c` is `q(c, f) / N(f)`;
- a deeper node `n` under piece `c` gets `T(n, f) / P(f) × ρ(c) / N(f)`;
- its PSX cost on `f` is that share times the judge's update on `f`;
- a frame where `P(f)` is 0 counts every share as 0;
- on save frames, the interval of the transfer check is computed the same
  way with the band's ratios.

Per population: mean cost a frame, 90th percentile, share of the
population's update, mean and maximum calls a frame (first profile of the
parent's set), and the count of frames the node runs on. Cost per call is
mean cost over mean calls. Costs a frame and per call get one decimal,
shares one decimal, ratios two decimals.

### Tooling (the only committed code)

- **`subtree` mode accepts an update root.** An optional leading
  `part=update` (default draw). For update it requires a `root update` line,
  the same in every profile, and takes the root's time from the `update`
  field of the `frame` line (`$4`), the `child update` lines and the `self
  update` line. Draw output stays byte-for-byte.
  - `make frame-profile-subtree ROOT=<symbol> PART=update` passes the
    symbol as update root with `cart_draw` as draw root, into
    `tmp/frame-profile/subtree-update/<symbol>/`. `PART` defaults to draw.
  - Trap: the subtree mode reads `$3 == "update" && NF == 6` and takes `$6`,
    the **draw** time; the update part must take `$4`.
- **New mode `ordinary` of `tools/frame_profile_check.sh`.**
  - Usage: `ordinary <threshold-pct> <spike-threshold> <judge-frames.txt>
    <ratios> <windows> <cart_update60 frames.txt>[,...]
    <input_step_run frames.txt>[,...] [<symbol>=<frames.txt>[,...]|unresolvable]...`.
    - Ordinary frames are the judge frames with update ≤ `<spike-threshold>`.
    - `<ratios>` is `<piece>=<ratio>[,...]` or `-`; a missing piece is 1.
      `<piece>` may carry an interval, `<piece>=<ratio>:<low>-<high>`, used
      on save frames only.
    - `<windows>` is `<name>=<first>-<last>[@<extra-threshold>][,...]` or
      `-`; the lower of the computed and the extra threshold applies.
    - Each `<symbol>=` set is rooted at that symbol; every set has K files.
  - Checks: every profile has the judge table's frame count; the two base
    sets' `root update` lines are `cart_update60` and `input_step_run`;
    each other set's is its symbol; malformed input exits 2.
  - A set is placed in the tree from the two base sets down, by `<symbol>`
    matching a child name of an already-placed set (with or without its
    `#<site>` suffix). A symbol under several parents or sites is
    `multi-site`. A set that cannot be placed is a usage error.
  - Output, in this order:
    - `population <name> frames <n> excluded <m> mean-total <t> threshold
      <h> mean-update <u> save-frames <s> save-share <pct>`, for `ordinary`
      and each window;
    - per placed node, per population, in tree order: `node <path>
      population <name> calls <mean>-<max> runs-on <r> cost <mean>
      [<low>-<high>] p90 <h> per-call <h> share <pct> above|under
      [multi-site] [leaf] [open] [unresolvable]`, where `<path>` starts at
      `update/input_step_run` or `update/cart_update60`;
    - per explored node and population: `self <path> population <name> cost
      <mean> share <pct> above|under`;
    - `open <symbol>` for each node above with no set rooted at it;
    - totals `nodes-above <n> leaves <l> open <o>`.
  - Exit 0 when nothing is open, 3 when something is, 1 on a usage error.
- **Make target `frame-profile-ordinary [FRAME_PROFILE_REPEATS=<k>]
  [FRAME_PROFILE_THRESHOLD=<pct>] [FRAME_PROFILE_SPIKE_THRESHOLD=<hblanks>]
  [UPDATE_RATIOS=<ratios>]`:**
  - profiles `worst-case/full_game.json` K times rooted at `cart_update60`
    and K times rooted at `input_step_run` (draw root `cart_draw`) into
    `tmp/frame-profile/ordinary/<symbol>/<k>/`, then runs the mode on the
    judge table with the windows `draw-5564@27.0` and `draw-6000@19.6`;
  - while the mode exits 3, profiles each `open` symbol K times as update
    root, with step 9's symbol check (a symbol without exactly one text
    address is passed as `<symbol>=unresolvable` and recorded in
    `tmp/frame-profile/ordinary/unresolvable.txt`), and runs the mode again,
    16 levels at most;
  - checks same-game on every profile; writes the last output to
    `tmp/frame-profile/ordinary/ordinary.txt`;
  - never boots; gets a legend entry; stays outside `make test`, `make
    test-all` and `make e2e`.
  - It keeps its own directory: step 9's `spikes/` profiles are not reused,
    so neither target can stale the other.

### Fixed-delay probes (throwaway)

- One patch per node above the threshold, except `cart_update60` (one call
  a frame, step 1.3) and `input_step_run` (one call a frame by the loop's
  code, checked once by its own probe if it is above, since it is new to
  the report): `psx_probe_delay(d)` at the node's entry, with `#include
  "platform/psx_probe.h"` as step 1.3's delay patch. A node reached through
  several sites is gated to the path's site with step 7's path-flag
  technique when its other sites would blur the count.
- Run with `make psx-probe PATCH=tmp/plan20-ordinary/<name>.patch
  PSX_SHORT_RUNS="draw-5564 draw-6000" PSX_PROBE_LABEL=<name>`.
- `d` is the largest value in 1..20 whose expected extra time (PC calls over
  the whole run × `d`) keeps each boot under `PSX_PROBE_CEILING` with a
  quarter to spare; at `d` 1 the report recalls step 7's +3.5% artefact.
- Valid when the boot is `valid`, the game state `same`, and the delay
  lands in update (draw and submit deltas within their 2-hblank floor).
- The PSX calls a window frame (delta ÷ `d` over the window ÷ 15) are set
  against the PC calls a window frame from `make frame-profile-subtree
  ROOT=<parent> PART=update PSX_SHORT_RUNS="draw-5564 draw-6000"`. Within 5%
  (or the `d` 1 artefact) is a match. A mismatch is investigated; the PSX
  count is the one the report keeps, with the gap explained.
- A node above only on the ordinary population and never called in either
  window gets no PSX count: its call column is "PC only, not called in a
  probed window". No window is added to `psx/short_runs.txt` (step 11
  measures every manifest run; a new run would change its scope).

### Do-less candidates

- Every node above the threshold gets at least one candidate, or an explicit
  "none found" with the reason, from its code and its figures. Each gives:
  - kind: repeated calls, identical work, or recomputation;
  - location: function and file:line at the commit measured;
  - evidence: calls a frame, cost a frame and per call, runs-on;
  - upper-bound gain: hblanks a frame on the ordinary population and on
    each window, and the relative gain on update over the ordinary
    population, labelled "estimated" with the cost's confidence label;
  - risk: "timing only" (game state and pixels the same), "game state",
    or "e2e re-record" (only if the spreading decision covers it).
- Candidates are numbered on from step 9's (`O1`, `O2`, …). One that is the
  same change as a step 9 candidate (U1–U8, for example U4's table CRC if
  the save shows here) names it and gives only its ordinary-frame gain; the
  report says the class frames are counted under the U candidate.
- For `input_step_run`'s nodes, the risk also says whether the change holds
  in the shipped `psx-game` (which calls the same function with live pad
  input) or only in the replay.
- Candidates are not ranked here: ranking is step 12's.

### What is recorded in `docs/psx-perf-levers.md`

A new section **"Ordinary update"**, after "Update spikes", and one clause
in the opening paragraph naming it. It holds:

- the commit measured, the commands, and "Step 9, as committed" in a few
  lines;
- the method: population, two roots, ordinary calibration, transfer check,
  calibrated and renormalised shares, threshold;
- **the population table:** per population, frames, excluded, mean total,
  threshold, mean update, save frames and their share;
- **the calibration table** and the input step's figures;
- **the transfer table:** ordinary ratios against step 9's;
- **the tree table:** per node, path, ratio and label, per population cost
  a frame (with its interval where save frames apply), p90, share of
  update, calls (PC and PSX), cost per call, runs-on, above/under, flags;
- the delay probes, one line each: node, `d`, deltas, PSX vs PC calls;
- per node above: its do-less candidates;
- the one line on the excluded short runs;
- **the verification table**, following the step's "verified when": every
  node above 2% in any population has a cost, a call count and its
  candidates;
- the throwaway patches (one line each) and the note that they were
  discarded.

### Not decided by a guideline call

Every question was settled from the plan, the committed report and the code:
- the population is the judge's ordinary frames, because the costs come
  from the judge's update and the short runs that hold a class frame cannot
  speak for ordinary ones;
- the input step is a second root, not a correction: it is game code the
  shipped build runs, the PC profile can see it once rooted, and folding it
  into `cart_update60`'s shares would hand half of ordinary update to the
  wrong functions;
- the calibration is step 9's span method repeated on ordinary frames:
  step 9 itself showed its ratios depend on the kind of frame, measured
  none for `update_entities` or the input step, and pointed step 10 at
  per-kind ratios. Step 9's figures stay the transfer reference and the
  save's ratio, which ordinary windows cannot measure;
- the threshold is 2% of the mean frame of each population, the lower one
  for the windows, so the descent is at least as strict as the draw steps';
- the delay probes count calls only, as the plan's update rule says.

No `guideline` agent was needed.

## Implementation Chunks

1. **Update-root subtree.** The `part=update` form of the `subtree` mode and
   the `PART` variable of `make frame-profile-subtree`. Tests in
   `test_frame_profile_check`: an update-rooted subtree summary over a
   window (root time from the `update` field, `child update` and `self
   update` lines), mismatched `root update` lines exiting 2, and the draw
   form unchanged (the existing subtree tests stay green unmodified).
   Commit.
2. **`ordinary` mode and `frame-profile-ordinary` target.** Tests on
   hand-written judge tables and profile sets:
   - the population lines, exclusion by the spike threshold, save frames
     and their share;
   - the two base sets and `P(f)`, with K-minimum on each;
   - ratios and renormalisation (a ratio moves shares, never the sum);
     the `cart_update60` self piece at ratio 1;
   - intervals on save frames only;
   - a frame with `P` 0;
   - a deeper node through a `<symbol>=` set, under each base root, and its
     inherited ratio;
   - a window with an extra threshold, the lower one applying;
   - above, leaf, `open` and exit 3; `<symbol>=unresolvable`;
   - a multi-site set;
   - mean, p90, per-call, runs-on;
   - a frame-count mismatch and a wrong `root update` exiting 2; usage
     errors exiting 1.

   The target with its two base sets, loop, cap, symbol check and legend
   entry. Commit.
3. **Ordinary calibration.** Write the calibration patch, run `make
   psx-probe` on `start`, `draw-5564` and `draw-6000`, profile the three
   recordings on PC (two roots, K = 8), compute the ratios and the transfer
   table. Write the section head, "Step 9, as committed", the method, the
   calibration and transfer tables and the input step's figures. Commit the
   report.
4. **Descent and costs.** Run `make frame-profile-ordinary
   FRAME_PROFILE_REPEATS=8 UPDATE_RATIOS=<chunk 3's ratios and the save
   bands>`. Write the population table and the tree table. Commit the
   report.
5. **Delay probes and candidates.** Run `make frame-profile-subtree
   PART=update` on the parents of the nodes above, then one delay probe per
   node above. Read each node's code for do-less candidates. Write the probe
   lines, the candidates, the excluded-runs line, the verification table and
   the patch list. Commit the report.

## Testing Decisions

- A good test checks the tool's observable contract (exit status, the lines
  it writes, the figures it computes from given inputs), never internals.
- **Tested:** the `ordinary` mode and the update form of the `subtree` mode,
  on hand-written inputs, in `tests/test_frame_profile_check.c` (prior art:
  the `subtree`, `coverage`, `unit-cost` tests there, and step 9's `spikes`
  and `spread` tests). They run in `make frame-profile-test`, outside `make
  test`, `make test-all` and `make e2e`.
- `input_step_run` as update root needs no new binary test: step 9's tests
  of the fifth argument (`tests/test_frame_profile.c`) cover any resolvable
  symbol.
- **Default behaviour pinned:** every existing test of
  `frame_profile_check.sh` (step 9's included) stays green unmodified.
- **Not tested:** the targets' loops beyond their mode (prior art: step 9's
  `frame-profile-spikes`), the throwaway patches and the calibration's awk,
  the descent's figures and the report. Their proof is the tools' verdicts
  (probe `valid`, game state `same`, same-game, frame counts, nothing
  open), the calibration's span-sum check, and the verification table.
- `make test-all` stays green and is not slower. No `*.sha256` moves.
  `psx-check` is not re-run: no file the PSX build compiles changes.

## Out of Scope

- The 166 spike frames in step 9's 19 classes: their causes, costs,
  candidates and spreads are step 9's. Their update is never recounted here.
- Re-measuring step 9's calibration on its own frames, re-running its
  fallback, or reclassifying any class with the ordinary ratios.
- Adding a short run to `psx/short_runs.txt`, or a full run.
- Draw, the backend and the meter (steps 6–8); submit, asserts and `-O`
  levels (step 11).
- Ranking levers, cumulative estimates and the per-frame verdict against
  263 (step 12).
- Implementing any lever. Changing the game, the engine, the runtime, the
  PSX platform code, `psx/`, the probe tool or any e2e file.
- Anything on the Miyoo or PCSX-ReARMed, audio, launching the raylib app.

## Further Notes

- Expected shape, not a target: about 40 of the 78.3 hblanks of a mean
  ordinary frame's update are the input step, so `input_step_run` is likely
  the largest node, with `pad_nav_step` (`src/game/pad_nav.c`, 1,155 lines)
  the first suspect under it. Inside `cart_update60`, step 1.4 put
  `ui_update` near 70% and `update_entities` near 19% of the full run's PC
  time, spike frames included; on ordinary frames, with about 38 hblanks
  left for `cart_update60`, `ui_update` is likely above 16.8 and
  `update_entities` near it.
- Step 9's three C13 frames read 9.7–31.5 hblanks outside `cart_update60`,
  less than the in-game 40: the input step is not constant, which is why
  it is profiled and calibrated per frame rather than taken as a fixed
  offset.
- The ordinary frames between 133 and 263 hblanks of update (584 of them)
  may hold the same events as step 9's classes at a smaller size (a project,
  a save alone). They are counted here, since they are outside the classes;
  the save-frame rule and the U cross-reference keep them apart from the
  spike figures.
- The draw windows' update (44 and 40 hblanks a frame on the judge) sits
  under the ordinary mean (78.3): the ordinary population, not the windows,
  drives the descent's verdicts.
