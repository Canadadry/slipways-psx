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

### Step 3.7 — N1

Lever N1, slipway lines from integer end points: `slipway_draw_line`
floors its two end points once and builds each segment with `int` adds;
its offsets and step are `int` parameters, an assert keeps the floored
ends inside ±32,765 (where an `int` add matches `p8_fixed_add`'s wrap).
Residual before the change (throwaway probe running the segment loops a
second time into a `volatile` sink): map-move-1057 +2.5 h a window frame,
menu-5954 +10.3, tech-panel-5895 +8.0 (12.9%, 11.0%, 10.1% of
`slipway_draw_method`, pooled 10.8%); per steady frame 4.9 h, above
T = 4.4 (rule (a)), and menu and tech-panel above 2% of their totals
(rule (b)); not covered by D3. Best tries: map-rest-5581 +6.9, build-5752
+6.2, score-panel-5876 +9 on its one frame. Probe baseline reused: taken
at D3's commit, its fingerprint matches HEAD on every run. Probe,
`game-state same` on every run: map-move-1057 draw 3,308 → 3,268 (−40 over
15 frames, −2.7 h a frame, 1.2% of draw) against 3.0 h (1.3%); menu-5954
4,251 → 4,073 (−178, −11.9 h, 4.2%) against 14.1 h (5.0%);
tech-panel-5895 2,878 → 2,797 (−81 over 9, −9.0 h, 2.8%) against 11.9 h
(3.6%). Pooled ratio 23.5 / 29.0 = 0.81, applied to the no-run
situations (map-rest 2.8%, build 3.0%, slipway-aim 3.1%, score-panel
1.9%); no `scale` line, one `share` lever measured. Kept: the three runs
sum negative and none is positive; map-move-1057's −2.7 h alone is under
its 3.5 h noise floor. Weaker evidence: map-rest-5581 −120 over 15
(−8.0 a frame), build-5752 −119 over 15 (−7.9), score-panel-5876 −9 on
one frame; update within ±4 h everywhere; text −236 bytes. Every e2e
`.sha256` and `.end-state` unchanged; `psx-check` green, all 39
conditions, every capture of both lists matching its reference. Steady
frames over: N1 alone removes 56 (projection after D3: 69), 1,132 → 1,076
nominal / 1,088 pessimistic (was 1,063 / 1,080); end of the list 563 /
741 (was 547 / 718). Re-ranked: N1, N2, N3, G4, N5, D3 (taken), N7, N6
(G4 before N5, N7 before N6). RAM: `ram-room` 141,696 bytes left (asserts
on, the new range assert), `game-ram-room` 295,736. Warn-review warnings
in the changed files: none in slipway_display.c, slipway_display.h or
test_slipway.c; docs/psx-game.md's "one frame late" vs F−2 wording is
outside the projection line N1 edits; all left open. No guideline
decision.

### Step 3.7 — N2

Lever N2, fixed-point and vector helpers inlined: `p8_fixed_from_int`,
`_add`, `_sub`, `_mul`, `_neg`, `_flr`, `_flr_int`, `_abs`, `_sgn`,
`_band`, `_bor` move to `fixed.h` and `vec_make`, `vec_add`, `vec_sub`,
`vec_mul`, `vec_neg`, `vec_dot`, `vec_length_squared`,
`vec_perpendicular` to `vec.h`, `static inline`, bodies and `uint32_t`
casts unchanged, the wrap comment moved with `p8_fixed_add`; the heavy
helpers stay out of line. `test_fixed` gained three wrap cases (`sub` at
both int32 edges, `from_int` outside ±32,767), passing before and after
the move, under UBSan too. Hypotheses on N1 all held (N1 kept, ratio
0.81, re-rank N1, N2, …, projection 563 / 741, no scale line, no rebase,
N1 in slipway_display.{c,h} and test_slipway.c only). Not covered (N1
touched only `slipway_draw_line`'s helpers); under 2%: no, its own probe
delta is 31.4 h per steady frame (rule (a), T = 4.4) and 23.1 / 45.0 /
47.3 h on map-move-1057 / menu-5954 / tech-panel-5895 (rule (b): 7.0,
6.2, 7.3). Probe baseline reused: retaken at N1's commit, its fingerprint
matches HEAD on all 12 runs. Probe, all 12 valid, `game-state same` on
every run, text −7,712 bytes on each: map-move-1057 draw −241 over 15
(−16.1 h a frame, 7.4%) against 5.8 h, update −106 (−7.1 h, 5.6%) against
8.8 h, submit 0, total −347 (−23.1 h); menu-5954 draw −642 (−42.8 h,
15.8%) against 13.9 h, update −33 (−2.2 h, 9.9%) against 1.6 h, submit
+1, total −674 (−44.9 h); tech-panel-5895 draw −400 over 9 (−44.4 h,
14.3%) against 14.5 h, update −26 (−2.9 h, 9.2%) against 2.2 h, submit 0,
total −426 (−47.3 h). Draw ratio 103.3 / 34.2 = 3.02, update ratio
12.2 / 12.6 = 0.96, applied to the no-run situations (draw map-rest
13.0%, build 14.5%, slipway-aim 12.4%, score-panel 13.9%; update 6.8%);
N2's own ratio 115.5 / 46.8 = 2.47. Kept: the three runs' totals are all
negative, beyond every noise floor (3.5, 3.0, 3.5 h), map-move-1057's
alone included. Weaker evidence: map-rest-5581 draw −36.2 h a frame
(14.7%, scaled 13.0%), update −1.9; build-5752 draw −24.1 (13.5%, scaled
14.5%), update −17.1 (34.5%, scaled 6.8%); score-panel-5876 draw −34,
update −4 on its one frame; start −1,681 of update on its one frame;
update-4737 −30, update-5500 −44, update-5719 −3 of update; draw-5564
−508, draw-6000 −670 of draw over 15. `scale share 1.64`: the median
(mean, two levers) of N1's 0.81 and N2's 2.47, the first `scale` line;
it scales G4, N5 and N6, not re-derived. Steady frames over: N2 alone
removes 453 (estimate 157); after N1 and N2, 1,076 → 597 nominal / 701
pessimistic (N1's projection: 924 / 969); end of the list 269 / 418
(was 563 / 741). Re-ranked to the greedy order: G4, N3, N1 (taken), N2
(taken), N5, D3 (taken), N6, N7 — G4 scaled to 85% of submit and N3 each
remove more alone than N1, and N2 cannot precede N1; the lever table and
the ranked list were reordered to match (`order ok`). No
`psx-full-game-check`, no rebase. `psx-check` green, all 39 conditions,
every capture of both lists matching its reference; `ram-room` 148,252
bytes left (was 141,696), `game-ram-room` 303,448 (was 295,736); `make
raylib` links. Every e2e `.sha256` and `.end-state` unchanged. Fast loop,
before → after (header change, so every object rebuilt): `make test` cold
11.3 → 11.3 s, warm 7.45 → 7.33 s (mean of six); `make e2e` warm 0.52 →
0.49 s. The PC profiler still instruments the inlined helpers (one copy
per translation unit): `docs/psx-levers.md`'s "fewer children" note was
corrected. Warn-review warnings in the changed files: none in fixed.{c,h},
vec.{c,h}, test_fixed.c, or the levers and game docs' edited lines (the
"one frame late" vs F−2 wording sits outside the "Still to do" projection
line); phases 1 and 2 and plan 21's lists all left open. No guideline
decision.
Caution: N2's ratio (2.47, whole-program call overhead) drives the
`scale share 1.64` applied to G4, N5 and N6; those three are not
inlining levers, so their scaled estimates (G4 at 85% of submit) are
likely optimistic until measured.

### Step 3.7 — G4

Lever G4, slot uploads skipped when unchanged: `psx_gpu_frame_end` now
queues only what `psx_slot_uploads_collect` returns
(`src/platform/psx_slot_uploads.{c,h}`, new, host-tested by
`test_psx_slot_uploads`): one record of the exact words each palette CLUT
line, pattern tile and pattern CLUT line last received, shared by both
primitive buffers (a per-buffer record would compare frame N with N−2),
`memcmp` on the words, tile and pattern CLUT decided apart, forgotten by
`psx_gpu_init`. The host test covers the first frame after `forget`, an
identical frame from the other buffer, one changed palette, A, B, A on one
slot, a cache shrinking and growing back, tile vs pattern CLUT, a swap and
`forget`. Asserts-on builds read back (`StoreImage`), every frame, the
slots the previous frame drew with and assert they hold its words;
`psx-check`'s `asserts-on` now also requires `asserts on` from the first
boot of both GPU capture replays (their boots moved before the condition;
still 39 conditions).
Baseline: the one re-taken at N2's commit, fingerprint matching HEAD on
all 12 runs, reused. Not covered (D3, N1, N2 touch neither
`upload_slots` nor the caches). Residual probe (throwaway: every upload
whose words equal its slot's queued a second time), all 12 valid,
`game-state same`: submit map-move-1057 +109 over 15 (+7.3 h a frame),
menu-5954 +109 over 15 (+7.3), tech-panel-5895 +89 over 9 (+9.9),
map-rest-5581 +109 over 15 (+7.3), build-5752 +126 over 15 (+8.4),
pooled representative 307 / 39 = 7.9 for slipway-aim and score-panel
(score-panel-5876 alone +7 on its one frame). Rule (a): 7.39 h per steady
frame (weights ÷ 4,276 as the PRD fixed; they sum to 4,264, 7.41), above
T = 4.4; rule (b): every situation above its threshold (7.3 > 6.2, 7.3 >
7.0, 7.9 > 6.6, 8.4 > 6.2, 7.3 > 6.2, 9.9 > 7.3, 7.9 > 7.2). Verdict:
build. Identical uploads: 3,994 of 4,188 over the 12 runs (95%;
map-move-1057 97%, menu-5954 95%, tech-panel-5895 96%), against the 70%
assumed. The doubled uploads cost more than the whole baseline submit
(about 6 h), so the residual is an upper bound only.
Mutation proof (throwaway: never re-upload a known slot): `psx-check`
failed at `gpu-replay-done`, exit 4 on both GPU replays, `assert:
psx_gpu.c:560 slots_hold_caches(...) && "a slot the last frame drew with
did not hold its content: an upload was skipped"` — `pad_slipway` right
after its frame-14 capture (first capture missing: f102), and
`pad_message_box` before its first capture (f142).
Probe of G4 (the diff of `src/`, `tests/`, `tools/` and the Makefile), all
12 valid, `game-state same`, text +580 bytes: map-move-1057 submit 90 →
40 (−50 over 15, −3.3 h a frame, 55.6% of submit), total −52 (−3.5 h);
menu-5954 submit 90 → 39 (−51, −3.4 h, 56.7%), total −53 (−3.5 h);
tech-panel-5895 submit 70 → 27 (−43 over 9, −4.8 h, 61.4%), total −42
(−4.7 h); draw and update within ±4 h on every run. Measured against
52%: pooled 144 / 250 = 57.6%, ratio 1.11; map-rest, build, slipway-aim
and score-panel get 57.6%. Kept: map-move-1057's submit and total
negative, the three totals all negative and beyond their noise floors
(3.3, 3.0, 3.0 h), no total positive on any run. Weaker evidence:
map-rest-5581 −53 over 15, build-5752 −55 over 15, draw-5564 −59,
draw-6000 −51, the one-frame runs −3 to −4 of submit.
`scale share 1.11`: the median of N1's 0.81, N2's 2.47 and G4's 1.11 — the
step 7 rule re-derived with a third `share` lever; it scales N5 and N6
(N5 alone 40 frames scaled, 35 unscaled; N6 26, 23). Steady frames over:
G4 alone removes 37 (estimate 68 scaled, 36 unscaled); after N1, N2 and
G4, 537 nominal / 655 pessimistic (N2's after N1 and N2: 597 / 701),
after the four taken levers (D3 too) 481 / 616; end of the list 309 / 456
(was 269 / 418: G4 measured under its 1.64-scaled estimate, and the lower
scale shrinks N5 and N6). Re-ranked to the greedy order: N1, N2, N3, D3,
N5, G4, N7, N6 — G4 alone now removes less than N2, N3, D3 and N5; the
lever table and the ranked list were reordered to match (`order ok`). No
`psx-full-game-check`, no rebase. `psx-check` green, all 39 conditions,
every capture of both lists matching its reference, no ceiling or limit
changed; `ram-room` 146,236 bytes left (was 148,252), `game-ram-room`
302,404 (was 303,448); `make raylib` links. Every e2e `.sha256` and
`.end-state` unchanged. Fast loop, before → after (warm, mean of four):
`make test` 7.70 → 7.55 s, `make e2e` 0.53 → 0.51 s.
Warn-review warnings in the changed files: none in `psx_gpu.c`,
`psx_slot_uploads.{c,h}`, `test_psx_slot_uploads.c` or `psx_check.sh`
(the readback helper is a static function inside an `assert`, not an
`#ifdef` block); hand-copied condition counts left open (the count stays
39); the Makefile warnings (phase 1's lists, constants and table
variables, `loading_screen.c` in `ENGINE_SRCS`, plan 21's 3,000-line
Makefile) left open, G4 adds one test target, one list entry and one
backend source; "one frame late" vs F−2 left open (G4 edits the backend
paragraph, the `asserts-on` line and the "Still to do" figure only);
every other warning left open, outside G4's files. G4 also adds two
lines to `tools/float_grep_exclusions.txt` for the new module. No
guideline decision.

### Step 3.7 — N3

Lever N3, PSX circle outline drawn as runs: `psx_tiles_circ` groups the
midpoint steps that share `x` and emits their pixels with `|dx| >= |dy|`
as column runs and the others (`y < x`) as row runs, the two halves of a
run merged on the axis, each run clipped once, no duplicate check;
`gpu_circ`'s callback sends a 1×1 run as a `TILE_1` and a longer one as a
`TILE` (`add_tile_rgb`, shared with `add_tile`). The midpoint loop,
`gfx_software.c` and `draw_cost.c` are unchanged. `test_psx_tiles` now
accepts runs (1 wide or 1 high), still asserts no overlap, the software
pixel set and real bytes (`TILE_1` 12, `TILE` 16) under the meter, pins
the tile counts (radius 2, 3, 4: 8, 12, 16; radius 9: 28, 24: 64, 26: 68;
red on per-pixel emission at radius 9) and draws 200 seeded random
circles (centre −60..220 × −60..180, radius 0..70, camera ±30; over 50 of
them cut by an edge). A mutation letting the row runs reach the diagonal
fails on "tiles overlap".
Hypotheses on N2 checked: N2 kept (2.47), step note `### Step 3.7 — N2`;
then G4, re-ranked before N3, was taken and kept (1.11), so N3's note
follows G4's and compares against G4's projection (309 / 456), per
H-N2-4. The `scale share` line is 1.11 (N1, N2, G4), no `scale calls`
line existed; no `psx-full-game-check`, no rebase; `psx-check` green with
39 conditions at G4's end; G4 changed `psx_gpu.c` (slot uploads,
readback assert), not the circle path.
Capture (chunk 1, own commit `06830fd`): `circ 245 cls,circ`, the pad
ring cut by the top edge, accepted on the code before N3 at `differ 0`
(ceilings, green `psx-check`, references); 18 captures on `pad_slipway`,
GPU replay 260 vsyncs, no limit raised.
Baseline: `make psx-probe-baseline` re-taken by the pilot at `06830fd`,
all 12 runs valid. Not covered (D3, N1, N2 and G4 leave the circle path
alone). Residual probe (throwaway: `gpu_circ` runs `psx_tiles_circ`
twice), all 12 valid, `game-state same`: map-move-1057 draw +1,414 over 15
(+94.3 h a frame, 20.1 h a `circ` at 4.7 a frame), map-rest-5581 +91 over
15 (+6.1), every other run within ±16 over its window. Rule (b): 94.3
above 2% of map-move-1057's total (322.7 → 6.5). Rule (a), an upper
bound (frames-over basis): 20.1 h × circles per frame over (map-move
294 / 388, map-rest 246 / 429, menu 10 / 91, score-panel 3 / 8) weighted
by steady frames ÷ 4,278: 11.1 h (8.2 with map-rest-5581's own 6.1 h a
call), above T = 4.4. Verdict: build.
Probe of N3 (the diff of `src/` and `tests/`), all 12 valid, `game-state
same`, text +368 bytes on each: map-move-1057 draw 3,023 → 2,369 (−654
over 15, −43.6 h a frame, 21.6% of draw), update −2, submit −1, total
−657 (−43.8 h); map-rest-5581 draw −22, update 0, submit +1, total −21;
build-5752 total +1, draw-5564 +1, draw-6000 −5, menu-5954 +1,
tech-panel-5895 0 (over their windows); score-panel-5876 +1, start −3,
update-4737 0, update-5500 0, update-5719 −1 (one frame each). Measured
against the window's own estimate 4.7 × 20.158 × 0.6 = 56.8 h: ratio
43.6 / 56.8 = 0.77; the run form removed 46% of the circles' measured
cost (the residual), against 60% assumed. Gains = estimate × 0.77:
map-move 3.2%, map-rest 0.6%, menu 0.4%, score-panel 1.4% of draw.
Kept: map-move-1057's draw and total deltas negative, no run's total
positive beyond its noise floor (largest +1), `game-state same`
everywhere. Weaker evidence: map-rest-5581 −1.5 h a frame against 0.6 ×
5.716 = 3.4 (ratio 0.43), under its 3 h noise floor.
`scale calls 0.60`: the median (mean, two levers) of D3's 0.42 and N3's
0.77 = 0.595, rounded half up; the first `calls` scale, it scales N7
alone (alone 12 frames scaled, 20 unscaled). Steady frames over: N3
alone removes 36, all in map-move (estimate 50, 48 in map-move); after
the five taken levers (N1, N2, N3, D3, G4) 423 nominal / 563
pessimistic (G4's note: 481 / 616 after the four); at N3's rank 526 /
660 (G4's projection, estimated: 509 / 645); end of the list 336 / 482
(was 309 / 456: N3 under its estimate, and the 0.60 scale shrinks N7).
Re-ranked to the greedy order: N1, N2, N3, N5, D3, G4, N6, N7 (N5 now
passes D3, N6 passes N7); the lever table and the ranked list were
reordered to match (`order ok`). No `psx-full-game-check`, no rebase.
`psx-check` green, all 39 conditions, every capture of both lists
matching its reference, no reference or ceiling changed after chunk 1;
`game-gpu-cost` bytes 7,976 of 8,016, build 10,644 → 10,388 of 10,916
(the planet ring image, built by `circ`); `ram-room` 145,868 bytes left
(was 146,236), `game-ram-room` 302,036 (was 302,404); `make raylib`
links. Every e2e `.sha256` and `.end-state` unchanged. Fast loop, warm,
HEAD and N3 copies interleaved, mean of four: `make test` 6.96 → 7.18 s,
spread 6.6–7.4 s on both, `make e2e` 0.45 → 0.43 s: within noise
(`test_psx_tiles` runs in 0.09 s in all).
`docs/psx-game.md`: the backend paragraph's `circ` sentence (runs), the
"Capture list" bullet (two `circ` captures, chunk 1) and the "Still to
do" figure (336 / 482).
Warn-review warnings in the changed files: none in `psx_tiles.{c,h}`,
`psx_gpu.c`, `test_psx_tiles.c` or the capture lists; "one frame late"
vs F−2 in `docs/psx-game.md` left open (N3 edits only the `circ`
sentence, one "Capture list" sentence and the "Still to do" figure);
the Makefile warnings left open (the Makefile is not changed);
hand-copied condition counts left open (the count stays 39); every
other warning left open, outside N3's files. No guideline decision.

### Step 3.7 — N5

Lever N5, on-screen test against the draw pass's screen box: **not taken,
under 2%**; no code changed, nothing committed but the bookkeeping.
Hypotheses on N3 checked, all held: N3 kept (ratio 0.77), its capture
chunk (`circ 245`) and the lever committed, note `### Step 3.7 — N3`;
it wrote `scale calls 0.60` and left `scale share 1.11`; its re-rank
(N1, N2, N3, N5, D3, G4, N6, N7) left N5 the next untaken lever; it ran
no `psx-full-game-check` (no rebase, no `in-base`), changed only
`psx_tiles.{c,h}`, `psx_gpu.c`, `test_psx_tiles.c`, capture files and
docs, acted on no warning, and left the projection at 336 / 482 with
`psx-check` green (39 conditions, both capture lists matching).
Baseline: the one re-taken at N3's commit, fingerprint matching `HEAD` on
all 12 runs, reused. Covered? No: N2 inlined the helpers but left the
out-of-line `main_camera()`, the camera's position read and the per-call
camera-plus-half-screen sums, N5's target; D3, N1, G4 and N3 do not touch
it. Residual probe (throwaway: `entity_geom_is_on_screen` looks up
`pos_of(main_camera())` a second time and adds the half screen per axis
into a `volatile` sink), all 12 valid, `game-state same`, text +64 bytes:
draw map-move-1057 +46 over 15 (+3.1 h a frame), menu-5954 +66 over 15
(+4.4), tech-panel-5895 +40 over 9 (+4.4), map-rest-5581 +67 over 15
(+4.5), build-5752 +72 over 15 (+4.8), pooled representative 152 / 39 =
3.9 for slipway-aim and score-panel (score-panel-5876 alone +4 on its one
frame); draw-5564 +67, draw-6000 +67 over 15; start +3, update-4737 +3,
update-5500 +3, update-5719 +4 on their one frame. Update within ±5 h
over every window (near 0, as expected). The probe counts every call,
also the ones N5 would leave on the wrapper (`emitter_update`,
`planet_draw`), so it is an upper bound on what N5 removes. Rule (a):
3.84 h per steady frame (weights ÷ 4,276 as the PRD fixed; ÷ 4,264, their
sum, 3.85), under T = 4.4. Rule (b): every situation under its threshold
(map-rest 4.5 < 6.2, map-move 3.1 < 7.0, slipway-aim 3.9 < 6.6, build 4.8
< 6.2, menu 4.4 < 6.2, tech-panel 4.4 < 7.3, score-panel 3.9 < 7.2).
Verdict: under 2%, not taken, as the PRD expected: N2 had already
removed the helper-call share the estimate counted on.
Skipped bookkeeping: N5's `lever` and `gain` lines removed from the lever
table; its ranked entry kept as a short record out of the numbered
ranking (residual per situation, both rules); the "What each function
gets" row for `entity_geom_is_on_screen` now says "no lever: N5's
residual under 2%", and `slipway_draw_method`'s row drops N5; plan 21's
D13 row says N5 was not taken. `scale share` stays 1.11 (N1, N2, G4; no
new ratio), so N6 is still scaled by 1.11; `scale calls` 0.60 untouched.
Steady frames over: after the five taken levers 423 nominal / 563
pessimistic (unchanged); end of the list 379 / 530 (N3's projection:
336 / 482; N5's projected 40 frames scaled are gone, plus N6's and N7's
overlap with it). Re-ranked to the greedy order: N1, N2, N3, D3, G4, N7,
N6 — without N5, N7 passes N6 again; the lever table and the ranked list
were reordered and renumbered to match (`order ok`), and plan 21's table
now ranks D3 4th and G4 5th by measured gain (it said 6th for both). The
next untaken lever is N7. No `psx-full-game-check`, no rebase.
No code, `psx/` or Makefile change, so `psx-check`, the captures, the e2e
hashes and `.end-state` files, `ram-room` / `game-ram-room` and the fast
loop are as N3 left them; `make test-all` green. `docs/psx-game.md`: the
"Still to do" figure (379 / 530).
Warn-review warnings: N5 changes no code file, so none of its code
warnings apply; "one frame late" vs F−2 in `docs/psx-game.md` left open
(only the "Still to do" figure changes); `situation.c`'s `end_status !=
NULL` and `loading_screen.c` in `ENGINE_SRCS` left open (files not
changed); every other warning left open, outside N5's files. No
guideline decision.

### Step 3.7 — N7

Lever N7, buttons drawn once through image slots: **taken and kept**
(ratio 1.62). A new game module `button_image.{c,h}` keeps 18 p8 image
slots appended after the ring slots (70–77, 16×11, icon pool; 78–87,
112×11, wide pool), keyed by four words, built through `p8_image_build`
into the least recently used slot of the pool not copied this frame, then
copied; it returns 0 (draws nothing) for a box over 112×11 or a full pool.
`layout_row_t` gains a `serial` (new and never 0 on each fill, by
`layout_row_new` or `layout_row_init`); `hud_palette_block_default_in_force`
compares the draw palette with block 0; `button_draw` keeps its contract:
its old body is one static function (text through the text cache or
direct), and a button whose width, side-tooltip width and item offsets are
whole, drawn with no fill pattern under block 0, is copied from an image
keyed by (row serial, side-row serial, body | top colour, width), built
with direct prints at (1, 0); otherwise the old direct path. `cart_draw`
starts the cache's frame next to `text_image_frame_begin`.
Hypotheses on N5 checked, all held: N5 skipped with no code change (note
`### Step 3.7 — N5`), `scale calls 0.60` (D3, N3), order N1, N2, N3, D3,
G4, N7, N6, projection 379 / 530, `psx-check` green with 39 conditions.
Residual (before any work), on the baseline re-taken at N3's commit
(fingerprint matching `3d3daca` on all 12 runs): a throwaway probe drawing
every button twice, all 12 valid, `game-state same`, text +44 bytes. Draw
a window frame: tech-panel-5895 +294 over 9 (+32.7 h), menu-5954 +193 over
15 (+12.9), map-move-1057 +103 over 15 (+6.9), map-rest-5581 +100 over 15
(+6.7), build-5752 +94 over 15 (+6.3); pooled representative 590 / 39 =
15.1 for slipway-aim and score-panel; draw-5564 +549, draw-6000 +182 over
15; one-frame runs +6 to +7. An upper bound (it counts the copy N7 keeps).
Rule (a): 7.6 h per steady frame (÷ 4,276), above T = 4.4. Rule (b): over
the threshold in six situations (tech-panel 32.7 > 7.3, menu 12.9 > 6.2,
slipway-aim 15.1 > 6.6, score-panel 15.1 > 7.2, map-rest 6.7 > 6.2, build
6.3 > 6.2), map-move 6.9 < 7.0. Verdict: build.
Captures (chunk 1, own commit `fa9f0f6`): `rectfill 300 cls,rectfill`
(24 calls, the map panel's fills and bars) and `all 155 all` on
`pad_message_box` (120 calls, the tech panel's buttons with side
tooltips), both at differ 0 on the code before N7; ceilings, a green
`psx-check`, references. Deviation: `tools/psx_capture_record.sh
ceilings` tolerates one failed ceilings condition per run, so with a new
capture in each list the ceilings were written in two `psx-check` passes
(`all 155` taken out for the first). 19 captures on `pad_slipway`, 4 on
the message box; no limit raised. The pilot then re-took the probe
baseline at `fa9f0f6`.
Probe of N7 (the diff of `src/`, `tests/` and the Makefile against
`fa9f0f6`), all 12 valid, `game-state same`, text +2,616 bytes. Window
deltas (draw, update, submit, total): tech-panel-5895 2,397 → 2,210
(−187 over 9, −20.8 h a frame), −3, 0, −190; menu-5954 3,432 → 3,345 (−87
over 15, −5.8 h), −1, 0, −88; map-move-1057 2,369 → 2,346 (−23 over 15,
−1.5 h), −8, −1, −32; map-rest-5581 −118, −1, −2, −121; build-5752 −34, +1,
+1, −32; draw-5564 −387, −1, −2, −390; draw-6000 −79, +4, +1, −74 (over
15); one frame: score-panel-5876 −4 / −5, start +7 / +8, update-4737 −2 /
−2, update-5500 0 / +2, update-5719 −8 / −6 (draw / total). Ratio, pooled
against the unscaled window estimates: (20.8 + 5.8 + 1.5) / (11.1 + 4.0 +
2.3) = 28.1 / 17.4 = 1.62; each run 1.87 (tech-panel), 1.45 (menu), 0.67
(map-move). Above 1: the estimate priced only `rect`, `rectfill` and
`spr`, not the label copies, the palette-block switches and the game code
around them, which the copy also removes. Weaker evidence: map-rest-5581
−7.9 h a frame and build-5752 −2.3 against 2.1 h, score-panel-5876 −4 on
its frame. Gains = estimate × 1.62: tech-panel 5.5%, menu 2.3%, map-move
1.6%, map-rest, build and score-panel 1.3% of draw; slipway-aim none.
Kept: tech-panel-5895's draw and total deltas negative, no run's total
positive beyond its noise floor (largest +8 on `start`'s one frame,
under its floor of max(3 h, 1% of 2,297) = 23 h), `game-state same`
everywhere, every
capture at differ 0. Measured on `tech-panel-5895` (lever line `measure
tech-panel-5895`), not `map-move-1057`, where the 2.3 h estimate sat under
the run's noise floor. The patch probed was taken before the 32-bit slot
sizes below were raised; that change moves no code path.
`scale calls 0.77`: the median of D3's 0.42, N3's 0.77 and N7's 1.62; no
`estimated` `calls` lever remains, so it scales nothing. `scale share`
stays 1.11 (N6 still scaled by it).
Alone, N7 removes 33 frames (estimate 12 scaled, 20 unscaled): map-move
16, build 7, map-rest 5, tech-panel 5. Steady frames over: after the six
taken levers 364 nominal / 521 pessimistic (after N5's five: 423 / 563);
end of the list 347 / 506 (N5's projection: 379 / 530). Re-ranked to the
greedy order N1, N2, N3, N7, D3, G4, N6: N7 moves from 6th to 4th; the
lever table and the ranked list were reordered and renumbered (`order
ok`), and plan 21's table now ranks D3 5th and G4 6th. N6 is the next
untaken lever. No `psx-full-game-check`, no rebase, no `in-base`.
References re-accepted under the agent rule, every capture of each label
at differ 0 (the copies carry the mask bit and the draw filter drops a
whole button from a family capture whose mask lacks one of its
families): `rectfill` (f11 now holds only `cls`, as the PRD foresaw, and
f300), `all` (f12, f102, f190, f235, f267), `pattern` (f193, f259: their
mask has no `rect` or `spr`, so the HUD buttons' fills drop out; not in
the PRD's list), `spr` (f236), `rect` (f302) on `pad_slipway`, and `all`
(f143, f155, f164) on `pad_message_box`. Accepting needs a green run, so
the stale reference lines of those labels were removed first (ceilings 0
then apply), `psx-check` run green, the labels accepted, and `psx-check`
run green again. `cls`, `display`, `pset`, `circ`, `circfill`, `print`,
`line` and `print 142` kept their references.
`psx-check` green, all 39 conditions, both lists matching; GPU replay 249
vsyncs (was 255). `game-gpu-cost` bytes 7,512 of 7,552, build 11,172 of
11,700. Memory: the software store grows by 13,728 bytes (18 slots); the
build buffer peak (`make state`, `zone gpu_build`, `attract`) rose from
11,256 to 12,040, over its 1/8 margin, so `GPU_BUILD_BUFFER_BYTES` went
13,312 → 14,336 by its rule (peak + 1/8, rounded up to 1 KiB; two
buffers). `largest-slots` failed on MIPS (`zone slot button full`, then
`slot row full`): every row is 4 bytes larger on 32 bits, and both slots
were sized to their peak, so the 32-bit sizes follow the new peaks,
`ui button` 3,784 → 3,944 and `ui row` 616 → 624 (64-bit sizes
unchanged; PC peaks 4,632 of 6,056 and 880 of 1,184). `ram-room` 145,868
→ 123,280 bytes left, `game-ram-room` 302,036 → 279,832. `make state`
green, every zone within its margin. Text image slots: a cached button
builds its labels with direct prints, so it takes no text slot and its
labels are no longer copied from the text cache; only a button that falls
back to the direct path still uses it, so N7 can only lower D3's text
slot pressure (not counted per frame). VRAM: icon slots from (784, 440),
wide slots two a band from (704, 456) to row 510, each inside its 15-bit
page (`test_psx_vram_layout`).
Every e2e `.sha256` and `.end-state` unchanged; `make raylib` links.
Tests: `test_button_image` (built once then copied, every edge and
three cameras, equal keys share a slot and each word rebuilds, the pools
never evict each other, a full pool and an oversized image draw nothing
and no slot copied this frame is rebuilt, random streams untouched),
`test_button` (eight kinds of button, the HUD icons lit and unlit, text
and sprite, no on-click, side tooltips hovered and not, a sprite hanging
past the border, at whole and fractional positions, every edge and three
cameras, compared pixel for pixel, palette and pen with today's direct
draw, red before the change; a row refilled in place and a hover change
rebuild; a fill pattern, block 8, a fractional width, a box over 112 and
an eleventh wide button draw directly and the same), a row-serial case in
`test_layout_row`, the block-0 check in `test_hud`, the slot list in
`test_image` and `test_psx_vram_layout`. `test_layout_row`'s copy count
froze the old behaviour (two label copies a button) and now counts one
image copy a button. Mutations that shorten the box by its last sprite
column, its side fill or the border, or drop the body colour from the
key, fail `test_button`. The pad-tab body is not tested apart from the
hovered one (same colour, same key). The pointer guard forbids new data
pointers in `src/game/`, so the pools are index ranges and the build
receives the button as its context.
Fast loop, warm, clean copies of `fa9f0f6` and N7 interleaved, mean of
four: `make test` 6.98 → 6.95 s, `make e2e` 0.430 → 0.422 s (in the main
tree `make e2e` reads 0.08 s slower on both because `make` parses its
29,000 `.d` files; not the code).
`docs/psx-game.md`: the image-cache paragraph (slots 70–87), the VRAM
table's images row (new places, free room left), the two capture
sentences (chunk 1) and the "Still to do" figure (347 / 506).
Warn-review warnings in N7's files, all left open as the PRD decided:
`cart.c`'s difficulty carrier and duplicated prologue (N7 adds one call
in `cart_draw`); `loading_screen.c` in `ENGINE_SRCS` (N7 adds
`button_image.c` there next to `text_image.c`, extending the pattern the
warning names, for step 9's list); the other Makefile warnings (N7 adds
a test target, a source entry and nothing in those recipes); e2e frame
numbers inline (the two capture lines use the list's own format);
hand-copied condition counts (still 39); "one frame late" vs F−2 in
`docs/psx-game.md` (not in the edited paragraphs). `ui_zone.c` and
`gpu_sizes.h`, changed by their sizing rules, carry no open warning.
Every other warning left open, outside N7's files. No guideline
decision.
