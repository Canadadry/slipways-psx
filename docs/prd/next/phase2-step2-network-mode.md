---
title: "Planet menu and network mode: △ opens a planet's menu, and colonising lands the pad selection on it"
description: "In free mode a pad player can probe and fold, but cannot open a planet's menu, so no planet can be colonised and the map has no notion of a selected planet."
status: needs-triage
---

## Hypotheses on step 2.1

Step 2.1 (free mode, `docs/prd/triage/phase2-step1-free-mode.md`) is being implemented while this PRD is written. This PRD reads only committed code: `pad_nav` at `HEAD` still has no free mode. Every assumption about 2.1's outcome is listed here.

**Rule for the implementer:** if step 2.1 landed with a different name or spelling, use what landed. Only a hypothesis whose *behavior* turns out false needs a line in this step's note. When that happens, its fallback applies.

- **H-2.1-1 Free-mode query.** Free mode is derived each frame, not stored: "a game is running and `pad_nav_panel_open()` is false". A read-only query exists (indicative name `pad_nav_free_mode`). The precedence is: panel open → step 1.4's panel navigation, else game running → free mode, else nothing. *This step* turns the map mode into stored state (decision N1). *Fallback:* if 2.1 already stored a mode, this step adds the network value to it instead of introducing it.
- **H-2.1-2 How × is engaged.** In free mode, only a press edge of × can put the left button down. On that edge pad navigation hit-tests the centre point with the camera as it stands after this frame's pan, using the classes' own `hit_test` and a `mouse_pos_t` built the way the ui builds it. If any entity with a `left_press`, other than the background, contains the point, the press is swallowed. Otherwise the left button stays down while × is held. A small int flag records the engaged hold, and it is cleared when × is released and when free mode stops being active. The hit-test helper is a private function of `pad_nav` that this step can reuse to find the planet under the centre. *Fallback:* if no reusable helper landed, this step writes one in `pad_nav`, built the same way, and 2.1's × filter is moved onto it in the same chunk.
- **H-2.1-3 `camera_pan`.** The camera module has an operation (indicative `camera_pan`) that adds an integer pixel step to the camera's map position and position, and bounds it. Pad navigation calls it in free mode only. This step does not move the camera at all (decision N9), so it does not call `camera_pan`. *Fallback:* none needed. If it is missing, this step does not add it.
- **H-2.1-4 `pad_probe` content.** `tests/e2e/pad_probe.json` lands as 2.1's PRD describes, in about 80–110 frames. It picks "reasonable" with Down and × on the title, taps ○ once in free mode (nothing happens), taps × for the starting probe, pans with Left and Up, holds × for at least 16 frames to confirm a fold, then idles until the revealed planets turn known. It never presses △. Its `.txt` lists the planets the fold revealed, with their positions and types. *Fallback:* if `pad_probe` reveals no known planet that the camera bound lets the pad bring under the centre, `pad_colonise` scripts its own fold after the shared title prefix (decision N12) and reads its planets from its own `make state` run.
- **H-2.1-5 The pad-mode camera bound.** In pad mode the camera is bounded so that the screen centre stays within the galaxy's planet area (16 to 368 on both axes: camera x −64 to 288, y −44 to 308). Mouse mode keeps the cart's bound. So every planet can be brought under the centre cursor. *Fallback:* if 2.1 landed with the cart's bound in pad mode, `pad_colonise` picks a known planet the bound lets the pad reach, and the step note says so.
- **H-2.1-6 Free mode's leaving and returning.** When a panel opens, 2.1 resets the direction counters and the × engagement. Panel navigation then applies (× held means the left button is down). When the panel closes, free mode resumes with the cursor at the centre `(80, 60)`. *Fallback:* none needed. This step replaces "the panel closes → back to the centre" with "back to the stored map mode" (decision N1).
- **H-2.1-7 Frozen tests.** `test_pad_nav` has a 2.1 test asserting that ○, □, △, L1, R1 and Start do nothing in free mode. This step updates it (decision N14): △ is removed from that list, and ○ stays in it because free mode still has no starting planet. *Fallback:* if the buttons are split over several tests, update each that names △.
- **H-2.1-8 The earlier pad hashes.** After 2.1, `pad_title` ends in free mode with nothing pressed in game. `pad_probe` presses no △, and it presses ○ only in free mode with no box open. *Consequence:* **this step expects no earlier `pad_*` replay to change** (decision N13). *Fallback:* if `pad_probe` or `pad_title` presses △, or ○ while a box is open, its hash may change. In that case, the implementation refreshes that one file under §2's rule, reruns `make state` on it, and the step note says why.

