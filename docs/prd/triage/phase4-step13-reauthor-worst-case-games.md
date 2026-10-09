---
title: "Re-author the worst-case full games"
description: "The four scripted worst-case full games were written for the old tab-ring scheme and collapsed in step 1b (27 colonies down to 2). Re-author them for the final jump-cursor mapping, with no cheat, so that they reach the 8c4bbf4 memory peaks again and become the games section 4 measures."
status: needs-triage
---

## Plan step (verbatim)

Plan file `docs/plan-12.md`, Phase 4 — Direct buttons instead of the tab ring, step 13:

> 13. **Re-author the `worst-case/` full games** for the final mapping, judged by the acceptance rule step 2b recorded for these games. Their step 2a references were taken after step 1b collapsed them (full_game 27 colonies → 2), so they are not this step's target: this step re-baselines those five references on purpose, the one named exception to the no-re-baseline guardrail. Verified by `make full-game`, `make save-worst-case` and `make save-fuzz` passing and, at minimum, by every `make state` peak matching or exceeding the 8c4bbf4 peak table committed in step 2b.

The acceptance rule step 2b recorded, quoted from its note:

> **Worst-case verdict:** the 5 `worst-case/` games are judged by the 8c4bbf4 peak rule: every zone and cap peak must match or exceed `worst-case/peaks-8c4bbf4.txt`. Their step 2a references stay, to catch accidental changes, and step 13 re-baselines them on purpose.

Section 4 (Done when) measures these games, quoted:

> Measured with the step 14 tool on the re-authored `worst-case/` games, against the baseline of section 1: zero cheat activations, no dead D-pad press in jump mode and no dead ○ anywhere outside the group the locked rules require, and less than 25% of game time in aim mode.

The step 14 tool does not exist yet. This step builds the games so that they meet those four measures by construction, and records its own approximate measures for step 14 to confirm (D-7).

Section 2 guardrails, as they apply here:

- `make test-all` is green on every commit, and `make test` and `make e2e` get no slower. Nothing in this step enters either loop.
- The game rules don't change. This step changes no `src/game/` behaviour. A pad bug found while authoring is fixed by a separate `fix:` commit through the `fix-bug` skill before authoring continues, as in steps 9 and 11.
- The no-re-baseline rule has exactly one exception, and this is it: the references of `full_game`, `full_game_new_game`, `full_game_pause_restart` and `full_game_pause_idle` are re-baselined. `title_idle` presses nothing, and its reference must come out byte-identical (D-9).
- New inputs must not type the cheat by accident, and here the cheat count must be 0 (G-2).
- The `worst-case/` games have been knowingly stale since step 5. This step ends that.
- One scheme only. The raylib app is never launched: everything is headless.
- The step note records the re-authoring cost (agent minutes) and the frame counts.

## Facts from the repository (read at `01e4bd8`, unchanged at `1fd5dd3`)

- **The five files.** `{"input": "pad", "frames": [...]}`, one `{ "pad": [...] }` per line, no `initial_save`.

  | File | Frames | Built as |
  |---|---|---|
  | `full_game.json` | 4778 | the whole game: title, play, years to the final score panel |
  | `full_game_new_game.json` | 4818 | `full_game` verbatim, then 40 frames: "again?", the title, a new game |
  | `full_game_pause_restart.json` | 4419 | `full_game`'s first 4375 frames, then Start, restart, a new game |
  | `full_game_pause_idle.json` | 4677 | the same 4375 frames, then Start and the pause menu left open about 300 frames |
  | `title_idle.json` | 300 | no button at all |

  The `full_game` script presses (frame counts): Left 929, Right 923, Up 754, Down 730, × 516, ○ 184, □ 136, R1 58, △ 19, L1 10. It starts with Down at f10 and × at f12 on the title (difficulty "reasonable").
- **How they were made.** The commit that wrote them (inside `8c4bbf4`) says: "pad recordings built by a throwaway closed-loop generator: the cheat, the 12 reachable techs, 27 colonies and structures of all seven kinds, 20 slipways, years to the final score panel, plus the new-game, restart and idle branches. Every make state peak is matched or exceeded and no limit was lowered; FULL_GAME_LIVE_ENTITIES is recounted at 130." That generator was never committed.
- **Their current end-state references** (step 2a, `8defcba`) describe the collapsed games: `full_game` ends in year 3421 with 2 colonies, 0 slipways, 12 techs and 30 cheat activations. Cheat counts: `full_game` 30, `full_game_new_game` 30, `full_game_pause_idle` 24, `full_game_pause_restart` 24, `title_idle` 0. Since step 6 the old cheat sequence no longer fires, so their live runs now count 0.
- **The target, `worst-case/peaks-8c4bbf4.txt`** (31 lines, `<zone|cap> <name> <peak> <holder>`), taken over the five games only. The holders that are not `full_game`:
  - `full_game_new_game`: `zone game` 119816, `cap exit_queue` 122, `cap ephemerals` 5.
  - `full_game_pause_restart`: `zone ui` 106974, `cap button` 12.
  - `title_idle`: `zone particles` 24485, `cap particle` 580, `cap elements_particles_help_screen` 580.
  - Everything else is held by `full_game`, for example `cap entity_table` 136, `cap row` 29, `cap transit_dots` 48, `cap toast` 7, `cap message_box` 3, `zone frame` 13840.
