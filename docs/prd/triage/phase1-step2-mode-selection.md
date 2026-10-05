---
title: "Mode selection and the replay driver"
description: "Pad frames exist as data, but neither the game nor the replay driver knows which input mode a session runs in, so pad input cannot be played, recorded or replayed, and the human's input path and the replay's input path are wired separately."
status: needs-triage
---

## Hypotheses on step 1.1

Checked against step 1.1 as landed (see its step note in `docs/plan-15.md`): every hypothesis holds in behavior. Real names: the mask is `pad_buttons`, the modes are `P8_INPUT_MOUSE` / `P8_INPUT_PAD`, the mode is the last parameter of `p8_input_save` / `p8_input_append`.

Step 1.1 (`docs/prd/triage/phase1-step1-pad-frames.md`) is being implemented while this PRD is written. This PRD reads only committed code and builds on step 1.1's PRD. Each assumption about step 1.1's outcome is listed here. **Rule for the implementer:** if step 1.1 landed with a different name, signature or spelling, use what step 1.1 actually landed. The behavior this PRD asks for does not change. Only a hypothesis whose *behavior* turns out false (not just its name) needs a note in the step note.

- **H-1.1-1** The input frame gains one integer bitmask of held PSX buttons, appended after the existing fields (called the *pad mask* below). Eleven runtime-prefixed constants name its bits: Up, Down, Left, Right, Cross, Circle, Square, Triangle, L1, R1, Start (e.g. `P8_PAD_CROSS`). A mask of 0 means nothing held.
- **H-1.1-2** A two-value input-mode type (mouse, pad) is declared next to the input frame in the runtime's input header, with mouse as the zero value (e.g. `p8_input_mode_t`, `P8_INPUT_MODE_MOUSE`, `P8_INPUT_MODE_PAD`).
- **H-1.1-3** The loaded-input struct has a mode field. The loader sets it from the replay header. Init and free reset it to mouse.
- **H-1.1-4** The runtime's save and append functions take an input-mode parameter alongside the existing `keyboard_cursor` one. Passing pad together with a set `keyboard_cursor` trips an `assert`. In pad mode they write only the pad mask of each frame (no mouse field, no `dpad`), with the `"input": "pad"` header. In mouse mode their output is byte-identical to today.
- **H-1.1-5** A loaded pad frame has every mouse field and every `dpad` entry at zero. Only its pad mask is set.
- **H-1.1-6** The replay JSON: header member `"input"` (`"pad"`, or `"mouse"`, or absent meaning mouse). A pad frame is `{ "pad": [<button names>] }`. The loader rejects a pad replay that contains any mouse-mode field (`mouse_x`, `mouse_y`, `mouse_buttons`, `dpad`, `keyboard_cursor`). So **a pad replay file can never carry mouse input.**
- **H-1.1-7** `p8_btn` and the mouse reader are unchanged: they read `dpad` and the mouse fields only, never the pad mask.
- **H-1.1-8** After step 1.1 the session recorder, the replay-source tests and the recording-continuity tests compile and pass by passing mouse mode explicitly. Nothing in the game or the raylib backend reads or fills the pad mask yet.
- **H-1.1-9** `tests/e2e/` holds no pad replay after step 1.1. The 26 mouse replays and 4 `keyboard_*` replays have their hashes unchanged.

## Problem Statement

Plan 15 makes the game playable with a PSX-style pad, with pad as the default mode and the mouse kept only through `--mouse`. Step 1.1 lets a replay file carry pad frames, but nothing uses that yet:

- The interactive game always plays the mouse. It turns on the keyboard cursor (`button_cursor`) for every fresh session. It has no `--mouse` switch and no pad mode.
- The replay driver ignores the replay's `"input"` header.
- The session recorder always writes mouse frames.
- The cheat code reads only PICO-8's O and X, which a pad frame never sets.
- The game and the replay driver each wire their own per-frame input handling: the keyboard cursor, the cheat, and setting the input the cart reads. So a replay can pass while the human's path is broken, and the reverse.

Every later step (keymap, pad navigation, the `pad_*` replays) needs one place where a frame's input is interpreted according to the session's mode. It must be the same place for the human and for the replays.

## Solution

- **Mode is fixed for the whole session, at startup.**
  - The game runs in pad mode by default, and in mouse mode with `--mouse`.
  - When the game is given a recording, the recording's header decides the mode. `--mouse` with a pad recording is refused before the window opens.
  - The replay driver takes the mode from the replay's header only.
