# Plan 17 — Remappable pad: abstract buttons and compiled bindings

## 1. Goal

The player changes which button does what by editing a text file and rebuilding, without touching C code and without any e2e replay changing.

Today (after plan 12) `src/game/pad_nav.c` and its helpers test hardcoded PSX buttons (`P8_PAD_CROSS`, `P8_PAD_SQUARE`, …): about 45 tests in `src/game/`. The control scheme lives only in code. This plan moves it into two files:

```
physical key ──keymap──▶ button 1..12 ──bindings──▶ action of the current mode
```

Goals that justify every rule below:

- **The e2e replays stay the source of truth.** A player's personal file never touches them. Every refactor step keeps every `*.sha256` byte-identical.
- **One mechanism on every platform.** Both files are compiled in, so the PSX (paused, not abandoned) gets the same tables with no file system.
- **No hidden control rule.** The game code asks "which buttons play action A in mode M", and a guard stops a hardcoded button from coming back.

## 2. Rules

### Guardrails

- `make test-all` passes on every commit. `make test` and `make e2e` never get slower (`CLAUDE.md`).
- Every step until Phase 4 is a pure refactor for the default bindings: every `tests/e2e/*.json.sha256` and every `worst-case/` end state stays unchanged. A step that changes one is wrong. Never re-baseline to make it pass.
- In steps 4 and 9–13 the existing pad unit tests (`tests/test_pad_*.c` and the rest) change only mechanically: the rename, and passing the default table in. No expected value changes. They pin the default to plan 12's behaviour on the paths no replay presses, so `CLAUDE.md`'s permission to update a test that froze old behaviour does not apply here. An assertion that must change is a divergence between the code and section 2's table: follow the code and record it in the step note.
- Every step that touches `src/game/pad_*`, `src/devtools/pad_*`, `src/devtools/input_json.c` or the button constants also runs, outside `make test-all`: `make pad-author-proof`, then `make pad-author` followed by `git diff --exit-code worst-case/`, then `make pad-metrics-check` and `make pad-metrics-worst-case`, all unchanged unless the step says otherwise. These tools can change their output while every game frame stays identical.
- Never launch the raylib app (`CLAUDE.md`). `make raylib` only compiles and links.
- Everything written is in English. Comments follow `CLAUDE.md`'s comment rule.

### Prerequisite

- This plan builds on plan 12's code (jump, aim and slipway modes, the L1 build bar, the R1 tech/score cycle, Select, the × + ○ cheat chord). Plan 12 lives on branch `plan-12`. The user merges it into `master` (merging is the user's job, never an agent's), then branch `remap` is rebased on `master`. Step 1 checks this and stops if it is not done.

### Locked decisions

**Model**

- **Abstract buttons.** There are exactly 12 buttons, numbered 1–12 in the order of today's bit mask: 1 up, 2 down, 3 left, 4 right, 5 cross, 6 circle, 7 square, 8 triangle, 9 L1, 10 R1, 11 start, 12 select. Button N is bit `1 << (N-1)`. The game code no longer knows PSX names. The D-pad is buttons 1–4 like any other.
- **Two files, two layers.** `keymap` maps a physical key or gamepad button to button N. `bindings` maps `mode.action` to one or more buttons N.
- **Bindings format.** One line per `mode.action = N[, N…]`. `#` starts a comment. Same style as today's `keymap.cfg`.
- **A missing line means the action is unbound in that mode.** It never falls back to the default. What the file says is what the game does. The doc's `nothing` entries disappear.
- **Conflicts are errors.** The same button on two actions of one mode makes the file invalid (`bindings.cfg:12: button 7 already bound in mode jump`). The same button in different modes is normal. Unknown mode, unknown action, a button outside 1–12, or a mode.action given twice are also errors.
- **The cheat chord stays fixed on buttons 5 + 6**, below the bindings: `cheat.c` and `pad_nav.c`'s `without_chord` keep hiding buttons 5 and 6 from every mode from a chord frame until both are up, whatever the bindings say. `without_chord` takes the chord mask from the cheat module, so no button name is left in `pad_nav.c`. It is a dev tool. The default bindings file documents that 5 + 6 together is reserved.
- **On the map, the mode's ○ action** (`jump.back`, `aim.back`, `slipway.cancel_slipway`, `placement.cancel_placement`) **first closes a shown message box** (the year box) and does nothing else, as ○ does on plan 12. There is no separate message-box mode.

**When and how the files are read**

- **At compile time, on every platform.** A host tool parses the text and generates a C table. The game core receives a `bindings_t` table at init and never reads a file. An invalid file makes `make` fail with `file:line: message`.
- **Which file is compiled.** The personal `bindings.cfg` at the repository root if it exists, else the committed `bindings.default.cfg`. Same rule for `keymap.cfg` / `keymap.default.cfg`. The personal files are gitignored. The `.default.cfg` files are the single source of the defaults: no C string copy, no sync check.
- **Stale-build trap.** Make does not notice a file appearing or disappearing. The Makefile writes a stamp (`tmp/bindings.source`, `tmp/keymap.source`) holding the chosen path, rewritten only when the path changes. The generated table depends on the stamp and on the chosen file. `make raylib` prints one line per file: `bindings: bindings.cfg (personal)` or `bindings: bindings.default.cfg`.
- **The `replay` binary (and so `make e2e`) always compiles the defaults**, never the personal files.
- **PSX** compiles the defaults only. There is never an in-game remap screen, on any platform.

**E2e replays and recordings**

