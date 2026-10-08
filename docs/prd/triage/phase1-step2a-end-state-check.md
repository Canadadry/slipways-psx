---
title: "End-state check against a committed reference"
description: "Plan 12 lets re-authored replays change frames but never the game they reach, and nothing checks that mechanically yet: the end state is only judged by eye."
status: needs-triage
---

## Problem Statement

Plan 12 re-authors all 17 pad replays and the `worst-case/` games several times, once for each mapping change. Its guardrail says a re-authored replay must reach the same game end state as before, and that its hash is re-baselined only then. Today nothing checks this mechanically:

- `make state` writes a replay's last-frame state to `tmp/state/`, which is gitignored. The "before" state is gone as soon as anyone reruns it, and it is not tied to any commit.
- A state file mixes game facts (difficulty, planets, slipways, structures, techs, counts) with facts a re-authored replay changes without changing the game: the camera, each object's screen position and visibility, zone and cap peaks. A plain `diff` of two state files fails on every re-authoring, so the only judge left is a human reading two dumps.
- Re-authored inputs could type the cheat sequence (× × ○ ○ × × ○ ○) by accident and silently set credits to 200 and science to 99. Four e2e replays type it on purpose (`pad_bankrupt` once, `pad_planet_projects` 3 times, `pad_research` and `pad_tech_costs` once each), and the four `worst-case/` full games type it 24 to 30 times each. Nothing counts activations.
- Later steps must re-derive every test value pinned inside a replay, and nobody has listed those tests.

For the developer (or the unattended agent) re-authoring a replay in steps 5 to 13, the symptom is that there is no yes/no answer to "did my new inputs reach the same game?".

## Solution

A committed, normalized **end-state reference** sits next to every replay, e2e and `worst-case/` alike. It holds only the game fields and the replay's cheat activation count. One new make target, `make end-state`, replays each file, normalizes its last-frame state the same way and diffs the result against the reference. It fails on any difference, on a replay that stops, and on a replay with no reference. The **reference commit** is the commit of this step that adds the references: `git show <that commit>:<reference>` gives it back at any later time, and re-running the baseline at that commit regenerates identical files.

The replay tool now counts cheat activations and writes them as one more line of the state file, so `make state` reports them too.

The step ends with the plan's verification: every replay passes at the reference commit, a cut-short replay fails, and the step note holds the inventory of every test and tool that loads a replay and pins values from it.

## User Stories

1. As a developer re-authoring a replay, I want one command that says whether my new inputs reach the same game end state as the reference, so that I never judge it by eye.
2. As a developer, I want the reference to be committed in git, so that it survives `rm -rf tmp/` and is the same on every machine.
3. As a developer, I want the reference tied to a known commit, so that I can trace any reference line back to the code that produced it.
4. As a developer, I want the reference regenerated at the reference commit to be byte-identical to the committed one, so that the baseline is reproducible and not hand-edited.
5. As a developer, I want the camera position ignored, so that a different camera path never fails the check.
6. As a developer, I want each planet's and structure's screen position and visibility ignored, so that a different final camera never fails the check.
7. As a developer, I want zone, budget and cap lines ignored, so that a replay taking more or fewer frames or particles never fails the check.
8. As a developer, I want the difficulty compared, so that a re-authored title path that picks another difficulty fails.
9. As a developer, I want each planet's id, state, class, type and world position compared, so that a different revealed type, a lost colony or a moved placed structure fails.
10. As a developer, I want every placed slipway compared (both ends and its kind), so that a missing, extra or different slipway fails.
11. As a developer, I want every slipgate and infragate compared (id, kind, world position, link count), so that a different structure network fails.
12. As a developer, I want the invented techs compared, so that a different research route fails.
13. As a developer, I want the counts (planets, colonies, slipways, structures, techs, nodes) compared, so that a summary difference is visible at a glance.
14. As a developer, I want the year, the month and the resources compared exactly when they do not move with frame timing, so that the check is as strict as the game allows and no stricter.
15. As a developer, I want the order of planet, slipway, structure and tech lines ignored, so that building the same network in another order does not fail.
16. As a developer, I want the number of cheat activations of each replay compared, so that an accidental cheat (or a lost deliberate one) fails the check.
17. As a developer, I want `make state` to show the cheat activation count too, so that I can read it without running the check.
18. As a developer, I want a replay that stops (p8_fatal) to fail the check, so that a crash never passes as "same end state".
19. As a developer, I want a replay without a reference to fail the check, so that a new or renamed replay cannot slip past it.
20. As a developer, I want a state line of a kind the check does not know to fail it, so that a field added to `make state` later is classified on purpose and not compared or ignored by accident.
21. As a developer, I want the check to print the replay name and the differing normalized lines, so that I see at once what changed.
22. As a developer, I want the check to examine every replay before failing, so that one run lists every broken replay.
23. As a developer, I want to check one replay alone (`EVENTS=`), so that the re-authoring loop stays short.
24. As a developer, I want to check a scratch events file against another replay's reference (`REFERENCE=`), so that I can try inputs in `tmp/` before overwriting the real replay.
25. As a developer, I want the baseline target to write only missing references and never overwrite one, so that a changed end state can never be re-baselined by running a command.
26. As a developer, I want the check outside `make test` and `make e2e`, so that the fast loop does not get slower.
27. As a developer, I want the `worst-case/` games covered by the same check, so that step 13 can judge their re-authoring the same way.
28. As a developer, I want the title-only `worst-case/` game covered too, so that its "no game" state is pinned like the others.
29. As a developer, I want a cut-short replay to fail the check, so that I know the check can fail.
30. As a developer, I want a replay with idle frames appended to pass the check, so that I know frame count alone does not fail it.
31. As a developer, I want the normalizer tested on hand-written state files in `make test`, so that its classification cannot drift unnoticed.
32. As a developer in later steps, I want the step note to list every test and tool that loads an e2e or a `worst-case/` game and pins values in it, with what it pins, so that I know what to re-derive when I re-author a replay.
33. As a developer, I want the Makefile legend to describe the new targets, so that I find them where every other target is described.
34. As a plan reviewer, I want the step note to record which clock and resources fields are compared and why, so that the plan's guardrail is applied with known fields.

