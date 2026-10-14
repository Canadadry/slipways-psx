# Plan 20 — Find every way to make the PSX build do less per frame

## 1. Goal

The PSX build has to hold 60 fps. Today it is far from that: on
`make psx-full-game-check` (pcsx-redux, GPU path), 6,047 of 6,059 frames of
`worst-case/full_game.json` are over the 263-hblank budget (one NTSC vsync).
The mean frame is 891 hblanks (about 3.4 vsyncs) and draw holds 87% of the
over-budget time. Frame 12 takes 9,558 hblanks, 9,074 of them update: the
pad press that starts a game, almost certainly galaxy generation. Update
spikes also appear at frames 4737, 5500 and 5719. On draw-5564, `print`
takes 42% of draw and `spr` 22%, and 70–79% of draw is inside backend calls.
Even with free backend calls, every draw-5564 window frame stays over 263
(worst 358).

The gap is about 3.4× on average, so tuning will not close it. This plan
**finds and measures the levers**. It implements none of them. Its
deliverable is `docs/psx-perf-levers.md`, a ranked inventory of the ways to
make each frame do less work. Each lever has its measured or estimated gain,
its cost and its risk, and the inventory ends with a cumulative estimate
against 263. With that report in hand, the human chooses which levers to
pull, and a later plan implements them.

Why the target matters: the build is meant to be played on a **Miyoo Mini
Flip** under **PCSX-ReARMed** (RetroArch), at the core's default settings.
The emulator route is deliberate: the Miyoo's emulation front end gives save
states, launcher integration and box art for free. A native ARM build was
considered and rejected by the human: it is a pain to install, it has no
save states, and it does not fit the device's emulation-first workflow.

## 2. Rules

### What "60 fps" means (locked)

- **Strict.** Every frame (update, draw and submit) fits in 263 hblanks. No
  percentile and no tolerance for rare spikes. The loading frames are no
  exception: a frame that cannot fit has to be split across several frames.
- **Judge: pcsx-redux at 263 hblanks, no margin.** `frame-budget` green in
  `make psx-full-game-check` is the contract.
- **The Miyoo is never measured.** The human will not run anything on the
  device, and PCSX-ReARMed is not added to the harness (both were rejected).
  The two emulators count cycles differently, so "60 fps on the Miyoo" stays
  unverified. The implementation plan must say so plainly in the README and
  in `docs/psx-game.md`.
- **PCSX-ReARMed at default settings.** Raising the "PSX CPU clock" core
  option (`pcsx_rearmed_psxclock`) or shipping a `.opt` file is rejected: our
  pcsx-redux budget cannot model it, and every gain has to come from the
  code.

### Decisions carried to the implementation plan

These are locked now. The report repeats them in a section of its own,
because this plan file is deleted once it is finished.

- **Spreading work across frames is allowed on PC and PSX alike**, for
  galaxy generation and the update spikes. The `tests/e2e/*.sha256` files
  are then re-recorded once, explicitly. The game stays identical on both
  platforms: a PSX-only divergence was rejected.
- **The PSX picture stays pixel-identical to the PC's.** Every capture keeps
  its 0-pixel tolerance. Changes that alter pixels are not on the table
  unless the human reopens the question after getting stuck. One exception:
  a lever that only changes how much `visual_random` is consumed (for
  example a removed redundant draw call) may change e2e `*.sha256`. It is
  accepted after checking that each changed screen differs from the old one
  by only a few pixels of visual noise, with the game state unchanged.
- **Asserts go off** in `psx-game` and in the GPU replay that is measured,
  and stay on in the PC build and the tests. The PC with asserts on plays
  the same game, as `psx-check` proves.

### How discovery works

- **The principle is to do less.** Every lever answers "what can we stop
  doing, or do only once?": calls repeated N times, things redrawn
  identically every frame, work recomputed when nothing changed.
  Micro-optimizations (compiler level, asserts) are measured too, but they
  rank as a supplement.