- **One shared per-frame input step, used by both the game and the replay driver.** It takes the raw frame, interprets it according to the mode, runs the cheat, and installs the played input that the cart reads.
  - **Mouse mode** is exactly today's behavior. A recording that declares `keyboard_cursor` still runs through `button_cursor`.
  - **Pad mode** hands the cart a frame with no mouse movement, no mouse button and no D-pad. The mouse is ignored and the cart's camera scroll never sees the D-pad. × and ○ feed the cheat's O and X.
- **The session recorder writes the format of the session's mode**, so a pad session's recording is a pad replay.
- A fresh mouse session no longer turns on `button_cursor`: mouse mode is "today without `button_cursor`". The 4 `keyboard_*` replays keep it through their header until step 4.1.
- **Pad mode is not playable yet.** The raylib backend fills the pad mask only from step 1.3, and nothing navigates until step 1.4. The game says so at startup, and the human uses `--mouse` meanwhile.

## User Stories

1. As a player, I want the game to start in pad mode without any flag, so that the pad is the default control scheme as the plan states.
2. As a player, I want `--mouse` to start the game in mouse mode, so that I can still play with the mouse while pad navigation is built.
3. As a player in mouse mode, I want the game to behave exactly like the cart (mouse plus D-pad camera scroll), so that nothing I know changes.
4. As a player in mouse mode, I want the arrow keys to scroll the camera and not to drive a virtual cursor, so that mouse mode is "today without `button_cursor`".
5. As a player in pad mode, I want a moving mouse to have no effect on the game, so that a bumped mouse never moves a hidden cursor or hovers something.
6. As a player in pad mode, I want mouse clicks to have no effect, so that the two modes never mix.
7. As a player in pad mode, I want the D-pad never to scroll the camera through the cart's scroll, so that the navigation layer (later steps) alone owns the camera.
8. As a player in pad mode, I want to type the cheat as × × ○ ○ × × ○ ○, so that the cheat still exists on a pad.
9. As a player in pad mode, I want PICO-8's O and X keys not to feed the cheat, so that only PSX buttons count in pad mode.
10. As a player in mouse mode, I want the cheat to stay O O X X O O X X on PICO-8's buttons, so that mouse mode is unchanged.
11. As a player, I want the game to print the mode it runs in at startup, and in pad mode to point at `--mouse`, so that I am not puzzled by a pad mode that does nothing yet.
12. As a player, I want an unknown command-line option to stop the game with a usage line before any window opens, so that a typo never silently starts a session in the wrong mode.
13. As a player, I want more than one recording path on the command line to stop the game with a usage line, so that I know which recording is replayed.
14. As a player resuming a recording, I want the recording's header to decide the mode, so that a resumed session replays the way it was recorded.
15. As a player resuming a mouse recording, I want the session to stay in mouse mode without `--mouse`, so that all existing recordings resume as before.
16. As a player resuming a pad recording, I want the session to stay in pad mode, so that the resumed game continues with the same controls.
17. As a player passing `--mouse` with a pad recording, I want the game to refuse to start with a message naming the conflict, before the window opens, so that modes never mix and nothing is silently ignored.
18. As a player passing `--mouse` with a mouse recording, I want the session to start normally, so that the flag is harmless when it agrees with the header.
19. As a player resuming a `keyboard_cursor` recording, I want it to keep running through `button_cursor` and keep declaring `keyboard_cursor` in its new recording, so that the 4 `keyboard_*` scenarios stay resumable until step 4.1.
20. As a player in pad mode, I want my session recorded as a pad replay, so that what I play can become a `pad_*` replay.
21. As a player in mouse mode, I want my session recorded as a mouse replay, byte for byte as today apart from the `keyboard_cursor` flag that fresh sessions no longer declare, so that the recording format is unchanged.
22. As a player resuming a pad recording, I want the resumed recording to hold the replayed pad frames and then the new ones, as one pad replay, so that it is itself resumable.
23. As a replay author, I want the replay driver to pick the mode from the replay's `"input"` header, so that a pad replay plays in pad mode with no flag.
24. As a replay author, I want a replay without `"input"` to play in mouse mode, so that the 30 existing replays run unchanged.
25. As a replay author, I want a pad replay to go through the exact code the human's game runs each frame, so that every pad replay also covers the human's input path.
26. As a replay author, I want a pad frame recorded live and played back to give the cart exactly the same input as the live frame did, so that a recording replays to the game that was played.
27. As a replay author, I want the cheat typed in a pad replay to fire exactly as it did live, so that `pad_research` (step 3.1) can rely on it.
28. As a maintainer, I want every mouse `*.sha256` and every `keyboard_*` `*.sha256` unchanged, so that I know the shared step changed nothing for the existing replays.
29. As a maintainer, I want `make test` and `make e2e` no slower, so that the loop stays fast.
30. As a maintainer, I want the input step to be one small module with a narrow interface, so that pad navigation (step 1.4) plugs into it and nowhere else.
31. As a maintainer, I want the mode resolution (header, `--mouse`, default) to be testable without a window, so that the rules are pinned by unit tests even though the game binary never runs in CI.
32. As a maintainer, I want a headless pad-mode run with mouse noise and a held D-pad to prove that the camera, the hovered entity and the game state stay unchanged, so that "pad mode ignores the mouse" is checked on the real cart, not only on a frame struct.
33. As a maintainer, I want `make raylib` checked before and after this step, so that the interactive binary, which `make test` never compiles, is known to build.
34. As a future PSX porter, I want the input step to depend on the input frame only and not on raylib, so that the PSX driver can feed it the same frames.

