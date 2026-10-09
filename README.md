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
| D-pad move | arrows | arrows | D-pad |
| × act | X | X | bottom |
| ○ back | C | C | right |
| □ aim mode | Z | W | left |
| △ menu | V | V | top |
| L1 build bar | A | Q | left shoulder |
| R1 tech / score | S | S | right shoulder |
| Start pause | Enter | Enter | right middle |
| Select attention / end year | Right Shift | Right Shift | left middle |

Keys can be changed in `keymap.cfg`.

Point at a thing, × acts on it, ○ backs out.

- **Jump mode** (a blinking circle on an object; every game starts here): the D-pad jumps to the next object in that direction, anywhere on the map, and holding it keeps jumping. Nothing moves when no object lies that way. The camera follows only when the object nears the edge of the screen. × on a colony or structure starts a slipway; × on a known planet opens its projects. △ opens the hovered object's menu, and △ again closes it. □ enters aim mode. ○ jumps back to the last object × acted on.
- **Aim mode** (a crosshair at the centre): the D-pad moves the camera, faster the longer it is held. × on empty space launches the probe; once a probe is out, hold × to fold space. △ does nothing here. □ or ○ goes back to jump mode on the object nearest the crosshair.
- **Building a slipway:** after × on a colony or structure, the D-pad aims at another object. Above it the game shows the cost, or why it can't be built: "too far", or a forbidden sign when the two are already linked. × builds; a refused one aims again from the same start. Once built, the next slipway starts from the object just reached, so × D-pad × D-pad × chains them. ○ drops it. A full slipgate starts nothing.
- **Build bar (L1):** Left/Right pick a structure, × takes it. The D-pad then moves the camera under it, × places it and ○ cancels it. L1 or ○ closes the bar; R1 closes it and opens tech.
- **Tech and score (R1):** R1 opens tech, then score, then closes them. ○ goes back to the map, and L1 opens the build bar.
- **Select** (on the map, in jump or aim mode): a tap jumps to the next colony or structure whose panel lists a need. Holding it about half a second ends the year, once per hold. After a year ends, a fresh press within a quarter of a second ends one more.
- **Menus and pause:** the D-pad moves, × picks, ○ closes. A × held from before a menu opened picks nothing until pressed again. Start pauses from the map or the build bar, not from a menu, tech or score; Start or ○ closes pause.
- **○ order:** each ○ backs out one step: a pause or box (the year box included), then a slipway or a structure being placed, then aim mode, then the jump back to the last object.

## To do

Rule for this list: each item is one clear sentence of at most 30 words, readable without any outside reference.

- [ ] Add a cheat that grants bonus score points, so a short e2e can show the score panel's lit stars.
