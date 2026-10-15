---
title: "Draw tree: go down every draw branch above 2% of the mean window frame (plan 20, phase 2, step 7)"
description: "Step 6 splits cart_draw's cost among its direct children, but nobody knows yet which functions below draw_layers hold that cost on the PSX, nor what each of them could stop doing."
status: needs-triage
---

## Plan step (verbatim)

> 7. Go down the draw tree. Each branch above 2% of the mean window frame is
>    probed one level deeper, until it is a leaf or under 2%. Each function
>    explored gets its "do less" candidates in the report: repeated calls,
>    identical redraws, recomputation. Verified when every branch above 2% is
>    explored or explicitly marked as a leaf.

Plan file: `docs/plan-20.md`. Rules in force (section 2, "How discovery
works"): the principle is to **do less** (calls repeated N times, things
redrawn identically every frame, work recomputed when nothing changed); hot
paths are found top-down, going down only into branches above 2% of the
run's mean window frame; a pre-chosen list of suspects was rejected; drawing
functions are called twice and the frame grows by exactly the function's
cost; a second call that only advances `visual_random` is fine, but if the
run's game-state lines no longer match, that function falls back to the
fixed delay (which measures the call count, the cost then coming from PC
timing proportions); throwaway experiments are discarded and **nothing is
kept from an experiment except its numbers in the report; the only code this
plan commits is the measuring tooling**. Guardrails: nothing in the game or
the shipped drawing path changes; `make e2e`, every `*.sha256` and
`psx-check` stay as they are; new tooling is host-tested where the repo
host-tests its tools and stays out of `make test` and `make e2e`; never
launch the raylib app.

Step notes in force:

- **1.3**: `make psx-probe-baseline` / `make psx-probe PATCH=<file>
  [PSX_SHORT_RUNS=] [PSX_PROBE_LABEL=]`; the probe build doubles the GPU
  frame buffer, so anything up to `cart_draw` twice fits; `psx_probe_delay()`
  (1..200 hblanks, one value per build) counts its calls; noise floor 0;
  call-twice `cart_draw` costs +4.3% to +8.5% over double (second-call bias).
  Guideline decision (1): `help_screen_draw` (`src/game/help.c:98`) ticks the
  RNG once a call, so it and any branch running it on the title screen take
  the fixed delay. Guideline decision (2): a part's resolution floor is
  max(2% of delay × window frames, 2 hblanks).
- **1.4**: `make frame-profile` reports PC calls and time per direct child of
  `cart_draw` and `cart_update60` only (by call site, `CALL_PROFILE_CHILD_CAP`
  16 children a root); `make frame-profile-coverage` takes K = 4 profiles and
  the per-window median. The probe fingerprint is the tree ids of `src/` and
  `psx/` at `HEAD` plus the blob of `full_game.json`: any committed change
  under `src/` makes the probe baseline stale.
- **1.5**: no source change; full-run baseline recorded.

## Problem Statement

Step 6 tells the plan how `cart_draw`'s cost splits among its seven direct
children on `start`, `draw-5564` and `draw-6000`, and which of them weigh
more than 2% of the mean window frame (expected: `draw_layers#1` everywhere,
`draw_layers#2` on `draw-5564`). That is where the knowledge stops.
`draw_layers` is a dispatcher: it sorts the entities into fifteen layer lists
every frame and calls each one's `draw` or `draw_overlay` method through a
function pointer. Sixteen different methods sit behind those two calls
(planets, slipways, the background, buttons, the help screen, tooltips,
message boxes, particles...), and under them helpers such as
`hud_print_outlined`, which already calls `p8_print` nine times a text.

Nobody can say yet which of these functions hold the 19,000 hblanks of
`draw-5564`'s window, which are called many times a frame, which draw the
same picture every frame, or which recompute something that did not change.
Without that, step 12 has no draw levers below "make `print` cheaper" and
"drop the meter", and the human cannot choose.

The PC profile cannot answer either: it sees only `cart_draw`'s direct
children, its cap of 16 children a root is below the twenty-odd functions
`draw_layers` calls, and PC proportions are not PSX costs (the PC rasterises
every pixel in software, the PSX hands primitives to the GPU).

## Solution

From a commit at or after step 6, with no change to the game, and only one
tooling change committed:

