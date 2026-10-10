---
title: "Controls docs live in the two config files: README points to them, pad-modes doc deleted"
description: "The controls are explained in three places (README with PSX symbols, docs/pad-modes-and-actions.md, bindings.default.cfg comments), and two of them describe a hardcoded scheme the game no longer has."
status: needs-triage
---

## Problem Statement

Plan 17 step 20 (Phase 5, Docs and cleanup). The step's full text, from `docs/plan-17.md` section 3:

> 20. **Docs.** The README's Controls section becomes the pointer of section 2. Delete `docs/pad-modes-and-actions.md`. Verified by reading: no README line names a PSX symbol as a control, and every mode of section 2 is explained in `bindings.default.cfg`.

Section 2's locked decisions for docs:

- "`bindings.default.cfg` carries, as `#` comments, what each mode is and does. It replaces `docs/pad-modes-and-actions.md`, which is deleted once the file exists."
- "The README's Controls section becomes a pointer to `keymap.default.cfg` and `bindings.default.cfg` and how to override them."
- Rejected: "README describing controls per action and key: the user wants a pointer to the two files only." and "Keeping `docs/pad-modes-and-actions.md` with a sync check: a third copy of the list."
- Rejected: "PSX button names in the files or the replays: the code must not think in 'cross' any more."

State at HEAD df9e5e2 (steps 1–18 done; step 19 in parallel, see the hypotheses):

- **README, Controls section (lines 16–41).** A key table whose rows are PSX buttons (`× act`, `○ back`, `□ aim mode`, `△ menu`, `L1 build bar`, `R1 tech / score`, `Start pause`, `Select attention / end year`) with QWERTY, AZERTY and gamepad columns; line 30, already rewritten by step 15 ("Keys are set in `keymap.default.cfg`; to change them, copy it to `keymap.cfg`, edit it, then run `make raylib`."); then eight paragraphs (lines 32–41) explaining each mode in PSX symbols. A player who edits `bindings.cfg` gets a README that is wrong by construction: "□ enters aim mode" is no longer a fact, only the default.
- **README, other lines.** Line 9 "keyboard or gamepad" and line 12 "Space bookmarks a frame" name no PSX button. Space is the recorder's bookmark key, not one of the 12 buttons, and stays where it is.
- **`docs/pad-modes-and-actions.md`.** Lists the 9 modes, the actions of each with a `nothing` entry (which section 2 abolished), and five cross-mode rules, three of them in PSX symbols (`close_menu is both ○ and △`, `close_pause is both Start and ○`, `a ○ press first closes an open message box`). Its intro still says the binding file is "not yet read by the game". The only references to it are in `docs/plan-17.md` (the plan itself), so deleting it breaks no link.
- **`bindings.default.cfg`** (140 lines, 4768 bytes, longest line 78 characters). Header: format, the button layer and `keymap.default.cfg`, unbound-on-missing, conflicts, personal file replacing the default whole, the `"bindings": "personal"` recording flag (step 17), the reserved 5 + 6 cheat chord. Then one `# <mode>: …` comment block per mode, all 9 present. No PSX symbol anywhere.
- **`keymap.default.cfg`.** Header explains the `N = name[, name…]` format, raylib key and gamepad names, that letter keys are US-layout positions, points to `bindings.default.cfg`, and says how to override (copy to `keymap.cfg`, edit, `make raylib`). No PSX symbol (`RIGHT_FACE_DOWN` etc. are raylib names).

### Coverage check: `docs/pad-modes-and-actions.md` against `bindings.default.cfg`

