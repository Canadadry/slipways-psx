---
title: "Plan 8, step 3 — raylib wiring for the keyboard cursor"
description: "The button → mouse layer exists in the core, but the interactive PC binary neither runs it nor records that it did, so the keyboard still cannot play a game in the window."
status: needs-triage
---

## Problem Statement

Plan 8 wants a full game played on PC with the keyboard alone. Step 2 (commits 53ba99a, 109035d, b4ddd7a) gave the core a layer that turns the six PICO-8 buttons into a cursor and mouse buttons (D2 to D5), and taught the events loader and the headless replay driver one session declaration (D1): an `events.json` that declares the keyboard cursor is replayed with the layer on, every other file exactly as today.

None of that reaches the window. The interactive binary (`src/cmd/game.c` with the raylib backend) hands the raw frame straight to the game, so in the window the arrows still only scroll the map and Z and X only feed the cheat. Its recording writer cannot write the declaration either: `p8_input_save` and `p8_input_append` know only the frames. So even once the layer runs in the window, a recorded session would replay headlessly as a mouse session, and the human trial of step 5 (whose `events.json` is replayed by `make state`) would measure a different game from the one played. Nothing in the README tells a player which keys do what.

## Solution

The interactive binary runs the step 2 layer, the way the headless replay driver does, and its recording says so.

- A new game in the window is a keyboard-cursor session: the arrows move the game's cursor, Z holds the left button, X the right one, pushing the cursor against a screen edge scrolls the map, moving the real mouse takes the cursor back (D1 to D5). The keys read are the ones the backend already reads (arrows, Z, X); no new key.
- The recording stays the raw input (real mouse and the six buttons, frame by frame), and its head now carries `"keyboard_cursor": true`. Replayed with `make replay`, `make state` or the interactive binary, it runs the layer again on the same raw frames and reaches the same game.
- A resumed session follows its recording: a declared recording resumes as a keyboard-cursor session (the layer runs over replayed and live frames alike, with one state for the whole session); an older, undeclared recording resumes exactly as today, mouse only, and its continuation stays undeclared. One declaration per file is what keeps a resumed recording replayable as one session.
- The writers keep their byte-identity contract: for the same frames and the same declaration, `p8_input_append` leaves on disk exactly what `p8_input_save` writes. An undeclared file is byte-for-byte what they write today.
- The README gains a short "Controls" section.

Nothing changes for a headless replay of an existing file: every `*.sha256` in `tests/e2e/` is untouched, and `make test` / `make e2e` run nothing new beyond a few unit asserts on tiny files.

## User Stories

1. As a keyboard player in the window, I want the arrows to move the game's cursor, so that I can point at a planet without a mouse.
2. As a keyboard player in the window, I want Z to act as the left mouse button, held and released frame for frame, so that I can click, drag a slipway or carry a structure.
3. As a keyboard player in the window, I want X to act as the right mouse button, so that X with the arrows drags the map.
4. As a keyboard player in the window, I want pushing the cursor against a screen edge to scroll the map, so that I can reach planets out of view.
5. As a keyboard player in the window, I want to start a new game without any flag or menu to turn the keyboard cursor on, so that the binary is ready to play as launched.
6. As a player who has both, I want moving the real mouse to take the cursor back at once, so that I can switch device mid-game.
7. As a player who has both, I want the hidden host cursor to stay hidden, so that the game's own cursor is the only one I see whichever device drives it.
8. As a cheat user in the window, I want O O X X O O X X typed with Z and X to keep working, so that the cheat survives the keyboard cursor.
9. As a playtester, I want my session recorded as before, in `tmp/events_recorded_<timestamp>.json`, so that nothing in my workflow moves.
10. As a playtester, I want the recording to say it was a keyboard-cursor session, so that an agent replaying it headlessly sees the game I played.
11. As a playtester, I want the recording to hold the raw keys and the raw mouse, not the cursor the layer computed, so that a later change of default speeds (step 5) can be replayed against what I actually pressed.
12. As a playtester, I want the space bar marker to keep naming the right frame of the recording, so that my bookmarks stay usable.
13. As a playtester resuming a keyboard-cursor recording, I want the replayed part and my new moves to run through the same layer state, so that the resumed recording replays as one session.
14. As a playtester resuming an old mouse recording, I want it to replay exactly as it did, so that an old crash report still reproduces.
15. As a playtester resuming an old mouse recording, I want the continuation recorded without the declaration, so that the new file replays to the same game as the session I saw.
16. As a playtester watching a replay in the window, I want Page Up, Page Down and End to keep working, so that the replay controls are unchanged.
17. As a player, I want the README to list the controls, so that I can find out which key does what without reading the code.
18. As a player resuming an old recording, I want the README to say that such a session stays mouse-only, so that arrows scrolling the map there does not surprise me.
19. As an agent, I want `make replay EVENTS=<recording>` of a keyboard-cursor session to reach the state the human saw, so that a playtest finding can be investigated headlessly.
20. As an agent running step 5, I want the human trial's recording to be replayable by `make state`, so that its peaks can join `worst-case/` if they exceed `full_game.json`'s.
21. As a developer, I want `p8_input_append` to stay byte-identical to `p8_input_save` with the declaration on, so that the replay toolchain's founding contract holds for keyboard recordings too.
22. As a developer, I want an undeclared file written byte-for-byte as today, so that no fixture, recording or test that compares bytes moves.
23. As a developer, I want the writers to emit only `true` and to omit the key when off, so that "absent means off" (D1) is the only spelling of a mouse session.
24. As a developer, I want the decision "which declaration does this session have" in a module without raylib, so that it is unit-tested although the binary is never run.
25. As a developer, I want the interactive loop to feed the game, the cheat and the recorder in the same order as the headless replay driver, so that a session and its replay cannot drift apart.
26. As a developer, I want every existing `*.sha256` in `tests/e2e/` unchanged, so that the port's source of truth is untouched.
27. As a developer, I want `make test` and `make e2e` no slower, so that the loop stays fast.
28. As a developer, I want `make raylib` to still compile and link, so that the binary a human will run in step 5 builds.
29. As a developer, I want the step 2 interfaces this PRD builds on stated as they landed, so that the implementer starts from the code, not from a forecast.
30. As a future PSX port, I want the wiring to add nothing to the core beyond what step 2 put there, so that the controller can feed the same layer.

