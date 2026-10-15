---
title: "Update spikes: calibrate PC update proportions, then classify every full-run frame whose update alone exceeds 263 hblanks (plan 20, phase 3, step 9)"
description: "166 of the judge's frames overrun the budget on update alone, and nobody knows which function each spike comes from, why it runs then, how far the PC's update proportions can be trusted, or how many frames each spike's work would need if spread out."
status: needs-triage
---

## Plan step (verbatim)

> 9. Identify the update spikes on PC: every full-run frame whose update
>    alone exceeds 263 hblanks (166 frames in the step 1.5 judge table, 42 of
>    them over 1000), not only the named frames 12, 4737, 5500 and 5719.
>    First, a calibration on update code itself: a throwaway fine-clock
>    timing of `cart_update60`'s children on the short runs that keep an
>    update spike (`start` for frame 12, `update-5500`), compared with the PC
>    proportions on the same frames, the gap recorded. (Draw children are no
>    guide: step 7 found PC proportions 17–47% low there, driven by backend
>    calls whose PC and PSX costs diverge.) Then the PC frame profile groups
>    the frames by responsible `cart_update60` descendant into a few classes,
>    and says why each runs then. If PC cannot settle a class, fall back to
>    the full run. Verified when the calibration gap is recorded and the
>    report lists every such frame in a class and gives, per class, the
>    responsible function, its share of update with the calibration gap as
>    its confidence, a do-less candidate, and the number of frames it would
>    take if spread out under 263 after draw's estimated levers.

Plan file: `docs/plan-20.md`. Rules in force (section 2):
- 60 fps is strict: every frame (update, draw, submit) fits in 263
  hblanks, loading frames included. The judge is pcsx-redux at 263
  hblanks.
- **Spreading work across frames is allowed on PC and PSX alike** for
  galaxy generation and the update spikes. The e2e hashes are then
  re-recorded once, explicitly, by the implementation plan.
- The principle is to **do less**.
- **The update spikes are identified on PC first.** **The full run is the
  fallback** if PC does not settle them, and it is the only full run the
  plan allows besides step 5's.
- Update functions are probed with a fixed delay (it counts calls). Their
  cost comes from PC proportions, which this step now calibrates on update
  code first.
- Throwaway experiments are allowed and discarded: **the only code the plan
  commits is measuring tooling**.
- Guardrails: nothing in the game or the shipped drawing path changes.
  `make e2e`, every `*.sha256` and `psx-check` stay as they are. New
  tooling stays out of `make test` and `make e2e`. Never launch the raylib
  app.

Step notes in force:

- **1.3**: `make psx-probe PATCH=<file> [PSX_SHORT_RUNS=] [PSX_PROBE_LABEL=]`
  builds the GPU replay in a temporary worktree of HEAD with
  `-DPSX_PROBE=ON`, asserts off, one boot per run. It reports update, draw
  and submit deltas against the probe baseline and `game-state same|differs`.
  The noise floor is 0, and a part's resolution floor is 2 hblanks over the
  window.
- **1.4**: `make frame-profile` replays a recording with game, engine and
  runtime built `-O2 -DNDEBUG -finstrument-functions`. Per frame, it writes
  the calls and PC time of each direct child of `cart_update60` and of the
  draw root, by call site. `full_game.json` (6,059 frames) takes about 7.5 s
  and plays the same game as the reference. Single PC profiles flip
  verdicts on timer noise, so K = 4 profiles with the **per-frame minimum**
  is the rule.
- **1.5**: the judge table (`docs/psx-perf-levers-full-run-frames.txt`,
  asserts off, commit `7eb512f`) has 166 frames with update > 263, 42 of
  them over 1000. Frame 12 is 9,451 total, 8,980 of it update. Frames 4737,
  5500 and 5719 have 1,783, 1,416 and 1,430 of update. The resumed short
  runs lose the 4737 and 5719 spikes and keep 5500's (1,409 against 1,416).
  `start` equals the judge exactly.
- **2.6 / 2.7**: draw's step-7 candidates (15 "do less" items) have
  per-frame upper bounds on `start`, `draw-5564` and `draw-6000` in the
  report's "Draw tree" section. PC-derived draw costs read 17–47% low
  against measured ones. `frame_profile` takes an optional **draw**-root
  symbol, and the update root is fixed at `cart_update60`.
- **2.8** (committed, `3fe4b8b`; tooling `1e4015a`, `17c2602`): see "Step 8,
  as committed" below. Snag: `draw-6000`'s breakdown boot exited at 542
  vsyncs against the 540 default ceiling of `psx-short-runs`. The plan
  change of the same review made this step start with a calibration on
  update code and limited step 10 to the frames outside this step's
  classes.

## Problem Statement

The judge's full run has 166 frames where update alone is over the 263-hblank
budget, 42 of them over 1000. Even if draw cost nothing, those frames would
miss 60 fps. The plan named four of them (12, 4737, 5500, 5719), and the
report's "Ten worst frames" already shows that the named ones are not the
only spikes of their size. The judge table says how much each frame's update
costs on the PSX. It does not say **which function** is responsible, **why**
it runs on that frame and not the next, or **whether** the 166 frames are a
handful of causes or many.

The only tool that can split update per function, frame by frame, is the PC
frame profile, and PC proportions are known to mislead. On draw they read
17–47% low against the PSX. That was driven by backend calls, which update
code does not make, so the draw gap says nothing either way about update.
Nobody knows how far a PC share of update can be trusted on the PSX.