## Problem Statement

After step 2.1 a pad player can start a game, pan the camera, place the starting probe and fold space. That is still not a game:

- the planet a fold reveals cannot be colonised, because × on a planet does nothing in free mode, and the colonise menu is opened only by a left click on that planet;
- the map has no "selected planet". The plan's network mode, where the D-pad travels along the player's slipways, has nowhere to start from;
- no panel opened during a game can be closed with ○, as §2 promises for every panel.

The mouse game reaches all of this with a click on a planet. The pad has no equivalent yet.

## Solution

- **△ in free mode opens the planet's menu.** When the centre cursor lies on a known planet, △ is a left click on it: its colonise / project menu opens. On a colonised planet or a structure, △ opens the planet's information panel, as a click and release on it does with the mouse. On empty space, on an unexplored planet or on anything else, △ does nothing.
- **You land on that planet in network mode.** From the △ on, the selection is that planet. While the menu is open, generic panel navigation runs inside it: the D-pad moves between its rows, the focused row shows its tooltip as on hover, × clicks it. Picking the colonise row colonises the planet and closes the menu. The player is then on the new colony in network mode.
- **○ closes the menu.** ○ closes the planet's menu without choosing anything. The player stays on that planet in network mode.
- **Network-mode visuals.**
  - The cursor sprite is hidden.
  - The selected planet is circled by a blinking white circle.
  - The ui hovers the selected planet exactly as if the mouse were on it, so whatever mouse hover shows for a planet shows for the selection.
- **△ in network mode** opens the selected planet's menu again, through the same click.
- **The rest of network mode comes in step 2.3.**
  - × does nothing in network mode, as §2 says, and ○ does nothing there with no menu open.
  - The D-pad does nothing in this step. Left/Right slipway cycling and Up/Down travel are step 2.3's.
  - □ does nothing yet. It enters free mode in step 2.3, so in this step a player who has landed on a planet stays in network mode.
- **Proof:** a new pad replay, `tests/e2e/pad_colonise.json`, starts like `pad_probe` (probe, fold), brings a revealed planet under the centre, opens its menu with △, moves inside it with the D-pad, colonises it with ×, and idles in network mode long enough to show the circle blinking. Its `.txt` records `make state`'s end state next to `keyboard_slipway`'s state after its first colony.

## User Stories