## Implementation Decisions

### Preliminary check (plan rule)

- The step starts with `make raylib-vendor-setup`, then `make raylib` on the unchanged tree, and records the result in the step note.
- If vendoring cannot be done in the session (no network, no sudo: try `RAYLIB_VENDOR_SKIP_APT=1` first, see the raylib vendoring doc), the step note says so. The CI `raylib` job on the pushed commit then becomes the check.
- The diff touches `src/cmd/game.c` and `src/platform/`, so the step's check also lists "`make raylib` builds" (or "the CI raylib job is green") at the end, and the step note records it.

### Mode resolution (replay source module)

- **Mode accessor.** The replay source exposes the session's input mode (H-1.1-2):
  - loaded recording: the recording's header mode (H-1.1-3);
  - no recording: pad;
  - failed load: mouse. The value is irrelevant there, since the game exits, and mouse keeps it the zero value.
- **`--mouse` resolution.** The replay source also exposes one function that takes "was `--mouse` given" and returns the session mode, or a conflict:

  | Recording | `--mouse` absent | `--mouse` given |
  |---|---|---|
  | none | pad | mouse |
  | mouse | mouse | mouse |
  | pad | pad | **conflict** |

  It lives here, not in the game's `main`, so that it is unit-tested without raylib.
- **`keyboard_cursor` accessor.** No recording now answers 0, not 1: a fresh session no longer runs `button_cursor` (plan: mouse mode is "today without `button_cursor`"). A loaded recording still answers its header value, and a failed load still answers 0.

### Session recorder

- Opening the recorder takes the session's input mode alongside `keyboard_cursor`, and passes both to the runtime's append on every frame (H-1.1-4).
  - Pad together with `keyboard_cursor` set is a caller error, caught by an `assert` with a message.
  - Close resets the mode to mouse.
- The recorder keeps recording the **raw** frame, before the input step, as today. In pad mode the append writes only the pad mask (H-1.1-4), so whatever the raw frame holds besides the mask is dropped on disk. That is why the input step must, in pad mode, depend on the pad mask alone (next section).

### Shared per-frame input step (new game module)

- A new module in the game layer. It needs the cheat, which is game code, and step 1.4 adds `pad_nav` here, which is game code too. Working name: `input_step`.
- It holds per-session state:
  - the mode;
  - the `keyboard_cursor` flag;
  - the `button_cursor` state;
  - the cheat state;
  - the single played frame, wrapped as a one-frame input that the dispatch context points at.
- **Interface:**
  - an init function taking the mode and `keyboard_cursor` (pad with `keyboard_cursor` set → `assert` with a message);
  - one per-frame function taking the raw frame. It returns whether the cheat fired this frame and exposes the played frame read-only. The game needs the played frame for `present`, and the tests need it to compare.
  - The step allocates nothing.
