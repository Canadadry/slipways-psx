---
title: "README Controls section for the jump-cursor scheme"
description: "The README's Controls section still describes the tab ring, the end-turn tab and a red refused line, and says nothing about Select: rewrite it for the scheme steps 5 to 12 built, with every key and every rule traced to keymap.cfg, the code and a test or e2e frame."
status: needs-triage
---

## Plan step (verbatim)

Plan file `docs/plan-12.md` (at `5ee926f`), Phase 5 — Finish, step 15:

> 15. **README:** the Controls section rewritten for the new scheme (table plus one line per mode: jump, aim, slipway, menus). Method: the key columns come from `keymap.cfg`, the AZERTY column from the US key positions; each rule line comes from the behaviour in the step 5–12 notes (for example △ does nothing in aim mode, R1 from the bar opens tech, the 15-frame year chain); the step note lists, for each line, the unit test or e2e frame that shows it.

Section 4 (Done when), the line this step closes: "The README Controls section describes only the new scheme."

## Step 14 hypotheses

Step 14 (pad metrics tool) is not done when this PRD is written; its PRD is committed at `5ee926f` (`docs/prd/triage/phase5-step14-pad-metrics-tool.md`). Every assumption about its outcome is listed here. The implementer checks each one first (chunk 1), and the step note copies the list with "holds" or "fails" and what changed.

