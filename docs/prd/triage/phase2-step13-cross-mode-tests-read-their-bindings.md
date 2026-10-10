---
title: "Cross-mode tests read their bindings"
description: "The reads that run before the mode dispatch (pause opening, the shoulders, Select, the back button that closes a box or the message box, and the cheat-chord mask) still test hardcoded buttons, so a bindings table changes nothing in them."
status: needs-triage
---

## Problem Statement

This is plan 17, step 13 (Phase 2, bindings in the game). The step's text, from `docs/plan-17.md` section 3, as amended:

> 13. **Cross-mode tests read their bindings:** pause, the shoulders, Select, and `back` closing the message box. The mode is computed from the nav state where each test runs. Verified the same way. Moving the chord mask behind the cheat module also renames `cheat.c`'s `cross`/`circle` locals.

"The same way" means step 9's verification and rule:

> Verified: e2e unchanged, plus a unit test with a table that swaps actions in the converted mode and leaves every other mode unbound: the converted mode follows the table, and the same buttons do nothing in a neighbouring mode served by the same shared code. Without it a read tagged with the wrong mode passes, because the default puts most actions on the same button in every mode. In steps 9–13, the step that converts a read also renames the local or comment that carries a PSX name on that line (`cross_engaged`, `circle_returns`, a `×` in a comment, …) to the action's name.

Section 2's locked decisions this step implements:

- **The cheat chord stays fixed on buttons 5 + 6**, below the bindings. `without_chord` takes the chord mask from the cheat module, so no button name is left in `pad_nav.c`.
- **On the map, the mode's ○ action** (`jump.back`, `aim.back`, `slipway.cancel_slipway`, `placement.cancel_placement`) **first closes a shown message box** (the year box) and does nothing else. There is no separate message-box mode.
- **Replace on the spot.** Where a test runs before the mode dispatch (pause, the shoulders, Select, ○ closing a box), the mode is computed from the nav state at that point.

Since step 7, `pad_nav` holds a copy of the compiled bindings table. Steps 9–12 made every mode's own reads use it. What is left is the code in `pad_nav_step` and its helpers that runs before the dispatch, for several modes at once. Whatever `bindings.cfg` says:

- L1 opens or closes the build bar and R1 opens tech or cycles the panels (exactly one of the two pressed).
- Start opens pause from jump, aim, slipway, the bar and placement.
- Select held drives the tap and the year skip.
- ○ closes a non-pause box, otherwise closes a shown message box on the map, otherwise goes back (jump), leaves aim, cancels the slipway; off the map, ○ closes the bar or the tech/score panel.
- `without_chord` hides buttons 5 and 6 by naming them itself.

A player who moves `*.open_build_bar`, `*.open_tech_panel`, `panels.next_panel`, `*.pause`, `*.select_tap_or_year`, `*.back`, `slipway.cancel_slipway`, `placement.cancel_placement`, `menu.close_menu`, `panels.close_panel` or `build_bar.close_bar` sees nothing change. `pad_nav.c` is the last game file that names buttons outside the cheat and bindings modules.

## Solution

Each pre-dispatch read asks the table, for one mode decided where the read runs:

- **Shoulders.** The current mode, decided once in `switch_panels` before anything is unwound. The bar shoulder is that mode's `open_build_bar`. The tech shoulder is `panels.next_panel` on the panels, `open_tech_panel` elsewhere. They act when exactly one of the two is pressed, as today.
- **Pause opening.** The current mode's `pause`.
- **Select.** One "held" signal, the union of `jump.select_tap_or_year` and `aim.select_tap_or_year`, read in every mode so the latch still stops a Select held from another mode from firing on return.
- **○ (the back family).** With a non-pause box open: `panels.close_panel` while a cycle panel is open, `menu.close_menu` otherwise. In every other case, the map mode's ○ action: `placement.cancel_placement` while carrying, else `aim.back`, `slipway.cancel_slipway` or `jump.back`. The same rule holds under pause and on the title, where it acts on nothing, as today. That one action closes the box, or closes the message box and is then removed from the frame's presses, or goes back.
- **○ off the map.** `build_bar.close_bar` while the bar is open, `panels.close_panel` otherwise.
- **Cheat chord.** `pad_nav` asks the cheat module for the chord's mask. `cheat.c` names buttons 5 and 6 once each and loses its PSX locals and fields.

With the default table every frame is identical. Unit tests prove, for each family, that a mode follows a table with its actions off their default buttons while a neighbouring mode served by the same code stays unbound and does nothing. One more test shows that the chord still hides buttons 5 and 6 when the table binds them.

The guard allowance loses its last converting line, `src/game/pad_nav.c step-13 11`. After this step only `src/game/cheat.c fixed 2` and `src/game/bindings.c fixed 3` remain: step 14 closes the guard.

## Facts checked at HEAD 04912c2 (step 11 committed, step 12 not committed)

