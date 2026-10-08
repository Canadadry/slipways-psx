---
title: "Target picker"
description: "Jump mode (plan 12, phase 2) needs one rule that turns a D-pad direction into the next object on the map. That rule does not exist, so there is nothing to build jump mode on or to test it against."
status: needs-triage
---

## Plan step (verbatim)

Plan file `docs/plan-12.md`, Phase 1 — Groundwork, step 4:

> 4. **Target picker.** A pure function: given an origin, a direction and the live candidate set, it returns the best target or NULL, following the cone and score rule of section 2. Unit-tested on hand-built layouts: one candidate per direction, a tie broken by score, nothing in the cone, origin excluded. No behaviour change; e2e unchanged.

The rule it follows (section 2, Locked decisions, **Cursor and camera**):

> - On the map, the cursor always sits on an object: a known planet, a colony, or a structure (planet-family entities in states KNOWN or SETTLED). Unexplored planets are not drawn, so they are never targets. Wormholes and probes are not targets.
> - D-pad press: jump to the best target in that direction, searched over the whole map, not just the screen. The candidates lie in the 90° cone around the direction (|perpendicular| ≤ along). Score = along + 2 × |perpendicular|, lowest wins (the weight is tunable in the step). With no candidate, nothing moves, and there is no wrap-around.

## Problem Statement

The measured game in section 1 of the plan spent 61% of its time in the free-camera modes, panning pixel by pixel to put the crosshair on a planet. Plan 12 replaces that with a cursor that jumps from object to object. Every later step (5: jump mode, 6: slipway building with the D-pad, 7: returning from aim mode) depends on one question: *from this object, pressing this direction, which object do I land on?*

Today nothing in the code answers it. The closest thing, `pad_panel_neighbor`, picks menu items by screen boxes. It only looks at the open panel and uses a different rule. If step 5 had to invent the picker while also rewriting the map's root mode and re-authoring replays, a picker bug would show up as a red e2e with no obvious cause, mixed in with mapping changes.

## Solution

A small new game module, the **target picker**, with no caller yet. It exposes:

- a predicate that says whether an entity is a map target (planet family, not dead, state KNOWN or SETTLED);
- a pick function: given an origin object, one of the four D-pad directions and a list of candidates, it returns the best candidate in that direction's 90° cone by the score along + 2 × |perpendicular|, or NULL.

It works on each object's fixed world point (`map_pos`), never on its drawn position (`pos`), which bobs. Unit tests on hand-built layouts pin the rule. Nothing calls the module yet, so the game, the e2e replays and their hashes do not change.

## User Stories

1. As the agent implementing step 5, I want a ready, tested function that returns the next target in a direction, so that jump mode is wiring and not geometry.
2. As the agent implementing step 6, I want the same function to accept a candidate list I filtered myself (the slipway start removed), so that slipway building reuses the picker unchanged.
3. As a player (from step 5 on), I want Right to take me to the object that is most "to the right" of where I am, so that the jump lands where I expect.
4. As a player, I want an object slightly off-axis but close to win over one exactly on-axis but far away, only when the score says so, so that the choice follows one predictable rule.
5. As a player, I want an object outside the 90° cone (more sideways than forward) never to be chosen, so that Right never sends me mostly up or down.
6. As a player, I want an object exactly on the cone's edge (|perpendicular| = along) still to count, so that the "≤" of the rule holds at the boundary.
7. As a player, I want a press with nothing in its cone to do nothing, so that the cursor never jumps somewhere surprising.
8. As a player, I want no wrap-around (Right at the rightmost object does not go to the leftmost one), so that the map feels like a map.
9. As a player, I want the search to cover the whole map, not just the screen, so that I can reach an object that is off screen in a few presses.
10. As a player, I want the object I am on never to be returned as its own neighbour, so that a press always moves or does nothing.
11. As a player, I want objects directly beside me (along = 0) not to count for that direction, so that Up never "moves" me sideways to an object at the same height.
12. As a player, I want unexplored planets never to be targets, so that the cursor does not reveal what the map hides.
13. As a player, I want wormholes, probes, slipways and structures still being placed never to be targets, so that the cursor only rests on things × and △ can act on.
14. As a player, I want vanishing or dead planets skipped, so that the cursor never lands on an object about to disappear.
15. As a player, I want known planets, colonies, structures and trading hubs to be targets, so that every object the plan's × and △ act on can be reached.
16. As a player, I want the same press from the same place to land on the same object whatever frame I press on, so that a settled structure's bob never changes my jump.
17. As a replay author (steps 5 to 13), I want the picker's result to depend only on world positions and candidate order, never on frame timing or the camera, so that the step 2b discipline is not made harder by the picker.
18. As a replay author, I want equal-score ties broken by a fixed, documented rule, so that a replay's jumps are reproducible and I can predict them from the map.
19. As the agent tuning the controls after the plan, I want the perpendicular weight to be one named constant, so that tuning is a one-line change under the re-authoring guardrails.
20. As a developer, I want the picker to be pure (no registry access, no camera, no ui, no allocation, no state), so that it is tested on hand-built layouts without booting a game.
21. As a developer, I want a misuse (a direction that is not exactly one D-pad bit, a NULL origin, an origin or candidate with no world point) to stop on an assert with a message, so that a wiring bug in step 5 fails at its cause.
22. As a developer, I want a NULL or empty candidate list to return NULL, so that a game with only the origin on the map needs no special case.
23. As a developer, I want screen-up to mean decreasing world y, the same as the camera and the panels, so that Up on the pad matches up on the screen.
24. As a developer, I want the module to add no struct-field or file-scope pointer and no `entity_list_t` field, so that the pointer guard's exceptions list does not change.
25. As a developer, I want `make test` not to get noticeably slower, so that the fast loop stays fast (`CLAUDE.md`).
26. As a plan reviewer, I want every e2e `.json.sha256`, every `.end-state` reference and every existing unit test unchanged, so that "no behaviour change" is shown, not claimed.
27. As a plan reviewer, I want the PRD to state which position the picker reads and why, so that step 5 does not mix the bobbing position with the fixed one.