Without all of this, nobody can name a "do less" lever for the spikes or say
how far spreading their work would go once draw shrinks. Step 12 then has no
figures to apply to these frames, though it must apply per-class figures to
each of them, and step 10 cannot correct its own PC-derived update costs.

## Solution

On PC, plus one throwaway PSX probe, at `HEAD` (after step 8's `3fe4b8b`),
with no change to the game:

1. **Calibration first.** A throwaway patch times each of
   `cart_update60`'s five statements (its children) on the PSX with the fine
   clock, unwrapped by the hblank counter. It prints one TTY line a frame,
   and the probe tool runs it on `start` (frame 12) and `update-5500` (frame
   5500). The same frames are profiled on PC (K = 4, per-frame minimum).
   Per child and run, the PSX share of update is set against the PC share,
   and the ratio and the gap in points are recorded. This happens before any
   classification uses a PC share.
2. **An update root for the PC frame profile.** `frame_profile` takes an
   optional update-root symbol, as it already takes a draw root, so any
   `cart_update60` descendant can be profiled: its own PC time per frame and
   its direct children per call site.
3. **A `spikes` mode and a `frame-profile-spikes` target.** They read the
   judge table and pick every frame whose update is over 263. Using K = 4
   full-run profiles per root, they walk each frame down from
   `cart_update60` to the **responsible descendant**: the deepest node that
   still holds at least half of the frame's PC update. They group the
   frames into classes by that node's path. Per class they print the
   frames, the responsible function, its PC share of update, and how often
   the function runs outside spike frames. The target profiles the next
   level by itself until every frame's descent ends.
4. **Confidence from the calibration.** Each class's share is carried
   through the ratio of the `cart_update60` child its path goes through.
   That gives an interval and a confidence label (high, medium, low or
   uncalibrated). A low-confidence class whose interval straddles 50% counts
   as "PC cannot settle".
5. **A `spread` mode and a `frame-profile-spread` target.** For each spike
   frame, the responsible function's calibrated share becomes hblanks of
   work. The tool counts how many frames, from the spike frame on, it takes
   to fit that work into the room left under 263. That room is what each
   frame's other update, its submit and its draw leave, with draw reduced by
   a given relative draw gain. The count is given per frame and per class.
6. **The measurements.** Classification of all 166 frames, the "why it runs
   then" of each class, a do-less candidate per class, and the spreading
   count under three draw scenarios. The scenarios are: no draw lever,
   step 7's estimate, and step 7's estimate with step 8's measured one-print
   and no-meter deltas.
7. **The fallback**, only for a class PC cannot settle: one full run on a
   throwaway patch that times the class's suspect functions on the PSX.
8. **A new section "Update spikes"** in `docs/psx-perf-levers.md`, after
   "Backend: per-unit costs". It holds the calibration, the classes, every
   frame listed in its class, the per-class figures with their confidence,
   the candidates, the spreading table, and a verification table that
   follows the step's "verified when".

## User Stories

1. As the plan pilot, I want `cart_update60`'s five children timed on the PSX on frames 12 and 5500, so that PC update proportions are checked against update code itself, not against draw.
2. As the plan pilot, I want the calibration patch to read the fine clock unwrapped by the hblank counter, so that a child running for thousands of hblanks (frame 12's 8,980) is timed without wrap errors.
3. As the plan pilot, I want the calibration patch's TTY lines printed outside the timed spans and outside update, so that printing does not inflate what it measures.
4. As the plan pilot, I want the calibration run through `make psx-probe` on `start` and `update-5500` only, so that it is measured like every other probe, with the probe ceiling's headroom, and is never committed.
5. As the plan pilot, I want the calibration boots valid with the game state the same, and their update delta against the probe baseline recorded as the timing's own overhead, so that the measurement's cost is known.
6. As the plan pilot, I want the PSX children's sum checked against the window update, so that a missing span or a double count is caught.
7. As the plan pilot, I want the same frames profiled on PC K = 4 times with the per-frame minimum, so that the PC side of the comparison has the same noise rule as the rest of the plan.
8. As the human reading the report, I want, per child and per run, the PSX hblanks, the PSX share, the PC share, the ratio and the gap in points, so that the calibration gap is recorded as the plan requires.
9. As the human reading the report, I want children under 2% of update on both sides marked "below", so that I do not over-read a ratio of tiny numbers.
10. As the plan pilot, I want the PC frame profile to accept an update-root symbol, so that I can profile any `cart_update60` descendant and its direct children frame by frame.
11. As the plan pilot, I want the profile's header to name the update root as it already names the draw root, so that a profile file says which subtree it describes.
12. As the plan pilot, I want an unknown, ambiguous or forbidden update-root symbol (`cart_draw`, or the draw root itself) refused with its name, so that a typo never produces an empty profile.
13. As the plan pilot, I want profiles without an update root to stay byte-for-byte what they are today, apart from the new header line, so that the breakdown, coverage, subtree and unit-cost modes keep working unmodified.
14. As the plan pilot, I want the spike frames chosen from the judge table by a tool, with the strict rule update > 263, so that the 166 frames are selected the same way every time.
15. As the plan pilot, I want the tool to fail when the judge table and the PC profiles do not have the same frame count, so that frame numbers never drift between PSX and PC.
16. As the plan pilot, I want every spike frame's update split among `cart_update60`'s children with the per-frame minimum over K profiles, so that PC timer noise does not choose the responsible child.
17. As the plan pilot, I want the descent to go down while one child holds at least half of the frame's PC update, so that the responsible function is the deepest one that still explains most of the spike.
18. As the plan pilot, I want the descent to name the next symbol to profile when it reaches a node no profile covers yet, so that the walk can be continued without guessing.
19. As the plan pilot, I want the make target to profile the named symbols and repeat until no frame is open, so that the whole classification is one command.
20. As the plan pilot, I want a symbol that cannot be resolved as a root (a static function with a shared name) reported as such and the descent stopped there, so that the tool never profiles the wrong function.
21. As the plan pilot, I want a child reached through several call sites flagged as multi-site in the path, so that I know its root profile counts every site and its share may read high.
22. As the human reading the report, I want every one of the 166 frames listed in exactly one class, so that no spike is left unexplained.
23. As the human reading the report, I want each class named by its responsible function and its path from `cart_update60`, so that I can find the code.
24. As the human reading the report, I want each class's PC share of update (minimum, median and maximum over its frames), so that I know how much of the spike the responsible function explains on PC.
25. As the human reading the report, I want each class's share carried through its child's calibration ratio into an interval with a confidence label, so that I know how far the share holds on the PSX.
26. As the plan pilot, I want a class whose confidence is low and whose calibrated interval straddles 50% treated as "PC cannot settle", so that the fallback is triggered by a rule and not by impression.
27. As the human reading the report, I want each class's frame count, its frames over 1000, and its summed judge update, so that classes can be compared by weight.
28. As the human reading the report, I want to know, per class, on how many full-run frames the responsible function runs at all and how many of those are spike frames, so that I can tell an event-driven function from an always-running one that is heavy now and then.
29. As the human reading the report, I want "why it runs then" stated per class from the code and the recording, so that I know which game event triggers the spike (game start, end of turn, save, and so on).
30. As the human reading the report, I want the four named frames (12, 4737, 5500, 5719) shown explicitly with their class, so that the plan's original question gets a direct answer.
31. As the plan pilot, I want the fallback to cost at most one full run for all unsettled classes together, so that the plan's full-run budget is kept.
32. As the plan pilot, I want the fallback patch to time the suspect functions on the PSX and print them per frame, so that the full run gives each suspect's PSX cost on the spike frames directly.
33. As the plan pilot, I want every throwaway patch discarded after its numbers are recorded, so that nothing but tooling is committed.
34. As the human reading the report, I want a do-less candidate per class, typed (repeated calls, identical work, recomputation, or spread across frames), with its location, evidence, upper-bound gain and e2e risk, so that it can be ranked like the draw candidates.
35. As the human reading the report, I want the do-less candidate to say whether it changes game state or only timing, so that I know whether it changes e2e hashes beyond the re-record the spreading decision already allows.
36. As the plan pilot, I want the responsible function's work in hblanks taken as its calibrated share of the frame's update times the judge's update, so that the spread works on PSX hblanks.
37. As the plan pilot, I want the spreading count to use the real following frames of the judge table, each with its own update, draw and submit, so that the room left under 263 is the game's and not an average.
38. As the plan pilot, I want the spread computed by a tested tool under a given relative draw gain and calibration ratios, so that the count is reproducible for any scenario.
39. As the plan pilot, I want a frame whose work cannot fit within a cap of frames reported as "does not fit", so that a class that needs draw levers first is named as such and not given a misleading count.
40. As the human reading the report, I want the spreading count per class under three draw scenarios (none, step 7's estimate, step 7 with step 8's measurements), so that I see how much the draw levers buy the spikes.
41. As the human reading the report, I want the draw gain of each scenario derived from committed figures by a rule written in the report, so that I can check it and step 12 can replace it.
42. As the human reading the report, I want frame 12 to use the `start` run's draw gain and the other frames a bracket from `draw-5564` and `draw-6000`, so that the title frame and the in-game frames are not priced alike.
43. As the human reading the report, I want the spreading estimate labelled optimistic (work assumed divisible, no overhead to split it), so that I do not read it as a measurement.
44. As the human reading the report, I want spike frames whose spread overlaps another spike's window named, so that two spreads competing for the same room are not both counted as free.
45. As the plan pilot, I want the calibration ratios recorded where step 10 can use them, so that step 10 corrects its PC-derived costs without measuring them again.
46. As a developer, I want the new modes tested on hand-written inputs, and the update root tested on the built binary, all in `make frame-profile-test`, so that the tooling is checked outside `make test`.
47. As a developer, I want `make test` and `make e2e` untouched in content and duration, so that the fast loop stays fast.
48. As a developer, I want no change to the game, the engine, the runtime, the PSX platform code, `psx/` or the e2e files, so that every `*.sha256` and `psx-check` are untouched.
49. As the plan pilot, I want the step's verification table to answer the plan's "verified when" item by item, so that step 12 can rely on it.

## Implementation Decisions

### Step 8, as committed (checked on `3fe4b8b`)

These replace the drafting hypotheses. Each was checked against the step 2.8
note, the report's "Backend: per-unit costs" section and `git diff b6fd2f8
3fe4b8b`.

- **Section order.** "Backend: per-unit costs" is the report's last
  section, and the opening paragraph names it. This step's section goes
  after it, and one clause naming it is added to the opening paragraph.
- **Tooling format.** Step 8 changed only the family lines of `frames.txt`
  and `summary.txt`: two trailing fields, `units <u> switches <s>`. It added
  the `unit-cost` and `unit-cost-pooled` modes (a calls mismatch exits 2;
  1 stays the usage error) and the `make frame-profile-unit-cost` target.
  The header (`profile …`, `root draw …`), `frame <n> update … draw …`,
  `child` and `self` lines are unchanged, so this step's modes read those by
  leading position. `call_profile` silences its hooks while it counts
  units, through a module-wide flag. Unit counting happens inside the draw
  family wrapper only.
- **No game or PSX change.** Between `b6fd2f8` and `3fe4b8b`, `src/` changed
  only in `src/cmd/frame_profile.c` and `src/devtools/call_profile.*`.
  `psx/` and `psx/short_runs.txt` did not change. The judge table at
  `7eb512f` and step 7's figures still describe the game at `HEAD`. Step 8's
  rebuilt probe baseline equals step 7's on all six runs, part by part,
  including `start` 8980 / 465 / 6 and `update-5500` 1409 / 714 / 7
  (update / draw / submit).
- **Update-side PC time unaffected.** A full-run profile at `HEAD`, taken
  while this PRD was revised, gives `ui_update` 69.7% of `cart_update60`
  against step 1.4's 69.3%, `update_entities` 19.3% and `save_at_frame_end`
  8.8%. That is within 2 points, so no shift has to be reported.
- **One-print, measured** (`b-one-print`, window draw deltas, probe draw
  baseline in brackets): `start` −64 (465), `draw-5564` −8134 (19491),
  `draw-6000` −1532 (13987). Relative draw gains are 13.8%, 41.7% and
  11.0%. This is an upper bound of the outlined-font lever.
- **No-meter, measured** (`b-no-meter`): `start` −124, `draw-5564` −3170,
  `draw-6000` −2977. Relative draw gains are 26.7%, 16.3% and 21.3%. The
  report notes that dropping the meter from `psx-game` needs another source
  for `psx-check`'s zone and cap lines. That is a cost of the lever, not of
  this estimate.
- **Labels.** Step 8's other backend candidates (`p8_map`'s off-screen
  cells, about 150 a frame and overlapping step 7's candidate 5; rejecting
  off-screen calls once; four smaller per-call costs) are estimates. The
  two experiments are measured. Only the two measured deltas enter this
  step's draw scenarios.
- **Manifest.** Six runs, unchanged: `start`, `update-4737`, `update-5500`,
  `draw-5564`, `update-5719`, `draw-6000`.
- **Boot ceiling.** `psx-short-runs` boots stop at `PSX_SHORT_RUN_CEILING`
  (540 vsyncs, tight: `draw-6000`'s breakdown boot needed 542). `psx-probe`
  boots stop at `PSX_PROBE_CEILING` (1080). This step boots only through
  `psx-probe`, on `start` and `update-5500`, whose boots are far shorter
  than `draw-6000`'s, and adds no `psx-short-runs` boot. If a calibration
  boot still reaches its ceiling, it is re-run with `PSX_PROBE_CEILING=1440`
  and the report says so.

### Calibration on update code (first; throwaway, no committed code)

- **Patch** (`tmp/plan20-update/u-calibrate.patch`, base commit recorded):
  - In `cart_update60`, each of the five statements is wrapped in a span:
    `frame_zone_reset`, `cart_flush_exit_queue`, `update_entities`,
    `ui_update` (with its `dispatch_context_mem` and `main_ui` arguments,
    which the PC profile lists as separate children under 0.2%), and the
    `save_at_frame_end` statement with its `g_save_cut` test.
  - A span reads the PSX fine clock and the hblank frame counter at both
    ends, the pair the breakdown boot uses. When the coarse delta reaches
    the fine counter's wrap limit, the span counts the coarse delta in whole
    hblanks; otherwise it counts fine ticks, converted with the boot's
    calibration line, as `psx_short_run.sh` does.
  - The spans accumulate into a per-frame record. The GPU replay's frame
    loop prints it after the update part's timer has been read, so printing
    lands outside update and outside every span, as one line
    `probe-update: frame <n> zone <h> exit-queue <h> entities <h> ui <h> save <h>`
    in hundredths of an hblank.
  - The patch includes `platform/psx_probe.h` the way step 1.3's delay
    patch did. It is applied with
    `make psx-probe PATCH=tmp/plan20-update/u-calibrate.patch PSX_SHORT_RUNS="start update-5500" PSX_PROBE_LABEL=u-calibrate`.
- **Validity.** Both boots must be `valid` with `game-state same`. Their
  update delta against the probe baseline is the spans' own overhead,
  recorded. Over the window frame, the five spans' sum must be at most the
  window update minus that overhead. The remainder is `cart_update60`'s
  self, recorded. A failure is investigated and the probe re-run, never
  read.
- **PC side.** Each run's `run.json` (`tmp/psx/short-runs/<run>/`, cut by
  `psx-short-runs-cut`) is profiled K = 4 times by `tmp/frame_profile`
  into `tmp/frame-profile/calibration/<run>/<k>/`, with same-game checked
  against its `run.end-state`. On the window frame (from the run's
  `map.txt`), each child's PC time is its per-frame minimum over the 4
  profiles, and the denominator is `cart_update60`'s per-frame minimum. The
  `dispatch_context_mem` and `main_ui` children are added to `ui_update`
  to match the PSX span. The commands are written in the report. There are
  10 figures a side, so no tool is committed for this.
- **Recorded per run and child:** PSX hblanks, PSX share of window update,
  PC share, ratio `ρ = PSX share ÷ PC share`, and gap in points. Also,
  per run, PSX hblanks per PC µs of the whole update. A child under 2% on
  both sides is "below" and gets no ratio.
- **Confidence labels**, from a class's ratios (below):
  - high: every applicable `ρ` is within 0.85–1.15;
  - medium: within 0.67–1.5;
  - low: outside 0.67–1.5;
  - uncalibrated: the class's child is "below" on both runs, or is not one
    of the five spans.
- **For step 10.** The calibration table is the step 10 input the plan now
  names ("PC proportions corrected by step 9's calibration"). The report
  says so in one line.

### Tooling (the only committed code)

- **Update root in `frame_profile`.** The binary's usage becomes
  `frame_profile <events.json> <out-dir> <symbols> [<draw-root>
  [<update-root>]]`.
  - Both roots default to today's (`cart_draw`, `cart_update60`). An update
    root needs the draw root given (the target passes `cart_draw`).
  - The update root is resolved as the draw root is: an unknown or
    ambiguous symbol fails with its name, and so does an update root equal
    to `cart_draw` or to the draw root. The draw root keeps its
    `cart_update60` refusal.
  - `call_profile` already takes any two distinct roots, so its interface
    does not change.
  - A root is entered wherever it runs while no root is active. An update
    root called from inside `cart_draw` is therefore not counted there, as
    long as the draw root is `cart_draw`. That is why the spike target never
    changes the draw root.
- **Header.** `frames.txt` and `summary.txt` gain a line `root update
  <symbol>` right after `root draw <symbol>`, always written. The `frame <n>
  update <ns> draw <ns>` line keeps its name and meaning: `update` is the
  update root's PC time on that frame. No other line changes.
  - Trap: the coverage mode reads `$3 == "update" && NF == 6` on `frame`
    lines, and the subtree mode requires a `root draw` line. The new line
    starts with `root` and has 3 fields, so neither may match it.
  - The breakdown, coverage, subtree, `unit-cost` and `unit-cost-pooled`
    tests stay green unmodified, which proves it.
- **New mode `spikes` of `tools/frame_profile_check.sh`.**
  - Usage: `spikes <threshold> <judge-frames.txt> <base frames.txt>[,...]
    [<symbol>=<frames.txt>[,...]]...`. The base profiles are rooted at
    `cart_update60` and each `<symbol>=` set is rooted at that symbol. Every
    set has K files.
  - Checks: every profile has the judge table's frame count; the base set's
    `root update` is `cart_update60`; each other set's `root update` is its
    symbol. Malformed input exits 2.
  - For each frame `f` with judge update > threshold:
    - the denominator `D(f)` is the per-frame minimum, over the base set, of
      `cart_update60`'s PC time;
    - a node's time `T(node, f)` is the per-frame minimum, over the set
      rooted at its parent, of the child's `ns` (the child's own figure, by
      call site);
    - the descent starts at `cart_update60`. At each node it takes the
      child with the largest `T / D`. If that share is at least **50%**, it
      steps into the child: when a set rooted at the child's symbol exists,
      the walk goes on there; otherwise the frame is **open** on that
      symbol. If no child reaches 50%, or the node has no child, the walk
      stops, and that node is the **responsible descendant**;
    - a path component reached through a `#<site>` child name is written
      with its site, and the walk into its root set is marked `multi-site`,
      since the root set counts every activation of the symbol.
  - Output, one line per spike frame:
    `frame <f> update <hblanks> pc-update-ns <D> path <a>/<b>/... share <pct> [multi-site] [open <symbol>]`.
  - Then one line per class (the frames sharing a closed path):
    `class <path> frames <n> over-1000 <m> update-sum <hblanks> share <min>-<median>-<max> runs-on <r> spike-runs <s> hblanks-per-pc-us <median> list <f1>,<f2>,...`.
    `runs-on` counts the full-run frames where the responsible node has at
    least one call in the first profile of its parent's set; `spike-runs`
    counts how many of those are spike frames.
  - Then `open <symbol> frames <n>` lines, and the totals `spike-frames <n>
    over-1000 <m> classes <c> open <o>`.
  - Exit 0 when nothing is open, 3 when something is, 1 on a usage error.
  - A class whose path is `cart_update60` alone is tagged `unsettled`. A
    path that stops on a symbol the profile could not resolve as a root is
    closed and tagged `unresolvable`.
- **Make target `frame-profile-spikes [FRAME_PROFILE_REPEATS=<k>]
  [FRAME_PROFILE_SPIKE_THRESHOLD=<hblanks>]`** (default K = 4, threshold
  263):
  - It profiles `worst-case/full_game.json` K times rooted at
    `cart_update60` into `tmp/frame-profile/spikes/cart_update60/<k>/`, then
    runs the `spikes` mode on the committed judge table.
  - While the mode exits 3, it profiles each `open` symbol K times as update
    root (draw root `cart_draw`) into `tmp/frame-profile/spikes/<symbol>/<k>/`
    and runs the mode again, with an iteration cap of 16 levels.
  - A symbol that fails to resolve as a root is recorded in
    `tmp/frame-profile/spikes/unresolvable.txt` and passed to the mode as a
    closed stop.
  - Same-game is checked on every profile, as the other profile targets do.
    The last output goes to `tmp/frame-profile/spikes/spikes.txt`.
  - It never boots. It gets a legend entry and stays outside `make test`,
    `make test-all` and `make e2e`.
- **New mode `spread`.**
  - Usage: `spread <budget> <judge-frames.txt> <spikes.txt> <gains>
    [<ratios>]`.
    - `<gains>` is `default=<pct>[,<first>-<last>=<pct>]...`: the relative
      draw gain applied to every frame, with per-range overrides.
    - `<ratios>` is `<child>=<ratio>[,...]`, keyed by a path's first
      component under `cart_update60`. A missing child or no `<ratios>`
      means 1.
  - For each spike frame `f` of a closed class, with PC share `p` and its
    child's ratio `ρ`:
    - the work is `W = min(1, p × ρ) × update(f)`, rounded up;
    - the room on frame `f` is `budget − submit(f) − draw(f) × (1 − g(f)) −
      (update(f) − W)`, and on each following frame `i` it is `budget −
      submit(i) − draw(i) × (1 − g(i)) − update(i)`. Draw is rounded to
      whole hblanks after the gain, and negative room counts as 0;
    - `N` is the smallest count of frames from `f` on whose summed room is
      at least `W`. Past a cap of **600 frames** (10 s) or the end of the
      run, the frame is `does-not-fit`.
  - Output: `frame <f> class <path> work <W> frames <N|does-not-fit>
    [overlaps <f2>,...]`, where `overlaps` names the other spike frames
    inside `[f, f + N − 1]`. Then, per class, `class <path> work
    <min>-<median>-<max> frames <min>-<median>-<max> does-not-fit <n>`.
  - Exit 0, 2 on malformed input, 1 on a usage error.
- **Make target `frame-profile-spread DRAW_GAINS=<gains>
  [UPDATE_RATIOS=<ratios>] [SPREAD_LABEL=<label>]`.** It runs the `spread`
  mode on the judge table and `tmp/frame-profile/spikes/spikes.txt` and
  writes `tmp/frame-profile/spikes/spread-<label>.txt`. When `spikes.txt` is
  missing, it fails naming `make frame-profile-spikes`. It gets a legend
  entry and stays outside the suites.
- **The judge table is the committed companion file**
  `docs/psx-perf-levers-full-run-frames.txt`, read as `frame update draw
  submit total vsyncs`. It is not refreshed: the game has not changed since
  `7eb512f`.

### Classification and confidence

- **Order:** the calibration (above), then `make frame-profile-spikes` until
  no frame is open. Then each class is read: its path, the code of its
  responsible function and of the path above it, the per-frame calls of the
  path's nodes, and the recording's events on and just before the class's
  frames (`worst-case/full_game.json`). From these the report writes "why it
  runs then".
- **What counts as a class for the report:** the tool's classes as they
  are. When two paths are clearly the same cause (the same function reached
  through two sites, or two siblings run by the same game event), the report
  may merge them into one class. It names the merged paths and keeps each
  one's figures. A class is never split by hand.
- **A class's confidence.** Its path's first component is a
  `cart_update60` child `c`. The applicable ratios are `c`'s ratio on the
  calibration run whose frame is in the class, if one is (frame 12 is
  `start`, frame 5500 is `update-5500`); otherwise `c`'s ratios on both
  runs. The class's calibrated share interval is `[p_median × ρ_min,
  p_median × ρ_max]`, capped at 100%. Its label is the confidence label of
  those ratios.
- **The same ratio for every depth.** The calibration measures
  `cart_update60`'s children, and the report applies a child's ratio to the
  deeper responsible function under it. It says this is the best measured
  correction, not a measurement of that function.
- **"PC cannot settle" a class** when any of these holds:
  - it is `unsettled`;
  - it is `unresolvable` and the code does not settle it;
  - its label is low or uncalibrated and its calibrated interval straddles
    50%, so on the PSX the responsible function might not hold the
    majority.

### Fallback (only when a class is not settled)

- One throwaway patch on `HEAD` times each suspect function of the
  unsettled classes: their `cart_update60` child and its direct children on
  the class's path. It uses the calibration patch's span mechanism and
  prints one TTY line per frame where a suspect ran.
- It is built and run with `make psx-full-game-check
  PSX_FULL_GAME_ASSERTS=off` in a temporary worktree, as the probe tool
  builds its patches (about 20 minutes). Its numbers are the suspects' PSX
  hblanks on each frame of the unsettled classes.
- The responsible function is the suspect with the largest time, and its
  share is that time over the frame's update in the same boot. These
  replace the PC figures for those classes, labelled "measured on the full
  run", with confidence high.
- One full run covers every unsettled class, and the patch is discarded.
  When no class is unsettled, no full run is made, and the report says so.

### Do-less candidates

- Each candidate comes from the code of the responsible function and gives:
  - its kind: repeated calls, identical work, recomputation, or "spread
    across frames";
  - its location: function and file:line at the commit measured;
  - its evidence: calls and share from the profile;
  - its upper-bound gain in judge hblanks on the class's frames, labelled
    "estimated";
  - its risk: "timing only" when game state and pixels stay the same,
    "game state" when the result changes, or "e2e re-record" when the
    spreading decision covers the change.
- Every class gets at least the spread candidate, whose figure is the
  spreading count. A class may get more candidates when the code shows
  them. They are not ranked here: ranking is step 12's.

### Draw scenarios for the spread

Each scenario is run through `make frame-profile-spread` with
`UPDATE_RATIOS` set to the calibration's mean ratio per child (a "below"
child is left out, so its ratio is 1). Each is written in the report with
its derivation:

- **`none`:** `default=0`.
- **`step7`:** built from step 7's committed "do less" bounds per run
  (`start`, `draw-5564`, `draw-6000`).
  - A run's gain is the sum of the per-frame bounds of the candidates whose
    pixel risk is "identical", or "identical if/only …" (the lever must meet
    its condition), divided by the run's mean window draw per frame.
  - Overlaps: the text group (candidate 1 against 2, 3 and 4) counts as the
    larger of 1 and 2 + 3 + 4. Candidate 7 is dropped wherever 2, 3 or 6
    cover its rows. Candidates 12 and 15 count as step 7 bounded them (dots
    and particles). "Less a copy" is ignored. The pixel-changing variant of
    11, and candidate 14, count 0.
  - Frame 12 uses `start`'s gain. Every other spike frame is computed
    twice, with the smaller and the larger of the `draw-5564` and
    `draw-6000` gains, which gives a bracket `N_high` to `N_low`.
- **`step7+8`:** as `step7`, with two changes.
  - Candidate 1's bound is replaced by step 8's measured one-print relative
    gain on that run (13.8% `start`, 41.7% `draw-5564`, 11.0% `draw-6000`).
    The text group then counts the larger of that and 2 + 3 + 4.
  - Step 8's no-meter gain `m` (26.7%, 16.3%, 21.3%) is combined
    multiplicatively, `g' = 1 − (1 − g)(1 − m)`.
  - The meter's share of the calls the other levers remove is then counted
    twice; step 8 named that overlap, and step 12 settles it. The report
    says this scenario reads optimistic by that overlap.
  - Step 8's estimated candidates (`p8_map`'s off-screen cells and the
    others) are not included.

