---
title: "Pad constants renamed to button numbers, directions as their own enum"
description: "The C code still names pad bits after PSX buttons and passes directions around as button bits, so the game thinks in 'cross' and a direction cannot be told apart from a button."
status: needs-triage
---

## Problem Statement

Plan 17 step 4 (Phase 1, abstract buttons). After steps 2 and 3, the replays and the generator's `.pad` scripts speak button numbers 1–12, but the C code still uses the PSX vocabulary: `src/p8/input.h` defines `P8_PAD_UP … P8_PAD_SELECT`, and about 900 uses across `src/` and `tests/` say `P8_PAD_CROSS`, `P8_PAD_CIRCLE`, `P8_PAD_SQUARE`. A maintainer reading the code thinks in a pad layout the plan is removing. Steps 5–14 must count and replace every button read in `src/game/`, and the guard of step 5 greps for `P8_PAD_BUTTON_`, so that name has to exist first.

A second problem hides in the same constants. A direction is also passed around as a button bit. `pad_target_pick`, `pad_panel_neighbor`, `pad_panel_step` and `pad_nav_neighbor` take `P8_PAD_UP` as "up". `pad_jump_first_direction` and `pad_jump_repeat_step` return a button bit as a direction, and `pad_jump_repeat_step` then masks the held buttons with that direction. Once bindings arrive, "move up" and "button 1" are no longer the same thing. As long as they share one value, the compiler and the reader cannot tell a direction from a button press.

## Solution

The twelve constants become `P8_PAD_BUTTON_1 … P8_PAD_BUTTON_12` with the same values (button N is bit `1 << (N-1)`: 1 up, 2 down, 3 left, 4 right, 5 cross, 6 circle, 7 square, 8 triangle, 9 L1, 10 R1, 11 start, 12 select). Every use in `src/`, `tests/` and the tools moves to the new name. No old name is kept as an alias.

Directions get their own enum, `pad_direction_t`: none, up, down, left, right. `pad_target`, `pad_jump`, `pad_panel` (and `pad_nav_neighbor`, which forwards to it) take and return this enum. Code that needs the button behind a direction, or the direction behind a set of buttons, says so through two named conversions in `pad_jump`. Behaviour does not change: every e2e `.sha256`, every end state, every `worst-case/` file and every generator output stays byte-identical.

## User Stories

