---
title: "Generator high level reads the default bindings"
description: "The .pad generator's high-level commands still press hardcoded buttons and the devtools still borrow pad_jump's direction helpers, their own button counts and their own digit loops, so the control scheme has a second, silent copy in src/devtools/."
status: needs-triage
---

## Problem Statement

Plan 17 step 8 (Phase 2, bindings in the game). The step's text, from `docs/plan-17.md` section 3:

> 8. **Generator high level reads the default bindings.** Each high-level command in `src/devtools/pad_author.c` (`colonise`, `probe`, `slipway`, …) takes its button from the compiled default table for the mode it runs in. `tap` and `hold` stay raw numbers. Verified like step 3: `make pad-author-proof`, then `make pad-author` followed by `git diff --exit-code worst-case/`, `make pad-metrics-check` and `make pad-metrics-worst-case`, all repeated with a swapped personal `bindings.cfg` present, and `pad_author.c` names no `P8_PAD_BUTTON_` outside the low-level path. The direction taps of the high-level commands also take their buttons from the default table (`jump.jump_*`, `aim.pan_*`, …), and `pad_metrics.c` gets a devtools-side direction-to-button mapping (or reads the default table) in place of `pad_jump_first_direction`/`pad_jump_direction_button`, including its inline D-pad mask; its `first_direction` parameter that holds a button bit is renamed. `pad_author.c`'s `parse_buttons` and `input_json.c` switch to step 6's button-number reader and count. The PSX words step 1.3's note deferred go in this step: the messages `dead circle`, `square: …` and `closed by circle` name the action or button number, and the locals `settle_circle`, `circle_before`, `circle_pending`, `next_circle_required` and the field `circle_dead_other` are renamed. The `# expect:` line of `dead_circle.refused.pad` (and its file name, if wanted) changes with them: the only allowed change in the generator commands' output. Also verified: no call to `pad_jump_direction_button` or `pad_jump_first_direction` is left in `src/devtools/`, and no other `*_BUTTON_COUNT 12` or second button-number digit loop exists there.

Today (HEAD 7340154) the generator `src/devtools/pad_author.c` is a second copy of the control scheme. Every high-level command (`colonise`, `probe`, `fold`, `slipway`, `build`, `research`, `score`, `year`, `menu`, `bar`, `attention`, `back`, `pause`, `restart`, `again`, `title`/`new-game`, `jump`) presses a hardcoded `P8_PAD_BUTTON_N`: about 35 names. Once steps 9–13 make the game read its bindings, a generator that still thinks "square enters aim" would be a hidden control rule outside the bindings file, which section 1's third goal forbids.

The devtools also lean on game code and duplicate step 6's work:

- `pad_author.c` and `pad_metrics.c` call `pad_jump_first_direction` and `pad_jump_direction_button`, the game-side conversions that step 12 deletes. `pad_metrics.c` also writes the D-pad as an inline `P8_PAD_BUTTON_1 | … | P8_PAD_BUTTON_4` mask, and `is_required`/`open_event` carry a parameter named `first_direction` that holds a button bit, not a direction.
- `PAD_METRICS_BUTTON_COUNT 12` (in `pad_metrics.h`) and `PAD_BUTTON_COUNT 12` (in `input_json.c`, with its own compile check) duplicate `P8_PAD_BUTTON_COUNT`. `pad_author.c`'s `parse_buttons` and `input_json.c`'s `parse_pad_frame` each hand-roll the button-number digit loop that step 6 put in `bindings_read_button_number`.
- PSX words survive in the generator: the refusal `dead circle: …`, the failures `square: no aim mode`, `square: a known object to jump back to`, `square: no jump mode after aim mode`, `menu: closed by circle`, `bar: closed by circle`, `title: the cross at …`, the comment `□ in aim mode`, the function `settle_circle`, the macro `CIRCLE_EFFECT_FRAMES`, the `pad_author_t` fields `circle_pending`, `circle_required`, `next_circle_required`, `circle_before`, and `pad_metrics_section4_t.circle_dead_other`.

## Solution

- Every high-level command presses the button that the default bindings table gives to the action it plays, in the mode the generator is in at that frame. Direction taps, pans, the build bar's picks and the Select hold all go through the table. `tap` and `hold` keep their raw button numbers.
- The generator's frame-level checks that read a role, not a raw button, follow the table too: the jump-mode empty-cone refusal reads the `jump.jump_*` buttons, and the dead-press detector watches the mode's back action (`jump.back`, `aim.back`, `slipway.cancel_slipway`, `menu.close_menu`, …) instead of button 6.
- `pad_metrics.c` gets its own fixed D-pad mapping (buttons 1–4 are up, down, left, right) and stops calling `pad_jump`'s conversions.
- `input_json.c` and `pad_author.c` read button numbers with `bindings_read_button_number` and count them with `P8_PAD_BUTTON_COUNT`. The local counts go.
- The PSX words go: messages name the `mode.action` they are about, and the locals and fields carry the action's name. `tools/pad_author_proofs/dead_circle.refused.pad` becomes `dead_back.refused.pad`, and its `# expect:` line moves with the message.