- **H-14-1.** Step 14 changes no mapping, tuning value or game rule: nothing in `src/game/`, `src/engine/` or `src/p8/` changes (its PRD's D-1 and Out of Scope). *Check:* `git diff 5ee926f -- src/game src/engine src/p8` is empty, or touches only comments. *If it fails:* every rule line touched by the diff is re-derived from the code and the step 14 note, and its evidence from the tests that commit added or changed.
- **H-14-2.** Step 14 does not change `keymap.cfg`, the built-in keymap text in the platform keymap module, or its name tables. *Check:* `git diff 5ee926f -- keymap.cfg src/platform` is empty. *If it fails:* the table is rebuilt from the new `keymap.cfg`, by the same rules (D-2).
- **H-14-3.** Step 14 does not edit `README.md` (its PRD: "Docs: none"). *Check:* `git diff 5ee926f -- README.md` is empty. *If it fails:* step 15 starts from the edited README and keeps whatever step 14 added outside the Controls section.
- **H-14-4.** Step 14 re-authors no e2e replay and rewrites no e2e `.txt` (its PRD's Out of Scope): every frame this PRD cites still shows what the cited `.txt` line says. A G-2 regeneration touches only `worst-case/`, which this step does not cite. *Check:* `git diff 5ee926f -- tests/e2e` is empty. *If it fails:* each cited frame is re-read from the new `.txt`; a frame whose situation moved is cited at its new frame, and one whose situation is gone is replaced by a unit test.
- **H-14-5.** Step 14 deletes and renames no test in `tests/test_pad_*.c` or `tests/test_keymap.c`; it only adds `tests/test_pad_metrics.c`. *Check:* every test name in the evidence list (D-5) is still defined and registered. *If it fails:* the renamed test is cited under its new name; a deleted one is replaced by another test or an e2e frame showing the same behaviour.
- **H-14-6.** Section 4 is met on the four `worst-case/` games, or any miss is fixed by G-2 (a `.pad` script change and a regeneration). No `guideline` ruling in step 14 changes a mapping or a tuning value before step 15. *Check:* the "### Step 14" note. *If it fails:* that ruling's mapping is the scheme step 15 describes, H-14-1's fallback applies, and the step note names the ruling.
- **H-14-7.** No step 14 finding contradicts a behaviour the step 5–12 notes record. A Select tap, menu △ or fold × found dead over step 14's window is a window question for `guideline`, not a behaviour change. *Check:* the "### Step 14" note's dead-press lists and rulings. *If it fails:* the README follows the code as it stands at step 15's start, and the step note records the difference from the step 5–12 notes.
- **H-14-8.** The "### Step 14" note exists in section 6 of the plan, and the text of step 15 in section 3 is the one quoted above. *If it fails:* if step 14 has not ended, step 15 does not start. If the step 15 text changed (a plan-review round), the PRD follows the plan's new text, and the step note says where they differ.
- **H-14-9.** Step 15 needs nothing from the step 14 tool. No README line quotes a metric, and the evidence is unit tests and e2e frames, as the plan says. *If it fails:* nothing to do. This hypothesis records that step 15 does not depend on whether `make pad-metrics` exists.

## Facts from the repository (read at `5ee926f`)

- **The current Controls section** has a table (Button, Keyboard (QWERTY), AZERTY, Gamepad) with rows D-pad, × confirm, ○ back, □, △ planet menu, "L1 / R1 tabs", Start pause. It has no Select row. The line "Keys can be changed in `keymap.cfg`." follows the table, then bullets. Four bullets are stale since step 12:
  - "Tabs: L1/R1 switch between map, build, end turn, tech and score": the ring is gone.
  - "Build tab": the bar is not a tab. It opens on L1, and L1 or ○ closes it.
  - "End turn tab: × ends the year": the year ends only by the Select hold (step 12).
  - "The line turns red if it can't be built": wrong. The refused line is drawn in colour 2, and the reason appears above the target: the colour-8 "-too far-", or the forbidden sign on an already linked target (`pad_slipway.txt` f263–277, `pad_colonise.txt` f283). The colour-8 cost label shows on a target that can be built (`pad_slipway.txt` f249).
  - It also omits △ again closing a menu, ○ at the root, × on a known planet, R1's cycle, Select and Start's limits.
- **`keymap.cfg`** (the same text as the built-in default, pinned by `test_versioned_file_is_the_default_text_and_loads_as_the_defaults`): up/down/left/right = arrow keys and the left face; cross = X, RIGHT_FACE_DOWN; circle = C, RIGHT_FACE_RIGHT; square = Z, RIGHT_FACE_LEFT; triangle = V, RIGHT_FACE_UP; l1 = A, LEFT_TRIGGER_1; r1 = S, RIGHT_TRIGGER_1; start = ENTER, MIDDLE_RIGHT; select = RIGHT_SHIFT, MIDDLE_LEFT. Its header says letter keys are key positions named after the US layout. Reserved keys (refused in the file): SPACE, PAGE_UP, PAGE_DOWN, END, ESCAPE.
- **AZERTY from the US positions:** the US Z position is W on AZERTY, and A is Q. X, C, V, S, Enter, Right Shift and the arrow keys sit at the same positions.
- **The game runs at 60 updates per second** (`SetTargetFPS(60)`, `cart_update60`). The Select constants are tap ≤ 10 frames, hold 30 frames and chain window 15 frames (`PAD_SELECT_*`). Jump repeat is 18 frames, then every 8. The aim pan is 1, 2 or 4 px per frame after 10 and 40 frames held.
- **Behaviour, read in the pad navigation module at `5ee926f` and matching the step 5–12 notes:**
  - *Jump mode* (the map's root): the D-pad jumps through the cone picker over the whole map. With an empty cone nothing moves. A held direction repeats. The camera edge-scrolls only to bring the target inside the inner rectangle. × on a colony or structure that can start a slipway starts one; a refused start (a full slipgate) presses nothing. × on a known planet opens its projects menu and records it as the last object. △ opens the hovered object's menu (a known planet's projects, a colony's inspection panel), and △ again closes it. △ and × wait while the object is still scrolling in or under a HUD item. □ enters aim mode, the camera unmoved. ○ jumps to the last object × acted on, and does nothing when nothing is recorded, when that object is already hovered or when it is gone.
  - *Aim mode*: a crosshair at the centre. The D-pad pans, faster the longer it is held. × on empty space launches the first probe; with a probe out, a × held 16 frames folds space and a shorter one abandons the fold. × over a planet presses nothing. △ does nothing. □ or ○ goes back to jump mode on the target nearest the centre, and both wait while × holds a fold.
  - *Slipway mode*: the D-pad aims, start excluded. × on the target builds it or is refused by the game's rules. After a build the next slipway starts from the target; after a refusal it aims again from the same start. ○ drops it, back in jump mode on the start. L1, R1, Start and a box opening also cancel it. △, □ and Select do nothing here.
  - *Build bar* (L1, from the map in any mode, tech or score; it closes a planet menu or cancels a held slipway on the way): Left/Right pick a revealed structure, without wrapping; Up, Down, □ and △ press nothing. × takes the structure. L1 or ○ closes the bar; R1 closes it and opens tech on the same frame.
  - *Placement*: the structure stays at the centre and the D-pad pans under it. × places it, or the game refuses the drop, which kills the structure and charges nothing. ○ cancels it. A structure taken from jump mode leaves you in aim mode, so a second ○ returns to jump mode. L1 while carrying cancels and opens the bar. Start suspends the carry under pause, and closing pause resumes it.
  - *R1*: from the map or the bar, tech; on tech, score; on score, closed. ○ on tech or score returns to the map in one press. L1 on either opens the bar. In tech, the D-pad and × pick an invention; the score panel during play has no row.
  - *Select* (on the map, in jump or aim mode, with no menu, slipway, carry or held fold): a tap jumps to the next colony or structure whose panel lists an unmet need, in order, wrapping. From aim mode it lands in jump mode. With none, nothing happens. Holding 30 frames ends the year once, however long it is held. For 15 frames after a year, a fresh press ends one more year at once (the chain, step 9's D-1). A hold under 30 frames ends nothing.
  - *Start*: pause, from the map in any mode or from the bar. Ignored on the title, under a planet menu, on tech and score, and while × holds a fold. In pause the D-pad moves among the rows, × picks a row, and Start or ○ closes it.
  - *○ order*: (1) pause, then a box, then the shown message box (the year box) on the map; on the bar, tech or score, back to the map; (2) a slipway or a placement; (3) aim mode; (4) at the root, the jump to the last object. One rung per press.
  - *Menus and panels*: the D-pad moves, × picks, ○ closes. The × that opened a menu (or was held into pause or a panel) picks nothing until it is pressed again. ○ does nothing on the title. The final score panel ignores the shoulders, ○ and Start; × on "again?" goes to the title.
  - *The cheat* is four × + ○ chords (step 6). It is a dev tool, not a control, and the README does not mention it today.

## Problem Statement

A player opening the README to learn the controls is told about a tab ring that no longer exists, an end-turn tab whose × ends the year, and a red line that never turns red. Nothing tells them that Select exists, that it finds colonies needing attention, or that holding it ends the year. The rules they actually meet (△ again closes a menu, ○ jumps back to the last object, R1 cycles tech and score, L1 toggles the bar, Start only pauses from the map or the bar) are scattered across the step notes of a plan they will never read.

For the plan, section 4's line "The README Controls section describes only the new scheme" is unmet. The plan also asks that each written rule be backed by a test or frame, so a later change cannot silently make the README wrong.

## Solution

Rewrite only the Controls section of the README:

- A table with one row per pad button (D-pad, ×, ○, □, △, L1, R1, Start, Select) and the columns Keyboard (QWERTY), AZERTY and Gamepad, taken from `keymap.cfg` by fixed rules.
- The line saying keys can be changed in `keymap.cfg`.
- The scheme's rule in one line, then one line per mode and per button rule: jump, aim, building a slipway, the build bar and placement, tech and score, Select, menus and pause, and ○'s back order. Each line says only what the code does at step 15's start.

Then write the "### Step 15" note in the plan: the table derivation, and for every README line the unit test or e2e frame that shows each of its claims, plus the H-14 outcomes.

## User Stories

1. As a new player, I want one table that names every pad button with its keyboard key, so that I can start playing without guessing.
2. As a player on an AZERTY keyboard, I want an AZERTY column, so that I press W for □ and Q for L1 without reading raylib's key names.
3. As a gamepad player, I want each button named by its position (bottom, right, left, top, shoulders, middle buttons), so that it works whatever my pad's labels are.
4. As a player, I want a Select row in the table, so that I find the button that ends the year.
5. As a player, I want the table to say what each button is for in a word or two, so that the table alone is a usable reminder.
6. As a player, I want to be told that keys can be changed in `keymap.cfg`, so that I can rebind them.
7. As a player, I want the scheme's one rule (point at a thing, × acts, ○ backs out), so that I can guess the rest.
8. As a player, I want the jump mode line to say that the D-pad jumps between objects across the whole map and that holding it keeps jumping, so that I stop panning pixel by pixel.
9. As a player, I want to know that nothing moves when no object lies that way, so that a press that does nothing is understood rather than taken as a bug.
10. As a player, I want to know that × on a colony starts a slipway and × on a known planet opens its projects, so that I know what × does before pressing it.
11. As a player, I want to know that △ opens the hovered object's menu and △ again closes it, so that I can use △ as a toggle, as the measured player did.
12. As a player, I want to know that □ enters aim mode, so that I can reach empty space for probes and folds.
13. As a player, I want to know that ○ at the root jumps back to the last object × acted on, so that I can return to where I was building.
14. As a player, I want the aim mode line to say the D-pad moves the camera faster the longer it is held, so that I can both nudge and travel.
15. As a player, I want to know that × launches the probe and a held × folds space once a probe is out, so that I can explore.
16. As a player, I want to know that △ does nothing in aim mode, so that I don't expect a menu there.
17. As a player, I want to know that □ or ○ leaves aim mode onto the object nearest the crosshair, so that I know where the cursor lands.
18. As a player, I want the slipway line to say the D-pad aims and × builds, so that I can connect colonies in three presses.
19. As a player, I want to know that the game shows the cost above a target it can build, and why it can't build one ("too far", already linked), so that I read the feedback the game actually draws.
20. As a player, I want to know that after a build the next slipway starts from the object just reached, so that I can chain × D-pad × D-pad ×.
21. As a player, I want to know that ○ drops the slipway, so that I can back out without building.
22. As a player, I want to know that L1 opens the build bar, Left/Right pick and × takes, so that I can build structures.
23. As a player, I want to know that the D-pad moves the camera under a carried structure, × places it and ○ cancels it, so that I can place it where I want.
24. As a player, I want to know that L1 or ○ closes the bar and R1 from the bar opens tech, so that I move between panels without detours.
25. As a player, I want to know that R1 cycles tech, score and closed, and ○ returns to the map, so that I can reach both panels with one button.
26. As a player, I want to know that a Select tap jumps to the next colony or structure that needs something, so that I can find what needs attention.
27. As a player, I want to know that holding Select about half a second ends the year, so that I don't end one by accident.
28. As a player, I want to know that after a year ends, each fresh Select press within a quarter of a second ends another, so that I can skip several years quickly.
29. As a player, I want to know that Start pauses, and Start or ○ closes the pause menu, so that I can stop and resume.
30. As a player, I want to know that Start does not pause on tech, score or under a planet menu, so that I close those first.
31. As a player, I want one line on menus (D-pad moves, × picks, ○ closes), so that every menu works the same in my head.
32. As a player, I want to know that the × that opened a menu picks nothing until pressed again, so that a held × never buys something by accident.
33. As a player, I want ○'s back order spelled out (pause or box, then slipway or placement, then aim mode, then the last object), so that I can predict what each ○ undoes.
34. As a player, I want no mention of the tab ring, the end-turn tab or network mode, so that I don't look for them.
35. As a plan reader, I want every README claim traced to a unit test or an e2e frame in the step note, so that section 4's README line is checked, not asserted.
36. As a plan reader, I want the table's derivation from `keymap.cfg` written down, so that a later keymap change knows which README cells to update.
37. As a maintainer, I want durations in the README stated in seconds, with the frame constants named in the step note, so that a tuning change (section 5) knows which README words to update.
38. As a maintainer, I want the README to keep its other sections unchanged, so that this step's diff reads as one section.
39. As a maintainer, I want the H-14 outcomes in the step note, so that this PRD's assumptions about step 14 are audited.
40. As a maintainer, I want `make test-all` and every e2e hash unchanged by this step, so that a documentation step cannot break the game.
41. As a reviewer, I want the wrong "red line" claim corrected with its evidence, so that the README stops describing a colour the game never draws.
42. As a reviewer, I want every limit the README states (Start ignored here, Select blocked there) to come from a named test, so that no limit is invented.

## Implementation Decisions

### D-1. Scope of the edit

- Only the README's Controls section changes: the table, the `keymap.cfg` line and the bullets. Running the game, To do and the title stay byte-identical.
- The plan names four modes (jump, aim, slipway, menus). The section also needs lines for the button rules that are not map modes and that the old bullets carried: the build bar with placement (L1), tech and score (R1), Select, and ○'s order. Without them the plan's own examples ("R1 from the bar opens tech", "the 15-frame year chain") would have no line. Decided: one bullet per mode or button rule, nine bullets at most, each one or two sentences.
- The cheat chord is not documented. It is a dev tool (`CLAUDE.md` scope; step 6), and the current README omits it.
- The bookmark key (Space), the reserved keys and recording are outside Controls and stay as they are.

### D-2. The table, derived from `keymap.cfg`

- One row per PSX button in the file's order, with the D-pad's four lines folded into one row: D-pad, ×, ○, □, △, L1, R1, Start, Select. The Button cell is the glyph plus a role of one to three words (for example "× act", "○ back", "□ aim mode", "△ menu", "L1 build bar", "R1 tech / score", "Start pause", "Select attention / end year"). L1 and R1 get separate rows, since they no longer share a role.
- **Keyboard (QWERTY)** cell: the first name on the line (the keyboard key), written as a key cap. Letters as the letter, `UP`/`DOWN`/`LEFT`/`RIGHT` as "arrows", `ENTER` as "Enter", `RIGHT_SHIFT` as "Right Shift".
- **AZERTY** cell: the key at the same US position on an AZERTY layout. Z → W, A → Q. X, C, V, S, Enter, Right Shift and the arrows are unchanged.
- **Gamepad** cell: the second name, by position. `RIGHT_FACE_DOWN` bottom, `RIGHT_FACE_RIGHT` right, `RIGHT_FACE_LEFT` left, `RIGHT_FACE_UP` top, `LEFT_FACE_*` D-pad, `LEFT_TRIGGER_1` left shoulder, `RIGHT_TRIGGER_1` right shoulder, `MIDDLE_RIGHT` right middle, `MIDDLE_LEFT` left middle.
- The table describes the shipped `keymap.cfg`. The line after it ("Keys can be changed in `keymap.cfg`") stays, so a rebinding player knows the table shows the defaults.
- No automated check ties the table to `keymap.cfg`. A guard in `make test` would make it slower (`CLAUDE.md`: never) to protect nine cells that change only with a keymap commit. The step note records the derivation rules above, so the next keymap change applies them by hand. During the step, the implementer cross-checks the nine rows against `keymap.cfg` with a one-off script in `tmp/`, never committed, and records the result in the note.

### D-3. Rule lines: content

Each line states only behaviour the code has at step 15's start (H-14-1) and the step 5–12 notes record. Durations are given in seconds at 60 updates per second: 30 frames is "about half a second", 15 frames "a quarter of a second". The frame constants are named in the step note (D-5). The decided content, which the implementer words as plain English (CLAUDE.md's style):

- **The rule** (lead line): point at a thing, × acts on it, ○ backs out.
- **Jump mode** (the blinking circle; every game starts here): the D-pad jumps to the next object in that direction, anywhere on the map, and holding it keeps jumping. Nothing moves when no object lies that way. The camera follows only when the object nears the edge. × on a colony or structure starts a slipway; × on a known planet opens its projects. △ opens the hovered object's menu, and △ again closes it. □ enters aim mode. ○ jumps back to the last object × acted on.
- **Aim mode** (a crosshair at the centre): the D-pad moves the camera, faster the longer it is held. × on empty space launches the probe; once a probe is out, hold × to fold space. △ does nothing here. □ or ○ goes back to jump mode on the object nearest the crosshair.
- **Building a slipway:** after × on a colony or structure, the D-pad aims at another object. Above it the game shows the cost, or why it can't be built ("too far", already linked). × builds; a refused one aims again from the same start. Once built, the next slipway starts from the object just reached, so × D-pad × D-pad × chains them. ○ drops it. A colony or gate that takes no more slipways starts nothing.
- **Build bar (L1):** Left/Right pick a structure, × takes it. The D-pad then moves the camera under it, × places it and ○ cancels it. L1 or ○ closes the bar; R1 closes it and opens tech.
- **Tech and score (R1):** R1 opens tech, then score, then closes them. ○ goes back to the map, and L1 opens the build bar.
- **Select** (on the map, in jump or aim mode): a tap jumps to the next colony or structure whose panel lists a need. Holding it about half a second ends the year, once per hold. After a year ends, a fresh press within a quarter of a second ends one more.
- **Menus and pause:** the D-pad moves, × picks, ○ closes. A × held from before a menu opened picks nothing until pressed again. Start pauses from the map or the build bar, not from a menu, tech or score; Start or ○ closes pause.
- **○ order:** each ○ backs out one step: a pause or box (the year box included), then a slipway or a structure being placed, then aim mode, then the jump back to the last object.

Wording may change; no claim may be added without evidence (D-5), and none of the claims above may be dropped without a reason in the step note. If H-14-1, H-14-6 or H-14-7 fails, the affected claims are re-derived first.

### D-4. What the README must not say

- Nothing about the tab ring, the end-turn tab, network mode, a "build tab", the old drag or a red line.
- No tuning number in frames. No metric from step 14 (H-14-9).
- No claim that a refused slipway start gives feedback: it presses nothing (step 1, step 6).

### D-5. The step note's evidence list

"### Step 15" in section 6 of the plan holds:

- The H-14 outcomes, one line each (holds, or fails and what changed).
- The table derivation (D-2) and the one-off cross-check result.
- For each README line, each claim with its evidence: a unit test name (with its file) or an e2e file and frame. Starting list, read at `5ee926f`, which the implementer confirms still exists and still shows the claim (H-14-4, H-14-5):
  - **Table:** `test_versioned_file_is_the_default_text_and_loads_as_the_defaults`, `test_defaults_hold_the_plan_table_in_pad_bit_order` (`tests/test_keymap.c`); `keymap.cfg` lines.
  - **Jump, game start:** `test_a_new_game_starts_in_jump_mode_with_nothing_hovered`, `test_jump_mode_seeds_on_the_colony_over_a_nearer_known_planet`; `pad_game_end.txt` (stays in jump mode).
  - **Jump, D-pad and repeat:** `test_a_d_pad_press_jumps_to_the_picked_target`, `test_holding_a_direction_repeats_the_jump_after_18_then_every_8_frames`; `pad_colonise.txt` f266 (Up onto p25).
  - **Jump, empty cone:** `test_a_d_pad_press_with_an_empty_cone_changes_nothing`.
  - **Jump, edge scroll:** `test_a_jump_scrolls_only_to_bring_the_target_just_inside`, `test_the_edge_scroll_brings_the_hovered_object_just_inside_at_8_px_a_frame`; `pad_colonise.txt` f266–268.
  - **Jump, × on a colony:** `test_cross_on_a_hovered_colony_holds_its_slipway_and_the_d_pad_aims_past_it`; `pad_colonise.txt` f273.
  - **Jump, × on a known planet:** `test_cross_on_a_hovered_known_planet_opens_its_menu_and_records_it`; `pad_colonise.txt` f88.
  - **Jump, △ and △ again:** `test_triangle_on_a_hovered_known_planet_opens_its_menu_and_circle_comes_back`, `test_triangle_again_closes_a_known_planets_menu`, `test_triangle_again_closes_a_colonys_inspection_panel`; `pad_colonise.txt` f141, f144.
  - **Jump, □:** `test_square_in_jump_mode_enters_aim_mode_where_the_camera_is`; `pad_colonise.txt` f146.
  - **Jump, ○ at the root:** `test_circle_at_the_root_jumps_back_and_edge_scrolls_at_8_px_a_frame`, `test_circle_with_nothing_acted_on_and_cross_on_nothing_change_nothing`; `pad_colonise.txt` f268, f293.
  - **Aim, pan speed:** `test_aim_pans_with_the_speed_curve`; `pad_probe.txt` f19–24.
  - **Aim, probe and fold:** `test_aim_mode_first_cross_places_the_probe`, `test_aim_mode_short_cross_hold_abandons_the_fold`; `pad_probe.txt` f26, f41–57; `pad_colonise.txt` f258.
  - **Aim, △ does nothing:** `test_aim_mode_triangle_over_a_known_planet_or_a_colony_does_nothing`, `test_aim_mode_triangle_on_empty_space_does_nothing`.
  - **Aim, □ or ○ back:** `test_square_in_aim_mode_goes_back_to_jump_mode_on_the_nearest_target`, `test_circle_in_aim_mode_goes_back_to_jump_mode_on_the_nearest_target`; `pad_colonise.txt` f86, f173.
  - **Slipway, aim, cost and refusals:** `test_no_direction_aims_at_the_start`, `test_a_refused_target_aims_again_from_the_same_start`; `pad_slipway.txt` f249 (cost label), f263 (forbidden sign), f271 ("-too far-"), f251/f266/f277 (refused, pressed again from the same start).
  - **Slipway, chain:** `test_cross_dpad_cross_dpad_cross_builds_a_chain_of_two_slipways`; `pad_colonise.txt` f279–280, `pad_slipway.txt` f260–261.
  - **Slipway, ○ drops:** `test_circle_cancels_the_slipway_back_on_its_start`; `pad_colonise.txt` f291.
  - **Slipway, refused start:** `test_a_refused_start_holds_nothing`.
  - **Bar:** `test_l1_opens_the_bar_from_jump_mode_and_l1_closes_it_on_the_same_hovered_object`, `test_build_left_and_right_move_between_the_icons_without_wrapping`, `test_circle_closes_the_bar_on_the_same_hovered_object`, `test_r1_on_the_bar_closes_it_and_lands_on_tech_on_the_same_frame`; `pad_build.txt` f16, f19, f22, f41, f73.
  - **Placement:** `test_cross_takes_the_structure_and_a_second_cross_places_it`, `test_circle_cancels_the_structure_and_charges_nothing`, `test_the_bar_takes_and_aim_mode_places_or_cancels`; `pad_build.txt` f24–37, f52–53.
  - **R1 cycle:** `test_r1_cycles_tech_score_closed_from_the_map_and_opens_tech_from_the_bar`, `test_circle_on_tech_and_on_score_returns_to_jump_mode_on_the_hovered_object`, `test_l1_on_tech_and_score_opens_the_bar_and_hands_over_nothing`; `pad_score.txt` f20, f25, f56, f71, f77–88.
  - **Select tap:** `test_select_taps_cycle_the_colonies_needing_attention_in_order`, `test_select_in_aim_mode_ends_the_year_and_taps_back_to_jump_mode`; `pad_research.txt` f490–497.
  - **Select hold:** `test_a_select_hold_of_30_frames_presses_the_year_skip_button_once`, `test_a_select_hold_of_90_frames_ends_one_year`, `test_a_select_hold_under_30_frames_ends_nothing`; `pad_colonise.txt` f412–441, `pad_end_turn.txt` f64–83, f86–125.
  - **Select chain:** `test_fresh_select_presses_after_a_year_chain_more_years`; `pad_game_end.txt` f45–91.
  - **Select limits:** `test_select_does_nothing_while_something_is_held_or_open` (slipway mode, placement, a held fold, menus, pause), `test_select_on_the_bar_ends_no_year_and_presses_nothing`.
  - **Menus:** `test_planet_menu_is_reachable`, `test_cross_held_from_the_opening_click_presses_no_row_until_pressed_again`, `test_cross_held_into_pause_presses_no_row_until_pressed_again`; `pad_colonise.txt` f88–98, f139.
  - **Pause:** `test_start_opens_pause_from_aim_mode`, `test_pause_from_the_bar_keeps_the_bar_and_its_icon`, `test_start_is_ignored_under_a_planet_menu_and_on_tech_and_score`, `test_start_and_circle_close_pause_pressing_nothing_on_the_map`; `pad_pause_restart.txt` f16–26; `pad_message_box.txt` f122–138.
  - **○ order:** `test_circle_closes_the_year_box_over_jump_mode_and_moves_nothing`, `test_circle_closes_the_year_box_over_a_slipway_then_cancels_it`, `test_circle_cancels_a_structure_then_leaves_aim_mode_then_jumps_back`, `test_circle_closes_a_planet_menu_then_the_year_box`; `pad_build.txt` f52, f63.
- A claim without evidence is removed from the README, or gets evidence. Evidence can also be a test added by this step, which is not expected (Testing Decisions).
- The corrected "red line" claim, with its evidence (`pad_slipway.txt`'s colour-2 line and colour-8 labels).
- Re-authoring cost: none. No replay is touched.

### D-6. Commit

One `docs:` commit holds the README section and the step note. It touches no source, test, replay, fixture or reference. Section 2's extra checks (live slots, end-state, coverage, rng-sync, pad-author) are for chunks that touch pad input, a replay or `src/*/pad_*`. This step touches none, so they do not run. The note says so.

## Implementation Chunks

One chunk. It is too small to split:

1. Check H-14-1 to H-14-9 against the tree, then rewrite the Controls section (D-1 to D-4). Confirm every evidence item of D-5 by name or by reading its `.txt` line, then run each cited unit test binary once (`make unit-run-test_pad_nav`, `test_keymap`, `test_pad_select`, and the others cited). Write the "### Step 15" note. Run `make test-all` (unchanged, green) and commit.

## Testing Decisions

- **No new test.** The step changes no behaviour. Its "test" is the evidence list: each README claim maps to an existing unit test or e2e frame, which `make test-all` already runs (the unit tests through `make test`, the frames through each replay's `.sha256` in `make e2e`).
- **What a good check is here:** a claim is backed when a named test asserts that exact behaviour through the pad's public interface (`pad_nav_step` on a running cart), or when an e2e `.txt` line states it at a frame whose capture the replay's hash pins. A test that merely passes through the situation without asserting it does not count.
- **Prior art:** the step notes of steps 5–12, which already list tests per behaviour. `pad_colonise.txt` and `pad_research.txt` show how a `.txt` line pins a situation at a frame.
- **Mechanical cross-check (one-off, not committed):** the nine table rows against `keymap.cfg` by D-2's rules, and every cited test name against `grep` of its file. The outcome goes in the step note.
- **If a claim has no evidence and the implementer believes it true:** first look for a test or e2e frame that shows it. If there is none, drop the claim from the README (simplest). Adding a test for an untested behaviour is allowed only in `make test`'s existing pad test files, must not make `make test` slower in a measurable way, and is recorded in the note.

## Out of Scope

- Any change to a mapping, tuning value, game rule, keymap or keymap default (step 14 and section 5).
- README sections other than Controls (Running the game, To do).
- Documenting the cheat chord, the bookmark key, reserved keys or recordings.
- A guard in `make test` tying README to `keymap.cfg`.
- Section 4's metrics and the step 14 tool (H-14-9).
- The comment rework (step 16).
- Translations or other layouts beyond QWERTY and AZERTY.
- Audio, PSX, mouse input, launching the raylib app.

## Further Notes

- **Risk: stale evidence.** If step 14's work touches `tests/e2e` or `tests/test_pad_*.c` against its PRD, the frame and test lists here go stale. H-14-4 and H-14-5 are checked first for that reason.
- **Risk: wording drift.** "About half a second" and "a quarter of a second" follow `PAD_SELECT_HOLD_FRAMES` 30 and `PAD_SELECT_CHAIN_FRAMES` 15 at 60 updates per second. The step note names both constants, so section 5's tuning updates the README words with them.
- **Open question carried, not answered here:** the cone rule can rarely leave an object no jump reaches (section 5). The README says "the next object in that direction" and does not promise that every object is reachable.
- **The plan after this step:** step 16 (rework-comments) does not touch the README.
