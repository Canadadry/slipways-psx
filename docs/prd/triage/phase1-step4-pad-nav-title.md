---
title: "pad_nav skeleton and panel navigation on the title screen"
description: "In pad mode the D-pad and × reach nothing: no synthetic cursor exists, so a pad player cannot even pick a difficulty on the title screen."
status: needs-triage
---

## Hypotheses on step 1.3

Checked against step 1.3 as landed (its step note in `docs/plan-15.md`): every hypothesis holds. The pad-mode startup notice was reworded in 1.3; keep that wording.

Step 1.3 (`docs/prd/triage/phase1-step3-keymap.md`: `keymap.cfg`, and the raylib backend filling the pad mask) is not done while this PRD is written. This PRD reads only committed code (`HEAD`: steps 1.1 and 1.2 landed, 1.3 not). Step 1.4 hardly depends on it: every check in this step feeds pad masks straight into the input step (unit tests) or from a pad replay (e2e). Neither goes through the backend or the keymap.

**Rule for the implementer:** if step 1.3 landed with a different name or spelling, use what landed. Only a hypothesis whose *behavior* turns out false needs a line in the step note. When that happens, its fallback applies.

- **H-1.3-1** Step 1.3 leaves `p8_input_frame_t`, the 11 `P8_PAD_*` bits, the pad replay format, and the input step's interface and `TODO step 1.4` slot unchanged. It only fills `pad_buttons` in the backend's poll. *Fallback if the slot moved:* pad navigation runs wherever the shared per-frame step now sits, after the mode is applied and before the cheat and the dispatch install.
- **H-1.3-2** Step 1.3's diff stays in the platform layer, the new keymap module, `keymap.cfg`, the game's startup, the Makefile (the keymap test, the raylib target) and the float-grep exclusions. Step 1.4 touches none of the platform files and does not touch the game's `main`. The only overlap is lists (`UNIT_TESTS`, source lists), so any merge is trivial.
- **H-1.3-3** The game still prints its pad-mode startup notice ("pad controls are not playable yet…") after step 1.3. Step 1.4 keeps it word for word, because only the title screen becomes usable with the pad and a game is still not playable (phase 2). *Fallback if step 1.3 removed it:* do not restore it.
- **H-1.3-4** Whether step 1.3 has landed or not changes no check of this step. Without step 1.3, a human cannot drive the title screen from the keyboard. That is expected, and nothing in this step claims it.

## Problem Statement

Plan 15 makes the game playable with a PSX-style pad. After steps 1.1 and 1.2, the game starts in pad mode and pad replays exist. The shared input step then zeroes the mouse and hands the cart only the pad mask, and nothing in the game reads that mask except the cheat. So in pad mode:

- the title screen shows four difficulty buttons that no pad press can focus or click, and a pad player cannot start a game at all;
- no panel can be used with the pad: not the title, the planet menu, the tech panel, the score panel or the pause menu;
- every later step of the plan (free mode, network mode, tabs, build, pause) needs one mechanism that turns pad presses into what the game already understands. Without it, each step would duplicate game rules in a navigation layer.

The plan also asks for proof that its "generic panel navigation" idea holds before phase 2 builds on it. Every item of every panel listed in §2 "Panels and menus" must be reachable with the D-pad. If a panel cannot be navigated generically, we need to know now.

## Solution

- **A new game module, `pad_nav`,** runs inside the shared input step in pad mode. Each frame it turns the pad mask into a **synthetic cursor position and a synthetic left button**, written into the played frame. Everything downstream is the existing mouse path, unchanged: the mouse entity, `ui_update_hover`, `ui_handle_button`, `left_press`, drag and drop. No game rule is duplicated.
- **Generic panel navigation.** While a panel is open, the focusable items are the clickable elements of the top UI layer, the same entities the mouse can hover and click. The D-pad moves the focus to the nearest item in the pressed direction. The synthetic cursor sits on the focused item, so the game draws it as hovered, tooltip included. × is the left button: pressing × clicks the focused item, and holding × holds the button.
- **On the title screen**, the first difficulty is focused, Up/Down move between the four rows, and × starts the game with the focused difficulty.
- **Proof for every panel.** A unit test per panel of §2 opens it directly and checks two things. Every item is reachable from the initial focus. Each item, once focused, is the item the game hovers. Panels that turn out not to exist as the plan describes them are recorded in the step note, to settle before phase 2 (see Further Notes).
- **The first pad replay, `tests/e2e/pad_title.json`,** picks a difficulty with the D-pad and ×, and its `.txt` records the end state checked with `make state`.