With the default table every pressed frame is the same as today. `worst-case/*.json`, every `tests/e2e/*.json` the frames proofs regenerate, every end state, the pad-metrics reports and the console output of every guardrail command stay byte-identical. The one exception is the refusal text that `dead_back.refused.pad` expects.

## Hypotheses on step 7's outcome

Step 7 ("Compile-time bindings", PRD `docs/prd/triage/phase2-step7-compile-time-bindings.md`) is being implemented in parallel and is not committed. This PRD was written from HEAD 7340154 (step 6 complete) and from step 7's PRD. Each assumption about step 7 is a hypothesis:

- **H-7-1. The compiled default table.** `src/game/bindings.h` declares `extern const bindings_t bindings_compiled;`. The generated unit `tmp/bindings/default.c`, built from `bindings.default.cfg` by `tmp/bindings_generate`, defines it. That unit is in `PAD_AUTHOR_SRCS` and `REPLAY_SRCS`, and in the link of every test that calls `input_step_init`.
- **H-7-2. The table reaches the generator through `pad_nav`.** `input_step_init(step, bindings)` takes a `const bindings_t *`, and `pad_author_init` passes `&bindings_compiled`. `pad_nav_init` copies the table by value into `pad_nav_t`. `const bindings_t *pad_nav_bindings(const pad_nav_t *nav)` returns that copy: it lives as long as `nav`, and the caller never frees it.
- **H-7-3. The module is in the engine.** `src/game/bindings.c` is in `ENGINE_SRCS`, so `PAD_AUTHOR_SRCS`, `REPLAY_SRCS` and `test_pad_metrics` (which links `$(ENGINE_SRCS)`) all link it. Its two temporary lines in `tools/float_grep_exclusions.txt` are gone.
- **H-7-4. Only `game` compiles a personal file.** With a valid personal `bindings.cfg` at the root, only `tmp/game` links `tmp/bindings/game.c` built from it. `pad_author`, `replay` and every test link the default unit. So a swapped personal file changes nothing in `make test-all`, `make pad-author`, `make pad-author-proof` or the pad-metrics targets.
- **H-7-5. The default file is section 2's reconciled table.** `bindings.default.cfg` binds every action the generator uses to plan 12's button: `title.move_down 2`, `title.choose_row 5`; `jump.jump_* 1–4`, `jump.act_on_hovered 5`, `jump.back 6`, `jump.enter_aim 7`, `jump.open_menu 8`, `jump.open_build_bar 9`, `jump.open_tech_panel 10`, `jump.pause 11`, `jump.select_tap_or_year 12`; `aim.pan_* 1–4`, `aim.probe_or_fold 5`, `aim.back 6`, `aim.leave_aim 7`, `aim.open_build_bar 9`, `aim.open_tech_panel 10`, `aim.pause 11`, `aim.select_tap_or_year 12`; `slipway.jump_* 1–4`, `slipway.build_on_target 5`, `slipway.cancel_slipway 6`; `menu.move_* 1–4`, `menu.choose_row 5`, `menu.close_menu 6`; `pause.choose_row 5`, `pause.close_pause 6, 11`; `build_bar.pick_left 3`, `build_bar.pick_right 4`, `build_bar.take_structure 5`, `build_bar.close_bar 6`; `placement.pan_* 1–4`, `placement.place_structure 5`; `panels.move_* 1–4`, `panels.choose_row 5`, `panels.close_panel 6`, `panels.next_panel 10`. Step 7's `test_bindings` proves it.
- **H-7-6. `pad_nav` does not read the table yet.** Step 7 hands the table over but changes no button read in `src/game/` (steps 9–13 do). This step's one-off mutation proof (Testing Decisions) relies on it.
- **H-7-7. Step 7 leaves these files alone:** `src/devtools/pad_metrics.{c,h}` and `src/devtools/input_json.c`, the link lines of the six tests that build `input_json.c` without `$(ENGINE_SRCS)`, and `src/devtools/pad_author.c` except the `input_step_init` call in `pad_author_init`.

**Rule if a hypothesis is wrong.** Follow the committed step 7 code at this step's own `HEAD`. Use its names and signatures, and record the difference in the step note. In particular:

- if there is no `pad_nav_bindings` accessor, the generator reads `bindings_compiled` directly (H-7-2);
- if the symbol has another name, use that name (H-7-1);
- if `pad_author` does not link a default table, link the default unit into `PAD_AUTHOR_SRCS`, never the chosen personal one (H-7-4);
- if `bindings.c` is not in `ENGINE_SRCS`, add it to the link of `pad_author`, `replay` and `test_pad_metrics` explicitly (H-7-3).

Chunks 1 and 2 below do not depend on step 7. Only chunk 3 does.