1. As a maintainer, I want the pad bits named `P8_PAD_BUTTON_1` to `P8_PAD_BUTTON_12`, so that the C code uses the same numbers as the replays, the `.pad` scripts and plan 17's bindings table.
2. As a maintainer, I want each `P8_PAD_BUTTON_N` to keep the exact value of the constant it replaces, so that no frame, mask or recording changes.
3. As a maintainer, I want no `P8_PAD_UP … P8_PAD_SELECT` alias left in `src/p8/input.h`, so that the PSX vocabulary cannot creep back through an old name.
4. As a maintainer, I want a grep for the twelve old names across the repository to find nothing outside the archived plans and `pico8/`, so that I know the rename is complete.
5. As the implementer of step 5, I want every button read in `src/game/` to name `P8_PAD_BUTTON_`, so that my inventory is one grep and my guard has one pattern.
6. As a maintainer, I want directions typed as `pad_direction_t` (none, up, down, left, right), so that a direction can never be mistaken for a button bit.
7. As a maintainer, I want `pad_target_pick` to take a `pad_direction_t`, so that jump targeting is about directions, whatever button later plays them.
8. As a maintainer, I want `pad_panel_neighbor`, `pad_panel_step` and `pad_nav_neighbor` to take a `pad_direction_t`, so that panel navigation is about directions too.
9. As a maintainer, I want `pad_jump_first_direction` to return a `pad_direction_t` from a button mask, so that the one place that turns buttons into a direction is named.
10. As a maintainer, I want `pad_jump_direction(index)` to return a `pad_direction_t` in the order up, down, left, right, so that the priority order stays where it is.
11. As a maintainer, I want `pad_jump_repeat_step` to return a `pad_direction_t` and to check the held button through a named conversion, so that the hold-repeat no longer relies on a direction being a bit.
12. As a maintainer, I want one function that gives the button bit of a direction, so that every place that turns a direction back into a button (the hold-repeat, the aim pan, the generator's taps, the metrics' masks) is visible and step 9 can find it.
13. As a maintainer, I want that function to return 0 for "no direction", so that callers comparing against a button bit need no special case.
14. As a maintainer, I want the aim pan to keep reading its four held directions in the order up, down, left, right, so that the camera pans exactly as before.
15. As a maintainer, I want the build bar's left and right picks to compare the pressed direction against `PAD_DIRECTION_LEFT` and `PAD_DIRECTION_RIGHT`, so that the bar reads directions like every other mode.
16. As a maintainer, I want the generator's path search to keep storing directions and to tap the button of each direction, so that `make pad-author` produces the same frames.
17. As a maintainer, I want the generator's empty-cone refusal to print the same message as after step 3, so that `empty_cone.refused.pad` keeps passing unchanged.
18. As a maintainer, I want the metrics' `cone_empty` field and its hold `repeats` to stay button masks, so that the report keeps judging button presses and its text does not change.
19. As a maintainer, I want the PSX keymap table in `src/platform/keymap.c` to list `P8_PAD_BUTTON_1 … 12`, so that the platform layer uses the new names until step 15 replaces it.
20. As a maintainer, I want the existing pad unit tests to change only mechanically (old name to new name, button to direction where the argument or result is a direction), so that they keep pinning plan 12's behaviour.
21. As a maintainer, I want no expected value in an existing unit test to change meaning, so that the guardrail of plan 17 section 2 holds.
22. As a maintainer, I want one new unit test for the direction-to-button conversion, so that an off-by-one between direction and bit fails in `make test`.
23. As a maintainer, I want every `tests/e2e/*.json.sha256` unchanged after `make e2e`, so that the rename is proven pure on the replays.
24. As a maintainer, I want `make end-state`, `make pad-author-proof`, `make pad-author` with no diff in `worst-case/`, `make pad-metrics-check` and `make pad-metrics-worst-case` unchanged, so that the tools prove the rename on paths no replay covers.
25. As a maintainer, I want `make raylib` to still compile and link (when the vendor tree is set up), so that the platform layer's rename is checked by a compiler.
26. As a maintainer, I want `make test` and `make e2e` no slower than the step 1 baseline, so that the fast loop stays fast.
27. As a maintainer, I want comments and assert messages that name an old constant updated to the new name or to the direction, so that no message points at a symbol that no longer exists.
28. As the implementer of steps 9–13, I want local names (`cross_engaged`, `circle_returns`, `face`, …) left as they are, so that this step stays a rename of constants and my steps rename what they rewrite.

## Hypotheses

Step 3 ("Generator low level in numbers", PRD `docs/prd/triage/phase1-step3-generator-buttons-in-numbers.md`) is being implemented in parallel and is not committed. This PRD was written from `HEAD` 6048b1c only (step 2 committed, step 3 not). Each assumption about step 3's outcome is listed here. If one is false when this step starts, re-check the decisions that cite it.

- **H-3-1.** Step 3 is committed before this step starts, and its four generator commands (`make pad-author-proof`; `make pad-author` + `git diff --exit-code worst-case/`; `make pad-metrics-check`; `make pad-metrics-worst-case`) pass at its end. This step's "unchanged" checks compare against step 3's outputs, including its numbers-only `PAD_METRICS_PROOF_REQUIRED` and the numbers in the `tmp/` logs.
- **H-3-2.** Step 3 does not touch `src/p8/input.h`, the bit layout, or the signatures of `pad_jump_*`, `pad_target_pick`, `pad_panel_*`, `pad_nav_neighbor` and `pad_metrics_cone_empty`. They are as in 6048b1c.
- **H-3-3.** Step 3 deletes the twelve-name tables in `src/devtools/pad_author.c` and `src/devtools/pad_metrics.c` and the exported `pad_metrics_button_name`. If any survives, this step renames the constants it holds and does not delete it.
- **H-3-4.** Step 3's `tap`/`hold` parser turns a number N into `1 << (N-1)` arithmetically, without the `P8_PAD_*` constants. If it uses a table of constants, the rename covers that table like any other use.
- **H-3-5.** Step 3 leaves every high-level command's button use in `pad_author.c` (`tap(author, script, P8_PAD_CROSS)`, `pad_nav_neighbor(…, P8_PAD_DOWN)`, the pan's `P8_PAD_RIGHT : P8_PAD_LEFT`, the bar's left/right step, …) and every `P8_PAD_*` use in `pad_metrics.c` (`is_required`, the chord's `face`, the `section4` sums) in place. This step renames them all.
- **H-3-6.** Step 3's empty-cone refusal prints `empty cone: no candidate for button N of the hovered object in jump mode`, with N derived from the direction value returned by `pad_jump_first_direction(pressed)`, read as a button bit (bit index + 1). After this step that value is a `pad_direction_t`, so N is derived from its button bit (`pad_jump_direction_button`). The printed text stays identical. If step 3 derived N another way, keep whatever text it prints byte for byte.
- **H-3-7.** Step 3's mode log and report print button sets from button masks (pressed, held, `repeats`), never from a direction value. They are unaffected by the direction enum.
- **H-3-8.** Step 3 adds no new use of `P8_PAD_*` outside the files listed under Implementation Decisions. Whatever it adds is renamed by the same rule: the step starts with a fresh `git grep` of the old names on its own `HEAD`, not from the counts recorded here.