1. As a pad player, I want △ on a known planet under the centre cursor to open its colonise menu, so that I can colonise without a mouse.
2. As a pad player, I want △ on a colonised planet to open its information panel, so that I can read its needs, imports and exports as a mouse player does by clicking it.
3. As a pad player, I want △ on a structure to open its information panel, so that structures behave like planets, which they are.
4. As a pad player, I want △ on empty space to do nothing, so that it never launches a probe or a fold by accident.
5. As a pad player, I want △ on an unexplored planet to do nothing, so that it never falls through to the background and places a probe.
6. As a pad player, I want △ to act on its press only, so that holding it does not reopen the menu every frame.
7. As a pad player, I want △ ignored while I am holding × on a fold, so that one button never interrupts the other's gesture.
8. As a pad player, I want the planet's menu to take the D-pad and × for its own rows, so that I navigate it like any other panel.
9. As a pad player, I want the focused row of the menu to show its tooltip and its cost, so that I know what a row does before I pick it.
10. As a pad player, I want the first row focused when the menu opens, so that the usual choice is one press away.
11. As a pad player, I want × on the colonise row to colonise the planet and pay its cost, exactly as a mouse click does, so that the game rules stay the same.
12. As a pad player, I want × on a row I cannot afford to behave as a mouse click on it does, so that the pad is neither stricter nor looser than the mouse.
13. As a pad player, I want ○ to close the menu without choosing anything, so that I can back out of a menu I opened by mistake.
14. As a pad player, I want to land on the planet in network mode once its menu closes, so that the colony I just founded becomes my point on the map.
15. As a pad player, I want the cursor sprite hidden in network mode, so that I am not shown a pointer I cannot steer.
16. As a pad player, I want a blinking circle around the selected planet, so that I always see which planet I am on.
17. As a pad player, I want the circle visible on the frame I land, so that the selection shows at once.
18. As a pad player, I want the circle drawn with the planet, under the HUD and under any panel, so that it never covers a menu.
19. As a pad player, I want the circle hidden while a menu is open, so that the open panel is what I look at.
20. As a pad player, I want whatever mouse hover shows for a planet to show for the selected planet, so that I lose no information by playing with the pad.
21. As a pad player, I want △ in network mode to open the selected planet's menu, so that I can reread a colony's panel at any time.
22. As a pad player, I want × to do nothing in network mode, so that a confirm press never starts a slipway or a probe by surprise.
23. As a pad player, I want the × still held from the colonise click to do nothing once the menu has closed, so that the click never leaks onto the map.
24. As a pad player, I want ○ to do nothing in network mode with no menu open, so that back never throws me somewhere unexpected.
25. As a pad player, I want the D-pad to leave the camera still in network mode, so that the selected planet stays where I see it until travel is defined.
26. As a pad player, I want □, L1, R1 and Start to do nothing in network mode for now, so that no half-built behavior surprises me before the steps that define them.
27. As a pad player, I want to be put back in free mode, centre cursor, if my selected planet ceases to exist (for example a project replaced it), so that I never stay selected on nothing.
28. As a pad player, I want every new game to start in free mode with no selection, so that a previous game's planet is never carried over.
29. As a pad player, I want ○ to keep doing nothing in free mode, since there is still no starting planet to return to, so that step 2.1's rule holds.
30. As a pad player, I want ○ to do nothing on the title screen, so that I cannot close the difficulty choice and get stuck.
31. As a pad player, I want a panel that opens on its own during a game (the year box is not one, the final score panel is) to keep precedence over network mode, and to give network mode back when it closes, so that panels and the map never fight.
32. As a pad player, I want the pad cheat (× × ○ ○ × × ○ ○) to keep working, accepting that its ○ presses also close an open menu, so that step 1.2's behavior survives, with the side effects §2 accepts.
33. As a mouse player, I want mouse mode exactly as before, so that my controls and the 26 mouse replays are untouched.
34. As a keyboard-cursor replay, I want `button_cursor` to run exactly as before, so that the 4 `keyboard_*` hashes stay unchanged.
35. As a maintainer, I want △ to go through the existing dispatch as a synthetic left click, so that the menu that opens is the one the game opens for a mouse click, with no rule copied.
36. As a maintainer, I want the map mode and the selection to be plain ints and an entity id in the existing pad navigation state, so that the module still allocates nothing and the pointer-guard count does not change.
37. As a maintainer, I want the selection held as an `entity_id_t`, so that a planet that dies is noticed instead of being read through a stale pointer.
38. As a maintainer, I want one place that tells the drawing code what pad navigation selected, so that the hidden cursor and the circle come from the same fact.
39. As a maintainer, I want the circle and the hidden cursor to appear only in pad mode with a selection, so that no mouse or `keyboard_*` frame changes.
40. As a maintainer, I want tests proving that △ opens the same menu a mouse click opens, on a known planet and on a colonised one, so that the synthetic click is shown to be faithful.
41. As a maintainer, I want a test proving △ on empty space and on an unexplored planet creates nothing, so that the planet filter holds.
42. As a maintainer, I want a test proving colonising through the menu lands network mode on that planet, with the ui hovering it, the cursor hidden and the circle blinking, so that the step's three claims are each checked.
43. As a maintainer, I want a test proving ○ closes the menu during a game and does nothing on the title, so that the close rule is bounded.
44. As a maintainer, I want the 2.1 test that froze "△ does nothing in free mode" updated and not deleted, so that its remaining buttons are still checked.
45. As a maintainer, I want a pad replay that colonises a planet through △ and the menu, so that the code a human plays is covered end to end.
46. As a maintainer, I want that replay's `.txt` to put its end state next to `keyboard_slipway`'s state after its first colony, so that coverage of the old keyboard scenario is shown, not assumed.
47. As a maintainer, I want the replay under 180 frames and with no mouse field, so that it passes `tools/e2e_limits.sh` without an exception and `make e2e` stays fast.
48. As a maintainer, I want the earlier pad hashes stated up front (none expected to change), so that any unexpected change is investigated, not refreshed.
49. As a maintainer, I want `git diff --stat tests/e2e/` to show no mouse or `keyboard_*` `.sha256` changed, so that I know network mode cannot reach mouse or keyboard play.
50. As a future implementer of step 2.3, I want network mode's selection stored in one field, so that Left/Right, Up/Down, □ and ○ can read and move it.
51. As a future implementer of step 2.3, I want the synthetic cursor of network mode already on the selected planet, so that "the camera follows the selection" only has to move the camera.
52. As a future implementer of step 3.3, I want ○'s close rule to leave the pause menu alone, so that pause gets its own close rule with Start.

