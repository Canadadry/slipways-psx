---
title: "Free mode: the pad game starts with a fixed centre cursor and a panning camera"
description: "After the title screen, a pad player reaches a map where the D-pad and × do nothing, so the starting probe and the first space fold cannot be placed."
status: needs-triage
---

## Hypotheses on step 1.4

Checked against step 1.4 as landed (its step note in `docs/plan-15.md`): every hypothesis holds. `pad_title` presses × at f18 and idles to f29; no panel failed.

Step 1.4 (`docs/prd/triage/phase1-step4-pad-nav-title.md`) is not finished while this PRD is written. Its chunk 1 is committed at `HEAD` (`bc7088e`: `pad_nav`, generic panel focus, title tests). Chunk 2 (reachability tests for the other panels) and chunk 3 (the `pad_title` replay) are not. This PRD reads only committed code.

**Rule for the implementer:** if step 1.4 landed with a different name or spelling, use what landed. Only a hypothesis whose *behavior* turns out false needs a line in the step note. When that happens, its fallback applies.

- **H-1.4-1** `pad_nav` keeps the shape it has at `HEAD`: a plain state struct inside the input step (focus as an `entity_id_t`, the previous pad mask, the last cursor point), and a step that writes only the played frame's `mouse_x`, `mouse_y` and `mouse_buttons`. The panel-open rule is the three checks: no game running, a non-empty ephemerals list, or a registered modal backdrop. With no panel open, the cursor stays still and the left button stays up. *Fallback:* free mode replaces whatever "no panel open" branch landed, and panel navigation keeps precedence over it.
- **H-1.4-2** `tests/e2e/pad_title.json` lands as its PRD describes. It has about 30 frames, × on "reasonable" at about f18, then idle in-game frames into the first month, and every frame is in the capture. Free mode changes those in-game frames: the cursor jumps from the "reasonable" row to the screen centre, and its sprite follows what lies under the centre. So **`pad_title` is the one earlier pad replay this step expects to change.** *Fallback:* if `pad_title` ends on the × frame, or its hash does not change, it is left untouched and the step note says so.
- **H-1.4-3** `pad_title` presses nothing after the × that starts the game. Its `make state` end-state lines (clock year 3401, credits 100, planets 90, colonies 0, slipways 0, techs 0) are unchanged by free mode, which changes no game state while nothing is pressed. *Fallback:* if `pad_title` holds a direction after the start, the camera now pans. The step note records that only the camera and the framebuffer changed, and checks that the end-state lines still hold.
- **H-1.4-4** In chunk 2's results, every panel needed later passes its reachability test. The findings are the two already settled for the plan: "help" is the title screen, and the year message box (no button) is not a panel. That box does not make `pad_nav_panel_open` true, because it adds nothing to the ephemerals list. So free mode keeps running under it. *Fallback:* if the year box counts as an open panel, this step does not change the panel-open rule. Free mode simply pauses while the box is shown, and the step note records it for step 3.3 (modality).
- **H-1.4-5** No failure found by chunk 2 blocks free mode, because free mode opens no panel. *Fallback:* a failing panel is settled by its own fix before this step, as §2 asks ("settled before phase 2 starts"). This PRD does not absorb it.
- **H-1.4-6** Step 1.4 does not touch the cart's camera module, the background, the probe, the wormhole, the pad replay format or `tools/e2e_limits.sh`. In the input step, the cheat still runs after pad navigation. `test_pad_nav` still has a test freezing "no panel open leaves the cursor still". This step rewrites that test.

## Problem Statement

Plan 15 makes the game playable with a PSX-style pad. After phase 1, a pad player can pick a difficulty on the title screen, and that is all. Once the game starts there is no panel open, so pad navigation does nothing:

- the cursor stays frozen on the spot where the difficulty row was;
- the D-pad moves nothing, because the pad mode deliberately keeps it away from the cart's camera scroll;
- × presses no button, so the starting probe cannot be placed, and no space fold can be made.