## Implementation Decisions

- **New module, game layer.** A target-picker module in `src/game/` (its rule names planets and their states, so it cannot live in `engine/`), named `pad_target`. Exported names follow `module_verb[_object]`. It has no caller in this step. `pad_nav` is not touched. Its private `is_clickable_planet` predicate encodes the same state rule for another purpose (which presses stop in `planet_left_press`). It is left alone: step 5 decides whether to share code.

- **Interface.** Three items, all declared in the module's header:
  - `PAD_TARGET_PERPENDICULAR_WEIGHT`: an integer constant, 2, the weight of |perpendicular| in the score.
  - `int pad_target_is_candidate(const entity_t *e)`: 1 when `e` is non-NULL, not dead, of the planet family (`planet_is_family`), and in state `ENTITY_STATE_KNOWN` or `ENTITY_STATE_SETTLED`; 0 otherwise. So UNKNOWN (unexplored), VANISHING, PLACING and UNFOLDING planet-family entities are excluded. Wormholes, probes, slipways and ui entities are excluded because they are not of the planet family.
  - `entity_t *pad_target_pick(const entity_t *origin, int direction, const entity_list_t *candidates)`: the best candidate in that direction, or NULL when none qualifies, when `candidates` is NULL, or when it is empty. The header contract says the function reads only the origin's and candidates' world points and identities, does not filter by `pad_target_is_candidate` (the caller builds the live set), never allocates, and returns a pointer into `candidates`.

- **The candidate set is the caller's.** "Live candidate set" means the list the caller passes. In step 5 that is the planet-tag entities filtered by `pad_target_is_candidate`; in step 6 the same minus the slipway's start. The pick function trusts the list. It does not re-check the predicate, so a test can hand it bare entity headers, and step 6 can exclude the start without a second picker.