- **Replay format.** `"pad"` lists button numbers (`{ "pad": [5, 7] }`), not names. Every file with `"pad"` frames (the e2e and `worst-case/` replays, the unit-test fixtures, the inline JSON in tests) is converted by a script. The conversion must leave every `.sha256` and end state byte-identical. The parser then accepts numbers only.
- **A replay may name a fixture.** The header member `"bindings": "swap_aim_menu.cfg"` names a file in the replay's directory. `replay` reads it at runtime with the same parser module (devtools is PC-only, and it already reads files). Without the member, the replay uses the compiled default. The fixture is a Makefile prerequisite of its replay's frames, so editing it replays the file. Fixtures are frozen test data, never a player's file.
- **Personal recordings are flagged.** A `game` built with a personal `bindings.cfg` writes `"bindings": "personal"` in every recording's header. `replay` refuses such a recording with a clear message. A personal `keymap.cfg` needs no flag: recordings store buttons, which come after the keymap.
- **Generator.** The low level of `.pad` scripts (`tap`, `hold`) uses button numbers, converted like the JSON. The high-level commands (`colonise`, `probe`, `slipway`, …) translate through the default bindings. Every generated replay is default-bound. A replay with a fixture is written at low level.

**Code shape**

- **Replace on the spot.** Each `pressed & P8_PAD_X` test inside a mode's code becomes `bindings_mask(bindings, MODE, ACTION)`. The mode is the one the code runs in. Where a test runs before the mode dispatch (pause, the shoulders, Select, ○ closing a box), the mode is computed from the nav state at that point. The default reproduces today's constants, so frames are identical by construction. One group of modes per step.
- **Constants renamed.** `P8_PAD_UP … P8_PAD_SELECT` become `P8_PAD_BUTTON_1 … P8_PAD_BUTTON_12`. Directions used as directions (`pad_target`, `pad_jump`, `pad_panel_neighbor`) get their own enum and stop reusing button bits.
- **Guard.** A `make test` check (a grep, like `tools/pointer_guard.sh`) fails when a file under `src/game/` names `P8_PAD_BUTTON_` outside `cheat.c` and the bindings module. It starts at step 5 with an allowance for the reads not yet converted, which shrinks to nothing by step 14.

**Docs**

- `bindings.default.cfg` carries, as `#` comments, what each mode is and does. It replaces `docs/pad-modes-and-actions.md`, which is deleted once the file exists.
- The README's Controls section becomes a pointer to `keymap.default.cfg` and `bindings.default.cfg` and how to override them.

### Modes and actions (the spec of `bindings.default.cfg`)

The default reproduces plan 12's controls exactly. Each line gives `action = button(s)`:

- **title** (difficulty rows, no box): move_up 1, move_down 2, move_left 3, move_right 4, choose_row 5.
- **jump**: jump_up 1, jump_down 2, jump_left 3, jump_right 4, act_on_hovered 5, back 6, enter_aim 7, open_menu 8, open_build_bar 9, open_tech_panel 10, pause 11, select_tap_or_year 12.
- **aim**: pan_up 1, pan_down 2, pan_left 3, pan_right 4, probe_or_fold 5, back 6, leave_aim 7, open_build_bar 9, open_tech_panel 10, pause 11, select_tap_or_year 12.
- **slipway**: jump_up 1, jump_down 2, jump_left 3, jump_right 4, build_on_target 5, cancel_slipway 6, open_build_bar 9, open_tech_panel 10, pause 11.
- **menu** (the box of a hovered object): move_up 1, move_down 2, move_left 3, move_right 4, choose_row 5, close_menu 6, open_menu 8 (pressed again, closes the box), open_build_bar 9, open_tech_panel 10.
- **pause**: move_up 1, move_down 2, move_left 3, move_right 4, choose_row 5, close_pause 6 and 11.
- **build_bar**: pick_left 3, pick_right 4, take_structure 5, close_bar 6, open_build_bar 9 (pressed again, closes the bar), open_tech_panel 10, pause 11.
- **placement**: pan_up 1, pan_down 2, pan_left 3, pan_right 4, place_structure 5, cancel_placement 6, open_build_bar 9, open_tech_panel 10, pause 11.
- **panels** (tech and score): move_up 1, move_down 2, move_left 3, move_right 4, choose_row 5, close_panel 6, open_build_bar 9, next_panel 10.

Behaviours an action keeps whatever button it is on: the jump hold-repeat on `jump_*`, the pan acceleration on `pan_*`, tap versus half-second hold on `select_tap_or_year`, press versus hold (fold) on `probe_or_fold`, and "a button held from before a menu opened picks nothing until pressed again" on `choose_row`. From slipway, menu and placement, `open_build_bar` and `open_tech_panel` first cancel what is held or close the box. `pause` from placement keeps the structure.

If a step finds plan 12's code doing something this table does not list, it follows the code (e2e unchanged) and records the difference in its step note.

### Rejected

- **Runtime reading of the files (at launch, live reload, or on a reserved key):** the PSX has no file a player can edit. One compile-time mechanism serves every platform.
- **An in-game remap screen:** the user does not want one, on any platform.
- **Partial bindings files layered over the default:** the file must say exactly what the game does.
- **PSX button names in the files or the replays:** the code must not think in "cross" any more. Names kept as aliases would keep that vocabulary alive.
- **Replays recording actions instead of buttons:** a format rewrite. Buttons plus a named fixture are enough.
- **Inline bindings in a replay's header:** a second format next to the fixture file reference.
- **Mixing the fixture's bytes into the frames hash:** a comment edit would turn a replay red with no behaviour change. The Makefile prerequisite is enough.
- **One merged file (actions to keys):** it would drop the button layer the replays live on.
- **More than 12 buttons:** no use case yet. Raising it later is a constant, with no replay touched.
- **A bindable cheat chord, or a dedicated cheat key:** it is a dev tool, and four replays type it as buttons 5 + 6.
- **A message-box mode layered over jump and aim:** the only layered mode, for one case.
- **Translating buttons to actions before `pad_nav` (option b of the code shape):** `pad_nav_step` changes mode within a frame, so it means rewriting the dispatch. It may come later, once the bindings are pinned by replays.
- **The generator reading fixtures (`.pad` scripts written in actions):** it would have to simulate `pad_nav` to know the mode.
- **Compiling every e2e fixture into `replay`:** a code-generation step per fixture to avoid one `fopen` in a dev tool.
- **Copying the personal file next to each recording:** only worth it for e2e replays made from games played with personal controls, which nobody plans.
- **README describing controls per action and key:** the user wants a pointer to the two files only.
- **Keeping `docs/pad-modes-and-actions.md` with a sync check:** a third copy of the list.