- `tools/button_guard_allowance.txt` at HEAD holds three step-12 lines, `src/game/pad_nav.c step-13 11 | one_shoulder (2), shoulder_switch, switch_panels, pause_step opening, select_down, circle_returns, the box and message-box close (2), without_chord (2)`, `src/game/cheat.c fixed 2` and `src/game/bindings.c fixed 3`.
- The guard (`tools/button_guard.sh`) counts names in `.c` **and** `.h` files of `src/game/`, outside comments and strings. A chord mask defined in `cheat.h` would be a new, unlisted read: the mask must stay inside `cheat.c`.
- Step 2.5's rows owned by step 13, at HEAD's `pad_nav.c` line numbers (step 12 will move them):
  - Rows 26–27: `one_shoulder(pressed)` (lines 479–481): pressed 10 xor pressed 9. `shoulder_switch` (483–495): pressed 10 → close the bar, cycle the panels; else toggle the bar. Both are called from `switch_panels` (850–853) as `one_shoulder(pressed) && shoulders_can_switch(nav, release)`, then `unwind_sub_modes`, then `shoulder_switch`. `unwind_sub_modes` clears a planet menu's ephemerals and cancels a slipway, so a mode decided after it is wrong.
  - Row 28: `switch_panels` line 855: `!on_the_map(nav) && (pressed & 6) && !foreign_was_open && !release` → `leave_panel`, `close_panels`.
  - Row 29: `pause_step` line 910: `due || (pressed & 11)` → `pause_button_free`. That function already refuses with no game, any panel, a click release, a slipway release being judged, a fold engaged, or a cycle panel. Pause therefore opens only from jump, aim, slipway, the bar or placement.
  - Row 30: `pad_nav_step` line 995: `select_down = (pad_buttons & 12) != 0`, fed to `pad_select_step` both on the pause frame (with `allowed = 0`) and later.
  - Rows 31–33: `pad_nav_step` lines 1005–1013. `circle_returns = (pressed & 6) && !pad_panel_box_open()`. If 6 is pressed: `circle_closes_the_box()` (clears the ephemerals when a box is open and pause is not shown), then, if `circle_returns && on_the_map && shown_message_box()`: `message_box_close_shown()`, `circle_returns = 0`, `pressed &= ~6`. `circle_returns` goes to `map_step`, which passes it to `aim_mode_step`, `slipway_step` and `jump_step`.
  - Row 34: `without_chord` lines 965–973: `face = 5 | 6`, latched in `nav->chord_held` from a frame where both go down until both are up.
  - Row 35 (fixed): `cheat_step` (`cheat.c` lines 34–35) reads held 5 and held 6. The locals are `cross`, `circle`, `cross_edge`, `circle_edge`, and `cheat_t`'s fields are `previous_cross` and `previous_circle` (`cheat.h`). `cheat.h`'s comment on `cheat_step` names × and ○.
- The pause rows are ui ephemerals (`pause_menu_shown` scans `ui->ephemerals`), so under pause `pad_panel_box_open()` holds, `circle_returns` is 0, and `circle_closes_the_box` skips pause. Under pause, rows 31–33 act on nothing whatever their mode.
- `pad_nav_placing(nav)` is non-NULL only while carrying (`PAD_NAV_PLACEMENT_CARRYING` and the ui holding a placing structure). `map_step` checks it before any panel, so on the map placement beats an open panel.
- `cheat.c` is in `ENGINE_SRCS` next to `pad_nav.c`, and in the same core list. Every binary and test that links `pad_nav.c` links `cheat.c`, so no Makefile link line changes. No test names `previous_cross` or `previous_circle`. `tests/test_pad_nav.c` reads `cheat.chords` only.
- `bindings.default.cfg` binds, in every mode that has them: `open_build_bar = 9`, `open_tech_panel = 10`, `panels.next_panel = 10`, `pause = 11`, `select_tap_or_year = 12`, `back = 6`, `cancel_slipway = 6`, `cancel_placement = 6`, `close_menu = 6`, `close_panel = 6`, `close_bar = 6`. `bindings.h` has every mode and action needed. `pad_nav.c` has `action_pressed` and `action_held`.
- `git grep -E 'cross|circle|square|triangle|×|○|△|□'` over `src/game/pad_*` and `src/game/cheat.*` at HEAD finds, apart from step 12's lines: the `circle_returns` parameters and locals, `circle_closes_the_box`, `cheat.c`'s locals and `cheat.h`'s fields and comment, and `ui_set_pad_crosshair` (`crosshair`, a ui name, not a PSX word).
- Test prior art in `tests/test_pad_nav.c`: `start_title_with`, `begin_game_with`, `start_game_with`, `parsed_table`, `in_forked_copy`/`end_forked_copy`, `run_frame`, `tap`, `hold`, `settle`, `hold_and_measure`, `assert_moved`, `camera_at`, `open_build_bar`, `open_tech`, `take_structure`, `assert_carrying`, `assert_placement_over`, `settled_row`, `attention_row`, `hover_object`, `start_slipway`, `assert_slipway_mode`, `assert_hovering`, `year_box_with_the_last_object_elsewhere`, `circle_closes_the_year_box`, `score_panel_new_year`, `year_facts`/`assert_years_ended`, `year_skip_point`, `known_planet_under_centre`/`known_planet_closest_to_centre`, `planet_left_press`, `galaxy_menu_on_click`, `assert_shoulders_wait`, `type_chords`. Existing tests on these reads: `test_a_held_shoulder_moves_one_tab_and_both_together_move_none`, `test_r1_cycles_tech_score_closed_from_the_map_and_opens_tech_from_the_bar`, `test_circle_on_a_tab_returns_to_the_map`, `test_circle_closes_the_year_box_over_*`, `test_circle_closes_a_planet_menu_then_the_year_box`, `test_select_does_nothing_while_something_is_held_or_open`, `test_start_*`, `test_the_pad_hides_cross_and_circle_exactly_from_the_chords_cheat_step_counts`, `test_final_score_ignores_shoulders_circle_and_start`, and steps 9–11's table tests.
- Steps 9–11's table tests at HEAD bind the step-13 actions they press on their default buttons, as reach-only lines (`aim.open_build_bar = 9`). The ones that press 6 in placement (`test_the_bar_and_placement_follow_a_moved_table`) expect nothing to happen, and with `placement.cancel_placement = 5` that still holds after this step.