The game has no home planet: every game starts by placing a probe in empty space and folding space from it. Without a way to point at empty space, a pad game cannot even begin. Steering the old keyboard cursor (`button_cursor`, 1–2 px per frame) across the screen is the slowness the plan was written to remove.

## Solution

- **The game starts in free mode** (also called probe mode). Whenever a game is running and no panel is open, pad navigation is in free mode. In this step it is the map's only mode: network mode comes in step 2.2.
- **The cursor is fixed at the centre of the screen,** and the cart's cursor sprite is drawn there with its usual variants (whatever hovering shows for what lies under it).
- **The D-pad moves the camera** at 1 px per frame, then 2 px per frame after 10 frames held, then 4 px per frame after 40 frames held. A held direction keeps moving the camera: this is the plan's one exception to "no auto-repeat". Every planet of the galaxy can be brought under the centre.
- **× on empty space is a left click on the background:** the first one places the starting probe, and each later one starts a space fold (a wormhole), exactly as the mouse does. Holding × holds the button, so a fold is confirmed by holding × for 15 frames or more, then releasing it. You stay in free mode afterwards.
- **× on anything else does nothing** in this step: a planet, a structure, or any other clickable thing under the centre. With no starting planet, the plan says × on a planet does nothing. The slipway it will create comes in step 2.3, and △'s planet menu in step 2.2.
- **○ does nothing** while there is no starting planet, which is always the case in this step. □, △, L1, R1 and Start do nothing in free mode for now.
- **Panels keep precedence.** When a panel opens (the title, a box, the score panel), step 1.4's panel navigation takes over. When it closes, the cursor goes back to the centre.
- **Proof:** a new pad replay, `tests/e2e/pad_probe.json`, starts a game from the title. It places the starting probe with ×, pans the camera, and confirms a space fold by holding ×, which is `keyboard_slipway`'s fold. Its `.txt` records `make state`'s end state next to `keyboard_slipway`'s state after the same fold.

## User Stories

