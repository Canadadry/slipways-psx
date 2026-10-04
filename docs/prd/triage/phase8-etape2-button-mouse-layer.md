---
title: "Plan 8, step 2 — button → mouse layer in the core"
description: "Nothing turns the six PICO-8 buttons into a cursor and clicks, so the keyboard cannot reach anything the game only offers to the mouse."
status: needs-triage
---

## Problem Statement

Plan 8 wants a full game played with the keyboard alone. The game reads the mouse in one place only (the mouse entity, once per frame, through `p8_input_read_mouse`), and every gesture that matters goes through it: hover, left press, left hold while moving (slipway drag, carrying a structure or a probe), left release (the drop), right press-hold-move on the background (map drag). The six buttons do something else today: the four directions feed the camera, which sums them (times 3) and eases toward the result; O and X feed the cheat on their rising edges. No code path turns a button into a cursor position or a mouse button.

Step 1 of the plan decides the mapping (which button clicks, how scrolling stays reachable, how the cheat survives). Its mapping (D1 to D6) is settled in the plan's "Settled decisions" ("Keyboard mapping (default, step 1)"); its speeds are defaults the step 5 trial may change. Step 2 needs a layer, in the core, that applies that mapping frame by frame and is proven by unit tests, without moving a single `*.sha256` in `tests/e2e/` and without making `make test` or `make e2e` slower.

## Solution

A small, pure module in the core (no raylib, no game objects) takes the frame's raw input — the six buttons and the real mouse — and returns the frame the game should read: a cursor position, mouse buttons and the directions the camera should see. Its state is a handful of integers. It allocates nothing and uses no floating point.

The events loader learns one session declaration: an `events.json` may say that this session uses the keyboard cursor. The headless replay driver runs the layer only for a file that declares it; a file that does not (every existing e2e replay, every `worst-case/` game) takes exactly today's path, so its hash cannot move.

From a player's point of view, once steps 3 and 4 land: arrows move the cursor (slow, then faster), Z clicks and holds like the left button, X like the right one, pushing the cursor against a screen edge scrolls the map, moving the real mouse takes the cursor back, and the cheat still works. Step 2 delivers the behavior; step 3 connects the keys of the window to it.

### The settled mapping this step implements

Step 1's mapping, as `docs/plan-8.md` settles it, read for what the layer must do. The plan's text wins over this summary.

- **D1, session property**: the layer runs only in a session that declares it; a session that does not runs exactly as today (directions scroll, O/X only feed the cheat, the cursor is the recorded mouse).
- **D1, declaration format**: the proposed declaration is a top-level `"keyboard_cursor": true` in `events.json`, absent meaning off; step 2 owns the format. This PRD fixes the reading side (see Implementation Decisions), step 3 the writing side.
- **D2, owner**: the cursor has one owner at a time. The mouse takes it when the real mouse position differs from the previous frame's, or a real mouse button changes state, and the cursor jumps to the real mouse. The keyboard takes it on the rising edge of any of the six buttons, and the cursor stays where it is. The mouse owns it at the start of a session. **A device keeps the cursor while one of its buttons is held (a real mouse button; O or X): the other device takes it only after the release**, so an arrow pressed during a mouse drag scrolls as today instead of dropping what the mouse carries. Only the owner's buttons count as mouse buttons.
- **D3, movement**: while the keyboard owns the cursor, each held direction moves it on its axis, each axis independently (no normalization): 1 pixel per frame for the first 10 frames the direction is held, then 2 pixels per frame. The cursor stays an integer clamped to 0..159 by 0..119.
- **D4, buttons**: while the keyboard owns the cursor, O held is the left button held and X held is the right button held, frame for frame.
- **D5, edge scroll**: while the keyboard owns the cursor, a direction reaches the camera only when the cursor already sits on that edge (x = 0 for left, x = 159 for right, y = 0 for up, y = 119 for down) and pushes outward; it then reaches the camera exactly as a D-pad direction does today. Any other direction moves the cursor and does not scroll. No shoulder button: the existing six-boolean `dpad` frame is enough.
- **D6, cheat**: the cheat keeps reading the raw O and X buttons, unchanged. D6's advice on where to type it (title screen, away from the difficulty rows; on the map a left tap can place the probe or confirm a space fold) is player guidance about the side clicks D4 already implies; it asks nothing more of the layer.

## User Stories

