---
title: "Plan 22 close: the steady frames still over, the picture-changing options, the levers document retired"
description: "real-game-budget is still red after every pixel-identical lever, so the human needs the steady situations still over and the picture-changing options re-estimated on today's code, and the docs must stop describing finished work."
status: needs-triage
---

## The step, verbatim (plan 22, phase 4, step 9, as committed at HEAD)

> 9. Close the plan: write the result. If `real-game-budget` is green, say
>    so. If not, list the steady situations still over, with their frames,
>    worst and mean, what bounds them, and the picture-changing options
>    from step 5's separate section with their gains, re-estimated on step
>    8's frame table and on profiles taken at step 9's HEAD: `make
>    steady-profile` (its no-run shares then cover step 8's frames over,
>    its runs give the post-lever per-call costs) run before any estimate,
>    the note giving each option's old and new per-frame saving; for each
>    option alone, the steady frames still over (nominal and pessimistic,
>    per situation) from `lever_estimate.sh` run on a lever-table file in
>    `tmp/` (`LEVERS=`) for an option whose saving is proportional to the
>    frame's draw or update; an option whose saving is a fixed amount per
>    frame (P1) or falls only on some frames (P4: the frames that draw a
>    wormhole, from the PC replay's per-frame call counts as N3 did) is
>    applied per frame to a copy of step 8's table in `tmp/` and counted
>    with `make situation-report`; "Still to do" says which way each option
>    was computed and quotes phase 3's projected-against-measured figures
>    (361 / 516 against 198, per situation) as the estimates' known error;
>    options kept unranked, in the section's order; for the human to
>    choose from; put that list in `docs/psx-game.md`'s "Still to do".
>    Update `docs/psx-game.md`; delete `docs/psx-levers.md`. The
>    `lever-estimate` target, its script and its doc lines are re-sourced
>    or removed in the same step, as step 5 did for plan 21's (no target or
>    doc may name a deleted file). Every warn-review warning of plan 22
>    (phase 1's, phase 2's, phase 3's) is acted on only in code a step of
>    this plan already touches (as for plan 21's). Those still open at the
>    close are listed in `docs/psx-game.md`'s "Still to do", together with
>    the open findings that step notes recorded but no doc carries yet
>    (found by reading every step note for findings left open or handed to
>    step 9 — at least step 2.6's red `make worst-case-peaks` and step
>    3.8's D3 cold-cache rise). The step's note lists every warning of
>    phases 1–3 and every such finding on one line each, with where it
>    ends: the step note that acted on it, or its "Still to do" line.
>    Order: warning fixes in code first; if any changes `src/game`,
>    `src/p8` or the PSX backend, `psx-full-game-check` is rerun and its
>    table replaces step 8's for everything below; then the option
>    estimates; then the `lever-estimate` re-source or removal. Verified
>    when `make test-all` and
>    `psx-check` are green, `grep -rn psx-levers Makefile tools docs src
>    tests` finds nothing outside plan files, every line of that list
>    points to a step note or a "Still to do" entry that exists, `make
>    frame-profile-test` is green, and the docs describe only what is
>    true.

This PRD covers the whole of step 9. Every phase-3 lever is in the code
and in the measured base (step 3.8): **198 steady frames over 263**
(map-rest 60, map-move 43, build 38, menu 23, slipway-aim 14, tech-panel
13, score-panel 7), worst steady frame 4657 (`build`, 478), one-off 293
frames over for **674 of 10,800 vsyncs**. `real-game-budget` is **red**,
so the close takes the "if not" branch. `make frame-profile-test` is
green at HEAD (fixed in its own commit, `986b5d2`).

## Problem Statement

Plan 22's pass 1 is finished and did not reach its goal: the player of a
real game would still see 198 steady frames of `full_game.json` miss
60 fps. The plan's rule is that pass 1 stops here and **the human chooses**
which picture-changing options to try, in a later grilling. Today the
human cannot choose well:

- the only list of options (P1–P5) sits in `docs/psx-levers.md`, with
  savings estimated on profiles taken **before** any lever, against a
  frame table that no longer exists (step 6's);
- nobody has said, per steady situation still over, how far over its
  frames are and whether draw or update holds them there — an option
  that saves draw cannot clear a frame held by an update spike;
- `docs/psx-levers.md` describes finished work (every lever taken or
  skipped), which CLAUDE.md § Docs forbids, yet `make lever-estimate`,
  its script and `docs/psx-game.md` still name it as their default input;
- forty-odd warn-review warnings and a few open findings (a red
  `make worst-case-peaks`, D3's cold-cache rise) live only in step notes
  of a plan file that is deleted once the plan is done.

## Solution

One closing pass, in the order the step sets:

1. **Warning fixes in code: none.** Every code warning is listed, not
   fixed (decision below); the one doc-only warning is fixed with the
   doc rewrite. So no full-game rerun: step 8's committed table stays the
   base for everything below.
2. **Fresh profiles** at HEAD (`make psx-short-runs` boots of the six
   steady runs, then `make steady-profile`), **the steady situations still
   over** computed from step 8's table (frames, worst, mean, what bounds
   them), and **each picture-changing option re-estimated** alone, nominal
   and pessimistic, per situation, the proportional ones through
   `lever_estimate.sh` on a lever-table file in `tmp/`, P1 and P4 per frame
   on a copy of step 8's table counted by `make situation-report`.
3. **`docs/psx-game.md`'s "Still to do"** rewritten to say what is true at
   the close: the result, the situations still over, the options for the
   human (unranked, in P1–P5 order, each with how it was computed and the
   estimates' known error), every open warn-review warning and every open
   finding.
4. **`make lever-estimate` re-sourced**: it keeps working, but `LEVERS=`
   becomes required (no default document), and every doc line that named
   the levers document is rewritten; then **`docs/psx-levers.md` is
   deleted**.
5. **The ledger**: one line per warning of phases 1–3 and per open
   finding, with where it ends, for plan-pilot's step note; then the final
   checks.

## User Stories

1. As the human, I want to read in `docs/psx-game.md` whether `real-game-budget` is green, so that I know at once whether pass 1 reached the goal.
2. As the human, I want the steady situations still over listed with their frames over, worst frame and mean total, so that I see how much is left and where.
3. As the human, I want each situation's frames over split into draw-bound and update-bound, with their mean update, draw and submit, so that I know which options can help that situation at all.
4. As the human, I want to be told plainly that a draw-only option cannot clear an update-bound frame, so that I do not choose an option for a situation it cannot fix.
5. As the human, I want every picture-changing option (P1–P5) described in one or two sentences of what changes on screen, so that I can judge its visual cost without reading code.
6. As the human, I want each option's per-frame saving re-estimated on today's code, next to the old estimate, so that I see how the levers changed what an option is worth.
7. As the human, I want, for each option alone, the steady frames still over per situation, nominal and pessimistic, so that I can compare options on the plan's own measure.
8. As the human, I want each option's estimate to say how it was computed (proportional through the lever table, or applied per frame), so that I know how much to trust it.
9. As the human, I want the estimates' known error quoted (phase 3's projection 361 nominal / 516 pessimistic against 198 measured, per situation), so that I read every option figure as an estimate, not a promise.
10. As the human, I want the options kept unranked, in their P1–P5 order, so that no agent has implicitly chosen for me.
11. As the human, I want the wormhole-ring option (P4) given for both "three rings" and "two rings", so that I can pick the variant, not just the option.
12. As the human, I want every open warn-review warning of plan 22 listed in "Still to do", so that nothing the reviews found is lost when the plan file goes.
13. As the human, I want the open findings (red `make worst-case-peaks`, D3's cold-cache rise on f4887 and f5275, the resumed runs drawing less) listed in "Still to do", so that I can decide on them later.
14. As the human, I want `make worst-case-peaks`'s re-baseline left to me, as the step note said, so that a frozen reference is never moved by an agent.
15. As a developer, I want no doc that describes finished work, so that the docs stay true (CLAUDE.md § Docs).
16. As a developer, I want `docs/psx-levers.md` gone and no target, script or doc naming it, so that nothing points at a missing file.
17. As a developer, I want `make lever-estimate` to keep working on a lever table I pass, so that pass 2 can project the options the human picks.
18. As a developer, I want `make lever-estimate` without `LEVERS=` to fail with a message saying `LEVERS` is required, so that a missing default never reads a deleted file silently.
19. As a developer, I want the lever-table format still documented in `docs/psx-game.md`, so that I can write a table without the deleted document.
20. As a developer, I want `make frame-profile-test` green, so that the profile and estimate tools stay trustworthy.
21. As a developer, I want `make test-all` and `psx-check` green at the close, so that the plan ends on a sound tree.
22. As a developer, I want `make test` and `make e2e` no slower, so that the fast loop stays fast (nothing here touches them).
23. As a developer, I want every e2e `.sha256` and `.end-state` unchanged, so that the close changes no behaviour.
24. As the plan pilot, I want the profiles re-taken at HEAD before any estimate, so that the option estimates rest on post-lever costs, as the amended step requires.
25. As the plan pilot, I want the no-run shares to cover step 8's frames over, so that the four situations with no representative run are estimated on the frames that matter.
26. As the plan pilot, I want the old and new per-frame saving of each option in the note, so that the step's verification can check it.
27. As the plan pilot, I want one ledger line per warning of phases 1–3 and per open finding, each pointing to a step note or a "Still to do" entry that exists, so that the step's verification can check it.
28. As the plan pilot, I want the decision on which warnings to fix at the close (none in code) and its reason recorded, so that the note can quote it.
29. As the plan pilot, I want no full-game rerun when no fix changes `src/game`, `src/p8` or the PSX backend, so that the close costs only what the plan requires.
30. As the plan pilot, I want the throwaway computations (the bound split, the per-frame option tables) quoted in the note, so that a later reader can redo them.
31. As a reviewer, I want the "one frame late" vs F−2 wording in the display-capture paragraph made consistent, so that the doc no longer contradicts itself.
32. As a reviewer, I want "Still to do" entries self-contained (no reference to the plan file), so that they stay true after plan-pilot deletes the plan.
33. As a reviewer, I want the steady situations' "Situations" section of `docs/psx-game.md` untouched, so that the frozen rule stays frozen.

## Implementation Decisions

### Warning fixes at the close: none in code (decided here)

The step allows fixing a warning only in code a step of this plan touched,
and a fix that changes `src/game`, `src/p8` or the PSX backend forces a
full `psx-full-game-check` rerun whose table then replaces step 8's for
every estimate. Decision: **no code warning is fixed at the close; every
one is listed.** Reasons, per kind:

- the phase-3 game-code warnings (`ui_button.c`'s two draw paths, the
  `g_building_look` global, the const cast, the `*_text_image` twins,
  `draw_functions.c`'s override, the recomputed text width, `entity_list.c`'s
  NULL tolerance, `!= NULL` presence tests, `ui_button.h`'s pointer fields)
  are design changes, not one-line fixes; each would need its own PRD,
  tests and a full-game rerun;
- the magic numbers (`geom.c`'s `0x200000`, `0x2000`, `0x100`;
  `slipway_display.c`'s 32765) and the button slot sizes repeated across
  `p8/image.h`, `psx_vram_layout.c` and `button_image.h` are behaviour-free
  renames, but they sit in `src/game`, `src/p8` and the PSX backend, so
  even a rename forces the rerun (about the price of a whole step) and
  moves the base of every option estimate; not worth it at a close;
- `psx_gpu.c`'s readback inside an `assert` is in the PSX backend (rerun);
- the Makefile and script warnings (duplicated table joins and situation
  lists, inline 263 / 31 / 15 / 3, two variables for one table,
  `steady-profile`'s untested recipe, `loading_screen.c` and
  `button_image.c` in `ENGINE_SRCS`, e2e frame numbers inline,
  hand-copied condition counts, the `8000` mask repeated in the Lua and
  two awk scripts) span many files and tools; a partial fix is worse than
  a listed one;
- `pad_author.c`'s size and missing unit test, `cart.c`'s difficulty
  carrier and duplicated prologue, `psx_gpu_replay.c` reading a protocol
  value from the host parser: design changes, listed.

The **only warning acted on** is the doc wording "one frame late" vs F−2
in `docs/psx-game.md`'s display-capture paragraph (phase 2), since the
close rewrites that document anyway and it changes no code. The fix makes
the parenthesis say what the paragraph proves: at the top of frame F the
console still shows the frame drawn at F−2 (the frame drawn at F−1 is shown
only once F−1's `psx_gpu_frame_end` swaps, which is why the PSX shows each
frame one frame later than the PC). One sentence, no other change in that
section.

Consequence: no full-game rerun; **step 8's committed table
(`psx/perf/full-run-frames.txt`) is the base for every figure below.**

### Fresh profiles at HEAD, before any estimate

- Boot the six steady runs of the manifest at HEAD: `make psx-short-runs
  PSX_SHORT_RUNS="map-rest-5581 map-move-1057 build-5752 menu-5954
  tech-panel-5895 score-panel-5876" PSX_SHORT_RUN_REFERENCE=psx/perf/full-run-frames.txt`
  (step 8's table, the same bytes the full-game check wrote). The target
  is known to fail on `representative` for the best-try runs; that verdict
  is expected and not a blocker, as long as `done`, `deterministic`,
  `breakdown` and `asserts` hold on every run and each run has its
  `breakdown` and `plain-1` boots. Any other failure stops the step.
- Then `make steady-profile` with `PSX_SHORT_RUNS` unset: its pooled
  overhead, a profile per run (post-lever per-call costs and splits) and
  the `no-run` profiles of map-rest, slipway-aim, build and score-panel,
  which read the judge table (step 8's) and so cover **step 8's frames
  over**. Every run must print `adds-up ok`.
- Source of each situation's shares, as step 5 set them: map-move,
  menu, tech-panel from their representative runs; map-rest, build,
  score-panel, slipway-aim from `no-run`, with PSX per-call costs from
  their best-try run (slipway-aim: pooled over the representative runs).
- Cross-check, not a source: `tools/steady_profile.sh no-run` called
  directly for map-move, menu and tech-panel on the full-game profile sets
  `make steady-profile` already wrote (no tool change). Where an
  option's target share differs from the run's by more than a fifth, the
  note lists it (map-move-1057's window draws a wormhole on every frame,
  the full game's map-move frames over mostly do not).
- The profiles stay in `tmp/` (not committed); the note quotes the
  figures each estimate uses.

### The steady situations still over (from step 8's table)

A throwaway `awk` join of `psx/perf/full-run-frames.txt` with
`psx/perf/full-run-situations.txt`, quoted in the note, gives per steady
situation, over its frames whose total exceeds 263: count, worst frame
(number and total), mean total, mean update, draw and submit, and the
**bound**: a frame is draw-bound when its draw exceeds its update,
update-bound otherwise; the situation lists both counts. A pre-check at
HEAD gave (to be confirmed by the implementer, not copied):

| situation | over | worst | mean total | mean update / draw / submit | draw-bound |
|---|--:|--:|--:|--:|--:|
| map-rest | 60 | f2671, 428 | 284.4 | 63.6 / 218.2 / 2.7 | 53 |
| map-move | 43 | f3749, 467 | 312.8 | 149.7 / 160.7 / 2.5 | 29 |
| build | 38 | f4657, 478 | 280.5 | 31.6 / 245.7 / 3.2 | 37 |
| menu | 23 | f349, 388 | 292.3 | 70.3 / 219.1 / 2.9 | 21 |
| slipway-aim | 14 | f5718, 298 | 280.4 | 46.6 / 231.4 / 2.5 | 14 |
| tech-panel | 13 | f5579, 320 | 288.1 | 29.7 / 255.3 / 3.1 | 13 |
| score-panel | 7 | f5558, 353 | 321.3 | 102.6 / 216.1 / 2.6 | 6 |

The update-bound frames (map-move's 14 above all) are named by frame in
the note with their largest update piece where the full-game PC profile
shows it (e.g. `save_at_frame_end`, `update_entities`), since no P option
touches update except P5's emitter part.

### Option estimates, each option alone

Old per-frame savings (from `docs/psx-levers.md` before its deletion):
P1 3.6–5.6 h; P2 about 7 h (menu-5954), 6 h (map-rest-5581) plus game
code; P3 about 6–10 h; P4 up to 40–60 h on the frames that show a
wormhole; P5 about half of 7–20 h on menu and tech-panel. The new saving
comes from the fresh profiles, per situation; the note gives old and new
side by side.

How each is computed (the step's split, decided here in detail):

- **P1, background grid baked into the image — fixed per frame.** Saving
  per frame = the 18 `pattern` fills and the `rect` the grid and border
  draw, at the fresh per-call cost of that situation's run (best try for
  the no-run situations, pooled for slipway-aim), minus nothing else (the
  bigger image copy is assumed free, stated). Applied by subtracting it
  from `draw` (and `total`) of every **steady** frame of that situation in
  a copy of step 8's table in `tmp/`, then counted with `make
  situation-report FRAMES=<copy>`. One-off frames are left as measured
  (their 674 vsyncs are far under 10,800; conservative).
- **P4, fewer wormhole reveal rings — per frame, only frames that draw a
  wormhole.** Per frame, the `circ` calls under `wormhole_draw_method`
  from the full-game PC replay's per-frame call counts (the full-game
  profile `make steady-profile` writes; else a throwaway frame-profile
  run rooted at `wormhole_draw_method`), as N3 sized its gain. Saving on
  that frame = (its rings − kept rings, floored at 0) × the fresh
  post-N3 `circ` per-call cost from map-move-1057. Two variants, kept
  rings 3 and 2, each applied to its own copy and counted with `make
  situation-report`.
- **P2, slipways without the black border pass; P3, transit dots without
  their 2×2 backing; P5, fewer particles — proportional to draw.** Per
  situation, the saving as a percent of the source's mean draw: P2 = half
  of `slipway_draw_method`'s `line` time in primitives (its game code is
  not counted: the per-segment share cannot be split from the profile;
  conservative, stated); P3 = the dots' `rectfill` calls × the fresh
  `rectfill` per-call cost (the dot count from the run's PC call counts
  under `slipway_draw_method`); P5 = half of the `particles` group's total
  (primitives and game code). Each option gets **its own** lever-table
  file in `tmp/` (a markdown file with one fenced `lever-table` block: one
  `lever P<n> estimated <S|M|L> basis share measure <source>` line and its
  `gain` lines, **no `scale` line**, so no measured ratio scales an option)
  and is projected with `make lever-estimate LEVERS=<file>` on step 8's
  table, which prints nominal and pessimistic (× 0.8) per situation.
  A situation whose source shows none of the option's target gets no
  `gain` line.
- Options are **not** combined and **not** ranked. Only steady frames are
  counted per situation; the note gives each option's per-situation
  nominal / pessimistic counts and the total.

### `docs/psx-game.md`, "Still to do" (rewritten)

What it says at the close, every entry self-contained (no reference to
`docs/plan-22.md`, which plan-pilot deletes after the close):

- **The result.** `real-game-budget` is red after every pixel-identical
  lever of plan 22 (D3, N1, N2, N3, N7, G4, N6; N5 not taken, under 2%):
  198 steady frames over 263, one-off 674 of 10,800 vsyncs, worst steady
  frame 4657 (`build`, 478); `frame-budget` red (491 frames over, worst
  f14 at 2,304, galaxy generation). Replaces the two current bullets that
  carry these figures and the one naming the levers document.
- **The steady situations still over**: the table above (confirmed
  figures), and one sentence on what bounds them.
- **Picture-changing options, for the human to choose** (pass 2, a
  later grilling): P1–P5 in that order, unranked, each with what changes
  on screen, its per-frame saving, its steady frames still over alone per
  situation (nominal / pessimistic), and how it was computed (per frame on
  a copy of the table, or proportional through `make lever-estimate`).
  Followed by the known error: the short-run projection before the
  rebase said 361 nominal / 516 pessimistic, measured 198 (map-rest 116 /
  202 vs 60, map-move 141 / 187 vs 43, build 36 / 56 vs 38, menu 27 / 27
  vs 23, slipway-aim 21 / 24 vs 14, tech-panel 14 / 14 vs 13, score-panel
  6 / 6 vs 7): too pessimistic in the map situations, slightly optimistic
  in build and score-panel. And one line on the sources' known biases
  (no-run shares are PC time and overstate primitive-heavy functions;
  resumed runs draw 12–24% less). No agent says an option is "barely
  visible".
- **Open warn-review warnings of plan 22**, one bullet per group of the
  same kind with every file[:line] it covers (so every warning is
  findable by its file), phase given in words, not as plan steps.
- **Open findings**: `make worst-case-peaks` red (two lines frozen since
  plan 12 and `zone frame` since plan 21's 59fc46a: a stale frozen table,
  not a code bug; re-baselining is the human's call); D3's cold-cache rise
  (f4887 score-panel draw 146 → 160, f5275 130 → 147, neither over 263,
  cause unproven: text slots built on a cold cache); the resumed short
  runs drawing less (already there, kept, trimmed to what is still true);
  any other finding the note-reading pass below turns up.
- The other existing bullets (frame-table bytes in the GPU replays only,
  hardware renderers, disc for real hardware) stay as they are.

The rest of `docs/psx-game.md`: the tool paragraph on `make
lever-estimate` loses its levers-document default (below); the
"Situations" section is **not** edited (frozen); any other sentence that
describes plan 22 as in progress is made present-tense and true.

### `make lever-estimate`: re-sourced, not removed

Pass 2 will need to project the options the human picks, and the tool and
its test (`test_lever_estimate`, fixture documents only) do not depend on
the levers document. Decision:

- the target keeps its interface, but **`LEVERS` is required**: without
  it the target fails before running the script, with a message naming
  `LEVERS` and the lever-table format's place in `docs/psx-game.md`; the
  default document is dropped;
- the Makefile help lines for the target say so and name no document;
- `tools/lever_estimate.sh`'s usage comment stops naming the deleted
  document (it reads "a markdown document holding one `lever-table`
  block");
- `docs/psx-game.md`'s description of the tool says it reads the block of
  the document passed as `LEVERS` and keeps the format description;
- `test_lever_estimate` is unchanged (it already passes its own
  document); `make frame-profile-test` must stay green.

Then `docs/psx-levers.md` is deleted (`git rm`). Its content that stays
true and useful moves first: the options (into "Still to do", re-estimated)
and the two sources' biases (one line, above). The lever entries, profiles,
lever table and plan 21's verdicts are finished work: git holds them.

### Reading every step note for open findings

The implementer reads every step note of `docs/plan-22.md` (1.1 to 3.8)
for findings left open or handed to step 9. Known candidates and their
decided end:

- step 1.4 / step 1.5, resumed runs draw less, transit-dot cause unproven
  → "Still to do" (existing bullet, kept);
- step 2.6, red `make worst-case-peaks` → "Still to do";
- step 3.7 D3 / step 3.8, cold-cache rise → "Still to do";
- step 3.7 N2, the caution on scaled estimates → closed: every lever was
  measured and is `in-base` (ledger points to step 3.8);
- step 3.8, N6's two isolations disagreeing → closed by step 3.8's
  decision (kept on its own probe); ledger points to step 3.8;
- step 1.2's pad_author self-check and step 3.8's `frame-profile-test`
  → closed by their `fix:` commits; ledger points to their notes.

Anything else found open goes to "Still to do" with one line in the
ledger.

### The ledger (for plan-pilot's step note)

One line per warning of phases 1, 2 and 3 and per open finding:
`<phase or step> <file[:line] or finding>: <bare text> -> <Step x.y note | Still to do: "<entry's first words>">`.
The warnings come from the end-of-phase paragraphs of steps 1.5, 2.6 and
3.8 (13, 13 and 16). Where a paragraph names a group in one phrase (e.g.
"inline 263 / 31 / 15 / 3", "duplicated table join and situation lists
across five scripts and the Makefile"), the ledger splits it into one line
per constant or per file the phrase names, until the phase's line count
equals its recorded count; if a phase's text cannot be split to its count
without guessing, the ledger says so on one line with the shortfall and
names that phase's paragraph as the only record. Every warning ends in
"Still to do", except the "one frame late" wording, which ends in this
step's note (acted on). The implementer writes the ledger to
`tmp/close/ledger.txt` and hands it to plan-pilot in its report;
plan-pilot puts it in the `### Step 4.9` note (it also writes the note's
figures: profiles, bounds, options old/new, re-source).

## Implementation Chunks

0. **Warning fixes in code: none** (decision above). No work, no rerun;
   step 8's table is the base.
1. **Fresh profiles and option estimates** (nothing committed by itself:
   outputs in `tmp/`). `make psx-short-runs` on the six runs, `make
   steady-profile`, the no-run cross-check for the three representative
   situations, the bound join on step 8's table, P1 and P4 (two variants)
   per frame on table copies counted by `make situation-report`, P2, P3,
   P5 each through its own `tmp/` lever-table file and `make lever-estimate
   LEVERS=`. Old and new per-frame savings tabulated. Lands with chunk 2.
2. **`docs/psx-game.md`'s "Still to do" and the wording fix.** Rewrite as
   decided (result, situations still over, options P1–P5 with their
   method and the known error, open warnings, open findings); fix the
   "one frame late" vs F−2 sentence; make plan-22-in-progress sentences
   true. Commit (`docs:`).
3. **`lever-estimate` re-source and `docs/psx-levers.md` deletion.**
   `LEVERS` required in the Makefile target, help lines, script usage
   comment and the tool paragraph of `docs/psx-game.md`; delete the levers
   document. Check: `make lever-estimate` without `LEVERS` fails with the
   message; with a `tmp/` table it runs; `make frame-profile-test` green;
   `grep -rn psx-levers Makefile tools docs src tests` finds only plan
   files (and this PRD while it is in triage). Commit (`chore:` or
   `docs:`).
4. **Ledger and final checks.** Write `tmp/close/ledger.txt`; check every
   line's target exists (a step note heading in the plan, or a "Still to
   do" entry); run `make test-all`, `make frame-profile-test`,
   `psx-check`; confirm every e2e `.sha256` and `.end-state` unchanged
   (`git diff --stat` on `tests/e2e/` and `worst-case/` empty). Nothing
   to commit unless a check forces a doc correction.

## Testing Decisions

- No game, runtime or backend code changes, so no new unit test. The
  e2e replays and `psx-check` stay the net and must be untouched and
  green.
- The tool change is a Makefile guard (`LEVERS` required). It is checked
  by running the target with and without `LEVERS`, quoted in the note;
  `test_lever_estimate` already covers the script on fixture documents
  (prior art: its `run_doc` fixtures), and `make frame-profile-test`
  (which holds it) must be green.
- Estimates are not tests: their inputs (profile figures, per-call costs,
  ring counts) and the throwaway `awk` lines are quoted in the note so the
  numbers can be redone. A good check here is arithmetic one can repeat,
  not a frozen file.
- `make test` and `make e2e` timings: unaffected (no source, test or
  e2e change); no measurement required beyond the green runs.

## Out of Scope

- Implementing any picture-changing option, ranking them, or calling any
  of them "barely visible": the human chooses in a later grilling.
- Fixing any code warning (decision above), and plan 21's warnings
  (step 9 covers plan 22's).
- Re-baselining `make worst-case-peaks`'s frozen table (the human's call).
- Proving the cause of D3's cold-cache rise or of the resumed runs'
  missing draw.
- Changing the situation rule or table (frozen), the 263 budget, the
  short-run manifest or the one-off allowance.
- A new `psx-full-game-check` (none is needed: no code change).
- Deleting `docs/plan-22.md` and `progress.md` (plan-pilot's finish) and
  writing the step note itself (plan-pilot's; this PRD hands it the
  figures and the ledger).
- Measuring on the Miyoo or PCSX-ReARMed; audio.

## Further Notes

- "Done when" of plan 22 is met through its second branch: every step done
  and "Still to do" listing the steady situations still over and the
  picture-changing options for the human.
- The only booting in this step is `make psx-short-runs` (pcsx-redux,
  headless). The raylib app is never launched.
- Map-move's frames over are a third update-bound (mean update 150 h on
  its frames over, against about 38 h on all its frames): the update
  spikes there are something no P option targets; "Still to do" should
  make that visible rather than imply the options can clear map-move.
- No `guideline` question was needed: every open choice is decided above
  (no code warning fixed; `LEVERS` required rather than removing the
  target; P4 in two variants; P2 without its game code; one-off frames
  left as measured in the per-frame copies; options carry no `scale`).
