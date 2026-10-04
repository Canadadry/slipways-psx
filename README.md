# slipways-love

*Slipways* (PICO-8 edition) ported to C, running on PC through raylib.

## How it's built

- Ported from `pico8/lua.lua` (the cart's decompiled source), which stays as an archive. The C code is now the reference; known divergences are listed in `docs/porting-gaps.md`.
- The game talks to a fixed PICO-8 API (a 160×120 software framebuffer) served by two backends: a headless replay (`events.json` in, text dump of the framebuffer out) and interactive raylib.
- A raylib session records its input as `events.json`: whatever a human played, an agent can replay without a window.
- `worst-case/full_game.json` is a fully built-up game (50 planets, 192 slipways) that measures worst-case memory use:
  - `make state EVENTS=worst-case/full_game.json` writes each zone's peak and capacity to `tmp/state.txt`.
  - `make state` without `EVENTS` writes there the highest peak of each zone and cap across this game and every e2e, with its limit and the replay that reaches it, and prints the bytes left under `ZONE_BUDGET_BYTES`. It fails if a replay stops, or if a peak eats the 1/8 margin its limit was sized with (`9 × peak > 8 × limit`); the proven ceilings and fully reserved list slots listed in `tools/state_peaks.sh` only need to fit their limit.
  - `make full-game` (outside `make test-all`, run by CI) fails if a `p8_fatal` stops this game before its last frame, or stops one of its variants. It has three variants: `full_game_new_game.json` (a new game started after it), `full_game_pause_restart.json` (cut short by pause, "restart" and a new game) and `full_game_pause_idle.json` (cut short by a pause menu left open for 300 frames). It also replays the title screen left idle for 300 frames (`title_idle.json`). `make state` replays all of these too.

## To do

Rule for this list: each item is one clear sentence of at most 30 words, readable without any outside reference.

- [ ] On real PICO-8, check three behaviors: how -0.0001 prints, the color of a circle drawn with a nil color, and sin/cos between multiples of 1/64.
- [ ] Show a budget for slipways and relays, and block adding more once it is full.
- [ ] Add a cheat that grants bonus score points, so a short e2e can show the score panel's lit stars.
- [ ] Before adding any new place where memory is freed, make the list of references cut by `forget` (`src/game/cart.c`) mechanical: the grep inventory missed three (the chosen difficulty, `slipway->end`, `targeted_by`).
- [ ] On PC, before the PSX port, recompute only the trade routes a change affects, instead of rebuilding everything on every economy recompute.
- [ ] Decide on GAP-26: an off-screen planet loses the halo of the held object once that object is released, where the cart would reread its memory.
