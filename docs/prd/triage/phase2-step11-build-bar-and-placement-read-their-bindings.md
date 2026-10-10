---
title: "Build bar and placement read their bindings"
description: "The build bar's pick and take, and placement's place and cancel, still test hardcoded buttons, so a bindings table moves nothing in these two modes."
status: needs-triage
---

## Problem Statement

This is plan 17, step 11 (Phase 2, bindings in the game). The step's text, from `docs/plan-17.md` section 3:

> 11. **Build bar and placement read their bindings**, verified the same way.

"The same way" means step 9's verification and rule:

> Verified: e2e unchanged, plus a unit test with a table that swaps actions in the converted mode and leaves every other mode unbound: the converted mode follows the table, and the same buttons do nothing in a neighbouring mode served by the same shared code. Without it a read tagged with the wrong mode passes, because the default puts most actions on the same button in every mode. In steps 9–13, the step that converts a read also renames the local or comment that carries a PSX name on that line (`cross_engaged`, `circle_returns`, a `×` in a comment, …) to the action's name.

Since step 7, `pad_nav` holds a copy of the compiled bindings table. Step 9 made jump mode read it, and step 10 makes aim mode, slipway mode and the shared pan read it. The build bar and placement still play plan 12's buttons, whatever `bindings.cfg` says:

- `build_step` tests button 5 to take the focused structure. It moves the focus with the shared pressed direction, which `pad_nav_step` computes from the physical D-pad (`pad_jump_first_direction`): left on button 3, right on button 4.
- `placement_step` tests button 6 to cancel the carried structure and button 5 to place it.

A player who moves `build_bar.pick_left`, `build_bar.pick_right`, `build_bar.take_structure`, `placement.place_structure` or `placement.cancel_placement` in a personal file sees nothing change.

## Solution

The build bar and placement ask the table:

- `build_step` takes the focused structure on `build_bar.take_structure`, and moves the focus on `build_bar.pick_left` and `build_bar.pick_right`. It no longer reads the shared pressed direction.
- `placement_step` cancels on `placement.cancel_placement` and places on `placement.place_structure`.

With the default table every frame is identical, including one path no replay or test reaches: up or down pressed on the same frame as left or right stops the bar from moving, because the shared direction ranks up and down first. The bar keeps that rule by reading the buttons bound to `panels.move_up` and `panels.move_down` (decision under Implementation Decisions).

Unit tests prove three things:

- With a table that moves every bar and placement action off its default button, both modes follow it.
- The same buttons do nothing in aim mode, which `map_step` serves next to placement, and which shares button 5 and the pan with it by default.
- With the default table, a pick pressed together with up or down still moves nothing, and left wins over right, as on plan 12.

The guard allowance loses the line `src/game/pad_nav.c step-11 3`. The two comments carrying `×` on step 11's code lose it.

## Facts checked at HEAD 7000fd6 (step 9 committed; step 10 not committed)

- `tools/button_guard_allowance.txt` has `src/game/pad_nav.c step-11 3 | build_step: build_bar.take_structure; placement_step: placement.cancel_placement, placement.place_structure`. The guard requires strict per-file equality. At HEAD, `pad_nav.c` names 26 guarded buttons, and `make button-guard` prints `files 5 reads 36`.
- Step 2.5's rows owned by step 11, at HEAD's line numbers in `pad_nav.c` (step 10 will move them):
  - Row 14: `build_step` line 499, `icon != NULL && (pressed & P8_PAD_BUTTON_5) && !release` plays the left mouse button at the icon and sets `PAD_NAV_PLACEMENT_PRESSED` → `build_bar.take_structure`.
  - Row 15: `build_step` lines 502–503, `direction == PAD_DIRECTION_LEFT || direction == PAD_DIRECTION_RIGHT` → `pad_build_pick(…, -1 or 1)`. Indirect: `direction` is `pad_jump_first_direction(pressed)`, computed once in `pad_nav_step` (line 961), whose priority is up, down, left, right → `build_bar.pick_left`, `build_bar.pick_right`. The guard does not count it.
  - Row 16: `placement_step` line 577, `(pressed & P8_PAD_BUTTON_6) || pad_nav_panel_open()` → `cancel_point` → `placement.cancel_placement`.
  - Row 17: `placement_step` line 585, `if (pressed & P8_PAD_BUTTON_5) return;`, so the left button is not played on that frame and the ui drops → `placement.place_structure`.
