---
title: "Remove the tab ring and network mode"
description: "Every ring tab now has a direct button and network mode can no longer be entered, but pad_nav still carries the five-tab ring, the end-turn tab and network mode's code and fields; delete them, let the HUD highlight follow the open panel, and pay off phase 2's naming and duplication findings."
status: needs-triage
---

## Plan step (verbatim)

Plan file `docs/plan-12.md`, Phase 4 — Direct buttons instead of the tab ring, step 12:

> 12. **Remove the tab ring and network mode:** `pad_nav_tab_t` and the network fields go, and the HUD highlight follows the open panel. Remaining replays (`pad_title`, `pad_pause_restart`, `pad_message_box`, `pad_bankrupt`, `pad_game_end`, `pad_restart_resume`, `pad_resume_at_start`) re-authored where they broke. `tests/test_pad_nav.c` keeps only tests of the new scheme, under section 2's test-deletion rule. After the deletion, separate pure-refactor commits (every hash and `.end-state` unchanged) fix phase 2's leftover findings: `start_new_game` and `jump_ran` misnamed, `pad_jump_t` holding state only `pad_nav.c` uses, `press_start_again` duplicating `cross_starts_slipway`'s press, and `pad_jump.c`'s direction table duplicating `pan_directions`; the step note lists them.

The section 2 rules it follows, quoted:

> - **R1** cycles tech panel → score panel → closed.
> - The tab ring (`pad_nav_tab_t`, map/build/end turn/tech/score) and network mode disappear. The HUD highlight (`ui_set_pad_tab`) marks the button of whichever panel is open.

> - From step 8 on, each new mode's state lives in its own `pad_<mode>_t`, never in `pad_jump_t` nor as loose `pad_nav_t` fields, unless the step note gives the reason; each step note records `pad_nav.c`'s line count and `pad_nav_t`'s field count before and after. A unit test may be deleted only with the mechanic it exercises, which the step removes; the step note lists each deleted test next to that mechanic, and a test of a surviving behaviour that no longer compiles is rewritten, not deleted.

> - **Code coverage never drops** (decided by the user). [...] After every step that re-authors replays, every line hit by the baseline is still hit, except lines the step deletes (old pad_nav code) or that the plan replaces. A newly unhit line means a replay must be re-authored until it is hit again. [...]

> - A re-authored replay keeps every situation its old `.txt` lists (refusals, cancels, red lines, too far, duplicates and so on), produced the new scheme's way. Only situations whose mechanic the plan removes (network travel, the tab ring) may go, each noted in the step note. Checked by comparing the old and new `.txt` in the same commit.

