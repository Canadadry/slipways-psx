---
title: "Jump mode reads its bindings"
description: "Jump mode and its shared hold repeat still test hardcoded buttons 1–5, 7 and 8, so a bindings table handed to pad_nav changes nothing in the mode the player spends most time in."
status: needs-triage
---

## Problem Statement

Plan 17 step 9 (Phase 2, bindings in the game). The step's text, from `docs/plan-17.md` section 3:

> 9. **Jump mode reads its bindings.** Every button test in `jump_step` and its helpers goes through `bindings_mask(…, MODE_JUMP, …)`, including the hold repeat. Verified: e2e unchanged, plus a unit test with a table that swaps actions in the converted mode and leaves every other mode unbound: the converted mode follows the table, and the same buttons do nothing in a neighbouring mode served by the same shared code. Without it a read tagged with the wrong mode passes, because the default puts most actions on the same button in every mode. In steps 9–13, the step that converts a read also renames the local or comment that carries a PSX name on that line (`cross_engaged`, `circle_returns`, a `×` in a comment, …) to the action's name.

Since step 7, `pad_nav` holds a copy of the compiled bindings table (`pad_nav_bindings()`), but no code reads it. In jump mode the player still gets plan 12's controls whatever `bindings.cfg` says: `jump_step` tests `P8_PAD_BUTTON_7` (enter aim), `P8_PAD_BUTTON_8` (open the hovered object's box) and `P8_PAD_BUTTON_5` twice (act on the hovered object: click a known object, or start a slipway from a colony). The jump hold repeat (`pad_jump_repeat_step`), shared by jump and slipway mode, reads buttons 1–4 through `pad_jump_first_direction` and `pad_jump_direction_button`. A player who swaps `jump.enter_aim` and `jump.open_menu` in a personal file sees nothing change.

## Solution

Jump mode asks the table. Each of the four reads in `jump_step` becomes "is a button bound to this jump action pressed", and the hold repeat takes the table and the mode it serves, so jump mode moves on `jump.jump_*` and slipway mode on `slipway.jump_*`. With the default table, every frame is identical by construction: every e2e `.sha256` and every guardrail output stays byte-identical. With a swapped table, jump mode follows it. With a table that leaves slipway unbound, the same direction buttons do nothing in slipway mode, which proves the shared helper reads the right mode. The guard allowance loses its step-9 line.

## Facts checked at HEAD 7302873 (step 7 complete, committed)