### Out of scope

- The "replay then play" path of `src/cmd/game.c` (a `.json` replayed by the game binary, then control handed back). The user no longer uses it. It reads frames as buttons with whatever bindings were compiled, with no guarantee. Removing it is a separate piece of work: `replay_source` is shared with `test_replay_source`, `test_recording_continuity` and `test_marker_coherence`.
- Audio (`CLAUDE.md`).
- Any change to plan 12's control scheme itself. This plan only moves it into files.

## 3. Phases and steps

### Phase 0 — Base

1. **Base check.** `git merge-base --is-ancestor origin/plan-12 HEAD` succeeds on branch `remap`, `make test-all` passes, and `docs/pad-modes-and-actions.md` is present. If plan 12 is not in `master` yet, stop and ask the user. Time `make test` and `make e2e` on this base: the baseline for every "not slower" check. Verified by the commands' output and both timings in the step note.

### Phase 1 — Abstract buttons

2. **Replays in numbers.** A one-off script converts `"pad"` names to numbers in every file a grep for `"pad"` frames finds: the 17 `tests/e2e/*.json`, the 5 `worst-case/*.json`, the 17 `tests/fixtures/*.json` and the inline JSON in `tests/*.c`. The generator's own JSON writer (the `"pad"` output of `src/devtools/pad_author.c`) moves to numbers in the same step. Parser fixtures whose purpose is a bad name (unknown button, duplicate button) are rewritten for numbers instead, for example an unknown button becoming `13`. `src/devtools/input_json.c` then reads and writes numbers 1–12 only, and rejects names. Verified: every `.sha256` byte-identical (`make e2e`), `make end-state` unchanged on every replay, `make pad-metrics-worst-case` unchanged, `make pad-author-proof` then `make pad-author` followed by `git diff --exit-code worst-case/` (the generator rebuilds the script-converted files byte for byte), `make pad-metrics-check`, a unit test refuses `"cross"` and `13`, every unit test that loads a fixture passes unchanged, and a grep finds no PSX button name in any `"pad"` array in the repository.
3. **Generator low level in numbers.** `tap` and `hold` in `.pad` scripts take button numbers, and the 18 `.pad` files are converted. `src/devtools/pad_author.c` and `pad_metrics.c` lose their name tables, and the Makefile's expected `PAD_METRICS_PROOF_REQUIRED` text moves to numbers with the same three frames and verdicts. Verified: `make pad-author-proof`, then `make pad-author` followed by `git diff --exit-code worst-case/`, then `make pad-metrics-check` and `make pad-metrics-worst-case`.
4. **Rename the constants.** `P8_PAD_*` becomes `P8_PAD_BUTTON_1..12` everywhere. Directions get their own enum in `pad_target`, `pad_jump` and `pad_panel`. A pure rename, verified by `make test-all` with every `.sha256` unchanged.

### Phase 2 — Bindings in the game