## Facts checked at HEAD 7340154

- Step 6 is complete. `bindings_mode_t` has 9 modes (`TITLE, JUMP, AIM, SLIPWAY, MENU, PAUSE, BUILD_BAR, PLACEMENT, PANELS`) and `bindings_action_t` has 35 actions. Its API: `bindings_mask` (0 for an unbound action or one the mode lacks), `bindings_mode_has_action`, `bindings_mode_name` and `bindings_action_name` (the file spellings), and `int bindings_read_button_number(const char *text, const char **end)`, which returns 1–12 or 0, sets `*end` past the digits on success and restores it on failure. `P8_PAD_BUTTON_COUNT` is in `src/p8/input.h` with a compile check. `bindings.c` includes only `game/bindings.h`, `p8/input.h` and libc.
- `pad_metrics_mode_t` has 10 values (`TITLE, PAUSE, PLACEMENT, BAR, TECH, SCORE, MENU, JUMP, AIM, SLIPWAY`). `pad_metrics_classify` checks them in that order.
- `input_json.c` is in `ENGINE_SRCS` but not in `CORE_SRCS`, and it is a float-grep host tool. Besides the `$(ENGINE_SRCS)` builds, six tests link it from an explicit list without `bindings.c`: `test_input`, `test_replay_source`, `test_recording_continuity`, `test_marker_coherence`, `test_headless_capture` and `test_demo`.
- The button guard (`tools/button_guard.sh`) only scans `src/game/`. Nothing in this step touches its allowance.
- `tests/test_pad_metrics.c` calls `pad_jump_direction_button` once, in the hold-repeat cross-check against `pad_jump_repeat_step`, and reads `circle_dead_other` twice.
- `tests/test_input.c` already refuses `[13]`, `[05]`, `["cross"]` and `[4, "cross"]`. The refusal proofs `button_name.refused.pad` (`tap cross`) and `button_range.refused.pad` (`hold 13 4`) cover `parse_buttons`.
- The scripts call `pause` only from jump mode (after `title …` or after a `menu` that closed on the map), `restart` only on the pause menu, and `again` only on the final score (pad_metrics mode `menu`). `back` runs in jump mode (`dead_circle.refused.pad`) and in aim mode (`pad_probe.frames.pad`, `back --required`).

## User Stories