- `src/game/bindings.h`: `bindings_mode_t` (9 modes, `BINDINGS_MODE_JUMP`, `BINDINGS_MODE_SLIPWAY`, …), `bindings_action_t` (35 actions, among them `JUMP_UP`, `JUMP_DOWN`, `JUMP_LEFT`, `JUMP_RIGHT`, `ACT_ON_HOVERED`, `ENTER_AIM`, `OPEN_MENU`), `bindings_t { int masks[mode][action]; }`, `bindings_mask` (0 for an unbound action or one the mode lacks), `bindings_mode_has_action`, `bindings_parse`, and `extern const bindings_t bindings_compiled`.
- `pad_nav_t` has a field `bindings_t bindings` (a copy), set by `pad_nav_init(nav, const bindings_t *)`; `pad_nav_bindings(nav)` returns its address. `input_step_init` hands the table over.
- `bindings.default.cfg` binds `jump.jump_up..jump_right` 1–4, `jump.act_on_hovered` 5, `jump.enter_aim` 7, `jump.open_menu` 8, `slipway.jump_up..jump_right` 1–4 (step 2.7 note, proven by `test_bindings`).
- The inventory of step 2.5 (rows 1–6) was written before step 7 added lines to `pad_nav.c`; at HEAD its line numbers moved by +6. In `pad_nav.c`: `jump_step` (line 665) reads `pressed & P8_PAD_BUTTON_7` (670), `pressed & P8_PAD_BUTTON_8` (691), `pressed & P8_PAD_BUTTON_5` with a known object (694) and `pressed & P8_PAD_BUTTON_5` before `cross_starts_slipway` (700). `pad_jump_repeat_step(&nav->jump, pad_buttons, pressed)` is called from `jump_step` (675) and `slipway_step` (768).
- `pad_jump_repeat_step(jump, pad_buttons, pressed)` in `pad_jump.c`: the first of UP, DOWN, LEFT, RIGHT in `pressed` starts a repeat; otherwise the repeat continues while `pad_buttons` holds the repeat direction's button, firing at `PAD_JUMP_REPEAT_DELAY` then every `PAD_JUMP_REPEAT_PERIOD`. `repeat_direction` is a `pad_direction_t`, not a button, and survives the jump → slipway change (`repeat_kept`).
- `pad_jump_first_direction` and `pad_jump_direction_button` are also called from `pad_nav_step` (the shared pressed direction, line 953), `aim_pan` (154), `tests/test_pad_jump.c`, `tests/test_pad_nav.c`, and, at HEAD, from `src/devtools/pad_author.c` and `pad_metrics.c` (see H-8-2).
- Callers of `pad_jump_repeat_step`: `pad_nav.c` (2), `tests/test_pad_jump.c` (about 20 calls, through a `hold_frames` helper too), `tests/test_pad_metrics.c` (`repeats_against_pad_jump`, one call). No file under `src/devtools/` calls it.
- Makefile: `test_pad_jump` and `test_pad_metrics` link `$(ENGINE_SRCS)` (so `bindings.c`) but not `$(BINDINGS_DEFAULT_UNIT)`, so they cannot name `bindings_compiled` today. `test_pad_nav` and `test_input_step` link it.
- `tools/button_guard_allowance.txt` has `src/game/pad_nav.c step-9 4 | jump_step: jump.enter_aim, jump.open_menu, jump.act_on_hovered (twice)`. `pad_nav.c`'s lines sum to 30 today; the guard requires strict equality per file. `pad_jump.c`'s line (`step-12 4`) covers the four names inside `pad_jump_direction_button`.
- `tests/test_button_guard.c` uses its own temporary allowance (`nav.c step-9 2`), not the real file.
- `tests/test_pad_nav.c` can reach jump mode without pressing any button: `begin_game` (title then `cart_start_game`, no frame run), then `settle_planet` on a planet (frames with no button) gives jump mode seeded on the colony, as `test_jump_mode_seeds_on_the_colony_over_a_nearer_known_planet` does. `start_title` calls `input_step_init(step, &bindings_compiled)`. Helpers `expected_jump(origin, button)`, `slipway_pick(start, origin, button)`, `in_forked_copy`/`end_forked_copy`, `make_every_planet_known`, `assert_hovering` exist.

## Hypotheses on step 8's outcome

Step 8 ("Generator high level reads the default bindings", PRD `docs/prd/triage/phase2-step8-generator-reads-default-bindings.md`) is being implemented in parallel and is not committed. This PRD was written from HEAD 7302873 and from step 8's PRD. Each assumption about step 8 is a hypothesis:

- **H-8-1. Step 8 leaves `src/game/` and the guard alone.** It changes no file under `src/game/` (`pad_jump.{c,h}`, `pad_nav.{c,h}`, `bindings.{c,h}` included), nor `tools/button_guard_allowance.txt`, nor `tests/test_pad_jump.c` or `tests/test_pad_nav.c`.
- **H-8-2. The devtools are off `pad_jump`'s conversions.** After step 8, no file under `src/devtools/` calls `pad_jump_first_direction`, `pad_jump_direction_button` or `pad_jump_repeat_step` (`pad_metrics.c` has its own `pad_metrics_direction_button` and `pad_metrics_first_direction_button`). Step 9 keeps both conversion functions anyway (step 12 deletes them), so this hypothesis only matters for the signature change of `pad_jump_repeat_step`.
- **H-8-3. `tests/test_pad_metrics.c` still cross-checks against `pad_jump_repeat_step`.** `repeats_against_pad_jump` keeps its one call to `pad_jump_repeat_step(&jump, buttons[f], pressed)`; step 8 only changes the comparison on the next lines to `pad_metrics_direction_button(jumped)` and renames `circle_dead_other` → `button_6_dead_other`.
- **H-8-4. The `test_pad_metrics` and `test_pad_jump` link lines still lack the default unit.** Step 8's Makefile change adds `src/game/bindings.c` to the six tests that link `input_json.c` from an explicit list, and touches neither of these two lines.
- **H-8-5. The generator's outputs are byte-identical, apart from the renamed proof.** `make pad-author` regenerates `worst-case/` with no diff, `make pad-metrics-check` and `make pad-metrics-worst-case` are unchanged, and `make pad-author-proof` passes with `tools/pad_author_proofs/dead_circle.refused.pad` renamed `dead_back.refused.pad` and its `# expect:` line reading `dead jump.back: …`. Step 9's guardrail baseline is whatever the tree at step 9's own start produces.
- **H-8-6. The generator drives the game with the default table.** `pad_author_init` passes `&bindings_compiled` (the default unit) to `input_step_init`, and the high-level commands press the buttons `pad_nav_bindings()` gives. So once jump mode reads the table, the generator and the game agree, with or without a personal `bindings.cfg`.
- **H-8-7. Step 8 changes neither `input_step_init`, `pad_nav_init` nor `pad_nav_bindings`.**

**Rule if a hypothesis is wrong.** Follow the committed step 8 code at this step's own `HEAD`, use its names, and record the difference in the step note. In particular:

- if a file under `src/devtools/` still calls `pad_jump_repeat_step` (H-8-2), update the call mechanically with the default table it already holds (`pad_nav_bindings(...)` or `&bindings_compiled`) and the mode it simulates (`BINDINGS_MODE_JUMP` unless the code says slipway), and check the guardrail outputs stay byte-identical;
- if step 8 touched `pad_jump.{c,h}` or `pad_nav.c` (H-8-1), rebase on its version and keep its behaviour;
- if the cross-check in `test_pad_metrics.c` changed shape (H-8-3), change only its `pad_jump_repeat_step` call;
- if a link line already has `$(BINDINGS_DEFAULT_UNIT)` (H-8-4), do not add it twice;
- if a guardrail output differs at step 9's start from what step 8's PRD says (H-8-5), use the tree's output as the baseline: step 9 must not change it.

Step 8's one-off mutation proof (swap `jump.enter_aim`/`jump.open_menu` in `bindings.default.cfg`, expect `make pad-author` to fail) relied on the game not reading the table (H-7-6). After step 9 that proof would no longer fail at an aim entry; it is a recorded one-off, not something to rerun.

## User Stories