## Implementation Decisions

### What step 2 landed

Everything below that names a step 2 symbol uses these interfaces, read from the code after b4ddd7a.

- The layer is module `button_cursor` of `src/engine/` (header `engine/button_cursor.h`), with a caller-owned state struct `button_cursor_t`, `button_cursor_init(state)` and `button_cursor_step(state, raw, played)` taking a `const p8_input_frame_t *` raw frame and writing a `p8_input_frame_t *` played frame; `played`'s O and X always equal `raw`'s. It is in `ENGINE_SRCS`, so it is already linked into the interactive binary through `GAME_SRCS`.
- `p8_input_t` has an integer field `keyboard_cursor` (0 or 1), set by `p8_input_load`, zeroed by `p8_input_init` and `p8_input_free`.
- The loader (`find_keyboard_cursor` in `src/devtools/input_json.c`) looks for `"keyboard_cursor":` only before `"frames"`, skips spaces and tabs before `true` / `false`, fails the load on any other value, and does not inspect what follows the value. It therefore accepts the exact head this PRD's writers emit: `{`, newline, two spaces, `"keyboard_cursor": true,`, newline, then today's `  "frames": [` line. That head is the fixture `tests/fixtures/events_keyboard_cursor_true.json`. The loader's spelling is step 2's format: the writer follows it.
- The headless replay driver (`src/cmd/replay.c`) keeps one `button_cursor_t` initialised before the first frame; on a declaring file it steps it on each script frame and hands the dispatch context a one-frame `p8_input_t` over the played frame; the cheat always gets the raw input. An undeclared file takes exactly the old path.
- The interactive driver's one-frame `p8_input_t` initializer is designated; nothing else in `src/cmd/game.c` moved.
- `p8_input_save`, `p8_input_append`, `session_recorder_*`, `replay_source_*` and the raylib backend are unchanged by step 2. `replay_source_open` loads through `p8_input_load`, so the loaded recording's `keyboard_cursor` field already carries the declaration with no extra parsing.

### The backend reads no new key

`raylib_backend_poll_input` already fills the six buttons from the arrows, Z and X (D4 keeps those keys) and the real mouse in screen space (clamped by `raylib_backend_window_to_screen`). It stays unchanged: it keeps producing the raw frame. No shoulder button (D5), no toggle key. The host cursor stays hidden as today; the game draws its cursor at the played mouse position, which is what makes the keyboard cursor visible.

### The writers declare

- `p8_input_save` and `p8_input_append` each gain one integer parameter, `keyboard_cursor`. All current callers pass 0 except the session recorder.
- With 0, every byte written is today's: the same `JSON_PREFIX`, frame lines, separators and suffix. With 1, the head is today's prefix with one member line inserted before `"frames"`: `"keyboard_cursor": true,` indented like `"frames"`. The writers never write `false`: absent means off (D1).
- The head is chosen in one place shared by both writers, as the frame line already is, so the two cannot drift into different bytes.
- `p8_input_append` reads the parameter only on the call that starts a recording (`*tail_offset` 0); later calls overwrite only the tail, so the head written first stays. The new tail offset is computed from the head actually written, as today. Contract text: pass the same value on every call of one recording; the value of later calls is ignored.
- The contracts in `p8/input.h` are updated: the shape `p8_input_save` writes now names the optional member, and `p8_input_append`'s byte-identity is stated "for the same frames and the same declaration".