1. **A subtree root for the PC profile.** `make frame-profile-subtree
   ROOT=<function>` profiles the draw windows with the profile's draw root
   moved from `cart_draw` to any function, so the direct children of any node
   of the draw tree (direct calls and function-pointer calls alike) are
   listed with their calls per window frame and their PC share of the node.
   This is the map: it says what the children are and how often each is
   called. It never closes a branch.
2. **The probe baseline is rebuilt** once after that tooling commit (it
   changes `src/`, so the old baseline is stale).
3. **Top-down, level by level, on the PSX.** For every node above 2% of the
   mean window frame of a run, each of its children gets a PSX cost on that
   run with the probe tool: call-twice by default, the fixed delay plus a PC
   share where call-twice would change the game state or cannot be a clean
   second call. Small siblings are probed together as a group: a group whose
   total stays under 2% closes every member at once with an upper bound;
   a group above 2% is split until each piece is under 2% or a single
   function.
4. **The descent stops** at a leaf (a PICO-8 runtime call such as `p8_print`
   or `p8_spr`, whose inside is step 8's backend work, or a function with no
   instrumented children) or at a node under 2%.
5. **Each explored function** (every node above 2% that was opened) gets its
   "do less" candidates in the report, from its code and its measured
   figures: repeated calls, identical redraws, recomputation, each with an
   upper bound on the gain.
6. **A "Draw tree" section** in `docs/psx-perf-levers.md` holds the tree per
   run, the candidates, and the verification table: every node above 2% is
   "explored" or "leaf".

The per-node patches are generated in `tmp/`, used and discarded; the report
describes each in one line so it can be regenerated.

## User Stories

