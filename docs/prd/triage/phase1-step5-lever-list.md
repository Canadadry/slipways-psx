---
title: "The lever list for the steady frames over, docs/psx-levers.md"
description: "Phase 3 has no ranked list of pixel-identical levers for the steady situations over 263, and the only lever estimator still in the Makefile reads plan 20's deleted report and tables."
status: needs-triage
---

## Plan step (verbatim)

Plan 22, phase 1, step 5:

> 5. Lever list, `docs/psx-levers.md`: for each function above 2% of the
>    steady frames' mean cost, a lever with its location, estimated gain,
>    cost and risk (pixel identity, game state), ranked by its effect on
>    `real-game-budget` (steady frames over first). Pixel-identical levers
>    in the ranked list; picture-changing ideas in a separate, unranked
>    section kept for the close. The `lever-estimate` targets are re-sourced
>    to this list or removed. Each lever names the short run (step 4) that
>    measures it. For a situation marked "no representative run", a
>    function's share comes from `steady_profile.sh no-run` on the full
>    game's frames over in that situation (no resume involved); its
>    best-try run supplies only the PSX per-call costs. Where a function's
>    no-run share and its best-try share differ by more than a fifth, the
>    note lists it. Verified when the estimated gains, applied to the
>    committed frame table (step 1's until it is replaced), give the steady frames still over at the end of the
>    list, with a pessimistic variant (every gain cut by a fifth).

## Problem Statement

Plan 22 must bring every steady frame of the real game within 263 hblanks.
On the committed frame table (step 1's), `real-game-budget` is red with
**1,133 steady frames over 263** (map-rest 429, map-move 388, build 155,
menu 91, slipway-aim 41, tech-panel 21, score-panel 8; worst frame 4655,
`build`, 530), while the one-off frames use 1,391 of their 10,800 vsyncs.
Phase 3 implements levers "in step 5's order", one at a time (or in groups of
up to three per `psx-full-game-check`), and recomputes a projection after
each. Today:

- **No lever list exists.** Plan 20's report (`docs/psx-perf-levers.md`) was
  deleted at plan 21's close. Its levers were sized on plan 21's draw-cluster
  runs (`draw-5564`, `draw-6000`), against "every frame within 263", before
  23 levers changed the costs. Nothing says which function costs what in the
  steady situations, or which lever would remove which steady frames over.
- **Step 4 delivered the inputs but not the decision.** Its steady short
  runs (representative: `map-move-1057`, `menu-5954`, `tech-panel-5895`;
  kept as best tries but not representative: `map-rest-5581`, `build-5752`,
  `score-panel-5876`; none for `slipway-aim`) and `make steady-profile` say
  where the time goes, per function, not what to change. For the four
  situations without a representative run, only `slipway-aim` has a `no-run`
  profile from the full game's own frames; map-rest, build and score-panel
  have only their best-try runs' profiles, which draw 12–24% less than the
  full game.
- **The only estimator is dead weight.** `make lever-estimate` and `make
  lever-estimate-check` (`tools/lever_estimate.sh`,
  `tools/lever_estimate_check.sh`, `tests/test_lever_estimate.c`,
  `tests/fixtures/lever_estimate/`) read plan 20's gain table
  `psx/perf/gains.txt`, plan 20's judge table
  `psx/perf/report-judge-frames.txt` and plan 21's spike list
  `psx/perf/spike-frames.txt`. They model the strict "every frame" judge and
  spreading, both dropped by plan 22, and their comments cite the deleted
  report. They know nothing of situations, of `in-base` levers, or of step
  7's per-basis scaling.

Without a ranked list and a projection, phase 3 has no order, no estimate
to measure each lever against, and no way to know before starting whether
the pixel-identical levers can reach zero steady frames over.

## Solution

One document, one re-sourced tool, and one small extension of
`steady-profile`.

**`no-run` profiles for every situation without a representative run.**
`make steady-profile` (with `PSX_SHORT_RUNS` unset) also writes a `no-run`
profile for map-rest, build and score-panel, the situations the manifest
marks `# no representative run:`, beside their best-try run's profile.
Slipway-aim keeps the one it has.

**`docs/psx-levers.md`**, written from step 4's profiles:

1. **Inputs**: the profile figures it rests on, copied in (per steady
   situation: its source, run profile or `no-run`, with function and group
   lines, update pieces, submit, `adds-up`), the pooled overhead per call,
   the steady frames' mean cost on the committed frame table, and the 2%
   threshold. Also the known biases: PC hook cost may inflate game code
   (`slipway_draw_method` 45–76 h), and resumed runs draw fewer `pset`.
2. **Functions above 2%**: every draw function, update piece and submit
   whose cost per steady frame is above the threshold, with its
   in-primitives and game-code split per situation where one exists, and
   the functions whose `no-run` and best-try shares differ by more than a
   fifth.
3. **Ranked levers**: for each function above 2%, a pixel-identical lever
   (or "no pixel-identical lever found", with why), each with its location,
   estimated gain per situation and its basis, cost, risk (pixel identity,
   game state), the run that measures it, the run that probes it first when
   it is measured on `psx-full-game-check`, and an evidence flag; in the
   order the projection tool computes.
4. **The lever table**: a fenced block the tool reads.
5. **Projection**: the tool's output, the steady frames still over after
   each lever and at the end of the list, per situation, nominal and
   pessimistic (every gain × 0.8).
6. **Picture-changing ideas**: unranked, each with what it would change on
   screen and a rough gain, kept for the close; nothing here is implemented
   by this plan's pass 1.
7. **Plan 21's levers**: the ones already taken, recorded once; every lever
   plan 21 skipped or did not build re-judged against the new threshold and
   the steady situations; D11 kept, as plan 22 decided.

**`make lever-estimate`**, re-sourced: it reads the lever table from
`docs/psx-levers.md`, applies it to the committed frame table joined with the
situation table, ranks the levers by their effect on `real-game-budget`
(steady frames over first), checks that the document's order equals that
ranking, and prints the steady frames over after each lever and at the end
of the list, nominal and pessimistic, then `real-game-budget`'s own lines
on the projected tables. It already understands what step 7 will write:
`measured` gains, `in-base` levers (kept, no gain applied) and per-basis
scale ratios. `make lever-estimate-check` and every plan 20 input are
removed.

## User Stories

1. As the plan pilot, I want one document listing every function above 2% of the steady frames' mean cost, so that no costly function is left without a lever or an explicit "none found".
2. As the plan pilot, I want the 2% threshold computed from the committed frame table by a stated rule and written in the document, so that "above 2%" is checkable and does not drift between steps.
3. As the plan pilot, I want a function that matters only in one situation (a panel in `build` or `tech-panel`) kept in the list, so that the situations with the worst steady frames are not drowned out by map frames.
4. As the plan pilot, I want each function's cost split into time in primitives and time in the game's own code where the profile gives it, so that each lever is aimed at the part it can remove.
5. As the plan pilot, I want a situation without a representative run to take its function shares from the full game's own frames over (`no-run`), so that a resumed run that draws a fifth less does not skew the list.
6. As the plan pilot, I want such a situation's best-try run used only for PSX per-call costs, so that the `calls` estimates still rest on measured PSX figures.
7. As the plan pilot, I want the functions whose `no-run` and best-try shares differ by more than a fifth listed, so that the resume's bias is visible where it matters.
8. As the plan pilot, I want the profile figures the document rests on copied into it, so that it stays readable after `tmp/` is cleaned.
9. As a phase 3 implementer, I want each lever to name its location (file and function), so that I start from the right code.
10. As a phase 3 implementer, I want each lever's estimated gain in hblanks per frame for each situation it touches, so that I can compare my measured per-situation delta with it.
11. As a phase 3 implementer, I want each lever's estimation basis (`calls` or `share`) in the lever table, so that step 7's measured-to-estimated ratio scales the right estimates.
12. As a phase 3 implementer, I want each lever's cost in a stated scale, so that ties in effect go to the cheaper lever.
13. As a phase 3 implementer, I want each lever's risk on pixel identity and on game state spelled out, so that I know before starting whether a capture reference or an e2e hash is at stake.
14. As a phase 3 implementer, I want each lever to name the run that measures it (a representative short run or `psx-full-game-check`), so that I measure it where its evidence is valid.
15. As a phase 3 implementer, I want a lever measured on `psx-full-game-check` to also name its probe run, a run that draws its target function, so that step 7's probe-first rule needs no lookup.
16. As a phase 3 implementer, I want a lever whose effect comes mostly from situations without a representative run flagged as weaker evidence, so that a probe on a non-representative run is not taken for proof.
17. As the plan pilot, I want the levers ranked by the steady frames over they remove, then by steady hblanks saved, so that phase 3 implements first what turns frames green.
18. As the plan pilot, I want the ranking computed greedily on the levers already above in the list, so that two levers that each fall just short together still rank by what they achieve in sequence.
19. As the plan pilot, I want a lever whose gain is stated on what another leaves held after it, so that overlapping levers are never counted twice.
20. As the plan pilot, I want the document's order checked by the tool against the computed ranking, so that the list and the projection never disagree.
21. As the plan pilot, I want the projection to apply the estimated gains to the committed frame table frame by frame, so that the "steady frames still over" figure comes from the frames the judge counts.
22. As the plan pilot, I want the steady frames still over after each lever and at the end of the list, per situation, so that phase 3 sees where the projection reaches zero and the close knows which situations need the picture-changing options.
23. As the plan pilot, I want a pessimistic projection with every gain cut by a fifth, so that the verdict holds against plan 21's record of estimates falling short.
24. As the plan pilot, I want the projection to print `real-game-budget`'s own lines on the projected tables, so that "effect on real-game-budget" means exactly what the judge computes.
25. As a phase 3 implementer, I want measured gains to replace estimates in the same table, so that step 7's update after each lever is one edit and one `make lever-estimate`.
26. As a phase 3 implementer, I want a lever already in the measured code marked `in-base` and applied as no gain, so that after a new full-game table the projection is rebased without counting a lever twice.
27. As a phase 3 implementer, I want a per-basis scale ratio in the table applied to the remaining estimated gains of that basis, so that step 7's ratio scaling is one line, not a hand edit of every percent.
28. As the human, I want picture-changing ideas listed apart and unranked, with what they would change on screen, so that I choose them later and no agent decides that a visible change is acceptable.
29. As the plan pilot, I want plan 21's skipped and unbuilt levers re-judged on the steady situations and the new threshold, so that a lever skipped under the old 10.6 threshold is not lost now that the threshold is lower.
30. As the plan pilot, I want plan 21's levers already in the code recorded as taken, so that no one proposes them again.
31. As a developer, I want the `lever-estimate` target kept under its name but sourced from the new list, so that the plan's wording and the Makefile agree.
32. As a developer, I want `lever-estimate-check`, its script, its fixtures and plan 20's tables removed, so that nothing in the repository cites a deleted report.
33. As a developer, I want the re-sourced tool tested on hand-written tables, so that its join, composition, ranking, `in-base`, scaling and haircut are proven without a PSX boot.
34. As a developer, I want none of this in `make test` or `make e2e`, so that the fast loop stays fast.
35. As a reviewer, I want every figure the step note quotes to come from `make lever-estimate` or `make steady-profile` output, so that the verification is reproducible.
36. As a phase 3 implementer, I want one-off frames left unchanged by the projection, so that it never claims a one-off gain no profile supports.
37. As step 6's implementer, I want the lever table keyed by situation, not by frame number, so that step 6's renumbering of `full_game.json` does not touch it.

## Implementation Decisions

### Where each situation's figures come from

- **Representative run** (map-move, menu, tech-panel): the run's `make
  steady-profile` summary, `profile` mode, used as it is. Its window was
  checked within ±10% of the full game's on the same frames. Function,
  group, update and submit lines are hblanks per window frame on the judge.
- **No representative run** (map-rest, build, score-panel, slipway-aim): the
  function, group, update and submit figures come from `steady_profile.sh
  no-run` on the full game's frames over 263 in that situation. That
  profile has PC-time shares only, with no primitives/game-code split. The
  best-try run (`map-rest-5581`, `build-5752`, `score-panel-5876`; none for
  slipway-aim) supplies only two things:
  - the PSX per-call cost of each family, for `calls` estimates: the
    family's in-primitives hblanks ÷ its calls in the best-try's breakdown
    boot over the window, overhead removed as `steady-profile` does;
  - where a lever needs it, the function's primitives/game-code ratio,
    applied to its `no-run` total and labelled `split from <best-try>`.
    Slipway-aim, with no best try, has no split. A `calls` estimate there
    uses the pooled per-call cost of the representative runs for that
    family and is labelled so.
- **Share comparison** (plan): for every function above 2% in a
  no-representative situation, its share of the window total in the
  best-try profile against its share in the `no-run` profile. A difference
  above a fifth of the `no-run` share (|best − no-run| > no-run ÷ 5) is
  listed in the note and in the document's Inputs.
- **`steady-profile` extension** (chunk 1): with `PSX_SHORT_RUNS` unset, the
  target writes a `no-run` profile to `tmp/frame-profile/steady/<situation>/`
  for every steady situation that has no run in the manifest (as today) or
  that the manifest marks `# no representative run:` (new). The best-try
  run's profile stays in `tmp/frame-profile/steady/<run>/`. The full-game
  profiles are computed once and shared, as today. `tools/steady_profile.sh`
  is not changed; the only change is in the target's loop. Its legend line
  and `docs/psx-game.md` § PC frame profile say so.

### The steady frames' mean cost and the 2% threshold

- **Mean cost M**: the mean `total` (update + draw + submit) over every
  `steady` frame of the committed frame table
  (`psx/perf/full-run-frames.txt`, step 1's until it is replaced) joined with
  `psx/perf/full-run-situations.txt`, `title` included. Threshold
  **T = 0.02 × M**, in hblanks, one decimal. This is the literal reading of
  "the steady frames' mean cost". The tool computes it once (a `mean-cost`
  line) and the document quotes it.
- **A function's cost per steady frame**: for each steady situation with
  frames over 263, the function's hblanks per frame in that situation's
  source above (run window frames or `no-run` frames over), weighted by the
  situation's frame count in the committed table, divided by the steady
  frame count. `title` (no frame over, no profile) counts as zero for every
  function. The sources describe heavy frames (windows ranked by frames
  over, or frames over themselves), so this overstates a function's cost on
  the average steady frame. That bias is accepted: the list is about frames
  over.
- **Two inclusion rules**, the union listed: (a) cost per steady frame above
  T; (b) cost in one steady situation with frames over above 2% of that
  situation's measured total per frame in its source. A function in by rule
  (b) only is marked `situation-only (<situation>)`. Rule (b) exists because
  build, menu, tech-panel and score-panel hold the worst steady frames but
  about a sixth of the steady frames: weighting alone would hide their
  panels.
- "Function" means a line of a step 4 profile: a draw function (`function
  <symbol>` line, its group named), an update piece (`update <piece>`),
  `submit`, and the `draw_layers` and `cart_draw` selfs plus their `frame`
  group helpers as they appear.

### Levers: content and estimation

- Lever ids: plan 20's ids for levers that came from its report (D3, D12,
  D13, B2, G4, L3, …), `N1`, `N2`, … for new ones, in the order found.
- Each ranked lever entry gives:
  - **Function** it targets (a function above 2%) and its group;
  - **Location**: file and function where the change goes (no line
    numbers);
  - **Change**: what the lever does, in one or two sentences;
  - **Estimated gain**: hblanks per frame and % of the part, per situation it
    touches, with its basis;
  - **Cost**: S (under 50 changed lines, one module), M (50–300 lines or two
    to three modules), L (above 300 lines or a new runtime/backend entry);
  - **Risk, pixel identity**: `none` (same primitives, same order), `capture
    reference` (raw VRAM may change though the picture does not, e.g. the PSX
    mask bit; acceptable only under `docs/psx-game.md` § Acceptance's agent
    rule, every capture of the label at `differ 0`), or `capture` (the
    picture itself may move; a lever that cannot be made pixel-identical is a
    picture-changing idea, not a ranked lever);
  - **Risk, game state**: `none` (draw-only) or `e2e decides` (update code;
    any e2e hash change reverts it);
  - **Measured by** and **probe run** (rules below);
  - **Evidence**: `representative` or `weaker` (rule below);
  - **Overlaps**: the lever it must follow (`after <id>`), if any.
- **Estimation basis**, exactly one of the two step 7 scales:
  - `calls`: calls removed or merged × the family's PSX cost per call (the
    run's own, or the best-try's for a no-representative situation, as
    above). For levers that remove or batch primitives.
  - `share`: the fraction of the function's time the change removes, located
    with `make frame-profile-subtree ROOT=<symbol>
    PSX_SHORT_RUNS=<run>` (`PART=update` for update pieces) on the
    situation's run, or on its best-try run for a no-representative
    situation, applied to the function's figure from the situation's source.
    For levers that make the game's own code cheaper.
  A plan 21 leftover is estimated on one of these two bases from today's
  profile. Its plan 21 figure is quoted for comparison only.
- **Known bias**: `steady_profile.sh` gives game code as whole child time,
  which may include PC hook cost on call-heavy code (`slipway_draw_method`
  45–76 h). A `share` estimate on such a function says so, and its
  pessimistic figure is the honest one to quote.
- No PSX probe and no `psx-full-game-check` in this step: phase 3 measures
  levers. Host tools (`frame-profile`, `frame-profile-subtree`,
  `steady-profile`) are allowed. So is `make psx-short-runs` on the six steady
  runs, only to regenerate the breakdown and plain boots `steady-profile`
  needs if they are missing from `tmp/`.
- **Coverage**: before adding a lever, check whether a lever above it in the
  list, or one already in the code, removes the same cost. When two levers
  hit the same cost, the second's gain is stated on what the first leaves,
  and it carries `after <first>`. Of two alternatives (only one is to be
  built), only the one with the larger effect is listed; its entry names the
  other.
- **No lever for a function**: an entry "no pixel-identical lever found" with
  the reason. The function's cost stays unchanged in the projection. Any
  picture-changing idea for it goes to the separate section.
- **Measured by** (the plan: "each lever names the short run that measures
  it"): among the situations the lever touches that have a representative
  run, the one where the lever alone removes the most steady frames over
  names its run. If it touches none of them, `psx-full-game-check`.
- **Probe run** (step 7: a lever measured on `psx-full-game-check` is first
  probed on a run that draws its target function, a representative run of
  another situation if one does, else its best-try run): a representative
  run whose profile gives the target function at least 1% of the window
  total, the one with the largest share. Failing that, the best-try run of
  the situation where the lever removes the most steady frames over. For
  slipway-aim alone, with no best try, a representative run must be found;
  if none draws the function, the entry says `probe none` and step 7 goes
  straight to the full-game check. A lever measured on a short run has no
  separate probe run.
- **Evidence `weaker`** when more than half of the lever's estimated steady
  frames over removed (nominal, the lever alone on the committed table) come
  from map-rest, build, score-panel or slipway-aim.

### Plan 21's levers

- **Taken, recorded once in a short list, not proposed again**: F1, G1, B1,
  G2, B3, D5, D1/B4, L1, D8 (in the code since plan 21's squash merge), D6,
  D2, D10, D9, D11, D15, O2, O3, O5, U4, U6, U2, U1, U3.
- **D11 stays**, as plan 22 decided; no entry beyond the taken list.
- **Re-judged** on the steady profiles and the new threshold. Each becomes
  a ranked lever, a picture-changing idea, or "dropped" with the figure that
  drops it:
  - D3 (message-box text through the text image cache; skipped in plan 21
    for want of a capture; `message_box_draw_overlay_method` is 14% of
    map-rest's best-try window);
  - D4 (covered by D1/B4 in plan 21; its residual re-checked against T);
  - D12 (sprite dots; skipped because the PSX mask bit changes capture
    references, which the agent acceptance rule now allows at `differ 0`);
  - D13, L3 and G4 (under plan 21's 10.6 threshold), re-judged against T;
  - B2 (not built), its ceiling after D11 and D15;
  - O1 and L2 (covered in plan 21), dropped unless the steady profiles show
    their target above T.
- Step 4's open finding (resumed runs draw fewer `pset`, cause unproven) is
  not a lever. The document records it under Inputs as a known bias of the
  best-try runs.

### The lever table (fenced block in `docs/psx-levers.md`)

- One fenced code block with info string `lever-table`, the only machine
  input. Everything else in the document is prose for people. Lines are
  whitespace-separated; `#` comments to the end of the line:

  ```
  lever <id> <estimated|measured|in-base> <S|M|L> basis <calls|share> measure <run|psx-full-game-check> [probe <run|none>] [after <id>]
  gain <id> <situation> <update|draw|submit> <percent>
  scale <calls|share> <ratio>
  ```

  - `percent` is the gain as a percentage of that part on each frame of that
    situation, one decimal: the estimate in hblanks per frame divided by the
    situation's source figure for that part (run window, or `no-run` frames
    over). A lever has one `gain` line per steady situation it touches. Only
    steady situations may appear; a one-off situation or an unknown name is
    malformed.
  - `measured`: the percent is step 7's measured delta. `in-base`: the lever
    is in the code the committed frame table was measured on. Its `gain`
    lines stay for the record, but no gain is applied and it is not ranked.
  - `scale` (at most one line per basis, ratio > 0, default 1): multiplies
    every `estimated` gain of that basis. It never touches `measured` or
    `in-base` gains. This step writes no `scale` line; step 7 adds them.
  - `probe` is required with `measure psx-full-game-check` and forbidden
    otherwise.
- The order of the `lever` lines that are not `in-base` is the document's
  ranked order. `in-base` lines may sit anywhere; they are kept for the
  record.
- Keyed by situation, never by frame number, so step 6's renumbering leaves
  it alone.

### The tool (`tools/lever_estimate.sh`, rewritten; target `lever-estimate`)

- Interface: `lever_estimate.sh <frame-table> <situation-table> <levers-doc>
  <budget-hblanks> <one-off-allowance-vsyncs> [haircut=<pct>]
  [projected=<dir>]`. It joins the tables with `situation_report.sh`'s rules
  (same header, same numbering, fails on a mismatch) and extracts the
  `lever-table` block from the document (fails when there is none or two).
- **Composition** per frame of a steady situation s and per part p: part' =
  round-half-up(part × (1 − min(100, Σ g′ × (1 − haircut/100)) / 100)), where
  the sum runs over the applied levers' gains for (s, p), and g′ is the gain
  times its basis's `scale` if `estimated`, as is if `measured`. `in-base`
  levers are never applied. total' = update' + draw' + submit'. One-off
  frames and `title` frames are unchanged. Gains add within a part because
  each is stated on the committed table's figures, or on what its `after`
  lever leaves.
- **Ranking**: greedy over the levers that are not `in-base`. Starting from
  none, repeatedly pick, among the levers whose `after` lever is already
  picked (or is `in-base`), the one that leaves the fewest steady frames over
  the budget once added. Ties go to the most steady hblanks saved on the
  frames over 263 in the committed table, then cheaper cost (S < M < L),
  then id. The ranking is computed with haircut 0, scales applied.
- **Output**, in order:
  - `mean-cost steady <M> threshold <T>`;
  - `situation-threshold <situation> mean <m> threshold <t>` per steady
    situation with frames over (mean over its frames in the committed table,
    for reference; rule (b) uses the profile source's total);
  - `start steady-over <n> one-off-vsyncs <v>` on the committed table, and
    `in-base <id>...` (or `in-base none`);
  - per lever in computed rank order: `lever <rank> <id> <status> basis <b>
    steady-over <n> pessimistic <n'> saved <h>` (after this lever and every
    one above it);
  - `situation <name> over <before> end <n> pessimistic <n'>` per steady
    situation;
  - `order ok`, or `order differs rank <r> document <id> computed <id>` at
    the first difference;
  - `end steady-over <n> pessimistic <n'>`;
  - then `tools/psx_real_game_budget.sh`'s three lines on the nominal
    projected table, then on the pessimistic one, prefixed `nominal:` and
    `pessimistic:`.
  The projected tables are written to `projected=` (default: a
  `lever-estimate` directory beside the output) in the frame table's
  format. Their `vsyncs` column is `ceil(total / budget)` for a frame over
  the budget and 0 otherwise: a projected value, never a measured one.
- Exit 0 once written whatever the projection says. Exit 3 when the order
  differs, so the target fails on a stale document. Exit 2 on a malformed
  table or block, naming the file and line. Exit 1 on usage.
- `haircut=20` is the plan's pessimistic variant ("every gain cut by a
  fifth": gain × 0.8). The tool always prints both; `haircut=` sets the
  pessimistic one (default 20).
- `make lever-estimate [FRAMES=] [SITUATIONS=] [LEVERS=]` (defaults
  `psx/perf/full-run-frames.txt`, `psx/perf/full-run-situations.txt`,
  `docs/psx-levers.md`) runs it with 263 and 10,800, writes
  `tmp/lever-estimate/estimate.txt` and the projected tables, prints the
  output, and fails on a non-zero exit status. It never boots and is
  outside `make test-all`. Its Makefile legend replaces the old one.

### Removed

- `make lever-estimate-check`, `tools/lever_estimate_check.sh`,
  `tests/fixtures/lever_estimate/`, the old `tools/lever_estimate.sh`
  (replaced) and the old `tests/test_lever_estimate.c` (replaced).
- `psx/perf/gains.txt`, `psx/perf/report-judge-frames.txt` and
  `psx/perf/spike-frames.txt`, which only those targets read. A grep checks
  this before deleting; a reader found elsewhere keeps its file, and the note
  says so. `psx/perf/image-build-frames.txt` is not touched (no lever target
  reads it).
- The `ESTIMATE_*` Makefile variables other than those the new target uses.
- Plan 21's warn-review warning on the 3,000-line Makefile is not acted on:
  only the lever-estimate and steady-profile lines are touched.

### Document lifetime and docs

- `docs/psx-levers.md` is a "still to do" document: phase 3 updates it and
  step 9 deletes it. It opens with one line saying so.
- `docs/psx-game.md`: one paragraph under the PC frame profile naming `make
  lever-estimate`, its inputs and the `lever-table` statuses; the
  `steady-profile` paragraph says no-representative situations get a
  `no-run` profile too; "Still to do" gains a line pointing to
  `docs/psx-levers.md` for the steady frames' levers. No other section
  changes.

### Step note

- The note gives: step 4's outcome as used (the status list below); M and
  T; the functions above 2% (count, and which are in by rule (b) only); the
  functions whose `no-run` and best-try shares differ by more than a fifth;
  the ranked lever ids with nominal gains, basis, measuring and probe runs;
  `end steady-over`, nominal and pessimistic, per situation; the `nominal:`
  and `pessimistic:` `real-game-budget` lines; the picture-changing ideas
  by name; the files removed. If the nominal end is above zero, it says so
  plainly: phase 3 still runs the whole list, and the close lists the
  picture-changing options.

## Implementation Chunks

1. **Tooling** (host only): rewrite `tools/lever_estimate.sh` and
   `tests/test_lever_estimate.c` (TDD on fixture tables and a fixture
   document); the new `lever-estimate` target and legend; remove
   `lever-estimate-check`, its script, its fixtures, and plan 20's three
   tables after the grep; the `steady-profile` loop change for
   no-representative situations, with its legend and doc lines.
2. **The list**: run `make steady-profile` (after `make psx-short-runs` on
   the six steady runs if their boots are missing) to get the six run
   profiles and the four `no-run` profiles; compute M, T and every
   function's cost; compare the `no-run` and best-try shares; locate levers
   with `frame-profile-subtree`; re-judge plan 21's leftovers; write
   `docs/psx-levers.md` with its `lever-table` block; reorder until `make
   lever-estimate` prints `order ok`; copy the projection into the document;
   the `docs/psx-game.md` lines; the note.

## Testing Decisions

- A good test drives the script from outside: it writes a frame table, a
  situation table and a small markdown document holding a `lever-table`
  block, runs the script, and checks the printed lines, the projected
  tables and the exit status. It never inspects how the join or the ranking
  is coded.
- **`tests/test_lever_estimate.c`** (rewritten, stays in
  `frame-profile-test`, outside `make test-all`). Cases:
  - mean cost and threshold over steady frames only, `title` included,
    one-off excluded;
  - one lever on one situation's draw: a frame just over the budget goes
    under, another stays over; rounding half up;
  - gains on one part summed and capped at 100%;
  - one-off frames never change, even with a lever on every steady
    situation;
  - haircut 20 multiplies every gain by 0.8 (a frame that fits nominally and
    not pessimistically);
  - `in-base`: its gains are not applied, it is not ranked, it is listed on
    the `in-base` line, and it satisfies another lever's `after`;
  - `scale calls 0.5` halves `estimated` `calls` gains only: not `share`,
    not `measured`, not `in-base`;
  - greedy ranking: a lever that removes no frame alone but finishes another
    lever's frames ranks by its effect in sequence; ties by steady hblanks
    saved, then cost, then id;
  - `after` holds a lever behind another even when it would rank first;
  - `order ok`; `order differs` with exit 3 naming the first rank;
  - the projected tables pass `tools/psx_real_game_budget.sh`, and its three
    lines appear, prefixed;
  - malformed: no block, two blocks, a one-off or unknown situation in a
    `gain` line, a `gain` for an undeclared lever, an `after` cycle or
    unknown id, a percent above 100, `measure psx-full-game-check` without
    `probe` or `probe` with a short run, a second `scale` for one basis, a
    non-positive ratio, a basis other than `calls`/`share`; a table join
    mismatch.
  Prior art: `tests/test_psx_real_game_budget.c`,
  `tests/test_situation_report.c`, `tests/test_steady_windows.c` (fixture
  tables written by the test, the script run with `system`).
- The `steady-profile` loop change has no unit test: `tools/steady_profile.sh`
  is unchanged and already covered by `tests/test_steady_profile.c`. The
  target's output (four `no-run` summaries) is checked in the note.
- The document itself is not unit-tested: `make lever-estimate` printing
  `order ok` on it is its check, quoted in the note.
- Nothing in `make test` or `make e2e`.

## Out of Scope

- Implementing, probing or measuring any lever (phase 3).
- Running `psx-full-game-check` or `psx-probe`.
- Changing step 4's runs, manifest, `tools/steady_profile.sh`, its profile
  rule or `psx/perf/draw-functions.txt`, beyond the target loop above.
  Changing the situation rule or tables, or `real-game-budget`.
- Writing `scale` lines, `measured` gains or `in-base` statuses: the format
  and the tool support them; step 7 writes them.
- The loading screen (step 6) and any picture or e2e hash change.
- Levers for one-off situations: one-off frames already fit the 5%; the
  projection leaves them unchanged.
- Spreading work over frames.
- Acting on plan 21's warn-review warnings beyond the lines this step
  touches.

## Step 4's outcome — hypothesis status

This PRD was first written while step 4's chunk 4 was in progress, on
hypotheses H-4-1 to H-4-9. Step 4 is now committed (`868ba85`, note
`### Step 1.4` in `8633843`). Each hypothesis was checked against those
commits:

- **H-4-1 — held, extended.** `psx/perf/draw-functions.txt`,
  `tools/steady_profile.sh` and `make steady-profile` landed, writing
  `tmp/frame-profile/steady/<run>/summary.txt`. The script has three modes
  (`overhead`, `profile`, `no-run`), and the pooled overhead goes to
  `tmp/frame-profile/steady/overhead.txt`: 0.0558 h per call over six runs
  (plan 21: about 0.13).
- **H-4-2 — held for `profile`, replaced for `no-run`.** `profile` mode
  prints `window …`, `group <g> in-primitives game-code total share`,
  `function <s> group <g> in-primitives game-code share`, `update <piece> <h>
  share` (pieces seen: `ui_update`, `input_step_run`, `update_entities`,
  `save_at_frame_end`, `update other`), `submit`, and `adds-up ok|failed sum
  measured residual noise-floor`; a run the manifest marks is flagged
  `flag no-representative-run <situation>`. `no-run` mode prints `flag
  no-run`, `no-run <situation> frames <n> budget <b> shares pc-time`, then
  `group`/`function` lines with `total` and `share` only (no
  primitives/game-code split), the same `update` and `submit` lines, and
  `adds-up n/a no-run sum measured`. Game code is the whole child time and
  may be inflated by PC hook cost (`slipway_draw_method` 45–76 h). The
  decisions above follow these formats.
- **H-4-3 — replaced.** The six runs are as guessed (`map-rest-5581`,
  `map-move-1057`, `build-5752`, `menu-5954`, `tech-panel-5895`,
  `score-panel-5876`). Slipway-aim has a `no-run` profile (PC-time shares of
  its 41 full-game frames over: slipways 18.6%, map 14.4%, panels 13.9%,
  `ui_update` 17.3%). The map-move stand-in for slipway-aim is dropped. The
  revised plan text (P1) extends `no-run` to map-rest, build and score-panel,
  hence the `steady-profile` loop change.
- **H-4-4 — held.** Every profiled run is `adds-up ok` within its noise
  floor (residuals −2.2 to +1.8 h a window frame). The `weaker` flag no
  longer needs an `adds-up failed` trigger and was dropped from the rule.
- **H-4-5 — held.** Between step 1's table commit and step 4's last commit,
  `src/` changed only in `devtools/` and `cmd/` (situations, call profile,
  frame profile, pad_author). `psx/perf/full-run-frames.txt` is unchanged and
  `full-run-situations.txt` is step 2's. The committed frame table is still
  step 1's.
- **H-4-6 — held.** Representative runs: `map-move-1057` (draw −0.5%),
  `menu-5954` (+4.0%), `tech-panel-5895` (+0.8%). `# no representative run:`
  marks: map-rest (best `map-rest-5581`, −15.6%), slipway-aim (no run), build
  (best `build-5752`, −23.6%), score-panel (best `score-panel-5876`, −11.6%).
- **H-4-7 — replaced.** The note records the pooled overhead, the biggest
  groups per run with their sums against the measured totals, and
  slipway-aim's main `no-run` shares, but not every function line.
  Function-level figures live only in `tmp/`, so chunk 2 regenerates them
  with `make steady-profile`, after `make psx-short-runs` if the boots are
  missing, and copies them into the document.
- **H-4-8 — held.** Step 4 only added `steady-profile` to a `.PHONY` line
  near `frame-profile-subtree`. The target still takes `PSX_SHORT_RUNS` and
  `PART=draw|update`.
- **H-4-9 — held.** No lever-estimate file, fixture or plan 20 table changed
  in step 4.

## Further Notes

- Plan 21's record argues for the pessimistic variant: B3 delivered under a
  quarter of its estimate, D5 about 0.15 of it, D11 and D15 a third to an
  eighth of their probe ceilings. Estimates here use today's per-call costs
  from step 4's breakdown boots (pooled overhead 0.0558 h a call), not plan
  20's, which should narrow the gap. Step 7's per-basis `scale` corrects the
  rest as levers are measured.
- Step 4's biggest groups point where the list is likely to land: slipways
  lead map-rest (24.9%), menu (31.9%), tech-panel, build and score-panel;
  planets come second almost everywhere; map-move is led by the map (31.1%,
  `wormhole` 94.1 h in primitives) and by update (`update_entities` 55.8,
  `save_at_frame_end` 28.0); build carries `ui_update` 34.0; score-panel
  carries `input_step_run` 43.1 and `update_entities` 37.5. These are
  pointers, not decisions: chunk 2 works from the regenerated figures.
- The map situations hold 817 of the 1,133 steady frames over, with mean
  totals of about 210 against 263. Most of their frames over are a few tens
  of hblanks above the budget, so modest per-frame gains turn many of them
  green. Build's frames over are fewer but far worse (worst 530), so `build`
  may stay over at the end of the list; the close then names it.
- Decisions taken without a guideline agent: the literal M over all steady
  frames, with the per-situation rule (b) added; representative runs used as
  they are, no-representative situations from `no-run`, with best-try
  per-call costs and splits; the "more than a fifth" comparison relative to
  the `no-run` share; the `steady-profile` loop change; greedy ranking with
  an `after` constraint; additive composition; one-off frames unchanged; the
  S/M/L cost scale; the `in-base`, `measured` and `scale` table support from
  the start; the probe-run rule; keeping the `lever-estimate` name with a
  new source and removing `lever-estimate-check` and plan 20's tables; no
  probe in this step.
