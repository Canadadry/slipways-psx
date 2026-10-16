---
title: "Situation of every frame of full_game.json"
description: "Every frame of full_game.json needs a frozen, state-derived situation label and a steady/one-off family, so the real-game budget can judge each slow frame by its cause."
status: needs-triage
---

## Problem Statement

Plan 22 judges the PSX build against a model of a real 60-minute game, not
against "every frame within 263 hblanks". The model splits frames into two
families: **steady** situations (what the player stays in: map at rest, camera
or cursor moving, a panel or menu open), which must be 100% within 263, and
**one-off** events (galaxy generation, a new year, colonising, building a
slipway, game end, ...), whose over-budget vsyncs only have to stay under 5% of
216,000 vsyncs.

Nothing in the repository says which frame of `worst-case/full_game.json` is
which. The frame table (`psx/perf/full-run-frames.txt`) gives update, draw,
submit, total and vsyncs per frame, but not why the frame costs what it costs.
Without a label per frame, `real-game-budget` (step 3), the short-run selection
(step 4), the lever ranking (step 5) and every later measurement cannot be
computed, and anyone could move a costly frame from "steady" to "one-off" to
make the budget pass. The label must therefore come from the game state by a
written rule, be committed as a table, and then be frozen.

## Solution

A host-side, headless tool replays `full_game.json` on PC (no PSX boot, no
window) and, after each frame's update, reads a small snapshot of the game
state and compares it with the previous frame's snapshot. A pure classifier
turns each (previous, current) snapshot pair into one **situation** and its
**family**, `steady` or `one-off`. The tool writes one line per frame. Its
output on `full_game.json` is committed as `psx/perf/full-run-situations.txt`.

The labeling rule is written in `docs/psx-game.md` (new "Situations"
section). A report script joins the situation table with a frame table and
prints, per situation, its frame count, frames over 263, mean update and mean
draw; for each one-off situation, its occurrences and its longest run of
consecutive frames, flagging a run that is too long. That report, on step 1's
frame table, is the step note's evidence.

After this step the rule and the table are frozen: no later step changes them
except step 6's renumbering (the loading screen frame).

## User Stories

