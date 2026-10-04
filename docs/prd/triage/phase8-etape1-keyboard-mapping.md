---
title: "Plan 8, step 1 — decide the keyboard mapping"
description: "Nobody has decided which key clicks, how the map scrolls and how the cheat survives once the keyboard drives the cursor, so steps 2 to 4 have nothing to build against."
status: needs-triage
---

## Problem Statement

Plan 8 wants a full game played with the keyboard alone, and the keyboard stands in for the PSX controller: the six PICO-8 buttons (four directions, O, X), plus shoulder buttons if needed. Today those six buttons already mean something:

- the four directions scroll the map (the camera reads them directly, at its own eased speed);
- O and X feed the cheat (O O X X O O X X, on their rising edges; pressing both on the same frame resets the sequence);
- the game has no keyboard path to the cursor at all: everything that matters (menus, slipway drags, picking up a structure, research, pause rows) goes through the mouse, read in one place only.

The game needs exactly these mouse gestures: hover, left press, left hold while moving (slipway drag from planet to planet, carrying a structure from an icon or a probe from the background), left release (the drop), and right press-hold-move on the background (map drag, the only use of the right button).

Before step 2 writes the button → mouse layer, someone must decide what each button does in cursor mode, how scrolling stays reachable, and how the cheat keeps working. The hard part is a constraint the plan sets: **every existing `*.sha256` in `tests/e2e/` stays unchanged**. Thirteen of the 26 e2e replays, and four of the five `worst-case/` games, press O and X (the cheat) with the mouse standing still, and `camera_drag_dpad` holds right and down with the mouse standing still. Any rule that turns direction or O/X presses into cursor movement or clicks *in those replays* breaks their hashes. "The real mouse moving takes the cursor back" does not help there: the mouse never moves while those buttons are held.

## Solution