- **Order inside the per-frame function** (plan: mode, then pad navigation and cheat, then dispatch):
  1. **Mode.**
     - Mouse mode without `keyboard_cursor`: the played frame is the raw frame.
     - Mouse mode with `keyboard_cursor`: `button_cursor` turns the raw frame into the played frame, as today.
     - Pad mode: the played frame has mouse x, y and buttons at 0, every `dpad` entry at 0, and the raw pad mask copied as is.
     - The played mouse in pad mode stays at (0,0) until step 1.4 gives it the synthetic cursor.
  2. **Pad navigation.** The slot step 1.4 fills. Empty in this step, so nothing reads the pad mask yet besides the cheat.
  3. **Cheat.**
     - Mouse mode: the cheat reads the raw frame's PICO-8 O and X, exactly as both binaries do today (raw, not played: the `keyboard_research` replay depends on that).
     - Pad mode: the cheat reads a one-frame view whose O is the pad mask's × and whose X is its ○. The raw `dpad` is never read.
     - The cheat module itself is not changed.
  4. **Dispatch.** The played input is installed in the dispatch context.
- `cart_update60` and `cart_draw` stay in the callers, right after the step, as today. The replay driver captures between draw and the next frame, and the game presents. Neither belongs in a shared input step.
- Because pad mode depends only on the pad mask, a raw live frame and the same frame reloaded from a pad recording produce identical played frames and identical cheat input. The unit test pins this.
- The module joins the engine closure (the source list the replay driver, the game and the cart tests link). It is integer-only code, so it passes `float-guard` and `float-grep`. If `pointer-guard` counts its file-scope state or struct fields, the ratchet file is updated by hand in the same commit, as that guard intends.

### Replay driver (`replay` binary)

- Reads the mode from the loaded replay (H-1.1-3) and initialises the input step with that mode and the header's `keyboard_cursor`.
- Each frame it hands the current raw frame to the input step, then updates, draws, captures and steps the input, in today's order.
- It no longer wires `button_cursor`, the cheat or the dispatch input itself.
- The cart now always reads a one-frame played input, never the whole loaded replay. The only readers, the mouse and the camera, read only the current frame, so mouse replays are unaffected; the unchanged hashes prove it.
- No new command-line option: the header alone decides the mode. The usage line and the replay-usage test are unchanged.

### Interactive game (`game` binary) and raylib backend

- **Command line:** `game [--mouse] [<recording.json>]`.
  - `--mouse` may come before or after the path.
  - Any other argument starting with `-`, or a second path, prints the usage line to stderr and exits with status 2, before the window opens.
- **Startup order:**
  1. Parse the arguments.
  2. Open the replay source (a failed load exits 1 before the window, as today).
  3. Resolve the mode. On a conflict, print a message naming the recording and saying it is a pad recording that `--mouse` cannot play, then exit 1 before the window. The same `assert(!IsWindowReady() …)` guard as the failed-load path applies.
- **Startup notice.** It prints one stderr line naming the mode. In pad mode, the line adds that pad controls are not playable yet and that `--mouse` plays with the mouse. This line is removed by the step that makes pad mode playable, no later than phase 2.
- **Backend init** takes the mode and `keyboard_cursor` and opens the session recorder with both.
  - The backend's input poll is not changed: it still fills the mouse fields and `dpad`, and the pad mask stays 0. Filling the pad mask is step 1.3.
  - The OS cursor stays hidden in both modes, as today.
- **The loop:**
  1. Take the replayed or polled raw frame.
  2. Set the frame-rate cap.
  3. Record the raw frame.
  4. Run the input step on it.
  5. Update, draw, and present the played frame.
- The game's own `button_cursor`, cheat and dispatch-input wiring are deleted: they live only in the input step now.

### Decisions taken without a human

No `guideline` agent could be spawned in this session. Each decision below applies the `guideline` rules (make the plan work, keep every hash safe, refuse widening, take the smaller change) and cites what it rests on.

- **D1. A recording's header beats the default; `--mouse` with a pad recording is a refusal, not an override.**
  - Why: plan §2 "that header decides the mode" and "the two modes never mix". The plan's keymap rule says "a key that silently does nothing is not acceptable", and the same goes for a flag.
- **D2. Fresh sessions drop `button_cursor`.**
  - Why: plan §2 "Mouse mode behaves exactly as today without `button_cursor`". Pad mode must not synthesise a mouse from the D-pad either. Only a `keyboard_cursor` recording keeps it (plan invariant until step 4.1).
  - The two tests that froze the old default (fresh session declares `keyboard_cursor`) are updated, per CLAUDE.md "update that test".
