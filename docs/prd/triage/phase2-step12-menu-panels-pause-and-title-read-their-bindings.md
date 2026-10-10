---
title: "Menu, panels, pause and title read their bindings"
description: "The four modes served by panel_step still move, choose and close on hardcoded buttons and on a direction computed from the physical D-pad, so a bindings table changes nothing in them."
status: needs-triage
---

## Problem Statement

This is plan 17, step 12 (Phase 2, bindings in the game). The step's text, from `docs/plan-17.md` section 3:

> 12. **Menu, panels, pause and title read their bindings**, including `pad_panel_step`'s held button and the `choose_row` arming. Verified the same way. It deletes only the game-side direction conversions: the devtools no longer use them after step 8.

"The same way" means step 9's verification and rule:

> Verified: e2e unchanged, plus a unit test with a table that swaps actions in the converted mode and leaves every other mode unbound: the converted mode follows the table, and the same buttons do nothing in a neighbouring mode served by the same shared code. Without it a read tagged with the wrong mode passes, because the default puts most actions on the same button in every mode. In steps 9–13, the step that converts a read also renames the local or comment that carries a PSX name on that line (`cross_engaged`, `circle_returns`, a `×` in a comment, …) to the action's name.

Since step 7, `pad_nav` holds a copy of the compiled bindings table. Steps 9 and 10 made jump, aim and slipway modes read it, and step 11 (in parallel) does the build bar and placement. The four modes that run through `panel_step` (the title's difficulty rows, the box of a hovered object and any other box over the map, the pause menu, and the tech and score panels) still play plan 12's buttons, whatever `bindings.cfg` says:

- Moving the focus uses one direction that `pad_nav_step` computes before any mode is known, from the physical D-pad (`pad_jump_first_direction(pressed)`: buttons 1–4, up, down, left, right first).
- Choosing a row is `pad_panel_step` pressing the left mouse button while button 5 is held, with button 5 masked until it is pressed in the box (the "arming": `pause_cross_armed` under pause, `menu_cross_armed` elsewhere).
- Pressing button 8 again closes a hovered object's box (`map_step`).
- Buttons 11 or 6 close pause (`pause_step`).

A player who moves `title.*`, `menu.*`, `pause.*` or `panels.*` actions in a personal file sees nothing change. `pad_jump.c` also keeps a button-to-direction conversion (`pad_jump_direction_button`, `pad_jump_first_direction`) that only this shared direction still uses.

## Solution

The four modes ask the table, through one mode decided where `panel_step` runs:

- `pause` when the pause menu is shown; `title` with no game; `panels` while a tech or score cycle panel is open; `menu` otherwise.
- The focus moves on that mode's `move_up`, `move_down`, `move_left`, `move_right`, first pressed in that action order.
- A row is chosen while that mode's `choose_row` is held, and a `choose_row` held from before the box opened chooses nothing until pressed again, exactly as today.
- `menu.open_menu` pressed again closes the box, and `pause.close_pause` closes pause.

The shared pressed direction disappears from `pad_nav_step`, and `pad_jump_first_direction` and `pad_jump_direction_button` are deleted from `src/game/`. `pad_panel_step` no longer names a button: it receives whether the row is being chosen. With the default table every frame is identical.

The build bar's up-or-down block (step 11) stays as step 11 leaves it: it reads `panels.move_up`/`panels.move_down`, not the shared direction, so removing the direction does not touch it.

Unit tests prove, for each of the four modes, that it follows a table with its actions off their default buttons while a neighbouring mode served by `panel_step` stays unbound and does nothing on the same buttons; and, with the default table, that the D-pad keeps plan 12's up, down, left, right ranking in a box.

The guard allowance loses exactly its step-12 lines: `src/game/pad_nav.c step-12 7`, `src/game/pad_panel.c step-12 1` and `src/game/pad_jump.c step-12 4`.

## Facts checked at HEAD 27723d8 (step 10 committed, step 11 not committed)

- `tools/button_guard_allowance.txt` holds, besides the step-11 and step-13 lines and the two `fixed` lines:
  - `src/game/pad_nav.c step-12 7 | panel_step: pause and menu arming of choose_row (4); map_step: menu.open_menu; pause_step: pause.close_pause (2)`
  - `src/game/pad_panel.c step-12 1 | pad_panel_step: held choose_row to the left mouse button`
  - `src/game/pad_jump.c step-12 4 | pad_jump_direction_button: the button-to-direction conversion behind the shared pressed direction`
- Guarded names per file at HEAD: `pad_nav.c` 21, `pad_panel.c` 1, `pad_jump.c` 4, `cheat.c` 2, `bindings.c` 3 (`button-guard files 5 reads 31`).
- Step 2.5's rows owned by step 12, at HEAD's line numbers (step 11 will move `pad_nav.c`'s):
  - Rows 18–19: `panel_step` (`pad_nav.c` 239–254). Under pause, `pause_cross_armed |= pressed & 5`, and button 5 is cleared from `pad_buttons` while unarmed. Else, when `armed_panel_open` holds (`(on_the_map || cycle_open) && pad_nav_panel_open() && !pause_menu_shown()`), the same with `menu_cross_armed`. Otherwise (a panel over the open build bar) nothing is masked. Then `pad_panel_step(focus, pad_buttons, direction, …)`.
  - Row 20: `pad_panel_step` (`pad_panel.c` 247–262) writes `P8_MOUSE_LEFT` when there is a focus and `pad_buttons & P8_PAD_BUTTON_5`.
  - Row 21: `panel_step` receives `direction` from `map_step` (line 820) and `bar_or_cycle_step` (line 541).
  - Row 22: `map_step` line 817: `(pressed & P8_PAD_BUTTON_8) && !release && !nav->release_pending && planet_menu_open(nav)` clears the ephemerals, and `panel_step` does not run on that frame.
  - Row 23: `pause_step` line 893: under pause, `!(pressed & (P8_PAD_BUTTON_11 | P8_PAD_BUTTON_6)) || release` returns 0; otherwise a click at the centre.
  - Row 24: `pad_nav_step` line 971: `pad_direction_t direction = pad_jump_first_direction(pressed);`, passed to `map_step` (line 1045) and `bar_or_cycle_step` (line 1049).
  - Row 25: `pad_jump.c` lines 47–69: `pad_jump_first_direction` and `pad_jump_direction_button` (4 names).
- The arming flags are reset outside `panel_step`: `pad_nav_init`, `click_hovered` (`menu_cross_armed = 0`) and `pad_nav_step` lines 977–982. None of these reads a button.
- No file of `src/devtools/` calls `pad_jump_first_direction` or `pad_jump_direction_button` (step 8). `pad_jump_direction(i)` and `PAD_JUMP_DIRECTIONS` are still used by `pad_author.c`, `pad_metrics.c`, `pad_nav.c`'s `aim_pan` and `pad_nav.h`, and stay.
- Tests calling the two conversions: `tests/test_pad_jump.c` (`test_the_first_pressed_direction_goes_up_down_left_right`, `test_each_direction_is_played_by_buttons_1_to_4`) and `tests/test_pad_nav.c` (`reach_all` uses `pad_jump_direction_button`; `expected_jump` and `slipway_pick` use `pad_jump_first_direction`).
- `src/game/pad_direction.h`'s comment names `pad_jump_direction_button`. `src/game/pad_panel.h`'s comment on `pad_panel_step` says "while × is held". `pad_nav.h` declares the fields `pause_cross_armed` and `menu_cross_armed`; no file outside `pad_nav.{c,h}` names them.
- The title's rows are not ui ephemerals (`help.c` sets `clear_ephemerals_after`), so `planet_menu_open` is false on the title and row 22 only acts in menu mode.
- `bindings.default.cfg` binds title, menu, pause and panels `move_* = 1–4` and `choose_row = 5`, `menu.open_menu = 8`, `pause.close_pause = 6, 11`. `bindings.h` has every mode and action needed. `pad_nav.c` has `action_pressed` and `action_held` (steps 9–10).
- The generator maps its modes to bindings modes (title, pause, menu; tech and score → panels) and reads the default table only, so its output cannot change with this step.
- Test prior art in `tests/test_pad_nav.c`: `start_title_with`, `begin_game_with`, `start_game_with`, `parsed_table`, `in_forked_copy`/`end_forked_copy`, `tap`, `hold`, `run_frame`, `settle`, `box_entries`, `end_game`, `end_buttons`, `known_planet_closest_to_centre`, `focus_affordable_row`, `invented_count`, `close_pause`, `assert_played`, `assert_cursor_at_centre`, `title_button`; the tests `test_how_to_play_is_reachable_and_opens_nothing`, `test_planet_menu_is_reachable`, `test_score_panel_at_game_end_left_and_right`, `test_cross_held_into_pause_presses_no_row_until_pressed_again`, `test_cross_held_from_the_opening_click_presses_no_row_until_pressed_again`, `test_cross_held_through_r1_presses_nothing_on_tech_until_pressed_again`, `test_restart_from_pause_with_a_save_shows_resume`, and steps 9–10's rotated-table tests.

## Hypotheses on step 11's outcome

Step 11 ("Build bar and placement read their bindings", PRD `docs/prd/triage/phase2-step11-build-bar-and-placement-read-their-bindings.md`) is being implemented in parallel and is not committed. This PRD was written from HEAD 27723d8 and that PRD. Each assumption about step 11 is a hypothesis:

- **H-11-1. The bar no longer reads the shared direction.** `build_step` has no `direction` parameter. It picks on `build_bar.pick_left`/`pick_right` only on a frame where no button of `panels.move_up | panels.move_down` is pressed, left winning over right.
- **H-11-2. The shared direction serves only `panel_step`.** `pad_nav_step` still computes `direction = pad_jump_first_direction(pressed)` and passes it to `map_step` and `bar_or_cycle_step`, which pass it only to `panel_step`.
- **H-11-3. Guard after step 11.** The step-11 line is gone, `pad_nav.c` names 18 guarded buttons, the three step-12 lines are unchanged, and `make button-guard` prints `files 5 reads 28`.
- **H-11-4. Step 11 leaves step 12's code alone.** `panel_step`, `armed_panel_open`, the arming fields, `map_step`'s button-8 read, `pause_step`'s close, `pad_panel.{c,h}` and `pad_jump.{c,h}` are as at HEAD (apart from line numbers).
- **H-11-5. Step 11's tests survive step 12.** Its test 1 starts the game through `start_title_with` and `cart_start_game` (no title press), binds `panels.move_up = 1` and `panels.move_down = 2` as reach-only lines and leaves title, menu, pause and the other panels actions unbound, and never shows a box. Its test 2 runs on the default table. Neither goes through `panel_step` with a press whose effect step 12 changes.
- **H-11-6. Step 11 keeps the generic helpers.** `action_pressed` and `action_held` are unchanged. Step 11 adds no helper step 12 must reuse.
- **H-11-7. Guardrail outputs.** After step 11, `make pad-author` regenerates `worst-case/` with no diff, and `make pad-author-proof`, `make pad-metrics-check` and `make pad-metrics-worst-case` give the same outputs as at step 10's end.

**Rule if a hypothesis is wrong.** Follow the committed step 11 code at this step's own `HEAD`, use its names, and record the difference in the step note (`### Step 2.12`). In particular:

- **The bar's block (H-11-1).** If `build_step` still reads the shared direction, or reads up and down another way, step 12 must not drop plan 12's block when it removes the direction. Convert the bar's pick to the rule of step 11's PRD (pick only when no button of `panels.move_up | panels.move_down` is pressed, then `pick_left`, else `pick_right`) in chunk 1, before the direction goes, and add step 11's test 2 if it is missing. Record it. Do not touch `build_step` in any other way.
- **Other users of the direction (H-11-2).** If anything besides `panel_step` still uses `direction`, it must read its own mode's masks before chunk 1 removes the direction. Record what and why.
- **Guard counts (H-11-3).** If they differ, what matters is that the three step-12 lines go, nothing else in the allowance changes, and `pad_nav.c` names exactly 7 fewer buttons than at the step's start.
- **Step 11's tests (H-11-5).** If one breaks because a panel mode now reads the table, add the missing action to its table on its default button as a reach-only line, change nothing else in it, and record it.
- **Guardrail outputs (H-11-7).** If one differs at step 12's start, use the tree's output as the baseline. Step 12 must not change it.

## User Stories

1. As a player, I want the buttons bound to `title.move_up` and `title.move_down` to move between the difficulty rows, so that I can remap the title.
2. As a player, I want the button bound to `title.choose_row` to start the focused difficulty, so that the title follows my file.
3. As a player, I want `menu.move_*` to move the focus in a hovered object's box, so that I can remap box navigation.
4. As a player, I want `menu.choose_row` to press the focused row of a box, so that I choose on my button.
5. As a player, I want `menu.open_menu` pressed again to close a planet's box, so that the toggle follows the button that opens it in my file.
6. As a player, I want `pause.move_*` to move among the pause rows, so that pause follows my file.
7. As a player, I want `pause.choose_row` to press the focused pause row, so that restart and the other rows are on my button.
8. As a player, I want every button bound to `pause.close_pause` to close pause without pressing anything on the map, so that I can choose how to leave pause.
9. As a player, I want `panels.move_*` and `panels.choose_row` to work on the tech panel and the score panel opened by the shoulder, so that the cycle panels follow my file.
10. As a player, I want a `choose_row` button held from before a box, pause or panel opened to choose nothing until I press it again, as today, so that remapping keeps that safety.
11. As a player, I want the first pressed move in the order up, down, left, right to win when several are pressed on one frame, as today, so that chords on the D-pad behave the same wherever the moves are bound.
12. As a player, I want the old default buttons to do nothing in a mode once I move its actions, so that the mode follows my file only.
13. As a player, I want a mode's buttons to do nothing in another panel mode that does not bind them, so that the modes stay independent.
14. As a player, I want an action left out of my file to do nothing, so that an unbound action is never silently played.
15. As a player, I want the build bar's up-or-down block to keep following `panels.move_up` and `panels.move_down`, so that step 11's behaviour survives this step.
16. As a player, I want the default bindings to play exactly like plan 12, so that nothing changes without a personal file.
17. As a maintainer, I want one place that decides the panel mode, so that the arming, the moves and the choose read the same mode on a frame.
18. As a maintainer, I want the mode decision to follow step 2.5's rule (pause, title, panels, menu), so that the plan's inventory and the code agree.
19. As a maintainer, I want the shared pressed direction gone from `pad_nav_step`, so that no direction is computed before the mode is known.
20. As a maintainer, I want `pad_jump_first_direction` and `pad_jump_direction_button` deleted, so that no game file converts buttons to directions any more.
21. As a maintainer, I want `pad_panel_step` to name no button, so that the panel helper is independent of the bindings.
22. As a maintainer, I want the arming fields renamed after `choose_row`, so that step 14's PSX-word grep finds nothing this step owns.
23. As a maintainer, I want `pad_panel.h`'s and `pad_direction.h`'s comments to stop naming × and the deleted conversion, so that they state the current contract.
24. As a maintainer, I want the guard allowance to lose exactly the three step-12 lines, so that the guard proves the conversion and blocks a regression.
25. As a maintainer, I want every e2e `.sha256` and end state unchanged, so that this step is a proven refactor.
26. As a maintainer, I want `make pad-author-proof`, `make pad-author` with no diff in `worst-case/`, `make pad-metrics-check` and `make pad-metrics-worst-case` unchanged, so that the devtools still agree with the game.
27. As a maintainer, I want the existing pad tests unchanged except for the mechanical replacement of the deleted conversions, so that plan 12's behaviour stays pinned by the same expected values.
28. As a maintainer, I want one rotated-table test per converted mode with an unbound neighbour, so that a read tagged with the wrong one of the four modes fails.
29. As a maintainer, I want a default-table test pinning the D-pad ranking in a box, so that deleting the conversions' unit tests loses no pinned behaviour.
30. As a maintainer, I want a one-off mutation proof recorded in the step note, so that I know the new tests catch a wrong mode, a hardcoded read or a lost arming.
31. As a maintainer, I want `make test` and `make e2e` no slower, so that the fast loop stays fast.
32. As a reviewer, I want the step note to list which H-11-k held and every place where the code differs from section 2's table, so that I can audit the step.

## Implementation Decisions

### The panel mode

- A static helper in `pad_nav.c`, `panel_mode(nav)`, returns, in this order: `BINDINGS_MODE_PAUSE` when `pause_menu_shown()`; `BINDINGS_MODE_TITLE` when `main_economy_typed() == NULL`; `BINDINGS_MODE_PANELS` when `cycle_open(nav)`; `BINDINGS_MODE_MENU` otherwise. This is step 2.5's rule. `menu` therefore covers every box over the map (a planet's box, an inspection panel, an aim-mode click, a tech or score box opened by a HUD click with no cycle panel, the final score panel) and any panel over the open build bar.
- `panel_step` computes the mode once per call and uses it for the arming, the choose and the moves. `map_step`'s row-22 read uses `BINDINGS_MODE_MENU` (its own condition, `planet_menu_open`, only holds in menu mode). `pause_step`'s row-23 read uses `BINDINGS_MODE_PAUSE` (it runs only under pause).

### Moves (rows 21, 24, 25)

- `panel_step` computes its direction itself: the first of `move_up`, `move_down`, `move_left`, `move_right` (in that action order, through `pad_jump_direction(i)` like `aim_pan`) whose mask in the panel mode meets `pressed`; `PAD_DIRECTION_NONE` otherwise. A static helper in `pad_nav.c` (for example `first_pressed_move(nav, pressed, mode)`) with a static action array indexed like `aim_pan`'s, and an assert that the mode has `move_up`. It does not reuse `pad_jump.c`'s `first_pressed_direction`, which reads `jump_*`; `pad_jump_repeat_step` is not touched.
- Priority is by action, not by button: with `move_up` on 2 and `move_down` on 1, pressing 1 + 2 moves up. With the default this equals today's `pad_jump_first_direction(pressed)` for every combination of buttons 1–4.
- `panel_step`, `map_step` and `bar_or_cycle_step` lose their `direction` parameter. `pad_nav_step` no longer computes a direction.
- `pad_jump_first_direction` and `pad_jump_direction_button` are deleted from `pad_jump.{h,c}`. `pad_jump_direction`, `PAD_JUMP_DIRECTIONS` and the priority table stay. `pad_jump.c` drops `#include "p8/input.h"` if nothing else uses it.
- `pad_direction.h`'s comment keeps its trap without naming the deleted function, for example: "Not button bits: the bindings give the buttons that play a direction."

### Choose and arming (rows 18–20)

- `pad_panel_step(entity_t *focus, int choose_held, pad_direction_t direction, int *x, int *y, int *mouse_buttons)`: it writes `P8_MOUSE_LEFT` when there is a focus and `choose_held` is nonzero, 0 otherwise. Its header comment says so, with no ×. Nothing else in `pad_panel.c` changes.
- In `panel_step`: `choose_held = action_held(nav, pad_buttons, mode, BINDINGS_ACTION_CHOOSE_ROW)`. Under pause, `pause_choose_row_armed |= action_pressed(nav, pressed, BINDINGS_MODE_PAUSE, BINDINGS_ACTION_CHOOSE_ROW)`, and `choose_held` is 0 while unarmed. Else, when `armed_panel_open(nav)`, the same with `menu_choose_row_armed` and the panel mode (title, menu or panels). Otherwise nothing is masked. Conditions and order unchanged.
- Fields renamed at every use (`pad_nav.h`, `pad_nav_init`, `click_hovered`, `pad_nav_step`'s resets): `pause_cross_armed` → `pause_choose_row_armed`, `menu_cross_armed` → `menu_choose_row_armed`. The reset conditions do not change.

### Close actions (rows 22–23)

- Row 22: `action_pressed(nav, pressed, BINDINGS_MODE_MENU, BINDINGS_ACTION_OPEN_MENU) && !release && !nav->release_pending && planet_menu_open(nav)`. Same effect: the ephemerals are cleared and `panel_step` does not run on that frame.
- Row 23: `!action_pressed(nav, pressed, BINDINGS_MODE_PAUSE, BINDINGS_ACTION_CLOSE_PAUSE) || release` returns 0. Both default buttons (6 and 11) stay in one mask, read in one test.

### Differences from section 2's table, followed from the code (record in the step note)

- `menu` is wider than "the box of a hovered object": it is every non-pause box over the map and every panel over the open build bar (step 2.5's rule).
- Over the open build bar, `choose_row` has no arming: `armed_panel_open` is false there, and a held `choose_row` chooses at once. Section 2 states the arming for `choose_row` generally. Kept.
- `menu.open_menu` closes only what `planet_menu_open` sees (a non-pause box of ephemerals on the map) and waits for no click release pending. `pause.close_pause` waits for no release frame.
- The moves rank up, down, left, right by action. Section 2 does not say.
- The title arming shares the menu's flag: a `choose_row` held across a return to the title chooses nothing until pressed again.

### Guard allowance

- When the step ends, `tools/button_guard_allowance.txt` has lost exactly its three step-12 lines and nothing else has changed: `pad_nav.c` keeps only its step-13 line, plus the two `fixed` lines. `make button-guard` goes from `files 5 reads 28` to `files 3 reads 16` (H-11-3).
- Between commits the allowance stays valid: see each chunk.

### No other interface changes

- `pad_panel_step` is the only exported signature that changes; its only caller is `panel_step`. No Makefile link line changes. `pad_nav.h` changes only by the two field names.

## Implementation Chunks

1. **Moves through the table; the shared direction and the conversions go.** `panel_mode`, the per-mode move helper, `direction` removed from `pad_nav_step`, `map_step`, `bar_or_cycle_step` and `panel_step`'s parameters, `pad_jump_first_direction` and `pad_jump_direction_button` deleted, `pad_direction.h`'s comment. The `src/game/pad_jump.c step-12 4` line is deleted. In tests: the two conversion tests leave `test_pad_jump.c`; `test_pad_nav.c` gets the two test-local helpers (Testing Decisions); add test 5 first (it passes before and after), then the move checks of tests 1–4. Verified: `make test-all`, every `.sha256` and end state unchanged, the four guardrail commands unchanged, and `make raylib` links.
2. **Choose and arming through the table.** `pad_panel_step` takes `choose_held`, `panel_step` reads `choose_row` by panel mode and arms on it, fields renamed, `pad_panel.h`'s comment. The allowance's `src/game/pad_panel.c step-12 1` line is deleted, and the `pad_nav.c step-12` line goes to `src/game/pad_nav.c step-12 3 | map_step: menu.open_menu; pause_step: pause.close_pause (2)`. Add the choose and arming checks of tests 1–4. Same verification.
3. **Close actions through the table.** Rows 22 and 23. The `pad_nav.c step-12` line is deleted. Add the close checks of tests 1–3. Verified: everything under Testing Decisions.

## Testing Decisions

- **What a good test is.** It drives frames through `input_step_run` and asserts what the player sees: the focus, the hovered entity, the played mouse buttons and position, whether a box, pause or panel is open, the economy, inventions. It never reads static state.
- **Existing tests do not change** (section 2 guardrail), with two mechanical exceptions caused by the deleted functions:
  - `test_pad_nav.c` replaces `pad_jump_direction_button(d)` and `pad_jump_first_direction(button)` with two test-local static helpers holding plan 12's D-pad mapping (1 up, 2 down, 3 left, 4 right; the first pressed in that order). No expected value changes.
  - `test_pad_jump.c` deletes `test_the_first_pressed_direction_goes_up_down_left_right` and `test_each_direction_is_played_by_buttons_1_to_4` with the functions they test. What they pinned is still pinned: `pad_jump_repeat_step`'s existing priority test for the jump directions, and test 5 for the boxes. Record it in the step note.
  - Any other assertion that must change is a divergence between the code and section 2's table: follow the code and record it.
- **Tables.** Each new test parses its own table with `parsed_table`, starts through `start_title_with` or `start_game_with`, and opens boxes directly where possible (`galaxy_menu_on_click(4)(NULL)` for pause, `galaxy_menu_on_click(2)(NULL)` for a HUD tech box, `planet_left_press` on a known planet for a planet's box), so no other mode's action is needed. The converted mode's actions sit off their default buttons. Actions of modes step 13 converts, needed to reach a mode, are bound on their default buttons and marked reach-only. Every other mode stays unbound. Expected neighbours come from `pad_nav_neighbor` on the current focus, never from a hardcoded layout.
- **Checks in forks.** Each check that changes the mode or the world runs in its own `in_forked_copy`.

### Test 1: the title follows a rotated table and pause stays unbound

Table: `title.move_up = 2`, `title.move_down = 1`, `title.move_left = 4`, `title.move_right = 3`, `title.choose_row = 7`. Nothing else.

- Moves (chunk 1): the focus is "forgiving". Tap 1: "reasonable". Tap 2: "forgiving". Tap 1, then press 1 + 2 on one frame: "forgiving" (move_up wins by action although button 1 is move_down).
- Choose (chunk 2): in a fork, tap 5: no mouse button played, no game, focus unchanged. Press 7: the left button is played at "forgiving"'s focus point; settle: a game exists.
- Neighbour (chunk 2 for choose, chunk 1 for moves, chunk 3 for close): in the game, `galaxy_menu_on_click(4)(NULL)` and settle. Tap 1, 2, 3, 4: the focus stays on the first item. Tap 7 and 5: no mouse button, pause shown. Tap 6, 11 and 8: pause still shown (pause is unbound, and step 13's button-6 code never closes pause).

### Test 2: pause follows a rotated table and a planet's box stays unbound

Table: `pause.move_up = 2`, `pause.move_down = 1`, `pause.move_left = 4`, `pause.move_right = 3`, `pause.choose_row = 7`, `pause.close_pause = 8`. Started with `start_game_with`.

- Moves: open pause directly and settle. Let `up = pad_nav_neighbor(first, PAD_DIRECTION_UP)` (non-NULL: `test_how_to_play_is_reachable_and_opens_nothing` moves up from the first item). Tap 2: focus `up`. Tap 1: back on the first item. Press 1 + 2: focus `up`.
- Arming: in a fork, hold 7 for a few frames, open pause directly while 7 stays held, and keep 7 held for `SETTLE_FRAMES`: no mouse button on any frame. Release one frame, then press 7 on "restart": the left button is played; settle: no game (the title).
- Old buttons: tap 5: no mouse button, pause shown, focus unchanged.
- Close: in a fork, tap 6, then 11: pause still shown. Then `close_pause(step, P8_PAD_BUTTON_8)`, settle: aim mode, nothing pressed on the map, cursor at the centre.
- Neighbour: in a fork with pause closed, open the box of `known_planet_closest_to_centre` with `planet_left_press` and settle. Tap 1, 2: focus unchanged. Tap 7: no mouse button. Tap 8: the box stays open. Do not press 6 (`menu.close_menu` is step 13's).

### Test 3: a planet's box follows a rotated table and the tech panel stays unbound

Table: `menu.move_up = 2`, `menu.move_down = 1`, `menu.move_left = 4`, `menu.move_right = 3`, `menu.choose_row = 7`, `menu.open_menu = 5`, and `aim.open_tech_panel = 10` (reach-only, step 13).

- Arming: hold 7, open a known planet's box with `planet_left_press` while 7 stays held, settle holding 7: no mouse button on any frame, credits unchanged.
- Moves: release. With `down = pad_nav_neighbor(first, PAD_DIRECTION_DOWN)` (non-NULL), tap 1: focus `down`. Tap 2: back.
- Choose: in a fork, press 7: the left button is played at the focus point.
- Close: tap 8: the box stays open. Tap 5: the ephemerals are cleared and the game is back on the map.
- Neighbour (panels unbound): set research points to 5, tap 10, let the landing click pass: the tech panel is open. Tap 1, 2: focus unchanged. Tap 7, 5: no mouse button, tech still open, nothing invented.

### Test 4: the tech panel follows a rotated table and the same box opened over the map stays unbound

Table: `panels.move_up = 2`, `panels.move_down = 1`, `panels.move_left = 4`, `panels.move_right = 3`, `panels.choose_row = 7`, and `aim.open_tech_panel = 10` (reach-only, step 13). Research points set to 5.

- Neighbour first, in a fork: `galaxy_menu_on_click(2)(NULL)` and settle. The tech box is open with no cycle panel, so `panel_step` serves it in `menu` mode, which is unbound: tap 1, 2: focus unchanged; tap 7: no mouse button, nothing invented. This pins the mode decision: the same box is `panels` only through the cycle.
- Arming: press 7 + 10 on one frame, then hold 7 as `test_cross_held_through_r1_presses_nothing_on_tech_until_pressed_again` does: after the landing click, no mouse button on any held frame, nothing invented.
- Moves: release. With `down = pad_nav_neighbor(header, PAD_DIRECTION_DOWN)`, tap 1: focus `down`. Tap 2: back on the header.
- Old button: tap 5: no mouse button.
- Choose: tap 1 until the focus has an `on_click` (a local copy of `focus_affordable_row` taking the button), then press 7: one invention.

### Test 5: with the default table, the D-pad keeps its ranking in a box

`end_game`, so the final score panel's two buttons are focusable side by side. Let `focus` be the focused button and `side` the button (3 or 4) of the direction in which it has a neighbour.

- Press 1 + `side`: focus unchanged (up wins and has no neighbour). Frame with nothing. Press 2 + `side`: focus unchanged. Frame with nothing. Press 3 + 4: focus is `pad_nav_neighbor(focus, PAD_DIRECTION_LEFT)` if it exists, else unchanged (left wins over right).
- It passes before and after the step: it pins plan 12's rule on a path no replay presses, now that `pad_jump_first_direction`'s unit test is gone.

### Step 11's tests

- Step 11's test 1 and test 2 run unchanged (H-11-5). Test 2 is the proof that the bar's block survives the direction's removal.

### One-off mutation proof

Record it in the step note and do not commit it. Restore each mutation after it is checked.

- Make `panel_mode` return `MENU` where it returns `TITLE`: test 1 fails.
- Make `panel_mode` return `MENU` where it returns `PANELS`: test 4 fails (and test 3's neighbour).
- Arm pause from the menu mode's `choose_row`: test 2's arming or choose check fails.
- Leave `pad_panel_step` testing `P8_PAD_BUTTON_5`: tests 1–4 fail at their choose checks, and `make button-guard` reports `src/game/pad_panel.c` naming a button it does not list.
- Put back the shared `pad_jump_first_direction(pressed)` as `panel_step`'s direction: tests 1–4 fail at their move checks.
- Rank the moves by button instead of by action: test 1's 1 + 2 check fails.
- Leave row 23 on buttons 6 and 11: test 2's close check fails.
- Drop the `panels.move_up | move_down` block from `build_step`: step 11's test 2 fails.

### Step verification, in this order

1. `make test-all`: green, every `tests/e2e/*.json.sha256` unchanged, and `make end-state` unchanged on every replay.
2. Guardrails (section 2; this step touches `src/game/pad_*`): `make pad-author-proof`, then `make pad-author` followed by `git diff --exit-code worst-case/`, then `make pad-metrics-check` and `make pad-metrics-worst-case`. All unchanged from the tree at step 12's start.
3. `make button-guard` passes with the three step-12 lines gone. `git diff <start> -- tools/button_guard_allowance.txt` shows only those deletions. `git grep -c P8_PAD_BUTTON_ -- src/game/pad_panel.c src/game/pad_jump.c` finds nothing, and `pad_nav.c` names exactly 7 fewer than at the step's start.
4. `git grep -n -E 'pad_jump_first_direction|pad_jump_direction_button' -- src tests` finds nothing. `git grep -n -E 'cross_armed|×' -- src/game/pad_nav.* src/game/pad_panel.*` finds nothing.
5. `make raylib` compiles and links. It is never run.
6. Timings: `make test` and `make e2e`, three warm runs each, against step 1's baseline (4.27–4.58 s and 0.24–0.25 s). If `test_pad_nav` gets measurably slower, reduce frames, not checks.

## Out of Scope

- Everything step 13 converts: the shoulders (`open_build_bar`, `open_tech_panel`, `panels.next_panel`), `switch_panels`' button 6 (`build_bar.close_bar`, `panels.close_panel`), pause opening on 11, Select, `circle_returns`, the box and message-box close on button 6 (`menu.close_menu`), `without_chord`. With a table that moves these, the four modes still answer plan 12's buttons for them until step 13. The `circle_*` names are step 13's to rename.
- The build bar and placement reads (step 11). Step 12 only keeps the bar's block working.
- `pad_jump_direction`, `PAD_JUMP_DIRECTIONS` and `pad_metrics`' own D-pad mapping, which the devtools use.
- Any change under `src/devtools/`, to `bindings.default.cfg` or its comments (step 20 rewrites the docs), or to section 2's table.
- Renaming test functions or comments in `tests/` that carry PSX words.

## Further Notes

- **Trap: four modes share button 5 and the D-pad by default.** Title, menu, pause and panels all put `move_* = 1–4` and `choose_row = 5`, so a read tagged with the wrong one passes with the default. Each test leaves its neighbour unbound.
- **Trap: the same box can be two modes.** A tech box opened by a HUD click with no cycle panel runs in `menu`; through the shoulder it runs in `panels`. Test 4 pins it.
- **Trap: the move ranking is by action.** A per-button scan of the four D-pad bits, or of whatever buttons happen to be bound, would rank differently once moves are rotated. Read the four masks in action order.
- **Trap: the bar's block lives in `panels`.** `build_step` reads `panels.move_up`/`move_down` (step 11). Removing the shared direction must not touch it, and `panel_mode` must not be used by `build_step`.
- **Trap: arming resets happen outside `panel_step`.** The renamed fields are reset in `pad_nav_init`, `click_hovered` and at the top of `pad_nav_step`; rename all of them and change none of the conditions.
- **Trap: button 6 is still step 13's.** Until step 13, button 6 closes a non-pause box and returns from panels whatever the table says. Keep 6 out of the new tests except where they check that it does not close pause.
- The step note (`### Step 2.12`) records:
  - which H-11-k held, and how the code adapted where one did not;
  - the converted reads by action, and the panel-mode rule;
  - the differences from section 2's table listed under Implementation Decisions;
  - the two deleted conversion tests and what pins their behaviour now;
  - the mutation proof's results;
  - the guard output before and after;
  - the guardrail runs;
  - the timings.