1. As a pad player, I want to land in free mode as soon as the game starts, so that I can place my starting probe without pressing anything first.
2. As a pad player, I want the cursor fixed at the centre of the screen, so that I always know where × will act.
3. As a pad player, I want the cart's cursor sprite drawn at the centre with the same variants as under the mouse, so that I can see what I am pointing at.
4. As a pad player, I want a held direction to move the camera every frame, so that I can travel across the map without tapping.
5. As a pad player, I want the first 10 frames of a hold to move 1 px per frame, so that short presses give precise aim.
6. As a pad player, I want a hold longer than 10 frames to speed up to 2 px per frame, so that medium distances are quick.
7. As a pad player, I want a hold longer than 40 frames to speed up to 4 px per frame, so that crossing the whole map does not take forever.
8. As a pad player, I want releasing a direction to reset its speed, so that the next press is precise again.
9. As a pad player, I want each direction to keep its own speed, so that a diagonal made of two holds started at different times behaves predictably.
10. As a pad player, I want two opposite directions held together to cancel out, so that the camera does not drift.
11. As a pad player, I want a diagonal (two adjacent directions) to move the camera on both axes, so that I can aim diagonally.
12. As a pad player, I want to be able to bring every planet of the galaxy under the centre cursor, so that no part of the map is out of reach, as it is not with the mouse.
13. As a pad player, I want the camera to stop at the edge of the galaxy, so that I never get lost in empty space far from every planet.
14. As a pad player, I want the camera to move at once when I press a direction, so that the controls feel responsive.
15. As a pad player, I want × on empty space to place the starting probe, so that I can start my game.
16. As a pad player, I want × on empty space, once the probe exists, to start a space fold, so that I can reveal more of the map.
17. As a pad player, I want holding × to hold the fold and releasing it to confirm, so that folding works as the mouse hold does.
18. As a pad player, I want a fold released too early (under 15 frames) to be abandoned with nothing paid, as with the mouse, so that a quick tap is harmless.
19. As a pad player, I want a fold too close to a planet or too far from an anchor to be refused exactly as with the mouse, so that the game rules stay the same.
20. As a pad player, I want to stay in free mode after a probe or a fold, so that I can keep probing.
21. As a pad player, I want the D-pad to keep panning while I hold ×, so that holding and aiming work together as a mouse drag does.
22. As a pad player, I want × on a planet to do nothing for now, so that I never open a menu or start a slipway by accident before those controls exist.
23. As a pad player, I want × to do nothing on any other clickable thing under the centre, so that free mode only ever acts on empty space.
24. As a pad player, I want the × that chose my difficulty, still held when the game starts, not to place a probe, so that the title click never leaks into the map.
25. As a pad player, I want ○ to do nothing while there is no starting planet, so that the back button never surprises me at the start of a game.
26. As a pad player, I want □, △, L1, R1 and Start to do nothing in free mode for now, so that no half-built behavior surprises me before the steps that define them.
27. As a pad player, I want a panel opening during a game to take the D-pad and × for its own navigation, so that free mode never fights a panel.
28. As a pad player, I want the cursor back at the centre when that panel closes, so that free mode resumes where it was.
29. As a pad player, I want the pad cheat (× × ○ ○ × × ○ ○) to keep working in free mode, accepting that its × presses also act on the map, so that step 1.2's behavior survives.
30. As a pad player, I want the year message box not to stop free mode, so that a notice does not freeze my controls.
31. As a mouse player, I want mouse mode exactly as before, including the cart's camera limits and the D-pad scroll, so that my controls and the 26 mouse replays are untouched.
32. As a keyboard-cursor replay, I want `button_cursor` to run exactly as before, so that the 4 `keyboard_*` hashes stay unchanged.
33. As a maintainer, I want free mode to emit only a cursor point and a left button, so that probes and folds go through the existing dispatch and no game rule is copied.
34. As a maintainer, I want the camera moved by pad navigation, never by the cart's D-pad scroll, so that §2's rule "the navigation layer owns the camera" holds.
35. As a maintainer, I want the camera's velocity to stay zero in pad mode, so that it is provable that the D-pad never reached the cart's scroll.
36. As a maintainer, I want free mode's state to be a few ints in the existing pad navigation state, so that the module still allocates nothing and the pointer-guard count does not change.
37. As a maintainer, I want free mode in the float-free core, so that the PSX port inherits it unchanged.
38. As a maintainer, I want the speed curve tested pixel for pixel, so that its thresholds cannot drift.
39. As a maintainer, I want a test proving every planet can come under the centre cursor, so that "the whole map is reachable" is not just claimed.
40. As a maintainer, I want a test proving × on a planet emits no press, so that the swallowing rule holds.
41. As a maintainer, I want a test proving × held across the title click places nothing, so that the press-edge rule holds.
42. As a maintainer, I want the step 1.2 and 1.4 tests that froze the old "D-pad does nothing in a game" behavior updated and not deleted, so that their intent (mouse noise is ignored) is still checked.
43. As a maintainer, I want a pad replay that places the probe and confirms a fold, so that the code a human plays is covered end to end.
44. As a maintainer, I want that replay's `.txt` to put its end state next to `keyboard_slipway`'s state after the same fold, so that coverage of the old keyboard scenario is shown, not assumed.
45. As a maintainer, I want the replay to stay under 180 frames and contain no mouse field, so that it passes `tools/e2e_limits.sh` without an exception.
46. As a maintainer, I want only `pad_title`'s hash refreshed, one file at a time, with its end state checked again, so that the plan's pad-hash rule is followed exactly.
47. As a maintainer, I want `git diff --stat tests/e2e/` to show no mouse or `keyboard_*` `.sha256` changed, so that I know free mode cannot reach mouse or keyboard play.
48. As a maintainer, I want `make test` and `make e2e` no slower, so that the loop stays fast.
49. As a future implementer of step 2.2, I want free mode written as one of `pad_nav`'s modes, behind a single "which mode is active" decision, so that network mode can be added beside it.
50. As a future implementer of step 2.3, I want the × filter to sit in one place, so that "× on a planet creates a slipway from the starting planet" replaces "× on a planet does nothing" there.
51. As a future implementer of step 3.2, I want placement mode to reuse free mode's cursor and camera, so that it is free mode carrying a structure.