- **Position: `map_pos`, not `pos`.** The picker reads `entity_map_pos` on the origin and on each candidate:
  - `pos` is the drawn and hit-tested position. For settled structures and trading hubs, `planet_bob` adds a small sine to `pos.y` every frame (up to about 3 px; see step 2a's notes). A picker on `pos` could return a different target for the same press on another frame. Replays would then depend on press timing, which is exactly what step 2b's discipline has to keep under control.
  - `map_pos` is the fixed world point: set once from the galaxy roll or from the placement point (`targeting_track`), carried through project transforms and save/load, and never bobbed. It is also what the game itself uses for nearest-node and attach-distance queries (`entity_geom_nearest`).
  - `map_pos` is in map units: world pixels divided by `GALAXY_MAP_SCALE` (128). The picker scores in map units as stored, in `p8_fixed_t`, with no conversion. The rule is scale-invariant: the cone and the argmin of a linear score do not change when both axes are scaled by the same factor. The galaxy spans about 3 map units, so no sum can overflow.
  - Step 5 must still aim the played mouse at the target's `pos` (what the hit test reads). Choosing a target and pressing on it are two different questions (see Further Notes).

- **Geometry, exact in fixed point.** With `d = map_pos(candidate) − map_pos(origin)` and screen axes (x right, y down):
  - Right: along = d.x, perpendicular = d.y. Left: along = −d.x, perpendicular = d.y. Down: along = d.y, perpendicular = d.x. Up: along = −d.y, perpendicular = d.x.
  - In the cone when along > 0 **and** |perpendicular| ≤ along. The strict `along > 0` keeps out the origin's own point and objects directly beside it. The `≤` keeps the cone's edges, as the rule writes it.
  - score = along + `PAD_TARGET_PERPENDICULAR_WEIGHT` × |perpendicular|, lowest wins. Every value stays in `p8_fixed_t` (integer multiply of a fixed value, `p8_fixed_abs`), so no float and no rounding (the float guards stay green).

- **Origin excluded by identity too.** A candidate pointer equal to `origin` is skipped before any geometry, whether or not the origin is in the list. With the strict `along > 0`, this makes "origin excluded" hold even for a second object at the same point.

- **Tie-break.** At equal score, the smaller |perpendicular| wins (the more on-axis object). If that is equal too (two mirror-image objects on either side of the axis), the one earlier in `candidates` wins. Step 5 passes the registry's planet-tag order, which is creation order, so the result is reproducible in replays. Decided here because the plan's rule leaves equal scores open, and replays need a deterministic answer.

- **Asserts, each with a message:** `direction` is exactly one of `P8_PAD_UP`, `P8_PAD_DOWN`, `P8_PAD_LEFT`, `P8_PAD_RIGHT` (the same contract as `pad_panel_neighbor`); `origin` is non-NULL and has a map position; every candidate examined has a map position. Every planet-family entity in KNOWN or SETTLED has one: galaxy planets from the roll, structures from `targeting_track` before they settle, all of them through save/load. A missing one is a wiring bug, not a case to skip.

- **No new state, no registry access, no allocation.** The module declares no struct field and no file-scope variable, so `tools/pointer_guard_exceptions.txt` and `tools/alloc_guard_exceptions.txt` do not change.

- **Build wiring.** The new source joins the game sources the binaries and tests link, through the existing source lists, so the raylib binary, the replay tools and every test that links the game pick it up unchanged. One new unit-test binary is registered in the `Makefile` like the others (rule plus `UNIT_TESTS` entry).

## Testing Decisions

- **What a good test is here:** it calls only `pad_target_pick` and `pad_target_is_candidate`. It builds a layout by hand (bare entity headers on the stack with a map position set, gathered in an `entity_list_t` over the test's own storage) and asserts which entity comes back. It never inspects how the score is computed. Expected results are worked out by hand from the rule and written as plain pointer comparisons.

- **New test file `tests/test_pad_target.c`** (one test file per module, `CLAUDE.md`). No game boot, no cart, no registry: the picker tests use bare headers. The predicate tests use a test-local class whose lineage carries `LINEAGE_PLANET` and one that does not. It must run in a few milliseconds. Cases, each its own function:
  1. **One candidate per direction:** origin at the centre, one object straight up, down, left and right. Each direction returns its own object.
  2. **Tie broken by score:** two objects in the Right cone, one on-axis but far, one off-axis but near, positioned so the score (not the distance and not the perpendicular alone) picks the winner. A second layout swaps them so the other wins, which shows the weight 2 (not 1) is applied.
  3. **Nothing in the cone:** objects only outside the Right cone (|perpendicular| > along), behind (along < 0) and directly beside (along = 0). Right returns NULL. Also a NULL list and an empty list return NULL.
  4. **Origin excluded:** the origin is in the list, alone and with others. It is never returned. A layout where it is the only entry returns NULL.
  5. **Cone edge inclusive:** an object at exactly |perpendicular| = along is returned.
  6. **Equal-score tie-break:** equal score, different |perpendicular|, so the on-axis one wins. Mirror images, so the earlier in the list wins, checked in both list orders.
  7. **Whole map, no wrap:** a far object well beyond one screen is returned. Right from the rightmost object returns NULL, even with objects on its left.
  8. **Uses `map_pos`, not `pos`:** a candidate whose `pos` and `map_pos` disagree (as a bobbing structure's do) is judged by `map_pos`.
  9. **Predicate:** KNOWN and SETTLED planet-family headers are candidates. UNKNOWN, VANISHING, PLACING, UNFOLDING, a dead one, a non-planet class, and NULL are not.
- **Not tested:** the misuse asserts. They abort, and the codebase's tests do not exercise asserts.
- **Prior art:** `tests/test_pad_nav.c` checks `pad_nav_neighbor` (the panel's directional neighbour) with plain pointer asserts. `tests/test_entity_list.c` builds lists by hand. Follow their style: one small static function per case, called from `main`.
- **Guard rails checked once before committing:** `make test-all` green; `make e2e` gives every `*.sha256` unchanged; `make end-state` (outside the suite) gives all references unchanged. The module has no caller, so a change in either would mean wiring leaked in. `make test` wall time is compared before and after, and a new test binary that shows up in it is a defect.

## Out of Scope

- Any caller: jump mode, the D-pad repeat (18/8 frames), the hovered marker, the edge scroll, the camera rule, game start on the starting colony (all step 5); slipway building with the D-pad and excluding the start (step 6); "the target nearest the centre" when leaving aim mode (steps 5 and 7, a different query: nearest, not directional).
- Building the live candidate set from the registry. Step 5 does it with `pad_target_is_candidate`. This step only defines the predicate.
- Any change to `pad_nav`, `pad_panel`, the ui, the camera, or the cheat sequence.
- Re-authoring or re-baselining any e2e replay, `.txt`, `.end-state` reference or `worst-case/` game. None is expected to change.
- The Select button (step 3) and the feasibility study (step 2b).
- Tuning the weight or the cone. Section 5 keeps the first guesses for the whole plan.
- Audio, PSX, mouse input.

## Hypotheses

Steps 2b and 3 are not done when this PRD is written. Everything below that depends on their outcome is an assumption. If one turns out false, the implementer applies the stated fallback and records it in the step note.

- **H-3-1.** Step 3 adds `P8_PAD_SELECT` as one new bit in `src/p8/input.h`, alongside the existing D-pad bits, and changes none of `P8_PAD_UP/DOWN/LEFT/RIGHT`. The picker's direction assert names only those four, so it neither needs nor rejects the new bit. *Fallback:* if step 3 renumbers the D-pad bits, the assert and the tests use the names, not the values, so nothing changes.
- **H-3-2.** Step 3 leaves every e2e unchanged, as its text says. This step's "e2e unchanged" is judged against the tree it lands on, whether or not step 3 is in it. *Fallback:* if step 3 re-baselined something, compare against step 3's committed hashes, not the older ones.
- **H-3-3.** Step 3 and this step touch disjoint files, except possibly the `Makefile`'s test lists, where both add entries. *Fallback:* a textual merge, with no semantic conflict.
- **H-2b-1.** Step 2b's study confirms that `map_pos` does not move with frame timing or camera path once an object is KNOWN or SETTLED, and that the bob touches only `pos` (step 2a's finding, which 2b re-checks). The picker's choice of `map_pos` rests on this. *Fallback:* if 2b finds a timing-dependent `map_pos` (for example a structure's placement point moving with the camera), that is a placement or discipline issue for the re-authoring steps. The picker still reads `map_pos`, and the step note records it.
- **H-2b-2.** Step 2b's discipline does not require jumps to be decided from screen positions. The picker is camera-independent by design, which is the property 2b's discipline wants. *Fallback:* none needed. A camera-dependent picker would break the plan's "searched over the whole map" rule anyway.
- **H-2b-3.** Step 2b's coverage comparison script (`tools/coverage_compare.sh`, committed in its chunk) compares baseline lines only. This step deletes and rewrites no baseline line and only adds lines (the new module, hit by unit tests but by no e2e), so the coverage guardrail does not apply in a way that blocks it. *Fallback:* if the script also reports new unhit lines as failures, the module's lines are expected to be unhit by e2e until step 5 wires it. Record that, and do not add a replay for it.
- **H-2b-4.** No relaxed comparison or exceptions file from step 2b affects this step: it re-authors no replay. *Fallback:* none needed.

## Further Notes

- **For step 5:** the picker chooses by `map_pos`, but a mouse press must land where the hit test reads, which is `pos` (bobbed). Step 5 should move the played cursor to the chosen target's `pos` on each frame, or to its hit-box centre as `pad_panel_focus_point` does for panel items, not to `map_pos × 128`. For a bobbing structure the two differ by up to about 3 px.
- The rule is the one `pad_panel_neighbor` already uses for menu items in spirit (along + 2 × gap), so players meet one directional logic in menus and on the map. The two stay separate functions: one works on screen boxes of panel items, the other on world points of map objects.
- Step 14's "dead D-pad press into an empty cone" group is exactly the NULL return of `pad_target_pick`. Step 14 can count it by calling the picker, with no second definition.
