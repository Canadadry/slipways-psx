---
title: "Draw level 1: probe each direct child of cart_draw on the short runs (plan 20, phase 2, step 6)"
description: "Plan 20 knows what cart_draw costs per frame on the PSX but not how that cost splits among its seven direct children, so it cannot tell which draw branches step 7 must go down."
status: needs-triage
---

## Plan step (verbatim)

> 6. Level 1 of draw: probe each direct child of `cart_draw` with call-twice
>    on `start` and `draw-5564`. Verified when the report lists each child's
>    cost and calls per frame, and the children's costs add up to `cart_draw`'s
>    own within 10%. The gap is explained in the report if not.

Plan file: `docs/plan-20.md`. Rules in force (section 2, "How discovery
works"): hot paths are found top-down from `cart_draw`; drawing functions are
called twice and the frame grows by the function's cost; a second call that
only advances `visual_random` is fine, but if the run's game-state lines no
longer match the baseline's, that function falls back to the fixed delay
(which measures the call count, the cost then coming from PC timing
proportions scaled to the PSX total); throwaway experiments run in a
temporary worktree and are discarded, and **nothing is kept from an
experiment except its numbers in the report; the only code this plan commits
is the measuring tooling**. Guardrails: nothing in the game or the shipped
drawing path changes; `make e2e`, every `*.sha256` and `psx-check` stay as
they are; `make test` and `make e2e` do not get slower; never launch the
raylib app.

Step notes in force:

- **1.2**: the asserts-off short-run baseline is in `docs/psx-perf-levers.md`
  (`473bdcc`).
- **1.3**: `make psx-probe-baseline` and `make psx-probe PATCH=<file>
  [PSX_SHORT_RUNS=] [PSX_PROBE_LABEL=]` exist; probe baseline equals step 2's
  within one hblank; call-twice `cart_draw` gives draw +4.3% to +8.5% over
  double; null-patch noise floor 0. Guideline decision (1):
  `help_screen_draw` (`src/game/help.c:98`) ticks the RNG once a call, so
  call-twice `cart_draw` changes `start`'s game state; **Phase 2 probes
  `help_screen_draw`, and any `cart_draw` child that runs it on the title
  screen, with the fixed delay, not call-twice.** Guideline decision (2): a
  part's delay check uses max(2% of delay × window frames, 2 hblanks).
- **1.4**: `make frame-profile` and `make frame-profile-coverage` (K = 4
  profiles, per-window median) exist; the manifest `psx/short_runs.txt` now
  has six runs, `draw-6000` added for `pset`, "to be probed in steps 6–7 like
  draw-5564"; **the probe baseline must be rebuilt before step 6**. PC
  profile children of `cart_draw` (by call site): `dispatch_context_mem`,
  `p8_cls`, `hud_select_palette_block_default`, `draw_layers#1`,
  `draw_layers#2`, `dispatch_context_gfx`, `demo_overlay_draw`. PC shares of
  `cart_draw` per window (coverage table, K = 4 median): `draw_layers#1`
  99.5% at `start`, 75.4% at `draw-5564`, 99.1% at `draw-6000`;
  `draw_layers#2` 0.2%, 24.6%, 0.8%; every other child 0.0–0.2%.

Figures already in the report (`docs/psx-perf-levers.md` at HEAD), window
sums in hblanks: `start` (1 frame) draw 464–465 of total 9451;
`draw-5564` (15 frames) draw 19489–19491 of 20264; `draw-6000` (15 frames)
draw 13987 of 14687 (measured replay, no probe baseline yet).

## Problem Statement

The plan's draw tree has only one measured node: `cart_draw` itself. Step 7
must go down "each branch above 2% of the mean window frame", but nobody
knows yet which of `cart_draw`'s seven direct children are those branches on
the PSX, nor what each one costs per frame. The PC profile gives shares, but
PC proportions are not PSX costs (step 10 even plans to measure that gap), so
it cannot be the answer for draw, where the PSX cost is directly measurable.

The probe tool's baseline is also out of date: it predates `draw-6000`, the
window step 4 added precisely so that `pset`-heavy draw is probed in steps
6–7. Any probe run before a rebuild is refused as "no baseline" or "stale".

## Solution

From a commit at or after step 5, with no change to any committed file but
the report:

1. Rebuild the probe baseline on all six runs (`make psx-probe-baseline`).
2. Re-measure `cart_draw`'s own cost on the probed runs (`start`,
   `draw-5564`, `draw-6000`): the probe baseline's window draw part, which
   brackets exactly one `cart_draw` call a frame, plus the committed
   `call-twice-cart-draw` fixture for the method's bias.
