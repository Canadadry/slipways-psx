---
title: "R1 cycles tech, score, closed"
description: "Reaching the tech panel takes three R1 presses round a ring through build and end turn; R1 must open tech from the map, hand over to score, and close, so tech and score have a direct button before step 12 removes the ring."
status: needs-triage
---

## Plan step (verbatim)

Plan file `docs/plan-12.md`, Phase 4 — Direct buttons instead of the tab ring, step 11:

> 11. **R1 cycles tech → score → closed.** `pad_research`, `pad_tech_costs` and `pad_score` re-authored. From this step every ring tab has a direct button (map by ○, build on L1, end turn on the Select hold, tech and score on R1), so the ring is no longer needed to reach any old end state.

The section 2 rules it follows:

> - **R1** cycles tech panel → score panel → closed.

> - **○ always goes back one step**, in this order: close the box or panel; cancel the slipway or the placement; leave aim mode; at the root, jump to the last object × acted on (nothing at game start).

> - New inputs must not type the cheat sequence (× × ○ ○ × × ○ ○, see `src/game/cheat.c`) by accident. Checked by the end-state check of step 2a: each replay's cheat activation count must match its reference (some replays use the cheat on purpose).
> - Every step from 6 on that changes what × or ○ do (including the build bar and the ring's removal) checks whether ordinary input sequences, with D-pad presses between them (`cheat_step` ignores them), can complete × × ○ ○ × × ○ ○, and re-checks the deliberate cheat route of step 6. [...]

Every other guardrail of section 2 applies as written:

- `make test-all` is green on every commit, and `make test` and `make e2e` get no slower.
- The game rules don't change.
- Replays are re-authored to the step 2a end state against reference `8defcba`. An end state that changed is never re-baselined.
- The relaxed comparison and its exceptions file are used only if a replay cannot reach its reference.
- The situation guardrail: only ring situations may go, and each one is noted.
- The frame limits (180, or 600 if listed) are never raised.
- Shared prefixes are re-authored first. Every broken replay is re-authored, not only the three named ones.
- Pinned values are re-derived. Coverage never drops. The re-authoring cost is recorded.
- The 8 rules of the step 2b discipline. The `worst-case/` games are knowingly stale.
- From step 8 on, a new mode's state lives in its own `pad_<mode>_t`, and the step note gives `pad_nav.c`'s line count and `pad_nav_t`'s field count before and after.
- A test is deleted only with its mechanic.
- The checks outside `make test-all` run on every chunk that touches pad input or a replay.
- One scheme only. Never launch the raylib app.

## Facts from step 10 (done, `a65a828`)

Read from the "### Step 10" note of `docs/plan-12.md` and from the committed code, replays and fixtures at `a65a828` (`git show HEAD:<path>`). An earlier draft of this PRD held hypotheses H-10-1 to H-10-12. Each fact below says whether its hypothesis held.

- **H-10-1, held.** L1 on the bar closes it to the map. Anywhere else (the map in any mode, `end turn`, `tech`, `score`), L1 unwinds and opens the bar. L1 and R1 together do nothing. L1 waits as the shoulders do, and a held L1 toggles once.
- **H-10-2, held.** R1 keeps the forward ring: map → build → end turn → tech → score → map, and R1 on the bar goes to `end turn`.
  - `shoulder_target(tab, pressed)` decides where a shoulder leads, and returns `PAD_NAV_TAB_COUNT` for none. Its R1 case is `(tab + 1) % PAD_NAV_TAB_COUNT`.
  - `shoulder_switch` runs `leave_tab`, then `set_tab`. `switch_tab`, `tabs_can_switch` and the landing logic are as at `87a6a6f`.
- **H-10-3, held.** "Bar open" is `tab == BUILD`, read through `pad_nav_build_bar_open`. The focus lives in `pad_build_t`. `pad_nav_t` has 26 fields, and `pad_nav.c` has **1219** lines: these are step 11's "before" numbers.
- **H-10-4, held.** All 14 replay takes are on L1, every one frame-identical, and none was reverted. Step 10's note records that its PRD's list of 17 was wrong (6 tech chains counted as takes, 3 takes missed).
- **H-10-5, held.** The 17 R1 R1 R1 tech chains are at their `87a6a6f` frames. The frame given is each chain's first R1; the next two R1s follow 2 frames apart in every chain.
  - `pad_planet_projects` (9): f32, f43, f54, f69, f228, f267, f278, f289, f476.
  - `pad_research` (5): f48, f86, f97, f110, f125.
  - `pad_tech_costs` (3): f13, f117, f131.
- **H-10-6, held in content, failed in its number.** `pad_score` is **77 frames** (61 → 77), not about 90. Its presses, from a ring trace at `a65a828`:
  - f0–15 as before;
  - R1 R1 R1 (f16, f18, f20): `tech`; f25 R1: `score`;
  - f31 Down, f33 ×: nothing; f37 ○: the map;
  - R1 R1 R1 (f41, f43, f45): `tech`; f50 R1: `score`;
  - f56 L1 on `score`: the panel hidden and the bar open; f60 L1: the bar closed;
  - R1 R1 R1 (f62, f64, f66): `tech`; f71 ○ on `tech`: the map in one press.
- **H-10-7, held; the number is 223.** `pad_message_box` is **223 frames** (214 → 223).
  - f96 R1 (`build`) and f98 L1 (bar closed) are unchanged.
  - Its route to `score`: R1 R1 R1 at f146, f148, f150 (`tech`), R1 at f155 (`score`), ○ at f163.
- **H-10-8, held.** `pad_end_turn` (139 frames) still has R1 at f16 (`build`) and f18 (`end turn`), × held from f21 for one year, and ○ at f44.
- **H-10-9, held; the number is 49.** The `title_pick_*` fixtures are **49 frames** (40 → 49). Each reaches `score` by R1 R1 R1 at f16, f18, f20 (`tech`), then R1 at f25 (`score`).
- **H-10-10, failed.** Step 10 already moved the 25 fixture takes (`grabs_refused` **23**, not 22; `slipways_refused` 2) to L1. Step 11 has no fixture take to convert.
- **H-10-11, held.**
  - `go_to_tab` still taps R1 until the tab matches (at most 5).
  - `open_score_from_the_map` is `go_to_tab(TECH)` then R1. That route works unchanged under step 11's mapping.
  - `open_build_bar` (L1) exists. `tests/test_pad_nav.c` has 216 tests.
  - `test_l1_on_score_hands_over_to_tech_on_the_third_frame` was deleted with its mechanic.
  - **The landing-after-shoulder-cancel guard is untested.** Step 11 brings its test back (R1 → tech while carrying).
- **H-10-12, held.** The same 69 unhit lines, the cheat unchanged, and `full-game`, `save-worst-case` and `save-fuzz` clean. The step note re-baselines no `.end-state`.

**Chains to convert in step 11: 23.** These are the 17 above, plus `pad_score` f16, f41 and f62, `pad_message_box` f146, and `title_pick_forgiving` and `title_pick_reasonable` f16.

**Frame-identical R1 → L1 conversions left: 2.** These are `pad_end_turn` f16 (then R1 on the bar at f18: `end turn`) and `pad_message_box` f96 (then L1 at f98 closes the bar).

## Facts from the code (read at `a65a828`, `git show HEAD:<path>`)

- **The ring.** `pad_nav_tab_t` is map, build, end turn, tech, score.
  - `switch_tab` takes a shoulder only when `tabs_can_switch` allows it. It runs `unwind_sub_modes` (closes a planet menu, cancels a carry through `cancel_point`, cancels a slipway), then `shoulder_switch` (`leave_tab`, then `set_tab`).
  - `leave_tab` clears the tech box, or clicks the score backdrop, which takes a press anywhere above the HUD.
  - `set_tab` sets `landing_due` for `tech` and `score` (`opens_a_panel`).
  - The frame after a tab change, the landing clicks the tab's HUD button (tech is button 2, score button 3). It waits while a click's release is due or a shoulder cancelled something (`clicks_due`).
  - Once a tab's own panel is gone (an invention, or the score backdrop hidden), the tab returns to the map by itself.
- **R1 on `tech` already hands over to `score` in one frame, and R1 on `score` already closes to the map.** Both are pinned: `test_r1_on_tech_closes_the_box_and_shows_score_on_the_same_frame` and `test_r1_on_score_hides_it_and_returns_to_the_map`. **The only R1 transition that changes is the one from the map:** `shoulder_target` gives `build` there today, and gives `tech` after step 11.
- **`opens_a_panel`, `pause_button_free`, `own_panel_up` and `tab_cursor`** already handle `tech` and `score` reached from any tab. A tab whose HUD button is gone returns to the map: `test_a_tab_whose_button_is_gone_returns_to_the_map`.
- **The tech panel over the year box keeps the year box.** In `pad_planet_projects` f228–247, the year box has been up since f226. Through the tech visit it stays up, and f247's ○ closes it. The score box, by contrast, replaces the year box (`pad_message_box`'s route to `score`).
- **The HUD highlight** is `ui_set_pad_tab(tab_button(tab))`: the tech button is lit on `tech` and the score button on `score`, whatever tab came before. Step 12 makes it follow the open panel.
- **Replays with no R1 at all:**
  - `pad_bankrupt`, `pad_build` and `pad_empty_planet` use L1 only.
  - `pad_colonise`, `pad_game_end`, `pad_pause_restart`, `pad_probe`, `pad_restart_resume`, `pad_resume_at_start`, `pad_slipway` and `pad_title` press no shoulder.
  - `grabs_refused` and `slipways_refused` use L1 only.
  - `events_pad_unordered` only tests input loading.
