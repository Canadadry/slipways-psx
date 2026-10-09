---
title: "A game with nothing to jump to starts in aim mode"
description: "A new game (or a resume) has no known object, so jump mode hovers nothing and the player must press □ before placing the starting probe: start such a game in aim mode, make □ and ○ do nothing there until an object exists, and carry the change through the replays, the generator, the metrics tool, the worst-case games and the README."
status: needs-triage
---

## Plan step (verbatim)

Plan file `docs/plan-12.md` (at `1561217`), Phase 6 — After the human's first game, step 17:

> 17. **A game with nothing to jump to starts in aim mode** (decided by the user after playing: the first thing a game asks is to place the starting probe, and jump mode has nothing to hover). On a new game, or a resume, whose jump candidate set is empty, the map's root starts in aim mode with the crosshair at the centre; × places the probe as today. □ or ○ goes back to jump mode only once a candidate exists; with none, they do nothing (a required no-op for step 14's tool, like ○ at the root). A game with a candidate at start (a resume with colonies) starts in jump mode as today. The README's jump-mode line ("every game starts here") is corrected. Every replay and `worst-case/` game whose start changes is re-authored or regenerated from its `.pad` under section 2's guardrails (the frames proofs and `worst-case/*.pad` through the generator, whose `enter_aim` learns the new start); the `pad_colonise` proof's required group may gain the start's no-op. Verified by unit tests in `tests/test_pad_nav.c` (a new game starts in aim mode; □ and ○ do nothing there until a candidate exists; a resume with a colony starts in jump mode), every `.end-state` unchanged, and section 4 re-measured with `make pad-metrics-worst-case`: if the aim share crosses 25%, the games are regenerated under step 13's padding ruling, or the miss goes to `guideline`.

Section 2 applies in full. The checks outside `make test-all` that it requires on a chunk touching pad input, a replay, `src/game/pad_*`, `src/devtools/pad_*` or `src/cmd/pad_author.c`: `make unit-run-test_live_slots`, `make end-state EVENTS=` on every touched replay, `make coverage-compare`, `make rng-sync-probe` (whenever `pad_colonise` changes), `make pad-author-proof`, and `make pad-author` followed by `git diff --exit-code worst-case/`. Rule 32 (a step that changes what ○ does re-checks the cheat), rule 34 (`pad_nav.c` line count and `pad_nav_t` field count before and after; test deletion only with its mechanic) and the re-authoring-cost record also apply.

## Facts from the repository (read at `1561217`) and from a prototype

Every number below marked *prototype* was measured on a scratch copy of the tree outside the repository with the decisions of this PRD applied (nothing was committed). The implementer re-measures; the numbers say what to expect.

