# slipways-love

*Slipways* (PICO-8 edition) ported to C, running on PC through raylib.

## Running the game

```sh
make raylib
tmp/game            # keyboard or gamepad
```

Run it from the repository root. Each session is recorded in `tmp/`; pass a recording to resume it; replaying a recording replaces `save/slipways.sav` with the recording's startup save. Space bookmarks a frame.

The game saves after every action, and the title's `resume` row brings that game back; a finished game is erased.

## Controls

| Button | Keyboard (QWERTY) | AZERTY | Gamepad |
|---|---|---|---|
| D-pad | arrows | arrows | D-pad |
| × confirm | X | X | bottom |
| ○ back | C | C | right |
| □ | Z | W | left |
| △ planet menu | V | V | top |
| L1 / R1 tabs | A / S | Q / S | shoulders |
| Start pause | Enter | Enter | right middle |

Keys can be changed in `keymap.cfg`.

- **Menus:** the D-pad moves, × picks, ○ closes.
- **Tabs:** L1/R1 switch between map, build, end turn, tech and score.
- **Aim mode** (crosshair): the D-pad moves the camera. × launches a probe, hold × to fold space. △ opens a planet's menu. □ or ○ goes back to the object nearest the crosshair.
- **Network mode** (blinking planet): Left/Right pick a slipway, × travels along it. □ enters aim mode on this planet.
- **Building a slipway:** × on a hovered colony starts it, the D-pad aims at another object, × builds. The line turns red if it can't be built. ○ cancels it.
- **Build tab:** Left/Right pick a structure, × takes it, × again places it, ○ cancels.
- **End turn tab:** × ends the year.

## To do

Rule for this list: each item is one clear sentence of at most 30 words, readable without any outside reference.

- [ ] Add a cheat that grants bonus score points, so a short e2e can show the score panel's lit stars.