## Implementation Decisions

- **New constants.** `src/p8/input.h` defines `P8_PAD_BUTTON_1` (1) through `P8_PAD_BUTTON_12` (2048), with the values of the old `P8_PAD_UP … P8_PAD_SELECT` in the same order. The old twelve names are deleted, not aliased. `P8_MOUSE_LEFT` is not a pad constant and stays. No `P8_PAD_BUTTON_COUNT` is added: `input_json`'s local count and the keymap's own count stay as they are.
- **Rename rule.** Mechanical: `P8_PAD_UP`→`_1`, `DOWN`→`_2`, `LEFT`→`_3`, `RIGHT`→`_4`, `CROSS`→`_5`, `CIRCLE`→`_6`, `SQUARE`→`_7`, `TRIANGLE`→`_8`, `L1`→`_9`, `R1`→`_10`, `START`→`_11`, `SELECT`→`_12`, applied to every file of `src/`, `tests/` and `tools/` (substitute `L1`/`R1` with a word boundary so `P8_PAD_L1` does not touch anything longer). At 6048b1c the uses sit in `src/p8/input.h`, `src/game/{pad_nav,pad_panel,pad_target,pad_jump,cheat}.c`, `src/devtools/{pad_author,pad_metrics,input_json}.c`, `src/platform/keymap.{c,h}`, and `tests/test_{pad_nav,pad_metrics,pad_jump,input,pad_target,marker_coherence,recording_continuity,main_economy,keymap,input_step,replay_source,demo,economy}.c`. Then the direction sites below change from the button name to the direction enum.
- **Not renamed.** Archived plans (`docs/plan-12.md`, `docs/plan-17.md`) are history, and `pico8/` is the archive. Identifiers that contain a PSX word but are not the constants (`cross_engaged`, `pause_cross_armed`, `menu_cross_armed`, `circle_returns`, `circle_closes_the_box`, `face`, `select_down`, test helper names, the typedef `pad_button_count_ends_at_select`) stay: steps 9–13 rewrite those reads, and renaming locals is not part of a constant rename. Prose comments using PSX glyphs (`×`, `○`) stay too: they are step 21's (`rework-comments`) material. Comments and assert messages that name an old constant by its symbol are updated, since the symbol disappears.
- **The direction enum.** A small header of its own in `src/game/`, included by `pad_target.h`, `pad_jump.h` and `pad_panel.h`:

  ```c
  typedef enum {
      PAD_DIRECTION_NONE,
      PAD_DIRECTION_UP,
      PAD_DIRECTION_DOWN,
      PAD_DIRECTION_LEFT,
      PAD_DIRECTION_RIGHT
  } pad_direction_t;
  ```

  `PAD_DIRECTION_NONE` is 0, so the existing `direction != 0` / "0 for none" contracts keep their meaning. Code is written against the names (`!= PAD_DIRECTION_NONE`), not the numbers. The four values are not button bits on purpose: a leftover `buttons & direction` gives a wrong answer for left and right, which the `pad_jump` repeat tests (held left, held right) catch.