1. As a player, I want jump mode to enter aim mode on the button my bindings give `jump.enter_aim`, so that I can move aim mode to the button I prefer.
2. As a player, I want jump mode to open the hovered object's box on the button bound to `jump.open_menu`, so that the box opens where I expect.
3. As a player, I want the button bound to `jump.act_on_hovered` to click a known object, so that acting on what I hover follows my file.
4. As a player, I want the button bound to `jump.act_on_hovered` to start a slipway from a hovered colony, so that the same action keeps both of its meanings on its new button.
5. As a player, I want jump mode to jump up, down, left and right on the buttons bound to `jump.jump_*`, so that I can remap the jump directions.
6. As a player, I want the jump hold repeat to fire on whatever button plays `jump.jump_*`, so that a remapped direction still repeats after the same delay and period.
7. As a player, I want slipway mode's direction jumps to follow `slipway.jump_*`, so that slipway mode does not borrow jump mode's buttons.
8. As a player, I want a jump action left out of my file to do nothing in jump mode, so that what the file says is what the game does.
9. As a player, I want a button bound to an action of another mode to do nothing in jump mode, so that the modes stay independent.
10. As a player, I want the default bindings to play exactly like plan 12, so that nothing changes when I have no personal file.
11. As a maintainer, I want every e2e `.sha256` and end state unchanged, so that this step is a proven refactor.
12. As a maintainer, I want `make pad-author`, `make pad-author-proof`, `make pad-metrics-check` and `make pad-metrics-worst-case` unchanged, so that the devtools still agree with the game.
13. As a maintainer, I want the hold repeat to take the table and the mode it serves, so that jump and slipway mode each read their own actions through one helper.
14. As a maintainer, I want the hold repeat to refuse a mode without `jump_*` actions with an assert message, so that a wrong mode argument fails loudly in tests instead of silently doing nothing.
15. As a maintainer, I want one small helper in `pad_nav.c` that answers "is a button bound to this action of this mode pressed", so that steps 10–13 convert their reads the same way.
16. As a maintainer, I want the step-9 line gone from the button guard allowance and `pad_nav.c` to name exactly four fewer buttons, so that the guard proves the conversion and blocks a regression.
17. As a maintainer, I want `cross_starts_slipway` renamed after the action it serves, so that no PSX word sits on a converted line.
18. As a maintainer, I want the existing pad unit tests to change only mechanically (passing the default table and the mode), so that plan 12's behaviour stays pinned by the same expected values.
19. As a maintainer, I want a `pad_jump` unit test with a table that swaps the jump directions and leaves slipway unbound, so that the helper's mode argument is proven where it is cheapest.
20. As a maintainer, I want a `pad_nav` unit test where a table rotates jump mode's actions and leaves every other mode unbound, so that a read tagged with the wrong mode fails.
21. As a maintainer, I want that test to show the same direction buttons doing nothing in slipway mode, so that the shared hold repeat is proven to read `slipway`, not `jump`.
22. As a maintainer, I want that test to reach jump mode without pressing any button in another mode, so that steps 10–13 can unbind those modes without breaking it.
23. As a maintainer, I want a one-off mutation proof recorded in the step note, so that I know the new tests catch the wrong mode on the slipway call.
24. As a maintainer, I want `make test` and `make e2e` no slower, so that the fast loop stays fast.
25. As a reviewer, I want the step note to list which H-8-k held, so that I can see how the parallel step 8 landed against this one.

## Implementation Decisions

### The hold repeat takes the table and the mode

- New signature: `pad_direction_t pad_jump_repeat_step(pad_jump_t *jump, const bindings_t *bindings, bindings_mode_t mode, int pad_buttons, int pressed)`. `pad_jump.h` includes `game/bindings.h`. Step 2.5's rule: a helper shared by modes of different steps gets a mode argument in the earliest step.
- Inside `pad_jump.c`, a static function maps a direction to its `jump_*` action (UP → `BINDINGS_ACTION_JUMP_UP`, …; NONE → no action, mask 0) and returns `bindings_mask(bindings, mode, action)`.
- Behaviour, unchanged except for where the buttons come from:
  - The pressed direction is the first of UP, DOWN, LEFT, RIGHT (the order of `pad_jump_direction(i)`) whose mask meets `pressed`. It starts the repeat as today.
  - Otherwise the repeat continues while `pad_buttons` meets the mask of `repeat_direction` in the mode given now. An unbound direction (mask 0) never starts a repeat and ends one at once.
  - An action bound to several buttons: any of them presses it, any of them held keeps the repeat. The parser forbids one button on two actions of a mode, so two directions never share a button.
- Assert at entry: `bindings != NULL` and `bindings_mode_has_action(mode, BINDINGS_ACTION_JUMP_UP)`, with a message (`"pad_jump_repeat_step: a mode with jump_* actions (jump or slipway)"`). Today only jump and slipway have them.
- The repeat state stays a direction. Across a jump → slipway change with a direction held, the next frame checks the held button against slipway's mask. With the default the masks are equal, so frames are unchanged.
- `pad_jump_first_direction` and `pad_jump_direction_button` stay, untouched: `pad_nav_step`'s shared direction, `aim_pan` and tests still use them, and step 12 deletes them. `pad_jump_repeat_step` no longer calls them. `pad_jump.c`'s guard count (4, step 12) does not change.