1. As a keyboard player, I want a held arrow to move the cursor one pixel per frame at first, so that I can aim at a 3-pixel planet.
2. As a keyboard player, I want the cursor to go two pixels per frame after ten frames, so that I can cross the screen in about a second.
3. As a keyboard player, I want releasing an arrow to bring it back to the slow speed, so that each new press starts precise.
4. As a keyboard player, I want two arrows held to move the cursor diagonally, so that corners are one gesture away.
5. As a keyboard player, I want the cursor to stop at the border of the 160×120 screen, so that it never leaves the screen.
6. As a keyboard player, I want O held to read as the left button held, so that a tap clicks and a hold drags a slipway or a structure.
7. As a keyboard player, I want X held to read as the right button held, so that X with the arrows drags the map.
8. As a keyboard player, I want an arrow pushing against the edge the cursor sits on to scroll the map at today's speed and easing, so that I can reach planets out of view.
9. As a keyboard player, I want an arrow that is not pushing against an edge to only move the cursor, so that aiming never scrolls the map under me.
10. As a keyboard player, I want the first arrow press to move the cursor from where it is drawn, so that it never jumps when I switch from the mouse.
11. As a player who uses both, I want moving the real mouse to take the cursor back at once, so that switching device needs no menu.
12. As a player who uses both, I want a real mouse button press or release to take the cursor back, so that a click with the mouse is never swallowed.
13. As a player who uses both, I want a real mouse button resting down to be ignored while the keyboard owns the cursor, so that only one device clicks at a time.
14. As a player who uses both, I want a button held from before the mouse took over not to steal the cursor back, so that only a new press hands control to the keyboard.
15. As a player dragging with the mouse, I want an arrow pressed during the drag to scroll the map as today, so that the keyboard never drops what the mouse carries.
16. As a keyboard player holding O or X, I want a bump of the real mouse to leave the cursor with the keyboard, so that a keyboard slipway or structure drag is never cut short by the desk.
17. As a cheat user, I want O O X X O O X X to keep granting 99 research and $200, so that the cheat survives the keyboard cursor.
18. As a developer, I want every existing `*.sha256` in `tests/e2e/` unchanged, so that the port's source of truth is untouched by this step.
19. As a developer, I want `make test` and `make e2e` no slower, so that the loop stays fast.
20. As a developer, I want the layer in the core with no raylib and no game objects, so that the PC build and a future PSX build share it.
21. As a developer, I want the layer to be integer-only and allocation-free, so that the existing float and allocation guards cover it with no exception.
22. As a developer, I want the declaration read by the events loader, so that a replay knows whether to run the layer before its first frame.
23. As a developer, I want a file without the declaration to load exactly as today, so that old recordings and fixtures keep working untouched.
24. As a developer, I want a malformed declaration to make the load fail, so that a broken file never silently replays as a mouse session.
25. As a developer writing step 3, I want one call per frame that takes the raw frame and gives the played frame, so that the raylib driver wiring is a few lines.
26. As a developer writing step 4, I want the headless replay driver to honor the declaration already, so that a keyboard e2e is only a new `events.json`.
27. As a developer, I want the played frame to keep the raw O and X, so that handing the cheat either frame gives the same result and a wiring slip cannot break it.
28. As a developer, I want the speeds and the edge rule held in named constants of the module, so that step 5's human trial can change a default in one place.
29. As a developer, I want the layer tested only through its frame-in, frame-out interface, so that a corrected mapping changes expectations, not test plumbing.
30. As a future PSX port, I want the layer to read only the six PICO-8 buttons and a mouse frame, so that the controller feeds it without change.

## Implementation Decisions

### Where the layer lives: `engine/`, as module `button_cursor`

- It is a new module of `src/engine/` named `button_cursor` (exported names `button_cursor_init`, `button_cursor_step`). `engine/` is the shared core under `game/`, with no planets or slipways, and already depends on `p8/` types (the dispatch context holds a `p8_input_t`). `p8/` is rejected: it is the emulated PICO-8 runtime, and PICO-8 has no button-driven cursor. `platform/` is rejected: it is the raylib backend, and the PSX build must reuse the layer. `game/` is rejected: the layer knows nothing of the game, and it must sit before the game's single mouse reader, not inside it.
- The name says what it does on any device (buttons drive a cursor); "keyboard" stays only in the session declaration, which step 1 names (D1).
- It depends only on the frame type of `p8/input.h` and the screen size of `p8/vram.h`. No allocation, no floating point, no global state: the caller owns the state struct.
- It joins the engine closure (`ENGINE_SRCS`), hence the core list the guards read (float-grep, float-guard, alloc-guard, constants-check), with no exception line.

