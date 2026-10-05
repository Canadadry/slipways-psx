---
title: "Pad frames in the input layer and the replay format"
description: "The input frame and the replay files only know the mouse and PICO-8's 6 buttons, so no pad input can be recorded, replayed or guarded by make e2e."
status: needs-triage
---

## Problem Statement

Plan 15 makes the game playable with a PSX-style pad (D-pad, ×○□△, L1/R1, Start). Every later step of the plan (mode selection, keymap, pad navigation, the 11 `pad_*` replays) needs pad input to exist as data first. Today the input frame carries only the mouse position, the mouse buttons and the 6 PICO-8 buttons; the replay format stores only that; and the e2e guard counts a replay's frames by counting `"mouse_x"` keys, so a replay without mouse fields would count as 0 frames and slip past the 180/600-frame limit. A developer cannot write, load or save a single pad frame, and the guard cannot keep pad replays honest.

## Solution

The input frame gains the PSX button set. A replay file may declare `"input": "pad"` in its header; its frames then list the PSX buttons held during that frame, and nothing else. A file without `"input"` (or with `"input": "mouse"`) is a mouse replay exactly as today, so the 30 existing replays load, save and hash identically. The e2e guard recognises a pad replay, counts its frames correctly so the frame limits apply, and rejects it when it contains any mouse-mode field.

No game behaviour changes in this step: nothing reads the pad buttons yet, and no pad replay is added to `tests/e2e/`.

## User Stories

1. As a port developer, I want the input frame to carry Up, Down, Left, Right, ×, ○, □, △, L1, R1 and Start, so that later steps can drive pad navigation from it.
2. As a port developer, I want each PSX button to have a named constant, so that code reads `cross` and not a magic bit.
3. As a port developer, I want Select, L2 and R2 absent from the button set, so that nobody builds on buttons the plan says are unused.
4. As a replay author, I want to declare `"input": "pad"` in a replay's header, so that the file's mode is explicit.
5. As a replay author, I want a pad frame to list the held buttons by name, so that I can read and hand-edit a replay.
6. As a replay author, I want an empty list to mean "nothing held", so that idle frames are trivial to write.
7. As a replay author, I want a file without `"input"` to stay a mouse replay, so that the 26 mouse replays and the 4 `keyboard_*` replays need no edit.
8. As a replay author, I want `"input": "mouse"` accepted as an explicit spelling of a mouse replay, so that the header the plan describes is valid for both modes.
9. As a replay author, I want an unknown `"input"` value to fail the load, so that a typo never silently plays as a mouse replay.
10. As a replay author, I want an unknown button name in a pad frame to fail the load, so that a misspelt button never silently does nothing.
11. As a replay author, I want a button listed twice in one frame to fail the load, so that every frame has exactly one spelling.
12. As a replay author, I want a pad frame without its button list to fail the load, so that a truncated frame is caught.
13. As a replay author, I want a pad frame that carries a mouse or `dpad` field to fail the load, so that the two modes never mix inside a file.
14. As a replay author, I want a pad replay that also declares `"keyboard_cursor": true` to fail the load, so that the keyboard-cursor flag stays a mouse-mode concept until it is deleted.
15. As a replay author, I want a failed load to leave the previously loaded input untouched, as mouse loads already do, so that error handling stays uniform.
16. As a tool writer, I want the loaded input to expose its mode, so that the replay driver of step 1.2 can pick the mode from the header.
17. As a tool writer, I want saving pad frames to write a pad replay that loads back to the same frames, so that recording and playback agree.
18. As a tool writer, I want saved pad frames to list buttons in one fixed order, so that two recordings of the same input are byte-identical.
19. As a tool writer, I want appending pad frames one at a time to produce exactly the bytes a single save of all frames produces, as mouse appends already do, so that the session recorder can switch formats in step 1.2 without a new crash-safety story.
20. As a tool writer, I want saving or appending a mouse replay to stay byte-identical to today, so that no recorder output and no hash moves.
21. As a maintainer, I want `make e2e` to count a pad replay's frames, so that a pad replay over 180 frames not listed in the exceptions is rejected.
22. As a maintainer, I want the 600-frame exception list to work for pad replays as it does for mouse ones, so that `pad_game_end` can use it if it must.
23. As a maintainer, I want `make e2e` to reject a pad replay containing `mouse_x`, `mouse_y`, `mouse_buttons`, `dpad` or `keyboard_cursor`, and name the file, so that pad coverage can never secretly depend on the mouse.
24. As a maintainer, I want the guard to keep checking companion files and the `keyboard_*` rules exactly as before, so that nothing guarded today is lost.
25. As a maintainer, I want every `*.sha256` in `tests/e2e/` unchanged, so that I know the step changed no behaviour.
26. As a maintainer, I want `make test` and `make e2e` no slower, so that the loop stays fast.
27. As a future PSX porter, I want the button set to match the PSX controller, so that the PSX driver fills the same frame the PC does.