1. As the plan pilot, I want the PC profile to take any function as its draw root, so that the children of any node of the draw tree are listed from data instead of from code reading alone.
2. As the plan pilot, I want function-pointer calls (the `draw` and `draw_overlay` methods behind `draw_layers`) listed as children like direct calls, so that the dispatcher's sixteen methods appear as distinct nodes.
3. As the plan pilot, I want the profile's child cap raised above the number of functions `draw_layers` calls, so that profiling `draw_layers` does not stop on a `p8_fatal`.
4. As the plan pilot, I want an unknown or ambiguous root name refused with a clear message and exit 1, so that a typo never silently profiles `cart_draw`.
5. As the plan pilot, I want the default root to stay `cart_draw`, so that `make frame-profile`, `frame-profile-breakdown-check` and `frame-profile-coverage` give the same output as before.
6. As the plan pilot, I want one make target that profiles a chosen root on the draw windows K times and summarises each child's calls per window frame and median PC share of the root, so that a level of the tree is mapped in one command.
7. As the plan pilot, I want the subtree outputs kept apart per root and per run, so that mapping one node never overwrites another's figures.
8. As the plan pilot, I want the new tooling host-tested in `frame-profile-test`, so that it is checked where the profile tool already is, outside `make test`.
9. As a developer, I want `make test` and `make e2e` untouched in content and duration, so that the fast loop stays fast.
10. As the plan pilot, I want the probe baseline rebuilt after the tooling commit and compared with step 6's, so that no probe is refused as stale and any shift is seen before a delta is read.
11. As the plan pilot, I want the tree explored only from the runs step 6 probed (`start`, `draw-5564`, `draw-6000`), so that the step keeps step 6's scope.
12. As the plan pilot, I want a node explored on a run only when it is above 2% of that run's mean window frame, so that the plan's threshold is applied per run, as step 6's hand-off states it.
13. As the plan pilot, I want each child probed only on the runs where its parent is above 2%, so that no boot is spent on a branch already closed on that run.
14. As the plan pilot, I want a node defined as one function reached from one parent (by call site, or through the dispatcher for a method), so that a helper called from several parents is costed under each.
15. As the plan pilot, I want call-twice as the default method, the second call placed right after the first with the same arguments, so that the frame grows by the node's cost, as the rule assumes.
16. As the plan pilot, I want a method reached only through `draw_layers` doubled inside its own file (the method body renamed, a wrapper calling it twice), so that no function pointer has to be compared in the dispatcher.
17. As the plan pilot, I want a function with several parents doubled at the one call site being probed, never by wrapping the function, so that the delta belongs to that node alone.
18. As the plan pilot, I want any node whose call-twice gives `game-state differs` re-probed with the fixed delay on that run, so that the plan's fallback rule applies at every level.
19. As the plan pilot, I want `help_screen_draw_method` probed with the fixed delay, never call-twice, so that step 1.3's guideline decision holds below level 1.
20. As the plan pilot, I want the children of an RNG-ticking function still tried with call-twice first, so that only the function that ticks the RNG loses the direct measurement.
21. As the plan pilot, I want bookkeeping children with side effects (adding to a list, allocating from the frame zone, setting the camera) never call-twice'd, so that a doubled list entry never doubles the drawing below it.
22. As the plan pilot, I want a fixed-delay node's cost computed as its PC share of the parent times the parent's PSX cost, marked PC-derived, so that the plan's PC-proportion method is used exactly where it prescribes it.
23. As the plan pilot, I want the delay sized from the node's PC calls per frame, so that a function called fifty times a frame does not add thousands of hblanks per frame.
24. As the plan pilot, I want small siblings probed as one group, so that the many tiny children of a dispatcher do not cost one boot each.
25. As the plan pilot, I want a group under 2% to close each member with the group's delta as upper bound, so that closing them is sound whatever PC says.
26. As the plan pilot, I want a group above 2% split in two, by PC estimate, until each piece is under 2% or a single function, so that a heavy member hidden in a group is always found.
27. As the plan pilot, I want PC figures used only to choose groups, never to close a branch, so that a PC-light but PSX-heavy function is not missed.
28. As the plan pilot, I want each node's own time (its cost minus the sum of its measured children) reported as "self", so that a dispatcher's own work (building fifteen layer lists every frame) is visible.
29. As the human reading the report, I want a self time above 2% flagged and given its own candidates, so that work outside any child is not lost.
30. As the plan pilot, I want the descent to stop at the PICO-8 runtime API (`p8_print`, `p8_spr`, `p8_rectfill` and the rest), so that the backend's inside stays step 8's.
31. As the human reading the report, I want each runtime leaf's calls per frame and cost under its parent, so that "print called nine times per text" style findings are measured at the call site.
32. As the plan pilot, I want a function with no instrumented children marked "leaf (self)", so that the descent has a defined end for game code too.
33. As the plan pilot, I want the descent repeated level by level until every open node is a leaf or under 2%, so that the step's verification holds.
34. As the human reading the report, I want every explored function's "do less" candidates listed, so that each one is a lever step 12 can rank.
35. As the human reading the report, I want each candidate typed as repeated calls, identical redraws or recomputation, so that it maps on the plan's principle.
36. As the human reading the report, I want each candidate's evidence (calls per frame, code location, what is redrawn or recomputed), so that I can judge it without re-reading the code.
37. As the human reading the report, I want each candidate's upper-bound gain in hblanks per window frame, taken from the measured cost of the part it would remove, so that step 12 has a figure to rank.
38. As the human reading the report, I want each candidate's risk to the pixel-identical rule named (identical, `visual_random` only, or would change pixels), so that candidates the plan rules out are recognisable.
39. As the human reading the report, I want the known 9× outlined print and the shipped draw meter recorded where they appear in the tree, so that they are located, not rediscovered.
40. As the human reading the report, I want the tree per run as an indented table (node, method, calls per frame, cost per frame, % of parent, % of mean window frame, status), so that the hot paths read at a glance.
41. As the human reading the report, I want every delta at the timer's floor written "≤ 2 over the window", so that a zero is never read as exact.
42. As the human reading the report, I want PC-derived and group-bound costs marked as such, so that I never mistake them for measured ones.
43. As the plan pilot, I want each node's children-sum compared with the node's cost, gap in percent, so that a large unexplained gap is noticed.
44. As the human reading the report, I want a gap above 10% explained (second-call bias, self time, a PC-derived term) or named as open, so that the numbers are never adjusted to fit.
45. As the human reading the report, I want the verification table listing every node above 2% on any run with its status "explored" or "leaf", so that the step's verification is checkable line by line.
46. As a later reader, I want each patch described in one line (label, node, call site or wrapper, inserted statement, base commit), so that any probe can be regenerated.
47. As the plan pilot, I want the patches written under `tmp/` and never committed, so that only measuring tooling is committed.
48. As the plan pilot, I want each probe run with its own label, so that no probe's outputs overwrite another's.
49. As the plan pilot, I want an invalid probe investigated and re-run once fixed, never recorded as a delta, so that the report holds only valid measurements.
50. As the plan pilot, I want every assumption about step 6's outcome written as a hypothesis with a check and a fallback, so that this PRD holds whatever step 6 finds.
51. As the plan pilot, I want no backend per-unit cost, no 9× print experiment and no meter experiment run here, so that step 8 keeps its work.
52. As a developer, I want no change to the game, the engine, the runtime, the shipped PSX path, `psx/` or the e2e files, so that every `*.sha256` and `psx-check` are untouched.