### Interface

- `button_cursor_init(state)` puts a state in "session start": mouse owns the cursor, no previous frame seen.
- `button_cursor_step(state, raw, played)` is called once per frame with that frame's raw input (a `p8_input_frame_t`: real mouse x, y, buttons, and the six buttons) and writes the frame the game reads into `played` (same type). It returns nothing. The frame is copied out, matching `replay_source_next_frame`, so a played frame and a raw frame are the same shape at the call site and the game's readers (`p8_btn`, `p8_input_read_mouse`) need no change.
- No accessor for the owner or the cursor: everything observable goes through `played` (the cursor is `played`'s mouse position). Nothing in steps 3 or 4 needs more.

### State

The struct holds, as plain integers: the owner (mouse or buttons); the cursor x and y; for each of the four directions, how many consecutive frames it has been held (saturating just past the speed threshold, so it never overflows); the previous raw frame; and whether a previous frame exists. Fields are read through the interface only; the struct is public so the caller can own it on the stack, as `cheat_t` is.

### Rules of one step (D2 to D6, refined by G1–G8 in Further Notes)

In this order, every frame:

1. **First frame**: the previous raw frame is taken to be this one (no change seen), the owner is the mouse, the cursor is the raw mouse.
2. **Hold lock (D2)**: if the owner holds one of its buttons in this raw frame — the mouse owns and a raw mouse button is down, or the buttons own and raw O or X is down — the owner keeps the cursor and steps 3 and 4 are skipped (G8).
3. **Mouse change**: otherwise, if the raw mouse position or raw mouse buttons differ from the previous raw frame, the mouse owns the cursor (G1: this wins over a button edge on the same frame).
4. **Button edge**: otherwise, if any of the six raw buttons is down now and was up in the previous raw frame, the buttons own the cursor.
5. **Held counters**: each direction's counter goes up while its raw button is down and back to 0 when it is up, whoever owns the cursor (G2).
6. **Mouse owns** → `played` is a copy of `raw`, all nine fields (G3). The cursor follows the raw mouse. This is what makes an arrow pressed during a mouse drag reach the camera as a raw direction (D2).
7. **Buttons own** →
   - each held direction is "pushing" when the cursor, **before this frame's movement**, is on that direction's edge (G4); a pushing direction is copied into `played`'s directions, every other direction is written released;
   - the cursor moves: x by (+speed of right) − (speed of left), y by (+speed of down) − (speed of up), where a held direction's speed is 1 while its counter is 1..10 and 2 from 11 on; then it is clamped to 0..159 and 0..119 (G5);
   - `played`'s mouse position is the cursor; its mouse buttons are left when raw O is down, right when raw X is down, nothing else (no middle); the real mouse buttons are ignored;
   - `played`'s O and X are copied from raw (G6).
8. The raw frame becomes the previous raw frame.

The defaults (1 pixel, 2 pixels, 10 frames, the edge positions from the screen size) are named constants of the module, not runtime settings.

### The session declaration in the loader

- `p8_input_t` gains one field, the session declaration (named after the JSON key, `keyboard_cursor`, 0 or 1). `p8_input_init` and `p8_input_free` set it to 0. `p8_input_load` sets it from the file and, as today, leaves `input` untouched on a failed load.
- Format (D1 proposes the key; fixed here for the reading side): a top-level `"keyboard_cursor":` member, **before** the `"frames"` key, whose value is `true` or `false`. The loader looks for it only in the text before `"frames"`, the same substring way it reads frame fields: absent means 0, `true` means 1, `false` means 0, any other value fails the load (no partial load, as for a malformed frame). Searching only the head keeps a long recording's load cost unchanged and leaves frame parsing untouched.
- The `p8_input_load` contract in `p8/input.h` states the new member and its failure case. The writers (`p8_input_save`, `p8_input_append`) are **not** changed here: step 3 makes the interactive binary record the declaration, and must keep `p8_input_append` byte-identical to `p8_input_save` when it does.
- The interactive driver's positional initializer of `p8_input_t` becomes a designated one (`-Wextra` would otherwise reject the missing field under `-Werror` in `make raylib`). That one-line change is the only touch to the interactive driver in this step; it reads no declaration and runs no layer (step 3).

### The headless replay driver honors the declaration

- When the loaded file does not declare the keyboard cursor, the replay driver's frame loop is left exactly as it is today: it hands the whole script to the dispatch context and steps it. This, not the layer's mouse-owner pass-through, is what guarantees every existing hash (D1).
- When the file declares it, the driver keeps one `button_cursor` state, initialised before the first frame; each frame it runs `button_cursor_step` on the script's current frame and hands the dispatch context a one-frame input over the played frame (the same shape the interactive driver already builds per frame). The cheat keeps receiving the raw script (D6). No game reader reads `frame_count` or `current_frame` through the dispatch context (checked: only the camera, the cheat and the mouse read input, through `p8_btn` and `p8_input_read_mouse`), so the one-frame input is safe.
- `make state`, `make full-game` and `make sanitize-net` go through the same driver and replay only undeclared files, so they run today's path.

## Implementation Chunks

Each chunk is one TDD cycle and leaves `make test-all` green with every hash unchanged.

1. **The layer, alone.** The `button_cursor` module and its unit test, linked on their own (the test target names only the test and the module, not the engine closure). Added to the engine closure so the guards cover it. Nothing calls it yet.
2. **The declaration in the loader.** The new `p8_input_t` field, its reading in `p8_input_load`, the contract text in `p8/input.h`, new fixtures for the loader test, and the designated initializer in the interactive driver.
3. **The replay driver honors the declaration.** The gated branch in the headless replay driver. Proven by the unchanged hashes (undeclared path) and by the acceptance check in Testing Decisions (declared path); the automated proof of the declared path is step 4's keyboard e2e.

## Testing Decisions

- **What a good test is here**: raw frames in through `button_cursor_step`, played frames out, assertions on the played frame only (mouse position, mouse buttons, the six buttons). No test reads the state struct's fields. A test is a short sequence of hand-built raw frames, as the camera and mouse tests build theirs.
- **The layer's unit test** (new, one file for the module, one binary in `UNIT_TESTS`), one test per behavior:
  - first frame: played equals raw;
  - while the mouse owns the cursor, played equals raw, held directions included (the mouse moved while a direction was down);
  - a button held across the mouse's takeover does not take the cursor back until it is released and pressed again;
  - the rising edge of a direction hands the cursor to the buttons and moves it 1 pixel from where it was, with no jump;
  - 10 frames at 1 pixel, then 2 pixels from the 11th frame; release and press again starts at 1 pixel;
  - two directions held move both axes; opposite directions cancel;
  - the cursor clamps at 0 and 159 horizontally, 0 and 119 vertically;
  - a direction pushing against the edge the cursor sat on before the frame appears in played and the cursor stays; the frame the cursor reaches the edge does not scroll; a direction not pushing reads released in played;
  - O held reads as left, X held as right, both as both; released reads as no button; a real mouse button resting down is ignored while the buttons own the cursor;
  - the real mouse moving hands the cursor back, and the cursor jumps to it; a real mouse button changing state does too;
  - a mouse move and a button's rising edge on the same frame leave the mouse owning the cursor (G1);
  - arrow during a mouse-held drag: the mouse owns the cursor with a real button down and an arrow is pressed; the mouse keeps the cursor and played equals raw (the arrow reaches the camera as a raw direction, the mouse button stays down); after the mouse button is released, the still-held arrow does not take the cursor, a new press does (G8);
  - mouse bump during an O-held drag: the buttons own the cursor with O down and the real mouse moves (second case: a real mouse button is pressed); the buttons keep the cursor, played's mouse stays at the cursor with left down; on a frame where O is released and the mouse moves, the mouse takes the cursor;
  - the lock covers only O and X: with the buttons owning the cursor and only an arrow held, a mouse move takes the cursor back;
  - played O and X always equal raw O and X, whoever owns the cursor.
- **The loader's unit test** (the existing input test, extended): a file declaring `true` loads with the declaration on; every existing fixture (no declaration) loads with it off; `false` loads off; a value that is neither `true` nor `false` fails the load and leaves `input` untouched; a declaration written after the frames array is not read; `p8_input_init` and `p8_input_free` leave it off. Tiny new fixtures beside the existing ones.
- **Prior art**: the cheat's and the mouse button's state tests (frame sequences in, state out), the camera and mouse tests that build `p8_input_t` frames by hand, the input loader test with its small fixtures, and `raylib_backend_coords`, the precedent for a small pure module linked alone in its own unit test.
- **Speed**: the new unit binary links two files and runs in microseconds, in parallel with the others; the loader's new cases read files of a few lines. No new replay runs in `make test` or `make e2e`. The replay driver's declared branch costs nothing on an undeclared file (one integer test per run, not per frame).
- **Checks of the step**:
  - `make test-all` passes, with every `*.sha256` in `tests/e2e/` unchanged;
  - `make full-game`, `make state` and `make sanitize-net` pass (same undeclared path);
  - `make raylib` still compiles and links, when the vendored raylib is present (`make raylib-vendor-setup`); if it cannot be set up in the environment, say so in the commit message;
  - acceptance check of chunk 3, outside the suite: a throwaway `events.json` in the scratchpad (not committed) that declares the keyboard cursor, leaves the mouse still on empty space of the title screen, moves the cursor with the arrows onto a difficulty row and taps O, run with `make replay EVENTS=...`: the text dump shows the game started; the same file without the declaration stays on the title screen.

## Out of Scope

- Step 3: reading the keys in the raylib backend, running the layer in the interactive driver, recording the declaration into `events.json` (and the `p8_input_save` / `p8_input_append` change that goes with it), README controls. The recorder writes the raw mouse and buttons, never the layer's output: a declaring replay runs the layer again on the raw frames, which is how this PRD's replay driver consumes them.
- Step 4: the keyboard e2e replays and their notes; any new file in `tests/e2e/`.
- Step 5: the human trial and any change of default speeds.
- Approving or correcting step 1's mapping; this PRD follows whatever "Settled decisions" says once approved.
- Any change to the cheat, the camera or the mouse entity; a planet-to-planet jump mode; shoulder buttons; a physical controller; PSX button assignment; audio.

## Further Notes

- **Unattended run.** No human was available. The to-prd check-ins were resolved with the `guideline` skill (simplicity, no speculative feature, surgical changes, verifiable goals), and the outcomes are recorded here:
  - *Do the modules match expectations?* One new deep module (`button_cursor`: two functions over a rule set that holds all of D2 to D5), one field and one parsing rule in the loader, one gated branch in the replay driver. No enable flag in the layer: the driver decides whether to call it, so the layer has one behavior and the "undeclared means today" guarantee rests on code that does not change.
  - *Which modules get tests?* The layer (new unit test) and the loader (extended test). The replay driver's branch gets no new automated test in this step: a replay-level test would add a replay run to `make test`, which the project forbids; step 4's e2e is that proof.
  - *Does it need cutting?* Yes, into the three chunks above, in dependency order.
- **Step 1 graduated** with commit 3109013. Against the step 1 PRD, its review added D2's hold lock (rule 2, G8 and their tests) and reworded D6's advice on where to type the cheat (no effect on the layer). Every other rule this PRD first carried as a hypothesis matched the settled text and is now stated as fact.
- **Decisions this PRD makes on its own** (each refines a settled rule):
  - **G1**: a mouse change and a button's rising edge on the same frame give the cursor to the mouse. A still real mouse never triggers it, so a keyboard replay with a constant mouse is unaffected.
  - **G2**: direction counters follow the raw buttons whoever owns the cursor; speed is a property of how long the key has been held.
  - **G3**: while the mouse owns the cursor, the played frame is the raw frame, directions included: a direction held while the mouse moves scrolls as today.
  - **G4**: "already on the edge" (D5) is read before this frame's movement, so reaching the edge and scrolling are two different frames.
  - **G5**: the movement on one axis is the difference of the two opposite directions' speeds; the clamp follows the movement.
  - **G6**: the played frame keeps the raw O and X; nothing in the game reads them but the cheat, which receives the raw frame anyway.
  - **G7**: the declaration sits before `"frames"` and only `true`/`false` are accepted.
  - **G8**: the hold lock (D2) is read on the current raw frame, so on the frame that releases the owner's button the other device can already take the cursor if it changes on that same frame. After the release, the other device needs a fresh trigger (a mouse change, or a button's rising edge): a change or press swallowed during the lock is not replayed later. G1 only breaks a tie when no lock applies; G3 is what lets an arrow pressed during a mouse drag scroll as today.
- **Facts read in the code at this commit**: the camera reads directions 0–3 through `p8_btn` and sums them times 3 before easing; the cheat reads buttons 4 and 5 through `p8_btn`; the mouse entity is the only caller of `p8_input_read_mouse` and counts the middle button as right; both reach the frame through the dispatch context; the backend's window-to-screen conversion already clamps the real mouse to 0..159 by 0..119; `p8_input_load` is implemented in the devtools loader and reads fields by substring search; `p8_input_t` is built positionally in exactly one place (the interactive driver).