### The recorder carries the declaration

- `session_recorder_open(recorder, path, keyboard_cursor)` stores the declaration in the struct and passes it to every `p8_input_append`. `session_recorder_close` resets it to 0 with the other fields.
- `raylib_backend_init(backend, keyboard_cursor)` passes it to `session_recorder_open`. That is the backend's only change.

### The session's declaration is decided by the replay source

- `replay_source` gains one accessor, `replay_source_keyboard_cursor(source)`: 1 when no recording was given (a new game: D1, "the interactive PC binary always does"), the loaded recording's own declaration when one was loaded, 0 when the load failed (the driver exits before using it).
- **K1 (narrows D1), decided by the step 3 review: accepted.** A new game always declares; a resumed session inherits its recording's declaration. The code leaves no sound alternative. `game.c` runs one loop where a replayed frame and a live frame are recorded alike, before the game steps, and `session_recorder_record_frame` never asks which it got, so the resumed file is the replayed frames followed by the live ones under the single head `p8_input_append` writes on its first call: one declaration per file. "Always declare" would run an undeclared recording's replayed part through `button_cursor` in the window, while `make replay` reads the same file's own `keyboard_cursor` (via `p8_input_load`, which `replay_source_open` also uses) and keeps the layer off: the window would play a different game from the headless replay (for instance `camera_drag_dpad`'s arrows would move a cursor instead of scrolling), so resuming a recording would no longer show the session it holds, which is what resuming an old crash report is for. Declaring the live part only would need a mid-file switch the format does not have. The cost: resuming an old mouse recording gives a mouse-only session. Old recordings are a debugging tool, so that is accepted; the README says it.
- **D1 amendment.** Because K1 narrows D1's wording ("the interactive PC binary always does"), chunk 3 (the interactive wiring) also amends `docs/plan-8.md` "Settled decisions" D1 by one sentence: a resumed session keeps its recording's declaration, so a pre-step-3 mouse recording resumes mouse-only.
- Living in `replay_source` (no raylib) is what makes this decision unit-tested although the binary is never run.

### The interactive loop

