---
title: "Slipways from ×"
description: "Building a slipway on the pad still means leaving jump mode, panning a crosshair from the starting colony onto the target pixel by pixel and dropping it there; × on the hovered colony must start it, the D-pad must jump it from target to target, and × must build it."
status: needs-triage
---

## Plan step (verbatim)

Plan file `docs/plan-12.md`, Phase 2 — Jump mode on the map, step 6:

> 6. **Slipways from ×:** × on a colony starts the slipway, the D-pad jumps between targets with the button held, × builds and keeps the cursor on the target, ○ cancels. A refused start holds nothing. `pad_slipway` and the replays that build slipways re-authored. Unit test: building a chain of two slipways with × D-pad × D-pad ×. Also find and record one deliberate cheat route (10 replays type the cheat on purpose): a place and mode where the 8 presses leave nav, cursor and game state unchanged. If none exists, apply the cheat fallback of section 2.

The rules it follows (section 2):

> **Buttons**
> - **×** acts on the hovered object:
>   - On a colony or structure: start a slipway from it.
>   - On a known planet: open its projects (colonize) menu, as a mouse press there does.
>   - If the press is refused (for example a full slipgate), nothing is held and no press is retried or passed on: it fixes the bug above by design.
> - **Building a slipway:** the D-pad jumps between targets (same picker, start excluded) while the left button stays held on the target, so the game draws its own green or red line. × releases on the target: built or refused by the game's rules. When built, the cursor stays on the target, so × chains the next slipway from there. ○ cancels through the ui's own drop with no target (as `cancel_drag` does today).