1. As a maintainer, I want every high-level `.pad` command to press the button the default bindings give its action, so that the generator holds no second copy of the control scheme.
2. As a maintainer, I want each press to look up the mode the generator is in at that frame, so that a command crossing modes (jump → menu → map) presses each mode's own button.
3. As a maintainer, I want `tap` and `hold` to keep raw button numbers, so that the escape hatches and the replays with a fixture (section 2) can still write any frame.
4. As a maintainer, I want the D-pad taps of `jump`, `slipway` and the menu and panel walks to come from `jump.jump_*`, `slipway.jump_*`, `menu.move_*` and `panels.move_*`, so that directions are actions like every other press.
5. As a maintainer, I want `pan_to` to press `aim.pan_*` in aim mode and `placement.pan_*` while carrying, so that the pan reads the mode it runs in.
6. As a maintainer, I want `build` and `bar` to step the bar with `build_bar.pick_left` and `build_bar.pick_right`, so that the bar picks follow the table.
7. As a maintainer, I want the Select hold of `year` and the Select tap of `attention` to press `select_tap_or_year` of the map mode, so that the year command follows the table.
8. As a maintainer, I want a hold to take its button once, from the mode at its first frame, so that a held button never changes in the middle of a hold.
9. As a maintainer, I want a command whose action is unbound, or that the current mode does not have, to fail with `<mode>.<action>` in the message, so that a broken default file shows up as a clear generator error instead of a silent empty frame.
10. As a maintainer, I want a command to press only the lowest-numbered button of an action bound to several buttons, so that `pause.close_pause = 6, 11` never becomes a two-button chord.
11. As a maintainer, I want the jump-mode empty-cone refusal to find the pressed direction through `jump.jump_*`, so that the refusal judges the jump the game would play.
12. As a maintainer, I want the dead-press detector to watch the mode's back action instead of button 6, so that the `back --required` rule keeps meaning "a back that changes nothing".
13. As a script author, I want the dead-back refusal to name the action it judged (`dead jump.back: …`), so that I know which press to mark `back --required`.
14. As a script author, I want the aim failures to say `jump.enter_aim: no aim mode` and `aim.leave_aim: …`, so that the message names the action, not a PSX button.
15. As a script author, I want `menu` and `bar` to report `closed by menu.close_menu` and `closed by build_bar.close_bar`, so that no failure speaks of a circle.
16. As a script author, I want the title failures to name `title.choose_row` and `title.move_down`, so that the title command speaks the bindings' words.
17. As a maintainer, I want `dead_circle.refused.pad` renamed `dead_back.refused.pad` with the new `# expect:` line, so that the proof's name matches what it proves.
18. As a maintainer, I want that refusal text to be the only output of the generator commands that changes, so that the step is provably a refactor everywhere else.
19. As a maintainer, I want `pad_metrics.c` to map directions to buttons 1–4 itself, so that step 12 can delete `pad_jump_direction_button` and `pad_jump_first_direction` without touching the devtools.
20. As a maintainer, I want `pad_metrics.c`'s D-pad mask derived from that mapping, so that no inline list of four buttons is left in the report code.
21. As a maintainer, I want the parameter that holds the first pressed direction's button bit named for what it holds, so that nobody passes a `pad_direction_t` into it again (UP and DOWN equal the bits of buttons 1 and 2, the trap of step 1.4's note).
22. As a maintainer, I want `pad_metrics_section4_t.circle_dead_other` renamed after button 6, so that the field says what it counts.
23. As a maintainer, I want the pad-metrics report lines byte-identical, so that `make pad-metrics-check`, `make pad-metrics-worst-case` and `PAD_METRICS_PROOF_REQUIRED` stay unchanged.
24. As a maintainer, I want `input_json.c` to read numbers with `bindings_read_button_number` and count with `P8_PAD_BUTTON_COUNT`, so that a replay and a bindings file share one grammar.
25. As a maintainer, I want `pad_author.c`'s `parse_buttons` on the same reader, so that `tap 05`, `tap 13` and `tap cross` are refused by the same rule as everywhere else.
26. As a maintainer, I want `PAD_METRICS_BUTTON_COUNT` and `input_json.c`'s `PAD_BUTTON_COUNT` deleted, so that the button count lives in one place.
27. As a maintainer, I want the generator's locals and fields named after the back action (`settle_back`, `back_pending`, `back_required`, `next_back_required`, `back_before`, `BACK_EFFECT_FRAMES`), so that step 14's PSX-word grep has nothing to find in the generator's model of the game either.
28. As a maintainer, I want the generator to read the table it handed to the input step, so that the generator presses exactly what the game it drives was given.
29. As a maintainer, I want a swapped personal `bindings.cfg` to change nothing in the generator, the proofs or the metrics, so that a player's file never touches the replays.
30. As a reviewer, I want a recorded one-off proof that a changed default table changes what the generator presses, so that "reads the table" is shown, not assumed.
31. As a reviewer, I want greps proving that `pad_author.c` names `P8_PAD_BUTTON_` only on the low-level path and that `src/devtools/` has no `pad_jump` direction conversion, second count or second digit loop, so that the step's static claims are checked.
32. As the implementer of step 12, I want no devtools caller of the game-side direction conversions left, so that step 12 deletes them with a game-only diff.
33. As the implementer of step 15, I want the reader already shared by three callers, so that the keymap follows the same pattern.
34. As the implementer of steps 18 and 19, I want the generator's high level to stay default-bound, so that a replay with a fixture is written at low level as section 2 says.
35. As a maintainer, I want `make test` and `make e2e` no slower than step 1's baseline, so that the fast loop stays fast.
36. As a maintainer, I want every `.sha256`, end state and `worst-case/` file unchanged, so that the e2e replays stay the source of truth.
37. As a reader of the Makefile legend, I want the `make pad-author` entry to describe the dead-back refusal as "a back action that changes nothing", so that the legend matches the new rule.

## Implementation Decisions

### Shared button reader and count in the devtools

- **`input_json.c`.** `parse_pad_frame` calls `bindings_read_button_number(p, &end)`. A 0 return refuses the frame. The element must then end at a JSON separator or `]` (checked on `*end`), and a repeated button is still refused. The grammar does not change: digits only, 1–12, no leading zero, nothing glued on. Delete the local `PAD_BUTTON_COUNT` and its compile check, because `p8/input.h` already has one. `format_pad_frame_line` and its assert use `P8_PAD_BUTTON_COUNT`. The file includes `game/bindings.h`.
- **Makefile.** The six tests that link `input_json.c` from an explicit list add `src/game/bindings.c` to it. That module needs only `p8/input.h` and libc. No other link line changes, because every `$(ENGINE_SRCS)` build already has the module (H-7-3).
- **`pad_author.c` `parse_buttons`.** Loop over `+`-separated elements: read each with `bindings_read_button_number`, refuse 0, refuse a repeated bit, then accept `\0` or `+`. Same grammar, same `unknown buttons '<token>'` message.
- **Counts.** Delete `PAD_METRICS_BUTTON_COUNT` from `pad_metrics.h`. Every array dimension and loop in `pad_metrics.{c,h}` and `pad_author.c` (`first_button_number`, `write_buttons`) uses `P8_PAD_BUTTON_COUNT`. After this, `git grep -nE '_BUTTON_COUNT +12' -- src/devtools` is empty. A digit loop grep (`'9'`) over `src/devtools` finds only `replay_source.c`'s hex digit reader, which reads no button.