- **`make state`** replays the five games and every e2e, and `tools/state_peaks.sh` fails when a peak eats its 1/8 margin (`9 × peak > 8 × limit`, a few exempt lines need only `peak ≤ limit`). The step 13 check is different: per line, the maximum over the five games must be ≥ the table's peak. No committed tool does that comparison yet.
- **`make full-game`** runs the five games with the `-O0` binary and fails on any `p8_fatal`. **`make save-worst-case`** encodes, decodes and re-encodes the save at every frame of a running game with no wormhole (8 KiB limit). **`make save-fuzz`** runs `full_game.json` only, and resumes every stored record into a fresh game.
- **`tests/test_exit_queue.c`** pins `FULL_GAME_LIVE_ENTITIES 130`: "the largest reset_world burst a worst-case/ game has reached (119 with a release), with margin: keep it when those games are re-authored lower".
- **Coverage** (`make coverage`, `make coverage-compare`) runs the e2e replays only, so these games do not affect it.
- **A replay is deterministic from its inputs.** The replay tool's loop is `input_step_run` on the raw frame, then `cart_update60` and `cart_draw`. `p8_input_save` writes the pad JSON format. After the title, the only game-stream draws are reveals (step 2b). The galaxy depends on the frame the title's × lands on.

## Facts from step 12 (done, `1fd5dd3`)

An earlier draft of this PRD held hypotheses H-12-1 to H-12-11 about step 12's outcome. Each was checked against the "### Step 12" note and the committed `pad_nav`, `pad_cycle`, `pad_build`, `pad_jump`, `pad_select`, `pad_target` and `cheat` headers and sources at `1fd5dd3` (`git show HEAD:<path>`). **All eleven held.** What step 12 actually left:

- **H-12-1, held.** No `pad_nav_tab_t`, no `pad_nav_tab()`, no end-turn tab. A year ends only by the Select hold: `PAD_SELECT_HOLD_FRAMES` 30, `PAD_SELECT_TAP_FRAMES` 10, `PAD_SELECT_CHAIN_FRAMES` 15 (a fresh press inside 15 frames after a year ends one more). The hold presses HUD button 1 (year skip) directly.
- **H-12-2, held.** `pad_nav_map_mode_t` is jump, aim and slipway only. `pad_nav_selection`, `pad_nav_selected_slipway` and `pad_nav_next_slipway` are gone, along with the ui's `pad_slipway` and the colour-12 slipway draw.
- **H-12-3, held.** `shoulder_switch`: R1 closes the bar and moves `pad_cycle` on (closed → tech → score → closed), so R1 from the map or the bar opens tech, and the landing click comes on the R1 frame as from the map. L1 toggles the bar (opening it closes the cycle). One shoulder alone acts, and both together do nothing. ○ off the map returns to it.
- **H-12-4, held.** All the accessors the generator reads exist: `pad_nav_jump_mode`, `pad_nav_aim_mode`, `pad_nav_hovered`, `pad_nav_slipway_start`, `pad_nav_slipway_target`, `pad_nav_last_acted`, `pad_nav_select_held`, `pad_nav_year_chain_open`, `pad_nav_build_bar_open`, `pad_nav_tech_open`, `pad_nav_score_open`, `pad_nav_build_icon` and `pad_nav_placing`. The pad state is `input_step_t.pad_nav`. The bar's open flag lives in `pad_build_t` and the cycle panel in `pad_cycle_t`, both read only through these accessors.
- **H-12-5, held.** No tuning value changed:
  - `PAD_JUMP_REPEAT_DELAY` 18 and `PAD_JUMP_REPEAT_PERIOD` 8;
  - margins 20, and 31 at the bottom;
  - `PAD_JUMP_SCROLL_SPEED` 8;
  - `PAD_TARGET_PERPENDICULAR_WEIGHT` 2;
  - the aim pan is 1 px a frame for the first 10 held frames, then 2 px up to 40, then 4 px.
- **H-12-6, held.** `CHEAT_CHORDS` 4. `without_chord` hides × and ○ from every mode, from a chord frame until both are up.
- **H-12-7, held.** The bar and placement work as in step 10. L1 opens the bar, Left/Right pick (`pad_build_pick`) and × takes. Carrying (`PAD_NAV_PLACEMENT_CARRYING`) pans with the same `aim_pan`, × places and ○ cancels.
- **H-12-8, held.** `pad_target_pick` and `pad_target_is_candidate` (planet family, KNOWN or SETTLED, not dead) are public. `pad_jump_candidates` builds the list from `TAG_NODE` in a caller-given zone. `pad_jump` now also exposes `pad_jump_direction` and `pad_jump_first_direction` (one Up, Down, Left, Right order) and `pad_jump_inside` and `pad_jump_world_point`, which the generator can reuse.
- **H-12-9, held.** Only `pad_end_turn` was re-authored (139 → 154 frames), and its end state is unchanged. No `.end-state` was re-baselined. `make full-game`, `make save-worst-case` and `make save-fuzz` ran clean on the stale games.
  - No `worst-case/` script presses R1 on the bar. Their old end-turn visits (R1 R1 around the ring) now reach tech and score. This step rewrites the scripts anyway.