> **Guardrails** (the cheat)
> - New inputs must not type the cheat sequence (× × ○ ○ × × ○ ○, see `src/game/cheat.c`) by accident. Checked by the end-state check of step 2a: each replay's cheat activation count must match its reference (some replays use the cheat on purpose).
> - Every step from 6 on that changes what × or ○ do (including the build bar and the ring's removal) checks whether ordinary input sequences, with D-pad presses between them (`cheat_step` ignores them), can complete × × ○ ○ × × ○ ○, and re-checks the deliberate cheat route of step 6. If ordinary play can complete it, or no side-effect-free deliberate route exists, change the cheat sequence in `src/game/cheat.c` (a dev tool, not a game rule) to one ordinary play cannot type and that has such a route, and update the replays that type it, in the same step.

The other guardrails of section 2 apply as written: re-authoring to the step 2a end state against reference `8defcba`, the situation guardrail (every situation of the old `.txt` kept), the frame limits (180, or 600 if listed in `tools/e2e_long_exceptions.txt`), shared prefixes first, pinned values re-derived, coverage never drops, the cost record and the step 5b trigger, the step 2b discipline (its 8 rules), `worst-case/` games knowingly stale, one scheme only.

## Facts from step 5

Step 5 is done (`97f3814` to `6e841dc`, step note `f39fcdc`). This PRD builds on what it landed, read from its commits, its step note and the committed `pad_nav`. Every decision below rests on these facts, not on assumptions.

- **D-pad jumps** (`4a1dca9`). In jump mode, `pad_jump_repeat_step` gives the direction each frame: a press, then repeats at +18, +26, +34 … while held. `jump_from` sets the hovered object to `pad_target_pick(hovered, direction, candidates)` unless the pick is NULL. The candidates come from `live_candidates`: the `TAG_NODE` bucket filtered by `pad_jump_candidates` into a local array of `JUMP_CANDIDATES_CAP`. No jump runs on a click's press or release frame, but the repeat count keeps running through them. On every frame jump mode does not run, `pad_nav_step` clears the repeat state (`jump_ran`). `map_step`'s last branch asserts that jump mode is the only map mode left after placement, panel, free mode and network mode.
- **△** (`875cd0a`, `triangle_on_hovered`). △ clicks the hovered object only when its rest screen point (`rest_screen_point`: `map_pos` minus the camera before shake) lies inside the inner rectangle, and no higher-layer HUD item covers its `pos` screen point (`pressed_before`). The map mode stays jump. No test covers △ refused under a HUD item (a known gap in the step note).
- **○ in network mode** (`6e841dc`). With no box open and not on a release frame, ○ in network mode resets the map mode to jump, hovering the selection. It is free mode's only way back to jump mode before step 7.
- **× and ○ do nothing in jump mode.** □ from jump mode enters today's plain free mode with the camera unmoved (`enter_plain_free_mode`). Network mode's other presses, the old free mode, its □ select, the old drag and the probe/fold are unchanged. ○ in plain free mode still returns to the last planet.
- **Where the e2e cheats sit now** (read from the replays' `.txt`):
  - `pad_research` and `pad_bankrupt` type the cheat in plain free mode, unchanged.
  - `pad_tech_costs` types it in jump mode, hovering p31, a known planet that is not yet colonised (f128–142).
  - `pad_planet_projects` types its first cheat in jump mode on p25, which is known (f87–103). It types its second cheat in jump mode on p47, which is a colony by then (f295–310).
  - Step 6's × in jump mode starts a slipway on a hovered colony, so it would break that second cheat. The other two jump-mode cheats hover known planets, where × stays inert until step 8.
- **References.** All 17 e2e replays, `title_idle` and `full_game_pause_restart` pass `make end-state` against the `8defcba` references. No `.end-state` reference was re-baselined, and no relaxed-comparison exceptions file exists. `full_game`, `full_game_new_game` and `full_game_pause_idle` are stale: their scripts lack the □.
- **Cost and step 5b.** Step 5's re-authoring took about 24 minutes, under the 170-minute trigger: no step 5b and no generator. Replays are still written as per-frame JSON.
- **Frame counts**: `pad_research` 577, `pad_tech_costs` 571 and `pad_planet_projects` 568 of 600 (562 before step 5, which needed one Up tap under rule 8); `pad_bankrupt` 180 of 180.
- **Coverage.** `make coverage-compare` shows 0 unhit lines. `pad_planet_projects` lost its incidental s2 → p47 → s2 network travel; `pad_slipway` still covers it.
- **The jump state.** `pad_nav_t` holds one `pad_jump_t`. `pad_nav_jump_mode` and `pad_nav_hovered` keep the contracts in `pad_nav.h`.

## Problem Statement

The map's root is jump mode since step 5, but jump mode cannot build anything. To build a slipway, the player presses □ to reach the old free mode, then pans the crosshair onto a colony, presses □ to select it (network mode), and presses □ again to start the drag. Then they pan pixel by pixel to the target, and × drops the slipway there. In the measured game (plan section 1), about 100 slipways were started, 42 built and about 35 abandoned, and network mode and the drag alternated 153 times. □ selects, starts, cancels and leaves, depending on 6 sub-modes.

The section 1 bug class also stays open in the old path. A start the game refuses is only avoided by a guard in `begin_drag`, not by design.

The cheat is a separate issue. With × starting and building slipways and ○ cancelling them, ordinary play types × × ○ ○ × × ○ ○: build a slipway, back out with ○ twice, and do it again. With D-pad presses in between, which the matcher ignores, the cheat then fires during a real game.

## Solution

In jump mode, **×** acts on the hovered object:

- **On a colony or a structure** (a settled planet, structure, trading hub, slipgate or infraspace gate), × presses it as the mouse does. The game hands over a slipway, and the pad is now in **slipway mode** from that start. The left button stays held.
- **A refused start holds nothing.** On a slipgate or infraspace gate that takes no more slipways, on an object the HUD covers, or on an object still scrolling into the inner rectangle, × presses nothing. Nothing is held, nothing is retried, and the mode stays jump.
- **On a known planet**, × does nothing yet. Step 8 opens its projects menu. With nothing hovered, × does nothing.

In **slipway mode**:

- **The D-pad** jumps the aim between targets with the step 4 picker, over the whole map, with the start excluded and the same hold repeat as jump mode. The cursor and the held left button sit on the aimed target, so the game draws its own line: colour 1 with its cost, or colour 2 with the forbidden sign or "-too far-". The camera follows the aimed target with the same edge scroll as jump mode, so the start can go off screen.
- **×** releases the button on the aimed target, and the game's drop decides:
  - **Built:** the target becomes the new start. On the next frame the pad presses it, and slipway mode continues from there, aimed at it. So × D-pad × D-pad × builds a chain of two. If the new start cannot take another slipway, the pad is back in jump mode, hovering it.
  - **Refused** (a known planet, a crossed planet, too far, a slipway already there, not enough credits): on the next frame the pad presses the same start again, aimed at the start, so the player picks another target. If that press is refused, the pad is back in jump mode, hovering the start.
  - × while aimed at the start itself does nothing.
- **○** cancels through the ui's own drop with no target: nothing is built or charged. The pad is back in jump mode, hovering the start of the slipway it cancelled.
- △ and □ do nothing in slipway mode. A tab change, pause, or a panel or box opening cancels the slipway, as they cancel the drag today.

**The cheat** becomes four **× + ○ chords**: × and ○ going down on the same frame, four times, with no lone × or ○ press in between. From a chord's frame until both buttons are up, the pad ignores × and ○ everywhere, so a chord never presses, cancels, closes or builds anything. Ordinary play cannot type four exact-frame chords in a row, and any place in the game is a side-effect-free deliberate route.

The old free mode, its □ select, network mode and the old drag stay reachable through □ as in step 5. Step 7 removes the □ select and the drag, and step 12 deletes the code. Every slipway the e2e replays build is now built by the × route. Situations only the old drag can produce stay on the interim path until step 7.

## User Stories

1. As a player, I want × on my hovered colony to start a slipway from it, so that building begins where the cursor already is.
2. As a player, I want × on a structure, trading hub or gate to start a slipway too, so that every node of my network links the same way.
3. As a player, I want × on a full slipgate to do nothing visible and hold nothing, so that a refused start never turns into a probe, a wormhole or a stuck press.
4. As a player, I want × on an object the HUD covers to do nothing, so that × never presses a HUD button by accident.
5. As a player, I want × on an object still scrolling into view to do nothing, so that the press always lands on what I see.
6. As a player, I want × on a known, uncolonised planet to do nothing in this step, so that no stray press changes my game before step 8 gives it the menu.
7. As a player, I want the D-pad to jump my slipway's end from target to target, so that aiming takes a few presses, not a pan.
8. As a player, I want the start excluded from the jumps, so that the slipway's end never lands back on its own start.
9. As a player, I want holding a direction to sweep the end across targets with the jump mode repeat, so that a row of planets is quick to scan.
10. As a player, I want the game's own green or red line, cost label, forbidden sign or "-too far-" drawn on each target, so that I see the verdict before I press.
11. As a player, I want the camera to follow the aimed target with the same edge scroll as jump mode, so that the target never leaves the comfortable area of the screen.
12. As a player, I want the start allowed to leave the screen, so that long slipways are as easy to aim as short ones.
13. As a player, I want × on a valid target to build the slipway, so that one press commits it.
14. As a player, I want the slipway mode to continue from the target I just linked, so that × D-pad × D-pad × builds a chain.
15. As a player, I want × on a refused target to leave me aiming from the same start, so that I can pick another target without starting over.
16. As a player, I want ○ to cancel the slipway with nothing built or charged, so that backing out is always safe.
17. As a player, I want the cursor back on the start after ○, so that ○ is one step back.
18. As a player, I want a year box, the pause menu or a tab change to cancel the slipway, so that nothing stays held behind a panel.
19. As a player, I want △ and □ inert in slipway mode, so that one mode means one set of buttons.
20. As a player, I want × on the target that refuses because I lack credits to leave my credits untouched, so that the game's rules alone decide.
21. As a player, I want the blinking marker on the aimed target, so that I always see which object the end is on.
22. As a player, I want the cheat never to fire while I build and cancel slipways, so that my game stays honest.
23. As a developer, I want the cheat typed as four × + ○ chords, so that it has a route with no side effect anywhere.
24. As a developer, I want a chord ignored by every pad mode, so that the deliberate cheat route stays inert in every later step.
25. As a replay author, I want slipway mode's camera path to depend only on the sequence of aimed objects, so that the step 2b discipline (rule 8) still holds.
26. As a replay author, I want every slipway the replays build to go through the × route, so that the e2e covers the new scheme.
27. As a replay author, I want the situations only the old drag can produce kept on the interim path, so that no situation of an old `.txt` is lost before step 7 removes its mechanic.
28. As a replay author, I want the slipways' directions kept (start = the first end of each `slipway` line), so that the end state matches the reference.
29. As a replay author, I want shared prefixes re-authored first and reused verbatim, so that the work is done once.
30. As a reviewer, I want every re-authored replay to pass `make end-state` against `8defcba` with its cheat count kept, so that the game end state is unchanged.
31. As a reviewer, I want each replay's old and new `.txt` compared in the same commit, so that no situation silently disappears.
32. As a reviewer, I want `make coverage-compare` to show no newly unhit line except the old drag's build landing, which this step replaces, so that the replays still exercise what they did.
33. As a reviewer, I want the pinned values of step 2a's inventory re-derived with their intent kept, so that no assertion is weakened.
34. As a reviewer, I want the re-authoring cost and the replays broken recorded, so that the step 5b rule and later steps can be judged.
35. As the agent of step 7, I want the old drag's remaining situations listed in the step note, so that I know which ones go with the drag.
36. As the agent of step 8, I want × on a known planet left inert and ○ in jump mode untouched, so that my step only adds the menu and the ○ order.
37. As the agent of step 9, I want the new frame counts of the year-heavy replays recorded, so that the Select-hold budget of step 2b can be re-read.
38. As the agent of step 14, I want slipway mode readable from outside the pad navigation (its start and aimed target), so that the metrics tool can attribute time and dead presses to it.
39. As a developer, I want slipway mode's state and arithmetic in a pure module that `pad_nav` only calls, so that step 12 deletes old functions without touching it.
40. As a developer, I want `make test` and `make e2e` no slower, so that the fast loop stays fast.

## Implementation Decisions

### The cheat (decided first, because it decides the order of the chunks)

- **Verdict on ordinary play: it completes the old sequence.** With the rules of this step, × (start on A), D-pad, × (build A→B, chain continues from B), ○ (cancel the chain), ○ (a dead press at the root in this step, the jump to the last object from step 8) is an ordinary way to build one slipway and back out. Done twice, with D-pad presses in between, it types × × ○ ○ × × ○ ○. Section 1 measured 12 dead ○ presses in one game, so the dead ○ is ordinary too. Section 2's fallback therefore applies: the cheat sequence changes in this step.
- **The deliberate route on the old sequence, recorded but not kept.** In this step, jump mode hovering a known planet or hovering nothing leaves × and ○ inert, so it would be a side-effect-free route. `pad_tech_costs` and `pad_planet_projects`' first cheat already use it (step 5 moved them there). It does not hold for a hovered colony, and `pad_planet_projects`' second cheat hovers one: step 6's × would start a slipway there. Step 8 gives × on a known planet and ○ at the root jobs too. In any case, the route does not cure the ordinary-play verdict.
- **The new sequence: four × + ○ chords.** A chord is × and ○ both going down on the same frame. `cheat_step` already detects that frame (today it resets the matcher). The new matcher counts chords. A lone × or ○ press resets it to 0, and D-pad and other buttons are ignored, as today. The fourth chord fires. `cheat_t` keeps the previous × and ○ state and the match count. `cheat_step`'s contract stays the same: it is fed the raw pad buttons and returns 1 on the firing frame.
- **The chord is inert in the pad navigation.** A new rule in `pad_nav_step`, before any mode runs: on a frame where × and ○ both go down, and on every frame after it until both are up, × and ○ are removed from the buttons `pad_nav` sees, both the held mask and the pressed edges. One new field in `pad_nav_t` (a chord-held flag) supports it. The title, panels, pause, free mode, network mode, jump mode and slipway mode all see nothing, so every place is a side-effect-free route. This is a mapping rule (how pad input becomes presses), not a game rule. The guardrail of section 2 re-checks it in steps 7–12.
- **Recorded route:** in jump mode, hovering any object, four chords one every 2 frames (chord, release, …, 8 frames). Nav state, cursor, camera, played buttons (0 on every frame), world counts and the game stream are unchanged, and the cheat fires once.
- **What is updated** in the same chunk: `src/game/cheat.c` and its callers' tests (`tests/test_input_step.c`, `tests/test_economy.c`, `tests/test_main_economy.c`, which type the sequence), and the four e2e replays that type it (`pad_bankrupt` 1, `pad_planet_projects` 3, `pad_research` 1, `pad_tech_costs` 1). Each cheat is retyped as four chords at the same point of the game, so the money and research arrive before the same actions. The old taps had side effects that the `.txt` lists as situations: `pad_research`'s four X taps place probes that are refused beside p78, and `pad_tech_costs`'s X taps on p31 open no menu and show a colour-8 label. Those taps are kept as separate presses, so the situation stays. The other taps are dropped, which saves frames (`pad_bankrupt` is at 180/180). The `worst-case/` games type the old sequence too. Their cheat counts drop to 0, which is knowingly stale until step 13 retypes them (section 2). The step note records it.

### Modules

- **New `pad_slipway` module** (`src/game/`), pure helpers in the style of `pad_jump`: no registry lookup, no camera, no ui, no allocation, no file-scope state. `pad_nav` owns one `pad_slipway_t` and only calls these. The name follows the mode-per-module pattern (`pad_target`, `pad_jump`, then `pad_slipway`). It shares its name with the e2e replay `pad_slipway`, which is harmless: one is `src/game/pad_slipway.*` and `tests/test_pad_slipway.c`, the other `tests/e2e/pad_slipway.*`.
  - `pad_slipway_t`: the start's `entity_id_t`, the aimed target's `entity_id_t` (`ENTITY_ID_NONE` while aimed at the start), a phase (`PAD_SLIPWAY_NONE`, `PAD_SLIPWAY_PRESSED` on the start's press frame, `PAD_SLIPWAY_HELD`, `PAD_SLIPWAY_RELEASED` on the drop's frame), and the `linked_before` flag of the last release. It holds plain values and ids only, so `tools/pointer_guard_exceptions.txt` does not change.
  - `void pad_slipway_init(pad_slipway_t *slipway)`: phase NONE, no start, no target.
  - `int pad_slipway_can_start(const entity_t *e)`: 1 for a live planet-family entity in state SETTLED that takes a slipway (not a slipgate or infraspace gate refusing one, via `slipgate_takes_slipway`, the rule `takes_slipway` holds today); 0 otherwise, NULL included. It reads only the entity.
  - `int pad_slipway_candidates(const entity_list_t *candidates, const entity_t *start, entity_t **storage, int capacity, entity_list_t *out)`: `out` over the caller's storage, holding `candidates` minus `start`, order kept. It returns the count and asserts, with a message, that `capacity` is enough. The step 4 picker does not filter, as its step note says, so the exclusion lives here.
  - `entity_t *pad_slipway_between(const entity_t *start, const entity_t *target)`: the slipway that joins them, from `start`'s slipways list, or NULL. It is the same rule as `pad_nav`'s private `slipway_between`, which stays as it is (old code, which step 12 deletes).
  - `pad_slipway_outcome_t pad_slipway_outcome(const entity_t *start, const entity_t *target, int linked_before)`: `PAD_SLIPWAY_BUILT` when a slipway joins them now and none did before the release, otherwise `PAD_SLIPWAY_REFUSED`.
  - `void pad_slipway_release(pad_slipway_t *slipway, int linked_before)`, `void pad_slipway_rebase(pad_slipway_t *slipway, pad_slipway_outcome_t outcome)`: the state changes of the release and of the next frame. Built moves the start to the target. Built or refused clears the target and sets the phase to PRESSED (the re-press). They hold no rule beyond the description above. They exist so the transitions are unit-tested without a cart.
- **`pad_nav` gains slipway mode**, as wiring only (registry, camera, ui, and calls into `pad_slipway`, `pad_jump` and `pad_target`):
  - `PAD_NAV_MAP_SLIPWAY` in `pad_nav_map_mode_t`.
  - `entity_t *pad_nav_slipway_start(const pad_nav_t *nav)`: the start while the map mode is slipway; NULL in any other mode or once the start is dead.
  - `entity_t *pad_nav_slipway_target(const pad_nav_t *nav)`: the aimed target in slipway mode; NULL while aimed at the start, in any other mode, or once the target is no longer a `pad_target_is_candidate`.
  - `pad_nav_jump_mode` and `pad_nav_hovered` keep their contracts (jump mode only). `pad_nav_free_mode` is unchanged, so slipway mode draws no crosshair.
  - One new `pad_slipway_t` field and the chord flag.
- **Removed nothing.** The old free mode, its □ select, network mode, the drag (`begin_drag`, `drag_step`, `drag_due_step`, `cancel_drag`) and `return_to_last_planet` stay (steps 7 and 12).

### Jump mode: ×

A new branch of `jump_step`, after the seeding and the □ branch and before the D-pad. It runs when × is pressed, not on a click's press or release frame, and with no click pending:

1. The hovered object must exist and pass `pad_slipway_can_start`. It must also be pressable by △'s test: its rest screen point inside the inner rectangle and no higher-layer HUD item over its `pos` screen point. That test is factored out of `triangle_on_hovered` into one private predicate, which △ and × both call, with △'s behaviour unchanged. The left button must have been up on the previous frame (`left_was_down`, as `begin_drag` requires). Otherwise × does nothing this frame, and no later frame retries it.
2. The played mouse goes to the hovered object's screen point by `pos` (the hit test reads `pos`), with the left button down. The map mode becomes slipway, with start = hovered, no target and phase PRESSED. No jump and no edge scroll run on this frame.

A known planet, nothing hovered, and every refused case: × does nothing. ○ in jump mode stays inert (step 8). △ and □ keep their step 5 behaviour.

### Slipway mode, frame by frame

A new branch of `map_step`, before jump mode's last branch, runs when the map mode is slipway. That branch's assert names slipway mode too. In order:

1. `reset_free_mode`, focus cleared.
2. **Lost start** (dead, or no longer a planet-family entity): the ui's held slipway, if any, is dropped with no target (the same call `cancel_drag` makes: the class's `drop` with NULL, then `ui_set_left_held(NULL)`). The map mode becomes jump, hovering the target if it is still a candidate, otherwise nothing (the jump mode reseed then runs). Nothing else runs this frame.
3. **Phase RELEASED** (the frame after a ×): the outcome comes from `pad_slipway_outcome(start, target, linked_before)` and `pad_slipway_rebase` applies it. Then the re-press: if the new start passes `pad_slipway_can_start` and is not covered by the HUD (its screen point may be off screen, as the old drag's off-screen press was, `pad_slipway.txt` f236), the mouse goes to its screen point with the left button down. Otherwise the map mode becomes jump, hovering the new start. This frame takes no input.
4. **Phase PRESSED → HELD**: from the frame after a press, the ui holds the slipway (the old `dragged_slipway` rule: the ui's left-held entity is a live slipway). If it does not, the press was refused after all: back to jump mode on the start, nothing held. A refused start holds nothing, and nothing is retried.
5. **○ pressed** (no box open, `circle_returns`): the ui's own drop with no target, as above. Nothing is built or charged. The map mode becomes jump, hovering the start. Nothing else runs.
6. **× pressed** with a target, the target pressable (the same predicate as △), and not a release frame: `linked_before` = `pad_slipway_between(start, target) != NULL`, recorded by `pad_slipway_release`. The left button goes up at the target's screen point by `pos`, so the ui's release drops the slipway there and the game builds or refuses it. With no target, or the target not pressable, × does nothing and the button stays held.
7. **D-pad**: `pad_jump_repeat_step` (the same repeat state as jump mode) gives the direction. Slipway mode counts as running the repeat (`jump_ran`), so a held direction is not cleared on its frames. The aimed object (the target, or the start when there is none) is the origin. The new target is `pad_target_pick(origin, direction, pad_slipway_candidates(live candidates, start))` if it is not NULL. No jump runs on the frame of a × or ○ press.
8. **Edge scroll** on the aimed object: the same computation as jump mode (rest point by `map_pos`, camera before shake, `pad_jump_scroll`, `camera_pan`). The camera path depends only on the sequence of aimed objects.
9. **Cursor and button**: the played mouse goes to the aimed object's `pos` screen point, with the left button held (except on the release frame of step 6). The game draws the line, the snap, the cost or the refusal itself.
10. **Marker**: the aimed object goes to `show_selection` with no slipway, except on the press, release and re-press frames.

△ and □ do nothing in slipway mode. The live candidate set is built as in jump mode (`live_candidates`, a local array, no zone allocation), and only on frames that jump.

### Cancels from outside

- **Pause** (`pause_step`), **a tab change** (`unwind_sub_modes`, which reports `CANCELLED_SLIPWAY` for slipway mode too, so the frame's cursor rule stays the same), and **a panel or box opening** (`pad_nav_step`'s `cancel_drag` call) cancel slipway mode as they cancel the drag: the ui's null drop, then jump mode hovering the start. They call one new private `cancel_slipway`. The old `cancel_drag` calls stay where they are, and the old drag code is not edited.
- **A new game** (`start_new_game`) resets slipway mode with the rest of the map mode (`reset_map_mode` also calls `pad_slipway_init`).
- **The title, the build tab, placement**: placement can only begin from the build tab, which a tab change reaches after cancelling. Nothing else changes.

### Why the re-press and not a blind hold

The ui hands out a slipway only on a left press on a settled planet, and its drop ends it. The old drag already re-pressed the start after a refused drop (`begin_drag` on the next frame, `pad_slipway.txt` f127). Slipway mode does the same, explicitly, for both outcomes. A refusal by the start itself (a slipgate that became full with the slipway just built) is seen by `pad_slipway_can_start` before the press, so no press reaches a lower layer: the section 1 bug cannot come back.

### Re-authoring

- **Expected breakage.** Chunk 2 (the cheat) breaks the four replays that type the cheat. Chunk 3 (× in jump mode) breaks any replay that presses × in jump mode. After chunk 2 there should be none (the only jump-mode cheat on a colony, `pad_planet_projects`' second, is retyped as chords there), and any that breaks is re-authored in that commit. Chunks 4–8 re-author the slipway-building replays on purpose (the plan step names them): every `slipway` line of an end state is built by the × route. Those are `pad_colonise` (1), `pad_slipway` (1), `pad_research` (4), `pad_tech_costs` (6) and `pad_planet_projects` (3). `pad_resume_at_start`'s 4 slipways come from its save: it is re-cut only if `pad_research`'s last record changes.
- **The × route for each slipway**: in jump mode (□ is not needed: re-authored segments stay in jump mode), D-pad jumps onto the start, ×, D-pad jumps onto the target, ×. Each slipway keeps its direction (step 2b rule 6): the start is the first end of its `slipway` line (for example `slipway s2 s1` starts on s2, `slipway p78 s2` on p78). Consecutive slipways in a chain use the continuation. Costs and their number are unchanged (rule 5). Their timing is free.
- **Situations the × route produces**, re-authored onto it:
  - a refused known-planet target (`pad_slipway` f126);
  - a crossed planet (pick a target beyond a planet);
  - an already-linked target (duplicate);
  - not enough credits, where a replay has it;
  - an off-screen start (jump far enough that the edge scroll pushes the start off screen);
  - a cancel (○);
  - "too far", when a candidate lies beyond reach of the start. `pad_slipway`'s f261 measured "-too far-" on empty space, 105 px from p47. If no candidate in that galaxy lies beyond reach, "too far" stays on the interim path (next item).
- **Situations only the old drag produces** stay on the interim path (□ to free mode, □ select or network □ into the drag), because step 6 does not remove that mechanic:
  - a drop on empty space;
  - a fold held while a drag is out (`pad_slipway` f194–210, `pad_colonise` f258–260);
  - △ under the centre during a drag, and colonising from a drag that resumes aimed at the colony (`pad_slipway` f229–236);
  - "too far" to empty space, if needed (above).
  The step note lists each one, so step 7, which removes the □ select and the drag, knows which situations go with their mechanic.
- **Network mode situations** stay produced through the interim path where a replay is the only one producing them: `pad_slipway`'s travel along a slipway with ×, Left/Right on a planet with one slipway, □ from network mode into the drag, the duplicate seen from network mode, and □ selecting the colony under the centre. Network travel used only as a way to get somewhere (`pad_research` "X travels back to s1", `pad_tech_costs` "X back to p54") is replaced by jumps. Section 2 lists network travel among the mechanics the plan removes. Each replacement is noted, and is allowed only while `make coverage-compare` shows the network lines still hit by `pad_slipway`.
- **Coverage.** `make coverage-compare` after each re-authoring commit. The only lines allowed to become newly unhit are those of the old drag's build landing (the `drag_due_step` branch that finds the new slipway and lands network mode on its target), since this step's × route replaces it. They are listed in the step note as replaced by step 6. Any other newly unhit line means a replay is re-authored until it is hit again.
- **Camera discipline** (step 2b rule 8). After a jump or slipway segment, the camera is wherever the edge scroll left it. Before the next point action in free mode (probe, fold, placement, △ or □ on the centre), the replay re-pans to the reference camera. Every placed structure keeps its world point exactly.
- **Shared prefixes first**:
  - `pad_probe` (no slipway, unchanged by this step) → `pad_colonise`, whose f0–98 build no slipway and stay verbatim. Its slipway part (f146 on) is re-authored first, and `pad_slipway` (f0–98 of `pad_colonise`) after it.
  - `pad_research` → `pad_resume_at_start`.
  - `pad_tech_costs` and `pad_planet_projects` share `pad_probe`'s or `pad_title`'s first frames, which this step does not touch.
- **Frame limits**: within 180, or 600 if listed. The jump route is usually shorter than square, square, pan and X, and the chord cheat takes 8 frames where the old taps took 16 or more. Savings go first to `pad_research` (577), `pad_tech_costs` (571) and `pad_planet_projects` (568). The reasons in `tools/e2e_long_exceptions.txt` are updated with the new counts.
- **Pinned values** (step 2a's inventory), re-derived in the commit that changes the replay they read, each keeping its intent:
  - `tests/test_live_slots.c` frames on `pad_tech_costs` (f546, f555, f565, f570 and `p54`) and `pad_planet_projects` (f171, f172, f255, f256), and any fixture cut from a re-authored replay;
  - `tests/test_state.c`: `pad_tech_costs`'s counts, lines, slipways and camera, and its cheat count of 1. The `pad_research` cut moves to the frame the chord cheat fires, keeping credits 200, science 99 and cheat 1;
  - `tests/test_rng_sync.c`'s windows on `pad_colonise` (the burst and quiet cases), then `make rng-sync-probe` re-run;
  - the golden frame hashes, the `.txt` files and the `.json.sha256` files. The `.end-state` references are never re-baselined.
- **Discipline**: the 8 rules of step 2b. A replay that cannot reach its reference falls under section 2's relaxed-comparison fallback, and the first such replay adds the exceptions file and its support in `tools/end_state.sh`. Expected: none, since this step changes no reveal and no fold point.
- **`worst-case/` games**: knowingly stale (section 2). `make full-game`, `make save-worst-case` and `make save-fuzz` are run once. An assert or a crash is a bug to fix in this step. Any other change (the cheat counts dropping to 0 included) is ignored and noted.
- **Cost record** in the step note: replays broken, re-baselined and re-authored, and the agent time spent re-authoring. Step 5b's trigger is step 5's alone and is not re-judged here.
- **Step 9's budget**: the step note records the new frame counts of `pad_colonise`, `pad_planet_projects`, `pad_tech_costs`, `pad_research` and `pad_bankrupt`, so step 9 re-reads the Select-hold table of step 2b against them.

## Implementation Chunks

Each chunk ends on a commit where `make test-all` passes. A chunk that changes what a replay sees includes, in the same commit, the re-authoring of every replay it breaks and its pinned values.

1. **The `pad_slipway` module and `tests/test_pad_slipway.c`**: every helper above, no caller yet. Every e2e is unchanged.
2. **The chord cheat.** `cheat.c`'s new matcher, the chord rule in `pad_nav_step`, their unit tests (`tests/test_input_step.c`, `tests/test_economy.c`, `tests/test_main_economy.c` retyped, and the new chord cases in `tests/test_pad_nav.c`). Then the four cheat replays re-authored (only their cheat segments, with their side-effect situations kept by separate presses), and their pinned values (`test_state`'s `pad_research` cut, `test_live_slots` where a frame moves). It lands before × acts in jump mode, so no replay's cheat is ever broken by the new ×.
3. **× in jump mode and slipway mode**, wired in `pad_nav`, with every `tests/test_pad_nav.c` case below, the chain test included. No replay presses × in jump mode after chunk 2, so every e2e should stay unchanged. If one changes, it is re-authored in this commit.
4. **`pad_colonise` re-authored** onto the × route (its slipway rules from f146), with `test_rng_sync`'s windows and `make rng-sync-probe`.
5. **`pad_slipway` re-authored**: f0–98 of the new `pad_colonise` verbatim, the slipway on the × route, the old-drag-only and network situations on the interim path, as listed above.
6. **`pad_research` re-authored**, and `pad_resume_at_start` re-cut if `pad_research`'s last record changed, with `test_state`'s cuts.
7. **`pad_tech_costs` re-authored**, with `test_state`'s and `test_live_slots`' pins.
8. **`pad_planet_projects` re-authored**, with `test_live_slots`' pins. Then the step note: cost, the cheat verdict and route, the situations kept on the interim path, the replaced coverage lines, and the new frame counts.

If a chunk from 4 on breaks a replay an earlier chunk re-authored, that replay is re-authored again in the later chunk's commit, and the step note counts the time.

## Testing Decisions

- **A good test** asserts on public behaviour only. For `pad_slipway`: return values and the public fields of `pad_slipway_t`, on hand-built inputs. For `pad_nav`: drive the pad through `run_frame`, `tap` and `hold`, then assert on `pad_nav_jump_mode`, `pad_nav_hovered`, `pad_nav_slipway_start`, `pad_nav_slipway_target`, the played frame's mouse position and buttons, the camera, `ui_pad_selection`, `ui_left_held`, world counts, built slipways, credits and the game stream's state. Never on `pad_nav_t` internals.
- **`tests/test_pad_slipway.c`** (new; prior art `tests/test_pad_jump.c` and `tests/test_pad_target.c`: bare entities on the stack, lists over the test's own storage, no cart; milliseconds):
  1. `pad_slipway_can_start`: a settled planet and a settled structure start; a known planet, a dead entity, NULL and a non-planet do not; a slipgate that refuses another slipway does not (built through the same fields `slipgate_takes_slipway` reads, or through the cart if that costs less than a few ms).
  2. `pad_slipway_candidates`: the start removed, order kept, the start missing from the list leaves it whole, and the caller's storage is used.
  3. `pad_slipway_between` and `pad_slipway_outcome`: built only when a joining slipway exists and `linked_before` is 0; refused with no slipway, and refused when one existed before.
  4. `pad_slipway_release` and `pad_slipway_rebase`: built moves the start to the target and clears it; refused keeps the start; both leave the phase PRESSED.
- **`tests/test_pad_nav.c`**, new cases (prior art: `settle_planet`, `find_pair` with `near_pair`, `too_far_pair` and `off_screen_pair`, `assert_built_and_landed`, `assert_drop_refused`, `slipways_left_placing`, `world_count` and `assert_world_unchanged`, `full_slipgate_under_centre`, `cover_with_icon`, and `start_game_in_jump_mode`):
  1. **The chain** (the plan's unit test): three settled objects A, B, C placed so that one press of a direction moves from A to B and then to C. × on hovered A, the D-pad to B, ×, the D-pad to C, ×: `slipway A B` and `slipway B C` built, credits charged twice, slipway mode from C after the last re-press (start C, no target), ○ then back in jump mode hovering C.
  2. × on a hovered colony presses its `pos` screen point on that frame and holds the slipway from the next (`ui_left_held` is a slipway); the start is excluded: no D-pad direction ever aims at it.
  3. A refused start holds nothing: × on a full slipgate, on a colony the HUD covers, and on an object still outside the rectangle mid-scroll. No mouse button on that frame or the next 10, mode still jump, world counts unchanged.
  4. × on a hovered known planet and with nothing hovered: nothing pressed, mode jump.
  5. Refused targets (known planet, too far, already linked, not enough credits): nothing built, credits kept, and slipway mode from the same start with no target after the re-press.
  6. ○ in slipway mode: the null drop (`slipways_left_placing` 0), nothing built or charged, jump mode hovering the start, also after jumping to a target.
  7. The D-pad and hold repeat in slipway mode jump as `pad_target_pick` over the candidates minus the start, and the cursor is on the target's `pos` (`assert_cursor_on`). The edge scroll keeps the target in the rectangle at most 8 px per frame, and an off-screen start still builds.
  8. Pause, a tab change and a box opening cancel slipway mode: nothing held, nothing built, jump mode hovering the start.
  9. A start that dies in slipway mode drops the held slipway and returns to jump mode.
  10. △ and □ in slipway mode change nothing (mode, start, target, camera, world counts).
  11. **The chord**: four chords in jump mode hovering a colony fire the cheat once (credits 200, research 99), and leave the mode, the hovered object, the cursor, the camera, world counts and the game stream's state unchanged, with played buttons 0 on every frame. The same chords in slipway mode neither build nor cancel. A chord on a panel presses no row. × held after a chord's ○ is released still acts on nothing until × is released.
  12. **No accidental cheat**: the ordinary sequence × D-pad × ○ ○ × D-pad × ○ ○, played as real slipway building, fires nothing.
  13. The game stream is untouched: a slipway built, refused and cancelled, with jumps and scrolls between them, leaves `p8_fixed_rnd_state()` unchanged (no fold or probe in the sequence).
- **Updated tests**: the old-drag tests (`test_square_on_a_colony_holds_its_slipway_at_the_centre`, `test_cross_on_a_colony_builds_a_slipway_and_lands_on_it` and the others) stay as they are. The interim path is unchanged, and step 7 or 12 removes them with the drag.
- **`cheat_step`** tests: four chords fire; a lone × or ○ between them resets; × and ○ on consecutive frames are not a chord; the old sequence fires nothing.
- **Not tested**: the misuse asserts (the codebase does not exercise asserts).
- **Before each commit**: `make test-all` green; `make end-state` on the 17 replays green with cheat counts matching; `make coverage-compare` with no newly unhit line outside the listed build landing; `make test` and `make e2e` wall times compared before and after the step.

## Out of Scope

- × on a known planet opening its projects menu, the full ○ back order, the root jump to the last object × acted on (step 8).
- Aim mode on □, removing the □ select and the old drag, and □ or ○ from free mode back to jump mode (step 7).
- Select, the L1 build bar, R1 panels, and removing the tab ring and network mode (steps 9–12).
- Re-authoring the `worst-case/` games, including retyping their cheats as chords (step 13).
- △ again closing the planet menu (flagged by step 5's PRD, not claimed by any step).
- Any change to the cart's rules, HUD, panels or menus; audio; PSX; mouse input; tuning the constants.

## Further Notes

- **Why the chain continues automatically.** The plan's unit test, × D-pad × D-pad ×, builds two slipways with three ×. That only works if the second slipway starts without its own ×. "× chains the next slipway from there" is read through that test: the release builds, the next frame presses the target, and the next × builds again.
- **Why a refused drop keeps the start.** The player's likely next move after a refusal is another target from the same start. The old drag did the same (`pad_slipway.txt` f127), so this keeps the familiar behaviour and costs no extra press.
- **For step 7**: the situations listed in the step note as kept on the interim path are exactly those whose mechanic step 7 removes. Step 7's re-authoring drops them under section 2's rule, or produces them the new way if aim mode can.
- **For steps 7–12** (the cheat guardrail): with the chord rule at the top of `pad_nav_step`, the deliberate route holds as long as that rule stays first. Each step's re-check is a run of test case 11 above in its new modes.
- **For step 8**: × on a known planet and ○ at the root are the two presses this step leaves inert. Step 8's re-check of the ordinary-play verdict concerns the chord matcher, which neither press can complete.
- **For step 14**: `pad_nav_slipway_start` and `pad_nav_slipway_target` give the time in slipway mode. A × with no target, or on a target that is not pressable, and a D-pad press into an empty cone are its dead presses. The empty cone falls in the first group of step 14.