3. Probe each of the seven children with a throwaway call-twice patch on
   the three runs, except `draw_layers#1` on `start`, which takes the fixed
   delay (step 1.3's decision) and a PC-derived cost. Any other child whose
   call-twice changes a run's game state falls back the same way on that run.
4. Read each child's calls per frame from the PC profile of each window,
   and from the delay probe where one is run.
5. Add a "Draw level 1" section to `docs/psx-perf-levers.md`: per run and per
   child the method, window delta, cost per frame, share of `cart_draw` and
   of the mean window frame, calls per frame, the other parts' deltas and the
   game-state verdict; the additivity check against 10%, with the gap
   explained if it fails; and the list of children above 2% that step 7 takes
   over.

The patches are generated in `tmp/` and discarded; the report records what
each one does in one line so it can be regenerated.

## User Stories

1. As the plan pilot, I want the probe baseline rebuilt on all six runs first, so that no probe of this step or the next is refused as stale or missing, and `draw-6000` gets its first probe baseline.
2. As the plan pilot, I want the rebuilt baseline compared run by run with the step 3 baseline and the step 2 / step 4 measured-replay figures, so that a shift since step 3 is seen before any delta is read.
3. As the plan pilot, I want `cart_draw`'s own cost per run taken from the probe baseline's window draw part, so that the reference of the additivity check is a direct measurement of one `cart_draw` call per frame, not a derived one.
4. As the plan pilot, I want the `call-twice-cart-draw` fixture re-run on the three runs, so that the call-twice method's bias (the second call costing a few percent more) is known per run and the children's call-twice deltas are read with it in mind.
5. As the plan pilot, I want `start`, `draw-5564` and `draw-6000` probed, so that the title-screen draw, the print-heavy window and the `pset`-heavy window are all split at level 1, as step 4 asked for `draw-6000`.
6. As the plan pilot, I want the update-only short runs left out, so that the step stays at the plan's two named runs plus the one step 4 added.
7. As the plan pilot, I want one throwaway patch per child, so that every delta is attributable to exactly one call site.
8. As the plan pilot, I want each call-twice patch to add a second call right after the original at the same call site in `cart_draw`, with the same arguments, so that the second call sees the same state as the first, as the rule assumes.
9. As the plan pilot, I want the two `draw_layers` call sites probed separately (`#1` the draw message, `#2` the draw-overlay message), so that they match the PC profile's call-site keys.
10. As the plan pilot, I want the getters `dispatch_context_mem` and `dispatch_context_gfx` probed with a discarded second call at their call site, so that every child the PC profile lists has a PSX figure, however small.
11. As the plan pilot, I want `draw_layers#1` on `start` probed with a 20-hblank fixed delay at its call site instead of call-twice, so that the help screen's RNG tick is never doubled, per step 1.3's guideline decision.
12. As the plan pilot, I want the same delay probe run on `draw-5564` and `draw-6000` too, so that `draw_layers#1`'s one call a frame is confirmed on the PSX on every probed run for one extra minute.
13. As the plan pilot, I want any other child whose call-twice gives `game-state differs` on a run to be re-probed on that run with the fixed delay, so that the plan's fallback rule applies generically, not only where it is already expected.
14. As the plan pilot, I want a fixed-delay child's PSX cost computed as its PC share of `cart_draw` on that window (median of the K = 4 coverage profiles) times the run's PSX `cart_draw` cost, so that the plan's PC-proportion method is used exactly where the plan prescribes it.
15. As the human reading the report, I want every PC-derived cost marked as such, so that I never mistake it for a measured one.
16. As the plan pilot, I want the residual (`cart_draw`'s cost minus the measured children's costs) shown next to every PC-derived cost, so that the derived figure has an independent cross-check.
17. As the plan pilot, I want each child's cost taken as its draw-part delta, so that it is the CPU time spent inside the child, the same quantity as `cart_draw`'s draw part.
18. As the plan pilot, I want each probe's update and submit deltas recorded beside the draw delta, so that a second draw call's extra GPU work (submit) is kept for step 8 without being counted in the child's cost.
19. As the plan pilot, I want a probe whose update delta is outside the 2-hblank floor flagged, so that a call-twice that leaks into update is noticed.
20. As the human reading the report, I want each child's cost per frame (window delta ÷ window frames), so that windows of 1 and 15 frames compare.
21. As the human reading the report, I want each child's share of `cart_draw`'s own cost, so that the split of draw reads at a glance.
22. As the plan pilot at step 7, I want each child's share of the run's mean window frame (probe baseline window total ÷ window frames), so that the 2% threshold of step 7 is applied straight from this table.
23. As the plan pilot at step 7, I want the list of children above 2% of the mean window frame on each run, so that step 7 knows its starting branches without recomputing anything.
24. As the human reading the report, I want a delta at or under the timer's 2-hblank floor written as an upper bound ("≤ 2 over the window"), so that a getter's zero is not read as an exact zero.
25. As the human reading the report, I want each child's calls per frame from the PC profile's window frames, so that "calls per frame" is answered for every child on every probed run.
26. As the human reading the report, I want the PSX call count from the delay probe next to the PC count where both exist, so that the PC count is confirmed on the PSX.
27. As the plan pilot, I want the additivity check per run (sum of children's costs against `cart_draw`'s own, gap in percent), so that the step's verification is stated per run.
28. As the plan pilot, I want the additivity check also shown against the `call-twice-cart-draw` delta, so that a gap caused only by the method's bias is recognised as such.
29. As the human reading the report, I want a gap above 10% explained with the evidence that supports it (method bias, cost inside `cart_draw`'s own body, a PC-derived term), so that the verification holds even when the sum does not.
30. As the human reading the report, I want the `start` check labelled as partly PC-derived, so that I know it is not fully independent.
31. As the human reading the report, I want each probe's game-state verdict, so that I see which second calls only moved `visual_random` and which mutated the game.
32. As the human reading the report, I want each probe's ELF text-size change recorded, so that a code-layout effect could be suspected if a delta ever looked odd.
33. As a later reader, I want each patch described in one line (call site, inserted statement, base commit), so that the probe can be regenerated without a committed patch file.
34. As the plan pilot, I want the patches written under `tmp/` and never committed, so that the plan's "the only code this plan commits is the measuring tooling" rule holds.
35. As the plan pilot, I want each probe run with its own `PSX_PROBE_LABEL`, so that no probe's outputs overwrite another's before the figures are read.
36. As the plan pilot, I want an invalid probe (patch not applying, build failure, `p8_fatal`) investigated and re-run once fixed in the patch, never recorded as a delta, so that the report holds only valid measurements.
37. As the plan pilot, I want every assumption about step 5 written as a hypothesis with a check and a fallback, so that this step can start whether step 5's run is finished or not.
38. As the plan pilot, I want the probes never run while step 5's 20-minute full run is in flight in the same working copy, so that the host load cannot push a boot past its wall-clock timeout.
39. As the plan pilot, I want nothing deeper than `cart_draw`'s direct children probed, so that step 7's work stays step 7's.
40. As a developer, I want no change to `src/`, `psx/`, the Makefile, the tools or the tests, so that `make test-all`, `make e2e`, every `*.sha256` and `psx-check` are untouched.

## Implementation Decisions

### What is probed, and where

- **Runs: `start`, `draw-5564`, `draw-6000`** (passed as `PSX_SHORT_RUNS`).
  Decision on `draw-6000`: probed in this step. The plan's step 4 says any
  added window is "probed in steps 6–7 like draw-5564", and step 6's text
  names only the runs that existed when it was written. Probing it here costs
  one more boot per probe and gives step 7 a level-1 split for the `pset`
  window it was made for. The update-only runs (`update-4737`,
  `update-5500`, `update-5719`) are not probed: their draw is 700–1100
  hblanks of one frame, and the step names draw windows.
- **Children: the seven call sites of `cart_draw`**, keyed as the PC
  profile keys them: `dispatch_context_mem`, `p8_cls`,
  `hud_select_palette_block_default`, `draw_layers#1` (draw message),
  `draw_layers#2` (draw-overlay message), `dispatch_context_gfx`,
  `demo_overlay_draw`. Nothing below them.
- **`cart_draw`'s own cost** is the probe baseline's window draw part. The
  replay brackets the draw part between the timer marks just before and just
  after the one `cart_draw` call (only the trivial draw-filter mask call sits
  inside), so the draw part is that call's cost. The committed fixture
  `psx/probes/call-twice-cart-draw.patch` is re-run on the three runs only to
  measure the method's bias (its delta over the baseline draw part); its
  `start` game state differs as known (step 1.3) and its `start` delta is
  recorded but labelled.

### Methods

- **Call-twice** (default): the patch duplicates the child's statement in
  `cart_draw`, same arguments, right after the original. For the two getters
  the duplicate is a discarded call placed right before the statement that
  uses the getter. For `demo_overlay_draw` the duplicate repeats the whole
  call (its `gfx` argument re-fetched as the original does). The child's cost
  on a run is the probe's window **draw** delta.
- **Fixed delay**: `psx_probe_delay(20)` placed immediately before the
  child's call site in `cart_draw` (not at the callee's entry: `draw_layers`
  is called from two sites, and the child is a call site). The probe gives
  delta ÷ delay = the call site's calls over the run, and per window frame.
  20 hblanks is the sanity probe's value and is inside the helper's 1..200
  range.
- **Where the fixed delay is used**:
  - `draw_layers#1` on `start`, by step 1.3's guideline decision: it runs the
    help screen's draw method, `help_screen_draw`, on the title-screen frames
    that precede the window. It is not call-twice-probed on `start` at all.
  - Any child whose call-twice probe reports `game-state differs` on a run,
    on that run only (the plan's fallback rule). Its call-twice delta is
    still recorded, labelled "game state differs, not used".
  - Plus, as a PSX call-count confirmation only, the same `draw_layers#1`
    delay patch on `draw-5564` and `draw-6000` (one patch, three runs). There
    the call-twice delta stays the cost.
- **Cost of a fixed-delay child** (the plan's update method, applied to
  draw): its PC share of `cart_draw` on that window, from
  `make frame-profile-coverage` (median of the K = 4 profiles of that run,
  the same figure as the coverage table's window column), times the run's
  PSX `cart_draw` cost. The report marks the figure "PC-derived" and shows
  next to it the residual: `cart_draw`'s cost minus the sum of the measured
  children on that run. No guideline call was needed: the plan prescribes
  this method for fixed-delay functions.
- **Cost quantity**: the draw-part delta only. Update and submit deltas are
  recorded in their own columns. A call-twice of a drawing child queues more
  GPU primitives, so submit may grow; that is backend cost and belongs to
  step 8. An update delta beyond max(2% of the draw delta, 2 hblanks) is
  flagged as a leak in the report (it should be zero: the probes touch only
  `cart_draw`).
- **Resolution floor**: a window delta of 0, 1 or 2 hblanks is written as
  "≤ 2 over the window" (cost per frame "≤ 2 / frames"), never as an exact
  value. No amplified probe (calling a getter N times) is run: anything
  under the floor is already two orders of magnitude below the 2% threshold
  (27 hblanks a frame on `draw-5564`).

### Calls per frame

- Every child has exactly one call site in `cart_draw`, so each is called
  once per `cart_draw` call. The report still reads the figure from data:
  the calls column of each child in the window frames of the coverage
  profiles (`tmp/frame-profile/coverage/<run>/<k>/frames.txt`), stated as
  min–max calls per frame over the window. The PC profile replays the same
  recording as the probe's run (step 1.4's breakdown check proves equal
  family calls on the draw windows).
- Where a delay probe ran, its per-window-frame calls are written next to
  the PC figure.

### Additivity check (the step's verification)

- Per run: Σ children's costs (measured and PC-derived) against
  `cart_draw`'s own cost; gap = (Σ − own) ÷ own, one decimal. The check
  holds when |gap| ≤ 10%.
- Shown alongside: the same sum against the `call-twice-cart-draw` delta.
  The step-3 sanity showed that a second `cart_draw` costs 4–9% more than the
  first; children measured the same way are expected to carry a similar
  bias, so a gap of that size and sign against the baseline draw part is
  first read against this column.
- `start` is labelled "partly PC-derived": `draw_layers#1` holds about 99% of
  `start`'s PC draw, so its derived cost decides that sum. The independent
  figure there is the residual against the PC-derived cost, and the report
  states their gap.
- If a run's |gap| is above 10%, the report explains it from the data in
  hand: method bias (the `call-twice-cart-draw` column), time inside
  `cart_draw`'s own body (its prologue and locals, expected well under the
  floor), a PC-derived term (the residual comparison), or second-call
  effects visible in the submit delta. If none of those accounts for it, the
  report says so plainly and leaves the gap to step 7 as an open item. The
  numbers are never adjusted to pass.

### Patches: generated, used, discarded

- **Not committed.** The per-child patches are throwaway experiments in the
  plan's sense. The committed `psx/probes/` patches are the probe tool's
  fixtures (they check the tool itself), which is harness; level-1 probes
  check the game, which is experiment. Committing nine more patches would
  also make them fixtures that "are refreshed with" every commit moving
  `cart.c`, a maintenance cost the plan does not want.
- They are written by hand or by a one-off `sed` from HEAD's `cart.c` with
  `git diff` into `tmp/plan20-draw-l1/` (git-ignored), one file per probe,
  named `<child>-twice.patch` and `draw-layers-1-delay.patch`, and run with
  `PSX_PROBE_LABEL=l1-<child>-twice` / `l1-draw-layers-1-delay` so their
  outputs stay apart under `tmp/psx/probe/`.
- The report gives, per probe, a one-line description: the call site, the
  inserted statement, the base commit. That is enough to regenerate any of
  them.
- An `invalid` probe is never a figure. The cause is read from its
  `analysis/probe.txt` and `build.log`; a patch mistake is fixed and the
  probe re-run; a `p8_fatal` in a probe build (should not happen: the probe
  build fits `cart_draw` twice) is recorded as a finding and the child falls
  back to the fixed delay on that run.

### Order of operations

1. Pre-checks (see the hypotheses below): HEAD carries step 5 or step 5's
   run is not in flight; `src/`, `psx/` and `worst-case/full_game.json`
   unchanged since `b7d8226` apart from step 5's declared files.
2. `make psx-probe-baseline` (all six runs). Compare each run's window sums
   with step 3's probe baseline table (five runs) and with the measured
   replay's figures (step 2's table, step 4's `draw-6000` row): a gap above
   2 hblanks on a part is reported and investigated before any probe is
   read.
3. `make frame-profile-coverage` (about 1 min), for the PC shares and calls
   of the three windows; its verdict must still be `coverage ok`.
4. `make psx-probe PATCH=psx/probes/call-twice-cart-draw.patch
   PSX_SHORT_RUNS="start draw-5564 draw-6000" PSX_PROBE_LABEL=l1-cart-draw-twice`.
5. The seven call-twice probes on the three runs, except `draw_layers#1`'s,
   which runs on `draw-5564 draw-6000` only.
6. The `draw_layers#1` delay probe on the three runs.
7. Fallback delay probes for any child whose game state differed.
8. Report section.

About ten probe invocations at about one minute a run each: roughly half an
hour of wall time, all outside `make test` and `make e2e`.

### What is recorded in `docs/psx-perf-levers.md`

A new section **"Draw level 1: `cart_draw`'s children"**, appended at the
end of the file (after "PC frame profile and draw coverage" and after
anything step 5 added, H-5-3), and the file's opening paragraph extended by
one clause naming it. The section holds:

- the measured commit (full hash), the commands, and the rule recalled in
  two sentences (call-twice by default, fixed delay + PC share where the
  game state would change, `start`'s `draw_layers#1` by step 1.3's decision);
- **the rebuilt probe baseline**: one row per run (six), update, draw,
  submit, total, window frames, mean window frame and 2% of it, with the
  step 3 / step 2 total beside it and the gap;
- **`cart_draw`'s own cost** per probed run: baseline draw part, call-twice
  delta, bias in percent, game state;
- **one table per probed run**: child, method, draw delta (window), cost per
  frame, % of `cart_draw`, % of mean window frame, calls per frame (PC; PSX
  where measured), update delta, submit delta, game state, text-size change;
  PC-derived rows marked, with the residual beside them;
- **the additivity check**: per run, Σ children, `cart_draw`'s own, gap; Σ
  against the call-twice delta; the verdict (within 10% or not) and, if not,
  the explanation;
- **the patches**: one line each (probe label, call site, inserted
  statement), base commit, and the note that they were generated in `tmp/`
  and discarded;
- **hand-off to step 7**: per run, the children above 2% of the mean window
  frame (the branches step 7 goes down) and those under it (closed at level
  1). No do-less candidate is written at this level: candidates belong to
  the functions step 7 explores.

Integer hblanks; percentages with one decimal. The PC shares quoted are
labelled PC time, as the existing section does.

### Hypotheses about step 5 (being implemented in parallel)

Step 5 is `docs/prd/triage/phase1-step5-full-run-baseline.md`: a
`PSX_FULL_GAME_ASSERTS=on|off` option on `make psx-full-game-check`, an
explicit `-DPSX_MEASURED_REPLAY_ASSERTS` on `psx-build`'s configure, one
asserts-off full run, its `frames.txt` committed as
`docs/psx-perf-levers-full-run-frames.txt` and a "Full-run baseline (asserts
off)" section in the report. Nothing of its outcome is read here; each
assumption is checked when this step starts, with its fallback.

