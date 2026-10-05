# slipways-love

*Slipways* (PICO-8 edition) ported to C, running on PC through raylib.

## How it's built

- Ported from `pico8/lua.lua` (the cart's decompiled source), which stays as an archive. The C code is now the reference.
- The game talks to a fixed PICO-8 API (a 160×120 software framebuffer) served by two backends: a headless replay (`events.json` in, text dump of the framebuffer out) and interactive raylib.
- A raylib session records its input as `events.json`: whatever a human played, an agent can replay without a window.
- `worst-case/` holds fully built-up games: `make state` reports zone peaks, `make full-game` fails on any `p8_fatal`.

## Running the game

- `make raylib` builds `tmp/game`; run it as `tmp/game [--mouse] [<recording.json>]`.
- The game starts in pad mode by default. Pad controls are not playable yet, so run with `--mouse` to play.
- A recording decides its own mode: a mouse recording resumes in mouse mode, a pad recording in pad mode, and `--mouse` with a pad recording is refused.

## Controls (mouse mode)

- The mouse points and clicks as in the cart.
- The arrows scroll the map, as the cart's D-pad does.
- Z and X are PICO-8's O and X buttons. On an AZERTY keyboard, Z is the key labelled W.
- The space bar bookmarks the current frame of the recording in its `.markers.txt` file.
- During a replay, Page Up plays faster, Page Down slower, and End removes the frame-rate cap.
- A resumed recording that declares the keyboard cursor keeps it: the arrows move the game's cursor, Z and X are the left and right mouse buttons, and moving the mouse takes the cursor back.

## To do

Rule for this list: each item is one clear sentence of at most 30 words, readable without any outside reference.

- [ ] Add a cheat that grants bonus score points, so a short e2e can show the score panel's lit stars.