- **Interfaces that change type, not behaviour.**
  - `pad_target_pick(origin, pad_direction_t, candidates)`. Its `is_direction` check and `offset_in` switch move to the enum. The assert keeps its message with the direction names.
  - `pad_panel_neighbor(item, pad_direction_t)` and `pad_panel_step(focus, pad_buttons, pad_direction_t, …)`. Inside, `neighbor_score` switches on the enum. `pad_panel_step`'s `pad_buttons & P8_PAD_BUTTON_5` stays a button read.
  - `pad_nav_neighbor(item, pad_direction_t)` forwards unchanged.
  - `pad_jump_direction(index)` returns `pad_direction_t`, in the order up, down, left, right. `pad_jump_first_direction(buttons)` reads buttons 1–4 in that priority and returns the matching direction, or `PAD_DIRECTION_NONE`.
  - `pad_jump_t.repeat_direction` and `pad_jump_repeat_step`'s result become `pad_direction_t`.
  - `pad_metrics_cone_empty(metrics, from, pad_direction_t)`.
- **The one new function.** `pad_jump_direction_button(pad_direction_t)` in `pad_jump` returns the button bit that plays the direction today (up `P8_PAD_BUTTON_1` … right `P8_PAD_BUTTON_4`), and 0 for `PAD_DIRECTION_NONE` (a header contract line says so). It sits next to `pad_jump_first_direction`, its inverse, so the two conversions between buttons and directions live in one module. Step 9 later replaces its game-side uses with `jump.jump_*` bindings. Name chosen over a free-standing `pad_direction_*` module: it adds no `.c` file and keeps the header-only enum free of logic.
- **Sites that convert a direction back to a button**, all through `pad_jump_direction_button`:
  - `pad_jump_repeat_step`: the held check `pad_buttons & repeat_direction` becomes a check of the direction's button.
  - `pad_nav`'s `aim_pan`: `pad_buttons & pad_jump_direction(i)` becomes the button of direction i. Its assert on the index order is restated with `PAD_DIRECTION_*`.
  - `pad_author`: every `tap(author, script, directions[i])` from a path search (`pick_path`, `item_path`, the slipway aim) taps the button of the direction. `via[]` and `directions[]` hold `pad_direction_t`. The empty-cone message derives its button number from the direction's button (H-3-6).
  - `pad_metrics`: `pad_metrics_hold_step` keeps `repeats` a button mask (`|=` the direction's button). `empty_cones` keeps `cone_empty` a button mask (same). The `first_direction` handed to `is_required` stays a button bit: the call sites pass the button of `pad_jump_first_direction(…)`, so `is_required` compares bits as before.
- **Sites that compare a direction**: `pad_nav`'s `build_step` (`direction == PAD_DIRECTION_LEFT / RIGHT`), `pad_nav_step`'s computed `direction` (now `pad_direction_t`, passed on to `panel_step`, `build_step`, `map_step`, `bar_or_cycle_step`, whose parameters change type), `jump_from` and the slipway aim's `pad_target_pick` calls, and every `pad_nav_neighbor(…, P8_PAD_DOWN)` in `pad_author` (→ `PAD_DIRECTION_DOWN`).
- **Sites that stay buttons.** Everything else: the generator's direct taps (`tap(…, P8_PAD_DOWN)` to move a menu row becomes `P8_PAD_BUTTON_2`, not a direction), the aim pan's `bx`/`by` buttons in `pad_author`, the bar's `step` button, every face and shoulder read, `cheat.c`, `without_chord`, the metrics' `section4` masks, the keymap table, and the JSON loader's bit-range checks.
- **No game-state dump changes.** `repeat_direction` is not printed by any devtools dump at 6048b1c, so its new values change no `make state` output.

## Implementation Chunks

1. **Rename the constants.** Replace the twelve definitions in `src/p8/input.h` and every use, by the mechanical rule, in `src/`, `tests/` and `tools/` (a one-off substitution, described in the commit message, not committed). Update comments and assert messages that name an old symbol. Directions are still button bits at the end of this chunk: `pad_target_pick(&origin, P8_PAD_BUTTON_1, …)` is correct here. Verified: `make test-all` (every `.sha256` unchanged), a grep of the twelve old names finding nothing outside `docs/plan-*.md` and `pico8/`, `make raylib` if the vendor tree is present, and the four generator commands of plan 17 section 2.
2. **Directions as their own enum.** Add the header and `pad_jump_direction_button`, retype the interfaces above, convert the direction and back-to-button sites, and move the tests' direction arguments and results to `PAD_DIRECTION_*` (the mechanical test change below). Add the conversion test. Verified: `make test-all`, the four generator commands, `make end-state`, `make raylib` if available, and timings of `make test` and `make e2e`.

Each chunk leaves `make test-all` and the four generator commands green on its own.

## Testing Decisions

- A good test here checks behaviour through the public interface (which planet `pad_target_pick` returns, which direction `pad_jump_repeat_step` returns on which frame) and never how a function stores its state.
- **Existing tests change only mechanically** (plan 17 section 2 guardrail for step 4). Two kinds of edits are allowed, nothing else:
  - the name rename of chunk 1 (`P8_PAD_CROSS` → `P8_PAD_BUTTON_5`, …), in every test file;
  - in chunk 2, an argument or expected value that is a direction becomes `PAD_DIRECTION_*`, while held and pressed button masks stay `P8_PAD_BUTTON_*`. Concretely: `tests/test_pad_target.c`'s direction arguments, `tests/test_pad_nav.c`'s `pad_nav_neighbor` arguments and the `directions[4]` table of its breadth-first helper, `tests/test_pad_jump.c`'s results of `pad_jump_repeat_step`, `pad_jump_first_direction` and `pad_jump_direction` (for example `pad_jump_repeat_step(&jump, P8_PAD_BUTTON_4, P8_PAD_BUTTON_4) == PAD_DIRECTION_RIGHT`, and `pad_jump_first_direction(P8_PAD_BUTTON_5 | P8_PAD_BUTTON_7) == PAD_DIRECTION_NONE`). `tests/test_pad_metrics.c`'s `cone_empty` values stay button masks (`P8_PAD_BUTTON_1 | P8_PAD_BUTTON_4`).
  - If an assertion needs any other change to pass, the code diverges from plan 12's behaviour: follow the code, keep the e2e unchanged, and record it in the step note (section 2's rule). Not expected for a rename.