5. **Inventory of button reads.** After step 4's rename, list in this step's note every read of a `P8_PAD_BUTTON_*` in `src/game/`: presses and held buttons, including the values computed once before the mode dispatch and shared by several modes (the pressed direction, the ○ that closes a box, the menu and pause arming, `pad_panel_step`'s held button). Write next to each read the `mode.action` of section 2 it will become, and assign it to exactly one of steps 9–13. A read with no matching entry, or matching a different button, is settled here before step 6: section 2's table is amended with plan 12's behaviour (follow the code), so steps 6 and 7 build the enums and `bindings.default.cfg` from the reconciled table. Add the guard of section 2 now, with an allowance file seeded from this list (the remaining reads per file). Each of steps 9–13 removes exactly its entries from the allowance, and `make test` fails when the tree has a read the allowance does not list. Verified by the list matching a grep of `P8_PAD_BUTTON_` in `src/game/`, and by `make test` passing and not slower.
6. **Bindings module.** `src/game/bindings.{c,h}`: the mode and action enums of section 2, `bindings_t`, `bindings_mask(bindings, mode, action)`, and a pure parser from text to table with no I/O, returning `file:line: message` errors. Unit tests cover every error of section 2 (conflict, unknown mode or action, button out of range, duplicate line) and an unbound action. Not wired into the game yet. The module also owns the single button count (for example `P8_PAD_BUTTON_COUNT` next to `P8_PAD_BUTTON_12`) and the single button-number reader (the grammar of step 1.2's note: digits only, no leading zero, 1–12). The bindings parser uses it and its unit tests cover it.
7. **Compile-time bindings.** Add the committed `bindings.default.cfg` (section 2's table, with mode comments and the 5 + 6 reservation). A host generator in `src/cmd/` turns the chosen file into a generated header. The Makefile handles the stamp and prints its line, and `bindings.cfg` goes into `.gitignore`. `input_step` hands the table to `pad_nav`, which does not read it yet. Verified: `make` fails with `file:line` on a broken personal file, creating or deleting `bindings.cfg` regenerates the table, a no-change `make raylib` recompiles nothing, and `make test` is not slower. With a valid personal `bindings.cfg` that swaps two actions present, `make test-all` stays green and `make pad-author` followed by `git diff --exit-code worst-case/` shows nothing: only `game` compiles the personal file. Prove it on the built binaries without launching anything (for example by a symbol or table dump): with the swapped personal file, the `game` binary from `make raylib` holds the personal table and `replay` the default; after deleting the file and rebuilding, `game` holds the default again. Shared objects linked into both binaries must not carry the table.
8. **Generator high level reads the default bindings.** Each high-level command in `src/devtools/pad_author.c` (`colonise`, `probe`, `slipway`, …) takes its button from the compiled default table for the mode it runs in. `tap` and `hold` stay raw numbers. Verified like step 3: `make pad-author-proof`, then `make pad-author` followed by `git diff --exit-code worst-case/`, `make pad-metrics-check` and `make pad-metrics-worst-case`, all repeated with a swapped personal `bindings.cfg` present, and `pad_author.c` names no `P8_PAD_BUTTON_` outside the low-level path. The direction taps of the high-level commands also take their buttons from the default table (`jump.jump_*`, `aim.pan_*`, …), and `pad_metrics.c` gets a devtools-side direction-to-button mapping (or reads the default table) in place of `pad_jump_first_direction`/`pad_jump_direction_button`, including its inline D-pad mask; its `first_direction` parameter that holds a button bit is renamed. `pad_author.c`'s `parse_buttons` and `input_json.c` switch to step 6's button-number reader and count. The PSX words step 1.3's note deferred go in this step: the messages `dead circle`, `square: …` and `closed by circle` name the action or button number, and the locals `settle_circle`, `circle_before`, `circle_pending`, `next_circle_required` and the field `circle_dead_other` are renamed. The `# expect:` line of `dead_circle.refused.pad` (and its file name, if wanted) changes with them: the only allowed change in the generator commands' output. Also verified: no call to `pad_jump_direction_button` or `pad_jump_first_direction` is left in `src/devtools/`, and no other `*_BUTTON_COUNT 12` or second button-number digit loop exists there.
9. **Jump mode reads its bindings.** Every button test in `jump_step` and its helpers goes through `bindings_mask(…, MODE_JUMP, …)`, including the hold repeat. Verified: e2e unchanged, plus a unit test with a table that swaps actions in the converted mode and leaves every other mode unbound: the converted mode follows the table, and the same buttons do nothing in a neighbouring mode served by the same shared code. Without it a read tagged with the wrong mode passes, because the default puts most actions on the same button in every mode. In steps 9–13, the step that converts a read also renames the local or comment that carries a PSX name on that line (`cross_engaged`, `circle_returns`, a `×` in a comment, …) to the action's name.
10. **Aim and slipway modes read their bindings**, verified the same way.
11. **Build bar and placement read their bindings**, verified the same way.
12. **Menu, panels, pause and title read their bindings**, including `pad_panel_step`'s held button and the `choose_row` arming. Verified the same way. It deletes only the game-side direction conversions: the devtools no longer use them after step 8.
13. **Cross-mode tests read their bindings:** pause, the shoulders, Select, and `back` closing the message box. The mode is computed from the nav state where each test runs. Verified the same way. Moving the chord mask behind the cheat module also renames `cheat.c`'s `cross`/`circle` locals.
14. **Guard closed.** The allowance of step 5 is down to `cheat.c` and the bindings module, and the allowance file is removed in favour of those two fixed exceptions. Verified: the guard passes on the tree and fails on a planted `P8_PAD_BUTTON_5` in `src/game/pad_nav.c` (checked once and recorded in the step note, not committed). `make test` is not slower. Also verified: a grep of `src/game/pad_*` and `src/game/cheat.*` for `cross|circle|square|triangle|×|○|△|□` finds nothing.

### Phase 3 — Compile-time keymap

15. **Keymap compiled in.** Add the committed `keymap.default.cfg` (today's `builtin_text`, with button numbers instead of names). Reuse the host generator pattern with its own stamp and line, and delete the runtime `keymap_load` path and the C default string. Run `git rm --cached keymap.cfg` and add it to `.gitignore`. The old-format `keymap.cfg` left in the working tree is converted in place to button numbers, so local edits are kept and the build does not break. Verified: after the commit, `make raylib` from the same tree prints `keymap: keymap.cfg (personal)` and links, a broken personal `keymap.cfg` fails `make` with `file:line`, `tests/test_keymap.c` is rewritten for the parser (not deleted), and `make test` is not slower. With `keymap.cfg` moved out of the way, as on a fresh clone, `make test-all` passes and `make raylib` prints `keymap: keymap.default.cfg` and links. With the file put back, the personal line returns. The keymap parser reuses step 6's button-number reader and count; `KEYMAP_BUTTON_COUNT` goes.

### Phase 4 — Replays that prove the remap

16. **Fixtures in `replay`.** `replay` reads the header member `"bindings": "<file>.cfg"` from the replay's directory with the parser of step 6, and refuses `"bindings": "personal"` with a clear message. The Makefile makes each named fixture a prerequisite of its replay's frames. Verified by unit tests (fixture found, fixture missing, personal refused) and by `make e2e` unchanged.
17. **Personal recordings flagged.** A `game` built with a personal `bindings.cfg` writes `"bindings": "personal"` through the recorder's header member. The flag comes from the generated header. Verified by a unit test on the recorder with both header variants, and by checking that the `game` binary built with a personal file carries the flag and the one built without it does not.
18. **Swap twin.** Add `tests/e2e/swap_aim_menu.cfg` (the default with `jump.enter_aim` and `jump.open_menu` swapped, buttons 7 and 8). Build the twin from a prefix of an existing replay, cut under 180 frames, that presses both buttons in jump mode (for example `pad_colonise` up to about f155). Swap 7 and 8 only on frames where the original is in jump mode, as `make state` shows, and list those frames in the twin's `.txt`. Verified: `make end-state` gives the twin the same end state as the original cut at the same frame, and the twin passes `tools/e2e_limits.sh`. Once, recorded in the step note and not committed: touching the fixture makes `make e2e` replay the twin and only the twin, and swapping 7 and 8 back in it turns the twin red.
19. **Unbound action.** Add a fixture with no `jump.enter_aim` line and a short replay that presses button 7 in jump mode. The game stays in jump mode, and the `.txt` names the frames that show it. Before setting the baseline, prove it mechanically and record both results in the step note: with the fixture, `make pad-metrics` lists the press as a dead press of button 7 in jump mode at the frame the `.txt` names; without the `"bindings"` member, the same press is live and the aim-frame total is higher. Verified by those two runs, then its `.sha256` and `.txt`.

### Phase 5 — Docs and cleanup

20. **Docs.** The README's Controls section becomes the pointer of section 2. Delete `docs/pad-modes-and-actions.md`. Verified by reading: no README line names a PSX symbol as a control, and every mode of section 2 is explained in `bindings.default.cfg`.
21. skill: rework-comments — the files this plan created or rewrote (`src/game/bindings.*`, the two generators, `src/devtools/input_json.c`, `src/platform/keymap.*`).

## 4. Done when

- `make test-all` passes. Every `tests/e2e/*.json.sha256` from before step 2 is unchanged, and the two new replays are green.
- `make end-state` matches on every replay, and the swap twin matches its original cut at the same frame.
- No file under `src/game/` names `P8_PAD_BUTTON_` outside `cheat.c` and the bindings module, enforced by the guard.
- Creating a personal `bindings.cfg` that swaps two actions and running `make raylib` prints `bindings: bindings.cfg (personal)`, and the built `game` binary holds the swapped table while `replay` holds the default. Deleting the file prints the default line again. The same holds for `keymap.cfg` and `keymap: …`. `make test-all`, `make pad-author` (no diff in `worst-case/`) and the pad-metrics checks are unchanged in both cases.
- `keymap.cfg` and `bindings.cfg` are gitignored. `keymap.default.cfg` and `bindings.default.cfg` are committed and are the only copies of the defaults.
- `make test` and `make e2e` are not slower than at step 1 (timings recorded in step 1's and the last step's notes).

## 5. Open questions

(none)

## 6. Step notes

### Step 1.1

- `git merge-base --is-ancestor origin/plan-12 HEAD` fails: plan 12 reached `master` as one squash commit (af39be3, #64), not a merge. `git diff origin/master origin/plan-12 -- src tests worst-case` is empty. `guideline` decided the prerequisite holds, since what it requires is plan 12's code in `master`. The `Makefile` differs only by master's newer PSX targets.
- The run is on the harness-assigned branch `claude/loving-brahmagupta-ulwvej` (based on origin/master 8ea422e), not `remap`.
- `docs/pad-modes-and-actions.md` is present. `make test-all` passes (17 e2e match).
- Baseline on a warm build, 4 cores, three runs: `make test` 4.27–4.58 s, `make e2e` 0.24–0.25 s.
- No PRD: the step only checks things and changes no code.

### Step 1.2

- Three commits: the loader accepts numbers; the writer and every `"pad"` array move to numbers; the loader refuses names. The `pad_author.c` source did not change: it writes JSON through `p8_input_save`, so its output moved to numbers on its own.
- Converted with a one-off script (described in the commit message, not committed): 17 e2e replays, 4 of the 5 `worst-case/` files (`title_idle.json` has only empty frames), 12 fixtures with buttons, and the inline JSON of `test_input.c`, `test_replay_source.c` and `test_e2e_limits.c`. Bad-name fixtures: `["l2"]` → `[13]`, duplicate → `[5, 1, 5]`, select twice → `[12, 1, 12]`.
- Loader grammar: digits only, 1–12, no leading zero or sign, no fraction or exponent, nothing glued to the number, no repeats, any order. The writer emits buttons in ascending order with `, `.
- Every `.sha256` and end state unchanged. `pad-author-proof`, `pad-metrics-check` and `pad-metrics-worst-case` unchanged. After `git add`, `make pad-author` regenerates `worst-case/` with no diff. `make test` 4.22–4.41 s, `make e2e` 0.23–0.25 s.
- The generator's mode log (`[cross+up]`) is not a `"pad"` array and keeps names until step 3.

### Step 1.3

- Two commits: generator and scripts, then report, unit test and Makefile.
- `tap`/`hold` take `+`-joined numbers 1–12 (digits only, no leading zero, no empty element, no repeat). A name gets the existing `unknown buttons '<token>'` message. A repeated button such as `5+5` is now refused; the old name parser accepted it, and no script used it. New refusal proofs: `button_name.refused.pad` (`tap cross`) and `button_range.refused.pad` (`hold 13 4`).
- Only 6 of the 18 `.pad` files had `tap`/`hold` lines, converted by a one-off script. Button names in `.pad` comments were rewritten by hand.
- Mode log `[5]`/`[2+4]`. Messages now `chord: buttons 5 and 6 …` and `empty cone: no candidate for button N …`. Messages inside high-level commands (`dead circle`, …) stay for step 8.
- `pad_metrics` lost its name table and `pad_metrics_button_name`. `PAD_METRICS_PROOF_REQUIRED` = `dead f16 press 7 aim required;dead f17 press 6 aim required;dead f293 press 6 jump required`. `pad-metrics-worst-case` console output is byte-identical. Its report files differ only in the button field (checked by substitution).
- Every `.sha256` unchanged, `worst-case/*.json` regenerated with no diff. `make test` 4.17–4.69 s, `make e2e` 0.23–0.24 s.

### Step 1.4

- Two commits: the rename `P8_PAD_*` → `P8_PAD_BUTTON_1..12` (same values, no aliases, one-off script on word boundaries; old names remain only in `docs/plan-*.md`), then the direction enum.
- New header `src/game/pad_direction.h`: `pad_direction_t` (NONE = 0, UP, DOWN, LEFT, RIGHT). `pad_target_pick`, `pad_jump_*`, `pad_panel_neighbor`/`pad_panel_step`, `pad_nav_neighbor` and `pad_metrics_cone_empty` take or return it. The new `pad_jump_direction_button` (in `pad_jump.{h,c}`, next to its inverse `pad_jump_first_direction`) converts back wherever a direction is pressed, held or tapped: the hold repeat, `aim_pan`, the generator's path taps, the metrics masks. Trap: UP and DOWN equal the bits of buttons 1 and 2, so a misused direction shows only on LEFT/RIGHT.
- Pad tests changed only mechanically; one new conversion test in `test_pad_jump.c`.
- Every `.sha256` and end state unchanged; generator and metrics outputs byte-identical. `make raylib` links (raylib vendored in this container during the step). `make test` 4.18–4.27 s, `make e2e` 0.23 s.
- End of phase 1: `warn-review` flagged duplicated button parsing and counts, devtools depending on `pad_jump` for directions, and leftover PSX words. Accepted `plan-review` proposals: step 8 takes the devtools off the `pad_jump` direction helpers and renames the deferred PSX words, stating the `dead_circle.refused.pad` expectation change; steps 9–13 rename PSX-named locals on the lines they convert, and step 14 greps for PSX words; step 6 owns one button count and button-number reader, reused by step 8 and step 15. Not taken: the repeated chord mask (fixed on 5 + 6, can't drift), the duplicated `is_direction`, a unit test for `pad_author.c` (covered by the proofs).

### Step 2.5

- Two commits: the guard (`tools/button_guard.sh`, `tools/button_guard_allowance.txt`, `make button-guard` in `make test`, `tests/test_button_guard.c`), then this note and section 2's amendments. No file of `src/` changed.
- The inventory was read at 6b5b753 and re-checked on 52e00fb (no `src/` change between them): `git grep -o P8_PAD_BUTTON_ -- src/game` gives 37 names in 4 files, none in a comment or a string, and `make button-guard` prints `button-guard files 4 reads 37`. Count names, not lines: `git grep -c` gives 27 for `pad_nav.c`, because lines 461, 869 and 935 name two buttons each.
- Inventory. Lines are `pad_nav.c`'s unless a file is named. "Indirect" rows reach the buttons through a `pad_jump` helper or the shared pressed direction; the guard does not count them and their line is the call site. Buttons: 1–4 D-pad, 5 ×, 6 ○, 7 □, 8 △, 9 L1, 10 R1, 11 Start, 12 Select.

| # | Function | Line | Read | Mode(s) today | Becomes | Step |
|---|---|---|---|---|---|---|
| 1 | `jump_step` | 664 | pressed 7 | jump | `jump.enter_aim` | 9 |
| 2 | `jump_step` | 685 | pressed 8 | jump | `jump.open_menu` | 9 |
| 3 | `jump_step` | 688 | pressed 5, known object (click) | jump | `jump.act_on_hovered` | 9 |
| 4 | `jump_step` | 694 | pressed 5, slipway start | jump | `jump.act_on_hovered` | 9 |
| 5 | `pad_jump_repeat_step` from `jump_step` | 669 | indirect: pressed and held 1–4 (`pad_jump.c` 49, 79) | jump | `jump.jump_*` | 9 |
| 6 | `pad_jump_repeat_step` from `slipway_step` | 762 | indirect: pressed and held 1–4 | slipway | `slipway.jump_*` | 9 (shared helper) |
| 7 | `aim_mode_step` | 356 | pressed 7 | aim | `aim.leave_aim` | 10 |
| 8 | `aim_mode_step` | 360 | held 5 (fold release) | aim | `aim.probe_or_fold` | 10 |
| 9 | `aim_mode_step` | 362 | pressed 5 (engage) | aim | `aim.probe_or_fold` | 10 |
| 10 | `aim_pan` from `aim_mode_step` | 351 | indirect: held 1–4 (`aim_pan` 154) | aim | `aim.pan_*` | 10 |
| 11 | `aim_pan` from `placement_step` | 573 | indirect: held 1–4 | placement | `placement.pan_*` | 10 (shared helper) |
| 12 | `slipway_step` | 765 | pressed 5 (release on target) | slipway | `slipway.build_on_target` | 10 |
| 13 | `slipway_step` | 770 | not pressed 5 (gate on the re-aim) | slipway | `slipway.build_on_target` | 10 |
| 14 | `build_step` | 489 | pressed 5 | build_bar | `build_bar.take_structure` | 11 |
| 15 | `build_step` | 493–494 | indirect: shared direction LEFT/RIGHT | build_bar | `build_bar.pick_left`, `pick_right` | 11 |
| 16 | `placement_step` | 567 | pressed 6 | placement | `placement.cancel_placement` | 11 |
| 17 | `placement_step` | 575 | pressed 5 | placement | `placement.place_structure` | 11 |
| 18 | `panel_step` | 223, 225 | pressed 5 arms pause; held 5 masked until armed (2 names) | pause | `pause.choose_row` | 12 |
| 19 | `panel_step` | 228, 230 | pressed 5 arms menu; held 5 masked until armed (2 names) | title, menu, panels | `<mode>.choose_row` | 12 |
| 20 | `pad_panel_step` | `pad_panel.c` 260 | held 5 → left mouse button | title, menu, pause, panels | `<mode>.choose_row` | 12 |
| 21 | `panel_step` → `pad_panel_step` | 233 | indirect: shared direction | title, menu, pause, panels | `<mode>.move_*` | 12 |
| 22 | `map_step` | 793 | pressed 8, planet menu open | menu | `menu.open_menu` (closes the box) | 12 |
| 23 | `pause_step` | 869 | pressed 11 or 6 while pause is shown (2 names) | pause | `pause.close_pause` | 12 |
| 24 | `pad_nav_step` | 947 | indirect: `pad_jump_first_direction(pressed)` | all (rows 15, 21) | dissolved into rows 15 and 21 | 12 |
| 25 | `pad_jump_direction_button` | `pad_jump.c` 59, 61, 63, 65 | conversion naming 1–4 (4 names) | all | removed from `src/game/` | 12 |
| 26 | `one_shoulder` | 461 | pressed 10 xor pressed 9 (2 names) | jump, aim, slipway, menu, build_bar, placement, panels | `open_tech_panel` (`panels.next_panel`) xor `open_build_bar` | 13 |
| 27 | `shoulder_switch` | 466 | pressed 10 | same | `open_tech_panel` / `panels.next_panel` | 13 |
| 28 | `switch_panels` | 824 | pressed 6, off the map | build_bar, panels | `build_bar.close_bar`, `panels.close_panel` | 13 |
| 29 | `pause_step` | 879 | pressed 11 opens pause | jump, aim, slipway, build_bar, placement | `<mode>.pause` | 13 |
| 30 | `pad_nav_step` | 964 | held 12 (`select_down`) | every mode (tracker); acts in jump, aim | `jump/aim.select_tap_or_year` | 13 |
| 31 | `pad_nav_step` | 974 | pressed 6 → `circle_returns` | jump, aim, slipway | `jump.back`, `aim.back`, `slipway.cancel_slipway` | 13 |
| 32 | `pad_nav_step` | 975 | pressed 6 → closes a box, or the message box on the map | menu, panels; jump, aim, slipway, placement | `menu.close_menu`, `panels.close_panel`; the map mode's ○ action | 13 |
| 33 | `pad_nav_step` | 980 | clears 6 from `pressed` after closing the message box | same as 32 | same as 32 | 13 |
| 34 | `without_chord` | 935 | chord mask 5 + 6 (2 names) | all | cheat module mask, not a binding | 13 |
| 35 | `cheat_step` | `cheat.c` 34, 35 | held 5, held 6 (2 names) | all | stays | fixed |

- Per file, as the allowance lists it: `pad_nav.c` 30 = 4 (step 9: rows 1–4) + 5 (step 10: rows 7–9, 12–13) + 3 (step 11: rows 14, 16–17) + 7 (step 12: rows 18, 19, 22, 23) + 11 (step 13: rows 26–34); `pad_panel.c` 1 (step 12: row 20); `pad_jump.c` 4 (step 12: row 25); `cheat.c` 2 (fixed: row 35).
- Assignment rules: a read inside one mode goes to that mode's step; a helper shared by modes of different steps goes to the earliest one, which gives it a mode argument (hold-repeat → 9, `aim_pan` → 10); a pre-dispatch read serving several steps goes to 13; the shared pressed direction and `pad_jump.c`'s conversions go to their last game-side consumer, 12 (step 8 has moved the devtools off them by then).
- Mode of the pre-dispatch reads (step 13). Rows 31–33: `menu`, `panels` or `build_bar` with a non-pause box open; otherwise the map mode (`placement` when carrying, else from `map_mode`), even under pause, because the message-box close tests `on_the_map`, which stays true while pause is shown. Row 30: the union of `jump.select_tap_or_year` and `aim.select_tap_or_year` in every mode, so `pad_select_step`'s latch still stops a Select held from another mode from firing on return. Rows 26–27, 29: the current mode, with `shoulders_can_switch` and `pause_button_free` kept. Row 28: `build_bar` when the bar is open, else `panels`.
- Mode of `panel_step` (step 12): `pause` when shown; `title` with no game; `panels` over a tech or score panel; `menu` otherwise, which covers any box over the map (an aim-mode click too) and any panel over the build bar.
- Section 2 amended, following the code: menu's `close_menu 6 and 8` split into `close_menu 6` (○ before the dispatch, any non-pause box) and `open_menu 8` (`map_step`, planet menu only, no click releasing or pending); build_bar's `close_bar 6 and 9` split into `close_bar 6` (`switch_panels`) and `open_build_bar 9` (`one_shoulder`, exactly one of L1 and R1), so L1 + R1 together cannot close the bar through the ○ path; the message-box rule covers every map mode's ○ action, slipway and placement included (`pad_nav_step` closes it whenever the nav is on the map with no box); `without_chord` takes the chord mask from the cheat module. Not split: `pause.close_pause 6 and 11`, both read in one test with one condition. No new action name.
- Traps: step 11's build bar picks from the shared direction (first pressed of up, down, left, right), so up or down pressed with left or right does not move it today, and a plain `pick_left` binding would. The panels' `move_*` keep that priority. `jump.act_on_hovered` stays two reads with their own conditions.
- One-off red runs, not committed: a planted `P8_PAD_BUTTON_5` read in `pad_nav.c` gives `button_guard: src/game/pad_nav.c names 31 button(s), listed 30`; deleting the step-11 allowance line gives `names 30 button(s), listed 27`.
- Every `.sha256` unchanged. `make test` 4.46, 4.27, 4.31 s; `make e2e` 0.232, 0.236, 0.233 s.
- Pilot: the amendments to the message-box and cheat-chord sentences sit in section 2's Locked decisions, not the table, because the old wording would contradict the reconciled table. Step 6's PRD hypotheses (H-5-1…6) hold against this note.

### Step 2.6

- Two commits: the button count and reader, then the enums, table and parser. `src/game/bindings.{c,h}` are not linked into any binary yet. Only `tmp/test_bindings` builds them.
- `P8_PAD_BUTTON_COUNT` (12) is in `src/p8/input.h` next to `P8_PAD_BUTTON_12`, with a compile check. `bindings_read_button_number(text, &end)` returns 1–12 or 0 (digits only, no leading zero, every leading digit consumed, no overflow, `*end` restored on failure), and the caller checks what follows. The older copies in `input_json.c`, `pad_metrics.h` and `keymap.h` stay for steps 8 and 15.
- API: `bindings_mode_t` (9 modes, section 2's order), `bindings_action_t` (35 actions), `bindings_t { int masks[mode][action]; }` (no pointer), `bindings_mask` (0 for an unbound action or one the mode lacks, never an assert), `bindings_mode_has_action`, `bindings_mode_name`, `bindings_action_name`, `bindings_error_t { int line; char text[256]; }`, and `bindings_parse(file_name, text, out, error)`. The parser follows keymap's line rules. Choices the plan left open: an empty button list is an error, a button repeated on one line is an error, and the parser does not reserve buttons 5 and 6.
- Deviation: `float-grep` requires every `src/` file to be in the core or excluded, so `bindings.{c,h}` sit in `tools/float_grep_exclusions.txt` as temporary lines. Step 7 must delete them when it links the module into the core. Button guard: `src/game/bindings.c fixed 3` (the count appears in the parser's message too).
- `make test-all` green, every `.sha256` unchanged, generator and metrics commands unchanged. `make test` 4.20–4.49 s, `make e2e` 0.24–0.25 s.

### Step 2.7

- Three commits: the default file and generator; the table handed to `pad_nav`; the personal file for `game`.
- Deviation: the generator (`src/cmd/bindings_generate.c`, `bindings_generate <in.cfg> <out.c>`) writes a C unit, not a header. Two reasons: a header under `tmp/` would need an include path beyond `-Isrc`, and a separately compiled definition keeps the table out of shared objects. `tmp/bindings/default.c` (from `bindings.default.cfg`) is linked into `replay`, `pad_author` and the tests. `tmp/bindings/game.c` (from the chosen file) is linked into `tmp/game` only. Both define `const bindings_t bindings_compiled`, declared `extern` in `bindings.h` and named only in `src/cmd/` and `pad_author.c`. A parse error prints `file:line: message`, exits 1 and leaves no output.
- `bindings.default.cfg` holds section 2's reconciled table, explains each mode in comments and documents the 5 + 6 reservation. The pilot reworded one sentence before the commit: keymap lines are matched by name, so buttons follow the order `keymap.cfg` ships its lines, not any line's position. `test_bindings` parses the committed file (step 6's inline copy is gone) and checks the generated table equals it.
- `bindings.c` joined the core (`ENGINE_SRCS`). Its float-grep exclusions are gone and `float_guard.txt` went to 84 files. `input_step_init` and `pad_nav_init` take `const bindings_t *`; `pad_nav_t` keeps a copy by value (no pointer field) behind `pad_nav_bindings()`, not read yet. In `bindings.c`, a length computed once avoids a `-Wstringop-overread` false positive of the `-O2` sanitize build.
- Checks with a personal `bindings.cfg` swapping 7 and 8: `make raylib` printed `bindings: bindings.cfg (personal)`. `make test-all` stayed green and `make pad-author` + `git diff --exit-code worst-case/` stayed clean. Generator and metrics outputs were identical with and without the file. A broken file (line 41 `jump.enter_aim = 6`) failed with `bindings.cfg:41: button 6 already bound in mode jump` and relinked nothing. Creating, deleting or back-dating the file regenerated the table through the stamp `tmp/bindings.source`. A no-change `make raylib` compiled nothing.
- Binary proof, nothing launched: `nm -S` and `objdump -s` of `bindings_compiled` (0x4ec bytes) in `tmp/game` and `tmp/replay` differ only in `jump.enter_aim`/`jump.open_menu` with the swap file, and are identical after deleting it and rebuilding. `nm -A --defined-only` over the 94 replay objects finds only `tmp/obj/tmp/bindings/default.o`.
- `make test` 4.26–4.69 s (same as HEAD within noise, alternating runs). `make e2e` 0.22–0.26 s. Leftover `tmp/obj-asan`/`-sanitize`/`-coverage` trees from sanitize builds add about 0.08 s to `make e2e` (make re-reads their `.d` files); this is a local artifact, not the change.

### Step 2.8

- Four commits: one button reader and count in the devtools; `pad_metrics` off `pad_jump`'s conversions; high-level presses through the table; PSX words out of the generator.
- `input_json.c` and `pad_author.c`'s `parse_buttons` use `bindings_read_button_number`. `PAD_BUTTON_COUNT` and `PAD_METRICS_BUTTON_COUNT` are gone in favour of `P8_PAD_BUTTON_COUNT`. Six explicit-link tests gained `src/game/bindings.c`. Same inputs accepted and refused.
- `pad_metrics` maps the physical D-pad itself (`pad_metrics_direction_button`, `pad_metrics_first_direction_button`), independent of any table: metrics judge buttons, and from step 16 a replay may run a fixture. `first_direction` → `first_direction_button`, `circle_dead_other` → `button_6_dead_other`. The report text `section4 dead circle presses …` is kept so metrics output stays byte-identical.
- The generator looks up each high-level press through `pad_nav_bindings()` for the mode it runs in (tech and score → panels, bar → build_bar), right before the press, and presses the action's lowest button. An unbound action or one the mode lacks fails with `mode.action`. Direction taps use `jump_*`/`pan_*`/`move_*`/`pick_*` by mode. The dead-back and empty-cone checks read the mode's back and `jump_*` masks. `tap`/`hold` stay raw. The 5 + 6 chord is the only button named in `pad_author.c`, and `src/devtools/` calls no `pad_jump` direction helper.
- Outputs byte-identical through chunk 3, also with a personal `bindings.cfg` swapping 7 and 8. Mutation proof (not committed): swapping 7 and 8 in `bindings.default.cfg` itself fails `make pad-author` at `full_game.pad:9: frame 139: menu: a row focused: not within 20 frames`. Trap: restoring that file with an older timestamp left `tmp/bindings/default.c` stale until it was deleted.
- Chunk 4 messages: `dead <mode>.back`, `jump.enter_aim: no aim mode`, `aim.leave_aim: …`, `closed by menu.close_menu`/`build_bar.close_bar`, `title.choose_row …`. Locals renamed `back_*`. Deviation: the PRD expected `dead jump.back`, but the modes log shows the back press at f17 runs in aim mode, so `dead_back.refused.pad` (renamed from `dead_circle`) expects `dead aim.back`. That line is the only output change.
- `make test` 4.27–4.57 s (HEAD 4.42 s average over alternating runs), `make e2e` 0.24–0.26 s.