## Implementation Decisions

### Tooling: the subtree root (the only committed code)

- **Decision: extend the PC profile instead of mapping the tree from code
  alone.** The dispatcher's children are chosen at run time through function
  pointers, and "repeated calls" needs calls per frame for every node; the
  PSX delay probe gives one count per boot, the PC profile gives every child
  of a node in one run of a few seconds. The plan allows committing measuring
  tooling. No guideline call: the plan's rule ("the only code this plan
  commits is the measuring tooling") covers it.
- **The profile binary** takes an optional fourth argument, the draw root's
  symbol name, default `cart_draw`. It is resolved through the symbol table
  the binary already reads, with the same load offset. Refused with exit 1
  and a message naming the symbol: not found, several text symbols of that
  name (static functions of the same name in two files), or equal to
  `cart_update60`. The update root stays `cart_update60`.
- **Semantics, kept from the existing bookkeeping:** the root is recognised
  only when no root is running, so a function also called under
  `cart_update60` counts only its draw-side entries; every entry of the root
  in a frame adds up (`draw_layers` therefore sums its two messages; its
  children still split cleanly, since draw and overlay methods are distinct
  functions except the shared bookkeeping helpers); a child is one function
  from one call site, so the sixteen methods behind the one indirect call in
  `draw_layers` are sixteen children.
- **Output:** the files and line formats are unchanged; the header of
  `frames.txt` and `summary.txt` gains the root's name (`root draw
  <symbol>`), so a reader knows which node "draw" means. Existing readers
  (`tools/frame_profile_check.sh` breakdown and coverage) must keep parsing
  a default-root profile; the new header line is either already ignored by
  them or they learn to skip it.
- **`CALL_PROFILE_CHILD_CAP` goes from 16 to 64.** `draw_layers` calls about
  25 distinct functions (16 methods, plus list, layer and camera helpers);
  64 leaves room for any other dispatcher-like node. The frame table grows to
  about 15 MB of static memory on the host, acceptable for a dev tool.
- **The make target `frame-profile-subtree ROOT=<symbol>
  [PSX_SHORT_RUNS=<names>] [FRAME_PROFILE_REPEATS=<k>]`** profiles each
  named draw run's `run.json` (default: `start` and every `draw-*` run of the
  manifest) K times (default 4) with that root into
  `tmp/frame-profile/subtree/<symbol>/<run>/<k>/`, checks same-game as the
  other targets do, and writes `tmp/frame-profile/subtree/<symbol>/summary.txt`
  through a new `subtree` mode of `tools/frame_profile_check.sh`: per run,
  the root's mean PC time per window frame, then per child its calls per
  window frame (min–max over the window, from the first profile; calls do
  not vary between profiles) and its median PC share of the root over the K
  profiles (window frames only, from `map.txt` as coverage does). Outside
  the suite, never boots.
- **Interface sketch of the summary** (decision on the shape, not code):

  ```
  root draw_layers run draw-5564 window-frames 15 root-ns-per-frame <n>
  child draw_layers planet_draw_method calls 23-23 share 31.4%
  child draw_layers hud_print_outlined#2 calls 9-11 share 4.0%
  ...
  self draw_layers share 6.2%
  ```

### The tree: nodes, runs, threshold

- **Runs:** `start`, `draw-5564`, `draw-6000` — step 6's probed runs
  (H-6-2). The update-only runs are not explored.
- **Threshold:** 2% of the run's mean window frame, from the rebuilt probe
  baseline (window total ÷ window frames). Expected about 189 hblanks a
  frame on `start`, 27 on `draw-5564`, 19–20 on `draw-6000` (H-6-6).
- **Node:** a function reached from one parent: through a call site for a
  direct call, through the dispatcher for a method. Its cost is the total of
  all its calls under that parent per window frame.
- **Starting nodes:** the children of `cart_draw` that step 6 lists above
  2%, per run (H-6-3). Each is opened on the runs where it is above.
- **Opening a node on a run:** map its children with
  `make frame-profile-subtree ROOT=<node's function>`, then give every child
  a PSX cost on that run (methods below), then classify each child: above
  2% → opened next; under 2% → closed; leaf → closed as leaf. The node itself
  is then "explored".