**Checked against the committed step 5 (`d8a41fc`): all seven hold.** Step 5
changed only the `Makefile` (`PSX_FULL_GAME_ASSERTS`, `psx-build` passing
`PSX_MEASURED_REPLAY_ASSERTS` explicitly) and docs; nothing under `src/`,
`psx/`, `tools/` or `worst-case/`, and not the probe recipe. Its report
section "Full-run baseline (asserts off)" sits between "Short-run baseline"
and "Probe tool", with the frame table in
`docs/psx-perf-levers-full-run-frames.txt`. The full run is finished, so
H-5-5 no longer constrains anything.

- **H-5-1 — step 5 changes no file under `src/`, `psx/` nor
  `worst-case/full_game.json`.** Its PRD changes the Makefile, the docs and
  the report only. So the probe fingerprint (tree ids of `src/` and `psx/`,
  blob of `full_game.json`) is the same before and after step 5, and this
  step measures the same code as steps 2–4. Check: `git log b7d8226..HEAD --
  src psx worst-case/full_game.json` is empty. Fallback: if something there
  changed, the baseline is rebuilt anyway (step 2 above), the report names
  the change, and the comparison with the step 3 baseline is labelled
  cross-code.
- **H-5-2 — step 5 does not change the `psx-probe-baseline` / `psx-probe`
  recipe, `psx-short-runs-cut`, `tools/psx_probe.sh` or
  `tools/psx_boot.sh`.** Its `psx-build` change does not reach the probe
  recipe, which already passes `-DPSX_MEASURED_REPLAY_ASSERTS=OFF` on its
  own configure. Check: `git log b7d8226..HEAD -- Makefile tools/` and a
  read of the two recipes' diff if the Makefile moved. Fallback: if the
  probe interface changed, use it as found and record the commands actually
  run; the methods above do not change.