## Implementation Decisions

### Free mode in `pad_nav`

- **When it is active.** Free mode is active when a game is running (the main economy exists) and `pad_nav_panel_open` is false. It is derived each frame, not stored: in this step it is the only map mode. A read-only query (indicative name: `pad_nav_free_mode`) answers it, for the tests and for steps 2.2 and later. Step 2.2 turns the choice between free and network mode into stored state. The order of precedence is: panel open → panel navigation (step 1.4, unchanged), else game running → free mode, else nothing.
- **Cursor.** In free mode the synthetic cursor is the screen centre, `(P8_SCREEN_WIDTH / 2, P8_SCREEN_HEIGHT / 2)`, so `(80, 60)`, written every frame. The stored last cursor point becomes the centre too, so leaving free mode for a panel starts from the panel's first item, as in step 1.4. The cursor sprite is drawn by the existing ui overlay at the synthetic point, with the variants of the hovered entity. Nothing is added to the ui draw. The centre lies inside the ui's panel box (10,10)–(150,110), the area where a probe is not refused for being at the screen edge, and no HUD element covers it.
- **Camera pan.**
  - pad navigation moves the main camera itself, through a new operation of the camera module (indicative: `camera_pan`). It adds an integer pixel step to the camera's map position and to its position, and applies the bound below.
  - The pan runs before the cart's update, like everything in the input step. The cart's own camera update then adds its velocity (zero in pad mode) and its shake, and bounds again with the same bound.
  - Camera units are screen pixels for every layer of parallax 1 (planets, slipways, probes). The background star layer (parallax 0.5) scrolls at half speed, as it does with a mouse drag.
  - The direction convention is the cart's D-pad scroll: Right moves the camera right, so the world slides left.
  - With no camera (the placeholder before a game), there is no pan.
- **Speed curve.**
  - Each of the four directions has its own held-frames counter: four ints in the state, no pointer. A counter counts the frames its direction has been held in free mode, starting at 1 on the press frame. It is capped once past 40, and reset to 0 when the direction is released or when free mode is not active.
  - The speed for counter `n` is 1 px for `n ≤ 10`, 2 px for `11 ≤ n ≤ 40`, and 4 px for `n > 40`.
  - Horizontal step = Right speed − Left speed; vertical step = Down speed − Up speed. Opposite directions cancel, and a diagonal moves both axes.
  - Moving on the press frame itself is what "1 px/frame" means for a single-frame tap.
- **The camera bound in pad mode** (decision D2 below).
  - In pad mode, the camera is bounded so that the screen centre stays within the galaxy's planet area on both axes: from the galaxy margin to margin + span, in pixels (16 to 368). That gives camera x from −64 to 288 and camera y from −44 to 308.
  - Mouse mode keeps the cart's bound (the background grid fills the frame) byte for byte.
  - The bound is chosen from the installed input's mode: the input step sets the mode on the `p8_input_t` it installs (`played.mode`), and the camera's update reads it. That way no new global is needed.
  - The galaxy's margin and span become constants the camera module can read, instead of locals of the galaxy generation, with values unchanged.
- **× in free mode.**
  - Only a press edge of × that happens in free mode can put the left button down. On that edge, pad navigation hit-tests the cursor point with the camera as it stands after this frame's pan. It uses the classes' own `hit_test`, with a `mouse_pos_t` built the way the ui builds it (screen point, plus the world point = screen point + camera position).
  - If any hit-test entity with a `left_press`, other than the background, contains the point, the press is **swallowed**. The left button stays up until × is released and pressed again. Such an entity is a planet of any kind or state, structures included, or a HUD button or structure icon.
  - Otherwise the left button goes down, and it stays down every frame while × stays held, whatever the camera does meanwhile.
  - A small int flag in the state records "this × hold is engaged". It is cleared when × is released and when free mode stops being active.
  - The edge rule means the × still held from the title click places nothing: on the first in-game frame it is not an edge. The cart then sees a release with nothing held, which does nothing, as in step 1.4.
  - The filter is pad-mode behavior, not a game rule. Everything that happens after the button goes down is the existing dispatch: `background_left_press` makes the probe or a wormhole, the drag tracks the cursor, and the drop confirms or refuses.
  - A planet in the unknown state would let a mouse press through to the background. The resulting probe or fold would be refused anyway (a probe targeting a planet, or a fold within 20 px of one), so swallowing it loses nothing.