1. As the plan pilot, I want every frame of `full_game.json` labeled with a situation and a family, so that `real-game-budget` (step 3) can sum steady overruns and one-off vsyncs separately.
2. As the plan pilot, I want the label computed from the game state only (screen, open panel, cursor or camera motion, running animation or event), so that no frame timing can influence which family a frame lands in.
3. As a reviewer, I want the labeling rule written in `docs/psx-game.md`, so that I can check any label by hand against the rule without reading C.
4. As a reviewer, I want the labeled table committed at `psx/perf/full-run-situations.txt`, so that later steps join against a fixed, reviewable file instead of rerunning the tool.
5. As a reviewer, I want the table to have exactly one line per frame of `full_game.json`, numbered like the frame table, so that the join is a plain line-by-line match.
6. As a reviewer, I want a make target that regenerates the table and fails when it differs from the committed one, so that drift between the code and the table is caught.
7. As a developer, I want that regeneration check outside `make test` and `make e2e`, so that the fast loop does not get slower.
8. As a developer, I want the classifier to be a pure function of two snapshots, so that its every branch is unit-tested with hand-built snapshots in milliseconds.
9. As a developer, I want a unit test on hand-picked frames of `full_game.json`, one or more per situation that occurs there, checked against the committed table, so that a wrong label on a known frame is caught by `make test` without replaying the full game.
10. As a developer, I want a unit test that runs the real snapshot reader on one or two short e2e replays of `tests/e2e/`, so that the reading of the live game state is tested, not only the classifier.
11. As the plan pilot, I want the title screen labeled `title` (steady), so that the frames before the game starts are accounted for.
12. As the plan pilot, I want the attract demo labeled `attract` (steady), so that the tool gives a label to any replay, even if `full_game.json` never shows it.
13. As the plan pilot, I want the map with nothing moving labeled `map-rest` (steady), so that "map at rest" is a situation of its own.
14. As the plan pilot, I want map frames where the camera or the cursor moved labeled `map-move` (steady), so that motion cost is judged separately from rest.
15. As the plan pilot, I want the slipway target choice labeled `slipway-aim` (steady), so that the dragging preview is judged as a steady situation.
16. As the plan pilot, I want the build bar and structure placement labeled `build` (steady), so that browsing the build bar is a steady situation.
17. As the plan pilot, I want a planet menu or any other open box labeled `menu` (steady), so that "a panel or menu open" is judged as steady.
18. As the plan pilot, I want the tech panel labeled `tech-panel` and the score panel `score-panel` (steady), so that each heavy panel has its own figures.
19. As the plan pilot, I want the pause menu labeled `pause` (steady), so that it is accounted for.
20. As the plan pilot, I want the frame where the galaxy appears labeled `galaxy-generation` (one-off), so that the 15-vsync stall at frame 12 is a one-off event.
21. As the plan pilot, I want the frame where the game world disappears labeled `return-to-title` (one-off), so that the teardown frame is an event.
22. As the plan pilot, I want the frame where the game reaches its end status labeled `game-end` (one-off), so that the end of game is an event.
23. As the plan pilot, I want the frame where the year increases labeled `new-year` (one-off), so that end-of-year recomputation is an event.
24. As the plan pilot, I want the frame where a tech is invented labeled `research` (one-off), so that research is an event.
25. As the plan pilot, I want the frame where a planet becomes colonised labeled `colonise` (one-off), so that colonising is an event.
26. As the plan pilot, I want the frame where a structure appears labeled `structure-built` (one-off), so that building is an event.
27. As the plan pilot, I want the frame where a slipway appears labeled `slipway-built` (one-off), so that building a slipway is an event.
28. As the plan pilot, I want the frame where a probe is launched labeled `probe` (one-off), so that probing is an event.
29. As the plan pilot, I want the frame where the screen changes (a panel, menu, bar or pause opens or closes, a slipway is started or dropped) with no other event labeled `screen-change` (one-off), so that the cost of building a panel is an event tied to one action.
30. As the plan pilot, I want the frames right after an event, while the screen is still settling (an open panel's layout still moving, a shake still running), to carry that event's label, so that the per-action tail of an event is not counted as steady play.
31. As the plan pilot, I want that continuation to end at the first settled frame, so that a one-off run is always one event and its bounded animation, never open-ended.
32. As the plan pilot, I want a fixed priority when several events happen in the same frame, so that each frame has exactly one label and the rule is deterministic.
33. As the plan pilot, I want the first frame of a resumed replay judged against the state right after the resume, not against an empty state, so that step 4's resumed short runs get correct labels from their first frame.
34. As the plan pilot, I want the tool to accept any events file (title start or resume start), so that step 4 can label its short runs with the same rule.
35. As the plan pilot, I want a report joining the situation table with a frame table, printing per situation its frame count, frames over 263, mean update and mean draw, so that the step note can be filled without hand arithmetic.
36. As the plan pilot, I want the report to print, for each one-off situation, its occurrences (event frames) and its longest run of consecutive frames, so that the "long or not tied to one event" criterion is checked.
37. As the plan pilot, I want the report to mark a one-off run longer than 31 frames, so that a mislabeled stretch fails the step visibly.
38. As the plan pilot, I want the report to fail when the two tables have different frame counts or numbering, so that a join against the wrong table is never silent.
39. As the plan pilot, I want the report to fail on an unknown family or a situation name that appears with both families, so that a corrupt table is caught.
40. As step 6's implementer, I want the hand-picked frames listed in one place in the test, so that renumbering after the loading screen means editing that list and regenerating the table, nothing else.
41. As step 8 and the close, I want the rule text in `docs/psx-game.md` marked as step 2's and frozen, so that I can confirm it was not changed.
42. As a developer, I want the tool to never open a window and never boot the PSX emulator, so that it follows the project's headless rule and runs in seconds.

## Implementation Decisions

### Modules

- **Situation classifier** (new devtools module, `situation`): the deep module of this step.
  - `situation_snapshot_t`: a plain value with the facts the rule reads, and nothing else:
    - screen mode, using the existing pad-metrics classification (`title`, `pause`, `placement`, `bar`, `tech`, `score`, `menu`, `jump`, `aim`, `slipway`), plus whether the attract demo is running;
    - whether a game world (economy) exists, and whether the economy has its end status;
    - year;
    - counts: colonised planets, slipways, structures, invented techs, probes;
    - camera map position (before shake, whole pixels) and the hovered / pad-selected entity id;
    - "settling" facts: the open panel's layout (its items and positions, as the pad author's panel-settle wait compares them) and whether a shake (camera or planet) is running.
  - `situation_snapshot_read(const input_step_t *step)`: reads the live game after `cart_update60()` and before `cart_draw()`. Reuses pad-metrics' flag reading for the mode rather than duplicating it.
  - `situation_classify(const situation_snapshot_t *before, const situation_snapshot_t *after, situation_t previous)`: pure; returns the frame's `situation_t`. `previous` is the label of the previous frame, needed only for the continuation rule.
  - `situation_name(situation_t)` and `situation_family(situation_t)`; the family is fixed per situation, never per frame.
