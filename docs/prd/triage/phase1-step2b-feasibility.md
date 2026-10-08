---
title: "Feasibility of re-authoring replays to the same end state"
description: "Before any pad mapping changes, nobody knows whether a replay with another camera path and timing can still reach its step 2a reference, how strict a re-author must be to get there, how the worst-case games are judged, or whether the Select hold fits the frame limits."
status: needs-triage
---

## Problem Statement

Plan 12 re-authors every pad replay once per mapping change (steps 5 to 13), and its guardrail says a re-authored replay must reach the same game end state, judged by the step 2a end-state check. That guardrail only works if the reference can actually be reached by inputs that take a different camera path and a different number of frames. Nobody has shown that yet:

- Before step 1b, adding 5 idle frames to `pad_probe` turned a revealed planet from ocean into forgeworld, because on-screen effects drew from the game's random stream. Step 1b moved them to a visual stream, but nobody has checked that the same edit is now harmless, or whether anything else that depends on frame timing still changes a game decision or a compared field.
- Step 2a's measurement (24 timing variants of 6 replays) found two timing effects. Settled structures and trading hubs **bob**: their world y drifted by up to 3 px with idle frames. Step 2a fixed that in the state writer, which now prints their y at rest, recomputed exactly from the state time, with the known limit that it assumes the bob ran on every frame since the entity settled. The other effect is still open: idle frames inserted on the title screen (`pad_build` f7) gave another galaxy, because the title still seeds the game (step 1b's seed tick). Nobody has shown that nothing else after the title moves a compared field.
- Nobody has written down the discipline a re-author follows (what must land on the same frame, at the same world point, in the same order) so that step 5's agent does not discover it by trial and error.
- If the reference cannot be reached, the plan's fallback is a relaxed comparison and a committed exceptions file read by the step 2a target. Neither exists.
- The `worst-case/` games collapsed in step 1b (27 colonies to 2). Their step 2a references record the collapsed games, and the plan now judges step 13 by the `make state` peaks of commit `8c4bbf4`, a table nobody has committed.
- The coverage guardrail ("every line hit at `6346485` is still hit") was checked by hand in step 1b. Every re-authoring step needs it again, and there is no committed script for it.
- Step 9 replaces the `end turn` tab with a 30-frame Select hold. `pad_game_end` ends 25 years in 104 frames and `pad_message_box` 20 years in 190: at 31 frames a year they cannot fit the 600-frame limit. Nobody has counted, replay by replay, which ones still fit and which must start from an `initial_save`.

For the agent re-authoring replays in steps 5 to 13, the symptom is: no proof that the target is reachable, no rules for reaching it, and no mechanical tools for the fallback, the worst-case target and the coverage check.

## Solution

A study on the current pad scheme, plus four small committed artifacts, plus one step note.

1. **The study** (one-off, in `tmp/`, nothing of it committed). Variants of `pad_probe` and `pad_colonise` are written by hand with the same game actions but other timing and other camera paths, and checked with `make end-state ... REFERENCE=` against the committed references. Controls show which changes break the reference. A temporary local instrumentation (never committed) logs every game-stream draw by frame, so each break gets a cause. The result is a verdict (reachable or not), the **re-authoring discipline**, and the list of timing-dependent fields.
2. **The bob, confirmed.** Step 2a already prints a bobbing entity's rest y. This step's timing variants confirm that no compared field of a bobbing entity moves, and test the fix's known limit (bob on every frame since settling) against a save and resume. A fix is made only if that limit is broken.
3. **The relaxed comparison**, always defined in the step note (the section 2 fallback can fire in any later step). Its code (a committed, empty-at-first exceptions file and its support in the step 2a target) is committed in this step only when the verdict says a reference cannot be reached, as the plan text says. Otherwise the first step that needs it adds that code from the definition recorded here.
4. **The `8c4bbf4` peak table**, committed as a file, the target step 13 is measured against. The step note records the verdict that the `worst-case/` games are held to it, not to their step 2a references.
5. **The coverage comparison script** in `tools/`, with a make target outside `make test-all` and a unit test in `make test`.
6. **The Select-hold budget:** the years each replay ends, counted, and the route each year-heavy replay will take (hold within its limit, or start from an `initial_save`), with the `initial_save` route tried once on `pad_game_end` against its reference.

