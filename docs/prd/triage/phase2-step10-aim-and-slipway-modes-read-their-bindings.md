---
title: "Aim and slipway modes read their bindings"
description: "Aim mode, its pan (shared with placement) and slipway mode's build-on-target still test hardcoded buttons, so a bindings table moves nothing in these modes."
status: needs-triage
---

## Problem Statement

This is plan 17, step 10 (Phase 2, bindings in the game). The step's text, from `docs/plan-17.md` section 3:

> 10. **Aim and slipway modes read their bindings**, verified the same way.

"The same way" means step 9's verification and rule:

> Verified: e2e unchanged, plus a unit test with a table that swaps actions in the converted mode and leaves every other mode unbound: the converted mode follows the table, and the same buttons do nothing in a neighbouring mode served by the same shared code. Without it a read tagged with the wrong mode passes, because the default puts most actions on the same button in every mode. In steps 9–13, the step that converts a read also renames the local or comment that carries a PSX name on that line (`cross_engaged`, `circle_returns`, a `×` in a comment, …) to the action's name.

Since step 7, `pad_nav` holds a copy of the compiled bindings table. Once step 9 is in, jump mode reads it. Aim and slipway modes still play plan 12's buttons, whatever `bindings.cfg` says:

- `aim_mode_step` tests button 7 to leave aim mode. It tests button 5 held (to keep or end a probe or fold engagement) and button 5 pressed (to engage it).
- `aim_pan` turns held buttons 1–4 into camera pans through `pad_jump_direction_button`. It serves aim mode and also placement mode, which pans the same way while carrying a structure.
- `slipway_step` tests button 5 pressed to release the slipway on the aimed target, and tests "button 5 not pressed" to gate the re-aim on a direction.

A player who moves `aim.probe_or_fold`, `aim.leave_aim`, `aim.pan_*`, `placement.pan_*` or `slipway.build_on_target` in a personal file sees nothing change.

## Solution

Aim mode, placement's pan and slipway mode ask the table:

- Each of the five guarded reads becomes "is a button bound to this action of this mode pressed (or held)".
- `aim_pan` takes the mode it serves. Aim mode pans on `aim.pan_*` and placement on `placement.pan_*`.

With the default table every frame is identical by construction, so every e2e `.sha256` and every guardrail output stays byte-identical. Unit tests prove three things:

- With a rotated table, aim mode follows it, and the same direction buttons pan nothing in placement when placement is unbound.
- With the reverse setup, placement follows its own table and aim mode pans nothing.
- Slipway mode builds on the button bound to `slipway.build_on_target`, and the jump-mode button that started the slipway does not build.

The guard allowance loses its line `src/game/pad_nav.c step-10 5`. The struct field `cross_engaged` takes the action's name.

## Facts checked at HEAD deb5c3b (step 8 complete, committed; step 9 not committed)