- **Situations command** (new binary in `src/cmd/`): replays an events file exactly like the frame profiler does (replay source, bindings, title or resume start through record-start, input step, `cart_update60`, `cart_draw`), with no instrumentation, and writes the table to a path given on the command line. The first "before" snapshot is read after `cart_init()` (title start) or after the resume (resume start), before frame 0's update.
- **Situation report** (new script in `tools/`): joins a frame table and a situation table, prints the per-situation lines and the one-off lines, and the verdict lines.
- **Docs**: a new "Situations" section in `docs/psx-game.md` holding the rule (below), the table format, the targets, and the freeze statement; rows for the new targets in the Checks table.

### The labeling rule (to be copied into `docs/psx-game.md`)

For each frame, `before` is the snapshot after the previous frame's update (or the start snapshot for frame 0), `after` the snapshot after this frame's update.

1. **Event frame.** The first that holds, in this priority order, labels the frame as that one-off event:
   1. `galaxy-generation`: no game world in `before`, a game world in `after` (new game or resume);
   2. `return-to-title`: a game world in `before`, none in `after`;
   3. `game-end`: the economy's end status appears;
   4. `new-year`: the year increased;
   5. `research`: the invented tech count increased;
   6. `colonise`: the colonised planet count increased;
   7. `structure-built`: the structure count increased;
   8. `slipway-built`: the slipway count increased;
   9. `probe`: the probe count changed;
   10. `screen-change`: the screen mode changed (attract on/off included).
2. **Continuation.** Otherwise, if the previous frame was one-off and the screen is still settling (`after`'s open panel layout differs from `before`'s, or a shake is running in `after`), the frame keeps the previous frame's label.
3. **Steady.** Otherwise the frame is steady, by `after`'s screen:
   - `attract` while the attract demo runs; `title` on the title without a game world;
   - `pause`; `build` (bar or placement); `tech-panel`; `score-panel`; `menu` (any other open panel or box, final score included); `slipway-aim` (slipway started, target not chosen);
   - on the map (jump or aim mode): `map-move` when the camera map position or the hovered / pad-selected entity differs between `before` and `after`, else `map-rest`.

Families: `title`, `attract`, `map-rest`, `map-move`, `slipway-aim`, `build`, `menu`, `tech-panel`, `score-panel`, `pause` are `steady`; the ten event labels are `one-off`.

An **occurrence** of a one-off situation is one event frame (rule 1) of that label; its **run** is the event frame plus its continuation frames. A one-off run longer than **31 frames** (the camera shake of 3 set by `burst_construct`, decaying 0.1 a frame for 30 frames, plus the event frame) fails the step; a one-off run not starting with an event frame cannot exist by construction.

### Table format

`psx/perf/full-run-situations.txt`: a header line `frame situation family`, then one line per frame, `<frame> <situation> <family>`, frames numbered from 0 in replay order, the same numbering as the frame table. No other column, no comment line.

### Targets

- `make situations [EVENTS=<events>] [SITUATIONS_OUT=<file>]`: builds the command and writes the table (default events `worst-case/full_game.json`, default output under `tmp/situations/`). Outside `make test-all`.
- `make situations-check`: regenerates the full-game table and diffs it against `psx/perf/full-run-situations.txt`, failing on any difference with the first differing frame. Outside `make test-all`.
- `make situation-report [FRAMES=<frame table>] [SITUATIONS=<situation table>]`: runs the report script, defaults `psx/perf/full-run-frames.txt` and `psx/perf/full-run-situations.txt`. Outside `make test-all`.
- Each target gets its line in the Makefile's legend.

### Report output