- **D3. The raylib backend does not fill the pad mask in this step.**
  - Why: plan step 1.3 owns the keymap and the backend fill. Folding it in would widen this step.
  - Pad mode is therefore inert for a human until steps 1.3–1.4. That is accepted and announced at startup (user story 11).
- **D4. The input step stops at installing the dispatch input; update and draw stay in the callers.**
  - Why: the plan's order "mode, then pad navigation and cheat, then dispatch" names input work only, and the two callers do different things after draw. This is the smaller change.
- **D5. The cheat module is not changed; the input step builds the view it reads.**
  - Why: this is the smaller change. The cheat's O/X edge logic stays covered by the mouse replays `cheat_code` and `keyboard_research`.
- **D6. In pad mode the played mouse is (0,0) with no button.**
  - Why: it is what a pad frame already reads as (H-1.1-5, H-1.1-7), so it adds no new behavior. Step 1.4 replaces it with the synthetic cursor.
- **D7. The headless pad-mode run is a unit test that drives the cart through the input step, not a replay file.**
  - Why: a pad replay cannot carry mouse fields (H-1.1-6, plan §2), so "a pad-mode run with a moving mouse" can only be fed below the file format. Feeding it through the shared step is also what the human's game does.
- **D8. The headless run forks the process after the game starts.** One child runs the quiet frames, the other the noisy frames, and their snapshots are compared.
  - Why: the runtime's random generator is a file-scope state with no seed setter, so two sessions in one process diverge. Fork gives both runs the same starting state without touching the runtime. CI runs Linux, where fork is available.
- **D9. Mode resolution lives in the replay source.**
  - Why: the game binary is never run (CLAUDE.md), so its rules must sit in a module a unit test links.
  - The game's argument parsing stays a few lines in `main` and is checked only by `make raylib` building.
- **D10. The keyboard-cursor replays are checked alongside the mouse ones.**
  - Why: plan §2 invariant "every step before 4.1 checks them alongside the mouse hashes", which is stricter than the step's own "every mouse `*.sha256`" line.

## Implementation Chunks

0. **Raylib baseline.**
   - Run `make raylib-vendor-setup` and `make raylib` on the unchanged tree, and record the result, or the fallback to the CI raylib job.
   - No code.