- **The measuring loop is `make psx-short-runs`, judged before vs after on
  itself.** The short runs fail `representative` (about 15% off the full
  run, and the update spikes are lost after a resume). That is accepted,
  because a relative comparison only needs the runs to be deterministic,
  which they are. The runs are used with `PSX_SHORT_RUN_REFERENCE=` empty.
  `psx-full-game-check` (about 20 min) is used only at milestones, never
  per experiment.
- **Hot paths are found by slowing a function down, top-down.** Start at
  `cart_draw` and `cart_update60`, then their direct children, and go down
  only into branches that weigh more than 2% of the run's mean window frame.
  A pre-chosen list of suspects was rejected: it depends on intuition.
  - **Drawing functions are called twice.** Drawing the same thing twice
    gives the same picture, and the frame grows by exactly the function's
    cost. A second call that only advances `visual_random` is fine: the
    picture may differ by a few pixels. The run's game-state lines must
    still match the baseline's. If they do not (the function mutates
    gameplay state), that function falls back to the fixed delay.
  - **Update functions get a fixed delay** (a busy loop of known length at
    entry). The frame grows by delay × calls, so the delay measures the call
    count, not the cost. The cost then comes from PC timing proportions,
    scaled to the PSX update total.
- **Throwaway experiments are allowed**, for example drawing the outlined
  text once instead of nine times to bound a lever's gain. They run in a
  temporary worktree or branch, are measured, and are discarded. **Nothing
  is kept from an experiment except its numbers in the report.** The only
  code this plan commits is the measuring tooling.
- **The update spikes are identified on PC first** (PC timing or call counts
  per `cart_update60` child on frames 12, 4737, 5500 and 5719). The full run
  is the fallback if PC does not settle it.
- **Baseline.** Asserts off is decided, so the tooling builds the measured
  replays without asserts behind an option. All gains are measured against
  that asserts-off baseline, except the assert lever itself.

### Guardrails

- Nothing in the game or in the shipped drawing path changes. `make e2e`,
  every `*.sha256` and `psx-check` stay exactly as they are.
- New tooling is host-tested where the repo already host-tests its PSX
  scripts (`psx-tools-test`). It stays out of `make test` and `make e2e`,
  which must not get slower.
- Never launch the raylib app, and never run anything on the Miyoo.

### Already known (record it, do not rediscover it)

- `hud_print_outlined` (`src/game/hud.c:86`) calls `p8_print` **9 times**
  per text: 8 outline offsets plus the text itself. Each glyph therefore
  costs 9 backend calls, 9 meter cost computations and 9 GPU primitives.
  That is the likely bulk of `print`'s 42%. Candidate lever: an outlined
  font pre-drawn into VRAM, 1 primitive per glyph, pixel-identical.
- `src/cmd/psx_game.c:68` installs the draw meter in the **shipped** game.
  It computes a cost for every draw call, every frame, only to size
  `psx_gpu_frame_end`, and the GPU backend already checks overflow on its
  own. Candidate lever: drop it from `psx-game`.

### Out of scope

- Implementing any lever.
- Measuring on PCSX-ReARMed or on the device.
- Fixing the short runs' `representative` failure.
- Audio.

## 3. Phases and steps

### Phase 1 — Harness