### `jump_step` reads the table

- One static helper in `pad_nav.c`: `static int action_pressed(const pad_nav_t *nav, int pressed, bindings_mode_t mode, bindings_action_t action)`, returning `(pressed & bindings_mask(&nav->bindings, mode, action)) != 0`. Steps 10–13 reuse it with their modes; inside the module the field is read directly, the public `pad_nav_bindings()` stays for callers outside.
- The four reads of `jump_step`, same conditions and order as today:
  - `pressed & P8_PAD_BUTTON_7` → `action_pressed(nav, pressed, BINDINGS_MODE_JUMP, BINDINGS_ACTION_ENTER_AIM)`;
  - `pressed & P8_PAD_BUTTON_8` → `… BINDINGS_ACTION_OPEN_MENU`;
  - both `pressed & P8_PAD_BUTTON_5` → `… BINDINGS_ACTION_ACT_ON_HOVERED`, kept as two reads with their own conditions (known object click, then slipway start), as step 2.5's traps say.
- `jump_step` calls `pad_jump_repeat_step(&nav->jump, &nav->bindings, BINDINGS_MODE_JUMP, pad_buttons, pressed)`; `slipway_step` calls it with `BINDINGS_MODE_SLIPWAY` (inventory row 6, owned by step 9). Nothing else in `slipway_step` changes: its button-5 reads and `circle_returns` belong to steps 10 and 13.
- Renames on converted lines: `cross_starts_slipway` → `act_on_hovered_starts_slipway` (definition and its one call). No other PSX word sits on a converted line. `circle_returns` in `jump_step`'s and `slipway_step`'s signatures, `menu_cross_armed` in `click_hovered`, and `cross_engaged` in `reset_aim_mode` are on lines other steps convert (13, 12, 10), so they stay.
- No comment changes beyond what the rename needs. No new comment unless it carries a trap (`CLAUDE.md`).

### Guard allowance

- Delete the line `src/game/pad_nav.c step-9 4 | …` from `tools/button_guard_allowance.txt`, in the same commit that converts the four reads. At HEAD `src/game/` names 40 guarded buttons in 5 files (`pad_nav.c` 30, `pad_jump.c` 4, `bindings.c` 3, `cheat.c` 2, `pad_panel.c` 1; the one in `bindings.h` is in a comment), so `make button-guard` goes from `files 5 reads 40` to `files 5 reads 36`. If step 8 or another commit moved these numbers, what matters is that only the step-9 line goes and `pad_nav.c` names exactly 4 fewer.
- The commit that changes only `pad_jump_repeat_step` touches no guarded name, so the allowance stays valid at every commit.

### Makefile

- Add `$(BINDINGS_DEFAULT_UNIT)` to the link of `test_pad_jump` and `test_pad_metrics`, so their existing calls can pass `&bindings_compiled`. Nothing else changes; `make test` is not measurably slower (one already built object).

## Implementation Chunks

1. **The hold repeat takes the table and the mode.** New `pad_jump_repeat_step` signature and the direction-to-action mapping; `pad_nav.c`'s two calls pass `&nav->bindings` with `BINDINGS_MODE_JUMP` and `BINDINGS_MODE_SLIPWAY`; the Makefile links the default unit into `test_pad_jump` and `test_pad_metrics`; existing calls in `test_pad_jump.c` (and its `hold_frames` helper) and `test_pad_metrics.c` pass `&bindings_compiled, BINDINGS_MODE_JUMP` mechanically; new `test_pad_jump.c` tests (Testing Decisions). Verified: `make test-all`, every `.sha256` unchanged, the four guardrail commands unchanged.
2. **`jump_step` reads the table.** The `action_pressed` helper, the four reads, the rename, the allowance line removed; new `test_pad_nav.c` test. Verified: everything in Testing Decisions.