## Hypotheses on step 12's outcome

Step 12 ("Menu, panels, pause and title read their bindings", PRD `docs/prd/triage/phase2-step12-menu-panels-pause-and-title-read-their-bindings.md`) is being implemented in parallel and is not committed. This PRD was written from HEAD 04912c2 and that PRD. Each assumption about step 12 is a hypothesis:

- **H-12-1. `panel_mode` exists.** `pad_nav.c` has `static bindings_mode_t panel_mode(const pad_nav_t *nav)` returning, in order, `PAUSE` when pause is shown, `TITLE` with no game, `PANELS` while a cycle panel is open, and `MENU` otherwise.
- **H-12-2. No shared direction.** `pad_nav_step` no longer computes a direction. `map_step`, `bar_or_cycle_step` and `panel_step` have no `direction` parameter. `pad_jump_first_direction` and `pad_jump_direction_button` are gone from `src/game/`.
- **H-12-3. Arming renamed, resets in place.** The fields are `pause_choose_row_armed` and `menu_choose_row_armed`. Their resets at the top of `pad_nav_step` keep their conditions and still run before `select_down` and `pause_step`.
- **H-12-4. `pause_step` half converted.** Its pause-shown branch reads `pause.close_pause` through `action_pressed`. Its opening branch still reads `P8_PAD_BUTTON_11` (row 29), unchanged.
- **H-12-5. Step 12 leaves step 13's code alone.** `one_shoulder`, `shoulder_switch`, `switch_panels`, `shoulders_can_switch`, `unwind_sub_modes`, `pause_button_free`, `select_down`, `circle_returns`, `circle_closes_the_box`, the message-box block, `without_chord` and `cheat.{c,h}` are as at HEAD, apart from line numbers.
- **H-12-6. Guard after step 12.** The allowance holds exactly `src/game/pad_nav.c step-13 11`, `src/game/cheat.c fixed 2` and `src/game/bindings.c fixed 3`. `pad_nav.c` names 11 buttons, and `make button-guard` prints `button-guard files 3 reads 16`.
- **H-12-7. Helpers.** `action_pressed` and `action_held` are unchanged. Step 12 adds no helper step 13 must reuse, besides `panel_mode`.
- **H-12-8. Step 12's tests survive step 13.** Its new tests bind the step-13 actions they need on their default buttons as reach-only lines (`aim.open_tech_panel = 10` in its tests 3 and 4). They press 6 or 11 only under pause, with `pause.close_pause` unbound or moved, and expect pause to stay. They press no other step-13 button whose outcome they assert, except to check that an unbound mode ignores it.
- **H-12-9. Test helpers.** In `test_pad_nav.c`, `tap_path`, `reach_all`, `expected_jump` and `slipway_pick` use test-local D-pad helpers in place of the deleted conversions, with the same behaviour. `hover_object`, `settled_row` and `attention_row` work as at HEAD.
- **H-12-10. Guardrail outputs.** After step 12, `make pad-author` regenerates `worst-case/` with no diff, and `make pad-author-proof`, `make pad-metrics-check` and `make pad-metrics-worst-case` give the same outputs as at step 11's end.

**Rule if a hypothesis is wrong.** Follow the committed step 12 code at this step's own `HEAD`, use its names, and record the difference in the step note (`### Step 2.13`). In particular:

- **`panel_mode` (H-12-1).** If it is missing or named differently, write `nav_mode`'s panel branches inline with step 2.5's rule (pause, title, panels, menu), and use step 12's helper if one exists under another name. Never let two helpers disagree on a panel's mode.
- **Step 12 touched step-13 code (H-12-5).** Convert what is there with the rules below. If step 12 already converted one of rows 26–34, leave it as it is, check that it follows this PRD's mode rule, and record it.
- **Guard counts (H-12-6).** If they differ, what matters is that the step-13 line goes, nothing else in the allowance changes, and `pad_nav.c` names no button when the step ends.
- **Step 12's tests (H-12-8) or steps 9–11's tests.** If one breaks because a step-13 read now asks the table, add the missing action on its default button as a reach-only line. Change nothing else in it, and record it.
- **Guardrail outputs (H-12-10).** If one differs at step 13's start, use the tree's output as the baseline. Step 13 must not change it.

## User Stories