> - Every step from 6 on that changes what × or ○ do (including the build bar and the ring's removal) checks whether ordinary input sequences, with D-pad presses between them (`cheat_step` ignores them), can complete × × ○ ○ × × ○ ○, and re-checks the deliberate cheat route of step 6. [...]

Every other guardrail of section 2 applies as written:

- `make test-all` is green on every commit, and `make test` and `make e2e` get no slower.
- The game rules don't change.
- Replays are re-authored to the step 2a end state against reference `8defcba`. An end state that changed is never re-baselined. The relaxed comparison and its exceptions file are used only if a replay cannot reach its reference.
- The frame limits (180, or 600 if listed) are never raised. Shared prefixes are re-authored first, and every broken replay is re-authored, not only the named ones.
- Pinned values of step 2a's inventory are re-derived in the same commit, each assertion keeping its intent. The re-authoring cost is recorded.
- The 8 rules of the step 2b discipline. The `worst-case/` games are knowingly stale until step 13.
- The checks outside `make test-all` run on every chunk that touches pad input or a replay: `make unit-run-test_live_slots`, `make end-state EVENTS=` on every touched replay, `make coverage-compare`, and `make rng-sync-probe` whenever `pad_colonise` changes.
- One scheme only. Never launch the raylib app.

## Facts from step 11 (done, `a6585db`)

These come from the "### Step 11" note of `docs/plan-12.md` and from the committed code, replays and fixtures at `a6585db` (`git show HEAD:<path>`). An earlier draft of this PRD held hypotheses H-11-1 to H-11-11. Each fact below says whether its hypothesis held.

- **H-11-1, held.** R1 cycles map → tech → score → closed. R1 on the bar still goes to `end turn`, and R1 on `end turn` goes to `tech` (G-1).
  - `shoulder_target`'s R1 case is `tab == MAP ? TECH : (tab + 1) % COUNT`.
  - L1 and R1 together do nothing, a held shoulder counts its press edge only, and the waits are unchanged.
- **The landing:** the tech landing clicks on the R1 frame itself, unless a cancel is due (`clicks_due`). It then clicks one frame later. This was never a hypothesis, but this PRD's earlier draft said "the next frame", and that was wrong.
- **H-11-2, held.** `pad_nav_tech_open` and `pad_nav_score_open` exist, and both read `pad_nav_tab`. Step 11's new tests read them.
- **H-11-3, failed in its number.** `go_to_tab` reaches `end turn` through the bar: L1 if the bar is closed, then R1. It is called with `END_TURN` **14** times, not 13.
  - The extra call is in step 11's new `test_tech_and_score_read_open_from_the_tab_change_frame_and_closed_without_a_game`, which goes to `end turn`, then R1 to `tech`.
  - Step 11's ring test is now `test_r1_cycles_tech_score_closed_from_the_map_and_end_turn_from_the_bar`.
  - `test_landing_on_tech_opens_its_panel_and_an_invention_returns_to_the_map` still goes through `go_to_tab(END_TURN)`.
- **H-11-4, held.** `pad_end_turn` is 139 frames: L1 at f16, R1 at f18 (`end turn`), × held f21–40, ○ at f44, Select from f46.
  - It holds the only R1 on the bar → `end turn` among all replays and fixtures.
- **H-11-5, held.**
  - `pad_message_box` (223 frames): L1 at f96 and f98, R1 at f150 (`tech`), R1 at f155 (`score`).
  - `title_pick_*` (49 frames): R1 at f20 and f25.
  - `pad_score` is **96 frames**, with R1 at f20, f25, f45, f50, f66, f77, f82 and f88, and L1 at f56 and f60. None of them is on the bar.
  - 5 hashes were re-baselined in step 11, and every capture is identical from each landing on.
- **H-11-6, held.** `finish_step`'s `ui_set_pad_tab(tab_button(pad_nav_tab(nav)))` is unchanged.
- **H-11-7, held.** `pad_nav_t` has 26 fields, and `pad_nav.c` has **1227** lines (1219 → 1227). `tests/test_pad_nav.c` has **224** tests (216 → 224), none deleted. These are step 12's "before" figures.
- **H-11-8, held.** The same 69 unhit lines.
- **H-11-9, held.** No `.end-state` was re-baselined. Every cheat count is unchanged, and there is no exceptions file.
- **H-11-10, failed.** The "× held through R1" case was not carried forward as a finding: step 11 fixed it (`3b4caf8`).
  - `map_panel_open` became `armed_panel_open`: a panel on the map tab, or the panel of `tech` or `score` (`opens_a_panel`), outside pause.
  - The arm rule (`menu_cross_armed`) now keeps a held × off the tech header and the score panel until × is pressed again. Pinned by `test_cross_held_through_r1_presses_nothing_on_tech_until_pressed_again` and `test_cross_held_through_r1_r1_presses_nothing_on_score`.
  - The fix predates step 11: the old ring had the same bug.
  - **The ring deletion must keep this behaviour** (D-1).
- **H-11-11, held.** The ring's only transitions left are R1 on the bar → `end turn` and R1 on `end turn` → `tech`. Only `pad_end_turn` f16–44 and the end-turn unit tests use them.
- **Step 11's own finding:** a test that starts a second game in the same process (`start_title` twice) died on `zone ui button full`. It was worked around with a forked copy. Step 12's new-game tests use `cart_init` or a forked copy, as the passing ones do.

## Facts from the code (read at `a6585db`, `git show HEAD:<path>`)

- **The ring:**
  - `pad_nav_tab_t`: map, build, end turn, tech, score.
  - `pad_nav_tab()`, `set_tab`, `opens_a_panel`, `shoulder_target`, `shoulder_switch` and `leave_tab`.
  - `tab_button` maps end turn to HUD button 1, tech to 2 and score to 3.
  - `tab_cursor`, `tab_step` with its end-turn × press (`cross_armed`), and `tabs_can_switch`.
  - `own_panel_up` and `foreign_panel_open`, which read the tab.
  - `armed_panel_open` (step 11's `3b4caf8`): a panel on the map tab, or the tech or score tab's panel, outside pause. It decides whether `menu_cross_armed` applies, so a held × presses no row there until pressed again.
  - The landing (`landing_due`) and the "own panel gone → map" return.
  - `finish_step` clears `cross_armed` under pause and writes `ui_set_pad_tab(tab_button(pad_nav_tab(nav)))`.
  - `select_ends_year` reads `tab_button(PAD_NAV_TAB_END_TURN)` for the year-skip button. The button survives the tab: it is HUD button 1.
- **"Bar open" is `tab == BUILD`** (step 10's D-3). `pad_build_t` holds only the focused icon.
- **Network mode:**
  - `PAD_NAV_MAP_NETWORK`;
  - the fields `selection` and `selected_slipway`;
  - `pad_nav_selection`, `pad_nav_selected_slipway` and `pad_nav_next_slipway` (with `heading_t`, `in_left_half`, `turns_before` and `SLIPWAYS_CAP`);
  - `network_step`, `travel_to`, `click_planet`, `is_clickable_planet`, `centre_camera_on` and `drop_stale_map_mode`;
  - `map_step`'s network branch, `tab_cursor`'s selection branch, `begin_placement`'s selection branch and `enter_aim_mode`'s two selection resets.

  No path sets `map_mode` to network since step 7.
- **Network mode outside `pad_nav`:**
  - The ui's `pad_slipway` field, `ui_pad_slipway`, and the `slipway` parameter of `ui_set_pad_selection`.
  - `slipway_display`'s colour-12 line for the selected slipway, unhit since step 7.
- **`selection_frames`** is the blink counter of the hovered object's marker (`show_selection`), not network state. It survives.
- **The HUD:** 4 galaxy-menu buttons: 1 year skip, 2 tech, 3 score, 4 pause. The bar has no HUD button. The ui draws `ui_pad_tab`'s button as hovered (`ui_button`).
- **Phase 2 findings, at `a6585db`:**
  - `start_new_game` starts nothing. It detects a new game (the main economy's id changed) and resets the nav.
  - `jump_ran` is also set by slipway mode. It means "the hold-repeat count survives this frame".
  - `pad_jump_t` has `hovered`, `game` and `seeded`, which no function in `pad_jump.c` reads or writes beyond init. `pad_nav.c` alone uses them.
  - `press_start_again` and `cross_starts_slipway` both move the cursor to the start's screen point and press the left button, after their own guards.
  - The order Up, Down, Left, Right is spelled three times: `pad_jump.c`'s `directions_by_priority`, `pad_nav.c`'s `pan_directions`, and `pressed_direction`'s local `order`.
- **Tests touching the ring or network mode** in `tests/test_pad_nav.c` (224 tests at `a6585db`).
  - `go_to_tab` taps L1 (if the bar is closed), then R1, for `END_TURN`. For `TECH` and `SCORE`, it taps R1 from the map.
  - The 14 `go_to_tab(PAD_NAV_TAB_END_TURN)` calls are in:
    - `test_end_turn_focuses_and_hovers_the_year_skip_button`
    - `test_cross_on_end_turn_ends_one_year_per_press`
    - `test_a_new_game_starts_on_the_map`
    - `test_final_score_ignores_shoulders_circle_and_start`
    - `test_shoulders_wait_under_the_pause_menu`
    - `test_landing_on_tech_opens_its_panel_and_an_invention_returns_to_the_map`
    - `test_a_tab_whose_button_is_gone_returns_to_the_map`
    - `test_pause_from_end_turn_and_build_keeps_the_tab_and_its_cursor`
    - `test_cross_on_restart_gives_the_title_then_a_new_game_on_the_map`
    - `test_cross_held_into_pause_presses_no_row_until_pressed_again`
    - `test_the_cross_that_closes_pause_on_end_turn_ends_no_year`
    - `test_circle_on_end_turn_returns_to_the_map_under_the_year_box`
    - `test_pause_a_tab_and_a_slipway_keep_the_last_object`
    - `test_tech_and_score_read_open_from_the_tab_change_frame_and_closed_without_a_game` (step 11)
  - Other tests that reach `end turn` by R1 on the bar:
    - `test_r1_cycles_tech_score_closed_from_the_map_and_end_turn_from_the_bar` (step 11's ring test)
    - `test_cross_held_on_arrival_ends_no_year`
    - `test_a_press_that_hands_over_no_structure_stays_on_build`
    - `test_r1_on_the_bar_goes_to_end_turn_both_shoulders_do_nothing_and_a_held_l1_toggles_once`
  - Tests with `END_TURN` in a tab list: `test_circle_on_a_tab_returns_to_the_map`, `test_select_does_nothing_while_something_is_held_or_open` and `test_l1_on_end_turn_tech_and_score_opens_the_bar_and_hands_over_nothing`.
  - The network pure tests are `test_slipways_turn_clockwise_from_straight_up` and `test_straight_up_splits_the_two_halves`.

## Decisions (ratified by `guideline`)

`guideline` ratified R-1 to R-8 as recommended, with the refinements included below. The step note records them as decided.

### R-1. What R1 does on the bar once the ring goes

- **Decided: R1 on the bar closes it and opens `tech`.** The landing follows the same rule as R1 from the map.
  - R1 then means "the tech → score → closed cycle" everywhere, and L1 means "the bar" everywhere. Step 10 made L1 on `tech` or `score` open the bar, so the two shoulders mirror each other.
  - No press goes dead. Step 14 counts dead presses, and section 4 wants none outside the locked group.
  - The bar has no panel to leave, so the bar closes with no leaving click.
  - **Refinement (`guideline`): "the landing click comes one frame later, as from the map".** The two halves conflict with the code at `a6585db`. There the landing from the map clicks on the R1 frame itself, and one frame later only while a cancel or a release is due (`clicks_due`). The step 11 note says so; this PRD's earlier draft wrongly said "the next frame".
    - **Applied as "as from the map":** R1 on the bar runs the map's landing rule. Closing the bar cancels nothing, so the click lands on the R1 frame unless a release is due.
    - The step note records the discrepancy. If `guideline` meant one frame later literally, that is a second landing rule for the bar alone, and it needs a new ruling.
- Rejected: R1 does nothing on the bar. That creates a dead press in a mode the player enters often.
- Rejected: R1 closes the bar only. That duplicates L1 and ○, and contradicts "R1 cycles tech → score → closed".

### R-2. The end-turn tab and `pad_end_turn`

- **Decided:**
  - Delete the end-turn tab with the ring: its HUD focus, its × press and `cross_armed` (refinement: `cross_armed` goes too). A year then ends only by the Select hold (and step 9's chain). The year-skip button stays a HUD button, pressed by the Select hold.
  - `pad_end_turn` keeps its name and its end state (two years). Its first year becomes a Select hold of 30 frames in aim mode, in place of L1 / R1 / × held / ○ at f16–44.
  - **Trap:** the next Select tap (old f46) must wait out step 9's 15-frame chain window after that year. Otherwise it ends a third year. Every later frame shifts by the same amount. The probe and the fold keep their world points (rule 2). The file is about 157 frames, under 180: 139, minus the 29 ring frames, plus about 31 for the hold and about 16 for the window.
  - **Situations dropped with the ring, each noted:** L1 then R1 to `end turn`, the year skip lit as a tab, and × held on `end turn` for one year. Kept: one hold, one year (f71–110 already shows it; the new first year shows it again).
- Rejected: renaming the file. That churns its `.end-state`, `.sha256` and inventory entries for no behaviour.

### R-3. The end-turn unit tests

- **Decided:** delete the four tests whose mechanic is the end-turn tab itself. Rewrite every other test that reached `end turn` only as "some tab", onto the bar or `tech`, keeping its assertions' intent.
- **Refinement:** the step note lists each deleted test next to its mechanic.
- **Deleted, with the end-turn tab:**
  - `test_end_turn_focuses_and_hovers_the_year_skip_button`
  - `test_cross_on_end_turn_ends_one_year_per_press`
  - `test_cross_held_on_arrival_ends_no_year`
  - `test_the_cross_that_closes_pause_on_end_turn_ends_no_year`

  Also the end-turn half (the forked copy) of `test_pause_from_end_turn_and_build_keeps_the_tab_and_its_cursor`. Its bar half survives, renamed for the bar.
  - The year-end intents live on in the Select tests: one hold, one year; a refused press is never retried; the hold under the year box.
  - The "× held from pause acts on nothing beneath" intent lives on in `test_start_and_circle_close_pause_pressing_nothing_on_the_map` and `test_cross_held_into_pause_presses_no_row_until_pressed_again`.
- **Rewritten:**
  - `test_a_new_game_starts_on_the_map`: from `tech`.
  - `test_final_score_ignores_shoulders_circle_and_start`: the last year ended by the Select hold on the map. The final score panel is then on the map, and L1, R1, ○, Start, △ and □ leave it unchanged.
  - `test_shoulders_wait_under_the_pause_menu`: pause opened from the bar.
  - `test_landing_on_tech_opens_its_panel_and_an_invention_returns_to_the_map`: R1 from the map (step 11 left it on `go_to_tab(END_TURN)`).
  - `test_tech_and_score_read_open_from_the_tab_change_frame_and_closed_without_a_game`: R1 from the map, and also R1 from the bar (R-1). The accessor contracts are unchanged.
  - `test_a_tab_whose_button_is_gone_returns_to_the_map`: R1 from the map with the tech button dead. Renamed for the panel.
  - `test_cross_on_restart_gives_the_title_then_a_new_game_on_the_map`: from the bar.
  - `test_cross_held_into_pause_presses_no_row_until_pressed_again`: from the map.
  - `test_circle_on_end_turn_returns_to_the_map_under_the_year_box`: ○ on the bar and on `tech` under the year box returns to the map, and the year box stays. Renamed.
  - `test_pause_a_tab_and_a_slipway_keep_the_last_object`: a bar visit.
- **Edited** (the end-turn element or assertion removed, the rest unchanged):
  - `test_circle_on_a_tab_returns_to_the_map`
  - `test_select_does_nothing_while_something_is_held_or_open`
  - `test_l1_on_end_turn_tech_and_score_opens_the_bar_and_hands_over_nothing` (renamed)
  - `test_a_press_that_hands_over_no_structure_stays_on_build` (its last R1 now opens `tech`, R-1)
  - `test_r1_on_the_bar_goes_to_end_turn_both_shoulders_do_nothing_and_a_held_l1_toggles_once` (renamed; R1 on the bar opens `tech`)
  - `test_r1_cycles_tech_score_closed_from_the_map_and_end_turn_from_the_bar`: "from the bar, R1 goes to end turn, then tech" becomes "from the bar, R1 opens tech". Renamed.
- Rejected: keeping a test "end turn is gone". It asserts an absence, which no future change could break.

### R-4. Coverage of the deleted network lines

- **Decided:**
  - `make coverage-compare` reports the network-mode lines of `pad_nav.c` and `slipway_display.c`'s colour-12 line as *deleted*. Section 2 exempts lines the step deletes.
  - After chunk 1, the unhit set is exactly the H-11-8 set minus the deleted lines. That is expected to leave `save_state.c`'s point-ended-slipway skip alone. It has been unhit since step 7, its mechanic (the old drag over empty space) was replaced by the plan, and this step does not try to re-hit it.
  - Every other chunk ends with that same set. Lines deleted with the end-turn tab count as deleted.
  - **Refinement:** a line newly unhit in *any* chunk (the deletions, the highlight, the replays or a refactor) is hit again before that chunk commits.
- Rejected: a new unit test or replay to hit `save_state.c`'s skip now. It is a save path for slipways no pad route can make. It belongs with step 13's worst-case games or after the plan.

### R-5. Where the tech/score state lives

- **Decided:** a new `pad_cycle_t` module, in the style of `pad_build`, `pad_back` and `pad_select`. It holds which cycle panel is open (closed, tech or score) and whether its landing click is due. The bar's open flag moves into `pad_build_t` (step 10's D-3). Neither lives in `pad_nav_t` or `pad_jump_t`.
  - **Refinement:** `pad_cycle` is a pure module with its own tests. An assert in `pad_nav` states that the bar and a cycle panel are never open together.
- Rejected: one "open panel" enum (none, bar, tech, score) in `pad_nav_t`. It is the ring minus two entries, against the section 2 rule and step 10's D-3.

### R-6. The HUD highlight's timing and scope

- **Decided:**
  - The highlight is the tech button while the cycle panel is `tech`, and the score button while it is `score`. It is lit from the R1 frame that opens the panel, through the landing, until the panel closes, exactly the frames `tab_button(tab)` lights today. No capture changes on any tech or score frame.
  - There is no build button on the HUD (the 4 galaxy-menu buttons are year skip, tech, score and pause), so nothing lights for the bar.
  - Nothing is pad-lit with the cycle closed: on the map, on the bar, while carrying, on the title, or on the final score panel. The year skip is never pad-lit: only the end-turn tab lit it.
  - **Refinement:** pause is not special-cased. The rule reads only the cycle panel. "Nothing lit under pause" is a test case only, for pause opened from the map or from the bar (pause cannot open over `tech` or `score`).
- Rejected: lighting from the landing frame (the panel actually drawn). It changes one frame per R1 visit in about ten replays, re-baselining hashes for no player benefit.

### R-7. `pad_jump_t`'s state that only `pad_nav.c` uses

- **Decided:**
  - `hovered` and `seeded` stay in `pad_jump_t`, and `pad_jump.c` gains the functions that own them: hover an object, read the hovered candidate, and reseed from a candidate list and a centre point. These take over `seed_hovered`'s and `leave_aim_mode`'s seeding logic.
  - `game`, the new-game key, moves to `pad_nav_t` (refinement), because it resets every mode (jump, back, select, slipway), not jump mode alone.
  - The step note gives the reason, as section 2 requires for a loose `pad_nav_t` field.
- Rejected: a separate `pad_game_t` for one id. It is a struct for one field.
- Rejected: moving `hovered` to `pad_nav_t`. That is a loose field for jump mode's own state, which section 2 forbids.

### R-8. Network mode's ui and draw

- **Decided:** delete them in chunk 1 (refinement), with network mode:
  - the ui's `pad_slipway` field, `ui_pad_slipway`, and `ui_set_pad_selection`'s `slipway` parameter;
  - `slipway_display`'s colour-12 branch.

  They are network mode's "selected slipway drawn in colour 12", which step 7 already listed as dropped with the mechanic. No replay draws it (unhit since step 7), so every hash is unchanged.
- Rejected: keeping them as dead code. The plan's "network mode disappears" covers them, and they would stay unhit.

## Problem Statement

From the player's side, the ring's last pieces are traps. Since step 11, tech and score have R1, the bar has L1, and the year ends on the Select hold. But R1 on the build bar still detours to an end-turn tab that only lights a button the Select hold already presses. Network mode can no longer be entered, but its code, fields and drawing are still there, unhit and unexplained.

From the developer's side:

- `pad_nav_t` still carries a five-tab enum, a landing flag and an end-turn arming flag for one remaining transition, plus two dead network fields.
- About a hundred lines of `pad_nav.c` are network code no input reaches. They are the bulk of the 69 lines `make coverage-compare` reports unhit.
- Every new pad test has to work around `pad_nav_tab`.
- Phase 2 left five findings: two misleading names, jump-mode state `pad_jump.c` never touches, a duplicated slipway press, and three copies of the D-pad priority order.

## Solution

**For the player:**

- R1 on the map opens tech, on tech shows score, on score closes. R1 on the build bar also opens tech (R-1).
- L1 opens and closes the build bar from anywhere.
- ○ backs out of the bar, tech and score in one press.
- There is no end-turn tab: Select held 30 frames ends the year, as it has since step 9.
- The HUD lights the tech button while the tech panel is up and the score button while the score panel is up, and nothing else (R-6).
- Network mode is gone, along with its blue selected-slipway line, which nothing could show any more.

**For the developer:**

- `pad_nav_tab_t`, `pad_nav_tab()`, the end-turn tab and every network-mode function, field and draw are deleted.
- The bar's open flag lives in `pad_build_t`, and the tech/score cycle in a new `pad_cycle_t` (R-5).
- `tests/test_pad_nav.c` keeps only tests of the new scheme: six tests are deleted with their mechanics (R-3), and the rest are rewritten onto the bar, tech or the map.
- Four separate refactor commits, each leaving every hash and `.end-state` unchanged, fix phase 2's five findings.

## User Stories

1. As a player, I want R1 on the build bar to open the tech panel, so that R1 always means "tech, then score, then closed".
2. As a player, I want R1 on the map to open the tech panel in one press, as step 11 made it, so that nothing I learned changes.
3. As a player, I want R1 on the tech panel to show the score panel, so that I can glance at both with one button.
4. As a player, I want R1 on the score panel to close it, back where I was on the map, so that the cycle ends where it started.
5. As a player, I want L1 to open the build bar from the map, tech or score, and close it from the bar, so that L1 always means "build".
6. As a player, I want ○ on the bar, tech or score to return to the map in one press, so that ○ always backs out.
7. As a player, I want no end-turn tab to stumble into, so that no press lands on a screen that only lights a button.
8. As a player, I want Select held 30 frames to remain the way to end the year, so that ending a year stays deliberate.
9. As a player, I want the tech button lit while the tech panel is up, so that the HUD tells me where I am.
10. As a player, I want the score button lit while the score panel is up, so that the HUD tells me where I am.
11. As a player, I want nothing lit on the map or the build bar, so that a lit button always means an open panel.
12. As a player, I want the year-skip button never to look selected unless my Select hold presses it, so that the HUD never suggests a mode that no longer exists.
13. As a player, I want R1 on the bar to close the bar before opening tech, so that two panels never cover each other.
14. As a player, I want R1 while carrying a structure to cancel it at no cost and then open tech, as since step 11, so that nothing half-done stays behind.
15. As a player, I want R1 and L1 together to do nothing, so that a fumble does nothing.
16. As a player, I want the shoulders to wait under pause, on a click's release frame and on the frame after a slipway release, as today, so that one press never acts twice.
17. As a player, I want the final score panel to ignore the shoulders, ○, Start, △ and □, as today, so that the game's end cannot be skipped by accident.
18. As a player, I want a new game after a restart to start on the map in jump mode, whatever panel was open, so that every game starts the same way.
19. As a player, I want the build bar to remember its icon across a close, until a new game, as today, so that reopening it is quick.
20. As a player, I want pause from the bar to keep the bar and its focused icon, as today, so that pausing costs me nothing.
21. As a player, I want ○ on the bar or on tech under the year box to return to the map and keep the year box, so that the banner is not lost.
22. As a player, I want no ordinary sequence of R1, L1, ×, ○ and the D-pad to type the cheat, so that my game stays honest.
23. As a player, I want the deliberate cheat route (four × + ○ chords) to still work, so that the dev tool stays usable.
24. As a replay author, I want `pad_end_turn` to end its first year by the Select hold and keep its two-year end state, so that its reference still holds.
25. As a replay author, I want `pad_end_turn`'s next Select tap to wait out the 15-frame year chain, so that it does not end a third year.
26. As a replay author, I want every other replay's `.json.sha256` and `.end-state` unchanged by the deletion, so that the deletion is shown to change nothing else.
27. As a replay author, I want each named remaining replay (`pad_title`, `pad_pause_restart`, `pad_message_box`, `pad_bankrupt`, `pad_game_end`, `pad_restart_resume`, `pad_resume_at_start`) checked and re-authored only where it broke, so that no file churns for nothing.
28. As a replay author, I want each `.txt` that still describes the ring or network mode as current corrected, so that the notes say what is true.
29. As a reviewer, I want each dropped situation (L1 R1 to `end turn`, the lit year skip as a tab, × held on `end turn`) listed in the step note with its mechanic, so that only removed mechanics go.
30. As a reviewer, I want `make coverage-compare` to report the network lines as deleted and nothing newly unhit, so that coverage does not drop.
31. As a reviewer, I want every deleted test listed next to the mechanic it exercised, so that no surviving intent is lost.
32. As a reviewer, I want each refactor in its own commit with every hash and `.end-state` unchanged, so that each one is shown to be behaviour-free.
33. As a developer, I want `pad_nav_t` free of the tab enum, the landing flag, the end-turn arming flag and the network fields, so that it holds only the new scheme's state.
34. As a developer, I want the bar's open flag in `pad_build_t`, so that the bar owns its state, as step 10 planned.
35. As a developer, I want the tech/score cycle in its own `pad_cycle_t` with a pure "next panel" rule, so that R1's cycle is unit-tested without a game.
36. As a developer, I want the HUD highlight computed from the open cycle panel by one function, so that it has one source.
37. As a developer, I want `pad_nav_tech_open`, `pad_nav_score_open` and `pad_nav_build_bar_open` to keep their contracts, so that the tests written against them need no rewrite.
38. As a developer, I want `pad_nav_tab()` gone from the header, so that no new test can depend on the ring.
39. As a developer, I want `start_new_game` renamed for what it does, which is to reset the nav when a new game is detected, so that a reader does not look for a game being started.
40. As a developer, I want `jump_ran` renamed for what it means, which is that the hold-repeat count survives this frame, so that slipway mode setting it no longer looks like a bug.
41. As a developer, I want `pad_jump.c` to own the functions that read and write `hovered` and `seeded`, and the new-game key to live in `pad_nav_t` with a stated reason, so that `pad_jump_t` holds only state its module uses.
42. As a developer, I want one helper for "cursor on the slipway start and press the left button", shared by `cross_starts_slipway` and `press_start_again`, so that the press is written once.
43. As a developer, I want the Up, Down, Left, Right priority order defined once, so that the jump repeat, the aim pan and the pressed direction cannot drift apart.
44. As a developer, I want `make test` and `make e2e` no slower, so that the fast loop stays fast.
45. As the agent of step 13, I want the step note to say which `worst-case/` scripts press R1 on the bar or reach `end turn`, so that I know what to re-author.
46. As the agent of step 14, I want the step note to confirm that R1 on the bar is no dead press, so that the metrics tool counts it correctly.
47. As the agent of step 15, I want the step note to list the final shoulder mapping, so that the README Controls table is written from fact.

## Implementation Decisions

### D-1. The shoulders after the ring

| Where | R1 alone | L1 alone | ○ |
|---|---|---|---|
| Map (jump, aim, slipway, planet menu) | unwind, open `tech` | unwind, open the bar | the ○ order (unchanged) |
| Bar | close the bar, open `tech` (R-1) | close the bar | close the bar |
| `tech` | close its box, show `score` on the same frame | close its box, open the bar | close, back to the map |
| `score` | the backdrop click: closed | the backdrop click, open the bar | the backdrop click: closed |

- **Unchanged:**
  - Both shoulders together do nothing, and a held shoulder counts its press edge only.
  - The waits: a foreign panel, a click's release frame, a due landing, and the frame after a slipway release.
  - The unwind on the switch frame: a planet menu closed, a carry cancelled at `cancel_point`, a slipway dropped.
  - The landing click: on the switch frame itself, or later while a release or a cancel is due (`clicks_due`).
  - When an open cycle panel's own panel is gone (an invention, the backdrop hidden), it closes back to the map by itself.
  - Under pause, with no game and on the final score panel, the shoulders do nothing.
- **The switch frame** runs no map step and no bar step, as today, so one press never acts twice.
- **The functions keep their roles under panel names.** `shoulder_target` becomes a decision over (bar open, cycle panel). `set_tab`, `leave_tab`, `tab_cursor`, `tab_step`, `tab_button`, `tabs_can_switch`, `own_panel_up` and `opens_a_panel` lose "tab" from their names and read `pad_build_t` and `pad_cycle_t`.
- **The bar's step** (`build_step`) and **the cycle panels' step** (`panel_step`) are what was `tab_step`. With no end-turn tab, `tab_step`'s × press and `cross_armed` go. So does `finish_step`'s pause clear of `cross_armed`.
- **"On the map"** now means: a game runs, the bar is closed, and no cycle panel is open. It is a private predicate. `pad_nav_aim_mode`, `pad_nav_jump_mode`, `planet_menu_open`, `select_allowed` and the ○ year-box rung read it in place of `tab == MAP`. The header contracts that say "on the map tab" are reworded the same way.
- **Step 11's held-× fix (`3b4caf8`) is kept.** `armed_panel_open` becomes "a panel on the map, or the open cycle panel's panel, outside pause". The `menu_cross_armed` rule then still keeps a held × off the tech header and the score panel, whether R1 came from the map or from the bar (R-1).
  - Its two tests stay unchanged: `test_cross_held_through_r1_presses_nothing_on_tech_until_pressed_again` and `test_cross_held_through_r1_r1_presses_nothing_on_score`.
  - A new case in chunk 2 covers a × held through R1 on the bar.
- **Placement:** `begin_placement` closes the bar where it set the map tab. Its network-selection branch goes (R-8, chunk 1).

### D-2. `pad_cycle` (R-5)

A new pure module beside `pad_build`, with tests of its own. Type shape (the decision, not a working implementation):

```c
typedef enum { PAD_CYCLE_CLOSED, PAD_CYCLE_TECH, PAD_CYCLE_SCORE } pad_cycle_panel_t;

typedef struct {
    pad_cycle_panel_t panel;
    int landing_due;
} pad_cycle_t;

void pad_cycle_init(pad_cycle_t *cycle);
/* Sets landing_due for TECH and SCORE, clears it for CLOSED. */
void pad_cycle_set(pad_cycle_t *cycle, pad_cycle_panel_t panel);
/* CLOSED -> TECH -> SCORE -> CLOSED. */
pad_cycle_panel_t pad_cycle_next(pad_cycle_panel_t panel);
/* The galaxy-menu index of the panel's HUD button: 2 for TECH, 3 for SCORE, 0 for CLOSED. */
int pad_cycle_hud_button(pad_cycle_panel_t panel);
```

- `pad_nav_tech_open` and `pad_nav_score_open` read `cycle.panel` and keep step 11's contracts: 0 with no game, 1 from the frame R1 opens the panel.
- A new game and `pad_nav_init` reset the cycle to closed.

### D-3. `pad_build_t` holds the bar's open flag

- `pad_build_t` gains the open flag, with functions to open, close and read it. The focus is untouched by either, so the bar keeps remembering its icon across a close, until a new game (`test_the_bar_remembers_its_icon_across_a_close_until_a_new_game`).
- `pad_nav_build_bar_open` keeps its contract (0 with no game) and reads it.
- `pad_nav_build_icon`'s contract changes from "on the build tab" to "while the bar is open".
- New `tests/test_pad_build.c` cases cover open, close and the focus kept.

### D-4. The HUD highlight (R-6)

- `finish_step` writes `ui_set_pad_tab` with the button `pad_cycle_hud_button(cycle.panel)` names (looked up with `galaxy_menu_find_button`), or NULL for closed, a missing button or no game.
- The ui's contract wording changes from "the active tab's HUD button" to "the open panel's HUD button". The ui's field and function names stay (`pad_tab`): renaming them touches the ui for no behaviour, outside this step's list.
- `select_ends_year` finds the year-skip button directly (HUD button 1), not through the deleted end-turn tab.

### D-5. Network mode's deletion (R-8)

- **Deleted from the pad_nav header and source:**
  - `PAD_NAV_MAP_NETWORK`;
  - the fields `selection` and `selected_slipway`;
  - `pad_nav_selection`, `pad_nav_selected_slipway` and `pad_nav_next_slipway`, with their helpers and `SLIPWAYS_CAP`;
  - `network_step`, `travel_to`, `click_planet`, `is_clickable_planet`, `centre_camera_on` and `drop_stale_map_mode`;
  - the network branches of `map_step`, `tab_cursor` and `begin_placement`, and `enter_aim_mode`'s selection resets.
- **The `map_step` assert's message** loses "network". With network mode gone, the stale-mode drop is gone too, so jump mode is what remains after placement, panels, aim and slipway.
- **`selection_frames` and `show_selection`** are renamed `marker_frames` and `show_marker`: after the deletion they serve only the hovered object's blink. `show_marker` passes no slipway.
- **Deleted from the ui:** `pad_slipway` and `ui_pad_slipway`, the `slipway` parameter of `ui_set_pad_selection`, and `slipway_display`'s colour-12 branch.
- **`centre_camera_on` really is dead.** `begin_placement` called it only for a network selection, and `network_step` only after □. Neither survives. Its deletion removes the last pad caller of a camera snap, which step 2b's rule 8 warned about.

### D-6. `pad_end_turn` re-authored (R-2)

- f0–15 unchanged (pad_probe's title X at f12, □ at f13 into aim mode).
- From f16, Select is held 30 frames in aim mode. The year press goes out on its 30th frame: year 3402, research 1, $100. Then Select is released.
- Idle frames until step 9's chain window has closed.
- Then the old f46–138 situations, shifted by one constant offset:
  - the Select tap with no colony;
  - the 20-frame hold abandoned;
  - the 40-frame hold ending year 3403 once;
  - the 6-frame Up pan;
  - the probe;
  - the refused fold, "-too far-" and no anchor line.

  The probe's and the fold's world points are unchanged (rule 2): the camera path is unchanged, because aim mode does not pan in between.
- **Its `.txt`:**
  - the first line and the f16–44 block rewritten;
  - every later frame number shifted;
  - "N frames." updated;
  - the "Dropped in plan 12 step 12" paragraph naming the three ring situations of R-2;
  - the stale "pad_research and pad_score still cycle the tabs" removed.
- **Checks:** `make end-state EVENTS=` against `8defcba` (cheat 0), `.json.sha256` re-baselined, and ≤ 180 frames.

### D-7. The remaining replays

- Each of `pad_title`, `pad_pause_restart`, `pad_message_box`, `pad_bankrupt`, `pad_game_end`, `pad_restart_resume` and `pad_resume_at_start` keeps its hash: none presses R1 on the bar or reaches `end turn` (fact H-11-5). The same is expected of every other e2e and fixture.
- A replay whose hash moves anyway is re-authored in the chunk that broke it, under the section 2 guardrails, and the step note says why it broke.
- **Text only, in chunk 4:**
  - every `.txt` sentence that describes the ring or network mode as current (not as history) is corrected;
  - `tools/e2e_long_exceptions.txt`'s `pad_message_box` reason, if it still says "the forward way round the tabs".

### D-8. The refactors (each its own commit, every hash and `.end-state` unchanged)

1. `start_new_game` becomes `reset_on_new_game`, and `jump_ran` becomes `repeat_kept`. Pure renames.
2. `pad_jump_t` (R-7):
   - `hovered` and `seeded` are written and read through new `pad_jump` functions: hover, the hovered candidate, and reseed.
   - `game` moves to `pad_nav_t` as the new-game key, with the step note's reason.
   - `tests/test_pad_jump.c` drops its read of `jump.game` and tests the new functions instead.
3. A private `press_slipway_start` helper: the start's screen point into the cursor, and the left button pressed. `cross_starts_slipway` and `press_start_again` keep their own guards (`pressable` and `left_was_down`, versus `pressed_before` only) and call it.
4. The direction order:
   - one table in `pad_jump`, exposed through a function that returns the first pressed direction in priority order;
   - an accessor by index for the aim pan, whose per-direction speed array is indexed in that order.

   `pad_nav.c`'s `pan_directions` and `pressed_direction`'s local table go.

### D-9. Sizes, reported in the step note

- `pad_nav_t`'s fields, expected 26 → 22 after chunk 2: minus `tab`, `landing_due`, `cross_armed`, `selection` and `selected_slipway`, plus `cycle`. Then 23 after refactor 2, plus `game`.
- `pad_nav.c`'s line count: 1227 before, and after each chunk.
- `tests/test_pad_nav.c`'s test count: 224 before, and after, with the six deletions of R-3 and D-5 named.

### D-10. The cheat (section 2's re-check)

- × now acts on fewer things (no end-turn press). ○ is unchanged. R1 on the bar is neither × nor ○.
- The chord stays four × + ○, and `without_chord` and `cheat_step` are untouched.
- Step 11's `test_r1_cycles_circle_exits_inventions_and_bar_visits_never_type_the_cheat` is extended with R1 on the bar and bar → tech → score → closed cycles, with D-pad presses between them. It must never fire the cheat.
- The deliberate route is re-checked by the existing chord tests, unchanged. Every replay's cheat count is checked by `make end-state`.

## Implementation Chunks

Each chunk ends on a commit with `make test-all` green, `make raylib` linking, and section 2's checks outside the suite: `make unit-run-test_live_slots`; `make end-state EVENTS=` on every touched replay (all 17 on chunks 2 and 4); `make coverage-compare`; and `make rng-sync-probe` if `pad_colonise` moved (it is not expected to).

1. **Delete network mode** (D-5, R-8). This is a pure deletion.
   - Delete the two network pure tests. Drop the `pad_nav_selection(...) == NULL` clauses from the tests that had them; the rest of each assertion stays.
   - **Hard check:** every `.json.sha256`, `.end-state` and pinned value unchanged. `make coverage-compare` shows the network lines as deleted, and no line newly unhit.
2. **Delete the ring** (D-1 to D-4, R-1, R-2, R-3).
   - `pad_nav_tab_t` and `pad_nav_tab()` go, with the end-turn tab and `cross_armed`.
   - `pad_cycle` is added with its tests, and the bar's open flag moves into `pad_build_t`.
   - R1 on the bar opens `tech`. The highlight is written from the cycle panel, frame for frame as before (R-6).
   - `go_to_tab` is replaced by helpers that open `tech`, `score` or the bar directly.
   - The test deletions and rewrites of R-3.
   - **In the same commit, the only replay the deletion breaks** (fact H-11-4: R1 on the bar exists only at its f18): `pad_end_turn` re-authored (D-6).
   - **Hard check:** every other `.json.sha256` and every `.end-state` unchanged. Coverage holds the R-4 set.
3. **The HUD highlight follows the open panel** (D-4, R-6).
   - Name the rule in one function. Put the contract wording in the ui and pad_nav headers.
   - Rewrite `test_the_ui_pad_tab_is_the_tab_hud_button`, `test_the_tech_tab_lights_the_tech_button_under_its_panel` and `test_no_pad_selection_is_shown_behind_a_tab` (renamed for panels). Add new cases: nothing lit on the bar, while carrying, under pause or on the final score panel; the score button lit from R1 on `tech`.
   - **Hard check:** every hash unchanged. If chunk 2 already wrote the rule in its final form, this chunk is only the tests and the contracts.
4. **The remaining replays** (D-7).
   - Run the seven named replays and every other e2e and fixture. Re-author any that broke (none expected).
   - Correct the `.txt` and long-exception wording.
   - Run `make full-game`, `make save-worst-case` and `make save-fuzz` once. They are knowingly stale (their scripts press the old ring), so only an assert or a crash matters. Record which `worst-case/` scripts press R1 on the bar or reach `end turn`, for step 13.
5. **Refactor: the two renames** (D-8.1).
6. **Refactor: `pad_jump_t`'s state** (D-8.2, R-7).
7. **Refactor: the shared slipway press** (D-8.3).
8. **Refactor: one direction order** (D-8.4).

   Chunks 5 to 8 are pure refactors with the same hard check each: every `.json.sha256`, `.end-state` and pinned value unchanged, `make state` peaks within their margins, coverage unchanged.
9. **The step note**, "### Step 12" in `docs/plan-12.md`:
   - the step 11 facts this PRD relied on, as re-checked at chunk 1's base;
   - the R-1 to R-8 rulings;
   - the sizes of D-9;
   - each deleted test next to its mechanic, and the rewritten tests;
   - `pad_end_turn`'s frame count before and after, and the agent minutes spent re-authoring;
   - the situations dropped (R-2);
   - the coverage result (deleted lines, the remaining unhit set);
   - the cheat verdict;
   - the `make test` and `make e2e` wall times before and after, compared like for like;
   - the four refactor commits, listed against the five findings;
   - the notes for steps 13, 14 and 15 (Further Notes).

## Testing Decisions

- **A good test** drives public behaviour only.
  - Pad tests drive `run_frame`, `tap` and `hold`, then assert on:
    - the pad_nav accessors (`pad_nav_tech_open`, `pad_nav_score_open`, `pad_nav_build_bar_open`, `pad_nav_jump_mode`, `pad_nav_aim_mode`, `pad_nav_hovered`, `pad_nav_placing`, `pad_nav_build_icon`);
    - the played frame's mouse, `assert_clicked`, the camera and `ui_pad_tab`;
    - the ui's ephemerals, `score_panel_shown`, `backdrop`, the world counts, the economy and `p8_fixed_rnd_state()`.
  - No test reads `pad_nav_t`, `pad_cycle_t` or `pad_build_t` fields through the nav. The new modules' own tests read their structs through their own functions only.
- **Tested modules:**
  - `pad_cycle`, new `tests/test_pad_cycle.c`, pure and in milliseconds:
    - the next-panel cycle;
    - `pad_cycle_set`'s landing flag for each panel;
    - the HUD button index for each panel;
    - init closed.
  - `pad_build`: the open flag, with the focus kept across close.
  - `pad_jump` (refactor 2): hover, read and reseed. These are the old `seed_hovered` cases, now reachable without a game, plus the first-pressed-direction function (refactor 4).
  - `pad_nav`, through `tests/test_pad_nav.c`.
- **Prior art:**
  - `tests/test_pad_build.c`, `tests/test_pad_back.c` and `tests/test_pad_select.c` for pure pad modules;
  - in `tests/test_pad_nav.c`: `open_build_bar`, `open_score_from_the_map`, `assert_shoulders_wait`, `take_structure`, `test_r1_on_tech_closes_the_box_and_shows_score_on_the_same_frame`, `test_l1_on_end_turn_tech_and_score_opens_the_bar_and_hands_over_nothing`, and `test_ordinary_cross_triangle_and_circle_play_never_types_the_cheat`.
- **Deleted, each with its mechanic:**
  - with the end-turn tab: `test_end_turn_focuses_and_hovers_the_year_skip_button`, `test_cross_on_end_turn_ends_one_year_per_press`, `test_cross_held_on_arrival_ends_no_year` and `test_the_cross_that_closes_pause_on_end_turn_ends_no_year`, plus the end-turn half of `test_pause_from_end_turn_and_build_keeps_the_tab_and_its_cursor`;
  - with network travel's Left/Right turns: `test_slipways_turn_clockwise_from_straight_up` and `test_straight_up_splits_the_two_halves`.
- **Rewritten or edited:** the lists of R-3. Every test that compared `pad_nav_tab` with `PAD_NAV_TAB_MAP` now asserts that the bar is closed and no cycle panel is open, through one test helper. A comparison with `BUILD`, `TECH` or `SCORE` becomes the matching accessor.
- **New in chunk 2:**
  1. R1 on the bar: the bar closes on the switch frame and `pad_nav_tech_open` is 1. The landing clicks the tech button by the map's rule (R-1), and the tech panel is up the frame after. On no frame are the bar and `tech` open together.
  1b. A × held through R1 on the bar presses nothing on the tech panel until it is released and pressed again (`3b4caf8`'s rule).
  2. R1 on the bar while its icon is focused, then L1 from `tech`: the bar reopens on the same icon.
  3. R1 on the bar waits as shoulders do: on a click's release frame, and under pause.
  4. The cheat: step 11's ordinary sequence, extended with R1 on the bar (D-10), never fires it.
  5. Bar → tech → score → closed cycles leave `p8_fixed_rnd_state()` unchanged.
- **New in chunk 3:** the highlight cases listed in chunk 3.
- **Kept unchanged:** every jump, aim, slipway, placement, pause, Select, ○-order, chord, panel and title test that never named a tab.
- **At the end:** `make full-game`, `make save-worst-case` and `make save-fuzz` once (crash or assert only), and the `make test` and `make e2e` wall times compared before and after.
- **Not tested:** misuse asserts, including the bar-and-cycle exclusivity assert.

## Out of Scope

- Step 11's R1 cycle from the map, and any of its own re-authoring. Step 12 starts from its commit.
- Re-authoring the `worst-case/` games (step 13). They still press the ring. Their R1 on the bar now opens `tech`, which only changes their stale runs.
- The pad metrics tool (step 14), the README Controls section (step 15), and the comment rework (step 16).
- Renaming the ui's `pad_tab` field and functions, or the `pad_end_turn` replay.
- Re-hitting `save_state.c`'s point-ended slipway skip (R-4).
- Any change to what the HUD buttons, panels or menus contain; tuning values; audio; PSX; mouse input.
- `tools/pointer_guard.sh`'s `=`-in-header blind spot (step 8's finding, a follow-up outside this plan).

## Further Notes

- **For step 13:** list the `worst-case/` scripts that pressed R1 on the bar (now `tech`), visited `end turn` (gone), or typed the old cheat sequence. Their re-authoring maps ring years to Select holds and ring tech visits to R1 from the map.
- **For step 14:** R1 on the bar opens `tech`, so it is no dead press. R1 R1 R1 from the map opens and closes two panels, so it is no dead press either. No end-turn mode exists to measure. The modes to report are jump, aim, slipway, menus, placement, the bar and the cycle panels.
- **For step 15:** the final shoulder mapping is D-1's table. Select tap and hold are as in step 9, and ○ is as in step 8.
- **Order of risk:** chunk 2 is the large one. If the `pad_end_turn` re-authoring threatens to make it too big for one TDD cycle, the implementer may move `pad_end_turn` to a Select-hold first year *before* chunk 2. That route is valid under step 11's mapping (Select already ends years), it is frame-checked under the old code, and it leaves chunk 2 with every hash unchanged. Whichever route is taken, the step note names it.