These gains are upper bounds, and the report says so. They exist to size the
spreads, not to rank draw levers (step 12's job).

### What is recorded in `docs/psx-perf-levers.md`

A new section **"Update spikes"**, after "Backend: per-unit costs", and one
clause in the opening paragraph naming it. It holds:

- the commit measured, the commands, and the method in a few sentences: the
  calibration, the K-minimum descent, the 50% rule, the confidence rule;
- **the calibration table:** per run and child, PSX hblanks, PSX share, PC
  share, ratio, gap in points; the spans' overhead, the self, and the PSX
  hblanks per PC µs per run; the one-line hand-off to step 10;
- **the class table:** path, responsible function (file:line), frames, over
  1000, summed judge update, PC share min/median/max, calibrated interval,
  confidence label, runs-on / spike-runs, hblanks per PC µs, and flags
  (`multi-site`, `unresolvable`, settled by the fallback);
- per class: "why it runs then", the frame list, and the do-less
  candidate(s);
- the four named frames with their class and share;
- **the spreading table:** per class and scenario, work and frame count
  (min/median/max) and `does-not-fit` counts, with the overlaps named, and
  each scenario's gain spec and the ratios used;
- **the fallback:** either "not needed" or the patch line, the full run's
  conditions, the suspects' hblanks per frame and the classes it settled;
- **the verification table**, following the step's "verified when":
  - the calibration gap recorded;
  - all 166 frames classified (42 of them over 1000);
  - per class: responsible function, share with confidence, candidate,
    and spreading count;
- the throwaway patches (one line each) and the note that they were
  discarded.

Integer hblanks for work and room, one decimal for shares in percent and for
hblanks per PC µs, two decimals for ratios.

### Also checked at start

- Is the measured GPU replay's save storage the in-memory one, as on PC?
  Check: the PSX replay's source list in the Makefile. If the PSX build
  saves elsewhere, the `save_at_frame_end` child's calibration ratio is
  expected to differ, and the report says why.

### Not decided by a guideline call

Every question was settled from the plan and the code:
- The calibration times the five statements of `cart_update60` with the
  breakdown boot's clock pair, through the probe tool, on the two runs the
  plan names.
- The descent reuses the profile's existing root mechanism.
- The 50% rule picks the deepest function that explains most of a frame.
- The confidence comes from the child's ratio and its thresholds.
- The fallback is a single full run, the one this step is allowed.
- The draw-gain rule uses only committed figures, and step 12 replaces it.

No `guideline` agent was needed.

## Implementation Chunks

1. **Calibration (throwaway patch, report only).** Check the "Step 8, as
   committed" facts and the save-storage question; write the calibration
   patch; run `make psx-probe` on `start` and `update-5500`; profile both
   runs' recordings K = 4 times on PC; write the section head, the method
   and the calibration table, with the hand-off line for step 10. Commit
   the report. Nothing in later chunks uses a PC share before this chunk is
   committed.
2. **Update root in `frame_profile`.** The optional fifth argument, its
   refusals, and the `root update` header line. Tests in
   `test_frame_profile`:
   - the built binary on `tests/e2e/pad_title.json`, rooted at `ui_update`,
     writes `root update ui_update`, and its update children are the ones
     `ui_update` calls (checked by name and call count, never by time);
   - `cart_draw` and an unknown symbol as update root fail with their
     names;
   - the default profile's header has `root update cart_update60`.

   The breakdown, coverage, subtree and unit-cost tests stay green
   unmodified. Commit.
3. **`spikes` mode and `frame-profile-spikes` target.** The mode is tested
   in `test_frame_profile_check` on hand-written judge tables and profile
   sets:
   - selection under strict `>`;
   - K-minimum shares;
   - a two-level descent through a `<symbol>=` set;
   - the 50% stop;
   - an open symbol and exit 3;
   - a multi-site child;
   - an `unsettled` frame;
   - the class lines with `runs-on` and `spike-runs`;
   - a frame-count mismatch exiting 2.

   The target comes with its descent loop, iteration cap, `unresolvable`
   handling and legend entry. Commit.
4. **`spread` mode and `frame-profile-spread` target.** The mode is tested
   on hand-written judge tables and `spikes.txt` files:
   - room on the spike frame against the following frames;
   - negative room as 0;
   - gain ranges;
   - ratios, including the cap at 100% and a missing child;
   - the 600-frame cap and `does-not-fit`;
   - overlaps;
   - the class summary.

   The target comes with its legend entry. Commit.
5. **Classification.** Run `make frame-profile-spikes`. Read each class
   (why it runs then, candidates) and apply the calibration (interval,
   label, the "cannot settle" rule). Write the class table, the per-class
   paragraphs and the four named frames. Commit the report.
6. **Spreading and fallback.** Derive the three gain specs and the ratios,
   then run `make frame-profile-spread` for each scenario and write the
   spreading table. If any class cannot be settled, run the fallback full
   run, settle it and fill its rows (spread included). Then write the
   verification table and the patch list. Commit the report.

## Testing Decisions

- A good test checks the tool's observable contract (exit status, the lines
  it writes, the figures it computes from given inputs), never internals.