Phase 2 starts right after, whatever the verdict.

## User Stories

1. As the agent re-authoring replays in step 5, I want proof that `pad_probe` and `pad_colonise` can reach their references with another camera path and timing, so that I start re-authoring knowing the target is reachable.
2. As that agent, I want a written discipline (what must happen on the same frame, at the same world point, in the same order), so that I do not rediscover it by trial and error on every replay.
3. As that agent, I want to know which edits are free (extra idle frames after the title, another pan route to the same camera, split or merged holds), so that I can spend frames where the new mapping needs them.
4. As that agent, I want to know which edits are not free (the title X frame, the fold point, the reveal order), so that I keep them exactly.
5. As a plan reviewer, I want the "5 idle frames in `pad_probe`" edit re-run after step 1b, so that I see step 1b removed the old ocean-to-forgeworld change.
6. As a plan reviewer, I want every remaining game-stream draw after the title listed with its trigger, so that "frame timing no longer moves game decisions" is shown, not assumed.
7. As a developer, I want step 2a's rest-y fix confirmed on this step's own timing variants, so that a re-authored replay ending at another frame is known not to fail on a wobble.
8. As a developer, I want the fix's known limit (bob on every frame since settling) tested across a save and resume, so that the `initial_save` route of the Select budget does not fail on it.
9. As a developer, I want any fix to that limit to change no reference, or to regenerate the ones it changes in the same commit with the reason in the step note, so that the no-re-baseline guardrail is broken only on purpose and visibly.
10. As a developer, I want to know whether a bobbing endpoint can change a slipway's game outcome (too far, crossed planet) depending on the frame it is built, so that this edge case is either ruled out or added to the discipline.
11. As the agent in any later step, I want the relaxed comparison's fields written down now, so that the section 2 fallback is the same in every step.
12. As that agent, I want the relaxed comparison to keep difficulty, years, the counts of colonies, slipways, structures and techs, the structure kinds and the cheat count, so that a replay whose galaxy changed still has to reach the same shape of game.
13. As that agent, I want the relaxed comparison derived from the same strict reference file, so that there is never a second reference to keep in sync.
14. As that agent, I want the exceptions file to name each relaxed replay with its first differing field and reason, so that every relaxation is visible in review.
15. As a developer, I want the step 2a target to fail when a listed replay would pass the strict check, so that a stale exception cannot linger.
16. As a developer, I want the step 2a target to fail when a listed replay is not a replay it checks, so that a typo or rename cannot hide.
17. As the agent of step 13, I want the `8c4bbf4` peaks committed as a file, so that "every peak matches or exceeds the reference" has a reference that survives `rm -rf tmp/`.
18. As that agent, I want the file to name each zone and cap as `make state` printed it then, with its peak and the `worst-case/` game holding it, so that I compare like with like.
19. As that agent, I want the zones and caps renamed or added since `8c4bbf4` listed in the step note, so that I know which rows of today's `make state` have no target.
20. As a plan reviewer, I want the step note to say plainly that the `worst-case/` games are judged by the peak table and not by their step 2a references, so that step 13's re-baseline of those five references is expected.
21. As the agent of any re-authoring step, I want one command that says which lines hit at `6346485` are still hit, which are unhit and which were deleted, so that the coverage guardrail is mechanical.
22. As that agent, I want the coverage comparison to follow lines through edits and renames since `6346485`, so that a line moved by an unrelated change is not reported as lost.
23. As that agent, I want a line that was rewritten counted as deleted, not unhit, so that the step can justify it as "replaced by the plan" in its note.
24. As that agent, I want the comparison to exit non-zero when a baseline line is unhit, so that a script can stop on it.
25. As a developer, I want the coverage baseline built once from `6346485` and cached in `tmp/`, so that the comparison does not rebuild the old tree every time.
26. As a developer, I want the coverage comparison outside `make test` and `make e2e`, so that the fast loop does not get slower.
27. As a developer, I want the coverage comparison's mapping logic unit-tested on hand-written inputs in `make test`, so that it cannot drift unnoticed.
28. As a plan reviewer, I want the coverage comparison run once at the current tree with the expected result (no unhit line, only step 1b's rewritten draw-site lines deleted), so that the tool is shown to agree with step 1b's manual check.
29. As the agent of step 9, I want the years each replay ends counted now, so that I know which replays the Select hold makes longer.
30. As that agent, I want each year-heavy replay's route decided now (hold within its frame limit, or an `initial_save` start), so that step 9 does not stop on a frame-limit failure.
31. As that agent, I want the `initial_save` route shown to keep the reference end state on one replay, so that the fallback is known to work before I need it.
32. As that agent, I want the traps of the `initial_save` route written down (a cheat typed before the save point is not counted, a living wormhole writes no record, the title path is no longer covered), so that I place the save point right.
33. As a developer, I want no new long-replay exception and no raised frame limit in the budget, so that `make e2e` does not get slower.
34. As a plan reviewer, I want every assumption about step 2a's outcome checked against the committed step 2a before anything is built on it, so that this step does not build on a guess.

## Implementation Decisions

- **Starting point.** Step 2a is committed. Its facts are in the "Step 2a outcome" section below, and every decision here uses its real names and formats.

- **The study, part 1: the old edit.** Re-run the edit that broke `pad_probe` before step 1b: 5 idle frames inserted at an idle point between the starting probe's release and the first pan toward the fold (after f27, before f29), and separately 5 at the end of the unfolding (inside f66-76). Both variants are checked against `pad_probe`'s reference with `make end-state EVENTS=<variant> REFERENCE=<pad_probe's reference>`. Expected: both pass. A failure is a finding: its first differing line and the instrumented draw log go in the step note.

- **The study, part 2: timing alone.** On `pad_colonise`, variants that change only timing, at least: idle frames inserted at five or more idle points after the title X (including inside the network-mode idle run f99-134, between the two colonies, before and after the slipway, before the year); every long hold split into shorter holds with idle frames between them that reach the same camera; and 60 idle frames appended at the end. Plus timing variants with idle frames inserted (not only appended) after a structure settles, on `pad_research` (a slipgate, a trading hub) and `pad_bankrupt` (five labs), to confirm step 2a's rest-y fix beyond its appended-frames check. And one control on the title: idle frames inserted before the title X, expected to fail as step 2a found, with the first differing line recorded.

- **The study, part 3: camera path.** On `pad_colonise`, variants that reach the same camera at every press that acts on the map (the starting probe, the fold, each triangle, each square, each X on a planet or in space) by another route: Right before Up instead of Up+Right, an overshoot and back, 1 px taps instead of holds. Then one combined variant, timing and path together. Every variant is checked against `pad_colonise`'s reference.

- **The study, part 4: controls**, to show what is not free, each expected to fail, with its first differing line recorded: the title X one frame later (another galaxy), the fold point moved by 1 px and by 10 px (report from what distance the revealed set changes), and a reversed slipway (the drag from p47 to p25 instead of p25 to p47: step 2a keeps direction).

- **Instrumentation.** A temporary, uncommitted hook that prints the frame of every game-stream draw (the stream's state before and after each frame, compared) while replaying. It explains each result: after the title X, the expected draws are the reveals (one type roll and one vanish roll per revealed planet) and whatever else the log finds (year ends, starbirth, any other). Each trigger found goes in the step note with its frames in `pad_colonise`. Rejected: committing it as a devtools option. Nothing later needs it, and the plan's tools for re-authoring are the end-state and coverage checks.

- **The verdict and the discipline.** The step note states one of: *reachable* (every timing and path variant passes, controls fail as expected), or *not reachable*, with the variant and the first differing field. In both cases it records the discipline, written as rules a re-author follows. The study confirms or corrects this starting list, read from the code:
  1. The title screen keeps its frames exactly: no frame added or removed before the title X, which lands on the reference frame. The title takes one game draw per drawn frame (step 1b's seed tick), so the galaxy depends on it; step 2a saw idle frames inserted at `pad_build` f7 change the galaxy. Every frame budget a new mapping needs is spent after the title X.
  2. Every probe and fold is confirmed at a point that reveals the same planets as the reference. The same world point is the safe choice: the same camera at the press in aim mode.
  3. Reveals happen in the same order: no fold is confirmed while another fold's rings are still revealing, unless the reference does the same.
  4. Every other game-stream draw the instrumentation finds (for example a year end) keeps its position relative to the reveals.
  5. Game actions that charge months or credits happen in the same number and with the same costs. Their order is free only where the study shows it is (the end state compares month and credits, not when they were spent).
  6. A slipway keeps its direction. A structure is placed at the same world point.
  7. The cheat is typed the same number of times.
  The note also records the risk for steps 5 and 7: jump mode's edge scroll may leave a fractional camera, from which aim mode's 1/2/4 px pans cannot reach the reference's integer fold point. Rule 2 asks for the same revealed set, not the same point, so that case is judged by the check, not by a pixel rule.

- **Bob: confirm, and test the known limit.** Step 2a's state writer already prints a bobbing entity's rest y, recomputed exactly from its state time. This step confirms it with the inserted-idle variants of part 2 (every compared line of a bobbing entity unchanged). It then tests the fix's known limit, "the bob ran on every frame since the entity settled", in the one case this plan will create: a replay resumed from an `initial_save` (the Select budget's fallback). A structure that settled before the save point is compared between the original replay and its resumed variant, using `pad_bankrupt`'s labs and the `pad_resume_at_start` fixture, whose slipgate `s2` and trading hub `s1` settled before its record. If the rest y differs, it is fixed in the state writer only, the game untouched. That happens only if the save restores a position and a state time that disagree with "bob on every frame". The fix is exact, never a tolerance, and goes in its own commit. Its regenerated references (deleted, then rebuilt with `make end-state-baseline`) may differ from `8defcba`'s only on the y of bobbing entities; the step note names the commit and the reason. If the rest y cannot be recovered exactly from what a resumed entity holds, the fallback is to record the limit in the step note as a rule of the `initial_save` route: the cut is placed before any structure settles, or after the last one whose y the replay's end state compares.

- **Bob and game rules.** The study reads whether a bobbing position feeds a game rule (slipway length and its "too far", a slipway crossing a planet, structure spacing) and records the answer. If it can, rule 6 of the discipline gains: "a slipway with a bobbing end is built where the reference built it, with margin from any limit", and the risk is noted for steps 6 and 10. No game change: the game rules stay as they are.

- **The relaxed comparison (always defined in the step note).** Applied to both the state and the strict reference text, so there is no second reference file. It keeps: `difficulty`; the year of the `clock` line (the month is dropped); `count colonies`, `count slipways`, `count structures`, `count techs`; one kind line per placed structure, whose id starts with `s`: the kind of each `structure` line (a slipgate), and the class of each `planet` line with an `s` id (labs print as class `structure`, a trading hub as `trading_hub`). Ids, world positions and links are dropped, since links follow which colonies exist. Also `cheat activations`. It drops: every other `planet` line, every `slipway` line, every `techs` line, the `resources` lines, `count planets`, `count nodes` (a vanishing planet dies, and what vanishes is a type), and everything the strict comparison already drops. These are the plan's fields ("counts of colonies, slipways, structures, techs and years") plus difficulty, structure kinds and the cheat count, which do not derive from types.

- **The exceptions file and its support (committed only on a "not reachable" verdict).** A file in `tools/`, next to the long-replay exceptions and in their style: one line per relaxed replay, `<events path> <first differing field>: <reason>`. `tools/end_state.sh check` reads it, inside its single awk process, with no new fork per replay (its unit test runs in `make test`). For a listed replay it runs the relaxed comparison, and fails if the strict comparison would pass (stale exception) or if the listed path is not one of the replays it checks. An unlisted replay is checked strictly, as before. The file is committed empty unless this step's own study needs an entry. If the verdict is "reachable", none of this is committed. The step note says so, and the first step whose replay needs the fallback adds this file and support exactly as defined here.

- **Worst-case verdict.** The `worst-case/` games are judged by the `8c4bbf4` peak rule (every zone and cap peak over the five games matches or exceeds the table's), not by their step 2a references. This confirms the `guideline` decision of step 1b's note and the plan's revised step 13. Their step 2a references stay in place: they keep the check green until step 13 re-baselines them, and they catch an accidental change in the meantime.

- **The `8c4bbf4` peak table.** Built once from a git worktree of `8c4bbf4` in `tmp/`, by running that tree's `make state` and keeping, for each zone and cap line of its peaks file, the peak over the `worst-case/` games only (the five games step 13 re-authors; the e2e of that time do not count). Committed in `worst-case/` with the commit in its name, one line per zone or cap: `<zone|cap> <name> <peak> <worst-case game holding it>`, sorted. The step note records the command that rebuilds it and the names that changed since (a zone or cap renamed, removed or added in today's `make state`). No comparison tool is added: step 13 compares. If `8c4bbf4` no longer builds on this toolchain, the fallback is the parent of step 1b's first commit (`938e864~1`), whose `worst-case/` games still played the full game; the note records which commit was used, and the file is named after it.

- **The coverage comparison script.** POSIX sh and awk in `tools/`, like the other tools (no Python: the `tmp/` prototype is not committed). Inputs: a baseline lines file (the `coverage.sh` format, `<path>:<line> <0|1>`), the current lines file, and a unified diff with zero context between the baseline commit and the working tree, rename detection on, limited to the sources `coverage.sh` covers. It maps each baseline-hit line to the current tree through the diff's hunks. Output, one category per line: `hit`, `unhit` (mapped to a line that is now 0 or no longer executable) and `deleted` (inside a removed or rewritten hunk), then the three totals. Exit 1 if any line is `unhit`. A rewritten line counts as `deleted`: the step that rewrote it justifies it in its note, under the plan's "lines the step deletes or the plan replaces".
- **Its make target**, outside `make test`, `make e2e` and `make test-all`, in the Makefile legend: it builds the `6346485` baseline once into `tmp/` (a worktree of that commit and its `make coverage`), reuses it while it exists, runs `make coverage` on the current tree, makes the diff and runs the script. The baseline commit is a make variable defaulting to `6346485`.

- **Select-hold budget.** The implementer counts, for every e2e, the years it ends (from the clock of its step 2a reference and its start: 3401, or the `initial_save`'s year), its current frames and its frame limit (180, or 600 if listed). The cost of a year after step 9 is 31 frames (a 30-frame hold and a release frame before the next hold). The current cost of its years (the frames spent on `end turn` taps) is subtracted, the new cost added, and the replay *fits* when the estimate stays within its limit with 10% to spare for the other re-authorings. Raising a limit or adding a replay to the long-replay list is not a route. The starting estimate, to be confirmed: `pad_game_end` (25 years, 104 frames, about 800 frames after) and `pad_message_box` (20 years, 190 frames, about 770 after) do not fit; `pad_bankrupt` (8 years in 180 frames, already at its limit) does not fit; `pad_tech_costs` (several years, 571 of 600) is likely not to fit; `pad_colonise`, `pad_end_turn`, `pad_research` and `pad_planet_projects` (one or two years) are checked one by one. The step note gets one line per year-ending replay: years, frames now, estimate, route.
- **The `initial_save` route**, for a replay that does not fit: the replay starts from a record cut, as `pad_resume_at_start`'s is, from the same replay just before its year-heavy part. Its traps go in the step note: a cheat typed before the save point is not counted by the resumed replay (`pad_bankrupt` types it before its seven years), so the cut sits before the cheat or the reference's count no longer matches; no record is written while a wormhole lives; the title and its X are no longer replayed, so the coverage comparison must show the title lines still hit elsewhere. It is tried once now on `pad_game_end` in `tmp/` (with today's `end turn` taps, since Select does not exist yet), checked against its reference. `pad_game_end` ends on the title, so the check is weak there; the same trial is repeated on `pad_bankrupt`, whose end state is a full game, with the cut placed before its cheat. Both results go in the step note. Nothing of it is committed: step 9 re-authors.

- **What gets committed.** The coverage script, its unit test and its target; the peak table; a fix to the rest-y limit and its regenerated references, only if the resume test breaks it; the exceptions file and its support, only on a "not reachable" verdict. Every variant, the instrumentation and the trials stay in `tmp/`. Every `.json.sha256` is unchanged: no replay is re-authored.

- **The step note** (the implementer's report): the verdict; the discipline; the old-edit result; the game-stream draw log of `pad_colonise`; the bob confirmation and the resume test of its limit (and the new reference commit if a fix landed); the bob-and-rules finding; the relaxed comparison's fields and whether its code was committed; the worst-case verdict; the peak table's commit and the names changed since; the coverage comparison's first run; the Select budget table and the two `initial_save` trials.

## Step 2a outcome

Step 2a is committed: the tool in `b19f302`, the references in `8defcba`, its step note in `8c79c66`. This PRD was first written with hypotheses about it. Checked against that tree, they are replaced by these facts:

- **Targets.** `make end-state` checks all 22 replays, or one with `EVENTS=<file>` and optionally `REFERENCE=<reference>`, writing its state to `tmp/end-state/`. `make end-state-baseline` writes only missing references and never overwrites one. Both are outside `make test`, `make e2e` and `make test-all`.
- **Reference files.** One `<events>.end-state` per replay, next to it: 17 in `tests/e2e/`, 5 in `worst-case/`. Deleting them and re-baselining reproduces them byte for byte.
- **Format.**
  - The normalized text is sorted in C order.
  - World positions are printed as `make state` prints them and compared exactly. A coordinate whose hundredths are zero is printed as an integer (`world 246 245.66`, `world 300 262`); any other has two decimals.
  - A planet line reads `planet <id> <state> <class> <type> world <x> <y>`.
  - Placed objects have `s` ids. A slipgate is a `structure <id> <kind> world <x> <y> links <n>` line. Labs and the trading hub are `planet` lines, of class `structure` and `trading_hub`.
- **Compared fields.**
  - Kept: `clock`, `difficulty`, `resources` (credits, science, income), `techs`, `slipway` (direction as printed), `count`, `cheat activations`, `planet` lines through `world x y`, and the id, kind, world and links of `structure` lines.
  - Dropped: `camera`, the `screen` and `visible` fields, and the `zone`, `budget` and `cap` lines.
  - A `stopped` line or an unknown line kind fails.
  - No clock or resources field moved in step 2a's measurement, so all are compared, and the relaxed comparison above stands as written.
- **Cheat line.** The replay tool appends `cheat activations <n>` to the state. Counts: `pad_bankrupt` 1, `pad_planet_projects` 3, `pad_research` 1, `pad_tech_costs` 1, the other e2e 0. `full_game` and `full_game_new_game` 30, `full_game_pause_idle` and `full_game_pause_restart` 24, `title_idle` 0.
- **The script.** `tools/end_state.sh`, POSIX sh, with `normalize`, `check` and `baseline` entry points. It runs one awk process, with no sort or diff, because its unit test `tests/test_end_state.c` (33 ms) runs in `make test`. The relaxed support, if committed, extends that awk and that test the same way.
- **Reference commit `8defcba`.** The `worst-case/` references record the collapsed games. Verification: all 22 pass. `pad_research` cut to 45 frames fails, with its cheat count matching. `pad_research` with 60 idle frames appended passes.
- **Bob, already fixed by step 2a.**
  - Settled structures and trading hubs drifted by up to 3 px with idle frames.
  - The state writer now prints their rest y, recomputed exactly from the state time. The game is unchanged.
  - Known limit: the fix assumes the bob ran on every frame since the entity settled.
  - `tests/test_state.c` pins it on `pad_build` with 60 idle frames appended (`s1`, a settled processor).
  - This step only confirms the fix and tests its limit across a resume.
- **Title timing, still open.** Idle frames inserted on the title screen (`pad_build` f7) gave another galaxy, through step 1b's seed tick. Rule 1 of the discipline covers it, and part 2 repeats it as a control.
- **Inventory.** It is in step 2a's step note. This step adds the coverage script (every e2e through `make coverage`) and the peak table (the `8c4bbf4` peaks).

## Implementation Chunks

1. **Coverage comparison.** The script, its unit test, its make target and the legend line. The first run at the current tree is recorded. One commit. It does not depend on step 2a.
2. **The `8c4bbf4` peak table.** Built from the worktree, committed with the rebuild command and the changed names in the report. One commit. It does not depend on step 2a.
3. **The study.** Parts 1 to 4 and the instrumentation in `tmp/`, the verdict and the discipline, the bob confirmation and the resume test of its limit. Nothing committed, unless the resume test breaks the rest y: then the fix, its unit test and its regenerated references, in one commit.
4. **The fallback and the budget.** The relaxed comparison's definition. On a "not reachable" verdict, the exceptions file, its support in the step 2a script, and its tests, in one commit. The Select-hold budget and the two `initial_save` trials, nothing committed. The step note.

## Testing Decisions

- A good test drives the public surface (a script's entry points on files, the state file the replay tool writes) and asserts on exit status and printed text, never on internals.
- **Coverage script unit test**, one file named after the script, in `make test`, with the peaks script's test as prior art. No replay and no git: hand-written baseline and current lines files and a hand-written zero-context diff. It must run in well under 50 ms. Cases: an untouched file (every hit line `hit`); lines shifted by an insertion above them (mapped, still `hit`); a removed hunk (`deleted`); a rewritten line (`deleted`); a mapped line now 0 (`unhit`, exit 1); a renamed file (mapped through the rename); a baseline line that was 0 (ignored); the totals line.
- **Rest-y limit unit test** (only if the resume test forces a fix), in the existing state test: a settled bobbing entity in a resumed game prints the same world y as in the original replay at the same game point. Prior art: step 2a's `pad_build` 60-idle-frames assertion in that test. It pins values and joins the inventory.
- **Relaxed comparison tests** (only on a "not reachable" verdict), in step 2a's script test: two states that differ only in planet types, slipway ends, resources and month pass the relaxed check and fail the strict one; a different colony count, year, structure kind or cheat count fails both; a listed replay that passes strictly fails as a stale exception; a listed path that is not checked fails.
- **The study is verified by its records**, not by committed tests: each variant's `make end-state` result, the controls' first differing lines, the draw log. They replay e2e files and stay out of `make test`.
- **Plan checks recorded in the step note:** the coverage comparison at the current tree prints no `unhit` line, and its `deleted` lines are step 1b's rewritten draw-site lines; `make end-state` passes on all 22 replays after this step's commits (with the regenerated references if a rest-y fix landed); `make test-all` passes; every `.json.sha256` is unchanged; `make test` and `make e2e` are no slower beyond the new sub-50 ms unit test.

## Out of Scope

- Any pad mapping change, and the Select button itself (step 3 adds it, step 9 maps it).
- Re-authoring any replay or `worst-case/` game, and re-deriving any pinned value (no replay changes here).
- A tool that compares today's peaks with the `8c4bbf4` table (step 13 does that).
- The replay generator of step 5b: this step records nothing about re-authoring cost because it re-authors nothing.
- Changing a game rule, including the bob itself.
- Tuning values (score weight, repeat timing, margins).

## Further Notes

- **Why the bob is still in this step.** The plan asks "whether frame timing alone still moves game decisions". The bob is not a game decision, but it moves a field step 2a compares. Step 2a's fix covers appended frames. This step adds inserted frames and the resume case, so that no re-authored replay with a structure falls into the relaxed fallback by mistake.
- **The bob in numbers.** Each frame adds `0.06 × sin(0.005 × state time)` to y (PICO-8's `sin`, in turns). The period is 200 frames. Step 2a measured drifts of up to 3 px before its fix.
- **Bobbing classes, as read from the code.** The basic structure (labs), the trading hub, and the other planet-family classes whose settled update is the structure's (processor, ascension, protostar, synthesizer, the slipgate and infragate family). The plain planet class has no settled update and does not bob, so `pad_probe` and `pad_colonise` are not exposed; `pad_research`, `pad_tech_costs`, `pad_bankrupt`, `pad_planet_projects`, `pad_build` and `pad_resume_at_start` are.
- **Why the relaxed code waits for a "not reachable" verdict.** The plan commits it "if a relaxed comparison is defined". Defining the fields now costs nothing and makes the fallback the same in every step. Building unused code does not, and the first step that needs it can add it from this definition in one small commit.
- **The `tmp/` coverage prototype** mentioned when this step was planned is not read here and not committed. The sh and awk script follows the repository's tool style.
