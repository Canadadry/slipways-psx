# Plan 22 — Real-game 60 fps on PSX: steady play in budget, one-off stalls bounded

## 1. Goal

A player of a real game on the PSX build sees 60 fps in normal play, and
the moments that stall add up to less than 5% of the game's time.

The target device is a Miyoo Mini Flip running PCSX-ReARMed. It is never
measured: the judge is pcsx-redux, 263 hblanks per frame (one NTSC vsync),
on the GPU path, asserts off, O3.

**A real game is modeled, not recorded.** No human recording of a real
game exists or will be provided. `worst-case/full_game.json` plays a whole
game in 6,059 frames (about 100 s): it is a compressed worst case, nonstop
actions, no idle time. A real game lasts **60 minutes = 216,000 frames**
(the human's figure). So the plan judges each slow frame by its cause and
extrapolates:

- **Steady situations** — what the player stays in: map at rest, cursor
  or camera moving, a panel or menu open, anything whose share of time
  grows with play time. They must be **100% within 263**, whatever bounds
  them (draw or update). This is "the main game"; no exception.
- **One-off events** — what happens once per action or per game: galaxy
  generation, end of turn, building a slipway, colonising, game end, and
  so on. A real game has about as many of them as `full_game.json` (it
  plays a complete game), spread over 216,000 frames. They may be slow as
  long as **the vsyncs spent in their frames over 263, summed over
  `full_game.json`, stay under 5% of 216,000 = 10,800 vsyncs**.

At plan 21's close (after D8): 1,764 of 6,059 frames over 263, mean frame
233.6, worst frame 12 at 3,992 (galaxy generation, 15 vsyncs). The last
frame table in the repo (`psx/perf/full-run-frames.txt`, before D8, 3,078
frames over) has 451 frames above 400 and about 2,600 ordinary frames
over with a mean draw of about 260 against an update of about 55. So
one-off stalls already fit the 5% easily; **the work is the steady
situations**, mostly bound by draw.

Galaxy generation stays a stall but gets a **loading screen**: a black
screen with the word "loading" in a corner, no animation, shown while
generation runs.

## 2. Rules

### Decided in the grilling

- **The judge is the real-game model above**, not "every frame within
  263". `frame-budget` (strict) stays reported by `psx-full-game-check`
  but no longer closes the plan; a new condition, `real-game-budget`,
  does (step 3). Rejected: strict 60 fps on every frame (unreachable
  without moving galaxy generation over several frames, which the human
  does not want); a plain percentage of `full_game.json`'s frames (that
  replay is not a real game, its frame mix says nothing about one).
- **Update levers are in scope** when a steady situation is bound by
  update. The old "draw only" scope is dropped: the goal is the player's
  experience, whatever the cause.
- **Spreading one-off work over frames** (plan 21's skipped steps 19–20)
  stays out: one-off events fit the 5% already. Revisit only if step 3
  measures them over.
- **Loading screen on both platforms.** It lives in the shared game code
  (`src/game/`), so PC and PSX stay identical; a PSX-only screen would put
  platform-dependent behavior in the game. It is the plan's **only**
  allowed change to the game's picture and e2e hashes: the e2e replays
  whose hash changes because of it are re-recorded once, in its own step
  (step 6), and every replay's game outcome (`.end-state`) stays the same.
- **Two passes on picture changes.** Pass 1 (this plan): only levers that
  keep the picture pixel-identical and every e2e hash unchanged. If the
  steady situations are still over at the end of pass 1, the plan stops
  and lists the picture-changing options (fewer particles, simpler
  slipways, animations updated every other frame, …) with their measured
  or estimated gains; **the human chooses**, in a later grilling. No agent
  ever decides alone that a visible change is "barely visible".
- **No checkpoint before the end of pass 1**: the plan runs unattended to
  its close.
- **Plan 21's leftovers**: keep D11 (a slipway's four lines sent as one
  batch, measured -1.33 per frame; every bit counts toward 60 fps);
  update or remove the `lever-estimate` / `lever-estimate-check` Makefile
  targets and their scripts, which cite the deleted
  `docs/psx-perf-levers.md` (step 5); act on plan 21's warn-review
  warnings only in code a step of this plan already touches.

