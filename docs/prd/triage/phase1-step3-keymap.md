---
title: "keymap.cfg and the raylib backend"
description: "Pad mode exists in the game and the replays, but the PC game never sets a PSX button, so a keyboard or gamepad player cannot produce pad input at all."
status: needs-triage
---

## Hypotheses on step 1.2

Checked against step 1.2 as landed (its step note in `docs/plan-15.md`): every hypothesis holds. One detail: a usage error (unknown option, two paths) exits 2, not 1. `raylib_backend_init(&backend, keyboard_cursor, mode)` already takes the mode; the input step is `src/game/input_step.{c,h}`.

Step 1.2 (`docs/prd/triage/phase1-step2-mode-selection.md`) is being implemented while this PRD is written. This PRD reads only committed code (`HEAD`, which has step 1.1 but not step 1.2) and builds on step 1.2's PRD. Each assumption about step 1.2's outcome is listed here.

**Rule for the implementer:** if step 1.2 landed with a different name, signature or spelling, use what it actually landed; the behavior this PRD asks for does not change. Only a hypothesis whose *behavior* turns out false needs a note in the step note, and the fallback given with it applies.

- **H-1.2-1** The game's `main` parses `game [--mouse] [<recording.json>]` itself, opens the replay source, then resolves the session mode through the replay source, all before the window opens. Every startup failure follows one pattern: a message on stderr, the `assert(!IsWindowReady() …)` guard, exit status 1.
- **H-1.2-2** The backend is told the session mode at init: its init function takes the input mode (`p8_input_mode_t`) and the `keyboard_cursor` flag, and opens the session recorder with both. *Fallback if false (the backend never learns the mode):* the backend fills the pad mask in both modes, and the input step's mouse-mode passthrough carries it unread (H-1.2-4), since nothing reads the mask in mouse mode before step 1.4.
- **H-1.2-3** The backend's input poll is unchanged by step 1.2: it fills the mouse fields and the six `dpad` entries exactly as at `HEAD` (arrows, Z, X), and leaves the pad mask at 0.
- **H-1.2-4** The shared per-frame input step (working name `input_step`, in the game layer) takes the raw frame. In pad mode it hands the cart a frame with zero mouse, zero mouse buttons, zero `dpad` and the raw pad mask copied as is; the cheat reads × as O and ○ as X from that mask. In mouse mode the played frame is the raw frame. Its interface takes only the input frame and never depends on raylib.
- **H-1.2-5** The session recorder records the **raw** frame, before the input step, and in pad mode writes only its pad mask. So a pad mask filled by the backend reaches the recording unchanged, and a pad recording replays through the replay source with the same mask.
- **H-1.2-6** The game loop polls the backend only for frames that do not come from a recording, as at `HEAD`. A recording is replayed from its stored frames, never from the keyboard.
- **H-1.2-7** The pad-mode startup notice ("pad controls are not playable yet, use `--mouse`") exists and stays true after this step, because nothing navigates before step 1.4.
- **H-1.2-8** Step 1.2's note records whether `make raylib-vendor-setup` and `make raylib` worked in its session. If they did not, the CI `raylib` job on the pushed commit is the check, for this step too.

## Problem Statement

Plan 15 makes the game playable with a PSX-style pad, on PC through the keyboard or a gamepad. After step 1.2 the game starts in pad mode, records and replays pad frames, and runs one shared input step for the human and the replays. But the PC backend never sets a PSX button:

- A player in pad mode can press any key or gamepad button: the pad mask in the input frame stays 0, so nothing downstream (the cheat now, pad navigation from step 1.4) ever sees a press.
- A recording made in pad mode is a pad replay full of empty frames, so a human cannot record the `pad_*` scenarios the later steps need.
- There is no way to choose which key or gamepad button plays which PSX button. Keyboards differ (AZERTY, QWERTZ), gamepads differ, and the plan rejects a mapping compiled into the binary.

## Solution

