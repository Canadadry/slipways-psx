---
title: "Inventory of button reads and the button guard"
description: "Steps 9–13 must replace every hardcoded button read in src/game/ with a binding, but nobody has listed those reads, a few of them do not match plan 17's table, and nothing stops a new one from appearing."
status: needs-triage
---

## Problem Statement

Plan 17 step 5 (Phase 2, bindings in the game). The step's text, from `docs/plan-17.md` section 3:

> 5. **Inventory of button reads.** After step 4's rename, list in this step's note every read of a `P8_PAD_BUTTON_*` in `src/game/`: presses and held buttons, including the values computed once before the mode dispatch and shared by several modes (the pressed direction, the ○ that closes a box, the menu and pause arming, `pad_panel_step`'s held button). Write next to each read the `mode.action` of section 2 it will become, and assign it to exactly one of steps 9–13. A read with no matching entry, or matching a different button, is settled here before step 6: section 2's table is amended with plan 12's behaviour (follow the code), so steps 6 and 7 build the enums and `bindings.default.cfg` from the reconciled table. Add the guard of section 2 now, with an allowance file seeded from this list (the remaining reads per file). Each of steps 9–13 removes exactly its entries from the allowance, and `make test` fails when the tree has a read the allowance does not list. Verified by the list matching a grep of `P8_PAD_BUTTON_` in `src/game/`, and by `make test` passing and not slower.

After step 4, the game code names buttons by number (`P8_PAD_BUTTON_5`), but the control scheme is still spread over about thirty reads in `pad_nav.c` and a few helpers. Some reads run inside one mode, some run before the mode dispatch and serve several modes at once, and some reach the buttons indirectly through the direction helpers of `pad_jump`. Steps 9–13 each convert "one group of modes". Without a list, a read can fall between two steps or be converted twice. Without a guard, a new hardcoded button can come back while the conversion is under way.

Plan 17's table of modes and actions (section 2) was written from the doc `docs/pad-modes-and-actions.md`, not from the code. Reading the code shows reads that do not fit it: an action bound to two buttons read on two different code paths with different conditions, a message-box rule narrower than the code, and the cheat chord's mask living in `pad_nav.c` rather than in `cheat.c`. If step 6 builds the enums from the unreconciled table, steps 9–13 will either change frames or have to reopen the enums.

## Solution

The step produces three things, none of which changes a frame:

1. **The inventory**, written in the plan file's step note (`### Step 2.5`): every read of a pad button in `src/game/`, its function, its button, its current mode or modes, the `mode.action` it will become, and the one step of 9–13 that converts it. Reads made through the direction helpers (`pad_jump_first_direction`, `pad_jump_direction_button`, `pad_jump_repeat_step`) are listed too, flagged as indirect.
2. **The reconciled table.** Section 2's "Modes and actions" list is amended where the code differs, following the code. Steps 6 and 7 then build the enums and `bindings.default.cfg` from it.
3. **The button guard.** A `make test` check counts, per file of `src/game/`, the `P8_PAD_BUTTON_` names outside comments and string literals, and requires strict equality with an allowance file. The allowance has one line per file and owning step (or `fixed` for the cheat chord in `cheat.c`), so each of steps 9–13 deletes its own lines and the guard checks that the reads really went away.

## User Stories

1. As the implementer of steps 9–13, I want a list of every button read in `src/game/` with the step that owns it, so that I convert exactly my reads and no other step's.
2. As the implementer of steps 9–13, I want each read tagged with the `mode.action` it becomes, so that I know which `bindings_mask` call replaces it.
3. As the implementer of steps 9–13, I want reads made before the mode dispatch listed with the modes they serve, so that I know where the mode must be computed from the nav state.
4. As the implementer of step 9, I want the hold-repeat's held and pressed checks listed as jump reads, with slipway as their second caller, so that I give `pad_jump_repeat_step` a mode and know that slipway's call site moves with it.
5. As the implementer of step 10, I want `aim_pan`'s held directions listed with both callers (aim and placement), so that I convert the shared helper once and pass each caller's mode.
6. As the implementer of step 11, I want the build bar's left and right picks listed as `build_bar.pick_left` and `build_bar.pick_right`, even though they read the shared pressed direction and not a button name, so that I do not miss them.
7. As the implementer of step 12, I want `pad_panel_step`'s held button and the pause and menu arming listed as `choose_row` reads of title, menu, pause and panels, so that I convert all four modes served by `panel_step` together.
8. As the implementer of step 12, I want the rule that maps every state reaching `panel_step` to one of title, menu, pause or panels, so that a box opened over the map by something other than a hovered planet still has a mode.
9. As the implementer of step 12, I want the shared pressed direction and the button-to-direction conversions of `pad_jump` assigned to me as the last game-side consumer, so that they disappear before the guard closes in step 14.
10. As the implementer of step 13, I want the pre-dispatch reads (pause opening, the shoulders, Select, the ○ that closes a box or a message box, ○ closing the bar or a panel) listed with the mode each one is computed in, so that I can write the mode function once.
11. As the implementer of step 13, I want the cheat chord's mask in `without_chord` assigned to me, so that it moves behind the cheat module and `pad_nav.c` ends with no button name.
12. As the implementer of step 6, I want the mode and action lists of section 2 reconciled with the code before I start, so that my enums do not have to change in steps 9–13.
13. As the implementer of step 7, I want `bindings.default.cfg`'s content fixed by the reconciled table, so that the default reproduces plan 12's frames by construction.
14. As a maintainer, I want menu's ○ and △ split into two actions, because the code closes the box on two paths with different conditions, so that one `bindings_mask` per path keeps today's frames.
15. As a maintainer, I want the build bar's ○ and L1 split into two actions for the same reason, so that the shoulder code reads `open_build_bar` in every mode that has it.
16. As a maintainer, I want section 2 to say that ○ closes a shown message box in slipway and placement as well as in jump and aim, because the code does, so that step 13 does not narrow it.
17. As a maintainer, I want Select's held state read from the buttons of `select_tap_or_year` whatever the current mode, so that a Select held from a menu still does not fire on return, as today.
18. As a maintainer, I want `make test` to fail when a file of `src/game/` names a `P8_PAD_BUTTON_` the allowance does not list, so that no hardcoded button comes back during the conversion.
19. As a maintainer, I want `make test` to fail when the allowance lists more reads than the tree has, so that each of steps 9–13 must delete its lines and the allowance always says what is left.
20. As a maintainer, I want the guard to ignore names inside comments and string literals, so that an assert message naming a button does not count as a read.
21. As a maintainer, I want each allowance line to give its file, its owning step, its count and a reason, so that a reader knows why each read is still there.
22. As a maintainer, I want the cheat chord in `cheat.c` listed as `fixed`, so that it is visibly a permanent exception and not a read waiting for a step.
23. As a maintainer, I want an invalid allowance (unknown owner, missing file, empty list, duplicate line, line without a reason) to fail with exit 2, so that a guard over nothing never passes for green.
24. As a maintainer, I want the guard's failure message to name the file and both counts, so that I see at once which read is extra or missing.
25. As a maintainer, I want the guard to print a one-line summary on success, like the other guards, so that `make test` output stays uniform.
26. As a maintainer, I want the guard built like `random-guard` (shell plus awk, strict counts, a unit test driving it on a scratch tree), so that it needs no new tool and reads like its neighbours.
27. As a maintainer, I want `make test` no slower than step 1's baseline, so that the fast loop stays fast.
28. As a maintainer, I want every `tests/e2e/*.json.sha256` unchanged and no game source touched, so that this step is provably free of behaviour changes.
29. As a reviewer, I want the inventory's per-file totals to equal the guard's counts, so that the list and the grep are checked against each other mechanically.
30. As a reviewer, I want each amendment of section 2 justified by the function that does it, so that I can check it against the code.
31. As the implementer of step 14, I want the allowance format to already accept `fixed` lines, so that closing the guard is deleting the step lines and keeping the fixed ones.
32. As the implementer of step 6, I want to know that a `bindings.c` naming `P8_PAD_BUTTON_` needs a `fixed` line, so that my step keeps `make test` green.

## Starting point (checked at 6b5b753)

Step 4 ("Rename the constants") is fully committed: chunk 1 (the rename) at fd42f5a, chunk 2 (directions as their own enum) at 6b5b753, and its PRD was deleted as done. Every count, line number and function below was read at 6b5b753. If `HEAD` has moved when this step starts, re-run the grep and re-apply the assignment rules under Implementation Decisions: the rules are the decision, the counts follow from them.

- The twelve constants in `src/p8/input.h` are `P8_PAD_BUTTON_1 … P8_PAD_BUTTON_12` with the old values (1, 2, 4 … 2048), and no old name or alias is left in `src/` or `tests/`. Button numbers in this PRD use these names: 1–4 the D-pad, 5 ×, 6 ○, 7 □, 8 △, 9 L1, 10 R1, 11 Start, 12 Select.
- `pad_direction_t` (`PAD_DIRECTION_NONE, UP, DOWN, LEFT, RIGHT`) lives in `src/game/pad_direction.h`, which names no `P8_PAD_BUTTON_`. `pad_target.c` names none (8 at fd42f5a). `pad_panel.c` names exactly one, `pad_panel_step`'s held button 5 (11 at fd42f5a).
- `pad_nav.c` names 30 (37 at fd42f5a): `aim_pan`'s assert is stated with `PAD_DIRECTION_*`, and `build_step` compares the direction with `PAD_DIRECTION_LEFT` and `PAD_DIRECTION_RIGHT`.
- `aim_pan` and `pad_jump_repeat_step` test the held direction buttons through `pad_jump_direction_button`. `pad_nav_step` computes one `pad_direction_t direction = pad_jump_first_direction(pressed)` before the dispatch and hands it to `map_step` (then `panel_step`) and `bar_or_cycle_step` (then `panel_step` or `build_step`).
- `pad_jump.c` holds the two button-to-direction conversions. Only `pad_jump_direction_button` names buttons: 1–4 once each, in one `switch` (4 names). `pad_jump_first_direction` goes through it.
- No other file of `src/game/` names a `P8_PAD_BUTTON_`. `cheat.c` keeps its 2 (buttons 5 and 6, in `cheat_step`). The grep over `src/game/` gives 37 names in 4 files; none sits in a comment or a string.
- `pad_nav.c`'s structure is unchanged by step 4: the functions named in the inventory (`panel_step`, `aim_mode_step`, `one_shoulder`, `shoulder_switch`, `build_step`, `placement_step`, `jump_step`, `slipway_step`, `map_step`, `switch_panels`, `pause_step`, `without_chord`, `pad_nav_step`) hold the reads, and the locals and fields `cross_engaged`, `circle_returns`, `pause_cross_armed` and `menu_cross_armed` keep their names.
- The devtools still call the conversions: `pad_author.c` and `pad_metrics.c` both call `pad_jump_first_direction`, `pad_jump_direction_button` and `pad_jump_direction`. Step 8 moves them off these helpers (plan step 8, as amended after phase 1), so step 12 deletes only the game-side conversions.
- No guard on `P8_PAD_BUTTON_` exists yet: no `button_guard` script, allowance or Makefile target.

## Implementation Decisions

### What this step changes

- `docs/plan-17.md`: the step note `### Step 2.5` with the inventory below (re-checked against the grep on the step's own `HEAD`), and the amendments of section 2 below. The plan file is the only doc touched.
- A new guard: a shell script under `tools/`, its allowance file next to it, a `make button-guard` target added to `make test` and to the Makefile legend, and a unit test under `tests/`.
- No file of `src/` changes. No e2e baseline, replay, `.pad` script or `worst-case/` file changes.

### Assignment rules (the decision; the inventory applies them)

- A read inside one mode's code belongs to the step that converts that mode (9 jump; 10 aim, slipway; 11 build bar, placement; 12 menu, panels, pause, title; 13 cross-mode).
- A read in a helper shared by modes of different steps belongs to the earliest of those steps. That step gives the helper a mode argument, and every caller passes its own mode at once. So the hold-repeat (`pad_jump_repeat_step`, jump and slipway) goes to step 9, and `aim_pan` (aim and placement) goes to step 10. The later step's swap test still covers those reads.
- A read made before the mode dispatch that serves modes of several steps belongs to step 13, which computes the mode from the nav state where the read runs.
- Exception: the pressed direction computed once in `pad_nav_step` is not a cross-mode test. Each consumer replaces it by its own bindings (step 11 for `build_step`, step 12 for `panel_step`). The computation, and the button-to-direction conversions of `pad_jump.c` behind it, go with its last game-side consumer, step 12. By then the devtools no longer call them: step 8 moves `pad_author.c` and `pad_metrics.c` onto the default bindings table or a devtools-side mapping.
- A read that is not a binding (the cheat chord) is not converted. `cheat.c`'s reads are `fixed`. The chord mask in `pad_nav.c`'s `without_chord` belongs to step 13, which replaces it with a mask exported by the cheat module, not with a binding. That keeps section 2's rule that the chord stays on buttons 5 + 6 below the bindings, and leaves `cheat.c` as the only exception besides the bindings module.

### The inventory (at 6b5b753)

Counts are names outside comments and strings; a row with several names says so. Lines are `pad_nav.c`'s unless another file is named. "Indirect" rows reach the buttons through a `pad_jump` helper and are not counted by the guard; their line is the call site.

| # | Function (file) | Line | Read | Mode(s) today | Becomes | Step |
|---|---|---|---|---|---|---|
| 1 | `jump_step` (`pad_nav.c`) | 664 | pressed 7 | jump | `jump.enter_aim` | 9 |
| 2 | `jump_step` | 685 | pressed 8 | jump | `jump.open_menu` | 9 |
| 3 | `jump_step` | 688 | pressed 5, known object (click) | jump | `jump.act_on_hovered` | 9 |
| 4 | `jump_step` | 694 | pressed 5, slipway start | jump | `jump.act_on_hovered` | 9 |
| 5 | `pad_jump_repeat_step` from `jump_step` | 669 | indirect: pressed and held 1–4 (`pad_jump.c` 49, 79) | jump | `jump.jump_up/down/left/right` | 9 |
| 6 | `pad_jump_repeat_step` from `slipway_step` | 762 | indirect: pressed and held 1–4 | slipway | `slipway.jump_up/down/left/right` | 9 (shared helper) |
| 7 | `aim_mode_step` | 356 | pressed 7 | aim | `aim.leave_aim` | 10 |
| 8 | `aim_mode_step` | 360 | held 5 (fold release) | aim | `aim.probe_or_fold` | 10 |
| 9 | `aim_mode_step` | 362 | pressed 5 (engage) | aim | `aim.probe_or_fold` | 10 |
| 10 | `aim_pan` from `aim_mode_step` | 351 | indirect: held 1–4 (`aim_pan` line 154) | aim | `aim.pan_up/down/left/right` | 10 |
| 11 | `aim_pan` from `placement_step` | 573 | indirect: held 1–4 | placement | `placement.pan_up/down/left/right` | 10 (shared helper) |
| 12 | `slipway_step` | 765 | pressed 5 (release on target) | slipway | `slipway.build_on_target` | 10 |
| 13 | `slipway_step` | 770 | not pressed 5 (gate on the re-aim) | slipway | `slipway.build_on_target` | 10 |
| 14 | `build_step` | 489 | pressed 5 | build_bar | `build_bar.take_structure` | 11 |
| 15 | `build_step` | 493–494 | indirect: shared direction `PAD_DIRECTION_LEFT`/`RIGHT` | build_bar | `build_bar.pick_left`, `pick_right` | 11 |
| 16 | `placement_step` | 567 | pressed 6 | placement | `placement.cancel_placement` | 11 |
| 17 | `placement_step` | 575 | pressed 5 | placement | `placement.place_structure` | 11 |
| 18 | `panel_step` | 223, 225 | pressed 5 arms pause; held 5 masked until armed (2 names) | pause | `pause.choose_row` | 12 |
| 19 | `panel_step` | 228, 230 | pressed 5 arms menu; held 5 masked until armed (2 names) | title, menu, panels | `<mode>.choose_row` | 12 |
| 20 | `pad_panel_step` (`pad_panel.c`) | `pad_panel.c` 260 | held 5 → left mouse button | title, menu, pause, panels | `<mode>.choose_row` | 12 |
| 21 | `panel_step` → `pad_panel_step` | 233 | indirect: shared direction (up, down, left, right) | title, menu, pause, panels | `<mode>.move_up/down/left/right` | 12 |
| 22 | `map_step` | 793 | pressed 8, planet menu open | menu | `menu.open_menu` (amended: toggles closed) | 12 |
| 23 | `pause_step` | 869 | pressed 11 or 6 while pause is shown (2 names) | pause | `pause.close_pause` | 12 |
| 24 | `pad_nav_step` | 947 | indirect: `pad_jump_first_direction(pressed)` | all (rows 15, 21) | dissolved into rows 15 and 21 | 12 |
| 25 | `pad_jump_direction_button` (`pad_jump.c`), behind `pad_jump_first_direction` | `pad_jump.c` 59, 61, 63, 65 | conversion naming 1–4, one `switch` (4 names) | all | removed from `src/game/` | 12 |
| 26 | `one_shoulder` | 461 | pressed 10 xor pressed 9 (2 names) | jump, aim, slipway, menu, build_bar, placement, panels | `open_tech_panel` (`panels.next_panel`) xor `open_build_bar` | 13 |
| 27 | `shoulder_switch` | 466 | pressed 10 | same | `open_tech_panel` / `panels.next_panel` | 13 |
| 28 | `switch_panels` | 824 | pressed 6, off the map | build_bar, panels | `build_bar.close_bar`, `panels.close_panel` | 13 |
| 29 | `pause_step` | 879 | pressed 11 opens pause | jump, aim, slipway, build_bar, placement | `<mode>.pause` | 13 |
| 30 | `pad_nav_step` | 964 | held 12 (`select_down`) | every mode (tracker); acts in jump, aim | `jump/aim.select_tap_or_year` | 13 |
| 31 | `pad_nav_step` | 974 | pressed 6 → `circle_returns` | jump, aim, slipway | `jump.back`, `aim.back`, `slipway.cancel_slipway` | 13 |
| 32 | `pad_nav_step` | 975 | pressed 6 → closes a box, or the message box on the map | menu, panels; jump, aim, slipway, placement | `menu.close_menu`, `panels.close_panel`; the map mode's ○ action | 13 |
| 33 | `pad_nav_step` | 980 | clears 6 from `pressed` after closing the message box | same as 32 | same as 32 | 13 |
| 34 | `without_chord` | 935 | chord mask 5 + 6 (2 names) | all | cheat module mask (not a binding) | 13 |
| 35 | `cheat_step` (`cheat.c`) | `cheat.c` 34, 35 | held 5, held 6 (2 names) | all | stays | fixed |

Allowance at the start of step 5: `pad_nav.c` 4 (step 9: rows 1–4) + 5 (step 10: rows 7–9, 12–13) + 3 (step 11: rows 14, 16–17) + 7 (step 12: rows 18, 19, 22, 23) + 11 (step 13: rows 26–34) = 30; `pad_panel.c` 1 (step 12: row 20); `pad_jump.c` 4 (step 12: row 25); `cheat.c` 2 (fixed: row 35). Total 37 names in 4 files, which is the grep's count. Count names, not lines: `git grep -c` counts lines and gives 27 for `pad_nav.c`, because lines 461, 869 and 935 name two buttons each. If the grep on the step's `HEAD` disagrees, find which read moved, apply the rules, and record it in the step note.

### Mode of the reads that run before the dispatch (for step 13, settled here)

- **Rows 31–33, the ○ read.** With a box open (`pad_panel_box_open`, not pause): `menu` on the map, `panels` over a cycle panel, `build_bar` over the bar. On the map with no box: the map mode under any pause overlay (`placement` when carrying, else `slipway`, `aim` or `jump` from `map_mode`), whose ○ action is `back`, `cancel_slipway` or `cancel_placement`. Reason: the message-box close tests `on_the_map`, which stays true while pause is shown, so on a pause release frame ○ can still close a year box under the pause. Reading `pause.close_pause` (6 and 11) there would let Start close it too.
- **Row 30, Select.** `select_down` reads the union of `jump.select_tap_or_year` and `aim.select_tap_or_year` in every mode. Reason: `pad_select_step` runs on every frame, and while Select is not allowed it records `fired = down`. That latch stops a Select held from another mode from firing on return. Reading an unbound mode's mask (0) there would drop the latch and change behaviour.
- **Rows 26–27, 29, the shoulders and pause.** The mode is the current one (`panels` when a cycle panel is open, `build_bar` when the bar is open, `menu` when the planet menu is open, else the map mode). The existing gates (`shoulders_can_switch`, `pause_button_free`) stay. In title and pause, where those gates already refuse, the actions are unbound anyway.
- **Row 28.** `build_bar` when the bar is open, else `panels`.

### Mode of `panel_step` (for step 12, settled here)

`panel_step` serves every state where a ui panel is open: `pause` when the pause menu is shown; `title` when no game exists; `panels` when a cycle panel (tech or score) is open; `menu` otherwise, which covers any box over the map, including one opened by an aim-mode click, and any panel over the build bar. Section 2 already calls `menu` "the box of a hovered object". This rule extends it to every non-pause box, which is what the shared code does.

### Amendments to section 2 (written into `docs/plan-17.md` by this step)

1. **menu: `close_menu 6`, and a new `open_menu 8`** (in place of `close_menu 6 and 8`). The code closes the box on two paths: ○ before the dispatch (any non-pause box, whatever a pending click), and △ in `map_step` (planet menu only, and only with no click releasing or pending). One action read on both paths would give △ the ○ path and ○ the △ path. `open_menu` in menu mode is documented as "pressed again, closes the box", the same button that opened it from jump.
2. **build_bar: `close_bar 6`, and a new `open_build_bar 9`** (in place of `close_bar 6 and 9`). ○ closes the bar in `switch_panels` (off the map, no foreign panel, no release). L1 closes it through the shoulder path (`one_shoulder`: exactly one of L1 and R1, `shoulders_can_switch`). One action on both paths would let L1 + R1 together close the bar through the ○ path. With `open_build_bar` in build_bar mode ("pressed again, closes the bar"), the shoulder code reads `open_build_bar` in every mode that has it.
3. **Message box.** The locked decision "`back` first closes a shown message box (the year box) in jump and aim mode" becomes: "On the map, the mode's ○ action (`jump.back`, `aim.back`, `slipway.cancel_slipway`, `placement.cancel_placement`) first closes a shown message box and does nothing else." Reason: `pad_nav_step` closes it whenever the nav is on the map with no box open, which includes slipway and placement. A message box is not a ui panel, so those modes stay current while it is shown.
4. **The cheat chord.** Section 2's sentence "`cheat.c` and `pad_nav.c`'s `without_chord` keep hiding buttons 5 and 6" gains: "`without_chord` takes the chord mask from the cheat module, so no button name is left in `pad_nav.c`." No table change.
5. **`pause.close_pause 6 and 11`** stays: both buttons are read in one test with one condition. Recorded in the note so nobody "fixes" it like items 1 and 2.

Action names are kept from section 2 where possible. Adding `open_menu` to menu mode and `open_build_bar` to build_bar mode reuses names the enum already has, so step 6 adds no new action.

### The guard

- **Script.** `tools/button_guard.sh <allowance> <directory>`, built like `tools/random_guard.sh`: POSIX shell plus one awk program. It strips comments and string or character literals the same way, then counts every identifier starting with `P8_PAD_BUTTON_` in each `.c` and `.h` of the directory (the Makefile passes `src/game`). It prints the differences sorted, then `button-guard files N reads M`. It exits 1 on any difference and 2 on an invalid or empty allowance or a missing directory. The comment stripper is copied, not shared with `random_guard.sh`. Factoring it out would touch a guard outside this step.
- **Allowance format.** `tools/button_guard_allowance.txt`, one line `<path> <owner> <count> | <reason>`, where owner is `step-9` … `step-13` or `fixed`, and a leading `#` marks a comment. Several lines per path are allowed (one per owner). The guard sums them per path. A duplicate `<path> <owner>`, an unknown owner, a count that is not a positive integer, an empty reason, a listed file that does not exist, or an empty list is invalid (exit 2).
- **Strict equality per path.** A path with names but no line, a path whose sum differs from its count, and a listed path with no name left all fail, naming the path and both numbers. Strict equality is what makes "each of steps 9–13 removes exactly its entries" enforceable. A step that converts a read without deleting its line fails `make test`, and so does one that adds a read.
- **Granularity: per file and owner, not per button.** That matches the step's "remaining reads per file". The owner column makes each step's lines its own to delete. The per-read detail lives in the step note, not in the guard.
- **Seeded** from the inventory: one line per (file, owner), the reason naming the functions and actions (for example `src/game/pad_nav.c step-9 4 | jump_step: enter_aim, open_menu, act_on_hovered`).
- **Makefile.** `BUTTON_GUARD_ALLOWANCE := tools/button_guard_allowance.txt`, a `button-guard` target depending on the script and the allowance, added to `test:`, and a legend entry in the style of `random-guard`. The unit-test binary is registered like `test_random_guard`. Nothing is added to `make e2e`.
- **Bindings module.** The guard has no built-in exception. If step 6's `bindings.c` names `P8_PAD_BUTTON_`, step 6 adds a `fixed` line. Step 14 later removes the allowance in favour of the two fixed exceptions (plan step 14), so the `fixed` owner is a bridge, not a second mechanism.

## Implementation Chunks

1. **The guard.** Script, unit test, Makefile target and legend, and an allowance seeded from a fresh count of names (`git grep -o P8_PAD_BUTTON_ -- src/game`, not `-c`, which counts lines) split by owner per the inventory rules. Verified: `make test` green and the timing of `make test` against step 1's baseline.
2. **Inventory and reconciled table.** Write `### Step 2.5` with the full inventory (re-checked against the grep on the step's `HEAD`), and apply the five amendments to section 2 of `docs/plan-17.md`. Verified: the note's per-file totals equal the guard's counts, `make test-all` green, and every `.sha256` unchanged (no source changed).

## Testing Decisions

- A good test drives the guard as a black box: a scratch tree and an allowance in, the exit code and the printed lines out. It never inspects the awk program.
- **New test** `tests/test_button_guard.c`, prior art `tests/test_random_guard.c` (scratch directory under `tmp/`, files written by the test, the exit code read back from a file, directories made once). Cases:
  - a tree matching the allowance passes, with the summary line;
  - an extra name in a listed file fails, naming the file and both counts;
  - a name in an unlisted file fails;
  - a listed count higher than the tree fails (stale line);
  - a listed file with no name left fails;
  - two owner lines for one path are summed;
  - names inside a `/* */` comment, a `//` comment, a string literal (an assert message) and a character literal are not counted;
  - an identifier that only contains the prefix elsewhere (for example `MY_P8_PAD_BUTTON_5`) is not counted, and `P8_PAD_BUTTON_12` counts once;
  - an invalid allowance gives exit 2: unknown owner, duplicate path and owner, missing reason, non-positive count, listed file missing, empty list, missing directory.
- **On the real tree**: `make button-guard` passes on the step's `HEAD`. Once, recorded in the step note and not committed: adding a `P8_PAD_BUTTON_5` read to `pad_nav.c` fails `make test`, and so does deleting one allowance line.
- **No game test changes.** No source of `src/` changes, so no unit test, no e2e baseline and no generator output changes. `make e2e` and `make end-state` are run once to prove it. The section 2 guardrail's generator commands are not required: the step touches no `src/game/pad_*`, `src/devtools/pad_*`, `input_json.c` or button constant.
- **Timing**: `make test` and `make e2e` against step 1's baseline (`make test` 4.27–4.58 s, `make e2e` 0.24–0.25 s, warm build). The guard reads about five small files with one awk pass, a few milliseconds.

## Out of Scope

- Any `bindings_mask` call, the bindings module, the mode and action enums, `bindings.default.cfg` and the generator (steps 6–8).
- Converting any read, moving the chord mask behind the cheat module, or touching `pad_jump`'s conversions (steps 9–13).
- Closing the guard and removing the allowance (step 14).
- Renaming locals that carry a PSX word (`cross_engaged`, `circle_returns`, …).
- Reads of pad buttons outside `src/game/` (`src/devtools/`, `src/platform/`, `src/cmd/`): the guard and the plan's rule cover `src/game/` only.
- Deciding the build bar's behaviour when a vertical direction is pressed together with left or right (see Further Notes): step 11's call.
- Deleting `docs/pad-modes-and-actions.md` (step 20). This step does not edit it either: section 2 is the spec, and the doc dies with step 20.

## Further Notes

- **Trap for step 11.** Today the build bar picks from the shared direction, which takes the first pressed of up, down, left, right. On a frame where up or down is newly pressed together with left or right, the bar does not move. A plain `bindings_mask(build_bar, pick_left)` would move it. No replay is known to press that chord in the bar. Step 11 checks the unit tests: if one pins it, follow the code. Otherwise record the divergence in its step note.
- **Trap for steps 11 and 12.** The panels' `move_*` must keep the up, down, left, right priority among themselves (take the first pressed in that order), as `pad_jump_first_direction` does today.
- **Trap for step 9.** `jump.act_on_hovered` is read twice in `jump_step` (click a known object, then start a slipway), each with its own conditions. Both reads become the same action. Do not merge them into one test.
- **Trap for step 10.** In aim, `aim.back` (○, through `circle_returns`, step 13) and `aim.leave_aim` (□, step 10) have the same effect but are separate reads with separate owners.
- **Trap for step 13.** `circle_returns` is a boolean handed to `aim_mode_step`, `jump_step` and `slipway_step`. Those functions read no button for it, so steps 9 and 10 leave it alone. It is converted only where it is computed.
- The step note records whether the inventory still matched the grep on the step's `HEAD`, the guard's summary line, the two one-off red runs, and the timings. The plan-pilot step note convention is `### Step 2.5`.