- **Tested:**
  - the update root, end to end on the built binary
    (`tests/test_frame_profile.c`; prior art: it already runs the binary on
    `tests/e2e/pad_title.json` and checks children, headers and step 8's
    unit fields);
  - the `spikes` and `spread` modes, on hand-written inputs
    (`tests/test_frame_profile_check.c`; prior art: the `breakdown`,
    `coverage`, `subtree` and `unit-cost` modes' tests on hand-written
    profiles, TTYs and maps).

  All of them run in `make frame-profile-test`, outside `make test`, `make
  test-all` and `make e2e`.
- **Timing-free assertions on the binary:** PC times are noisy, so the
  binary test checks names, header lines, call counts and refusals, never a
  time value.
- **Default behaviour pinned:** the breakdown, coverage, subtree, unit-cost
  and unit-cost-pooled tests stay green unmodified.
- **Not tested:** the calibration patch (throwaway), the classification,
  the spreading figures and the report. Their proof is the tools' verdicts
  (probe `valid` and game state, same-game, frame counts, no open frame),
  the calibration's sum check, and the verification table.
- `make test-all` stays green and is not slower. No `*.sha256` moves.
  `psx-check` is not re-run, since no file the PSX build compiles changes.
  The throwaway patches live only in `tmp/` and temporary worktrees.