- **Tests that press R1 from the map**, besides `go_to_tab`, in `tests/test_pad_nav.c`:
  - the ring and hold tests;
  - the shoulder-cancels-a-carry, shoulder-closes-a-menu and shoulder-in-aim-mode tests;
  - the slipway-cancel-by-tab test and the wait-after-slipway-release test;
  - `test_the_ui_pad_tab_is_the_tab_hud_button`;
  - the tech and score landing tests;
  - Start + R1;
  - `test_a_new_game_starts_on_the_map`, `test_restart_under_pause_drops_the_suspended_structure` and `test_no_pad_selection_is_shown_behind_a_tab`, or whichever of them step 10 left on R1.

  `go_to_tab` is called for `END_TURN` 13 times. With R1 from the map going to `tech`, its loop reaches `tech` and `score` but never `end turn`.
- **Other tests with shoulder bits:** `tests/test_input_step.c`, `test_marker_coherence.c` and `test_recording_continuity.c` use R1 and L1 as recorded input bits, not through `pad_nav`. They are unaffected.

## Decisions (ratified by `guideline`)

`guideline` ratified G-1 to G-5 as recommended, with the refinements included below.

### G-1. R1 on the bar and on `end turn`: the ring, unchanged

- R1 changes only from the map, where it opens `tech`. R1 on the bar still goes to `end turn`, and R1 on `end turn` goes to `tech`. The end-turn tab stays reachable through L1 then R1 until step 12 deletes it.
  - It matches step 9's D-4 (`pad_end_turn` keeps its year on the end-turn tab until step 12).
  - It keeps the 13 `end turn` unit tests and the end-turn tab's coverage, and `pad_end_turn` stays frame-identical.
  - The rule's "tech → score → closed" holds from where the cycle is closed, the map.