- `build_step(nav, pressed, direction, release, played)` is called only from `bar_or_cycle_step`, which also passes `direction` to `panel_step` (step 12's). The pick runs after the take, on release frames too, and whether or not an icon is focused.
- Comments with `×` on step 11's code:
  - line 487, above `PARKED_X`: "Off screen, so × finds no left_press under the cursor." The parked point serves `place_cursor` when the bar has no icon and `cancel_point` when placement cancels with no icon.
  - line 584, in `placement_step`: "× drops by letting the held left button go for one frame: the ui's drop runs the cart's rules."
- `placement_step` also holds `nav->cross_engaged = 0;` (step 10 renames the field) and `aim_pan(nav, pad_buttons);` (step 10 gives it a mode).
- `pad_nav_step` clears button 6 from `pressed` when ○ closes a message box on the map (row 33, step 13's), before `placement_step` runs. `test_circle_closes_the_year_box_over_a_carried_structure_and_keeps_it` pins this with the default table.
- `static int action_pressed(const pad_nav_t *nav, int pressed, bindings_mode_t mode, bindings_action_t action)` exists in `pad_nav.c` (step 9).
- `bindings.h` has `BINDINGS_MODE_BUILD_BAR`, `BINDINGS_MODE_PLACEMENT`, `BINDINGS_ACTION_PICK_LEFT`, `PICK_RIGHT`, `TAKE_STRUCTURE`, `PLACE_STRUCTURE`, `CANCEL_PLACEMENT`. `bindings.default.cfg` binds `build_bar.pick_left = 3`, `pick_right = 4`, `take_structure = 5`, `close_bar = 6`, `open_build_bar = 9`; `placement.pan_* = 1–4`, `place_structure = 5`, `cancel_placement = 6`, `open_build_bar = 9`. Only `build_bar` has `pick_*`.
- `tests/test_pad_nav.c` has `start_title_with(step, bindings)` (step 9), `start_title`, `begin_game`, `start_game` (asserts aim mode with nothing hovered), `open_build_bar` (taps 9), `take_structure` (presses 5, then asserts carrying), `assert_carrying`, `pan_clear_of_every_node` (holds 2), `assert_placement_over`, `basic_structure_icon`, `processor_icon` (10 px left of the basic one), `assert_build_focus`, `assert_played`, `world_count`, `assert_world_unchanged`, `camera_at`, `assert_moved`, `tagged_count`, `pan_to_empty_space`, `tap`, `hold`, `in_forked_copy`/`end_forked_copy`. Step 9's `rotated_jump_table()` parses an inline table with `bindings_parse`: prior art for a test table.
- The existing bar tests press only one direction at a time: no test in `tests/` presses up or down together with left or right while the bar is open.
- **Measured, not committed.** In a scratch export of HEAD, `build_step`'s pick was replaced by "left if button 3 pressed, else right if button 4 pressed", logging every frame where that differs from today's pick. `make e2e` (17 match), `make end-state` (22 replays match), `make pad-author-proof`, `make pad-author` (no diff in `worst-case/`), `make pad-metrics-check` and every unit test passed. Nothing was logged: no replay, generator run or unit test reaches a bar frame where up or down is pressed with left or right. `make button-guard` failed only because the patch named two extra buttons.

## Hypotheses on step 10's outcome

Step 10 ("Aim and slipway modes read their bindings", PRD `docs/prd/triage/phase2-step10-aim-and-slipway-modes-read-their-bindings.md`) is being implemented in parallel and is not committed. This PRD was written from HEAD 7000fd6 and from step 10's PRD. Each assumption about step 10 is a hypothesis:

- **H-10-1. The pan takes the mode.** `aim_pan(nav, mode, pad_buttons)` exists, reads `mode`'s `pan_*` masks, and `placement_step` calls it with `BINDINGS_MODE_PLACEMENT`. Step 11 does not touch that call.
- **H-10-2. The engagement field is renamed.** `cross_engaged` is now `probe_or_fold_engaged`, including the reset line in `placement_step`. Step 11 keeps that line as it is, under whatever name step 10 chose.
- **H-10-3. Aim mode reads the table.** `aim.probe_or_fold` and `aim.leave_aim` are read through the table, so with both unbound, buttons 5 and 7 do nothing in aim mode.
- **H-10-4. Helpers.** `action_pressed` is unchanged. Step 10 may have added `action_held`. Step 11 needs only `action_pressed`.
- **H-10-5. The guard moved by 5.** After step 10, `pad_nav.c` names 21 guarded buttons, the allowance has no step-10 line, the step-11 line is unchanged, and `make button-guard` prints `files 5 reads 31`.
- **H-10-6. Step 10 leaves step 11's code alone.** `build_step`, `placement_step`'s two button reads, the comments at `PARKED_X` and in `placement_step`, `bar_or_cycle_step`, the shared `direction` and `pad_jump_first_direction` are as at HEAD.
- **H-10-7. Step 10's new tests survive step 11.** Wherever they reach the bar or placement, they bind `aim.open_build_bar = 9` and `build_bar.take_structure = 5` as reach-only lines, and in placement they press only pan buttons, never a button bound or meant for `place_structure` or `cancel_placement`. So converting `build_step` and `placement_step` keeps them green.
- **H-10-8. Existing pad tests are unchanged.** They still start with `&bindings_compiled`, and `take_structure` still presses button 5.
- **H-10-9. The guardrail outputs are byte-identical.** After step 10, `make pad-author` regenerates `worst-case/` with no diff, and `make pad-author-proof`, `make pad-metrics-check` and `make pad-metrics-worst-case` give the same outputs as at step 9's end.

**Rule if a hypothesis is wrong.** Follow the committed step 10 code at this step's own `HEAD`, use its names, and record the difference in the step note (`### Step 2.11`). In particular:

- **The pan (H-10-1).** If `placement_step` still calls `aim_pan` without a mode or with the aim mode, that is step 10's row 11, not this step's. Record it and do not fix it here. Test 1 binds `placement.pan_*` on 1–4 and `aim.pan_*` on 1–4, so it passes either way.
- **The field (H-10-2).** Use whatever name it has. Do not rename it here.
- **Aim mode (H-10-3).** If aim mode still hardcodes 5 or 7, test 1's aim-mode neighbour check fails for a reason outside this step. Drop that check, keep the rest, and record it.
- **Guard counts (H-10-5).** If they differ, what matters is that the step-11 line goes and `pad_nav.c` names exactly 3 fewer buttons than at the step's start.
- **Step 10's tests (H-10-7).** If one breaks because the bar or placement now reads the table, add the missing action to its table on its default button, change nothing else in it, and record it.
- **Guardrail outputs (H-10-9).** If one differs at step 11's start, use the tree's output as the baseline. Step 11 must not change it.

## User Stories

1. As a player, I want the button bound to `build_bar.pick_left` to move the bar's focus to the icon on the left, so that I can remap the bar's directions.
2. As a player, I want the button bound to `build_bar.pick_right` to move the focus to the icon on the right, so that both directions follow my file.
3. As a player, I want the bar to stop at its ends as today, without wrapping, whatever buttons the picks are on, so that only the button changes.
4. As a player, I want a button bound to `panels.move_up` or `panels.move_down`, pressed on the same frame as a pick, to stop the pick as up and down do today, so that the bar keeps plan 12's ranking of the D-pad wherever I move it.
5. As a player, I want `pick_left` to win when both picks are pressed on the same frame, as left does today, so that a chord stays predictable.
6. As a player, I want the button bound to `build_bar.take_structure` to take the focused structure, so that I can choose the button that starts a placement.
7. As a player, I want a take button held from before the bar opened to take nothing until it is pressed again, as today, so that remapping keeps the safety.
8. As a player, I want the button bound to `placement.place_structure` to drop the carried structure, so that I can place on my chosen button.
9. As a player, I want holding the take button after taking not to drop the structure, so that taking and placing stay two presses even when they share a button.
10. As a player, I want the button bound to `placement.cancel_placement` to cancel the carried structure at its icon and charge nothing, so that I can cancel on my chosen button.
11. As a player, I want the bar's old default buttons to do nothing in the bar and in placement once I move those actions, so that the modes follow my file only.
12. As a player, I want placement's buttons to do nothing in aim mode when aim does not bind them, so that the modes stay independent.
13. As a player, I want a bar or placement action left out of my file to do nothing, so that an unbound action is never silently played.
14. As a player, I want the default bindings to play exactly like plan 12 on every recorded path, so that nothing changes without a personal file.
15. As a maintainer, I want every e2e `.sha256` and end state unchanged, so that this step is a proven refactor.
16. As a maintainer, I want `make pad-author-proof`, `make pad-author` with no diff in `worst-case/`, `make pad-metrics-check` and `make pad-metrics-worst-case` unchanged, so that the devtools still agree with the game.
17. As a maintainer, I want the four reads converted with step 9's `action_pressed`, so that every converted mode reads the table the same way.
18. As a maintainer, I want `build_step` to stop reading the shared pressed direction, so that step 12 can dissolve it without touching the bar.
19. As a maintainer, I want the up-or-down block kept and recorded in the step note, so that step 12 and reviewers know where the bar's up and down come from.
20. As a maintainer, I want the step-11 line gone from the guard allowance and `pad_nav.c` to name exactly three fewer buttons, so that the guard proves the conversion and blocks a regression.
21. As a maintainer, I want the `×` gone from the comment above `PARKED_X` and from `placement_step`'s drop comment, so that step 14's grep finds no PSX word left by this step.
22. As a maintainer, I want the existing pad unit tests unchanged, so that plan 12's behaviour stays pinned by the same expected values.
23. As a maintainer, I want a unit test where the bar and placement follow a table with every action off its default button, so that a read left hardcoded fails.
24. As a maintainer, I want that test to give `take_structure`, `place_structure` and `cancel_placement` three different buttons, so that a read tagged with the wrong one of these modes fails.
25. As a maintainer, I want the same test to show that placement's buttons do nothing in aim mode, so that an aim read tagged with the placement mode fails too.
26. As a maintainer, I want a unit test pinning plan 12's pick rule with the default table, so that the block cannot silently disappear.
27. As a maintainer, I want the tests to bind actions of other modes only as reach-only lines on their default buttons, so that steps 12–13 converting those modes keep the tests valid.
28. As a maintainer, I want a one-off mutation proof recorded in the step note, so that I know the new tests catch a wrong mode or a hardcoded read.
29. As a maintainer, I want `make test` and `make e2e` no slower, so that the fast loop stays fast.
30. As a reviewer, I want the step note to list which H-10-k held, so that I can see how the parallel step 10 landed against this one.

## Implementation Decisions

### The build bar reads the table

- `build_step` loses its `direction` parameter: `build_step(nav, pressed, release, played)`. `bar_or_cycle_step` keeps its own `direction` parameter for `panel_step` (step 12).
- Row 14: `icon != NULL && action_pressed(nav, pressed, BINDINGS_MODE_BUILD_BAR, BINDINGS_ACTION_TAKE_STRUCTURE) && !release`. Same effects and order as today.
- Row 15: after the take, as today, the pick follows the rule of the decision below. `pick_left` and `pick_right` are read through `action_pressed` with `BINDINGS_MODE_BUILD_BAR`. The block reads `pressed` against `panels.move_up` and `panels.move_down` (through `action_pressed` with `BINDINGS_MODE_PANELS`, or one combined mask). No `!release` gate and no icon gate, as today.

### Decision: the bar keeps plan 12's up-or-down block (guideline, option b)

- **Today.** The bar moves on the first pressed direction in the order up, down, left, right. So 1 + 3 pressed on one frame moves nothing, and so does 2 + 4, while 3 + 4 moves left. Step 2.5 flagged this as step 11's trap. Section 2's `build_bar` line lists only `pick_left 3, pick_right 4`.
- **Decision.** Plan 12's behaviour is kept, decided by `guideline`. Section 2 says every step until Phase 4 is a pure refactor for the default bindings, that a step follows the code where the table is silent, and that changing plan 12's controls is out of scope. An unchanged e2e does not excuse a control change, and the pad tests pin plan 12 on paths no replay presses. The rejected option let the bar read only its own actions, so 1 + 3 would pick.
- **Rule.** In `build_step`, a pick happens only on a frame where no button in `bindings_mask(PANELS, MOVE_UP) | bindings_mask(PANELS, MOVE_DOWN)` is pressed. When that holds, `build_bar.pick_left` fires if its mask is pressed, else `build_bar.pick_right` if its mask is pressed, so left wins when both are pressed. With the default table this equals today for every combination of buttons 1–4. The rule reads only masks and names no button. It does not depend on the shared direction, which step 12 dissolves (inventory row 24).
- **Why `panels`.** The bar has no up or down action. `panels` is the off-map mode that shares the D-pad ranking with the bar, so the bar borrows its up and down. A personal file that moves `panels.move_up` moves the bar's block with it.
- **Record.** The step note records the difference from section 2's table: its `build_bar` line does not list the block, and the bar borrows `panels.move_up`/`move_down` as the off-map pair that shares the D-pad ranking. Section 2's table itself is not amended.

### Placement reads the table

- Row 16: `action_pressed(nav, pressed, BINDINGS_MODE_PLACEMENT, BINDINGS_ACTION_CANCEL_PLACEMENT) || pad_nav_panel_open()`.
- Row 17: `if (action_pressed(nav, pressed, BINDINGS_MODE_PLACEMENT, BINDINGS_ACTION_PLACE_STRUCTURE)) return;`.
- No other line of `placement_step` changes. The engagement reset and the `aim_pan` call are step 10's (H-10-1, H-10-2). The cancel's early return still runs before the pan, and the place check still runs after it.
- `pad_nav_step` still clears button 6 from `pressed` when ○ closes the message box (row 33, step 13). With the default table this keeps the carry under a closed year box, as today. With `cancel_placement` moved off 6, the message box still closes on 6 until step 13. That is step 13's to settle, and no step 11 test shows a message box.

### Comments on converted code

- Above `PARKED_X`: "Off screen, so a press finds no left_press under the cursor." Both users of the parked point belong to this step's modes: the bar with no icon, and `cancel_placement` with no icon.
- In `placement_step`: "place_structure drops by letting the held left button go for one frame: the ui's drop runs the cart's rules."
- No other comment changes. `pad_panel.h`'s `×` is step 12's, and `circle_*` names are step 13's. No new comment unless it carries a trap (`CLAUDE.md`).

### Guard allowance

- When the step ends, `tools/button_guard_allowance.txt` has lost exactly the line `src/game/pad_nav.c step-11 3 | …`, and nothing else in it has changed. `make button-guard` goes from `files 5 reads 31` to `files 5 reads 28` (H-10-5).
- Between commits the allowance must stay valid. The bar commit converts row 14 (one name) and lowers the line to `src/game/pad_nav.c step-11 2 | placement_step: placement.cancel_placement, placement.place_structure`. The placement commit deletes the line.

### No public interface changes

- No header changes. `build_step` is static. No Makefile link line changes.

## Implementation Chunks

1. **The bar reads the table.** Rows 14–15, `build_step` without `direction`, the `PARKED_X` comment, and the allowance line lowered to `step-11 2`. Add test 1's bar half and test 2. Verified: `make test-all`, every `.sha256` and end state unchanged, and the four guardrail commands unchanged.
2. **Placement reads the table.** Rows 16–17, the drop comment, and the allowance line deleted. Add test 1's placement half and its aim-mode neighbour check. Verified: everything in Testing Decisions.

## Testing Decisions

- **What a good test is.** It drives frames through `input_step_run` and asserts what the player sees: the mode, the bar's focused icon, the played mouse position and button, the carried structure, the structure's state, credits, probes. It never reads static state.
- **Existing tests do not change** (section 2 guardrail). One helper may gain a parameter mechanically: `take_structure` may become a call of `take_structure_with(step, button)` with button 5, with no assertion changed. An assertion that must change is a divergence between the code and section 2's table: follow the code and record it in the step note.
- **Tables.** Each test parses its own table from inline text with `bindings_parse` (as `rotated_jump_table` does), asserts the parse succeeds, and starts through `start_title_with`, then `cart_start_game` with "forgiving" and `settle`, as `start_game` does.
  - The converted actions sit off their default buttons, so a read left hardcoded fails.
  - Some actions of modes that other steps convert are needed to reach a mode or move the camera. These are bound on their default buttons and marked in a comment as reach-only lines, so steps 12–13 converting those modes keep the test valid. Every other mode stays unbound.
- **Checks in forks.** Each check that changes the mode or the world runs in its own `in_forked_copy`. Prior art: `test_build_left_and_right_move_between_the_icons_without_wrapping`, `test_cross_takes_the_structure_and_a_second_cross_places_it`, `test_circle_cancels_the_structure_and_charges_nothing`, `test_the_cross_that_takes_the_structure_drops_nothing_while_held`, and step 9's `test_jump_mode_follows_a_rotated_table_and_slipway_mode_stays_unbound`.

### Test 1: the bar and placement follow a moved table, and aim mode does nothing on their buttons

The table:

```
aim.pan_up = 1                # reach-only (step 10): pan_to_empty_space
aim.pan_down = 2
aim.pan_left = 3
aim.pan_right = 4
aim.open_build_bar = 9        # reach-only (step 13)
build_bar.pick_left = 4
build_bar.pick_right = 3
build_bar.take_structure = 8
placement.pan_up = 1          # reach-only (step 10): pan_clear_of_every_node
placement.pan_down = 2
placement.pan_left = 3
placement.pan_right = 4
placement.place_structure = 7
placement.cancel_placement = 5
panels.move_up = 1            # reach-only (step 12): the bar's pick block
panels.move_down = 2
```

`aim.probe_or_fold` and `aim.leave_aim` are unbound, and so are jump, slipway, menu, pause and title, and every other panels action. Leaving `panels.move_up`/`move_down` unbound would make the pick block empty: 1 + 4 would pick, so the block check below needs them.

- **Neighbour: aim mode.** In a fork, `pan_to_empty_space`, then tap 5, then tap 7. On each press frame no mouse button is played. No probe exists, and the game is still in aim mode. With the default table, 5 would place a probe and 7 would leave aim mode. This catches an aim read tagged with placement's mode (H-10-3).
- **Bar picks.** `open_build_bar` (taps 9). The focus is the basic structure icon. Tap 4: the focus is `processor_icon()`. Tap 4 again: it stays there (no wrap). Tap 3: the focus is back on the basic icon. With the bar still reading the shared direction, the first tap of 4 would keep the basic icon.
- **The block follows `panels`.** In a fork, from the basic icon, press 1 + 4 on one frame: the focus stays on the basic icon. Press 2 + 4: it still stays. One frame with nothing, then press 4 alone: the focus is `processor_icon()`. A block read from `build_bar` (which has no up or down) or left out would pick on 1 + 4.
- **Old take button.** In a fork, tap 5 on the bar: no mouse button is played, nothing is held, and the bar is still open.
- **Take on 8.** Press 8: the left button is played at the icon's focus point. One frame with nothing pressed: the structure is carried (`assert_carrying`). Then hold 8 for 10 frames: still carried on every frame.
- **Placement checks**, after `pan_clear_of_every_node`, each in its own fork:
  - Press 7, then a frame with nothing: the structure is settled and the credits went down by its cost (`assert_placement_over`).
  - Press 5: the release is played at the basic icon's focus point. One more frame: the structure is dead, the world is unchanged, and the game is in aim mode with the camera where it was.
  - Press 6, then a frame with nothing: still carried on both frames. Press 8, then a frame with nothing: still carried. This catches `place_structure` tagged with the bar's `take_structure`, and `cancel_placement` left on 6.

### Test 2: with the default table, up or down still block a pick

- `start_game`, `open_build_bar`. Press 1 + 3 on one frame: the focus stays on the basic icon. One frame with nothing. Tap 3: the focus is `processor_icon()`. Press 2 + 4: it stays on `processor_icon()`. One frame with nothing. Press 3 + 4 from the basic icon (tap 4 first to return): the focus is `processor_icon()` (left wins).
- This test passes before and after the step: it pins plan 12's rule on a path no replay presses.

### One-off mutation proof

Record it in the step note and do not commit it. Restore each mutation after it is checked.

- Tag the take read with `BINDINGS_MODE_PLACEMENT, BINDINGS_ACTION_PLACE_STRUCTURE`. Test 1 must fail at the take on 8.
- Leave row 17 hardcoded on `P8_PAD_BUTTON_5`. Test 1 must fail. `make button-guard` must report `src/game/pad_nav.c names 19 button(s), listed 18`.
- Drop the `panels` block from the pick. Test 1's block check and test 2 must fail.
- Read the block from `BINDINGS_MODE_BUILD_BAR` instead of `BINDINGS_MODE_PANELS`. Test 1's block check and test 2 must fail.
- Put back the shared-direction pick in `build_step`. Test 1's bar picks must fail.

### Step verification, in this order

1. `make test-all`: green, every `tests/e2e/*.json.sha256` unchanged, and `make end-state` unchanged on every replay.
2. Guardrails (section 2; this step touches `src/game/pad_*`): `make pad-author-proof`, then `make pad-author` followed by `git diff --exit-code worst-case/`, then `make pad-metrics-check` and `make pad-metrics-worst-case`. All unchanged from the tree at step 11's start (H-10-9).
3. `make button-guard` passes with the step-11 line gone. `git grep -o P8_PAD_BUTTON_ -- src/game/pad_nav.c | wc -l` counts exactly 3 fewer than at the step's start. `git diff <start> -- tools/button_guard_allowance.txt` shows only that line deleted.
4. `build_step` names no `direction`. `git grep -n '×' -- src/game/pad_nav.c` no longer finds the `PARKED_X` and drop comments.
5. `make raylib` compiles and links. It is never run.
6. Timings: `make test` and `make e2e`, three warm runs each, against step 1's baseline (4.27–4.58 s and 0.24–0.25 s). If `test_pad_nav` gets measurably slower, reduce frames, not checks.

## Out of Scope

- The shared pan `aim_pan` and its mode, the engagement field's rename, and aim and slipway reads (step 10).
- `build_bar.close_bar`, `build_bar.open_build_bar`, `*.open_tech_panel`, `build_bar.pause`, `placement.open_build_bar`, `placement.pause`, and the message-box close on placement's ○ action (step 13). With a table that moves these, the bar and placement still answer plan 12's buttons for them until step 13.
- The panels' `move_*`, `choose_row` arming, `pad_panel.h`'s `×`, and the deletion of the shared pressed direction and `pad_jump_first_direction`/`pad_jump_direction_button` (step 12). `bar_or_cycle_step` keeps passing `direction` to `panel_step`.
- Renaming test functions or comments in `tests/` that carry PSX words (`test_cross_takes_the_structure_…`, …), because tests change only mechanically.
- Any change under `src/devtools/`. The generator already presses `build_bar.*` and `placement.*` from the default table (step 8).

## Further Notes

- **Trap: three actions share button 5 by default.** `aim.probe_or_fold`, `build_bar.take_structure` and `placement.place_structure` are all on 5, so a read tagged with the wrong one passes with the default. Test 1 leaves `aim.probe_or_fold` unbound and puts `take_structure` on 8 and `place_structure` on 7.
- **Trap: the take press is still held when placement starts.** The take happens on the press frame, and placement begins the next frame with the button held. Placement reads `pressed`, never `pad_buttons`, for both its actions. A held read would drop the structure at once when take and place share a button.
- **Trap: the message box eats button 6, not `cancel_placement`.** Until step 13, `pad_nav_step` clears button 6 from `pressed` when ○ closes the year box. A test that moves `cancel_placement` and shows a year box would see the box stay and the carry cancel. Keep such a case out of step 11's tests.
- **Trap: the bar's block lives in another mode.** The bar's pick reads `panels.move_up`/`move_down`. Step 12, converting the panels, must keep those two actions where the bar can read them, and must not drop the block when it dissolves the shared direction.
- The step note (`### Step 2.11`) records:
  - which H-10-k held, and how the code adapted where one did not;
  - the converted reads by action;
  - the difference from section 2's table: the `build_bar` line does not list the up-or-down block, and the bar borrows `panels.move_up`/`move_down` as the off-map pair sharing the D-pad ranking (test 2 pins it);
  - the mutation proof's results;
  - the guard output before and after;
  - the guardrail runs;
  - the timings.