- **H-5-3 — step 5 inserts its report section right after "Short-run
  baseline (asserts off)" and adds a companion file, without renaming the
  existing sections.** This step appends its section at the end of the file
  and touches nothing else but the opening paragraph. Fallback: if step 5
  rewrote the opening paragraph or moved sections, this step adds its clause
  and section to whatever is there; if the two edits conflict, merge by hand
  keeping both.
- **H-5-4 — this step needs no figure from step 5.** Every figure here is
  relative to the probe baseline of the same short run; the 2% threshold is
  of the run's own mean window frame. The full-run table is not read. So
  step 5 may land before or after this step's measurements. Fallback: none
  needed; carrying level-1 shares onto the judge's frames is step 12's job.
- **H-5-5 — step 5's run and this step's probes do not share outputs:**
  step 5 writes under `tmp/psx/full-game/`, this step under `tmp/psx/probe/`,
  `tmp/psx/short-runs/` (re-cut, same content) and `tmp/frame-profile/`.
  But both boot pcsx-redux with wall-clock timeouts (the probe's is 20 s a
  boot) and both run `psx-tools-test` as a prerequisite. Decision: the
  probes are never started while step 5's full run is in flight in the same
  working copy or container; the implementer waits for it (or for step 5's
  commit). The emulator's hblank counts do not depend on host load, but a
  timeout or a racing rebuild of the same test binary would. Fallback: if a
  probe boot timed out while another emulator ran, re-run that probe alone.