- **Leaves:** (a) a PICO-8 runtime function (`p8_` prefix: drawing
  primitives, `p8_print`, palette and camera calls) — its inside is the
  backend, step 8's; (b) a game or engine function with no instrumented
  child in the subtree profile. A leaf above 2% is still costed and still
  gets its candidates (counted calls, call site), it is only not opened.
- **Self:** a node's cost minus the sum of its children's costs. Reported
  for every explored node. A self above 2% is flagged and gets candidates
  from the node's own body (for `draw_layers`: the fifteen layer lists
  rebuilt from the message bucket every frame). It is not probed separately:
  it is a difference of measurements, labelled so.

### Methods

- **Call-twice (default).** Two shapes:
  - *site patch*: the call statement duplicated right after the original in
    the parent, same arguments; used for any direct call;
  - *wrapper patch*: for a method reached only through `draw_layers`, its
    body renamed (`<method>_once`) and the method made to call it twice;
    used because the method's symbol is static to its file and comparing
    function pointers in the dispatcher would change `draw_layers` itself.
  The node's cost on a run is the probe's window **draw** delta. Update and
  submit deltas are recorded beside it; an update delta beyond
  max(2% of the draw delta, 2 hblanks) is flagged as a leak; submit growth is
  backend cost for step 8, not the node's.
- **Not call-twice'd, ever:** `help_screen_draw_method` and
  `help_screen_draw` (RNG tick, step 1.3), and bookkeeping children whose
  second call changes what is drawn or allocated (adding to a list,
  allocating from the frame zone, `apply_camera`). These take the fixed
  delay, or stay in the parent's self if they are under 2% together by PC
  estimate and the self is under 2%.
- **Fixed delay.** `psx_probe_delay(d)` at the node's call site (or at the
  method's entry for a wrapper-shaped node), d = clamp(⌊200 ÷ PC calls per
  window frame⌋, 1, 20) hblanks, so a node adds at most about 200 hblanks a
  frame. Gives the PSX call count (delta ÷ d), checked against the PC
  calls within step 1.3's tolerance. **Cost**: the node's median PC share
  of its parent (subtree profile) × the parent's PSX cost on that run,
  marked "PC-derived", with the parent's residual (parent cost − measured
  siblings) beside it as a cross-check.
- **Game-state fallback:** any call-twice probe reporting `game-state
  differs` on a run is re-run with the fixed delay on that run; the
  call-twice delta is kept, labelled "game state differs, not used".
- **Groups.** Children of one node whose PC-estimated cost (PC share × the
  parent's PSX cost) is under 2% go into one call-twice group patch per
  parent (each member doubled at its own site or wrapper). Group delta under
  2% → every member closed, "≤ group delta" as its cost. Group delta at or
  above 2% → split the group in two by PC estimate and probe each half,
  recursively, until each piece is under 2% or one function (then probed and
  classified as usual). Members excluded from call-twice (above) are never in
  a group. Sound because a call-twice delta is a sum of non-negative member
  costs; PC only chooses the grouping.
- **Children estimated above 2% by PC** are probed one by one.
- **Resolution:** a window delta of 0–2 hblanks is written "≤ 2 over the
  window". No amplified probe.
- **Second-call bias:** the +4–9% of step 1.3 is expected on every
  call-twice cost; it is noted, never corrected.

### "Do less" candidates

- **Who gets them:** every explored node, every leaf above 2%, and every
  self above 2%.
- **Each candidate:** kind (repeated calls / identical redraw /
  recomputation); where (function and file, file:line at the base commit);
  evidence (calls per frame, what is drawn or computed again, why it does
  not change between frames or between calls); upper-bound gain in hblanks
  per window frame on each run where the node was explored (the measured
  cost of the part it would remove: the whole node, a fraction proportional
  to the calls it would skip, or the self); pixel risk (identical /
  `visual_random` only / would change pixels — the latter kept but marked
  "ruled out by the plan unless reopened").
- **From code reading plus the measured figures**, never from a throwaway
  experiment: bounding a lever by experiment is step 8's (print, meter) or a
  later plan's. The known items (9× outlined print in `hud_print_outlined`,
  the shipped draw meter) are attached to the nodes where they appear,
  referencing the plan's "Already known".