## Testing Decisions

- A good test drives the public interface (`pad_jump_repeat_step`; `input_step_run` frames through `pad_nav`) and asserts what the player sees: mode, hovered object, slipway start and target, box open. It never reads static state.
- **Existing tests change only mechanically** (section 2 guardrail): `test_pad_jump.c` and `test_pad_metrics.c` pass `&bindings_compiled` and `BINDINGS_MODE_JUMP` to `pad_jump_repeat_step`. No expected value changes. `test_pad_nav.c`'s existing tests do not change. An assertion that must change is a divergence: follow the code and record it in the step note.
- **New `test_pad_jump.c` tests** (prior art: the repeat tests already there). A table parsed with `bindings_parse` from inline text `jump.jump_up = 2`, `jump.jump_down = 1`, `jump.jump_left = 4`, `jump.jump_right = 3`, and nothing else:
  - in `BINDINGS_MODE_JUMP`, pressing button 2 gives UP, 1 gives DOWN, 4 gives LEFT, 3 gives RIGHT; pressing 3 + 4 gives LEFT (priority follows directions, not button numbers);
  - holding button 2 after pressing it repeats UP at `PAD_JUMP_REPEAT_DELAY` then every `PAD_JUMP_REPEAT_PERIOD`; holding button 1 (the default UP button) after pressing 2 ends the repeat on the next frame;
  - in `BINDINGS_MODE_SLIPWAY` (unbound there), pressing or holding any of buttons 1–4 gives NONE on every frame for at least `PAD_JUMP_REPEAT_DELAY + 2 * PAD_JUMP_REPEAT_PERIOD` frames.
- **New `test_pad_nav.c` test: jump mode follows a rotated table, slipway mode served by the same repeat stays unbound.**
  - Table, parsed from inline text, only jump lines: `jump.jump_up = 2`, `jump.jump_down = 1`, `jump.jump_left = 4`, `jump.jump_right = 3`, `jump.act_on_hovered = 7`, `jump.enter_aim = 8`, `jump.open_menu = 5`. Every converted read is off its default button, so a read left hardcoded fails. Every other mode is unbound. The rotation is needed because three actions move: any pairwise swap would leave one read on its default button.
  - Reaching jump mode presses no button in any other mode: a test helper starts the title with a given table (the existing `start_title` becomes that helper called with `&bindings_compiled`), then `cart_start_game` as `begin_game` does, `make_every_planet_known`, `settle_planet` on the planet closest to the centre, a few empty frames, and `assert_hovering` the colony. This keeps the test valid when steps 10–13 make the other modes read the table.
  - Each check below runs in its own `in_forked_copy`, so no check has to undo a box or a mode through another mode's button.
  - Button 8 (`enter_aim`): aim mode, hovered NULL.
  - Button 5 (`open_menu`): the colony's box opens (`pad_nav_panel_open`), no slipway starts.
  - Button 7 (`act_on_hovered` on a colony): slipway mode, `pad_nav_slipway_start` is the colony; jump mode would have entered aim mode on 7 with the default.
  - Directions: for each direction, tapping its swapped button hovers `expected_jump(colony, <that direction's default button>)` when that target is not NULL; the test asserts at least one direction has a target that differs from what the same button would reach as its default direction. Holding a swapped button for `PAD_JUMP_REPEAT_DELAY` frames repeats the jump once more in the bound direction, when a second target exists.
  - Neighbouring mode: after button 7 has started a slipway, the slipway mode is unbound. Assert at least one of buttons 1–4 has a `slipway_pick(colony, colony, button)` target (the default would move there), then tap each of buttons 1–4 and hold each for `PAD_JUMP_REPEAT_DELAY + PAD_JUMP_REPEAT_PERIOD` frames: `pad_nav_slipway_target` stays NULL and `pad_nav_slipway_start` stays the colony throughout. No button 5 or 6 is pressed in slipway mode (steps 10 and 13 own those).
  - Prior art: `test_jump_mode_seeds_on_the_colony_over_a_nearer_known_planet` (jump mode with no button), `test_the_pad_hides_cross_and_circle_exactly_from_the_chords_cheat_step_counts` (slipway from a colony), `test_init_hands_the_table_to_pad_nav` in `test_input_step.c` (an inline swap table).