### `pad_metrics`: a fixed D-pad of its own

- **Decision: a fixed devtools-side mapping, not the table.** `pad_metrics` judges buttons, not actions. Its reports and arrays are per button number, `is_required` names buttons 6 and 7, and the section 4 group is "dead D-pad presses in jump mode". `replay` links it, and from step 16 `replay` may run a fixture while the compiled default says something else. Reading `bindings_compiled` would tie the report to a table the replay may not be playing. A pointer to the step's table would also mean a new init argument for every caller and test. A fixed physical D-pad keeps `pad_metrics` a button-level tool, as it is today. (The plan allows either option.)
- **Interface**, in `pad_metrics.h`:
  - `int pad_metrics_direction_button(pad_direction_t direction)`: UP → button 1's bit, DOWN → 2, LEFT → 3, RIGHT → 4, NONE → 0.
  - `int pad_metrics_first_direction_button(int buttons)`: the bit of the first of UP, DOWN, LEFT, RIGHT whose button is in `buttons` (the order of `pad_jump_direction(i)`), or 0.
  - Both are pure. A one-line contract comment says that buttons 1–4 are the D-pad of section 2's numbering and are independent of any bindings table.
- Inside `pad_metrics.c`, `pad_metrics_hold_step`, `empty_cones` and `judge_frame` call these functions instead of `pad_jump_direction_button`/`pad_jump_first_direction`. `pad_jump_direction(i)` (index to direction) and the repeat constants stay: they are not the conversions step 12 deletes.
- `is_required(…, int first_direction)` and `open_event(…, int first_direction)` rename the parameter to `first_direction_button`.
- `pad_metrics_section4` builds its D-pad mask as the OR of `pad_metrics_direction_button` over the four directions, not as a literal list.
- `pad_metrics_section4_t.circle_dead_other` becomes `button_6_dead_other`. The report line `section4 dead circle presses in the other group …` keeps its exact text: the guardrails require byte-identical metrics output, and that text is not one of the generator messages the step lists. A later step may reword it together with the report format.
- `pad_metrics.c` keeps `P8_PAD_BUTTON_6`, `P8_PAD_BUTTON_7` in `is_required`/`settle`/`section4` and the fixed 5 + 6 chord. Those are button-level rules, outside this step's text.

### `pad_author`: high-level presses through the table

- **Which table.** The generator reads `pad_nav_bindings(&author->step.pad_nav)`, the copy of `&bindings_compiled` that `pad_author_init` handed to `input_step_init` (H-7-2). It is one table: the default, because `pad_author` never links a personal file (H-7-4). Reading it through the nav means the generator presses what the game it drives was given. If the accessor does not exist, read `bindings_compiled` directly.
- **Mode mapping** (static in `pad_author.c`). Convert the `pad_metrics_mode_t` that `current_mode` gives into a `bindings_mode_t`: `title → TITLE`, `pause → PAUSE`, `placement → PLACEMENT`, `bar → BUILD_BAR`, `tech → PANELS`, `score → PANELS`, `menu → MENU`, `jump → JUMP`, `aim → AIM`, `slipway → SLIPWAY`. It reads the mode before the frame is run, as `emit` already does. It is the generator's view of the mode. Step 5's note gives the game's own rule for `panel_step` (for example, a panel over the build bar is `menu`). No generator command presses in that case. If steps 9–13 find a difference, the generator follows the game then.
- **Direction to action** (static). It is a function of the bindings mode and a `pad_direction_t`:
  - `JUMP`, `SLIPWAY`: `jump_up/down/left/right`;
  - `AIM`, `PLACEMENT`: `pan_*`;
  - `TITLE`, `MENU`, `PAUSE`, `PANELS`: `move_*`;
  - `BUILD_BAR`: `pick_left`/`pick_right` for LEFT/RIGHT, and nothing for UP/DOWN.
