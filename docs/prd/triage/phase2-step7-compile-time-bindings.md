---
title: "Compile-time bindings: bindings.default.cfg, host generator, stamp and table handed to pad_nav"
description: "The control scheme still has no file: there is no committed bindings.default.cfg, nothing turns a bindings file into a C table at build time, and the game core receives no table, so a player cannot remap and steps 8–19 have no compiled table to read."
status: needs-triage
---

## Problem Statement

Plan 17 step 7 (Phase 2, bindings in the game). The step's text, from `docs/plan-17.md` section 3:

> 7. **Compile-time bindings.** Add the committed `bindings.default.cfg` (section 2's table, with mode comments and the 5 + 6 reservation). A host generator in `src/cmd/` turns the chosen file into a generated header. The Makefile handles the stamp and prints its line, and `bindings.cfg` goes into `.gitignore`. `input_step` hands the table to `pad_nav`, which does not read it yet. Verified: `make` fails with `file:line` on a broken personal file, creating or deleting `bindings.cfg` regenerates the table, a no-change `make raylib` recompiles nothing, and `make test` is not slower. With a valid personal `bindings.cfg` that swaps two actions present, `make test-all` stays green and `make pad-author` followed by `git diff --exit-code worst-case/` shows nothing: only `game` compiles the personal file. Prove it on the built binaries without launching anything (for example by a symbol or table dump): with the swapped personal file, the `game` binary from `make raylib` holds the personal table and `replay` the default; after deleting the file and rebuilding, `game` holds the default again. Shared objects linked into both binaries must not carry the table.

After step 6 the repository has a bindings module (enums, `bindings_t`, a pure parser), but nothing uses it. The control scheme still lives only as `P8_PAD_BUTTON_*` tests in `src/game/pad_nav.c`, and the reconciled table of section 2 exists only as prose in the plan and as an inline text copy inside step 6's unit test. A player who wants a different button for "enter aim" has no file to edit. Steps 9–13 need a table already inside `pad_nav_t` to replace each read with `bindings_mask`. Step 8 needs a compiled default table for the generator. Step 17 needs the build to know whether the personal file was compiled.

The plan's rules this step must keep (section 2): the default file is the only copy of the defaults (no C string copy, no sync check). The personal `bindings.cfg` wins when it exists. Make does not notice a file appearing or disappearing, so a stamp records the chosen path. `replay` (and so `make e2e`, the tests and the generator) always compiles the defaults. The game core receives a table at init and never reads a file. Every `*.sha256` and `worst-case/` end state stays byte-identical.

## Solution

- A committed `bindings.default.cfg` at the repository root holds section 2's reconciled table, one `mode.action = N[, N…]` line per binding, with `#` comments that explain each mode, the format, the "missing line means unbound" rule and the 5 + 6 cheat-chord reservation.
- A host tool `tmp/bindings_generate`, built from `src/cmd/bindings_generate.c` and the bindings module, reads one bindings file, parses it with `bindings_parse`, and writes a generated C unit that defines the table `bindings_compiled`. On an invalid file it prints step 6's `file:line: message` and exits non-zero, so `make` stops with that line.
- Two generated units:
  - `tmp/bindings/default.c`, always from `bindings.default.cfg`. Linked into `replay`, `pad_author`, the unit tests that need a table, and the sanitizer and coverage builds of `replay`.
  - `tmp/bindings/game.c`, from the chosen file (`bindings.cfg` if it exists, else `bindings.default.cfg`). Linked into `game` only.
- The Makefile writes `tmp/bindings.source` (the chosen path), rewritten only when the path changes. `tmp/bindings/game.c` depends on the stamp, the chosen file and the generator. `make raylib` prints `bindings: bindings.cfg (personal)` or `bindings: bindings.default.cfg`.
- `/bindings.cfg` goes into `.gitignore`.
- `input_step_init` takes a `const bindings_t *` and hands it to `pad_nav_init`, which copies the table into `pad_nav_t`. `pad_nav` does not read it yet (steps 9–13 do). Every caller passes `&bindings_compiled`: `game` gets the chosen table, everything else the default.
- Step 6's inline full-table test is replaced by a test that parses the committed `bindings.default.cfg` and checks it against plan 12's buttons and against the generated default unit.

Nothing a replay presses changes meaning. No frame, `.sha256`, end state, generator output or metrics output changes.

## Hypotheses on step 6's outcome

Step 6 ("Bindings module", PRD `docs/prd/triage/phase2-step6-bindings-module.md`) is being implemented in parallel. Its chunk 1 is committed at 78f3cd6. Its chunk 2 is not. This PRD was written from 78f3cd6 and from step 6's PRD. Each assumption about step 6 is a hypothesis.

- **H-6-1. The button count and the reader are in place (checked at 78f3cd6).** `P8_PAD_BUTTON_COUNT` is in `src/p8/input.h`. `int bindings_read_button_number(const char *text, const char **end)` is in `src/game/bindings.{c,h}`. Step 7 does not use the reader directly.
- **H-6-2. Temporary float-grep exclusions (checked at 78f3cd6).** `tools/float_grep_exclusions.txt` lists `src/game/bindings.c` and `src/game/bindings.h` as outside the core, with the reason "plan 17 step 7 links it into the core and deletes this line". `tools/float_grep.sh` (part of `make test`) fails with "in the core and excluded" when a core file is also listed there.
- **H-6-3. The button guard line (checked at 78f3cd6).** `tools/button_guard_allowance.txt` has `src/game/bindings.c fixed 2 | bindings module: button count`. `bindings.h` names no `P8_PAD_BUTTON_`. Step 6's chunk 2 may change that count. Step 7 adds no `P8_PAD_BUTTON_` name under `src/game/`, so it leaves the line as step 6 leaves it.
- **H-6-4. The table type.** `typedef struct { int masks[BINDINGS_MODE_COUNT][BINDINGS_ACTION_COUNT]; } bindings_t;`. It holds no pointer and can be initialised statically. Bit `1 << (N-1)` is button N. An entry for an action the mode does not have is 0.
- **H-6-5. The enums.** `bindings_mode_t` holds `BINDINGS_MODE_TITLE … BINDINGS_MODE_PANELS` (9 modes) and then `BINDINGS_MODE_COUNT`. `bindings_action_t` holds `BINDINGS_ACTION_<UPPER>` for 35 actions and then `BINDINGS_ACTION_COUNT`. Each enumerator's name is `BINDINGS_MODE_` or `BINDINGS_ACTION_` followed by the file spelling in upper case.
- **H-6-6. The parser and the query functions.** `int bindings_parse(const char *file_name, const char *text, bindings_t *out, bindings_error_t *error)` returns 1 on success and 0 on the first invalid line, with no I/O. `bindings_error_t { int line; char text[256]; }`, where `text` is the whole `file:line: message`. `int bindings_mask(const bindings_t *, bindings_mode_t, bindings_action_t)`. `int bindings_mode_has_action(bindings_mode_t, bindings_action_t)`. `const char *bindings_mode_name(bindings_mode_t)` and `const char *bindings_action_name(bindings_action_t)` return the file spellings.
- **H-6-7. Step 6's test.** `tests/test_bindings.c` (binary `test_bindings`, built from `tests/test_bindings.c` and `src/game/bindings.c`, listed in `UNIT_TESTS`) contains a full-table test. That test writes section 2's reconciled table inline as file text, parses it, and checks every mask against the `P8_PAD_BUTTON_*` constants. Step 6's PRD says step 7 replaces it.
- **H-6-8. Not linked yet.** Step 6 adds `src/game/bindings.c` to none of `ENGINE_SRCS`, `CORE_SRCS`, `REPLAY_SRCS`, `GAME_SRCS` or `PAD_AUTHOR_SRCS`.
- **H-6-9. The name `bindings_compiled` is free.** Step 6 does not use it. Its PRD suggests it for step 7, because the data guard rejects `<x>_default_table`.

**Rule if a hypothesis is wrong.** Follow the committed step 6 code at this step's own `HEAD`: its names, signatures and the type of the masks. Adapt the generator, the test and the call sites. Record the difference in the step note. The decisions below do not depend on the exact spellings. They depend only on three things: a statically initialisable, pointer-free table type; a pure parser that returns a `file:line: message`; and enumerators that can be derived from the file spellings. If enumerators cannot be derived that way (H-6-5), the generator writes numeric indices with the names as comments.

## User Stories

1. As a player, I want a committed `bindings.default.cfg` at the repository root, so that I can see the whole control scheme in one text file.
2. As a player, I want to copy `bindings.default.cfg` to `bindings.cfg`, edit it and run `make raylib`, so that the game I build plays with my controls without my touching C code.
3. As a player, I want `bindings.cfg` gitignored, so that my personal controls never end up in a commit or a pull request.
4. As a player, I want `make raylib` to print `bindings: bindings.cfg (personal)` when my file is compiled, so that I know my edit was taken.
5. As a player, I want `make raylib` to print `bindings: bindings.default.cfg` when I have no personal file, so that I know which file the game uses.
6. As a player, I want a broken `bindings.cfg` to stop `make raylib` with `bindings.cfg:<line>: <message>`, so that I fix the exact line instead of getting a game with half my controls.
7. As a player, I want a failed generation to leave no half-written table that a later build would take as up to date, so that the next `make raylib` reports the same error until I fix it.
8. As a player, I want creating `bindings.cfg` to rebuild the game with it even when the file is older than the last build (copied with its date kept, or restored from a backup), so that the stale-build trap never shows me the defaults.
9. As a player, I want deleting `bindings.cfg` to rebuild the game with the defaults, so that removing my file really restores the default controls.
10. As a player, I want a second `make raylib` with nothing changed to recompile nothing, so that the build stays instant.
11. As a player, I want editing `bindings.cfg` to regenerate the table and relink the game, so that every change reaches the binary.
12. As a player, I want the comment at the top of `bindings.default.cfg` to explain the line format, that a missing line leaves the action unbound, and that the same button may be used in different modes but not twice in one mode, so that I can write a valid file without reading C.
13. As a player, I want the file to say that button N is the N-th button line of `keymap.cfg`, so that I can tell which physical key a number means without seeing a PSX name.
14. As a player, I want the file to say that buttons 5 and 6 pressed together are a fixed developer chord below the bindings, so that I am not surprised when that combination does nothing in any mode.
15. As a player, I want each mode introduced by a comment that says when the game is in it, so that I know which section to edit for the situation I want to change.
16. As a player, I want the comments to name the behaviours an action keeps whatever button it is on (hold repeat, pan acceleration, tap versus hold, press versus fold, a held `choose_row` not firing), so that I know what moves with the action when I move it.
17. As a player, I want the comments to say that the map mode's back or cancel action first closes a shown message box, so that I understand why my first press only closes the year box.
18. As a maintainer, I want `bindings.default.cfg` to be the only copy of the defaults, with no C string and no sync check, so that the defaults cannot drift between two places.
19. As a maintainer, I want the e2e replays, the unit tests and the generator always built against `bindings.default.cfg`, so that a personal file never changes `make test-all`, `make pad-author` or the pad-metrics checks.
20. As a maintainer, I want the table defined in a generated unit linked into one binary, never in an object shared by `game` and `replay`, so that one binary can hold the personal table and the other the default.
21. As a maintainer, I want one symbol name, `bindings_compiled`, in both generated units, so that the same symbol dump compares `game` and `replay` directly.
22. As a maintainer, I want the game core to receive the table at init through `input_step_init` and to keep its own copy, so that no core code reads a file or keeps a pointer to the caller's memory.
23. As a maintainer, I want `pad_nav_t` to hold the table by value, so that the pointer guard's ratchet is not raised and copies of the input state (the save forks of the tests) carry their table.
24. As a maintainer, I want a read-only accessor on `pad_nav` that returns its table, so that tests and later steps check what was handed over through the public interface.
25. As the implementer of steps 9–13, I want the table already inside `pad_nav_t`, so that each step only replaces reads with `bindings_mask(&nav->bindings, …)` and swaps the table in its unit tests through `input_step_init`.
26. As the implementer of step 8, I want a compiled default table available to `src/devtools/pad_author.c` as `bindings_compiled`, so that the high-level commands read their buttons from it.
27. As the implementer of step 15, I want a generator pattern I can copy (tool in `src/cmd/`, generated unit in `tmp/`, a stamp rewritten only on change, one printed line), so that the keymap follows the same shape.
28. As the implementer of step 17, I want the generated unit to be the single place that knows which file it came from, so that the "personal" flag can be added next to the table.
29. As a maintainer, I want a unit test that parses the committed `bindings.default.cfg` and checks every mask against plan 12's buttons, so that the default file is proven to reproduce today's controls.
30. As a maintainer, I want the same test to check that the generated default unit equals the parse of the file, so that the generator is proven faithful without a test of its own.
31. As a maintainer, I want the inline table text of step 6's test removed, so that no copy of the defaults outlives the default file.
32. As a maintainer, I want the bindings module linked into the core, with its two temporary float-grep exclusions deleted, so that float-grep, float-guard and alloc-guard cover it as they cover the rest of the core.
33. As a maintainer, I want the generator listed in the float-grep exclusions as a host tool, so that the partition of `src/` stays complete.
34. As a maintainer, I want the button guard unchanged by this step, so that the inventory of step 5 still matches the tree.
35. As a maintainer, I want `make test` and `make e2e` no slower than step 1's baseline, so that the fast loop stays fast.
36. As a maintainer, I want every `.sha256`, end state, `worst-case/` file and generator and metrics output unchanged, so that this step is provably a pure refactor.
37. As a reviewer, I want the binary-level proof done with `nm` and `objdump`, never by running `game`, so that the check follows `CLAUDE.md`'s rule not to launch the raylib app.
38. As a reviewer, I want the proof results (dumps compared, lines printed, timings) recorded in the step note, so that I can check the claim without rebuilding.
39. As a future PSX porter, I want the default table to be a plain generated C unit with no file I/O, so that the PSX build links the same `tmp/bindings/default.c` with no file system.
40. As a reviewer, I want the Makefile legend's `make raylib` entry to mention the printed bindings line, so that the target's output is documented where the targets are.

## Starting point (checked at 78f3cd6)

- `tmp/game` is built by one `cc` call straight from `GAME_SRCS` (`src/cmd/game.c`, the raylib backend, keymap, a few devtools, and `ENGINE_SRCS` minus `save_storage_memory.c`). Its prerequisites are those sources, `$(wildcard src/*/*.h)` and `libraylib.a`. It shares no object file with `tmp/replay`, but it compiles the same game sources.
- `tmp/replay` links `$(call objs,$(REPLAY_SRCS))`. `REPLAY_SRCS` is `src/cmd/replay.c`, `pad_metrics.c`, `screencast.c`, `state.c`, `replay_source.c` and `ENGINE_SRCS`. `replay_sanitize`, `replay_coverage` and `float-link` reuse `REPLAY_SRCS`.
- `tmp/pad_author` links `PAD_AUTHOR_SRCS` (`src/cmd/pad_author.c`, `src/devtools/pad_author.c`, `pad_metrics.c`, `state.c`, `ENGINE_SRCS`).
- `.DEFAULT_GOAL := tmp`, so a bare `make` only creates the directory. The target that compiles the chosen file is `make raylib` (and `make tmp/game`).
- `.DELETE_ON_ERROR:` is set.
- Callers of `input_step_init`: `src/cmd/game.c`, `src/cmd/replay.c`, `src/devtools/pad_author.c` (`pad_author_init`), and the tests `test_input_step`, `test_pad_nav`, `test_live_slots` (ASan objects), `test_resume_check`, `test_rng_sync`, `test_save_fuzz` and `test_save_worst_case`. `test_pad_nav` also calls `pad_nav_init` directly.
- `pad_nav_init` sets every field by hand. `input_step_init` clears the whole struct first, then calls `cheat_init` and `pad_nav_init`.
- The pointer guard counts data pointers in struct fields of `src/game/` with strict equality. `pad_nav.h` has no line. The list says no step may raise a count.
- The data guard rejects `<x>_default_(table|…)`. CLAUDE.md requires includes relative to `src/` with `-Isrc` only.
- `tools/float_grep_exclusions.txt` lists every `src/cmd/*.c` entry point as a host tool.
- `keymap.cfg` is committed and is a prerequisite of `test_keymap`, which reads it at runtime from the repository root. That is the prior art for a test reading a committed `.cfg`.
- Section 2 as amended by step 5: menu `close_menu 6` plus `open_menu 8`; build_bar `close_bar 6` plus `open_build_bar 9`; `pause.close_pause 6 and 11`; the map mode's ○ action closes a shown message box first; the cheat chord is fixed on 5 + 6 below the bindings.

## Implementation Decisions

### `bindings.default.cfg`

- At the repository root, committed. The bindings are exactly section 2's reconciled table, in section 2's mode order and in its action order within each mode:
  - **title**: move_up 1, move_down 2, move_left 3, move_right 4, choose_row 5.
  - **jump**: jump_up 1, jump_down 2, jump_left 3, jump_right 4, act_on_hovered 5, back 6, enter_aim 7, open_menu 8, open_build_bar 9, open_tech_panel 10, pause 11, select_tap_or_year 12.
  - **aim**: pan_up 1, pan_down 2, pan_left 3, pan_right 4, probe_or_fold 5, back 6, leave_aim 7, open_build_bar 9, open_tech_panel 10, pause 11, select_tap_or_year 12.
  - **slipway**: jump_up 1, jump_down 2, jump_left 3, jump_right 4, build_on_target 5, cancel_slipway 6, open_build_bar 9, open_tech_panel 10, pause 11.
  - **menu**: move_up 1, move_down 2, move_left 3, move_right 4, choose_row 5, close_menu 6, open_menu 8, open_build_bar 9, open_tech_panel 10.
  - **pause**: move_up 1, move_down 2, move_left 3, move_right 4, choose_row 5, close_pause 6, 11.
  - **build_bar**: pick_left 3, pick_right 4, take_structure 5, close_bar 6, open_build_bar 9, open_tech_panel 10, pause 11.
  - **placement**: pan_up 1, pan_down 2, pan_left 3, pan_right 4, place_structure 5, cancel_placement 6, open_build_bar 9, open_tech_panel 10, pause 11.
  - **panels**: move_up 1, move_down 2, move_left 3, move_right 4, choose_row 5, close_panel 6, open_build_bar 9, next_panel 10.
- If the section 2 committed at this step's `HEAD` differs from this list, follow the committed table and record it.
- **Header comment:**
  - the format (`mode.action = N[, N…]`, `#` comments);
  - buttons are numbered 1–12, and button N is the N-th button line of `keymap.cfg`. The comment uses no PSX button name. It must still read true after step 15 converts the keymap to numbers;
  - a missing line leaves the action unbound in that mode, and nothing falls back to these defaults;
  - the same button on two actions of one mode is an error, and the same button in different modes is normal;
  - this file is the default, and a personal `bindings.cfg` next to it replaces it entirely when present (gitignored, compiled by `make raylib` only);
  - the reservation: buttons 5 and 6 pressed together are the developer cheat chord, fixed below the bindings. From the frame both are down until both are up, neither button reaches any action of any mode, whatever this file says.
- **One comment block per mode**, saying when the game is in that mode. Use step 2.5's note: title is the difficulty rows with no game; jump, aim and slipway are the three map modes; menu is any box over the map, or a panel over the build bar; pause is when pause is shown; build_bar is the open bar; placement is carrying a structure; panels is the tech and score panels. Add only the cross-mode behaviours that concern the mode:
  - `open_menu` pressed again in menu closes the box, and `open_build_bar` pressed again closes the bar;
  - from slipway, menu and placement, `open_build_bar` and `open_tech_panel` first cancel what is held or close the box. `pause` from placement keeps the structure;
  - the map mode's back or cancel action first closes a shown message box and does nothing else;
  - in aim, `back` and `leave_aim` have the same effect;
  - the kept behaviours of section 2: jump hold-repeat, pan acceleration, Select tap versus half-second hold, probe press versus fold hold, a held `choose_row` from before the box opened picks nothing until pressed again.
- The comments must explain every mode of section 2, because step 20 deletes `docs/pad-modes-and-actions.md` on the strength of this file. Comments follow CLAUDE.md's language rule. The comment rule targets code, and this file's comments are its user documentation, so section 2 asks for them.

### Host generator `src/cmd/bindings_generate.c`

- One binary, `tmp/bindings_generate`, linked from its own object and `src/game/bindings.c` only. Usage: `bindings_generate <input.cfg> <output.c>`. A wrong argument count prints a usage line and exits 2.
- It reads the whole input with stdio into a fixed buffer (64 KiB). An unreadable or oversized file prints `<input>: cannot read` or `<input>: larger than 65536 bytes` and exits 1. It calls `bindings_parse` with the path exactly as given on the command line, so a broken personal file prints `bindings.cfg:12: button 7 already bound in mode jump`. On a parse error it prints `error.text` and a newline to stderr and exits 1.
- On success it writes the unit to `<output>.tmp` and renames it over `<output>`. A failure never leaves a partial `<output>`, and `.DELETE_ON_ERROR` remains a second net. The generator never writes `<output>` when parsing fails, so the old unit stays older than the edited input, and the next `make` retries and fails again.
- **The generated unit:**
  - a first-line comment `/* Generated by bindings_generate from <input>. Do not edit. */`;
  - `#include "game/bindings.h"` and `#include "p8/input.h"`;
  - `const bindings_t bindings_compiled = { .masks = { [BINDINGS_MODE_JUMP] = { [BINDINGS_ACTION_BACK] = P8_PAD_BUTTON_6, … }, … } };`. Designated initialisers name each enumerator, derived from `bindings_mode_name` and `bindings_action_name` in upper case (H-6-5). Each mask is written as the OR of its `P8_PAD_BUTTON_<N>` constants. Only bound entries are written, and the others are zero by static initialisation.
  - Deriving the names means a drift between the enums and the name functions breaks the compile of the generated unit instead of silently shifting a table.
- It is a host tool. `$(CC)` builds it today, since PSX is paused. A separate host compiler variable is left to the PSX work.
- Add `src/cmd/bindings_generate.c | entry point of the bindings table generator, host tool` to `tools/float_grep_exclusions.txt`.
- **Why a generated `.c` and not a generated header (deviation from the plan's wording, recorded on purpose):**
  - CLAUDE.md allows `-Isrc` only, and a header written under `tmp/` would need a new include path or a `../../tmp/` include;
  - a definition in its own unit is exactly what the step's last sentence asks for. No object shared by `game` and `replay` carries the table, because the table lives in an object only one binary links;
  - the plan's intent ("a host tool … generates a C table") is unchanged.

### Declaration of the table

- `extern const bindings_t bindings_compiled;` goes in `src/game/bindings.h`, after `bindings_t`. Its contract comment says: defined by the generated unit each binary links (the chosen file for `game`, `bindings.default.cfg` for every other binary); game code never names it and receives the table through `input_step_init`.
- Only `src/cmd/`, `src/devtools/` and `tests/` may name `bindings_compiled`. This step verifies that once with a grep, recorded in the step note. No new guard: the plan's guard list is fixed, and steps 9–13 never need the symbol.

### The table in the game core

- `void input_step_init(input_step_t *step, const bindings_t *bindings)` calls `pad_nav_init(&step->pad_nav, bindings)` after its `memset`.
- `void pad_nav_init(pad_nav_t *nav, const bindings_t *bindings)` asserts `bindings != NULL` (with a message) and copies `*bindings` into a new field `bindings_t bindings` of `pad_nav_t`. `pad_nav.h` includes `game/bindings.h`.
- **By value, not by pointer.** A pointer field would raise the pointer guard's ratchet (`pad_nav.h` has no line, and the list forbids raising a count). By value also gives copies of `input_step_t` their own table and keeps no reference to the caller's memory. The cost is `9 × 35` masks per `pad_nav_t` (1260 bytes with `int` masks), once per input state, which is fine on PC and on the PSX's 2 MB.
- `const bindings_t *pad_nav_bindings(const pad_nav_t *nav)` returns `&nav->bindings`. Contract: the pointer lives as long as `nav`, and the caller never frees it. No other function changes. `pad_nav_step` does not read the field in this step.
- **Call sites:**
  - `src/cmd/game.c` passes `&bindings_compiled`, which in `game` is the chosen table;
  - `src/cmd/replay.c` and `src/devtools/pad_author.c` pass `&bindings_compiled` (the default);
  - each test that calls `input_step_init` or `pad_nav_init` passes `&bindings_compiled` from the default unit. That is the mechanical change the guardrails allow, and no expected value changes.

### Linking the bindings module into the core

- Add `src/game/bindings.c` to `ENGINE_SRCS` (so to `CORE_SRCS`, `GAME_SRCS`, `REPLAY_SRCS` and `PAD_AUTHOR_SRCS`, all of which derive from it), not to `CART_ONLY_SRCS`.
- `pad_nav.h` now includes `bindings.h`, so `-MM` puts the header in the core in any case. With both files in the core, delete the two `src/game/bindings.*` lines from `tools/float_grep_exclusions.txt` (H-6-2). Otherwise float-grep fails with "in the core and excluded".
- The module object holds no table, only the membership flags and names. So it may be shared by `game`, `replay` and every test without breaking the "shared objects must not carry the table" rule.
- `tools/button_guard_allowance.txt` is not touched. `src/game/bindings.c fixed 2`, or whatever step 6 leaves (H-6-3), stays exact, because step 7 adds no `P8_PAD_BUTTON_` name under `src/game/`.
- The pointer guard gets no line: no pointer field, and the accessor is a function.
- The alloc guard and float guard now see `bindings.o`, which has no allocation and no floating point (step 6's contract). If either reports it, fix the module instead of adding an exception.

### Makefile

- `BINDINGS_DEFAULT_UNIT := $(TMP_DIR)/bindings/default.c` and `BINDINGS_GAME_UNIT := $(TMP_DIR)/bindings/game.c`.
- `BINDINGS_CHOSEN := $(if $(wildcard bindings.cfg),bindings.cfg,bindings.default.cfg)`.
- `$(TMP_DIR)/bindings_generate` links `$(call objs,src/cmd/bindings_generate.c src/game/bindings.c)`.
- `$(BINDINGS_DEFAULT_UNIT)` depends on `bindings.default.cfg` and the generator. It never depends on `bindings.cfg` or the stamp.
- **Stamp** `$(TMP_DIR)/bindings.source`. A rule forced on every run (a `FORCE` prerequisite) compares the file's content with `$(BINDINGS_CHOSEN)` and rewrites it only when they differ (the Linux kernel's `filechk` idiom). GNU make re-reads the file's time after the recipe, so an unchanged stamp rebuilds nothing.
- `$(BINDINGS_GAME_UNIT)` depends on the stamp, on `$(BINDINGS_CHOSEN)` and on the generator. Creating `bindings.cfg`, even with an old date, changes the stamp and regenerates. Deleting it changes the stamp back. Editing it is newer than the unit.
- `tmp/game`: add `$(BINDINGS_GAME_UNIT)` to its prerequisites and to its single `cc` command line.
- The `raylib` phony target keeps `$(TMP_DIR)/game` as prerequisite and gains one recipe line that always runs: `@echo "bindings: $(BINDINGS_CHOSEN)$(if $(wildcard bindings.cfg), (personal))"`. That gives `bindings: bindings.cfg (personal)` or `bindings: bindings.default.cfg`, on every run, even when nothing is rebuilt.
- Add `$(BINDINGS_DEFAULT_UNIT)` to `REPLAY_SRCS` and `PAD_AUTHOR_SRCS`, and to the link lines of the tests that call `input_step_init` or `pad_nav_init` (list in the starting point; `test_live_slots` through `asan_objs`), and to `test_bindings` (see Testing Decisions). The existing pattern rules compile it to `tmp/obj/tmp/bindings/default.o` (and the ASan, sanitize and coverage twins) with no new rule.
- Add `$(OBJ_DIR)/tmp/bindings/*.d` (and the twins' directories) to the `-include $(wildcard …)` lines, so that the generated objects recompile when `bindings.h` or `input.h` changes.
- `make test`'s cost: the generator and `default.c` are built once. Afterwards they are up to date and add only a few `stat` calls. `make test` and `make e2e` do not depend on the stamp or on `bindings.cfg`.
- Legend at the top of the Makefile: the `make raylib` entry says it also prints which bindings file it compiled (`bindings.cfg` when present, else `bindings.default.cfg`), and that every other target compiles `bindings.default.cfg`.

### `.gitignore`

- Add `/bindings.cfg` (anchored at the root, so that a future fixture under `tests/e2e/` named like it is never hidden).

## Implementation Chunks

1. **Default file and generator.** Commit `bindings.default.cfg`. Add `src/cmd/bindings_generate.c`, its float-grep exclusion line, the generator and `$(BINDINGS_DEFAULT_UNIT)` rules. Link the default unit into `test_bindings`, and make `bindings.default.cfg` its prerequisite. Replace step 6's inline full-table test with the file-based test of Testing Decisions. Declare `bindings_compiled` in `bindings.h`. Verified: `make test` green and not slower, and a hand run of `tmp/bindings_generate` on a copy of the default with one conflicting line prints `file:line: button N already bound in mode M`, exits 1 and writes no output (recorded in the step note).
2. **Table handed to `pad_nav`.**
   - Add `src/game/bindings.c` to `ENGINE_SRCS` and delete its two float-grep exclusions.
   - Change `input_step_init` and `pad_nav_init`, and add the `bindings` field and `pad_nav_bindings`.
   - Update every caller to pass `&bindings_compiled`. Link the default unit into `REPLAY_SRCS`, `PAD_AUTHOR_SRCS` and the listed tests.
   - At this point `game` also links `$(BINDINGS_DEFAULT_UNIT)`, so `make raylib` keeps linking.
   - Add the `input_step` handing tests.
   - Verified: `make test-all` green with every `.sha256` unchanged, `make end-state` unchanged, then the guardrail commands (`make pad-author-proof`; `make pad-author` then `git diff --exit-code worst-case/`; `make pad-metrics-check`; `make pad-metrics-worst-case`), and `make raylib` links.
3. **Personal file for `game`.**
   - Add the `BINDINGS_CHOSEN`, stamp and `$(BINDINGS_GAME_UNIT)` rules, and switch `tmp/game` from the default unit to the game unit.
   - Add the printed line, the `.gitignore` entry and the legend.
   - Run every check of the step's text (below) and record them in the step note.

## Testing Decisions

- A good test goes through public interfaces only: a file's text through `bindings_parse`, masks through `bindings_mask`, the handed-over table through `pad_nav_bindings`. It does not inspect the generated C text or `pad_nav_t`'s fields.
- **`tests/test_bindings.c`** (prior art: `tests/test_keymap.c`, which reads the committed `keymap.cfg` from the repository root):
  - Remove the inline copy of the table text from step 6's full-table test (H-6-7). In its place, a test reads `bindings.default.cfg` from the repository root, parses it with the file name `bindings.default.cfg`, and asserts success.
  - Every mask equals plan 12's button, written with the `P8_PAD_BUTTON_*` constants exactly as step 6's test had them, for example `menu.open_menu == P8_PAD_BUTTON_8`, `build_bar.close_bar == P8_PAD_BUTTON_6` and `pause.close_pause == (P8_PAD_BUTTON_6 | P8_PAD_BUTTON_11)`. The expected values stay in the test: they are plan 12's frozen behaviour, not a copy of the file's text.
  - Every action a mode has is bound in the default (`bindings_mode_has_action` implies a non-zero mask), and every other entry is 0.
  - The parsed table equals `bindings_compiled` from the linked default unit, mask by mask over every mode and action. That proves the generator writes what the parser reads.
  - Step 6's other tests are unchanged.
- **`tests/test_input_step.c`**, new cases:
  - Init with a swap table built by `bindings_parse` from two lines (`jump.enter_aim = 8`, `jump.open_menu = 7`). Then `bindings_mask(pad_nav_bindings(&step.pad_nav), BINDINGS_MODE_JUMP, BINDINGS_ACTION_ENTER_AIM) == P8_PAD_BUTTON_8`, and a mode left out of the swap (for example `aim.back`) reads 0.
  - The copy is the core's own. Overwrite the caller's table after init (for example clear it with `memset`), and the handed-over table still reads the swap.
  - Init with `&bindings_compiled` reads the default, for example `jump.enter_aim == P8_PAD_BUTTON_7`.
- **No test that `pad_nav` ignores the table.** Step 9 would have to delete it. The e2e replays (every `.sha256` unchanged), `make end-state` and the unchanged pad unit tests already prove that no behaviour moved.
- **Mechanical test changes only:** the tests listed in the starting point gain the `&bindings_compiled` argument. No expected value changes.
- **Build behaviour is verified by hand and recorded in the step note**, not by a `make test` target. Such a check would rebuild `tmp/game`, which is slow and needs raylib, and `make test` must not get slower. The checks:
  1. A broken personal `bindings.cfg`: a copy of the default whose `jump.enter_aim` line is changed to 6. `make raylib` exits non-zero and prints `bindings.cfg:<line>: button 6 already bound in mode jump`. `tmp/bindings/game.c` is not rewritten (its date does not change), and `tmp/game` is not relinked. A second `make raylib` fails the same way.
  2. A valid swap file: the default with `jump.enter_aim = 8` and `jump.open_menu = 7`. `make raylib` prints `bindings: bindings.cfg (personal)`, regenerates `tmp/bindings/game.c` and relinks. A second `make raylib` prints the line and runs no `cc` (and the date of `tmp/game` is unchanged).
  3. `touch -d '2000-01-01' bindings.cfg` after a build without it: `make raylib` still regenerates. This is the stamp trap.
  4. With the swap file present: `make test-all` green with every `.sha256` unchanged; `make pad-author` followed by `git diff --exit-code worst-case/` empty; `make pad-author-proof`, `make pad-metrics-check` and `make pad-metrics-worst-case` unchanged.
  5. **Binary proof, nothing launched.** For each of `tmp/game` and `tmp/replay`:
     - `nm -S <binary> | grep ' bindings_compiled$'` gives the address and size;
     - `objdump -s -j .rodata --start-address=<addr> --stop-address=<addr+size> <binary>` dumps the bytes. On another platform, use the equivalent of `-j .rodata` for the section `nm` reports.
     - With the swap file, the two dumps differ in exactly the two words of `jump.enter_aim` and `jump.open_menu`: `game` holds 0x80 and 0x40, `replay` holds 0x40 and 0x80.
     - After `rm bindings.cfg` and `make raylib` (which prints `bindings: bindings.default.cfg`), the two dumps are identical.
  6. **Shared objects carry no table.** `nm -A --defined-only` over every object of `$(call objs,$(REPLAY_SRCS))` defines `bindings_compiled` in `tmp/obj/tmp/bindings/default.o` only. `tmp/game` is built from sources and links `tmp/bindings/game.c` as its only definition, since a second definition would fail the link.
  7. Grep: `bindings_compiled` appears under `src/` only in `src/cmd/`, `src/devtools/` and the declaration in `src/game/bindings.h`.
  8. Timings: `make test` and `make e2e`, three warm runs each, against step 1's baseline (4.27–4.58 s and 0.24–0.25 s).
- Delete `bindings.cfg` after the checks. Never commit it (`.gitignore`).

## Out of Scope

- Any `bindings_mask` call in the game, any change to `pad_nav_step` or the pad helpers, and the function that computes the mode from the nav state (steps 9–13).
- The generator's high-level commands reading the table, and `input_json.c` and `pad_author.c` switching to the shared reader and count (step 8). In this step `pad_author.c` only passes `&bindings_compiled` to `input_step_init`.
- The keymap generator, `keymap.default.cfg`, ignoring `keymap.cfg` and deleting `keymap_load` (step 15).
- Fixtures in `replay` and the `"bindings"` header member (step 16). The `"personal"` recording flag (step 17), although the generated unit is where it will go.
- Deleting `docs/pad-modes-and-actions.md` and rewriting the README's Controls section (step 20). Until then the two may say the same thing. No sync check is added.
- A PSX build of the game, and a separate host compiler variable.
- The "replay then play" path of `src/cmd/game.c` (plan section 2's out of scope). It plays with whatever table `game` compiled.
- Any change to the button guard allowance, the cheat module or the chord.

## Further Notes

- **Trap: the stamp must not be rewritten on every run.** An unconditional `echo > tmp/bindings.source` would regenerate and relink `game` on every `make raylib` and break "a no-change `make raylib` recompiles nothing". Compare first, write only on change.
- **Trap: `make -q tmp/game` reports "out of date" because of the forced stamp rule,** even when nothing would be rebuilt. Check "recompiles nothing" by the absence of a `cc` line and an unchanged date of `tmp/game`, not with `-q`.
- **Trap: do not link `tmp/bindings/default.c` into `game`.** Two definitions of `bindings_compiled` fail the link, which is the intended guard. Do not "fix" it by making the symbol weak or `static`.
- **Trap: float-grep.** Linking `bindings.c` into the core without deleting its two exclusion lines fails `make test` with "in the core and excluded". `pad_nav.h` including `bindings.h` is enough to pull the header into the core.
- **Trap: name.** Do not call the table `bindings_default_table` or `bindings_default_*`: the data guard rejects `<x>_default_(table|…)`.
- **Trap: guardrail commands.** The step touches `src/game/pad_nav.*`, so the four generator and metrics commands of section 2 run once without and once with the swap file, all unchanged.
- The step note (`### Step 2.7`) records which hypotheses H-6-k held, the deviation "generated `.c` instead of a generated header" and why, the eight hand checks with their outputs, and the timings.
