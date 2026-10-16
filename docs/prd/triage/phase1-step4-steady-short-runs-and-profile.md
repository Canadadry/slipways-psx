---
title: "Steady short runs and the profile of the steady frames over"
description: "The steady situations that hold 1,133 frames over 263 have no short run that lies in them and no per-function cost split, so no lever can be ranked or measured against them."
status: needs-triage
---

## Plan step (verbatim)

Plan 22, phase 1, step 4:

> 4. Short runs, then the profile of the steady frames over. First, each
>    steady situation with frames over 263 gets at least one short run
>    (`psx/short_runs.txt`) whose window lies in it, runs added or replaced
>    until that holds; resumed runs are known to diverge from the full game
>    (`docs/psx-game.md`, Short runs), so each run is checked on its own
>    replay: step 2's labeler gives its window the same situation, and its
>    draw and update are within the `representative` tolerance of the full
>    game's. The step starts with that check on one resumed run in the
>    steady situation with the most frames over. A situation with no
>    representative run after three tries is marked "no representative
>    run" with its gap; its levers are measured on `psx-full-game-check`
>    instead of `psx-probe`, and step 5 flags them as weaker evidence. Then
>    the profile: per game function (planets,
>    slipways, particles, HUD, panels, map, overlays, and update when it
>    matters), split into time inside the primitives and time in the game's
>    own code, from the PSX draw breakdown and the PC frame profile with
>    `cart_draw` (and `cart_update`) as roots, scaled to the judge. Verified
>    when the note lists, per steady situation over, its short run with its
>    own labels and its gap to the full game, and the shares add up to the
>    measured frame within the noise floor on every one of these runs.

## Problem Statement

Plan 22 has to bring every steady frame of the real game within 263 hblanks.
Step 2's report says where the steady frames over are (on step 1's table,
frames / over 263 / mean update / mean draw):

| Situation | Frames | Over | Update | Draw |
|---|---:|---:|---:|---:|
| map-rest | 1,564 | 429 | 41 | 168 |
| map-move | 1,907 | 388 | 41 | 160 |
| build | 366 | 155 | 36 | 215 |
| menu | 252 | 91 | 29 | 203 |
| slipway-aim | 111 | 41 | 73 | 189 |
| tech-panel | 47 | 21 | 31 | 242 |
| score-panel | 17 | 8 | 98 | 192 |

(`title` has none over.) Seven steady situations, 1,133 frames over.

Nothing measures them today. The six short runs of `psx/short_runs.txt` were
chosen for plan 21's spikes and draw clusters, not for situations: four are
one-frame update spikes, and `draw-6000`'s window (6000–6014) straddles `menu`
and the frames after it. No run is known to lie in one steady situation, and
the resumed runs are known to diverge from the full game (red on
`representative`, draw lower after the resume). So `psx-probe` cannot measure a
lever on a steady situation, and step 5 cannot rank levers by their effect on
the steady frames over.

Nor is there a per-function cost split. The PSX draw breakdown gives time per
primitive family (`spr`, `line`, …) and in-calls versus outside-calls, never
per game function; the PC frame profile gives time per direct child of
`cart_draw` (mostly two `draw_layers` calls) or of another root, and in-call
time per family for the whole frame, but never the primitive time *inside* one
function. Step 5 needs, per steady situation, how much of the frame each game
function (planets, slipways, particles, HUD, panels, map, overlays, update)
costs on the judge, and how much of that is primitives versus the game's own
code, to pick levers above 2%.

## Solution

Two halves, in order.

**Short runs per steady situation.** A host tool ranks, for each steady
situation with frames over 263, candidate windows from the committed frame
table and situation table: windows of at most 15 consecutive frames that all
carry that situation, ranked by frames over 263. The best candidate becomes a
resumed run named `<situation>-<first>` in `psx/short_runs.txt`. Each run is
then checked on its own replay:

- **labels**: a new host check runs step 2's labeler (`tmp/situations`) on
  the run's own `run.json`, maps the run's frames to full-game frames with its
  `map.txt`, and requires every window frame to carry the run's situation;
- **representative**: `psx-short-runs` on that run, with the full game's frame
  table as reference, must give `representative ok` (window draw sum and update
  sum each within ±10%).

The step starts with `map-rest` (429 frames over, the most). A situation whose
first candidate fails either check gets its next candidate, up to three tries;
after three failures it is marked "no representative run" in the manifest and
the docs, with the best try's gap, and step 5 measures its levers on
`psx-full-game-check`.