- **Not ranked here:** ranking by gain over cost is step 12's.

### Patches: generated, used, discarded

- Written in `tmp/plan20-draw-tree/` (git-ignored), one per probe, named
  `<node>-twice.patch`, `<node>-delay.patch`, `group-<parent>-<n>.patch`;
  run with `PSX_PROBE_LABEL=l2-…`, `l3-…` (level prefix) so outputs stay
  apart. Never committed (same reasoning as step 6: experiments, not
  fixtures).
- The report gives one line per probe: label, node(s), shape (site,
  wrapper, delay d), inserted statement, base commit.
- An `invalid` probe is never a figure: the cause is read from
  `analysis/probe.txt` and `build.log`, the patch fixed and re-run. A
  `p8_fatal` that is not a patch mistake (should not happen: no probe doubles
  more than `cart_draw` does) is recorded as a finding and that node falls
  back to the fixed delay.

### Order of operations

1. Pre-checks: H-6-1, H-6-5 (step 6 committed, its section present, no
   source change since).
2. Tooling chunk (below), committed with `make test-all` and
   `frame-profile-test` green.
3. `make psx-probe-baseline` (all runs); compare each run's window sums with
   step 6's rebuilt baseline: equal within 2 hblanks per part (the tooling
   touches no file the PSX build compiles), otherwise investigate before any
   probe.
4. Level 2: `frame-profile-subtree ROOT=draw_layers`; probe the children of
   `draw_layers#1` on its runs and of `draw_layers#2` on its runs. Because
   both sites call the same function, every site patch for level 2 is in a
   method or below `draw_layers`; a wrapper patch doubles a method under
   whichever site calls it, which is the node by construction (a draw method
   only runs under `#1`, an overlay method only under `#2`).
5. Levels 3+: for each node opened, its subtree profile, then its children's
   probes, until the open set is empty.
6. Candidates for every explored node; report section; verification table.

Wall time estimate: one probe boot is about a minute per run; with groups,
on the order of 40–80 probe invocations, one to three hours, all outside
`make test` and `make e2e`. Many invocations cover one run only.

### What is recorded in `docs/psx-perf-levers.md`

A new section **"Draw tree"**, appended after step 6's "Draw level 1"
section (H-6-8), and one clause in the opening paragraph naming it. It
holds:

- measured commit, commands, the method recalled in a few sentences
  (call-twice, wrapper vs site, fixed delay + PC share, groups and their
  bound, leaves, self);
- the rebuilt probe baseline beside step 6's;
- **one tree table per run**, indented by level: node, method, calls per
  window frame (PC; PSX where a delay ran), cost per frame, % of parent,
  % of mean window frame, status (explored / leaf / under 2% / ≤ group
  bound), update and submit deltas, game state; self rows under each
  explored node;
- per explored node, the children-sum vs node cost gap, explained when above
  10% or named as open;
- **"do less" candidates**, grouped by explored node;
- **verification table:** every node above 2% on any run, with run, status
  (explored or leaf) and where its candidates are;
- the patch list (one line each) and the note that they were discarded;
- a short "hand-off" paragraph naming the leaves step 8 will cost per unit
  (which runtime families carry the heaviest leaves).

Integer hblanks, percentages with one decimal; PC shares labelled PC time.

### Hypotheses about step 6 (not done yet; drafted in parallel)

Step 6 is `docs/prd/triage/phase2-step6-draw-level1.md`. Nothing of its
outcome is known while this PRD is written; each assumption is checked when
this step starts.