## Implementation Decisions

- **Input frame.** The emulated-runtime input frame gains one integer field holding a bitmask of held PSX buttons, appended after the existing fields so positional initialisers in existing code and tests keep compiling and mean the same thing. Eleven constants name the bits, in this order: Up, Down, Left, Right, Cross, Circle, Square, Triangle, L1, R1, Start (runtime-prefixed, e.g. a `P8_PAD_CROSS`-style name). A mask of 0 is "nothing held". The existing mouse fields and the 6-entry `dpad` array stay; in a pad frame they are all zero.
- **Input mode.** A two-value mode type (mouse, pad) lives next to the input frame; mouse is the zero value. The loaded-input struct gains a mode field set by the loader; init and free reset it to mouse, as they do for `keyboard_cursor`.
- **`p8_btn` and the mouse reader are not changed.** They keep reading `dpad` and the mouse fields; a pad frame therefore reads as no PICO-8 button and a mouse at (0,0) with nothing pressed. Routing pad buttons anywhere is step 1.2's job.
- **Replay header.** The loader looks for an `"input"` member before `"frames"` (same scoping rule as `keyboard_cursor`: a member after `"frames"` is not read). Values: absent or `"mouse"` → mouse; `"pad"` → pad; anything else → load fails. `"input": "pad"` together with `"keyboard_cursor": true` → load fails.
- **Pad frame JSON.** One object per frame with exactly one member: `"pad"`, an array of button-name strings. Names, which are also the canonical save order: `"up"`, `"down"`, `"left"`, `"right"`, `"cross"`, `"circle"`, `"square"`, `"triangle"`, `"l1"`, `"r1"`, `"start"`. The loader accepts any order. It fails on: a missing `"pad"` member, a non-string entry, an unknown name, a duplicated name, or any of `mouse_x`, `mouse_y`, `mouse_buttons`, `dpad` inside a pad frame. A mouse frame inside a mouse replay is parsed exactly as today (a `"pad"` member in a mouse replay is not looked for and not rejected — mouse parsing is left byte-for-byte as is).
- **Saved shape.** A pad replay is written as `{`, then `"input": "pad",` on its own line, then `"frames": [`, one frame per line such as `{ "pad": ["up", "cross"] }`, and the same closing as today. Mouse output is unchanged byte for byte, including the absence of any `"input"` member (as with `keyboard_cursor`, the absent member stays the only spelling written for a mouse replay).
- **Save/append interface.** Both functions gain an input-mode parameter alongside the existing `keyboard_cursor` one; passing pad together with a set `keyboard_cursor` is a caller error caught by an `assert`. The frame-line buffer and the append tail buffer are sized so the longest pad line (all 11 buttons) fits; append stays a single write of the whole tail. The session recorder and every other existing caller pass mouse in this step; the recorder writing pad frames is step 1.2.
- **e2e guard.** A replay is a pad replay when its file contains `"input"` with value `"pad"`. For a pad replay: frames are counted as the number of `"pad"` keys; the presence of any `"mouse_x"`, `"mouse_y"`, `"mouse_buttons"`, `"dpad"` or `"keyboard_cursor"` key fails the file with a message naming it. For any other replay, counting stays `"mouse_x"` keys. The limit/exception logic, the companion checks and the `keyboard_*` rules are untouched and apply to both kinds. The script's header usage comment and the Makefile `make e2e` legend line gain one clause each for the pad rule (reading-help only, per CLAUDE.md § Comments).
- **No new e2e replay** and no change to the replay driver, the raylib backend or the game in this step.
- **Decisions taken without a human** (guideline rules: make the step work, smaller change, modes never mix):
  - Bitmask field rather than an 11-entry array: one integer, trivial to compare and to record, and the PSX pad report is itself a bitmask.
  - Mouse-mode fields include `dpad` and `keyboard_cursor`: the plan says a pad replay stores "PSX buttons per frame, and no mouse field at all", and both belong to mouse mode only.
  - The loader also rejects mixed frames, not only the e2e guard: a mixed fixture outside `tests/e2e/` would otherwise load silently, contradicting "the two modes never mix … in a replay".
  - Pad frames are not wired to `p8_btn` here: the plan assigns all routing (mode, camera scroll, cheat) to step 1.2.