## Implementation Decisions

- **Cheat activation count.** The replay tool sums what the input step returns for each frame (the cheat step already reports a firing). After writing the state file, it appends one line, `cheat activations <n>`. It is not written on the p8_fatal exit path, because a stopped state fails the check anyway. The state module, the input step and the cheat module are unchanged. The line is fed by the raw pad buttons, so it depends only on the inputs, which is what the guardrail wants. `make state` (one file, or the whole campaign) gets the line for free. The peaks script reads only zone, cap and budget lines and is unaffected.
- **A new tool script, the end-state normalizer and check**, next to the peaks script and in its style (POSIX sh and awk, `LC_ALL=C`). It has three entry points:
  - *normalize* a state file to stdout;
  - *check* a list of `<events> <state> <reference>` triples: it examines every triple and exits 1 if any replay is stopped, has no state, has no reference, has an unknown line, or differs. Each failure prints `end-state: <events> <reason>` followed by the differing normalized lines (a unified diff of the two normalized texts, capped at 40 lines per replay). A passing run prints `end-state: <n> replays match their reference`;
  - *baseline* a list of `<events> <state> <reference>` triples: it writes the normalized state to each reference that does not exist yet, refuses (exit 1) on a stopped state or an unknown line, and leaves every existing reference untouched, saying so.
- **The field table (the core decision of this step).** Every state line kind is listed. The normalizer fails on any line kind not in this table:

  | State line | Kept in the reference | Dropped |
  |---|---|---|
  | `clock year <y> month <m>` | whole line (see the clock and resources decision) | — |
  | `difficulty <name>` | whole line | — |
  | `resources credits / science / income <v>` | whole line (see the clock and resources decision) | — |
  | `techs <name>` | whole line | — |
  | `camera world <x> <y>` | — | whole line |
  | `planet <id> <state> <class> <type> world <x> <y> screen … visible …` | through `world <x> <y>` | `screen` and `visible` fields |
  | `slipway <start> <end> <kind>` | whole line | — |
  | `structure <id> <kind> world <x> <y> screen … visible … links <n>` | id, kind, world, links | `screen` and `visible` fields |
  | `count <name> <n>` (planets, colonies, slipways, structures, techs, nodes) | whole line | — |
  | `cheat activations <n>` | whole line | — |
  | `zone …`, `budget …`, `cap …` | — | whole line |
  | `stopped …` | — | check fails: the replay stopped |

  The normalized text is sorted (C locale), so line order never matters. World positions stay as `make state` prints them (two decimals of the 16.16 value), with no tolerance: same game, same positions. Slipway direction (start, end) is kept as printed. Placed structures keep their ids (`s1`, `s2`, …) as printed. Relaxing either, or adding a tolerance, belongs to step 2b's relaxed comparison, not here.