- `src/game/` names 40 guarded buttons in 5 files: `pad_nav.c` 30, `pad_jump.c` 4, `bindings.c` 3, `cheat.c` 2, `pad_panel.c` 1. The one in `bindings.h` is in a comment. Step 8 changed no file under `src/game/`, no allowance line, and not `tests/test_pad_nav.c`.
- `tools/button_guard_allowance.txt` has `src/game/pad_nav.c step-10 5 | aim_mode_step: aim.leave_aim, aim.probe_or_fold (held and pressed); slipway_step: slipway.build_on_target (twice)`. The guard requires strict per-file equality.
- Step 2.5's rows owned by step 10, at HEAD's line numbers in `pad_nav.c` (the inventory's numbers have moved by +6 since step 7). Step 9 will move them again.
  - Row 7: `aim_mode_step` line 362, `(pressed & P8_PAD_BUTTON_7) && !nav->cross_engaged` → `aim.leave_aim`.
  - Row 8: line 366, `!(pad_buttons & P8_PAD_BUTTON_5)` ends the engagement → `aim.probe_or_fold` held.
  - Row 9: line 368, `(pressed & P8_PAD_BUTTON_5) && !release` engages it when nothing with a `left_press` (other than the background) lies under the centre → `aim.probe_or_fold` pressed.
  - Row 10: `aim_pan` (line 151), called from `aim_mode_step` (357). The read is indirect: held 1–4 through `pad_jump_direction_button(pad_jump_direction(i))` (154) → `aim.pan_*`.
  - Row 11: `aim_pan` called from `placement_step` (579), the same indirect read → `placement.pan_*`.
  - Row 12: `slipway_step` line 771, `(pressed & P8_PAD_BUTTON_5) && !release && target != NULL && pressable(target)` → `slipway.build_on_target`.
  - Row 13: line 776, `direction != PAD_DIRECTION_NONE && !(pressed & P8_PAD_BUTTON_5)` → `slipway.build_on_target` (the gate on the re-aim).
- `aim_pan` keeps `held_frames[PAD_JUMP_DIRECTIONS]` indexed by `pad_jump_direction(i)` (Up, Down, Left, Right, asserted), with the speed curve `pan_speed` (1 px per frame for frames 1–10, 2 px for 11–40, 4 px after that). The cap is `HELD_FRAMES_CAP` 41.
- `cross_engaged` is a field of `pad_nav_t` (`pad_nav.h`). It is named in `pad_nav.c` only:
  - `reset_aim_mode` (138);
  - `aim_mode_step` (359, 362, 367, 369, 371);
  - `placement_step` (572);
  - `pause_button_free` (840);
  - `select_allowed` (926).

  No test and no devtool names it. The comment above `aim_mode_step` reads "On a click's release frame × starts nothing: finish_step forces its press up."
- `placement_step`'s own reads (row 16 button 6, row 17 button 5) and its comment "× drops by letting the held left button go…" belong to step 11. `slipway_step`'s `circle_returns` belongs to step 13. `slipway_step`'s hold-repeat call belongs to step 9.
- `bindings.default.cfg` binds:
  - `aim.pan_up..pan_right` 1–4, `aim.probe_or_fold` 5, `aim.leave_aim` 7, `aim.open_build_bar` 9;
  - `slipway.jump_*` 1–4, `slipway.build_on_target` 5;
  - `placement.pan_*` 1–4;
  - `build_bar.take_structure` 5;
  - `jump.jump_*` 1–4, `jump.act_on_hovered` 5.
- `bindings_mode_has_action(mode, action)` exists. Only aim and placement have `PAN_*`, and only slipway has `BUILD_ON_TARGET`.
- `tests/test_pad_nav.c` reaches aim mode with no button pressed: `start_game` asserts aim mode with nothing hovered. Its helpers and their buttons:
  - `start_title` calls `input_step_init(step, &bindings_compiled)`;
  - `open_build_bar` taps 9 and `take_structure` presses 5;
  - `hover_object` and `enter_jump_mode` tap 7 from aim mode;
  - `start_slipway` presses 5 in jump mode, and `cross_on_target` presses 5 in slipway mode;
  - `settled_row(step, &a, &b, &c)` sets three colonies 10 px apart with 999 credits;
  - `tap_path`, `slipway_pick`, `expected_jump`, `assert_slipway_mode`, `world_count`, `built_slipways`, `camera_at`, `assert_moved`, `hold`, `make_known`, `known_planet_closest_to_centre`, `in_forked_copy`/`end_forked_copy` and `in_fresh_child` exist.

  `tests/test_input_step.c` builds an inline swap table (`swap_table()`), prior art for a test table.

## Hypotheses on step 9's outcome

Step 9 ("Jump mode reads its bindings", PRD `docs/prd/triage/phase2-step9-jump-mode-reads-its-bindings.md`) is being implemented in parallel and is not committed. This PRD was written from HEAD deb5c3b and from step 9's PRD. Each assumption about step 9 is a hypothesis:

- **H-9-1. The hold repeat takes the table and the mode.** `pad_jump_repeat_step(jump, bindings, mode, pad_buttons, pressed)` exists. `jump_step` calls it with `BINDINGS_MODE_JUMP` and `slipway_step` with `BINDINGS_MODE_SLIPWAY`. Step 10 does not touch these calls.
- **H-9-2. One read helper exists in `pad_nav.c`.** It is `static int action_pressed(const pad_nav_t *nav, int pressed, bindings_mode_t mode, bindings_action_t action)`, returning whether `pressed` meets `bindings_mask(&nav->bindings, mode, action)`.
- **H-9-3. Step 9 converts only jump mode.** The four `jump_step` reads go through `action_pressed` with `BINDINGS_MODE_JUMP`. `cross_starts_slipway` is renamed `act_on_hovered_starts_slipway`. Step 9 leaves alone `aim_mode_step`, `aim_pan`, `placement_step`, `reset_aim_mode`, the field `cross_engaged`, and `slipway_step`'s two button-5 reads.
- **H-9-4. The guard moved by 4.** After step 9, `pad_nav.c` names 26 guarded buttons and the allowance no longer has a step-9 line. The step-10 line is unchanged (`src/game/pad_nav.c step-10 5 | …`). `make button-guard` prints `files 5 reads 36`.
- **H-9-5. The direction conversions are untouched.** `pad_jump_first_direction` and `pad_jump_direction_button` still exist (step 12 deletes them) and `aim_pan` still calls `pad_jump_direction_button`. `pad_jump.c`'s allowance line stays `step-12 4`.
- **H-9-6. `test_pad_nav.c` can start a game with any table.** Step 9 turned `start_title` into a call of a helper that takes a `const bindings_t *` (the PRD does not fix its name; call it the "table-taking title helper" here). It can parse an inline table with `bindings_parse`. `in_forked_copy`, `start_game`, `begin_game`, `open_build_bar`, `take_structure`, `hover_object`, `settled_row`, `tap_path` and the other helpers listed above keep their behaviour.
- **H-9-7. Step 9's new `test_pad_nav.c` test presses nothing that step 10 converts outside jump mode.** Its table binds only jump lines. It reaches jump mode through `begin_game` and `settle_planet`, with no aim-mode button. In slipway mode it presses only buttons 1–4, never 5 or 6. Its button-8 check enters aim mode, then the fork ends. So step 10's unbound aim and slipway reads cannot break it.
- **H-9-8. Existing pad tests are unchanged.** They still use `&bindings_compiled`. Step 9's mechanical changes are limited to `pad_jump_repeat_step` call sites.
- **H-9-9. The guardrail outputs are byte-identical.** After step 9, `make pad-author` regenerates `worst-case/` with no diff, and `make pad-author-proof`, `make pad-metrics-check` and `make pad-metrics-worst-case` give the same outputs as at step 8's end.

**Rule if a hypothesis is wrong.** Follow the committed step 9 code at this step's own `HEAD`, use its names, and record the difference in the step note (`### Step 2.10`). In particular:

- **The helper (H-9-2).** If it has another name or shape, reuse it as it is. If it does not exist, add it as described under Implementation Decisions.
- **The hold-repeat calls (H-9-1, H-9-3).** If step 9 left `slipway_step`'s hold-repeat call with the jump mode, or without a mode, that is step 9's row 6 and not this step's. Record it, and do not fix it here, unless it would break this step's slipway test. In that case, pass `BINDINGS_MODE_SLIPWAY` and record it.
- **The guard counts (H-9-4).** If they differ, what matters is that the step-10 line goes and `pad_nav.c` names exactly 5 fewer buttons than at the step's start.
- **The title helper (H-9-6).** If no table-taking title helper exists, add one: `start_title` keeps calling it with `&bindings_compiled`, so no existing test changes.
- **Step 9's own test (H-9-7).** If it breaks because aim or slipway now read the table, add the missing action to its table on its default button, and change nothing else in it. Then record it.
- **Guardrail outputs (H-9-9).** If one differs at step 10's start, use the tree's output as the baseline. Step 10 must not change it.

## User Stories

1. As a player, I want aim mode to pan on the buttons bound to `aim.pan_up`, `aim.pan_down`, `aim.pan_left` and `aim.pan_right`, so that I can remap the camera directions.
2. As a player, I want a remapped pan to keep its acceleration (slow, then faster, then fastest), so that only the button changes, not the feel.
3. As a player, I want two opposite pans held together to cancel each other as today, whatever buttons they are on, so that the behaviour stays predictable.
4. As a player, I want placement mode to pan on the buttons bound to `placement.pan_*`, so that carrying a structure follows my file for placement, not for aim.
5. As a player, I want the button bound to `aim.probe_or_fold` to place a probe on empty space, so that I can move the probe to the button I prefer.
6. As a player, I want holding the `aim.probe_or_fold` button to fold, and releasing it to end the fold, so that press versus hold keeps both meanings on the new button.
7. As a player, I want a button that is not bound to `aim.probe_or_fold` to neither start nor keep a fold, so that releasing my probe button always ends the fold.
8. As a player, I want the button bound to `aim.leave_aim` to return to jump mode on the nearest object, so that I can choose that button.
9. As a player, I want `aim.leave_aim` to still wait while a fold is engaged, so that remapping does not change when I can leave aim mode.
10. As a player, I want the button bound to `slipway.build_on_target` to release the slipway on the aimed target, so that I can build on my chosen button.
11. As a player, I want a direction pressed together with the `slipway.build_on_target` button not to re-aim the slipway, so that a build press is never mistaken for a re-aim.
12. As a player, I want the button that started a slipway in jump mode not to build in slipway mode once I have moved `slipway.build_on_target`, so that the two modes stay independent.
13. As a player, I want an aim, slipway or placement action left out of my file to do nothing, so that what the file says is what the game does.
14. As a player, I want the default bindings to play exactly like plan 12, so that nothing changes without a personal file.
15. As a maintainer, I want every e2e `.sha256` and end state unchanged, so that this step is a proven refactor.
16. As a maintainer, I want `make pad-author-proof`, `make pad-author` with no diff in `worst-case/`, `make pad-metrics-check` and `make pad-metrics-worst-case` unchanged, so that the devtools still agree with the game.
17. As a maintainer, I want `aim_pan` to take the mode it serves, so that aim and placement each read their own `pan_*` through one helper.
18. As a maintainer, I want `aim_pan` to assert, with a message, that its mode has `pan_*` actions, so that a wrong mode argument fails loudly in tests.
19. As a maintainer, I want the five reads converted with step 9's helper, so that every converted mode reads the table the same way.
20. As a maintainer, I want the step-10 line gone from the button guard allowance and `pad_nav.c` to name exactly five fewer buttons, so that the guard proves the conversion and blocks a regression.
21. As a maintainer, I want `cross_engaged` renamed after the action it tracks, and the `×` gone from `aim_mode_step`'s comment, so that no PSX word sits on a converted line.
22. As a maintainer, I want the existing pad unit tests unchanged, so that plan 12's behaviour stays pinned by the same expected values.
23. As a maintainer, I want a unit test where aim mode follows a rotated table and placement, served by the same pan, stays unbound, so that a pan read tagged with the aim mode inside placement fails.
24. As a maintainer, I want the mirror test where placement follows its table and aim stays unbound for pans, so that a pan read tagged with the placement mode inside aim fails, and placement's conversion is proven too.
25. As a maintainer, I want a unit test where slipway mode builds only on the button bound to `slipway.build_on_target`, so that a slipway read tagged with jump's `act_on_hovered`, or left hardcoded, fails.
26. As a maintainer, I want those tests to press buttons outside the converted modes only for actions bound on their default buttons, so that steps 11–13 converting those modes keep the tests valid.
27. As a maintainer, I want a one-off mutation proof recorded in the step note, so that I know the new tests catch a wrong mode on the shared pan.
28. As a maintainer, I want `make test` and `make e2e` no slower, so that the fast loop stays fast.
29. As a reviewer, I want the step note to list which H-9-k held, so that I can see how the parallel step 9 landed against this one.

## Implementation Decisions

### The pan takes the mode

- New static signature: `aim_pan(pad_nav_t *nav, bindings_mode_t mode, int pad_buttons)`. Step 2.5's rule applies: a helper shared by modes of different steps gets a mode argument in the earliest step, and here that is step 10 for both aim and placement.
- Inside, a `static const bindings_action_t` array maps the direction index to its action: Up → `BINDINGS_ACTION_PAN_UP`, Down → `PAN_DOWN`, Left → `PAN_LEFT`, Right → `PAN_RIGHT`. The existing assert on `pad_jump_direction(i)`'s order stays, because it guards that array as well as `speed[]`. Held direction `i` is `pad_buttons & bindings_mask(&nav->bindings, mode, pan_action[i])`, not 0. `aim_pan` no longer calls `pad_jump_direction_button`.
- An entry assert carries a message: `bindings_mode_has_action(mode, BINDINGS_ACTION_PAN_UP) && "aim_pan: a mode with pan_* actions (aim or placement)"`.
- Behaviour is unchanged apart from where the buttons come from:
  - `held_frames`, the cap and the speed curve are kept;
  - opposite pans held together still cancel (the difference of speeds);
  - an unbound pan direction counts as not held and resets its `held_frames`;
  - an action bound to several buttons is held when any of them is.
- `aim_mode_step` calls `aim_pan(nav, BINDINGS_MODE_AIM, pad_buttons)`. `placement_step` calls `aim_pan(nav, BINDINGS_MODE_PLACEMENT, pad_buttons)`.
- `held_frames` is still kept per direction, not per mode. Across an aim → placement change, a held button keeps counting only if it is bound to the same direction in both modes. With the default this is always the case, so frames are unchanged.

### `aim_mode_step` and `slipway_step` read the table

- Reuse step 9's `action_pressed` (H-9-2) for the four press reads. For the held read (row 8), add a sibling `static int action_held(const pad_nav_t *nav, int pad_buttons, bindings_mode_t mode, bindings_action_t action)` with the same mask test, so the call site says what it reads. Steps 11–13 may reuse it. If step 9's helper already takes any mask under a neutral name, reuse it for the held read too, and add nothing.
- The five reads keep the same conditions and order as today:
  - row 7: `(pressed & P8_PAD_BUTTON_7) && !engaged` becomes `action_pressed(nav, pressed, BINDINGS_MODE_AIM, BINDINGS_ACTION_LEAVE_AIM) && !engaged`;
  - row 8: `!(pad_buttons & P8_PAD_BUTTON_5)` becomes `!action_held(nav, pad_buttons, BINDINGS_MODE_AIM, BINDINGS_ACTION_PROBE_OR_FOLD)`;
  - row 9: `(pressed & P8_PAD_BUTTON_5) && !release` becomes `action_pressed(…, BINDINGS_MODE_AIM, BINDINGS_ACTION_PROBE_OR_FOLD) && !release`;
  - row 12: `(pressed & P8_PAD_BUTTON_5) && !release && …` in `slipway_step` becomes `action_pressed(…, BINDINGS_MODE_SLIPWAY, BINDINGS_ACTION_BUILD_ON_TARGET) && !release && …`;
  - row 13: `!(pressed & P8_PAD_BUTTON_5)` in the re-aim gate becomes `!action_pressed(…, BINDINGS_MODE_SLIPWAY, BINDINGS_ACTION_BUILD_ON_TARGET)`.
- No other line of `slipway_step` changes. Its hold-repeat call is step 9's and its `circle_returns` is step 13's.

### Renames on converted lines

- The field `cross_engaged` becomes `probe_or_fold_engaged`, named after the action whose press sets it and whose release clears it. This is a field rename, so every use changes with it, including those outside the converted lines:
  - `reset_aim_mode`;
  - `placement_step`'s reset;
  - `pause_button_free`;
  - `select_allowed`;
  - the struct in `pad_nav.h`.

  Behaviour is unchanged, and no test names the field.
- The comment above `aim_mode_step` becomes "On a click's release frame probe_or_fold starts nothing: finish_step forces its press up." It keeps its trap and loses only the `×`.
- No other comment changes. `placement_step`'s `×` comment is on step 11's line, `pause_cross_armed` and `menu_cross_armed` belong to step 12, and `circle_returns` to step 13. No new comment unless it carries a trap (`CLAUDE.md`).

### Guard allowance

- When the step ends, `tools/button_guard_allowance.txt` has lost exactly the line `src/game/pad_nav.c step-10 5 | …`, and nothing else in it has changed. `make button-guard` goes from `files 5 reads 36` to `files 5 reads 31` (H-9-4).
- Between commits the allowance must stay valid, because the guard requires exact counts. The commit that converts the aim reads lowers that line to `src/game/pad_nav.c step-10 2 | slipway_step: slipway.build_on_target (twice)`. The commit that converts the slipway reads deletes it. The commit that changes only `aim_pan` touches no guarded name.

### No public interface changes

- `pad_nav.h` changes only by the field rename. No header outside `src/game/pad_nav.*` changes, and no Makefile link line changes.

## Implementation Chunks

1. **The pan takes the mode.** New `aim_pan` signature and its direction-to-action array. `aim_mode_step` calls it with `BINDINGS_MODE_AIM` and `placement_step` with `BINDINGS_MODE_PLACEMENT`. Add both pan tests (Testing Decisions, tests 1 and 2). Verified: `make test-all`, every `.sha256` unchanged, the four guardrail commands unchanged.
2. **Aim mode reads the table.** Rows 7–9 through the helpers, the `cross_engaged` → `probe_or_fold_engaged` rename, the comment, and the allowance line lowered to `step-10 2`. Extend test 1 with its probe, fold and leave-aim checks. Verified the same way.
3. **Slipway mode reads the table.** Rows 12–13 and the allowance line removed. Add test 3. Verified: everything in Testing Decisions.

## Testing Decisions

- **What a good test is.** It drives frames through `input_step_run` and asserts what the player sees: the mode, the camera position, the played mouse button, probes and wormholes, the slipway start and target, the slipway count, the carried structure. It never reads static state or the renamed field.
- **Existing tests do not change** (section 2 guardrail). Step 10 changes no public signature, so `test_pad_nav.c`, `test_pad_jump.c`, `test_pad_metrics.c` and `test_input_step.c` keep their code. An assertion that must change is a divergence: follow the code and record it in the step note.
- **Tables.** Each test parses its own table from inline text with `bindings_parse`, asserts that the parse succeeds, and starts through the table-taking title helper (H-9-6), then `cart_start_game` and the settle frames as `start_game` does.
  - The converted actions sit off their default buttons, so a read left hardcoded fails.
  - The modes under test are unbound wherever the plan's check needs a neighbour.
  - Some actions of modes that later steps convert are needed to reach a mode. These are bound on their default buttons and named in a comment as reach-only lines. They are what keep the test valid when steps 11–13 make those modes read the table. Every other mode stays unbound.
- **Checks in forks.** Each check runs in its own `in_forked_copy`, so no check has to undo a mode through a button that a later step converts. Prior art: `test_aim_pans_with_the_speed_curve`, `test_aim_mode_first_cross_places_the_probe`, `test_aim_mode_short_cross_hold_abandons_the_fold`, `assert_leaving_aim_mode_hovers_the_nearest`, `test_cross_takes_the_structure_and_a_second_cross_places_it`, `test_cross_dpad_cross_dpad_cross_builds_a_chain_of_two_slipways`, and step 9's rotated-table test.

### Test 1: aim mode follows a rotated table, and placement, served by the same pan, stays unbound

The table:

```
aim.pan_up = 2
aim.pan_down = 1
aim.pan_left = 4
aim.pan_right = 3
aim.probe_or_fold = 8
aim.leave_aim = 5
aim.open_build_bar = 9      # reach-only (step 13)
build_bar.take_structure = 5   # reach-only (step 11)
```

The rotation is needed because three aim actions move: a pairwise swap would leave one read on its default button. Every placement action is unbound. The game starts in aim mode, as `start_game` asserts, with nothing pressed.

- **Pans.** Holding button 2 for 11 frames, then releasing, moves the camera 12 px up (x unchanged). Button 1 moves it 12 px down, button 4 12 px left, and button 3 12 px right. These are the 11-frame, 12-pixel row of `test_aim_pans_with_the_speed_curve`. Holding 2 + 1 together moves nothing.
- **Probe.** Move the camera to empty space without pad buttons, for example with `camera_pan` the way `test_aim_mode_cross_on_a_planet_presses_nothing` does. Then:
  - Tapping 8 places one probe and no wormhole, and the game stays in aim mode.
  - In another fork, tapping 5 (the default probe button, now `leave_aim`) places nothing, and the game stays in aim mode, because nothing is known.
- **Fold (the held read).** After the probe tap, settle, then hold 8 for 10 frames. Every frame plays the left mouse button, and the ui holds a wormhole. Then run one frame with button 7 alone (unbound): it plays no mouse button. With a held read left on button 5, the first held frame would already play nothing.
- **Leave aim.** `make_known` a planet as `assert_leaving_aim_mode_hovers_the_nearest` does. Tapping 5 lands in jump mode on the nearest object. In another fork, tapping 7 (the old default) leaves the game in aim mode. In a third fork, a fold is engaged with 8, then 5 is pressed while 8 stays held: the game stays in aim mode, because `leave_aim` waits for the engagement as today.
- **Neighbour: placement served by the same pan.** Tap 9 (bar open, `open_build_bar` passes), then press and release 5 as `take_structure` does: the game is now carrying. Hold each of buttons 1, 2, 3 and 4 in turn for 11 frames. The camera does not move, and the structure is still carried on every frame (`assert_carrying`-style checks). With the default table, or with `aim_pan` tagged `BINDINGS_MODE_AIM` in `placement_step`, the camera would move.

### Test 2: placement follows its table, and aim mode, served by the same pan, stays unbound

The table:

```
placement.pan_up = 2
placement.pan_down = 1
placement.pan_left = 4
placement.pan_right = 3
aim.open_build_bar = 9      # reach-only (step 13)
build_bar.take_structure = 5   # reach-only (step 11)
```

Aim's pans are unbound.

- **Aim pans nothing.** In aim mode, holding each of buttons 1–4 for 11 frames does not move the camera.
- **Placement follows its table.** Tap 9, then take the structure with 5. Holding 2 for 11 frames moves the camera 12 px up while still carrying, and 1 moves it down, 4 left and 3 right. Use one fork per direction. The structure is still carried after each one.
- This test also proves the placement half of the conversion, which test 1 cannot: with aim unbound, a pan tagged `BINDINGS_MODE_AIM` in `placement_step` would not move.

### Test 3: slipway mode builds on its own button, and jump mode's button does not build there

The table:

```
jump.jump_up = 1            # reach-only (step 9, default)
jump.jump_down = 2
jump.jump_left = 3
jump.jump_right = 4
jump.act_on_hovered = 5     # reach-only: starts the slipway
aim.leave_aim = 7           # reach-only: hover_object enters jump mode on 7
slipway.jump_up = 1         # reach-only (step 9, default): aims the target
slipway.jump_down = 2
slipway.jump_left = 3
slipway.jump_right = 4
slipway.build_on_target = 8
```

- **Setup.** `start_game`-style start with the table, then `settled_row` (a, b, c settled on a row, 999 credits), then `hover_object(a)`, then `start_slipway(a)` (button 5 in jump mode), then tap 4 to aim at b. `assert_slipway_mode(a, b)`.
- **Builds on 8.** Pressing 8 releases on b. The next frames show `assert_slipway_mode(b, NULL)`, and `world_count().slipways` has grown by one, as in `cross_on_target` with 8.
- **5 does not build.** In another fork, pressing 5 for one frame, then releasing, builds nothing: the slipway count is unchanged and the game is still `assert_slipway_mode(a, b)`. With the default table, or with a read tagged `BINDINGS_MODE_JUMP, BINDINGS_ACTION_ACT_ON_HOVERED`, it would build.
- **The gate follows the table.** In one fork, pressing 8 + 3 together releases on b and does not re-aim. In another fork, pressing 5 + 3 together re-aims as a plain 3 would (`slipway_pick(a, b, P8_PAD_BUTTON_3)`) and builds nothing. If `slipway_pick` returns NULL for 3 from b, use the direction that has a target and assert that one exists.
- **Neighbour: jump mode.** Before the slipway starts, while hovering a in jump mode, tapping 8 opens no box, starts no slipway, and leaves the game in jump mode hovering a. `jump.open_menu` is unbound in this table, so this holds once step 9 is in, and it shows that 8 acts only in the mode the table gives it.
- No button 6 is pressed in slipway mode (`cancel_slipway` is step 13's).

### One-off mutation proof

Record it in the step note and do not commit it. Restore each mutation after it is checked.

- In `placement_step`, pass `BINDINGS_MODE_AIM` to `aim_pan`. Test 2's placement check must fail. Test 1's neighbour check may pass, because aim's pans are bound there, and the step note says which checks failed.
- Leave row 8 hardcoded on `P8_PAD_BUTTON_5`. Test 1's fold check must fail, and `make button-guard` must report `src/game/pad_nav.c names 22 button(s), listed 21`.
- Leave row 12 hardcoded. Test 3 must fail.

### Step verification, in this order

1. `make test-all`: green, every `tests/e2e/*.json.sha256` unchanged, and `make end-state` unchanged on every replay.
2. Guardrails (section 2; this step touches `src/game/pad_*`): `make pad-author-proof`, then `make pad-author` followed by `git diff --exit-code worst-case/`, then `make pad-metrics-check` and `make pad-metrics-worst-case`. All are unchanged from the tree at step 10's start (H-9-9).
3. `make button-guard` passes with the step-10 line gone. `git grep -o P8_PAD_BUTTON_ -- src/game/pad_nav.c | wc -l` counts exactly 5 fewer than at the step's start. `git diff <start> -- tools/button_guard_allowance.txt` shows only that line deleted.
4. `git grep -n 'cross_engaged' -- src tests` finds nothing. `aim_mode_step`'s comment has no `×`. `git grep -n pad_jump_direction_button -- src/game/pad_nav.c` finds nothing: the pan was its last caller in that file, and `pad_nav_step`'s shared direction uses `pad_jump_first_direction`, which stays.
5. `make raylib` compiles and links. It is never run.
6. Timings: `make test` and `make e2e`, three warm runs each, against step 1's baseline (4.27–4.58 s and 0.24–0.25 s). The three new tests use forks and short holds. If `test_pad_nav` gets measurably slower, reduce frames, not checks.

## Out of Scope

- Jump mode's reads and the hold repeat (step 9), including `slipway_step`'s `pad_jump_repeat_step` call.
- Placement's own reads (`placement.place_structure`, `placement.cancel_placement`), its `×` comment, and the build bar (step 11). Only placement's pan moves here, through the shared `aim_pan`.
- Menu, panels, pause and title, `choose_row` arming, and the deletion of `pad_jump_first_direction`, `pad_jump_direction_button` and the shared pressed direction (step 12).
- The cross-mode reads that also serve aim, slipway and placement (step 13):
  - `aim.back`, `slipway.cancel_slipway` (`circle_returns`), the message-box close;
  - `aim.pause`, `slipway.pause`, `placement.pause`;
  - `*.open_build_bar`, `*.open_tech_panel`;
  - `aim.select_tap_or_year`;
  - the chord mask.

  With a table that moves these, aim, slipway and placement still answer plan 12's buttons for them until step 13.
- Renaming test functions or comments in `tests/` that carry PSX words (`test_aim_mode_first_cross_places_the_probe`, …), because tests change only mechanically.
- Any change under `src/devtools/`. `pad_metrics.c` keeps its own fixed D-pad mapping (step 8).
- Fixtures in `replay`, and the swap and unbound replays (steps 16–19).

## Further Notes

- **Trap: the wrong mode passes with the default.** Aim and placement both put `pan_*` on 1–4, so only tests 1 and 2 catch a pan tagged with the wrong mode. They are a pair: each one catches one direction of the mistake.
- **Trap: a direction is not a button.** The pan compares `pad_buttons` with masks from the table, never with a `pad_direction_t`. UP and DOWN equal the bits of buttons 1 and 2, so the mistake would show only on LEFT and RIGHT.
- **Trap: the engagement flag gates other modes' reads.** `probe_or_fold_engaged` stops `aim.leave_aim`, the Select tap, the pause opening (`pause_button_free`) and `select_allowed`. The rename must keep every one of these uses. Its value still comes only from `aim.probe_or_fold`.
- **Trap: build-on-target is two reads.** One releases on the target. The other keeps a direction pressed in the same frame from re-aiming. Both must use the same action, or a remapped build press would also re-aim.
- **Trap: reach-only lines.** A test that reaches a mode through a button of a mode that a later step converts must bind that action in its table, on its default button. Otherwise steps 11–13 turn the test red for a reason unrelated to what it checks.
- The step note (`### Step 2.10`) records:
  - which H-9-k held, and how the code adapted where one did not;
  - the converted reads by action;
  - the mutation proof's results;
  - the guard output before and after;
  - the guardrail runs;
  - the timings.