### Carried over from plan 21

- **The PSX picture stays pixel-identical to the PC's**: every capture at
  `differ 0` (or under its existing ceiling). A raw-VRAM reference may
  change only when every capture of its label stays at `differ 0`; an
  agent may then accept it (`docs/psx-game.md`, Acceptance). The loading
  screen's new frames follow the same rule.
- **Every e2e hash stays unchanged**, except the single re-record of
  step 6.
- **Asserts off, O3** in `psx-game` and every measured build.
- **Measuring**: each lever is measured on the short runs (`make
  psx-probe` against a fresh baseline), with its gain against its
  estimate in the step note; `psx-full-game-check` at the end of each
  phase that changes code, its table replacing
  `psx/perf/full-run-frames.txt`; every other condition of that check
  must hold.
- A lever whose measured delta on its target is not a gain is reverted and
  noted.
- Before a lever, check whether a lever already taken covers it; "covered"
  and "under 2%" are decided on the lever's remaining cost, measured.
- A lever under its estimate is noted and the plan goes on.

### Out of scope

- Measuring on the Miyoo or PCSX-ReARMed; audio; changing the 263 budget.
- Any change to the picture or to e2e hashes beyond the loading screen.
- Spreading one-off work over frames (see above).

## 3. Phases and steps

### Phase 1 — Measure and model the real game

1. Baseline: one `psx-full-game-check` on the plan's starting HEAD, its
   table replacing `psx/perf/full-run-frames.txt`. Verified when the note
   gives frames over, worst and mean against plan 21's 1,764 / 3,992 /
   233.6.