- **One-off mutation proof, recorded in the step note, not committed:** pass `BINDINGS_MODE_JUMP` instead of `BINDINGS_MODE_SLIPWAY` in `slipway_step`'s repeat call; the new `test_pad_nav.c` test must fail on the neighbouring-mode check. Then leave one of the four reads hardcoded (for example `P8_PAD_BUTTON_7`); the test must fail and `make button-guard` must report `pad_nav.c names 27 button(s), listed 26`. Restore both.
- **Step verification**, in this order:
  1. `make test-all`: green, every `tests/e2e/*.json.sha256` unchanged; `make end-state` unchanged on every replay.
  2. Guardrails (section 2, this step touches `src/game/pad_*`): `make pad-author-proof`, then `make pad-author` followed by `git diff --exit-code worst-case/`, then `make pad-metrics-check` and `make pad-metrics-worst-case`, all unchanged from the tree at step 9's start (H-8-5).
  3. `make button-guard` passes with the step-9 line gone, and `git grep -c P8_PAD_BUTTON_ -- src/game/pad_nav.c` counts 4 names fewer than at the step's start (count names with `git grep -o`, since three lines name two buttons).
  4. `git grep -n 'cross_starts_slipway' -- src tests`: nothing.
  5. `make raylib` compiles and links (never run).
  6. Timings: `make test` and `make e2e`, three warm runs each, against step 1's baseline (4.27–4.58 s, 0.24–0.25 s).

## Out of Scope

- Aim and slipway modes' own reads (`aim.leave_aim`, `aim.probe_or_fold`, `slipway.build_on_target`) and `aim_pan` (step 10). Only the shared hold repeat's slipway call moves here.
- Build bar, placement, menu, panels, pause and title reads (steps 11–12), including the deletion of `pad_jump_first_direction` and `pad_jump_direction_button` and the shared pressed direction in `pad_nav_step` (step 12).
- The cross-mode reads that also serve jump mode: `jump.back` (`circle_returns`), `jump.pause`, `jump.open_build_bar`, `jump.open_tech_panel`, `jump.select_tap_or_year`, the message-box close and the chord mask (step 13). With a table that moves these, jump mode still answers plan 12's buttons for them until step 13.
- Renaming test functions or comments in `tests/` that carry PSX words (`test_square_in_jump_mode_…`): tests change only mechanically.
- Any change under `src/devtools/` (step 8), except the mechanical fallback of H-8-2.
- `pad_metrics.c`'s own hold-repeat simulation reading the table: it keeps its fixed D-pad mapping (step 8); fixtures with remapped directions are not planned in phase 4.
- Fixtures in `replay`, swap and unbound replays (steps 16–19).

## Further Notes

- **Trap: a direction is not a button.** UP and DOWN equal the bits of buttons 1 and 2, so a direction passed where a mask is expected only goes wrong on LEFT and RIGHT. The repeat compares `pad_buttons` with masks, never with `repeat_direction`.
- **Trap: the wrong mode passes with the default.** Jump and slipway both put `jump_*` on 1–4, so only the unbound-slipway test catches a repeat call tagged with the wrong mode. Keep the mutation proof.
- **Trap: the test must not cross modes through hardcoded reads.** Box closing (○), aim reads and the shoulders are converted by later steps; a check that relies on them would break when those steps unbind the other modes. Forked copies avoid every cleanup.
- **Trap: `act_on_hovered` is two reads.** The known-object click and the slipway start keep their own conditions and order; merging them would change which wins on a known colony.
- The step note (`### Step 2.9`) records which H-8-k held and how the code adapted where one did not, the converted reads by action, the mutation proof's two failures, the guard output, the guardrail runs and the timings.
