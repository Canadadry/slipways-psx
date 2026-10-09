---
title: "Aim mode on □"
description: "After step 6 the pad still keeps the old free mode, with its □ select, its slipway drag and its pixel-aimed △; □ must become a plain aim-mode toggle (pan, × probe and fold) that □ or ○ leaves for jump mode on the target nearest the centre."
status: needs-triage
---

## Plan step (verbatim)

Plan file `docs/plan-12.md`, Phase 2 — Jump mode on the map, step 7:

> 7. **Aim mode on □** (today's free-mode panning and its × probe/fold behaviour, without its □ select and its drag). □ or ○ returns to jump mode on the target nearest the centre. `pad_probe` and any fold replay re-authored. Verified as in step 5.

The rules it follows (section 2):

> **Buttons**
> - **○ always goes back one step**, in this order: close the box or panel; cancel the slipway or the placement; leave aim mode; at the root, jump to the last object × acted on (nothing at game start).
> - **□ toggles aim mode:** a free crosshair at the screen centre with the camera panned under it, the way free mode pans today (1/2/4 px per frame). × presses the background as today: probe, or fold once a probe is out; hold × to fold. □ or ○ goes back to jump mode, on the target nearest the centre. This is the only pixel aiming left.

> **Rejected**
> - **A modifier button (L2 + D-pad) for free aiming:** it needs a chord and a new button. □ as a toggle uses buttons that already exist.
> - **Launching probes from a colony and steering them:** probes target a point in space, so a crosshair is needed anyway.
> - **Snapping to exact pixel offsets for structure placement:** placement needs empty space, so it uses the aim crosshair.

> **Guardrails** (the cheat)
> - New inputs must not type the cheat sequence (× × ○ ○ × × ○ ○, see `src/game/cheat.c`) by accident. Checked by the end-state check of step 2a: each replay's cheat activation count must match its reference (some replays use the cheat on purpose).
> - Every step from 6 on that changes what × or ○ do (including the build bar and the ring's removal) checks whether ordinary input sequences, with D-pad presses between them (`cheat_step` ignores them), can complete × × ○ ○ × × ○ ○, and re-checks the deliberate cheat route of step 6. If ordinary play can complete it, or no side-effect-free deliberate route exists, change the cheat sequence in `src/game/cheat.c` (a dev tool, not a game rule) to one ordinary play cannot type and that has such a route, and update the replays that type it, in the same step.

The other guardrails of section 2 apply as written:

- re-authoring to the step 2a end state against reference `8defcba`;
- the situation guardrail: every situation of the old `.txt` is kept, except those whose mechanic the plan removes, each noted;
- the frame limits (180, or 600 if listed in `tools/e2e_long_exceptions.txt`);
- shared prefixes first, and pinned values re-derived;
- coverage never drops;
- the cost record;
- the 8 rules of the step 2b discipline;
- the `worst-case/` games are knowingly stale;
- one scheme only.

## Facts from step 6

Step 6 is done (`ce775f0` to `1c8777f`, step note `1e07b5e`). These facts come from that note, the committed `pad_nav.c` and the e2e `.txt` and `.json` at `1e07b5e`. The labels H-6-k are kept, because the sections below cite them. Each entry says whether the PRD's assumption held and, where it did not, what replaces it. **Where a fact here contradicts a later section, the fact wins.**

- **H-6-1, held.** Slipway mode is as described:
  - `PAD_NAV_MAP_SLIPWAY`, the pure `pad_slipway` module (`ce775f0`), `pad_nav_slipway_start` and `pad_nav_slipway_target`;
  - × on a hovered colony or structure that can start a slipway, inside the rectangle and clear of the HUD;
  - △ and □ inert in slipway mode, and a private `cancel_slipway` beside the old `cancel_drag` and `dragged_slipway`.
  - Added beyond the step 6 PRD: tabs and pause wait one frame after a slipway release, as they do after a drag's.
- **H-6-2, held.** The cheat is four × + ○ chords (`a4deb2a`). A lone × or ○ resets the count, and `pad_nav_step` hides × and ○ from every mode from a chord frame until both are up. Deliberate route: four chords, one every 2 frames, anywhere.
- **H-6-3, held.** Every e2e slipway is built by the × route, in its reference direction, and every end state matches `8defcba`.
- **H-6-4, failed in its detail.** The situations still on the old drag are these (frames at `1e07b5e`):
  - `pad_colonise`: f146–155 (□ from network mode into the drag, then □ dropping it into plain free mode) and f215–260. f215–260 hold the crossed planet (p47 crossed by a drag aimed past it, f257), the drop on empty space (f258) and the fold under the drag (f259–260). Then f261–266: the drag pressed again, □ into plain free mode, ○ back to network mode on p25, ○ to jump mode.
  - `pad_slipway`, whose f0–238 are the old file byte for byte:
    - f100: □ from network mode into the drag;
    - f125–142: partial line and cost, snap on a known planet, refusal, and the crossed planet (f131–142);
    - f194–210: the drop on empty space and the fold under the drag;
    - f229–236: △ under the centre during a drag, and colonising from the resumed drag;
    - f237–238: the drag pressed from the off-screen p47;
    - f239–241: ○ dropping the drag into network mode, then ○ to jump mode;
    - f302–312: □ selecting p28 under the centre, then network travel with × (f305) and Left/Right;
    - f313–361: □ from network mode into the drag, the duplicate seen from the drag, the drop on it, and ○ (f359).
  - **"Too far" is no longer on the drag.** It is built the × way (p28 → p25), so the "Too far" row of the situation table is already satisfied and is not re-done.
  - **The build from an off-screen start was dropped by step 6** (a `guideline` decision): no × route in that galaxy leaves the start off screen. `test_an_off_screen_start_still_builds` stands in. The f237–238 off-screen drag press goes with the drag.
  - **The crossed planet is a game rule, not a drag mechanic, so it may not go.** It survives only on the drag, in `pad_colonise` and `pad_slipway`, because in those galaxies no known object lies beyond p47 or p25. This PRD adds a decision: chunk 3 produces it in slipway mode in the first slipway-building replay (`pad_research`, `pad_tech_costs`, then `pad_planet_projects`) whose known candidates include a target behind another planet from a colony the replay already has at that point. The aim is refused, then ○ or a re-aim. It costs no credit, reveal or slipway, so the end state is unchanged. The crossed planet then goes from `pad_colonise` and `pad_slipway` as a drag-only form, noted. If no replay has such a pair within its frame limit, it is noted as lost from the e2e, with the cart lines it alone hit listed under coverage category 3, and a unit test in `tests/test_pad_nav.c` produces it in slipway mode.
  - **Partial line and snap** (`pad_slipway` f125–142): the partial line to empty space goes with the drag. The cost label, the snap on a target and the known-planet refusal are slipway mode's own (`pad_slipway`'s × route) and stay.
- **H-6-5, held, with one change.** Free mode is unchanged apart from the chord rule. Its pan, × probe and fold, △ on the centre, ○ returns, □ select and drag behave as at `25e4d77`. □ from jump mode still enters plain free mode with the camera unmoved, and every replay presses □ on its first map frame.
  - The change: **re-authored replays now use free mode's □ select as their way to reach an object.** They press □ (plain free mode), pan, □ (network mode on the colony under the centre), then ○ (jump mode on it):
    - `pad_research` f245, f259–262, f304–306;
    - `pad_tech_costs` f410–412, f480–482, f541;
    - `pad_planet_projects` f383, f485–487.
  - Chunk 2 breaks every such pair. Each becomes □ (aim mode leaves for jump mode on the target nearest the centre, which is the colony that was under the centre), followed by D-pad jumps if the nearest is another object. The ○ that left network mode is dropped. Network mode's □ select is a removed mechanic (noted).
- **H-6-6, held.** `pad_probe` is unchanged (77 frames: f13 □, f16 ○, f17–18 idle, f19 Up). Its prefix is still shared by `pad_colonise` (f0–68, and the old file byte for byte up to f174), `pad_slipway` (`pad_colonise` f0–98), `pad_planet_projects` (f0–68), `pad_empty_planet` (f0–28) and `pad_restart_resume` (f0–76). The f17 □ edit of chunk 2 applies to all of them.
- **H-6-7, held.** Menus are still opened by △ on the centre in free mode (then landing network mode) in:
  - `pad_colonise` f88, f135 and f208;
  - `pad_planet_projects` f159, f202 and f215;
  - `pad_tech_costs` f172, and f544 on p54 in network mode after the f541 □ select.

  `pad_research` now opens its menus from jump mode (f322, f339, f396) and from the drag-free network path above. `pad_tech_costs` f117 is △ in network mode.
- **H-6-8, held.** `pad_research` keeps its f28 ○ in plain free mode, its four X taps that place refused probes beside p78 (f30–36, aim mode after this step) and its chord cheat at f38–44.
- **H-6-9, held, with two lines named.**
  - No `.end-state` was re-baselined, and there is no exceptions file.
  - `make coverage-compare` already reports 2 unhit lines, `src/game/pad_nav.c:538–539`: `drag_due_step`'s `land_on` after an old-drag build, which the plan replaces. Chunk 3 deletes them, so they leave the report as deleted lines.
  - Two more groups of lines depend on network mode, which this step makes unreachable. Each is justified as a removed mechanic, not re-authored:
    - **`drop_stale_map_mode`'s reset**, reached only by `pad_planet_projects` f172–173: the harvest kills p25 while network mode is on it after f159's free-mode △. After chunk 4, f159 becomes jump-mode △ on the hovered p25, and the harvest kills the hovered object. Jump mode's reseed (`seed_hovered`) takes over, so that game situation is kept. `drop_stale_map_mode`'s lines become unhit: category 2 (network-mode code, step 12 deletes it).
    - **`pad_nav_next_slipway`'s insertion lines**, reached only by `pad_tech_costs`' network ending (f541 □ select on p54, f552–554 Left twice to turn the selected slipway, f556 × travel to s2). That ending is network travel, a mechanic section 2 removes. Chunk 2 or 4 re-authors it as jump-mode moves and △ (p54's panel, then s2's). The insertion lines become unhit: category 2. `pad_nav_next_slipway` stays a public pure function until step 12, unit-tested by `test_slipways_turn_clockwise_from_straight_up` and `test_straight_up_splits_the_two_halves`.
  - **The `test_state` pin on `pad_tech_costs`' end camera, `127 234`,** comes from that network travel's `centre_camera_on(s2)`. Under the pinned-values guardrail it is re-derived: the re-authored ending ends with the camera wherever its jumps leave it, and the pin takes that value. Its intent (the end state's camera line is the one the replay ends on) is kept. The other `test_state` pins on `pad_tech_costs` (counts, lines, slipways, save summary, cheat 1) are unchanged.
- **H-6-10, held. Frame counts at `1e07b5e`:**
  - limit 600: `pad_planet_projects` 583, `pad_tech_costs` 565, `pad_research` 458, `pad_slipway` 362, `pad_colonise` 315, `pad_message_box` 190;
  - limit 180: `pad_bankrupt` 170, `pad_restart_resume` 155, `pad_empty_planet` 148, `pad_game_end` 104, `pad_end_turn` 91, `pad_probe` 77, `pad_pause_restart` 76, `pad_build` 63, `pad_score` 61, `pad_resume_at_start` 48, `pad_title` 30.

  `pad_planet_projects` has 17 frames of margin, so its re-authoring in chunk 4 (three menus moved to jump-mode △, and the □ select pairs) compresses idle frames first.
- **H-6-11, held.** ○ in jump mode is inert, and △ on the hovered object is as step 5 landed it.
- **Worst-case games.** Four are stale: `full_game`, `full_game_new_game` and `full_game_pause_idle` (their scripts lack the □), and `full_game_pause_restart`, which joined them in step 6 (the scripts type the old cheat, so the count is 0). `title_idle` passes. This step runs `make full-game`, `make save-worst-case` and `make save-fuzz` once, for asserts and crashes only.
- **Step 6's cost**: 8 replays broken, about 49 minutes of re-authoring. Step 7's note records its own cost the same way.

## Facts from the code at `HEAD`

- **Free mode today** (`free_mode_step` in `pad_nav.c`). On every frame:
  - the cursor is the screen centre, and the D-pad pans the camera (`free_mode_pan`: 1, 2 or 4 px per frame);
  - a drag phase left due from the previous frame runs first (`drag_due_step`: the probe after a drop on empty space, △'s click, or the landing after a build);
  - □ (when × holds no fold) selects the settled planet under the centre (`square_selects_under_centre`, which lands network mode). Otherwise, with a starting planet, □ forgets it (`leave_starting_planet`: plain free mode, the last planet remembered);
  - a slipway the ui holds runs `drag_step`;
  - × engages the background press, probe or fold, when nothing but the background is under the centre (`cross_engaged`, the left button held while × is held);
  - △ clicks the clickable planet under the centre (`triangle_on_centre`, which lands network mode while the menu is up and resumes free mode afterwards only from a starting planet);
  - ○ returns to the starting planet, or else to the last planet (`land_on`, network mode);
  - with a settled starting planet, the drag is pressed out again (`begin_drag`).
- **Free mode is entered** by □ from jump mode (`enter_plain_free_mode`, camera unmoved), □ from network mode (`enter_free_mode`, camera centred, starting planet set, drag pressed), a structure taken from the build tab (`begin_placement`), and the resume after △ (`resume_free_mode`).
- **Network mode is entered** by `land_on` (□ select, ○ returns, the drag's build landing) and by `click_planet` (△ on the centre, and △ in network mode). Since step 5 it is not entered from jump mode. Its ○ goes to jump mode (`6e841dc`).
- **Jump mode helpers** used here: `pad_jump_nearest(x, y, candidates)` (nearest world point, earlier entry at equal distance), `centre_world_point` (the screen centre in world pixels, camera before shake), `live_candidates`, `seed_hovered` (reseeds only when nothing valid is hovered), and `edge_scroll`.
- **The public surface** that names free mode or the drag: `PAD_NAV_MAP_FREE`, `pad_nav_free_mode`, `pad_nav_starting_planet`, `pad_nav_drag_t` and `pad_nav_drag_phase_t`. Outside `pad_nav.c`, only `tests/test_pad_nav.c` uses them (76 lines). `ui_set_pad_crosshair` reads `pad_nav_free_mode` inside `pad_nav.c`.
- **The compiler flags** are `-Wall -Wextra -Werror`, so a static function that loses its last caller has to be deleted in the same commit.
- **The slipway range**: `slipway_obstruction` returns "-too far-" when the squared map length is over 0.5 (1.08 once "space folding" is invented). With `GALAXY_MAP_SCALE` 128, that is about 90.5 px without the tech.
  - In `pad_slipway`'s end state, p28 (297.98, 336.29) and p25 (280.01, 229.19) are about 108.6 px apart. From p28, Up aims at p47 (score 126 against 143), and Up again aims at p25: "-too far-" can be reached in slipway mode in that galaxy.

## Problem Statement

From the map's root (jump mode), □ still drops the player into the old free mode. That mode is several modes in one:

- the D-pad pans and × probes or folds;
- △ acts on whatever pixel is under the crosshair and lands network mode;
- □ selects a colony under the crosshair, or forgets a starting planet;
- ○ jumps back to a planet chosen by history;
- a slipway drag can be pressed out of a starting planet and dropped anywhere.

Section 1 measured the cost: 61% of game time in the free-camera modes, △ and □ acting only on the exact centre pixel, and □ meaning four things across 6 sub-modes. Since step 6, slipways are built from jump mode. The drag and the □ select survive only as an interim path, and there is still no clean way back from free mode to jump mode: ○ goes to network mode or does nothing, and □ selects or does nothing.

## Solution

□ in jump mode enters **aim mode**: a crosshair at the screen centre, the camera unmoved on entry. In aim mode:

- **The D-pad** pans the camera under the crosshair exactly as free mode does today (1 px per frame for the first 10 frames held, 2 px up to 40, then 4).
- **×** presses the background exactly as today:
  - a tap on empty space places the probe;
  - with a probe out, a hold folds space, and a short hold is abandoned;
  - over a planet, a structure or a HUD item, × presses nothing.
- **□ or ○** returns to jump mode, hovering the target nearest the crosshair (the nearest known planet, colony or structure by world point). With no target in the game, nothing is hovered. While × holds a probe or fold press, □ and ○ wait: the press goes on until × is released.
- **○ with a box open** closes the box and stays in aim mode (section 2's first rung). During a placement, ○ cancels the structure and stays in aim mode (second rung). The next ○ leaves aim mode (third rung).
- **△ does nothing.** The planet menu is jump mode's △ on the hovered object.
- **No slipway can be held.** A settled planet under the crosshair is never pressed, and nothing selects it. Slipways are built in jump and slipway mode (step 6).

Placement keeps using the aim crosshair as today: taking a structure from the build tab enters aim mode, and after the drop or the cancel the map stays in aim mode.

Network mode can no longer be entered: none of the ways in that are left go through jump or aim mode. Its code stays for step 12, which deletes it.

## User Stories

1. As a player, I want □ on the map's root to give me a crosshair, so that I can aim a probe or a fold at a point in space.
2. As a player, I want the camera unmoved when I enter aim mode, so that the crosshair starts where I was looking.
3. As a player, I want the D-pad to pan with today's speed curve, so that small corrections stay precise and long pans stay quick.
4. As a player, I want × on empty space to place my probe, so that exploring still starts the same way.
5. As a player, I want holding × with a probe out to fold space, so that folds work as before.
6. As a player, I want a short × hold abandoned and charged nothing, so that a slip of the finger costs no credits.
7. As a player, I want × over a planet in aim mode to press nothing, so that aiming never opens a menu or starts a slipway by accident.
8. As a player, I want □ in aim mode to take me back to jump mode, so that the same button opens and closes the crosshair.
9. As a player, I want ○ in aim mode to take me back to jump mode too, so that ○ is always "back".
10. As a player, I want the cursor to land on the object nearest the crosshair when I leave aim mode, so that I continue from what I was looking at.
11. As a player, I want nothing hovered if there is no target yet, so that leaving aim mode at the very start of a game is harmless.
12. As a player, I want ○ to close an open box first and stay in aim mode, so that one ○ never does two things.
13. As a player, I want ○ during a placement to cancel the structure first, so that the back order is predictable.
14. As a player, I want □ and ○ to wait while I hold a fold, so that a fold in progress is never cut short by a stray button.
15. As a player, I want △ to do nothing in aim mode, so that the planet menu belongs to the hovered object in jump mode, not to a pixel.
16. As a player, I want no slipway ever to stick to the crosshair, so that the old drag's half-built lines are gone.
17. As a player, I want the crosshair drawn only in aim mode and placement, so that I always know whether I am aiming.
18. As a player, I want pause, a tab visit and the year box to keep me in aim mode, so that I come back to the crosshair I left.
19. As a player, I want the four-chord cheat to stay inert in aim mode, so that it never places a probe or leaves the mode.
20. As a player, I want ordinary aiming, probing and backing out never to fire the cheat, so that my game stays honest.
21. As a player, I want the camera after leaving aim mode to follow jump mode's edge scroll, so that the hovered object comes into the comfortable area.
22. As a replay author, I want aim mode's pan, × and fold to be today's free mode exactly, so that every probe and fold of the replays lands on its reference world point.
23. As a replay author, I want a ○ tap that used to do nothing in free mode to be produced as ○ then □, keeping later frame indices, so that shared prefixes stay cheap to re-author.
24. As a replay author, I want menus opened by △ on the centre re-authored as jump-mode △ on the hovered planet, so that every menu situation stays covered.
25. As a replay author, I want "too far" produced in slipway mode on a real target beyond reach, so that the rule stays covered without the drag.
26. As a replay author, I want the situations whose mechanic this step removes (the drag, the □ select, network travel) listed in the step note, so that their loss is deliberate and visible.
27. As a replay author, I want shared prefixes re-authored first and reused verbatim, so that the work is done once.
28. As a reviewer, I want every re-authored replay to pass `make end-state` against `8defcba` with its cheat count kept, so that no game end state changes.
29. As a reviewer, I want each replay's old and new `.txt` compared in the same commit, so that no situation disappears silently.
30. As a reviewer, I want `make coverage-compare` newly unhit lines limited to deleted code, unreachable network-mode code and lines that only a removed mechanic reached, each listed, so that coverage only drops where the plan says it may.
31. As a reviewer, I want the pinned values of step 2a's inventory re-derived with their intent kept, so that no assertion is weakened.
32. As a reviewer, I want the re-authoring cost and the replays broken recorded, so that later steps can be judged.
33. As the agent of step 8, I want ○ in jump mode still inert and the aim rung of the ○ order already in place, so that my step only adds the root jump and the menu rungs.
34. As the agent of step 10, I want placement still entered into aim mode, so that the L1 build bar can reuse the crosshair.
35. As the agent of step 12, I want the network-mode code that is now unreachable, and the tests deleted with it, listed, so that I know what is left to delete.
36. As the agent of step 14, I want aim mode readable from outside the pad navigation, so that the metrics tool can measure its share of time.
37. As the agent of step 9, I want the new frame counts of the year-heavy replays recorded, so that the Select-hold budget can be re-read.
38. As a developer, I want "free mode" renamed "aim mode" in the code, so that the code speaks the plan's language.
39. As a developer, I want `make test` and `make e2e` no slower, so that the fast loop stays fast.

## Implementation Decisions

### What aim mode is

- **Aim mode is today's plain free mode, cut down.** Kept, unchanged: the pan (`free_mode_pan`, its speed curve and per-direction hold counts), × with `cross_engaged` (probe on a tap, fold on a hold, nothing over anything but the background, the press held while × is held, never started on a release frame), the centre cursor and the crosshair.
- **Removed from it**: the □ select, the starting planet and the drag, △ on the centre, and ○'s return to the starting or last planet.
- **Added to it**: □ and ○ back to jump mode.
- **No new pure module.** The only new arithmetic is "the target nearest the centre", which is `pad_jump_nearest` over `live_candidates` at `centre_world_point`. All three exist. Step 5's pattern ("pure helpers where it fits") is met by the existing `pad_jump`. The aim-mode step stays in `pad_nav.c`. Its pan and × lines are the old free-mode lines, so step 12 deletes nothing of it.

### Naming

- `PAD_NAV_MAP_FREE` becomes `PAD_NAV_MAP_AIM`, and `pad_nav_free_mode` becomes `pad_nav_aim_mode`, with the same contract: a game is running on the map tab, no panel is open, and the map mode is aim. The private names follow (`free_mode_step` → `aim_mode_step`, `free_mode_pan` → `aim_pan`, `enter_plain_free_mode` → `enter_aim_mode`, `reset_free_mode` → `reset_aim_mode`). Test names and helpers in `tests/test_pad_nav.c` follow. This is chunk 1 alone, a pure rename: every e2e stays byte-identical.
- The header comment of `pad_nav_aim_mode` says what it is (the cursor is the screen centre and the D-pad pans the camera), as `pad_nav_free_mode`'s does today.

### □ and ○ in aim mode

- **When.** □ or ○ is pressed, × engages no press (`cross_engaged` is 0), and, for ○, no box is open (`circle_returns`). The release frame of a click does not block it: leaving presses nothing.
- **What.**
  1. The map mode becomes jump.
  2. The hovered object becomes `pad_jump_nearest(centre world point, live candidates)`, or nothing if there is none.
  3. The jump state's `seeded` flag is set when something is hovered, so jump mode does not later reseed on the first colony.
  4. The hold-repeat state is cleared.
  5. On that frame the cursor stays at the centre, and no jump, edge scroll or marker runs. From the next frame, jump mode runs as step 5 landed it (edge scroll included).
- **The camera does not move on the frame of the press.** The edge scroll that follows depends only on the hovered object, as everywhere in jump mode (step 2b rule 8: a replay re-pans before its next point action in aim mode).
- **○ with a box open** closes the box (`circle_closes_the_box`) and stays in aim mode: the existing `circle_returns` rule.
- **○ during placement** goes to `placement_step`, as today: the structure is cancelled and the map stays in aim mode. □ during placement stays ignored (`test_triangle_and_square_wait_while_carrying`).
- **While × holds a probe or fold press**, □ and ○ do nothing. This keeps today's guard (`(pressed & P8_PAD_SQUARE) && !nav->cross_engaged`) and gives ○ the same one. The fold is not cut short and the button stays held.

### △ in aim mode

- △ does nothing in aim mode: no click, no mode change, the camera unmoved. `triangle_on_centre`, `resume_free_mode` and `resume_start` are deleted.
- Why:
  - Section 2's △ is "the planet menu of the hovered object", and aim mode has no hovered object.
  - Section 1 measured "△ and □ act only on the object exactly under the centre crosshair" as a defect.
  - The step text lists what aim mode keeps (panning and × probe/fold), and △ is not in it.
  - `click_planet` lands network mode, which the plan removes. Keeping a pixel-aimed △ that does not land network mode would be a new rule the plan does not state.
  - Step 6's precedent, "△ and □ do nothing in slipway mode", gives one mode one set of buttons.
- Cost: every replay that opens a menu with △ on the centre is re-authored (H-6-7). The player and the replays use jump mode's △, which has worked since step 5.

### The □ select, the drag and the starting planet

- **Deleted**:
  - `square_selects_under_centre`;
  - `leave_starting_planet`, `return_to_starting_planet` and `return_to_last_planet`;
  - `begin_drag`, `drag_step`, `drag_due_step`, `dragged_slipway`, `cancel_drag`, `clear_drag` and `drag_due`;
  - `pad_nav_drag_t`, `pad_nav_drag_phase_t` and the `drag` field;
  - `pad_nav_starting_planet` and the `starting_planet`, `starting_slipway`, `last_planet` and `last_slipway` fields;
  - `takes_slipway`, if nothing else uses it.
  These are the free mode's □ select and its drag that the step text removes, plus the fields only they read. Their callers lose those calls: `unwind_sub_modes` keeps only the placement and step 6's slipway-mode cancels, and `pause_step`, `pad_nav_step`, `tabs_can_switch` and `pause_button_free` drop their drag terms.
- **Network mode's □** (`network_step`), which entered free mode on the planet and pressed the drag, becomes: `enter_aim_mode` with the camera centred on the planet (`centre_camera_on`), no starting planet and no press. It exists only for chunk 3's commit, while network mode can still be entered. After chunk 4, network mode cannot be entered, and this line waits for step 12 with the rest of `network_step`.
- **`begin_placement` from network mode** (`enter_free_mode` with the selection) becomes `enter_aim_mode` after `centre_camera_on`. It is unreachable after chunk 4 as well. `enter_free_mode` is deleted once it has no caller.
- **Kept, untouched, for step 12** (unreachable after chunk 4): `network_step`, `travel_to`, `click_planet`, `land_on` while it still has a caller, `pad_nav_selection`, `pad_nav_selected_slipway`, `pad_nav_next_slipway`, the `selection`, `selected_slipway` and `selection_frames` fields, `drop_stale_map_mode` and the network branch of `map_step`. The rule: a function or field that `-Werror` forces out because it lost its last caller is deleted in this step. Nothing else of network mode is edited. The plan says step 12 "deletes the code", and folding it in here would do step 12's work.
- **`map_step`'s assert**: jump mode is still the last branch. It now names aim mode and slipway mode among the branches before it.

### Where the interim situations go (the situation guardrail)

Each situation step 6 left on the interim path (H-6-4) is either produced the new scheme's way or dropped with its mechanic and noted. The rule: if the new scheme has a way to produce the same game event, it is produced that way. If only the old drag, the □ select or network travel could produce it, it goes.

| Situation (old `.txt`) | Decision |
|---|---|
| A slipway dropped on empty space, refused | **Dropped** with the drag. Slipway mode aims only at targets, so nothing can drop a slipway in space. |
| A fold held while a drag is out (`pad_slipway` f194–210, `pad_colonise` f258–260) | **Dropped** with the drag. Slipway mode ignores □, and its × builds. The fold itself, and the fold abandoned on a slipway's middle (`pad_colonise` f332–337), stay in aim mode. |
| △ under the centre during a drag; colonising from a drag that resumes aimed at the colony (`pad_slipway` f229–236) | **The colonising is kept**, re-authored as jump-mode △ on the hovered planet. The drag's drop and its resume are **dropped** with the drag. |
| "Too far" to empty space (`pad_slipway` f261), if step 6 left it there | **Produced in slipway mode**: aimed at a candidate beyond reach of the start. In `pad_slipway`'s galaxy: start p28, Up, Up, aimed at p25, about 108.6 px against the 90.5 px limit, then ○. The range is a game rule, not a drag mechanic, so it may not go. If step 6's galaxy for that replay has no pair beyond reach, another slipway-building replay with one produces it, and the note says which. |
| Network travel along a slipway with ×; Left/Right on a planet with one slipway; the duplicate seen from network mode; the selected slipway drawn bright | **Dropped** with network mode (section 2 names network travel among the removed mechanics). |
| □ from network mode into the drag; □ selecting the colony under the centre; ○ back to the starting or last planet | **Dropped** with the □ select and the drag. The new-scheme counterpart (□ in aim mode back to jump mode, hovering that colony when it is the nearest) is produced instead where the replay needs to continue from it. |
| The blinking circle round a selected planet | **Kept**: it is jump mode's hovered marker (`show_selection`), produced in jump mode. |
| A ○ tap in free mode that changes nothing (`pad_probe` f16, `pad_research` f28) | **Produced the new way**: ○ leaves aim mode (nothing hovered at that point of the game), and □ on the next idle frame comes back. The camera, the entities and the ephemerals are identical before and after. Every later frame keeps its index (see "Re-authoring"). |

### The cheat (section 2's re-check)

- **What changes for × and ○**: ○ in aim mode now leaves the mode. × in aim mode no longer drops or holds a slipway. Nothing else changes for × or ○ in any mode.
- **Ordinary play (under H-6-2)**: the matcher counts only frames where × and ○ go down together. No aim-mode action needs both on the same frame. □/○ leaving, × probing and × folding are single-button presses, and every lone × or ○ resets the matcher. Ordinary play still cannot complete the sequence.
- **The deliberate route**: the chord rule runs before any mode in `pad_nav_step`, so in aim mode four chords reach neither the ○ that leaves nor the × that probes. A new unit test checks it there (test 10 below). The route of step 6 holds unchanged.
- **If H-6-2 is false** (step 6 kept × × ○ ○ × × ○ ○): ordinary aim-mode play types × (probe), ○ (leave), □, × (probe), ○ (leave), and so on. With the fold's × that is × ○ … × ×, and two probes and two ○ make × × ○ ○ easy to reach. Section 2's fallback then applies in chunk 2: the cheat becomes step 6's chord design (four × + ○ chords, inert in `pad_nav`), and the replays that type it are retyped in that same chunk.
- Each replay's cheat count is checked by `make end-state`, as always.

### Unchanged

- Jump mode (step 5) and slipway mode (step 6). The one exception: on □/○ from aim mode, the hovered object is set to the one nearest the centre.
- ○ in jump mode stays inert (step 8). × on a known planet stays inert (step 8).
- Placement, the build tab, the tab ring, pause, panels and boxes. The year box over aim mode behaves as it does over free mode today.
- `ui_set_pad_crosshair` stays `pad_nav_aim_mode(nav) && pad_nav_placing(nav) == NULL`.

## Implementation Chunks

Each chunk ends on a commit where `make test-all` passes. A chunk that changes what a replay sees includes, in the same commit, the re-authoring of every replay it breaks, its `.txt`, `.json.sha256` and pinned values. Within a chunk, shared prefixes come first: `pad_probe` → `pad_colonise` → `pad_slipway`, then `pad_planet_projects`, `pad_empty_planet` and `pad_restart_resume` (each reusing the new prefix verbatim), then `pad_research` → `pad_resume_at_start`, and the others.

1. **Rename free mode to aim mode.** The public and private names and the test names. No behaviour change: every e2e and every `.end-state` stays as it is.
2. **□ and ○ in aim mode go back to jump mode on the target nearest the centre.** This replaces the □ select and ○'s returns *in plain aim mode* (no starting planet). Free mode with a starting planet (the interim drag) keeps its □ and ○ for one more chunk. Unit tests 1–6 and 9–11 below.
   - Re-authoring:
     - `pad_probe`: f16 ○ is kept and □ goes on f17, so every later frame keeps its index.
     - Then every replay sharing that prefix gets the same one-frame edit.
     - `pad_research` f28 (H-6-8): the same edit on its next idle frame.
     - Every replay that pressed □ or ○ in plain free mode for another reason.
   - `make rng-sync-probe` is run if `pad_colonise` changed. Its windows keep their frames when the indices are kept.
3. **The drag removed.**
   - No starting planet and no drag code.
   - Network mode's □ and placement from network mode enter aim mode centred on the planet.
   - Unit tests 7 and 8 below, and the old drag tests deleted (see Testing Decisions).
   - Re-authoring: every replay that still used the interim drag path (H-6-4: `pad_colonise`, then `pad_slipway`), following the situation table. "Too far" goes into slipway mode here.
4. **△ in aim mode does nothing.** `triangle_on_centre` and the resume are deleted, so network mode can no longer be entered. Unit test 12 below, and the network-mode tests that can no longer reach network mode deleted.
   - Re-authoring: every replay that opened a menu with △ on the centre (H-6-7). The expected minimum is `pad_colonise`, `pad_slipway`, `pad_planet_projects`, `pad_research` (then `pad_resume_at_start` is re-cut if its last record changed) and `pad_tech_costs`.
   - The route: □ (or ○) out of aim mode, D-pad jumps onto the planet, △; then the menu as today; then □ back into aim mode and a re-pan to the reference camera before the next probe, fold or placement (rule 8).
   - Network-only situations are dropped and noted.
5. **The step note** in `docs/plan-12.md`:
   - the replays broken, re-baselined and re-authored per chunk, and the agent time spent re-authoring;
   - the situation table as applied (kept new-way, dropped with mechanic, per replay);
   - the cheat verdict;
   - the coverage lines newly unhit, per category;
   - the network-mode code left unreachable for step 12, and the tests deleted with it;
   - the new frame counts of `pad_colonise`, `pad_planet_projects`, `pad_tech_costs`, `pad_research`, `pad_bankrupt` and `pad_message_box` for step 9's budget;
   - each hypothesis that turned out false and what was done instead.

If chunk 4 breaks a replay that chunk 2 or 3 re-authored, it is re-authored again in chunk 4's commit, and the note counts the time.

### How to find what breaks, per chunk

1. Run `make e2e` after the code change.
2. For each red replay, `make e2e-ref` gives the first diverging frame, and the `.txt` names the press there.
3. Classify the press:
   - ○ in aim mode that did nothing: ○ then □ on the next idle frame;
   - ○ or □ that went to network mode: replaced by □/○ to jump mode and D-pad jumps to the same object;
   - △ on the centre: the route of chunk 4;
   - a drag press: the situation table.
4. Re-author it, then run `make end-state EVENTS=<replay>` and compare the old and new `.txt`.

## Re-authoring

- **Discipline**: the 8 rules of step 2b. Every probe and fold keeps its reference world point and reveal order, and the title X keeps its frame. A replay that cannot reach its reference falls under section 2's relaxed-comparison fallback. The first such replay adds the exceptions file and its support in `tools/end_state.sh`. Expected: none, since this step moves no probe or fold point.
- **Keeping frame indices** where one press replaces another: the ○+□ pair takes an idle frame that already exists, so `pad_probe`'s f17, an idle frame after the f16 tap, takes the □. The capture changes only on the frames where the crosshair is absent (jump mode draws none). The end state and every later frame are identical, so `pad_probe`'s prefix chain needs one frame edited per file, then a re-baselined `.json.sha256` and an updated `.txt` line.
- **Camera discipline** (rule 8): after any jump-mode segment, the camera is where the edge scroll left it. Before the next point action in aim mode (probe, fold, placement), the replay re-pans to the reference camera. Every placed structure keeps its world point exactly.
- **Frame limits**: within 180, or 600 if listed. A jump-mode △ route is usually no longer than a pan and △. Where it is longer, idle frames are compressed first (`pad_research`, `pad_tech_costs` and `pad_planet_projects` are near 600 at `HEAD`; step 6's counts apply, H-6-10). The reasons in `tools/e2e_long_exceptions.txt` are updated with the new counts and contents. `pad_slipway`'s reason, which describes "every drag state", is rewritten for slipway mode.
- **Pinned values** (step 2a's inventory), re-derived in the commit that changes the replay they read, each keeping its intent:
  - `tests/test_live_slots.c`'s frames on `pad_tech_costs`, `pad_planet_projects` and `pad_empty_planet`, and every fixture cut from a re-authored replay;
  - `tests/test_state.c`'s `pad_tech_costs` lines and camera (`127 234` at `HEAD`, whatever step 6 left), and its `pad_research` cut;
  - `tests/test_rng_sync.c`'s windows on `pad_colonise`, then `make rng-sync-probe`;
  - the golden frame hashes, the `.txt` and the `.json.sha256`. The `.end-state` references are never re-baselined.
- **Coverage.** `make coverage-compare` after each re-authoring commit. Newly unhit lines are allowed only in these categories, each listed in the step note:
  1. lines this step deletes;
  2. network-mode lines in `pad_nav.c` left unreachable for step 12;
  3. lines outside `pad_nav.c` that only a removed mechanic reached (the drag's drop on empty space, the drag held during a fold, network mode's selected-slipway drawing), each named with the removed situation that hit it and a reason why no aim, jump or slipway input reaches it.

  Any other newly unhit line means a replay is re-authored until it is hit again.
- **`worst-case/` games**: knowingly stale. `make full-game`, `make save-worst-case` and `make save-fuzz` are run once at the end. An assert or a crash is a bug to fix in this step. Any other change is ignored and noted.

## Testing Decisions

- **A good test** asserts on public behaviour only: drive the pad through `run_frame`, `tap` and `hold`, then assert on `pad_nav_aim_mode`, `pad_nav_jump_mode`, `pad_nav_hovered`, `pad_nav_slipway_start` (H-6-1), the played frame's mouse position and buttons, the camera, `ui_pad_selection`, `ui_left_held`, world counts, credits, the crosshair pixels and `p8_fixed_rnd_state()`. Never on `pad_nav_t` internals.
- **Prior art** in `tests/test_pad_nav.c`: `leave_jump_mode`, `start_game_in_jump_mode`, `pan_to_empty_space`, `planet_near_centre`, `known_planet_under_centre`, `pan_planet_under_centre`, `make_known`, `settle_planet`, `assert_hovering`, `assert_cursor_at_centre`, `world_count` and `assert_world_unchanged`, `crosshair_pixels_lit`, `take_structure` and the fold tests.
- **New cases in `tests/test_pad_nav.c`:**
  1. □ in aim mode returns to jump mode hovering the candidate nearest the centre's world point (two known planets, the nearer one hovered). The camera is unmoved on the press frame, and the cursor is at the centre with no button on it.
  2. ○ in aim mode does the same.
  3. □ in aim mode over a settled colony hovers that colony in jump mode. Nothing is selected, `ui_left_held` stays NULL, and no mouse button goes down on that frame or the next 10.
  4. □ or ○ in aim mode with no candidate in the game gives jump mode with nothing hovered. The cursor is at the centre, nothing is pressed, and the first object that appears later is hovered.
  5. ○ in aim mode with the year box open closes the box and stays in aim mode. A second ○ leaves.
  6. □ and ○ while × holds a fold do nothing: the fold completes on × release and is charged, and aim mode is kept. The next □ leaves.
  7. × in aim mode on a settled planet presses nothing, holds nothing and builds nothing, also with × held 20 frames. This replaces the old drag tests' entry.
  8. A structure taken from jump mode enters aim mode. ○ cancels it and stays in aim mode, and the next ○ leaves for jump mode (the back order's second and third rungs).
  9. Aim mode's pans, its □/○ toggles and the jump frames between them leave the game stream's state unchanged, as long as no probe or fold is pressed.
  10. **The cheat**: four chords in aim mode fire it once and leave the mode, the camera, world counts (no probe) and the game stream unchanged, with played buttons 0 on every frame (H-6-2). The ordinary sequence ×(probe) ○ □ ×(probe) ○ □ ×(hold fold) ○ fires nothing.
  11. The crosshair is drawn in aim mode and not after □ or ○ back to jump mode.
  12. △ in aim mode over a known planet, over a colony and over empty space does nothing: no click on that frame or the next, mode aim, camera and world counts unchanged.
- **Updated tests**:
  - The free-mode pan, probe, fold, bound, crosshair, tab-visit and pause tests are renamed for aim mode and keep their assertions. `test_free_mode_other_buttons_do_nothing` keeps △ and the shoulders' parts, and its □ and ○ parts move to cases 1–2.
  - `test_square_in_jump_mode_enters_plain_free_mode_where_the_camera_is` keeps its intent under the new name.
  - `leave_jump_mode` asserts aim mode only, since there is no starting planet.
- **Deleted tests** (they froze behaviour this step removes, as `CLAUDE.md` allows):
  - every □-select test, every drag test (`test_square_on_a_colony_holds_its_slipway_at_the_centre` through `test_a_drag_across_a_planet_draws_the_refused_feedback`, and `test_a_full_slipgate_as_starting_planet_presses_nothing`, whose bug class step 6's `pad_slipway_can_start` test covers), the ○-return tests and the free-mode △ tests;
  - every test that reaches network mode through free-mode △, □ or ○, since network mode can no longer be entered: `test_colonising_through_the_menu_lands_network_mode_on_the_planet`, `test_left_and_right_turn_the_selected_slipway`, `test_network_mode_hides_the_cursor_and_blinks_a_circle` and the others.

  A deleted test whose intent survives in the new scheme is rewritten through it instead: colonising through the menu reached by jump-mode △ ends in jump mode hovering the colony; pause from jump mode keeps the hovered object. `test_slipways_turn_clockwise_from_straight_up` and `test_straight_up_splits_the_two_halves` test the public pure `pad_nav_next_slipway` and stay until step 12. The step note lists each deleted test.
- **Not tested**: the misuse asserts (the codebase does not exercise asserts), and that network mode cannot be entered (no test can prove that no input reaches it; coverage shows it).
- **Before each commit**: `make test-all` green; `make end-state` on the 17 replays green with cheat counts matching; `make coverage-compare` with no newly unhit line outside the three categories above; `make test` and `make e2e` wall times compared before and after the step.

## Out of Scope

- ○ at the root jumping to the last object × acted on, × on a known planet opening its projects menu, △ again closing the planet menu (step 8).
- Deleting network mode's code and fields, and the tab ring (step 12). This step leaves network mode unreachable, not deleted, and only removes what `-Werror` forces out.
- The L1 build bar and placement entered from it (step 10). This step keeps placement as it is, in aim mode.
- Select, R1 panels (steps 9 and 11).
- Re-authoring the `worst-case/` games (step 13).
- Tuning the pan curve, the repeat timing, the score weight or the edge-scroll constants.
- Any change to the cart's rules, HUD, panels or menus; audio; PSX; mouse input.

## Further Notes

- **Why the nearest target and not the last hovered one.** Section 2 says "on the target nearest the centre". The player panned on purpose, and the nearest object is what they were looking at. The previous hovered object may be far off screen by then.
- **Why □ and ○ wait during a fold.** A fold is a held × whose release confirms or abandons it. Leaving the mode under it would hand a held left button to jump mode, which never holds one, so the background would see a release at a point the player did not choose. Today's □ guard already does this.
- **Why chunk 3 lets network □ enter aim mode.** For one commit, network mode can still be entered (by △ on the centre). Its □ must do something that is not the drag, and aim mode centred on the planet is the closest to today with no drag. Chunk 4 makes it unreachable.
- **For step 8**: the ○ order's aim rung is in place (box, then placement or slipway, then aim). Step 8 adds the root rung in jump mode and the menu's own rungs.
- **For step 12**: what is left of network mode is `network_step` and its private helpers, the selection fields, `pad_nav_selection`, `pad_nav_selected_slipway` and `pad_nav_next_slipway`, `drop_stale_map_mode` and the network branch of `map_step`. All unreachable since chunk 4.
- **For step 14**: `pad_nav_aim_mode` gives aim time. In aim mode, the dead presses are △, and □ or ○ while × holds a fold. △ is a designed no-op, so it falls in the "required by the locked rules" group, if step 14 reads △'s inertness as required by "△: the planet menu of the hovered object".