| What the doc explains | In `bindings.default.cfg` today | Gap and how this step fills it (in the cfg) |
|---|---|---|
| The 9 modes | A `# <mode>:` block for each of title, jump, aim, slipway, menu, pause, build_bar, placement, panels | none |
| Actions of each mode | Every action is a binding line (35 actions; the reconciled table of step note 2.5 adds `build_bar.open_build_bar` and `menu.open_menu`) | none. The `nothing` entries disappear by section 2's decision |
| "Every press is read in the current mode and maps to one action" | Header: "Which buttons trigger each action of the game, mode by mode." | none |
| `menu` is the box of any hovered object, **opened by `open_menu`** | menu block says what the box is and that `open_menu` pressed again closes it | **Gap:** the jump block never says `jump.open_menu` opens the hovered object's box. Add it to the jump block |
| `close_menu` is both ○ and △ | menu block: "open_menu pressed again closes the box" + the two lines | none (reconciled split, step note 2.5) |
| In aim, `back` and `leave_aim` have the same effect | aim block | none |
| From slipway, menu or placement, `open_build_bar`/`open_tech_panel` first cancel or close; `pause` from placement keeps the structure | slipway, menu and placement blocks | none |
| `close_pause` is both Start and ○ | `pause.close_pause = 6, 11` | none |
| On the map, a ○ press first closes a message box | jump, aim, slipway, placement blocks | none |

