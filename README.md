# slipways-love

*Slipways* (PICO-8 edition) ported to C, running on PC through raylib.

## How it's built

- Ported from `pico8/lua.lua` (the cart's decompiled source), which stays as an archive. The C code is now the reference; known divergences are listed in `docs/porting-gaps.md`.
- The game talks to a fixed PICO-8 API (a 160×120 software framebuffer) served by two backends: a headless replay (`events.json` in, text dump of the framebuffer out) and interactive raylib.
- A raylib session records its input as `events.json`: whatever a human played, an agent can replay without a window.
- `worst-case/` holds fully built-up games: `make state` reports zone peaks, `make full-game` fails on any `p8_fatal`.

## Controls

- The mouse points and clicks as in the cart.
- The arrows move the game's cursor, slowly for a sixth of a second, then faster.
- Z (PICO-8's O) is the left mouse button and X the right one, held and released like a mouse button.
- Pushing the cursor against a screen edge scrolls the map; X held with the arrows drags it.
- Moving the mouse takes the cursor back from the keyboard at once.
- Z and X are key positions named after the US layout: on an AZERTY keyboard, the left button is the key labelled W, and X stays X.
- The space bar bookmarks the current frame of the recording in its `.markers.txt` file.
- During a replay, Page Up plays faster, Page Down slower, and End removes the frame-rate cap.
- A resumed recording made before the keyboard cursor stays mouse-only: there, the arrows scroll the map.

## To do

Rule for this list: each item is one clear sentence of at most 30 words, readable without any outside reference.

- [ ] Add a cheat that grants bonus score points, so a short e2e can show the score panel's lit stars.
