---
title: "Slipways from free mode, and network travel: □, ×, ○ and the D-pad on the pad player's own network"
description: "A pad player can colonise a planet and stand on it in network mode, but cannot build a slipway, move along the network or step back out into free mode."
status: needs-triage
---

## Hypotheses on step 2.2

Step 2.2 (planet menu and network mode, `docs/prd/triage/phase2-step2-network-mode.md`) is being implemented while this PRD is written. This PRD reads only committed code. At `HEAD`, `pad_nav` still has only step 2.1's derived free mode: it has no stored map mode, no selection, no △ and no ○ close. Every assumption about 2.2's outcome is listed here.

**Rule for the implementer:** if step 2.2 landed with a different name or spelling, use what landed. Only a hypothesis whose *behavior* turns out false needs a line in this step's note. When that happens, its fallback applies.

- **H-2.2-1 Stored map mode and selection.** `pad_nav`'s state holds a map mode (free or network) and a selection (`entity_id_t`, the selected planet), plus plain ints. It holds no pointer and allocates nothing. The precedence is: panel open → panel navigation; else game running → the stored map mode; else nothing. The state resets to free mode with no selection in three cases: no game is running, the running economy is not the one the state recorded, or the selected entity is gone or dead (2.2's N1/N10). `pad_nav_free_mode()` answers "game running, no panel, mode free", and a read-only `pad_nav_selection` returns the selected planet, or NULL. *Fallback:* if 2.2 derived network mode instead of storing it, this step stores it, adding the fields described here, in chunk 1.
- **H-2.2-2 Planet under a screen point.** `pad_nav` has a private helper, generalised from 2.1's `cross_reaches_background`, that finds the planet-kind entity whose own `hit_test` contains a screen point. It builds the `mouse_pos_t` the way the ui does (screen, plus world = screen + camera position). △ uses it to find its target in free mode. *Fallback:* if 2.2 inlined it, this step extracts it in chunk 1 and moves 2.2's call onto it, with no behavior change.
- **H-2.2-3 △ as a synthetic click with a pending release.** △ emits the left button down on its press frame and up on the next frame, whatever △ does meanwhile, through a "pending release" int in `pad_nav`'s state. △ counts only on a known or settled target, and it lands network mode on the target on the press frame (2.2's N2/N3). This step's × gesture (decision S5) reuses that two-frame shape. Its release point is a stored point, not the centre. *Fallback:* if 2.2 built the click differently, the gesture gets its own phase field. The two must never be pending on the same frame: × is ignored while △'s release is pending, and △ is ignored while the gesture is pending.
- **H-2.2-4 ○ closes boxes.** On a ○ press edge, while a game runs, the ui holds a box and pause is not up, `pad_nav` empties the ephemerals with `ui_clear_ephemerals` and leaves the map mode as it is (2.2's N4). This step's ○ (return to the starting planet) applies only in free mode with no panel open, so the two never compete. *Fallback:* none needed.
- **H-2.2-5 Network mode's buttons in 2.2.** In network mode, the synthetic cursor is the selected planet's screen point: its position minus the camera position, floored. × leaves the left button up. ○ does nothing with no box open. □, L1, R1, Start and the four directions leave the played frame and the camera unchanged (2.2's N5/N6). This step replaces "directions and □ do nothing" and keeps the rest. *Fallback:* none needed.
- **H-2.2-6 Network visuals.** The ui holds a pad selection (`entity_id_t`, none from `ui_new`) and a blink frame count. Both are written every pad-mode frame through one setter (indicative `ui_set_pad_selection`): the selected planet in network mode with no panel open, none otherwise. `ui_draw_overlay` draws no cursor sprite while the selection is set. The planet draw circles the selected planet (radius 9, colour 7). The circle is visible for frames 0–15 of each 32-frame period, counted from the frame the selection was last shown (2.2's N7/N8). This step adds a selected slipway to the same fact (decision S9). *Fallback:* if 2.2 kept the visual fact elsewhere, the selected slipway goes next to it, under the same "pad mode only, none otherwise" rule.
- **H-2.2-7 △ leaves the camera alone.** A planet selected by △ in free mode is under the centre cursor (within its ±7 px hit box), but not necessarily centred, and 2.2 never moves the camera (its N9). *Fallback:* none needed. This step centres the camera only on its own selection changes (decision S3).
- **H-2.2-8 `pad_colonise` content.** `tests/e2e/pad_colonise.json` copies `pad_probe`'s frames up to the × that starts the game, so its galaxy is `pad_probe`'s. It colonises one planet through △ and the menu, then idles in network mode. It may press △ again and ○ to close the inspection panel. It never presses □. In network mode, outside the menu, it presses no direction and no ×, and its colony has no slipway. *Consequence:* **this step expects no earlier `pad_*` replay to change** (decision S14). *Fallback:* if `pad_colonise` presses □, presses a direction or ○ in network mode, or reaches free mode with a starting planet, its hash may move. The implementation then refreshes that one file under §2's rule, reruns `make state` on it, and the step note says why.
- **H-2.2-9 Frozen 2.2 tests.** `test_pad_nav` has a 2.2 test asserting that in network mode × and every direction emit no press and leave the camera still, and that □, L1, R1 and Start change nothing. It also has a test asserting that ○ in free mode with no box does nothing. This step updates them (decision S13). The planet in the first test has no slipway, so the directions keep doing nothing there. □ leaves that test's list and gets its own test. The ○ test stays true, because it runs with no starting planet. *Fallback:* if the buttons are split over several tests, update each that names □ or a direction in network mode.
- **H-2.2-10 The startup notice.** The pad-mode startup notice in the game binary ("not playable yet") is unchanged by 2.2. *Fallback:* none needed. This step leaves it too (decision S12).

## Problem Statement

After step 2.2, a pad player can start a game, probe, fold space and colonise a planet. The player then stands on that colony in network mode, with a blinking circle around it. And there they are stuck:

- □ does nothing, so the player cannot leave the colony to look at the rest of the map. △ on the same planet is the only button that does anything.
- No slipway can be built. With the mouse, a slipway is a drag from one colony to another. The pad has no drag and no second point to drag to.
- Network mode exists, but the D-pad does nothing in it, so the player cannot move along their own slipways. That was the reason network mode exists: "the D-pad jumps between planets along the player's own slipway network".
- Nothing brings the player back to where they were after looking around.

A mouse player covers all of this with clicks and drags. A pad player cannot yet build an economy at all, because trade only flows along slipways.

## Solution

- **□ steps out into free mode.** In network mode, □ centres the camera on the selected planet and switches to free mode. The centre cursor is then on that planet, which becomes the **starting planet**.
- **× on a planet builds a slipway.** In free mode with a starting planet, the player pans until another planet is under the centre cursor and presses ×. Pad navigation then plays the mouse's own gesture through the existing dispatch: a left press on the starting planet, then a release on the target.
  - If the game accepts the drop, the slipway is built and paid for exactly as with the mouse. The player lands on the target in network mode, with the camera centred on it and the new slipway selected.
  - If the game refuses it (target not colonised, too far, crossing, a duplicate, not enough credits), nothing happens. Nothing is charged and nothing is left behind, and the player stays in free mode with the same starting planet.
  - The starting planet may be anywhere: on screen, or off screen because the player panned a long way. The gesture is the same in both cases.
- **○ goes back.** In free mode with a starting planet, ○ returns to that planet in network mode. The camera is centred on it, and the slipway that was selected before □ is selected again. With no starting planet (game start), ○ still does nothing.
- **Left/Right pick a slipway.** In network mode the selected planet's slipways are ordered clockwise on screen, starting from straight up. Right selects the next one clockwise and Left the previous one, wrapping around. The selected slipway is drawn in bright blue (colour 12).
- **Up/Down travel.** Up or Down moves the selection to the other end of the selected slipway, and the camera centres on the new planet. The slipway stays selected, so pressing again brings the player back.
- **No auto-repeat.** Every one of these acts on the press only.
- **A planet with no slipway:** Left/Right/Up/Down do nothing, and □ is the way out.
- **Proof:** a new pad replay, `tests/e2e/pad_slipway.json`, colonises two planets and builds a slipway between them. The slipway's starting planet is off screen when × is pressed. The replay travels back and forth along it, and shows a refused target. Its `.txt` records `make state`'s end state next to `keyboard_slipway`'s.

## User Stories

1. As a pad player, I want □ in network mode to switch to free mode, so that I can look at and act on the rest of the map.
2. As a pad player, I want the free cursor to start exactly on the planet I was on, so that I know where I left from.
3. As a pad player, I want the planet I left from to be remembered as my starting planet, so that × and ○ know where I came from.
4. As a pad player, I want □ to do nothing in free mode, so that a stray press never changes my starting planet.
5. As a pad player, I want the blinking circle and the hidden cursor to go away when I enter free mode, so that I see the centre cursor I now steer.
6. As a pad player, I want to pan freely with the usual speed curve while I have a starting planet, so that I can reach any planet in range.
7. As a pad player, I want × on another colony under the centre cursor to build a slipway from my starting planet to it, so that I can link colonies without a mouse.
8. As a pad player, I want the slipway to cost and advance the clock exactly as a mouse-built one does, so that the pad changes no game rule.
9. As a pad player, I want to land on the target colony in network mode after the slipway is built, so that I can keep building from there.
10. As a pad player, I want the camera centred on the planet I land on, so that I see where I am.
11. As a pad player, I want the slipway I just built to be the selected one, so that Up or Down takes me straight back.
12. As a pad player, I want × on a planet that is not colonised to do nothing, so that a refused target costs me nothing.
13. As a pad player, I want × on a planet too far away, behind a crossing or already linked to do nothing, so that every refusal the mouse has is kept.
14. As a pad player, I want × with too few credits to do nothing, so that I am never charged for a refused slipway.
15. As a pad player, I want to stay in free mode with the same starting planet after a refusal, so that I can simply pan to another target and try again.
16. As a pad player, I want × on my starting planet itself to do nothing, so that it never opens its information panel by surprise.
17. As a pad player, I want × on a planet to do nothing when my starting planet is not colonised, so that it never opens that planet's colonise menu instead.
18. As a pad player, I want × on a planet to do nothing when I have no starting planet, so that step 2.1's game-start rule holds.
19. As a pad player, I want × on empty space to keep launching a probe or a fold while I have a starting planet, so that probing still works the same.
20. As a pad player, I want × still held after a refused slipway to start nothing, so that one press never becomes a fold.
21. As a pad player, I want a slipway to work when its starting planet has scrolled off the screen, so that I can link two colonies that do not fit on one screen.
22. As a pad player, I want × to do nothing when my starting planet sits under a HUD item that would take the click, so that × never presses a structure icon by accident.
23. As a pad player, I want ○ in free mode to return me to my starting planet, so that I can back out of looking around.
24. As a pad player, I want the camera centred on the starting planet when ○ returns me there, even after a long pan, so that I see it at once.
25. As a pad player, I want the slipway I had selected before □ to be selected again after ○, so that coming back changes nothing.
26. As a pad player, I want ○ in free mode at game start, with no starting planet, to do nothing, so that step 2.1's rule holds.
27. As a pad player, I want △ on a planet in free mode to land me there as before, so that step 2.2's menu path is unchanged.
28. As a pad player, I want Right to select the next slipway clockwise and Left the previous one, so that the direction I press matches the way the selection turns.
29. As a pad player, I want the order to follow the slipways' directions on screen, starting from straight up, so that it is predictable.
30. As a pad player, I want the selection to wrap around, so that I can reach every slipway in either direction.
31. As a pad player, I want a slipway selected as soon as I stand on a planet that has any, so that Up/Down work without a Left/Right first.
32. As a pad player, I want the selected slipway drawn in bright blue, so that I see which way Up/Down will take me.
33. As a pad player, I want a freshly built slipway to show its build animation before turning bright blue, so that the game's build feedback is kept.
34. As a pad player, I want an obstructed or probe-targeted slipway to keep its red colour while selected, so that warnings are not hidden.
35. As a pad player, I want Up and Down both to travel to the other end of the selected slipway, so that I do not have to think about which end is "up".
36. As a pad player, I want the camera to follow the selection when I travel, so that the planet I am on stays in view.
37. As a pad player, I want the slipway I travelled along to stay selected, so that pressing again brings me back.
38. As a pad player, I want a held direction to act once, so that I never overshoot along the network.
39. As a pad player, I want Left/Right/Up/Down to do nothing on a planet with no slipway, so that nothing surprising happens.
40. As a pad player, I want the blinking circle visible at once on every planet I land on, so that I never lose track of the selection.
41. As a pad player, I want the planet menu (△) and its ○ close to work as before in network mode, so that step 2.2 is unchanged.
42. As a pad player, I want a starting planet that stops existing (replaced by a project) to be forgotten, so that ○ and × never act on nothing.
43. As a pad player, I want every new game to start with no starting planet and no selected slipway, so that nothing carries over.
44. As a pad player, I want the pad cheat (× × ○ ○ × × ○ ○) to keep working, accepting the navigation side effects of its presses, so that step 1.2's behavior survives.
45. As a mouse player, I want mouse mode exactly as before, so that my controls and the 26 mouse replays are untouched.
46. As a keyboard-cursor replay, I want `button_cursor` to run exactly as before, so that the 4 `keyboard_*` hashes stay unchanged.
47. As a maintainer, I want the slipway built through the existing `left_press` / `drag` / `accept_drop` / `drop` dispatch, so that no slipway rule is copied into pad navigation.
48. As a maintainer, I want acceptance read from the world after the drop, not predicted before it, so that pad navigation never guesses what the game would refuse.
49. As a maintainer, I want the off-screen starting planet handled by the same code path as an on-screen one, so that there is no special case to rot.
50. As a maintainer, I want the clockwise order in one read-only function with a small interface, so that it can be unit-tested with planets at known angles.
51. As a maintainer, I want the new state to be plain ints and entity ids in `pad_nav`'s existing state, so that the module still allocates nothing and the pointer-guard count does not change.
52. As a maintainer, I want the selected slipway published through the same ui fact as the selected planet, so that the drawing reads one source.
53. As a maintainer, I want the bright slipway drawn only in pad mode with a selection, so that no mouse or `keyboard_*` frame changes.
54. As a maintainer, I want the slipway draw's random draws left untouched, so that transit dots and every hash stay deterministic.
55. As a maintainer, I want unit tests for the clockwise order, Left/Right, Up/Down travel, □, ○, an accepted ×, each refusal, the off-screen start and the HUD guard, so that each claim is checked.
56. As a maintainer, I want a pad replay that builds a slipway from an off-screen starting planet, so that the plan's key risk is covered by the code a human plays.
57. As a maintainer, I want that replay's `.txt` to put its `make state` end state next to `keyboard_slipway`'s, so that coverage of the old keyboard scenario is shown, not assumed.
58. As a maintainer, I want the earlier pad hashes stated up front (none expected to change), so that any unexpected change is investigated, not refreshed.
59. As a maintainer, I want `git diff --stat tests/e2e/` to show no mouse or `keyboard_*` `.sha256` changed, so that I know none of this reaches mouse or keyboard play.
60. As a future implementer of phase 3, I want the starting planet and the gesture to be cancellable state in one place, so that L1/R1's "cancel the sub-mode, as ○ would" has one thing to clear.

## Implementation Decisions

### State added to `pad_nav`

- **S1 — Three ids and a gesture, nothing else.** On top of 2.2's map mode and selection (H-2.2-1), `pad_nav`'s state gains:
  - the **starting planet** (`entity_id_t`), set by □ and meaningful in free mode only;
  - the **selected slipway** (`entity_id_t`), meaningful in network mode;
  - the **starting slipway** (`entity_id_t`), the selected slipway saved by □ so that ○ can restore it;
  - the **gesture**: a phase int (none, release due, landing check), the target planet id, its screen point for the release, and a "linked before" int.

  There is no pointer and no allocation, so the pointer-guard count is unchanged.
- **Reset rules** extend 2.2's. All new ids go back to none when 2.2's state resets: the title, a new economy, or a dead selection. In free mode, a starting planet that is gone or dead is treated as none, and so is its saved slipway. That is checked through `entity_get` and `is_dead`, as 2.2 does for the selection. A panel that opens cancels a pending gesture (S5). It does not clear the starting planet, so free mode comes back intact when the panel closes.

### Network mode — slipways and travel

- **S2 — Clockwise order, one read-only function.** A public, read-only `pad_nav` function (indicative `pad_nav_next_slipway(planet, current, step)`) returns a slipway attached to the planet:
  - `step` +1 returns the next one clockwise from `current`, and −1 the previous one, wrapping around;
  - with `current` NULL, or `current` not one of the planet's slipways, it returns the first one clockwise from straight up;
  - it returns NULL when the planet has no slipway.

  The order is the direction from the planet's position to the other end's position (`slipway_other_end`), measured clockwise on screen from straight up (screen y grows downward). Straight up itself comes first.
  - The comparison uses plain C ints on floored pixel deltas: first the half (right half, straight up included, before the left half), then the sign of the cross product. There is no trigonometry and no `p8_fixed` product, so it cannot overflow.
  - Two slipways with the same direction keep the order of the planet's slipways list. The game's obstruction rule makes that case practically impossible.
- **S3 — The camera follows by centring, at once.** Whenever this step changes the selection (travel, ○'s return, landing after a built slipway) or enters free mode with □, the camera is panned through `camera_pan` by the whole-pixel difference between the planet's position, floored, and the camera's position before shake plus the screen centre (80, 60). `camera_pan` applies the pad-mode bound, which keeps every planet reachable under the centre (2.1's step note), so a planet near the bound may be left a few pixels off centre.
  - The move is one jump, not an animated scroll.
  - 2.2's △ landing is not changed (H-2.2-7).
- **S4 — Buttons in network mode**, with no panel open:
  - The **selected-slipway invariant** is checked first, every frame. If the selected planet has slipways and the selected slipway is not one of them, it becomes the first clockwise from straight up (S2). If the planet has none, it is none.
  - **Right / Left**, on a press edge, select the next / previous slipway (S2). Nothing happens with no slipway.
  - **Up / Down**, on a press edge, move the selection to the other end of the selected slipway. The selected slipway is kept, and the camera centres (S3). Nothing happens with no slipway. When several directions go down on the same frame, 2.1's priority applies: Up, Down, Left, Right. Up or Down wins, so a frame never travels and cycles at once.
  - **□**, on a press edge: centre the camera on the selected planet (S3), save the selected slipway as the starting slipway, set the starting planet to the selected planet, and switch to free mode.
  - **△** and **○** keep 2.2's behavior. **×**, **L1**, **R1** and **Start** do nothing.
  - The network cursor stays 2.2's: the selected planet's screen point.
  - Every landing restarts the circle's blink from the visible phase (2.2's "frames since last shown" counter is zeroed), so the circle shows on the landing frame.

### Free mode with a starting planet

- **S5 — × on a planet is the mouse's press-then-release, split over two frames.** On a × press edge in free mode, after this frame's pan, with no △ release pending:
  1. Find the planet-kind entity under the centre (H-2.2-2): the **target**.
  2. The gesture starts only if all of these hold:
     - there is a starting planet;
     - the target exists and is not the starting planet;
     - the starting planet is settled, which is the state whose `left_press` hands the ui a slipway;
     - the starting planet's screen point is not inside any entity that has a `left_press` and sits on a higher layer than the starting planet (decision S7).
  3. If the gesture starts:
     - **Press frame (this frame):** the synthetic cursor is the starting planet's screen point, its position minus the camera position, floored. Nothing clamps that point to the screen. The left button is down. `pad_nav` records the target, the target's screen point, and whether a slipway already linked the starting planet and the target ("linked before"). It finds that out by walking the starting planet's slipways list with `slipway_other_end`. The gesture phase becomes "release due".
     - **Release frame (next frame):** the synthetic cursor is the recorded target's current screen point, recomputed after this frame's pan so a held D-pad cannot move the drop off the target. The left button is up, whatever × does. The phase becomes "landing check".
     - **Check frame (the frame after):** if a slipway now links the two planets and none did before, the drop was accepted. The mode becomes network on the target, the selected slipway is that new slipway, and the camera centres (S3). Otherwise nothing changes: free mode, same starting planet, centre cursor. Either way the phase returns to none.
  4. If the gesture does not start, 2.1's rule applies unchanged. × on anything with a `left_press` other than the background presses nothing, and × on the background engages the held left button.
  5. × never engages the held left button during the gesture or after it. Engagement needs a fresh press edge, so a × held through a refusal starts no fold.
  6. During the release frame, △, □, ○ and × are ignored. The D-pad keeps panning. A panel that opens on either frame cancels the gesture (phase back to none), and step 1.4's panel navigation takes over.

  Everything between the press and the drop is the existing dispatch. On the press frame, the ui hovers the starting planet, and its `left_press` makes the slipway (`planet_left_press`, or `slipgate_left_press` for a gate). `ui_handle_button` stores it as held and calls `drag` (end = start, so no tooltip). On the release frame, the ui hovers the target, `drag` hangs the end on it and computes the obstruction and the cost, `accept_drop` goes to the hovered layers, and `drop` calls `targeting_confirm`, which pays and advances the clock or kills the slipway. No slipway rule exists in `pad_nav`.
- **S6 — The off-screen starting planet (the step's key risk).** It needs no special case.
  - The played frame's `mouse_x` / `mouse_y` are plain ints that nothing clamps (`p8_input_read_mouse`, `mouse_update`). The ui computes the world point as the mouse's screen point plus the camera position.
  - A planet's `hit_test` is `entity_geom_contains_cursor` on that world point (planets are layer 6, under the screen-space layers that start at 10). The hit-test bucket holds every registered planet, whether it is on screen or not: only the planet's draw is culled by `entity_geom_is_on_screen`, never its hit test.
  - So a press point such as (210, 40) or (−30, 140) hovers the starting planet, and its `left_press` makes the slipway exactly as for an on-screen point.
  - The cursor sprite and the placing tooltip are drawn at that point and are simply clipped.
  - The release frame happens on screen, at the target, so `accept_drop` and `drop` are unaffected.
  - The slipway's own draw needs one end on screen, and the target is on screen.
  - The longest slipway the game allows is about 133 px (squared map length 1.08), so the press point stays within about one screen of the centre.
  - The unit test and the replay both prove this case (Testing Decisions).
- **S7 — The HUD guard.** A screen point is hit-tested in screen space by layers 10 and up (buttons 11, structure icons 12) and in world space by the map. An on-screen starting planet can therefore lie under a revealed structure icon, which `ui_send_to_hovered` would press first. To keep × from ever pressing a HUD item, the gesture does not start when any entity with a `left_press` on a layer above the starting planet's contains the press point. × then does nothing, and the player can pan to move the starting planet out from under the HUD and try again.
  - This check only decides whether to emit the press. It takes no decision of its own about the slipway, and it uses the same class `hit_test`s as the helper of H-2.2-2.
  - An off-screen press point can never hit a screen-space item.
  - At game start no structure icon is revealed (they sit at (999, 999) until a tech reveals them), so `pad_slipway` never meets the guard.
- **S8 — ○ and □ in free mode.**
  - **○**, on a press edge, with a starting planet and no gesture pending: the mode becomes network, the selection the starting planet, and the selected slipway the saved starting slipway (S4's invariant repairs it if it no longer applies). The camera centres (S3). Without a starting planet, ○ does nothing (2.1's rule).
  - **□** does nothing in free mode.
  - **△** keeps 2.2's behavior. It lands network mode on its target, and the starting planet is dropped, because network mode does not use it.

### Network visuals, this step's part

- **S9 — The bright slipway.**
  - The ui's pad-selection fact (H-2.2-6) gains a selected slipway (`entity_id_t`, none from `ui_new`). `pad_nav` writes it with the planet, through the same setter: the selected slipway in network mode with no panel open, none otherwise. Mouse and `keyboard_cursor` modes never call the setter, so their ui keeps none.
  - The slipway draw replaces its core line colour by 12 when the slipway is the ui's selected slipway, its fade-in has finished (fade-in step 20, the last trail colour), and it is not in the red targeted state (colour 2), which keeps priority.
  - The border line, the infraspace variant's border and the transit dots are unchanged. So is the order of the draw's random draws (`slipway_spawn_transit_dot`), so the RNG stream cannot move.
  - Colour 12 is PICO-8's bright partner of the slipway's settled colour 1, and the plan's hover colour.
- **S10 — Free mode shows no marker on the starting planet.** The plan asks for none: free mode shows the cart's cursor sprite, and 2.2's pad selection is none in free mode.
- **S11 — The gesture's one-frame cursor jump is accepted.** On the press frame, the cart's cursor sprite is drawn at the starting planet's screen point. It is invisible when that point is off screen. On the release frame it is drawn at the target, within 7 px of the centre. After an accepted drop, network mode hides the cursor on the next frame. Hiding the sprite on these two frames would need a third ui state for one frame of difference.

### Files not touched

- The input step is unchanged: `pad_nav` still runs in the same slot, before the cheat and the dispatch install.
- `src/cmd/game.c`, `src/cmd/replay.c` and `src/platform/` are not edited, so "`make raylib` builds" is not part of this step's check (§2 Invariants).
- **S12 — The pad-mode startup notice stays word for word.** A pad game still cannot end a year (the `end turn` tab is phase 3's), so it is still not playable end to end, and editing `game.c` would add a `make raylib` check for nothing.
- No README change: the controls section moves to the pad in step 4.2.

### Decisions settled without a human (guideline rules)

No `guideline` agent could be spawned in this run, so each decision was settled directly with the `guideline` skill's rules, in its DECISION / WHY form.

- **S2 — Right is clockwise, and a slipway is always selected when one exists.** DECISION: Right = next clockwise, Left = previous. The first slipway counted from straight up is selected as soon as the planet has one. WHY: §2 network mode, "Left/Right cycle through the slipways … clockwise on screen" and "Up and Down both move the selection to the other end of the selected slipway". Up/Down need a selected slipway to do anything, and Right reads as "turn right" (clockwise).
- **S3 — Instant centring.** DECISION: the camera jumps so the new selection is centred, through `camera_pan`. WHY: §2, "The camera follows the selection", with no speed given. Between two options that both work, take the smaller change. An animated follow is new camera code.
- **S5 — A two-frame gesture, and acceptance read afterwards.** DECISION: press on the starting planet, release on the target the next frame, then compare the slipway links before and after. WHY: §2 structure, "synthetic presses … through `left_press` / `drag` / `accept_drop` / `drop`, so no game rule is duplicated". Predicting acceptance would copy `planet_accept_drop`, `slipway_obstruction` and `targeting_confirm`.
- **S5 — The starting planet must be settled, and must not be the target.** DECISION: otherwise × presses nothing. WHY: §2 free mode, "If there is no starting planet, or the drop is refused, × does nothing". A press on a known starting planet would open its colonise menu (`planet_left_press`), and a release on the starting planet itself opens its inspection panel (`planet_accept_drop`). Both break "does nothing". The state check is the same kind as 2.2's N2.
- **S6 — No special case for the off-screen start.** DECISION: emit the starting planet's true screen point, unclamped. WHY: the code: input frames are unclamped ints, world = screen + camera, planets are hit-tested in world space whether they are on screen or not.
- **S7 — The HUD guard.** DECISION: no gesture when an item with a `left_press` on a higher layer contains the press point. WHY: "Keep it safe": × must never press a structure icon. The rare case where a revealed icon covers the starting planet costs a pan, not a wrong action.
- **S8 — ○ restores the saved slipway.** DECISION: □ saves the selected slipway and ○ restores it. WHY: §2, "○ returns to the starting planet". Returning means coming back to the state the player left.
- **S9 — Colour 12, after fade-in, red keeps priority.** DECISION: as S9. WHY: §2, "The selected slipway is drawn in a brighter colour". 12 is the brighter partner of 1, and keeping the fade-in and the red warning hides no cart feedback.
- **S11 — Accept the one-frame cursor jump.** DECISION: no special hiding on the gesture's frames. WHY: don't get drawn into detail. One frame, and none when the start is off screen.
- **S12 — Keep the startup notice.** DECISION: unchanged. WHY: the game is still not playable end to end, and touching `game.c` widens the step.
- **S13 — Frozen tests updated, not deleted.** DECISION: 2.2's "nothing happens in network mode" test keeps × and the four directions on its slipway-less planet, plus L1, R1 and Start, and drops □. 2.1's "× on a planet presses nothing" and 2.2's "○ in free mode does nothing" keep running unchanged, because they run with no starting planet. WHY: CLAUDE.md, "update that test"; guideline, "no test is skipped or weakened".
- **S14 — Earlier pad replays expected to change: none** (H-2.2-8). DECISION: neither `pad_title`, `pad_probe` nor `pad_colonise` is refreshed.
  - `pad_title` and `pad_probe` never have a starting planet. Their ○ (`pad_probe` f16) and their × presses (f26, f41–56, on empty space) take 2.1's unchanged path.
  - `pad_colonise` never presses □ and has no slipway, so no direction, no ○ return and no bright slipway reaches it, and its △ landing does not centre the camera (S3).

  If one of these hashes moves anyway, find out why before going further. Refresh it only if the cause is this step's own behavior, one file at a time, with `make state` rerun and the step note saying why. WHY: §2 Invariants, "Each such step's check states up front which earlier `pad_*` replays it expects to change".
- **S15 — `pad_slipway`'s frame budget.** DECISION: aim for 180 frames or fewer. If the galaxy does not allow it, add one line for `tests/e2e/pad_slipway.json` to `tools/e2e_long_exceptions.txt`, with its reason, and keep the replay at 300 frames or fewer. WHY: the off-screen case needs two colonies at least 53 px apart vertically (or 73 px horizontally). One fold reveals planets within about 33 px of its point, and `pad_probe`'s two (p12, p14) are 49 px apart vertically. So the replay needs a second fold, which by estimate puts it around 200–260 frames. §2 asks for this scenario in one file. The exception list is the repository's mechanism for it, and plan step 3.4 uses the same rule. `make e2e`'s wall time is measured before and after, and must not move beyond noise.

## Implementation Chunks

1. **□, ○ and the × gesture (free mode with a starting planet).**
   - The new state and reset rules (S1).
   - □ in network mode and the camera centring (S3, the □ part of S4).
   - ○'s return (S8).
   - The gesture with its three frames, the starting-planet checks and the HUD guard (S5, S7).
   - The off-screen path (S6), which needs no code of its own but gets its own test.
   - H-2.2-2's helper extracted if needed.
   - Update the frozen 2.2 test (S13).
   - New tests listed under Testing Decisions, chunk 1.
   - Run `make e2e`: no `*.sha256` moved (S14).
2. **Network travel.**
   - The clockwise function (S2).
   - The selected-slipway invariant, Left/Right, Up/Down with centring, and the blink restart on landing (S4).
   - The selected slipway saved and restored by □/○, wired to chunk 1.
   - Tests listed under chunk 2.
   - Run `make e2e`: still no hash moved.
3. **The bright slipway.**
   - The ui's selected slipway next to the pad selection, written by `pad_nav` (S9), and the colour-12 core line in the slipway draw.
   - Tests listed under chunk 3.
   - Run `make e2e`: still no hash moved, because no earlier replay ever selects a slipway.
4. **The `pad_slipway` replay.**
   - Write `tests/e2e/pad_slipway.json` (`"input": "pad"`, no mouse field). Add the exception line only if S15 requires it.
   - Take the hash of that file only (never `make e2e-baseline` over the directory).
   - Run `make state EVENTS=tests/e2e/pad_slipway.json`, read the framebuffer dump at the key frames, and write `pad_slipway.txt` with the pairing.
   - Measure `make e2e` before and after.

## Testing Decisions

- **What a good test is here:** it drives the public interface and checks what a player sees, never `pad_nav`'s internal fields:
  - pad masks into the input step, then the played frame out (cursor point, left button);
  - `pad_nav_free_mode`, `pad_nav_selection` and the clockwise function's results;
  - the ui's hovered entity, its held entity, its open box and its pad selection (planet and slipway);
  - the world: slipways that exist and link which planets, the planets' slipways lists, entity counts (no stray probe, wormhole or slipway), credits and clock, and the camera's position before shake;
  - for the visuals, the framebuffer after a draw.
- **`test_pad_nav`** (existing binary, more tests, each in a forked child like the 2.1 and 2.2 tests).
  - **Setup:** a game is started directly. Planets are put in the settled or known state the way the existing planet-menu test and 2.2's tests do. Slipways that a test needs to exist beforehand are built through the game's own path, as `test_slipway` does (create from the start planet, drag, drop with enough credits), so that both planets' slipways lists hold them. The camera is placed with `camera_pan`. Frames then run through the input step, with `cart_update60()` between them.
  - *Chunk 1:*
    - □ in network mode: free mode, starting planet = the former selection, played cursor at the centre, the planet's screen point equal to the centre (or as close as the bound allows), ui pad selection none.
    - □ in free mode changes nothing.
    - **Accepted ×:**
      - two settled planets within range and unobstructed, the target under the centre;
      - on the × press frame, the played cursor is the starting planet's screen point and the left button is down, and after it the ui holds a slipway whose start is the starting planet;
      - on the next frame, the cursor is the target's screen point and the button is up;
      - one frame later, a slipway links the two planets, credits dropped by its cost, the clock advanced, the mode is network on the target, the selected slipway is the new one and the camera is centred on the target.
    - **Off-screen start:** the same with the planets about 100 px apart horizontally (or 70 px vertically). Assert that the starting planet's screen point lies outside 0..159 × 0..119 on the press frame, and that the slipway is still built and paid for.
    - **Refusals**, each leaving slipway count, credits, clock and entity count unchanged, no box open, free mode with the same starting planet, and the centre cursor back on the check frame:
      - the target known, not settled;
      - the target too far (beyond the squared 1.08 map length);
      - a duplicate of an existing slipway: "linked before" holds, so there is no landing;
      - credits set below the cost.
    - **No gesture**, so nothing is pressed or opened:
      - × on the starting planet itself, with no inspection panel;
      - × with a known (unsettled) starting planet, with no projects panel;
      - × on a planet with no starting planet (2.1's frozen test, unchanged);
      - a structure icon moved over the starting planet's screen point (HUD guard), with no structure held and no slipway.
    - × held 20 frames through a refusal engages nothing: no wormhole is created.
    - × on empty space with a starting planet still makes the fold or probe, held as in 2.1.
    - **○ with a starting planet**, after panning the starting planet off screen: network mode on it, the camera centred on it, the saved slipway selected. ○ with no starting planet does nothing (the existing test).
    - △ during the gesture's release frame is ignored, and so is a × press while △'s release is pending.
    - The starting planet marked dead: ○ and × on a planet do nothing.
  - *Chunk 2:*
    - **Clockwise function**, on a settled planet with slipways to four planets placed at known directions (straight up, right-down, straight down, left):
      - NULL start → the straight-up slipway;
      - +1 steps go round clockwise and wrap;
      - −1 steps go round the other way;
      - a planet with no slipway → NULL;
      - a slipway not attached to the planet → the first one.

      A fifth case puts two planets either side of straight up (dx −1 and +1) to pin the half boundary.
    - **Left/Right** through the input step select the previous/next slipway: the ui's selected slipway follows. A held Right moves once.
    - **Up and Down** each travel to the other end: the selection changes, the selected slipway is kept, and the camera is centred on the new planet. Up then Down returns to the start. A held Up travels once. A frame with Up and Right travels and does not cycle.
    - On a planet with no slipway, every direction changes nothing (2.2's updated test).
    - Landing on a planet that has slipways (by travel, by an accepted ×, by ○) selects a slipway at once.
  - *Chunk 3:*
    - In network mode the ui's selected slipway is the selected one. It is none in free mode and while a menu is open.
    - One draw, after the fade-in, shows colour-12 pixels on the selected slipway's core line and none on an unselected slipway's.
    - A mouse-mode draw of the same scene shows no colour 12 on either.

    The draws are kept to these two, so the binary stays at its current cost.
- **`test_input_step`:** no change expected. The live-against-recorded test must pass unchanged, because the gesture and travel are deterministic given the world and the pad masks.
- **E2E — `pad_slipway.json`.** The script below is indicative: the exact frames come from this galaxy's dump.
  - **Title, probe and first fold:** `pad_probe`'s frames f0–57 verbatim, which picks the same galaxy and makes the same probe and first fold. Then pan (it may start during the unfolding) until p12 or p14, known from f68, is under the centre.
  - **First colony:** △ opens its menu, × on the colonise row. This is colony A, and the player is in network mode on it.
  - **Refused target:** □ (A becomes the starting planet), pan to the other known planet of the first fold (not colonised), ×. Refused: credits, clock and slipways unchanged, still in free mode.
  - **Second fold and colony:** pan to a fold point, anchored on the probe or on colony A, that reveals a planet B at least 53 px vertically or 73 px horizontally from A, within slipway range, with no planet or slipway near the segment A–B. Hold × at least 16 frames. Once B is known, pan to it, △, × on the colonise row: the player is in network mode on B.
  - **The off-screen slipway:** □ (B becomes the starting planet), pan back to A until A is under the centre and B is off screen, ×. The slipway B–A is built, and the player lands on A, centred, with the slipway selected (bright blue after its fade-in).
  - **Travel:** Down (to B, camera centred), Up (back to A). Left/Right on a planet with one slipway keeps it selected.
  - **Idle:** a few frames at the end.
  - Avoid the cheat pattern × × ○ ○ × × ○ ○.
  - If the galaxy offers a cheaper legal order (for example, the refused target taken on the way to the second fold point), use it. S15 bounds the length.
- **`pad_slipway.txt`** follows the `keyboard_*` notes. It gives the frame-by-frame script and what the dump shows at the key frames: the menu rows; the refused × with nothing drawn after it; the second fold; the frame where × is pressed, with B's screen point off screen as `make state` gives it; the new slipway's fade-in and then colour 12; the blinking circle on each landing; the camera centred after travel.
  - **End state:** `make state`'s lines: `slipway <B> <A> normal`, colonies 2, slipways 1, credits, income, clock, techs 0.
  - **Pairing:** beside the end state sits `keyboard_slipway`'s end state: `slipway p88 p30 normal`, 2 colonies, 1 slipway, $80, income 3, month 9. The note says the galaxies differ (2.1's note), that the pairing compares the shape (two colonies, one slipway, its cost and clock effect), and that this replay covers `keyboard_slipway`'s second colony and its slipway (f94–159). The probe and fold are `pad_probe`'s, and the first colony is `pad_colonise`'s.
  - **Length:** the frame count, and the exception line if S15 required one.
- **Unchanged:** every mouse and `keyboard_*` `*.sha256`, plus `pad_title`, `pad_probe` and `pad_colonise` (S14). `git diff --stat tests/e2e/` shows only the three new `pad_slipway` files.
- **Speed:** the new unit tests follow `test_pad_nav`'s patterns (forked children, a few hundred `cart_update60()` calls at most, two draws in all). `make test` and `make e2e` must not get measurably slower.
- **Prior art:**
  - `tests/test_pad_nav.c`: 2.1's free-mode tests (`test_free_mode_cross_on_a_planet_presses_nothing`, `test_free_mode_pans_with_the_speed_curve`, the forked-copy helpers) and 2.2's network-mode tests;
  - `tests/test_slipway.c`: building slipways through the game's own path;
  - `tests/e2e/slipway_build.txt` and `tests/e2e/slipway_refused.txt`: the mouse gesture and its refusals;
  - `tests/e2e/keyboard_slipway.txt`: the keyboard scenario this replay pairs with;
  - `tests/e2e/pad_probe.txt`: the shared prefix and its known planets.

## Out of Scope

- An animated camera follow (S3).
- A slipway preview (rubber band, cost tooltip) while panning in free mode with a starting planet, and any marker on the starting planet (S10).
- Hiding the cursor sprite on the gesture's two frames (S11).
- Changing 2.2's △ landing to centre the camera (H-2.2-7), and 2.2's menu, ○ close or visuals.
- Tabs, L1/R1, Start, pause, the build tab and placement mode, and the modality rules ("L1/R1 during free mode: cancel the sub-mode, as ○ would"), all phase 3.
- Following a planet replaced by a project with the selection or the starting planet (2.2's N10).
- Dropping the pad-mode startup notice (S12); README and `docs/plan-10.md` (step 4.2).
- Audio.

## Further Notes

- **The step note for 2.3** must record:
  - that no earlier pad hash changed (S14), or, if one did, which file, why, and that `make state` still gives its end-state lines;
  - `pad_slipway`'s end state next to `keyboard_slipway`'s, and that it covers that replay's second colony and slipway, which completes the `keyboard_slipway` → `pad_probe`, `pad_colonise`, `pad_slipway` mapping that step 4.1 copies;
  - the frame count, and the long-exception line if S15 needed one, with the measured `make e2e` time before and after;
  - decisions S3 (instant centring), S5 (two-frame gesture, acceptance read afterwards), S6 (no special case for the off-screen start) and S7 (HUD guard);
  - any H-2.2-k whose behavior turned out false, with the fallback applied;
  - that every mouse and `keyboard_*` `*.sha256` is unchanged.
- Phase 3's modality step inherits one place to cancel: dropping the starting planet and any pending gesture is "cancel free mode, as ○ would".
- Every later pad replay that selects a slipway shows colour 12. A step that changes those visuals refreshes `pad_slipway` under §2's rule.