- **H-6-1 — step 6 is committed with a "Draw level 1" section in the
  report**, holding per run and per child the cost per frame, the share of
  the mean window frame and the list of children above 2% (its "hand-off to
  step 7"). Check: the section and its hand-off list are in
  `docs/psx-perf-levers.md` at `HEAD`. Fallback: if the section is missing
  or has no hand-off list, this step does not start and reports step 6 as
  not done; it never redoes step 6's level-1 probes itself. If the list is
  missing but the per-child table is there, the list is derived from the
  table (2% of each run's mean window frame) and the derivation is written
  in this step's section.
- **H-6-2 — step 6 probed `start`, `draw-5564` and `draw-6000`**, and the
  manifest still has those six runs. Check: the section's run tables and
  `psx/short_runs.txt`. Fallback: this step explores exactly the runs step 6
  probed; a run step 6 dropped is not explored here; a `draw-*` run step 6
  added is explored like the others.
- **H-6-3 — the children above 2% are `draw_layers#1` on all three runs and
  `draw_layers#2` on `draw-5564` only.** (From the PC shares: 99.5 / 75.4 /
  99.1% and 0.2 / 24.6 / 0.8% of `cart_draw`.) Check: step 6's hand-off
  list. Fallback: the starting nodes are whatever the list names, per run;
  the method is the same for any starting node. A getter or `p8_cls` above
  2% would be a leaf at once (no instrumented child, or a runtime call).
- **H-6-4 — `draw_layers#1` on `start` was costed with the fixed delay and
  a PC share** (step 6's decision), so its `start` cost is PC-derived.
  Check: its row's method. Fallback: if step 6 measured it otherwise, use
  its figure as given. Either way, under it on `start`, only
  `help_screen_draw_method` is excluded from call-twice; its siblings are
  call-twice'd with the game-state check, and the `start` tree is labelled
  "rooted on a PC-derived cost" when H-6-4 holds.
- **H-6-5 — step 6 committed no code and did not change the probe tool, the
  manifest or `src/`.** Its probe baseline therefore carries the same
  fingerprint as `HEAD` before this step's tooling commit. Check: `git log`
  since step 6's base commit `-- src psx tools Makefile`. Fallback: if the
  probe interface changed, use it as found and record the commands run; if
  `src/` changed, the comparison in "Order of operations" step 3 is labelled
  cross-code.
- **H-6-6 — step 6's rebuilt baseline gives mean window frames near the
  step 3/4 figures** (about 9451, 1351 and 979 hblanks for `start`,
  `draw-5564`, `draw-6000`, so thresholds about 189, 27 and 20). Check: its
  baseline table. Fallback: the thresholds are recomputed from this step's
  own rebuilt baseline in any case; this hypothesis only sizes the effort.
- **H-6-7 — step 6's additivity check held within 10%, or its gap was
  explained.** Check: its verdict per run. Fallback: if step 6 left a gap
  "to step 7 as an open item", this step addresses it at level 2: the
  `draw_layers` self row and the children-sum gap of each starting node are
  the evidence, and the report closes or re-states the item explicitly.
- **H-6-8 — step 6's section is the last section of the report.** Check:
  the file's section order. Fallback: this step's section goes right after
  "Draw level 1" wherever it is; a conflict in the opening paragraph is
  merged by hand keeping both clauses.
- **H-6-9 — step 6's per-child patches are not in the repository**, so this
  step regenerates nothing of them and builds its own. Check:
  `git ls-files psx/probes`. Fallback: if step 6 committed patches, they are
  left as they are and not refreshed by this step.

### Not decided by a guideline call

Every question was settled from the plan's Rules, the step notes and the
code: the PC profile extended (tooling is committable; the dispatcher's
children are only known at run time), PC used to map and group but never to
close (the plan rejects pre-chosen suspects and PC proportions are not PSX
costs), group bounds (sound by non-negativity), runtime calls as leaves
(step 8 owns the backend), help screen by delay (step 1.3), candidates from
code plus measurements without experiments (step 8 owns the named
experiments). No `guideline` agent was needed.

## Implementation Chunks

1. **Subtree root in the profile binary.** `CALL_PROFILE_CHILD_CAP` to 64;
   the optional root argument, its refusals, the `root draw <symbol>`
   header line; existing readers still parse a default-root profile. Tests
   in `test_call_profile` (more than 16 children under one root) and
   `test_frame_profile` (root `draw_layers` on a short recording lists a
   draw method as a child; unknown and ambiguous roots exit 1; default root
   output unchanged). Commit.
2. **`make frame-profile-subtree` and the `subtree` summary.** The target,
   its Makefile legend entry, the `subtree` mode of
   `tools/frame_profile_check.sh`, tested in `test_frame_profile_check` on
   hand-written profiles and map files. Then `make psx-probe-baseline` and
   the comparison with step 6's baseline. Commit (code; baseline numbers go
   into the report in chunk 3).
3. **Level 2.** Pre-check hypotheses; subtree map of `draw_layers` on the
   three runs; probes of the children of `draw_layers#1` and
   `draw_layers#2` on their runs (individual, groups, delays, fallbacks).
   Write the section's head, the rebuilt baseline, the level-2 rows and the
   candidates of `draw_layers` (explored). Commit the report.
4. **Levels 3 and below, `draw-5564`.** Every node opened on `draw-5564`
   (the heaviest window, where both starting nodes are open), down to
   leaves or under 2%, with candidates. Commit the report.
5. **Levels 3 and below, `draw-6000` and `start`, then verification.** The
   remaining open nodes of the two other runs (reusing `draw-5564`'s
   patches where the node is the same, re-run on the other run), candidates,
   the verification table, the patch list and the hand-off. Commit the
   report.

## Testing Decisions

- A good test checks the tool's observable contract (exit status, the lines
  it writes, the children it lists), never its internals or a recipe's text.
- **Tested:** the profile bookkeeping's raised cap (`tests/test_call_profile.c`,
  prior art: its existing cap test fills exactly `CALL_PROFILE_CHILD_CAP`
  children); the profile binary's root argument end to end
  (`tests/test_frame_profile.c`, prior art: it already runs the built
  binary on `tests/e2e/pad_title.json` and checks usage errors exit 1); the
  `subtree` summary mode (`tests/test_frame_profile_check.c`, prior art: the
  `breakdown` and `coverage` modes' tests on hand-written inputs). All in
  `make frame-profile-test`, outside `make test`, `make test-all` and
  `make e2e`.
- **Default behaviour pinned:** a default-root profile's `frames.txt` and
  `summary.txt` lines (other than the new header line) are unchanged; the
  existing breakdown and coverage tests stay green unmodified.
- **Not tested:** the probes and the report are measurements, not code.
  Their proof is the tool's own verdicts (validity, game state, delay count
  vs PC calls) and the children-sum gaps in the report.
