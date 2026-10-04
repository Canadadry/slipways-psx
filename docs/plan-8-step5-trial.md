# Plan 8, step 5 — human keyboard trial (checklist)

Steps 0 to 4 of `docs/plan-8.md` are done. This step needs a human at a PC. Delete this file once step 5 is settled.

## Before playing

- `make raylib-vendor-setup` (once), then `make raylib`. Run `tmp/game` (no argument = new game).
- Keys are read **by position, US layout**: on AZERTY the left button is the key labelled **W**, the right button is **X**. The arrows are the arrows.
- Start a **new game** (a resumed old recording stays mouse-only, see D1).

## What to check, keyboard only (don't touch the mouse)

1. **Title:** the arrows move the cursor; W/Z on a difficulty row starts the game.
2. **Cursor feel:** slow for ~1/6 s, then faster (1 then 2 px/frame after 10 frames). Can you aim at a planet? Is it too slow to cross the screen?
3. **Scrolling:** push the cursor against an edge to scroll; X held + arrows drags the map. Are all planets reachable?
4. **Probe and space fold:** tap on empty space away from planets (a probe or fold within ~20 px of a planet is refused); hold ~15 frames to confirm a fold.
5. **Colonize:** open a planet's build menu and colonize.
6. **Slipway:** hold W/Z on a colony, move to another, release. Also try a target **off screen** (keep holding and push the edge).
7. **Research:** open the tech screen, invent something.
8. **Pause menu:** pause button bottom-left, its rows, "restart".
9. **Cheat:** on the map, tap X once, then O O X X O O X X (W W X X W W X X on AZERTY) → $200 and 99 research. On the title screen it is wiped when the game starts (D6).
10. **End of game:** play to the final score, then "again?".
11. **Mouse handover:** move the mouse → the cursor jumps to it; press an arrow → the keyboard takes over without a jump. Press an arrow **during a mouse drag**: the drag must not drop (D2).
12. Anything hover-only or unreachable by keyboard.

## After playing (an agent can do this part)

- Write the findings into "Settled decisions" of `docs/plan-8.md` (speeds, edge scroll, anything awkward). If the game is unplayable, go back to step 1.
- The window records to `tmp/events_recorded_<timestamp>.json` (space bar bookmarks a frame in its `.markers.txt`: use it to flag problems). Commit that file, then run `make state EVENTS=<file>` and compare each peak with `worst-case/full_game.json`'s. If any peak is higher, add it to `worst-case/` and `FULL_GAME_EVENTS`, then run `make state`, `make full-game` and `make test-all`.
- Known weakness: since plan 12 the `full_game*` replays diverge at frame 358 and end small (8 colonies, 2 slipways). A real full keyboard game in `worst-case/` gives the margin guard of `make state` its teeth back.
- Then delete this file and mark plan 8 complete.