1. As a player, I want the button bound to my current mode's `open_build_bar` to open the build bar, and to close it from the bar, so that L1's job follows my file.
2. As a player, I want the button bound to `open_tech_panel` to open the tech panel from the map, a box, the bar or placement, so that R1's job follows my file.
3. As a player, I want `panels.next_panel` to cycle tech, score and closed, so that the cycle follows my file.
4. As a player, I want pressing the bar and tech shoulders together to do nothing, as today, so that remapping keeps the "exactly one" rule.
5. As a player, I want a shoulder pressed with a planet's box open to close the box and act in one press, as today, whatever buttons the menu binds, so that remapping keeps that shortcut.
6. As a player, I want a shoulder pressed while carrying a structure to cancel it and act, as today, on the buttons placement binds.
7. As a player, I want the button bound to my mode's `pause` to open pause from jump, aim, slipway, the bar and placement, so that Start's job follows my file.
8. As a player, I want pause opened from placement to keep the structure for when pause closes, whatever button opens it.
9. As a player, I want `select_tap_or_year` to tap and to end the year on a half-second hold, on my buttons, in jump and aim modes.
10. As a player, I want a Select button held from another mode, from pause, a panel or the bar, to do nothing on my return until I press it again, as today.
11. As a player, I want `jump.back` to jump back to the last object I acted on, on my button.
12. As a player, I want `aim.back` to leave aim mode, on my button.
13. As a player, I want `slipway.cancel_slipway` to drop the slipway, on my button.
14. As a player, I want `placement.cancel_placement`'s first press to close a shown year box and keep my structure, and its next press to cancel the structure.
15. As a player, I want the ○ action of my map mode to close a shown year box and do nothing else on that frame, so that closing the box never also moves, leaves or cancels.
16. As a player, I want `menu.close_menu` to close any box over the map or over the bar, so that one action closes boxes.
17. As a player, I want `panels.close_panel` to close the tech and score panels, so that leaving the cycle follows my file.
18. As a player, I want `build_bar.close_bar` to close the bar, so that leaving the bar follows my file.
19. As a player, I want the old default buttons to do nothing for these actions once I move them, so that the game follows my file only.
20. As a player, I want a mode's buttons to do nothing in a neighbouring mode that does not bind them, so that the modes stay independent.
21. As a player, I want an action left out of my file to do nothing, so that an unbound action is never silently played.
22. As a developer, I want buttons 5 + 6 pressed together to type the cheat and stay hidden from every mode until both are up, whatever my bindings put on 5 and 6, so that the dev chord keeps working.
23. As a player, I want the default bindings to play exactly like plan 12, so that nothing changes without a personal file.
24. As a maintainer, I want one helper that decides the current mode for pre-dispatch reads, sharing the panel rule with `panel_step`, so that the shoulders, pause and the panels never disagree.
25. As a maintainer, I want one helper that decides the ○ action for the frame, so that the box close, the message-box close, the press removal and the map modes' back read the same buttons.
26. As a maintainer, I want the shoulders' mode decided before `unwind_sub_modes` runs, so that closing a box or cancelling a slipway cannot change which button counted.
27. As a maintainer, I want `pad_nav.c` to name no button and get the chord mask from the cheat module, so that the guard can close in step 14.
28. As a maintainer, I want `circle_returns`, `circle_closes_the_box` and `cheat.{c,h}`'s `cross`/`circle` names renamed, so that step 14's PSX-word grep finds nothing of this step's.
29. As a maintainer, I want the guard allowance to lose exactly the `pad_nav.c step-13` line, lowered chunk by chunk, so that each commit's guard is strict.
30. As a maintainer, I want every e2e `.sha256` and end state unchanged, so that this step is a proven refactor.
31. As a maintainer, I want `make pad-author-proof`, `make pad-author` with no diff in `worst-case/`, `make pad-metrics-check` and `make pad-metrics-worst-case` unchanged, so that the devtools still agree with the game.
32. As a maintainer, I want the existing pad tests unchanged, apart from adding `_with` variants of helpers that take a table, so that plan 12's behaviour stays pinned by the same expected values.
33. As a maintainer, I want one rotated-table test per family with an unbound neighbour, so that a read tagged with the wrong mode fails.
34. As a maintainer, I want a one-off mutation proof recorded in the step note, so that I know the new tests catch a wrong mode, a mode decided too late, or a hardcoded read.
35. As a maintainer, I want `make test` and `make e2e` no slower, so that the fast loop stays fast.
36. As a reviewer, I want the step note to list which H-12-k held and every place where the code differs from section 2's table, so that I can audit the step.

## Implementation Decisions

### Two mode helpers in `pad_nav.c`

- **`map_binding_mode(nav)`** (static) gives the bindings mode of the map: `PLACEMENT` when `pad_nav_placing(nav) != NULL`, otherwise from `map_mode_now(nav)`: `AIM`, `SLIPWAY`, or `JUMP`. It does not look at panels, pause or the game's existence.
- **`nav_mode(nav)`** (static) gives the mode the dispatch would run, from the nav state at the point of the call, in this order:
  1. On the map and carrying (`on_the_map(nav) && pad_nav_placing(nav) != NULL`): `PLACEMENT`. Carrying beats an open panel, as in `map_step`.
  2. `pad_nav_panel_open()`: `panel_mode(nav)` (H-12-1). That gives `PAUSE`, `TITLE`, `PANELS` for any panel while a cycle is open, and `MENU` for any other box or backdrop, including a panel over the bar.
  3. A cycle panel open with nothing shown yet (landing): `PANELS`.
  4. The bar open: `BUILD_BAR`.
  5. Otherwise `map_binding_mode(nav)`.
- `nav_mode` and `panel_step` agree wherever `panel_step` runs, because both go through `panel_mode`. `build_step` keeps reading the panels' up and down (step 11) and does not use `nav_mode`.

### Shoulders (rows 26–27)

- `switch_panels` computes `mode = nav_mode(nav)` once, after the no-game early return and before `one_shoulder`. It does not recompute it after `unwind_sub_modes`.
- `one_shoulder(nav, pressed, mode)` returns `action_pressed(…, mode, tech) != action_pressed(…, mode, OPEN_BUILD_BAR)`. Here `tech` is `NEXT_PANEL` when `mode == PANELS` and `OPEN_TECH_PANEL` otherwise, through one small static `tech_shoulder_action(mode)`.
- `shoulder_switch(nav, pressed, mode, played)` tests the same tech action in the same mode where it tested 10. The rest is unchanged: `leave_panel`, then cycle or toggle the bar.
- `shoulders_can_switch` and its call order are unchanged. In modes with neither action (`PAUSE`, `TITLE`) both masks are 0, and `shoulders_can_switch` already refuses there.
- The mode is read after the ○ block on the same frame. A ○ press that closes a planet's box on the frame a shoulder is pressed leaves the shoulder read in the map mode. That is the nav state where the test runs, and with the default it is the same button.

### Pause opening (row 29)

