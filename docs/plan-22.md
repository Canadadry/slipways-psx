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
   small levers that touch the same code.) Each lever's PRD lists the
   open warn-review warnings (phases 1 and 2) that fall in the files it
   changes, and decides each one: acted on in that PRD, or left with its
   reason. The step note records which. A lever measured on
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
   from step 5's separate section with their gains, re-estimated on step
   8's frame table and profiles; for each option alone, the steady frames
   still over (nominal and pessimistic, per situation) from
   `lever_estimate.sh` run on a lever-table file in `tmp/` (`LEVERS=`),
   options kept unranked, in the section's order; for the human to
   choose from; put that list in `docs/psx-game.md`'s "Still to do".
   Update `docs/psx-game.md`; delete `docs/psx-levers.md`. The
   `lever-estimate` target, its script and its doc lines are re-sourced
   or removed in the same step, as step 5 did for plan 21's (no target or
   doc may name a deleted file). Every warn-review warning of plan 22
   (phase 1's, phase 2's, phase 3's) is acted on only in code a step of
   this plan already touches (as for plan 21's). Those still open at the
   close are listed in `docs/psx-game.md`'s "Still to do", together with
   the open findings that step notes recorded but no doc carries yet
   (step 2.6's red `make worst-case-peaks`). Verified when `make test-all` and
   `psx-check` are green, `grep -rn psx-levers Makefile tools docs src
   tests` finds nothing outside plan files, every warn-review warning of
   the plan is either named as acted on in a step note or listed in
   "Still to do", and the docs describe only what is true.

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

### Step 1.5

`docs/psx-levers.md`: M = 218.9 h (steady frames' mean cost), T = 4.4 h;
19 functions above 2% (16 overall, 3 in one situation only). Eight
pixel-identical levers ranked: D3, N1, N2 (after N1), N3, N5 (after N2),
G4, N7, N6 (after N2). `make lever-estimate` re-sourced to its
`lever-table` block (estimated / measured / in-base, basis calls /
share, `scale` lines for step 7); `lever-estimate-check` and plan 20's
three tables removed. Projection on step 1's table, steady frames over
(pessimistic): 1,133 → D3 1,058 (1,068) → N1 991 (1,036) → N2 774 (861)
→ N3 711 (806) → N5 624 (771) → G4 551 (724) → N7 507 (690) → N6 487
(656); end per situation: map-rest 212, map-move 158, build 49, menu
29, slipway-aim 19, tech-panel 14, score-panel 6. **Pass 1 cannot reach
`real-game-budget` on these estimates**; five picture-changing ideas
(P1–P5) are kept unranked for the close. Weaker evidence: D3, N1, G4, N6.

No-run vs best-try shares differ by more than a fifth on many functions
(map-rest `background_draw_method` 17.6% vs 3.9%; build
`message_box_draw_overlay_method` 13.3% vs 0.0%; score-panel 27.1% vs
9.8%), listed in the document; no-run shares are PC time and overstate
primitive-heavy functions (menu-5954: background 18.2% of PC draw time,
5.9% of PSX draw). Plan 21's leftovers: D3, G4 kept; D4, D12 (a loss),
D13 (covered by N5), L3, B2, O1, L2 dropped. Deviation: N3's map-move
gain is sized on the full game's circle count (62 of 388 frames over
draw one), not on map-move-1057's window, which draws a wormhole every
frame (4.1% vs 25%). Estimates resting on assumptions: N2, N3, G4, N6.

End of phase 1: warn-review raised 13 warnings on phase 1's code
(duplicated table join and situation lists across five scripts and the
Makefile, inline 263 / 31 / 15 / 3, two variables for one table, 55
untested recipe lines in steady-profile, `end_status != NULL` next to
its bit in situation.c, pad_author.c at 1,540 lines). plan-review: two
changes accepted to step 9 — lever-estimate re-sourced or removed with
`docs/psx-levers.md`, warnings acted on only in touched code and the rest
listed in "Still to do"; the picture-changing options re-estimated on
step 8's table as steady frames over per option.

### Step 2.6

Loading screen in the shared game code (`src/game/loading_screen.c`,
`cart_request_start_game`): the press frame and the next draw a black
screen with "loading" in colour 7 at (132, 114) and run no game code;
the third frame generates the galaxy. **Two frames, not one** (PRD
decision): the PSX shows each frame one frame later than the PC, so one
loading frame would still show the title during the stall. A new
`display` capture (what the PSX shows at the top of a frame, compared
with PC frame F−2) proves it: `display 14` and `all 12` at differ 0.
Recordings: two copies of each difficulty press inserted (14 e2e, 7
fixtures, the worst-case games; `full_game.json` 6,059 → 6,061 frames,
f ≥ 12 → f + 2). Only these 14 `.sha256` changed (pad_build,
pad_colonise, pad_empty_planet, pad_end_turn, pad_game_end,
pad_message_box, pad_pause_restart, pad_probe, pad_research,
pad_restart_resume, pad_score, pad_slipway, pad_swap_aim_menu,
pad_title), after checking each new capture is the old one plus two
loading frames; every `.end-state` unchanged (25 match). Renumbered:
situation table (old one + `12/13 title steady`), hand-picked frames,
short-run windows (names kept; all six steady runs `same`, the three
representative ones still `ok`), capture list (`pattern 190` → 193,
off digest frame 192), image-build frames; `psx-check` all 32
conditions. `psx-full-game-check` on the new numbering: 1,763 frames
over (step 1: 1,764), worst f14 3,989, mean 233.5; real-game-budget
1,132 steady over, one-off 631 frames / 1,391 vsyncs (not worse).
`lever-estimate` reordered N5 before N3 on the new table (nothing
re-estimated): end 489, pessimistic 655. Probe baseline re-taken after
the commit, valid. Deviation: the text sits 1 px from the edges, not
the PRD's 2 (its coordinates kept). Outside the step: `make
worst-case-peaks` has failed since plan 12 on two frozen lines and
since plan 21 (59fc46a) on `zone frame`, a stale table, not a code
bug; re-baselining it is the human's call, left open.

End of phase 2: warn-review raised 13 warnings on step 6's code (the
display mask as a 0x8000 sentinel in the family bitmask, repeated as
`8000` in the Lua and inline in two awk scripts; psx_gpu_replay.c
reading a protocol value from the host parser; cart.c holding a whole
`button_t` as the difficulty carrier, copying its fields one by one, and
duplicating cart_update60's prologue and epilogue; "one frame late" vs
F−2 wording; loading_screen.c in ENGINE_SRCS; e2e frame numbers inline
everywhere; pad_author.c at 1,549 lines with no unit test). plan-review:
two changes accepted — step 9 carries every phase's warnings and the
unrecorded findings to "Still to do", and checks it; step 7's PRDs list
and decide the open warnings in the files they change.

### Step 3.7 — D3

Lever D3, message-box text through the text image cache
(`hud_print_outlined_text_image`, `draw_functions_draw_row_text_image`,
`message_box_draw_article` routed through them and through
`layout_row_draw_at_text_image`). Residual before the change (throwaway
probe drawing the box's articles twice): tech-panel-5895 +36.3 h a
window frame, above T = 4.4, not covered. Captures: `psx-check` gained
a second capture replay, `pad_message_box` (year box `print 142`,
`all 143`, score box `all 164`), accepted at differ 0 on the code before
D3 (agent rule); after D3 all three match their references at differ 0,
all 39 conditions hold. Probe against the baseline at chunk 1's commit,
`game-state same` on every run: tech-panel-5895 draw 2,948 → 2,878
(−70 h over 9 frames, −7.8 h a frame, 2.4% of draw) against 18.6 h
(5.7%): ratio 0.42, applied to D3's other estimates (no `scale` line,
one `calls` lever measured). Weaker evidence: map-rest-5581 −94 over 15
frames, draw-5564 −102 over 15, score-panel-5876 +36 on its one frame
(the box's text slots built there), build-5752 +3 (no box); text +1,240
bytes. Every e2e `.sha256` and `.end-state` unchanged. Steady frames
over: D3 alone removes 35 (estimate 75); end of the list 547 nominal /
718 pessimistic (was 489 / 655). Re-ranked to the greedy order on the
measured gain: N1, N2, N3, N5, G4, D3 (6th, taken), N6, N7. RAM:
`ram-room` 142,056 bytes left, `game-ram-room` 295,500. Warn-review
warnings in the changed files: none in message_box.c, draw_functions.c,
hud.c, layout.c, text_image.c or the two tests; the Makefile's (phase 1
situation lists, inline constants, two table variables, steady-profile's
recipe lines; phase 2 loading_screen.c in ENGINE_SRCS) left, D3 changing
only test_hud's link line. No guideline decision.
Open risk: score-panel-5876's +36 h is beyond its 3 h noise floor and
its cause (slots built on a cold cache right after the resume) is
unproven; step 8's per-situation comparison checks score-panel and
menu, where the box stays open.
