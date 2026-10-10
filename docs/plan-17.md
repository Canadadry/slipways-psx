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
- **The cheat chord stays fixed on buttons 5 + 6**, below the bindings: `cheat.c` and `pad_nav.c`'s `without_chord` keep hiding buttons 5 and 6 from every mode from a chord frame until both are up, whatever the bindings say. It is a dev tool. The default bindings file documents that 5 + 6 together is reserved.
- **`back` first closes a shown message box** (the year box) in jump and aim mode, as ○ does on plan 12. There is no separate message-box mode.

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
- **menu** (the box of a hovered object): move_up 1, move_down 2, move_left 3, move_right 4, choose_row 5, close_menu 6 and 8, open_build_bar 9, open_tech_panel 10.
- **pause**: move_up 1, move_down 2, move_left 3, move_right 4, choose_row 5, close_pause 6 and 11.
- **build_bar**: pick_left 3, pick_right 4, take_structure 5, close_bar 6 and 9, open_tech_panel 10, pause 11.
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
6. **Bindings module.** `src/game/bindings.{c,h}`: the mode and action enums of section 2, `bindings_t`, `bindings_mask(bindings, mode, action)`, and a pure parser from text to table with no I/O, returning `file:line: message` errors. Unit tests cover every error of section 2 (conflict, unknown mode or action, button out of range, duplicate line) and an unbound action. Not wired into the game yet.
7. **Compile-time bindings.** Add the committed `bindings.default.cfg` (section 2's table, with mode comments and the 5 + 6 reservation). A host generator in `src/cmd/` turns the chosen file into a generated header. The Makefile handles the stamp and prints its line, and `bindings.cfg` goes into `.gitignore`. `input_step` hands the table to `pad_nav`, which does not read it yet. Verified: `make` fails with `file:line` on a broken personal file, creating or deleting `bindings.cfg` regenerates the table, a no-change `make raylib` recompiles nothing, and `make test` is not slower. With a valid personal `bindings.cfg` that swaps two actions present, `make test-all` stays green and `make pad-author` followed by `git diff --exit-code worst-case/` shows nothing: only `game` compiles the personal file. Prove it on the built binaries without launching anything (for example by a symbol or table dump): with the swapped personal file, the `game` binary from `make raylib` holds the personal table and `replay` the default; after deleting the file and rebuilding, `game` holds the default again. Shared objects linked into both binaries must not carry the table.
8. **Generator high level reads the default bindings.** Each high-level command in `src/devtools/pad_author.c` (`colonise`, `probe`, `slipway`, …) takes its button from the compiled default table for the mode it runs in. `tap` and `hold` stay raw numbers. Verified like step 3: `make pad-author-proof`, then `make pad-author` followed by `git diff --exit-code worst-case/`, `make pad-metrics-check` and `make pad-metrics-worst-case`, all repeated with a swapped personal `bindings.cfg` present, and `pad_author.c` names no `P8_PAD_BUTTON_` outside the low-level path.
9. **Jump mode reads its bindings.** Every button test in `jump_step` and its helpers goes through `bindings_mask(…, MODE_JUMP, …)`, including the hold repeat. Verified: e2e unchanged, plus a unit test with a table that swaps actions in the converted mode and leaves every other mode unbound: the converted mode follows the table, and the same buttons do nothing in a neighbouring mode served by the same shared code. Without it a read tagged with the wrong mode passes, because the default puts most actions on the same button in every mode.
10. **Aim and slipway modes read their bindings**, verified the same way.
11. **Build bar and placement read their bindings**, verified the same way.
12. **Menu, panels, pause and title read their bindings**, including `pad_panel_step`'s held button and the `choose_row` arming. Verified the same way.
13. **Cross-mode tests read their bindings:** pause, the shoulders, Select, and `back` closing the message box. The mode is computed from the nav state where each test runs. Verified the same way.
14. **Guard closed.** The allowance of step 5 is down to `cheat.c` and the bindings module, and the allowance file is removed in favour of those two fixed exceptions. Verified: the guard passes on the tree and fails on a planted `P8_PAD_BUTTON_5` in `src/game/pad_nav.c` (checked once and recorded in the step note, not committed). `make test` is not slower.

### Phase 3 — Compile-time keymap

15. **Keymap compiled in.** Add the committed `keymap.default.cfg` (today's `builtin_text`, with button numbers instead of names). Reuse the host generator pattern with its own stamp and line, and delete the runtime `keymap_load` path and the C default string. Run `git rm --cached keymap.cfg` and add it to `.gitignore`. The old-format `keymap.cfg` left in the working tree is converted in place to button numbers, so local edits are kept and the build does not break. Verified: after the commit, `make raylib` from the same tree prints `keymap: keymap.cfg (personal)` and links, a broken personal `keymap.cfg` fails `make` with `file:line`, `tests/test_keymap.c` is rewritten for the parser (not deleted), and `make test` is not slower. With `keymap.cfg` moved out of the way, as on a fresh clone, `make test-all` passes and `make raylib` prints `keymap: keymap.default.cfg` and links. With the file put back, the personal line returns.

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
