---
title: "× on a known planet and the ○ back order"
description: "In jump mode × does nothing on a known planet, ○ does nothing at the root, and △ cannot close the planet menu it opened; × must open the projects menu, △ must toggle the planet menu, and ○ must always go back exactly one step, ending at the root on the last object × acted on."
status: needs-triage
---

## Plan step (verbatim)

Plan file `docs/plan-12.md`, Phase 3 — × and ○ everywhere, step 8:

> 8. **× on a known planet** opens its projects menu. **○ back order** as in section 2, including the root jump to the last object × acted on. Unit tests: one per rung of the ○ order, and ○ at game start does nothing. Affected replays (`pad_colonise`, `pad_planet_projects`, `pad_empty_planet`) re-authored. Method: the step's first commit is a pure refactor, before any mapping change: `pad_jump_candidates` and `pad_slipway_candidates` stop writing the caller's storage address into `*out` and use the engine's `entity_list_filter` (as `probe.c` does), `JUMP_CANDIDATES_CAP` stops depending on a number in `cart.c`, and `tools/pointer_guard.sh` (or a millisecond sibling in `make test`) also counts local `entity_t *name[...]` arrays in `src/game/`; checked by every `*.sha256` and `.end-state` unchanged, `make state` peaks within their margins and no new unhit line. The same commit series adds the `.txt` frame-count check of section 2. The last object × acted on does not go in `pad_jump_t`.

The rules it follows (section 2):

> **Buttons**
> - **×** acts on the hovered object:
>   - On a colony or structure: start a slipway from it.
>   - On a known planet: open its projects (colonize) menu, as a mouse press there does.
>   - If the press is refused (for example a full slipgate), nothing is held and no press is retried or passed on: it fixes the bug above by design.
> - **△:** the planet menu of the hovered object. △ again, or ○, closes it. Inside menus, the D-pad and × work as today (`pad_panel_step`).
> - **○ always goes back one step**, in this order: close the box or panel; cancel the slipway or the placement; leave aim mode; at the root, jump to the last object × acted on (nothing at game start).

