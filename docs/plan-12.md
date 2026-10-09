# Plan 12 — Jump-cursor pad controls

## 1. Goal

A pad control scheme where reaching what you want on the map takes a few presses, not pixel-precise camera panning. The rule the player learns: **point at a thing, × acts on it, ○ backs out.**

The current scheme (`src/game/pad_nav.c`) was measured on a 30-minute human game (110,530 frames, 14 years, 30 colonies, 42 slipways), replayed headlessly with a per-frame trace of the `pad_nav_t` state. The numbers below justify every rule in section 2, and are the baseline that section 4 compares against:

- **Aiming costs the most.** 61% of game time is spent in the free-camera modes. There were 1,066 direction presses there and 35,600 px of camera travel. A third of the holds lasted 10 frames or less (1 px per frame), and 12% were followed by the opposite direction within 40 frames (overshoot). △ and □ act only on the object exactly under the centre crosshair.
- **Network mode is not spatial.** Up/Down were pressed 47 times in network mode and 41 of those did nothing (only Left/Right are mapped, and they cycle slipways clockwise). 15 Left/Right presses did nothing because the planet had no slipway. 12 ○ presses did nothing.
- **The planet menu has no obvious way out.** ○ (the only close) was never pressed in it. □ was pressed 11 times and △ 6 times without effect. The player uses △ as a toggle.
- **One button means too many things.** □ selects, starts a slipway, cancels it, or leaves the planet, depending on 6 map sub-modes. Network mode and slipway building alternated 153 times. About 100 slipways were started, 42 built and about 35 abandoned.
- **The tab ring is slow.** Map, build, end turn, tech and score sit on a ring of 5: bursts of 5–7 shoulder presses in a row. End turn was used twice in the game.
- **Bug:** □ on a slipgate that refuses new slipways gives no feedback. `begin_drag` then presses every other frame and the press falls through to a lower layer (gdb: `slipgate_left_press` returns `stop = 0`), for as long as the player stays there: about 25 s in that game.

## 2. Rules

### Guardrails