Per frame, in `src/cmd/game.c`, in this order (the headless replay driver's order):

1. The raw frame comes from the recording or from `raylib_backend_poll_input`, as today.
2. It is recorded raw, before the game steps, as today (the recorder never sees the played frame: a declaring replay runs the layer again on raw frames).
3. If the session declares the keyboard cursor, `button_cursor_step` turns it into the played frame, with one `button_cursor_t` initialised once before the loop and stepped over replayed and live frames alike; otherwise the played frame is the raw frame.
4. The dispatch context gets a one-frame `p8_input_t` over the played frame; the cheat gets a one-frame `p8_input_t` over the raw frame (D6; equal O/X either way, but this mirrors the replay driver).
5. `cart_update60`, `cart_draw`, then `raylib_backend_present` with the played frame (its frame argument is unused).

The declaration is read once, after `replay_source_open` and before `raylib_backend_init`. The replay controls, the marker, the frame-rate cap and the start-up messages are unchanged. The undeclared path (an old recording resumed) is exactly today's loop.

### README

A "Controls" section of a few lines, under the rule of one clear sentence per item: mouse as before; arrows move the cursor (slow for a sixth of a second, then faster); Z is the left button and X the right, held like a mouse button; pushing the cursor against an edge scrolls, X held with the arrows drags the map; moving the mouse takes the cursor back; space bookmarks a frame; Page Up, Page Down and End control a replay; a resumed recording made before the keyboard cursor stays mouse-only, arrows scrolling. The README names Z and X once with their layout caveat: raylib's `KEY_Z` and `KEY_X` are key positions named after the US layout (its header: "Keyboard keys (US keyboard layout)"), so on an AZERTY keyboard the left button is the key labelled W, and X stays X.

## Implementation Chunks

Each chunk is one TDD cycle and leaves `make test-all` green with every hash unchanged.

1. **The writers declare.** New parameter of `p8_input_save` and `p8_input_append`, shared head, updated contracts; every existing caller passes 0 (tests and the recorder, whose own declaration arrives in chunk 2). Tests in the input test.
2. **The recorder and the replay source carry the declaration.** `session_recorder_open`'s new parameter, `replay_source_keyboard_cursor`, and their tests (replay source, recording continuity, marker coherence call sites).
3. **The interactive binary.** `raylib_backend_init`'s new parameter, the loop of `src/cmd/game.c`, the README "Controls" section, and the one-sentence amendment of D1 in `docs/plan-8.md` "Settled decisions" (K1). Proven by `make raylib` (compile and link only) and by review against the replay driver's order; there is no headless run of this binary.

## Testing Decisions

- **What a good test is here**: behavior seen from outside each module — bytes on disk and what the loader reads back, what the replay source answers, what a resumed recording replays — never a struct field.
- **Input test (writers)**, extended:
  - save with the declaration on writes the member before `"frames"` and loads back with the declaration on and every frame field intact (round trip through step 2's loader);
  - append with the declaration on is byte-identical to save with the declaration on after every frame, like the existing undeclared identity test;
  - save and append with the declaration off never write `"keyboard_cursor"`, and the existing byte-identity and shape tests pass unchanged (they now pass 0);
  - append ignores the declaration after the first call (a recording started off stays off).
- **Replay source test**, extended: no path → declares; a loaded fixture without the declaration → does not; a loaded fixture declaring `true` → declares (reuse step 2's fixtures `tests/fixtures/events_keyboard_cursor_true.json` and `events_keyboard_cursor_false.json`).
- **Recording continuity test**, extended, still driving the real modules in the order of `src/cmd/game.c`: a declared source resumed with the recorder opened on the source's declaration gives a resumed recording that loads with the declaration on and the same frames in order; an undeclared source gives an undeclared resumed recording whose bytes contain no declaration; a fresh session (no source) gives a declared recording. Its loop mirror stays without the layer: the layer never touches what is recorded, and asserting that in a mirror would test the mirror.
- **Marker coherence test**: call-site update only; markers do not depend on the declaration.
- **Not tested automatically**: the interactive loop itself (it needs a window, which agents never open). Covered by `make raylib` linking it, by the loop order stated above matching `src/cmd/replay.c`, and by step 5, whose recording is replayed by `make state`.
- **Prior art**: the input test's save/append identity test and its tiny fixtures; `test_recording_continuity` and `test_marker_coherence`, which already stand in for the raylib loop without a window; `test_replay_source`.
- **Speed**: a handful of asserts on files of a few lines in binaries that already exist; no new binary, no new replay in `make test` or `make e2e`.
- **Checks of the step**: `make test-all` passes with every `*.sha256` in `tests/e2e/` unchanged; `make full-game`, `make state` and `make sanitize-net` pass (they replay only undeclared files); `make raylib` compiles and links when `make raylib-vendor-setup` can run in the environment (otherwise the commit message says it could not be checked); the binary is never launched. Outside the suite, a throwaway declared recording written by the recorder in a scratch test or by hand replays with `make replay` the same way as step 2's acceptance file.

## Out of Scope

- Step 2's layer, loader and replay driver; any change to their rules or speeds.
- Step 4: the keyboard e2e replays and any new file in `tests/e2e/`.
- Step 5: the human trial, a change of default speeds, adding its recording to `worst-case/`.
- New keys: shoulder buttons, a mode toggle, a remappable layout, a physical controller, PSX button assignment.
- A mid-recording switch of the declaration, or converting old recordings.
- Any change to the cheat, the camera, the mouse entity, the replay controls or the marker; audio.

## Further Notes

- **Unattended run.** No human was available; the to-prd check-ins were resolved with the `guideline` skill (simplicity, surgical changes, surfaced assumptions, verifiable goals):
  - *Do the modules match expectations?* No new module: one parameter on each writer, one on the recorder's open and the backend's init, one accessor on the replay source, a few lines in the interactive loop, a README section. A shared "session input" module for the two drivers was considered and rejected: two call sites of a few lines, and step 2 owns the replay driver's shape.
  - *Which modules get tests?* The writers, the replay source and the recording seam, which are reachable without a window. The interactive loop gets none (it needs a window).
  - *Does it need cutting?* Yes, into the three chunks above.
- **Facts read at c12a110, still true after b4ddd7a**: the backend's poll already maps arrows to buttons 0–3 and Z/X to 4/5; the interactive loop records before stepping and builds a one-frame `p8_input_t` per frame; the writers share `JSON_PREFIX`, `FRAME_SEPARATOR`, `JSON_SUFFIX` and one frame-line formatter; `p8_input_save` is called only from tests, `p8_input_append` only from the session recorder and tests; `tools/e2e_limits.sh` counts frames by `"mouse_x"`, so an extra head member does not change its count; the README has no controls section yet.