> **Guardrails** (the cheat)
> - New inputs must not type the cheat sequence (× × ○ ○ × × ○ ○, see `src/game/cheat.c`) by accident. Checked by the end-state check of step 2a: each replay's cheat activation count must match its reference (some replays use the cheat on purpose).
> - Every step from 6 on that changes what × or ○ do (including the build bar and the ring's removal) checks whether ordinary input sequences, with D-pad presses between them (`cheat_step` ignores them), can complete × × ○ ○ × × ○ ○, and re-checks the deliberate cheat route of step 6. If ordinary play can complete it, or no side-effect-free deliberate route exists, change the cheat sequence in `src/game/cheat.c` (a dev tool, not a game rule) to one ordinary play cannot type and that has such a route, and update the replays that type it, in the same step.

Section 2's method bullets (commit `d549a67`):

> - Since step 6 the chord is defined twice, in `cheat_step` (`cheat.c`) and in `pad_nav.c`'s `without_chord`, which hides × and ○ from every mode: a cheat change edits both in one commit (or single-sources the chord first), and a unit test in `make test` drives one button sequence through both and asserts the pad hides × and ○ on exactly the frames `cheat_step` counts as chords.
> - Checks outside `make test-all` run on every chunk that touches pad input or a replay: `make unit-run-test_live_slots`, `make end-state EVENTS=` on every touched replay, `make coverage-compare`, and `make rng-sync-probe` whenever `pad_colonise` changes (a test outside the suite went red unseen for a chunk in step 7).
> - From step 8 on, each new mode's state lives in its own `pad_<mode>_t`, never in `pad_jump_t` nor as loose `pad_nav_t` fields, unless the step note gives the reason; each step note records `pad_nav.c`'s line count and `pad_nav_t`'s field count before and after. A unit test may be deleted only with the mechanic it exercises, which the step removes; the step note lists each deleted test next to that mechanic, and a test of a surviving behaviour that no longer compiles is rewritten, not deleted.
> - Each e2e `.txt` states its replay's length as "N frames.", and `tools/e2e_limits.sh` (in `make e2e`) checks it against the `.json` (added in step 8).

Step 5's recorded `guideline` decision applies: "△ again closes the planet menu" is left to step 8, with the ○ "close the box or panel" rung. It is in scope here.

The other guardrails of section 2 apply as written:

- re-authoring to the step 2a end state against reference `8defcba`;
- the situation guardrail: every situation of the old `.txt` is kept, except those whose mechanic the plan removes, each noted;
- the frame limits (180, or 600 if listed in `tools/e2e_long_exceptions.txt`), with `make e2e` no slower;
- shared prefixes first, every broken replay re-authored (the per-step list is a minimum), and pinned values re-derived;
- coverage never drops (`make coverage-compare`);
- the cost record (replays broken, agent time spent re-authoring);
- the 8 rules of the step 2b discipline, rule 8 above all (re-pan to the reference camera before a point action in aim mode after any jump-mode segment);
- the `worst-case/` games are knowingly stale;
- one scheme only.

## Facts from step 7 (done)

Step 7 is done (`0d452f7` to `24cec17`, step note `57a3d8d`). These facts come from that note, the committed `pad_nav.c`, `pad_nav.h`, `message_box.c`, `score_panel_display.c` and `tests/test_pad_nav.c`, and the e2e `.txt` at `57a3d8d`. The labels H-7-k are those of the earlier draft of this PRD. Each says whether the assumption held and, where it did not, what replaces it. **Where a fact here contradicts a later section, the fact wins.**

- **H-7-1, held in its names, failed in one detail.** Aim mode is `PAD_NAV_MAP_AIM` and `pad_nav_aim_mode`, with the private names `aim_mode_step`, `aim_pan`, `enter_aim_mode`, `reset_aim_mode` and the new `leave_aim_mode`.
  - □ or ○ leaves aim mode for jump mode, hovering the target nearest the centre, with the camera unmoved. Both wait while × holds a fold.
  - △ does nothing in aim mode.
  - **The detail that failed**: ○ closes only a **ui ephemeral box** first (`circle_closes_the_box`, and `circle_returns` is 0 only while `pad_panel_box_open()`, which is "the ui holds an ephemeral"). The planet menu, the inspection panel, the tech box and the pause menu are ephemerals. **The year box and the other message boxes are not.**
    - A message box (`score_panel_new_year`'s year box, or `score_panel_update`'s score box running out after its tab closes) is the single `shown_message_box()`. It is not modal, opens no panel, and closes itself on a timer (`message_box_draw_overlay` marks it dead and clears the shown box).
    - With the year box up, ○ in aim mode leaves aim mode and the box stays. The step note leaves this to step 8.
    - **So rung 1 of the ○ order is not already in place for message boxes**: step 8 adds it (see "The ○ back order").
- **H-7-2, held.** The □ select, the drag, the starting and last planet, `triangle_on_centre` and the ○ return to the last planet are deleted. Network mode can no longer be entered. Its code is left for step 12: `network_step`, `travel_to`, `click_planet`, `is_clickable_planet`, `centre_camera_on`, `drop_stale_map_mode`, `map_step`'s network branch, `pad_nav_selection`, `pad_nav_selected_slipway`, `pad_nav_next_slipway`, the selection fields and `PAD_NAV_MAP_NETWORK`. Step 8 gives network mode no rung and adds no way into it.
- **H-7-3, held.** `jump_step` is as steps 5 and 6 left it:
  - ○ does nothing ("○ does nothing here");
  - × presses only what `cross_starts_slipway` accepts;
  - × on a known planet does nothing;
  - △ clicks through `triangle_on_hovered`, gated by `pressable`;
  - nothing remembers what × last acted on.
- **H-7-4, held.** The cheat is still four × + ○ chords, hidden from every mode by `without_chord`. Step 7 unit-tested that chords do nothing else in aim mode (`test_four_chords_in_aim_mode_cheat_and_change_nothing_else`), and that probing and leaving aim mode cannot type it (`test_probing_and_leaving_aim_mode_never_types_the_cheat`).
- **H-7-5, held.** A structure taken from jump mode enters aim mode, and ○ cancels it and stays in aim mode; the next ○ leaves (`test_circle_cancels_a_structure_taken_from_jump_mode_then_leaves_aim_mode`). `placement_step` reads `pressed & P8_PAD_CIRCLE` directly, not `circle_returns`.
- **H-7-6, held.** Slipway mode is unchanged: ○ cancels back on the start, and a built slipway rebases the start onto the target.
- **H-7-7, held, with the route named.** Every menu opens with □ from aim mode into jump mode on the target nearest the centre, then △ on the hovered planet. The colonising is × on a menu row.
  - `pad_colonise`:
    - f86 □, then f88 △ on p47 (colonised f98);
    - f135 △ on the colony (its inspection panel, on the f136 release), and f139 ○ closes it;
    - f206 □, then f208 △ on p25 (colonised f213).
  - `pad_planet_projects`:
    - f159 □, then f161 △ on p25 ("harvest");
    - f202 □, then f204 △ on p47 ("mechanize");
    - f217 △ on p47, still hovered ("nanotech").
- **H-7-8, held.** `pad_planet_projects`' "X taps on a planet open nothing" (f87–95) come after f17's □, in aim mode, where × over a planet still presses nothing.
- **H-7-9, open; `make e2e` decides.** Every ○ that the `.txt` files name in jump or slipway mode is a slipway cancel (`pad_colonise` f287 and f372; `pad_planet_projects` f399 and f505) or leaves aim mode (`pad_probe` f16, shared; `pad_colonise` f173). A ○ at the root that did nothing may still exist without a `.txt` line. Chunk 8 finds it with `make e2e`.
- **H-7-10, failed: the frame counts changed.**
  - Limit 600: `pad_planet_projects` 585 (15 frames of margin), `pad_tech_costs` 566, `pad_research` 468, `pad_colonise` 411, `pad_slipway` 318, `pad_message_box` 190.
  - Limit 180: unchanged from step 6 (`pad_bankrupt` 170, `pad_restart_resume` 155, `pad_empty_planet` 148, and the rest).

  Step 8's edits keep frame indices wherever they replace a press. The few that add frames go in `pad_colonise`, which has 189 frames of margin.
- **Size at `d549a67`** (the plan commit, `src/` as at `57a3d8d`): `pad_nav.c` has 1142 lines, and `pad_nav_t` has 23 fields (`jump` and `slipway` counted as one each).
- **H-7-11, held.** No `.end-state` was re-baselined, there is no exceptions file, and every cheat count is unchanged.
- **H-7-12, failed in its detail.** `make coverage-compare` reports **69 unhit lines** at `57a3d8d`:
  - the network-mode lines of `pad_nav.c` left for step 12 (76–123, 286–287, 297–301, 304–312, 420–425, 430–454, 472, 935–936);
  - `save_state.c:888` (a point-ended slipway skipped in a save, reached only by the old drag over empty space);
  - `slipway_display.c:128` (network mode's selected-slipway colour).

  Step 8's coverage rule: no unhit line beyond these 69. Line numbers in `pad_nav.c` move as step 8 edits it, so the comparison is by `make coverage-compare`'s mapping, not by number.
- **H-7-13, held.** The helpers this PRD's tests use exist at `57a3d8d`:
  - `start_game_in_jump_mode`, `hover_object`, `start_slipway`, `cross_on_target`, `settled_row`, `assert_hovering`, `make_known`, `planet_farthest_outside`;
  - `box_entries`, `is_button_of`, `click_colonise_row`, `world_count`, `assert_world_unchanged`, `type_chords`, `crosshair_pixels_lit`, `take_structure`;
  - `cover_with_icon`, `start_title_with_a_save`, `in_forked_copy`, `triangle_click`, `leave_jump_mode` and `shown_message_box`.

  `tests/test_pad_nav.c` has 165 tests. Step 8 must update these step 7 tests:
  - `test_circle_in_aim_mode_closes_a_box_before_leaving` (an ephemeral box), whose intent is extended by the message-box rung;
  - `test_shoulders_switch_and_aim_pans_under_the_year_box` and `test_start_opens_pause_over_the_year_box`, which pin that the year box survives shoulders and Start. They stay true.
- **H-7-14, held.** `pad_slipway` f0–98 are `pad_colonise`'s, `pad_planet_projects` f0–68 and `pad_empty_planet` f0–28 are `pad_probe`'s, and `pad_resume_at_start` is re-cut from `pad_research`'s last record. `pad_colonise`'s f88 △ on p47 lies inside the part `pad_slipway` shares.
- **New: `test_live_slots` is not in `make test`.** In step 7, its fixtures broke in chunk 2 and went unseen until chunk 3. **Every chunk of step 8 runs `make unit-run-test_live_slots` before its commit**, besides `make test-all`.
- **Step 7's cost**: about 62 minutes of re-authoring (8, 5 + 1 and 5 replays per chunk). Step 8's note records its own cost the same way.

## Facts from the code at `HEAD` (`57a3d8d`)

These facts were read with `git show HEAD:<path>` at `57a3d8d`, after step 7.

- **What a mouse press does on a planet** (`planet_left_press`):
  - on a planet in state KNOWN, the press opens its projects panel (`planet_projects_panel`, a message box whose rows are buttons carrying `project_planet`) and stops the dispatch;
  - on a SETTLED planet, it hands the ui a new slipway. The release over the same planet opens the inspection panel (`planet_accept_drop` → `planet_inspection_panel`).

  So jump mode's △ click on a known planet already opens the projects menu, on the press frame. "× on a known planet opens its projects menu, as a mouse press there does" is the same click.
- **△ in jump mode** (`jump_step`): `triangle_on_hovered` clicks only when `pressable(hovered)` is true (the rest point is inside the inner rectangle and no HUD item covers it). On a click's press and release frames, no jump runs, and the camera and the marker wait.
- **× in jump mode**: `cross_starts_slipway` only, gated by `pad_slipway_can_start`, `pressable` and `left_was_down`. Otherwise × does nothing. `test_cross_and_circle_in_jump_mode_change_nothing` freezes that, including × on a known planet.
- **○ in jump mode** does nothing: `jump_step` takes no ○ argument.
- **The existing ○ rungs** (in `pad_nav_step`, `switch_tab`, `pause_step`, `placement_step` and `slipway_step`):
  - `circle_returns` is ○ pressed with no box open, so a ○ that closes a box this frame reaches no mode (`circle_closes_the_box`, which leaves pause to `pause_step`);
  - ○ closes pause (`pause_step`);
  - ○ on a tab other than the map closes its panel and returns to the map (`switch_tab`, `leave_tab`);
  - ○ cancels the carried structure (`placement_step` → `cancel_point`);
  - ○ cancels the slipway back on its start (`slipway_step`);
  - ○ leaves aim mode (`aim_mode_step` → `leave_aim_mode`).

  No rung closes a message box: `circle_returns` stays 1 under the year box, so the ○ reaches the mode below it.
- **Panels on the map tab** go to `panel_step` → `pad_panel_step`, which reads only the D-pad and ×: △ does nothing inside a menu. `pad_panel_step` writes the left button while × is held on a focus. Only pause has an arming rule (`pause_cross_armed`: a × held into pause presses no row until pressed again).
- **`planet_menu_open(nav)`** is "map tab, an ephemeral box open, not pause". With network mode gone, the only ephemeral boxes on the map tab are those jump mode's △ click opens: a known planet's projects menu and a colony's or structure's inspection panel. The tech box is on the tech tab, and pause is excluded. The year box and other message boxes are not ephemerals, so `planet_menu_open` is false under them.
- **Message boxes** (`message_box.c`): `message_box_construct` kills the previous shown box, so at most one is shown (`shown_message_box()`). It closes itself in `message_box_draw_overlay` once its time runs out (marked dead, the shown box cleared), with `keep_open` extending it (the year box's countdown keeps it 90 ticks). The pad has no way to close one today. The `make state` and end-state fields do not count message boxes (`tools/end_state.sh` keeps only the planets, colonies, slipways, structures, techs and nodes counts).
- **The jump state** (`pad_jump_t`): `hovered`, `game`, `seeded` and the repeat fields. `pad_jump_init` runs at each new game (`start_new_game`). `reset_map_mode` keeps `game` and `seeded`.
- **Slipway mode's outcome** (`pad_slipway_rebase`): when built, the start moves to the target. When refused, the start stays.
- **A KNOWN planet can be replaced** by a project with a `class_name` (`planet_apply_project` kills it and creates the new kind with the same position). Colonising ("nanotech", "strip" and so on) keeps the same entity and sets it SETTLED.
- **The chord rule** runs first in `pad_nav_step` (`without_chord`: × and ○ going down on the same frame, against `previous_pad`). `cheat_step` counts a chord when both edges fall on one frame (against its own `previous_cross` and `previous_circle`). A lone × or ○ resets its count. No test checks that the two definitions agree.
- **The candidate lists.**
  - `pad_jump_candidates(nodes, storage, capacity, out)` and `pad_slipway_candidates(candidates, start, storage, capacity, out)` fill a caller-owned `entity_t *storage[]` and write its address into `*out`.
  - `pad_nav.c` declares `entity_t *storage[JUMP_CANDIDATES_CAP]` (and `others[...]`) as locals in `leave_aim_mode`, `seed_hovered`, `jump_from` and `slipway_step`.
  - `JUMP_CANDIDATES_CAP` is 128, "above the TAG_NODE reservation in cart.c" (`[TAG_NODE] = 110`).
  - `tests/test_pad_jump.c`, `tests/test_pad_slipway.c` and two `tests/test_pad_nav.c` helpers call both functions with local arrays.
  - `entity_list_filter(zone, list, predicate, ctx)` (engine) returns a new list grown in `zone`. `probe.c`, `slipway.c` and `economy_score.c` call it with `frame_zone()`.
- **The pointer guard** (`tools/pointer_guard.sh`, `make pointer-guard` in `make test`, the ratchet `tools/pointer_guard_exceptions.txt`, tested by `tests/test_pointer_guard.c`) counts data pointers in struct fields and file-scope variables, and `entity_list_t` declarations. It does not count local arrays of entity pointers.
- **The e2e limits check** (`tools/e2e_limits.sh`, run by `make e2e`) counts a replay's frames as its `"pad"` keys, and checks the limit, the companions, `"input": "pad"` and the mouse fields. It does not read the `.txt`. Every `.txt` states its length in its first paragraph:
  - 15 as "N frames.";
  - `pad_message_box.txt` as "190 frames (listed in tools/e2e_long_exceptions.txt).";
  - `pad_tech_costs.txt` with "566" and "frames." split across two lines.
- **The tests** at `HEAD` that touch what step 8 changes:
  - `test_cross_and_circle_in_jump_mode_change_nothing` (× on a known planet, and ○ in jump mode);
  - `test_triangle_on_a_hovered_known_planet_opens_its_menu_and_circle_comes_back`;
  - `test_triangle_on_a_hovered_colony_opens_its_panel_and_no_jump_shares_the_click`;
  - `test_triangle_on_a_hovered_object_still_scrolling_in_presses_nothing`;
  - `test_building_and_backing_out_never_types_the_cheat` (two ○ after a build: cancel, then ○ at the root while hovering the built target);
  - `test_circle_closes_the_pause_menu`, `test_circle_on_tech_closes_the_box_and_returns_to_the_map_in_one_press` and `test_circle_on_a_tab_returns_to_the_map`;
  - `test_circle_cancels_the_slipway_back_on_its_start` and `test_circle_cancels_the_structure_and_charges_nothing`;
  - `test_four_chords_on_a_hovered_colony_cheat_and_change_nothing_else` and `test_a_chord_on_a_planet_menu_presses_no_row_and_closes_nothing`.

## Problem Statement

On the map's root (jump mode), the player points at a thing, but the buttons still do not follow "× acts on it, ○ backs out":

- **× on a known planet does nothing.** The player has to remember that colonising goes through △, while × is the button that acts everywhere else (a colony starts a slipway, a menu row is chosen).
- **△ opens the planet menu but cannot close it.** Section 1 measured △ pressed 6 times in the planet menu without effect: the player uses △ as a toggle. Only ○ closes it, and ○ was never pressed there.
- **○ does nothing at the root.** Section 1 measured 12 dead ○ presses. After a slipway, a menu or some jumps, the player has no one-button way back to the object they were last working on.
- **The ○ order is not stated anywhere as one rule** the player can count on: each mode has its own ○, and the root has none.

## Solution

**× on a known planet.** In jump mode, × on a hovered known planet opens its projects menu, exactly as △ does on it: the same click, at the planet's screen point, once the planet is inside the inner rectangle and clear of the HUD. A planet still scrolling in, or under a HUD item, presses nothing, and no later frame retries. Inside the menu, the D-pad and × work as today. The × that opened the menu chooses no row: × has to be released and pressed again on a row.

**△ toggles the planet menu.** △ on the hovered object opens its planet menu (the projects menu for a known planet, the inspection panel for a colony or structure). △ again closes it, and so does ○. In both cases the cursor stays on the same object in jump mode. △ closes only the menu that jump mode's △ or × opened: under the year box, a message box, pause or a tab's panel, △ still does nothing.

**○ always goes back one step**, in this order. Each ○ does exactly one rung, the first that applies:

1. **A box or panel is open**: ○ closes it, the topmost first: pause, then a planet menu or inspection panel, then the year box or another message box. On another tab, ○ closes that tab's panel and returns to the map, as today (a year box shown there stays). The mode underneath is unchanged: a slipway stays held, a structure stays carried, aim mode stays.
2. **A slipway or a placement is in progress**: ○ cancels it. A slipway returns to jump mode on its start. A structure is cancelled and the map stays in aim mode.
3. **Aim mode**: ○ leaves it for jump mode, on the target nearest the centre (step 7).
4. **The root (jump mode)**: ○ jumps to the last object × acted on, as a D-pad jump would: the hovered marker moves and the edge scroll brings it in. ○ does nothing when:
   - × has acted on nothing yet in this game (game start, and right after a resume);
   - that object is already hovered;
   - that object is gone or no longer a target.

"The object × acted on" is the object a × press went out to on the map:

- the known planet whose projects menu × opened;
- the colony or structure × started a slipway from;
- the target × built a slipway to.

× in aim mode (probe or fold), × on a menu row, and a × the game refused (a full slipgate, a refused slipway, a planet not yet pressable) change nothing.

## User Stories

1. As a player, I want × on a hovered known planet to open its projects menu, so that × is the button that acts on whatever I point at.
2. As a player, I want × and △ on a known planet to open the same menu, so that I don't have to remember which button colonises.
3. As a player, I want the × that opened the menu not to choose a row, so that one press never opens a menu and buys a project at once.
4. As a player, I want a second × on a row to choose it as today, so that colonising stays two deliberate presses.
5. As a player, I want × on a known planet that is still scrolling into view to do nothing, so that I never open a menu half off screen.
6. As a player, I want × on a known planet under a HUD item to do nothing, so that × never presses the HUD by accident.
7. As a player, I want a refused × never retried on later frames, so that nothing happens behind my back.
8. As a player, I want × on a colony to keep starting a slipway, so that step 6's building is unchanged.
9. As a player, I want △ again to close the planet menu, so that △ works as the toggle I already use it as.
10. As a player, I want △ again to close a colony's inspection panel too, so that the toggle works on every planet menu.
11. As a player, I want the cursor to stay on the same object after the menu closes, so that I continue from where I was.
12. As a player, I want △ to leave the year box, message boxes, pause and the tech and score panels alone, so that △ only ever means "this object's menu".
13. As a player, I want ○ to close any open box or panel first, so that one ○ never closes a menu and moves my cursor at once.
13a. As a player, I want ○ on the map to close the year box or a message box before anything else, so that the banner I am looking at goes away before ○ backs out of my mode.
14. As a player, I want ○ to cancel a slipway in progress before anything else on the map, so that backing out of a build is one press.
15. As a player, I want ○ to cancel a carried structure and keep me in aim mode, so that I can aim again without re-entering it.
16. As a player, I want ○ in aim mode to take me back to jump mode, so that ○ is always "back".
17. As a player, I want ○ at the root to jump to the last object I acted on with ×, so that I get back to my work after looking around.
18. As a player, I want that jump to scroll the camera as a D-pad jump does, so that the object comes into the comfortable area without a sudden snap.
19. As a player, I want ○ at the root to do nothing at game start, so that the first ○ of a game is harmless.
20. As a player, I want ○ at the root to do nothing when I am already on that object, so that a repeated ○ is harmless.
21. As a player, I want ○ at the root to do nothing once that object is gone, so that ○ never jumps to something that no longer exists.
22. As a player, I want a slipway I built to make its target the object ○ returns to, so that ○ follows my chain of slipways.
23. As a player, I want a refused × to leave the ○ target unchanged, so that a failed press does not move my anchor.
24. As a player, I want a menu row choice not to change the ○ target, so that the anchor is always a place on the map.
25. As a player, I want a resumed game to start with no ○ target, so that a resume behaves like a new game.
26. As a player, I want ○ during a △ click's two frames to do nothing at the root, so that the cursor never jumps away while a panel is opening.
27. As a player, I want the four-chord cheat to open no menu and jump nowhere, so that the dev cheat stays inert outside its effect.
28. As a player, I want ordinary ×, △ and ○ play never to fire the cheat, so that my game stays honest.
29. As a replay author, I want × on a known planet to produce the very same frames as △ on it, so that swapping one for the other keeps every frame index and capture.
30. As a replay author, I want a ○ that used to do nothing at the root replaced by an idle frame, so that later frame indices stay the same.
31. As a replay author, I want shared prefixes re-authored first and reused verbatim, so that the work is done once.
32. As a replay author, I want each new behaviour (× opening a projects menu, △ closing a menu, ○ jumping to the last object) produced at least once in a replay, so that the e2e covers what the player will use.
33. As a reviewer, I want every re-authored replay to pass `make end-state` against `8defcba` with its cheat count kept, so that no game end state changes.
34. As a reviewer, I want each replay's old and new `.txt` compared in the same commit, so that no situation disappears silently.
35. As a reviewer, I want one unit test per rung of the ○ order and one for ○ at game start, so that the order is pinned rung by rung.
36. As a reviewer, I want `make coverage-compare` to report no newly unhit line, so that coverage does not drop.
37. As a reviewer, I want the re-authoring cost and the replays broken recorded, so that later steps can be judged.
38. As the agent of step 9, I want the new frame counts of the year-heavy replays recorded, so that the Select-hold budget can be re-read.
39. As the agent of step 14, I want the last object × acted on readable from outside the pad navigation, so that the metrics tool can tell a required dead ○ at the root from any other.
40. As a developer, I want the last-object state in its own `pad_back_t`, not in the jump cursor's state, so that each mode's state stays where section 2 puts it and step 12 deletes old code without touching it.
40a. As a developer, I want the candidate lists built by the engine's filter into the frame zone, so that no game file sizes a local pointer array from a number in `cart.c`.
40b. As a reviewer, I want each `.txt`'s "N frames." checked against its replay by `make e2e`, so that a re-authored replay's notes cannot go stale on its length.
40c. As a reviewer, I want one unit test to prove the pad's chord and the cheat's chord agree, so that a cheat change cannot desynchronise them silently.
41. As a developer, I want `make test` and `make e2e` no slower, so that the fast loop stays fast.

## Implementation Decisions

### The pure refactor (before any mapping change)

- **Candidate lists through `entity_list_filter`.**
  - `pad_jump_candidates` becomes `entity_list_t pad_jump_candidates(const allocator_t *zone, const entity_list_t *nodes)`: the entries of `nodes` that pass `pad_target_is_candidate`, in order, in a list grown in `zone`. A NULL `nodes` gives an empty list.
  - `pad_slipway_candidates` becomes `entity_list_t pad_slipway_candidates(const allocator_t *zone, const entity_list_t *candidates, const entity_t *start)`: the entries other than `start`, in order.
  - Both are built on `entity_list_filter`, as in `probe.c`. The header comment states the C contract: the list's storage belongs to `zone`, so with `frame_zone()` it lives until the end of the frame and the caller frees nothing.
  - `pad_nav.c` passes `frame_zone()`.
- **`JUMP_CANDIDATES_CAP` is deleted**, with every local `entity_t *storage[...]` and `others[...]` array in `pad_nav.c`. Nothing then depends on `cart.c`'s TAG_NODE reservation. Its assert ("raise the caller's cap") goes with it. The frame zone has no fixed cap of 128.
- **The tests** follow: `tests/test_pad_jump.c`'s and `tests/test_pad_slipway.c`'s candidate tests keep their layouts and assertions (order kept, the start dropped, an empty result), with `frame_zone()` or a test zone in place of the local array. The two `tests/test_pad_nav.c` helpers do the same. No test is deleted (section 2's rule: the mechanic survives).
- **The pointer guard counts local entity arrays.** `tools/pointer_guard.sh` gains a third count per file of `src/game/`: local declarations matching `entity_t *name[...]` (const ones included), recorded in `tools/pointer_guard_exceptions.txt` as `entity-arrays <file> <count>`. Strict equality, as for the other counts.
  - The ratchet is written from the tree after the refactor, so `pad_nav.c` has 0.
  - Files with other such arrays keep theirs, listed with their counts.
  - `tests/test_pointer_guard.c` gains cases: one array counted, a pointer that is not an array not counted, a count above the list failing, and one below it failing.
  - It stays one script in `make test`, milliseconds, not a sibling.
- **The check that it is pure**:
  - every `*.json.sha256` and every `.end-state` unchanged;
  - `make state`'s peaks within their 1/8 margins. The frame zone now holds the candidate lists, so `zone frame` may rise; the margin rule decides, and the step note records the new peak;
  - `make coverage-compare` with no line beyond step 7's 69;
  - `make unit-run-test_live_slots` green.

### The `.txt` frame-count check

- `tools/e2e_limits.sh` also reads each replay's `.txt`. Its first paragraph, with lines joined by single spaces, must contain exactly one `<N> frames.`, and N must equal the replay's frame count (its `"pad"` keys, the count the limit check already makes). Otherwise it reports `e2e: <events>: .txt says <N> frames, the file has <M>` (or `states no "N frames."`) and fails, after examining every file, as the script does today.
- The two `.txt` files out of line are aligned in the same commit:
  - `pad_message_box.txt`: "190 frames (listed in tools/e2e_long_exceptions.txt)." becomes "190 frames." followed by "Listed in tools/e2e_long_exceptions.txt."
  - `pad_tech_costs.txt`: the paragraph is re-wrapped so "566 frames." sits on one line. The joined-paragraph rule would accept the split anyway; the alignment is for grep.
- The script's tests (wherever `e2e_limits.sh` is tested; if nowhere, the check is verified by a scratch `.txt` with a wrong count failing, recorded in the step note) gain:
  - a right count passing;
  - a wrong count failing;
  - a missing statement failing;
  - two statements in the first paragraph failing.
- `make e2e` must not get slower: one `awk` over 17 short files.
- From this chunk on, every re-authored replay's `.txt` updates its "N frames." in the same commit, and the check enforces it.

### The cheat chord consistency test

- A unit test in `make test` (in `tests/test_pad_nav.c`, or a test file of its own when it needs no game) drives one button sequence through both `cheat_step` and `pad_nav_step`. It asserts that the frames on which the pad hides × and ○ from the modes start exactly at the frames `cheat_step` counts as chords, and that the cheat fires on the fourth.
- The sequence covers:
  - chords one and two frames apart;
  - × then ○ on consecutive frames (no chord);
  - ○ held then × pressed (no chord);
  - a chord with × still held after ○ is released;
  - a chord typed while a D-pad direction is held.
- "Hide" is observed from outside: on a chord frame and while both have not been released, × presses nothing (no slipway, no menu) and ○ cancels nothing. The test asserts that through the played frame and the modes, never `chord_held`.
- Step 8 does not change the cheat. The test exists because section 2 asks for it now, and it guards this step's cheat re-check.

### The last object × acted on (its own struct, `pad_back_t`)

- **A new pure module `pad_back`** (in the style of `pad_jump`, `pad_slipway` and `pad_target`) holds the ○ back order's state: `pad_back_t`, with one field, `last_acted` (an entity id). `pad_nav_t` gains one field, `back` (a `pad_back_t`).
  - It does not go in `pad_jump_t`: plan step 8 and section 2 forbid it. `pad_jump_t` is the jump cursor's position and repeat, while the anchor belongs to ○'s rung 4 and survives slipway mode, aim mode, panels and tabs.
- **Functions**:
  - `pad_back_init(back)`: records nothing.
  - `pad_back_record(back, e)`: sets `last_acted` to `e`. A NULL `e` leaves it as it was.
  - `pad_back_target(back, hovered)`: the `last_acted` entity when it is a `pad_target_is_candidate` and is not `hovered`. NULL otherwise, including when nothing was recorded.
- **Lifetime**: `pad_back_init` runs in `pad_nav_init` and in `start_new_game` (beside `pad_jump_init`), so the anchor is empty at every new game and after every resume. `reset_map_mode` does not touch it.
- Unit-tested in a new `tests/test_pad_back.c` on hand-built entities, as `tests/test_pad_jump.c` builds its own.

### Where × records it (in `pad_nav`)

- × on a hovered known planet that clicked (below): record the planet.
- × that started a slipway (`cross_starts_slipway` returned 1): record the start.
- × that built a slipway: record the target, on the frame `slipway_step` reads the outcome as `PAD_SLIPWAY_BUILT`. A refused outcome records nothing. The start × recorded earlier stays the anchor.
- Nothing else records: not aim-mode ×, panel ×, build-tab ×, placement ×, refused presses, △ or any D-pad move.
- An accessor for tests and step 14: `pad_nav_last_acted(nav)`, the recorded object while a game runs, or NULL when none was recorded or it is no longer a candidate. It does not depend on the map mode. Its header comment states that contract.

### × on a known planet (jump mode)

- **When.** In `jump_step`, × is pressed, no click is in progress (neither its press nor its release frame), and the hovered object is in state KNOWN.
- **What.** The same click as △ (`triangle_on_hovered`, renamed `click_hovered` because △ and × now share it). It is gated by `pressable(hovered)`: the press goes out at the hovered object's screen point on this frame, with a forced release on the next. `planet_left_press` opens the projects menu on the press frame. The map mode stays jump and the hovered object stays.
- **Refused** (not `pressable`): no press, nothing recorded, no retry on a later frame, and the frame runs as a plain jump-mode frame (marker, edge scroll). It is the same rule as △ and as a refused slipway start.
- **The order in `jump_step`** stays △ first, then ×. A known object goes to the click and a settled one to `cross_starts_slipway`. × and △ on the same frame give one click.
- **Generality.** "Known" means state KNOWN of any hovered candidate. Every KNOWN candidate is a planet whose left press opens its projects panel. No other left press is introduced.

### The × held from the opening click

- A new field, `menu_cross_armed`, works like `pause_cross_armed`. It is a loose `pad_nav_t` field, which section 2 allows with a reason, recorded in the step note: it is no mode's state but the twin of the existing loose `pause_cross_armed` (the same rule for the panel × under a box the pad just reached), and the two belong together in whatever struct a later refactor gives the panel arming. While the planet menu is up (`planet_menu_open`, next section), × presses no row until it has been released and pressed again. `panel_step` masks × out of `pad_buttons` until then.
- Why: `pad_panel_step` presses the focused row while × is held, and the focus starts on the menu's header. Without the arming, the opening × would press the header or a row on the frame after the release.
- The flag is cleared by jump mode's click (from △ or ×) and set by the first × press edge while the planet menu is up. △ opens the menu with × up, so the arming changes nothing on the △ route.

### △ again closes the planet menu

- **The planet menu is `planet_menu_open(nav)`** (see Facts): on the map tab, every ephemeral box except pause is one jump mode's click opened, and the year box and message boxes are not ephemerals. No new field is needed.
- **When.** `planet_menu_open(nav)`, the frame is neither a click's press frame nor its release frame, and △ is pressed.
- **What.** The box is cleared (`ui_clear_ephemerals`, as ○'s `circle_closes_the_box` does). No mouse button goes down, no row is pressed, the map mode stays jump and the hovered object stays. The marker shows again from the next frame, as after ○.
- △ under any other box (the year box, a message box, pause, the tech box) still does nothing. With the year box and a planet menu both up, △ closes the planet menu only.
- A trap the implementer must not reintroduce: if a later step adds an ephemeral box on the map tab that is not a planet menu, `planet_menu_open` stops meaning "the planet menu". An `assert` in the △ branch states the assumption (the box's rows are the hovered object's, or it is its inspection panel), or a comment of at most 40 words does.

### The ○ back order

- **Rung 1 (box or panel)** is in place for pause (`pause_step`), ephemeral boxes (`circle_closes_the_box`, with `circle_returns` 0 while one is open) and tabs (`switch_tab`). **New: the message box.**
  - **When.** ○ is pressed on the map tab, pause is not shown, no ephemeral box is open, and `shown_message_box()` is not NULL. This covers the year box and the score box running out after its tab closes.
  - **What.** The shown box is closed the way its own timeout closes it: marked dead and the shown box cleared. This is a new function in the message box module, `message_box_close_shown()`, which does nothing when no box is shown. The frame's ○ is then consumed: `circle_returns` is 0, and `placement_step` must not see the ○ either (it reads `pressed` directly, so the ○ bit is masked out of `pressed` for the map step on that frame). No mouse button goes down. The mode, the hovered object, a held slipway and a carried structure are unchanged.
  - **Order with the other boxes.** Pause first (`pause_step` runs before), then an ephemeral box, then the message box: one box per ○.
  - **Not on other tabs.** On a tab other than the map, ○ keeps returning to the map (`switch_tab`), and a year box shown there stays. That keeps `pad_message_box`'s f62 ○ on `end turn` as it is.
  - **Why it is a pad rule and not a game rule.** The box is a timed banner that no game rule reads: no end-state field counts it, and closing it early changes no economy, planet or random draw. ○ already clears ephemeral boxes directly (`ui_clear_ephemerals`). The timer and `keep_open` are untouched for boxes ○ does not close.
  - **Chord frames**: a chord hides ○, so four chords close no message box.
- **Rung 2 (slipway or placement)** is already in place: `slipway_step`'s ○ and `placement_step`'s ○ (H-7-5, H-7-6). Kept as is.
- **Rung 3 (aim mode)** is step 7's (H-7-1). Kept as is.
- **Rung 4 (root), new.** `jump_step` takes `circle_returns`. When it is set, no click is in progress (neither press nor release frame), and `pad_back_target` returns an object:
  1. the hovered object becomes that object;
  2. the hold-repeat state keeps running (a held direction does not jump on this frame);
  3. the frame then runs as a D-pad jump that landed there: edge scroll toward it (at most 8 px per axis per frame), the marker on it, and the cursor at its screen point.

  ○ and a D-pad press on the same frame: ○ wins and no D-pad jump runs. ○ is not deferred: a ○ dropped on a click frame is gone.
- **Nothing to go back to**: when `pad_back_target` is NULL, ○ is a plain jump-mode frame. Nothing is pressed, nothing moves, and the cursor stays.
- **One ○, one rung.** The rungs are checked in the order above on the frame's state before the press. Rung 1 (pause, an ephemeral box or the message box) consumes the ○ for every mode. Modes 2 and 3 return to jump mode on that frame without running `jump_step`. So one ○ can never do two rungs, and each ○ below rung 1 changes the mode only on its own frame.

### The cheat (section 2's re-check)

- **What changes for × and ○**: × on a known planet now clicks it, ○ on the map closes a message box first, and ○ at the root now jumps.
- **Ordinary play**: `cheat_step` counts only frames where × and ○ go down together (H-7-4). No new action needs both on one frame: × opening a menu, △ or ○ closing it, and ○ jumping are single-button presses, and each lone × or ○ resets the count. Ordinary play still cannot complete the sequence, so the cheat is not changed.
- **The deliberate route**: the chord rule hides × and ○ from every mode before `jump_step` runs. Four chords on a hovered known planet, with a last object elsewhere, open no menu and jump nowhere. A new unit test checks it (case 17). Step 6's route holds.
- Each replay's cheat count is checked by `make end-state`, as always.

### Unchanged

- Aim mode, slipway mode, placement, the build tab, the tab ring, pause, and the panels' D-pad and × (except the arming above).
- △ on a hovered object, in all its rules (`pressable`, click frames, no jump during a click).
- × on a colony or structure (step 6).
- `ui_set_pad_selection`, the crosshair rule and `ui_set_pad_tab`.
- Network-mode code, which waits for step 12 (H-7-2).

## Implementation Chunks

Each chunk ends on a commit where `make test-all` is green. Section 2's checks outside `make test-all` also run on **every** chunk, since every chunk here touches pad input, a replay or the tools that judge them:

- `make unit-run-test_live_slots` (not in `make test`; step 7 found its fixtures broken a chunk late);
- `make end-state EVENTS=` on every touched replay (all 17 for chunk 1);
- `make coverage-compare` (nothing beyond step 7's 69);
- `make rng-sync-probe` whenever `pad_colonise` changes.

A chunk whose code change breaks a replay re-authors it in the same commit: its `.json`, its `.txt` (with its "N frames." from chunk 2 on), its `.json.sha256` and its pinned values. Within a chunk, shared prefixes come first (H-7-14): `pad_probe` → `pad_colonise` → `pad_slipway`, then `pad_planet_projects`, `pad_empty_planet` and `pad_restart_resume`, then `pad_research` → `pad_resume_at_start`, then the others.

1. **Pure refactor: candidate lists, the cap and the pointer guard.** No mapping change, and it is the step's first commit (plan step 8's Method).
   - `pad_jump_candidates` and `pad_slipway_candidates` are built on `entity_list_filter` with a zone, and `pad_nav.c` passes `frame_zone()`.
   - `JUMP_CANDIDATES_CAP` and every local `entity_t *storage[...]` / `others[...]` array go.
   - `tools/pointer_guard.sh` counts local `entity_t *name[...]` arrays in `src/game/` (`entity-arrays` lines in its ratchet), with new `tests/test_pointer_guard.c` cases.
   - The candidate tests are rewritten for the new signatures; none is deleted.
   - Checked by:
     - every `*.json.sha256` and `.end-state` unchanged;
     - `make state` peaks within their margins (the step note records the frame-zone peak before and after);
     - no unhit line beyond the 69;
     - `make unit-run-test_live_slots` green.
2. **The `.txt` frame-count check.** `tools/e2e_limits.sh` checks the first paragraph's "N frames." against the `.json`, with its tests. `pad_message_box.txt` and `pad_tech_costs.txt` are aligned. No replay changes: every hash and end state is unchanged.
3. **The cheat chord consistency test** (`cheat_step` against `without_chord`), in `make test`. Test only: no code change, unless the test finds the two definitions disagreeing. In that case the fix is single-sourcing the chord in this chunk, with every replay's cheat count unchanged.
4. **`pad_back`.** The module, `pad_back_t` and its three functions, and `tests/test_pad_back.c`. `pad_nav_t` gains `back`, initialised, read by nothing yet. No behaviour change: every e2e and `.end-state` stays as it is.
5. **× on a known planet, the arming and the recording.**
   - Code: `click_hovered` shared by △ and ×; × on a KNOWN hovered object; `menu_cross_armed`; `pad_back_record` at the three × sites; `pad_nav_last_acted`.
   - Unit tests: cases 7–12 (`test_cross_and_circle_in_jump_mode_change_nothing` is rewritten here, since its × on a known planet no longer holds), and case 17's four chords on a hovered known planet.
   - Re-authoring: only the replays this breaks (H-7-8): a × in jump mode on a hovered known planet that used to do nothing. Each is replaced by an idle frame, which keeps every later index. Expected: none.
6. **△ again closes the planet menu.** Code in `panel_step` or `map_step`. Unit tests: cases 13–16. Re-authoring: only replays that pressed △ inside a planet menu opened from jump mode (it used to do nothing). The △ is replaced by an idle frame. Expected: none.
7. **○ closes the message box first** (rung 1 completed).
   - Code: `message_box_close_shown()` in the message box module, and the rung in `pad_nav_step` (the ○ consumed for the map step).
   - Unit tests: case 1's message-box parts, and `test_circle_in_aim_mode_closes_a_box_before_leaving`'s year-box companion (case 1b).
   - Re-authoring: every replay with a ○ on the map tab while a year or score box is shown. Each now closes the box instead of reaching its mode, so the replay gets a second ○ on the next idle frame (the old effect, one frame later), or waits until the box has closed. Where the replay's later frames depend on the box being drawn, the box-closing ○ is the second one. Expected: few. `pad_message_box`'s ○ presses are under pause or on a tab, so they do not change. `make e2e` decides.
8. **The root rung of ○.**
   - Code: `jump_step` takes `circle_returns`, and ○ jumps to `pad_back_target`.
   - Unit tests: cases 1–6, 17's ordinary sequence, 18 and 19.
   - Re-authoring: every replay with a ○ in jump mode that now jumps (H-7-9). Each such ○ is replaced by an idle frame where nothing depended on it. If a later frame depended on the cursor staying put, the ○ is kept as the new rung (the jump) and the D-pad route that follows is re-derived. Rule 8 applies after it.
9. **Re-authoring that covers the new behaviours** (shared prefixes first). No code. Each new situation is produced once and named in the replay's `.txt`:
   - **× opens the projects menu and colonises from it**: `pad_colonise`'s first colony (p47). The jump-mode △ that opens p47's menu (f88, after f86's □, H-7-7) becomes × on the same frame. The colonise row's × that follows is a fresh press, so it chooses the row on the same frame as before. × and △ play the same click, so the expected capture is byte-identical. The `.txt` changes, and the `.json.sha256` is re-baselined only if the capture differs. Because this frame lies inside the part `pad_slipway` shares (f0–98), `pad_slipway` gets the same one-frame edit.
   - **The △ route is kept**: `pad_colonise`'s second colony (p25), the menus of `pad_planet_projects`, `pad_tech_costs` and `pad_research` stay on △. "△ on a known planet opens its menu" is not a removed situation.
   - **△ again closes a menu**: in `pad_colonise`, after its inspection panel is closed with ○ (f139 at `HEAD`, kept), add △ (the panel opens on the release), then △ again (the panel closes), on idle frames that exist or added before f146's □. The frames each costs are paid inside the replay (411 of 600).
   - **○ at the root jumps to the last object**: in `pad_colonise`. Once f88's △ becomes ×, p47 is the last object from f88 on. f262's □ lands jump mode on p47, and f266's Up moves the cursor onto p25. A ○ there jumps back to p47, and a second Up returns to p25 before f269's × on it, so the slipway keeps its direction p25 → p47. After the jump, the replay re-pans to the reference camera before its next point action in aim mode (rule 8). `pad_colonise` f173's ○ is aim mode's (rung 3), unchanged. After f287's or f372's slipway cancel, the cursor is on p47, which is the last object, so a ○ there is the designed no-op of the next item. If no such frame exists in `pad_colonise` within its limit, `pad_planet_projects` produces it (its chain s3 → p47 → s2 makes s2 the last object), and the step note says which.
   - **○ at the root that does nothing because the cursor is on the last object**: produced where it falls naturally, for example the second ○ after a slipway cancel. Step 14 needs one such designed no-op in a replay. Not required here if none falls naturally; the step note says whether one exists.
   - **`pad_planet_projects`**: re-authored if chunks 5–8 broke it (585 of 600: any added frame is paid by compressing idle frames). Otherwise its only planned change is none: its "X taps on a planet open nothing" stay in aim mode (H-7-8). If it took the ○ situation above, it is re-authored for that.
   - **`pad_empty_planet`**: the plan names it. Its × presses are the build tab's and placement's, and it never hovers a known planet with × or presses ○ at the root, so no step 8 rule changes its frames. Decision: it is re-authored only if `make e2e` breaks it. Otherwise it stays byte-identical, and the step note records "named by the plan, not broken, unchanged" with this reason. Re-authoring a green replay adds risk and covers nothing new.
   - Each touched `.txt` updates its "N frames." (checked since chunk 2).
   - `make rng-sync-probe` is re-run because `pad_colonise` changes. Its quiet window {288, 410} moves with the frames added after f139.
10. **The step note** in `docs/plan-12.md`:
   - the replays broken per chunk, those re-baselined and those re-authored, and the agent time spent re-authoring;
   - `pad_nav.c`'s line count (1142 before) and `pad_nav_t`'s field count (23 before) after the step, and why `menu_cross_armed` is a loose field;
   - the frame-zone peak of `make state` before and after chunk 1, and the `entity-arrays` counts the pointer guard's ratchet recorded;
   - the chord consistency test's verdict (agreeing, or the single-sourcing done);
   - the tests deleted, each next to the mechanic it exercised (expected: none, since step 8 removes no mechanic), and the tests rewritten;
   - the new situations and the replay and frame of each;
   - the cheat verdict;
   - the replays the message-box rung broke and how each was re-authored;
   - the `make coverage-compare` result (no unhit line beyond step 7's 69);
   - the frame counts of `pad_colonise`, `pad_planet_projects`, `pad_tech_costs`, `pad_research`, `pad_bankrupt` and `pad_message_box`, for step 9's budget;
   - `pad_empty_planet`'s status;
   - each fact of "Facts from step 7" the implementation found different, and what was done instead.

If a later chunk breaks a replay an earlier chunk re-authored, it is re-authored again in the later chunk's commit, and the note counts the time.

### How to find what breaks, per chunk

1. Run `make e2e` after the code change.
2. For each red replay, `make e2e-ref` gives the first diverging frame, and the `.txt` names the press there.
3. Classify the press:
   - × in jump mode on a known planet that did nothing: an idle frame;
   - △ inside a pad-opened menu that did nothing: an idle frame;
   - ○ on the map under a year or score box: a second ○ on the next idle frame (chunk 7);
   - ○ at the root that did nothing: an idle frame, or the new rung kept (chunk 8).
4. Re-author it, then run `make end-state EVENTS=<replay>` and `make unit-run-test_live_slots`, and compare the old and new `.txt`.

## Re-authoring

- **Discipline**: the 8 rules of step 2b. This step moves no probe, fold, placement or slipway, and adds no reveal, so every end state is expected to match `8defcba`. A replay that cannot reach its reference falls under section 2's relaxed-comparison fallback (H-7-11). Expected: none.
- **Keeping frame indices**: every replacement above (× for △ on a known planet, an idle frame for a press that did nothing) is one frame for one frame. Only the added △ open/close pair and the ○ jump with its re-pan cost frames. They are paid inside `pad_colonise` (or `pad_planet_projects`), compressing idle frames first if a limit is close (H-7-10).
- **Camera discipline** (rule 8): the ○ jump edge-scrolls the camera. Before the next probe, fold, placement or structure point action in aim mode, the replay re-pans to the reference camera.
- **Pinned values** (step 2a's inventory), re-derived in the commit that changes the replay they read, each keeping its intent:
  - `tests/test_live_slots.c`'s frames and fixtures on `pad_planet_projects`, `pad_tech_costs` and `pad_empty_planet`, and its SLIPWAYS fixture (re-authored by step 7, 625 frames), if those replays shift; checked by `make unit-run-test_live_slots`, which `make test` does not run;
  - `tests/test_state.c` on `pad_tech_costs` (camera `101 242` since step 7) and `pad_research`, if they change;
  - `tests/test_rng_sync.c`'s windows on `pad_colonise` (the quiet window {288, 410} since step 7), then `make rng-sync-probe`. `pad_colonise`'s △ pair and root ○ shift frames after f139, so the windows move with them;
  - the golden frame hashes, the `.txt` and the `.json.sha256`.

  The `.end-state` references are never re-baselined.
- **Coverage**: `make coverage-compare` after each re-authoring commit. It must report no unhit line beyond step 7's 69 (the network-mode lines of `pad_nav.c`, `save_state.c:888` and `slipway_display.c:128`): step 8 deletes no old code and removes no mechanic. The △ route to the projects menu stays covered by the menus that keep △. A newly unhit line means a replay is re-authored until it is hit again.
- **`worst-case/` games**: knowingly stale. `make full-game`, `make save-worst-case` and `make save-fuzz` are run once at the end. An assert or a crash is a bug to fix in this step. Any other change is ignored and noted.

## Testing Decisions

- **A good test** asserts only on public behaviour. It drives the pad through `run_frame`, `tap` and `hold`, then asserts on:
  - `pad_nav_jump_mode`, `pad_nav_aim_mode`, `pad_nav_hovered`, `pad_nav_slipway_start`, `pad_nav_last_acted` and `pad_nav_panel_open`;
  - the played frame's mouse position and buttons, and the camera;
  - `ui_pad_selection`, `ui_left_held` and `box_entries` / `is_button_of`;
  - world counts, credits, planet states and `p8_fixed_rnd_state()`.

  Never on `pad_nav_t` internals.
- **Prior art**: `tests/test_pad_jump.c`'s hand-built entities (`test_nearest_of_several`, `test_candidates_keep_known_and_settled_in_order`). In `tests/test_pad_nav.c`: `test_triangle_on_a_hovered_known_planet_opens_its_menu_and_circle_comes_back`, `triangle_click`, `test_triangle_on_a_hovered_object_still_scrolling_in_presses_nothing`, `test_cross_dpad_cross_dpad_cross_builds_a_chain_of_two_slipways`, `test_building_and_backing_out_never_types_the_cheat`, `test_four_chords_on_a_hovered_colony_cheat_and_change_nothing_else`, `test_jumps_repeats_and_edge_scrolls_leave_the_game_stream_unchanged` and `in_forked_copy`.
- **Chunk 1's tests**:
  - the candidate tests of `tests/test_pad_jump.c` (`test_candidates_keep_known_and_settled_in_order`) and `tests/test_pad_slipway.c` rewritten for the zone signatures, with the same layouts and assertions, plus an empty or NULL input giving an empty list;
  - `tests/test_pointer_guard.c`: a local `entity_t *name[N]` counted, a `const entity_t *name[N]` counted, a local pointer that is not an array not counted, and a count above or below the ratchet failing.
- **Chunk 2's tests**, for `tools/e2e_limits.sh`: the right "N frames." passes, a wrong N fails, a missing statement fails, two statements in the first paragraph fail, and a statement split across lines passes. Where the script has no test harness, a small test in `make test` drives it on scratch files, in milliseconds.
- **Chunk 3's test**: the chord consistency test described under Implementation Decisions.
- **New cases in `tests/test_pad_back.c`** (chunk 4):
  - `init` records nothing, and `pad_back_target` is NULL;
  - after `pad_back_record(e)`, `pad_back_target` is `e` while another object is hovered, and NULL while `e` is hovered;
  - `pad_back_record(NULL)` keeps the previous record;
  - a recorded entity that is dead, or no longer KNOWN or SETTLED, gives NULL;
  - a later record replaces an earlier one.
- **New cases in `tests/test_pad_nav.c`.** The ○ order, one test per rung (case 1's message-box parts in chunk 7, the rest in chunk 8):
  1. **Rung 1.** With a last object elsewhere:
     - the planet menu open over jump mode: ○ closes it and the hovered object stays;
     - the year box (`score_panel_new_year`) over jump mode: ○ closes it (`shown_message_box()` NULL, the box dead), mode and hovered object unchanged;
     - the year box over aim mode: ○ closes it and stays in aim mode; the next ○ leaves (1b, the companion of `test_circle_in_aim_mode_closes_a_box_before_leaving`);
     - the year box over slipway mode: ○ closes it, the slipway still held; the next ○ cancels it;
     - the year box over a carried structure: ○ closes it, the structure still carried;
     - a planet menu and the year box both up: the first ○ closes the menu, the second the year box;
     - on the `end turn` tab with the year box up: ○ returns to the map and the box stays;
     - in all of these, no jump on that frame or the next, and no mouse button.

     Pause, tech and score keep their existing tests (`test_circle_closes_the_pause_menu`, `test_circle_on_tech_closes_the_box_and_returns_to_the_map_in_one_press`, `test_circle_on_a_tab_returns_to_the_map`), listed as the rest of rung 1.
  2. **Rung 2, slipway.** × starts a slipway from colony A (A recorded), D-pad to B, ○: the slipway is cancelled, jump mode hovers A and nothing is built. A second ○ does nothing (A is hovered and is the last object).
  3. **Rung 2, placement.** With a last object A: take a structure (aim mode, H-7-5). ○ cancels it and stays in aim mode. ○ leaves aim mode for the nearest target (rung 3). If that is not A, ○ jumps to A (rung 4). Each ○ changes exactly one of these.
  4. **Rung 4.** × on colony A, ○ (cancel), D-pad to a far B. ○ hovers A, with no mouse button on that frame. The camera moves at most 8 px per axis per frame until A is inside the rectangle, and the marker is on A.
  5. **○ at game start does nothing**: a fresh game in jump mode. Also after a resume from the title (`start_title_with_a_save`). ○ leaves the hovered object, the cursor, the camera and the world unchanged, with no mouse button and `pad_nav_last_acted` NULL.
  6. **○ at the root does nothing** when the last object is dead (a known planet replaced by a `class_name` project, or killed in the test) and after a new game following a game that had one.
- × on a known planet (chunk 5):
  7. × on a hovered known planet: press on that frame at its screen point, release on the next. The box's rows are buttons of that planet. Jump mode, the same object hovered, and `pad_nav_last_acted` is the planet.
  8. × held from the opening click across 10 frames presses no row: no colonising, credits unchanged. Released and pressed on the colonise row, it colonises (the planet SETTLED, credits charged), and the box closes. The colony is hovered and is still the last object.
  9. × on a known planet still scrolling in (`planet_farthest_outside`) and under a HUD item (`cover_with_icon`): nothing pressed on that frame or the next 10, nothing recorded.
  10. × and △ on the same frame on a known planet give one click.
  11. A refused slipway (target already linked) leaves the last object on the start. A built one moves it to the target (with `settled_row`).
  12. `test_cross_and_circle_in_jump_mode_change_nothing` is rewritten as `test_circle_with_nothing_acted_on_and_cross_on_nothing_change_nothing`. ○ at game start and × with nothing hovered change nothing (its intent for what stays inert). Its × on a known planet part moves to case 7.
- △ again (chunk 6):
  13. △ on a hovered known planet opens its menu, and △ again closes it: no mouse button on the closing frame, no row pressed, jump mode, the same object hovered, the marker back on the next frame.
  14. The same on a colony's inspection panel (it opens on the release, then △ closes it).
  15. A △ on the click's release frame does not close the menu that frame (with `in_forked_copy`).
  16. △ under the year box does nothing: the box stays and nothing is pressed. With a planet menu and the year box both up, △ closes only the menu. The planet menu opened by × is closed by △ as well.
- The cheat and the game stream:
  17. Four chords in jump mode on a hovered known planet, with a last object elsewhere: the cheat fires once, no box opens, the hovered object is unchanged, and the played buttons are 0 on every frame. The ordinary sequence × (menu) ○ × (menu) △ × (slipway from a colony) ○ ○ ○ fires nothing.
  18. ○ root jumps, × opening a menu then ○ or △ closing it without a row, and △ toggles leave `p8_fixed_rnd_state()` unchanged.
  19. Pause, a tab visit and a slipway keep the last object: after each, ○ at the root still jumps to it.
- **Kept unchanged**: every △ test listed in Facts, `test_shoulders_switch_and_aim_pans_under_the_year_box` and `test_start_opens_pause_over_the_year_box` (the year box survives shoulders and Start), `test_building_and_backing_out_never_types_the_cheat` (its second ○ is at the root on the last object, so it still does nothing), and the slipway, placement and pause tests.
- **Deleted tests**: none expected. Section 2's rule applies: a test goes only with the mechanic it exercises, and step 8 removes none. A test of a surviving behaviour that no longer holds or compiles is rewritten (`test_cross_and_circle_in_jump_mode_change_nothing`, and the candidate tests of chunk 1).
- **Not tested**: misuse asserts (the codebase does not exercise asserts).
- **Before each commit**: `make test-all` green; `make unit-run-test_live_slots` green; `make end-state EVENTS=` green on every touched replay with cheat counts matching (all 17 on chunk 1 and at the end of the step); `make coverage-compare` with no unhit line beyond step 7's 69; `make rng-sync-probe` when `pad_colonise` changed; `make test` and `make e2e` wall times compared before and after the step.

## Out of Scope

- Select, the L1 build bar, R1 panels and the tab ring's removal (steps 9 to 12). ○ on a tab keeps returning to the map as today.
- Deleting network-mode code (step 12).
- Any change to aim mode, slipway mode or placement beyond the ○ order they already follow (step 7 and step 6).
- Re-authoring the `worst-case/` games (step 13).
- The step 14 metrics tool (this step only exposes `pad_nav_last_acted` for it).
- Tuning the repeat, scroll or score constants.
- Any change to the cart's rules, to what the HUD, panels or menus contain, audio, PSX or mouse input.

## Further Notes

- **Why × uses △'s click.** Section 2 says "as a mouse press there does". The cart opens the projects panel on the press (`planet_left_press`), and △ already sends that press with its release. A second press path would be a second scheme for one action. It also makes × and △ frame-identical on a known planet, which makes chunk 9's re-authoring a one-frame edit.
- **Why the arming of ×.** The cart's own mouse never holds the button into a box it just opened. The pad does, because × is also the panel's "choose" button. Pause already solved the same problem the same way.
- **Why △ leaves the year box alone while ○ closes it.** Section 2's toggle is "the planet menu of the hovered object", while ○'s first rung is "close the box or panel", any box. A △ that closed the year box would give △ a second meaning, the defect section 1 measured for □.
- **Why the message-box rung is step 8's.** Step 7's note found that ○ under the year box skips the box and leaves aim mode, which breaks "○ always goes back one step", and left it to step 8, whose rule this is.
- **Why the anchor is the slipway's target when built.** Section 2 keeps the cursor on the target after a build "so × chains the next slipway from there". The target is what × acted on last. A refused build acted on nothing new.
- **For step 9**: the frame counts of the year-heavy replays are in the step note. `pad_colonise` grows by the △ pair and the ○ jump only.
- **For step 12**: nothing of step 8 lives in network-mode code. The last-acted state is in `pad_back`, and the ○ rung and the arming are in jump-mode and panel code that stays.
- **For step 14**: the dead ○ presses the locked rules require at the root are exactly those where `pad_nav_last_acted` is NULL or is the hovered object, with no box or message box shown. △ under a non-planet box is a dead press in the "others" group.