- **A versioned `keymap.cfg` at the repository root** says which keyboard keys and which gamepad 0 buttons play each of the 11 PSX buttons, one line per PSX button: `button = key[, key…]`, with raylib's key and gamepad names written without their `KEY_` / `GAMEPAD_BUTTON_` prefix.
- **The game reads it at startup, before the window opens.**
  - If the file is missing, built-in defaults apply. They are byte for byte the versioned file, and the game says on stderr that it uses them.
  - If any line is invalid, the game stops with a message naming the file and the line number, and no window opens. Nothing is ever silently ignored.
- **In pad mode, every frame the backend sets the PSX buttons held** on the keyboard or on gamepad 0, through the keymap. The recorder records them, the input step hands them on, and the cheat typed as × × ○ ○ × × ○ ○ on the keyboard (X X C C X X C C) or on the gamepad works for a human.
- **Mouse mode is unchanged.** The backend leaves the pad mask at 0 there, and the arrows, Z and X keep feeding the cart as today.
- **The parser is plain C with no raylib call**, so its rules are pinned by unit tests in the window-free suite. Only the backend touches raylib.

## User Stories

1. As a player in pad mode, I want the arrow keys to press the PSX D-pad, so that I can move once pad navigation exists.
2. As a player in pad mode, I want X to press ×, C to press ○, Z to press □ and V to press △, so that the four face buttons sit side by side under my left hand.
3. As a player in pad mode, I want A and S to press L1 and R1, so that tab cycling has keys right above the face buttons.
4. As a player in pad mode, I want Enter to press Start, so that pause sits where I expect it.
5. As a player with a gamepad, I want gamepad 0's D-pad to press the PSX D-pad, so that I can play without a keyboard.
6. As a player with a gamepad, I want the right face buttons to press ×, ○, □ and △ in the PlayStation layout (bottom ×, right ○, left □, top △), so that the controls match the PSX controller the game is designed for.
7. As a player with a gamepad, I want the first shoulder buttons to press L1 and R1 and the right middle button to press Start, so that every PSX button the game uses is on the pad.
8. As a player, I want the keyboard and the gamepad to work at the same time, with no setting to switch, so that I can pick up either one mid-game.
9. As a player, I want a button held on both the keyboard and the gamepad to count as one held button, so that pressing both never produces anything strange.
10. As a player with no gamepad plugged in, I want the game to work on the keyboard alone, with no error, so that the gamepad is optional.
11. As a player, I want a gamepad plugged in during the game to start working without a restart, so that I don't lose my session.
12. As a player, I want to edit `keymap.cfg` and restart to rebind a key, so that I never need to rebuild the game.
13. As a player, I want to bind more than one key to a PSX button, so that I can keep an alternative key (for example WASD and the arrows).
14. As a player, I want the names in `keymap.cfg` to be raylib's own key and gamepad names without their prefix, so that I can look up any name in raylib's documentation.
15. As a player, I want the PSX button names in `keymap.cfg` to be the same as in the replay files (`up`, `cross`, `l1`, `start`…), so that there is one spelling for a PSX button everywhere.
16. As a player, I want blank lines, `#` comments and spaces around `=` and `,` to be accepted, so that I can lay the file out readably.
17. As a player, I want the versioned file to start with a short comment giving its syntax, so that I can edit it without reading the code.
18. As a player who deleted or never had `keymap.cfg`, I want the game to start with the default mapping, so that a missing file never blocks a game.
19. As a player without `keymap.cfg`, I want one stderr line saying the built-in defaults are used, so that I notice when my edited file is not the one being read (for example, the game started from another directory).
20. As a player with a typo in `keymap.cfg`, I want the game to refuse to start with `keymap.cfg:<line>: <reason>` on stderr, so that I find and fix the line at once.
21. As a player, I want an unknown key name to be refused with its line number, so that a misspelt key never silently does nothing.
22. As a player, I want an unknown PSX button name to be refused with its line number, so that a misspelt button never silently does nothing.
23. As a player, I want a line without `=`, or with no key after it, or with an empty item between commas, to be refused with its line number, so that a half-written line is never half-applied.
24. As a player, I want the same PSX button on two lines to be refused at the second line, so that I never wonder which line wins.
25. As a player, I want a PSX button with no line at all to be refused, naming that button, so that no PSX button is silently unreachable.
26. As a player, I want the same key or gamepad button bound twice (to two PSX buttons, or twice on one line) to be refused at the second occurrence, so that one key never presses two PSX buttons by accident.
27. As a player, I want binding Space, Page Up, Page Down, End or Escape to be refused with its line number, so that no PSX button collides with the marker key, the replay speed keys, or the key that closes the window.
28. As a player, I want a line that is too long or holds too many keys to be refused with its line number, so that the limits are explicit instead of truncating my line.
29. As a player, I want a `keymap.cfg` that exists but cannot be read to be refused, not replaced by the defaults, so that a permissions problem is not mistaken for a missing file.
30. As a player, I want `keymap.cfg` checked at startup in mouse mode too, so that a broken file is reported the first time I start the game, whatever the mode.
31. As a player in mouse mode, I want my controls exactly as before (mouse, arrows scrolling the camera, Z and X as PICO-8's buttons), so that mouse mode does not change with the keymap.
32. As a player in pad mode, I want the session recording to hold the PSX buttons I pressed, so that what I play can become a `pad_*` replay.
33. As a player, I want the cheat typed with my mapped × and ○ to fire, so that the pad cheat works for a human, not only in a test.
34. As a player replaying a recording, I want the replay to ignore `keymap.cfg`, so that a replay plays the same whatever my keymap is.
35. As a player during a replay, I want Page Up, Page Down and End to keep controlling the replay speed and Space to keep dropping markers, so that the dev controls are untouched.
36. As a maintainer, I want the keymap parser to be plain C with no raylib call, so that `tests/test_keymap.c` runs in `make test` with no window and no vendored raylib.
37. As a maintainer, I want the built-in defaults and the versioned file to be the same text, checked by a test, so that they can never drift apart.
38. As a maintainer, I want the turning of held keys into a PSX button mask to be unit-tested through a fake "is down" function, so that the backend's raylib code is a few lines with no logic of its own.
39. As a maintainer, I want the parser's numeric key codes checked against raylib's own constants when `make raylib` compiles, so that a raylib upgrade that renumbers a key fails the build instead of binding the wrong key.
40. As a maintainer, I want the parser to allocate nothing and to use fixed-size buffers, so that it adds no failure path and no `free` contract.
41. As a maintainer, I want `make test` and `make e2e` no slower, so that the loop stays fast.
42. As a maintainer, I want every `*.sha256` of `tests/e2e/` unchanged, so that I know the backend change cannot reach the replays.
43. As a future PSX porter, I want the keymap to stay on the PC side, so that the PSX driver fills the same pad mask from its controller directly, with no keymap.

## Implementation Decisions

### Keymap module (new, in the platform layer)

- **A new module next to the raylib backend, named `keymap`** (the plan's test is `tests/test_keymap.c`). It is PC-only, like the backend, but it includes no raylib header and calls no raylib function. It depends only on the runtime's input header, for the `P8_PAD_*` bits.
- **The keymap value.** For each of the 11 PSX buttons, a fixed array of bindings and a count. A binding is a device (keyboard or gamepad) and that device's raylib numeric code. No allocation anywhere in the module; the caller owns the value (a plain struct, copied freely).
  - Limits: at most 4 bindings per PSX button, at most 127 characters per line. Exceeding either is an error with the line number, never a truncation.
- **The name table.** One list of `(name, device, code)` covering raylib 6.0's whole `KeyboardKey` enum and whole `GamepadButton` enum, except `KEY_NULL` and `GAMEPAD_BUTTON_UNKNOWN`, written without their prefix. Keyboard and gamepad names never collide in raylib 6.0 (`LEFT` against `LEFT_FACE_LEFT`, `RIGHT_SHIFT` against `RIGHT_TRIGGER_1`), so a name alone identifies the device; the unit test pins the uniqueness.
  - The codes are raylib's numeric values, duplicated in plain C. The list lives in a small header shared by the parser and the backend (an X-macro list). The backend, the only translation unit that includes raylib, expands it into one compile-time check per entry against `KEY_<name>` / `GAMEPAD_BUTTON_<name>` (a C99 negative-array-size typedef, since the build is `-std=c99`). A raylib upgrade that renumbers or drops a name then fails `make raylib`.
- **PSX button names** are the replay format's own: `up`, `down`, `left`, `right`, `cross`, `circle`, `square`, `triangle`, `l1`, `r1`, `start`. The keymap module has its own 11-entry table in the `P8_PAD_*` bit order; the unit test checks it against the bits. Sharing the replay loader's table would make the platform depend on devtools for 11 strings.
- **Syntax, exactly:**
  - `#` starts a comment that runs to the end of the line. Blank lines and comment-only lines are skipped.
  - Every other line is `button = item[, item…]`. Spaces and tabs around the button, `=`, `,` and each item are ignored.
  - Names are case-sensitive: PSX buttons lowercase, raylib names uppercase as raylib writes them. A wrong case is an unknown name, reported with the line.
  - A trailing `\r` is accepted, so a file saved with Windows line endings parses.
- **Errors, each reported with its 1-based line number and a message naming the offending text:**
  - a line without `=`;
  - an unknown PSX button;
  - an empty item (nothing after `=`, `a,,b`, a trailing comma);
  - an unknown key or gamepad name;
  - a PSX button already given on an earlier line;
  - a binding (key or gamepad button) already used, on this line or an earlier one;
  - a reserved key: `SPACE` (marker), `PAGE_UP`, `PAGE_DOWN`, `END` (replay speed) and `ESCAPE` (raylib's window-close key);
  - more than 4 items on one line;
  - a line longer than 127 characters.
  - After the last line, a PSX button that never got a line is an error with line number 0 and a message naming the button.
  - The first error stops the parse. One error at a time matches the "stop at startup" rule and keeps the error value to one line number and one message.
- **Interface** (names indicative, `module_verb[_object]`):
  - *Default text:* returns the built-in default file text, a constant string. The built-in defaults are **that text parsed by the same parser**, so "the defaults are identical to the versioned file" becomes one byte comparison in the test. Parsing it can't fail; an `assert` with a message says so.
  - *Parse:* takes a NUL-terminated text and fills the keymap, or fills the error (line, message) and returns failure. On failure the keymap's content is unspecified; the caller must not use it.
  - *Load:* takes a path and returns one of three outcomes:
    - loaded from the file;
    - file missing (`fopen` fails with `ENOENT`), defaults applied;
    - invalid, with the error filled. A file that exists but cannot be opened or read is invalid with line 0 and a message saying it can't be read, never the defaults.
    The whole file is read into a fixed buffer (4 KiB, far above the default file). A larger file is invalid with line 0.
  - *Pad buttons:* takes the keymap and an "is down" function `(device, code, context) → 0/1` plus its context, and returns the `P8_PAD_*` mask of every PSX button with at least one binding down. This is where the OR across keyboard and gamepad lives, unit-tested with a fake. The backend passes a two-line raylib wrapper.
  - Trap for the implementer: the `data-guard` pattern rejects identifiers like `<x>_default_table` or `<x>_default_string` and a `void *self` parameter. Name the default-text function and the callback's context so they don't match.
- **Guards:** the module and its list header go in the float-grep exclusions as platform files, like the backend's coordinate module (one line each, with the same kind of justification). They are outside `CORE_SRCS`, `pointer-guard` (which counts `src/game/` and the entity classes only) and `alloc-guard` (core only).

### The versioned `keymap.cfg`

- At the repository root, versioned, byte for byte the built-in default text. Content, after a short header comment (syntax, the prefix rule, a pointer to raylib's key list, and that letter keys are key positions named after the US layout):

  | PSX button | Bindings |
  |---|---|
  | `up` | `UP, LEFT_FACE_UP` |
  | `down` | `DOWN, LEFT_FACE_DOWN` |
  | `left` | `LEFT, LEFT_FACE_LEFT` |
  | `right` | `RIGHT, LEFT_FACE_RIGHT` |
  | `cross` | `X, RIGHT_FACE_DOWN` |
  | `circle` | `C, RIGHT_FACE_RIGHT` |
  | `square` | `Z, RIGHT_FACE_LEFT` |
  | `triangle` | `V, RIGHT_FACE_UP` |
  | `l1` | `A, LEFT_TRIGGER_1` |
  | `r1` | `S, RIGHT_TRIGGER_1` |
  | `start` | `ENTER, MIDDLE_RIGHT` |

  This is the plan's default mapping table, with the gamepad D-pad spelled as raylib's `LEFT_FACE_*` buttons.
- The lines are in the `P8_PAD_*` bit order, the same order the replay format saves.

### Raylib backend

- **Init** takes the loaded keymap (by `const` pointer, copied into the backend struct) alongside the mode and `keyboard_cursor` of H-1.2-2, and stores the mode.
- **Input poll:**
  - The mouse fields and the six `dpad` entries are filled exactly as today (H-1.2-3), in both modes. In pad mode the input step throws them away (H-1.2-4). Not filling them in pad mode would be a second mode branch for no behavior.
  - In **pad mode**, the pad mask is the keymap's pad-buttons result. The "is down" wrapper answers `IsKeyDown(code)` for a keyboard binding and `IsGamepadAvailable(0) && IsGamepadButtonDown(0, code)` for a gamepad binding. Checking availability every frame is what makes a gamepad plugged in mid-game work.
  - In **mouse mode** the pad mask stays 0. That keeps mouse mode "reads no pad buttons beyond what the cart and the cheat read today" by construction, not by relying on every later reader to check the mode.
  - Fallback when H-1.2-2 is false: see that hypothesis.
- The replay speed keys, the Space marker and raylib's Escape stay outside the keymap, polled exactly as today. The parser refuses to bind them (see Errors).
- No analog stick, no gamepad other than index 0 (plan § Keymap).
- The poll stays the only place that reads keys. A replayed frame never goes through it (H-1.2-6), so replays never read the keymap.

### Interactive game binary

- **Startup order** (extends H-1.2-1): parse the arguments → open the replay source → resolve the mode → **load `keymap.cfg`** → load the assets → init the backend (opens the window).
  - The keymap path is `keymap.cfg`, relative to the working directory, like `pico8/gfx.json` today. The game is run from the repository root.
  - The keymap is loaded in both modes, every time the game starts, and also when a recording is being resumed. Loading it only in pad mode would let a broken file go unreported until the player switches mode.
  - **Missing file:** one stderr line, `game: keymap.cfg not found, using the built-in defaults.`, then the game goes on.
  - **Invalid file:** the game prints `game: keymap.cfg:<line>: <message>` (`game: keymap.cfg: <message>` when the line is 0) on stderr. It then hits the same `assert(!IsWindowReady() …)` guard as the other startup failures, closes the replay source and exits 1. No window opens.
- Nothing else in the loop changes: the backend fills the mask, the recorder records it (H-1.2-5), the input step hands it on (H-1.2-4).

### Build

- The keymap module joins the raylib game's source list.
- A new `test_keymap` unit binary links the test and the keymap module only, with `keymap.cfg` as a prerequisite, and joins the unit list. It is a few milliseconds of parsing, so `make test` stays as fast.
- The Makefile legend needs no new line: no new target.

### Decisions taken without a human

No `guideline` agent could be spawned in this session (no agent tool). Each decision applies the `guideline` rules: make the plan work, keep every hash safe, refuse widening, take the smaller change. Each one cites what it rests on.

- **D1. The built-in defaults are the default file text, parsed.**
  - Why: plan § Keymap "They are identical to the versioned file". One text makes that a byte comparison instead of two tables kept in step by hand.
- **D2. The keymap module lives in the platform layer, has no raylib call, and is named `keymap`.**
  - Why: plan step 1.3 "plain C, with a name table and no raylib call" and "`tests/test_keymap.c`". The raylib coordinate module is prior art for a platform helper tested without raylib.
- **D3. The numeric codes are duplicated, and the raylib build checks them at compile time.**
  - Why: the unit test can't include raylib (no `-I` for it, and raylib may not be vendored where `make test` runs). The compile-time check keeps "a key that silently does nothing is not acceptable" true across raylib upgrades, at no runtime cost.
- **D4. Every PSX button must have a line, and a binding may be used only once.**
  - Why: plan § Keymap "One line per PSX button" and "a key that silently does nothing is not acceptable". A missing button would be silently unreachable. A shared key would press two buttons.
- **D5. Space, Page Up, Page Down, End and Escape are refused as bindings.**
  - Why: plan § Keymap keeps the replay keys and the marker "outside the keymap", and Escape closes the raylib window. Binding them would make one key do two things. Refusing is a few names in a check, not a redesign.
- **D6. The keymap is loaded and validated in both modes. The pad mask is filled in pad mode only.**
  - Why: plan § Keymap "read at startup" has no mode condition. Plan § Input modes says mouse mode "reads no pad buttons beyond what the cart and the cheat read today". Keeping the mask 0 in mouse mode guarantees that at the source.
- **D7. The game binary loads the keymap and passes it to the backend init, rather than the backend loading it inside init.**
  - Why: the plan wants the error "before the window opens". At `HEAD`, the game's `main` is where every startup failure is printed and the process exits, before `InitWindow` (H-1.2-1). The backend init opens the window and has no failure path. The file is still read for the backend, by the backend's own platform module.
- **D8. The PSX button names are the replay format's.**
  - Why: one spelling per PSX button across the repository (step 1.1 note: `{ "pad": ["up", "cross"] }`). The plan names no other spelling.
- **D9. The first error stops the parse; a missing button is reported with line 0.**
  - Why: plan § Keymap "an invalid line stops the game … naming the file and the line". The startup stops at the first problem anyway. A missing button has no line, so line 0 means "the file as a whole".
- **D10. A missing file prints one notice line, but an unreadable file is an error.**
  - Why: the defaults are harmless, but a file edited in the wrong directory would silently do nothing. Only "missing" is the plan's defaults case: an unreadable existing file is not missing.
- **D11. The default text carries a short header comment.**
  - Why: the file is user-facing configuration, not code, so CLAUDE.md § Comments doesn't apply. The player editing it has no other doc until step 4.2 updates the README.
- **D12. The keyboard and the gamepad are read together, OR-ed, with no device switch.**
  - Why: plan § Keymap maps both onto the same PSX buttons and names no switch. That is the smaller change.

## Implementation Chunks

1. **Keymap module, default text and versioned file.**
   - The module (name list, parser, load, pad buttons from an "is down" function) and the versioned `keymap.cfg`.
   - The `test_keymap` unit binary and its unit-list entry. The float-grep exclusion lines.
   - All of `tests/test_keymap.c` (see Testing Decisions). `make test` green.
2. **Backend and game wiring.**
   - The backend init takes the keymap, and the poll fills the pad mask in pad mode. The compile-time code checks go in the backend.
   - The game loads `keymap.cfg` before the window (notice on missing, refusal on invalid). The keymap module joins the raylib source list.
   - `make raylib` builds, or the CI raylib job is green (H-1.2-8). `make test-all` green, every `*.sha256` unchanged.

## Testing Decisions

- **What makes a good test here.**
  - Only the keymap module's public interface: parse a text, load a path, read the default text, compute the pad mask through a fake "is down" function.
  - Assert on outcomes (the outcome value, the error's line number, that the message names the offending text, the bindings a PSX button ends up with, the mask), never on private parser state.
  - Build test texts inline as C strings. Touch files only for the load cases.
- **`tests/test_keymap.c`** (prior art: the raylib coordinate module's test, which links one platform module without raylib; and the replay-source tests for the open-outcome style):
  - **Defaults when the file is missing:** load a path under `tmp/` that does not exist → the missing outcome, and the keymap equals the default text parsed. Spot-check: `cross` holds the keyboard code of `X` and the gamepad code of `RIGHT_FACE_DOWN`, and `start` holds `ENTER` and `MIDDLE_RIGHT`.
  - **Every default line parses:**
    - The versioned `keymap.cfg`, read from the repository root, is byte for byte the default text.
    - Loading it gives the loaded outcome, and a keymap equal to the defaults.
    - Every PSX button has exactly the bindings of the plan's table: 2 each, in the `P8_PAD_*` order.
  - **An invalid line is reported with its number.** For each error kind under Errors, a small text where the bad line is not line 1 (with comments and blank lines before it, so the count is proven to include them) → failure, the expected line number, and a message containing the offending token. One case also goes through load, on a file written under `tmp/`, so the load path reports the line too.
  - **A missing PSX button:** a text with 10 valid lines → failure, line 0, message naming the missing button.
  - **An unreadable path:** loading a directory (`tmp/`) → the invalid outcome, never the defaults.
  - **Syntax tolerance:** spaces and tabs everywhere allowed, a trailing comment, `\r\n` line endings, and a last line without a newline all parse to the same keymap as the plain form.
  - **Name table:** every name is unique across both devices. `NULL` and `UNKNOWN` are absent. A few codes are spot-checked against raylib 6.0's documented values (`A` = 65, `ENTER` = 257, `UP` = 265, `RIGHT_FACE_DOWN` = 7). The full check is the compile-time one in `make raylib`.
  - **PSX button table:** the 11 names map to `P8_PAD_UP` … `P8_PAD_START` in order.
  - **Pad mask:**
    - nothing down → 0;
    - the keyboard binding of `cross` down → `P8_PAD_CROSS` only;
    - the gamepad binding of `cross` down → the same mask;
    - both down → still `P8_PAD_CROSS` once;
    - all 22 default bindings down → all 11 bits.
    The fake "is down" function reads a small set of `(device, code)` pairs held through its context.
- **Not unit-tested:** the backend's raylib wrapper and the game's load-and-refuse lines. They need raylib, and CLAUDE.md forbids running the window. They are a few lines delegating to the tested module, checked by `make raylib` building (the compile-time code checks included).
- **Regression:** `make e2e` with every mouse, `keyboard_*` and (if any exists by then) `pad_*` hash unchanged; `git diff --stat tests/e2e/` empty. The replay binary does not link the keymap, so no hash can move. `make test` and `make e2e` take no longer.

## Out of Scope

- Pad navigation, the synthetic cursor, and any `pad_*` replay (step 1.4 onward).
- Anything about the shared input step, mode selection, the recorder format or the cheat mapping (step 1.2).
- Analog sticks, gamepads other than index 0, rebinding at runtime, a key-capture UI.
- Moving the replay speed keys or the Space marker into the keymap.
- Showing the bound key names in the game's HUD or tooltips.
- The README's controls section (step 4.2).
- The PSX controller driver.
- Audio.

## Further Notes

- After this step, pad mode still does nothing visible for a human: the mask is filled, recorded and replayed, and the pad cheat fires from real keys, but nothing navigates until step 1.4. The step 1.2 startup notice stays (H-1.2-7).
- The step note lists:
  - `make raylib` built, or the CI raylib job result (H-1.2-8);
  - which step 1.2 hypotheses held as written, which were renamed, and any whose fallback applied;
  - that no `*.sha256` changed.