- **Rejected:** R1 on the bar goes to `tech`. It would make the end-turn tab unreachable one step early, against step 9's D-4, and would fold step 12's deletion of the 13 end-turn tests into step 11. Step 12 decides what R1 on the bar does once the ring is gone.

### G-2. Each tech chain is converted frame-aligned

- In each R1 R1 R1 chain from the map, the first two R1s (map → build, build → end turn) become idle frames. The third keeps its frame, now map → tech.
  - Every frame from the tech landing on is meant to be identical, so frame counts, every pinned frame of the step 2a inventory and the camera path are kept.
  - The `.json.sha256` is re-baselined only because the capture now shows the map on the two freed frame pairs, where the build tooltip and the lit year skip were. The `.end-state` is unchanged.
- **Refinement:** each chain is checked at its landing with `make state`. If the camera or the map mode drifts, the freed frames move instead (D-3), the total frame count is kept, and the step note names that chain.
- **Rejected:** compressing the chains. It would force every later pin to be re-derived, and would expose all 23 chains to frame-timing risk.

### G-3. Two accessors, no new struct

- Add `int pad_nav_tech_open(const pad_nav_t *nav)` and `int pad_nav_score_open(const pad_nav_t *nav)`.
- **Header contracts:** each returns 0 with no game, and 1 from the frame the tab changes.
- New and rewritten tests read them instead of comparing `pad_nav_tab`, so step 12's deletion of `pad_nav_tab_t` needs no rewrite of them.
- The state stays `nav->tab`. **The step note gives the reason there is no new `pad_<mode>_t`:** step 11 adds no state. While the ring lives, R1 and the ring must reach one state for one screen (step 10's D-3).

### G-4. Every replay and fixture the new R1 breaks; the frame-identical conversions in chunk 1

- Besides the three named replays, step 11 also re-authors:
  - `pad_planet_projects` and `pad_message_box`, for their chains;
  - `pad_end_turn` (G-1);
  - the two `title_pick_*` fixtures.

  This follows section 2's "a step re-authors every replay it breaks".
- **Chunk 1** holds the frame-identical R1 → L1 conversions, `pad_end_turn` f16 and `pad_message_box` f96. They are frame-identical under step 10's mapping, because L1 and R1 from the map both run `set_tab(BUILD)` with no landing.
- No take is left to convert (H-10-10 failed: step 10 already moved the fixture takes).

### G-5. `pad_score`'s situation list

`pad_score` shows the whole R1 cycle from the map, both ○ exits, and step 10's L1 situation, within 180 frames (D-4).

## Problem Statement

To open the tech panel, the player presses R1 three times, passing the build bar and the end-turn tab, which flash by with their tooltip and highlight. The score panel takes a fourth press. In the measured game, shoulder presses came in bursts of 5–7. Build has its own button now (L1, step 10), and ending the year has Select (step 9), so the ring's detours lead nowhere the player needs.

## Solution

**R1 cycles tech → score → closed.**

- From the map, in jump or aim mode, R1 opens the tech panel. On the way, it closes a planet menu, cancels a held slipway or cancels a carried structure, as shoulders do today.
- R1 on the tech panel closes it and shows the score panel on the same frame.
- R1 on the score panel hides it, back on the map in the map mode the player left.
- ○ on either panel goes back to the map in one press, as today.
- L1 on either panel leaves it for the build bar (step 10, unchanged).
- Until step 12, R1 on the build bar still goes to the end-turn tab (G-1), the ring's last reachable piece.

## User Stories

1. As a player, I want R1 on the map to open the tech panel in one press, so that researching costs one press instead of three.
2. As a player, I want R1 on the tech panel to show the score panel, so that I can glance at both with the same button.
3. As a player, I want R1 on the score panel to close it, so that the cycle ends where it started.
4. As a player, I want closing the cycle to return me to the same hovered object in jump mode, so that looking at a panel never costs my place.
5. As a player, I want closing the cycle to return me to aim mode with the camera unmoved when I opened it from there, so that I can keep aiming.
6. As a player, I want the build bar and the end-turn highlight not to flash by on the way to tech, so that the screen shows only what I asked for.
7. As a player, I want ○ on the tech panel to return to the map in one press, so that ○ always backs out.
8. As a player, I want ○ on the score panel to return to the map in one press, so that ○ always backs out.
9. As a player, I want × on a tech row to invent it and return to the map, as it does today, so that inventing is unchanged.
10. As a player, I want the D-pad on the tech panel to move between rows, as today, so that choosing a technology is unchanged.
11. As a player, I want × and the D-pad on the score panel to press nothing, as today, so that the panel's backdrop never hides by accident.
12. As a player, I want R1 to close a planet menu and open tech, so that I never have to close the menu first.
13. As a player, I want R1 to cancel a held slipway at no cost and open tech, so that nothing half-done stays behind.
14. As a player, I want R1 while carrying a structure to cancel it at no cost and open tech once the cancel is released, so that the cancel's click and the tech button's click never share a frame.
15. As a player, I want R1 under the year box to keep the year box, so that the banner is not lost behind the tech panel.
16. As a player, I want R1 to do nothing under pause, with no game, or on the final score panel, so that it never acts behind a box.
17. As a player, I want R1 to wait on a click's release frame and on the frame after a slipway release, as shoulders do, so that one press never acts twice.
18. As a player, I want holding R1 to move one step only, so that the panels do not flicker.
19. As a player, I want R1 and L1 pressed together to do nothing, so that a fumble does nothing.
20. As a player, I want L1 on the tech or score panel to open the build bar, as step 10 made it, so that L1 means "build" everywhere.
21. As a player, I want the tech button lit while the tech panel is up and the score button while the score panel is up, so that the HUD tells me where I am.
22. As a player, I want a × held through R1's press to invent nothing, as a × held into any tab does today, so that a press meant for the map never buys a technology.
23. As a player, I want R1 never to type the cheat, so that my game stays honest.
24. As a player, until the ring goes, I want L1 then R1 to still reach the end-turn tab, so that nothing the old game offered is cut off before step 12.
25. As a replay author, I want every R1 R1 R1 tech chain converted by idling its first two presses, so that every later frame, pin and end state stays where it was.
26. As a replay author, I want every remaining R1 from the map to `build` turned into L1 before the mapping changes, so that the fixtures and `pad_end_turn` change in no frame.
27. As a replay author, I want a mechanical ring trace before and after, so that I find every broken press without reading every `.txt`.
28. As a reviewer, I want `pad_score` to show R1 from the map to tech, R1 to score, R1 closed, and both ○ exits, so that the e2e covers the new mapping.
29. As a reviewer, I want every touched replay to pass `make end-state` against `8defcba` with its cheat count, so that no game end state changes.
30. As a reviewer, I want the dropped situations limited to ring passes through `build` and `end turn` on the way to tech, each noted, so that only removed mechanics go.
31. As a reviewer, I want `make coverage-compare` to show nothing newly unhit, so that coverage does not drop.
32. As a developer, I want `pad_nav_tech_open` and `pad_nav_score_open`, so that the new tests survive step 12's deletion of `pad_nav_tab_t`.
33. As a developer, I want the landing-after-cancel guard tested again through R1 while carrying, so that the guard step 10 left untested is pinned.
34. As a developer, I want every test that pinned surviving behaviour rewritten, not deleted, so that no intent is lost.
35. As the agent of step 12, I want the end-turn tab reachable only through the bar's R1, so that deleting the ring removes one transition and its tests.
36. As a developer, I want `make test` and `make e2e` no slower, so that the fast loop stays fast.

## Implementation Decisions

### D-1. R1's destination, by where it is pressed

- **On the map** (jump, aim or slipway mode, or a planet menu): `tech`. This is the only change to R1.
- **On `tech`:** `score`, the hand-over in one frame (unchanged).
- **On `score`:** the map: the leaving click hides the backdrop (unchanged).
- **On the bar:** `end turn` (unchanged, G-1).
- **On `end turn`:** `tech` (unchanged).
- **Unchanged:**
  - R1 and L1 together do nothing, and a held R1 counts its press edge only.
  - The waits (`tabs_can_switch`) and the unwind on the tab-change frame.
  - The landing on the next frame, and its wait after a cancel (`clicks_due`).
  - No game, pause or the final score panel: nothing.
- **The map mode is kept** across the visit, as it is across any tab visit today. The hovered object and the aim-mode camera come back. The aim pan restarts slow, as after any tab change.
- **In code:** one case of `shoulder_target` changes (R1 alone on the map gives `PAD_NAV_TAB_TECH`). Nothing else in `switch_tab`, `leave_tab`, `set_tab`, the landing or `tab_step` changes.
- **Nothing records into `pad_back`:** R1 acts on no map object.
- **No new `pad_<mode>_t`** (G-3). `pad_nav_t` keeps its field count.

### D-2. The two accessors (G-3)

- `int pad_nav_tech_open(const pad_nav_t *nav)` and `int pad_nav_score_open(const pad_nav_t *nav)` go in `src/game/pad_nav.h`.
- **Header contracts:** each returns 0 with no game, and 1 from the frame the tab changes, whether the landing is due or the panel is drawn.
- They read `pad_nav_tab` and nothing else. Only the ring's own code keeps comparing the tab directly.

### D-3. Converting the replays (G-2, G-4)

- **Before the mapping change** (chunk 1, frame-identical under step 10's mapping), the two R1s from the map to `build` that do not start a tech chain become L1:
  - `pad_end_turn` f16 (R1 on the bar at f18 still reaches `end turn`);
  - `pad_message_box` f96 (L1 at f98 closes the bar).

  L1 from the map and R1 from the map both run `set_tab(BUILD)` with no landing. **Hard check:** each file keeps its `.json.sha256`, `.end-state`, every pinned value and its frame count. Only the `.json` and the `.txt` wording change ("R1: `build`" → "L1: the bar").
- **With the mapping change** (chunk 2), each of the 23 chains of "Facts from step 10" is converted frame-aligned. The first two R1s become idle and the third, now map → tech, keeps its frame. R1s pressed afterwards on `tech` (to `score`) keep their frames.
  - **Check, per chain, with `make state` at the landing:** the camera and the map mode equal the old route's. A jump-mode edge scroll still running when the chain starts would keep scrolling on the freed frames, where the old route had left the map.
  - **If one drifts,** the freed frames move instead (after the invention, or before the chain), the total frame count is kept, and the step note names that chain.
  - Afterwards, re-pan to the reference camera before the next point action (rule 8).
- **`pad_message_box`** (f146 chain): R1 at f150 (map → tech) and R1 at f155 (tech → score). The year box must still be shown when the score box replaces it, as file shows.
- **The `title_pick_*` fixtures** (f16 chain): R1 at f20 (tech) and f25 (score). Both still end with the score panel up, showing the chosen difficulty.
- **`pad_resume_at_start`** is cut from `pad_research`'s last record. The conversions change no game state, so the record and the file are expected unchanged. If `make end-state` or its hash says otherwise, it is re-cut.

### D-4. `pad_score` re-authored (G-5)

The file is 77 frames. Its three chains (f16, f41, f62) are converted as in D-3, which keeps:

- f16 chain: R1 (f20) to `tech`, R1 (f25) to `score` in one frame; Down (f31) and × (f33) press nothing; ○ (f37): the map.
- f41 chain: R1 (f45) to `tech`, R1 (f50) to `score`; L1 (f56) on `score`: the panel hidden and the bar open, no structure taken (step 10's situation); L1 (f60): the bar closed.
- f62 chain: R1 (f66) to `tech`; ○ (f71) on `tech`: the map in one press.

**Appended:** R1 R1 R1 from the map: `tech`, `score`, then **R1 on `score`: closed**. The backdrop click hides the panel and the map comes back in the mode it was left in. This is the only situation of the new rule the file lacks. The file ends with idle frames for the backdrop to go, within 180 frames.

- Each situation keeps its readings in the `.txt` (HUD fill pixels, the backdrop count, the tech header row). The tech button lights from the tab-change frame of R1 from the map, with no `build` or `end turn` frame between.
- **The end state is unchanged.** The `.json.sha256` is re-baselined. "N frames." is updated.
- **Situations dropped, ring only, each noted:** "R1 from `map`: `build`" and "R1 on `build`: `end turn`" on the way to `tech`. "R1 on `build`: `end turn`" lives on in `pad_end_turn` (G-1).

### D-5. The cheat (section 2's re-check)

- R1 is neither × nor ○, and step 11 changes neither. The chord stays × + ○ on one frame.
- Unit-tested: a long ordinary sequence of R1 cycles, ○ exits, inventions and L1 bar visits, with D-pad presses between them, never fires the cheat.
- Step 6's deliberate route is re-checked by the existing chord tests, unchanged.
- The step note records the verdict. Every replay's cheat count is checked by `make end-state`.

## Implementation Chunks

Each chunk ends on a commit with `make test-all` green. Section 2's checks outside the suite run on every chunk:

- `make unit-run-test_live_slots`;
- `make end-state EVENTS=` on every touched replay (all 17 on chunk 2);
- `make coverage-compare`, with nothing beyond the 69 unhit lines;
- `make rng-sync-probe` if `pad_colonise` changes (it is not expected to).

1. **Frame-identical preparation, under step 10's mapping.**
   - The two D-3 L1 conversions: `pad_end_turn` f16 and `pad_message_box` f96.
   - `pad_nav_tech_open` and `pad_nav_score_open` (D-2), with their tests.
   - `go_to_tab` reaches `end turn` through the bar (L1, then R1). `tech` and `score` it still reaches by R1 from the map. This route works under both mappings.
   - Check: every `.json.sha256`, `.end-state` and pinned value unchanged, and every unit test green.
2. **R1 from the map opens tech, with everything it breaks, in one commit.**
   - D-1's one-case change in `shoulder_target`.
   - The `tests/test_pad_nav.c` rewrites and new cases (Testing Decisions).
   - The frame-aligned chains in `pad_research` (first, as `pad_resume_at_start`'s source), `pad_tech_costs` and `pad_planet_projects`.
   - `pad_message_box`'s route to `score`, the `title_pick_*` fixtures, and `pad_score` (D-4).
   - Each `.txt` rewritten for the converted frames ("R1 R1 R1 (`build`, `end turn`, `tech`)" → "R1: `tech`"), with its "N frames." and any long-exception reason updated.
3. **The step note** in `docs/plan-12.md`, "### Step 11":
   - the ratified G-1 to G-5 and their refinements;
   - `pad_nav.c`'s line count (1219 before) and `pad_nav_t`'s field count (26) before and after, and the reason no `pad_<mode>_t` was added (G-3);
   - the tests rewritten, and any deleted next to its mechanic;
   - the replays and fixtures broken and re-authored per chunk, frame counts before and after, and the agent minutes;
   - any chain whose freed frames moved (G-2);
   - the situations dropped (ring passes, by replay and frame) and added (`pad_score`'s R1 on `score`);
   - the cheat verdict, the `make coverage-compare` result, and the `make test` and `make e2e` wall times before and after;
   - the note for step 12 (see Further Notes).

## Testing Decisions

- **A good test** drives public behaviour only.
  - Pad tests drive `run_frame`, `tap` and `hold`, then assert on `pad_nav_tech_open`, `pad_nav_score_open`, `pad_nav_build_bar_open`, `pad_nav_tab` (ring tests only), `pad_nav_jump_mode`, `pad_nav_aim_mode`, `pad_nav_hovered`, `pad_nav_placing`, the played frame's mouse, `assert_clicked`, the camera, `ui_left_held`, the ui's ephemerals, `score_panel_shown`, `backdrop`, the world counts, research and credits, and `p8_fixed_rnd_state()`.
  - No test reads `pad_nav_t` fields.
- **Prior art** in `tests/test_pad_nav.c`:
  - helpers: `go_to_tab`, `open_score_from_the_map`, `assert_shoulders_wait`, `take_structure`, `open_build_bar`;
  - `test_landing_on_tech_opens_its_panel_and_an_invention_returns_to_the_map`;
  - `test_r1_on_tech_closes_the_box_and_shows_score_on_the_same_frame`, `test_r1_on_score_hides_it_and_returns_to_the_map`;
  - `test_a_tab_whose_button_is_gone_returns_to_the_map`;
  - `test_shoulders_cancel_the_carried_structure_then_switch`, `test_shoulders_close_a_planet_menu_then_switch`, `test_shoulders_in_aim_mode_switch_and_come_back_to_it`;
  - `test_ordinary_cross_triangle_and_circle_play_never_types_the_cheat`.
- **Rewritten** (surviving behaviour; each keeps its intent):
  - Step 10's forward-ring test (R1 round the five tabs) becomes "R1 cycles map → tech → score → map; from the bar, R1 goes to end turn, then tech".
  - `open_score_from_the_map` (`go_to_tab(TECH)`, then R1) needs no change: under the new mapping it is R1 R1 from the map. The tests that call it keep their assertions.
  - **Tests that press R1 from the map and meant "the build tab"** switch to L1. **Tests that meant "any shoulder from the map"** keep R1 and expect `tech`, with its landing on the next frame and `assert_clicked(step, 2)`:
    - the carry cancel, the menu close, aim mode surviving a visit, the year box under a shoulder;
    - the slipway cancelled by a tab change, no pad selection behind a tab;
    - `test_the_ui_pad_tab_is_the_tab_hud_button`, `test_a_new_game_starts_on_the_map`;
    - the restart under pause.
  - `go_to_tab(END_TURN)` reaches the tab through the bar (chunk 1). The 13 end-turn tests keep their assertions.
- **Restored:** the landing-after-shoulder-cancel test, untested since step 10. R1 while carrying a structure: the cancel frame is at the icon point with the button up, nothing is clicked, and the landing clicks the tech button only once the cancel's frame is past. The tech panel is then up, the structure dead, and the credits unchanged.
- **New** (chunk 2):
  1. R1 from jump mode on a hovered colony. On the tab-change frame `pad_nav_tech_open` is 1, the cursor is on the tech button and nothing is pressed. The next frame clicks it and the panel is up. No frame has `pad_nav_build_bar_open` or the end-turn tab.
  2. R1 R1 R1 from jump mode: `tech`, `score`, closed. Back in jump mode on the same hovered object, with the camera unmoved and the score backdrop gone the frame after.
  3. The same from aim mode: aim mode comes back, the camera unmoved, the pan restarting slow.
  4. ○ on `tech` and ○ on `score` return to the map in one press, back in the mode left.
  5. R1 in slipway mode cancels the slipway (nothing built or charged) and opens `tech`. R1 under a planet menu closes it and opens `tech`. R1 under the year box opens `tech` and keeps the year box.
  6. R1 waits as shoulders do: on a click's release frame and on the frame after a slipway release (`assert_shoulders_wait` with R1). Under pause, with no game and on the final score panel, it does nothing.
  7. A × held through R1's frame invents nothing on the landing or after, until released and pressed again. If today's tech tab invents in that case, the test pins today's behaviour instead, and the step note records it as a finding for step 12.
  8. L1 on `tech` and on `score` opens the bar (step 10's cases, kept unchanged).
  9. The cheat: the long ordinary sequence of D-5 never fires it.
  10. R1 cycles leave `p8_fixed_rnd_state()` unchanged.
- **Kept unchanged:** every placement, pause, build-bar, tab-panel, year-box, jump, aim, slipway, Select and ○-order test.
- **Before each commit:** `make test-all`, the checks under Implementation Chunks, and `make raylib` (it compiles and links only; the keymap is unchanged).
- **At the end:** `make full-game`, `make save-worst-case` and `make save-fuzz` once. They are knowingly stale (their scripts press the old ring), so only an assert or a crash matters. Also compare the `make test` and `make e2e` wall times before and after.
- **Not tested:** misuse asserts.

## Out of Scope

- Step 10's L1 build bar and its placement (done, `a65a828`).
- Removing `pad_nav_tab_t`, the end-turn tab, network mode and the ring's remaining code. Deciding what R1 on the bar does once the ring is gone. The HUD highlight following the open panel. Re-authoring `pad_end_turn`'s ring year. All of this is step 12.
- Any change to what the tech and score panels contain, how their rows are navigated, or what an invention costs.
- Re-authoring the `worst-case/` games (step 13). They press R1 and L1 in the old sense.
- Step 14's metrics tool and the README Controls section (step 15).
- Tuning values, audio, PSX, mouse input.

## Further Notes

- **For step 12:**
  - After step 11, the ring's only transitions left are R1 on the bar → `end turn` and R1 on `end turn` → `tech` (G-1). Only `pad_end_turn` f16–44 and the 13 `go_to_tab(END_TURN)` tests use them.
  - Deleting `pad_nav_tab_t` moves the tech and score "open" state out of `nav->tab`. `pad_nav_tech_open`, `pad_nav_score_open` and `pad_nav_build_bar_open` keep their contracts, so the tests written against them need no rewrite.
- **For step 14:**
  - R1 R1 R1 with nothing done opens and closes two panels: not a dead press.
  - R1 on the bar reaching `end turn` is a ring transition that step 12 removes. It does not exist in the final scheme.