So against the doc to delete there is one gap (jump's `open_menu`). The README paragraphs this step removes carry much more, and section 2 sends all of it to `bindings.default.cfg`, not to the README. Coverage check against README lines 32–41:

| README fact (lines 32–41) | In the cfg today | Fill |
|---|---|---|
| Point at a thing, the act action acts on it, back backs out | no | header, "Across modes" paragraph |
| Jump: a blinking circle on an object; the game starts here once an object is known | partly ("the cursor jumps from object to object") | jump block |
| Jump: the jump goes to the next object that way, anywhere on the map; nothing moves when none lies that way; the camera follows only near the screen edge | no | jump block |
| `act_on_hovered`: on a colony or structure starts a slipway; on a known planet opens its projects | no | jump block |
| `open_menu` opens the hovered object's box (and closes it again, from menu) | menu half only | jump block |
| `enter_aim` enters aim mode | name only | jump block |
| `back` in jump: jumps back to the last object `act_on_hovered` acted on | no | jump block |
| Aim: a crosshair at the centre; a game with nothing known starts here | "a free crosshair pans over it" | aim block |
| `probe_or_fold`: on empty space launches the probe; once a probe is out, held, it folds space | "probes on a press and folds while held" | aim block, made precise |
| Until an object is known, `leave_aim` and `back` do nothing | no | aim block |
| No `open_menu` in aim | implied by the missing line | aim block, one clause |
| Slipway: started by `jump.act_on_hovered` on a colony or structure; the jump actions aim at another object; the cost or the refusal ("too far", a forbidden sign when already linked) shows above it | "a slipway is being drawn from a planet" | slipway block |
| `build_on_target` builds; a refused one aims again from the same start; once built, the next slipway starts from the object reached, so act and jump chain them; a full slipgate starts nothing | no | slipway block |
| `cancel_slipway` drops it | name only | slipway block |
| Build bar: `pick_left`/`pick_right` pick, `take_structure` takes it into placement; `open_build_bar` again or `close_bar` closes it; `open_tech_panel` closes it and opens tech | partly | build_bar block |
| Placement: the pan actions move the camera under the structure; `place_structure`, `cancel_placement` | partly | placement block |
| Panels: `next_panel` goes tech, then score, then closed; `close_panel` back to the map; `open_build_bar` opens the bar | "the tech and score panels" | panels block |
| `select_tap_or_year`: on the map in jump or aim mode; a tap jumps to the next colony or structure whose panel lists a need; held about half a second, it ends the year, once per hold; a fresh press within a quarter of a second after a year ends ends one more | "acts on a tap; held for half a second, it shows the year" | jump block, and **fix**: the code (`select_ends_year`, which clicks the year-skip button, `PAD_SELECT_HOLD_FRAMES` 30, `PAD_SELECT_CHAIN_FRAMES` 15 at 60 fps) ends the year; "shows the year" is wrong |
| Menus and pause: moves, choose, close; a `choose_row` held from before picks nothing | yes (menu, pause, panels, title) | none |
| `pause` opens from the map modes and the build bar, not from a menu, tech or score | implied by the missing lines | header, "Across modes" |
| Back order: each back press backs out one step: a pause or box (the year box included), then a slipway or a structure being placed, then aim mode, then the jump back to the last object | no | header, "Across modes" |

Hidden couplings found in the step notes, which a player editing a personal file would hit and which no comment states today (section 1's goal: "No hidden control rule"):

- Step note 2.13: `select_tap_or_year` bound in only one of jump or aim also works in the other (the code reads the union).
- Step note 2.11: the build bar picks only on a frame where neither `panels.move_up` nor `panels.move_down` is pressed (plan 12's D-pad ranking, kept).

## Solution

1. **`bindings.default.cfg` comments** gain everything in the two "Fill" columns above, written with action names, never PSX symbols or "D-pad", and the Select sentence is corrected to "ends the year". The header gains a short "Across modes" paragraph (point and act, back order, where pause opens, the Select union). Each mode block gains its facts. The build_bar block states the `panels.move_up`/`move_down` coupling. No binding line changes.
2. **README's Controls section** becomes a pointer: the game reads 12 numbered buttons; `keymap.default.cfg` says which keys and gamepad buttons press each; `bindings.default.cfg` says what each button does in each mode and explains every mode; to change either, copy it to `keymap.cfg` or `bindings.cfg`, edit the copy, run `make raylib`, which prints which file it compiled. The key table and the eight mode paragraphs go.
3. **`docs/pad-modes-and-actions.md` is deleted**, after (1) is in the tree.

## User Stories

1. As a player, I want the README to tell me where the controls are defined, so that I find the real, current scheme instead of a copy that can drift.
2. As a player, I want the README to say how to change keys, so that I can adapt the game to my keyboard or gamepad.
3. As a player, I want the README to say how to change what a button does, so that I can rebind an action without reading C code.
4. As a player, I want the README to say that `make raylib` prints which file it compiled, so that I can check my personal file was picked up.
5. As a player with a personal `bindings.cfg`, I want no README sentence that says "□ enters aim mode", so that the README is never wrong about my game.
6. As a player, I want `bindings.default.cfg` to explain what every mode is, so that I know which mode a binding line applies to.
7. As a player, I want each mode's comment to say what each of its actions does, so that I can pick which action to move to which button.
8. As a player, I want the jump block to say that `act_on_hovered` starts a slipway on a colony or structure and opens a known planet's projects, so that I know that one action does both.
9. As a player, I want the jump block to say that `open_menu` opens the hovered object's box, so that the box of the menu mode has a known entry point.
10. As a player, I want the jump block to say that jumping goes to the next object that way anywhere on the map, and does nothing when none lies there, so that a still cursor is not mistaken for a broken binding.
11. As a player, I want the jump block to say that `back` returns to the last object `act_on_hovered` acted on, so that I know what back does when nothing is open.
12. As a player, I want the aim block to say what `probe_or_fold` does on empty space and once a probe is out, so that I know a hold folds space.
13. As a player, I want the aim block to say that a game with nothing known starts in aim, and that `leave_aim` and `back` do nothing until an object is known, so that a dead press there is expected.
14. As a player, I want the slipway block to say how a slipway starts, what the game shows above the target, what a refusal does and how slipways chain, so that I can bind the actions knowing the flow.
15. As a player, I want the build_bar and placement blocks to say how a structure goes from the bar to the map, so that I bind `take_structure` and `place_structure` knowing they follow each other.
16. As a player, I want the build_bar block to say that a pick is ignored on a frame where `panels.move_up` or `panels.move_down` is pressed, so that moving those actions does not surprise me.
17. As a player, I want the panels block to say that `next_panel` goes tech, then score, then closed, so that I know the cycle.
18. As a player, I want the Select comment to say a hold ends the year (not "shows the year"), so that the file says what the game does.
19. As a player, I want the header to say that `select_tap_or_year` bound in only one of jump or aim also works in the other, so that unbinding it in one mode does not seem to fail.
20. As a player, I want the header to give the back order (pause or box, then slipway or carried structure, then aim, then the jump back), so that I know what one back press undoes.
21. As a player, I want the header to say where `pause` opens and where it does not, so that a missing `pause` line in menu or panels is understood as deliberate.
22. As a player, I want every comment to use action names and button numbers only, so that the file never thinks in PSX names a personal keymap may not have.
23. As the developer, I want `docs/pad-modes-and-actions.md` gone, so that there is no third copy of the mode list to keep in sync.
24. As the developer, I want the deletion to come after the cfg carries its one missing fact, so that nothing the doc said is lost.
25. As the developer, I want the cfg edit to change comments only, so that the generated default table, every `.sha256` and every end state stay byte-identical.
26. As the developer, I want every new comment line to stay within the parser's line limit, so that the build does not fail on documentation.
27. As the developer, I want the frozen e2e fixtures left untouched, so that test data does not move with the default's comments.
28. As the developer, I want the README check to be a grep anyone can rerun, so that "no PSX symbol as a control" is not a matter of opinion.

## Implementation Decisions

### `bindings.default.cfg`

- **Comments only.** No binding line is added, removed, reordered or changed. The generated default unit (`tmp/bindings/default.c`) must be byte-identical before and after (step note 3.15 shows the generator drops comments).
- **Vocabulary.** Comments name actions (`jump.act_on_hovered`, or `act_on_hovered` inside its own mode block) and, where the file already does, button numbers (the 5 + 6 chord). No `×`, `○`, `□`, `△`, cross, circle, square, triangle, L1, R1, Start, Select as a button name, and no "D-pad": the jump and pan actions are named instead. The comments describe actions, so they stay true in a personal copy that rebinds them.
- **Line length.** Keep every line at 78 characters or less, as today. The parser refuses lines over 127 characters (`BINDINGS_MAX_LINE_LENGTH`), comments included: a long comment line would fail `make`. The file stays far below the generator's 65536-byte input limit (and `replay`'s fixture limit, which concerns copies).
- **Header "Across modes" paragraph**, after the reserved-chord paragraph, before the title block. It carries: point at a thing, the mode's act action acts on it, its back action backs out; the back order (pause or a box, the year box included; then a slipway or a structure being placed; then aim mode; then the jump back to the last object acted on); `pause` opens from jump, aim, slipway, build_bar and placement, not from a menu, the panels or the title (hence no `pause` line there); `select_tap_or_year` is read in jump and aim together, so a button bound to it in either works in both.
- **Mode blocks**: add the "Fill" facts of the Problem Statement's tables to the block they name, keeping each block's existing sentences unless they are wrong. The one wrong sentence: jump's "held for half a second, it shows the year" becomes "held about half a second, it ends the year, once per hold; a fresh press within a quarter of a second after a year ends ends one more", plus the tap (jump to the next colony or structure whose panel lists a need). The aim block's "select_tap_or_year as in jump" stays.
- **build_bar block** gains the coupling: a pick happens only on a frame where neither `panels.move_up` nor `panels.move_down` is pressed (the bar borrows the panels' up/down; plan 12's ranking).
- **What not to add**: the arcane edge cases of step note 2.13 (the map mode's back action under pause, on the title or on the final score panel; boxes over the build bar closing on `menu.close_menu` is already said by the menu block) and the hold-repeat and pan-acceleration timings. They are true but no player rebinding a button needs them, and a long file is not read.
- `CLAUDE.md`'s comment rule (40 words, traps and C contracts) governs code comments. These `#` lines are the file's documentation by section 2's decision, the way `keymap.default.cfg`'s header is. Keep each block short: the block plus its lines should read on one screen.

### README

- The `## Controls` heading stays (other text may refer to it). Its body becomes one short paragraph, or two, holding only: the game reads 12 numbered buttons; `keymap.default.cfg` maps keys and gamepad buttons to them; `bindings.default.cfg` maps each mode's actions to them and explains every mode; to override, copy a file to `keymap.cfg` or `bindings.cfg` (gitignored), edit the copy and run `make raylib`, which prints the file it compiled for each; a personal `bindings.cfg` replaces the default whole (a missing line leaves the action unbound) and recordings made with it can't be replayed. Step 15's line 30 is folded into this paragraph, not kept twice.
- Removed: the key table (its AZERTY column is covered by `keymap.default.cfg`'s "letter keys are key positions named after the US layout"), the "Point at a thing" line, the eight mode paragraphs.
- Untouched: "Running the game" (including "keyboard or gamepad" and "Space bookmarks a frame", the recorder's key, not one of the 12 buttons), "To do".
- `keymap.default.cfg` is not edited: its header already points to `bindings.default.cfg` and says how to override.

### `docs/pad-modes-and-actions.md`

- Deleted with `git rm` in the same commit as the README, after the cfg commit. `docs/plan-17.md` keeps its mentions (it is the plan that orders the deletion). No other file links it.

### Hypotheses on step 19 (implemented in parallel)

- **H-19-1. Step 19 touches no file this step edits.** Per `docs/prd/triage/phase4-step19-unbound-action.md`, it adds `tests/e2e/unbound_enter_aim.cfg`, `pad_unbound_enter_aim.json` and its `.txt`, `.json.sha256`, `.json.end-state`, and changes nothing under `src/`, `tools/`, the `Makefile`, `worst-case/`, `README.md`, `bindings.default.cfg`, `keymap.default.cfg` or `docs/pad-modes-and-actions.md`. If wrong (one of these files changed), rebase this step's edits onto it and re-run the coverage check on the result.
- **H-19-2. Its fixture is a frozen copy, whichever lands first.** The fixture is `bindings.default.cfg` at step 19's base minus `jump.enter_aim = 7`, plus its own header. If step 19 lands first, its comments will differ from the new default's; if this step lands first, the fixture may carry the new comments. Both are fine: fixtures are frozen test data (as `swap_aim_menu.cfg` says, "later edits to the default are not carried here"), and the replay hashes do not include fixture bytes. This step never edits `tests/e2e/*.cfg`.
- **H-19-3. The suite count is the only difference.** With step 19 in, `make e2e` checks 19 replays and `make end-state` 24; without, 18 and 23. Either is green for this step; it adds or changes no replay.

## Implementation Chunks

1. **Explain every mode in `bindings.default.cfg`.** Comments only, per the decisions above. Commit `docs: bindings.default.cfg explains every mode and action`.
2. **README points to the two files; delete the pad-modes doc.** Commit `docs: README controls point to the config files`.

## Testing Decisions

- Documentation only: no unit test is added. A test that every mode has a comment block would be the sync check section 2 rejected, and would add to `make test`.
- Mechanical checks, run and quoted in the step note:
  - Before chunk 1, save `tmp/bindings/default.c`; after it, `make test-all` green and `cmp` with the saved copy shows no difference. `test_bindings` (which parses the committed file and checks the generated table equals it) passes. Every `tests/e2e/*.json.sha256` unchanged (`git status` shows no change under `tests/`), `make end-state` matches on every replay.
  - `awk 'length > 78' bindings.default.cfg` prints nothing.
  - Every mode has its block: for each of the 9 modes, `grep -c '^# <mode>:' bindings.default.cfg` gives 1.
  - No PSX vocabulary in either config file or the README: `grep -n -i -E '×|○|□|△|d-pad|\b(cross|circle|square|triangle|L1|R1)\b' README.md bindings.default.cfg keymap.default.cfg` prints nothing, and `grep -n -E '\b(Start|Select)\b' README.md bindings.default.cfg` prints nothing (case-sensitive, so prose like "starts" and the action `select_tap_or_year` do not match).
  - `git grep -n pad-modes-and-actions` lists only `docs/plan-17.md`.
  - `make raylib` links (if vendored) and prints its two `bindings:`/`keymap:` lines unchanged.
- Verified by reading, recorded in the note: the coverage tables of this PRD, row by row, each "Fill" found in the new comments, and each comment sentence true against the code it describes (`pad_nav.c`, `pad_select.c`, `pad_build.c`, `pad_slipway.c`).
- Prior art: step 15 edited `bindings.default.cfg`'s keymap comment and checked `tmp/bindings/default.c` byte-identical (step note 3.15).

## Out of Scope

- Any binding line or control behaviour change, including the Select union and the bar's panels coupling (documented, not changed).
- `keymap.default.cfg` content, the frozen fixtures under `tests/e2e/`, `docs/plan-17.md`, other docs (`docs/save-format.md`'s `×` is a multiplication sign, `docs/psx-hello-world.md` is the paused PSX).
- A README table of actions or keys (rejected in section 2).
- Step 21's comment rework.
- Audio.

## Further Notes

- `make test` and `make e2e` cannot get slower: no code, Makefile or test changes. The note still records one timing of each for the phase's record.
- Trap for the implementer: the bindings parser counts comment lines against its 127-character limit, so an unwrapped paragraph breaks `make` with `bindings.default.cfg:<line>: line longer than 127 characters`.
