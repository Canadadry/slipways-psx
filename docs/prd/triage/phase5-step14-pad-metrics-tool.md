---
title: "Pad metrics tool"
description: "Nothing in the repository measures how a pad game is played under the new scheme, so section 4 of plan 12 cannot be checked: move pad_author's bookkeeping into a shared pad_metrics module and expose it as a replay hook that reports time per mode, dead presses per mode in two groups, hold repeats apart, and section 4's verdict."
status: needs-triage
---

## Plan step (verbatim)

Plan file `docs/plan-12.md` (at `a4d8cc9`), Phase 5 — Finish, step 14:

> 14. **Pad metrics tool.** A headless devtools target, outside `make test-all`, that replays an events file and reports the share of time in each mode of the new scheme (jump, aim, slipway, menus, placement) and the dead presses per mode (presses that change no nav state, cursor, camera or game state). It is the measure for section 4. Dead presses are reported in two groups: those the locked rules require (○ at the root with no last object or with the cursor already on it, a D-pad press or hold repeat into an empty cone) and all others. Hold repeats are reported apart from presses. Method: the step's first commit is a pure refactor. `pad_author`'s mode classifier, view snapshot, change test, hold-repeat counter, empty-cone check and totals writer move to `src/devtools/pad_metrics.{c,h}`, which `pad_author`'s `emit` calls; between frames it keeps entity ids, never pointers, sizes its direction array by `PAD_JUMP_DIRECTIONS`, and takes its scratch zone from the caller. That commit is checked by `make pad-author-proof`, by `make pad-author` followed by `git diff --exit-code worst-case/`, and by every `tmp/pad-author/*.modes.txt` and totals output unchanged; `tests/test_pad_metrics.c` tests the classifier and the change test on hand-built states, and `make test` is not slower. The tool is a `PAD_METRICS=<path>` hook in `src/cmd/replay.c`, beside `STATE`, `REPLAY_FILL` and `SAVE_HEX`, calling `pad_metrics` once per frame, with a `make pad-metrics EVENTS=` target outside `make test-all`; no new frame loop, and `make e2e` is not slower with the variable unset. A press counts as dead when nothing in the change test moves between its press frame and 2 frames after its release. Verified on `tests/e2e/pad_colonise.json`, which `tools/pad_author_proofs/pad_colonise.frames.pad` regenerates byte for byte: mode frames, presses per mode and hold repeats equal `pad_author`'s totals for that proof, and the first group holds exactly the proof's `back --required` no-op (f293), found from game state, not from the script. Also verified on the four `worst-case/` pad games against `pad_author`'s totals, with no Select tap, menu △ or fold × counted dead. Last check: run section 4's measurement on the four re-authored `worst-case/` games and record each figure in the step note; a miss goes to `guideline` within this step, before step 15.

Section 2 (at `a4d8cc9`) adds, for every chunk that touches `src/game/pad_*`, `src/devtools/pad_*` or `src/cmd/pad_author.c`: `make pad-author-proof` plus `make pad-author` followed by `git diff --exit-code worst-case/`. Every chunk of this step touches `src/devtools/pad_*`, so every chunk runs both.

Section 4 (Done when), which this tool measures, quoted:

> Measured with the step 14 tool on the re-authored `worst-case/` games, against the baseline of section 1: zero cheat activations, no dead D-pad press in jump mode and no dead ○ anywhere outside the group the locked rules require, and less than 25% of game time in aim mode.

Step notes this PRD relies on:

- Step 8: `pad_colonise` f293, "○ on the last object does nothing", is "the designed no-op for step 14".
- Step 12: "the modes are jump, aim, slipway, menus, placement, the bar and the cycle panels".
- Step 13: the generator (`pad_author`) writes a per-frame mode log and totals to `tmp/pad-author/<name>.modes.txt`; "the mode log's `effect` flag reads the press frame only, so a press whose effect lands later (menu △, Select, a fold's ×) is logged `effect 0`"; aim shares with and without browsing, and browsing frames, per game.

## Step 13 outcomes (checked at `8c9a663`)

The draft of this PRD was written before step 13 ended and assumed its outcome as hypotheses H-13-1 to H-13-10. Each was checked against the "### Step 13" note and the committed tree, and the generator was rebuilt and rerun on a clean export of `8c9a663` (`a4d8cc9` changes only the plan). The step note copies this list.