## Implementation Decisions

### Map mode becomes stored state in `pad_nav`

- **N1 — A stored map mode.** `pad_nav`'s state gains a map mode (free or network), a selection (`entity_id_t`, the selected planet) and a few ints (the blink counter, the pending △ release, and the economy the state belongs to). No pointer, no allocation. The precedence becomes: panel open → panel navigation (step 1.4, unchanged); else game running → the stored map mode; else nothing. The free-mode query of 2.1 (H-2.1-1) now answers "game running, no panel open, map mode is free". A second read-only query (indicative `pad_nav_selection`) returns the selected planet in network mode, or NULL.
- **Reset rules.** The state goes back to free mode with no selection:
  - when no game is running (the title);
  - when the running economy is not the one the state recorded, which means a new game has started;
  - when the selected entity no longer exists or is dead. The case arises when a project such as "collapse" or "harvest" replaces the planet with a new entity. The player is then in free mode, cursor at the centre, camera where it is. Following the replacement entity is left out: it is a rare, tech-gated case, and free mode always reaches it again.

### △ — the planet's menu

- **N2 — △ is a synthetic left click on the planet.**
  - On a press edge of △, in free mode or network mode, pad navigation looks for the target planet.
    - In free mode, the target is a planet-kind entity whose own `hit_test` contains the centre point, found with 2.1's helper (H-2.1-2).
    - In network mode, the target is the selected planet.
  - The target counts only if its state is known or settled, the two states whose `left_press` stops the dispatch. A planet in any other state would let the press fall through to the background, which would place a probe or a wormhole, so △ does nothing on it.
  - When the target counts, the played frame's left button is down on this frame and up on the next one, whatever △ does meanwhile. The cursor point is the one the mode writes: the centre in free mode, the planet's screen point in network mode (N5).
  - Everything after that is the existing dispatch:
    - a known planet's `left_press` opens its projects panel (the colonise menu) on the press frame;
    - a settled planet's `left_press` hands the ui a slipway, and the release on the same planet drops it on its own start, which opens the planet's inspection panel. That is exactly the mouse's click on a colony (`colony_panel`).
  - The settled case needs the release frame to open its panel. That is why △ is a click and not a hold.