- **Other buttons.** In free mode, ○, □, △, L1, R1 and Start leave the played frame unchanged: no cursor move, no button, no pan. ○ will return to the starting planet once one exists (step 2.3). In this step none ever exists, so "○ does nothing while there is no starting planet" holds by construction.
- **Cheat.** The cheat still reads the raw pad mask after pad navigation. Its × presses also act in free mode (a probe or a fold may appear while it is typed), which §2 accepts.
- **Leaving and re-entering free mode.** When a panel opens, the direction counters and the × engagement are reset, and step 1.4's rules apply (× held means the left button is down). When the panel closes, free mode resumes with the cursor at the centre and all counters at 0.
- **No allocation, no float, no new data pointer.** The state grows by four counters and one flag. The camera module gains one function and one bound. The pointer-guard and alloc-guard counts do not change.

### Input step

- It sets the installed input's mode every frame, next to the other fields it re-points there. Mouse mode and `keyboard_cursor` mode get `P8_INPUT_MOUSE`, as their zeroed struct gives them today, so their played input is unchanged.
- Nothing else changes: pad navigation is still called in the same slot, before the cheat and the dispatch install.

### Files not touched

- `src/cmd/game.c`, `src/cmd/replay.c` and `src/platform/` are not edited. So, by §2 Invariants, "`make raylib` builds" is not part of this step's check. The game binary links the changed modules through the engine source list as before.
- The game's pad-mode startup notice ("not playable yet") stays word for word. A game still cannot be played to the end with the pad (no colonising until step 2.2).
- No README change: the controls section moves to the pad in step 4.2.

### Decisions settled without a human (guideline rules)

Each is recorded here, as the to-prd skill asks.