## Out of Scope

- Ordinary update, which is step 10's: update's share in the frames outside
  this step's classes, through fixed-delay probes and PC proportions
  corrected by this step's calibration. This step hands over the
  calibration table and the class frame lists; it measures nothing outside
  the classes.
- Calibrating below `cart_update60`'s children, or on runs other than
  `start` and `update-5500`.
- Any draw probe, any new draw candidate, and per-unit costs: steps 6–8.
- Implementing a spread or any other lever. Changing the game, the engine,
  the runtime, the PSX platform code, `psx/`, the probe tool or any e2e
  file.
- Ranking levers, cumulative estimates, and applying the class figures to
  the full-run table: step 12's. This step only gives each class its
  figures.
- A second full run, or a full run when every class is settled. A
  `psx-short-runs` boot (its 540-vsync ceiling is tight).
- Submit's content, asserts and `-O` levels: step 11.
- Anything on the Miyoo or PCSX-ReARMed, audio, launching the raylib app.

## Further Notes

- Expected shape, not a target. The `HEAD` profile taken while revising this
  PRD showed:
  - `ui_update` holding nearly all of frame 12's update (galaxy generation
    goes through `cart_start_game`, `galaxy_populate` and
    `recompute_economy`);
  - `save_at_frame_end` at roughly a fifth to a third of update on 1244,
    3313, 4737, 5500 and 5719, next to a large `ui_update`;
  - on the regular smaller spikes about every 27 frames (553, 604, 658,
    710, …), nothing heavy beyond `ui_update`.

  So expect a game-start class, an end-of-turn class (economy
  recomputation, possibly with the save beside it) and a periodic class.
  The 50% rule may leave the save as "rest of update" inside the
  end-of-turn frames. If it does, the report names the save as the class's
  second-largest node, since spreading the responsible function alone
  leaves it in the frame.
- `update-5500` holds both `ui_update` and `save_at_frame_end` on a spike
  frame, and `start` holds the galaxy generation. Between them, the
  calibration covers the three children that matter. `update_entities`
  may read "below" on both runs; if so, any class under it is
  "uncalibrated".
- `save_at_frame_end` runs only when `save_pending` is set and no wormhole
  is live. Its frames are therefore event-driven, which `runs-on` and
  `spike-runs` should show.
- The resumed short runs lose the 4737 and 5719 spikes. A likely reason is
  state the save does not carry. If a class's "why" explains it, the report
  says so in one line.