- **Back action** (static). It is a function of the bindings mode: `jump.back`, `aim.back`, `slipway.cancel_slipway`, `menu.close_menu`, `pause.close_pause`, `build_bar.close_bar`, `placement.cancel_placement`, `panels.close_panel`, and nothing for `title`. These are the map modes' back actions of section 2's message-box rule, plus each box mode's close.
- **One lookup helper.** It takes the action (or the direction, or "the back action") and the current mode, and gives the button bits to press. If the mode lacks the action, it fails with `mode <mode> has no <action> action`. If the action is unbound, it fails with `<mode>.<action> is unbound`. Otherwise it returns the **lowest-numbered button** of the mask. Every converted call site uses it through small wrappers (press-and-release, emit-held-for-N-frames, emit-once), so each line reads as the action it plays. The exact helper names are the implementer's choice, following `module_verb` and the 40-line rule for short names.
- **Timing of the lookup.** Look up right before the frame that presses, so a command crossing modes (for example `score`: `jump.open_tech_panel`, then `panels.next_panel` twice) gets each mode's button. A hold (`emit_repeated` of `fold` and `year`) looks up once, at its first frame.
- **Call sites converted.** Each one is listed with the generator's mode at the press:
  - `run_title`: `title.move_down` per difficulty row, then `title.choose_row`.
  - `run_jump`: each path tap is `jump.<direction>`.
  - `aim_slipway_at`: each path tap is `slipway.<direction>`.
  - `move_to_item`, `move_down_rows`: the current mode's direction action (`menu.move_*` in `colonise` and `menu`, `panels.move_*` in `research`).
  - `pan_to`: the current mode's `pan_*` for x and for y, ORed on the diagonal frames as today (aim in `probe`/`fold`, placement in `build`).
  - `enter_aim`: `jump.enter_aim`. `leave_aim`: `aim.leave_aim`.
  - `run_probe`: `aim.probe_or_fold` tapped. `run_fold`: `aim.probe_or_fold` held `FOLD_HOLD_FRAMES`, then `aim.back` (still only when a candidate exists), then a release.
  - `run_colonise`: `jump.act_on_hovered`, the menu walk, then `menu.choose_row`.
  - `run_slipway`: `jump.act_on_hovered` to start, `slipway.build_on_target` per target, `slipway.cancel_slipway` (at most 3 taps while in slipway mode).
  - `run_build`: the map mode's `open_build_bar`, `build_bar.pick_left`/`pick_right`, `build_bar.take_structure`, the placement pan, `placement.place_structure`.
  - `run_research`: the map mode's `open_tech_panel`, `panels.move_*`, `panels.choose_row`, then `panels.close_panel` if still on the tech panel.
  - `run_score`: the map mode's `open_tech_panel`, then `panels.next_panel` twice.
  - `run_year`: the map mode's `select_tap_or_year` held `PAD_SELECT_HOLD_FRAMES`. `run_attention`: the same action tapped.
  - `run_menu`: `jump.open_menu`, the walk, `menu.close_menu`. `run_bar`: the map mode's `open_build_bar`, `build_bar.pick_left`, `build_bar.close_bar`.
  - `run_back`: the current mode's back action. In `title` it fails with the "has no back action" message: no script does this today, and before this step it would only have hit the dead-press refusal.
  - `run_pause`: the current mode's `pause`. The scripts only pause from jump. A pause from `menu` or `panels` now fails with the "has no pause action" message instead of pressing a button those modes do not bind.
  - `run_leave_game`: `pause.choose_row` for `restart`, `menu.choose_row` for `again`.
- **`emit`'s frame checks.** `emit` runs for every frame, raw `tap`/`hold` included:
  - **Empty cone.** In jump mode, the pressed direction is the first of UP, DOWN, LEFT, RIGHT whose `jump.jump_<direction>` mask meets `pressed`. The refusal names `first_button_number(pressed & that mask)`, which is still `button 1` in `empty_cone.refused.pad`. It no longer calls `pad_jump_first_direction`/`pad_jump_direction_button`.
  - **Dead back.** The detector triggers when `pressed` meets the mask of the back action of the mode before the frame. This matches today on every frame the scripts press: 6 is every back action's lowest button, and title had no scripted 6. It widens to button 11 on the pause menu (`pause.close_pause = 6, 11`). No script presses 11 there, and the proofs show it. The pending state records the `mode.action` it judged, for the message.
  - **Chord.** The 5 + 6 chord refusal keeps its fixed `P8_PAD_BUTTON_5 | P8_PAD_BUTTON_6` mask. Section 2 fixes the chord below the bindings, and it applies to raw frames. Together with `tap`/`hold`, it is the "low-level path" of the step's text. After the step it is the only `P8_PAD_BUTTON_` line in `pad_author.c` (2 names). Step 13 moves the chord mask behind the cheat module for `pad_nav.c`. Reusing it here is left to that step.
- **Renames and messages**, the PSX words of step 1.3's note plus the same family on the lines this step rewrites:
  - `settle_circle` → `settle_back`, `CIRCLE_EFFECT_FRAMES` → `BACK_EFFECT_FRAMES`.
  - `pad_author_t` fields: `circle_pending` → `back_pending`, `circle_required` → `back_required`, `next_circle_required` → `next_back_required`, `circle_before` → `back_before`. Add one field that remembers which `mode.action` is pending, for the message: an enum pair or the two names, the implementer's choice, with no pointer into a script line.
  - `dead circle: it changed no mode, object, camera, box or panel (mark it `back --required`)` → `dead <mode>.<action>: it changed no mode, object, camera, box or panel (mark it `back --required`)`, for example `dead jump.back: …`.
  - `square: no aim mode` → `jump.enter_aim: no aim mode`. `square: a known object to jump back to` → `aim.leave_aim: a known object to jump back to`. `square: no jump mode after aim mode` → `aim.leave_aim: no jump mode after aim mode`.
  - `menu: closed by circle` → `menu: closed by menu.close_menu`. `bar: closed by circle` → `bar: closed by build_bar.close_bar`.
  - `run_title`'s parameter `cross_frame` → `choose_row_frame`. Its messages become `title: the title.choose_row press at f%d needs its first title.move_down at f%d` and `title: no %s game started on title.choose_row`.
  - The comment `/* With nothing known, □ in aim mode is a dead press: stay in aim mode. */` → `aim.leave_aim` in place of `□`.
  - `menu: Down missed row` and `bar: Left did not move` stay. Directions are not PSX words, and those messages are not on the step's list.
