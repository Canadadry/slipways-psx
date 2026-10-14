# PSX performance levers

This file will become plan 20's ranked inventory of the ways to make each PSX
frame do less work. For now it holds the baseline every lever is
measured against, the probe tool's sanity checks and the PC frame profile's
draw coverage.

## Short-run baseline (asserts off)

Measured on commit `473bdccfdb35829420f7e12ed6d1c2cc5a678ee3` (pcsx-redux,
GPU path) with:

```
make psx-short-runs PSX_SHORT_RUN_ASSERTS=off PSX_SHORT_RUN_REFERENCE=
```

The measured replays (`psx-gpu-replay`, `psx-gpu-replay-breakdown`) are built
without asserts. The figures come from each run's `analysis/summary.txt`
(`window plain ...` line, plain-1 boot) and are in hblanks, summed over the
window frames.

These figures are **relative-only**: use them to compare before and after on
the same short runs. The short runs read about 15% off the full run and lose
the update spikes that follow a resume, so they are not the judge's numbers
(the full-run baseline is plan 20's step 5).

| Run | Full-game frames | Frames | Update | Draw | Submit | Total |
|---|---|---:|---:|---:|---:|---:|
| `start` | 12..12 | 1 | 8980 | 464 | 7 | 9451 |
| `update-4737` | 4737..4737 | 1 | 118 | 706 | 8 | 832 |
| `update-5500` | 5500..5500 | 1 | 1409 | 714 | 7 | 2130 |
| `draw-5564` | 5564..5578 | 15 | 657 | 19489 | 118 | 20264 |
| `update-5719` | 5719..5719 | 1 | 46 | 1078 | 8 | 1132 |
| `draw-6000` (added by step 4, measured by step 4 on `303da7c`) | 6000..6014 | 15 | 593 | 13987 | 107 | 14687 |

### Verdicts

| Run | done | deterministic | representative | asserts |
|---|---|---|---|---|
| `start` | ok 3 boots, 100 frames each | ok 101 psx-frame lines identical | skipped no reference frame table | ok off in 3 boots |
| `update-4737` | ok 3 boots, 29 frames each | ok 30 psx-frame lines identical | skipped no reference frame table | ok off in 3 boots |
| `update-5500` | ok 3 boots, 25 frames each | ok 26 psx-frame lines identical | skipped no reference frame table | ok off in 3 boots |
| `draw-5564` | ok 3 boots, 54 frames each | ok 55 psx-frame lines identical | skipped no reference frame table | ok off in 3 boots |
| `update-5719` | ok 3 boots, 65 frames each | ok 66 psx-frame lines identical | skipped no reference frame table | ok off in 3 boots |
| `draw-6000` (step 4) | ok 3 boots, 123 frames each | ok 124 psx-frame lines identical | skipped no reference frame table | ok off in 3 boots |

`representative` is skipped on purpose: plan 20 uses the short runs with no
reference table. `breakdown` was `ok` on all five runs, and on `draw-6000`.

### Game state versus the PC

Each run's plain-1 TTY was compared with the PC state the cut wrote for that
run (`run.state.txt`), with `tools/psx_state_compare.sh`. The game lines match
on all five runs (`start` 103 lines, `update-4737` 119, `update-5500` 131,
`draw-5564` 131, `update-5719` 132; `draw-6000` 138, step 4): turning
asserts off changes no game state.

## Probe tool

Measured on commit `cb3cf014089e3157924c4a1ba56f982511267442` with
`make psx-probe-baseline`, then `make psx-probe PATCH=psx/probes/<name>.patch`
for the three sanity patches (all five runs each; tool in
`docs/psx-game.md`, "Probes"). Figures in hblanks, summed over the window
frames; every boot is `valid` (no `p8_fatal`, all frames done, asserts off).

### Probe baseline next to step 2's

| Run | Update | Draw | Submit | Total | Step 2 total | Gap |
|---|---:|---:|---:|---:|---:|---:|
| `start` | 8980 | 465 | 6 | 9451 | 9451 | 0 |
| `update-4737` | 118 | 706 | 8 | 832 | 832 | 0 |
| `update-5500` | 1409 | 714 | 7 | 2130 | 2130 | 0 |
| `draw-5564` | 655 | 19491 | 118 | 20264 | 20264 | 0 |
| `update-5719` | 46 | 1078 | 7 | 1131 | 1132 | −1 |

The probe build (doubled GPU buffer, fine clock started, delay helper
linked) sits within one hblank of the measured replay; parts move by one or
two hblanks between draw, update and submit.

### Call-twice `cart_draw`

| Run | Draw | Probe draw | Delta vs baseline draw | Game state |
|---|---:|---:|---:|---|
| `start` | 465 | 956 | +5.6% | **differs** |
| `update-4737` | 706 | 1460 | +6.8% | same |
| `update-5500` | 714 | 1489 | +8.5% | same |
| `draw-5564` | 19491 | 40410 | +7.3% | same |
| `update-5719` | 1078 | 2202 | +4.3% | same |

Draw doubles within 10% on every run, and the sanity probe passes. The
`start` mismatch is an explained result, not a tool failure:
`help_screen_draw` (`src/game/help.c:98`, under `cart_draw`) ticks the game's
random state once a call, the cart's own seed tick, so a second call on the
title screen generates another galaxy (the probe's p1 sits where the
baseline's p7 does, and frame 12's update moves by −409). It shows the
game-state check works. Per the plan's rule, Phase 2 probes
`help_screen_draw`, and any `cart_draw` child that runs it on the title
screen, with the fixed delay, not with call-twice.

### Fixed delay at `cart_update60` and `cart_draw` (20 hblanks)

| Run | Frames | Calls | Delta ÷ delay, run | vs calls | Window frames | Update delta | Draw delta |
|---|---:|---:|---:|---:|---:|---:|---:|
| `start` | 100 | 200 | 200.85 | +0.4% | 1 | +21 | +19 |
| `update-4737` | 29 | 58 | 58.05 | +0.1% | 1 | +20 | +20 |
| `update-5500` | 25 | 50 | 50.00 | 0.0% | 1 | +19 | +21 |
| `draw-5564` | 54 | 108 | 108.20 | +0.2% | 15 | +304 | +298 |
| `update-5719` | 65 | 130 | 130.45 | +0.3% | 1 | +19 | +22 |

The counted calls are twice the frame count on every run, and delta ÷ delay
over the run equals them within 0.4%. Each window part's delta must sit
within max(2% of 20 × window frames, 2 hblanks) of 20 × window frames: the
2-hblank floor is the frame timer's one-hblank resolution at each of a
part's two marks. Every run passes: 19 to 22 for 20 on the one-frame
windows, 304 and 298 for 300 (±6) on `draw-5564`; each delay lands in its
own part.

### Null patch: noise floor

| Run | Text | Total delta | % of window total |
|---|---:|---:|---:|
| `start` | +80 bytes | 0 | 0.00% |
| `update-4737` | +80 bytes | 0 | 0.00% |
| `update-5500` | +80 bytes | 0 | 0.00% |
| `draw-5564` | +80 bytes | 0 | 0.00% |
| `update-5719` | +80 bytes | 0 | 0.00% |

Every part's delta is 0 on every run: the noise floor is 0%, under the
0.5% rule. The emulator's timing does not move with an 80-byte shift of the
code after `cart_draw`; a code-layout effect on real hardware is not
measured by this.

## PC frame profile and draw coverage

Plan 20 step 4. The tool is `tmp/frame_profile`, built at `303da7c`. This step
adds the K-profile coverage rule. Its commands:

```
make psx-short-runs PSX_SHORT_RUNS=draw-5564 PSX_SHORT_RUN_ASSERTS=off PSX_SHORT_RUN_REFERENCE=
make frame-profile-breakdown-check
make frame-profile-coverage
```

All shares below are **PC time** shares (PC software raster at `-O2` with
asserts off), not PSX costs. They tell which children and families matter
where, not what they cost on the PSX.

**Runtime.** `tmp/frame_profile` profiles all of `full_game.json` (6,059
frames) in 7.5 s wall, `same-game yes`, with a hook pair costing 4.6–5.0 ns.
`make frame-profile-coverage` (4 profiles of the full game and of each draw
window) takes about 1 min.

### Breakdown check

The PC per-family calls are equal to the breakdown boot's `calls` column on
every window frame (`calls-match ok 15 frames`, `same-game yes`). The
`draw-5564` boot is fresh from `303da7c`, asserts off, with every verdict `ok`.
Window totals:

| Family | `draw-5564` PC | `draw-5564` PSX | `draw-6000` PC | `draw-6000` PSX |
|---|---:|---:|---:|---:|
| cls | 15 | 15 | 15 | 15 |
| pset | 2083 | 2083 | 4327 | 4327 |
| line | 1800 | 1800 | 2640 | 2640 |
| rect | 225 | 225 | 144 | 144 |
| rectfill | 1383 | 1383 | 1959 | 1959 |
| circ | 90 | 90 | 123 | 123 |
| circfill | 195 | 195 | 285 | 285 |
| spr | 7541 | 7541 | 7707 | 7707 |
| print | 4437 | 4437 | 1269 | 1269 |
| pattern | 275 | 275 | 270 | 270 |

### Coverage rule: K profiles

A single profile does not give a stable verdict. PC timer noise (preemption
inside one call) puts isolated frames above 2%. Over four single profiles of
`full_game.json`:

| Item | Frames above 2% | Best 15-frame span (mean) | Spans with a mean above 2% |
|---|---|---|---|
| pset | 433 / 395 / 440 / 400 | 5529 / 5533 / 5524 / 5527 (3.24 / 3.49 / 3.27 / 3.27%) | 383 / 372 / 384 / 347 |
| rect | 146 / 162 / 163 / 154 | 1095 / 0 / 1402 / 291 (3.30 / 2.43 / 5.48 / 6.23%) | 91 / 30 / 78 / 48 |
| circ | 22 / 11 / 17 / 15 | 3035 / 4013 / 3383 / 1632 (1.36 / 1.44 / 2.64 / 1.22%) | 0 / 0 / 15 / 0 |

`circ` alternated between `uncovered` and `sporadic`. `rect`'s proposed
window moved with whichever single-frame spike was largest. So
`make frame-profile-coverage` now profiles every recording
`FRAME_PROFILE_REPEATS` times (default 4):

- **Full run:** a frame counts as above the threshold only when it is above
  in all K profiles. The share is the per-frame minimum, because timer noise
  only adds time.
- **Draw window:** the share is the median of its K window shares.

The threshold (2%), "significant" (at least 15 frames), `sporadic` (1–14) and
`below` (0) are unchanged. The script prints the rule.

### Coverage table (K = 4, threshold 2%)

| Item | Frames above | Mean | Max | `start` | `draw-5564` | `draw-6000` | Verdict |
|---|---:|---:|---:|---:|---:|---:|---|
| child `dispatch_context_mem` | 0 | 0.0% | 0.0% | 0.0% | 0.0% | 0.0% | below |
| child `p8_cls` | 0 | 0.0% | 0.4% | 0.2% | 0.0% | 0.0% | below |
| child `hud_select_palette_block_default` | 0 | 0.0% | 0.1% | 0.0% | 0.0% | 0.0% | below |
| child `draw_layers#1` | 6059 | 91.7% | 99.7% | 99.5% | 75.4% | 99.1% | covered |
| child `draw_layers#2` | 1959 | 6.8% | 33.5% | 0.2% | 24.6% | 0.8% | covered |
| child `dispatch_context_gfx` | 0 | 0.0% | 0.0% | 0.0% | 0.0% | 0.0% | below |
| child `demo_overlay_draw` | 0 | 0.0% | 0.0% | 0.0% | 0.0% | 0.0% | below |
| family cls | 0 | 0.0% | 0.3% | 0.2% | 0.0% | 0.0% | below |
| family pset | 254 | 0.7% | 3.2% | 0.0% | 0.8% | 2.3% | covered |
| family line | 4307 | 3.6% | 11.3% | 0.0% | 4.3% | 8.1% | covered |
| family rect | 12 | 1.2% | 2.8% | 1.6% | 1.3% | 1.2% | sporadic |
| family rectfill | 6059 | 19.4% | 37.5% | 17.4% | 33.0% | 15.1% | covered |
| family circ | 0 | 0.3% | 1.3% | 0.0% | 0.2% | 0.3% | below |
| family circfill | 6015 | 5.7% | 11.5% | 9.9% | 3.6% | 7.0% | covered |
| family spr | 6059 | 44.4% | 57.6% | 55.9% | 30.8% | 42.3% | covered |
| family print | 5935 | 4.8% | 28.8% | 3.2% | 12.3% | 3.2% | covered |

`coverage ok`: every child and family above 2% on at least 15 full-run frames
is above 2% in a probed draw window.

- **Sporadic:** `rect` on frames 0–11. Its only real signal is the title
  screen at about 2.7% (4 calls a frame). That is 12 contiguous frames, under
  the 15 that make it significant, so it gets no window. Every other frame
  where a single profile had `rect` above 2% was a timer spike.
- **Below:** `circ`. Its K-profile maximum is 1.3%.

### Window added: `draw-6000`

`draw-6000 resume 6000 6014`, cut at 5904 (96 frames of lead), covers `pset`.

The coverage check first proposed the full-run `pset` peak, 5529–5543 (about
3%, frames 5504–5552). The cut accepts that span, but a resume there covers
nothing:

- The save does not hold the slipway transit dots
  (`src/game/slipway_display.c`, most of the `pset` calls). After a resume
  they build up from zero by about 4 calls a frame.
- With the cut at 5500, the window shows 118–175 `pset` calls a frame against
  about 350 in the full game. Its window share is 1.2–1.3%.
- `draw-5564` loses dots the same way: 2083 window calls against 4597 in the
  full game.

Following the PRD's shift clause, the span moved to the accepted resume with
the longest lead in a `pset`-heavy stretch of the game (about 2.1% over
5941–6043). There the dots are back:

- 4327 window calls against 3831 for full-game frames 6000–6014.
- Window `pset` share 2.2 / 2.2 / 2.4 / 3.6% over the four coverage profiles
  (median 2.3%). An earlier exploration measured 2.3 / 2.4 / 1.9 / 2.3%.

The resumed game diverges from the full one: at its last frame, planet s10
is colonized as a processor instead of an ascension, and credits are 168
against 123. As with the other resumed runs, this is reported, not gating.
The window stands for the run's own draw. Its asserts-off boot is `done`,
`deterministic`, `asserts` and `breakdown` ok. Its PSX game lines equal the
PC's (138 lines), and its window sums are in the baseline table above.

The PC `cart_update60` child figures at frames 12, 4737, 5500 and 5719 are
step 9's to analyze.
