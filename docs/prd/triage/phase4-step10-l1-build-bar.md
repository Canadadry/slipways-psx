---
title: "L1 build bar, and placement with the aim crosshair"
description: "Taking a structure means going round the tab ring with shoulder presses whose meaning depends on the tab; L1 must toggle the build bar directly, with Left/Right to pick, × to take and ○ or L1 to close, while the tab ring stays on R1 until step 12."
status: needs-triage
---

## Plan step (verbatim)

Plan file `docs/plan-12.md`, Phase 4 — Direct buttons instead of the tab ring, step 10:

> 10. **L1 build bar** and placement with the aim crosshair (× places, ○ cancels). `pad_build` re-authored. Until step 12 deletes it, the tab ring stays reachable on the shoulder button this step leaves free, so replays not yet re-authored keep their path.

The section 2 rules it follows:

> - **L1** toggles the build bar: Left/Right pick a structure, × takes it, ○ or L1 closes the bar. Placing a structure uses the aim-mode crosshair (structures go in empty space): × places, ○ cancels.

> - **○ always goes back one step**, in this order: close the box or panel; cancel the slipway or the placement; leave aim mode; at the root, jump to the last object × acted on (nothing at game start).

> - New inputs must not type the cheat sequence (× × ○ ○ × × ○ ○, see `src/game/cheat.c`) by accident. Checked by the end-state check of step 2a: each replay's cheat activation count must match its reference (some replays use the cheat on purpose).
> - Every step from 6 on that changes what × or ○ do (including the build bar and the ring's removal) checks whether ordinary input sequences, with D-pad presses between them (`cheat_step` ignores them), can complete × × ○ ○ × × ○ ○, and re-checks the deliberate cheat route of step 6. [...]
> - Checks outside `make test-all` run on every chunk that touches pad input or a replay: `make unit-run-test_live_slots`, `make end-state EVENTS=` on every touched replay, `make coverage-compare`, and `make rng-sync-probe` whenever `pad_colonise` changes (a test outside the suite went red unseen for a chunk in step 7).
> - From step 8 on, each new mode's state lives in its own `pad_<mode>_t`, never in `pad_jump_t` nor as loose `pad_nav_t` fields, unless the step note gives the reason; each step note records `pad_nav.c`'s line count and `pad_nav_t`'s field count before and after. A unit test may be deleted only with the mechanic it exercises, which the step removes; the step note lists each deleted test next to that mechanic, and a test of a surviving behaviour that no longer compiles is rewritten, not deleted.
> - Each e2e `.txt` states its replay's length as "N frames.", and `tools/e2e_limits.sh` (in `make e2e`) checks it against the `.json` (added in step 8). The same awk pass also checks `tools/e2e_long_exceptions.txt`: a listed replay's stated frame count matches its `.json`, and a listed replay runs over 180 frames (added in step 9).

Every other guardrail of section 2 applies as written: `make test-all` green on every commit and `make test` / `make e2e` no slower; game rules unchanged; re-authoring to the step 2a end state against reference `8defcba`, never re-baselining a changed end state; the relaxed-comparison fallback and its exceptions file only if a replay cannot reach its reference; the situation guardrail (only ring situations may go, each noted); the frame limits (180, or 600 if listed), never raised; shared prefixes first; every broken replay re-authored, not only `pad_build`; pinned values re-derived; coverage never drops; the cost record; the 8 rules of the step 2b discipline; the `worst-case/` games knowingly stale; one scheme only; never launch the raylib app.

## Facts from step 9 (done, `80f7289`)

Read from the "### Step 9" note of `docs/plan-12.md` and the committed code and replays at `80f7289` (`git show HEAD:<path>`). An earlier draft of this PRD held hypotheses H-9-1 to H-9-12. Each fact below says whether its hypothesis held.

- **H-9-1, held.** `select_allowed` requires the map tab, no tab change on the frame, no pause or panel, jump or aim mode, `placement == NONE`, no fold held and no click under way. So Select is inert on the bar (tab `build`) and while a structure is carried or suspended. Step 10 adds no Select gating; new case 13 pins it.
- **H-9-2, held.** `pad_nav_tab_t`, `shoulder_step`, `switch_tab`, `tabs_can_switch`, `unwind_sub_modes`, `build_step`, `begin_placement`, `placement_step`, `cancel_point` and × on `end turn` are as at `5c38b14`, apart from the Select gating. Fix `7286d84` arms `menu_cross_armed` on every map panel; the bar is not a map panel, so it does not change the bar.
- **H-9-3, failed in its numbers.** `pad_nav.c` has 1245 lines (not about 1,170 plus a little), `pad_nav_t` has 26 fields (`select` added, as expected), and `tests/test_pad_nav.c` has 202 tests (not 203). These are step 10's "before" numbers. 25 tests reach `build` through `go_to_tab` (R1).
- **H-9-4, held.** Ring trace at `80f7289`:
  - `pad_end_turn` (139 frames) has no L1, and `pad_research` and `pad_tech_costs` lost their "L1 L1, the map" with their ring years. No replay presses L1 on `end turn` any more.
  - `pad_bankrupt` takes its refused lab with R1 from the map (f88, × f89), inside the chain window.
  - `pad_message_box` (214 frames) keeps R1 to `build` at f96 and L1 back to the map at f98, and L1 from the map to `score` at **f146** (was f122). The year box stays drawn up to f146.
  - `pad_score` (61 frames) is untouched: L1 from the map to `score` at f16, L1 from `score` to `tech` at f47.
- **H-9-5, held, with new numbers.** Step 9's table: `pad_colonise` 445, `pad_end_turn` 139, `pad_research` 500, `pad_game_end` 112, `pad_message_box` 214, `pad_bankrupt` 95 (cut), `pad_planet_projects` 524 (cut at f86), `pad_tech_costs` 558 (cut after the six years), `pad_resume_at_start` 48. Only `pad_score` (limit 180) and `pad_message_box` (limit 600) grow in step 10, and both have room.
- **H-9-6, held through its "moved into a save" clause.** `pad_bankrupt` now resumes from the record after its fourth lab: its first four takes are in the save, so nothing is converted there. It keeps 2 takes (the fifth lab at f24, the refused lab at f88). Every take of `pad_planet_projects` and `pad_tech_costs` is after its cut. **Takes left to convert: 17** (was 20).
- **H-9-7, held.** No `.end-state` was re-baselined and there is no exceptions file.
- **H-9-8, held.** `pad_select_init` runs only inside `reset_map_mode` (init, a new game, and network mode's exits), which no shoulder path calls. A frame with Select not allowed counts the chain window down, so `pad_bankrupt`'s refused lab inside the window keeps its timing.
- **H-9-9, held.** `make coverage-compare` reports the same 69 unhit lines.
- **H-9-10, held.** Step 9 re-derived the pins. `test_live_slots` now seeds a replay's `initial_save`, and `test_rng_sync`'s quiet window is f292–444 on `pad_colonise`. Step 10 edits only frame-identical presses in pinned replays. `pad_colonise` has no take and is not touched, so `make rng-sync-probe` is not needed unless something changes it.
- **H-9-11, held.** Select is no part of the chord (unit-tested in step 9).
- **H-9-12, held.** `pad_bankrupt`'s refused lab is R1 then × at f88–89, inside the window. Converting it to L1 is frame-identical (D-2), so its toast peak of 42 and `entity_table` 161 hold.

## Facts from the code (read at `80f7289`, `git show HEAD:<path>`)

- **The ring.** `pad_nav_tab_t` is map, build, end turn, tech, score. `shoulder_step` gives R1 +1 and L1 −1, and both on one frame give 0. `switch_tab` runs `unwind_sub_modes` (closes a planet menu, cancels a carry through `cancel_point`, cancels a slipway), then `leave_tab` (clears the tech box, or clicks the score backdrop), then `set_tab`. `tabs_can_switch` waits on a foreign panel, a click's release, a landing, and a slipway's release frame. ○ on any tab other than map returns to the map in one press.
- **The build tab is already the bar.** On it, `build_step` picks icons with Left/Right (`icon_beside`, no wrap) and × presses the focused icon. The focus is `pad_nav_t.build_icon`, repaired to the rightmost revealed icon, the basic structure, when stale (`repaired_build_icon`). `tab_cursor` puts the cursor on the focused icon from the tab-change frame. `ui_set_pad_tab` gets NULL, since build has no HUD button.
- **Placement is already aim-mode placement.** The frame after × hands a structure over, `begin_placement` sets the map tab and `CARRYING`, and enters aim mode from jump mode with the camera unmoved. `placement_step` holds the cursor at the centre, pans with `aim_pan` (1/2/4 px per frame), places on ×'s press edge (the button goes up and the ui's drop judges it), and cancels on ○ through `cancel_point`. After it ends, aim mode stays. ○ order rung 2 (cancel the placement) and rung 3 (leave aim mode) already apply. `ui_set_pad_crosshair` is off while a structure is carried, pinned by `test_a_carried_structure_draws_no_crosshair`.
- **L1 at `80f7289`, by where it is pressed:**
  - From `build` to the map: `pad_message_box` f98.
  - From the map to `score`: `pad_score` f16, `pad_message_box` f146.
  - From `score` to `tech`: `pad_score` f47.
- **Structure takes through R1 from the map** (R1 to `build`, then × on an icon): 17. They are in `pad_bankrupt` (2: f24, f88), `pad_build` (2: f16, f41), `pad_empty_planet` (2: f76, f134), `pad_planet_projects` (5: f47, f73, f282, f364, f404), `pad_research` (3: f90, f114, f264) and `pad_tech_costs` (3: f135, f356, f426). The frame is the R1's. The other R1 visits to `build` are passes on the way to `end turn`, `tech` or `score`.
- **Unit tests that press L1** (`tests/test_pad_nav.c`):
  - `test_r1_and_l1_cycle_the_tabs` and `test_l1_on_score_hands_over_to_tech_on_the_third_frame` press L1 backwards round the ring.
  - The helper `open_score_from_the_map` and `test_the_landing_after_a_shoulder_cancel_waits_for_its_release` press L1 from the map to `score`.
  - Eight others use L1 only from `build` back to the map, or as a waiting shoulder.
- **The score backdrop** takes a press anywhere above the HUD (`leave_tab`'s comment), so the leaving click of a shoulder on `score` hides the panel wherever the cursor is.

## Decisions (ratified by `guideline`)

`guideline` ratified D-1 to D-6 as written below, with its refinements included. D-5 is amended.

### D-1. What L1 does, by where it is pressed

- **On the map** (any map mode): opens the bar.
  - **L1 waits exactly as the shoulders do** (`tabs_can_switch`): under a foreign panel, on a click's release frame, while a landing is due, and on the frame after a slipway release.
  - The tab-change frame runs `unwind_sub_modes` first, as today: it closes a planet menu, cancels a carried structure (its frame at `cancel_point`), and cancels a held slipway.
  - The map mode is kept, so closing the bar comes back to the same jump-mode hovered object or the same aim-mode camera.
- **On the bar:** closes it, back to the map. This is exactly today's L1 from `build`.
- **On `end turn`, `tech` or `score`:** leaves that tab the way any shoulder does (`leave_tab`) and opens the bar. From `end turn` this is exactly today's L1.
- **R1 is unchanged:** it cycles map → build → end turn → tech → score → map, including from the bar to `end turn`. That is the ring kept reachable on the shoulder L1 leaves free: R1.
- **L1 and R1 on one frame do nothing**, as today. **A held L1 toggles once**: only its press edge counts.
- **No game, pause, the final score panel:** L1 does nothing, as today's shoulders.
- **Why:** "L1 toggles the build bar" read as a toggle: closed, from anywhere, it opens; open, it closes. It is also the reading that keeps every L1 pressed on `build` or `end turn` frame-identical: only L1 from the map, `tech` or `score` changes meaning. The backward ring step (L1 = −1) is the mechanic this step removes.

### D-2. Every structure take moves to L1 now

- Every R1 from the map to `build` followed by a × on an icon (the 17 takes above) becomes L1.
- L1 from the map and R1 from the map both run `set_tab(BUILD)` with no landing, so the edit is meant to be **frame-identical**. Only the `.json` and the `.txt` wording change.
- **Hard check, per converted take:** the replay keeps its `.json.sha256`, its `.end-state`, every pinned value of the step 2a inventory and its frame count. **A conversion that moves any of them reverts to R1**, and the step note names it.
- **Why now:** the take is this step's mechanic, and step 11 gives R1 to tech and score. A take left on R1 would break there.
- R1 visits to `build` that pass on to another tab stay R1: they are ring paths, and step 11 or 12 re-authors them.

### D-3. The bar's state

- **Open** means `nav->tab == PAD_NAV_TAB_BUILD`. **Reason, for the step note** (section 2's "unless the step note gives the reason"): while the ring lives, R1 and L1 must reach one state for one screen. A separate `open` flag would let them disagree. Step 12, deleting `pad_nav_tab_t`, moves "open" into `pad_build_t`.
- **The open state is read only through the new `pad_nav_build_bar_open`**: in tests, in the bar's own code paths outside the ring, and in any later step. Only the ring's own code (`switch_tab`, `tab_step`, `tab_cursor`) compares the tab directly.
- **The bar's own state** moves into a new `pad_build_t` (`src/game/pad_build.{c,h}`): the focused icon (`icon`, today's `pad_nav_t.build_icon`). `pad_nav_t` swaps `build_icon` for `build`, so the field count stays 26.

### D-4. Placement keeps its current form

- **"Placing uses the aim crosshair"** means the free centre point and the aim-mode pan, not the crosshair glyph. Placement stays as it is: centre point, pan, × places, ○ cancels. Step 10 changes none of it.
- **The crosshair stays undrawn while a structure is carried**: the carried structure, its label and its hatched disk mark the point.
- **After a placement ends** (built, refused or cancelled), aim mode stays. That keeps the ○ order: rung 2 cancels the placement, rung 3 leaves aim mode.
- **Rejected:** going back to jump mode after a placement taken from jump mode. It would add a rung the locked ○ order does not have, and re-author every replay that places.

### D-5 (amended). The three L1 presses that break

- **`pad_score` f16** (L1 from the map to `score`): it opens `score` the forward way, R1 four times (through `build`, `end turn` and `tech`), as its f32–41 segment already does.
- **`pad_score` f47** (L1 from `score` to `tech`, the third-frame hand-over): it becomes **L1 on `score`, which hides the panel and opens the bar**. This is the new D-1 situation: the leaving click lands on the backdrop and takes no structure.
- **"○ on `tech`: map in one press" stays in `pad_score`'s own frames.** After the bar closes, R1 is pressed until `tech` (through `build` and `end turn`), the tech panel comes up, and ○ returns to the map in one press. Delegating that situation to another replay's `.txt` is not allowed. `pad_score` stays within 180 frames.
- **`pad_message_box` f146**: it opens `score` the forward way, R1 four times. The score box must still replace the year box. The re-authoring confirms that the year box is still shown when the score box replaces it (it is held by its `keep_open`).
- **Situations dropped, ring mechanic, each noted:** "L1 from `map`: `score`" (two replays) and "L1 on `score`: `tech`, the hand-over on the third frame".
- **Frame cost:** about +8 frames per forward route, and about +12 for `pad_score`'s second `tech` visit. `pad_score` goes from 61 to about 90 (limit 180). `pad_message_box` goes from 214 to about 222 (limit 600), and its long-exception reason is updated.

### D-6. `pad_build` re-authored

- f0–62 stay frame-identical. The only edits there are f16 and f41, R1 → L1 (D-2). The `.txt` is rewritten for the bar ("L1: the bar", "the build bar"), with every reading kept.
- **The six bar situations, appended**, with the whole replay within 180 frames:
  1. ○ leaves aim mode for jump mode.
  2. L1 opens the bar from jump mode, with the basic structure focused.
  3. × takes it. The carry starts in aim mode where the camera is, and no crosshair is drawn.
  4. L1 while carrying: the structure is cancelled at the icon and the bar is open on the next frame, with nothing charged.
  5. L1 closes the bar, back in aim mode (rung 3 is not spent).
  6. ○ leaves aim mode, and L1 then ○ open and close the bar on the hovered object.
- Every appended action leaves the game unchanged, so the `.end-state` is unchanged, and the `.json.sha256` is re-baselined for the longer capture.

## Problem Statement

To take a structure, the player presses a shoulder and lands on a tab of a five-tab ring. Which tab depends on where they started: R1 from the map gives `build`, but L1 from the map gives `score`. The same button means "next tab" or "previous tab", never "build". The ring is about to go (steps 11 and 12), and the build bar needs a button of its own before it does.

## Solution

**L1 toggles the build bar.**

- From the map, in jump or aim mode, L1 opens the bar on the last focused structure icon (the basic structure at first). It closes a planet menu, a held slipway or a carried structure on the way, as shoulders do today.
- On the bar, Left/Right pick a structure (no wrap), and × takes it.
- ○ or L1 closes the bar, back to the map mode the player left.
- A taken structure is carried at the screen centre in aim mode. The D-pad pans the camera under it, × places it and ○ cancels it. A second ○ leaves aim mode.
- L1 from the `end turn`, `tech` or `score` tabs leaves them for the bar.
- R1 keeps cycling the old ring until step 11.

## User Stories

1. As a player, I want L1 to open the build bar from the map, so that taking a structure is one press.
2. As a player, I want L1 again to close the bar, so that the same button undoes what it did.
3. As a player, I want ○ to close the bar, so that ○ always backs out.
4. As a player, I want the bar to open on the structure I last picked, so that building several of the same kind is quick.
5. As a player, I want the bar to open on the basic structure in a new game, so that the default is the cheapest.
6. As a player, I want Left and Right to pick a structure, so that I can choose among the revealed ones.
7. As a player, I want Left and Right to stop at the ends, so that I always know where the basic structure is.
8. As a player, I want × on the bar to take the focused structure, so that × means "act" here too.
9. As a player, I want a × held while I open the bar to take nothing, so that a press meant for the map never takes a structure.
10. As a player, I want the taken structure carried at the screen centre with the camera panning under it, so that I can place it in empty space.
11. As a player, I want × to place the carried structure, so that × acts on the spot.
12. As a player, I want a refused spot to cost nothing, so that a mistake is free.
13. As a player, I want ○ to cancel the carried structure at no cost, so that I can change my mind.
14. As a player, I want a second ○ to leave aim mode, so that ○ keeps going back one step at a time.
15. As a player, I want L1 while carrying to cancel the structure and reopen the bar, so that I can pick another one straight away.
16. As a player, I want closing the bar to return me to the hovered object in jump mode, so that opening the bar costs me my place on the map.
17. As a player, I want closing the bar to return me to aim mode with the camera unmoved when I opened it from there, so that I can keep aiming.
18. As a player, I want L1 to close a planet menu and open the bar, so that I never have to close the menu first.
19. As a player, I want L1 to cancel a held slipway and open the bar, as shoulders did, so that nothing half-done stays behind.
20. As a player, I want L1 under pause or the final score panel to do nothing, so that it never acts behind a box.
21. As a player, I want L1 to keep the year box shown, so that the banner is not lost.
22. As a player, I want L1 from the `end turn`, `tech` or `score` tabs to open the bar, so that L1 means "build" wherever I am.
23. As a player, I want L1 on the score panel to hide it without taking a structure, so that the panel's own click is harmless.
24. As a player, I want R1 to keep working round the old tabs until they go, so that end turn, tech and score stay reachable.
25. As a player, I want L1 and R1 pressed together to do nothing, so that a fumble does nothing.
26. As a player, I want holding L1 to toggle once, so that the bar does not flicker.
27. As a player, I want Select to do nothing while the bar is open, so that it never ends a year behind the bar.
28. As a player, I want the bar's ×, ○ and L1 never to type the cheat, so that my game stays honest.
29. As a replay author, I want every structure take moved from R1 to L1 frame for frame, so that no capture, hash or end state changes.
30. As a replay author, I want only L1 presses on the map, `tech` or `score` to break, so that I re-author three presses, not seventeen replays.
31. As a replay author, I want a mechanical ring trace of each replay, so that I find every broken press without reading every `.txt`.
32. As a reviewer, I want `pad_build` to show the bar's own situations (open, close by L1 and by ○, L1 while carrying), so that the e2e covers the new mapping.
33. As a reviewer, I want every touched replay to pass `make end-state` against `8defcba` with its cheat count, so that no game end state changes.
34. As a reviewer, I want the dropped situations limited to the backward ring hand-overs, each named, so that only removed mechanics go.
35. As a reviewer, I want `make coverage-compare` to show nothing newly unhit, so that coverage does not drop.
36. As a developer, I want the bar's focus and icon picking in their own `pad_build` module, so that step 12 can delete the ring without touching the bar.
37. As a developer, I want a `pad_nav_build_bar_open` accessor, so that the new tests do not depend on `pad_nav_tab_t`, which step 12 deletes.
38. As a developer, I want every test that pinned surviving behaviour rewritten rather than deleted, so that no intent is lost.
39. As the agent of step 11, I want no structure take left on R1, so that giving R1 to tech and score breaks no take.
40. As the agent of step 12, I want the bar's "open" to be the only thing tied to the ring, so that the deletion is small.
41. As a developer, I want `make test` and `make e2e` no slower, so that the fast loop stays fast.

## Implementation Decisions

### The module `pad_build` (own state, `pad_build_t`)

- New `src/game/pad_build.{c,h}`, in the style of `pad_jump`, `pad_slipway`, `pad_back` and `pad_select`: `pad_nav.c` only calls them.
- `pad_build_t { entity_id_t icon; }`: the focused structure icon.
- Functions, with header comments stating the contracts:
  - `void pad_build_init(pad_build_t *b)`: nothing focused.
  - `entity_t *pad_build_icon(const pad_build_t *b)`: the focused icon while it is a revealed structure icon; NULL otherwise. It reads only.
  - `entity_t *pad_build_focus(pad_build_t *b)`: the focused icon, after moving a stale or missing focus to the rightmost revealed icon (the basic structure). NULL when no icon is revealed.
  - `void pad_build_pick(pad_build_t *b, int step)`: step −1 moves the focus to the nearest revealed icon on its left, step 1 to the nearest on its right. Nothing moves past an end (no wrap) or when nothing is focused.
- `icon_beside` and `repaired_build_icon` move here from `pad_nav.c` unchanged in behaviour. The helpers read the hit-test bucket and `pad_panel_focus_point`, as `pad_jump_candidates` reads the node list.
- `pad_nav_build_icon` keeps its contract (the focused icon on the bar; NULL elsewhere), now reading `pad_build_icon`. `cancel_point` reads the focus through `pad_build_icon`.

### L1 in `pad_nav`

- `shoulder_step` becomes `shoulder_target(tab, pressed)`, a private function that returns the tab a shoulder leads to, or "none":
  - R1 alone: the next tab round the ring.
  - L1 alone: the map from the bar, the bar from anywhere else.
  - Both, or neither: none.

  `shoulder_switch` and `switch_tab` keep their order (unwind, leave the tab, set the tab) and their waits (`tabs_can_switch`).
- **New accessor** `int pad_nav_build_bar_open(const pad_nav_t *nav)`: a game runs and the bar is open (D-3). Its header states that it is 0 with no game.
- **Unchanged:** `build_step`'s picking and ×, through `pad_build`; `begin_placement`, `placement_step` and `cancel_point`; the suspended carry under pause; R1's ring; ○ on any non-map tab; `ui_set_pad_tab` and `ui_set_pad_crosshair`; jump, aim and slipway modes; △, × and ○ on the map; Select.
- **The trap to keep** (a comment of 40 words at most where `leave_tab` clicks): L1 on `score` clicks with the cursor already on the bar's icon. The press reaches the score backdrop, which takes presses above the HUD, and never the icon. A unit test pins it: no structure is handed over.
- **Nothing records** into `pad_back`: the bar acts on no map object. × that places a structure records nothing either (as today).

### The cheat (section 2's re-check)

- The bar's ×, ○ and L1 are lone presses, and the chord stays × + ○ on one frame (`without_chord`, `cheat_step`). So no ordinary sequence of bar, placement and ring presses, with D-pad presses between them, completes four chords.
- Unit-tested: a long ordinary sequence (open, pick, take, place, cancel, close by ○ and by L1, with D-pad presses) never fires the cheat.
- Step 6's deliberate route is re-checked in two places:
  - in jump mode on a colony (the existing test, unchanged);
  - with the bar open: four chords fire the cheat once, take nothing, close nothing, and leave the focus where it was.
- The step note records the verdict. Every replay's cheat count is checked by `make end-state`.

## Re-authoring

Discipline: the 8 rules of step 2b. No action moves in the action sequence: every edit is either frame-identical (D-2) or adds ring or bar presses that change no game state (D-5, D-6).

1. **Ring trace, mechanical.** A throwaway script in `tmp/` (not committed) walks each replay's presses through the ring and lists every L1 by the tab it is pressed on, and every R1-from-map take. It runs before and after the re-authoring. After: no L1 on the map, `tech` or `score` (except the bar's new situations, D-5, D-6), and no R1 take left.
2. **D-2 conversions** in every replay that takes a structure. The check is that every `.json.sha256`, `.end-state` and pinned value is unchanged and only the `.txt` wording moves ("R1: `build`" → "L1: the bar").
3. **D-5** in `pad_score` and `pad_message_box`. Their hashes are re-baselined, their end states unchanged, and their "N frames." and long-exception reason updated.
4. **D-6** in `pad_build`.

Shared prefixes: the D-2 conversions change no capture, so no prefix moves. `pad_message_box` shares `pad_end_turn`'s prefix only before f146.

## Implementation Chunks

Each chunk ends on a commit with `make test-all` green. Section 2's checks outside the suite run on every chunk:

- `make unit-run-test_live_slots`;
- `make end-state EVENTS=` on every touched replay (all 17 on chunks 1 and 2);
- `make coverage-compare`, with nothing beyond step 9's 69 unhit lines;
- `make rng-sync-probe` if `pad_colonise` changes (no chunk is expected to change it).

1. **A pure refactor: `pad_build`.**
   - Add `src/game/pad_build.{c,h}`: `pad_build_t` replaces `pad_nav_t.build_icon`, and `icon_beside` and `repaired_build_icon` move. Add `tests/test_pad_build.c`.
   - Nothing changes in behaviour: every `.json.sha256`, `.end-state` and pinned value is unchanged, and every `tests/test_pad_nav.c` test passes untouched.
2. **L1 toggles the bar, with the replays it breaks.**
   - `shoulder_target`, `pad_nav_build_bar_open`, and the `leave_tab` trap comment.
   - `tests/test_pad_nav.c`: the rewrites, the deletion and the new cases below.
   - In the same commit, D-5: `pad_score` and `pad_message_box` re-authored, plus any other replay the ring trace shows broken.
3. **Every take onto L1, and `pad_build`.** D-2 in every replay that takes a structure, each conversion under D-2's hard check (hash, end state, pins and frame count unchanged, or back to R1), then D-6 in `pad_build` (hash re-baselined, end state unchanged).
4. **The step note** in `docs/plan-12.md`, "### Step 10":
   - the ratified D-1 to D-6 (D-5 amended), and D-3's reason for the open state living in `nav->tab`;
   - any take D-2's hard check sent back to R1;
   - any fact of "Facts from step 9" the implementation found different, and what was done instead;
   - `pad_nav.c`'s line count and `pad_nav_t`'s field count before and after;
   - the deleted test next to its mechanic, and the rewritten ones;
   - the replays broken and re-authored per chunk, frame counts before and after, and the agent minutes;
   - the situations dropped (the backward ring hand-overs, by replay and frame) and those added (D-6, `pad_score`'s L1 on `score`);
   - the cheat verdict, the `make coverage-compare` result, and the `make test` and `make e2e` wall times before and after;
   - the note for step 11 (see Further Notes).

## Testing Decisions

- **A good test** drives public behaviour only.
  - Pad tests drive `run_frame`, `tap` and `hold`, then assert on `pad_nav_build_bar_open`, `pad_nav_build_icon`, `pad_nav_placing`, `pad_nav_tab` (ring tests only), `pad_nav_jump_mode`, `pad_nav_aim_mode`, `pad_nav_hovered`, the played frame's mouse, the camera, `ui_left_held`, the world counts and credits, and `p8_fixed_rnd_state()`.
  - No test reads `pad_nav_t` or `pad_build_t` fields.
- **Prior art:**
  - `tests/test_pad_jump.c` and `tests/test_pad_select.c` for a pure pad module on a started cart;
  - in `tests/test_pad_nav.c`: `go_to_tab`, `assert_build_focus`, `basic_structure_icon`, `processor_icon`, `take_structure`, `pan_clear_of_every_node`, `assert_shoulders_wait`, `open_score_from_the_map`, `test_shoulders_cancel_the_carried_structure_then_switch`, `test_four_chords_on_a_hovered_colony_cheat_and_change_nothing_else`, `test_building_and_backing_out_never_types_the_cheat`.
- **`tests/test_pad_build.c`** (chunk 1), on a started game's HUD icons:
  - nothing focused after init, and `pad_build_icon` is NULL;
  - `pad_build_focus` moves a missing or stale focus to the basic structure, and returns NULL with no revealed icon;
  - `pad_build_pick` walks left in decreasing x over every revealed icon, skips hidden ones, and stops at both ends;
  - a focus on an icon of an old game is stale in a new one.
- **`tests/test_pad_nav.c`** (chunk 2).
  - **Rewritten** (surviving behaviour; each keeps its intent):
    - `test_r1_and_l1_cycle_the_tabs` becomes "R1 cycles the five tabs forward": the R1 half is kept, and the L1 half moves to the new toggle cases.
    - The helper `open_score_from_the_map` uses R1 forward, and the tests that call it keep their assertions.
    - `test_the_landing_after_a_shoulder_cancel_waits_for_its_release` loses its only route, L1 from the map to `score` while carrying, since no shoulder from the map lands on a panel tab in this step. It becomes "L1 while carrying cancels at the icon, then opens the bar, pressing nothing on either frame". The landing-after-cancel guard stays in the code, untested this step. Step 11 brings back its test with R1 → tech (Further Notes).
    - The 25 tests that reach `build` through `go_to_tab` switch to a new `open_build_bar` helper (L1), since the bar is the surviving mechanic. Ring tests that pass through `build` keep R1.
  - **Deleted, with its mechanic** (L1 one tab back round the ring): `test_l1_on_score_hands_over_to_tech_on_the_third_frame`. Listed in the step note.
  - **New:**
    1. L1 from jump mode opens the bar on the basic structure from the tab-change frame: the cursor is on the icon and nothing is pressed. L1 again closes it, back in jump mode on the same hovered object, with the camera unmoved.
    2. ○ closes the bar, as in case 1.
    3. From aim mode, L1 L1 brings back aim mode with the camera unmoved and the pan restarting slow.
    4. The bar remembers its focus across a close and reopen (Left, close, open: the processor), until a new game.
    5. × on the bar takes the structure. The next frame carries it in aim mode with no crosshair, × places it, and ○ on another take cancels it at no cost. A second ○ leaves aim mode.
    6. L1 while carrying (case 5's take): the cancel frame is at the icon with the button up, then the bar is open, the structure dead, and the world and credits unchanged.
    7. L1 on `end turn`, on `tech` (the box closes) and on `score` (the backdrop click hides the panel): the bar opens each time, and no structure is handed over on the click or its release.
    8. R1 on the bar goes to `end turn`. R1 and L1 together on the map or the bar do nothing. A held L1 toggles once.
    9. L1 under a planet menu closes it and opens the bar. L1 in slipway mode cancels the slipway (nothing built or charged) and opens the bar. L1 under the year box keeps the box.
    10. L1 waits as shoulders do: under pause, on a click's release frame, and on the frame after a slipway release (`assert_shoulders_wait` with L1). With no game and on the final score panel, it does nothing.
    11. A × held through L1's frame takes nothing until pressed again (`test_cross_held_on_arrival_on_build_takes_nothing`, through L1).
    12. Cheat: four chords with the bar open fire the cheat once, take nothing, and leave the bar open on the same icon. A long ordinary bar, placement and ring sequence never fires it.
    13. Select on the bar: a 40-frame hold and a tap end no year, press nothing, and leave the bar open.
    14. Bar toggles, takes, places and cancels leave `p8_fixed_rnd_state()` unchanged.
- **Kept unchanged:** every placement, pause-suspend, ring-forward, tab-panel, year-box, jump, aim, slipway and ○-order test.
- **Before each commit:** `make test-all`, the checks listed under Implementation Chunks, and `make raylib` (compiles and links only; the keymap is unchanged).
- **At the end:** `make full-game`, `make save-worst-case` and `make save-fuzz` once. They are knowingly stale (their scripts press L1 as the old ring), so only an assert or a crash matters. Also compare the `make test` and `make e2e` wall times before and after the step.
- **Not tested:** misuse asserts.

## Out of Scope

- R1 cycling tech → score → closed, and re-authoring the ring's tech and score visits (step 11).
- Removing `pad_nav_tab_t`, network mode and the ring's remaining code. Moving the bar's "open" into `pad_build_t`, and the HUD highlight following the open panel (step 12).
- Any change to placement's mechanics, the crosshair's drawing, or what the HUD icons and tooltips show.
- Re-authoring the `worst-case/` games (step 13). They press L1 in the old sense.
- Step 14's metrics tool, the README Controls section (step 15).
- Tuning values, audio, PSX, mouse input.

## Further Notes

- **For step 11:**
  - No structure take is left on R1 (D-2). Every R1 still in the replays is a ring pass to `end turn`, `tech` or `score`.
  - The landing-after-cancel guard (`clicks_due`'s `cancelled`) has no pad route in step 10. With R1 → tech, a shoulder pressed while carrying lands on a panel tab again: step 11 restores that unit test.
- **For step 12:** the bar's "open" is `nav->tab == PAD_NAV_TAB_BUILD` (D-3). When the ring goes, it becomes a `pad_build_t` field, and `pad_nav_build_bar_open` keeps its contract, so the new tests need no rewrite.
- **For step 14:** "L1 then L1" with nothing taken is not a dead press: it opens and closes a panel. A Left or Right at the bar's ends is a dead press in the "others" group (no locked rule requires it).
- **Why the bar is the build tab and not a new screen:** the HUD's structure icons are always drawn, and the build tab already focuses them, picks with Left/Right, takes with × and closes with ○. The step gives that screen its own button. It builds no new one, as section 2's "Out of scope" asks.