- `pause_step`'s opening branch: `due || action_pressed(nav, pressed, nav_mode(nav), BINDINGS_ACTION_PAUSE)`. `pause_button_free`, the deferred `pause_due` click and the suspend or cancel are unchanged. Modes without `pause` (menu, panels, title) give 0, and `pause_button_free` already refuses there.

### Select (row 30)

- `select_down = (pad_buttons & (bindings_mask(jump, SELECT_TAP_OR_YEAR) | bindings_mask(aim, SELECT_TAP_OR_YEAR))) != 0`, through one static `select_held(nav, pad_buttons)`. It is read in every mode, as step 2.5 locks: `pad_select_step`'s latch needs the held state while Select is not allowed (pause, bar, panels, slipway, placement), so that a Select held across a return to jump or aim does not fire.
- `select_allowed` is unchanged.

### The ○ action (rows 31–33)

- **`back_mode(nav)`** (static): `PANELS` when `pad_panel_box_open() && !pause_menu_shown() && cycle_open(nav)`; `MENU` when `pad_panel_box_open() && !pause_menu_shown()`; otherwise `map_binding_mode(nav)`. The "otherwise" also covers pause, the title and off the map with no box. There it acts on nothing, as 6 does today.
- **`back_action(mode)`** (static): `JUMP`, `AIM` → `BACK`. `SLIPWAY` → `CANCEL_SLIPWAY`. `PLACEMENT` → `CANCEL_PLACEMENT`. `MENU` → `CLOSE_MENU`. `PANELS` → `CLOSE_PANEL`. It asserts on any other mode, which `back_mode` never returns.
- In `pad_nav_step`, the block reads `back = bindings_mask(…, back_mode, back_action)` once:
  - `back_pressed = (pressed & back) != 0`.
  - `back_returns = back_pressed && !pad_panel_box_open()`.
  - If `back_pressed`: close the box (unless pause). Then, if `back_returns && on_the_map(nav) && shown_message_box() != NULL`: close the message box, `back_returns = 0`, and `pressed &= ~back`.
  - Order and conditions are unchanged. Row 33 removes the ○ action's own buttons, not button 6. Then a placement cancel or anything else reading that button later on the frame sees nothing, which is "does nothing else" in section 2.
- `back_returns` replaces `circle_returns` in `map_step`, `aim_mode_step`, `slipway_step` and `jump_step` (parameter names only). `circle_closes_the_box` becomes `back_closes_the_box`, unchanged otherwise.

### ○ off the map (row 28)

- In `switch_panels`: `!on_the_map(nav) && action_pressed(nav, pressed, pad_build_is_open(&nav->build) ? BUILD_BAR : PANELS, pad_build_is_open(&nav->build) ? CLOSE_BAR : CLOSE_PANEL) && !foreign_was_open && !release`. This is step 2.5's rule. It is read from the bar's state, not from `nav_mode`, because the ○ block may have just cleared a box over the bar on the same frame. `foreign_was_open` is still computed before the ○ block.

### Cheat chord (row 34) and `cheat.{c,h}`

- `cheat.h` declares `int cheat_chord_buttons(void);`. Its comment states the contract: the pad mask of the chord's two buttons, fixed below the bindings.
- `cheat.c` holds the only two names: a file-local `enum { CHORD_FIRST = P8_PAD_BUTTON_5, CHORD_SECOND = P8_PAD_BUTTON_6 };`. `cheat_chord_buttons` returns `CHORD_FIRST | CHORD_SECOND`, and `cheat_step` reads `CHORD_FIRST` and `CHORD_SECOND`. The guard's `cheat.c fixed 2` line stays true and unchanged. The mask must not move to `cheat.h`: the guard counts headers, and step 14 allows only `cheat.c`.
- Renames: locals `cross` → `first`, `circle` → `second`, `cross_edge` → `first_edge`, `circle_edge` → `second_edge`. Fields `previous_cross` → `previous_first`, `previous_circle` → `previous_second` (in `cheat_init` too). `cheat_step`'s header comment is rewritten without × and ○, for example "returns 1 on the frame that completes CHEAT_CHORDS chords of cheat_chord_buttons() (both going down on the same frame) with no lone press of either between them". Behaviour is unchanged.
- `without_chord` reads `const int face = cheat_chord_buttons();`, and `pad_nav.c` includes `game/cheat.h`. The latch logic is unchanged.

### Differences from section 2's table, followed from the code or from step 2.5 (record in the step note)

- **Select is a union.** A button bound to `select_tap_or_year` in only one of jump and aim also plays Select in the other, and its hold is tracked in every mode. Step 2.5 locks it for the latch. Section 2 lists the action per mode. In the default both are 12.
- **A box over the build bar closes on `menu.close_menu`**, not `build_bar.close_bar`. Step 2.5's wording lists `build_bar` among the box modes for rows 31–33. This PRD reads the box over the bar as a `menu` box, for three reasons: `panel_step` serves it in `menu` (step 12), `bindings.default.cfg` says `menu` covers "a panel over the build bar", and `build_bar` has no box-closing action (`close_bar` closes the bar, and `foreign_was_open` keeps the bar open on that frame). Default identical (all 6).
- **A HUD-opened tech box with no cycle panel closes on `menu.close_menu`.** Through the shoulder the same box closes on `panels.close_panel`. Same split as step 12's moves.
- **The ○ reads the map mode under pause, on the title, and off the map with no box.** It acts on nothing there: pause is a box but is never closed by this block, the message box needs the map with no box, and `back_returns` is unused off the map.
- **The final score panel** (a backdrop, no box, on the map) lets the map mode's ○ action close a shown message box, as 6 does today.
- **Exactly one shoulder.** A mode's `open_build_bar` and tech action pressed together do nothing. Section 2 does not say.
- **Same-frame order.** The ○ block runs before `switch_panels`, so a shoulder pressed on the frame ○ closes a box reads the mode left after the close.