1. Delete `docs/prd/triage/phase1-step4.1-short-runs-draw-breakdown.md`. It
   is plan 11's step 4.1 PRD, already implemented (chunks 1–5, `make
   psx-short-runs`), and prd-autopilot would otherwise pick it up. Verified
   when `docs/prd/triage/` holds no file from an earlier plan.
2. An option of `make psx-short-runs` builds the measured replays without
   asserts. Verified when one asserts-off run of all five short runs is
   `done` and `deterministic`, and its per-run window sums of draw, update
   and submit are recorded as the baseline in `docs/psx-perf-levers.md`.
3. A probe tool takes a throwaway patch (call-twice, fixed delay or
   experiment), applies it in a temporary worktree, runs the chosen short
   runs, and prints each run's window draw, update and submit delta against
   the baseline. It also reports whether the game-state lines match the
   baseline's. Probe builds get room for the extra primitives a probe sends
   (the GPU frame buffer is sized at the metered peak + 1/8, and an overrun is
   `p8_fatal`), and the meter's accounting matches; a probe boot that still
   ends in `p8_fatal` is reported as invalid, never as a delta. Host-tested on
   hand-written boot outputs. Verified by two sanity probes on the real build,
   neither ending in `p8_fatal`:
   - calling `cart_draw` twice roughly doubles draw (within 10%) and leaves
     the game-state lines identical (`visual_random` may drift);
   - a fixed delay at the entry of a function called a known number of times
     per frame gives delta ÷ delay equal to that count.
   - a null patch (changes the code layout, does no work at run time) gives
     the noise floor, recorded per run in the report. It must sit clearly
     below the 2% threshold, or the threshold or the probe method is
     revisited before Phase 2.
4. A PC tool prints per-frame call counts and PC time per `cart_draw` and
   `cart_update60` child, and per draw family, for any recording. It runs in
   seconds and its own test sits outside `make test`. Verified on
   `draw-5564`'s window: its per-family call counts equal the breakdown
   boot's `calls` column. Then a coverage check: over all of
   `full_game.json`, compare each `cart_draw` child's and family's share of
   draw with its share in draw-5564's window. A child or family above 2% of
   draw on a significant share of full-run frames but under 2% at 5564 gets
   an extra draw window in `psx/short_runs.txt`, probed in steps 6–7 like
   draw-5564. Verified when the comparison is in the report and every child
   above 2% somewhere in the full run is covered by a probed window.
5. Milestone: one asserts-off `make psx-full-game-check`, the only full run
   of this plan besides step 9's fallback. Verified when its frame table is
   recorded in the report as the judge's baseline, next to the step 2 short-run
   baseline, so step 12's verdict can be stated against both.

### Phase 2 — Draw

6. Level 1 of draw: probe each direct child of `cart_draw` with call-twice
   on `start` and `draw-5564`. Verified when the report lists each child's
   cost and calls per frame, and the children's costs add up to `cart_draw`'s
   own within 10%. The gap is explained in the report if not.
7. Go down the draw tree. Each branch above 2% of the mean window frame is
   probed one level deeper, until it is a leaf or under 2%. Each function
   explored gets its "do less" candidates in the report: repeated calls,
   identical redraws, recomputation. Verified when every branch above 2% is
   explored or explicitly marked as a leaf.
8. Backend side: for each draw family, per-unit cost (hblanks per call,
   glyph or primitive) from the breakdown boot, and the ways to do less
   inside the backend and the meter. Two experiments run here: the 9× print
   bounded by a one-print throwaway, and the meter removed from the measured
   replay. Verified when each family has a per-unit cost and each
   experiment a measured delta.

### Phase 3 — Update

9. Identify the four spikes on PC (frames 12, 4737, 5500, 5719 of
   `full_game.json`): which `cart_update60` descendant holds the time, and
   why it runs then. If PC cannot settle it, fall back to the full run.
   Verified when the report names, for each spike, the responsible function
   and its share of that frame's update. Also given: a do-less candidate,
   and the number of frames it would take if spread out under 263 after
   draw's estimated levers.
10. Ordinary update. First, a calibration: compare the PC time proportions
    of `cart_draw`'s children with their PSX costs from step 6, and record
    the gap. A large gap marks the PC-derived update costs as low-confidence,
    or switches method, decided in the step note. Then the 171 frames where
    update alone overruns, and update's share in normal frames, through the
    fixed-delay probes and PC proportions. Verified when the calibration gap
    is recorded and update's children above 2% of the mean window frame have
    a cost, a call count and their do-less candidates in the report.

### Phase 4 — Global levers

11. Measure on all five short runs: asserts off vs on, and `-O2` / `-O3` /
    `-Os` (`-Os` can win on the R3000's 4 KB instruction cache). Also
    identify what the submit part spends its time on. Verified when each has
    a measured delta in the report.

### Phase 5 — Report

12. Finish `docs/psx-perf-levers.md`. It holds:
    - the decisions carried to the implementation plan (section 2 above),
      verbatim in substance;
    - one entry per lever: where it is (file:line), the measured or
      estimated gain in hblanks per short run, the cost to implement, and
      the risk (e2e hashes, PC/PSX identity, RAM);
    - the levers ranked by gain over cost;
    - per frame of the step 5 full-run table (the judge's frames), the
      cumulative estimate against 263, with the overlaps between levers
      named (for example, removing the 9× print also shrinks the meter's
      share). Each lever's short-run delta is carried over as a relative
      gain on its part (draw, update or submit), since the short runs read
      low in absolute terms; frames 12, 4737, 5500 and 5719 use step 9's
      spike figures and spreading estimates instead;
    - the spikes and their spreading estimates;
    - which stop condition ended discovery.

    Verified when every lever entry has all four fields, and the stop
    condition holds: either the cumulative estimated gain brings every
    full-run frame to 263 with about 20% extra gain to spare
    (estimates overlap, so their sum is optimistic), or every branch above
    2% is explored, whichever came first.

## 4. Done when

- `docs/psx-perf-levers.md` is committed, complete per step 12, and states
  whether the levers found reach 263 on paper.
- `make test-all` and `psx-check` are green and unchanged. No game or
  shipped-path code changed, and every `*.sha256` is unchanged.
- The only code committed is the harness of phase 1, host-tested where the
  repo host-tests its PSX scripts.
- The plan stops for the human: they choose the levers to pull, and the
  implementation plan is written from the report in a new grilling session.
  The report is deleted once that plan exists.

## 5. Open questions

(none)

## 6. Step notes

### Step 1.1

Deleted `docs/prd/triage/phase1-step4.1-short-runs-draw-breakdown.md` (plan
11's step 4.1 PRD, already implemented). `docs/prd/triage/` is now empty.
Deviation: done directly by plan-pilot, without a PRD or prd-autopilot run,
since the step is one file removal with nothing to test.

### Step 1.2

`make psx-short-runs PSX_SHORT_RUN_ASSERTS=on|off` (default `on`): `off` sets
the CMake option `PSX_MEASURED_REPLAY_ASSERTS=OFF`, which drops `-UNDEBUG`
from `psx-gpu-replay` and `psx-gpu-replay-breakdown` only. `psx_short_run.sh`
gained an `asserts` verdict (read from each boot's TTY) and a
`window plain update U draw D submit S` summary line, printed without a
reference table. Baseline recorded in `docs/psx-perf-levers.md` at
`473bdcc`: window totals start 9451, update-4737 832, update-5500 2130,
draw-5564 20264, update-5719 1132 hblanks; all done, deterministic, asserts
off, game-state lines equal to the PC's.
Deviation: asserts off under `-Werror` raised 35 unused-* warnings from
assert-only code; the four unused-* kinds are demoted from errors in the
asserts-off measured build only, game sources untouched (PRD decision, no
guideline call). As known, the resumed short runs lose the update spikes at
4737 and 5719 (update 118 and 46 hblanks).

### Step 1.3

`make psx-probe-baseline` and `make psx-probe PATCH=<file>
[PSX_SHORT_RUNS=] [PSX_PROBE_LABEL=]`: the GPU replay built in a temporary
worktree of HEAD with `-DPSX_PROBE=ON` (GPU frame buffer doubled, meter and
`gpu_frame` zone on the same constant; `psx_probe_delay()` fine-clock
busy-wait counting its calls) and asserts off, one boot per run, analysed by
`tools/psx_probe.sh` (host-tested in `psx-tools-test`). Fixtures in
`psx/probes/`. Probe baseline equals step 2's within 1 hblank per run.
Sanity probes, all boots valid: call-twice `cart_draw` gives draw +4.3% to
+8.5% over double; delay probe delta/delay within 0.4% of the counted calls;
null patch delta 0 on every run (noise floor 0.00%, but the emulator does
not model code-layout effects).
Guideline decisions: (1) `start`'s game state differs under call-twice
`cart_draw` because `help_screen_draw` (`src/game/help.c:98`) ticks the RNG
per call — accepted as an explained result; Phase 2 probes
`help_screen_draw` and any title-screen child running it with the fixed
delay. (2) The PRD's per-part delay check uses max(2% of 20 × window frames,
2 hblanks), since the frame timer reads whole hblanks.