- **N3 — △ lands network mode at once.** On the △ edge that emits the click, the map mode becomes network and the selection becomes the target. This follows §2 literally: "△ on a planet opens its menu, and you land on that planet in network mode".
  - The menu then opens, and panel precedence takes over until it closes.
  - When it closes, by a row clicked (colonise, a project) or by ○, the player is in network mode on that planet, whether or not it was colonised.
  - "Colonising lands the selection on that planet" (the step's wording) is the colonise path of this one rule.
- **△ is ignored** while × is engaged in free mode (a probe or a fold is being held, H-2.1-2), and while a panel is open (step 1.4's rule).

### ○ — closing the menu

- **N4 — ○ closes an open box during a game.**
  - On a press edge of ○, when a game is running, the ui holds a box (its ephemerals list is not empty) and the pause menu is not up, pad navigation empties the ui's ephemerals through the ui's own close operation (`ui_clear_ephemerals`). That is the operation a mouse click outside a box ends in.
  - Nothing else is pressed, and the map mode is left as it is: network mode on the planet after a planet menu.
  - Rules out:
    - ○ on the title (no game running), so the difficulty box cannot be closed;
    - ○ on the pause menu: it keeps its open flag outside the ui, and its close is step 3.3's;
    - ○ on the score panel: it is a modal backdrop, not a box, and is phase 3's.
  - In this step the only box a pad player can open during a game is a planet's menu. The tech panel and pause are reached through tabs and Start, in phase 3. The rule is still written generically, as §2 asks.

### Network mode, this step's part

- **N5 — The cursor sits on the selected planet.**
  - In network mode with no panel open, the synthetic cursor is the selected planet's screen point: its position minus the camera's position, floored, the same world-to-screen rule as step 1.4's world-space panel items.
  - The ui then hovers it as it would hover it under the mouse, so the planet's hover state (`is_hovered`, the hovered entity) is the selection's.
  - At `HEAD`, a hovered planet draws nothing of its own: no planet code reads `is_hovered`, and planets carry no cursor-sprite variant. "The planet's hover info" is therefore met by construction, with nothing new drawn. If a later step gives planets a hover display, network mode shows it for free.
- **N6 — Buttons in network mode.**
  - × leaves the left button up. That includes the × still held from the colonise click, which reaches network mode as a release with nothing held, so nothing happens.
  - ○ does nothing with no box open.
  - □, L1, R1, Start and the four directions leave the played frame and the camera unchanged.
  - Network mode never pans. Directions, □ and ○'s return are step 2.3's.

### Network visuals

- **N7 — One fact for the drawing: the ui's pad selection.**
  - The ui gains a pad selection, an `entity_id_t` initialised to none in `ui_new`, and a blink frame count, set through one setter (indicative `ui_set_pad_selection`). Pad navigation writes them every pad-mode frame:
    - the selected planet and the frames since it was last shown, in network mode with no panel open;
    - none otherwise, including while the planet's menu is open.
  - Mouse and `keyboard_cursor` modes never call the setter, so their ui stays at none and every one of their frames is unchanged.
  - The ui is rebuilt with each game, and pad navigation rewrites the selection every frame, so nothing goes stale.
- **N8 — Hidden cursor, blinking circle.**
  - `ui_draw_overlay` draws no cursor sprite while the ui's pad selection is set.
  - The planet draw draws a circle around the planet when it is the ui's pad selection and the blink is in its visible phase. The circle is drawn after the planet's sprite and ring, at the shaken position the planet itself uses, so it follows bob and shake. Being drawn with the planet, it sits under the HUD and under any panel.
  - Circle: radius 9 (outside the colony ring at 6 and the black disc at 7), colour 7.
  - Blink: visible for frames 0–15 of each 32-frame period, counted from the frame the selection was last shown, so the circle is visible on the landing frame and again each time a menu closes.
  - The frame count lives in pad navigation, which is deterministic, and not in a clock, so a replay gives the same blink every run.
- **N9 — No camera move in this step.** The selected planet is always under the centre cursor (within its hit box) when △ selects it, so it is on screen. "The camera follows the selection" is about Up/Down travel and is step 2.3's.

### Input step, files not touched

- The input step is unchanged: pad navigation still runs in the same slot, before the cheat and the dispatch install.
- `src/cmd/game.c`, `src/cmd/replay.c` and `src/platform/` are not edited, so "`make raylib` builds" is not part of this step's check (§2 Invariants).
- The pad-mode startup notice ("not playable yet") stays word for word. A pad game still cannot build a slipway, which comes in step 2.3.
- No README change: the controls section moves to the pad in step 4.2.

### Decisions settled without a human (guideline rules)

Each is recorded here, as the to-prd skill asks. No `guideline` agent could be spawned in this run, so each was settled by the `guideline` skill's rules directly, in its DECISION / WHY form.

- **N1 — Stored map mode.** DECISION: replace 2.1's derived free mode with a stored free/network mode plus a selection id. WHY: §2 network mode needs a selected planet that outlives panels; 2.1's PRD (Further Notes) hands this to 2.2.
- **N2 — △ on known and settled planets only, as a press-then-release click.** DECISION: △ emits a one-frame press and a release on the next frame, only when the target is known or settled. WHY: `planet_left_press` stops the dispatch only in those two states; any other press falls through to `background_left_press` (a probe). The settled case opens its panel only on the release (`planet_accept_drop`).
- **N3 — Network mode from the △ on, colonised or not.** DECISION: the selection becomes the planet when △ opens its menu, and stays there however the menu closes. WHY: §2 free mode, "△ on a planet opens its menu, and you land on that planet in network mode". The step's "colonising lands the selection" is one path of it. A player who backs out is left with △ only until step 2.3 brings □. That is accepted, because □ belongs to 2.3 ("question why the change is asked").
- **N4 — ○ closes boxes during a game, through the ui's close operation, not pause.** DECISION: ○ empties the ui's ephemerals when a game is running, a box is open and pause is not up. WHY: step 1.4's PRD assigned "○ closes a panel" to 2.2 for the planet menu. A synthetic click outside the box needs a screen point that hovers no box item, and no such point is guaranteed: a planet menu opens at the planet's own position. Calling the ui's own close operation copies no game rule. Pause is excluded because its open flag lives in `menu.c` and its close is step 3.3's.
- **N5 — Hover info by construction.** DECISION: put the synthetic cursor on the selected planet and draw nothing new for "hover info". WHY: at `HEAD` no planet code reads `is_hovered`, so a hovered planet shows nothing beyond the cursor, which §2 hides. "Between two options that both work, take the smaller change."
- **N7/N8 — Drawn by the planet, keyed by the ui.** DECISION: the ui holds the pad selection, the planet draw draws the circle, and the ui overlay skips the cursor. WHY: drawing the circle in the ui overlay would put it over the HUD and over open panels. Keeping one fact in the ui keeps the hidden cursor and the circle consistent.
- **N10 — Replaced or dead selection → free mode.** DECISION: fall back to free mode instead of following the replacement entity. WHY: smaller change. Collapse and harvest are tech-gated, and free mode reaches the new entity again.
- **N11 — △ in network mode belongs to this step.** DECISION: implement it here. WHY: §2 network mode lists △, and step 2.3's text does not name it. "Planet menu and network mode" is this step's title.
- **N12 — `pad_colonise`'s script.** DECISION: copy `pad_probe`'s frames verbatim up to and including the × that starts the game, so the galaxy is the same and `pad_probe.txt`'s planet list applies (H-2.1-4). After that, script freely and keep the file under 180 frames. One colony only: a second needs □ (step 2.3), and `keyboard_slipway`'s second colony and slipway are `pad_slipway`'s. WHY: §2, "a new pad replay proves what it claims", with no long-replay exception, so `make e2e` stays fast.
- **N13 — Earlier pad replays expected to change: none** (H-2.1-8). DECISION: neither `pad_title` nor `pad_probe` is refreshed. If either hash moves, find out why before going further. Refresh it only if the cause is this step's own behavior (a △ press, or a ○ press with a box open), one file at a time, with `make state` rerun and the step note saying why. WHY: §2 Invariants, "Each such step's check states up front which earlier `pad_*` replays it expects to change".
- **N14 — Frozen tests updated, not deleted.** DECISION: 2.1's "other buttons do nothing in free mode" test drops △ and keeps ○, □, L1, R1 and Start. A 1.4 test that froze "○ is ignored in a panel" keeps checking it on the title, where ○ is still ignored. WHY: CLAUDE.md, "update that test"; guideline, "no test is skipped or weakened".

## Implementation Chunks

1. **Stored map mode, △, ○, network mode's input.**
   - The map mode, the selection, the reset rules (N1), △ in free and network mode (N2, N3, N11), ○'s close (N4), the network cursor on the planet and the buttons that do nothing there (N5, N6), and the fallback on a dead selection (N10).
   - The selection query.
   - Update the frozen 2.1 test (N14).
   - New tests listed under Testing Decisions, chunk 1.
   - Run `make e2e` and confirm that no `*.sha256` moved (N13).
2. **Network visuals.**
   - The ui's pad selection and its setter (N7), the hidden cursor in the overlay and the blinking circle in the planet draw (N8). Pad navigation writes the selection each frame.
   - Tests listed under chunk 2.
   - Run `make e2e` again: still no hash moved, because neither earlier pad replay ever reaches network mode.
3. **The `pad_colonise` replay.**
   - Write `tests/e2e/pad_colonise.json` (`"input": "pad"`, no mouse field, under 180 frames), take its hash for that file only, run `make state EVENTS=tests/e2e/pad_colonise.json`, read the framebuffer dump at the key frames, and write `pad_colonise.txt` with the pairing.

## Testing Decisions

- **What a good test is here:** it drives the public interface and checks what a player sees, never `pad_nav`'s internal fields:
  - pad masks into the input step, then the played frame out (cursor point, left button);
  - the ui's hovered entity, its open box and its pad selection;
  - the planets' states, the entities that exist, and credits and clock;
  - for the visuals, the framebuffer after a draw.
- **`test_pad_nav`** (existing binary, more tests, each in a forked child like the others). A game is started directly, a planet is made known or settled the way the existing planet-menu test does, and the camera is put so that the planet lies under the centre. Then frames run through the input step, with `cart_update60()` between them.
  - *Chunk 1:*
    - △ on a known planet under the centre opens the same projects panel a mouse click opens: same row count, first row focused on the next frame. The selection query returns that planet.
    - △ on empty space, and on a planet that is neither known nor settled, creates nothing: no probe, no wormhole, no box. The mode stays free, with the centre cursor.
    - △ on a settled planet opens its inspection panel within two frames. No slipway is left, credits are unchanged, and the mode is network on that planet.
    - Colonising through the menu: Down/Up move the focus between rows, then × on the colonise row settles the planet and charges its cost. On the next frame no box is open, the mode is network on that planet, the played cursor is the planet's screen point, the ui hovers the planet, and the left button stays up although × is still held.
    - ○ with the menu open closes it. The planet stays known, the mode is network on it, and nothing is charged.
    - In network mode, × and every direction emit no press, and the camera does not move. □, L1, R1 and Start change nothing.
    - △ in network mode reopens the menu.
    - ○ on the title does nothing: the difficulty rows stay. ○ in free mode with no box does nothing.
    - △ while × is engaged on a fold changes nothing: the wormhole is still the ui's held entity.
    - The selected planet marked dead → free mode, centre cursor, no selection.
    - A new game started after a network-mode selection → free mode, no selection.
  - *Chunk 2:*
    - In network mode the ui's pad selection is the planet, and it is none while the menu is open and in free mode.
    - One draw in network mode shows no cursor-sprite pixels at the cursor point, and colour-7 pixels on the radius-9 circle in the visible phase. One draw 16 frames later shows none.
    - A mouse-mode draw of the same scene has no circle.

    The draws are kept to these few, so the binary stays at its current cost.
- **`test_input_step`:** no change expected. The live-against-recorded test must pass unchanged, because network mode is deterministic given the world and the pad masks.
- **E2E — `pad_colonise.json`,** indicative script; the exact frames come from this galaxy's dump:
  - `pad_probe`'s frames up to the × that starts the game, verbatim (N12). Then the starting probe and a fold, shortened if `pad_probe`'s idle frames allow;
  - pan until a planet revealed by the fold (known) lies under the centre;
  - △: the colonise menu opens;
  - one Down and one Up inside the menu, showing panel navigation (skipped if the menu has a single row);
  - × on the colonise row: the planet is colonised, the menu closes, network mode;
  - at least 32 idle frames: the cursor sprite is absent, and the circle is shown, then hidden, then shown again;
  - if the frame budget allows: △ again (the inspection panel), then ○ (closed), then a few idle frames.

  Its `.txt` follows the `keyboard_*` notes: the frame-by-frame script and what the dump shows at the key frames (the menu and its focused row, the colony ring, the circle's visible and hidden phases, no cursor sprite). Then come the `make state` end-state lines (credits, clock, colonies 1, slipways 0, techs 0). Beside them sits `keyboard_slipway`'s state after its first colony: f92, p30 colonised, $97 → $92, month 2 → 5. The note says that the galaxies differ (2.1's D8), that the pairing compares the colony's cost and clock effect, and that the replay covers `keyboard_slipway`'s first-colony part. The second colony and the slipway are covered by `pad_slipway` (step 2.3), and the probe and fold by `pad_probe` (step 2.1).
- **Unchanged:** every mouse and `keyboard_*` `*.sha256`, and `pad_title` and `pad_probe` (N13). `git diff --stat tests/e2e/` shows only the three new `pad_colonise` files.
- **Speed:** the new unit tests follow `test_pad_nav`'s patterns (forked children, a few hundred `cart_update60()` calls at most, three draws in total). One replay under 180 frames stays within the e2e budget. `make test` and `make e2e` must not get measurably slower.
- **Prior art:**
  - `tests/test_pad_nav.c`: `test_planet_menu_is_reachable` (planet menu in world space with the camera moved), and 2.1's free-mode tests;
  - `tests/e2e/colony_panel.txt`: colonising, then the inspection panel through press and release on the colony;
  - `tests/e2e/keyboard_slipway.txt`: the keyboard colonies this replay pairs with;
  - `tests/e2e/pad_title.txt`: framebuffer-dump reading for colour checks.

## Out of Scope

- □ entering free mode from the selected planet, × on a planet creating a slipway, Left/Right slipway cycling and the brighter slipway, Up/Down travel and the camera following the selection, ○ returning to the starting planet (all step 2.3).
- A second colony and a slipway in a pad replay (`pad_slipway`, step 2.3).
- Tabs, L1/R1, Start, pause and its close, the score panel's close, the tech panel through the pad, the build tab and placement mode, and the modality rules (phase 3).
- Following a planet replaced by a project (collapse, harvest) with the selection (N10).
- Drawing a hover display for planets that the mouse game does not have (N5).
- Changing free mode's pan, × filter or camera bound (step 2.1).
- Dropping the pad-mode startup notice; README and `docs/plan-10.md` (step 4.2).
- Audio.

## Further Notes

- **The step note for 2.2** must record:
  - that no earlier pad hash changed (N13), or, if one did, which file, why, and that `make state` still gives its end-state lines;
  - `pad_colonise`'s end state next to `keyboard_slipway`'s state after its first colony, and that it covers that part of `keyboard_slipway`;
  - decisions N3 (network mode from the △ on, colonised or not), N4 (○ closes boxes through the ui's close operation, pause excluded) and N5 (hover info by construction);
  - any H-2.1-k whose behavior turned out false, with the fallback applied;
  - that every mouse and `keyboard_*` `*.sha256` is unchanged.
- Step 2.3 inherits the stored map mode. □ sets it to free with the selection as the starting planet. ○ in free mode returns to that planet. × in free mode on a planet goes through 2.1's single filter. Network mode's directions move the selection, and the camera follows it through `camera_pan`.
- Every later pad replay that reaches network mode shows the circle and the hidden cursor. A step that changes those visuals refreshes `pad_colonise` under §2's rule.