### Guard allowance

- When the step ends, `tools/button_guard_allowance.txt` has lost `src/game/pad_nav.c step-13 11` and nothing else. `make button-guard` goes from `files 3 reads 16` (H-12-6) to `files 2 reads 5`.
- Between commits the line is lowered to the remaining count with its reason updated (see each chunk), so every commit's guard is strict.

### No other interface changes

- `cheat_chord_buttons` is the only new exported function, and `cheat_t`'s two fields the only renamed exported names. `pad_nav.h` does not change. No Makefile line changes. `src/devtools/` does not change.

## Implementation Chunks

1. **Chord mask from the cheat module.** `cheat_chord_buttons`, `cheat.{c,h}` renames and comment, `without_chord` reads the mask. Allowance: `src/game/pad_nav.c step-13 9 | one_shoulder (2), shoulder_switch, switch_panels, pause_step opening, select_down, circle_returns, the box and message-box close (2)`. Add test 1. Verified: `make test-all`, every `.sha256` and end state unchanged, the four guardrail commands unchanged, and `make raylib` links.
2. **Shoulders through the table.** `map_binding_mode`, `nav_mode`, `tech_shoulder_action`, `one_shoulder` and `shoulder_switch` take the mode, decided once in `switch_panels` before unwinding. Allowance: `src/game/pad_nav.c step-13 6 | switch_panels: close_bar/close_panel; pause_step opening; select_down; circle_returns, the box and message-box close (2)`. Add test 2. Same verification.
3. **Pause opening and Select through the table.** Row 29 reads `nav_mode`'s `pause`, and row 30 reads `select_held`. Allowance: `src/game/pad_nav.c step-13 4 | switch_panels: close_bar/close_panel; circle_returns, the box and message-box close (2)`. Add tests 3 and 4. Same verification.
4. **The ○ family through the table.** `back_mode`, `back_action`, rows 28 and 31–33, renames (`back_returns`, `back_closes_the_box`). The `pad_nav.c step-13` line is deleted. Add tests 5, 6 and 7. Verified: everything under Testing Decisions.

Row 28 goes with the ○ family, not the shoulders. On the tech panel, `panels.close_panel` is read both by the box close (row 32) and by `switch_panels` (row 28). Converting one without the other would leave the old button closing tech, and test 7's "6 does nothing" check would fail in between.

## Testing Decisions

