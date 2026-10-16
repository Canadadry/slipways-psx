# Plan 22 — Bring the PSX build's draw under budget

## 1. Goal

`frame-budget` green in `make psx-full-game-check`: every frame of
`worst-case/full_game.json` within 263 hblanks (one NTSC vsync) in
pcsx-redux, on the GPU path.

Plan 21 closed with D8 taken afterwards: **1,764 of 6,059 frames over 263**,
mean frame **233.6**, worst frame 12 at **3,992** (galaxy generation). The
frames still over are mostly ordinary frames bound by draw: at plan 21's
close (before D8) 2,818 of 3,078 frames over had a mean draw of 270.6
against an update of 55.5. Plan 20's lever list is used up: every draw
lever worth more than 2% was built, skipped or covered in plan 21 (its
step notes and commit messages are the record; `docs/psx-perf-levers.md`
is deleted).

So this plan first finds where draw time goes now, then builds new levers
in impact order, and stops at green or when its list runs out. The draw
breakdown of a draw-heavy window (before D8) put about half of draw outside
the primitives, in the game's own draw code, and the rest spread over
primitive families (circfill 14%, spr 7%, circ 7%, pset 7%, line 6%,
rectfill 6%).

The target device is a Miyoo Mini Flip running PCSX-ReARMed. It is never
measured. The contract stays pcsx-redux at 263.

## 2. Rules

### Carried over from plan 21 (to confirm in the grilling)

- **Strict 60 fps**: every frame, update + draw + submit, within 263
  hblanks. No percentile, no tolerance.
- **Judge: pcsx-redux at 263, no margin.** The Miyoo is never measured.
- **The PSX picture stays pixel-identical to the PC's**: every capture at
  `differ 0` (or under its existing ceiling). A raw-VRAM reference may
  change only when every capture of its label stays at `differ 0`; an
  agent may then accept it (`docs/psx-game.md`, Acceptance).
- **Every e2e hash stays unchanged**: no lever may change the game.
- **Asserts off, O3** in `psx-game` and every measured build.
- **Measuring**: each lever is measured on the short runs (`make
  psx-probe` against a fresh baseline), with its gain against its
  estimate in the step note; `psx-full-game-check` at the end of each
  phase, its table replacing `psx/perf/full-run-frames.txt`; every other
  condition of that check must hold.
- A lever whose measured delta on its target is not a gain is reverted and
  noted.
- Before a lever, check whether a lever already taken covers it; "covered"
  and "under 2%" are decided on the lever's remaining cost, measured.
- No checkpoint with the human; a lever under its estimate is noted and
  the plan goes on.
- **End of list**: when every step is done and `frame-budget` is still
  red, the plan stops and reports the frames over, the worst and the mean.

### Out of scope

- Spreading spike work over frames (plan 21's skipped steps 19–20). Frame
  12 and the other spike frames stay over unless a draw lever happens to
  bring them under.
- Update levers, unless phase 1 finds update over budget on ordinary
  frames.
- Measuring on the Miyoo or PCSX-ReARMed; audio; changing the 263 budget.

## 3. Phases and steps

### Phase 1 — Find where draw goes

1. Baseline: one `psx-full-game-check` on the plan's starting HEAD, its
   table replacing `psx/perf/full-run-frames.txt`. Verified when the note
   gives frames over, worst and mean against plan 21's 1,764 / 3,992 /
   233.6, and splits the frames over into spike frames and ordinary frames
   with each group's mean update and draw.
2. Draw profile, per game function: which draw functions (planets,
   slipways, particles, HUD, panels, map, overlays) cost what on the judge's
   ordinary frames over budget, split into time inside the primitives and
   time in the game's own draw code. Built from the PSX draw breakdown and
   the PC frame profile with `cart_draw` as root, scaled to the judge.
   Verified when the shares add up to the measured draw within the noise
   floor, on at least the two draw windows.
3. Lever list: for each function above 2% of the ordinary frames' mean
   draw, a lever with its location, estimated gain, cost and risk
   (pixel-identity, game state), ranked by judge-wide gain, written to
   `docs/psx-draw-levers.md`. Verified when the estimated gains, applied
   to step 1's table, give the frames over at the end of the list, with a
   pessimistic variant (every gain cut by a fifth).

### Phase 2 — Draw levers, in impact order

4. Each lever of step 3's list, in its order, one step note each: measured
   gain against its estimate, pixel-identical captures, unchanged e2e
   hashes. (plan-pilot: one PRD per lever, or per group of small levers
   that touch the same code.)
5. End of phase: `psx-full-game-check`, with its summary in the note.

### Phase 3 — Close

6. Close the plan: write the result (`frame-budget` green, or the frames
   still over with their worst and mean, and what bounds them); update
   `docs/psx-game.md`'s "Still to do"; delete `docs/psx-draw-levers.md`.
   Verified when `make test-all` and `psx-check` are green and the docs
   describe only what is true.

## 4. Done when

- `frame-budget` is green in `psx-full-game-check`, or every step is done
  and the note gives the frames still over, the worst and the mean.
- `make test-all` and `psx-check` are green; every capture at `differ 0`
  or under its existing ceiling.
- No e2e hash changed.
- `docs/psx-draw-levers.md` is removed and `docs/psx-game.md` is up to
  date.

## 5. Open questions

To settle in the grilling before plan-converge:

- **Target**: strict 60 fps on every frame, or a share of frames (e.g. 99%)
  with frame 12 and other spikes allowed over? Plan 21 kept strict.
- **Pixel identity**: must the PSX stay pixel-identical to the PC, or may a
  draw lever change the picture slightly (on both platforms, e2e hashes
  re-recorded) if it buys a large gain — e.g. fewer particles, simpler
  slipway lines, no outline on some text?
- **Game changes**: may a lever change what is drawn on PC and PSX alike
  (e.g. draw particles every other frame), or is the game frozen?
- **Spikes**: are frame 12 (galaxy generation, 3,992) and the other spike
  frames in scope, or accepted as over?
- **Lever list shape**: phase 1 writes a report like plan 20's, then phase
  2 takes it in order — or phase 2's steps are fixed now from the known
  suspects (game draw code, particles, slipways)?
- **Checkpoint**: run unattended to the end as plan 21, or stop after
  phase 1 for the human to choose the levers?
- **Leftovers from plan 21**: drop D11 (500 lines for 1.3 hblanks)? Clean
  the `lever-estimate` targets that cite the deleted report? Act on any
  of plan 21's warn-review warnings in the code phase 2 touches?

## 6. Step notes