2. Situation of every frame: a host-side tool (in `src/devtools/` or
   `tools/`, PC replay, no PSX boot) that labels each frame of
   `full_game.json` with its situation and its family, `steady` or
   `one-off`, from the game state (screen, open panel, cursor or camera
   motion, running animation or event), written to a committed table
   (`psx/perf/full-run-situations.txt`). The labeling rule is written in
   `docs/psx-game.md`. Verified by a unit test on hand-picked frames of
   each situation, and by the note listing, per situation, its frame count,
   frames over 263, and mean update and draw on the judge (joined with
   step 1's table); for each one-off situation, also its number of
   occurrences in `full_game.json` and its longest run of consecutive
   frames — a one-off stretch that is long or not tied to one action or
   one game event (section 1) fails the step. The labeling rule and the
   table are then frozen: no later step changes them, except step 6's
   renumbering; step 8 and the close confirm the rule is step 2's.
3. `real-game-budget`: a new condition in `psx-full-game-check` that joins
   the frame table with the situation table and is green when **no steady
   frame is over 263** and **the vsyncs of one-off frames over 263 sum to
   at most 10,800** (5% of 216,000). It prints both figures; `frame-budget`
   stays reported but is no longer required by this plan. Verified by a
   test on fixture tables (green, red on one steady frame, red on one-off
   sum) and by its figures on step 1's table in the note.
4. Short runs, then the profile of the steady frames over. First, each
   steady situation with frames over 263 gets at least one short run
   (`psx/short_runs.txt`) whose window lies in it, runs added or replaced
   until that holds; resumed runs are known to diverge from the full game
   (`docs/psx-game.md`, Short runs), so each run is checked on its own
   replay: step 2's labeler gives its window the same situation, and its
   draw and update are within the `representative` tolerance of the full
   game's. The step starts with that check on one resumed run in the
   steady situation with the most frames over. A situation with no
   representative run after three tries is marked "no representative
   run" with its gap; its levers are measured on `psx-full-game-check`
   instead of `psx-probe`, and step 5 flags them as weaker evidence. Then
   the profile: per game function (planets,
   slipways, particles, HUD, panels, map, overlays, and update when it
   matters), split into time inside the primitives and time in the game's
   own code, from the PSX draw breakdown and the PC frame profile with
   `cart_draw` (and `cart_update`) as roots, scaled to the judge. Verified
   when the note lists, per steady situation over, its short run with its
   own labels and its gap to the full game, and the shares add up to the
   measured frame within the noise floor on every one of these runs.
5. Lever list, `docs/psx-levers.md`: for each function above 2% of the
   steady frames' mean cost, a lever with its location, estimated gain,
   cost and risk (pixel identity, game state), ranked by its effect on
   `real-game-budget` (steady frames over first). Pixel-identical levers
   in the ranked list; picture-changing ideas in a separate, unranked
   section kept for the close. The `lever-estimate` targets are re-sourced
   to this list or removed. Each lever names the short run (step 4) that
   measures it. For a situation marked "no representative run", a
   function's share comes from `steady_profile.sh no-run` on the full
   game's frames over in that situation (no resume involved); its
   best-try run supplies only the PSX per-call costs. Where a function's
   no-run share and its best-try share differ by more than a fifth, the
   note lists it. Verified when the estimated gains, applied to the
   committed frame table (step 1's until it is replaced), give the steady frames still over at the end of the
   list, with a pessimistic variant (every gain cut by a fifth).

### Phase 2 — Loading screen

6. Loading screen during galaxy generation, in the shared game code: a
   black screen with "loading" in a corner, no animation, on the screen
   while `galaxy_populate` (`src/game/cart.c`) runs, on PC and PSX alike.
   The e2e replays whose hash changes are re-recorded once; if the extra
   frame shifts the inputs that follow, the recordings are adjusted so
   each plays the same game. If `full_game.json` changes, everything keyed
   by its frame numbers follows: the frame table, the situation table, the
   short-run windows and the probe baseline. Verified when every
   `.end-state` (in `tests/e2e/` and `worst-case/`) is unchanged, only the
   expected hashes changed (listed in the note), the PSX capture of the
   loading frame is at `differ 0`, the frame on display during the
   generation stall's vsyncs is the loading frame on both platforms (the
   PSX shows each frame one frame late), the situation table and the new frame
   table have the same frame count, step 2's hand-picked frames still
   carry their situations at their new frame numbers, every short run
   passes step 4's check again on the new numbering (a run marked "no
   representative run" stays marked), and
   `psx-full-game-check` still finishes with `real-game-budget`'s one-off
   sum not worse.

### Phase 3 — Pixel-identical levers, in impact order

7. Each lever of step 5's ranked list, in its order, one step note each:
   measured gain against its estimate, pixel-identical captures, unchanged
   e2e hashes. After each lever, its measured gain replaces its estimate
   in `docs/psx-levers.md` and, once two levers of the same estimation
   basis (`calls` or `share`) are measured, the median of their
   measured-to-estimated ratios scales the remaining estimates of that
   basis before re-ranking (the ratio and the levers it rests on are in
   the note); the projected steady frames still over are
   recomputed (step 5's calculation) and the remaining levers re-ranked
   if the order changes. Whenever a `psx-full-game-check` replaces
   `psx/perf/full-run-frames.txt`, the projection is rebased on that
   table: every lever already in the measured code is marked `in-base` in
   the lever table (kept for the record, no gain applied), and the note
   gives the measured steady frames over next to the last projection,
   before and after the rebase. When the projection reaches zero steady frames
   over, `psx-full-game-check` confirms `real-game-budget` green before the
   phase stops early. (plan-pilot: one PRD per lever, or per group of
   small levers that touch the same code.) A lever measured on
   `psx-full-game-check` is first probed (`psx-probe`) on a run that
   draws its target function — a representative run of another situation
   if one does, else its best-try run. If that probe shows no gain on the
   target function, the lever is reverted with no full-game check.
   Consecutive levers of this kind in rank order then share one
   `psx-full-game-check`, at most three per check, the last group sharing
   step 8's. Each situation's mean update and draw are compared with the
   previous full-game table (`make situation-report` on both). A lever's
   measured gain is that per-situation delta, split between the group's
   levers in proportion to their probe gains. If a group shows no gain in
   the situation its levers target, it is split and re-measured one lever
   at a time.
8. End of phase: `psx-full-game-check`, with `real-game-budget`'s figures
   and the steady frames still over, per situation, in the note.

### Phase 4 — Close

9. Close the plan: write the result. If `real-game-budget` is green, say
   so. If not, list the steady situations still over, with their frames,
   worst and mean, what bounds them, and the picture-changing options
   from step 5's separate section with their gains, for the human to
   choose from; put that list in `docs/psx-game.md`'s "Still to do".
   Update `docs/psx-game.md`; delete `docs/psx-levers.md`. Verified when
   `make test-all` and `psx-check` are green and the docs describe only
   what is true.

## 4. Done when

- `real-game-budget` is green in `psx-full-game-check`, or every step is
  done and `docs/psx-game.md`'s "Still to do" lists the steady situations
  still over and the picture-changing options for the human.
- `make test-all` and `psx-check` are green; every capture at `differ 0`
  or under its existing ceiling.
- No e2e hash changed except step 6's re-record; every `.end-state`
  unchanged.
- `docs/psx-levers.md` is removed and `docs/psx-game.md` is up to date.

## 5. Open questions

None.

## 6. Step notes

### Step 1.1

One `psx-full-game-check` on the starting HEAD (asserts off, O3); its
table replaces `psx/perf/full-run-frames.txt` (6,059 frames). Red on
`frame-budget` only, every other condition green. Frames over 263: 1,764
(plan 21: 1,764); worst frame 12 at 3,992, update 3,711, draw 275, 15
vsyncs (plan 21: 3,992); mean frame 233.6 (plan 21: 233.6). No drift: the
old committed table predated D8. Over-budget frames: draw 424,627 of
598,793, mean overrun 76 hblanks. No deviation, no guideline decision.

### Step 1.2

`make situations` labels every frame of `full_game.json` from two game
snapshots (`src/devtools/situation.c`, `src/cmd/situations.c`); table
committed at `psx/perf/full-run-situations.txt` (6,059 frames), checked by
`make situations-check`; rule in `docs/psx-game.md` § Situations, frozen;
`test_situation_table` holds the hand-picked frames (one per situation
present) in one list for step 6. `make situation-report` on step 1's table
(frames / over 263 / mean update / mean draw):

- steady: map-rest 1,564 / 429 / 41 / 168; map-move 1,907 / 388 / 41 /
  160; build 366 / 155 / 36 / 215; menu 252 / 91 / 29 / 203; slipway-aim
  111 / 41 / 73 / 189; tech-panel 47 / 21 / 31 / 242; score-panel 17 / 8 /
  98 / 192; title 12 / 0. Steady frames over: 1,133.
- one-off (frames / over / occurrences / longest run): screen-change
  1,460 / 460 / 353 / 31; slipway-built 166 / 82 / 34 / 14; colonise 74 /
  32 / 18 / 11; new-year 61 / 36 / 24 / 11; structure-built 10 / 10 / 10 /
  1; research 9 / 9 / 9 / 1; galaxy-generation, game-end, probe 1 each.
  One-off frames over: 631. `return-to-title`, `pause`, `attract` do not
  occur in `full_game.json`.

Guideline decisions: the "long" bound is 31 frames, not the PRD's 30 — a
planet project's burst sets the camera shake to 3, decaying 0.1 a frame
(30 frames) plus the event frame; full_game has two such `screen-change`
runs, at f4952 and f5339; no burst event added, shake continuation kept.
`screen-change` (a panel sliding in or out, 5–18 frames per action) stays
one-off although it covers 24% of the frames. A stretch of one one-off
label counts as one occurrence (event frames are not marked; can only
over-flag). Deviation: the table's default output is
`tmp/situations-table/`, since `tmp/situations` is the binary. A
pre-existing bug surfaced (pad_author's self-check failed since plan 10
on the draw-meter lines), fixed in its own `fix:` commit with a
`pad-author-self-check` in `make test`.

### Step 1.3

`tools/psx_real_game_budget.sh` joins the frame and situation tables;
`real-game-budget` is a condition of `psx-full-game-check` (after
`cap-sizes`), and `make psx-real-game-budget` runs it on the committed
tables without booting. Fixture tests in `psx-tools-test` (green, red on
one steady frame, red on the one-off sum, edges, join failures). On step
1's table: **red** — 1,133 steady frames over 263 (worst f4655, 530,
`build`, the steady build-mode screen); 631 one-off frames over, 1,391 of
10,800 vsyncs. PRD decisions, not the plan's text: a one-off frame counts
`ceil(total / 263)` vsyncs (the measured column depends on where the frame
started; 2,265 vs 3,658 over all 1,764 frames, both far under 10,800);
`frame-budget` stays a failing condition of the check, unchanged — the
plan's verdicts read `real-game-budget`.

### Step 1.4

`make steady-windows` ranks 15-frame windows per steady situation;
`make short-run-situations` checks a run's labels against the frozen
table. Tries (label, then `representative` vs step 1's table; PSX draw /
update gap): map-rest 5538 −27.5/−4.2, 5423 −29.1/−1.5, 5581 −15.6/−12.2;
**map-move-1057 ok** −0.5/−0.1; slipway-aim 5123, 5015, 5715 all relabelled
after the resume (not booted); build 5923 −23.7/−0.2, 5054 −32.3/−21.2,
5752 −23.6/−9.1; **menu-5954 ok** +4.0/+0.3; **tech-panel-5895 ok**
+0.8/−1.0; score-panel 5556 −38.4, 5441 −37.8, 5876 −11.6/−2.2. So
**no representative run** for map-rest (429 frames over), build (155),
score-panel (8) and slipway-aim (41): best tries kept in the manifest,
flagged; their levers are measured on `psx-full-game-check`. Every failed
resumed try draws less, mostly fewer `pset`; cause unproven (a transit dot
is one `pset` + one `rectfill`, yet map-rest-5538 lacks 2,686 `pset` and
183 `rectfill`), recorded as an open finding in `docs/psx-game.md`.
Probe baseline refreshed: 12 runs valid.

`make steady-profile` (judge hblanks per window frame; draw groups split
primitives / game code; overhead pooled, 0.0558 h per call vs plan 21's
0.13). Biggest groups: map-rest slipways 71.5 (24.9%), planets 65.3,
panels 40.3, frame 36.0, sum 285.7 vs 287.3; map-move map 109.5 (31.1%,
`wormhole` 94.1 in primitives), update_entities 55.8, save_at_frame_end
28.0, sum 353.8 vs 352.0; build slipways 61.1, planets 50.9, ui_update
34.0, sum 243.6 vs 242.1; menu slipways 99.5 (31.9%), planets 81.3, sum
309.4 vs 311.6; tech-panel slipways 87.3, planets 77.9, hud 53.9, panels
41.3, sum 367.3 vs 366.9; score-panel slipways 75.5, planets 61.7, update
input_step_run 43.1 + update_entities 37.5, sum 342.3 vs 341.0. Every run
`adds-up ok` within its noise floor; slipway-aim has a no-run profile (PC
shares of its 41 frames over: slipways 18.6%, map 14.4%, panels 13.9%,
ui_update 17.3%), weaker evidence. `slipway_draw_method`'s game code
(45–76 h) may be overstated by PC hook cost.

Guideline decisions: follow the PRD's three tries and fallback rather
than investigate a longer warm-up before the window (out of the PRD's
scope); the transit-dot cause is written as unproven. PRD choices: noise
floor per run = max(3 h a frame, 1% of the window); whole child time, not
"net", for game code (net was below in-call time); `score_panel` in `hud`
(always-on bar).
plan-review after this step: four method changes accepted and applied —
step 5 takes a no-representative situation's shares from the full game's
own frames (`no-run`); step 7 probes a full-game-measured lever first and
groups up to three per `psx-full-game-check`, rebases the projection on
each new frame table (`in-base`), and scales remaining estimates by the
measured-to-estimated ratio per estimation basis.