- **Clock and resources: compared, unless measured to move with frame timing.** The plan ignores "the clock and resources fields that move with frame timing". Reading the code at the base of this step finds none that do: the clock advances only when a probe or fold is confirmed, when a project lands, when a slipway is placed, and at an end turn. Credits and science change only by spending, at a new year, and by the cheat. Income is recomputed from the colonies and slipways. So, by the plan's own criterion, all five fields (year, month, credits, science, income) are compared. Chunk 1 checks this with a one-off measurement in `tmp/`: for a few replays (at least `pad_colonise`, `pad_research` and `pad_tech_costs`), insert 30 idle frames at three idle points (a frame and the one before with no button held) and append 60 idle frames at the end. Then compare the clock and resources lines with the original's. A variant whose planet, slipway or structure lines also changed is discarded: that is a random-stream or timed-box effect, which is step 2b's subject. A clock or resources field that changes in a variant whose other compared lines are equal moves to the "dropped" column. The step note records the measurement and the final table. Rejected: dropping all five as the plan's wording might suggest. That would let a re-authored replay spend an extra probe, or skip a year, and still pass, although its game differs.
- **Reference files.** One per replay, committed next to it and named after the events file with an `.end-state` suffix, the way `.json.sha256` is (for example `pad_probe.json.end-state`, `full_game.json.end-state`). There are 22: the 17 e2e and the 5 `worst-case/` games. They hold the normalized text only, so a later change to zones, caps or the camera never touches them. The e2e companion check (`make e2e`'s limits script) does not require the file: the end-state target is the one that fails without it. Rejected: building the reference commit in a git worktree on every check, the way `e2e-ref` does. It is slower, needs the old tree to build, and gives the same answer as a committed file.
- **Make targets** (both outside `make test`, `make e2e` and `make test-all`, both in the Makefile legend):
  - `make end-state`: with no `EVENTS`, it replays every file `make state` replays (the `worst-case/` games, then every e2e), reusing the per-replay state runs of `make state`, and runs the check on all 22 triples. With `EVENTS=<file>`, it checks that file alone, against `<file>.end-state`, or against `REFERENCE=<reference>` when given (a scratch file in `tmp/` against a real replay's reference).
  - `make end-state-baseline`: same replays, the script's baseline entry point. It writes only missing references. To re-baseline one on purpose, a human deletes its reference first, which shows in the diff and is covered by the plan's "never re-baseline a replay whose game end state changed".
- **The reference commit** is the commit of chunk 2 that adds the 22 references. The step note names its hash. Reproducibility is checked before committing: delete every reference, run the baseline again, and expect no difference from the files about to be committed.
- **Preconditions from step 1b** (see Step 1b outcome). The baseline is taken on the tree as committed when this step starts, after step 1b. If a `worst-case/` game stops there, the baseline target refuses it and no reference is written for it. The fallback is step 1b's own rule, applied in a separate commit before chunk 2: fix that game's input minimally until `make full-game` passes, with a line in the step note. Nothing else of step 1b is redone here.
- **The inventory.** The implementer compiles it from the tree at implementation time (step 1b moves pinned frames and values), grepping `src/`, `tests/`, `tools/`, `docs/` and the Makefile for the e2e directory, `worst-case/` and every replay name. It goes in the step note as one line per test or tool: what it loads, and which values it pins (frames, counts, positions, ids, sizes), or "loads but pins nothing". The starting list, read at `236b93f` (after step 1b), is in Further Notes.

## Implementation Chunks

1. **Cheat count, normalizer, check and baseline, with no reference yet.** The replay tool's `cheat activations` line. The tool script with its three entry points and the field table. The unit test of the script, and the test-state assertions on the cheat line. Both make targets, plus the legend. The clock and resources measurement in `tmp/`, which settles the table before anything is baselined. With no reference committed, `make end-state` fails with "no reference" on every replay. That is expected, and the target is outside `make test-all`. `make test-all` passes and every `.json.sha256` is unchanged (the replay's capture does not change, only its state file gains a line). One commit.
2. **The reference commit.** Run `make end-state-baseline`, check reproducibility (delete, re-baseline, no difference), run `make end-state` (22 pass), and commit the 22 references. The verification in Testing Decisions is run on this tree. The step note (the reference commit's hash, the final field table with the measurement, the cut-short and idle-append results, the cheat count of every replay, and the inventory) is the implementer's report. One commit, plus any `worst-case/` precondition fix before it.

## Testing Decisions

- A good test drives the public surface (the script's entry points on files, the replay tool's state file) and asserts on exit status and printed text. It never relies on the script's internals.
- **New unit test of the end-state script**, one file named after it, run by `make test`. It has no replay, hand-written state files only, and the peaks script's test as prior art. It must run in well under 50 ms. Cases:
  - two states differing only in camera, screen, visible, zone, budget and cap lines: check passes;
  - the same planet, slipway, structure and tech lines in another order: check passes;
  - a different planet type, world position or state, an extra slipway, a different structure link count, a different tech, a different count, a different difficulty, a different cheat count, a different year or credits (fields compared by the table): check fails, naming the replay and showing the line;
  - a state starting with `stopped`: check fails; baseline refuses;
  - a missing state or a missing reference: check fails;
  - an unknown line kind: normalize, check and baseline all fail;
  - several triples, one bad: every triple is examined, and the exit is 1;
  - baseline writes a missing reference and leaves an existing one byte-identical.
  If the chunk 1 measurement moves a clock or resources field to the dropped column, its case moves from "fails" to "passes" accordingly.
- **Test-state additions.** On the `pad_research` cut after its cheat frame (already in that test), assert `cheat activations 1`. On the full `pad_tech_costs` replay (already in that test), assert that the line is present with its actual count. Both are pinned values, and they join the inventory.
- **Plan verification, run on the reference commit and recorded in the step note** (not committed as tests, since they replay e2e files):
  - `make end-state`: every one of the 22 replays passes;
  - `pad_research` cut to its first 45 frames (the cut the test-state test already uses; its cheat fires at f44), checked with `REFERENCE=` against `pad_research`'s reference: fails, with differing techs, slipways and structures, while its cheat count matches. This shows that fields other than the cheat count are compared;
  - `pad_research` with 60 idle frames appended, against the same reference: passes, which shows that frame count alone fails nothing.
- `make test-all` passes, every `.json.sha256` unchanged, and `make test` and `make e2e` no slower beyond the new sub-50 ms unit test running in parallel.

## Out of Scope

- Step 2b: whether a reference can still be reached with another camera path and timing, the relaxed comparison and its fields, the `worst-case/` acceptance verdict, and the Select-hold year budget.
- Re-authoring any replay or `worst-case/` game, except the minimal precondition fix above.
- Any pad mapping change (steps 3 onwards).
- Re-deriving pinned test values: this step only lists them. Steps that re-author replays re-derive them.
- Changing what `make state` prints apart from the new cheat line, and changing the peaks rule.
- Adding the check to `make e2e` or `make test-all`.

## Step 1b outcome

Step 1b is committed (`938e864`, `5a5520b`, its step note at `236b93f`). This PRD was first written with hypotheses about it. They were checked against that tree and are replaced by these facts:

- **Done before this step.** All of step 1b is committed and every e2e hash is re-baselined: `pad_title`, `pad_score`, `pad_message_box`, `pad_game_end`, `pad_build`, `pad_end_turn`, `pad_pause_restart` and `pad_restart_resume` re-baselined; `pad_probe`, `pad_colonise`, `pad_slipway`, `pad_empty_planet`, `pad_bankrupt`, `pad_planet_projects`, `pad_research` and `pad_tech_costs` re-authored; `pad_resume_at_start` re-cut. The references of this step are taken on that tree, after the stream split.
- **`make state` output is unchanged.** Step 1b touched neither the state module nor the replay tool, so the line kinds and fields are those of the field table above. The new `cheat activations` line is the only addition, and this step adds it.
- **Save format unchanged.** It is still version 1, and the visual stream is not saved. `pad_resume_at_start`'s re-cut `initial_save` decodes. The visual stream appears in no state, save summary or save check output.
- **Replay set unchanged:** 17 e2e and 5 `worst-case/` files, so there are 22 references.
- **Cheat activations per replay** (counted on the committed inputs with the cheat step's rule, to be confirmed by the new line): `pad_bankrupt` 1, `pad_planet_projects` 3, `pad_research` 1, `pad_tech_costs` 1, every other e2e 0; `full_game` 30, `full_game_new_game` 30, `full_game_pause_idle` 24, `full_game_pause_restart` 24, `title_idle` 0. The plan's "10 replays type the cheat" does not hold: the other six `.txt` files only mention the cheat. The worst-case counts are their reference values now. Plan section 4 wants zero after step 13, and that is a step 13 matter.
- **`pad_research` cut.** The test-state cut is now 45 frames (the cheat fires at f44, title X at f24), so the cut-short verification above uses 45 frames.
- **The `worst-case/` games run clean but collapsed** (`make full-game`, `make save-worst-case`, `make save-fuzz` and `make rng-sync-probe` pass). `full_game` reaches year 3421 with 2 colonies (it was 3426 with 27), `full_game_pause_idle` 2 colonies (27 before), and `full_game_new_game` 2 (0 before). Their peaks dropped. By the `guideline` decision recorded in step 1b's note, they are not re-authored before step 13, and step 2b chooses how they are judged (the `8c4bbf4` peak rule, not their step 2a end state). This step still baselines and checks them like the e2e, because the plan says the reference covers them. Their references record the collapsed games, and whether later steps are held to them is step 2b's verdict. The precondition fallback (a stopped `worst-case/` game) is not needed.
- **Idle frames and the measurement.** Step 1b moved every per-frame visual draw to the visual stream, so the clock and resources measurement is expected to give clean variants. Any variant whose planet lines still change is discarded and noted for step 2b, as decided above.
- **Pinned values re-derived by step 1b**, so the inventory in Further Notes is read at `236b93f`.

## Further Notes

- **Why committed references rather than a worktree replay.** The plan asks for a baseline "reproducible from git, not from the gitignored `tmp/state/`". A committed normalized file is in git, readable in a review diff, and regenerable at its commit (chunk 2 proves it). It costs nothing at check time. The worktree approach of `e2e-ref` would rebuild the reference commit on every check, and it breaks as soon as that tree stops building on a newer toolchain.
- **Strictness that step 2b may relax.** World positions without tolerance, slipway direction and structure ids are kept exactly. A re-authored replay that builds the same slipway from the other end, or places a structure one pixel off, fails. That is deliberate: "same game end state" is the guardrail, and step 2b defines the relaxed comparison if references prove unreachable.
- **Starting inventory** (read at `236b93f`, after step 1b; the implementer re-checks it at implementation time):
  - the live-slots test: loops over every e2e (live slots after each frame, idle windows that must not grow the generation, `pad_count >= 11`). It pins frames of `pad_pause_restart` (f1 and f25 to f39: restart, title, the challenging press, releases, 5 × 12 help particles), `pad_game_end` (f21 and f69), `pad_tech_costs` (f546, f555, f565, f570, and planet `p54`'s kept route), `pad_message_box` (f115 to f121), `pad_empty_planet` (f98 and f133), and `pad_planet_projects` (f171, f172, f255, f256: the harvest burst and its 140 particles). It also uses its four fixtures (refused grabs near `p47`, refused slipways, wormholes, refused probes), and replays `pad_pause_restart` and `pad_planet_projects` through the replay tool with the fill file;
  - the test-state test: `pad_tech_costs` (difficulty, nodes 90, planets 90, slipways 6, colonies 6, structures 0, the `p31` and `p2` lines with world and screen, the slipways `p54 p31` and `s2 p31`, camera `127 234`, two techs, a message box peak, and the save summary lines). It also checks `pad_research` cut at 45 frames (credits 200, science 99), and gains the cheat lines of this step;
  - the rng-sync probe (`make rng-sync-probe`): `pad_colonise`, burst case save f98 to last f198 with the harvest on `p12`, quiet case f280 to f344. It compares the game stream of both runs, and run A's visual stream must move over the window;
  - the exit-queue test: `FULL_GAME_LIVE_ENTITIES 130`, now described as the largest reset_world burst a `worst-case/` game has reached (119 with a release), with margin, to keep when those games are re-authored lower. It is a constant, not a load;
  - the save-load, save-fuzz and resume-check tests: a wormhole game seed and probe screen position re-derived by step 1b (`0x5EED2026`, probe at 90, 50);
  - `make full-game`, `make save-worst-case`, `make save-fuzz`: they load the `worst-case/` games and pin nothing but clean runs, the 8 KiB record limit and round trips. The save-format doc still quotes the largest record as 4471 bytes (`full_game`, frame 4360), but step 1b measured 3973 bytes. The doc is stale, and fixing it is outside this step;
  - `make state` and the peaks script: every replay, the 1/8 margin over each peak (the peak holder is printed, not pinned; the e2e now hold most peaks);
  - `make e2e`, `make e2e-baseline`, `make e2e-ref`, the frames check, the e2e limits script and the long-exceptions list: hashes, frame limits, and frame counts quoted in the exception reasons (`pad_slipway` 348, `pad_message_box` 190, `pad_planet_projects` 562, `pad_colonise` 345, `pad_research` 577, `pad_tech_costs` 571);
  - `make sanitize-net` and `make coverage`: every e2e, nothing pinned beyond clean runs and the coverage baseline (7613 lines hit after step 1b);
  - comments only, nothing loaded: the nil-item layout test (names `pad_colonise`) and the cart (names `pad_game_end`'s hash);
  - each e2e's own `.txt` (frames, ids, positions, credits), and the `pad_resume_at_start` fixture cut from `pad_research`'s last record;
  - after this step: the 22 end-state references themselves, and the test-state cheat lines.
- The measurement in chunk 1 is a one-off in `tmp/`, like step 1b's coverage comparison. No new make target is added for it.