- **Proof file.** `git mv tools/pad_author_proofs/dead_circle.refused.pad tools/pad_author_proofs/dead_back.refused.pad`. Its first line becomes `# expect: dead jump.back: it changed no mode, object, camera, box or panel`. Its body (`title 12 reasonable`, `idle 4`, `back`) is unchanged. The Makefile picks it up through its `*.refused.pad` wildcard. Nothing else names the file outside `docs/plan-*.md`.
- **Includes.** `pad_author.c` keeps `game/pad_jump.h` for the candidates, world points and `pad_jump_direction(i)`. It adds `game/bindings.h` (or gets it through `pad_nav.h`, H-7-2).
- **Makefile legend.** In the `make pad-author` entry, "a button 6 that changes nothing unless `back --required`" becomes "a back action (button 6 in the default bindings) that changes nothing unless `back --required`". No other Makefile change in chunk 3.

### What does not change

- `tests/test_pad_jump.c` and `tests/test_pad_nav.c` keep calling `pad_jump_first_direction`/`pad_jump_direction_button`. They test game code, and step 12 handles them.
- The game: no file under `src/game/` changes, so the button guard and its allowance are untouched.
- `src/cmd/pad_author.c`, `src/cmd/replay.c`: no change (step 7 already passes the table).

## Implementation Chunks

1. **One button reader and count in the devtools.** Switch `input_json.c` and `pad_author.c`'s `parse_buttons` to `bindings_read_button_number`. Delete `PAD_BUTTON_COUNT` and `PAD_METRICS_BUTTON_COUNT` in favour of `P8_PAD_BUTTON_COUNT`. Add `src/game/bindings.c` to the six explicit test links. Independent of step 7. Verified:
   - `make test-all` green, with `test_input`'s refusals (`[13]`, `[05]`, `["cross"]`) unchanged;
   - `make pad-author-proof`, with `button_name` and `button_range` refused with the same text;
   - `make pad-author` then `git diff --exit-code worst-case/`;
   - `make pad-metrics-check` and `make pad-metrics-worst-case` unchanged;
   - the count and digit-loop greps.
2. **`pad_metrics` off `pad_jump`'s conversions.** Add `pad_metrics_direction_button` and `pad_metrics_first_direction_button`. Convert `hold_step`, `empty_cones` and `judge_frame`, derive the D-pad mask, and rename `first_direction` → `first_direction_button` and `circle_dead_other` → `button_6_dead_other`. Update `tests/test_pad_metrics.c` mechanically and add the mapping tests. Independent of step 7. Verified: `make test-all` and the four guardrail commands byte-identical (report files included), and `git grep -n 'pad_jump_direction_button\|pad_jump_first_direction' -- src/devtools/pad_metrics.c` empty.
3. **High-level presses through the table** (needs step 7). Add the mode mapping, the direction and back action maps, and the lookup helper. Convert every high-level call site and `emit`'s empty-cone and dead-back checks. Messages and names are **not** renamed yet, so the generator's output is fully byte-identical in this chunk. Verified:
   - `make test-all`, every guardrail command unchanged (the `dead_circle` proof still expects `dead circle: …`), run once without and once with a swapped personal `bindings.cfg`;
   - `git grep -n 'pad_jump_direction_button\|pad_jump_first_direction' -- src/devtools` empty;
   - `grep -n P8_PAD_BUTTON_ src/devtools/pad_author.c` shows only the chord line.
4. **PSX words out of the generator.** Do every rename and message change of "Renames and messages", the `dead_back.refused.pad` move with its new `# expect:` line, and the Makefile legend line. Verified: the full check list of Testing Decisions, recorded in the step note.

## Testing Decisions

- A good test goes through the public interface: `p8_input_load` on JSON text, `pad_metrics_*` functions on button masks, and the generator through its proofs. Tests never inspect a static helper.
- **`tests/test_pad_metrics.c`.**
  - Mechanical changes: the hold-repeat cross-check compares against `pad_metrics_direction_button(jumped)` instead of `pad_jump_direction_button(jumped)`, and the two `circle_dead_other` reads become `button_6_dead_other`. No expected value changes.
  - New tests: each direction maps to buttons 1–4 and NONE to 0; `pad_metrics_first_direction_button` follows UP, DOWN, LEFT, RIGHT priority (all four → 1, `2|3|4` → 2, `3|4` → 3), ignores non-D-pad buttons (`5|7` → 0, with 5 added to a direction it still gives that direction), and gives 0 for 0. Prior art: `test_pad_jump.c`'s conversion test from step 1.4.