- **H-13-1 holds.** The generator is committed: a `cmd` file, a devtools module, `make pad-author` and `make pad-author-proof` outside `make test-all`, and one `.pad` script per re-authored game (four; `title_idle` has none and is unchanged). Rerunning each script gives byte-identical JSON. `make pad-author` with no `SCRIPT` rewrites the references in place; `make pad-author SCRIPT=worst-case/<game>.pad` writes to `tmp/pad-author/` and leaves them alone.
- **H-13-2 holds.** Same five files and `FULL_GAME_EVENTS`. Frames: `full_game` 6,024, `full_game_new_game` 6,037, `full_game_pause_idle` 6,269, `full_game_pause_restart` 6,002, all under 7,200. `title_idle` is 300 frames with no button down.
- **H-13-3 holds, under the generator's own view.** Cheat activations 0 in all four. The generator refuses a jump-mode D-pad press whose first direction has an empty cone, and counts 0 hold repeats.
- **H-13-4 fails.** The step 13 note gives per game only the aim share with and without browsing and the browsing frames, not frames per mode, presses per mode and button, hold repeats or required no-ops. Those totals exist only in the generator's mode log, regenerated on demand (H-13-1). **Change:** the reference totals are the generator's `total` lines, regenerated from the committed scripts; the values read at `8c9a663` are quoted in the facts below.
- **H-13-5 holds, with a caveat on `effect`.** The log is written on every run, one line per frame: `f<n> <mode> [<buttons>]`, plus ` pressed [<buttons>] effect <0|1>` on a frame with a press edge. Buttons are `+`-joined from `up down left right cross circle square triangle l1 r1 start select`; modes are `title pause placement bar tech score menu jump aim slipway`; totals follow as `total ...` lines. `effect` compares the view before and after the press frame only. **Change:** `effect` is never the dead/live verdict; the tool judges over the plan's window (D-4). In `full_game` the 93 `effect 0` presses are 35 aim × (folds), 40 jump Select (taps fire on release) and 18 jump △ (the menu opens later); in the `pad_colonise` proof the 9 are 4 aim ×, 2 jump △, 1 slipway ×, 1 jump Select and f293's ○.
- **H-13-6 fails in part.** The classifier reads only public functions, but it classifies **before** the frame's step (the mode the frame's input is read in), and its last two rungs are `slipway` when `pad_nav_slipway_start` is not NULL, otherwise `jump`; it never calls `pad_nav_jump_mode`. **Change:** the plan moves that classifier into `pad_metrics` unchanged, so the tool uses the generator's convention and order (D-3) and the logs compare with no shift.
- **H-13-7 fails as worded** (the classifier is a `static` function of the generator module, not reusable). **Superseded** by the plan's Method: the classifier, snapshot, change test, repeat counter, empty-cone check and totals writer all move to `pad_metrics`.
- **H-13-8 holds for the interfaces, fails on one detail.** `pad_nav.h` and `pad_jump.h` are unchanged since step 12. But not every step 13 `fix:` commit kept every e2e hash: `5537a8f` re-baselined `pad_build` and `pad_bankrupt` (one frame each now draws the refusal disk; end states unchanged). Nothing here depends on those frames; `pad_bankrupt.json.end-state` still records 1 cheat activation.
- **H-13-9 holds.** `pad_colonise.json` and its `.txt` are unchanged (445 frames, f293's ○ at the root on p47). The proof script `pad_colonise.frames.pad` regenerates it byte for byte and marks f293 `back --required`.
- **H-13-10 fails.** The aim shares are 24.7% / 24.6% / 23.7% / 24.8% of all frames, not under 20%: a `guideline` decision moved step 13's gate to section 4's 25% line. Without browsing they are 31.8% / 31.7% / 30.1% / 31.9% (browsing 1,379 / 1,379 / 1,367 / 1,379 frames); step 13's `guideline` ruled that checked clock-free browsing counts as play. The "no dead press under its own approximation" half holds (required no-ops 0; refusals for chords, empty cones and dead ○). **Change:** under G-4's denominator the margins are thin (facts below), so shares print to two decimals and the verdict is an exact integer comparison (D-6).

## Rulings and how they apply under the plan's Method

The five rulings below were made by `guideline` on the draft, before `a4d8cc9` added the Method. Where the Method fixes the point, the plan wins and the ruling is recorded as superseded.

- **G-1 (changed by guideline) — applied.** The verdict fails on any dead D-pad press in jump mode, in either group, and on any dead ○ in the other (non-required) group, in any mode. Dead hold repeats are reported apart (count and one line each, by group) and do not decide the verdict. The lenient count (dead D-pad presses in jump mode, other group only) is printed beside the strict one.
- **G-2 (ratified) — the standing answer to "a miss goes to `guideline`".** If section 4 misses on a `worst-case/` game, this step fixes that game's committed `.pad` script and regenerates its JSON through `make pad-author`; the JSON is never edited by hand. It reruns step 13's checks (`make worst-case-peaks`, `make full-game`, `make save-worst-case`, `make save-fuzz`, `make state`) and re-baselines only the affected `.end-state` references (delete, then `make end-state-baseline`), in one commit whose message names this extension of step 13's exception. A miss G-2 cannot fix by a script change (for example one needing a mapping change) goes to a fresh `guideline` agent, as the plan says, before step 15.
- **G-3 (ratified) — superseded; no accessor is added.** The Method moves the generator's hold-repeat counter into `pad_metrics`, and the tool's repeats must equal `pad_author`'s totals. A `pad_nav.h` accessor would be a second definition of a repeat, could disagree with the generator's count (it follows `repeat_kept`, the counter does not), and would put a change in `src/game/` that the Method does not ask for. Instead, the drift risk the ruling guarded against is pinned by a unit test: `tests/test_pad_metrics.c` drives the counter and `pad_jump_repeat_step` (public in `pad_jump.h`) with the same jump-mode hold sequences and asserts they fire on the same frames (18, 26, 34 of a hold; nothing on the press frame; a fresh press restarts the count). The known difference, a mode change mid-hold, is listed in the step note, not fixed.
- **G-4 (changed by guideline) — applied in the report only.** Section 4's denominator is in-game frames excluding title and pause (frames whose mode is neither `title` nor `pause`; the final score panel counts, as `menu`). The report also prints the aim share over in-game frames with pause and over all frames, and every mode's share over all frames. The generator's totals text stays byte-identical (the Method requires it), so its `total aim share` line keeps the all-frames figure. A later commit of this step makes the generator's aim gate use the same denominator through `pad_metrics`, so a regenerated game the generator accepts cannot miss section 4 on aim share; the four games pass both, so their JSON is unchanged.
- **G-5 (ratified fork-and-replay counterfactual) — superseded.** The Method fixes the rule: a press is dead when nothing in the change test moves between its press frame and 2 frames after its release. No fork, no second pass, no framebuffer digest. The residual risk is a false "live" when something moves on its own inside the window (an edge scroll still running after an earlier jump, an aim pan held with another button, a message box timing out). That can only hide a dead press, never invent one; it is measured, not designed away: each live press whose only moving fields are the camera is listed in the step note.

## Facts from the repository (read at `8c9a663`)

- **The replay loop** (`src/cmd/replay.c`): `input_step_run` (returns 1 when the cheat fired), `cart_update60`, `cart_draw` per frame, the `input_step_t` local to `main`. `STATE` (`state_write_on_exit` and a final `state_write` plus `cheat activations`), `REPLAY_FILL` and `SAVE_HEX` are read with `getenv` and acted on after the loop. The generator's `emit` runs the same frame sequence and self-checks its final state against `tmp/replay`.
- **The generator's bookkeeping** (all in its devtools module, all moving to `pad_metrics`):
  - **classifier** (static `current_mode`), first match wins: no economy → `title`; `pause_menu_shown` → `pause`; `pad_nav_placing` → `placement`; `pad_nav_build_bar_open` → `bar`; `pad_nav_tech_open` → `tech`; `pad_nav_score_open` → `score`; `pad_nav_panel_open` → `menu`; `pad_nav_aim_mode` → `aim`; `pad_nav_slipway_start` not NULL → `slipway`; else `jump`. Read before `input_step_run`; a press is attributed to that mode.
  - **view snapshot** (`pad_author_view_t`, public): mode; hovered, slipway start and target, build icon, placing and focus as entity ids; camera x, y; panel open, box open, message box; credits, year, year ticks; node, probe and tech counts.
  - **change tests**: `navigation_changed` (mode, the ids, camera, panel, box, message box) and `view_changed` (that plus credits, year, year ticks, nodes, probes, techs). The clock advances only on actions (`economy_advance_clock` from probes, slipways and colonies), so no field of `view_changed` but the camera moves on a still game.
  - **hold-repeat counter** (`count_hold_repeats`): per direction, raw hold length; a repeat counts when the frame's mode is `jump` or `slipway` and the hold is at 18 + 8k frames. It ignores `repeat_kept` and the first-direction rule. Its `direction_held[4]` is the array the Method sizes by `PAD_JUMP_DIRECTIONS`.
  - **empty-cone check** (in `emit`): in jump mode, `pad_target_pick(hovered, first direction, live candidates)` NULL is refused. Candidate lists are built in a static 64 KiB scratch zone, never `frame_zone()`, so `make state`'s peaks do not see them.
  - **dead-○ check** (`settle_circle`): `navigation_changed` within `CIRCLE_EFFECT_FRAMES` (2) after the press, or a refusal unless `back --required` (then counted as a required no-op).
  - **totals writer** (`pad_author_write_totals`, public): `frames`, `mode <m> frames`, `aim share` (over all frames, one decimal), `presses <m> <b> <n>`, `hold repeats`, `required no-ops`, `cheat activations`, each after a prefix.
  - The state held between frames is `pad_author_t`'s `previous_buttons`, `direction_held`, `mode_frames`, `presses`, `hold_repeats`, `required_noops` and the pending-○ fields (`circle_before` is a view of ids, already pointer-free).
- **Timing of the delayed effects:** Select fires its tap on the release frame (a press of at most 10 frames) and its year at frame 30 of a hold; a fold's × confirms after a 16-frame hold; menu △ opens its panel on a later frame; a slipway × builds on release. All inside "press frame to release + 2" except possibly menu △, which the verification must show.
- **The picker and the root rung:** `pad_target_pick` returns NULL for an empty cone; `pad_jump_candidates` and `pad_slipway_candidates` build lists in a caller-given zone; a press of several directions moves along the first in Up, Down, Left, Right order. `pad_back_target` gives NULL when nothing is recorded, when the recorded object is hovered, or when it is gone; `pad_nav_last_acted` exposes the recorded object.
- **Guards over `src/`**: float (and `make float-link` over the replay's objects), alloc (strict list), pointer (strict list), random and the PICO-8 API guard.
- **Generator totals at `8c9a663`**:

  | events | all | title | pause | jump | aim | slipway | menu | placement | bar | tech | score |
  |---|---|---|---|---|---|---|---|---|---|---|---|
  | `pad_colonise` (proof) | 445 | 13 | 0 | 106 | 255 | 51 | 20 | 0 | 0 | 0 | 0 |
  | `full_game` | 6,024 | 13 | 0 | 2,729 | 1,487 | 389 | 730 | 293 | 114 | 209 | 60 |
  | `full_game_new_game` | 6,037 | 24 | 0 | 2,729 | 1,487 | 389 | 732 | 293 | 114 | 209 | 60 |
  | `full_game_pause_idle` | 6,269 | 13 | 301 | 2,696 | 1,487 | 389 | 707 | 293 | 114 | 209 | 60 |
  | `full_game_pause_restart` | 6,002 | 26 | 9 | 2,700 | 1,487 | 389 | 707 | 293 | 118 | 211 | 62 |

  Hold repeats 0 and cheat activations 0 everywhere; required no-ops 1 for the proof, 0 for the four games. Presses 1,257 / 1,260 / 1,254 / 1,264 for the games. `full_game` never presses ○ in jump mode.
- **Aim share under G-4**, from that table: `full_game` 1,487 / 6,011 = 24.74%; `new_game` 1,487 / 6,013 = 24.73%; `pause_idle` 1,487 / 5,955 = 24.97%; `pause_restart` 1,487 / 5,967 = 24.92%. All pass, `pause_idle` by under two aim frames (25% of 5,955 is 1,488.75); a one-decimal print would show it as 25.0%.
- **`pad_colonise`** facts the step note uses to explain each dead press: the title × at f12; □ into aim at f13 (aim from f14 under D-3); ○ back to jump at f16, □ at f17; □ to jump on p47 at f86; × opens the menu at f88 and colonises at f98; △ and ○ on the inspection panel at f135–144; aim from f146, left by ○ at f173; the slipway at f273–291; **○ at the root on p47, the last object, at f293**; the HUD-aimed slipway at f345–378; the Select hold at f412–441. Its first moves include a two-frame Down+Right diagonal (in aim mode).

## Problem Statement

From the plan's point of view, section 4 cannot be checked. Its four measures are cheat activations, dead D-pad presses in jump mode, dead ○ outside the required group, and the aim share. They need a tool that classifies every frame and judges every press of a whole game, on any events file, and no such tool exists.

- Section 1's baseline came from a throwaway trace of the old `pad_nav_t`.
- Step 13's generator already does the bookkeeping, but only while it writes a game from a script, and its `effect` flag reads the press frame only. Its classifier and counters are private to it.

From the human's point of view, after the plan they will play a game and record it (section 4's last line). The same tool must measure that recording, and only an events file exists then, no script.

From the maintainer's point of view, a later tuning change (section 5) needs a cheap way to see whether a regenerated game still meets section 4. The four games pass the aim line by 2 to 16 aim frames, so a tuning change can tip them over.

## Solution

- The generator's mode classifier, view snapshot, change tests, hold-repeat counter, empty-cone check and totals writer move, unchanged in behaviour, into a shared devtools module `pad_metrics`, which the generator calls. The generator's output stays byte-identical.
- `pad_metrics` gains what the tool needs on top: press and repeat events with their windows, the dead rule over the window, the two groups, per-mode and per-button dead counts, and the report with section 4's verdict.
- The tool is the replay itself: `PAD_METRICS=<path>` makes `tmp/replay` call `pad_metrics` once per frame and write the report to that path at the end. `make pad-metrics EVENTS=<file>` runs it. With the variable unset, the replay does nothing new.
- It is proven on `pad_colonise` through the generator's proof run (totals equal; the first group is exactly f293), then on the four `worst-case/` games (totals equal; no Select tap, menu △ or fold × dead), and then runs section 4's measurement on those four games for the step note.

## User Stories

1. As a plan reader, I want one command that tells me whether a game meets section 4, so that "Done when" is checked mechanically and not by eye.
2. As a plan reader, I want the frames and share per mode (jump, aim, slipway, menu, placement, bar, tech, score, pause, title), so that I can compare them with section 1's 61% in free-camera modes.
3. As a plan reader, I want the aim share over in-game frames without title and pause as the verdict figure, so that "less than 25% of game time" has one meaning.
4. As a plan reader, I want the aim share with pause and over all frames printed too, so that the generator's figure and any other reading come from the same run.
5. As a plan reader, I want shares printed to two decimals and the verdict computed exactly, so that 24.97% is neither shown as 25.0% nor judged by rounding.
6. As a plan reader, I want dead presses counted per mode and per button, so that I can see where the scheme still wastes presses.
7. As a plan reader, I want dead presses split into the required group and the rest, so that a no-op the locked rules impose does not count against the scheme.
8. As a plan reader, I want hold repeats counted apart from presses, so that a long hold does not inflate the press count or hide a dead press.
9. As a plan reader, I want dead hold repeats judged and grouped but kept out of the verdict, so that a hold running into nothing is visible without failing the game.
10. As a plan reader, I want cheat activations and chord frames reported apart, so that a deliberate cheat in an e2e is not counted as a pile of dead × and ○ presses.
11. As a plan reader, I want the verdict to fail on any dead D-pad press in jump mode, empty cones included, so that section 1's complaint cannot come back under the required group.
12. As a plan reader, I want the lenient count printed beside the strict one, so that a later ruling can change the verdict without a rerun.
13. As a plan reader, I want the verdict to fail on a dead ○ outside the required group in any mode, so that ○ always goes back one step.
14. As a plan reader, I want a verdict block that names each failing measure, so that a red result says what to fix.
15. As the human after the plan, I want to run the tool on my own recorded game, so that my complaints can be put next to numbers.
16. As the human, I want the tool to accept a recording with an `initial_save`, so that a resumed session can be measured too.
17. As a maintainer, I want the tool to be a hook of the replay and not a second frame loop, so that what it measures is exactly the game the e2e and state tools replay.
18. As a maintainer, I want `make e2e` and `make test` no slower, with the hook off when its variable is unset, so that the fast loop stays fast.
19. As a maintainer, I want one module holding the classifier, snapshot, change test, repeat counter, empty-cone check and totals for both the generator and the tool, so that the two can never disagree on what a mode, a press or a repeat is.
20. As a maintainer, I want the extraction proven pure by byte-identical `worst-case/` JSON, unchanged mode logs and totals, and `make pad-author-proof`, so that the generator is not silently changed.
21. As a maintainer, I want `pad_metrics` to keep entity ids between frames, never pointers, so that an entity freed between frames cannot leave a dangling reference.
22. As a maintainer, I want `pad_metrics` to take its scratch zone from its caller, so that neither the generator nor the replay touches `frame_zone()` and `make state`'s peaks stay as they are.
23. As a maintainer, I want the classifier and change test unit-tested on hand-built states in milliseconds, so that a regression is caught on every `make test`.
24. As a maintainer, I want the repeat counter tested against `pad_jump_repeat_step`, so that the copy cannot drift from the game's rule unseen.
25. As a maintainer, I want the generator's aim gate to use section 4's denominator, so that a regenerated game the generator accepts never misses section 4 on aim share.
26. As a maintainer, I want `pad_colonise`'s first group to hold exactly f293's ○, found from game state, so that the designed no-op of step 8 proves the first group without reading the script.
27. As a maintainer, I want every other dead press in `pad_colonise` explained from its `.txt` in the step note, so that each one is understood or recognised as a finding.
28. As a maintainer, I want the tool's totals for the proof and the four games equal to `pad_author`'s, so that the tool and the generator are reconciled.
29. As a maintainer, I want no Select tap, menu △ or fold × counted dead on the four games, so that the window is shown long enough for the delayed effects step 13 named.
30. As a maintainer, I want the tool to never write to the save storage on disk, never open a window and never launch the raylib app, so that it follows `CLAUDE.md`'s headless rule.
31. As a maintainer, I want a `p8_fatal` or an unwritable report to fail the run loudly, so that a broken measure never looks like a clean one.
32. As the agent of this step, I want a section 4 miss handled by G-2's regeneration, or by `guideline` when a script fix cannot help, so that the step finishes before step 15.
33. As a reviewer, I want the step note to put each game's section 4 figures next to section 1's baseline, so that the plan's goal is visibly met or missed.
34. As a reviewer, I want the hook's cost on the longest game recorded, so that its price is known before anyone wires it into a loop.
35. As a reviewer, I want the percentages computed with integer arithmetic, so that the float guard and `make float-link` stay green.
36. As a reviewer, I want each `H-13-k` outcome and each superseded ruling in the step note, so that the PRD's assumptions are audited.
37. As a future tuner (section 5), I want to regenerate a game and rerun the tool with two commands, so that a tuning change is judged against section 4 in minutes.

## Implementation Decisions

### D-1. Modules

- **`pad_metrics` (new devtools module).** Two layers inside one module:
  - **Moved from the generator, behaviour unchanged:** the mode enum and names (order and spelling kept), the view snapshot (ids only), `navigation_changed` and `view_changed` as the change tests, the hold-repeat counter (its array sized by `PAD_JUMP_DIRECTIONS`), the empty-cone check (scratch zone from the caller), and the totals writer with its prefix argument. The classifier is split into a read of plain flags from an `input_step_t`'s pad state through public functions, and a pure function from those flags to a mode, so it is testable on hand-built states.
  - **New for the tool:** press, repeat and chord events; the window and dead rule (D-4); the group facts and rule (D-5); dead counts per mode, button, kind and group; the report writer and section 4 verdict (D-6); the G-4 aim test (`4 × aim < in-game frames`).
  - Its state is one struct the caller owns (statically, as the generator's `pad_author_t` is). Between frames it holds only ids, counters and the previous snapshot.
  - Interface, in prose: initialise with a scratch zone; observe the state once before the first frame; record one frame after it ran, given the raw buttons and the cheat return; write totals; write the report. The generator's `emit` uses the same observe and record calls around its own step, plus its refusals, which read the module's classifier and empty-cone check before the step.
- **The generator** keeps its script runner, commands, refusals and `settle_circle`, now calling `pad_metrics`. `pad_author_t` loses the moved fields for a `pad_metrics` struct. `pad_author_mode_t` and `pad_author_view_t` are replaced by `pad_metrics`'s types, and `pad_author_write_totals` by the module's writer (same text).
- **`src/cmd/replay.c`**: a `PAD_METRICS` hook beside `STATE`, `REPLAY_FILL` and `SAVE_HEX`. When set: observe after `cart_init`, record after each frame's `cart_draw`, write the report after the loop. Its scratch zone is a static buffer in the replay command. No new loop, no fork. With the variable unset no `pad_metrics` call is made.
- **Nothing changes in `src/game/`, `src/engine/` or `src/p8/`** (G-3 superseded).

### D-2. Events

- **A press edge** is a button bit newly down (`buttons & ~previous`): Up, Down, Left, Right, ×, ○, □, △, L1, R1, Start, Select. Its release frame is the first later frame with the bit up, or the end of the file.
- **A repeat** is a frame where the moved counter fires (mode `jump` or `slipway`, hold at 18 + 8k). Its button is that direction. Repeats are counted exactly as the generator counts them, so the totals agree.
- **A chord** is a frame where × and ○ are both newly down. Its × and ○ edges are chord events, counted apart, never judged. Cheat activations come from `input_step_run`'s return.

### D-3. Modes

- The mode of frame f is the classifier's verdict on the state before frame f's step (the snapshot recorded after frame f−1, or the initial observation for f0). An event is attributed to its frame's mode. Per-mode frames, presses and repeats therefore equal `pad_author`'s totals on a file it wrote.
- The plan's "menus" is `menu` (planet menu, inspection panel, final score panel); "the cycle panels" are `tech` and `score`; `pause` and `title` get their own rows.
- Known edge, accepted: when a slipway's start dies mid-slipway, `pad_nav_slipway_start` reads NULL and the classifier says `jump` while `pad_nav` is still in slipway mode. The classifier is shared and moved unchanged, so the tool keeps it.

### D-4. Dead or live (the Method's rule)

- A press of B at frame f, released at frame r, is **dead** when nothing in the change test (`view_changed` against the snapshot before f) moves from f through r + 2 (clipped at the end of the file). The window ends early, just before the first press edge of any other button that comes after r. A press edge on r itself is a roll and does not cut it. While B is held, the window is not cut, and the existing cut at B's first repeat still applies. A repeat's window is unchanged. Otherwise the press is live. The `+ 2` is a named constant of the module, and the early end is one condition beside it (G-6, `guideline`, during chunk 3: under the bare release + 2 window, `pad_colonise`'s f293 ○ read live from f295's □).
- For a direction, the press's window is cut before its first repeat, so a later repeat that moves the cursor cannot make a dead press look live.
- **A repeat** at frame t is judged the same way, its window from t to the next repeat of B (excluded) or r + 2, whichever comes first.
- Events are settled in frame order as their windows close; an event whose window reaches the end of the file is settled there.
- `navigation_changed` keeps its single use, the generator's dead-○ refusal, unchanged.
- If a Select tap, menu △ or fold × comes out dead on the four games (the plan says none may), the agent finds from the frames when its effect lands and sends the window question to `guideline`; the window constant is not changed on the agent's own call, since the plan fixes it.

### D-5. The required group (found from game state)

The snapshot taken before each frame also records, outside the change test (so the generator's outputs do not move): `pad_nav_last_acted`'s id, and, when the mode is `jump` or `slipway`, one empty-cone flag per direction (`PAD_JUMP_DIRECTIONS` entries), computed with the caller's scratch zone from the hovered object over the jump candidates in jump mode, or the aimed target (or the start) over the slipway candidates in slipway mode.

A dead event is in the required group exactly when one of the locked rules' no-ops matches on the snapshot before it:

- **○ at the root:** the mode is `jump`, with no shown message box and no box open, and the last-acted id is none or equals the hovered id.
- **A D-pad press or hold repeat into an empty cone:** the mode is `jump` or `slipway`, the button is the frame's first direction in Up, Down, Left, Right order, and its empty-cone flag is set.

Every other dead event is in the other group (for example a diagonal's second direction in jump mode, △ in aim mode, ○ in jump mode with the last object elsewhere). Nothing reads the script: on `pad_colonise`, f293 must land in the first group from these facts alone.

### D-6. Report and exit status

- **Plain text, one fact per line, stable order.** Shares are percentages with two decimals, by integer arithmetic (basis points, rounded half up). The report holds:
  - frames total, in-game frames without title and pause, in-game frames with pause;
  - per mode: frames and share of all frames;
  - the aim share over in-game frames without title and pause (the verdict figure), with pause, and over all frames;
  - per mode and button: presses, repeats, chords, live, dead-required, dead-other (presses and repeats on separate lines);
  - one line per dead event (frame, kind, button, mode, group) and one per repeat (frame, button, mode, verdict);
  - the module's totals block, prefixed `total `, byte-for-byte the generator's format, so `grep '^total '` of both outputs diffs directly (the `required no-ops` line is the tool's first-group ○ count; the generator counts `back --required` ○, which on these files must be the same frames);
  - the section 4 block: cheat activations (ok at 0); dead D-pad presses in jump mode, strict count (decides) and lenient count (printed); dead ○ in the other group, any mode; dead hold repeats by group (marked as not deciding); the aim share against 25% by `4 × aim < in-game frames`; an overall `ok` or `fail`.
- **Exit status of the replay** is unchanged by section 4's verdict: the report states it, and `make pad-metrics` prints the verdict line and fails when it reads `fail` only if `CHECK=1` is given. An unwritable report path fails the replay (non-zero), as an unwritable `STATE` path does.

### D-7. `make` targets and cost

- `make pad-metrics EVENTS=<file> [CHECK=1]`: replays the file with `PAD_METRICS=tmp/pad-metrics/<name>.txt` and prints the report. Outside `make test-all`, with a legend line.
- `make pad-metrics-worst-case`: the same for the four re-authored games (`FULL_GAME_EVENTS` minus `title_idle`), `CHECK=1`. This is the plan's last check.
- No hand-written expected report: the Method's verification is against `pad_author`'s totals and the proof's `back --required` frame, which a short script in the target can diff (`make pad-metrics-check`: run `pad_colonise.frames.pad` through the generator to `tmp/`, run the tool on `pad_colonise.json`, diff the `total` lines, and check the first group is exactly one ○ at f293).
- Cost: the hook adds a snapshot, a classification and, in jump or slipway mode, up to four cone picks per frame. On a 6,000-frame game that is well under a second; the step note records it. `make e2e` and `make test` wall times are measured like for like before and after.

### D-8. Verification and the last check

- **`pad_colonise`:** the tool's `total` lines equal the proof run's (mode frames, presses per mode, hold repeats 0, cheat 0, required no-ops 1); the first group holds exactly f293's ○. Every dead press in the other group is listed in the step note with its frame, button and mode and explained from the `.txt`. The proof's 9 `effect 0` presses are listed with the tool's verdict (only f293 may be dead).
- **The four games:** `make pad-author SCRIPT=worst-case/<game>.pad` (references untouched) and the tool give equal `total` lines; no Select tap, menu △ or fold × is dead; every generator `effect 0` press is listed with the tool's verdict.
- **Last check:** `make pad-metrics-worst-case`, each game's section 4 figures in the step note. A miss follows G-2, or goes to `guideline` when a script fix cannot help, before step 15.

## Implementation Chunks

Each chunk ends on a commit with `make test-all` green, `make raylib` linking, `make pad-author-proof` passing and `make pad-author` followed by `git diff --exit-code worst-case/` clean (section 2). Every e2e hash and `.end-state` stays unchanged. If G-2 applies, the `worst-case/` references are the only ones that change, in chunk 4.

1. **The pure refactor (the Method's first commit).**
   - Create `pad_metrics` with the moved classifier (split into flag read and pure classify), snapshot, change tests, repeat counter, empty-cone check and totals writer; `emit` calls it; `pad_author_t` holds a `pad_metrics` struct; ids between frames; `PAD_JUMP_DIRECTIONS`; caller's scratch zone.
   - `tests/test_pad_metrics.c`: classify on hand-built flag sets (each rung, the first-match order, the `jump` remainder), the change tests on hand-built snapshots (each field moves it; the D-5 extras do not), and the repeat counter against `pad_jump_repeat_step`.
   - Check: the section 2 gate, every `tmp/pad-author/*.modes.txt` and the totals output byte-identical before and after (saved copies diffed), `make test` not slower.
2. **The tool's logic in `pad_metrics`, and the generator's aim gate.**
   - Events, windows, the dead rule, the group facts in the snapshot, the groups, dead counts, the report and the section 4 block; the G-4 aim test.
   - Tests on hand-built frame and snapshot sequences: press edge and release; the window to release + 2, cut at the first repeat and at the end of the file; repeat windows; a diagonal's second direction; chords apart; root ○ (nothing recorded; hovered equals last) and empty-cone cases in jump and slipway mode, with an "other" case for each; two-decimal percentages (24.97% does not print as 25); the verdict failing on a required-group dead D-pad press in jump mode and on an other-group dead ○, passing with only required ○ and dead repeats, printing the lenient count.
   - The generator's aim gate switches to the aim test, as a separate commit. Check: the section 2 gate (the four games pass both denominators).
3. **The replay hook and `make pad-metrics`.**
   - `PAD_METRICS` in the replay command, the targets of D-7, legend lines.
   - Proof: a throwaway replay in `tmp/` (never committed) taps ○ at game start (nothing recorded) and a direction into an empty cone; both land in the first group and the verdict fails on the tap. `pad_bankrupt` shows its chords apart and 1 cheat activation. `make e2e` is not slower with the variable unset.
   - `pad_colonise` verification (D-8) and `make pad-metrics-check`.
4. **The four games, the last check and the step note.**
   - D-8 on the four games; `make pad-metrics-worst-case`; any miss per G-2 or `guideline`.
   - Measure the hook's cost and `make test` and `make e2e` like for like.
   - Write "### Step 14" in `docs/plan-12.md`.

## Testing Decisions

- **What a good test is here:** it drives `pad_metrics` through its public interface (flags, snapshots, button frames in; modes, events, groups, totals and report text out) and asserts observable results, never internal fields.
- **`tests/test_pad_metrics.c`** (in `make test`, milliseconds, as `test_pad_target`), covering the lists in chunks 1 and 2. Hand-built states are plain flag sets and snapshots; no cart runs.
- **The repeat counter against the game:** the same test file drives `pad_jump_repeat_step` (public) and the counter with identical jump-mode holds; this is the drift guard G-3 asked for.
- **End to end:** `make pad-metrics-check` on `pad_colonise`, the section 2 gate on every chunk, and the byte-identical generator outputs of chunk 1. The hook itself has no unit test: it is three calls in the replay, covered by those runs.
- **Prior art:** `tests/test_pad_target.c` (hand-built layouts), `tests/test_end_state.c` (a tool's text logic), `tests/test_pad_select.c` (a test pinning a copy to its source), the replay's `STATE` and `REPLAY_FILL` hooks, the generator's self-check against `tmp/replay`.

## Out of Scope

- Changing any mapping, tuning value or game rule, or anything in `src/game/`, `src/engine/` or `src/p8/`.
- Changing the generator beyond moving its bookkeeping into `pad_metrics` and the aim gate's denominator: its commands, refusals, `effect` flag, `settle_circle` and output text stay as they are.
- Re-authoring any e2e replay or changing its `.txt`, hash, fixture or `.end-state`.
- A fork-and-replay counterfactual or a framebuffer digest (G-5 superseded by the Method).
- Detecting "browsing" in a recording (a script notion of step 13).
- Measuring section 1's human recording (old scheme, not in the repository).
- The human's game after the plan; the README (step 15) and the comment rework (step 16).
- Wiring the tool into `make test`, `make e2e` or CI. Audio, PSX, mouse input, launching the raylib app.

## Further Notes

- **The step note** ("### Step 14") records: each `H-13-k` outcome; the rulings G-1 to G-5 and which the Method superseded; the refactor's byte-identical evidence; the D-3 convention and the slipway-start edge; the repeat counter's known difference from `repeat_kept`; the throwaway first-group case; the `pad_colonise` dead list explained from the `.txt`; the `effect 0` lists with the tool's verdicts; live presses whose only moving field was the camera; per worst-case game, the section 4 table next to section 1's baseline (aim share three ways, next to section 1's 61% in free-camera modes and step 13's with/without browsing; dead D-pad presses in jump mode strict and lenient; dead ○ in the other group, 12 in section 1's network mode; dead repeats; cheat activations); any G-2 regeneration or `guideline` ruling; the hook's cost; `make test` and `make e2e` times before and after.
- **Docs:** none. The usage comment in the replay command and the Makefile legend describe the hook. The README Controls section is step 15's.
- **Risk: the aim margin.** `full_game_pause_idle` passes by under two aim frames. If the tool's `total` lines differ from the generator's by even one frame, the convention is wrong before the game is: the totals diff must be clean first. A real miss goes to G-2 (fewer aim frames or more play in the script), never to a different denominator.
- **Risk: false live.** The window rule cannot see a dead press made while the camera still scrolls from an earlier jump. The step note lists every live press whose only moving field was the camera, so a reader can judge how many dead presses this could hide; changing the rule is a `guideline` question, not this step's call.