- **H-12-10, held.** The modes step 14 reports are jump, aim, slipway, menus, placement, the bar and the cycle panels (tech, score). There is no end-turn mode and no network mode.
- **H-12-11, held.** Step 12 found `make test` and `make e2e` no slower, compared like for like, but recorded no wall times. This step's "before" times are measured at `1fd5dd3`.
- **Coverage after step 12:** 4 unhit lines. One is `save_state.c:888`. The other three are histogram-diff pairings of deleted `pad_nav.c` lines (baseline 596, 855, 863).
- **Size after step 12:** `pad_nav.c` 1061 lines; `pad_nav_t` 23 fields.

## Decisions (ratified by `guideline`)

`guideline` ratified G-1, G-2, G-4, G-5 and G-6 as recommended, with the refinements added below each. It changed G-3 in its proof clause only. The step note records them as decided.

### G-1. Authoring method: a closed-loop generator, committed

About 4,700 frames per game of hand-typed JSON is not workable. The plan's step 5b generator was never triggered, and the generator used for `8c4bbf4` was thrown away.

- **Decided: a closed-loop generator, committed** as a headless binary built by its own `make` target, outside `make test-all`.
  - **Refinement (`guideline`):** one `cmd` file plus a devtools module, with its own `make` target and a line in the Makefile legend. Each game's high-level script is committed next to its JSON in `worst-case/`.
  - It runs the same frame loop as the replay tool, but builds each raw frame itself. It reads the game state (entities, economy, camera) and the public pad_nav accessors to decide each frame's buttons, then writes the per-frame JSON with `p8_input_save`.
  - **Why closed-loop.** Its commands are about goals ("jump to p12", "probe at world 140 210", "colonise p12"), not frame counts. A closed-loop controller finds the presses, checks the outcome, and fails loudly at the command that went wrong. An open-loop compiler (step 5b's `idle`, `tap`, `hold`) would need every pan length and wait counted by hand, and every miss found later by replaying.
  - **Why committed.** The games will need regenerating at least three times after this step: when step 14's tool finds a dead press this step's guards missed (D-7), when the human's game leads to tuning (section 5: a mapping change under the re-authoring guardrails), and at any later mapping change. Each plan step runs in a new session, and `tmp/` does not carry over. A thrown-away generator would cost this step's biggest job again each time.
  - The JSON stays the checked artifact. Nothing in `make test-all` runs the generator, so `make test` and `make e2e` are untouched.
- Alternative: a throwaway generator in `tmp/`, as `8c4bbf4` did. It is cheaper today, with no review surface, but it is lost at the end of the session. Rejected for the reasons above.
- Alternative: an open-loop script compiler (step 5b's design). Rejected: it handles waits, not planets, so the agent would do the closed-loop work by hand.

### G-2. Zero cheat activations

The `8c4bbf4` games typed the cheat 24 to 30 times. Section 4 asks for zero cheat activations on these games.

- **Decided:** none of the four games types the cheat. Every colony, structure and tech is paid for by the game's own economy, and the years pass by Select holds. The generator refuses to emit a chord (four × + ○ frames).
- The content floor of `8c4bbf4` still applies (D-3): the 12 reachable techs, 27 colonies, all seven structure kinds, 20 slipways, and years to the final score panel. Without the cheat this takes more years of play. The game's length is free within G-4.
- **Refinement (`guideline`):** the four re-baselined references must show `cheat activations 0`.
- Rejected: keeping the cheat in the worst-case games and reading "zero" as "zero accidental". Section 4's text says zero, and its baseline (the human game) used none.

### G-3. When a peak cannot be reached without the cheat

The plan never waits, so the fallback is locked here.

- **Decided** (step 2 changed by `guideline`):
  1. Add more content first: more colonies, slipways, structures and years, more boxes and toasts at once. The cut point of the pause branches can also move (D-5).
  2. Only if that fails, prove the cause with a parent/child commit pair: the same input gives the same normalized end state (`tools/end_state.sh`) on both commits, but a lower peak on the child. The pair is one a step note already measured (for example step 8's frame zone, 13872 → 13840), or one found by bisecting with an unchanged input. Replaying the old `full_game` at `8c4bbf4` and at today's commit is **not** a proof, because their end states differ.
  3. A line short for that proven cause is listed in the step note with both numbers and the commit. It is never reached by lowering a limit, editing the table or typing the cheat.
  4. A line short for any other cause goes to `guideline` with the attempts listed.
- Rejected: an exceptions file read by the peak comparison. The table is a one-time target, and the step note is enough history.

### G-4. Game length and limits

- **Decided:**
  - Each game stays at 7,200 frames or fewer, 1.5 × the old length. `make save-fuzz`, `make save-worst-case` and `make state` run every frame, so their wall time grows with the games. **Refinement (`guideline`):** the step note records the wall times of `make save-fuzz`, `make save-worst-case` and `make state` before (at `1fd5dd3`) and after.
  - No zone or cap limit is raised. If a game pushes a peak over its 1/8 margin, `make state` goes red, and the script trims that content (for example fewer toasts at once) until it is green again.
  - Raising a limit is a memory-budget change outside this plan. If a table peak cannot be matched without crossing a margin, that goes to `guideline`.
- Rejected: no length cap. A 20,000-frame game would make `save-fuzz` several times slower for no extra peak.

### G-5. The galaxy, the difficulty and the first frames

- **Decided:**
  - Keep difficulty "reasonable", as the old games had: Down at f10, × at f12 on the title.
  - The title × frame may move, to pick a galaxy with enough reachable colonisable planets for the content floor without the cheat. The generator tries a few title frames. **Refinement (`guideline`):** the script records the chosen title frame (its `title` command). The references are re-baselined, so the galaxy is free.
- Rejected: a harder difficulty. It makes the cheat-free floor harder to reach, and no peak depends on it.

### G-6. Keep the five files and their roles

- **Decided:**
  - Same file names, same `FULL_GAME_EVENTS` list, same roles: the full game; that game plus a new game from "again?"; a shared prefix then pause → restart; the same prefix then pause left open about 300 frames; the title left alone.
  - `full_game_new_game` is `full_game` verbatim plus its tail. The two pause games share one prefix of `full_game`, verbatim.
  - `title_idle` is not touched. **Refinement (`guideline`):** it and its reference come out byte-identical.

## Problem Statement

From the player's point of view, the new pad scheme has never been played through a whole game by anything. Every e2e replay is under 600 frames, and the only full games in the repository are `worst-case/`'s four scripted ones.

Those four type the old tab ring and network mode. Step 1b already collapsed them (full_game now ends with 2 colonies instead of 27), and since step 5 every new mapping step has made them more stale. They still exit cleanly, but they no longer reach the situations they exist for. Their green checks mean nothing:

- `make full-game`, `make save-worst-case` and `make save-fuzz` no longer stress a crowded late game.
- `make state` no longer finds the memory peaks that sized the zones and caps.
- Section 4's ergonomic measures (cheat count, dead presses, aim share) need a whole game played with the final mapping.

From the developer's point of view, rewriting about 19,000 frames of per-frame JSON by hand is not realistic. Without a method that can be re-run, the games will go stale again at the first tuning change.

## Solution

- A committed closed-loop generator turns a short, readable script per game ("title at f12; jump to p3; probe at world 140 210; colonise p12 with its first project; slipway p3 → p12; build a lab at world …; research …; end the year; …") into the per-frame pad JSON.
  - It plays the game frame by frame while it writes, uses only the final mapping, checks the outcome of every command, and refuses inputs section 4 forbids: a chord, a D-pad press into an empty cone in jump mode, a ○ that changes nothing.
- With it, the four games are re-authored without the cheat, from one shared `full_game` script:
  - `full_game` plays a full game to the final score panel, with at least the `8c4bbf4` content.
  - `full_game_new_game` adds "again?" and a new game.
  - `full_game_pause_restart` and `full_game_pause_idle` branch off a late, crowded moment of it.
- A new peak comparison target shows every line of `worst-case/peaks-8c4bbf4.txt` matched or exceeded over the five games.
- The four end-state references are re-baselined on purpose. `title_idle`'s comes out byte-identical. `FULL_GAME_LIVE_ENTITIES` is recounted.
- The step note records each game's frame count, its mode split, its aim share and its dead-press guard result, ready for step 14 to confirm with its own tool.

## User Stories

1. As a maintainer, I want the worst-case games to play the final pad mapping, so that `make full-game` exercises the controls a player will actually use.
2. As a maintainer, I want `full_game` to reach at least 27 colonies, 20 slipways, all seven structure kinds and the 12 reachable techs again, so that the late-game load is back.
3. As a maintainer, I want every `make state` peak over the five games to match or exceed `worst-case/peaks-8c4bbf4.txt`, so that the zone and cap limits are again justified by a game that reaches them.
4. As a maintainer, I want a single `make` target that prints, line by line, the table's peak, the five games' peak and the verdict, so that the step 2b acceptance rule is mechanical.
5. As a maintainer, I want no limit lowered or raised by this step, so that the memory budget does not move under a test-data change.
6. As a maintainer, I want `make save-worst-case` to pass on the new games and report its largest record, so that the 8 KiB save limit is checked against a crowded game again.
7. As a maintainer, I want `make save-fuzz` to pass on the new `full_game`, so that every save of a long, rich game resumes exactly.
8. As a maintainer, I want `make full-game` to pass on all five games with the `-O0` binary, so that no assert fires anywhere in a full game.
9. As a maintainer, I want `FULL_GAME_LIVE_ENTITIES` recounted from the new games, so that the exit-queue test still covers the largest real reset burst.
10. As a maintainer, I want the four references re-baselined in one commit, with the reason in its message, so that the plan's one exception to the no-re-baseline rule is visible in history.
11. As a maintainer, I want `title_idle`'s reference to stay byte-identical, so that the exception covers only the games that changed.
12. As a maintainer, I want `make test` and `make e2e` unchanged in content and speed, so that the fast loop stays fast.
13. As a section 4 reader, I want zero cheat activations in every worst-case game, so that the measured game is an honest one.
14. As a section 4 reader, I want no D-pad press in jump mode that moves nothing, so that the games show the jump picker used as intended.
15. As a section 4 reader, I want every ○ in the games to do something, except where the locked rules require a no-op, so that "○ always goes back one step" is demonstrated.
16. As a section 4 reader, I want less than 25% of each game's frames in aim mode, with margin, so that the jump cursor visibly replaced pixel aiming.
17. As the agent of step 14, I want the step note to give each game's mode split, aim share and press counts per mode as the generator saw them, so that my tool has known answers to check against.
18. As the agent of step 14, I want the generator to emit a per-frame mode log next to its JSON (in `tmp/`, never committed), so that my tool's per-frame classification can be diffed against it.
19. As a future agent after tuning, I want to regenerate all four games with one command from committed scripts, so that a mapping change does not cost this step's work again.
20. As a script author, I want commands that name game objects by the ids `make state` prints (`p12`, `s2`), so that I can read a state dump and write the next command.
21. As a script author, I want "jump to p12" to find the D-pad path itself and press only directions whose cone holds a candidate, so that I never count jumps by hand.
22. As a script author, I want "probe at world x y" and "fold" to enter aim mode, pan with the fewest frames, act and leave aim mode, so that pixel aiming stays short and exact.
23. As a script author, I want "colonise p12", "slipway p3 p12", "build <kind> at x y" and "research <tech>" to check that the game did it, so that a failure stops at the command that caused it.
24. As a script author, I want "end the year" to emit a 30-frame Select hold and wait out the 15-frame chain window, so that years never chain by accident.
25. As a script author, I want "idle N", "tap B" and "hold B N" as escape hatches, so that a situation no controller covers can still be scripted.
26. As a script author, I want the generator to stop with the command, the frame and the reason when a check fails, so that a broken script is fixed in minutes.
27. As a script author, I want to run a script prefix and get a `make state` dump at its end, so that I can plan the next commands against the real galaxy.
28. As a script author, I want the generator to refuse a × + ○ chord frame, so that the cheat cannot be typed even by an escape hatch.
29. As a script author, I want the generator's last state to be compared with `make state EVENTS=` on the JSON it wrote, so that what it saw is proved to be what the replay plays.
30. As a script author, I want the pause games to share `full_game`'s frames verbatim up to their branch, so that they stay one game with three endings.
31. As a script author, I want the branch frame of the pause games picked where the ui zone and button cap peak, so that `zone ui` and `cap button` are matched.
32. As a script author, I want `full_game_new_game` to append "again?", the title and a new game to `full_game` verbatim, so that `zone game`, `cap exit_queue` and `cap ephemerals` see the full game's reset.
33. As a reviewer, I want the generator to reproduce an existing e2e replay byte for byte from a script, so that its frame loop is proven equivalent to the replay tool's before it writes anything new.
34. As a reviewer, I want each game's script committed beside its JSON, so that a review reads 100 lines of intent instead of 5,000 frames.
35. As a reviewer, I want the step note to list each game's frame count before and after, and the agent minutes spent, as every re-authoring step does.
36. As a reviewer, I want any pad bug the generator finds fixed by its own `fix:` commit through `fix-bug`, so that test data never hides a game change.
37. As a reviewer, I want the step 12 facts this PRD relied on (H-12-1 to H-12-11, all held at `1fd5dd3`) restated in the step note, so that the PRD's assumptions about step 12 are audited.
38. As the human after the plan, I want the worst-case games to look like a plausible pad game, so that my own recorded game can be compared with them.

## Implementation Decisions

### D-1. The generator (G-1)

A new headless binary, built by its own `make` target, outside `make test-all` and documented in the Makefile legend. It lives under the devtools and cmd split of `CLAUDE.md`: a `cmd` file for the binary, with its logic in a devtools module. It never opens a window.

- **Input:** one script file. **Output:** the pad JSON (via `p8_input_save`, the same format as today's files) and, with `STATE=`, the state file the replay tool would write. It also writes a per-frame mode log in `tmp/` (D-7).
- **Frame loop:** the replay tool's exact loop (`input_step_init`, then `input_step_run` on the frame it built, `cart_update60` and `cart_draw`), with no capture. The game has no other input, so the JSON replays to the same frames.
- **Reading the game:**
  - It uses the public pad_nav accessors (step 12 facts, H-12-4) for the mode, the hovered object, the slipway start and target, and the bar and panel state.
  - It uses the entity registries and `map_pos` for object positions, the economy for credits and science, the main camera for the view, and `pad_target_pick` over the `pad_jump_candidates` list for the D-pad path, plus `pad_jump_inside` and `pad_jump_world_point` (H-12-8).
  - It never reads `pad_nav_t` fields directly.
  - Object ids are the ones `make state` prints, resolved through the same devtools code. The generator does not invent a second naming.
- **Command set** (the script language: one command per line, `#` comments):
  - Escape hatches: `idle N`, `tap <buttons>`, `hold <buttons> N`.
  - `title <frame> <difficulty>` presses the title's buttons so that × lands on that frame.
  - `jump <id>` reaches the object in jump mode.
    - It runs the picker from the hovered object for each direction, and searches for the shortest sequence of taps whose picks end on the target. Taps only: no hold repeat, so step 14 has no repeats to separate.
    - Every tap has a candidate in its cone. It fails if the target is unreachable, or not inside the inner rectangle once reached.
  - `probe <x> <y>` and `fold [<x> <y>]` enter aim mode with □, pan the centre onto the world point, and press × (a fold is the × hold). They leave with □ and check the probe exists, or the fold revealed something.
    - The pan picks the frame count that reaches the point exactly at 1/2/4 px a frame. The world point is checked to the pixel.
    - Before entering aim mode, it jumps to the candidate nearest the point, so the pan stays short (D-7's aim budget).
  - `colonise <id> [<project row>]` jumps, presses × on the known planet, moves to the row in the projects menu, presses ×, and checks that the planet settled.
  - `slipway <from> <to> [<to>...]` jumps to the start, presses ×, D-pad jumps between targets with the button held, and presses × on each target in a chain. It checks each slipway was built in that direction.
  - `build <kind> <x> <y>` opens the bar with L1, picks with Left/Right, takes with ×, pans the centre onto the point and places with ×. It checks the structure exists at that world point.
  - `research <tech>` opens tech with R1, moves to the tech and invents it with ×. It checks the tech count rose, then closes with ○ or the R1 cycle.
  - `score` runs R1 R1 R1 (tech → score → closed).
  - `year` holds Select 30 frames, checks the year advanced by one, then idles until `pad_nav_year_chain_open` is 0.
  - `attention` taps Select once and checks the hovered object changed.
  - `back` presses ○ once and fails if nothing changed (D-7).
  - `pause`, `restart`, `again` and `new-game` handle the pause menu and the final score panel.
  - `until-game-end` repeats `year` until the final score panel shows.
  - `include <script>` and `stop-at <label>` share prefixes between games.
- **Refusals,** each a hard failure with the command, frame and reason:
  - a frame with × and ○ pressed on the same edge (a chord);
  - a D-pad tap in jump mode whose cone holds no candidate;
  - a `back` that changes no pad mode, hovered object, camera, open box or panel;
  - a frame budget over G-4.
- **Self-check at the end of every run:** the generator writes its own state file, re-reads the JSON it wrote, replays it through the replay path, and compares the two state files byte for byte. A difference is a generator bug.
- **Proven before use:** a script for an existing e2e that uses only escape hatches regenerates that replay's JSON byte for byte, and its state matches `make state EVENTS=`. `pad_end_turn` (154 frames since step 12) or `pad_probe` (title, aim, probe, fold) are the candidates. A second script, using closed-loop commands, reaches that replay's `.end-state` and passes `make end-state EVENTS=` on its output. This proves the controllers without matching frames.

### D-2. Committed scripts

- Each re-authored game gets a script beside its JSON in `worst-case/` (`full_game.pad`, and so on; the extension is the implementer's choice). The two pause games and `full_game_new_game` `include` the shared `full_game` script up to a label.
- The scripts are inputs to the generator, not to any check. `make full-game` and the others still read only the JSON.
- The generator's `make` target regenerates all four JSON files. Re-running it on unchanged scripts gives byte-identical JSON. The step note shows this was checked.

### D-3. The content floor of `full_game` (G-2)

At least the `8c4bbf4` content, without the cheat:

- the title at difficulty "reasonable";
- the 12 reachable techs invented, through the R1 tech panel;
- 27 colonies or more;
- structures of all seven kinds, through the L1 bar;
- 20 slipways or more, including chains of two or more (× D-pad ×) and at least one through a slipgate;
- several probes and folds from aim mode;
- years by Select hold up to the final score panel;
- a few Select taps (attention), △ menus opened and closed, ○ used at each rung of its order, and the score panel visited through R1.

The last group exists so the games exercise every mode step 14 measures, not to reach a peak.

### D-4. `full_game_new_game`

`full_game`'s frames verbatim, then × on "again?" on the final score panel, the title, and a new game started. That is what the old 40-frame tail did. The tail's length is free. It must keep the reset burst that holds `zone game`, `cap exit_queue` and `cap ephemerals`.

### D-5. `full_game_pause_restart` and `full_game_pause_idle`

- **One shared prefix of `full_game`.** The branch frame is chosen by trying candidate frames late in the game (the generator runs a prefix plus a fixed suffix for each) and keeping the one where the pause menu over the map gives the highest `zone ui` and `cap button`, the two lines `full_game_pause_restart` holds in the table. A menu, panel or year box open at the branch is allowed if it raises them.
- **`full_game_pause_restart`:** Start, restart from the pause menu, then a new game from the title.
- **`full_game_pause_idle`:** Start, then the pause menu left open 300 frames.

### D-6. The peak comparison target

- A new `make` target outside `make test-all` (with a small script in `tools/`). It runs `make state`'s per-game runs for the five `FULL_GAME_EVENTS` only. For each line of `worst-case/peaks-8c4bbf4.txt` it prints the table's peak, the maximum over the five games, the game holding it, and `ok` or `short`. It exits 1 on any `short`.
- It reuses `tools/state_peaks.sh` for the per-line maxima, so the zone and cap names and the tie rule are the same as the table's.
- `make state` (all replays, margins) must stay green as well (G-4).

### D-7. Preparing section 4's measures before step 14 exists

Step 14's tool will define dead presses exactly ("presses that change no nav state, cursor, camera or game state", with a required group). This step cannot run it. Instead:

- **Built in:**
  - The generator refuses the inputs section 4 forbids (D-1's refusals). Every D-pad press in jump mode comes from `jump`, which taps only into non-empty cones and checks that the hovered object changed.
  - Every ○ comes from a controller that checks its effect. A deliberate root ○ with nothing to go back to is allowed only through `back --required`, which records it as a required no-op.
  - Chords are refused, so the cheat count is 0 by construction. `make end-state` confirms it.
- **Aim budget:** aim mode is entered only by `probe`, `fold` and `build`, after a jump to the nearest candidate. The generator counts aim-mode frames (`pad_nav_aim_mode`) and fails a run whose share is 25% or more, section 4's line (guideline ruling during chunk 3: the cheat-free full_game cannot reach 20%). Padding counts only clock-free commands checked for an effect, each on a new object or panel.
- **Recorded for step 14:**
  - The generator writes a per-frame mode log to `tmp/`: frame, mode (jump, aim, slipway, menu, placement, bar, tech, score, pause, title), buttons, and for each press edge whether the controller saw an effect.
  - The step note copies each game's totals: frames per mode, aim share, presses per mode and button, hold repeats (expected 0), cheat count, and required no-ops.
  - Step 14 checks its tool against these numbers. If its stricter definition finds a dead press, the script is fixed and the games regenerated (G-1).
- **Not done here:** no committed metrics tool. That is step 14's job, and its definitions may differ from this approximation.

### D-8. `FULL_GAME_LIVE_ENTITIES`

- Measure the largest `reset_world` burst (live entities, counted with a release) over the five new games, with a throwaway instrumented build in `tmp/`. That is how the 119 behind 130 was found.
- If the burst is 119 or less, keep 130 (the test's own comment).
- If it is higher, set the constant to the burst plus about 1/10, rounded up to the nearest 5, and update the comment's number. The constant must still fit the exit-queue cap, which `make state`'s `cap exit_queue` margin already guards.

### D-9. The references

- One commit deletes and rebuilds the end-state references of `full_game`, `full_game_new_game`, `full_game_pause_restart` and `full_game_pause_idle` with `make end-state-baseline` (it writes only missing references). Its message names the plan's exception and the new reference commit for these four.
- `title_idle.json` and its reference are untouched. `make end-state` on it passes unchanged.
- The new references must show cheat activations 0.
- After that commit, `make end-state` passes on all 22 files (17 e2e and 5 worst-case).

### D-10. What changes outside `worst-case/`

- New: the generator (cmd and devtools files), its `make` target and legend line, the peak comparison target and its script.
- Edited: `tests/test_exit_queue.c`, only if D-8 says so.
- Edited: the Makefile legend lines for `full-game`, `save-worst-case` and `save-fuzz`, only where they describe the games' content.
- Nothing in `src/game/`, `src/engine/` or `src/p8/` changes, unless a `fix:` commit through `fix-bug` is needed.
- No e2e replay, `.txt`, hash or fixture changes.

## Implementation Chunks

Each chunk ends on a commit with `make test-all` green and `make raylib` linking. Once chunk 3 exists, each chunk also runs `make full-game`, `make save-worst-case` and `make save-fuzz` on the games it touched, and the peak target. `make e2e` and every e2e `.end-state` stay unchanged throughout.

1. **Build the generator core.** (Step 12's facts are already checked above: all held.)
   - Measure the "before" wall times at `1fd5dd3`: `make test`, `make e2e`, `make save-fuzz`, `make save-worst-case` and `make state`.
   - Write the generator's frame loop, the escape-hatch commands, `title`, `include` and `stop-at`, the JSON output, the state output and the end self-check.
   - Write the peak comparison target (D-6). Run it on today's stale games and record the shortfall as the "before" figure.
   - **Proof:** an existing e2e regenerated byte for byte from an escape-hatch script (D-1).
2. **Closed-loop controllers.**
   - `jump`, `probe`, `fold`, `colonise`, `slipway`, `build`, `research`, `score`, `year`, `attention`, `back`, `pause`, `restart`, `again`, `new-game` and `until-game-end`, with their checks, the refusals and the aim budget. Plus the per-frame mode log.
   - **Proof:** a closed-loop script reaches an existing e2e's `.end-state` (`make end-state EVENTS=` on the generated file in `tmp/`).
   - **Proof:** each refusal fires on a deliberately bad script (a chord, an empty-cone tap, a ○ with no effect).
3. **`full_game`.**
   - Pick the galaxy (G-5). Write the script in stages (opening, expansion, techs, late game, years to the end), running the prefix and reading `make state` between stages.
   - Reach the D-3 floor without the cheat, inside G-4's length.
   - Commit the script and the JSON. The peak target runs with partial results: lines held by the other games may still be short.
   - The four old references are not touched yet, so `make end-state` is red on `full_game` from here until chunk 5. The plan allows this: these games have been stale since step 5. The commit message says so.
4. **`full_game_new_game`, `full_game_pause_restart` and `full_game_pause_idle`.**
   - The new-game tail (D-4). The branch frame search and the two pause tails (D-5).
   - Commit the three scripts and JSON files. The peak target is now complete: every line `ok`, or a G-3 shortfall listed with its proof.
5. **References, constants and the step note.**
   - Re-baseline the four references (D-9). Recount `FULL_GAME_LIVE_ENTITIES` (D-8).
   - Run `make full-game`, `make save-worst-case` (largest record), `make save-fuzz`, `make state`, the peak target and `make end-state`.
   - Regenerate all four from their scripts and check the JSON is byte-identical (D-2).
   - Measure the `make test` and `make e2e` wall times like for like against step 12's last commit, and the long targets' times before and after.
   - Write "### Step 13" in `docs/plan-12.md` (Further Notes lists its contents).

If chunk 3 proves too big for one cycle, split it at the stage boundary after the expansion: first the script and JSON up to 27 colonies, then the rest. `full_game.json` is committed only once it reaches the final score panel, because `make save-fuzz` and the pause branches need the whole game.

## Testing Decisions

- **What a good check is here.** The output is test data, and the checks are the plan's: the peak rule (D-6), `make full-game`, `make save-worst-case`, `make save-fuzz`, `make state` margins, and `make end-state` against the re-baselined references. The generator's own correctness is proven by reproducing existing e2e replays (D-1) and by its end self-check, not by unit tests of its internals.
- **No new unit test in `make test`.** The generator is a devtools authoring tool outside the loop, as `tools/gen_quarter_wave.c` is for `make sin-table`. A test in `make test` would slow the loop for a tool that runs a few times per plan.
- **The refusals are checked once,** in chunk 2, by deliberately bad scripts, and the result goes in the step note.
- **Pinned values:**
  - `tests/test_exit_queue.c`'s constant (D-8) is the only test that pins a worst-case number.
  - The save-load, save-fuzz and resume-check tests pin the wormhole seed `0x5EED2026` and the probe at 90, 50, which do not come from these games.
  - `make rng-sync-probe` and `make unit-run-test_live_slots` read only e2e replays and do not change. They are run once in chunk 5 as a sanity check.
- **Coverage:** `make coverage-compare` reads only the e2e replays, so this step cannot move it. It is run once in chunk 5 to confirm the unhit set step 12 left (4 lines).
- **Prior art:**
  - `src/cmd/replay.c` for the frame loop and the `STATE=` output;
  - `tests/test_save_worst_case.c` for driving the cart headless from a frame source;
  - `tools/state_peaks.sh` for the peak lines;
  - `tools/end_state.sh` and `make end-state-baseline` for the references;
  - `tools/gen_quarter_wave.c` for a committed generator outside the suite.

## Out of Scope

- The pad metrics tool and its exact dead-press definition (step 14). This step records approximate figures for step 14 to confirm.
- The README Controls section (step 15) and the comment rework (step 16).
- Any change to the mapping, the tuning values, the game rules, zone or cap limits, or `worst-case/peaks-8c4bbf4.txt`.
- Any e2e replay, its `.txt`, hash, fixture or `.end-state`.
- Re-hitting `save_state.c`'s point-ended slipway skip, which step 12's R-4 left unhit. No pad route can make a point-ended slipway, and these games are not in the coverage set.
- `title_idle.json`, which presses nothing.
- Audio, PSX and mouse input.

## Further Notes

- **The step note** ("### Step 13" in `docs/plan-12.md`) records:
  - the step 12 facts relied on (H-12-1 to H-12-11, all held at `1fd5dd3`);
  - the `guideline` rulings on G-1 to G-6, including G-3's commit-pair proof;
  - the chosen title frame and galaxy;
  - each game's frame count before and after;
  - the D-3 content reached (colonies, slipways, structure kinds, techs, years);
  - the peak table with each line's new value and holder, and any G-3 shortfall with its proof;
  - the `FULL_GAME_LIVE_ENTITIES` recount;
  - `make save-worst-case`'s largest record;
  - D-7's per-game metrics for step 14;
  - the agent minutes spent (generator, then each game);
  - the wall times of `make test` and `make e2e` (unchanged), and of `make save-fuzz`, `make save-worst-case` and `make state` (before at `1fd5dd3`, and after).
- **For step 14:** verify the tool on one e2e replay as the plan says, then run it on the four games. Compare its numbers with D-7's figures. A stricter dead-press finding means a script fix and a regeneration through the committed generator, not a hand edit of the JSON.
- **For later tuning (section 5):** after a tuning change, run the generator's `make` target. If every script still passes its checks, the new JSON files plus a re-baseline of the four references are the whole re-authoring. That re-baseline needs its own exception from whichever plan makes the change.
- **Risk:** the cheat-free economy is the unknown. The human game in section 1 reached 30 colonies in 14 years without the cheat, so the D-3 floor looks reachable within 25 years. G-3's order (more content first, then proof) keeps the step moving if a single peak line resists.