- **H-5-6 — step 5 does not add or change any run of
  `psx/short_runs.txt`.** Its PRD does not touch the manifest. So the
  probed runs are the three named above and the baseline covers six runs.
  Fallback: if a `draw-*` window was added, it is probed like `draw-6000`
  and the decision above extends to it; an added update window is
  baselined, not probed.
- **H-5-7 — whatever step 5's verdict (baseline recorded, or "not a
  baseline" under its stop rule), this step proceeds.** A red full run would
  question the full-run baseline, not the short runs, whose asserts-off game
  state already equals the PC's (step 2's and step 4's checks) and is
  re-checked by every probe's game-state verdict. Fallback: if step 5's
  stop rule fired on `gpu-state-game-lines`, this step's report adds one
  line noting that its short-run figures are unaffected (their game lines
  match), and goes on.

### Not decided by a guideline call

Every question was settled from the plan's Rules, the step notes and the
code: `draw-6000` probed (step 4's text), `start`'s `draw_layers#1` by fixed
delay (step 1.3's guideline decision), PC-derived cost for fixed-delay
children (the plan's update method), patches discarded (the plan's
experiment rule), cost = draw-part delta (the draw part brackets
`cart_draw`), reference = baseline draw part with the call-twice delta as
bias column. No `guideline` agent was needed.

## Implementation Chunks

1. **Baselines.** Pre-checks (H-5-1, H-5-2, H-5-5, H-5-6);
   `make psx-probe-baseline`; `make frame-profile-coverage`; the
   `call-twice-cart-draw` fixture on the three runs. Write the section's
   head: commit, commands, rule, rebuilt baseline table, `cart_draw`'s own
   cost table, PC shares and calls per child per window. Commit the report.
2. **Children and verification.** Generate the patches in
   `tmp/plan20-draw-l1/`; run the six call-twice probes on three runs, the
   `draw_layers#1` call-twice on the two draw windows, the delay probe on
   three runs, and any fallback delay probe. Write the per-run tables, the
   additivity check (with explanation if a gap exceeds 10%), the patch list
   and the hand-off to step 7. Commit the report.

## Testing Decisions

- A good test checks an observable contract, never a recipe's text. This
  step writes no code: it runs existing, host-tested tooling and records
  numbers. There is nothing to unit-test, so **no new automated test**.
- The proof of the tooling is already in place and is re-exercised: the
  probe baseline is compared with step 3's (the tool is still sound), the
  `call-twice-cart-draw` fixture is re-run (the method still doubles draw
  within 10%), the delay probe's delta ÷ delay must match the PC call
  count within step 1.3's tolerance, and every probe carries its own
  game-state and validity verdicts.
- `psx-tools-test` runs as the probe's prerequisite and must stay green;
  `make frame-profile-test` is not required (the tool is unchanged).
- `make test-all` stays green and unchanged, and is run once before the
  first commit as the commit skill does. No `*.sha256` moves; `psx-check` is
  not re-run (no build input changes).

## Out of Scope

- Going below `cart_draw`'s direct children: the entities `draw_layers`
  dispatches to, `help_screen_draw` itself, the draw families (step 7).
- Do-less candidates (step 7, per explored function).
- Per-unit backend costs, the 9× print and meter experiments, and reading
  the submit deltas as backend cost (step 8).
- Probing the update-only runs, `cart_update60`'s children, or calibrating
  PC proportions against PSX costs (step 10, which will use this step's
  figures).
- Carrying the shares onto the full run's frames (step 12).
- Committing the probe patches, or changing the probe tool, the manifest,
  the game, the Makefile or any test.
- Any full run; anything on the Miyoo or PCSX-ReARMed; audio; launching the
  raylib app.

## Further Notes

- Expected shape, from the PC shares (not a target): `draw_layers#1`
  dominates every window (about 99% of `start` and `draw-6000`, 75% of
  `draw-5564`), `draw_layers#2` is the only other sizeable child and only on
  `draw-5564` (about 25%, the window where `print` is heaviest), and the
  remaining five are at or under the timer floor. The 2% threshold of the
  mean window frame is about 27 hblanks a frame on `draw-5564`, about 19 on
  `draw-6000`, and 189 on `start`, whose one window frame is dominated by
  the update that generates the galaxy: on `start` only `draw_layers#1` can
  clear it.
- The call-twice bias seen in step 3 (+4.3% to +8.5% over double) means the
  measured children's sum is expected to land a few percent above
  `cart_draw`'s own; that is inside the 10% budget, and the call-twice
  column of the additivity check makes it visible.
- Step 10 will compare these PSX costs with the PC proportions of the same
  children. The PC-derived `start` figure must not be reused there as a
  PSX measurement; the report's label makes that explicit.
- Hypotheses H-5-1 to H-5-7 above are the only assumptions about step 5.