**The profile.** For each of these runs, a new target `make steady-profile`
splits the run's window frame (plain boot, the judge) into game functions:
each function's time inside the primitives (from the PSX breakdown boot's
per-family time, shared among functions by the PC profile's per-function,
per-family time) and in the game's own code (the PC profile's time outside
primitives, scaled to the PSX outside-calls time), plus update (split down
`cart_update60`'s children and `input_step_run`) and submit. It prints one line
per function group, per window frame and as a share of the measured frame, and
a closing `adds-up` line: the sum of every line against the plain boot's
measured window total, with the residual and the run's noise floor.

The step note lists, per steady situation over, its run, its own labels, its
gap to the full game (draw and update, %), and its profile with the `adds-up`
verdict.

## User Stories

1. As the plan pilot, I want every steady situation with frames over 263 to have a short run whose window lies in it, so that a lever on that situation can be measured in minutes with `psx-probe`.
2. As the plan pilot, I want candidate windows ranked by a stated rule from the committed tables, so that the run chosen for a situation is reproducible and not hand-picked.
3. As the plan pilot, I want a candidate window to contain only frames of one steady situation in the full game's table, so that "the window lies in it" is checked, not assumed.
4. As the plan pilot, I want candidates ranked by frames over 263 first, so that each run measures the frames the plan must fix.
5. As the plan pilot, I want up to three disjoint candidates per situation, so that a failed try has a ready replacement.
6. As the plan pilot, I want each run's own replay labeled by step 2's labeler, so that a resumed run that has drifted into another situation is caught.
7. As the plan pilot, I want the label check to name the first window frame whose label differs, with both labels, so that I can tell a camera drift from a panel that is not open.
8. As the plan pilot, I want the label check to run on the host without booting the PSX, so that a candidate is rejected in seconds before a PSX build.
9. As the plan pilot, I want the existing `representative` condition used unchanged (±10% on window draw and update sums), so that "representative" means what it already means in `docs/psx-game.md`.
10. As the plan pilot, I want `representative` judged against the committed full-game frame table when the game code has not changed since it was measured, so that the step does not need a 20-minute full check per try.
11. As the plan pilot, I want the step to start with the map-rest run, so that the riskiest assumption (a resumed run is representative) is tested on the situation that matters most before the rest is built.
12. As the plan pilot, I want a situation with no representative run after three tries marked "no representative run" with its gap, so that step 5 knows its levers are measured on `psx-full-game-check` and flags them as weaker evidence.
13. As the plan pilot, I want the marking written where step 5 and later steps read it (the manifest and `docs/psx-game.md`), so that it does not live only in a step note.
14. As step 5's author, I want each run named after its situation, so that a lever names the run that measures it without a lookup table.
15. As a developer, I want the existing six runs kept, so that the probe fixtures, the frame-profile targets and `FRAME_PROFILE_ORDINARY_WINDOWS` that name them keep working.
16. As a developer, I want an existing run reused for a situation when its window already passes both checks for it, so that no run is duplicated.
17. As step 6's implementer, I want the new runs in the same manifest format, so that step 6's renumbering moves them with the others.
18. As the plan pilot, I want, per run, the profile split per game function group (planets, slipways, particles, HUD, panels, map, overlays, frame scaffold, update, submit), so that step 5 can list every function above 2% of the steady frames' mean cost.
19. As the plan pilot, I want each draw function's time split into time inside primitives and time in the game's own code, so that a lever can target the backend or the game code knowingly.
20. As the plan pilot, I want every figure in hblanks of the judge per window frame, so that it compares directly with 263 and with the frame tables.
21. As the plan pilot, I want each function's share of the measured window frame, so that the 2% threshold of step 5 reads off the output.
22. As the plan pilot, I want the shares to add up to the plain boot's measured window total within a stated noise floor, so that no function is missing and nothing is counted twice.
23. As the plan pilot, I want the `adds-up` line to print the sum, the measured total, the residual and the noise floor, so that the note quotes it as is.
24. As the plan pilot, I want the breakdown boot's own per-call overhead removed from the primitive times, so that the profile describes the plain build the judge runs, not the instrumented one.
25. As the plan pilot, I want update split down its children when it matters, so that slipway-aim (73) and score-panel (98) are not reduced to one "update" line.
26. As a developer, I want the PC frame profile to report, per root child, its calls and time per primitive family, so that the primitive time inside one game function is measured, not guessed.
27. As a developer, I want the new profile lines to leave every existing `frame_profile_check.sh` mode's output unchanged, so that earlier targets keep their meaning.
28. As a developer, I want every draw symbol mapped to a group by a committed table, and an unmapped symbol to fail the profile, so that a new draw method can never vanish into "other".
29. As a developer, I want the PC profile repeated K times and medians taken, as the other frame-profile targets do, so that one noisy PC run does not move a share.
30. As a developer, I want the profile to check that the PC profile's per-family calls equal the breakdown boot's over the window, so that the PC and PSX runs are known to play the same frames.
31. As a developer, I want the window ranking and the profile join tested on hand-written fixtures, so that their rules are proven without a PSX boot.
32. As a developer, I want none of this in `make test` or `make e2e`, so that the fast loop stays fast.
33. As a reviewer, I want `docs/psx-game.md` to list the new runs, the label check, the ranking rule, the profile target and its noise floor, so that the docs say what is true.
34. As a reviewer, I want the note to give, per steady situation over, its run, labels, gap and `adds-up` verdict, so that the step's verification is checkable from the note alone.

## Implementation Decisions

### Candidate windows (new host script, `tools/steady_windows.sh`)

- Interface: `steady_windows.sh <frame-table> <situation-table> <budget-hblanks> <max-window> <candidates>`; the Makefile target `steady-windows [FRAMES=] [SITUATIONS=]` runs it with `psx/perf/full-run-frames.txt`, `psx/perf/full-run-situations.txt`, 263, 15, 3. It joins the two tables with the same header and numbering rules as `tools/situation_report.sh` (fails on a mismatch).
- For each steady situation with at least one frame over the budget, in the rule order of `docs/psx-game.md` § Situations, it considers every window of `min(max-window, stretch length)` consecutive frames inside a stretch of that situation (a stretch is consecutive frames with that label). It ranks windows by frames over the budget (descending), then by summed total (descending), then by first frame (ascending), and keeps the best, then the next best not overlapping any kept one, up to `<candidates>`.
- Output, one line per candidate: `candidate <situation> <rank> <first> <last> frames <n> over <o> mean-update <u> mean-draw <d>`; then `situation <name> frames-over <o> candidates <c>` per situation. Exit 0 once written; 1 on usage or join error.
- On step 1's table the first candidates are expected around map-rest 5637–5651, map-move 1046–1079 (a 15-frame part), build 5914–5928, menu 5954–5968, slipway-aim 5123–5132, tech-panel 5895–5903, score-panel a single frame (its stretches are one frame long, e.g. 5441). These are what a quick join gives; the script is the authority.

### Label check (new Makefile target `short-run-situations`)

- For each run of the manifest (or `PSX_SHORT_RUNS=`), after `psx-short-runs-cut`: runs `tmp/situations` on `<run>/run.json` into `<run>/situations.txt`, maps run frame r to full frame `source-first + (r − first-game-frame)` with `map.txt`, and compares each window frame's label with the committed table's label for the full frame.
- The comparison is a small host script, `tools/short_run_situations.sh <run-situations> <map.txt> <full-situations>`: prints `short-run-situation <name> same <situation>` when every window frame of the run carries the same label as the full game, and that label is one steady situation; `differs frame <full-frame> run <label> full <label>` for the first difference; `mixed <labels>` when the full window itself carries several labels (such a run cannot stand for a situation). Exit 0 once written; 1 on a usage or parse error. The target prints one line per run and fails when any requested run is not `same`.
- Host only, no PSX build, outside `make test-all`.

### Choosing the runs

- Run names are `<situation>-<first>` (at most 31 characters, the cut's cap), kind `resume`, window = the candidate. An existing run whose window passes both checks for a situation may serve instead; the existing six runs are kept as they are.
- The first try is the map-rest first candidate. If its label check or `representative` fails, the step note records why before any other run is added: this is the plan's early test of whether resumed runs can be representative at all.
- A try is: add the line, `make psx-short-runs-cut PSX_SHORT_RUNS=<name>`, `make short-run-situations PSX_SHORT_RUNS=<name>`, then (only if `same`) `make psx-short-runs PSX_SHORT_RUNS=<name> PSX_SHORT_RUN_REFERENCE=psx/perf/full-run-frames.txt`. A failed try's line is removed from the manifest; the next candidate is tried. The third try may instead be a `prefix` run when the candidate lies below the cut's 1,024-frame cap (a prefix run never diverges); otherwise it is the third candidate.
- The reference is the committed `psx/perf/full-run-frames.txt` as long as no file the PSX GPU replay compiles from has changed since step 1 measured it (step 2 only added `src/devtools/situation.c`, `src/cmd/situations.c` and touched `pad_author`, none of them in the PSX replay). The note states this with the `git diff --stat` of `src/` between step 1's commit and the measuring HEAD. If game or platform code did change, the step runs `psx-full-game-check` once and uses its fresh table.
- After three failed tries a situation is marked: a comment line in `psx/short_runs.txt`, `# no representative run: <situation> (best try <name>: <reason>, draw <+x%>, update <+y%>)`, and the same in `docs/psx-game.md` § Short runs. Its best try stays in the manifest (it still serves the profile, flagged) only if its label check passed; otherwise the profile of that situation uses the full game's frames for its totals and is flagged `no run`.
- A run that fails only `representative` because of `done`, `deterministic`, `breakdown` or `asserts` is a tooling failure, not a try: it is fixed, not counted.
- Run lengths: a resume run is at most the cut's 1,024 frames plus the title resume; if a run reaches `PSX_SHORT_RUN_CEILING` (1,080 vsyncs) the ceiling and the probe's are raised to fit, recorded in the note, never the window shortened to dodge it.

### Per-child family figures in the PC frame profile

- `call_profile` gains, per root child and per frame, per family calls and ns (and the child's in-call ns, their sum without `pattern`), counted when a backend call happens while that child is the active direct child of a root. Accessor: `call_profile_child_family_figures(frame, root, child, family)` returning the existing `call_profile_family_figures_t`. Memory: the tables are sized by `CALL_PROFILE_FRAME_CAP × CALL_PROFILE_CHILD_CAP × families` per root only for the draw root; if that is too big for a static table, the frame cap for this table is the short-run cap (1,024) and a longer recording simply skips the lines (documented contract), since the profile only uses short runs.
- `frame_profile` writes, per frame, per child with at least one call in a family, `frame <f> child-family <root> <child> <family> <calls> <ns>`. The third field `child-family` is new, so every existing mode of `frame_profile_check.sh` (they select lines by `$3`) ignores it; this is tested.
- A call made by a root itself (outside any child) counts under the root's self, as a `self-family` line.

### Function groups (new committed table, `psx/perf/draw-functions.txt`)

- Lines `<symbol> <group>`, groups `planets`, `slipways`, `particles`, `hud`, `panels`, `map`, `overlays`, `frame`. Every child of `draw_layers` (entity class draw and draw-overlay methods: `planet_draw_method`, `slipway_draw_method`, `emitter_update_method`, `ui_draw_overlay_method`, …) and every child of `cart_draw` other than `draw_layers` (`p8_cls`, `text_image_frame_begin`, `hud_select_palette_block_default`, `demo_overlay_draw`) is listed; `draw_layers`' own self time (the dispatch loop and camera) is `frame`. The implementer assigns each symbol from what the method draws; an ambiguous one goes where its largest primitive family's content belongs, and the table is the record.
- Update is not grouped by this table: its pieces are `input_step_run` and the direct children of `cart_update60` (`update_entities`, `ui_update`, `save_at_frame_end`, `cart_flush_exit_queue`, `frame_zone_reset`) plus `cart_update60`'s self, one line each.

### The profile (new script `tools/steady_profile.sh`, target `steady-profile`)

- Inputs per run: its `map.txt`, its plain-1 and breakdown boots' TTYs (from `psx-short-runs`), K PC profiles rooted at `draw_layers` (draw) and `cart_update60` (update), K rooted at `cart_draw` and `input_step_run`, the group table, and the pooled per-call overhead.
- `make steady-profile [PSX_SHORT_RUNS=] [FRAME_PROFILE_REPEATS=]` (default: every manifest run whose name starts with a steady situation name) profiles each run's `run.json` K times per root pair with `tmp/frame_profile` (same `profile` function as the other targets, `same-game` checked), then runs the script; output in `tmp/frame-profile/steady/<run>/summary.txt`. It needs each run's breakdown boot (`make psx-short-runs` first) and names the missing one otherwise. It never boots.
- Calls check: per family, the PC profile's window calls equal the breakdown boot's (`frame_profile_check.sh breakdown`'s rule, reused); a mismatch fails the run's profile.
- Draw, per function f over the window (PSX values from the breakdown boot, summed over window frames):
  - per family F, PSX in-call hblanks of F minus `o × calls_F` (o: the pooled overhead per call) are shared among functions in proportion to each function's PC ns in F (median of K);
  - the PSX outside-calls hblanks (breakdown draw − in-calls) are shared among functions in proportion to each function's PC time outside primitives (its ns minus its in-call ns, median of K; `draw_layers` and `cart_draw` selfs included);
  - **in-primitives(f)** and **game-code(f)** are those two parts.
- The pooled overhead o is one number for the step: Σ(breakdown draw − plain draw) ÷ Σ backend calls over every steady run's window. Pooling, not each run's own ratio, is chosen so that the draw half of the sum is not equal to the plain draw by construction: the residual then measures how well one per-call overhead describes every run (plan 21 measured 0.129–0.133 per call).
- Update: the plain boot's window update sum, shared among the update pieces in proportion to their PC ns (median of K) — the `ordinary` mode's two-root rule without its ratios. Submit: the plain boot's window submit sum, one line. "When it matters": update is always split; a piece under 1% of the window total is folded into an `update other` line.
- Output, per window frame (sum ÷ window frames, one decimal) and share of the plain window total: `group <name> in-primitives <h> game-code <h> total <h> share <pct>`, then `function <symbol> group <name> in-primitives <h> game-code <h> share <pct>` by decreasing total, then `update <piece> <h> share <pct>`, `submit <h> share <pct>`, then `adds-up ok|failed sum <S> measured <M> residual <R> noise-floor <N>`. Exit 0 once written; 1 on usage, 2 on malformed input or a calls mismatch.
- **Noise floor** of a run: the larger of 3 hblanks per window frame (one hblank of rounding for each of the update, draw and submit counters every frame) and 1% of the measured window total. `adds-up ok` when |residual| ≤ noise floor.

### Docs

- `docs/psx-game.md` § Short runs: the manifest's runs including the new steady ones, the ranking rule, the label check, any "no representative run" mark; § PC frame profile: the `child-family` lines, `steady-profile` and its noise floor. "Still to do" entry on `representative` updated with what the step found.

## Implementation Chunks

1. **Candidate windows and the label check** (host only): `tools/steady_windows.sh`, `tools/short_run_situations.sh`, targets `steady-windows` and `short-run-situations`, their tests. Then the first try: the map-rest first candidate added to the manifest, cut, labelled, measured with `psx-short-runs` against the committed reference; its outcome written in the note before going on.
2. **The other six situations' runs**: up to three tries each, as decided above; manifest, `docs/psx-game.md` § Short runs, and the "no representative run" marks. `make psx-probe-baseline` refreshed for the new runs (the manifest is under `psx/`, so the old fingerprint is stale).
3. **Per-child family figures in the PC profile**: `call_profile` and `frame_profile` changes, unit tests, the check that existing `frame_profile_check.sh` outputs are unchanged on a profile with the new lines.
4. **The profile**: `psx/perf/draw-functions.txt`, `tools/steady_profile.sh`, target `steady-profile`, fixture tests; then the measured profile of every steady run, the pooled overhead, the `adds-up` lines, and the note.

## Testing Decisions

- A good test drives a tool from outside: it writes fixture tables or profiles, runs the script or binary, and checks the printed lines and exit status; it never inspects how a join is done.
- **`steady_windows.sh`** (`tests/test_steady_windows.c`, new, in `psx-tools-test`): fixture tables of a few dozen frames. Cases: a window never crosses a label change; ranking by frames over, then summed total, then first frame; candidates do not overlap; a stretch shorter than the max window gives a window of the stretch's length; a situation with no frame over gets no candidate; one-off frames are never candidates; a join mismatch fails. Prior art: `tests/test_psx_frame_budget.c`, the `situation_report.sh` tests.
- **`short_run_situations.sh`** (`tests/test_short_run_situations.c`, new, in `psx-tools-test`): fixture run tables and maps for a resume map (`first-game-frame 13`) and a prefix map. Cases: `same`; `differs` naming the first full frame and both labels; frames 0–12 of a resume run never compared; `mixed` full window; malformed map fails.
- **`call_profile`** (`tests/test_call_profile.c`, extended): a scripted clock and fake functions: a family call under child A counts in A's figures only; a nested call deep under A still counts in A; a call from the root's self counts as self; per-child in-call sums equal the frame's in-call. **`frame_profile`** (`tests/test_frame_profile.c`, extended): the binary on `pad_title.json` writes `child-family` lines whose per-family calls sum to the frame's family calls.
- **Existing modes unchanged** (`tests/test_frame_profile_check.c`, extended): one existing fixture case run again with `child-family` and `self-family` lines added gives byte-identical output.
- **`steady_profile.sh`** (`tests/test_steady_profile.c`, new, in `frame-profile-test`): hand-built TTYs and profiles with round numbers. Cases: in-call time shared by PC ns per family; outside time shared by PC outside ns; the overhead removed per call; update shared among pieces; `adds-up ok` with a residual inside the floor and `failed` one hblank past it; floor = max(3 × frames, 1%); a symbol missing from the group table fails; a calls mismatch fails with exit 2; medians over K profiles.
- Nothing in `make test` or `make e2e`. The PSX boots (`psx-short-runs`, `psx-probe-baseline`) are evidence for the note, not tests.

## Out of Scope

- Changing the situation rule, the labeler or `psx/perf/full-run-situations.txt` (frozen by step 2).
- Changing the cut rule, the `representative` tolerance, or the existing six runs' windows.
- Making a resumed run restore more state (camera, jump selection, crosshair): the game is not changed for a short run.
- The lever list, its estimates and `docs/psx-levers.md` (step 5); the `lever-estimate` targets (step 5).
- Any game, platform or picture change; any e2e hash change.
- One-off situations: they get no run and no profile here.
- Running `psx-full-game-check`, except once if game or platform code changed since step 1 (see Choosing the runs).

## Hypotheses on step 3

Step 3 (`docs/prd/triage/phase1-step3-real-game-budget.md`) is not done when this PRD is written. Everything below is assumed; the step note confirms or refutes each one.

- **H-3-1**: step 3 changes no game, engine, runtime or platform code, so the GPU replay that `psx-short-runs` builds is the program step 1 measured, and the committed `psx/perf/full-run-frames.txt` is a valid `representative` reference without a fresh `psx-full-game-check`.
- **H-3-2**: step 3 does not change `psx/perf/full-run-frames.txt` or `psx/perf/full-run-situations.txt`; the steady situations over are the seven listed in the Problem Statement, and `make psx-real-game-budget` on the committed tables reports 1,133 steady frames over (the sum of step 2's per-situation figures). If it reports another count, the per-situation figures of `make situation-report` are what this step uses, and the note records the gap.
- **H-3-3**: step 3 does not touch `tools/psx_short_run.sh`, `tools/psx_probe.sh`, `src/devtools/short_run_cut.c`, `psx/short_runs.txt`, `src/devtools/call_profile.c`, `src/cmd/frame_profile.c` or `tools/frame_profile_check.sh`.
- **H-3-4**: step 3's new test joins `PSX_TOOLS_TESTS`; since `psx-short-runs`, `psx-probe` and `psx-probe-baseline` depend on `psx-tools-test`, step 3's tests must pass for this step's runs to start. If they fail, that is step 3's bug, fixed under fix-bug, not worked around here.
- **H-3-5**: step 3 keeps `psx-full-game-check`'s frame table at `run/frame-budget/frames.txt` under `PSX_FULL_GAME_DIR`, so `PSX_SHORT_RUN_REFERENCE`'s default still points at a fresh check's table if one is run.
- **H-3-6**: step 3's `real-game-budget` judges steady frames by `total > 263`, the same "over" as this step's ranking; if it reads "over" differently, the ranking follows step 3's definition and the note says so.

## Further Notes

- The riskiest part is the first try: plan 21 already found resumed runs red on `representative` (draw lower after the resume: no camera, jump selection or crosshair in a save). map-rest's draw depends on the camera position, so the first try tells early whether the steady situations can be measured on short runs at all; the step note records it before anything else is added.
- `score-panel`'s stretches are one frame each: its runs have one-frame windows, so its `representative` check compares single frames.
- Expect the step's PSX time to be dominated by `psx-short-runs` per try (two plain boots and one breakdown boot, a few minutes) and by one `psx-probe-baseline` for the new runs.
- No guideline agent was consulted. The judgment calls recorded above: the ranking rule and 15-frame window, the run naming, the committed reference table, the third try as a prefix run where possible, the per-child family lines as the way to split primitives per function, the pooled overhead, the group table with a `frame` group, update always split, and the noise floor of max(3 hblanks per window frame, 1%).
