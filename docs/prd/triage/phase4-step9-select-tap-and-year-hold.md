---
title: "Select: a tap cycles colonies needing attention, a 30-frame hold ends the year"
description: "Ending a year takes two or three shoulder presses round the tab ring and the player has no way to find the colonies that lack a resource; Select must end the year on a deliberate 30-frame hold and cycle the colonies with an unmet need on a tap."
status: needs-triage
---

## Plan step (verbatim)

Plan file `docs/plan-12.md`, Phase 4 — Direct buttons instead of the tab ring, step 9:

> 9. **Select:** a tap cycles colonies needing attention, a 30-frame hold ends the year. Read the code for what the game already tracks as an unmet need. If it tracks nothing usable, cycle every colony in creation order and note it in step notes. `pad_end_turn` re-authored. Unit tests: tap order, a hold under 30 frames ends nothing. The year-heavy replays follow the route step 2b recorded.

The section 2 rules it follows:

> - **Select** (new button): a tap cycles through colonies needing attention. A hold of 30 frames ends the year: end turn was used twice in the measured game, and the hold prevents accidents.

> - New inputs must not type the cheat sequence (× × ○ ○ × × ○ ○, see `src/game/cheat.c`) by accident. Checked by the end-state check of step 2a: each replay's cheat activation count must match its reference (some replays use the cheat on purpose).
> - Every step from 6 on that changes what × or ○ do (including the build bar and the ring's removal) checks whether ordinary input sequences, with D-pad presses between them (`cheat_step` ignores them), can complete × × ○ ○ × × ○ ○, and re-checks the deliberate cheat route of step 6. [...]
> - Checks outside `make test-all` run on every chunk that touches pad input or a replay: `make unit-run-test_live_slots`, `make end-state EVENTS=` on every touched replay, `make coverage-compare`, and `make rng-sync-probe` whenever `pad_colonise` changes (a test outside the suite went red unseen for a chunk in step 7).
> - From step 8 on, each new mode's state lives in its own `pad_<mode>_t`, never in `pad_jump_t` nor as loose `pad_nav_t` fields, unless the step note gives the reason; each step note records `pad_nav.c`'s line count and `pad_nav_t`'s field count before and after. A unit test may be deleted only with the mechanic it exercises, which the step removes; the step note lists each deleted test next to that mechanic, and a test of a surviving behaviour that no longer compiles is rewritten, not deleted.

The other guardrails of section 2 apply as written: re-authoring to the step 2a end state against reference `8defcba`; the situation guardrail; the frame limits (180, or 600 if listed in `tools/e2e_long_exceptions.txt`), never raised; shared prefixes first; every broken replay re-authored; pinned values re-derived; coverage never drops; the cost record; the 8 rules of the step 2b discipline; the `.txt` "N frames." check; the `worst-case/` games knowingly stale; one scheme only.

Step 2b's `initial_save` fallback (section 3, step 2b): "The fallback, when holding does not fit, is to start that replay from an `initial_save` placed just before its year-heavy part, as `pad_resume_at_start` does."

## Decisions (ratified by `guideline`)

The plan left these open (step 2b's "Open for step 9 (`guideline` decides)" and the recomputed budget below). `guideline` ratified each as recommended. The rest of this PRD follows them.

### D-1. Several years in a row: the chain, window 15 frames

- **Decided:** a hold ends exactly one year. For 15 frames after a year the pad ended, a fresh Select press ends one more year on its press frame, and each further year opens a new 15-frame window.
- **This extends the locked rule** ("a hold of 30 frames ends the year"). The step note flags it as a `guideline`-ratified extension. The same commit adds the 15-frame window to section 5's tuning values in `docs/plan-12.md`.
- **`pad_bankrupt` must keep its toast peak of 42**, the only e2e evidence for `TOAST_COUNT` 48.
- **Rejected:**
  - A, strict: `pad_bankrupt` has no route without breaking a guardrail (a raised limit or a changed end state).
  - B, a repeat while held: a human gets 3 or 4 years from one hold, and the toast peak drops to about 37.
- **The cost:** a tap within 15 frames of a year end ends another year instead of cycling colonies.

### D-2. `pad_bankrupt`'s route

- **Decided:**
  1. First compress its pans and idle frames to ≤ 162 frames, for at most 20 minutes.
  2. Otherwise, cut at the f94 record (after s4, $40, before the cheat) and retype the cheat after the resume. The cheat sets $200 and research 99 absolutely, so the state matches.
- **Either way:** the step note records the toast and `entity_table` peaks before and after, and the toast peak must stay 42 (D-1).
- **Under the cut:** the four labs move into the save. Their placement stays covered by `pad_build`, `pad_tech_costs` and `pad_planet_projects` (`make coverage-compare` decides).

### D-3. `pad_tech_costs` accepted at up to 570 frames

- **Decided:** the cut after the six years' record, accepted at up to 570 frames (the limit is 600; at least 30 frames of margin are left for steps 10 and 11), plus any idle compression found at no risk.
- **Rejected:** a later cut, which would move the probe, the fold and "nanomaterials" into the save, situations whose mechanic the plan keeps.

### D-4. Step 9 moves every replay's year ends to Select now

- **Decided:** step 9 re-authors the year ends of all 8 year-ending replays onto Select.
- **The exception:** `pad_end_turn` keeps one year on the ring tab (see Re-authoring), which keeps the end turn tab's × lines covered until step 12 deletes them.

### Not a decision: step 2b's `pad_research` open point is moot

At step 2b, `pad_research` had 577 frames. It now has 468, and its one year costs 22 frames more as a hold (490 ≤ 540).

## Facts from step 8 (done, `2e364f6`)

Read from the "### Step 8" note of `docs/plan-12.md` and the committed code at `2e364f6`. The earlier draft of this PRD held hypotheses H-8-1 to H-8-11. Each fact says whether its hypothesis held.

- **H-8-1, held.**
  - × on a hovered known planet sends △'s click (`click_hovered`), which opens the projects menu. The × that opened it presses no row until it is pressed again (`menu_cross_armed`).
  - △ again closes the planet menu or panel jump mode opened. It never closes the year box, and on a hovered object under the year box it still opens that object's menu.
  - ○ does one rung per press:
    1. pause, then an ephemeral box, then the shown message box (year box included, `message_box_close_shown`) on the map tab only; on other tabs ○ returns to the map;
    2. a slipway or a placement;
    3. leaving aim mode;
    4. at the root, a jump to the last object × acted on, with edge scroll; nothing when nothing is recorded, when that object is hovered or when it is gone.
- **H-8-2, held.** The last object lives in `pad_back_t` (`pad_nav_t.back`, `pad_back_record`, `pad_back_target`). `pad_nav_last_acted(nav)` reads it. Only × records: the planet whose menu it opened, a slipway's start and a built slipway's target. Select records nothing.
- **H-8-3, held.**
  - `entity_list_t pad_jump_candidates(const allocator_t *zone, const entity_list_t *nodes)` is built on `entity_list_filter`, and `pad_nav.c` passes `frame_zone()`. `JUMP_CANDIDATES_CAP` is gone.
  - `tools/pointer_guard.sh` counts local `entity_t *name[...]` arrays against its ratchet. Step 9's code declares none.
  - Known gap, not ours to fix: the guard skips a block whose header holds a real `=`.
- **H-8-4, held.** `tools/e2e_limits.sh` checks each `.txt`'s "N frames." (`7874f7a`).
- **H-8-5, held.** The cheat is still four × + ○ chords, and the chord consistency test (`4e1f51b`) shows `cheat_step` and `without_chord` agree. Ordinary × △ ○ play cannot type it (unit-tested).
- **H-8-6, failed in its numbers.**
  - Frame counts now:
    - limit 600: `pad_colonise` 417 (not 420–440), `pad_planet_projects` 585, `pad_tech_costs` 566, `pad_research` 468, `pad_slipway` 318, `pad_message_box` 190;
    - limit 180: `pad_bankrupt` 170, `pad_end_turn` 91, `pad_game_end` 104, `pad_resume_at_start` 48.
  - `pad_colonise`'s year is now R1 R1 × at **f410–414** (was f404–408). It gained step 8's situations: × opens a menu f88, △ opens then closes a panel f141 and f144, the ○ root jump f268, and the designed no-op ○ on the last object f293, which step 14 needs. Its f268–293 segment must stay.
  - `pad_slipway` f88 was edited (shared prefix f0–98).
- **H-8-7, held.** Step 8 re-authored every ○ its rung 1 spends on a year box: `pad_planet_projects` f293 (○ closes the year box, f295 ○ leaves aim mode), `pad_research` f364, and `pad_tech_costs` f315 and f504. `pad_research` f306's root ○ became idle. No `.end-state` changed, and there is no exceptions file.
  - **For step 9, the trap is real:** once a year ends by Select on the map, any ○ on the map while its year box shows closes the box first. Those year-box ○ presses also move with their years, and must stay matched to them.
- **H-8-8, held.** `test_select_changes_nothing_yet` still exists in `tests/test_pad_nav.c`, and no replay presses `select`.
- **H-8-9, held.** `pad_nav.c` has 1170 lines (it had 1142), and `pad_nav_t` has 25 fields (it had 23: `back`, `menu_cross_armed`). These are step 9's "before" numbers. `test_pad_nav.c` has 189 tests.
- **H-8-10, held.** Step 9 re-derives the pins from step 8's values. `test_rng_sync`'s quiet window is now f292–416 on `pad_colonise`, and it includes the year at f410–414 that step 9 turns into a 30-frame hold.
- **H-8-11, held.** `make coverage-compare` reports the same 69 unhit lines as step 7.
- **New, from the step 8 note:** frame shifts move visual draws. `slipway.c:92` in `pad_colonise` depends on an 8 % transit-dot draw on its HUD-aim frames. Re-authoring `pad_colonise`'s year (after that segment) must not shift those frames. Frames added after f414 are safe; check with `make coverage-compare`.

## Facts from the code (read at `48d67a6`; step 8 at `2e364f6` changed none of them)

Read with `git show HEAD:<path>`.

- **Select reaches the pad but is ignored.** `P8_PAD_SELECT` is bit 2048 (step 3). `pad_nav_step` reads no Select bit, and `test_select_changes_nothing_yet` pins that.
- **How a year ends today on the pad.** On the end turn tab, `tab_step` holds the left button while × is held and armed (`cross_armed`), with the cursor on `tab_button(PAD_NAV_TAB_END_TURN)`, the HUD year-skip button (`galaxy_menu_find_button(1)`). `button_left_press` runs `on_click` on the press edge only (`mouse_button_update`), so one press ends one year: `galaxy_menu_button1_on_click_method` → `economy_new_year`. Step 2b found that a year end draws nothing from the game stream.
- **Years also pass with actions** (`economy` month ticks, "the year passes on its own on the way" in `pad_research` and `pad_tech_costs`), never with idle frames. So a re-authored year end must stay at the same point in the action sequence: it is an action whose timing is free (step 2b rule 5).
- **What the game tracks as an unmet need.** Every planet-family entity has `needs` (a list of `need_t`). `economy_tiers_compute_tier` sets each need's `satisfied` to the number of incoming routes carrying its resource. The inspection panel's "needs:" row (`planet_inspection_panel`, `planet_inspection_panel_filter_step`) lists exactly the needs whose `satisfied` is 0 or not yet set. A colony or structure that shows "needs:" lacks a resource. Without unmet needs, the panel shows "wants more:" instead. This is usable: the plan's fallback (every colony in creation order) is not needed.
- **Candidate order.** Candidates come from `TAG_NODE` in registration order (`entity_list_add` at `entity_register`). A colonised planet keeps its entity, so its place is its generation order. Structures come after the galaxy, in the order they were placed. A project with a `class_name` replaces the entity, and the new one goes last.
- **Clicks.** `click(nav, played)` presses the left button on this frame and sets `release_pending`, which forces the release on the next. `tabs_can_switch`, `pause_button_free` and jump mode's "clicking" all wait on that release.
- **Message boxes** are not ephemerals and do not block the HUD buttons. `pad_game_end`: "The year boxes from 3421 on block none of them."
- **Toasts** are entities in the ui zone's toast slots (`TOAST_COUNT` 48 in `ui_zone.c`), and they live 50 frames. `pad_bankrupt`'s peak of 42 is the only e2e evidence for that count.
- **Resume overhead**: a title with a save takes Up at f10 and × at f12, and the map runs from f13 (`pad_resume_at_start`). A new game also starts at f13. So a cut at frame f saves about f − 13 frames.

## Problem Statement

Ending a year means going round the tab ring: R1 R1 to `end turn`, × on it, then L1 L1 or ○ back to the map. That is 5 to 9 presses for an action the player wants rarely and must never trigger by accident. Nothing helps the player find the colonies that lack a resource. They have to open each colony's panel and read whether it says "needs:".

## Solution

**Select** is a new direct button on the map. It acts in jump mode and in aim mode, with no panel, menu or pause open, no slipway held, no structure carried and no fold held. The year box and other message boxes do not block it.

- **Tap** (Select released within 10 frames of its press): the cursor jumps to the next colony or structure that needs attention, meaning it has an unmet need (its panel's "needs:" row). The order is the candidate order, starting after the hovered object and wrapping round. From aim mode, the tap leaves aim mode for jump mode on that object. The jump scrolls the camera and moves the marker as a D-pad jump does. When nothing needs attention, a tap does nothing.
- **Hold** (Select held 30 frames): on the 30th frame, the pad presses the HUD year-skip button, exactly as × on `end turn` did. The year ends, and the release follows on the next frame. Holding longer does nothing more: one hold, one year. A hold released between frames 11 and 29 is abandoned and does nothing.
- **Chain (D-1, option C)**: for 15 frames after a year the pad ended, a fresh Select press ends one more year on its press frame. Taps every other frame skip years at the old HUD taps' pace.
- A refused year press (the left button still down, or the HUD button missing) ends nothing. Nothing is retried: the hold must be released and held again.
- On the year press and release frames, the cursor is on the HUD button. No jump, pan or marker runs on those frames, as during △'s click. The cursor then returns to the hovered object (jump mode) or the centre (aim mode).

The tab ring is untouched in this step. `end turn` keeps working until step 12.

## User Stories

1. As a player, I want to end the year by holding Select, so that I don't have to go round the tab ring.
2. As a player, I want the hold to take a full 30 frames, so that a brushed Select never ends a year.
3. As a player, I want a hold I let go of before 30 frames to do nothing at all, so that I can change my mind.
4. As a player, I want a hold kept down past the year to end no second year, so that one hold is always one year.
5. As a player, I want to end several years quickly with fresh Select presses right after a year, so that skipping ahead does not cost a second per year.
6. As a player, I want the fast presses to stop working 15 frames after the last year, so that a later tap goes back to meaning "next colony".
7. As a player, I want the year to end while the year box of the previous year is still shown, so that the banner never blocks me.
8. As a player, I want Select to do nothing while I hold a slipway, carry a structure or hold a fold, so that it never drops what I am holding.
9. As a player, I want Select to do nothing while a menu, a panel or pause is open, so that it never acts behind a box.
10. As a player, I want Select to do nothing on the build, end turn, tech and score tabs, so that the tab ring keeps working as before until it goes.
11. As a player, I want a Select tap to jump to the next colony that lacks a resource, so that I find what needs a slipway without opening every panel.
12. As a player, I want the colonies to come in a fixed order and wrap round, so that tapping repeatedly visits every one of them.
13. As a player, I want colonies with every need met skipped, so that the tap only shows what needs attention.
14. As a player, I want structures with an unmet need included, so that a lab without its supplies shows up too.
15. As a player, I want the tap's jump to scroll the camera as a D-pad jump does, so that the colony comes into view smoothly.
16. As a player, I want a tap from aim mode to bring me back to jump mode on the colony, so that the tap always ends pointing at something.
17. As a player, I want a tap with nothing needing attention to do nothing, so that it is harmless.
18. As a player, I want a refused year press not retried later, so that no year ends behind my back.
19. As a player, I want the year press to show the cursor on the year-skip button for its two frames, so that I can see what happened.
20. As a player, I want Select never to touch the cheat, so that my game stays honest.
21. As a replay author, I want the year end as a single Select hold at the same point in the action sequence, so that the end state stays the reference's.
22. As a replay author, I want the chain to take one press every other frame, so that multi-year replays keep their old cadence and their toast peaks.
23. As a replay author, I want the `initial_save` cut only where a replay still does not fit, so that titles and prefixes stay shared where possible.
24. As a replay author, I want a recomputed budget per replay before I start, so that I know which route each takes.
25. As a reviewer, I want every year-ending replay to pass `make end-state` against `8defcba` with its cheat count kept, so that no game end state changes.
26. As a reviewer, I want the old and new `.txt` compared per replay, so that only ring situations disappear, each noted.
27. As a reviewer, I want unit tests for the tap order and for a hold under 30 frames ending nothing, so that the plan's two named tests exist.
28. As a reviewer, I want `make coverage-compare` to show nothing newly unhit, so that coverage does not drop.
29. As a reviewer, I want `pad_bankrupt`'s toast peak 42 kept, so that `TOAST_COUNT` 48 stays evidenced.
30. As a developer, I want Select's state in its own `pad_select_t`, so that each mode's state stays where section 2 puts it.
31. As a developer, I want the hold, tap and chain timing in a pure module, so that it is unit-tested without a game.
32. As a developer, I want "needs attention" defined in one predicate that matches the panel's "needs:" row, so that the pad and the panel never disagree.
33. As the agent of step 12, I want `pad_end_turn` to keep one ring year, so that the end turn tab's code stays covered until step 12 deletes it.
34. As the agent of step 14, I want Select taps and holds to be ordinary pad presses with observable effects, so that the metrics tool can count dead Select presses.
35. As a developer, I want `make test` and `make e2e` no slower, so that the fast loop stays fast.

## Implementation Decisions

### The pure module `pad_select` (own state, `pad_select_t`)

- New `src/game/pad_select.{c,h}`, pure helpers in the style of `pad_jump`, `pad_slipway`, `pad_target` and (H-8-2) `pad_back`. `pad_nav.c` only calls them.
- `pad_select_t`: the frames Select has been down in the current press (`held_frames`), whether this press already ended its year (`fired`), and the frames left in the chain window (`chain_frames`, D-1 C). `pad_nav_t` gains one field, `select`. No loose field.
- Constants, tunable under section 5:
  - `PAD_SELECT_HOLD_FRAMES` 30;
  - `PAD_SELECT_TAP_FRAMES` 10 (a release at or under it is a tap);
  - `PAD_SELECT_CHAIN_FRAMES` 15 (D-1 C).
- Functions:
  - `void pad_select_init(pad_select_t *s)`.
  - `pad_select_event_t pad_select_step(pad_select_t *s, int down, int allowed)`: one frame. It returns `PAD_SELECT_NONE`, `PAD_SELECT_TAP` or `PAD_SELECT_YEAR`.
    - `allowed` 0 (a blocked state): the press is dropped. `held_frames` goes to 0 and `fired` is set, so the rest of that press is inert. Nothing is returned, and the chain window keeps counting down.
    - Press edge with the chain window open: `YEAR` on that frame, and `fired`.
    - Otherwise, `YEAR` on the frame `held_frames` reaches 30, once per press.
    - `TAP` on the release frame of a press that did not fire and lasted ≤ 10 frames. A release at 11–29 frames returns nothing.
  - `void pad_select_year_ended(pad_select_t *s)`: the caller reports that the year press went out. It opens the chain window (15 frames). The window does not open on a refused press.
  - `int pad_select_needs_attention(const entity_t *e)`: `e` is a `pad_target_is_candidate`, SETTLED, and has at least one need whose `satisfied` is unset or 0. That is exactly the panel's "needs:" filter. The header states the contract: it reads only, and is NULL-safe (0).
  - `entity_t *pad_select_next(const entity_list_t *candidates, const entity_t *current)`: the first entry after `current` in list order that needs attention, wrapping round. When `current` is not in the list or is NULL, the search starts from the beginning. It may return `current` itself when that is the only one. It returns NULL when none needs attention.
- The predicate lives here and not in `planet_display.c`: the panel's filter also collects received needs, and the pad must not grow display state. A unit test pins that both agree on hand-built needs. A trap comment (≤ 40 words) at the predicate says that it must follow `planet_inspection_panel_filter_step`.

### Select in `pad_nav`

- **Allowed** (`allowed` passed to `pad_select_step`) when all of these hold:
  - a game runs;
  - the tab is map;
  - pause is not shown;
  - no ephemeral box is open (planet menu, inspection panel);
  - the map mode is jump or aim;
  - no placement is in progress or suspended;
  - × holds no fold (`cross_engaged`);
  - no click is in progress (neither its press nor its release frame);
  - the frame does not change the tab.

  A shown message box (the year box) does not block. Slipway mode blocks, because the left button is held.
- **Order in `pad_nav_step`.** Select is stepped once per frame, after pause and the tab switch and before `map_step`, on `pad_buttons & P8_PAD_SELECT` (the chord mask does not touch Select).
  - On `YEAR`: when the HUD button exists and the left button was up on the previous frame (`left_was_down` 0), the cursor goes to the button's focus point (`pad_panel_focus_point(tab_button(PAD_NAV_TAB_END_TURN))`). `click` presses, and `pad_select_year_ended` runs. The map step does not run on that frame. The release frame then runs as any click's release frame: jump mode and aim mode skip their jump, pan and marker, as △'s click does. Otherwise the press is refused: nothing goes out and nothing is retried.
  - On `TAP`, handled inside `map_step`'s jump or aim branch, in a new private `select_tap`: `pad_select_next(candidates, hovered)` on `pad_jump_candidates(frame_zone(), nodes)` (H-8-3).
    - When it returns an object other than the hovered one, the hovered object becomes it, and the frame runs as a D-pad jump that landed there: edge scroll, marker and cursor. From aim mode, `back_to_jump_mode` on it first.
    - When it returns NULL or the hovered object, nothing changes: in aim mode, aim mode stays.
- **The cursor** is the HUD button's point on the press frame only. On the release frame it is restored from the mode, the hovered object's screen point or the centre, as `tab_cursor` does for the map.
- **Nothing records** into `pad_back` (H-8-2): Select acts on no object.
- `pad_select_init` runs in `pad_nav_init` and in `start_new_game`. `reset_map_mode` calls `pad_select_init` too: a mode reset drops a press in progress and closes the chain window.
- **Accessors for tests and step 14:** `int pad_nav_select_held(const pad_nav_t *nav)`, the frames the current press has been down, 0 when none, and `int pad_nav_year_chain_open(const pad_nav_t *nav)`. Header comments state both contracts.
- **HUD:** no new drawing. `ui_set_pad_tab` is unchanged until step 12. The year-skip button draws hovered on the click's press frame because the cursor is on it, as with × on `end turn`.

### Unchanged

The tab ring, including × on `end turn`; jump, aim, slipway and placement modes; △, × and ○ in every mode (step 8's rungs, H-8-1); pause; panels; network-mode code (step 12).

### The cheat (section 2's re-check)

Step 9 changes neither × nor ○, so section 2's re-check is not triggered. The step note still records the following:

- Select is not part of the chord.
- `cheat_step` reads only × and ○, so Select presses between chords neither reset nor complete it.
- Step 6's deliberate route (four chords in jump mode on a colony) still leaves nav, cursor and game unchanged with Select held across it.

A unit test pins the last point (case 14). Each replay's cheat count is checked by `make end-state`.

## Year-hold budget, recomputed

The base counts are the committed counts after step 8 (H-8-6); "≈" marks estimates of the re-authored lengths. A route fits at ≤ 90 % of the limit (step 2b: 540 for 600, 162 for 180). Strict cost is 31 frames per year. Under D-1 C, a run of n years costs 30 + 2 × (n − 1) frames, plus 1 for the release. A cut at frame f saves about f − 13.

| Replay | Years (route today) | Frames now | Strict hold (A) | Chain (C) | Limit / 90 % | Route (C) |
|---|---|---|---|---|---|---|
| `pad_colonise` | 1 (R1 R1 × at f410–414) | 417 | ≈ 443 | same | 600 / 540 | hold |
| `pad_end_turn` | 2 (ring) | 91 | redesigned, ≈ 145 | same | 180 / 162 | 1 ring year kept + 1 hold + an abandoned hold + a tap (Re-authoring) |
| `pad_research` | 1 (f383–391, 9 frames) | 468 | 490 | same | 600 / 540 | hold (step 2b's open point is moot) |
| `pad_planet_projects` | 1 (f270–274, then R1 to tech) | 585 | 610 → cut at f86: ≈ 537 | same | 600 / 540 | `initial_save` cut at f86 (step 2b), then idle compression if step 8 grew it |
| `pad_tech_costs` | 6 at f24–34 + 1 at f514 | 566 | 760 → cut after the six years: ≈ 568 | 614 → cut: ≈ 568 | 600 / 540 | cut after the six years' record, accepted up to 570 (D-3) |
| `pad_message_box` | 20 (f20–58) | 190 | 778 → cut: ≈ 175 | ≈ 212, no cut | 600 / 540 | chain |
| `pad_game_end` | 25 (f21–69) | 104 | 879 → cut at f64: ≈ 140 | ≈ 128, no cut | 180 / 162 | chain |
| `pad_bankrupt` | 8 (7 at f130–142, 1 at f167) | 170 | ≈ 396; cut: ≈ 300: **no route** | ≈ 193; cut: ≈ 110 | 180 / 162 | chain; compress to ≤ 162 (at most 20 min), else the f94 cut with the cheat retyped (D-2) |

Total e2e frames: about 2,560 today → about 2,600 under C. The cut in `pad_bankrupt` or `pad_planet_projects` gives back most of the growth (`make e2e` wall time is compared before and after; see Testing Decisions).

## Re-authoring

Discipline: the 8 rules of step 2b.

- The year end is an action whose timing is free (rule 5), at the same place in the action sequence.
- A Select hold in aim mode moves no camera, so `pad_planet_projects`' off-screen year keeps its camera.
- After a tap's jump (edge scroll), the replay re-pans before its next aim-mode point action (rule 8).
- **Step 8's year-box trap (H-8-1, H-8-7):** a year now ends on the map tab, so the year box is shown there, and a ○ on the map within about 90 frames is consumed closing it (step 8's rung 1). Each replay's ○ after a Select year gets a second ○ on the next idle frame, or waits for the box to go.

Order: shared prefixes first (`pad_probe` → `pad_colonise` → `pad_slipway`, `pad_end_turn` → `pad_game_end`, `pad_research` → `pad_resume_at_start`).

### Per replay

- **`pad_end_turn`** (redesigned, about 145 of 180).
  - Kept: the title and the f16–17 build tooltip.
  - The ring year: the year with × held 20 frames on `end turn`, "one press, one year". It keeps `tab_step`'s end turn lines covered until step 12, and is noted as the ring situation step 12 removes.
  - Back to the map with ○.
  - A Select tap with no colony: nothing happens, the designed no-op.
  - A Select hold released at frame 20: abandoned, no year, no mouse press.
  - A Select hold kept 40 frames: one year on its 30th frame, the cursor on the year-skip button for the press frame, colour 12, then back.
  - The probe and the refused fold, unchanged.
  - Dropped, ring mechanic: the × tap year and the L1/L1 R1/R1 cycling, which `pad_research` and `pad_score` still cover.
  - End state: 2 years, as the reference.
  - `pad_game_end` shares f0–20: it is re-cut from the new prefix where it shares.
- **`pad_colonise`.**
  - Its year (R1 R1 × at f410–414) becomes a 30-frame Select hold in jump mode. Every frame before f410 stays, including step 8's f268–293 situations and the HUD-aim frames `slipway.c:92` depends on.
  - **The tap cycle situation** is produced here, after the year, at the replay's tail (no later point action depends on the cursor). It needs two taps through at least two objects that need attention, with one settled object skipped, and the wrap seen.
  - If `pad_colonise` has fewer than two objects needing attention at that point, the situation goes to `pad_research`'s tail, then to `pad_tech_costs`'s. The `.txt` names the panels' "needs:" rows that prove which objects need attention.
  - `make rng-sync-probe` is re-run, and `test_rng_sync`'s quiet window is re-derived.
- **`pad_research`.** The f383–391 ring year becomes a hold. `pad_resume_at_start` is re-cut from its new last record (the year's).
- **`pad_message_box`** (D-1 C).
  - The 20 years become a hold plus 19 chain presses on the map tab, in aim mode.
  - The f62 ○ (end turn → map) goes, since the replay is already on the map.
  - The year boxes, "5 years remain", pause over the box, the score box replacing it and running out are all kept. `pad_message_box`'s later ○ presses are under pause or on the score tab, so H-8-1 does not consume them.
  - The long-exception reason is updated.
- **`pad_game_end`** (D-1 C).
  - The 25 years become a hold plus 24 chain presses.
  - "× held into the panel": × is held in jump mode with nothing hovered (× does nothing there), from before the 25th press until after the final score panel opens, and presses nothing.
  - "again?" and the title are kept.
- **`pad_bankrupt`** (D-1 C, D-2).
  - The 7 years become a hold plus 6 chain presses.
  - Then, inside the 15-frame window, the refused lab: R1 to `build`, × takes it, × drops it at a free centre spot, and "-no money-". The lab's spot is not in the end state, so the camera is left where the centre is free. Then a chain press for the 8th year: bankrupt.
  - Toast peak 42 kept (D-1). Route: compress to ≤ 162 frames for at most 20 minutes, else the f94 cut with the cheat retyped after the resume (D-2). The toast and `entity_table` peaks before and after go in the step note.
- **`pad_planet_projects`.**
  - `initial_save` cut at f86 (the record after `pad_probe`'s fold, before the first cheat), cut by the `make replay SAVE_HEX=` recipe of `pad_resume_at_start.txt`, which goes in the `.txt`.
  - The year becomes a hold, and the tech trip goes from the map (L1 L1).
  - Its cheats stay after the cut (count 3).
  - It no longer shares `pad_probe`'s f0–68: the note records the prefix lost and the replays still covering it.
- **`pad_tech_costs`** (D-3).
  - `initial_save` cut at the six years' last record (year 3407). "Nanomaterials" is invented after the resume on the tech tab, so only the six years (ring mechanic) move into the save.
  - The f514 year becomes a hold.
  - Every `test_live_slots` (f512, f544, f559, f564, `p54`) and `test_state` (counts, the `p31`, `p2` and slipway lines, camera `101 242`, save summary, cheat 1) pin is re-derived with its intent.
- **`initial_save` traps** (step 2b) are checked per cut:
  - the cheat after the cut;
  - no wormhole alive at the record;
  - the resume title's × and the cheat matcher;
  - the resumed game on `map`;
  - the title lines covered by other replays.

### Pins and records

- **Pins re-derived** in the commit that changes the replay they read:
  - `tests/test_live_slots.c`: `pad_game_end` f21 and f69, `pad_tech_costs`, `pad_message_box` f115–121 and `pad_planet_projects` f171–256, plus its fixtures if they are cut from a changed replay;
  - `tests/test_state.c`: `pad_tech_costs`, and `pad_research` cut at 45 frames, which comes before its year and is expected unchanged;
  - `tests/test_rng_sync.c`;
  - the golden hashes, each `.txt` "N frames." (H-8-4), each `.json.sha256`, and `tools/e2e_long_exceptions.txt`'s reasons.
- **No `.end-state` is re-baselined.**
- **Cost record:** the replays broken and re-authored per chunk, and the agent minutes.

## Implementation Chunks

Each chunk ends on a commit with `make test-all` green. Section 2's checks outside the suite run on every chunk:

- `make unit-run-test_live_slots`;
- `make end-state EVENTS=` on every touched replay (all 17 on chunk 2);
- `make coverage-compare`, with nothing beyond step 8's set (H-8-11);
- `make rng-sync-probe` whenever `pad_colonise` changes.

1. **`pad_select` and its tests.** The module, `pad_select_t`, the step, chain, predicate and next functions, and `tests/test_pad_select.c`. Nothing calls it yet: every e2e is unchanged.
2. **Select in `pad_nav`.**
   - `pad_nav_t.select`, the allowed rule, the year click, the tap and the accessors.
   - `tests/test_pad_nav.c` cases 1–15. `test_select_changes_nothing_yet` is deleted with its mechanic ("Select is ignored", which this step removes), listed in the note. Its intent (Select in a blocked state changes nothing) moves into case 9.
   - No replay presses Select: every `.json.sha256` and `.end-state` is unchanged.
3. **Budget check, then the plain holds.**
   - First, confirm the table above against the replays as re-authored, and ask `guideline` again if a route no longer fits.
   - Then re-author `pad_end_turn`, `pad_colonise` (with the tap cycle situation; `pad_slipway` is untouched, since it shares only f0–98) and `pad_research`, and re-cut `pad_resume_at_start`.
4. **The chains** (D-1 C): `pad_game_end` (re-cut from `pad_end_turn`'s prefix), `pad_message_box` and `pad_bankrupt` (D-2).
5. **The cuts**: `pad_planet_projects` and `pad_tech_costs` (D-3), with their pins.
6. **The step note** in `docs/plan-12.md`:
   - the ratified D-1 to D-4, with D-1 flagged as a `guideline`-ratified extension of the locked rule and its 15-frame window added to section 5's tuning values;
   - the real budget table;
   - each route taken, with frame counts before and after, and each cut's frame and recipe;
   - the replays broken and re-authored per chunk, and the agent minutes;
   - `pad_nav.c`'s line count and `pad_nav_t`'s field count before and after (H-8-9);
   - the deleted test next to its mechanic;
   - the situations dropped, all ring ones, each named, and those moved (the tap cycle's replay and frames);
   - `pad_bankrupt`'s toast and `entity_table` peaks;
   - the `make coverage-compare` result, the cheat verdict, and the `make e2e` and `make test` wall times before and after;
   - any fact of "Facts from step 8" the implementation found different, and what was done instead.

## Testing Decisions

- **A good test** drives public behaviour only.
  - Pure tests call `pad_select_*` on hand-built states and entities.
  - Pad tests drive `run_frame`, `tap` and `hold`, then assert on `pad_nav_jump_mode`, `pad_nav_aim_mode`, `pad_nav_hovered`, `pad_nav_select_held`, `pad_nav_year_chain_open`, the played frame's mouse position and buttons, the camera, the economy's year and credits, and `p8_fixed_rnd_state()`.
  - No test asserts on `pad_nav_t` internals.
- **Prior art:**
  - `tests/test_pad_jump.c` (hand-built entities, repeat timing) and `tests/test_pad_target.c`;
  - in `tests/test_pad_nav.c`: `triangle_click`, `test_triangle_on_a_hovered_object_still_scrolling_in_presses_nothing`, `test_jumps_repeats_and_edge_scrolls_leave_the_game_stream_unchanged`, `test_four_chords_on_a_hovered_colony_cheat_and_change_nothing_else`, `settled_row`, `start_game_in_jump_mode`, `hover_object`, `shown_message_box`, and the select-run harness that `test_select_changes_nothing_yet` uses.
- **`tests/test_pad_select.c`** (chunk 1), on states and hand-built entities with needs:
  - held 29 frames then released: nothing (not a tap, since it was over 10 frames);
  - held 30: `YEAR` on frame 30 exactly, once;
  - held 90: still one `YEAR`;
  - released at 1 and at 10 frames: `TAP` on the release frame;
  - at 11: nothing;
  - a press made while not allowed, then allowed while still down: never fires; it must be released;
  - the chain: after `pad_select_year_ended`, a press on window frame 15 gives `YEAR` on its press frame, while one on frame 16 starts a normal press; a press in the window held 40 frames gives one `YEAR`; without `pad_select_year_ended` (a refused press), no window opens;
  - the predicate: a settled colony with one unmet need is 1, with all needs met 0; a known planet, a dead entity and NULL are 0; a structure with an unmet need is 1; a need not yet computed counts as unmet, as in the panel's filter;
  - next: order after current, the wrap, satisfied ones skipped, current not in the list starts from the first, the only one returns itself, none returns NULL.
- **`tests/test_pad_nav.c`** (chunk 2):
  1. **Tap order** (the plan's test): three colonies A, B, C in registration order, B with every need met (`settled_row` plus a hand-satisfied need). Taps from A: C, then A (wrap), with B skipped. Each tap's frame runs the edge scroll (≤ 8 px per axis per frame) and moves the marker.
  2. **A hold under 30 frames ends nothing** (the plan's test): Select held 29 frames, then released. The year, credits and research are unchanged, no mouse button goes down on any frame, and the hovered object is unchanged.
  3. Held 30 frames: on frame 30 the left button goes down at the year-skip button's focus point, then is released on frame 31. The year is +1 and the income is applied once.
  4. Held 90 frames: one year.
  5. Chain: a hold, then 6 presses every 2 frames: 7 years. A press 16 frames after the last year starts a fresh hold and ends nothing at once.
  6. Under the year box (`score_panel_new_year` shown): a hold ends the year.
  7. In aim mode: a hold ends the year, and the cursor is back at the centre after the release frame. A tap leaves aim mode for jump mode on the first object needing attention.
  8. No object needs attention: a tap changes nothing (hovered object, camera, cursor and mode).
  9. Blocked states (the intent of `test_select_changes_nothing_yet`), where a 40-frame hold and a tap each change nothing and press nothing:
     - slipway mode (the slipway still held after);
     - a carried structure;
     - × holding a fold;
     - a planet menu open;
     - pause;
     - the build, end turn, tech and score tabs.
  10. A refused press: with the left button down on the previous frame (forced through a click's release frame), the 30th frame presses nothing, and holding on to 60 frames presses nothing.
  11. A tap or hold records nothing: `pad_nav_last_acted` is unchanged (H-8-2).
  12. Taps, holds and the chain leave `p8_fixed_rnd_state()` unchanged (step 2b: a year end draws nothing).
  13. A new game resets Select: a hold begun on the last frame of a game counts from 0 in the next.
  14. Four chords typed while Select is held: the cheat fires once and no year ends early. Select taps between ordinary × and ○ presses never fire the cheat.
  15. The hovered object is not in the attention list: the tap goes to the first one in order.
- **Kept unchanged:** every △, ×, ○, slipway, placement, pause, ring and year-box test.
- **Deleted:** `test_select_changes_nothing_yet`, with the mechanic it pinned (Select ignored).
- **Before each commit:**
  - `make test-all`, `make unit-run-test_live_slots`, `make end-state EVENTS=` on touched replays (all at chunks 2 and 6), and `make coverage-compare`;
  - `make rng-sync-probe` when `pad_colonise` changed;
  - at the end, `make full-game`, `make save-worst-case` and `make save-fuzz` once (knowingly stale: only an assert or crash matters);
  - `make test` and `make e2e` wall times compared before and after the step.
- **Not tested:** misuse asserts.

## Out of Scope

- The L1 build bar, the R1 panels and the ring's removal (steps 10–12). The end turn tab keeps working, and `ui_set_pad_tab` is unchanged.
- Any HUD drawing for Select: no hold progress, no attention markers.
- Re-authoring the `worst-case/` games (step 13). They press no Select.
- Step 14's metrics tool.
- Tuning the hold, tap and chain constants beyond their first values.
- Any change to the cart's rules, to what panels show, audio, PSX or mouse input.

## Further Notes

- **Why the HUD year-skip press and not `economy_new_year` directly:** section 2 limits the pad to mouse presses and cursor positions. The press is the one × on `end turn` makes today, so the end states match.
- **Why taps act on release:** a tap on the press would move the cursor at the start of every hold. The 10-frame tap limit and the abandoned hold (11–29 frames) make "a hold under 30 frames ends nothing" true in the strongest sense: it does nothing at all.
- **Why "needs attention" is the panel's "needs:" row:** the game already computes it (`satisfied` per need, refreshed at each recompute), and it is what the player would see by opening the panel. The plan's fallback (every colony) is not used, and the step note says so.
- **For step 12:** `pad_end_turn`'s ring year is the last ring use of the end turn tab. Step 12 removes it, and with it `tab_step`'s end turn branch and `finish_step`'s pause re-arming line.
- **For step 14:** a Select tap with nothing needing attention, or with only the hovered object needing it, is a dead press in the "others" group (no locked rule requires it). An abandoned hold is not a dead press: it is the designed way out of a hold.