- **D1 — The replay's "starting wormhole" and "probe" are the same press.**
  - *Facts:* the code has exactly two kinds of background press. The first makes the probe (`probe_new`, tag probe, a fold anchor), which starts the game. Every later one makes a wormhole (`wormhole_new`), the space fold, which is confirmed after being held 15 ticks or more. `keyboard_slipway` does exactly this: a tap that "places the probe", then O held 16 frames for the fold.
  - *DECISION:* `pad_probe` taps × once on empty space to place the starting probe (the plan's "starting wormhole … launch a probe"), pans, then holds × for at least 16 frames on empty space and releases it to confirm the fold.
  - *WHY:* the plan's §2 rule "× on empty space launches a probe, or a wormhole when a probe already exists" matches `background_left_press` exactly. "Make the plan work" with the code as it is.
- **D2 — The pad camera is bounded to the galaxy's planet area, not left unbounded, and mouse mode keeps the cart's bound.**
  - *Facts:* §2 says "the camera is not clamped (scroll.c), so the whole map is reachable". In fact the camera module clamps the camera so that the 160×120 frame stays inside the background grid (camera x 0–206, y 0–286, shown by `camera_drag_dpad`). Planets lie between 16 and 368 px on both axes. With that clamp, the centre cursor covers only x 80–286 and y 60–346, so planets near the edges could never come under it. The mouse reaches them, because it can move to the edge of the screen.
  - *DECISION:* in pad mode, bound the camera so that the screen centre stays within 16–368 on both axes. Mouse mode keeps the cart's bound unchanged.
  - *WHY:* the plan's stated aim is "the whole map is reachable", and pad coverage must never drop below the mouse's (§1). Changing the cart's bound would change `camera_drag_dpad`'s mouse hash, which the invariants forbid. A bound on the galaxy instead of none keeps the player from drifting into empty space, and costs the same as no bound.
- **D3 — × on anything but empty space is swallowed in this step.**
  - *DECISION:* × puts the left button down only when nothing with a `left_press` other than the background lies under the centre at the press edge.
  - *WHY:* without it, × on a known planet would open its menu and × on a settled one would start a slipway. That is △'s job (step 2.2) and step 2.3's starting-planet rule. §2 says "If there is no starting planet … × does nothing". "Question why the change is asked": map behavior beyond probes and folds is a later step's.
- **D4 — No auto-repeat except the camera; × is a held button.**
  - *DECISION:* the D-pad pans every frame while held. × acts on its press edge and then stays a held left button until released.
  - *WHY:* §2 "No auto-repeat", with its exception for free mode, and "a held × is still a held left button … for example the space fold".
- **D5 — One counter per direction.**
  - *DECISION:* each direction counts its own held frames. Two directions pressed together follow the same curve, and a direction added later starts at 1 px.
  - *WHY:* it is the smaller and more predictable of the two working options, and it mirrors `button_cursor`'s per-direction counters, which the plan replaces.
- **D6 — Free mode is derived, not stored, in this step.**
  - *DECISION:* no mode enum yet. "Game running and no panel open" is free mode.
  - *WHY:* the only other map mode, network mode, is step 2.2's. "Between two options that both work, take the smaller change."
- **D7 — The pad replays this step changes, stated up front: `pad_title` only** (H-1.4-2).
  - *DECISION:* the implementation refreshes `pad_title.json.sha256` alone, by copying its current capture hash from `make e2e`'s output (never `make e2e-baseline` over the directory). It reruns `make state EVENTS=tests/e2e/pad_title.json` and confirms that the end-state lines of `pad_title.txt` still hold (H-1.4-3). If `pad_title.txt` describes the in-game frames (the cursor left on the row), that description is updated to the centre cursor.
  - *WHY:* §2 Invariants, "Refreshing pad hashes is allowed, and only pad hashes".
- **D8 — `pad_probe`'s galaxy need not equal `keyboard_slipway`'s.**
  - *Facts:* the galaxy depends on the frame the game starts. `keyboard_slipway` clicks the title at f1, and a pad title needs at least a Down then × after the rows appear.
  - *DECISION:* the replay picks "reasonable" (the difficulty `keyboard_slipway` plays, $100 at start) by D-pad and ×, as early as the title allows. The pairing in its `.txt` compares the decisive lines: credits before and after the fold, clock, colonies 0, slipways 0, and the number of planets the fold revealed. Planet ids are not compared, and the note says the galaxies differ and why.
  - *WHY:* matching the galaxy frame for frame is detail the step does not need ("don't get drawn into detail"). The pairing still shows the same scenario with the same cost and clock effect.
- **D9 — The speed curve and the reach are proven by unit tests, not by the replay.**
  - *DECISION:* the replay pans only as far as its fold needs. The pixel-exact curve, the bound and the reach of every planet are checked in `test_pad_nav`.
  - *WHY:* a replay hash proves nothing by itself (§2), and exact pixel counts are cheaper and clearer to assert in a unit test. That also keeps the replay short, so `make e2e` stays fast.
- **D10 — Two existing tests are updated, not deleted.**
  - `test_pad_nav`'s "no panel open leaves the cursor still" froze step 1.4's placeholder. It becomes the free-mode cursor test: in a game with no panel, the cursor is the centre and only × on empty space presses.
  - `test_input_step`'s quiet-against-noisy headless pad run gave the noisy run held pad directions and the quiet run none. In free mode those directions now pan the camera, correctly. The quiet run is fed the same pad mask as the noisy one, so the test still proves that mouse and `dpad` noise change nothing. "The D-pad never reaches the cart's scroll" moves to the new free-mode test: the camera's velocity stays zero and its displacement equals the curve exactly.
  - *WHY:* CLAUDE.md, "update a unit test that froze the old behavior". "No test is skipped or weakened": both intents stay checked.

## Implementation Chunks

1. **Centre cursor, camera pan, pad bound.**
   - Free-mode query, centre cursor, direction counters, speed curve, `camera_pan` and the pad-mode bound chosen from the installed input's mode. The input step sets that mode. The galaxy's margin and span become shared constants.
   - In this chunk × in free mode still emits nothing.
   - Rewrite `test_pad_nav`'s "no panel open" test, and update the quiet-against-noisy test in `test_input_step` (D10).
   - New tests: the curve pixel for pixel, cancel and diagonal, reset on release, zero velocity, the bound, every planet reachable, panel precedence and return to the centre, the other buttons doing nothing.
   - Refresh `pad_title.json.sha256` on its own, rerun `make state` on it, and check or update `pad_title.txt` (D7). Check that no mouse or `keyboard_*` hash moved.
2. **× in free mode.**
   - Press-edge engagement, the hit-test filter (D3), the held left button, and the reset on leaving free mode.
   - Tests: the first × on empty space places a probe and you stay in free mode; a second × hold starts a wormhole held by the ui; a short tap abandons it with nothing paid; × on a planet emits no press and creates nothing; × held across the title click places nothing; the D-pad pans while × is held.
   - `pad_title` must not change again in this chunk, because it presses nothing in game (H-1.4-3). If it does, find out why before going further.
3. **The `pad_probe` replay.**
   - Write `tests/e2e/pad_probe.json` (`"input": "pad"`, no mouse field, under 180 frames).
   - Take its hash for that file only.
   - Run `make state EVENTS=tests/e2e/pad_probe.json`, read the framebuffer dump at the key frames, and write `pad_probe.txt` with the pairing (D8).

## Testing Decisions

- **What a good test is here:** it drives the public interface and checks what a player sees, never internal fields:
  - pad masks into the input step, then the played frame out: cursor point, left button;
  - the camera's position and velocity after the cart's update;
  - the entities that exist (probe, wormhole) and what the ui holds;
  - credits and clock.
- **`test_pad_nav`** (existing binary, more tests, each in a fresh forked child like the existing ones). A game is started directly (`cart_start_game`) or through the title with pad presses, then frames are run through the input step with `cart_update60()` between them.
  - Free mode is active right after the start, and the played cursor is (80, 60) with the left button up.
  - Right held 50 frames moves the camera's map x by exactly 10×1 + 30×2 + 10×4 = 110, and its velocity stays zero all along. The same holds for each direction, using a start position the bound does not cut.
  - Up+Left together move both axes. Left+Right together move nothing. A release and a new press start at 1 px again.
  - Held long enough, each direction stops at the pad bound. For every planet of the started galaxy, its position lies inside the area the centre can reach under that bound.
  - ○, □, △, L1, R1 and Start in free mode change neither the played frame nor the camera, and create nothing.
  - A panel opened during the game (for example the tech panel, opened the way the mouse game opens it) takes the cursor to its first item, and a held direction does not pan. Once it closes, the cursor is back at the centre.
  - *(Chunk 2)* The first × tap on empty space creates one probe. Free mode is still active after it.
  - *(Chunk 2)* A second × press creates a wormhole that the ui holds as its left-held entity. Released before 15 frames, it dies and the credits do not move.
  - *(Chunk 2)* A planet put under the centre (the test sets the camera through the pan operation) swallows ×: no left press, no new entity, no panel.
  - *(Chunk 2)* × held from the title click into the game creates nothing and leaves nothing held.
  - *(Chunk 2)* With × engaged, a held direction pans the camera while the left button stays down.
- **`test_input_step`:** the quiet-against-noisy headless run is updated (D10). The live-against-recorded test must pass unchanged: pad navigation is still deterministic given the world and the pad masks.
- **E2E — `pad_probe.json`,** about 80–110 frames (indicative script; the exact frames come from this galaxy's dump):
  - title: idle while the rows slide in, Down to "reasonable", × (the game starts), release;
  - one ○ tap in free mode: nothing changes;
  - × tap at the centre: the starting probe. Pan first if the dump shows a planet within 20 px of the centre;
  - Left and Up held to put the centre at a fold point within the anchor's reach and more than 20 px from every planet. `keyboard_slipway`'s offset from the probe, (−18, −12), takes 14 frames of Left and 11 of Up with the curve;
  - × held at least 16 frames, then released: the fold is confirmed;
  - idle frames until the revealed planets turn known.

  Its `.txt` follows the `keyboard_*` notes. It gives the frame-by-frame script and what the dump shows at the key frames: the cursor sprite at the centre, the probe, the fold's disk, the planets revealed. Then come the `make state` end-state lines (credits, clock, colonies 0, slipways 0, techs, revealed planets). Beside them sits `keyboard_slipway`'s state after its fold ($100 → $97, month 2, two planets known), with the note that the galaxies differ (D8). The note also says it covers the probe and fold part of `keyboard_slipway`, and that the colonies and the slipway are covered by `pad_colonise` (step 2.2) and `pad_slipway` (step 2.3).
- **Unchanged:** the 26 mouse and 4 `keyboard_*` `*.sha256`. `git diff --stat tests/e2e/` shows only `pad_title.json.sha256`, possibly `pad_title.txt`, and the three new `pad_probe` files.
- **Speed:** the new unit tests use the existing patterns: no draw, at most a few hundred `cart_update60()` calls each, forked children. One more replay of about 100 frames is within the e2e budget. `make test` and `make e2e` must not get measurably slower.
- **Prior art:**
  - `tests/test_pad_nav.c` (step 1.4): pad frames through the input step, title, forked children;
  - `tests/test_input_step.c`: the headless pad run and its camera snapshot;
  - `tests/test_button_cursor.c`: per-direction held-frame speed tests;
  - `tests/e2e/keyboard_slipway.txt` and `tests/e2e/space_fold_same_spot.txt`: notes on the probe and the fold;
  - `tests/e2e/camera_drag_dpad.txt`: the cart's camera limits, which stay as they are in mouse mode.

## Out of Scope

- Network mode, its visuals (hidden cursor, blinking circle, the planet's hover info) and △'s planet menu (step 2.2).
- □ entering free mode from a planet, × on a planet creating a slipway, ○ returning to the starting planet, Left/Right/Up/Down on slipways (step 2.3).
- Tabs, L1/R1, Start, pause, modality, the build tab and placement mode (phase 3). Placement will reuse this step's cursor and camera.
- Changing the cart's camera bound in mouse mode, or any mouse or `keyboard_*` behavior.
- Dropping the pad-mode startup notice (once a pad game can be played, step 2.2 at the earliest).
- Fixing any panel that failed step 1.4's reachability tests (H-1.4-5).
- README and `docs/plan-10.md` (step 4.2).
- Audio.

## Further Notes

- **The step note for 2.1** must record:
  - the pad hashes that changed and why: expected `pad_title`, because its in-game frames now show the centre cursor (D7). If H-1.4-2 turned out false, say so;
  - that `make state` on `pad_title` still gives its recorded end-state lines;
  - `pad_probe`'s end state next to `keyboard_slipway`'s state after the fold, and that it covers `keyboard_slipway`'s probe and fold part;
  - decision D2 (the pad-mode camera bound differs from the plan's "not clamped" wording, and why);
  - any H-1.4-k whose behavior turned out false, with the fallback applied;
  - that every mouse and `keyboard_*` `*.sha256` is unchanged.
- Step 2.2 inherits this module. Network mode becomes a second map mode beside free mode, and D6's derived rule becomes stored state ("□ enters free mode", "○ returns to the starting planet"). The × filter (D3) is the single place where step 2.3's "× on a planet creates a slipway" goes.
- Every later pad replay starts its game in free mode. Steps after this one that change free mode's look (for example network visuals) will refresh `pad_probe` under the same §2 rule.