- **How a game starts today.** `pad_nav_step` calls `reset_on_new_game` first: when the main economy differs from `pad_nav_t.game`, it resets `pad_jump`, `pad_back` and the map mode to jump mode with nothing hovered (`reset_map_mode`). The economy is created inside the cart update of the title's × frame (f12 in most replays), after that frame's pad step, so the reset runs on the first map frame (f13). Until then `pad_nav_aim_mode` and `pad_nav_jump_mode` read the stale `map_mode`.
- **Jump mode with nothing hovered** keeps the cursor at the centre and seeds on the first object that appears (`seed_hovered` → `pad_jump_reseed`: first colony, else nearest the centre). □ there enters aim mode; `leave_aim_mode` (□ or ○ in aim mode) always returns to jump mode, hovering the target nearest the centre, or nothing when there is none.
- **Candidates** are `TAG_NODE` entities passing `pad_target_is_candidate` (live, planet family, KNOWN or SETTLED). A new game has none; the first reveal comes from the first fold, several frames after the fold's release (prototype: 11 frames in `full_game`).
- **Every replay presses □ on its first map frame**, except `pad_game_end` (stays in jump mode with nothing hovered) and `pad_tech_costs` (its resumed record knows no object; R1 first, □ at f29). `pad_probe` and the four replays that share its prefix press ○ at f16 (back to jump mode with nothing hovered) and □ at f17; `pad_research` does the same at f28/f29.
- **The generator** (`src/devtools/pad_author.c`) asks the mode through `pad_metrics_mode`, which reads the `pad_nav` accessors before the frame's step. `enter_aim` returns at once in aim mode and otherwise taps □; `leave_aim` taps □ and requires jump mode after; `run_probe` taps ×, leaves aim, then idles `PROBE_REFUSAL_FRAMES` (10) before counting probes; `run_fold` holds × 16 frames then releases with ○ (which also leaves aim mode) and leaves aim if still there; `run_build` leaves aim at its end; `run_jump` requires jump mode within 2 frames; `menu <id>` takes no dwell. A dead ○ is refused unless `back --required`. In the escape-hatch scripts, `tap B` is **one** frame (`back` is two: ○ then idle).
- **`pad_metrics`' required group** today: ○ in jump mode with no box and nothing to go back to (or the last object hovered), and a D-pad press or repeat into an empty cone in jump or slipway mode. `read_group_facts` reads facts only in jump and slipway mode. `make pad-metrics-check` compares the `pad_colonise` proof's required group with `PAD_METRICS_PROOF_REQUIRED` (one line, `dead f293 press circle jump required`).
- **Sizes before:** `pad_nav.c` 1011 lines; `pad_nav_t` 23 fields; `tests/test_pad_nav.c` 224 tests, `tests/test_pad_metrics.c` 33, `tests/test_pad_jump.c` 23.
- **Prototype, the rule alone** (start rule plus the □/○ guard, every replay unchanged): 9 captures stay byte-identical (the start □ lands in aim mode with no candidate and does nothing); 8 change, and only on these frames: `pad_probe`, `pad_colonise`, `pad_slipway`, `pad_empty_planet`, `pad_restart_resume` f16 (the crosshair no longer disappears under the ○); `pad_research` f28 (same); `pad_tech_costs` f13–16 and f28 (the crosshair is drawn: the record starts in aim mode); `pad_game_end` f13–92 (the crosshair is drawn: the game is in aim mode until the final score panel). All 17 `.end-state` references pass. Replacing every no-op start □ by an idle frame (and swapping `pad_probe`'s f16/f17, see D-7) leaves exactly the same 8 captures changed on exactly the same frames, and `pad_resume_at_start`'s fixture still equals `pad_research`'s last record.
- **Prototype, the generator:** without the accessor decision (D-2) the generator still taps □ at f13, because the mode it reads there is the stale jump mode. With D-2 to D-5, all 4 frames proofs regenerate their (edited) replays byte for byte, the 4 end-state proofs reach their references and the 4 refusal proofs are refused with their messages. `make pad-metrics-check` totals equal the generator's; the required group becomes f16 □, f17 ○, f293 ○. `make coverage-compare`: the same 3 unhit lines as before the change (`pad_nav.c` baseline 855 and 863, `save_state.c:888`). `make unit-run-test_live_slots` and `make rng-sync-probe` pass. `make test` fails only in `test_pad_nav` (its start helper asserts jump mode).
- **Prototype, the aim share** (section 4 over in-game frames, `make pad-metrics-worst-case`), aim frames / in-game frames:

  | Variant | `full_game` | `new_game` | `pause_idle` | `pause_restart` |
  |---|---|---|---|---|
  | Before the step (step 14 note) | 24.74% | 24.73% | 24.97% | 24.92% |
  | Rule + generator exit rules (D-4) | 25.09% (1507/6006) | 25.08% | 25.33% (1507/5950) | 25.34% |
  | + probe check on the release (D-5) | 24.97% (1497/5996) | 24.96% | 25.20% (1497/5940) | 25.22% |
  | + `score` into the first reveal wait, `menu s10` before the branch with dwell 55 (D-9) | 24.69% (1493/6046) | 24.69% | 24.88% (1493/6002) | 24.89% (1497/6014) |

  The new rule itself costs about 12 aim frames per game that no script change can remove: the first map frame, and the wait for the first fold's reveals, which happens in aim mode because there is nothing to jump to. The probe's 10 refusal-check frames move into aim mode too, but they were a measurement artefact (D-5). Every regenerated game keeps its `.end-state` in every variant.
- **Padding ruling of step 13** (plan step 13 note): padding counts only clock-free commands checked for an effect, on a new object or panel, dwell at most about 60 frames. Every one of the 44 objects known at `full_game`'s `pause-branch` label is already browsed by a `menu` line before it, except `s10`, whose `menu` line comes right after the label. A `score` visit costs about 6 frames.

## Problem Statement

The human's first game showed it: a new game opens on a map where nothing is known, so jump mode — the root every game starts in — has nothing to hover, the D-pad and × do nothing, and the very first thing the game asks for, the starting probe, needs a □ first to reach aim mode. The same happens on a resume whose record knows no object. The first press of every game is a press the player should not have to know about.

## Solution

When a game appears (new game, restart, or resume) and no object is a jump candidate, the map's root starts in aim mode: the crosshair at the centre, the D-pad pans, × places the probe as today. While no candidate exists, □ and ○ do nothing in aim mode (there is nowhere to jump back to). Once an object is known, □ or ○ goes back to jump mode on the object nearest the crosshair, as today; the first reveal does not switch modes by itself. A game that appears with at least one candidate (a resume with colonies, structures or known planets) starts in jump mode as today, seeded on the first colony.

The tools follow: the metrics tool counts those □ and ○ no-ops as required, the generator stops pressing □ at the start and leaves aim mode only once it can, the replays drop their start □, the worst-case games are regenerated (with a small change to the generator and two script edits that keep their aim share under 25%), and the README says where a game starts.

## User Stories

1. As a player starting a new game, I want to land in aim mode with the crosshair at the centre, so that I can place the starting probe without pressing anything first.
2. As a player starting a new game, I want × to place the probe exactly as it did after □, so that nothing I learned about aim mode changes.
3. As a player holding × from the title into the game, I want that held × to place nothing, so that starting a game never drops a probe by accident.
4. As a player in aim mode before anything is known, I want □ to do nothing, so that I am not sent to a jump mode with nothing to hover.
5. As a player in aim mode before anything is known, I want ○ to do nothing, so that I stay where the game needs me.
6. As a player whose fold has just revealed planets, I want to stay in aim mode, so that a reveal never takes the crosshair away while I may want another fold.
7. As a player in aim mode once something is known, I want □ and ○ to go back to jump mode on the object nearest the crosshair, as before.
8. As a player resuming a game with colonies, I want to start in jump mode on my first colony, as before.
9. As a player resuming a game saved before any probe or reveal, I want to start in aim mode, so that a resume behaves like the new game it is.
10. As a player restarting from the pause menu into a new game, I want the new game to start in aim mode whatever mode the old game was in.
11. As a player in aim mode with nothing known, I want ○ to still close the year box or a message box, so that the ○ order keeps working before anything is known.
12. As a player in aim mode with nothing known, I want a held Select to still end the year, and a Select tap to do nothing, as before.
13. As a player in aim mode with nothing known, I want L1, R1 and Start to work as before.
14. As a player, I want the four ×+○ chords to remain the only way to type the cheat, so that the new no-ops cannot type it.
15. As a reader of the README, I want the jump-mode line to stop saying every game starts there, and the aim-mode line to say a game with nothing known starts there and that □ and ○ do nothing until something is known.
16. As the metrics tool, I want □ and ○ that do nothing in aim mode with no candidate counted in the required group, so that the locked rule is not reported as a design fault.
17. As the metrics tool, I want a dead ○ in aim mode when a candidate exists, or with a box open, to stay in the other group, so that a real dead ○ still fails the verdict.
18. As the metrics tool, I want the mode at a game's first map frame to be the mode the rule gives, so that the frame and the presses on it are attributed to the right mode.
19. As the generator, I want to see aim mode from the first map frame of a game with nothing known, so that `enter_aim` presses no □ there.
20. As the generator, I want to leave aim mode only when a candidate exists, and to wait for one when a command needs jump mode, so that I never type a dead □ or ○.
21. As the generator, I want a fold released while nothing is known to release with an idle frame, not ○, so that the release is not a dead ○.
22. As the generator, I want to check that a probe was accepted on its release frame, so that I do not idle 10 frames in aim mode for a check the game already answers.
23. As the author of a worst-case script, I want a `menu` line to take an optional dwell of at most 60 frames, so that the one object left unbrowsed can carry the padding the step 13 ruling allows.
24. As the maintainer of the e2e replays, I want every no-op start □ replaced by an idle frame, so that frame counts, later frames and every unchanged capture stay identical.
25. As the maintainer of `pad_probe`, I want its f16–f17 presses kept as □ then ○ doing nothing, so that one replay shows the new rule and its proof shows the required no-ops.
26. As the maintainer of the e2e notes, I want every `.txt` to say where its game starts under the new rule, so that no note claims every game starts in jump mode.
27. As the maintainer of `pad_game_end`, I want its × held into the final score panel to still press nothing, now in aim mode, with the reason written down.
28. As the maintainer of the live-slots fixtures, I want the fixtures cut from replay prefixes to follow those prefixes, so that they stay true cuts.
29. As the plan, I want every `.end-state` unchanged, so that the step changes how input becomes presses and nothing else.
30. As the plan, I want every worst-case game under 25% aim share, with some margin, so that section 4 still holds after the step.
31. As the plan, I want no new `pad_nav_t` field, so that the rule lives in the existing map mode.
32. As a test author, I want unit tests that a new game starts in aim mode, that □ and ○ do nothing there until a candidate exists, that a resume with a colony starts in jump mode and that a resume with nothing known starts in aim mode.
33. As a test author, I want the `pad_nav` accessors tested on the frame the game appears, before any pad step sees it.
34. As a test author, I want a pure test of "is there any candidate" on hand-built node lists.
35. As a reviewer, I want `make test` and `make e2e` not slower.

## Implementation Decisions

**D-1 — The rule.**
- "Empty at start" means: at the first `pad_nav_step` that sees a main economy different from `pad_nav_t.game` (a new game, a restart, or a resume), no live `TAG_NODE` entity passes `pad_target_is_candidate`. Then `reset_on_new_game` resets as today and then enters aim mode (crosshair at the centre, repeat cleared, exactly as `enter_aim_mode` does). With at least one candidate, it resets to jump mode as today, and the existing seed (first colony, else nearest the centre) picks the hovered object on that frame.
- The □/○ guard lives in leaving aim mode, not in a start flag: in aim mode, □ and ○ (the "leave aim mode" rung) go back to jump mode only when at least one candidate exists **now**; otherwise they do nothing — no mode change, no hover, no press, no camera move. This covers the start, a resume, and any later moment with no candidate. Everything ○ does before that rung is unchanged (pause, ephemeral box, the year and message boxes, a slipway, a placement).
- The first reveal does not move the player: aim mode stays aim mode until □ or ○ (decided by the user).
- Jump mode that loses every candidate later in a game (all known planets vanishing before any colony) is unchanged: it stays in jump mode with nothing hovered, and □ still enters aim mode. Out of scope.
- No new state: the rule uses `map_mode`. `pad_nav_t` stays at 23 fields.

**D-2 — The accessors answer for the game on screen.** Between the update that creates a game and the first `pad_nav_step` that sees it, `pad_nav_aim_mode`, `pad_nav_jump_mode`, `pad_nav_hovered` and the slipway accessors report the start state the rule will give (aim mode when no candidate exists, jump mode otherwise; nothing hovered, no slipway), not the stale state of the previous game or of the title. One private helper in `pad_nav.c` gives "the map mode now" from `map_mode` and that pending check. Reason: the generator and the metrics tool classify a frame before its step; without this the first map frame reads as jump mode, the generator taps a dead □ there, and the metrics attribute that frame and press to jump mode (prototype). The game's own output (crosshair, presses) is unchanged by this, since `finish_step` already runs after the reset.

**D-3 — One candidate test, no allocation.** `pad_jump` gains `pad_jump_any_candidate(nodes)`: 1 when some entry of the node list passes `pad_target_is_candidate`, 0 for an empty or NULL list; it allocates nothing. `pad_nav` (D-1, D-2), `pad_metrics` (D-6) and the generator (D-4) use it, so the three cannot disagree on "nothing to jump to".

**D-4 — The generator learns the new start and the guard.**
- `enter_aim` is unchanged: through D-2 it sees aim mode on the first map frame of a game with nothing known and presses no □ (this is "`enter_aim` learns the new start").
- `leave_aim` (used by commands that need jump mode: `colonise`, `slipway`, `menu`, `jump`) first waits, up to `REVEAL_FRAMES_MAX` idle frames, for a candidate, then taps □ and requires jump mode, failing with a message naming the missing candidate otherwise.
- `run_jump` leaves aim mode itself (through `leave_aim`) after its wait for the target, instead of requiring jump mode within 2 frames, so `jump <id>` right after a fold works.
- The trailing leaves of `probe`, `fold` and `build` become "leave if able": they tap □ only when a candidate exists, otherwise they stay in aim mode and press nothing.
- `fold` releases its × with ○ only when a candidate exists on the release frame; otherwise the release frame is idle.
- The aim gate, the refusals (chord, empty cone, dead ○ unless `back --required`) and every other command are unchanged.

**D-5 — The probe check reads the release, not 10 idle frames.** A refused probe is marked dead by `targeting_confirm` on its release frame (`probe.c`), so `probe` checks acceptance right after its × tap as "one more live, not-dead probe than before the tap" and drops its `PROBE_REFUSAL_FRAMES` idle wait (the constant goes). In the new flow those 10 frames would be aim frames that measure nothing. Precedent: step 13's `34358bc` cut aim frames around folds and pans in the generator.

**D-6 — `pad_metrics`' required group gains the new no-ops.**
- The view gains one group fact, `no_candidate`, read by `read_group_facts` (judging only) in aim mode through D-3; like the other group facts it moves neither change test.
- A dead □ press is required when the mode before it is aim and `no_candidate` is set; any other dead □ stays in the other group.
- A dead ○ press is required, in addition to today's root rule, when the mode before it is aim, `no_candidate` is set and no box or message box is shown (with one shown, ○ closes it and is live; if it is somehow dead it stays "other").
- `required no-ops` keeps counting ○ only (it must keep equalling the generator's count of `back --required`). The verdict rules are unchanged (they fail on other-group dead ○, never on □).
- `PAD_METRICS_PROOF_REQUIRED` becomes three lines — f16 □ aim, f17 ○ aim, f293 ○ jump — and `pad-metrics-check` compares the multi-line group (for example both sides joined with a separator); its message and the Makefile legend name the three presses.

**D-7 — The e2e replays: keep frame counts and later frames.** Every □ that lands in aim mode with no candidate at a game's start becomes an idle frame (frame counts, later frames and captures unchanged). `pad_probe`'s f16 ○ / f17 □ become f16 □ / f17 ○: both do nothing, they stay as the e2e situation of the rule (the old situation "○ back to jump mode with nothing hovered" goes with its mechanic), and the order lets the proof script mark the ○ `back --required` (`back` is ○ then an idle frame, which f18 already is). `pad_research`'s duplicate f28 ○ / f29 □ become idle frames (pad_probe and its prefix sharers carry the situation). Resumed games that start with candidates keep their f13 □ (it still enters aim mode from jump mode).

**D-8 — Inventory of the 17 replays** (json edit; capture and hash; `.txt`). Frame counts are unchanged everywhere, so `tools/e2e_long_exceptions.txt` and every "N frames." stay.

| Replay | Start | JSON edit | Capture / `.sha256` | `.txt` change |
|---|---|---|---|---|
| `pad_probe` | new game, aim | f13 □ → idle; f16 ○, f17 □ → f16 □, f17 ○ | f16 changes (crosshair stays); re-baseline | f13 idle; f16–17: □ and ○ do nothing, nothing known (f15–17 identical) |
| `pad_colonise` | new game, aim | same prefix as `pad_probe` | f16; re-baseline | header; f16–17 required no-ops beside f293; prefix wording |
| `pad_slipway` | new game, aim | same prefix | f16; re-baseline | header and prefix wording |
| `pad_empty_planet` | new game, aim | same prefix | f16; re-baseline | header and prefix wording |
| `pad_restart_resume` | new game aim; resume f132 has p25/p47, jump | same prefix; f133 □ kept | f16; re-baseline | header: the resumed game has known planets, so it starts in jump mode and f133 □ enters aim mode |
| `pad_research` | new game (X f24), aim | f25 □ → idle; f28 ○, f29 □ → idle | f28; re-baseline | header; f25 idle; f28–29 idle |
| `pad_tech_costs` | resume, nothing known, aim | f29 □ → idle | f13–16, f28 (crosshair); re-baseline | f13: aim mode, the record knows no object; f29 idle; "the cheat is typed in jump mode" kept (it is, after f97) |
| `pad_game_end` | new game, aim | none | f13–92 (crosshair); re-baseline | stays in aim mode (not jump); × held from f90: f90 is the release frame of f89's year-skip click, so the press engages nothing, and held it is no press edge — the implementer confirms this reading with `make state`/dump before writing it |
| `pad_build`, `pad_score`, `pad_message_box`, `pad_end_turn` | new game, aim | f13 □ → idle | identical; hash unchanged | "square on f13 leaves it for aim mode" → the game starts in aim mode, f13 idle |
| `pad_pause_restart` | two new games, aim | f13 and f39 □ → idle | identical | same, for both games |
| `pad_title` | new game, aim | f19 □ → idle | identical | same, f19 |
| `pad_bankrupt`, `pad_planet_projects`, `pad_resume_at_start` | resume with objects, jump | none (f13 □ still live) | identical | only the "every game starts" sentence: a resumed game with objects starts in jump mode |

Re-baselined hashes: 8 (`pad_probe`, `pad_colonise`, `pad_slipway`, `pad_empty_planet`, `pad_restart_resume`, `pad_research`, `pad_tech_costs`, `pad_game_end`). JSON edits: 13 replays. `.txt` edits: all 17. No `.end-state` changes; `pad_resume_at_start`'s `initial_save` is checked to still equal `pad_research`'s last record (prototype: equal).

**D-9 — Fixtures, proofs and the worst-case games.**
- Fixtures cut from replay prefixes follow them: `grabs_refused`, `slipways_refused`, `wormhole_abandoned`, `wormholes` (pad_probe prefix: f13 idle, f16 □, f17 ○) and `probes_refused` (f13 idle). `title_pick_*` press no □ and stay. `test_live_slots` pins are re-derived if they move (prototype: none move).
- Frames proofs: `pad_probe.frames.pad` becomes `title 12 reasonable`, `idle 3`, `tap square`, `back --required`, then the rest without its old `idle 1` before `hold up 6`; `pad_end_turn.frames.pad` replaces `tap square` + `idle 2` with `idle 3`. `pad_colonise` and `pad_restart_resume` include `pad_probe` and need no edit. Refusal proofs: `aim_budget.refused.pad`'s `tap square` becomes `idle 1` (same 21 frames, still refused); `dead_circle.refused.pad` stays (its ○ is now in aim mode with nothing known, still dead, still refused); `empty_cone.refused.pad` stays.
- `menu <id> [dwell]`: an optional third token, idle frames with the menu open after its row move and before its ○; refused above 60 frames ("dwell at most 60 frames", step 13's ruling). A refusal proof `menu_dwell.refused.pad` pins the limit. The command list where the generator documents its commands gains it.
- `worst-case/full_game.pad`, two edits, both under step 13's padding ruling: (1) the first `score` line moves from after `colonise p33 strip` to right after `fold 168 238`, so that clock-free visit fills part of the first reveal wait instead of idling in aim mode (no line added); (2) `menu s10` moves from after `label pause-branch` to before it, with a dwell D, the smallest multiple of 5 in 0..60 that leaves every one of the four games at or under 24.9% (prototype: D = 55; it is the only object left unbrowsed in the shared part, and ≤ 60 frames of dwell). `full_game_new_game.pad`, `full_game_pause_idle.pad` and `full_game_pause_restart.pad` are unchanged. The four `.json` are regenerated by `make pad-author`; their `.end-state` references must pass unchanged (prototype: they do, every variant).
- If D = 60 leaves a game at or above 25%, the chunk stops there and the miss goes to a fresh `guideline` agent (the plan's fallback), with the measured table. If D = 60 leaves all four under 25% but one above 24.9%, D = 60 is accepted and the step note records the margins.

**D-10 — README.** In Controls, the jump-mode parenthesis "every game starts here" becomes "where a game starts once an object is known"; the aim-mode line gains: "A game with nothing known yet starts here; until an object is known, □ and ○ do nothing." Nothing else in the section changes. The step note lists the unit test and e2e frame behind each changed sentence (step 15's method).

**D-11 — The cheat (rule 32).** This step changes what ○ (and □) do only by making them inert in one state; a lone ○ already resets the chord matcher, so ordinary play still cannot type four ×+○ chords. The chord consistency test and the deliberate-route test keep passing (rewritten onto the new start helper only); cheat counts in every `.end-state` are unchanged.

## Implementation Chunks

Each chunk ends on a commit with `make test-all` green.

1. **Groundwork, no behaviour change.** `pad_jump_any_candidate` (D-3) with its tests; `pad_metrics`' `no_candidate` fact and the extended required rule (D-6, except `PAD_METRICS_PROOF_REQUIRED`) with its tests — today no dead □/○ happens in aim mode with nothing known (they leave to jump mode), so every report is byte-identical; the generator's probe check on the release frame (D-5) and the `menu` dwell token with its refusal proof (D-9). Effect: the four `worst-case/*.json` lose the 10 refusal-check frames after their probe and are regenerated; every e2e hash, every `.end-state`, every frames proof and the `pad_colonise` metrics report unchanged.
   Checks: `make pad-author-proof`; `make pad-author` then `git diff --exit-code worst-case/` (byte-identical to the committed regeneration); `make end-state` (all 22); `make pad-metrics-check`; `make pad-metrics-worst-case` (aim share drops about 0.15 points on each game); `make full-game`, `make save-worst-case`, `make save-fuzz`, `make worst-case-peaks` (same two short peaks as step 13); `make unit-run-test_live_slots`; `make coverage-compare`; `make test` not slower.
2. **The rule and everything it breaks, one commit** (the rule changes 8 captures, the frames proofs, the generator's output and the worst-case games at once, so no green split exists). In TDD order: the `pad_nav` start rule, the □/○ guard and the accessors (D-1, D-2) with the `test_pad_nav` helpers and tests (Testing Decisions); the generator's exit rules (D-4); the 13 replay JSON edits, 8 re-baselined hashes, 17 `.txt` (D-7, D-8); the 5 fixtures and the proof scripts (D-9); `PAD_METRICS_PROOF_REQUIRED`, the check's comparison and the legend (D-6); `full_game.pad`'s two edits and the four regenerated games (D-9).
   Checks: `make end-state` (all 22 unchanged); `make pad-author-proof`; `make pad-author` then `git diff --exit-code worst-case/`; `make pad-metrics-check` (required group exactly f16 □, f17 ○, f293 ○; totals equal the generator's); `make pad-metrics-worst-case` (verdict ok on all four, each aim share recorded); `make unit-run-test_live_slots`; `make rng-sync-probe` (`pad_colonise` changed); `make coverage-compare` (no new unhit baseline line; prototype: the same 3); `make full-game`, `make save-worst-case`, `make save-fuzz`, `make worst-case-peaks` (largest record and `FULL_GAME_LIVE_ENTITIES` still within their limits); the old and new `.txt` compared for lost situations; `make test` and `make e2e` not slower, like for like.
3. **README** (D-10). Docs only; no pad input or replay is touched, so section 2's extra checks do not apply.

## Testing Decisions

- Good tests here drive `input_step` frames through the public `pad_nav` accessors, the played frame and the world, never `pad_nav_t` fields; `pad_metrics` tests stay on hand-built views (`start`, `run`, `dead_circle_group` pattern); the pure candidate test uses hand-built node lists as `tests/test_pad_jump.c` and `tests/test_pad_target.c` do.
- **`tests/test_pad_jump.c`:** `pad_jump_any_candidate` is 0 for NULL and for an empty list, 0 for unknown planets and for a dead known planet, 1 with one known planet, 1 with one settled structure.
- **`tests/test_pad_metrics.c`:** a dead □ in aim mode with `no_candidate` is required; a dead □ in aim mode with a candidate, and one in jump mode, are "other"; a dead ○ in aim mode with `no_candidate` is required and leaves the verdict ok, and the existing aim case without `no_candidate` stays "other" (verdict fails); with a message box shown it stays "other"; `no_candidate` moves neither change test (extend `test_the_group_facts_move_neither_change_test`); `required no-ops` counts the required ○ and not the □.
- **`tests/test_pad_nav.c`, helpers:** `start_game` stops tapping □ and asserts aim mode with the crosshair at the centre; `start_game_in_jump_mode` goes (its contract, a new game in jump mode with nothing hovered, is the removed mechanic). Its 73 call sites are rewritten one by one, each keeping its intent, with one of: (a) build the layout, then `enter_jump_mode` (□ from aim mode, asserts jump mode on the object nearest the centre) — the default; (b) `begin_game` (title and `cart_start_game`, no frame run), build the layout, then settle — a game that appears with candidates, for the seeding tests; (c) for tests that need jump mode with nothing hovered, (a) then kill every candidate. Timing-sensitive tests that relied on `start_game`'s two □ frames get explicit idle frames.
- **`tests/test_pad_nav.c`, new or rewritten tests** (no deletion: the two start tests are rewritten, listed in the step note under rule 34):
  - `test_a_new_game_starts_in_jump_mode_with_nothing_hovered` → a new game starts in aim mode: crosshair drawn, played point at the centre, nothing hovered, no press; the D-pad pans; × places the probe.
  - `test_a_new_game_after_aim_mode_starts_in_jump_mode` → a new game after a jump-mode game with objects starts in aim mode.
  - □ and ○ do nothing in aim mode with no candidate (mode, hover, camera, world, mouse unchanged over several presses), and each goes back to jump mode on the nearest object once one is known.
  - A fold's reveal leaves the player in aim mode.
  - ○ in aim mode with nothing known still closes the year box.
  - A resume with a colony starts in jump mode on the colony (prior art: `test_circle_after_a_resume_does_nothing`'s pause, restart and resume path).
  - A resume of a record with nothing known starts in aim mode (prior art: `start_title_with_a_save`).
  - The accessors report aim mode on the frame the game appears, before any pad step (D-2), and jump mode for a game that appears with candidates.
  - `test_jump_mode_seeds_on_the_colony_over_a_nearer_known_planet` and `test_jump_mode_seeds_on_the_first_object_that_appears` keep their intent through pattern (b) and (c) respectively.
  - The held title × places no probe (existing test, kept on the new start).
- **Pinned values (step 2a inventory):** `test_live_slots` (prototype: passes with the 5 fixtures and 13 JSON edits, no pin moves), `test_state` (`pad_tech_costs`, `pad_research` cut at 45, `pad_build`: unchanged, frames after the edits are identical), `test_rng_sync` (`pad_colonise` windows f98–198 and f292–444 unchanged), `test_exit_queue`'s `FULL_GAME_LIVE_ENTITIES` 150 (re-checked by `make full-game`), the save-load, fuzz and resume-check seeds (untouched). Each is re-derived only if it moves, keeping its intent.
- **E2e:** the replays are the source of truth; `make e2e` checks the 17 hashes, `make end-state` the 22 references.

## Out of Scope

- Jump mode losing every candidate in the middle of a game (D-1): unchanged.
- An automatic switch to jump mode on the first reveal (rejected by the user).
- Any tuning value of section 5, the cone rule's stranded objects, the post-resume HUD hover issue.
- The follow-ups listed in the plan (pointer guard, `save_state.c:888`, `pad_metrics` scratch-zone notes, `pad_author` usage text) — except that the usage or command documentation gains the `menu` dwell token.
- The step note itself (plan-pilot writes it); this PRD lists what it must record.

## Further Notes

- The step note records: `pad_nav.c` lines before (1011) and after; `pad_nav_t` 23 → 23 fields; `test_pad_nav` 224 → n tests, the two rewritten start tests named under rule 34; the 8 re-baselined hashes and the 13 JSON edits; the situations removed with their mechanic (○ back to jump mode with nothing hovered at `pad_probe` f16 and its sharers, `pad_research` f28; □ from jump mode with nothing hovered at every new game's start; `pad_game_end` in jump mode with nothing hovered) and the one added (□ and ○ inert with nothing known, `pad_probe` f16–17); the four games' frame counts and aim shares from `make pad-metrics-worst-case`; the dwell D chosen; the re-authoring time.
- The prototype's aim-share table is the expectation, not the acceptance: the acceptance is the regenerated games' verdict.
- Prefer `make e2e-ref` to locate any divergence beyond the frames listed in D-8: a divergence on any other frame means a replay edit was not a no-op and must be undone.
