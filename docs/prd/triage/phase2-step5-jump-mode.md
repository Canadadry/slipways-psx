---
title: "Jump mode on the map"
description: "On the map, the pad's root mode is a free crosshair the player pans pixel by pixel, so reaching a planet costs long, overshooting D-pad holds; the root must become a cursor that jumps from object to object."
status: needs-triage
---

## Plan step (verbatim)

Plan file `docs/plan-12.md`, Phase 2 — Jump mode on the map, step 5:

> 5. **Jump mode replaces network mode and plain free mode as the map's root:** D-pad jumps with hold repeat, the hovered marker, the edge scroll and △ on the hovered object. At game start the cursor is on the starting colony, or on the target nearest the screen centre when there is none. Until steps 6 and 7 land, □ from jump mode still enters today's free mode, with its probe/fold, its □ select and its drag, so every replay keeps a path to its old end state. Network mode stops being the root but stays reachable exactly as today through that free mode's □ select (the drag needs a starting planet, which only network mode's □ sets), until step 7 removes the □ select and the drag; step 12 deletes the code. Every slipway-building replay reaches its reference end state through that interim path. The affected e2e are re-authored under the guardrails of section 2. Method: the jump cursor's state, hold-repeat timing and edge-scroll computation go in a new `src/game/pad_jump.{c,h}`, pure helpers in the style of `pad_target`, which `pad_nav.c` only calls; steps 6 to 11 follow the same pattern for each new mode where it fits, so step 12 deletes old functions without touching new code. Before any replay is re-authored, a static check in `make test` (milliseconds, modelled on `tools/float_grep.sh` with an exceptions file) allows game-stream draws (`p8_fixed_rnd*`) only from the current game-stream sites (galaxy, gen, the background tile map, the reveal roll, and `help.c`'s title seed tick named with its reason) and visual-stream draws only from the display files step 1b moved: step 2b's finding that reveals are the only game draws after the title then fails loudly instead of as an unreachable end state. Verified by unit tests in `tests/test_pad_jump.c` and `tests/test_pad_nav.c` (jump, repeat timing, edge scroll keeps the target inside the rectangle, and N frames of jump, repeat and edge scroll leave the game stream's state unchanged) and by the end-state check of step 2a.

The rules it follows (section 2, Locked decisions, **Cursor and camera**, and the △ line of **Buttons**):

> - On the map, the cursor always sits on an object: a known planet, a colony, or a structure (planet-family entities in states KNOWN or SETTLED). Unexplored planets are not drawn, so they are never targets. Wormholes and probes are not targets.
> - D-pad press: jump to the best target in that direction, searched over the whole map, not just the screen. [...] With no candidate, nothing moves, and there is no wrap-around.
> - D-pad hold: repeat the jump after 18 frames, then every 8 frames (tunable).
> - **The camera does not re-centre on every jump** (decided by the user). The cursor moves on screen. The camera scrolls only when the target leaves an inner rectangle: a 20 px margin, more at the bottom so the HUD bar never covers the target. It scrolls the minimum distance, at most 8 px per frame.
> - The hovered object gets a visible marker (reuse the pad selection blink, `ui_set_pad_selection`).
> - **△:** the planet menu of the hovered object. [...]

## Problem Statement

The measured game in section 1 of the plan spent 61% of its time in the free-camera modes: 1,066 direction presses, 35,600 px of camera travel, a third of the holds 10 frames or less, 12% overshoots. △ and □ act only on the object exactly under the centre crosshair, so every action on a planet starts with pixel aiming. Network mode, the other root the player can be in, is not spatial: Up/Down do nothing there, and Left/Right only cycle the slipways of the selected planet.

Today a game starts in plain free mode. To reach any planet the player pans the crosshair onto it. Nothing on the map answers "take me to the next thing to the right".

Step 4 built the rule that answers it (the target picker), with no caller. This step makes it the map's root: the player's cursor sits on an object, the D-pad jumps between objects, and the camera only follows when the object would leave a comfortable area of the screen. The old free mode stays one □ away, unchanged, because steps 6 and 7 have not yet moved probes, folds and slipways onto the new scheme, and every e2e replay must still reach its old end state.

## Solution

On the map tab, with no panel open, the pad is in **jump mode** by default:

- The cursor (the played mouse) sits on the **hovered object**: a known planet, a colony, a structure, a trading hub or a gate. A blinking marker (the pad selection circle network mode already draws) shows it.
- **D-pad press**: the cursor jumps to the best object in that direction, over the whole map, by the step 4 picker. Nothing in that direction: nothing moves.
- **D-pad hold**: the jump repeats 18 frames after the press, then every 8 frames.
- **Edge scroll**: when the hovered object lies outside an inner rectangle of the screen (20 px from the left, right and top edges, 31 px from the bottom so the bottom HUD row never covers it), the camera scrolls the shortest way to bring it back in, at most 8 px per frame on each axis. A jump inside the rectangle moves only the cursor.
- **△** clicks the hovered object, as a mouse press there: a known planet opens its projects menu, a settled one its panel. The menu works as today; when it closes, jump mode is back on the same object.
- **□** enters today's free mode exactly as a game starts in it today: the crosshair at the screen centre, the camera where it is, no starting planet. From there everything is as today: pan, probe, fold, □ select into network mode, the drag, ○ back to the last planet.
- **○ in network mode** (where it did nothing) leaves network mode for jump mode, hovering the selected planet.
- × and ○ in jump mode do nothing yet (steps 6 and 8 give them their jobs).

