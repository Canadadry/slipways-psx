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

### Keyboard mapping (default, step 1)

Approved at step 1 review; speeds are defaults the step-5 trial may change.

- **D1 — Keyboard cursor is a session property, off by default.** The button → mouse layer runs only in a session that declares it: the interactive PC binary always does and records the declaration in the `events.json` it writes; a replay turns it on only if its `events.json` declares it. Without the declaration (every existing e2e replay and `worst-case/` game) nothing changes: directions scroll, O/X only feed the cheat, the cursor is the recorded mouse, so every `*.sha256` stays unchanged. Proposed declaration: top-level `"keyboard_cursor": true`, absent meaning off (step 2 owns the format; `p8_input_append` must stay byte-identical to `p8_input_save`). A resumed session keeps its recording's declaration, so a mouse recording made before step 3 resumes mouse-only.
- **D2 — The device used last owns the cursor.** The mouse takes it when the real mouse position differs from the previous frame's or a real mouse button changes state; the cursor jumps to the real mouse. The keyboard takes it on the rising edge of any of the six buttons; the cursor stays where it is drawn. The mouse owns it at session start. A device keeps the cursor while one of its buttons is held (a real mouse button; O or X): the other device takes it only after the release, so an arrow pressed during a mouse drag scrolls as today instead of dropping what the mouse carries. Only the owner's buttons count as mouse buttons, so a replay with no mouse movement is fully keyboard-driven after its first key press.
- **D3 — Directions move the cursor.** While the keyboard owns it, each held direction moves the cursor on its axis; two directions move it diagonally, each axis independently (no normalization). Speed: 1 pixel per frame for the first 10 frames a direction is held, then 2 (at 60 fps: fine aim for a sixth of a second, the 160-pixel screen in about 1.3 s). Integer position clamped to 0..159 × 0..119. The game already draws its cursor sprite at the mouse position.
- **D4 — O held is the left button, X held is the right button.** Frame for frame, like a real button: a click is a tap of O; a slipway is O pressed on a planet, moved to the target with O held, released; carrying a structure is the same gesture from its icon; X held with directions drags the map. Keys stay Z for O and X for X.
- **D5 — Scrolling: the cursor pushes against the screen edge.** While the keyboard owns the cursor, a direction scrolls only when the cursor already sits on that edge (x = 0, x = 159, y = 0, y = 119) and the direction pushes outward; the camera receives it exactly like a D-pad direction today, same speed and easing. Otherwise the direction moves the cursor and does not scroll. The X right-drag of D4 is a second way to scroll. No shoulder button.
- **D6 — The cheat stays on the raw O and X buttons.** It reads the six buttons before the layer turns them into clicks; sequence (O O X X O O X X) and reset rule unchanged. Typed while the keyboard owns the cursor, it also makes four left and four right clicks where the cursor stands. Its grant typed on the title screen is lost when the game starts, so type it on the map, after an X tap that resets the matcher if a click came before. On the map, empty space is not inert: a left tap there places the probe if none is out yet, and O held 15 frames or more confirms a space fold ($3, one clock tick). Right clicks on the background without movement drag the map by zero. Non-declaring sessions see no clicks, which keeps the cheat replays' hashes.

Rejected alternatives:
- *A toggle button for cursor mode* (shoulder or Select): a seventh button in every recorded frame and a mode to manage; D1 + D2 give the same compatibility without it.
- *O + X together toggles the mode*: O pressed first has already clicked, and it collides with the cheat's reset rule.
- *Implicit activation on any direction or O/X press, everywhere*: breaks `camera_drag_dpad` and the cheat replays.
- *Directions always scroll, the cursor moves only with a held button*: two gestures for one cursor, and a slipway drag needs three keys at once.