## Testing Decisions

- Good tests here drive the public interface only: load a fixture or a written file, save/append frames, and assert on the loaded frames, the loaded mode, the return value and the bytes written. They never inspect parser internals.
- **Input layer** (the existing input unit test file, prior art: its round-trip, append-byte-identity and malformed-fixture tests):
  - pad round trip: save frames covering every button alone, all 11 together and none, load back, every mask and the pad mode match; the mouse fields and `dpad` read as zero;
  - pad append is byte-identical to a pad save of the same frames, frame by frame;
  - saved pad output lists buttons in canonical order and contains no `mouse_x`/`dpad`/`keyboard_cursor`;
  - a pad fixture written in non-canonical order loads to the same mask;
  - malformed pad frame fixtures each fail and leave the previous input untouched: unknown name, duplicated name, missing `"pad"`, mouse field inside a pad frame;
  - header cases: no `"input"` → mouse (existing mouse fixtures report mouse mode); `"input": "mouse"` → mouse; unknown value → fail; `"pad"` with `"keyboard_cursor": true` → fail; `"input"` after `"frames"` is not read;
  - existing mouse save tests extended to assert the output still contains no `"input"`.
  - New fixtures go next to the existing `events_*` ones and are added to the test's Makefile prerequisites.
- **e2e guard** (the existing guard test, prior art: its `write_e2e_mouse` builder and the `keyboard_*` cases): a pad-replay builder writing `"input": "pad"` and `{ "pad": [] }` frames, plus:
  - a pad replay of 180 frames passes;
  - a pad replay of 181 frames not in the exceptions fails and is named with its frame count;
  - a listed pad replay passes at 600 and fails at 601;
  - a pad replay with one frame carrying `mouse_x` (and, separately, `dpad`) fails and is named;
  - a pad replay missing a companion file still fails.
- **Regression:** every `*.sha256` in `tests/e2e/` unchanged (`make e2e`), and the recorder/replay-source tests pass unmodified apart from the new mode argument.

## Out of Scope

- `--mouse`, picking the mode from the header in the replay driver, ignoring the mouse in pad mode, keeping the D-pad off the camera scroll, the cheat mapping, the shared per-frame input step, the session recorder writing pad frames (step 1.2).
- `keymap.cfg` and filling pad buttons from the keyboard or a gamepad (step 1.3).
- `pad_nav` and any `pad_*` replay (step 1.4 onward).
- Removing `keyboard_cursor` or `button_cursor` (step 4.1).
- Audio.

## Further Notes

- The PSX buttons are named in English (`cross`, `circle`, …) rather than with symbols, to stay grep-able and ASCII-only in replays.
- The fixture `events_keyboard_cursor_*` family is deleted in step 4.1; new pad fixtures must not depend on it.