- **What a good test is.** It drives frames through `input_step_run` and asserts what the player sees: which of the bar, tech, score, pause, a box or the year box is open; the mode (`pad_nav_aim_mode`, `pad_nav_jump_mode`, `assert_slipway_mode`, `assert_carrying`); the hovered object; the played mouse buttons and position; the camera; the year and the economy; cheats fired. It never reads `pad_nav_t`'s fields.
- **Existing tests do not change** (section 2 guardrail). Helpers that hardcode a default button for a step-13 action (`year_box_with_the_last_object_elsewhere`'s `start_game` and its slipway-cancelling tap of 6) get a `_with` variant taking a table and that button. The original keeps its body by calling the variant with `&bindings_compiled` and `P8_PAD_BUTTON_6`, as `start_title` calls `start_title_with`. Any assertion that must change is a divergence between the code and section 2's table: follow the code and record it.
- **Tables.** Each new test parses its table with `parsed_table` and starts through `start_game_with` (aim mode, nothing hovered). Lines marked `/* reach-only */` bind, on their default buttons, actions other steps converted that the test needs to reach a state (`aim.leave_aim = 7`, `jump.jump_* = 1–4`, `jump.act_on_hovered = 5`, `build_bar.take_structure = 5`, `aim.pan_* = 1–4` when a helper pans). Every other action stays unbound. Buttons for the converted actions are picked so that no reach-only line puts the same button in the same mode, and no button of the Select union (`jump`/`aim.select_tap_or_year`) is used for anything else in jump or aim.
- **Checks in forks.** Each check that changes the mode or the world runs in its own `in_forked_copy`.

### Test 1: the chord hides buttons 5 and 6 whatever the table binds on them (chunk 1)

Table: `aim.pan_up = 5`, `aim.pan_down = 6`.

- `hold_and_measure(5, 11)` pans up by plan 12's 11-frame distance, and `hold_and_measure(6, 11)` pans down, each in a fork. The table is live on 5 and 6.
- Press 5 + 6 on one frame and hold both for 11 frames, then release 6 and keep 5 held for 11 more frames: the camera has not moved (hidden from the chord frame until both are up). Release everything, then hold 6 alone for 11 frames: the camera pans down.
- With the same table, `type_chords` fires the cheat once.

### Test 2: the shoulders follow the current mode's table, placement stays unbound (chunk 2)

Table: `aim.open_build_bar = 10`, `aim.open_tech_panel = 9`, `build_bar.open_build_bar = 7`, `build_bar.open_tech_panel = 8`, `panels.next_panel = 7`, `panels.open_build_bar = 8`, `menu.open_tech_panel = 7`, and `build_bar.take_structure = 5` (reach-only). Placement and menu's `open_build_bar` are unbound.

- Aim: tap 9 + 10 on one frame: nothing opens. Tap 7 and 8: nothing opens. In a fork, tap 10: the bar is open. Tap 9: tech is open.
- Panels (tech): tap 9 and 10: tech stays. In a fork, tap 8: the bar is open and tech closed. Tap 7: score is open. Tap 7: back on the map.
- Bar (from aim by 10): tap 9 and 10: the bar stays. In a fork, tap 8: tech is open and the bar closed. Tap 7: the bar closes, back on the map.
- Menu: open a known planet's box with `planet_left_press` and settle. Tap 10 and 9: the box stays, nothing opens. Tap 7: the box is gone and tech is open, not the bar. This pins "mode decided before `unwind_sub_modes`": decided after it, the mode would be aim, where 7 is unbound.
- Placement neighbour: from aim tap 10, then `take_structure`. Tap 9, 10, 7 and 8, one at a time: still carrying (`assert_carrying`), nothing opened.

### Test 3: pause opens on the current mode's `pause`, neighbours stay unbound (chunk 3)

Table: `aim.pause = 7`, `build_bar.pause = 8`, `placement.pause = 12`, `aim.open_build_bar = 9` (reach-only), `build_bar.take_structure = 5` (reach-only).

- Aim: tap 11, 8 and 12: no pause. In a fork, tap 7 and settle: pause shown, camera unchanged.
- Bar (tap 9): tap 11, 7 and 12: no pause, the bar stays. In a fork, tap 8 and settle: pause shown and the bar still open.
- Placement (`take_structure`): tap 11, 7 and 8: still carrying, no pause. Tap 12 and settle: pause shown, the structure alive and no longer held by the ui (suspended).

### Test 4: Select reads the jump and aim union in every mode (chunk 3)

Table: `jump.select_tap_or_year = 10`, `aim.select_tap_or_year = 9`, `aim.leave_aim = 7`, `jump.jump_* = 1–4` (reach-only), `pause.close_pause = 11` (reach-only). Local helpers take the Select button: `hold_select_to_the_year_on(step, button)` and `select_tap_on(step, button)`, copies of the existing two with the button as a parameter.

- Aim: in a fork, hold 12 for `PAD_SELECT_HOLD_FRAMES + 1` frames: no mouse button and no year ended. In a fork, `hold_select_to_the_year_on(9)`: one year ended. In a fork, `hold_select_to_the_year_on(10)`: one year ended (the union).
- Latch across pause, in a fork from the start (aim mode): hold 9 for 5 frames, open pause directly (`galaxy_menu_on_click(4)(NULL)`) while 9 stays held, settle holding 9, then close pause with 11 while 9 stays held. Keep 9 held for `PAD_SELECT_HOLD_FRAMES + 5` frames: no year ended. Release one frame, then `hold_select_to_the_year_on(9)`: one year ended.
- Jump: `attention_row`, then `hover_object(first)`. Tap 12: still hovering `first`. `select_tap_on(10)`: hovering `last`. `select_tap_on(9)`: hovering `first` (the union).

### Test 5: ○ on the map follows jump's and aim's tables, each the other's neighbour (chunk 4)

Table: `jump.back = 8`, `aim.back = 12`, `slipway.cancel_slipway = 7`, `jump.jump_* = 1–4`, `jump.act_on_hovered = 5`, `jump.enter_aim = 7`, `aim.leave_aim = 7` (reach-only). Reached with `year_box_with_the_last_object_elsewhere_with(step, &table, P8_PAD_BUTTON_7, …)`: hovering `b`, last acted `a`, year box shown.

- Jump: tap 6 and 12: the year box stays, hovering `b`. Run 8 (the variant of `circle_closes_the_year_box` taking the button): the year box closes, hovering `b`, camera unchanged, last acted still `a`. Tap 8: hovering `a`.
- Aim, in a fork from the year-box state: tap 7 (aim mode). Tap 6 and 8: the year box stays, still aim mode. Tap 12: the year box closes, still aim mode, cursor at the centre. Tap 12: jump mode.

### Test 6: ○ over a slipway and over a carried structure (chunk 4)

Same table as test 5 plus `jump.open_build_bar = 9` and `build_bar.take_structure = 5` (reach-only), and `placement.cancel_placement = 12`.

- Slipway, in a fork: `start_slipway(b)`. Tap 6 and 8: the year box stays, `assert_slipway_mode(b, NULL)`. Press 7 (button-taking variant of `circle_closes_the_year_box`, left button held): the year box closes and the slipway stays. Tap 7: the slipway is dropped and `b` is hovered.
- Placement, in a fork: tap 9, then `take_structure`. Tap 6, 7 and 8: still carrying, the year box stays. Press 12: the year box closes and the structure is still carried and alive (row 33: 12 removed from that frame). Tap 12: the structure is dead and placement is over.

### Test 7: boxes and off-map ○ follow menu's, panels' and the bar's tables (chunk 4)

Table: `menu.close_menu = 7`, `panels.close_panel = 8`, `build_bar.close_bar = 12`, `aim.open_build_bar = 9`, `aim.open_tech_panel = 10`, `panels.next_panel = 10`, `aim.pan_* = 1–4` (reach-only).

- Menu: open a known planet's box with `planet_left_press` and settle. Tap 6, 8 and 12: the box stays. Tap 7: the box is gone, aim mode.
- HUD tech box over the map (`galaxy_menu_on_click(2)(NULL)`, no cycle), in a fork: tap 8: the box stays. Tap 7: it closes.
- Panels, in a fork: tap 10 (tech). Tap 6, 7 and 12: tech stays. Tap 8: back on the map. In another fork: tap 10, 10 (score), tap 6 and 7: score stays. Tap 8: back on the map.
- Bar, in a fork: tap 9. Tap 6, 7 and 8: the bar stays. Tap 12: back on the map.
- Box over the bar, in a fork: tap 9, then `galaxy_menu_on_click(2)(NULL)` and settle. Tap 12: the box and the bar stay. Tap 7: the box is gone and the bar is still open. If the HUD click cannot leave a box over the open bar, use any other box the ui can show over the bar and record which.

### Existing tests

- Every test in `test_pad_nav.c`, `test_input_step.c` and `test_economy.c` (`test_cheat_on_provisional`) runs unchanged on the default table. `test_button_guard.c` does not change.

### One-off mutation proof

Record it in the step note and do not commit it. Restore each mutation after it is checked. Run each named test alone.

- `without_chord` stops hiding (returns `raw_buttons`): test 1 fails.
- `switch_panels` recomputes the mode after `unwind_sub_modes`: test 2's menu check fails (the bar opens).
- `tech_shoulder_action` returns `OPEN_TECH_PANEL` for panels: test 2's panels check fails.
- Pause opening reads `map_binding_mode` instead of `nav_mode`: test 3's bar check fails.
- `select_held` reads only `nav_mode`'s Select (0 outside jump and aim): test 4's latch check fails.
- `select_held` reads only jump's mask: test 4's aim checks fail.
- `back_mode` ignores the box (always the map mode): test 7's menu check fails.
- `back_mode` returns `BUILD_BAR`/`CLOSE_BAR` for a box over the bar: test 7's box-over-the-bar check fails.
- `back_action` gives `BACK` for placement, or `back_mode` drops the placement branch: test 6's placement check fails.
- Row 33 clears `P8_PAD_BUTTON_6` instead of the ○ mask: test 6's placement check fails (the structure is cancelled on the closing frame), and `make button-guard` reports `src/game/pad_nav.c` naming a button it does not list.
- Row 28 reads `PANELS` for the bar: test 7's bar check fails.

### Step verification, in this order

1. `make test-all`: green, every `tests/e2e/*.json.sha256` unchanged, and `make end-state` unchanged on every replay.
2. Guardrails (section 2; this step touches `src/game/pad_*`): `make pad-author-proof`, then `make pad-author` followed by `git diff --exit-code worst-case/`, then `make pad-metrics-check` and `make pad-metrics-worst-case`. All unchanged from the tree at step 13's start.
3. `make button-guard` prints `button-guard files 2 reads 5`. `git diff <start> -- tools/button_guard_allowance.txt` shows only the deletion of the `pad_nav.c step-13` line. `git grep -c P8_PAD_BUTTON_ -- src/game/pad_nav.c` finds nothing.
4. `git grep -n -E 'circle|cross_|previous_cross|×|○' -- src/game/pad_nav.* src/game/cheat.*` finds nothing (`crosshair` does not match `cross_`).
5. `make raylib` compiles and links. It is never run.
6. Timings: `make test` and `make e2e`, three warm runs each, against step 1's baseline (4.27–4.58 s and 0.24–0.25 s). If `test_pad_nav` gets measurably slower, reduce frames, not checks.

## Out of Scope

- Closing the guard (step 14): removing the allowance file in favour of the two fixed exceptions, and the planted-read check.
- The modes' own reads (steps 9–12), including `panel_step`, `map_step`'s `menu.open_menu` and `pause_step`'s `pause.close_pause`. If step 12 left one unconverted, it is recorded, not converted here, unless H-12-5's rule says otherwise.
- `src/devtools/`, including `pad_author.c`'s own chord buttons and `pad_metrics`' D-pad mapping. The generator reads the default table only, so its output cannot change.
- `bindings.default.cfg` and its comments (step 20 rewrites the docs), and section 2's table.
- Renaming test functions or test comments that carry PSX words.
- `ui_set_pad_crosshair` and other ui names that contain `cross` without meaning the button.
- Narrowing Select to the current mode, or making the chord bindable: both are locked.

## Further Notes

- **Trap: every converted action is on 6, 9, 10, 11 or 12 in every mode by default.** A read tagged with the wrong mode passes on the default. Each test moves its family off those buttons and leaves the neighbour unbound.
- **Trap: decide the shoulders' mode before `unwind_sub_modes`.** It clears a planet menu and cancels a slipway. A mode read after it is the map mode, and the menu's or slipway's shoulder buttons stop working. Test 2's menu check pins this.
- **Trap: row 33 removes the ○ action's buttons, not 6.** Clearing a fixed button would let `placement.cancel_placement` on another button cancel the structure on the frame that closes the year box.
- **Trap: the chord mask lives in `cheat.c`, not `cheat.h`.** The guard counts headers. A `#define` in `cheat.h` would be an unlisted read that step 14 cannot allow.
- **Trap: the Select union leaks across jump and aim.** Pick test buttons so that no reach-only action in jump or aim sits on a Select button. Otherwise a reach press is also a Select tap.
- **Trap: pause and the title fall into the map mode for ○.** That is deliberate. The block acts on nothing there, and a `TITLE` or `PAUSE` branch would need a ○ action those modes do not have.
- **For step 14.** After this step, `git grep -E 'cross|circle|square|triangle|×|○|△|□'` over `src/game/pad_*` and `src/game/cheat.*` still matches `ui_set_pad_crosshair` (`crosshair`) in `pad_nav.c`. That is a ui name, not a PSX word: step 14's grep needs word boundaries or an exclusion. The rest of the grep must find nothing of this step's.
- The step note (`### Step 2.13`) records:
  - which H-12-k held, and how the code adapted where one did not;
  - the converted reads by action, with the mode rules (`nav_mode`, `back_mode`, the Select union, row 28's rule);
  - the differences from section 2's table listed under Implementation Decisions;
  - the mutation proof's results;
  - the guard output before and after each chunk;
  - the guardrail runs;
  - the timings.