One line per situation, steady first then one-off, each in the rule's order:
`situation <name> <family> frames <n> over <n> update-mean <u> draw-mean <d>`
(means as integers, hblanks, over all the situation's frames; over = total > 263). One line per one-off situation:
`one-off <name> occurrences <n> longest <n> [long]`. Then verdict lines: `frame-count ok|failed <detail>`, `families ok|failed <detail>`, `one-off-runs ok|failed <names over 31>`. Exit 0 once written when the inputs parse, whatever the verdicts; 1 on a usage or parse error. Same shape as `tools/psx_frame_budget.sh`'s verdict lines.

### Other decisions

- The labeler reads only state, never input buttons and never frame times.
- The snapshot is taken after the update because both the update's and the draw's cost of frame k follow from what frame k's update did.
- The step note lists the report's lines on step 1's table, the commit of that table, and, for each one-off situation, which action or game event of `full_game.pad` its occurrences correspond to.
- If step 1's frame table is not committed when this step runs, or does not join (H-1-1, H-1-2), the implementation is still committed with the table; the note records the report as blocked rather than joining against another table.
- The rule's text in `docs/psx-game.md` says it is step 2's rule of plan 22, frozen, changed only by step 6's renumbering.

## Implementation Chunks

1. **Classifier**: snapshot type, `situation_classify`, names and families, with its unit test on hand-built snapshots (every situation, priority, continuation start and end, first frame of a title start and of a resume start).
2. **Snapshot reader and command**: `situation_snapshot_read`, the situations command, the `situations` target, the live test on short e2e replays.
3. **Table and report**: generate and commit `psx/perf/full-run-situations.txt`, the `situations-check` target, the report script and its fixture test, the `situation-report` target, the committed-table test with hand-picked frames.
4. **Docs**: the "Situations" section and the Checks table rows in `docs/psx-game.md`.

## Testing Decisions

- A good test checks external behavior: the label for given snapshots, the lines a table yields, the report for given tables. It does not check how the snapshot is gathered or which helper is called.
- **Classifier** (`tests/test_situation.c`): hand-built snapshot pairs, one case per situation at least, each priority conflict that matters (new year and screen change in one frame gives `new-year`; galaxy appears and screen changes gives `galaxy-generation`), continuation kept while the layout moves and dropped on the first settled frame, continuation never following a steady frame, `map-move` on camera motion and on selection change alone. Fast, part of `make test`.
- **Committed table** (in the same test or a sibling): reads `psx/perf/full-run-situations.txt`; checks the header, 6,059 frame lines numbered 0..6058, each family matching its situation, and a list of hand-picked frames with their expected situations, at least one per situation present in `full_game.json` (frame 12 is `galaxy-generation`; one frame inside a score panel; one map-rest frame; one frame of each event the pad script performs). The implementer picks the frames by reading `full_game.pad` and the tool's output, then checks each one by hand against the rule. The list sits in one place so step 6 can renumber it. File read only, fast.
- **Snapshot reader** (live test): replays one or two short e2e recordings of `tests/e2e/` (at most 180 frames each) through the command's loop and checks a few expected labels (a panel open frame, an event frame). Prior art: `test_replay_from_title`, `test_replay_from_resume`.
- **Report script** (`tests/test_situation_report.c`): hand-written fixture tables in a work directory under `tmp/`; checks the per-situation counts, over count, integer means, occurrences, longest run, the `long` mark past 31 frames, and failing verdicts on a frame count mismatch, a numbering gap, an unknown family and a name with two families. Prior art: `tests/test_psx_frame_budget.c`.
- The regeneration diff (`situations-check`) is not part of `make test` (a full-game replay); it is run once in this step and its result given in the note.
- No e2e hash may change: the step adds no game code.

## Hypotheses on step 1

Step 1 (baseline `psx-full-game-check` replacing `psx/perf/full-run-frames.txt`) is not done when this PRD is written. Everything below is assumed, and the note must confirm or refute each one.

- **H-1-1**: step 1's table is committed at `psx/perf/full-run-frames.txt` in the format `tools/psx_frame_budget.sh` writes today: header `frame update draw submit total vsyncs`, one line per frame, values in hblanks.
- **H-1-2**: it has 6,059 frame lines, frames 0..6058, and frame k is the k-th input frame of `worst-case/full_game.json` as the PC replay plays it (the same numbering the situations command uses).
- **H-1-3**: step 1 changes no game code, no recording and no replay tool (it is a measurement only), so labels computed at this step's HEAD describe the game the step-1 table timed.
- **H-1-4**: "over 263" in the note means total > 263, as `frame-budget` counts it, and the update and draw columns are the ones to average.
- **H-1-5**: frame 12 is still the single worst frame (galaxy generation, about 4,000 hblanks) and the bulk of frames over 263 are ordinary frames bound by draw, as at plan 21's close; this is used only as a sanity check of the labels (frame 12 must be `galaxy-generation`), never to shape the rule.
- **H-1-6**: the PSX GPU replay plays the same game state frame for frame as the PC replay (the full-game check's state and digest conditions hold in step 1), so a label computed on PC applies to the PSX frame of the same number.

## Out of Scope

- `real-game-budget` and any summing of vsyncs against 10,800 (step 3).
- Short runs, their selection, and labeling them (step 4 uses this tool).
- The profile and the lever list (steps 4 and 5).
- The loading screen and the renumbering it causes (step 6).
- Any change to game code, to the picture, to e2e hashes or to `full_game.json`.
- Measuring on PSX: this step boots nothing; it reads step 1's table.

## Further Notes

- The 31-frame bound and the event list are this step's decisions; once committed they are frozen with the rule.
- Occurrence counts are what the real-game model needs: a real 60-minute game is assumed to have about as many one-off events as `full_game.json`.
- The labeler reads the game's state through existing accessors (pad navigation, economy, camera, entity lists); if one fact needs a new read-only accessor in game code, it must not change behavior, the picture or any e2e hash.