- **New test** in `tests/test_pad_jump.c` (prior art: `test_the_directions_by_index_are_up_down_left_right`): `pad_jump_direction_button` gives buttons 1, 2, 3, 4 for up, down, left, right and 0 for none, and is the inverse of `pad_jump_first_direction` on each single direction button. This catches an off-by-one between enum value and bit, the trap that a 1-based enum invites.
- **End to end**: `make e2e` (every `tests/e2e/*.json.sha256` byte-identical) and `make end-state` on every replay. The generator commands of section 2 cover the paths no replay presses: `make pad-author-proof`, `make pad-author` + `git diff --exit-code worst-case/`, `make pad-metrics-check`, `make pad-metrics-worst-case`, all unchanged from step 3's outputs (H-3-1).
- **Completeness**: a `git grep -nwE 'P8_PAD_(UP|DOWN|LEFT|RIGHT|CROSS|CIRCLE|SQUARE|TRIANGLE|L1|R1|START|SELECT)'` finds nothing outside `docs/plan-*.md` and `pico8/`. This is a one-time check recorded in the step note, not a `make test` guard: the guard of step 5 is the lasting one.
- **Timing**: `make test` and `make e2e` against step 1's baseline (`make test` 4.27–4.58 s, `make e2e` 0.24–0.25 s on a warm build). The step adds one test function and no binary.

## Out of Scope

- The guard on `P8_PAD_BUTTON_` in `src/game/` and the inventory of button reads: step 5.
- Replacing any button read by `bindings_mask`, and the bindings module: steps 6–13. `pad_jump_direction_button`'s game-side uses stay until step 9.
- Renaming locals, struct fields, functions or test helpers that carry a PSX word (`cross_engaged`, `circle_returns`, `pause_cross_armed`, …), and rewording prose comments with `×`/`○`: steps 9–13 and step 21.
- The generator's high-level messages that name PSX buttons (`dead circle: …`, `menu: closed by circle`): step 8.
- The keymap's PSX name table and `keymap_button_name`: step 15. Only its `P8_PAD_*` constants are renamed here.
- The `section4` labels of the pad-metrics report.
- Any change to frames, game behaviour, e2e baselines, replay files or `.pad` scripts.

## Further Notes

- Plan 17 section 2 guardrail: this step touches `src/game/pad_*`, `src/devtools/pad_*`, `src/devtools/input_json.c` and the button constants, so the four generator commands are mandatory after each chunk.
- The step note should record which hypotheses held, the grep proving no old name is left, whether `make raylib` was run, and the timings.
- Trap for a later step: after chunk 2, `PAD_DIRECTION_UP` and `PAD_DIRECTION_DOWN` happen to equal the bits of buttons 1 and 2, so a direction misused as a mask goes unnoticed on up and down. Left and right expose it, which is why the repeat and conversion tests cover them.