- `make test-all` stays green and is not slower (nothing is added to it);
  no `*.sha256` moves; `psx-check` is not re-run since no file the PSX build
  compiles changes. `psx-tools-test` runs as the probe's prerequisite and
  stays green.

## Out of Scope

- The backend's inside: per-unit cost per family, the 9× print bound by a
  one-print experiment, the meter removed from the measured replay (step 8).
- Any throwaway experiment that implements a candidate to bound its gain.
- Ranking the levers, cumulative estimates, carrying shares onto the full
  run's frames (step 12).
- The update tree, `cart_update60`'s children, update spikes, PC/PSX
  calibration (steps 9–10), though the subtree root may serve them later.
- The update-only short runs.
- Committing probe patches; changing the probe tool, the manifest, the
  game, the engine, the runtime, `psx/` or any e2e file.
- Any full run; anything on the Miyoo or PCSX-ReARMed; audio; launching the
  raylib app.

## Further Notes

- Expected shape (not a target): under `draw_layers#1`, the planet,
  slipway, background and structure-icon methods should carry most of
  `draw-6000` (`spr`, `line`, `rectfill`, `pset` heavy); under
  `draw_layers#2` on `draw-5564`, the tooltip, message-box and UI overlays
  should carry the `print` weight, through `hud_print_outlined`'s nine
  prints per text. On `start`, with a threshold of about 189 hblanks a frame
  and a draw of about 465, at most two children of `draw_layers#1` can be
  above 2%, so its tree is shallow.
- `draw_layers` itself rebuilds fifteen layer lists from the message bucket
  on every call, twice a frame; whatever its self time measures, it is a
  natural "recomputation" candidate (the layer of an entity rarely changes).
- Probe costs are CPU time in `cart_draw` only. A candidate that removes
  primitives also shrinks submit and the meter's work; those overlaps are
  step 12's to name.
- Hypotheses H-6-1 to H-6-9 are the only assumptions about step 6.