1. **Mode in the replay source and the recorder.**
   - Replay source: the mode accessor, the `--mouse` resolution and the new `keyboard_cursor` default.
   - Session recorder: takes and writes the mode.
   - Tests in the replay-source and recording-continuity unit tests. The old fresh-session `keyboard_cursor` assertions are updated.
   - The game binary is adapted only as far as needed to keep `make raylib` compiling (pass mouse and the header's `keyboard_cursor`).
2. **The shared input step, and the replay driver on it.**
   - New module with mouse passthrough, `button_cursor` and pad sanitisation, the cheat mapping and the dispatch install.
   - The replay driver switches to it.
   - Unit tests: played-frame contract per mode, the pad cheat sequence, mouse cheat unchanged, live frame against recorded-and-replayed frame.
   - `make e2e` with every mouse and `keyboard_*` hash unchanged.
3. **Headless pad-mode run.**
   - The forked quiet-against-noisy cart run in the input step's test (D8).
4. **The game binary on the shared step.**
   - `--mouse` parsing, mode resolution with conflict refusal, the startup notice, backend init with the mode, and the loop through the input step.
   - Delete the game's own input wiring.
   - `make raylib` builds, or the CI raylib job is green.

## Testing Decisions

- **What makes a good test here.**
  - Drive public interfaces only: open a replay source, resolve a mode, open and feed a recorder, init and step the input step, read the played frame, the cheat result, and the cart state through its existing accessors.
  - Never inspect a module's private fields.
  - Compare frames field by field, pad mask included, as the existing replay-source and continuity tests already do for the mouse fields.
- **Replay-source unit test** (prior art: its open-outcome and `keyboard_cursor` tests):
  - no recording → mode pad;
  - a mouse fixture without `"input"` → mouse;
  - a pad file written by the runtime's save in pad mode → pad;
  - the six cells of the resolution table, the conflict included;
  - no recording now declares no `keyboard_cursor` (the old test asserting 1 is updated);
  - a pad recording serves its frames in order with their masks.
- **Recording-continuity unit test** (prior art: its `run_session` loop, which mirrors the game's loop):
  - `run_session` takes the session mode from the replay source, with `--mouse` as a parameter;
  - a fresh pad session records a pad replay. Its reload is in pad mode and holds the live frames' masks only, even though the simulated live frames also carry mouse and `dpad` noise;
  - a resumed pad recording records replayed pad frames, then live ones, as one pad replay, and is itself resumable;
  - a fresh `--mouse` session records a mouse replay with no `"input"` and no `keyboard_cursor` (the old "fresh session records declared" test is updated);
  - the `keyboard_cursor` resume tests keep passing.
- **New input-step unit test** (links the engine closure plus the replay source, the recorder and the JSON input; prior art: the cart-level tests that call `cart_load`, `cart_init` and `cart_start_game(NULL)`):
  - **Mouse passthrough:** without `keyboard_cursor`, the played frame equals the raw frame.
  - **`button_cursor`:** with `keyboard_cursor`, the played frame equals what `button_cursor` alone produces from the same raw frames.
  - **Pad contract:** in pad mode, for raw frames carrying mouse movement, every mouse button, every `dpad` entry and every pad bit, the played frame has zero mouse, zero buttons, zero `dpad` and the raw mask.
  - **Pad cheat:**
    - In a started game, the pad presses × × ○ ○ × × ○ ○ (each press followed by a release frame) make the step report the cheat fired on the last press, and the economy reads 200 credits and 99 research.
    - The same sequence typed with the raw `dpad` O/X in pad mode never fires.
    - In mouse mode the `dpad` sequence O O X X O O X X fires and the pad buttons never do.
  - **Live against recorded:**
    - Feed a sequence of noisy live raw frames in pad mode (mouse moving, buttons pressed, `dpad` and pad bits varying, the cheat sequence included) through one input step, and through a session recorder opened in pad mode.
    - Reload the recording through a replay source and feed it through a second input step.
    - Frame by frame, the played frames and the cheat results are identical.
  - **Headless pad-mode run (D8):**
    - Load the assets, `cart_load`, `cart_init`, `cart_start_game(NULL)`, then fork.
    - Both children run the same number of pad-mode frames through the input step, each followed by `cart_update60` and `cart_draw`. About 120 frames: enough for camera inertia to show, and cheap.
      - The quiet child: raw frames that are all zero.
      - The noisy child: the mouse moves across the screen, all mouse buttons are pressed and released, every `dpad` entry is held, and the pad D-pad bits are held.
    - Each child writes a snapshot to a file under `tmp/`: camera position and velocity, the hovered entity of the main UI (or none), and the full state text.
    - The parent asserts the two snapshots are byte-identical and that both children exited 0.
    - Phase 2.1 deliberately gives the pad D-pad to the camera through pad navigation. That step revises this run's held pad D-pad bits, while the mouse and `dpad` noise stays.
- **Regression:**
  - `make e2e` keeps every one of the 26 mouse hashes and the 4 `keyboard_*` hashes unchanged.
  - `git diff --stat tests/e2e/` is empty.
  - `make test` and `make e2e` take no longer.
- **Not unit-tested:** the game binary's argument parsing and loop, which need raylib (D9). They are checked by `make raylib` building and are kept to a few lines delegating to tested modules.

## Out of Scope

- `keymap.cfg` and filling the pad mask from the keyboard or gamepad 0 (step 1.3).
- `pad_nav`, the synthetic cursor and any `pad_*` replay (step 1.4 onward).
- Any change to the replay JSON format, the loader or `tools/e2e_limits.sh` (step 1.1).
- Removing `button_cursor`, the `keyboard_cursor` field or the `keyboard_*` replays (step 4.1).
- Showing or hiding the OS cursor differently per mode (it stays hidden, as today).
- Changing the cheat's own logic.
- Audio.

## Further Notes

- Pad mode after this step is deliberately inert for a human. The pad mask stays 0 live until step 1.3, and nothing navigates until step 1.4. The value of this step is the shared input path, so that from step 1.4 on every pad replay exercises the human's code.
- The step note lists:
  - the `make raylib` baseline and final results (or the CI fallback);
  - which step 1.1 hypotheses held as written and which were renamed;
  - that no `*.sha256` changed.