An agent (this step) writes a default mapping into the "Settled decisions" section of `docs/plan-8.md`. It is a decision, not code. Step 2 does not start until a separate review approves or corrects it (in this unattended run a fresh `guideline` agent plays the human's part; see Further Notes).

The default mapping below is the content to write. It is phrased as behavior a player can check, not as code.

**D1 — Keyboard cursor is a session property, off unless the session says so.**
The button → mouse layer runs only in a session that declares it. The interactive PC binary always declares it and records the declaration into the `events.json` it writes; a replay turns the layer on only when its `events.json` declares it. A file that does not declare it — every existing e2e replay and every `worst-case/` game — runs exactly as today: directions scroll, O/X only feed the cheat, the cursor is the recorded mouse. This is what keeps every `*.sha256` unchanged. Proposed name for the declaration: a top-level `"keyboard_cursor": true` in `events.json`, absent meaning off (step 2 owns the exact format, and the recorder must keep `p8_input_append` byte-identical to `p8_input_save`).

**D2 — Who owns the cursor: the device used last.**
In a declaring session, the cursor has one owner at a time:
- the mouse takes it whenever the real mouse position differs from the previous frame's, or a real mouse button changes state; the cursor jumps to the real mouse;
- the keyboard takes it on the rising edge of any of the six buttons; the cursor stays where it is drawn and moves from there (no jump).
At the start of a session the mouse owns the cursor, so the cursor starts at the recorded mouse position. While a device owns the cursor, only that device's buttons count as mouse buttons. A keyboard replay with no mouse movement at all (step 4) is therefore fully keyboard-driven after its first key press.

**D3 — Directions move the cursor.**
While the keyboard owns the cursor, each held direction moves it on its axis; two directions move it diagonally (each axis moves on its own, no normalization). Speed: 1 pixel per frame for the first 10 frames a direction is held, then 2 pixels per frame (at 60 frames per second: fine aim for a sixth of a second, then the 160-pixel screen in about 1.3 s). The cursor stays an integer position clamped to the screen, 0..159 by 0..119. The cursor sprite needs no change: the game already draws it at the mouse position.

**D4 — O is the left button, X is the right button, as held states.**
While the keyboard owns the cursor, O held means "left button down" and X held means "right button down", frame for frame, exactly like a real mouse button. Press, hold and release fall out of that: a click is tapping O; a slipway is O pressed on a planet, directions moved to the target planet with O still held, O released; carrying a structure is the same gesture from its icon; X held with directions drags the map like a right-drag. Keyboard keys stay the ones the backend reads today: Z is O, X is X.

**D5 — Scrolling: the cursor pushes against the screen edge.**
While the keyboard owns the cursor, a direction scrolls the map only when the cursor already sits on that edge of the screen (left at x = 0, right at x = 159, up at y = 0, down at y = 119) and the direction pushes outward. The camera then receives that direction exactly as it receives a D-pad direction today, with today's speed and easing. A direction that is not pushing against an edge moves the cursor and does not scroll. The right-drag of D4 stays available as a second way to scroll. No shoulder button is used.

**D6 — The cheat stays on the raw O and X buttons.**
The cheat keeps reading the same six buttons, before the layer turns them into clicks, with the same sequence and the same reset rule; nothing in the cheat changes. Typing it while the keyboard owns the cursor also produces four left clicks and four right clicks where the cursor stands: park the cursor on empty space first (on the title screen, away from the three difficulty rows). Right clicks on the background with no movement drag the map by zero. Sessions that do not declare the keyboard cursor see no clicks at all, which is why the cheat replays keep their hashes.

**Alternatives considered and rejected (recorded so the review can overturn them):**
- *A toggle button for cursor mode* (a shoulder button, or Select): works, but needs a seventh button in every recorded frame and asks the player to manage a mode; D1 + D2 give the same compatibility with no extra button.
- *O + X pressed together toggles the mode*: no format change, but a press of O before X has already clicked, and it collides with the cheat's reset rule.
- *Implicit activation on any direction or O/X press, everywhere*: breaks `camera_drag_dpad` and the cheat replays.
- *Directions always scroll, cursor moves only with a held button*: two gestures for one cursor, and slipway drags need three keys at once.

## User Stories

1. As a keyboard player, I want the arrows to move the visible cursor, so that I can point at any planet, icon or menu row without a mouse.
2. As a keyboard player, I want the cursor to start slow and then speed up, so that I can both aim at a 3-pixel planet and cross the screen quickly.
3. As a keyboard player, I want diagonal movement when two arrows are held, so that reaching a corner does not take two trips.
4. As a keyboard player, I want the cursor to stop at the screen border, so that it never disappears off screen.
5. As a keyboard player, I want Z to click, so that the title screen difficulty rows, the year skip, the pause button and the pause rows all respond.
6. As a keyboard player, I want Z held to behave like the left button held, so that I can draw a slipway from one planet to another by holding Z while moving.
7. As a keyboard player, I want releasing Z over a target to drop what I carry, so that placing a structure from its icon works like with the mouse.
8. As a keyboard player, I want a refused slipway to be refused exactly as with the mouse, so that the rules do not depend on the input device.
9. As a keyboard player, I want pushing the cursor against a screen edge to scroll the map, so that I can reach planets outside the current view.
10. As a keyboard player, I want edge scrolling at today's D-pad speed and easing, so that scrolling feels the same as before.
11. As a keyboard player, I want X held with the arrows to drag the map, so that every mouse gesture of the game has a keyboard equivalent.
12. As a keyboard player, I want hovering with the keyboard cursor to show tooltips and hover states, so that I can read planet and project information.
13. As a keyboard player, I want the research screen reachable and its projects clickable, so that I can research without a mouse.
14. As a keyboard player, I want to finish a game and read the end screen, so that a whole game is playable on the keyboard.
15. As a player who uses both, I want moving the real mouse to take the cursor back at once, so that switching device needs no menu.
16. As a player who uses both, I want pressing a key to take the cursor back from the mouse without the cursor jumping, so that I can continue from where I am pointing.
17. As a player who uses both, I want only the owning device's buttons to count, so that a resting finger on a key does not interfere with a mouse drag.
18. As a cheat user, I want O O X X O O X X on Z and X to keep granting 99 research and $200, so that the cheat is not lost.
19. As a cheat user, I want to know that the cheat's presses also click where the cursor is, so that I park the cursor on empty space before typing it.
20. As a developer, I want every existing e2e replay to keep its hash, so that the port's source of truth is untouched by plan 8.
21. As a developer, I want the keyboard cursor declared per session in `events.json`, so that old replays stay mouse replays and new ones can be keyboard replays.
22. As a developer, I want the interactive binary to record that declaration, so that a human keyboard session replays identically under `make replay` and `make state`.
23. As a developer writing step 2, I want the mapping as checkable behaviors (speeds, edges, owner rules), so that I can unit-test the layer against it without guessing.
24. As a developer writing step 3, I want the keys to stay Z, X and the arrows, so that the raylib wiring adds no new key for play.
25. As a developer writing step 4, I want a keyboard replay with a constant mouse position to be fully keyboard-driven, so that "no mouse movement at all" is a meaningful test.
26. As the reviewer of this step, I want rejected alternatives listed with their reason, so that I can overturn the default with full information.
27. As a future PSX port, I want the mapping to use only the six PICO-8 buttons, so that it maps onto the controller without a mouse.
28. As the human trying the game at step 5, I want the default speeds called defaults, so that my findings can change them without reopening the whole mapping.

## Implementation Decisions

- The deliverable is documentation only: the "Settled decisions" section of `docs/plan-8.md` gains the mapping D1 to D6, worded as above, with the rejected alternatives in one short list. Step 1's line in "Steps" stays as it is (the plan file is not a progress log); nothing else in the repository changes. No other doc is needed: `README` controls are written when step 3 wires the keys, since `docs/` only states what is true or still to do.
- No module is built or modified in this step. For steps 2 and 3, the mapping implies, without fixing their design:
  - one core layer (no raylib, outside `game/` input readers) that takes the frame's raw six buttons and raw mouse, plus its own small state (owner, cursor position, how long each direction has been held, previous raw mouse), and returns the mouse the game reads and the directions the camera reads;
  - the cheat keeps consuming the raw buttons;
  - the camera consumes the layer's filtered directions in a declaring session (only edge pushes), the raw ones otherwise;
  - the session declaration travels in `events.json`; the loader treats its absence as off.
  These are pointers for steps 2 and 3, not decisions they must follow to the letter.
- Speeds (1 then 2 pixels per frame, switch after 10 frames) are defaults that step 5's human trial may change by editing "Settled decisions"; they are not a contract for the e2e hashes of step 4 beyond what step 4 records.
- The PICO-8 O/X keyboard keys stay Z and X; the arrows stay the arrows. No shoulder button is used in plan 8.

## Testing Decisions

- This step writes no code, so it adds no test. Its checks are:
  - `make test-all` passes, with every `*.sha256` in `tests/e2e/` unchanged (a docs-only change cannot move them; the run confirms nothing else drifted);
  - the plan file's "Settled decisions" contains D1 to D6 and the rejected alternatives, in English;
  - the approval gate below has passed before step 2 starts.
- **Approval gate (a separate review, not part of this step's work):** a reviewer (the human, or in an unattended run a fresh `guideline` agent) reads the mapping and checks that:
  1. every mouse gesture the game uses (hover; left press, hold-move, release; right press-hold-move on the background) has a keyboard equivalent;
  2. no existing e2e replay or `worst-case/` game can reach the layer (D1): verifiable by reading which replays press directions or O/X (`camera_drag_dpad`, the cheat replays) and confirming none declares the keyboard cursor;
  3. scrolling stays reachable with the keyboard owning the cursor (D5);
  4. the cheat keeps its sequence and stays reachable, and its side clicks are stated (D6);
  5. nothing in the mapping needs more than the six PICO-8 buttons.
  The reviewer approves or writes corrections into "Settled decisions"; step 2 starts from that text.
- What a good test looks like later (for step 2, recorded here because the mapping defines it): behavior through the layer's interface only — given raw frames in, assert the mouse and camera directions out (cursor moves 1 then 2 pixels, clamps at the border, edge push forwards a direction, mouse movement takes ownership, absent declaration passes everything through untouched). Prior art: the unit tests of the cheat and of the mouse button state in `tests/`, and the e2e replays with their `.txt` notes.

## Out of Scope

- Any code: the button → mouse layer (step 2), the raylib wiring and the recorded declaration (step 3), the keyboard e2e replays (step 4), the human trial (step 5).
- The exact `events.json` format of the declaration (step 2/3 own it; D1 only proposes a name).
- A mode that jumps from planet to planet (excluded by plan 8).
- A physical controller on PC, shoulder buttons, PSX button assignment.
- Audio.
- Step 0's `make state` margin guard.

## Further Notes

- **Unattended run.** No human is available for this step. The to-prd check-ins ("do these modules match", "which modules get tests", "does this need cutting") were resolved by applying the `guideline` skill's rules to the sketch (simplicity, no speculative features, surgical scope): no modules are built, so no module needs tests or cutting; the default mapping avoids a seventh button for that reason. The step's own "the human approves or corrects" checkpoint is the approval gate in Testing Decisions, run as a separate review before step 2.
- **Facts the mapping rests on (read in the code at this commit):** the camera sums the held directions times 3 and eases toward it; the cheat reads O and X rising edges and resets on a simultaneous pair; the mouse is read once per frame into a left button and a right button (middle counts as right); the right press is handled by the background only, and starts a map drag; the backend reads arrows, Z (O) and X (X) and the three mouse buttons, and uses Space for markers and Page Up, Page Down, End for replay speed — none of which the mapping touches; the game draws its own cursor sprite at the mouse position and the system cursor is hidden; the cart runs at 60 updates per second. The cart also accepts player-2 directions for scrolling; the port does not, and the mapping does not need them.
- **Hypotheses about step 0** (not done yet):
  - **H-0-1**: step 0 only changes `make state` and its scripts (margin check, remaining space under `ZONE_BUDGET_BYTES`); it does not touch the input format, `events.json`, the backend's keys or the game. D1 and D3 to D6 do not depend on it.
  - **H-0-2**: step 0 adds no replay that presses directions or O/X; the list of replays D1 must keep off the layer is the one read here (the cheat replays, `camera_drag_dpad`, `worst-case/`).
  - **H-0-3**: the invariant "`make state` passes with the margin" is unaffected by a docs-only change.
  - **Sections to re-check once step 0 lands:** Problem Statement (count of replays pressing directions or O/X), D1 (the set of files that must stay off), Testing Decisions (the invariant list), and this note.