At game start (a new game or a resumed one), jump mode hovers the first colony; with no colony, the object nearest the screen centre; with no object at all (a fresh galaxy, nothing explored), nothing: the cursor rests at the screen centre, no marker and no crosshair are drawn, and the D-pad and △ do nothing until objects exist and the player comes back from free mode.

Every e2e replay that the new root breaks is re-authored to reach its reference end state, mostly by one □ tap after the title (or after a resume), under the step 2b discipline.

Before any of that, a static check in `make test` pins who may draw from each random stream: game-stream draws only from the galaxy, the reveal roll, the background tile map and the title's seed tick; visual-stream draws only from the display files step 1b moved. A future draw anywhere else then fails `make test` at once, instead of showing up as a re-authored replay that can no longer reach its end state.

For developers, jump mode's state and arithmetic (hovered object, seeding choice, hold repeat, edge-scroll shift, the inner rectangle) live in a new pure module, `pad_jump`, tested without booting a game; `pad_nav` only calls it.

## User Stories

1. As a player, I want a new game to start with the cursor on an object, not on empty space, so that my first press already acts on something.
2. As a player resuming a saved game, I want the cursor on my first colony, so that I start from my own network.
3. As a player in a fresh galaxy with nothing explored, I want the map to start quietly (no marker, no crosshair, no movement), so that nothing pretends to be selectable.
4. As a player, I want one D-pad press to take me to the next object in that direction, so that reaching a planet takes a few presses, not a pan.
5. As a player, I want the jump to search the whole map, so that an off-screen object is reachable without panning first.
6. As a player, I want a press with nothing in its direction to do nothing, so that the cursor never lands somewhere surprising.
7. As a player, I want no wrap-around, so that the map behaves like a map.
8. As a player, I want holding a direction to repeat the jump after a short pause and then steadily, so that I can sweep across a row of planets without tapping.
9. As a player, I want the first repeat to wait 18 frames, so that a normal tap never jumps twice.
10. As a player, I want a new direction pressed during a hold to jump at once and restart the repeat, so that changing my mind is immediate.
11. As a player, I want the hovered object marked with the blink I already know from network mode, so that I always see where I am.
12. As a player, I want the camera to stay still while I jump between objects on screen, so that the whole screen does not move at each press.
13. As a player, I want the camera to follow when my target would leave a comfortable area, so that the target never ends under the screen's edge or the HUD.
14. As a player, I want that scroll to be the smallest move that brings the target back in, so that the view keeps as much of what I was looking at as possible.
15. As a player, I want the scroll to glide at no more than 8 px per frame, so that I can follow where the view goes.
16. As a player, I want the bottom margin larger than the others, so that the bottom HUD row never hides my target.
17. As a player, I want the camera not to wobble with a structure's bob, so that a still cursor means a still screen.
18. As a player, I want △ on the hovered planet to open its menu, so that colonising starts from where the cursor is.
19. As a player, I want △ on a colony or structure to open its panel, so that inspection needs no aiming.
20. As a player, I want the cursor back on the same object when the menu closes, so that I continue where I was.
21. As a player, I want △ to do nothing while the target is still scrolling into view or covered by the HUD, so that △ never presses a HUD button by accident.
22. As a player, I want □ to give me today's free crosshair from jump mode, so that probes, folds and slipways still work until steps 6 and 7 move them.
23. As a player, I want that free crosshair to start where the camera already is, so that entering it does not jolt the view.
24. As a player in network mode, I want ○ to bring me back to jump mode on the same planet, so that network mode is no longer a dead end.
25. As a player, I want × and ○ in jump mode to do nothing for now, so that no stray press changes my game before their jobs arrive.
26. As a player, I want a game I restart from pause to start in jump mode again, whatever mode I left the old game in, so that every game starts the same way.
27. As a player, I want a selection that dies in network mode to drop me into jump mode, so that I am never left with nothing selected and no root.
28. As a player, I want placing a structure from jump mode to work as today with the centre crosshair, so that the build tab does not change in this step.
29. As a player, I want the tab ring, pause and every panel to work as today from jump mode, so that nothing else moves in this step.
30. As a replay author, I want every old end state reachable with one □ tap after the title, so that re-authoring is mostly an insertion, not a rewrite.
31. As a replay author, I want the jump, the nearest-centre pick and the edge scroll to depend only on world positions and creation order, never on frame timing or bob phase, so that the step 2b discipline still holds.
32. As a replay author, I want □ from jump mode to leave the camera untouched, so that no camera snap (step 2b rule 8) is introduced by the new root.
33. As a replay author, I want × and ○ to be inert in jump mode, so that typing the cheat there has no side effect.
34. As the agent of step 6, I want the hovered object and a candidate set builder already in place, so that slipway building only adds the held button and the start exclusion.
35. As the agent of step 7, I want a "nearest the centre" query already in place and tested, so that aim mode's return to jump mode is wiring.
36. As the agent of step 14, I want jump mode readable from outside pad navigation (is it on, what is hovered), so that the metrics tool can attribute time and dead presses to it.
37. As a developer, I want the repeat delay, repeat period, margins and scroll speed as named constants, so that tuning after the human's game is a one-line change.
38. As a developer, I want the candidate set built without zone allocation, so that no frame-zone peak or allocation guard moves.
39. As a developer, I want slipgates and infraspace gates to be targets like other structures, so that every object × will act on in step 6 is reachable.
40. As a reviewer, I want every re-authored replay to pass `make end-state` against the reference commit and keep every situation of its old `.txt`, so that the port's coverage of the game is unchanged.
41. As a reviewer, I want the pinned values of every test on step 2a's inventory re-derived with their intent kept, so that no assertion is weakened by the re-authoring.
42. As a reviewer, I want `make coverage-compare` to show no baseline line newly unhit, so that a re-authored replay still exercises what it did.
43. As a reviewer, I want the re-authoring cost recorded, so that the step 5b trigger can be judged.
44. As a developer, I want `make test` and `make e2e` no slower, so that the fast loop stays fast.
45. As a replay author, I want `make test` to fail when any code outside the known game-stream sites draws from the game stream, so that a broken end state never has a hidden random cause.
46. As a replay author, I want visual-stream draws restricted to the display files, so that a display effect cannot move back onto, or be confused with, the game stream unnoticed.
47. As a reviewer, I want each allowed site listed with its reason and its call count, so that a new draw in an allowed file is caught too.
48. As a replay author, I want a unit test showing that jumps, hold repeats and edge scrolls leave the game stream's state unchanged, so that jump-mode timing can never change a revealed planet.
49. As the agent of steps 6 to 12, I want jump mode's logic in its own pure module that `pad_nav` only calls, so that step 12 deletes old `pad_nav` functions without touching new code.
50. As a developer, I want the repeat timing, the scroll shift and the seeding choice testable without a cart, so that their tests take milliseconds.

