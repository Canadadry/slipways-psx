---
title: "Keymap compiled in: keymap.default.cfg, button numbers, host generator and stamp"
description: "The keymap is the last control file the game reads at runtime, names PSX buttons, keeps a C copy of its defaults and is tracked in git, so it follows none of plan 17's compile-time rules and cannot reach the PSX."
status: needs-triage
---

## Problem Statement

Plan 17 step 15 (Phase 3, compile-time keymap). The step's text, from `docs/plan-17.md` section 3, with its amendment:

> 15. **Keymap compiled in.** Add the committed `keymap.default.cfg` (today's `builtin_text`, with button numbers instead of names). Reuse the host generator pattern with its own stamp and line, and delete the runtime `keymap_load` path and the C default string. Run `git rm --cached keymap.cfg` and add it to `.gitignore`. The old-format `keymap.cfg` left in the working tree is converted in place to button numbers, so local edits are kept and the build does not break. Verified: after the commit, `make raylib` from the same tree prints `keymap: keymap.cfg (personal)` and links, a broken personal `keymap.cfg` fails `make` with `file:line`, `tests/test_keymap.c` is rewritten for the parser (not deleted), and `make test` is not slower. With `keymap.cfg` moved out of the way, as on a fresh clone, `make test-all` passes and `make raylib` prints `keymap: keymap.default.cfg` and links. With the file put back, the personal line returns. The keymap parser reuses step 6's button-number reader and count; `KEYMAP_BUTTON_COUNT` goes.

After steps 1–13 the bindings follow every rule of section 2: one committed default file, an optional gitignored personal file, a host generator, a stamp, a line printed by `make raylib`. The keymap follows none of them:

- `tmp/game` reads `keymap.cfg` at launch through `keymap_load`, and falls back to a C string (`builtin_text` in `src/platform/keymap.c`) when the file is missing. Section 2 locks "at compile time, on every platform" and "the `.default.cfg` files are the single source of the defaults: no C string copy, no sync check". Today there are two copies, and `tests/test_keymap.c` is the sync check.
- `keymap.cfg` is tracked. A player who edits it gets a dirty tree, and their edit can end up in a commit.
- Its lines are keyed by PSX names (`cross = X, RIGHT_FACE_DOWN`). Section 2 rejects PSX button names in the files: buttons are numbered 1–12.
- It has its own button count (`KEYMAP_BUTTON_COUNT`) and name table. Step 6 made one count (`P8_PAD_BUTTON_COUNT`) and one button-number reader (`bindings_read_button_number`), meant to be reused here.
- A broken `keymap.cfg` only shows up when the game is launched, which an agent never does. With a compiled keymap, `make raylib` fails with `file:line` instead.

## Solution

- A committed `keymap.default.cfg` at the repository root holds today's default keymap, one `N = name[, name…]` line per button, `N` from 1 to 12. Its comments explain the format and point to `bindings.default.cfg` for what each button does. It names no PSX button.
- The keymap parser (`keymap_parse` in `src/platform/keymap.c`) reads the button number with step 6's `bindings_read_button_number` and counts with `P8_PAD_BUTTON_COUNT`. `KEYMAP_BUTTON_COUNT`, the PSX name table and `keymap_button_name` are deleted. Every other rule of the parser stays: the raylib names, the reserved keys, at most four names per button, no key on two buttons, every button needs a line, `#` comments, blanks, CRLF, 127-character lines.
- A host tool `tmp/keymap_generate`, built from `src/cmd/keymap_generate.c`, reads one keymap file, parses it and writes a C unit defining `const keymap_t keymap_compiled`. On an invalid file it prints `file:line: message` and exits 1, leaving no output.
- Two generated units: `tmp/keymap/default.c` from `keymap.default.cfg` (linked only into `test_keymap`), and `tmp/keymap/game.c` from the chosen file (`keymap.cfg` if present, else `keymap.default.cfg`), linked only into `tmp/game`. The stamp `tmp/keymap.source` holds the chosen path and is rewritten only when the path changes. `make raylib` prints a second line: `keymap: keymap.cfg (personal)` or `keymap: keymap.default.cfg`.
- `tmp/game` hands `&keymap_compiled` to the raylib backend. `keymap_load`, `keymap_defaults`, `keymap_default_text`, the load status enum and the runtime messages of `src/cmd/game.c` are deleted.
- `keymap.cfg` leaves the index (`git rm --cached keymap.cfg`) and goes into `.gitignore`. In this tree it was converted to numbers by the step's first commit while still tracked, so the working copy left behind is already in the new format and becomes the personal file.
- `tests/test_keymap.c` is rewritten around the parser, the committed default file and the compiled default unit.

No frame, `.sha256`, end state, generator output or metrics output changes: replays store buttons, which come after the keymap, and nothing but `tmp/game` and `test_keymap` links the keymap.

## Hypotheses on step 14's outcome

Step 14 ("Guard closed") is being implemented in parallel. This PRD was written from `HEAD` 1ab81ff, where step 13 is done and step 14 is not. Step 14's scope, as handed over: `tools/button_guard*`, `tests/test_button_guard.c` and the Makefile's `button-guard` target. Each assumption about it is a hypothesis.

- **H-14-1. No shared file but the Makefile.** Step 14 does not touch `src/platform/`, `src/cmd/`, `tests/test_keymap.c`, `keymap.cfg`, `bindings.default.cfg`, `.gitignore`, `README.md` or `tools/float_grep_exclusions.txt`.
- **H-14-2. Separate Makefile neighbourhoods.** Step 14 changes the `button-guard` legend lines (the `make button-guard ->` entry), the `BUTTON_GUARD_ALLOWANCE` variable and `button-guard` target, and possibly the `test_button_guard` rule. This step changes the `make raylib ->` legend entry, the `test_keymap` rule and the lines after it (the bindings generator block), the `raylib` block at the end and the `-include` lines. No hunk overlaps.
- **H-14-3. The guard scans `src/game/` only.** `tools/button_guard.sh` still counts `P8_PAD_BUTTON_` names under `src/game/` (with `cheat.c` and the bindings module exempt), so `src/platform/keymap.c` and the generated units are outside it whatever they name.

**Rule if a hypothesis is wrong.** Follow the committed code at this step's own `HEAD`. If a Makefile hunk conflicts, keep step 14's version of its lines and reapply this step's change around it. If the guard's scope grew to `src/platform/`, `keymap.c` names no `P8_PAD_BUTTON_` after this step anyway (it shifts `1 << i`), so nothing needs an exception. Record the difference in the step note.

## Starting point (checked at 1ab81ff)

- `src/platform/keymap.{c,h}`: `keymap_t { keymap_button_t buttons[KEYMAP_BUTTON_COUNT]; }`, each button holding up to four `keymap_binding_t { keymap_device_t device; int code; }` and a count. Pointer-free, statically initialisable. `keymap_parse(text, out, error)` with `keymap_error_t { int line; char message[192]; }`, line 0 for whole-file errors ("no line for PSX button …"). `keymap_load(path, …)` reads up to 4096 bytes and returns `KEYMAP_LOADED`, `KEYMAP_MISSING` (fills the defaults) or `KEYMAP_INVALID`. `keymap_defaults` parses `builtin_text`. `keymap_pad_buttons` turns held bindings into the pad mask through a `button_bits` table naming `P8_PAD_BUTTON_1..12`. `keymap_find_binding` maps a raylib name to a binding through `keymap_names.h`. `keymap_button_name` returns the PSX name.
- `keymap.cfg` is tracked, byte-identical to `builtin_text` (a test checks it). Its button lines, in order: `up down left right cross circle square triangle l1 r1 start select`, keys `UP DOWN LEFT RIGHT X C Z V A S ENTER RIGHT_SHIFT`, gamepad `LEFT_FACE_UP LEFT_FACE_DOWN LEFT_FACE_LEFT LEFT_FACE_RIGHT RIGHT_FACE_DOWN RIGHT_FACE_RIGHT RIGHT_FACE_LEFT RIGHT_FACE_UP LEFT_TRIGGER_1 RIGHT_TRIGGER_1 MIDDLE_RIGHT MIDDLE_LEFT`. The working copy has no local change.
- `src/cmd/game.c` loads `keymap.cfg` before the window opens, prints `game: keymap.cfg:N: …` and exits 1 when invalid, prints `game: keymap.cfg not found, using the built-in defaults.` when missing, then calls `raylib_backend_init(&backend, &keymap)`, which copies the table by value.
- `src/game/bindings.h`: `int bindings_read_button_number(const char *text, const char **end)` returns 1–`P8_PAD_BUTTON_COUNT` or 0 (digits only, no leading zero, `*end` restored on failure; the caller checks what follows). `P8_PAD_BUTTON_COUNT` (12) is in `src/p8/input.h`. `src/game/bindings.c` is in `ENGINE_SRCS`, so `tmp/game` already links it.
- `src/cmd/bindings_generate.c`: reads the input (64 KiB limit), parses, writes `<out>.tmp`, renames it to `<out>`, prints `file:line: message` and exits 1 on a parse error.
- Makefile: `$(TMP_DIR)/test_keymap` links `tests/test_keymap.c src/platform/keymap.c` and depends on `keymap.cfg` (a fresh clone without it would fail with "no rule to make target"). The bindings block: `BINDINGS_DEFAULT_UNIT`, `bindings_generate`, `BINDINGS_CHOSEN`, the `bindings.source` stamp on `FORCE`, `BINDINGS_GAME_UNIT` in `GAME_SRCS`, `raylib` printing `bindings: …`. The `-include` lines read `.d` files under `tmp/obj*/tmp/bindings/`.
- `tools/float_grep_exclusions.txt` lists `src/platform/keymap.c` ("keymap.cfg parser"), `keymap.h`, `keymap_names.h` and `src/cmd/bindings_generate.c`. Every `src/` file must be in the core or listed, or `make float-grep` fails.
- `README.md` line 30: "Keys can be changed in `keymap.cfg`." `bindings.default.cfg` lines 6–7: "Buttons are numbered 1 to 12, in the order keymap.cfg ships its button lines; keymap.cfg says which keys and gamepad buttons press each one."
- The data guard rejects names like `<x>_default_table`. `keymap_compiled` passes, mirroring `bindings_compiled`.

## User Stories

1. As a player, I want a committed `keymap.default.cfg` at the repository root, so that I can read every default key and gamepad button in one text file.
2. As a player, I want to copy `keymap.default.cfg` to `keymap.cfg`, edit it and run `make raylib`, so that the game I build reads my keys.
3. As a player, I want `keymap.cfg` gitignored, so that my personal keys never show as a change in `git status` or end up in a commit.
4. As a player, I want `make raylib` to print `keymap: keymap.cfg (personal)` when my file is compiled, so that I know my edit was taken.
5. As a player, I want `make raylib` to print `keymap: keymap.default.cfg` when I have no personal file, so that I know which keys the game uses.
6. As a player, I want the keymap line printed next to the bindings line, so that one glance tells me both files the build used.
7. As a player, I want a broken `keymap.cfg` to stop `make raylib` with `keymap.cfg:<line>: <message>`, so that I fix the line before I ever open the game.
8. As a player with an old `keymap.cfg` that still says `cross = X`, I want the build to fail on its first button line with a message saying a number from 1 to 12 was expected, so that I know the format changed and how.
9. As a player, I want a failed generation to leave no half-written keymap unit, so that the next `make raylib` reports the same error until I fix the file.
10. As a player, I want creating `keymap.cfg` to rebuild the game with it even when the file is older than the last build, so that a copied or restored file is never ignored.
11. As a player, I want deleting `keymap.cfg` to rebuild the game with the defaults, so that removing my file really restores the default keys.
12. As a player, I want editing `keymap.cfg` to regenerate the keymap and relink the game, so that every change reaches the binary.
13. As a player, I want a second `make raylib` with nothing changed to compile nothing, so that the build stays instant.
14. As a player, I want the keymap file to number buttons 1 to 12 like `bindings.default.cfg`, so that both files speak the same language.
15. As a player, I want `keymap.default.cfg`'s comments to say that `bindings.default.cfg` tells what each button does, so that I can find out what button 9 is for.
16. As a player, I want the comments to explain the line format, the raylib names, the US-layout letter positions and the `#` comment, so that I can write a valid file without reading C.
17. As a player, I want the button lines in any order, so that I can group them the way I think.
18. As a player, I want a missing button line to be an error naming the button number, so that I never build a game where a button cannot be pressed.
19. As a player, I want a key reserved by the game (Space, Page Up, Page Down, End, Escape) to stay refused, so that my keymap cannot break markers, the replay speed or closing the window.
20. As a player, I want a key bound to two buttons to stay refused, so that one key never presses two buttons.
21. As a player, I want at most four names per button to stay the limit, so that the rules I knew still hold.
22. As a player, I want a button number given twice to be an error at the second line, so that two lines never fight over one button.
23. As a player, I want `0`, `13`, `07`, `+5` or `5x` refused as a button, so that a typo never binds a key to the wrong button.
24. As a player, I want `README.md` to tell me that keys are set in `keymap.default.cfg` and overridden by `keymap.cfg`, so that the README stays true.
25. As a player, I want `bindings.default.cfg`'s comment about the keymap to name the right files, so that it no longer says buttons follow the order of `keymap.cfg`'s lines.
26. As a player on a fresh clone with no `keymap.cfg`, I want `make test-all` to pass and `make raylib` to link with the default keymap, so that a personal file is never required.
27. As a player, I want the game to stop printing "keymap.cfg not found, using the built-in defaults", so that a normal start makes no noise.
28. As the developer, I want the keymap's defaults to exist only in `keymap.default.cfg`, so that there is no C string and no sync test to keep in step.
29. As the developer, I want the keymap parser to use `bindings_read_button_number` and `P8_PAD_BUTTON_COUNT`, so that the button grammar and count live in one place.
30. As the developer, I want `KEYMAP_BUTTON_COUNT`, the PSX name table and `keymap_button_name` gone, so that no PSX word survives in the keymap module.
31. As the developer, I want the keymap generator to follow `bindings_generate`'s shape (input, parse, write to a temporary file, rename), so that both generators fail and succeed the same way.
32. As the developer, I want `keymap_compiled` defined only in generated units, so that a shared object never carries a keymap.
33. As the developer, I want only `tmp/game` to link the chosen keymap unit, so that no test or replay depends on a personal file.
34. As the developer, I want `test_keymap` to check the compiled default unit against the parse of `keymap.default.cfg`, so that the generator is covered by `make test`.
35. As the developer, I want `test_keymap` to no longer depend on `keymap.cfg`, so that a fresh clone builds it.
36. As the developer, I want the parser's error tests kept, rewritten for numbers, so that every refusal stays pinned.
37. As the developer, I want a test that button N's bindings set bit `1 << (N-1)` of the pad mask, so that the number-to-bit rule is pinned now that names are gone.
38. As the developer, I want a test that lines out of order give the same keymap, so that buttons are matched by number, not position.
39. As the developer, I want `make test` and `make e2e` not slower, so that the fast loop stays fast.
40. As the developer, I want every `.sha256`, end state and generator output unchanged, so that the step is a proven no-op for replays.
41. As the developer, I want proof on the built `tmp/game` that it holds the personal keymap with a personal file and the default without, so that the claim is checked without launching anything.
42. As the developer, I want the conversion of this tree's `keymap.cfg` to happen through a tracked commit before it leaves the index, so that the working copy is already valid when it becomes the personal file.
43. As the developer, I want the raylib backend's interface unchanged (it still takes a `const keymap_t *`), so that the change stays in `game.c` and the keymap module.

## Implementation Decisions

### File format

- One line per button: `N = name[, name…]`. `N` is read with `bindings_read_button_number` from the trimmed text before `=`. The whole trimmed token must be consumed: `5 x`, `5x`, `05`, `+5`, `0`, `13` and a name such as `cross` are refused with `bad button "<token>": expected a number from 1 to 12` (the wording of the bindings parser, with `P8_PAD_BUTTON_COUNT` in the message).
- `keymap_t.buttons[N - 1]` holds button N. Its bindings press bit `1 << (N - 1)` of `pad_buttons`. `keymap_pad_buttons` computes the bit by shifting; the `button_bits` table goes.
- Messages that named a PSX button now name the number: `button %d already given`, `empty name in the line of button %d`, `no line for button %d` (line 0). The other messages are unchanged.
- The header comment of `keymap_t` says `buttons[i]` is button `i + 1`, bit `1 << i` of `p8_input_frame_t.pad_buttons`.

### `keymap.default.cfg`

- Content: today's `builtin_text`, with `up … select` replaced by `1 … 12` in the same order and the same names after `=`. The header comments are rewritten without PSX words: which keyboard keys and gamepad 0 buttons press each of the 12 buttons; one line per button `N = name[, name…]`; the raylib name rule and the US-layout letter positions (kept); `#` starts a comment; `bindings.default.cfg` says what each button does in each mode; copy this file to `keymap.cfg` and run `make raylib` to change keys.
- It is the only copy of the defaults. No C string, no sync check.

### Keymap module (`src/platform/keymap.{c,h}`)

- Kept: `keymap_device_t`, `keymap_binding_t`, `keymap_button_t`, `keymap_t` (now sized by `P8_PAD_BUTTON_COUNT`; `keymap.h` includes `p8/input.h`), `keymap_error_t`, `keymap_parse`, `keymap_pad_buttons`, `keymap_find_binding`, `keymap_is_down_fn`, `KEYMAP_MAX_BINDINGS`, `KEYMAP_MAX_LINE_LENGTH`.
- Deleted: `KEYMAP_BUTTON_COUNT`, `keymap_button_name`, `button_names`, `button_bits`, `button_index`, `builtin_text`, `keymap_default_text`, `keymap_defaults`, `keymap_load`, `keymap_load_status_t`, `KEYMAP_MAX_FILE_SIZE` (moves to the generator).
- New declaration: `extern const keymap_t keymap_compiled;`, with a comment saying it is defined by the generated unit each linking binary carries (the chosen file for `game`, `keymap.default.cfg` for `test_keymap`), never by a source file.
- `keymap.c` includes `game/bindings.h` for the reader. This is a platform-to-game dependency on a pure function; `src/platform/save_storage_file.c` already includes game headers. Every binary that links `keymap.c` links `src/game/bindings.c` too (`tmp/game` through `ENGINE_SRCS`, `test_keymap` and `keymap_generate` explicitly).
- The parser code stays in `keymap.c` and is linked, unused, into `tmp/game`. Splitting it into its own file would add a module for no behaviour.

### Host generator (`src/cmd/keymap_generate.c`)

- `keymap_generate <input.cfg> <output.c>`, linked from `src/cmd/keymap_generate.c`, `src/platform/keymap.c` and `src/game/bindings.c`. Usage error exits 2.
- Reads at most 4096 bytes (the old `keymap_load` limit): a larger file fails with `<input>: larger than 4096 bytes`, an unreadable one with `<input>: cannot read`.
- A parse error prints `<input>:<line>: <message>`, or `<input>: <message>` for line 0 (the "no line for button N" error, about the file as a whole), and exits 1.
- Writes `<output>.tmp`, then renames it to `<output>`; on any failure removes the temporary file. A failed run leaves no output, as in `bindings_generate`.
- Output: a header comment `/* Generated by keymap_generate from <input>. Do not edit. */`, `#include "platform/keymap.h"`, then `const keymap_t keymap_compiled = { .buttons = { … } };` with one line per button 1–12, in order, each preceded by `/* button N */`, each binding written as `{ KEYMAP_KEYBOARD, <code> }` or `{ KEYMAP_GAMEPAD, <code> }` followed by `.count`. Numeric codes only: the generator does not include raylib.
- The file reading and temporary-file writing are duplicated from `bindings_generate.c` (about 40 lines). Each binary keeps its own file in `src/cmd/`, as `CLAUDE.md` asks; a shared host-I/O module for two generators is not worth a new module now. Step 21 (rework-comments) and the phase-end review may revisit it.
- `tools/float_grep_exclusions.txt` gains `src/cmd/keymap_generate.c | entry point of the keymap table generator, host tool`. The `src/platform/keymap.c` line's justification becomes `raylib backend, keymap file parser and pad mask`.

### `tmp/game` (`src/cmd/game.c`)

- The `keymap_load` block, `KEYMAP_PATH`, the invalid-keymap assert and both messages go. `raylib_backend_init(&backend, &keymap_compiled)`. `raylib_backend.{c,h}` do not change.
- `tmp/game` no longer opens any file to know its keys. Running it from another directory no longer silently drops the personal keymap.

### Makefile

- `KEYMAP_DEFAULT_UNIT := $(TMP_DIR)/keymap/default.c`, built from `keymap.default.cfg` and `$(TMP_DIR)/keymap_generate`, placed next to the bindings generator rules.
- `$(TMP_DIR)/keymap_generate` links `$(call objs,src/cmd/keymap_generate.c src/platform/keymap.c src/game/bindings.c)`.
- `$(TMP_DIR)/test_keymap` links `tests/test_keymap.c src/platform/keymap.c src/game/bindings.c $(KEYMAP_DEFAULT_UNIT)` and depends on `keymap.default.cfg`. The `keymap.cfg` prerequisite goes.
- In the `raylib` block: `KEYMAP_CHOSEN := $(if $(wildcard keymap.cfg),keymap.cfg,keymap.default.cfg)`; `$(TMP_DIR)/keymap.source: FORCE`, rewritten only when the chosen path changes (same recipe as `bindings.source`, reusing the existing `FORCE`); `KEYMAP_GAME_UNIT := $(TMP_DIR)/keymap/game.c` depending on `$(KEYMAP_CHOSEN)`, the stamp and the generator; `KEYMAP_GAME_UNIT` added to `GAME_SRCS`.
- `raylib` prints, after the bindings line: `keymap: $(KEYMAP_CHOSEN)` with ` (personal)` when `keymap.cfg` exists.
- The `-include` lines for `$(OBJ_DIR)` add `$(OBJ_DIR)/tmp/keymap/*.d` (the default unit's object lives there). The asan, sanitize and coverage trees never build a keymap object, so their lines stay.
- Legend, `make raylib ->` entry: it prints the bindings and keymap files it compiled, `<name>.cfg` when present, else `<name>.default.cfg`; every other target compiles `bindings.default.cfg`, and only `test_keymap` compiles `keymap.default.cfg`.
- No change to `make test`'s list or `make e2e`: `test_keymap` is already in `UNIT_TESTS`.

### Git and docs

- Chunk 1 rewrites the tracked `keymap.cfg` to button numbers (it must equal the converted `builtin_text`, as the existing test checks). Chunk 2 runs `git rm --cached keymap.cfg` and adds `/keymap.cfg` to `.gitignore` next to `/bindings.cfg`. The working copy stays, already converted, and is the personal file from then on. This is the step's "converted in place": the conversion travels in a commit, so the tree's local copy needs no separate script. If the working copy differs from `HEAD` when chunk 1 starts (a local edit), convert only the left side of its button lines (names to numbers, one-off `sed`, not committed) and keep the rest.
- `README.md` line 30 becomes: keys are set in `keymap.default.cfg`; copy it to `keymap.cfg` to change them, then run `make raylib`. The rest of the Controls section is step 20's.
- `bindings.default.cfg` lines 6–7 become: buttons are numbered 1 to 12; `keymap.default.cfg`, or a personal `keymap.cfg`, says which keys and gamepad buttons press each one. The table does not change: `tmp/bindings/default.c` must be byte-identical before and after (`cmp`).

## Implementation Chunks

1. **The keymap reads button numbers.** Parser on `bindings_read_button_number` and `P8_PAD_BUTTON_COUNT`, PSX names, `KEYMAP_BUTTON_COUNT`, `keymap_button_name` and `button_bits` gone, messages naming numbers. `builtin_text` and the tracked `keymap.cfg` converted together (still identical). `keymap_load` and `game.c` unchanged. `test_keymap` links `src/game/bindings.c`; its tests move to numbers (see Testing Decisions). `make test-all` green; `make raylib` links.
2. **The keymap is compiled in.** `keymap.default.cfg` (from `builtin_text`, header comments rewritten), `src/cmd/keymap_generate.c`, both generated units, the stamp, the `keymap:` line, `GAME_SRCS`, `game.c` on `keymap_compiled`, `keymap_load`/`keymap_defaults`/`keymap_default_text`/`builtin_text` deleted, `test_keymap` rewritten for the file and the compiled unit, float-grep exclusions, `git rm --cached keymap.cfg`, `.gitignore`, the README line, the `bindings.default.cfg` comment. Then the hand checks of Testing Decisions.

## Testing Decisions

- A good test drives the parser through `keymap_parse`, `keymap_pad_buttons` and `keymap_find_binding` and checks what they return. It never inspects private tables. The generator is covered through its output: the compiled default unit must equal the parse of the committed file.
- Module tested: the keymap module, in `tests/test_keymap.c` (rewritten, not deleted). Prior art: `tests/test_bindings.c`, which reads `bindings.default.cfg` from the repository root and checks `bindings_compiled` against its parse (`test_the_compiled_default_table_is_the_parse_of_the_default_file`).
- Chunk 1, in `test_keymap.c`:
  - `PREAMBLE` and `plain_text` use `1 = …` to `12 = …`.
  - The unknown-PSX-button test becomes a bad-button test: `13 = DOWN`, `0 = DOWN`, `02 = DOWN`, `+2 = DOWN`, `2x = DOWN`, `2 x = DOWN` and the old-format `down = DOWN` each fail at line 4 naming the token.
  - The given-twice, empty-item, unknown-name, reserved-key, more-than-four, long-line and blanks/CRLF tests keep their cases with numbers.
  - The missing-line test removes the `8 = ` and `12 = ` lines and expects line 0 naming `8` and `12`.
  - `test_psx_button_names_follow_the_pad_bits` becomes a test that, for each N, holding the first binding of button N in the defaults gives exactly `1 << (N - 1)`, and that `P8_PAD_BUTTON_12 == 1 << (P8_PAD_BUTTON_COUNT - 1)`.
  - New: a text listing the 12 lines in reverse order parses to the same `keymap_t` as `plain_text`.
  - The versioned-file test and the load tests stay in this chunk (the file is still tracked and loaded).
- Chunk 2, in `test_keymap.c`:
  - A helper reads `keymap.default.cfg` from the repository root (the tests run there, as `test_bindings` does) and parses it.
  - `keymap_compiled` (from `tmp/keymap/default.c`) equals the parse of `keymap.default.cfg` (`memcmp`; the struct has no padding and both sides are zero-filled).
  - The default-table test (two names per button, keyboard then gamepad, the expected raylib names) and the button-5 test read the parsed file instead of `keymap_defaults`.
  - The missing-line test edits a copy of the file's text instead of `keymap_default_text()`.
  - `keymap_pad_buttons` tests use the parsed file.
  - Deleted with the code they test: the versioned-file test, `test_missing_file_loads_the_defaults`, `test_unreadable_path_is_invalid_not_the_defaults`, `test_file_over_four_kib_is_invalid`, `test_invalid_file_reports_the_line_through_load`. The size limit moves to the generator and is checked once by hand.
  - The name-table test (`109 + 17` unique names, raylib codes) stays.
- Hand checks for the step note (not committed). Run them on a warm build:
  1. `make test-all` green, every `.sha256` unchanged.
  2. After the commit, in this tree (personal `keymap.cfg` present, converted): `make raylib` prints `bindings: bindings.default.cfg` and `keymap: keymap.cfg (personal)` and links. A second `make raylib` compiles nothing.
  3. A broken `keymap.cfg` (for example a line `13 = Q`) fails `make raylib` with `keymap.cfg:<line>: bad button "13": expected a number from 1 to 12`, relinks nothing and leaves no `tmp/keymap/game.c.tmp`. An old-format copy (`up = UP, …`) fails at its first button line the same way. A 5000-byte file fails with `keymap.cfg: larger than 4096 bytes`.
  4. With `keymap.cfg` moved out of the tree: `make test-all` passes and `make raylib` prints `keymap: keymap.default.cfg` and links. Putting it back with an old date (`touch -d 2000-01-01`) prints the personal line again and regenerates `tmp/keymap/game.c` through the stamp.
  5. Binary proof, nothing launched: with a personal `keymap.cfg` that swaps the keys of buttons 7 and 8 (`Z` and `V`), `nm -S` and `objdump -s` of `keymap_compiled` in `tmp/game` differ from the default build only in those two entries; after deleting the file and rebuilding, they are identical to the default. `nm -A --defined-only` over `tmp/replay`'s objects finds no `keymap_compiled`.
  6. `tmp/bindings/default.c` is byte-identical before and after the `bindings.default.cfg` comment edit.
  7. `git grep -n KEYMAP_BUTTON_COUNT` is empty. `git grep -n -i -E 'psx|cross|circle|square|triangle|\bl1\b|\br1\b' -- src/platform/keymap.c src/platform/keymap.h src/cmd/keymap_generate.c keymap.default.cfg tests/test_keymap.c` finds nothing. `git ls-files keymap.cfg` is empty and `git check-ignore keymap.cfg` succeeds.
  8. `make test` and `make e2e` timed over alternating runs against `HEAD` before the step: not slower.
- The pad-author and pad-metrics proofs are not required: the step touches no `src/game/pad_*`, `src/devtools/pad_*`, `src/devtools/input_json.c` or button constant, and check 6 proves the bindings table unchanged.

## Out of Scope

- Any change to the default keys or to the parser's rules other than the button key (reserved keys, four names, raylib names stay).
- The bindings side: `bindings.default.cfg`'s table, the bindings generator, `replay` fixtures (step 16), the personal-recording flag (step 17). A personal `keymap.cfg` needs no recording flag (section 2).
- The README's Controls section beyond line 30, and `docs/pad-modes-and-actions.md` (step 20).
- Comment rework of the keymap module and the generators beyond the lines this step writes (step 21).
- The stale default unit when `keymap.default.cfg` is restored with an older date (step 2.8's trap for the bindings): same Makefile shape, same limit, not fixed here.
- PSX: it has no keymap (pad buttons come from the hardware), so nothing of this step reaches it.
- Audio (`CLAUDE.md`).

## Further Notes

- Other clones of the repository: pulling chunk 2 deletes an unmodified tracked `keymap.cfg` from their working tree (git removes a file the pulled commit untracks), and the build then uses `keymap.default.cfg`, which holds the same keys. A clone with a locally edited `keymap.cfg` has git refuse the pull until the edit is moved aside; the user then converts their copy to numbers (names to `1 … 12` on the left side) and puts it back as `keymap.cfg`. The step note says this in one line.
- Never launch `tmp/game` (`CLAUDE.md`). Every check above is a build, a test or a binary dump.
- The step note (`### Step 3.15`) records which hypotheses H-14-k held, the two commits, the eight hand checks with their outputs and the timings.
