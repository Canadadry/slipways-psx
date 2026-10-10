---
title: "Bindings module: modes, actions, button reader and pure parser"
description: "The game has no data type for a control scheme, no parser for the bindings file and three copies of the button count and button-number grammar, so steps 7–16 have nothing to build on."
status: needs-triage
---

## Problem Statement

Plan 17 step 6 (Phase 2, bindings in the game). The step's text, from `docs/plan-17.md` section 3, as amended after phase 1:

> 6. **Bindings module.** `src/game/bindings.{c,h}`: the mode and action enums of section 2, `bindings_t`, `bindings_mask(bindings, mode, action)`, and a pure parser from text to table with no I/O, returning `file:line: message` errors. Unit tests cover every error of section 2 (conflict, unknown mode or action, button out of range, duplicate line) and an unbound action. Not wired into the game yet. The module also owns the single button count (for example `P8_PAD_BUTTON_COUNT` next to `P8_PAD_BUTTON_12`) and the single button-number reader (the grammar of step 1.2's note: digits only, no leading zero, 1–12). The bindings parser uses it and its unit tests cover it.

Today the control scheme exists only as hardcoded `P8_PAD_BUTTON_*` tests in `src/game/pad_nav.c` and its helpers. There is no type that says "in mode M, action A is played by buttons X and Y", no way to turn a `bindings.cfg` text into such a table, and no way to report a broken file as `file:line: message`. Every later step of the plan needs this: step 7's host generator parses the chosen file, steps 9–13 replace each read by `bindings_mask`, step 16's `replay` parses a fixture at runtime, and step 8's generator reads the default table.

Meanwhile the button count 12 is written three times (`PAD_BUTTON_COUNT` in `src/devtools/input_json.c`, `PAD_METRICS_BUTTON_COUNT` in `src/devtools/pad_metrics.h`, `KEYMAP_BUTTON_COUNT` in `src/platform/keymap.h`), and the button-number digit loop twice (`parse_pad_frame` in `input_json.c`, `parse_buttons` in `pad_author.c`). The end-of-phase-1 review flagged it. Step 6 provides the one count and the one reader. Steps 8 and 15 switch the copies over.

## Solution

A new module in `src/game/` holds the whole bindings vocabulary and its text format, with no I/O and no dependency on the rest of the game:

- the nine modes and the action names of section 2 as two enums, plus which actions each mode has;
- `bindings_t`, a plain table of button masks (one per mode and action) that a generated C file can initialise statically, on PC and PSX alike;
- `bindings_mask`, the one question the game will ask: "which buttons play action A in mode M";
- `bindings_parse`, which turns the text of a bindings file into a table, or into the first error as `file:line: message`;
- `bindings_read_button_number`, the single reader of a button number (1–12), which the parser uses and which the devtools and the keymap will use in steps 8 and 15.

The single button count `P8_PAD_BUTTON_COUNT` is added next to `P8_PAD_BUTTON_12`, with a compile-time check that it matches the top bit.

Nothing in the game calls the module yet. No frame, replay, `.pad` script or generator output changes.

## Hypotheses on step 5's outcome

Step 5 ("Inventory of button reads", PRD `docs/prd/triage/phase2-step5-inventory-button-reads.md`) is being implemented in parallel. This PRD was written from commit 7e70639, where section 2 is still unamended. The enums below follow the table as step 5's chunk 2 reconciles it. Each assumption is a hypothesis:

- **H-5-1. menu: `close_menu = 6` only, plus `open_menu = 8`.** Menu mode has both actions. `open_menu` in menu mode means "pressed again, closes the box".
- **H-5-2. build_bar: `close_bar = 6` only, plus `open_build_bar = 9`.** Build-bar mode has both actions.
- **H-5-3. The message box needs no action.** On the map, the mode's ○ action (`jump.back`, `aim.back`, `slipway.cancel_slipway`, `placement.cancel_placement`) first closes a shown message box. This is a rule of the game code (step 13), not a new action or mode. The enums do not change.
- **H-5-4. The cheat chord moves behind the cheat module** (step 13) and stays below the bindings. The bindings module knows nothing about it. The parser does not reserve buttons 5 and 6: binding 5 or 6 alone to an action is valid. Only the comment in `bindings.default.cfg` (step 7) documents the reservation.
- **H-5-5. The button guard exists** (`tools/button_guard.sh`, run by `make test`) with an allowance file whose lines read `<path> <owner> <count> | <reason>` and whose owners include `fixed`. It counts every identifier starting with `P8_PAD_BUTTON_` outside comments and strings in `src/game/`, per file, with strict equality. So `P8_PAD_BUTTON_COUNT` counts, and step 6 adds a `fixed` line for each file of the module that names it. Step 5's PRD says so ("If step 6's `bindings.c` names `P8_PAD_BUTTON_`, step 6 adds a `fixed` line").
- **H-5-6. Step 5 adds no action and no mode beyond H-5-1 and H-5-2,** and keeps every action and mode name of section 2. `pause.close_pause = 6, 11` stays one action.

**Rule if a hypothesis is wrong.** The decision is "the enums are exactly section 2's reconciled table as committed at the step's own `HEAD`". The lists below follow from it. If the committed section 2 differs (an extra or a missing action, a renamed mode), follow the committed table, adjust the enums and the full-table test, and record the difference in the step note. If the guard of H-5-5 is not there, skip the allowance line. If its format differs, follow the committed format.

## User Stories

1. As the implementer of step 7, I want a parser that takes text and a file name and returns a table or the first `file:line: message`, so that my host generator only reads the file, calls the parser and writes C.
2. As the implementer of step 7, I want `bindings_t` to be a plain struct of integer masks, so that the generated header can initialise it statically with no code at startup.
3. As the implementer of step 7, I want the mode and action names available by enum value, so that the generator can write readable designated initialisers and comments.
4. As the implementer of steps 9–13, I want `bindings_mask(bindings, mode, action)` to return the button bit mask, so that each `pressed & P8_PAD_BUTTON_X` becomes `pressed & bindings_mask(...)` with the same truth value under the default.
5. As the implementer of steps 9–13, I want to build a test table from a few lines of text through the parser, so that my swap test ("the converted mode follows the table, the neighbouring mode is unbound") is three lines and not a hand-filled array.
6. As the implementer of steps 9–13, I want `bindings_mask` to return 0 for an action the mode does not have, so that a shared read computed in a mode without that action is simply unbound, as section 2 says.
7. As the implementer of step 13, I want the mode enum to cover every mode a pre-dispatch read can be computed in (title, jump, aim, slipway, menu, pause, build_bar, placement, panels), so that I write the nav-state-to-mode function against a closed list.
8. As the implementer of step 16, I want the same parser to read a replay fixture, with the fixture's path in the message, so that a broken fixture names itself.
9. As the implementer of step 8, I want one button-number reader and one button count, so that `parse_buttons` and `input_json.c` drop their own digit loops and counts.
10. As the implementer of step 15, I want the reader to stop at the first non-digit and tell me where, so that the keymap parser checks what follows (`=`, blank) itself.
11. As a player editing `bindings.cfg`, I want one line per `mode.action = N[, N…]` with `#` comments, so that the file reads like `keymap.cfg`.
12. As a player, I want a missing line to leave the action unbound, so that what the file says is what the game does.
13. As a player, I want the same button on two actions of one mode to be an error naming the button and the mode, so that I never get two actions on one press.
14. As a player, I want the same button in different modes to be accepted, so that cross means "confirm" everywhere.
15. As a player, I want an unknown mode to be an error that quotes what I wrote, so that a typo is obvious.
16. As a player, I want an action that does not exist in that mode (for example `title.pause`) to be an error, so that I do not think I bound something the game never reads.
17. As a player, I want a button outside 1–12, a button with a leading zero, a sign or a PSX name like `cross` to be an error, so that the file speaks only in button numbers.
18. As a player, I want the same `mode.action` given twice to be an error that points to the first line, so that I know which one to delete.
19. As a player, I want an empty button list or an empty item between commas to be an error, so that "unbound" has only one spelling: no line.
20. As a player, I want a button repeated on one line to be an error, so that the format is as strict as the replays and the `.pad` scripts.
21. As a player, I want a line without `=` or without `.` to be an error, so that a half-written line never parses silently.
22. As a player, I want blank lines, comment lines, trailing comments, blanks around tokens and CRLF line ends accepted, so that the file is easy to edit on any system.
23. As a player, I want only the first error reported, with the right line number counting comment and blank lines, so that I fix the file one line at a time like `keymap.cfg`.
24. As a maintainer, I want the module to have no file I/O, no allocation, no floating point and no data pointer in a struct or at file scope, so that it passes the alloc, float and pointer guards and compiles for the PSX core later.
25. As a maintainer, I want `P8_PAD_BUTTON_COUNT` next to `P8_PAD_BUTTON_12` with a compile-time check of the top bit, so that the count and the mask can never drift.
26. As a maintainer, I want the reader to refuse a number longer than two digits without overflowing, so that `1234567890123` is just a bad button.
27. As a maintainer, I want the module's unit tests to pin every error message exactly, so that step 7's `make` failure and step 16's `replay` failure print stable text.
28. As a maintainer, I want a test that parses the whole reconciled section 2 table and checks each mask against plan 12's buttons, so that the enums are proven to hold the table step 7 will commit.
29. As a maintainer, I want the module not to be linked into the game, `replay` or the PSX build in this step, so that no frame can change.
30. As a maintainer, I want `make test` no slower than step 1's baseline, so that the fast loop stays fast.
31. As a maintainer, I want the button guard to stay green with the module's own `P8_PAD_BUTTON_COUNT` names listed as `fixed`, so that step 14 closes the guard on the two permanent exceptions it names.
32. As a reviewer, I want every name in the module to be a full English word with the `bindings_` prefix, so that it follows `CLAUDE.md`'s naming rule.

## Starting point (checked at 7e70639)

- `src/p8/input.h` defines `P8_PAD_BUTTON_1 … P8_PAD_BUTTON_12` as `1 << (N-1)`. No count is defined there.
- Copies of the count: `PAD_BUTTON_COUNT 12` (with a typedef-array compile check against `P8_PAD_BUTTON_12`) in `src/devtools/input_json.c`, `PAD_METRICS_BUTTON_COUNT 12` in `src/devtools/pad_metrics.h`, `KEYMAP_BUTTON_COUNT 12` in `src/platform/keymap.h`.
- Copies of the reader: `parse_pad_frame` in `input_json.c` and `parse_buttons` in `src/devtools/pad_author.c`. Both start on `1`–`9`, accumulate digits while the number is at most 12, then refuse a number above 12 and leave the check of what follows to the caller's context (a JSON separator or `]`, or `+` or the end).
- `src/platform/keymap.c`'s `keymap_parse` is the prior art for the line format: lines split on `\n` with an optional `\r`, `#` starts a comment, blanks trimmed, one `name = item[, item…]` per line, first error wins, `keymap_error_t { int line; char message[192]; }`, a 127-character line limit.
- `CFLAGS` is `-std=c99`: compile-time checks use the typedef-array trick already used in `input_json.c`.
- Guards that see `src/game/`: the pointer guard counts data pointers in struct fields and file-scope variables (const pointers to static data included, a ratchet nobody may raise). The data guard rejects names matching `<x>_default_table` and the like. The alloc and float guards cover the core.
- No `src/game/bindings.*` and no `tests/test_bindings.c` exist.

## Implementation Decisions

### The button count

- `P8_PAD_BUTTON_COUNT` (value 12) is defined in `src/p8/input.h` directly after `P8_PAD_BUTTON_12`, with a typedef-array compile check that `1 << (P8_PAD_BUTTON_COUNT - 1) == P8_PAD_BUTTON_12`. That is where the plan's example puts it. The runtime layer owns the bit mask, and every layer (game, devtools, platform, cmd) can include it without depending on `src/game/`.
- The bindings module uses this count for its range check and its table width. It does not define a second one.
- The existing copies stay untouched in this step. Step 8 switches `input_json.c` and `pad_author.c` over (and checks no other `*_BUTTON_COUNT 12` is left in `src/devtools/`), and step 15 drops `KEYMAP_BUTTON_COUNT`.

### The button-number reader

- `int bindings_read_button_number(const char *text, const char **end)`. It returns the button number 1–12 when `text` starts with one, and 0 otherwise.
- Grammar (step 1.2's note): one or more ASCII digits, the first in `1`–`9` (no leading zero, no sign, no blank), and a value from 1 to `P8_PAD_BUTTON_COUNT`. The reader consumes every leading digit, so `123` is one bad number, not `12` followed by `3`. Accumulation saturates once the value passes the count, so a long run of digits cannot overflow.
- Contract (in the header, since the name does not say it): on success, `*end` points just past the last digit. On failure, `*end` is set to `text`. What follows the digits is the caller's business (`,`, `]`, `+`, a blank, the end). `end` must not be NULL.
- It is the only digit loop for button numbers in the module, and the parser calls it for each item.

### Modes

`bindings_mode_t`, in section 2's order, with `BINDINGS_MODE_COUNT` last:

`BINDINGS_MODE_TITLE`, `BINDINGS_MODE_JUMP`, `BINDINGS_MODE_AIM`, `BINDINGS_MODE_SLIPWAY`, `BINDINGS_MODE_MENU`, `BINDINGS_MODE_PAUSE`, `BINDINGS_MODE_BUILD_BAR`, `BINDINGS_MODE_PLACEMENT`, `BINDINGS_MODE_PANELS`.

File spellings: `title`, `jump`, `aim`, `slipway`, `menu`, `pause`, `build_bar`, `placement`, `panels`.

### Actions

One `bindings_action_t` enum shared by every mode, since section 2 reuses names across modes (`move_up`, `open_build_bar`, `pause`, …). `BINDINGS_ACTION_COUNT` is last. The order is by first appearance in section 2, which keeps related actions together. It does not affect behaviour:

| Group | Actions (file spelling; enum `BINDINGS_ACTION_<UPPER>`) |
|---|---|
| title rows | `move_up`, `move_down`, `move_left`, `move_right`, `choose_row` |
| jump | `jump_up`, `jump_down`, `jump_left`, `jump_right`, `act_on_hovered`, `back`, `enter_aim`, `open_menu`, `open_build_bar`, `open_tech_panel`, `pause`, `select_tap_or_year` |
| aim | `pan_up`, `pan_down`, `pan_left`, `pan_right`, `probe_or_fold`, `leave_aim` |
| slipway | `build_on_target`, `cancel_slipway` |
| menu | `close_menu` |
| pause | `close_pause` |
| build_bar | `pick_left`, `pick_right`, `take_structure`, `close_bar` |
| placement | `place_structure`, `cancel_placement` |
| panels | `close_panel`, `next_panel` |

That is 35 actions. H-5-1 and H-5-2 add no name: they reuse `open_menu` and `open_build_bar` (step 5's PRD says the same).

### Which actions each mode has (the reconciled table, H-5-1 to H-5-6)

Defaults in brackets: step 7 commits them in `bindings.default.cfg`. Here they are used only by the full-table test.

- **title**: move_up [1], move_down [2], move_left [3], move_right [4], choose_row [5].
- **jump**: jump_up [1], jump_down [2], jump_left [3], jump_right [4], act_on_hovered [5], back [6], enter_aim [7], open_menu [8], open_build_bar [9], open_tech_panel [10], pause [11], select_tap_or_year [12].
- **aim**: pan_up [1], pan_down [2], pan_left [3], pan_right [4], probe_or_fold [5], back [6], leave_aim [7], open_build_bar [9], open_tech_panel [10], pause [11], select_tap_or_year [12].
- **slipway**: jump_up [1], jump_down [2], jump_left [3], jump_right [4], build_on_target [5], cancel_slipway [6], open_build_bar [9], open_tech_panel [10], pause [11].
- **menu**: move_up [1], move_down [2], move_left [3], move_right [4], choose_row [5], close_menu [6], open_menu [8], open_build_bar [9], open_tech_panel [10].
- **pause**: move_up [1], move_down [2], move_left [3], move_right [4], choose_row [5], close_pause [6, 11].
- **build_bar**: pick_left [3], pick_right [4], take_structure [5], close_bar [6], open_build_bar [9], open_tech_panel [10], pause [11].
- **placement**: pan_up [1], pan_down [2], pan_left [3], pan_right [4], place_structure [5], cancel_placement [6], open_build_bar [9], open_tech_panel [10], pause [11].
- **panels**: move_up [1], move_down [2], move_left [3], move_right [4], choose_row [5], close_panel [6], open_build_bar [9], next_panel [10].

The membership lives in one static table inside the module (a `[mode][action]` array of flags, or one bit set of actions per mode), with no pointer.

### Public interface

- `typedef struct { int masks[BINDINGS_MODE_COUNT][BINDINGS_ACTION_COUNT]; } bindings_t;`. Bit `1 << (N-1)` is button N, the same layout as `p8_input_frame_t.pad_buttons`, so the game tests `pressed & bindings_mask(...)` directly. A plain struct with no pointer can be initialised statically by step 7's generated header and passed as `const bindings_t *`. An entry for an action the mode does not have is always 0.
- `int bindings_mask(const bindings_t *bindings, bindings_mode_t mode, bindings_action_t action)`. It asserts that `bindings` is not NULL and that `mode` and `action` are in range (with messages). For an action the mode does not have, it returns 0 instead of asserting: that action is unbound in that mode by definition, and steps 9–13 may compute a shared read in a mode without the action (step 5's PRD expects "the actions are unbound anyway" in title and pause).
- `int bindings_mode_has_action(bindings_mode_t mode, bindings_action_t action)`: the membership table. The parser uses it, and so do the tests and later the generator.
- `const char *bindings_mode_name(bindings_mode_t mode)` and `const char *bindings_action_name(bindings_action_t action)`: the file spellings, from fixed-width `char` arrays (no pointer table, for the pointer guard). They return static storage that the caller never frees, and assert on an out-of-range value.
- `typedef struct { int line; char text[256]; } bindings_error_t;`. `text` is the whole `file:line: message` string, ready to print. `line` repeats the line number for tests and tools. There is no whole-file error: an empty file is valid and leaves everything unbound.
- `int bindings_parse(const char *file_name, const char *text, bindings_t *out, bindings_error_t *error)`. It returns 1 and fills `*out` on success. On the first invalid line it returns 0 and fills `*error`, and `*out` is then unusable (the keymap's contract). `file_name` is used only in the message. The caller passes the path it read, such as `bindings.cfg`, `bindings.default.cfg` or a fixture path. The function does no I/O and no allocation. It clears `*out` and `*error` first.
- The header carries the contracts above as short comments (return values and who owns what), per `CLAUDE.md`'s comment rule.

### Line format and error messages

The line format is the keymap's. Lines split on `\n`, with an optional `\r` dropped. `#` starts a comment to the end of the line. Blanks (space, tab) around tokens are trimmed. A line empty after that is skipped. Line numbers count every line from 1. A line longer than 127 characters (before comment stripping, like the keymap) is an error. The first invalid line stops the parse.

A content line is `<mode>.<action> = <button>[, <button>…]`. The left side is trimmed and split on its first `.`. Checks, in order, with the exact message after `file:line: ` (`<…>` is what the line says, quoted with `"` where it is user text):

1. Line too long: `line longer than 127 characters`.
2. No `=`: `no '=' in "<content>"`.
3. No `.` on the left side: `expected mode.action, got "<left>"`.
4. Unknown mode: `unknown mode "<mode>"`.
5. Action unknown, or not in that mode: `unknown action "<action>" in mode <mode>`.
6. Pair already given: `<mode>.<action> already given on line <k>`. The parser keeps a local `[mode][action]` line array to report `k`.
7. Empty list or empty item (`jump.back =`, `5,,6`, a trailing comma): `empty button in the line of <mode>.<action>`.
8. Bad button (anything `bindings_read_button_number` refuses, or digits followed by anything but blanks before the next comma or the end, for example `13`, `0`, `05`, `+5`, `cross`, `5x`): `bad button "<item>": expected a number from 1 to 12`. The `12` is printed from `P8_PAD_BUTTON_COUNT`.
9. Button repeated on the line: `button <n> listed twice for <mode>.<action>`.
10. Conflict in the mode: `button <n> already bound in mode <mode>`. This is the plan's own wording (`bindings.cfg:12: button 7 already bound in mode jump`). It is checked against every earlier line of that mode, and against items already read on the same line only through check 9.

Quoted user text is copied into the message truncated to fit `text`. The message never overflows.

Reasons for the choices the plan does not dictate:

- Empty lists and repeated buttons are errors, not "unbound" or harmless. The plan wants one meaning per file ("what the file says is what the game does"), and the replay and `.pad` grammars already refuse repeats (steps 1.2 and 1.3).
- Buttons 5 and 6 are not reserved by the parser (H-5-4). The chord is below the bindings, and refusing them would forbid the defaults (`choose_row = 5`, `back = 6`).
- No limit on how many buttons one action has, beyond the 12 distinct ones the conflict and repeat checks allow.

### Build and guards

- `src/game/bindings.c` is **not** added to `ENGINE_SRCS`, `CORE_SRCS`, `CART_ONLY_SRCS`, `REPLAY_SRCS`, the raylib build or `tools/psx_sources.txt` in this step ("Not wired into the game yet"). Step 7 links it where the table is needed. The only new Makefile entries are the `test_bindings` binary (`tests/test_bindings.c` plus `src/game/bindings.c`) and its name in `UNIT_TESTS`. `make test` gains one tiny binary built and run in parallel.
- The module declares no data pointer in a struct field or at file scope (name tables are `char` arrays, `bindings_error_t` holds a `char` array), so `tools/pointer_guard_exceptions.txt` gets no line. It makes no allocation and uses no floating point. No name matches the data guard's `_default_(table|…)` pattern.
- Button guard (H-5-5): `bindings.c` names `P8_PAD_BUTTON_COUNT`. `bindings.h` does not: `bindings_t` is indexed by mode and action, not by button, so the header needs no count. Add one `fixed` line for `bindings.c` with its exact count and a reason (`bindings module: button count`). Step 14 later replaces the `fixed` lines by the guard's two permanent exceptions.
- Guardrail of section 2: the step adds a button constant to `src/p8/input.h`. So, outside `make test-all`, run `make pad-author-proof`, then `make pad-author` followed by `git diff --exit-code worst-case/`, then `make pad-metrics-check` and `make pad-metrics-worst-case`. All must be unchanged (nothing uses the new define yet).

## Implementation Chunks

1. **Button count and reader.** `P8_PAD_BUTTON_COUNT` with its compile check in `src/p8/input.h`. A first `src/game/bindings.{c,h}` with `bindings_read_button_number` only. `tests/test_bindings.c` with the reader's tests, registered in the Makefile and `UNIT_TESTS`. The button guard's `fixed` line (H-5-5). Verified: `make test` green and not slower, then the four generator and metrics commands above unchanged.
2. **Enums, table and parser.** The mode and action enums, the membership table, the name functions, `bindings_t`, `bindings_mask`, `bindings_error_t` and `bindings_parse` on top of the reader. The parser tests, including the full-table test. Update the guard's count if it changed. Verified: `make test-all` green with every `.sha256` unchanged, `make test` not slower than step 1's baseline (4.27–4.58 s warm), `make e2e` not slower (0.24–0.25 s).

## Testing Decisions

- A good test goes through the public interface only: text in, a table or an error out, then masks read back with `bindings_mask`. It never inspects the membership table's layout or the parser's internals. Error tests compare the whole `text` string and `line`, because those strings are what step 7's `make` and step 16's `replay` print.
- **New test file** `tests/test_bindings.c`, prior art `tests/test_keymap.c` (inline texts, a parse call, asserts on the error line and message) and the assert-per-case style of the other `tests/test_*.c` (`int main` calling each `test_*` function).
- **Reader cases:**
  - `1` to `12` each return their value, with `*end` at the end of the digits;
  - the stop character is left to the caller: `5,6` gives 5 with `*end` at `,`, `12]` gives 12 at `]`, `5+6` gives 5 at `+`, `1x` gives 1 at `x`;
  - refused with 0 and `*end == text`: `13`, `0`, `05`, `012`, the empty string, `+5`, `-1`, ` 5`, `cross`, `123`, and a 20-digit run (no overflow);
  - a compile-time check in the test that `P8_PAD_BUTTON_COUNT` is 12 and `1 << (P8_PAD_BUTTON_COUNT - 1) == P8_PAD_BUTTON_12`.
- **Parser success cases:**
  - an empty text and a comment-only text parse, with every mask 0;
  - one line binds one action, and `bindings_mask` returns its bit;
  - several buttons on one action are ORed (`pause.close_pause = 6, 11`);
  - blanks around tokens, a trailing comment, CRLF line ends and no final newline are accepted;
  - **unbound action:** with `jump.back = 6` and no `jump.enter_aim` line, `enter_aim` in jump is 0 while `back` is 32;
  - the same button in two modes is accepted (`jump.back = 6` and `aim.back = 6`);
  - `bindings_mask` returns 0 for an action the mode does not have (`title`, `pause`);
  - **full table:** the reconciled section 2 table above, written inline in the test as file text, parses, and every mask equals the `P8_PAD_BUTTON_*` constants plan 12 tests today (for example `menu.open_menu == P8_PAD_BUTTON_8`, `build_bar.close_bar == P8_PAD_BUTTON_6`, `pause.close_pause == (P8_PAD_BUTTON_6 | P8_PAD_BUTTON_11)`). Every action a mode has is bound and every other entry is 0. This proves the enums hold H-5-1 to H-5-6. Step 7 replaces the inline text by a test that parses the committed `bindings.default.cfg`, so the inline copy does not outlive the default file's arrival;
  - a swap table (`jump.enter_aim = 8`, `jump.open_menu = 7`, nothing else) parses, with every other mode unbound: the shape steps 9–13 will use.
- **Parser error cases**, each checking the full `text` with a file name that is not `bindings.cfg` (for example `tests/e2e/swap.cfg`) to prove the name is passed through, and the `line`:
  - conflict at line 12 after eleven comment or valid lines: `…:12: button 7 already bound in mode jump`;
  - unknown mode (`jmup.back = 6`);
  - unknown action, both a name that exists nowhere (`jump.fly = 5`) and one that exists in another mode (`title.pause = 11`);
  - button out of range (`13`, `0`), leading zero (`05`), a PSX name (`cross`), digits glued to a letter (`5x`);
  - duplicate `mode.action`, the message naming the first line;
  - no `=`, no `.`, an empty list, an empty item between commas, a trailing comma;
  - a button repeated on one line;
  - a line of 128 characters (and one of 127 accepted);
  - first error wins: two bad lines report the first;
  - line numbers count blank and comment lines.
- **Name functions:** every mode and action name is non-empty and unique, and each mode's names round-trip through the parser (a line built from `bindings_mode_name` and `bindings_action_name` for every member pair parses).
- **No other test changes.** No game file includes the module, so no pad unit test, e2e baseline, end state or generator output can change. `make e2e` is run to prove it, plus the section 2 guardrail commands listed above.
- **Timing:** `make test` and `make e2e` against step 1's baseline, recorded in the step note.

## Out of Scope

- `bindings.default.cfg`, the host generator, the Makefile stamp, `.gitignore`, `input_step` handing a table to `pad_nav`, and linking the module into any binary (step 7).
- Switching `input_json.c`, `pad_author.c` and `pad_metrics` to the new count and reader, and the high-level generator commands (step 8). Switching the keymap parser (step 15).
- Any `bindings_mask` call in the game, and any change to `pad_nav.c`, `cheat.c` or the pad helpers (steps 9–13).
- The function that computes the mode from the nav state (steps 12 and 13).
- Reading a fixture from a replay header, and the personal-recording flag (steps 16 and 17).
- Amending section 2 or writing step 5's inventory (step 5). This PRD only assumes its outcome (H-5-1 to H-5-6).
- Deleting `docs/pad-modes-and-actions.md` and the README pointer (step 20).

## Further Notes

- **Trap for step 7.** Do not name the generated table `bindings_default_table`: the data guard rejects `<x>_default_table`. Something like `bindings_compiled` passes.
- **Trap for steps 9–13.** `bindings_mask` returning 0 for an action the mode does not have hides a read tagged with the wrong mode under the default table only if the wrong mode lacks the action. That is why the plan's swap test leaves every other mode unbound. A table that binds everything would hide the mistake.
- **Trap for step 8.** The reader leaves the stop character to the caller. `input_json.c` must still check for a JSON separator or `]`, and `pad_author.c` for `+` or the end, so the old "nothing glued to the number" rule survives the switch.
- The step note (`### Step 2.6`) records which hypotheses H-5-k held against the committed section 2, the guard's `fixed` line, the generator and metrics checks, and the timings.