- **`tests/test_input.c`.** Its existing refusals already cover the reader's grammar through `p8_input_load` (`[13]`, `[05]`, names). Add one case only if a grammar edge is missing: a long digit run such as `[99999999999]`, which step 6's reader consumes whole and refuses, as the old loop did.
- **`pad_author.c` has no unit test** (step 1.4's note: covered by the proofs). Proof of behaviour:
  1. `make pad-author-proof`: every frames proof regenerates its e2e file byte for byte, every end-state proof reaches its end state, and every refusal proof is refused with its `# expect:` text, including the new `dead jump.back: …`.
  2. `make pad-author` then `git diff --exit-code worst-case/`: no diff.
  3. `make pad-metrics-check` (`PAD_METRICS_PROOF_REQUIRED` unchanged) and `make pad-metrics-worst-case`: console output byte-identical to before the step.
  4. Items 1–3 again with a valid personal `bindings.cfg` that swaps `jump.enter_aim` and `jump.open_menu` (7 ↔ 8): all unchanged (H-7-4). Delete the file afterwards and never commit it.
  5. **One-off mutation proof, recorded in the step note and not committed.** Swap `jump.enter_aim = 8` and `jump.open_menu = 7` in `bindings.default.cfg` itself, rebuild `tmp/pad_author`, and run `make pad-author`. It must fail at the first command that presses either action, because the game still answers plan 12's constants (H-7-6). An aim entry fails with `jump.enter_aim: no aim mode`. A `menu` command fails waiting for its row (`menu: a row focused: not within 20 frames`). Record which one came first. That shows the generator presses what the table says. Restore the file (`git checkout bindings.default.cfg`), then rerun item 2 to show a clean tree.
  6. Greps, recorded in the step note:
     - `grep -n P8_PAD_BUTTON_ src/devtools/pad_author.c`: only the chord line in `emit`;
     - `git grep -n 'pad_jump_direction_button\|pad_jump_first_direction' -- src/devtools`: empty;
     - `git grep -nE '_BUTTON_COUNT +12' -- src/devtools`: empty;
     - `git grep -n "'9'" -- src/devtools`: only `replay_source.c`'s hex reader;
     - `git grep -niE 'circle|square|cross|triangle|×|○|□|△' -- src/devtools/pad_author.[ch]`: nothing.
  7. `make e2e` with every `.sha256` unchanged, `make end-state` unchanged, and `make raylib` links.
  8. Timings: `make test` and `make e2e`, three warm runs each, against step 1's baseline (4.27–4.58 s and 0.24–0.25 s).
- Guardrail commands run after every chunk, because each chunk touches `src/devtools/pad_*` or `input_json.c` (section 2).

## Out of Scope

- Any change under `src/game/`: the game's button reads (steps 9–13), the deletion of `pad_jump_first_direction`/`pad_jump_direction_button` (step 12), and the cheat module's chord mask (step 13).
- Rewording the pad-metrics report (`section4 dead circle presses …`) and `pad_metrics.c`'s button-level rules on buttons 6 and 7.
- The generator reading fixtures, or `.pad` scripts written in actions (rejected in section 2).
- PSX words in `tests/e2e/*.txt` notes (`square: aim mode`, `triangle on …`): they are not in this step's text.
- The keymap parser's own reader and `KEYMAP_BUTTON_COUNT` (step 15).
- Fixtures in `replay`, the `"bindings"` header member and the personal flag (steps 16–17).
- A unit test for `pad_author.c`.

## Further Notes

- **Trap: a direction is not a button.** UP and DOWN have the values of buttons 1 and 2, so a `pad_direction_t` passed where a button bit is expected only goes wrong on LEFT and RIGHT. Keep `pad_metrics_direction_button` between them, and keep the `_button` suffix on every parameter that holds a bit.
- **Trap: press one button per action.** An action may have several buttons (`pause.close_pause = 6, 11`). Pressing the whole mask would emit a chord no script asked for. The lookup returns the lowest button. The detectors, though, match the whole mask.
- **Trap: look up before the frame.** The mode must be read before `input_step_run`, as `emit` already does for its log. Reading it after a press gives the next mode (a `jump.open_menu` frame reads as `menu`).
- **Trap: `pad_metrics_cone_empty` resets the shared scratch zone.** The empty-cone conversion must not hold a candidate list across that call, as today.
- **Trap: guardrails twice.** Run the four guardrail commands without and with the swapped personal file. Only the second run proves H-7-4 for this step.
- The step note (`### Step 2.8`) records which H-7-k held (and how the code adapted where one did not), the list of converted call sites by action, the mutation proof's output, the greps, the two guardrail runs and the timings.
