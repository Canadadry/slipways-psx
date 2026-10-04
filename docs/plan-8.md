# Plan 8 — Play without a mouse, on the keyboard

Draft: run it through `plan-converge` before executing anything. Second of the five plans toward a PSX `.bin/.cue` (plans 7 to 11).

## 1. Goal

A full game can be played on PC **with the keyboard alone**: menus, colonization, slipways, research, end of game. The keyboard stands in for the PSX controller: the six PICO-8 buttons (four directions, O, X), plus the controller's shoulder buttons if needed.

The mouse stays connected. The existing e2e replays keep using the mouse and their `*.sha256` stay unchanged. Invariant for every commit: `make test-all` passes; `make sanitize-net`, `make full-game` and `make state` (with the margin, see plan 8 step 0) pass too.

## 2. Constraints already in the code

- The arrow keys scroll the map (`src/game/scroll.c`).
- The cheat reads O/X sequences (`src/game/cheat.c`).
- The game reads the mouse only through `p8_input_read_mouse` (in `src/game/mouse.c` alone).

## 3. Approach

A core layer, with no raylib, that turns button presses into a cursor and clicks:
- a cursor mode, where the directions move a cursor and a button clicks;
- map scrolling stays reachable, for example when the cursor pushes against the screen edge;
- the cheat stays reachable.

When the real mouse moves, it takes the cursor back from the keyboard.

## 4. Steps

0. **Margin guard.** `make state` fails when the peak of a zone or cap exceeds its limit minus a 1/8 margin (the limit itself for `button`, `row` and `particles_help_screen`, whose ceilings are proven), and prints the space left under `ZONE_BUDGET_BYTES` (1,980 bytes at the end of plan 7).
1. **Decide the mapping**: which key clicks, how scrolling works, how the cheat stays compatible. An agent writes a default mapping here; the human approves or corrects it before any code.
2. **Button → mouse layer** in the core, unit-tested.
3. **raylib wiring** for the keys (the backend already reads the arrows, Z and X).
4. **Keyboard e2e**: with no mouse movement at all, one short replay per screen named in the goal (menus, colonization and slipway, research, end of game).
5. **Human trial**: one game played on the keyboard on PC. Findings go in "Settled decisions"; if it's unplayable, back to step 1. The recorded `events.json` is replayed by `make state`: if a peak exceeds `full_game.json`'s, it joins `worst-case/` and `FULL_GAME_EVENTS`.

## 5. Settled decisions

- The keyboard is enough: it represents the controller, so no physical controller is needed on PC.
- A mode that jumps from planet to planet is **out of this plan**.
- The keyboard e2e come before the human trial: they catch, without a window, a screen that can't be reached by keyboard.