## User Stories

1. As a pad player, I want the first difficulty row of the title screen focused as soon as the rows appear, so that I know where I am without pressing anything.
2. As a pad player, I want the focused row drawn exactly as a row under the mouse is drawn (colour 12 instead of 13), so that the focus looks like the game I already know.
3. As a pad player, I want Down to move the focus to the next row below and Up to the next row above, so that I can choose any difficulty.
4. As a pad player, I want Up on the top row and Down on the bottom row to do nothing, so that the focus never jumps somewhere unexpected.
5. As a pad player, I want Left and Right to do nothing on the title screen, where every row is in one column, so that a sideways press never changes my choice.
6. As a pad player, I want a held direction to move the focus once, so that I never overshoot a row (the plan's no-auto-repeat rule).
7. As a pad player, I want × to choose the focused difficulty and start the game, so that one press does what one click does.
8. As a pad player, I want × held to act as a held mouse button, so that holds and drags work in later steps exactly as with the mouse.
9. As a pad player, I want a move and a × pressed on the same frame to click the newly focused item, so that fast input does what I meant.
10. As a pad player, I want the focus to stay on the same row while the title rows slide in, so that the screen does not flicker.
11. As a pad player, I want a tooltip to show for the focused item wherever the mouse would show it, so that I can read it before I choose.
12. As a pad player, I want items without an action (an unaffordable tech, a description row) to stay focusable, so that I can still read their tooltip, as with the mouse.
13. As a pad player, I want ○, □, △, L1, R1 and Start to do nothing in panels for now, so that no half-built behavior surprises me before the steps that define them.
14. As a pad player, I want the game itself to ignore the D-pad and × after the title screen until phase 2 gives them a meaning, so that nothing random happens on the map.
15. As a pad player, I want the pad cheat (× × ○ ○ × × ○ ○) to keep working, so that step 1.2's behavior survives this step.
16. As a mouse player, I want mouse mode exactly as before, so that the 26 mouse replays and my controls are untouched.
17. As a keyboard-cursor replay, I want `button_cursor` to run exactly as before, so that the 4 `keyboard_*` hashes stay unchanged.
18. As a maintainer, I want pad navigation to emit only a cursor position and a left button, so that every click goes through the existing dispatch and no game rule is copied.
19. As a maintainer, I want the focusable items computed from what the dispatch would actually let a click reach, so that a focusable item is never one the mouse could not click.
20. As a maintainer, I want an open box's buttons (planet menu, tech, pause) to be the only focusable items while it is open, so that the focus never escapes to the HUD behind a box, as the mouse dispatch already ensures.
21. As a maintainer, I want a screen-wide catcher (the score panel's modal backdrop) to hide everything under it from the focus, so that the score panel has no stray focus.
22. As a maintainer, I want world-space items (the planet menu, drawn on layer 9) focused at their screen position, so that a planet menu opened anywhere on the map works.
23. As a maintainer, I want the focus stored as an entity id, so that a focused item that dies just resets the focus, with no dangling pointer and no change to the pointer-guard count.
24. As a maintainer, I want the module to allocate nothing, so that it adds no zone use, no cap and no `free` contract.
25. As a maintainer, I want the module in the float-free core, so that the PSX port inherits it unchanged.
26. As a maintainer, I want the navigation rule (nearest item in a direction) exposed as a pure function over the current world, so that reachability can be tested without driving frames.
27. As a maintainer, I want a reachability test for the title screen, so that every difficulty is proven reachable.
28. As a maintainer, I want a reachability test for the planet menu (project / colonise panel), so that phase 2.2 can rely on it.
29. As a maintainer, I want a reachability test for the tech panel, so that phase 3.1 can rely on it.
30. As a maintainer, I want reachability tests for the score panel during play and at game end ("again?" and back), so that phases 3.1 and 3.4 can rely on them.
31. As a maintainer, I want a reachability test for the pause menu, so that phase 3.3 can rely on it.
32. As a maintainer, I want a test for a message box and for help, so that whatever they turn out to be in the mouse game is pinned and recorded before phase 2.
33. As a maintainer, I want each reachability test to also check that the focused item is the one the game hovers, so that "focus drawn as hover" is proven, not assumed.
34. As a maintainer, I want any panel that fails its test recorded in the step note, so that it is settled before phase 2 starts, as the plan says.
35. As a maintainer, I want a pad replay that chooses a difficulty with D-pad and ×, so that the code a human plays is covered end to end.
36. As a maintainer, I want that replay to contain no mouse field, stay under 180 frames and pass `tools/e2e_limits.sh`, so that it respects the plan's invariants.
37. As a maintainer, I want the replay's `.txt` to record the end state from `make state` (clock, credits, planets, colonies, slipways, techs) and what the framebuffer shows at the key frames, so that the hash is backed by a check that means something.
38. As a maintainer, I want the replay to cover Down, Up and a no-op Right before ×, so that it exercises both vertical directions and the "no item there" case.
39. As a maintainer, I want `make test` and `make e2e` no slower, so that the loop stays fast.
40. As a maintainer, I want every existing `*.sha256` unchanged, so that I know the new layer cannot reach mouse or `keyboard_*` play.
41. As a future implementer of phase 2, I want pad navigation's state and its frame output in one small module, so that free mode, network mode and tabs plug in as more modes of the same module.
42. As a future PSX porter, I want pad navigation to read only the pad mask and the world, so that the PSX controller driver only has to fill the mask.

## Implementation Decisions

### The `pad_nav` module (new, game layer)

- **Place.** A new game module, `pad_nav`, as the plan names it. It is in the engine closure, the core source list (float-free, no allocation) and the cart-only list, next to the input step that calls it. It depends on the game's ui (hover layers, ephemerals, main ui and camera), on the button module (for the modal backdrop class) and on the main economy accessor. It never depends on raylib or devtools.
- **State.** A plain struct embedded in the input step's state: the focused item as an `entity_id_t`, the previous frame's pad mask (for press edges), and the last synthetic cursor point (two ints). It holds no data pointer, so the pointer-guard count does not change. It allocates nothing: every query walks the live lists directly, with no array and no frame-zone use. It runs before `cart_update60()` resets the frame zone.
- **Interface** (names indicative; follow the `module_verb[_object]` rule and keep clear of the `data-guard` patterns, for example no `void *self` and no `*_default_table`):
  - *Init:* zero state, no focus, cursor at (0, 0).
  - *Step:* takes the state, the frame's pad mask and the played frame. It writes the played frame's `mouse_x`, `mouse_y` and `mouse_buttons`, and nothing else (the `dpad` stays zero and the pad mask stays as the input step left it).
  - *Focus:* returns the focused entity, or NULL when no panel is open or the panel has no item.
  - *Panel open:* returns 1 when a panel is open (rule below).
  - *First item:* returns the item that gets the initial focus, or NULL.
  - *Neighbor:* takes an item and one `P8_PAD_UP/DOWN/LEFT/RIGHT` bit, and returns the item a press of that direction moves to, or NULL. An `assert` with a message rejects anything but a single direction bit.
  - *Is item:* returns whether an entity is a focusable item of the current panel.
  The last four are pure reads of the world, so the reachability tests run on them without driving frames.

### Which panel is open, and which items it has

- **A panel is open** when one of these holds:
  - no game is running (the main economy is absent): that is the title screen;
  - the main ui's ephemerals list is not empty: an open box from `message_box_open_box`, so the planet menu, the tech panel or the pause menu;
  - a modal backdrop is registered: the score panel, during play or at game end.

  With no panel open, step 1.4 has no mode for the pad (free and network modes are phase 2). The synthetic cursor stays at its last point, the left button stays up, and the D-pad and × do nothing. This is what the mouse game does after a click on the title screen: the cursor stays where it was.
- **Candidates** mirror what a left press can reach in `ui_update`:
  - while the ephemerals list is not empty, the candidates are its live entries, because `ui_update` lets a left press through only to a hovered ephemeral;
  - otherwise, the candidates are the entities of the hit-test bucket whose class has a `left_press`.
- **The top layer** is the highest layer among the candidates. `ui_send_to_hovered` walks the layers from 15 down and the first `stop` wins, so a lower layer never gets a click while something above it catches it.
- **Items** are the candidates on the top layer that have a hit box. A candidate without a hit box is a screen-wide catcher (today only the modal backdrop, whose hit test is always true). When it is on top, the panel has no item. On the score panel during play the result is: panel open, zero items. × still clicks at the last point, and the backdrop closes the panel. That close is not claimed by this step.
- Items without an action stay focusable: a description row, an unaffordable tech (`on_click` NULL), "how to play?". The mouse can hover them, and their tooltip is worth reading.
- **Checked against the code at `HEAD`:**
  - title: the four difficulty buttons, layer 11. The help screen entity on layer 10 has no `left_press`, and its own ephemerals were cleared by `clear_ephemerals_after`.
  - planet menu: the box's buttons, layer 9, world space.
  - tech panel and pause menu: the box's buttons.
  - score panel during play: the backdrop on layer 14, so no item.
  - score panel at game end: "again?" and back, on layer 15, above the backdrop.

### Focus point, initial focus and movement

- **Focus point:** the centre of the item's hit box at the item's current position, floored to integer pixels. Items on layer 10 and above are hit-tested in screen space, so the point is used as is. Items below layer 10 are hit-tested in world space (`entity_geom_contains_cursor`), so the point is converted to the screen by subtracting the main camera's position. The point is not clamped to the screen. A planet menu partly off screen still gets a point the hit test accepts.
- **One-frame lag while sliding.** Pad navigation runs before the cart's update, so it reads positions from the previous frame. A box's buttons slide toward their targets for about 8 frames after opening. During that slide the point trails the item by one frame, and a × in that window can miss. Settled items never move, so there is no lag after the slide. This is accepted. The tests and the replay wait for the slide to end. The title's first row does not slide (its target is its spawn point), so the initial focus is steady from the first frame.
- **Initial focus.** When the stored focus is NULL, dead, or no longer an item of the current panel, the focus becomes the first item in candidate order: the ephemerals list order, or the hit-test bucket order. That is creation order, so "forgiving" on the title and the first row of a box.
- **Neighbor rule** for a direction:
  - a candidate item qualifies only if its focus point lies strictly beyond the current item's hit box edge in that direction, so in a single column Left and Right find nothing;
  - its score is the distance between the two focus points along the direction's axis, plus twice the gap between the two hit boxes on the perpendicular axis (0 when their projections overlap);
  - the lowest score wins, and a tie goes to the earlier item in candidate order.
  - Boxes are measured, not just centres, because rows of one column have different widths and so different centre x: comparing centres would send Left/Right across a column and let rows of different widths beat the adjacent row on Up/Down.
  - The plan rejected the cone heuristic for *planets*, where some planets are unreachable. For panels it is accepted, because a test per panel proves reachability.
- **Press edges, no auto-repeat.** A direction acts on the frame it goes down, once. When several directions go down on the same frame, only the first in the order Up, Down, Left, Right acts. The move happens first, then the point and the button are written, so × pressed on a move frame clicks the new focus.
- **× is the left button.** While a panel is open, `mouse_buttons` has `P8_MOUSE_LEFT` for every frame × is down. The press and release edges come from `mouse_button_update` as with a real mouse. With no panel open, the button is up even if × is held. So when × starts a game from the title, the next frame shows a release with nothing held, which does nothing.
- ○, □, △, L1, R1 and Start are ignored by pad navigation in this step. "○ closes the panel" is in §2, but this step's text names only the focus, the hover and ×. Its first use is phase 2.2 (planet menu) and phase 3.3 (pause). Following the guideline, a widening the step does not need is a no.
- **The cursor sprite stays drawn** at the synthetic point, as `ui_draw_overlay` does today. The plan hides it only in network mode (phase 2.2). Hiding it here would need a pad flag in the ui draw for no benefit in this step.

### Input step changes

- In pad mode, `input_step_run` calls pad navigation's step in the `TODO step 1.4` slot: after the played frame is reduced to the pad mask, and before the cheat and the dispatch install. The `TODO` comment goes.
- Mouse mode and `keyboard_cursor` mode do not run pad navigation at all. Their played frames are byte for byte as before.
- The cheat still reads the raw pad mask, so × types the cheat's O while it also clicks. The plan accepts that side effect.
- The session recorder still records the raw frame (pad mask only). Pad navigation is deterministic given the world and the pad masks, so a recording replays to the same synthetic cursor. Step 1.2's live-against-recorded test keeps holding.
- `src/cmd/game.c`, `src/cmd/replay.c` and the platform layer are not edited. The game binary still links the new module through the engine source list, so the check includes "`make raylib` builds". If raylib cannot be vendored in the session, the CI `raylib` job on the pushed commit is the check (§2 Invariants), and the step note records which.

### Decisions settled without a human (guideline rules)

Each is recorded here, as the to-prd skill asks.

- **D1 — "help" is not a separate panel.**
  - *Facts:* in the mouse game, and in the cart, "how to play?" in the pause menu is an empty function (`function()end`), and the port's row 4 handler is empty too. The only "help" entity is the title screen itself (`help.c`, the cart's help screen with "need help? slipways.net/help").
  - *DECISION:* the help test runs on the title screen entity under its help name. It asserts that "how to play?" is a reachable item of the pause menu and that clicking it opens nothing. The step note records "help: no help panel exists in the mouse game" as a finding to settle before phase 2, because phase 3.3's `pad_message_box` "opens help from the pause menu and navigates it".
  - *WHY:* the plan says these panels already exist and the tests open them directly. A panel that does not exist is a failure to record, not one to invent ("make the plan work", "don't get drawn into detail").
- **D2 — "a message box" is the year message box.**
  - *Facts:* it is a `message_box_t` ("5 years remain", as in the replay `message_box_hold`). It has no hit test and no button, and it is not modal: the HUD under it stays clickable with the mouse.
  - *DECISION:* the test opens it over a running game. It asserts that the box adds no candidate, removes none, and does not make a panel open. The step note records that §2 Modality's "close a message box with × or ○" has nothing to act on in the mouse game, to settle before phase 2.
  - *WHY:* the boxes with buttons (planet menu, tech, pause) already have their own tests. The year box is the only other thing the game calls a message box.
- **D3 — The score panel is tested in both shapes:** during play (backdrop only, zero items) and at game end ("again?" and back, side by side).
  - *WHY:* the plan's panel list says "score", and both shapes exist. The game-end one is the only panel whose items sit side by side, so it is the one that tests Left/Right.
- **D4 — The open-panel rule is three explicit checks** (no game running, open box, modal backdrop), not a layer threshold. Without it, the HUD's structure icons (layer 12) would be the in-game top layer: the focus would land on them and × would spawn a structure, which is the build tab's behavior (phase 3.2).
  - *WHY:* "keep it safe" and "question why the change is asked": map behavior is phase 2's.
- **D5 — The state dump gets no difficulty line.** The replay's claim ("choose a difficulty with D-pad and ×") is shown two ways. The framebuffer dump at the × frame shows the "reasonable" row in colour 12 and the others in 13. The end state shows a game started.
  - *WHY:* the smaller of two working options ("between two options that both work, take the smaller change").
- **D6 — No README change.** The controls section moves to the pad in step 4.2, and the pad-mode startup notice stays (H-1.3-3).

## Implementation Chunks

1. **`pad_nav` skeleton, generic focus, title screen.**
   - Build the module: state, step, panel-open rule, candidates, items, first item, neighbor rule, press edges, focus point, × as left button.
   - Plug it into the input step's slot.
   - Add the module to the engine, core and cart-only source lists, and add the new test binary to `UNIT_TESTS`.
   - Update the input step test that froze "pad mode zeroes the mouse" (see Testing Decisions).
   - Title-screen tests: initial focus, Up/Down/Left/Right, hover, × starting a game with the focused difficulty.
2. **Reachability tests for the other panels:** planet menu, tech panel, score panel during play and at game end, pause menu, the year message box, help (D1, D2, D3). Each opens its panel directly in a started game, lets it settle, then runs the reachability and hover checks. Any panel that fails is not fixed here: it is recorded for the step note.
3. **The `pad_title` replay.**
   - Write `tests/e2e/pad_title.json` (`"input": "pad"`, no mouse field).
   - Take its hash for that file only, by copying its current capture hash from `make e2e`'s output to `pad_title.json.sha256` (never `make e2e-baseline` over the directory).
   - Run `make state EVENTS=tests/e2e/pad_title.json`, read the framebuffer dump at the key frames, and write `pad_title.txt`.

## Testing Decisions

- **What a good test is here:** it drives the public interface and checks what a player sees, never internal fields:
  - pad masks in, synthetic cursor and buttons out;
  - which entity the game hovers after an update;
  - which difficulty the started game holds;
  - which items the navigation reaches.
- **New test binary `test_pad_nav`** (one test file per module, in `UNIT_TESTS`, linked like `test_input_step` against the engine closure):
  - **Title screen**, through the input step in pad mode with `cart_update60()` between frames:
    - after the slide, the focus is "forgiving" and the ui's hovered entity is that button;
    - Down, Down, Up lands on "reasonable", and each step's hovered entity is the focused one;
    - Up on "forgiving" and Down on "tough" do nothing;
    - Left and Right do nothing;
    - a direction held for 10 frames moves once;
    - × starts a game whose difficulty (`economy_score_difficulty()`) is the focused row's name;
    - × held across the click leaves nothing held afterwards;
    - with no panel open, any pad input leaves the synthetic cursor still and the button up.
  - **Reachability, one test per panel:** title, planet menu, tech panel, score panel during play, score panel at game end, pause menu, the year message box, help (D1). Each test:
    - opens the panel directly, the way the mouse game opens it, without tabs or pad navigation: `cart_start_game`, then the planet's projects panel on a planet with at least two projects, `menu_show_tech_tree_screen`, the galaxy menu's score and pause handlers, the economy's end status for game end, a new year that opens the "5 years remain" box;
    - runs `cart_update60()` enough frames for the box to settle;
    - asserts the panel-open rule's answer;
    - builds the expected item set on its own, from the panel's buttons, never from `pad_nav`;
    - walks the neighbor function over the four directions, breadth first, from the first item;
    - asserts the visited set equals the expected set;
    - asserts the item count is at least 2 wherever the panel has items, so that no test passes vacuously;
    - for each item, drives the input step with the pad presses of the path that reached it, and asserts that after the update the ui's hovered entity is that item.
  - The game-end score test also covers Left/Right between "again?" and back.
  - The planet menu test also covers the world-space point: the camera is moved off its start position before the panel opens.
- **`test_input_step` updates.** Step 1.2's `test_pad_keeps_only_the_pad_mask` froze "pad mode zeroes the mouse". It becomes: the `dpad` is zero, the pad mask is kept, and the synthetic mouse fields do not depend on the raw mouse fields (two steps fed the same pad masks with different raw mice give equal played frames). CLAUDE.md allows updating a unit test that froze the old behavior. The quiet-against-noisy headless test and the live-against-recorded test must pass unchanged.
- **E2E:** `pad_title.json`, about 30 frames:
  - f0–f9: idle while the rows slide in;
  - f10: Down ("reasonable"), f12: Down ("challenging"), f14: Right (nothing to the right), f16: Up ("reasonable"), each followed by a release frame;
  - f18: × ("reasonable", the game starts), f19: release;
  - then idle frames into the first month.

  Its `.txt` gives the frame-by-frame script, then the key-frame dump reading: at f17, "reasonable" in colour 12 and the others in 13. Then the `make state` end-state lines: clock year 3401, credits 100, planets 90, colonies 0, slipways 0, techs 0. It also says it covers no `keyboard_*` scenario on its own (`keyboard_menus` is covered by `pad_pause_restart`, step 3.3).
- **Speed:** the new tests use the same patterns as `test_input_step`: no draw, a few hundred `cart_update60()` calls in all. `make test` and `make e2e` must not get measurably slower. One more 30-frame replay is within the e2e budget.
- **Unchanged:** every existing `*.sha256` (26 mouse, 4 `keyboard_*`). `git diff --stat tests/e2e/` shows only the three new `pad_title` files.
- **Prior art:**
  - `tests/test_input_step.c`: pad frames through the input step, cart started directly;
  - `tests/test_ui.c` and `tests/test_help_screen.c`: title screen and hover;
  - `tests/test_score_panel.c` and `tests/test_modal_backdrop.c`: the score panel and backdrop;
  - `tests/test_message_box.c`: the year box;
  - `tests/e2e/keyboard_menus.txt`: an e2e note with end-state lines.

## Out of Scope

- Free mode, network mode, the planet menu opened with △, slipways from the pad (phase 2).
- ○ closing a panel, Start, L1/R1 tabs, colour-12 tabs, modality rules (phases 2.2 and 3).
- Hiding the cursor sprite, the blinking selection circle, other network visuals (phase 2.2).
- Fixing any panel that fails its reachability test, or settling D1 and D2: recorded here, settled before phase 2.
- The keymap and the backend filling the pad mask (step 1.3).
- Dropping the pad-mode startup notice (phase 2 at the latest).
- Porting mouse replays, deleting `button_cursor` (phase 4 and a later plan).
- Audio.

## Further Notes

- **The step note for 1.4** must record:
  - the result of each panel's reachability and hover test;
  - finding D1 (no help panel in the mouse game) and finding D2 (the year message box has nothing to close or focus), both to settle before phase 2;
  - any panel that failed;
  - the one-frame lag while a box slides;
  - `make raylib` (or the CI job) result;
  - that every mouse and `keyboard_*` `*.sha256` is unchanged;
  - that `pad_title` covers no `keyboard_*` scenario.
- Phase 2 inherits this module: free and network modes become further modes of `pad_nav`, active when no panel is open. They take over the "no panel open → do nothing" branch this step leaves.
- The `pad_title` hash will change when phase 2 starts the game in free mode (its in-game frames). §2 allows that refresh for `pad_*` files, done by the step that causes it.