## Implementation Decisions

### The random-stream guard (first, before any mapping change)

- **What it checks.** A new script in `tools/`, modelled on `tools/float_grep.sh` (a text scan with a committed list, exit 1 on a finding, exit 2 on an inconsistent or empty list so a measure over nothing never passes), run by a new `make` target that `make test` runs beside `float-grep` and the other guards, and listed in the `Makefile` legend. It scans every `.c` and `.h` file of `src/` for calls of two families: the game stream (`p8_fixed_rnd`, `p8_fixed_rnd_default`, `p8_fixed_rnd_state`, and `p8_fixed_rnd_from_state` outside the visual-stream module) and the visual stream (`visual_random_draw`, `visual_random_draw_one`).
- **The committed list** (one file in `tools/`, one line per allowed file: `<path> <family> <count> | <reason>`). Counts are strict equalities, as in `tools/alloc_guard_exceptions.txt`, so a new draw inside an allowed file fails too. The list at this step, read from the tree:
  - game stream: `src/game/galaxy.c` (the galaxy roll), `src/game/gen.c` (the random walk behind the reveal roll and the type picker), `src/game/background.c` (the tile map, kept on the game stream by step 1b, and the seed it records), `src/game/help.c` (the title's seed tick: one game draw per drawn title frame, so a new game's galaxy depends on when the title's X lands), `src/game/save_state.c` (reads the state for the save, draws nothing);
  - visual stream: `src/game/particle.c`, `src/game/probe.c` (the emitter), `src/game/planet_family.c` (the burst particles), `src/game/planet_display.c` (the vanishing flicker), `src/game/slipway_display.c` (the transit dots);
  - definitions: `src/p8/fixed.c` and `src/p8/fixed.h` (the generator), `src/game/visual_random.c` and `src/game/visual_random.h` (the visual stream on its own state).
  The implementer counts the calls on the tree the chunk lands on; a count that differs from this reading is recorded in the step note, never silently adjusted.
- **What it buys.** Step 2b proved that after the title the only game draws are reveals. The guard makes that a `make test` failure instead of an unreachable end state found during re-authoring. A comment is not a call: the scan ignores `//` and `/* */` comment text (the float scan's own rule, reused if it has one; otherwise a line whose only match sits after `//` or inside a comment block is not counted).
- **Self-test.** A small test in the same style as the float guard's own (`tests/` prior art for `tools/*.sh` checks: `test_frames_check`, `test_constants_check`) runs the script on a fixture tree: clean passes, an extra call in an allowed file fails, a call in an unlisted file fails, a listed file with no call fails, an empty list fails. It must take milliseconds.
- **Speed.** `make test` gets one more grep over `src/`: milliseconds. Compared before and after.

### Modules

- **New `pad_jump` module** (`src/game/`, pure helpers in the style of `pad_target`: no registry, no camera, no ui, no allocation, no file-scope state). `pad_nav` owns one `pad_jump_t` and only calls these:
  - `pad_jump_t`: the jump cursor's state. The hovered object's `entity_id_t`, the `entity_id_t` of the game it was seeded in, a seeded flag, and the repeat state (the repeating direction and its held-frame count). Plain values and ids only, so `tools/pointer_guard_exceptions.txt` does not change.
  - `void pad_jump_init(pad_jump_t *jump)`: nothing hovered, unseeded, no repeat.
  - `int pad_jump_repeat_step(pad_jump_t *jump, int pad_buttons, int pressed)`: the direction to jump this frame, or 0. A direction pressed this frame (priority Up, Down, Left, Right) returns at once and restarts the count at 0; while that direction stays held the count goes up by one per frame, and it returns again when the count reaches `PAD_JUMP_REPEAT_DELAY` (18) and every `PAD_JUMP_REPEAT_PERIOD` (8) frames after (press at f, repeats at f+18, f+26, f+34 …); released, the repeat clears. `void pad_jump_repeat_clear(pad_jump_t *jump)` for frames jump mode does not run.
  - `void pad_jump_world_point(const entity_t *e, int *x, int *y)`: the rest point in world pixels, `floor(map_pos × GALAXY_MAP_SCALE)` per axis. It reads `map_pos`, never `pos`, so a bobbing structure's point does not move with the frame. Asserts a map position.
  - `entity_t *pad_jump_nearest(int x, int y, const entity_list_t *candidates)`: the candidate whose world point is nearest the world pixel `(x, y)`, squared Euclidean distance in integer pixels (the galaxy spans about 400 px, so the square fits an `int`); at equal distance the earlier in the list; NULL for a NULL or empty list. Integer pixels, not `p8_fixed_t` map units: squaring sub-pixel map distances in 16.16 loses the low bits and turns near-ties into ties.
  - `entity_t *pad_jump_seed(const entity_list_t *candidates, int centre_x, int centre_y)`: the first colony in list order (a candidate of class `planet_class()` in state SETTLED; structures, hubs and gates are not colonies), else `pad_jump_nearest` from the centre's world pixel, else NULL.
  - `int pad_jump_candidates(const entity_list_t *nodes, entity_t **storage, int capacity, entity_list_t *out)`: fills `out` over the caller's `storage` with the entries of `nodes` that pass `pad_target_is_candidate`, in order; returns the count; asserts, with a message, that `capacity` is enough. The caller passes the `TAG_NODE` bucket and a local array: no zone allocation, so no `make state` zone peak moves.
  - `int pad_jump_inside(int x, int y)`: 1 when the screen point lies in the inner rectangle `x ∈ [20, 139]`, `y ∈ [20, 88]` (inclusive; 88 = 119 − 31).
  - `void pad_jump_scroll(int x, int y, int *dx, int *dy)`: the camera shift for a target at screen point `(x, y)`: on each axis how far it lies outside the inner rectangle (0 inside), clamped to ±`PAD_JUMP_SCROLL_SPEED` (8).
  - Constants in the header: `PAD_JUMP_REPEAT_DELAY` 18, `PAD_JUMP_REPEAT_PERIOD` 8, `PAD_JUMP_SCROLL_MARGIN` 20, `PAD_JUMP_SCROLL_MARGIN_BOTTOM` 31 (20 plus the 11 px of the bottom HUD row, which starts at `P8_SCREEN_HEIGHT − 11`), `PAD_JUMP_SCROLL_SPEED` 8.
  - Step 4's `pad_target` module is not changed: the picker stays one function, and these helpers sit beside it.
- **`pad_nav` gains jump mode**, as wiring only (reading the registry, the camera and the ui, and calling `pad_jump` and `pad_target`). Its public additions:
  - `PAD_NAV_MAP_JUMP` in `pad_nav_map_mode_t`. It is the mode `pad_nav_init` and every map-mode reset set (it replaces `PAD_NAV_MAP_FREE` there).
  - `int pad_nav_jump_mode(const pad_nav_t *nav)`: a game is running, the tab is map, no panel is open and the map mode is jump. `pad_nav_free_mode` keeps its meaning (map mode free), so the crosshair is drawn only in free mode.
  - `entity_t *pad_nav_hovered(const pad_nav_t *nav)`: the hovered object while the map mode is jump, panel open or not; NULL in any other mode, when nothing is hovered, or once the hovered object is no longer a candidate.
  - One new field, the `pad_jump_t`.
- **Removed nothing** in this step. Network mode, the drag, `return_to_last_planet`, the tab ring and the free-mode code all stay (steps 7 and 12 delete them).
- `is_clickable_planet` (pad_nav's private predicate) is left as it is: its callers are the old free and network paths, which must not change. Jump mode uses `pad_target_is_candidate`.

### The candidate set

- The live candidate set is the `TAG_NODE` bucket filtered by `pad_target_is_candidate`, in bucket order (creation order), built by `pad_jump_candidates`. Not `TAG_PLANET`: slipgates and infraspace gates carry `TAG_SLIPGATE | TAG_NODE` and no `TAG_PLANET` (their classes do carry `LINEAGE_PLANET`, so the predicate accepts them), and a planet-tag set would leave them unreachable. This deviates from the step 4 PRD's "planet-tag entities"; the picker takes any list, so nothing in step 4 changes.
- The local array's cap sits above the `TAG_NODE` reservation (110 in `cart.c`).
- It is built only on frames that need it: a jump, a seed, a reseed. Not every frame.

### Hovered object and seeding

- **Game start.** pad_nav records the id of the main economy entity (`main_economy()`) when it seeds. A frame where a game is running and that id no longer resolves to the current main economy is a new game: the map mode is reset (jump, nothing hovered, unseeded), whatever mode the old game was in. The title and a new game rebuild the id table above every old generation, so an old id resolves to NULL; this needs no title frame in between (the unit tests start games with `cart_start_game` directly). The tab is not reset by this (today's rule: the tab goes back to map while no game runs).
- **Seed** (jump mode, nothing hovered, game not yet seeded): the first colony in candidate order (a candidate of class `planet_class()` in state SETTLED; structures, hubs and gates are not colonies); with none, `pad_jump_nearest` from the world point under the screen centre (camera position before shake, floored, plus 80, 60); with no candidate at all, nothing, and the game stays unseeded so the seed rule runs again on the next jump-mode frame.
- **Reseed** (jump mode, game seeded, hovered object no longer a candidate: dead, vanishing, gone): `pad_jump_nearest` from the screen centre's world point; nothing if no candidate.
- Entering jump mode from network mode (○) hovers the network selection, which is settled or known, so a candidate.

### Jump mode, frame by frame

A new branch of `map_step`, after placement, panel, free mode and network mode, runs when the map mode is jump. In order:

1. `reset_free_mode` (free-mode pan counters and × engagement cleared), focus cleared.
2. A new game resets the `pad_jump_t` (`pad_jump_init`); then seed (`pad_jump_seed`) or reseed (`pad_jump_nearest`) as above.
3. **□ pressed**: the map mode becomes free, with no starting planet, no selection and no last planet; the cursor goes to the screen centre; the camera does not move; the repeat state is cleared (`pad_jump_repeat_clear`, also on every frame jump mode does not run). Nothing else runs this frame. This is byte for byte the state a game starts in today, so an old replay with one □ inserted after its title continues as before.
4. **D-pad**: `pad_jump_repeat_step` gives the direction to jump this frame (a press, or a repeat at +18, +26, +34 … while held). For a direction, the hovered object becomes `pad_target_pick(hovered, direction, candidates)` if it is not NULL. No jump runs on a click's press or release frame (the cursor must stay on the clicked object, as network mode's comment says); the count keeps running through those frames. With nothing hovered, the D-pad does nothing.
5. **△ pressed**, not on a release frame, nothing pending: if the hovered object's screen point lies inside the inner rectangle (`pad_jump_inside`) and no higher-layer HUD item is under it (the existing `pressed_before` test), the pad clicks it (`click`: press this frame, release next). The map mode stays jump; there is no network selection. Otherwise △ does nothing.
6. **Edge scroll** (skipped on a click's press and release frames, and with nothing hovered): the hovered object's rest point (`pad_jump_world_point`) minus the camera position before shake, floored, gives its screen point; `pad_jump_scroll` turns it into the shift (how far outside the inner rectangle, clamped to ±8 per axis), and `camera_pan` applies it. Using the rest point and the camera before shake, the scroll never follows the bob or a shake, so the camera path depends only on the sequence of hovered objects (step 2b rule 8). The camera bound always lets the target reach the rectangle: the bound keeps the screen centre over the planet area, every object lies in it, and the centre is inside the rectangle.
7. **Cursor**: the played mouse goes to the hovered object's screen point by `pos` (the existing `screen_point`, bob and shake included), because the hit test reads `pos`. With nothing hovered: the screen centre.
8. **Marker**: the hovered object is passed to `show_selection` with no slipway, except on a click's press and release frames and while a panel is open (as network mode does), so `ui_set_pad_selection` draws the blinking circle.

× and ○ do nothing in jump mode. `tab_cursor` on the map tab puts the cursor on the hovered object in jump mode (screen centre when nothing is hovered), as it does on the network selection.

### Changes to the old modes

- **Network mode, ○** (no box open, so `circle_returns`): the map mode becomes jump, hovering the selected planet; the selection and selected slipway are cleared. This is the only change to network mode. Every other press there behaves as today.
- **Free mode** is unchanged, including ○ in plain free mode (network on the last planet, or nothing at game start), its □ select into network mode, △ into network mode, the drag and the probe/fold. Its only entry from the root is □ in jump mode. So in this interim the way back from free mode to jump mode goes through network mode (□ select or ○ to the last planet, then ○). With no colony and no last planet, the player stays in free mode until a colony exists; step 7 gives free mode its own way back.
- **A dead network selection** (`drop_stale_map_mode`) resets to jump mode, which reseeds from the screen centre (it was plain free mode).
- **Placement** begun from jump mode switches the map mode to plain free mode without moving the camera, as from free mode today; from network mode it still calls `enter_free_mode` on the selection. After the structure is dropped or cancelled the player is in free mode, as today.
- **Pause, tabs, panels**: unchanged. While a panel is open the panel step runs, as for every map mode.

### Re-authoring

- Every replay whose `.json.sha256` changes is either re-baselined (inputs unchanged, `make end-state` passes, every situation of its `.txt` still produced, `.txt` readings rewritten from the new frames) or re-authored (inputs changed under the same three checks). Expected: every replay that reaches the map, since jump mode at a fresh game draws no crosshair where free mode did; probably all but `pad_title`, and `pad_title` too if its last frames show the map.
- **The default route**: one □ tap on the first map frame after the title's X (after a resume's X in a resumed game, after every new game in replays that restart). Step 2b rule 1 allows it: the title's X stays on its reference frame and the extra frames come after it. In a fresh game there is no candidate, so jump mode moves no camera and □ leaves it untouched; the rest of the replay runs as before, shifted by the inserted frames.
- **Resumed games** (`pad_resume_at_start`, `pad_restart_resume`, the `initial_save` replays): jump mode seeds on a colony or the nearest object and the edge scroll may move the camera before the □. Then, per step 2b rule 8, the replay re-pans to the reference camera before its next point action (probe, fold, △ on the centre, placement, drag drop).
- **○ in network mode**, a dead press before, now leaves network mode. Replays that press it there (including inside a typed cheat) are re-routed: the cheat moves to a place where × and ○ do nothing (jump mode is one), keeping its activation count; any other such ○ is dropped or moved, and the step note lists it.
- **Shared prefixes first**, each prefix re-authored once and reused verbatim by the replays built on it: `pad_probe` (the early check named in section 2) → `pad_colonise` → `pad_slipway`, and `pad_probe` → `pad_restart_resume`; `pad_title`'s f0–12 → `pad_bankrupt`, `pad_build`; `pad_end_turn` → `pad_game_end`; `pad_research` → `pad_resume_at_start` (its `initial_save` re-cut from `pad_research`'s new last record if that record changes). Then the rest: `pad_empty_planet`, `pad_message_box`, `pad_pause_restart` (its 48-frame prefix version too), `pad_planet_projects`, `pad_score`, `pad_tech_costs`.
- **Frame limits**: a replay must stay within 180 frames, or 600 if listed in `tools/e2e_long_exceptions.txt`. `pad_bankrupt` is at 180/180: if it needs the □ (it reaches the map through the build tab, whose placement already switches to free mode, so it may not), the frames come from compressing its idle frames, never from the limit. The frame counts in `tools/e2e_long_exceptions.txt` reasons are updated where they change.
- **Discipline**: the 8 rules of step 2b apply to every re-authored replay. A replay that cannot reach its reference end state under them falls under section 2's relaxed-comparison fallback; the first such replay adds the exceptions file to `tools/` and its support to `tools/end_state.sh`, as step 2b's note says. Expected: none.
- **Pinned values** of step 2a's inventory re-derived in the same commit as the replay they read: `tests/test_live_slots.c` and its 4 fixtures (re-cut from the new replays), `tests/test_state.c` (`pad_tech_costs` camera and lines, the `pad_research` cut, `pad_build` plus 60 idle frames), `tests/test_rng_sync.c` windows on `pad_colonise` (then `make rng-sync-probe` re-run), golden frame hashes, the `.end-state` references (unchanged: they are the oracle, never re-baselined), the `.txt` files.
- **Coverage**: `make coverage-compare` after each re-authoring commit: no baseline line newly unhit. Lines this step replaces (the map-mode reset that set free mode, `pad_nav_init`'s) are reported as such.
- **`worst-case/` games**: knowingly stale from this step (section 2). `make end-state` is judged on the 17 e2e replays only. `make full-game`, `make save-worst-case` and `make save-fuzz` are run once: an assert or crash is a bug to fix in this step; any other change is ignored and noted.
- **Cost record**: the step note gives the number of replays broken, re-baselined and re-authored, and the agent time spent re-authoring. If it exceeds 170 minutes (twice step 1b's 85), the step note says the step 5b trigger fired and step 5b, as section 2 words it, is added to section 3 before step 6.

## Implementation Chunks

Each chunk ends on a commit where `make test-all` passes. A chunk that changes what a replay sees includes, in the same commit, the re-authoring of every replay it breaks.

1. **The random-stream guard**: the script, its committed list, its `make` target run by `make test`, the `Makefile` legend line, and its self-test. No source changes, every e2e unchanged. It lands before any replay is re-authored, as the plan requires.
2. **The `pad_jump` module and `tests/test_pad_jump.c`**: every helper and constant listed above, no caller yet. Every e2e unchanged.
3. **The jump root, with every replay re-authored.** In `pad_nav`: `PAD_NAV_MAP_JUMP` as the default and reset mode; the game-start detection and seeding; the reseed; the candidate set; the cursor on the hovered object; the marker; the edge scroll; □ into plain free mode; a dead network selection into jump mode; placement from jump mode; `pad_nav_jump_mode` and `pad_nav_hovered`. Unit tests for these in `tests/test_pad_nav.c`, and the existing tests that assumed free mode at game start updated to tap □ first (a helper, so each keeps its intent). Then the re-authoring, prefixes first, with `pad_probe` first: if `pad_probe` cannot reach its reference end state, stop and apply section 2's fallback before the others. All pinned values, `.txt` files and `.sha256` files in the same commit. This is the big chunk: the mapping and its replays must land together (section 2), so the commit is the only green state of this work.
4. **D-pad jumps and hold repeat** wired in jump mode, with their `tests/test_pad_nav.c` cases and the game-stream test (N frames of jump, repeat and edge scroll leave the game stream's state unchanged). After chunk 3 no replay presses a direction in jump mode (they press □ first, and resumed games re-pan in free mode), so every e2e should stay unchanged; if one changes, it is re-authored in this commit.
5. **△ on the hovered object**, with its unit tests. Same expectation: no replay presses △ in jump mode yet.
6. **○ in network mode back to jump mode**, with its unit test, and the re-authoring of every replay that pressed ○ in network mode (cheat routes included), with their pinned values, in the same commit.

Chunks 4–6 are kept apart from chunk 3 so that chunk 3's re-authoring targets the smallest mapping change. If one of them breaks replays chunk 3 re-authored, those replays are re-authored again in that chunk's commit, and the step note counts the time.

## Testing Decisions

- **A good test** asserts on public behaviour only. For `pad_jump`: return values and the hovered id read through `pad_jump_t`'s public fields, on hand-built inputs. For `pad_nav`: drive the pad through `run_frame` / `tap` / `hold` and assert on `pad_nav_jump_mode`, `pad_nav_hovered`, `pad_nav_free_mode`, `pad_nav_selection`, the played frame's mouse position and buttons, the camera position, `ui_pad_selection`, world counts, and the game stream's state (`p8_fixed_rnd_state`). Never on `pad_nav_t` internals.
- **`tests/test_pad_jump.c`** (new, one file per module; prior art `tests/test_pad_target.c`: bare entity headers on the stack with a map position, an `entity_list_t` over the test's own storage, no cart). Milliseconds. Cases:
  1. Repeat timing: a press returns its direction; held, it returns again at +18, +26, +34 and on no other frame up to +60; a release clears it; a new press restarts at 0; a second direction pressed during a hold returns at once and takes over; two directions pressed together give the priority one; `pad_jump_repeat_clear` stops it.
  2. Scroll shift: inside the rectangle 0, 0; on each edge exactly 0 (inclusive bounds); 5 px outside, 5; 30 px outside, clamped to 8; the bottom edge at y 88, not 99; both axes at once.
  3. `pad_jump_inside` on the four edges and just outside them.
  4. `pad_jump_world_point` reads `map_pos` × 128 floored and ignores a disagreeing `pos`.
  5. `pad_jump_nearest`: the nearest of several; the earlier of two at equal distance in both list orders; NULL for NULL and empty lists; a point far outside the screen.
  6. `pad_jump_seed`: a colony wins over a nearer known planet; the first colony in list order wins over a later one; a settled structure is not a colony; with no colony the nearest; NULL with no candidate.
  7. `pad_jump_candidates`: keeps KNOWN and SETTLED planet-family entries in order, drops the others, uses the caller's storage.
- **`tests/test_pad_nav.c`**, new cases (prior art: `test_a_new_game_starts_in_free_mode`, the free-mode pan tests with `hold_and_measure`, `known_planet_closest_to_centre`, `settle_planet`, `start_title_with_a_save`, `test_select_changes_nothing_yet` for a with/without comparison):
  1. A new game starts in jump mode with nothing hovered in a fresh galaxy: cursor at the centre, no crosshair, no marker, the camera still over 30 idle frames, the D-pad and △ do nothing.
  2. Seeding wiring: with a colony and a nearer known planet, the colony is hovered; once objects appear (made known), the next jump-mode frame seeds.
  3. A restart (a new game started after the old game was left in free mode and in network mode) is back in jump mode.
  4. Jump: from a hovered object, each direction lands on what `pad_target_pick` gives over the live candidate set the test builds from `TAG_NODE` and `pad_target_is_candidate`; a direction with an empty cone changes nothing (hovered, cursor, camera).
  5. Repeat wiring: holding Right across a row of known planets changes the hovered object on the press frame and at +18 and +26, and on no other frame.
  6. Edge scroll keeps the target inside the rectangle: after a jump to an object outside it, each frame's camera move is at most 8 px per axis; it stops with the object's rest point inside the rectangle, on the edge it came from (minimal distance); a jump to an object inside moves no camera.
  7. The scroll does not follow the bob: with a bobbing settled object hovered (a structure, or if building one costs too much, a settled planet whose `pos` the test offsets from its rest point), 60 idle frames move no camera.
  8. The game stream is untouched: after the title, N frames (at least 120) mixing jump presses, a long hold with repeats and edge scrolls across the map leave `p8_fixed_rnd_state()` equal to its value before them, with no fold or probe in the sequence.
  9. The cursor is on the hovered object's `pos` screen point (the existing `assert_cursor_on`).
  10. △ on a hovered known planet opens its menu; ○ closes it and jump mode is back on the same planet; △ on a hovered colony opens its panel on the release frame; no jump happens on that release frame even with a direction pressed.
  11. △ on a hovered object still outside the rectangle (mid-scroll) presses nothing.
  12. □ from jump mode: plain free mode, camera unchanged, cursor at the centre, crosshair shown, no starting planet; the next frames pan as free mode does.
  13. ○ in network mode returns to jump mode hovering that planet.
  14. × and ○ in jump mode change nothing (hovered, cursor, camera, world counts, played buttons 0).
  15. A dead network selection falls back to jump mode (replaces `test_a_dead_selection_falls_back_to_free_mode`).
- **Updated tests**: `test_a_new_game_starts_in_free_mode` becomes case 1. Every test that started a game and assumed free mode gets a □ tap through one helper; its assertions stay as they are. `test_select_changes_nothing_yet` compares runs with and without the Select bit, so it should pass as written; if its sequence relied on free mode, it gets the same helper and keeps its intent.
- **The random-stream guard's self-test**, as described in its section.
- **Not tested**: the misuse asserts (the codebase's tests do not exercise asserts).
- **Guard rails checked before each commit**: `make test-all` green; `make end-state` on the 17 replays green; `make coverage-compare` with no newly unhit baseline line; `make test` and `make e2e` wall times compared before and after the step. `tests/test_pad_jump.c` stays in milliseconds; the new `pad_nav` cases keep their frame counts small.

## Out of Scope

- × on a colony starting a slipway, D-pad jumps while the slipway is held, the slipway start excluded from candidates (step 6), and the deliberate cheat route search (step 6), apart from moving any cheat a replay typed with ○ in network mode.
- Aim mode on □ without the □ select and drag, and □ or ○ from free mode back to jump mode (step 7).
- × on a known planet, the full ○ back order and the root jump to the last object × acted on (step 8).
- Select, L1 build bar, R1 panels, removal of the tab ring and network mode (steps 9–12).
- Re-authoring the `worst-case/` games (step 13).
- △ again closing the planet menu: section 2 states it, but no step's text claims it; this step does not add it (see Further Notes).
- Any change to the cart's rules, HUD, panels or menus; audio; PSX; mouse input.
- Tuning the constants (section 5 keeps the first guesses for the whole plan).

## Facts from steps 3 and 4

Steps 3 and 4 are committed; this PRD builds on what they landed, read from their commits and step notes. None of the decisions above rests on an assumption about them.

- **Step 3** (`c789432`, step note in the plan): `P8_PAD_SELECT` is bit 2048; no other bit moved. Nothing reads it yet, and `test_select_changes_nothing_yet` in `tests/test_pad_nav.c` shows it inert. Every e2e and `.end-state` reference stayed unchanged, so the references this step's replays must reach are still those of `8defcba`. Jump mode reads no Select bit.
- **Step 4** (`cb5e377`, step note in the plan): `src/game/pad_target.{c,h}` exposes `pad_target_is_candidate(e)` (live, planet family, KNOWN or SETTLED) and `pad_target_pick(origin, direction, candidates)` with `PAD_TARGET_PERPENDICULAR_WEIGHT` 2. The pick reads only `map_pos`, does not filter the list, never returns `origin`, returns NULL for an empty cone, a NULL or an empty list, and breaks equal scores by smaller |perpendicular| then list order. It asserts a non-NULL origin with a map position, a single D-pad direction, and a map position on every candidate: jump mode calls it only with something hovered and with the single bit from `pad_jump_repeat_step`. It has no caller; e2e and end-state unchanged; `pad_nav` untouched, so `is_clickable_planet` is as before.
- **Read from the tree for this PRD**: slipgate and infraspace gate classes carry `LINEAGE_PLANET` (so `pad_target_is_candidate` accepts them) but not `TAG_PLANET`, hence the `TAG_NODE` candidate set; `TAG_NODE` is reserved at 110 in `cart.c`.

## Further Notes

- **Why □ does not centre the camera**: `enter_free_mode` from network mode centres on the selection with `centre_camera_on`, which reads `pos` (bob included): exactly the snap step 2b rule 8 warns about. From jump mode there is nothing to centre on that the player chose for free mode, and an unmoved camera makes the inserted □ invisible to the rest of an old replay.
- **Why the scroll reads `map_pos`**: the step 2b evidence (`pad_research`, idle frames at f244, s2 placed 2 px off, 2 slipways lost) came from a camera that depended on bob phase. A rest-point scroll is camera-path deterministic: the same hovered objects in the same order give the same camera, whatever the timing.
- **The cheat in jump mode**: × and ○ are inert there in this step, so typing × × ○ ○ × × ○ ○ in jump mode changes neither nav, cursor, camera nor game. That is a side-effect-free route for this step's replays. Step 6 makes × act on colonies, so step 6 must re-check it (section 2).
- **△ toggle**: section 2's "△ again, or ○, closes it" is not in any step's text. ○ already closes the menu (`circle_closes_the_box`). △ inside a menu stays a dead press after this step. Flagged for the plan's owner; the `guideline` skill can decide which step takes it.
- **Interim dead ends, by design**: with no colony and no last planet, free mode has no way back to jump mode until step 7. Recorded so step 14's dead-press report does not count it as a regression of this step.
- **Step 7's return** ("□ or ○ returns to jump mode on the target nearest the centre") can call `pad_jump_nearest` with the centre's world point, as this step's reseed does.
- **For step 14**: `pad_nav_jump_mode` and `pad_nav_hovered` give the mode split and the jump-mode cursor; a D-pad press whose `pad_target_pick` returns NULL is the "empty cone" dead press of the first group.