- `make test-all` passes on every commit. `make test` and `make e2e` must not get slower (`CLAUDE.md`).
- The game rules don't change: only how pad input becomes mouse presses and cursor positions in `played` (the `pad_nav_step` contract), plus the cursor's drawing. Audio is out of scope.
- **The e2e replays change.** All 17 `tests/e2e/pad_*.json` drive the pad, so a step that changes a mapping re-authors the replays it breaks, in the same commit. A re-authored replay must reach the same game end state as before, judged by the end-state check of step 2a against the reference commit, not by eye. Then its `.json.sha256` is re-baselined and its `.txt` rewritten from the new frames. Never re-baseline a replay whose game end state changed.
- **The plan never waits for a human** (decided by the user). Every point where the plan could block has a locked fallback in this plan; any other blocking question goes to the `guideline` skill.
- If, in any step, a re-authored replay cannot reach its reference end state with the step 2b discipline, the step does not stop. That replay is judged by the relaxed comparison step 2b defines (the fields that do not depend on revealed planet types), and it must still produce every situation its old `.txt` lists. The replay, the first differing field and what was tried go in the step notes, and the replay is listed in a committed exceptions file (replay, reason, first differing field) that the step 2a target reads: it applies the relaxed comparison only to listed replays, and fails when a listed replay would pass the strict one. Step 5 re-authors the `pad_probe` → `pad_colonise` prefix first, as the early check.
- **Code coverage never drops** (decided by the user). Before step 1b, `make coverage` over the e2e replays gives the baseline (`tmp/coverage/lines.txt`, reproducible from that commit). After every step that re-authors replays, every line hit by the baseline is still hit, except lines the step deletes (old pad_nav code) or that the plan replaces. A newly unhit line means a replay must be re-authored until it is hit again. This is the check that a replay whose galaxy changed (planet A became B) still covers what it covered. The baseline commit is `6346485`; a committed script in `tools/` (outside `make test-all`, added in step 2b) maps its hit lines to the current tree through `git diff` and prints the lines still hit, unhit and deleted.
- Every step that re-authors replays records in its step note the number of replays it broke and the agent time spent re-authoring them (step 1b's chunk 2: 8 re-authored replays, about 85 minutes). If step 5's re-authoring costs more than twice that, step 5b is inserted before step 6: a devtools generator, outside `make test` and `make e2e`, that compiles a small committed source per replay (`idle N`, `tap B`, `hold B N`, `include <replay>`, `jump-to <id>` through the step 4 picker) into the per-frame JSON, which stays the checked artifact; the replays steps 6 to 13 re-author are then written through it.
- A re-authored replay keeps every situation its old `.txt` lists (refusals, cancels, red lines, too far, duplicates and so on), produced the new scheme's way. Only situations whose mechanic the plan removes (network travel, the tab ring) may go, each noted in the step note. Checked by comparing the old and new `.txt` in the same commit.
- A re-authored replay stays within the e2e frame limit (180 frames, or 600 if listed in `tools/e2e_long_exceptions.txt`), and `make e2e` must not get slower: a mapping that costs frames is paid for inside the replay, never by raising the limit.
- A step re-authors every replay it breaks, not only the ones it names: the per-step lists are the expected minimum. Replays share verbatim prefixes (pad_probe → pad_colonise → pad_slipway, and other chains), so re-author the prefix first and reuse it.
- New inputs must not type the cheat sequence (× × ○ ○ × × ○ ○, see `src/game/cheat.c`) by accident. Checked by the end-state check of step 2a: each replay's cheat activation count must match its reference (some replays use the cheat on purpose).
- Some tests pin frames, camera or screen positions, or counts inside specific replays (for example `tests/test_live_slots.c` on `pad_pause_restart`, `tests/test_state.c` on `pad_tech_costs` and `pad_research`, and `make rng-sync-probe` on `pad_colonise`). A step that re-authors a replay re-derives, in the same commit, the pinned values of every test on step 2a's inventory. Each assertion keeps its intent (the same event, at its new frame), and none is weakened or removed. `make rng-sync-probe` is re-run whenever `pad_colonise` changes.
- Every step from 6 on that changes what × or ○ do (including the build bar and the ring's removal) checks whether ordinary input sequences, with D-pad presses between them (`cheat_step` ignores them), can complete × × ○ ○ × × ○ ○, and re-checks the deliberate cheat route of step 6. If ordinary play can complete it, or no side-effect-free deliberate route exists, change the cheat sequence in `src/game/cheat.c` (a dev tool, not a game rule) to one ordinary play cannot type and that has such a route, and update the replays that type it, in the same step. Since step 6 the chord is defined twice, in `cheat_step` (`cheat.c`) and in `pad_nav.c`'s `without_chord`, which hides × and ○ from every mode: a cheat change edits both in one commit (or single-sources the chord first), and a unit test in `make test` drives one button sequence through both and asserts the pad hides × and ○ on exactly the frames `cheat_step` counts as chords.
- Checks outside `make test-all` run on every chunk that touches pad input or a replay: `make unit-run-test_live_slots`, `make end-state EVENTS=` on every touched replay, `make coverage-compare`, and `make rng-sync-probe` whenever `pad_colonise` changes (a test outside the suite went red unseen for a chunk in step 7).
- From step 8 on, each new mode's state lives in its own `pad_<mode>_t`, never in `pad_jump_t` nor as loose `pad_nav_t` fields, unless the step note gives the reason; each step note records `pad_nav.c`'s line count and `pad_nav_t`'s field count before and after. A unit test may be deleted only with the mechanic it exercises, which the step removes; the step note lists each deleted test next to that mechanic, and a test of a surviving behaviour that no longer compiles is rewritten, not deleted.
- Each e2e `.txt` states its replay's length as "N frames.", and `tools/e2e_limits.sh` (in `make e2e`) checks it against the `.json` (added in step 8).
- The `worst-case/` full games (scripted pad input, feeding `make state` peaks, `make full-game`, `make save-worst-case` and `make save-fuzz`) are knowingly stale from step 5 until step 13 re-authors them: their green checks mean nothing in between.
- One scheme only: no option or flag keeps the old scheme alongside the new one (two schemes would double the e2e and the code).
- Never launch the raylib app. A human judges the feel after the plan is done; no step waits for it.

### Locked decisions

**Cursor and camera**
- On the map, the cursor always sits on an object: a known planet, a colony, or a structure (planet-family entities in states KNOWN or SETTLED). Unexplored planets are not drawn, so they are never targets. Wormholes and probes are not targets.
- D-pad press: jump to the best target in that direction, searched over the whole map, not just the screen. The candidates lie in the 90° cone around the direction (|perpendicular| ≤ along). Score = along + 2 × |perpendicular|, lowest wins (the weight is tunable in the step). With no candidate, nothing moves, and there is no wrap-around.
- D-pad hold: repeat the jump after 18 frames, then every 8 frames (tunable).
- **The camera does not re-centre on every jump** (decided by the user). The cursor moves on screen. The camera scrolls only when the target leaves an inner rectangle: a 20 px margin, more at the bottom so the HUD bar never covers the target. It scrolls the minimum distance, at most 8 px per frame. Rejected: re-centring on every jump, which moves the whole screen at each press.
- The hovered object gets a visible marker (reuse the pad selection blink, `ui_set_pad_selection`).

**Buttons**
- **×** acts on the hovered object:
  - On a colony or structure: start a slipway from it.
  - On a known planet: open its projects (colonize) menu, as a mouse press there does.
  - If the press is refused (for example a full slipgate), nothing is held and no press is retried or passed on: it fixes the bug above by design.
- **Building a slipway:** the D-pad jumps between targets (same picker, start excluded) while the left button stays held on the target, so the game draws its own green or red line. × releases on the target: built or refused by the game's rules. When built, the cursor stays on the target, so × chains the next slipway from there. ○ cancels through the ui's own drop with no target (as `cancel_drag` does today).
- **△:** the planet menu of the hovered object. △ again, or ○, closes it. Inside menus, the D-pad and × work as today (`pad_panel_step`).
- **○ always goes back one step**, in this order: close the box or panel; cancel the slipway or the placement; leave aim mode; at the root, jump to the last object × acted on (nothing at game start).
- **□ toggles aim mode:** a free crosshair at the screen centre with the camera panned under it, the way free mode pans today (1/2/4 px per frame). × presses the background as today: probe, or fold once a probe is out; hold × to fold. □ or ○ goes back to jump mode, on the target nearest the centre. This is the only pixel aiming left.
- **L1** toggles the build bar: Left/Right pick a structure, × takes it, ○ or L1 closes the bar. Placing a structure uses the aim-mode crosshair (structures go in empty space): × places, ○ cancels.
- **R1** cycles tech panel → score panel → closed.
- **Select** (new button): a tap cycles through colonies needing attention. A hold of 30 frames ends the year: end turn was used twice in the measured game, and the hold prevents accidents.
- **Start:** pause, as today.
- The tab ring (`pad_nav_tab_t`, map/build/end turn/tech/score) and network mode disappear. The HUD highlight (`ui_set_pad_tab`) marks the button of whichever panel is open.

### Rejected

- **A modifier button (L2 + D-pad) for free aiming:** it needs a chord and a new button. □ as a toggle uses buttons that already exist.
- **Launching probes from a colony and steering them:** probes target a point in space, so a crosshair is needed anyway.
- **Snapping to exact pixel offsets for structure placement:** placement needs empty space, so it uses the aim crosshair.

### Out of scope

- PSX build work; mouse input (removed earlier); audio.
- Any change to what the HUD buttons, panels or menus contain.

## 3. Phases and steps

### Phase 1 — Groundwork

1. skill: fix-bug — In the current scheme, □ on a slipgate that refuses new slipways pulses a mouse press every other frame (`begin_drag` retries; `slipgate_left_press` returns `stop = 0` and the press falls through). Expected: the drag gives up until the starting planet changes, and no press reaches a lower layer. Reproduce with a short pad e2e that fills a slipgate's capacity, then presses □ on it.
1b. **Separate random streams** (decided by the user). Visual-only effects (particles in `particle.c` and the probe emitter, transit dots in `slipway_display.c`, `planet_display.c`, `background.c`) draw from their own random stream, so camera path and timing no longer change game decisions (planet types in `gen.c`, `galaxy.c`, `planet_family.c`). The save keeps working: update `save_state.c` and `docs/save-format.md` if the visual stream needs saving. Every e2e changes once, and probes may reveal different planets. Each replay is replayed and compared with its `.txt`: if it still produces every situation listed there, re-baseline its `.json.sha256` and update the `.txt` readings; if not (for example it colonises a planet whose type changed), re-author its inputs until it does, under the situation guardrail of section 2. Shared prefixes are fixed first. All of this happens in this step, and rerun `make full-game`, `make save-worst-case` and `make save-fuzz`. Verified by a unit test: the same game-stream draws with and without extra visual draws in between. And by `make coverage`: no baseline line left unhit.
2a. **End-state check.** Make the guardrail of section 2 mechanical before any mapping changes. The reference commit is the one this step lands on: every replay's state at that commit is the baseline, reproducible from git, not from the gitignored `tmp/state/`. Fix which `make state` lines and fields are compared: the game ones, such as difficulty, planets' state, type and world position, slipways, structures and counts. Fix which are ignored: camera, screen and visible fields, zone and cap peaks, and the clock and resources fields that move with frame timing (a re-authored replay takes a different number of frames). It also reports the number of cheat activations, which must match too. The reference covers the e2e replays and the `worst-case/` games. One `make` target, outside `make test` and `make e2e`, diffs a replay's end state against the reference and fails on any compared difference. Verified by running it on every replay at the reference commit (all pass), and on one replay deliberately cut short (it fails). Record in the step notes an inventory of every test and tool that loads a `tests/e2e/*.json` or a `worst-case/` game and pins values in it.
2b. **Feasibility, before any mapping changes.** Before step 1b, on-screen-only effects shared the game's random stream, so adding 5 idle frames to `pad_probe` changed a revealed planet from ocean to forgeworld. Check that step 1b removed that (the same edit no longer changes it), and whether frame timing alone still moves game decisions. On the `pad_probe` → `pad_colonise` prefix, show whether the reference end state can still be reached with a different camera path and timing, and what discipline that takes. Record the result in the step notes. If the reference cannot be reached, define the relaxed comparison the fallback of section 2 uses: the step 2a fields minus revealed planet types and what derives from them, keeping counts of colonies, slipways, structures, techs and years. Extend the verdict to the `worst-case/` games: decide whether they are held to the end-state check or only to the peak rule they were generated under (every `make state` peak matched or exceeded, commit 8c4bbf4). Commit the `make state` peak table of 8c4bbf4 as a file, the target step 13 is measured against. Commit the coverage comparison script of section 2's coverage guardrail, and, if a relaxed comparison is defined, the exceptions file and its support in the step 2a target. Also budget the 30-frame Select hold now: count the years each replay ends (`pad_game_end` 25 years in 104 frames, `pad_message_box` 20 in 190), and record the allowed route each year-heavy replay will take within the frame-limit guardrail The fallback, when holding does not fit, is to start that replay from an `initial_save` placed just before its year-heavy part, as `pad_resume_at_start` does. Phase 2 starts right after, whatever the verdict.
3. **Select button.** Add `P8_PAD_SELECT` to `src/p8/input.h`, `"select"` to the name tables in `src/devtools/input_json.c` and `src/platform/keymap.c`, and a `select =` line to `keymap.cfg` (keyboard key plus `MIDDLE_LEFT`). Verified by `tests/test_keymap.c` and `tests/test_replay_source.c` reading the new name. Every e2e is unchanged (no replay presses it yet).
4. **Target picker.** A pure function: given an origin, a direction and the live candidate set, it returns the best target or NULL, following the cone and score rule of section 2. Unit-tested on hand-built layouts: one candidate per direction, a tie broken by score, nothing in the cone, origin excluded. No behaviour change; e2e unchanged.

### Phase 2 — Jump mode on the map

5. **Jump mode replaces network mode and plain free mode as the map's root:** D-pad jumps with hold repeat, the hovered marker, the edge scroll and △ on the hovered object. At game start the cursor is on the starting colony, or on the target nearest the screen centre when there is none. Until steps 6 and 7 land, □ from jump mode still enters today's free mode, with its probe/fold, its □ select and its drag, so every replay keeps a path to its old end state. Network mode stops being the root but stays reachable exactly as today through that free mode's □ select (the drag needs a starting planet, which only network mode's □ sets), until step 7 removes the □ select and the drag; step 12 deletes the code. Every slipway-building replay reaches its reference end state through that interim path. The affected e2e are re-authored under the guardrails of section 2. Method: the jump cursor's state, hold-repeat timing and edge-scroll computation go in a new `src/game/pad_jump.{c,h}`, pure helpers in the style of `pad_target`, which `pad_nav.c` only calls; steps 6 to 11 follow the same pattern for each new mode where it fits, so step 12 deletes old functions without touching new code. Before any replay is re-authored, a static check in `make test` (milliseconds, modelled on `tools/float_grep.sh` with an exceptions file) allows game-stream draws (`p8_fixed_rnd*`) only from the current game-stream sites (galaxy, gen, the background tile map, the reveal roll, and `help.c`'s title seed tick named with its reason) and visual-stream draws only from the display files step 1b moved: step 2b's finding that reveals are the only game draws after the title then fails loudly instead of as an unreachable end state. Verified by unit tests in `tests/test_pad_jump.c` and `tests/test_pad_nav.c` (jump, repeat timing, edge scroll keeps the target inside the rectangle, and N frames of jump, repeat and edge scroll leave the game stream's state unchanged) and by the end-state check of step 2a.
6. **Slipways from ×:** × on a colony starts the slipway, the D-pad jumps between targets with the button held, × builds and keeps the cursor on the target, ○ cancels. A refused start holds nothing. `pad_slipway` and the replays that build slipways re-authored. Unit test: building a chain of two slipways with × D-pad × D-pad ×. Also find and record one deliberate cheat route (10 replays type the cheat on purpose): a place and mode where the 8 presses leave nav, cursor and game state unchanged. If none exists, apply the cheat fallback of section 2.
7. **Aim mode on □** (today's free-mode panning and its × probe/fold behaviour, without its □ select and its drag). □ or ○ returns to jump mode on the target nearest the centre. `pad_probe` and any fold replay re-authored. Verified as in step 5.

### Phase 3 — × and ○ everywhere

8. **× on a known planet** opens its projects menu. **○ back order** as in section 2, including the root jump to the last object × acted on. Unit tests: one per rung of the ○ order, and ○ at game start does nothing. Affected replays (`pad_colonise`, `pad_planet_projects`, `pad_empty_planet`) re-authored. Method: the step's first commit is a pure refactor, before any mapping change: `pad_jump_candidates` and `pad_slipway_candidates` stop writing the caller's storage address into `*out` and use the engine's `entity_list_filter` (as `probe.c` does), `JUMP_CANDIDATES_CAP` stops depending on a number in `cart.c`, and `tools/pointer_guard.sh` (or a millisecond sibling in `make test`) also counts local `entity_t *name[...]` arrays in `src/game/`; checked by every `*.sha256` and `.end-state` unchanged, `make state` peaks within their margins and no new unhit line. The same commit series adds the `.txt` frame-count check of section 2. The last object × acted on does not go in `pad_jump_t`.

### Phase 4 — Direct buttons instead of the tab ring

9. **Select:** a tap cycles colonies needing attention, a 30-frame hold ends the year. Read the code for what the game already tracks as an unmet need. If it tracks nothing usable, cycle every colony in creation order and note it in step notes. `pad_end_turn` re-authored. Unit tests: tap order, a hold under 30 frames ends nothing. The year-heavy replays follow the route step 2b recorded.
10. **L1 build bar** and placement with the aim crosshair (× places, ○ cancels). `pad_build` re-authored. Until step 12 deletes it, the tab ring stays reachable on the shoulder button this step leaves free, so replays not yet re-authored keep their path.
11. **R1 cycles tech → score → closed.** `pad_research`, `pad_tech_costs` and `pad_score` re-authored. From this step every ring tab has a direct button (map by ○, build on L1, end turn on the Select hold, tech and score on R1), so the ring is no longer needed to reach any old end state.
12. **Remove the tab ring and network mode:** `pad_nav_tab_t` and the network fields go, and the HUD highlight follows the open panel. Remaining replays (`pad_title`, `pad_pause_restart`, `pad_message_box`, `pad_bankrupt`, `pad_game_end`, `pad_restart_resume`, `pad_resume_at_start`) re-authored where they broke. `tests/test_pad_nav.c` keeps only tests of the new scheme, under section 2's test-deletion rule. After the deletion, separate pure-refactor commits (every hash and `.end-state` unchanged) fix phase 2's leftover findings: `start_new_game` and `jump_ran` misnamed, `pad_jump_t` holding state only `pad_nav.c` uses, `press_start_again` duplicating `cross_starts_slipway`'s press, and `pad_jump.c`'s direction table duplicating `pan_directions`; the step note lists them.
13. **Re-author the `worst-case/` full games** for the final mapping, judged by the acceptance rule step 2b recorded for these games. Their step 2a references were taken after step 1b collapsed them (full_game 27 colonies → 2), so they are not this step's target: this step re-baselines those five references on purpose, the one named exception to the no-re-baseline guardrail. Verified by `make full-game`, `make save-worst-case` and `make save-fuzz` passing and, at minimum, by every `make state` peak matching or exceeding the 8c4bbf4 peak table committed in step 2b.

### Phase 5 — Finish

14. **Pad metrics tool.** A headless devtools target, outside `make test-all`, that replays an events file and reports the share of time in each mode of the new scheme (jump, aim, slipway, menus, placement) and the dead presses per mode (presses that change no nav state, cursor, camera or game state). It is the measure for section 4. Dead presses are reported in two groups: those the locked rules require (○ at the root with no last object or with the cursor already on it, a D-pad press or hold repeat into an empty cone) and all others. Hold repeats are reported apart from presses. Verified on one re-authored e2e whose mode split and dead presses are known from its `.txt`, including at least one designed no-op landing in the first group.
15. **README:** the Controls section rewritten for the new scheme (table plus one line per mode: jump, aim, slipway, menus).
16. skill: rework-comments — `src/game/pad_nav.c`, `src/game/pad_nav.h` and any new pad file.

## 4. Done when

- `make test-all` passes, and every e2e passes the end-state check of step 2a against the reference commit.
- The README Controls section describes only the new scheme.
- Measured with the step 14 tool on the re-authored `worst-case/` games, against the baseline of section 1: zero cheat activations, no dead D-pad press in jump mode and no dead ○ anywhere outside the group the locked rules require, and less than 25% of game time in aim mode.
- After the plan, outside it: the human plays a game and records it. The same tool measures that recording, and the human's complaints become fix-bug runs or a new plan.

## 5. Open questions

- The tuning values (score weight 2, repeat 18/8 frames, margin 20 px, 8 px per frame scroll) are first guesses, kept as written for the whole plan. Tuning them comes after the human's game, as a mapping change under the re-authoring guardrails of section 2.
- "Colonies needing attention" depends on what the game tracks (step 9 decides, with a fallback).

## 6. Step notes

### Step 1
- Fixed by `6059d8e` (fix-bug, unattended). Cause proven under gdb: `begin_drag` pressed on any settled start, `slipgate_left_press` refused, and the press fell through to the background, which made a wormhole (or a probe) that died on the next frame's release.
- Fix: new `slipgate_takes_slipway` (`planet_family_trade.c`), used by `slipgate_accept_drop` and by a guard in `begin_drag`, so no press goes out to a full slipgate or infraspace gate. The `network_step` □ assert accepts that refusal.
- Deviation: the plan asked for a short pad e2e as the reproduction. The proof used a throwaway replay in `tmp/`, and the regression test is a unit test (`test_a_full_slipgate_as_starting_planet_presses_nothing` in `tests/test_pad_nav.c`, 8 ms), per check-e2e's rule that a bug's test is a unit test, never a new e2e. No e2e was added or changed: there are still 17 replays.

### Step 1b
- `938e864`: `p8_fixed_rnd_from_state` (the game stream rebuilt on it, bit for bit) and the `visual_random` module. `5a5520b`: the emitter spawn loop, the probe and burst particles, the transit-dot roll and the vanishing flicker draw from the visual stream. Save unchanged (version 1, visual stream not saved).
- Deviations from the step text, both decided by the PRD: the background's tile map stays on the game stream (drawn once per game, before the galaxy; moving it would change what `background_seed` means), and the title takes exactly one game draw per drawn frame, so a new game's galaxy still depends on when the title's X lands, not on particle counts.
- E2e: re-baselined only `pad_title`, `pad_score`, `pad_message_box`, `pad_game_end`, `pad_build`, `pad_end_turn`, `pad_pause_restart`, `pad_restart_resume`. Re-authored `pad_probe` (first difference f68: the fold revealed one planet), `pad_colonise`, `pad_slipway` (401 → 348 frames), `pad_empty_planet`, `pad_bankrupt`, `pad_planet_projects`, `pad_research` (title X f12 → f24), `pad_tech_costs` (title X f12 → f16). `pad_resume_at_start` re-cut from `pad_research`'s new last record. No situation lost, cheat counts unchanged (bankrupt 1, planet_projects 3, research 1, tech_costs 1). Some pixel readings below situation level changed (for example `pad_slipway`'s empty-space drag no longer ends on the centre pixel).
- Pinned values re-derived: `test_state` (the `pad_research` cheat cut is 45 frames, was 33), `test_live_slots` and its 4 fixtures, `test_rng_sync` (now asserts that run A's visual stream moves over the window), `test_save_load`, `test_save_fuzz`, golden frame hashes.
- Long checks pass: `make full-game`, `make save-worst-case` (largest record 3973 B), `make save-fuzz`, `make rng-sync-probe`. Coverage at `6346485`: 7608 lines hit. Now 7613: every baseline line is still hit, the 10 rewritten draw-site lines in their new form.
- `guideline` decision: the `worst-case/` games are not re-authored before step 13. They run clean but collapsed: full_game 3426 with 27 colonies → 3421 with 2; full_game_pause_idle 27 → 2 colonies; full_game_new_game 0 → 2. Their `make state` peaks dropped (cap row 29 → 8, zone game 119816 → 84136, transit_dots 48 → 25). The e2e replays now hold most peaks. Step 2b chooses how they are judged: the 8c4bbf4 peak rule, not their step 2a end state.

### Step 2a
- `b19f302`: the replay tool appends `cheat activations <n>` to its state file. `tools/end_state.sh` normalizes, checks and baselines. `make end-state` (all 22, or `EVENTS=`, with optional `REFERENCE=`) and `make end-state-baseline` (writes only missing references) sit outside `make test-all`. `tests/test_end_state.c` takes 33 ms.
- **Reference commit `8defcba`:** one `<events>.end-state` per replay, 17 in `tests/e2e/` and 5 in `worst-case/`. Re-baselining after deleting them reproduces them byte for byte, and all 22 pass. `pad_research` cut to 45 frames fails with its cheat count matching. `pad_research` with 60 idle frames appended passes.
- **Field table.**
  - Kept: `clock`, `difficulty`, `resources` (credits, science, income), `techs`, `slipway` (direction as printed), `count`, `cheat activations`. From `planet` lines, everything through `world x y`. From `structure` lines, id, kind, world and links. World positions are compared exactly.
  - Dropped: `camera`, the `screen` and `visible` fields, and `zone`, `budget` and `cap` lines.
  - A `stopped` line or an unknown line kind fails the check. The normalized text is sorted in C order.
- **Measurement and deviations.** 24 timing variants of 6 replays (30 idle frames inserted at idle points, or 60 appended):
  - No clock or resources field moved, so they are compared rather than ignored.
  - Deviation: settled structures and trading hubs bob (`planet_bob`), so their world y drifted by up to 3 px with idle frames. The state writer now prints their y at rest, recomputed exactly from `state_time` (`planet_bob_total`, `planet_family_bobs`). This changes `make state` output for those lines. Game behaviour is unchanged.
  - Known limit: the rest y assumes the bob ran on every frame since the planet settled.
  - One variant failed: idle frames inserted on the title screen (`pad_build` f7) gave a different galaxy, because title timing still seeds the game (step 1b's seed tick). Step 2b studies it.
- **Cheat activations:** `pad_bankrupt` 1, `pad_planet_projects` 3, `pad_research` 1, `pad_tech_costs` 1, the other e2e 0. `full_game` 30, `full_game_new_game` 30, `full_game_pause_idle` 24, `full_game_pause_restart` 24, `title_idle` 0.
- **Inventory of tests and tools that load a replay and pin values in it:**
  - `tests/test_live_slots.c`: every e2e.
    - Pinned frames: `pad_pause_restart` f1 and f25–39; `pad_game_end` f21 and f69; `pad_tech_costs` f546, f555, f565 and f570, with `p54`; `pad_message_box` f115–121; `pad_empty_planet` f98 and f133; `pad_planet_projects` f171, f172, f255 and f256.
    - Its 4 fixtures in `tests/fixtures/` are prefixes cut from replays.
  - `tests/test_state.c`:
    - `pad_tech_costs`: the counts, the `p31` and `p2` lines, slipways `p54 p31` and `s2 p31`, camera `127 234`, the save summary, and cheat 1.
    - `pad_research` cut at 45 frames: credits 200, science 99, cheat 1.
    - `pad_build` plus 60 idle frames: `s1` is a settled processor and its world line is unchanged.
  - `tests/test_rng_sync.c` (`make rng-sync-probe`): `pad_colonise`. The burst case runs f98–198 with the harvest on `p12`; the quiet case runs f280–344.
  - `tests/test_exit_queue.c`: the constant `FULL_GAME_LIVE_ENTITIES 130`, the largest worst-case reset burst with margin. It is not loaded.
  - The save-load, save-fuzz and resume-check tests: the wormhole seed `0x5EED2026` and the probe at 90, 50.
  - `make full-game`, `make save-worst-case` and `make save-fuzz`: the `worst-case/` games. They require clean runs, the 8 KiB limit and round trips.
  - `make state` and its peaks: every replay, with the 1/8 margin.
  - `make e2e` and its tooling, plus `tools/e2e_long_exceptions.txt`: the frame counts in its reasons.
  - `make sanitize-net` and `make coverage`: every e2e.
  - The 22 `.end-state` references, every e2e `.txt`, and the `pad_resume_at_start` fixture, cut from `pad_research`'s last record.
  - Comments only: `tests/test_layout_nil_item_draws_nothing.c` and the cart.

### Step 2b
- **Committed:** `8f17c25`, `make coverage-compare` and `tools/coverage_compare.sh`. It maps the hit lines of baseline `6346485` through `git diff -U0 -M --histogram`; Myers paired the wrong braces. First run: 7596 hit, 0 unhit, 12 deleted (step 1b's draw sites and step 2a's `bob_delta`). `0bd59c7`, `worst-case/peaks-8c4bbf4.txt`, with its rebuild command in the commit message.
- **Verdict: reachable.** Measured on `pad_probe` → `pad_colonise`.
  - Pass: the old 5-idle-frame edit, idle frames inserted anywhere after the title X, pan holds split up, other camera paths (x first, y first, overshoot, 1 px taps, fast pans), and combinations up to 680 frames.
  - Fail, as controls: title X one frame later, idle frames before the title X, a reversed slipway, and fold points moved by ≥ 2.24 px. A fold moved that far keeps the revealed set but flips the reveal order, so the types swap.
  - After the title, the only game-stream draws are reveals (`planet_roll_type`). Year ends, structures and toasts draw nothing.
  - The end-state check does not see a dropped situation: a `pad_colonise` without its refusals still passes. The `.txt` guardrail stays necessary.
- **Re-authoring discipline:**
  1. The title X lands on the reference frame; extra frames go after it. In a resumed game, title timing is free.
  2. Every probe and fold is confirmed at the reference world point.
  3. Reveal order is kept, across and within folds.
  4. Reveals are the only game draws after the title.
  5. Actions keep their number and costs; their timing is free.
  6. A slipway keeps its direction, and a structure keeps its world point. A press on a bobbing entity aims at least 4 px inside its hit area.
  7. The cheat count is kept.
  8. After a camera snap onto a structure or trading hub (`centre_camera_on` uses `pos`, bob included), re-pan to the reference camera before the next point action. Evidence: `pad_research` with idle frames inserted at f244 places s2 2 px off and loses 2 slipways. Jump mode's edge scroll in steps 5–7 risks the same.
- **Bob:** no game rule reads it (lengths, crossings and placement use `map_pos`). The rest-y fix holds across resumes: 214 idle-appended `pad_resume_at_start` variants pass, and so do 15 `pad_bankrupt` cut-and-resume variants.
- **Relaxed comparison**, defined but not committed, since the verdict is reachable.
  - Kept: difficulty, year, colony, slipway, structure and tech counts, one kind line per `s` id, and the cheat count.
  - Dropped: month, resources, every other planet line, slipways, techs, and planet and node counts.
  - The first step that needs it adds the exceptions file in `tools/` (`<events> <first differing field>: <reason>`) and its support in `tools/end_state.sh`.
- **Worst-case verdict:** the 5 `worst-case/` games are judged by the 8c4bbf4 peak rule: every zone and cap peak must match or exceed `worst-case/peaks-8c4bbf4.txt`. Their step 2a references stay, to catch accidental changes, and step 13 re-baselines them on purpose.
- **Select-hold budget:** 31 frames per year. A route fits when it needs ≤ 90% of the limit.

  | Replay | Years | Frames with holds | Limit | Route |
  |---|---|---|---|---|
  | `pad_colonise` | 1 | 374 | 600 | hold |
  | `pad_end_turn` | 2 | 130 | 180 | hold |
  | `pad_planet_projects` | 1 | 522 | 600 | `initial_save` cut at f86; 2 extra X taps after the resume keep the cheat count at 3 |
  | `pad_message_box` | 20 | 181 | 600 | `initial_save` cut at f58 |
  | `pad_game_end` | 25 | 136 | 180 | `initial_save` cut at f64; passes strictly at f80 |
  | `pad_tech_costs` | 7 | 564 | 600 | `initial_save` cut at f49; fits the limit but not 540 |
  | `pad_research` | 1 | 606 | 600 | none: no record exists before its cheat (f30–44) |
  | `pad_bankrupt` | 8 | 330 at best | 180 | none: the cut must precede the cheat (f96); 7 years × 5 toasts inside 50 frames can't fit at 31 frames a year |

  - **Open for step 9 (`guideline` decides):**
    - `pad_research` fits only by dropping the tab presses (8 frames) and compressing idle frames (≤ 63).
    - `pad_tech_costs` fits only up to 564, against the 540 target.
    - `pad_bankrupt` has no allowed route.
- **`initial_save` traps:**
  - A cheat typed before the cut is lost.
  - A cut between the cheat and the next record loses the cheat's money.
  - No record is written while a wormhole lives.
  - There is no record before the first record-writing action.
  - The resume title's X moves the cheat matcher.
  - The resumed game starts on `map`.
  - The title lines must stay covered by other replays.
- Re-authoring cost: no replay was re-authored in this step.

### Step 3
- Select is bit 2048, `select` in `src/devtools/input_json.c` and `src/platform/keymap.c` (`KEYMAP_BUTTON_COUNT` 12). `keymap.cfg` and the built-in default get `select = RIGHT_SHIFT, MIDDLE_LEFT`. The plan named no keyboard key: Right Shift is free and sits in the same place on QWERTY and AZERTY.
- A player's own older `keymap.cfg` without a `select` line is refused at launch, naming `select`, as for any missing button.
- The unknown-button fixture and keymap test used `select` as their unknown name. They now use `l2`.
- `test_select_changes_nothing_yet` (`tests/test_pad_nav.c`) shows the bit is ignored. Every e2e and end-state reference is unchanged. `make raylib` links.
- Re-authoring cost: none.

### Step 4
- `src/game/pad_target.{c,h}`: `pad_target_pick(origin, direction, candidates)` and `pad_target_is_candidate(e)`, with `PAD_TARGET_PERPENDICULAR_WEIGHT` 2. The caller builds the candidate list; the pick does not filter it, so step 6 can drop the slipway start.
- Positions are `map_pos`, never `pos`: `pos` bobs (step 2b) and would make a jump depend on the frame of the press. Presses must still aim at `pos`, which hit tests read.
- Added beyond the plan: at equal score, the smaller |perpendicular| wins, then the earlier entry in the list.
- `tests/test_pad_target.c`: 9 hand-built layouts, 2 ms. A verifier's mutation run found that weight 3 survived. One layout was tightened, and weights 0, 1, 3 and 4 are now caught.
- No caller yet. E2e and end-state unchanged. Re-authoring cost: none.

### Step 5
- **Commits:**
  - `97f3814`: random-stream guard. `tools/random_guard.sh` and `tools/random_guard_exceptions.txt`, run by `make test`. It counts game-stream, generator and visual-stream uses per file. It also lists `save_load.c`, which restores the stream.
  - `5fa102e`: `src/game/pad_jump.{c,h}`.
    - Repeat timing: 18, then every 8.
    - Inner rectangle: x 20–139, y 20–88. The bottom margin is 31, over the HUD row at y 109.
    - Edge scroll: at most 8 px per axis per frame.
    - Also: world point from `map_pos`, nearest, seed, candidates.
  - `162af4d`: jump root.
  - `4a1dca9`: D-pad jumps and hold repeat. A unit test shows jumps, repeats and scrolls leave the game stream unchanged.
  - `875cd0a`: △ on the hovered object.
  - `6e841dc`: ○ in network mode returns to jump mode.
- **Behaviour:**
  - At game start the cursor hovers the first colony, else the known object nearest the centre, else nothing until one appears. A new game is detected when the main economy id changes.
  - Candidates come from `TAG_NODE`: gates carry no `TAG_PLANET`.
  - □ from jump mode enters today's plain free mode with the camera unmoved.
  - × and ○ do nothing in jump mode.
  - △ presses only once the object's rest point is inside the rectangle and no HUD item covers it.
- **`guideline` decisions:**
  - ○ in network mode returning to jump mode is in scope: it gives a dead press a job and is free mode's only way back before step 7.
  - "△ again closes the planet menu" is left to step 8, with the ○ "close the box or panel" rung.
- **Re-authoring:**
  - Chunk 3: all 17 replays and 5 live-slots fixtures broke. Each replaced one idle frame (two after a new game or resume) with □ on its first map frame. Every capture stayed byte-identical: no hash, `.txt` reading or pinned value changed. About 9 minutes.
  - Chunk 6: `pad_tech_costs` and `pad_planet_projects` typed their cheats in network mode. The cheats are now typed in jump mode, with the same counts and end states. `pad_planet_projects` grew from 562 to 568 frames and needs one Up tap under rule 8. Its incidental s2 → p47 → s2 network travel inside the cheat is gone; `pad_slipway` still covers it. About 15 minutes.
  - Total: about 24 minutes, well under the step 5b trigger (170 minutes). No generator is inserted.
- **Checks:**
  - `make end-state`: all 17 e2e, `title_idle` and `full_game_pause_restart` pass. `full_game`, `full_game_new_game` and `full_game_pause_idle` are stale (their scripts lack the □); a scratch `full_game` with □ at f13 matches.
  - `make coverage-compare`: 0 unhit. Chunks 4–5 had briefly left one baseline line unhit (`map_step` turned `int`), and chunk 6 restored it.
  - `make full-game` runs clean.
- **Known gaps:**
  - No replay hovers anything yet, so jump mode is covered only by unit tests.
  - No test covers △ refused under a HUD item.

### Step 6
- **Commits:**
  - `ce775f0`: `src/game/pad_slipway.{c,h}`, pure helpers: can-start (`slipgate_takes_slipway` for gates), candidates minus the start, between, outcome, release and rebase.
  - `a4deb2a`: the chord cheat.
  - `7ec1e81`: × and slipway mode.
  - `66d81e4`, `00dc472`, `fd74c1a`, `2afddde`, `1c8777f`: replays.
- **Slipway mode:**
  - × on a hovered colony or structure that can start a slipway, once it is inside the rectangle and clear of the HUD, presses it and holds the button.
  - The D-pad jumps between targets, start excluded.
  - × releases on the target. A built slipway continues from its target, so × D-pad × D-pad × builds the chain of two.
  - ○, pause, a tab change or a box opening cancel through the ui's null drop.
  - A refused start presses nothing.
  - Added beyond the PRD: tabs and pause wait one frame after a slipway release, as they do after a drag's.
- **Cheat verdict:** ordinary building could type × × ○ ○ × × ○ ○ (build, then ○ twice), so section 2's fallback applied. The cheat is four × + ○ chords: a lone × or ○ resets the count. `pad_nav_step` hides × and ○ from every mode from a chord frame until both are up.
  - Deliberate route: four chords, one every 2 frames, anywhere, for example in jump mode on a colony. Unit-tested to leave nav, cursor and game unchanged apart from the money and research.
  - The `worst-case/` games type the old sequence: their cheat count is 0 and `full_game_pause_restart` joins the stale set.
- **Re-authored:** 8 replays broke.
  - Chunk 2: `pad_bankrupt`, `pad_research`, `pad_tech_costs`, `pad_planet_projects`.
  - `pad_colonise`, `pad_slipway` (twice), `pad_research`, `pad_tech_costs`, `pad_planet_projects`.
  - `pad_resume_at_start` re-cut from `pad_research`'s last record.
  - About 49 minutes of re-authoring, 80 minutes for the whole step.
- **Frame counts:**

  | Replay | Before | After |
  |---|---|---|
  | `pad_bankrupt` | 180 | 170 |
  | `pad_research` | 577 | 458 |
  | `pad_tech_costs` | 571 | 565 |
  | `pad_planet_projects` | 568 | 583 |
  | `pad_colonise` | 345 | 315 |
  | `pad_slipway` | 348 | 362 |

  136 frames fewer in total. Every e2e slipway is now built by the × route, each in its reference direction, and every end state matches `8defcba`.
- **Situations kept on the old drag until step 7:**
  - `pad_colonise` f146–155 and f215–260: the crossed planet, a drop on empty space, a fold under a drag.
  - `pad_slipway`:
    - f100: □ from network mode into the drag.
    - f125–142: snap, cost, refusal, crossed planet.
    - f194–210: drop on empty space, fold under a drag.
    - f229–236: △ under the centre during a drag, colonising from a resumed drag.
    - f237–238: the drag pressed from the off-screen p47.
    - f313–361: □ from network mode into the drag, the duplicate seen from the drag, the drop on it, ○.
  - Network travel stays in `pad_slipway`.
  - `pad_tech_costs` ends through network mode on p54, the only reach of `pad_nav_next_slipway`'s insertion lines.
  - `pad_planet_projects` f172–173 is the only reach of `drop_stale_map_mode`.
- **Dropped or moved:**
  - `guideline` decision: the build from an off-screen start is dropped from `pad_slipway`. No × route keeps the reference slipway's direction in that galaxy; `test_an_off_screen_start_still_builds` stands in.
  - "Too far" moved to the × route (p28 → p25).
  - Network travel used only to get somewhere (`pad_research`, `pad_tech_costs`) became jumps.
  - The drag's build landing into network mode became the chain continuation.
- **Coverage:** `make coverage-compare` reports unhit 2: `src/game/pad_nav.c:538–539`, `drag_due_step`'s `land_on` after an old-drag build. The plan replaces those lines (the × route), and step 7 deletes the drag. Every other baseline line is hit.
- **Pins re-derived:**
  - `test_rng_sync`: the quiet window moved from {280, 344} to {288, 314}.
  - `test_live_slots` on `pad_tech_costs`: f512, f544, f559 and f564. The slipway arena is taken at the f503 re-press, and the rebuild grows nothing.
  - `test_circle_on_a_tab_returns_to_the_map` was retyped for the chord.
  - `test_state` is unchanged.

### Step 7
- **Commits:** `0d452f7` renames free mode to aim mode (`PAD_NAV_MAP_AIM`, `pad_nav_aim_mode`). `44a4329`: □ or ○ leaves aim mode for jump mode. `906af3d`: the drag is removed. `24cec17`: △ is inert in aim mode.
- **Aim mode now:**
  - The pan and × probe/fold work as before.
  - □ or ○ goes back to jump mode, hovering the target nearest the centre, with the camera unmoved. Both wait while × holds a fold.
  - ○ closes a ui ephemeral box first. The year box is not one: with it open, ○ leaves aim mode and the box stays (for step 8).
  - △ does nothing in aim mode. That is a PRD decision: section 2 defines △ on the hovered object.
- **Deleted:** the □ select, the drag, the starting and last planet, `triangle_on_centre`, and the ○ return to the last planet. Network mode can no longer be entered.
  - Left for step 12: `network_step`, `travel_to`, `click_planet`, `is_clickable_planet`, `centre_camera_on`, `drop_stale_map_mode`, `map_step`'s network branch, `pad_nav_selection`, `pad_nav_selected_slipway`, `pad_nav_next_slipway` with its two pure tests, the selection fields and `PAD_NAV_MAP_NETWORK`.
- **Re-authored:**
  - Chunk 2: 8 replays. Chunk 3: 5, plus `pad_colonise` again and the 4 `test_live_slots` fixtures. Chunk 4: 5. `pad_resume_at_start` was re-cut twice.
  - Frame counts: `pad_colonise` 315 → 411, `pad_slipway` 362 → 318, `pad_planet_projects` 583 → 585, `pad_research` 458 → 468, `pad_tech_costs` 565 → 566. The others are unchanged.
  - About 62 minutes of re-authoring, about 1 h 50 min for the whole step.
  - No `.end-state` was re-baselined, there is no exceptions file, and every cheat count is unchanged.
- **Situations dropped, all with their mechanic:**
  - Every ○ back to the starting or last planet, and every □ select into network mode.
  - Every situation of the old drag (lines and costs to empty space, the snap, a drop on empty space, a fold under a drag, △ during a drag).
  - Network travel, Left/Right turns and the selected slipway drawn in colour 12.
  - Every □ select became □ into jump mode on that colony. Every menu is kept, opened from jump mode, with re-pans standing in for network mode's camera centring (rule 8).
- **Situations moved:**
  - The crossed planet: now in slipway mode, `pad_research` f308–316 (s2 aimed at p23 passes p47).
  - A slipway aimed at a target under the top HUD: `pad_colonise` f337–372. It is the only e2e reach of `slipway.c:46, 92, 126–135`. Line 92 needs a transit-dot draw below 0.08, reached with 24 idle frames before the Up.
- **Coverage:** `make coverage-compare` shows 69 unhit lines.
  - Network-mode code in `pad_nav.c` that is now unreachable (76–123, 286–287, 297–301, 304–312, 420–425, 430–454, 472, 935–936). The plan replaces it and step 12 deletes it.
  - `save_state.c:888`: skipping a point-ended slipway in a save, reached only by the old drag over empty space.
  - `slipway_display.c:128`: network mode's selected-slipway colour.
- **Pins re-derived:**
  - `test_state` on `pad_tech_costs`: camera `127 234` → `101 242`, and its screen pins.
  - `test_rng_sync`: the quiet window is now {288, 410}.
  - `test_live_slots`: the SLIPWAYS fixture was re-authored the slipway-mode way (583 → 625 frames, pins moved). One assertion changed from "held over empty space" to "held aimed at p47", since the end over empty space no longer exists. The `pad_tech_costs` and `pad_planet_projects` pins shifted.
- **Tests:** `tests/test_pad_nav.c` went from 207 to 165 tests. Deleted: the drag, □ select, ○ return and network-mode tests. New: the PRD's 12 cases, the wait after a slipway release, the re-press refused under the HUD, the hovered circle's blink, and the reseed after a hovered object dies.
- **Cheat:** unchanged. Chords do nothing else in aim mode, and probing then leaving aim mode cannot type it (both unit-tested).
- **Long checks:** `make full-game`, `save-worst-case` (largest record 3995 B), `save-fuzz`, `sanitize-net` and `rng-sync-probe` are clean.
- **Process gap found:** the `test_live_slots` fixtures broke in chunk 2 and went unseen until chunk 3, because `make test` does not run that test. Verifiers now run `make unit-run-test_live_slots` every chunk.
