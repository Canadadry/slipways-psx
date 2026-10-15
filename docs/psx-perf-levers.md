# PSX performance levers

This file will become plan 20's ranked inventory of the ways to make each PSX
frame do less work. For now it holds the baseline every lever is
measured against, the judge's asserts-off full-run baseline, the probe
tool's sanity checks, the PC frame profile's draw coverage, the split of
`cart_draw` among its direct children ("Draw level 1") and the draw tree
below them ("Draw tree"). The full-run
frame table is the companion file `docs/psx-perf-levers-full-run-frames.txt`,
part of this report and deleted with it.

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
(see "Full-run baseline (asserts off)" below).

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

## Full-run baseline (asserts off)

The judge's own frames, asserts off: plan 20's step 5, the baseline step 12's
verdict is stated against, next to the short-run baseline above. Measured on
commit `7eb512f22aaab7a459d6b70abacd8733fc6cb40d` (pcsx-redux at
`third_party/pcsx-redux`, GPU path, 263 hblanks a frame) with:

```
make psx-full-game-check PSX_FULL_GAME_ASSERTS=off
```

Wall time 1,166 s (19 min 26 s), build included. The GPU boot
(`psx-gpu-replay`, the one measured) ran without asserts; the CPU-path boot
(`psx-replay`) kept them:

```
gpu.tty.txt:    psx-replay: asserts off
replay.tty.txt: psx-replay: asserts on
```

Same code as step 2: between `473bdcc` and `7eb512f`, no file compiled into
`psx-gpu-replay` nor `worst-case/full_game.json` changed, except step 3's
`PSX_PROBE` additions (`psx/CMakeLists.txt`, `src/cmd/psx_gpu_replay.c`,
`src/p8/gpu_sizes.h`), which the non-probe build does not compile or
compiles to empty functions.

### Conditions

As expected: every condition `ok` but `frame-budget`, so the make exits
non-zero.

| Condition | Verdict | Detail |
|---|---|---|
| `replay-done` | ok | |
| `state-game-lines` | ok | |
| `sprite-sheet` | ok | |
| `screen` | ok | |
| `map` | ok | |
| `gpu-replay-done` | ok | |
| `gpu-state-game-lines` | ok | |
| `timer` | ok | hblanks-per-vsync 263, 6059 frames |
| `frame-budget` | failed | 6047 frames over 263 hblanks, worst frame 12 total 9451 |
| `zone-sizes` | ok | 6 zones and the budget fit |
| `cap-sizes` | ok | 28 caps fit |
| `gpu-ram-room` | ok | 349676 bytes left in 2097152 (kernel 65536 + ELF + stack 65536 = 1747476) |

### Frame table

The whole per-frame table is the companion file
`docs/psx-perf-levers-full-run-frames.txt`: the run's
`run/frame-budget/frames.txt`, byte for byte. Header `frame update draw
submit total vsyncs`, then one line a frame, in hblanks. Frames are 0-based as
the judge writes them, so frame 12 is the 13th line after the header.

### Frame summary

`run/frame-budget/summary.txt`, verbatim:

```
frames 6059 budget 263 hblanks
over 6047 frames (998‰), worst frame 12 total 9451 (+9188) update 8980 draw 464 submit 7
overrun mean 595 hblanks over the over-budget frames
overrun distribution <=1/4 vsync 0 <=1/2 0 <=1 3 <=2 2620 >2 3424
parts over-budget frames update 597047 draw 4548306 submit 44992 dominant draw
parts all frames update 597148 draw 4550826 submit 45018 mean total 857
```

| Figure | Value |
|---|---:|
| Frames | 6059 |
| Over 263 hblanks | 6047 |
| Worst frame | 12: total 9451, update 8980, draw 464, submit 7 |
| Mean total | 857 |
| Overrun mean (over-budget frames) | 595 |
| Overrun ≤ 1 vsync / ≤ 2 / > 2 | 3 / 2620 / 3424 |
| Update / draw / submit, all frames | 597148 / 4550826 / 45018 |
| Update / draw / submit, over-budget frames | 597047 / 4548306 / 44992 |
| Dominant part | draw, 87.6% of over-budget time |

### Zones, caps and RAM

From `measurements.txt`: zone peaks (limit) game 113752 (466616), load 1904
(24984), frame 11944 (20256), ui 66286 (120248), particles 19803 (57123),
gpu_frame 65028 (73728); budget zones used 793195 of 794584; every one of the
28 caps fits (tightest at margin 0/8: `particles_help_screen` 1/1,
`elements_particles` 140/158, `bytes_particles` 680/696,
`bytes_particles_help_screen` 2664/2680, `bytes_burst` 376/424,
`bytes_message_box` 2728/3072). GPU ELF text 414208, data 2904, bss 1199292;
`gpu ram-room` 1747476 used of 2097152, 349676 left.

Against asserts on: no asserts-on zone, cap or ELF figure is recorded at a
nearby commit. The only asserts-on `gpu-ram-room` in the repo (175,928 bytes
left, plan 11 step 2) predates the 88,224 bytes plan 11 freed afterwards, so
it cannot isolate the asserts.

### Update-spike frames

| Frame | Update | Draw | Submit | Total |
|---:|---:|---:|---:|---:|
| 12 | 8980 | 464 | 7 | 9451 |
| 4737 | 1783 | 834 | 7 | 2624 |
| 5500 | 1416 | 1080 | 7 | 2503 |
| 5719 | 1430 | 1128 | 8 | 2566 |

### Ten worst frames

By total, ties by frame number:

| Frame | Update | Draw | Submit | Total |
|---:|---:|---:|---:|---:|
| 12 | 8980 | 464 | 7 | 9451 |
| 4737 | 1783 | 834 | 7 | 2624 |
| 5719 | 1430 | 1128 | 8 | 2566 |
| 5500 | 1416 | 1080 | 7 | 2503 |
| 5727 | 1428 | 1040 | 7 | 2475 |
| 3634 | 1601 | 825 | 7 | 2433 |
| 5133 | 1480 | 937 | 7 | 2424 |
| 3313 | 1589 | 807 | 7 | 2403 |
| 5387 | 1514 | 861 | 8 | 2383 |
| 5098 | 1463 | 824 | 8 | 2295 |

Frame 12 stands alone. The next nine, the three known spikes included, sit
within 330 hblanks of each other, all update-heavy (1416–1783 update): the
known spikes are not the only ones of their size.

### Against the plan's Goal figures

Cross-commit, not step 11's assert measurement: the Goal figures are asserts
on, from plan 11's last run.

| Figure | Goal (asserts on) | This run (asserts off) |
|---|---:|---:|
| Frames over 263 | 6047 of 6059 | 6047 of 6059 |
| Mean frame | 891 | 857 |
| Frame 12 total | 9558 | 9451 |
| Frame 12 update | 9074 | 8980 |
| Draw share of over-budget time | 87% | 87.6% |

### Against the step 2 short runs

For each run of `psx/short_runs.txt` at `7eb512f`, the full run's window
(the run's window frames from the manifest) summed over the companion file,
next to the short-run sums of the baseline table above (step 2's runs on
`473bdcc`, `draw-6000` from step 4 on `303da7c`). Ratio is short ÷ full.
The sums come from:

```
awk -v f=<first> -v l=<last> 'NR > 1 && $1 >= f && $1 <= l { u += $2; d += $3; s += $4; t += $5 } END { print u, d, s, t }' docs/psx-perf-levers-full-run-frames.txt
```

| Run | Frames | Full update | Full draw | Full submit | Full total | Short update | Short draw | Short submit | Short total |
|---|---|---:|---:|---:|---:|---:|---:|---:|---:|
| `start` | 12..12 | 8980 | 464 | 7 | 9451 | 8980 | 464 | 7 | 9451 |
| `update-4737` | 4737..4737 | 1783 | 834 | 7 | 2624 | 118 | 706 | 8 | 832 |
| `update-5500` | 5500..5500 | 1416 | 1080 | 7 | 2503 | 1409 | 714 | 7 | 2130 |
| `draw-5564` | 5564..5578 | 679 | 22567 | 118 | 23364 | 657 | 19489 | 118 | 20264 |
| `update-5719` | 5719..5719 | 1430 | 1128 | 8 | 2566 | 46 | 1078 | 8 | 1132 |
| `draw-6000` | 6000..6014 | 597 | 13252 | 108 | 13957 | 593 | 13987 | 107 | 14687 |

| Run | Update | Draw | Submit | Total |
|---|---:|---:|---:|---:|
| `start` | 100.0% | 100.0% | 100.0% | 100.0% |
| `update-4737` | 6.6% | 84.7% | 114.3% | 31.7% |
| `update-5500` | 99.5% | 66.1% | 100.0% | 85.1% |
| `draw-5564` | 96.8% | 86.4% | 100.0% | 86.7% |
| `update-5719` | 3.2% | 95.6% | 100.0% | 44.1% |
| `draw-6000` | 99.3% | 105.5% | 99.1% | 105.2% |

- `start` matches the judge exactly: a prefix run plays the same frames.
- The resumed runs lose the update spikes at 4737 and 5719 (6.6% and 3.2%
  of the judge's update) but keep 5500's (99.5%).
- Draw reads 66–96% of the judge's on the resumed runs, except `draw-6000`
  at 105.5%, whose resumed game diverges from the full one (see "Window
  added: `draw-6000`").

`psx-short-runs`' default reference
(`tmp/psx/full-game/run/frame-budget/frames.txt`) is now this asserts-off
table in this working copy: a later `representative` verdict compares
asserts-off runs with an asserts-off judge.

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

## Draw level 1: `cart_draw`'s children

Plan 20 step 6. Measured on commit
`e5d54ab18e1be917c682c3c59eda29e394b63afd` (pcsx-redux, probe build,
asserts off) with:

```
make psx-probe-baseline
make frame-profile-coverage
make psx-probe PATCH=psx/probes/call-twice-cart-draw.patch PSX_SHORT_RUNS="start draw-5564 draw-6000" PSX_PROBE_LABEL=l1-cart-draw-twice
```

Each direct child of `cart_draw` is probed with call-twice by default, its
cost being the window draw-part delta. Where a second call would change the
game state (`draw_layers#1` on `start`, by step 1.3's decision, and any
child whose call-twice gives `game-state differs`), the child gets the
20-hblank fixed delay for its call count, and its cost is its PC share of
`cart_draw` times the run's PSX `cart_draw` cost, marked PC-derived.

Pre-checks, all held: `git log b7d8226..HEAD -- src psx
worst-case/full_game.json` is empty, so the probe fingerprint is the code
steps 2–4 measured; the only `Makefile` change since `b7d8226` (`7eb512f`)
touches `psx-build` and `psx-full-game-check`, not the probe recipe, which
still configures with `-DPSX_MEASURED_REPLAY_ASSERTS=OFF`; `tools/` is
unchanged; step 5's full run is committed and no emulator ran alongside;
`psx/short_runs.txt` still has the six runs. Probed runs: `start`,
`draw-5564`, `draw-6000`.

### Rebuilt probe baseline

Window sums in hblanks; every boot `valid`, `game-state same`. Mean window
frame is the window total ÷ window frames, rounded; 2% of it is step 7's
threshold. Step 3's probe baseline is the "Probe tool" table above
(`cb3cf01`); the measured replay is the "Short-run baseline" table (step 2,
`draw-6000` from step 4).

| Run | Update | Draw | Submit | Total | Window frames | Mean window frame | 2% of it | Step 3 probe total | Gap | Measured replay total | Gap |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| `start` | 8980 | 465 | 6 | 9451 | 1 | 9451 | 189.0 | 9451 | 0 | 9451 | 0 |
| `update-4737` | 118 | 706 | 8 | 832 | 1 | 832 | 16.6 | 832 | 0 | 832 | 0 |
| `update-5500` | 1409 | 714 | 7 | 2130 | 1 | 2130 | 42.6 | 2130 | 0 | 2130 | 0 |
| `draw-5564` | 655 | 19491 | 118 | 20264 | 15 | 1351 | 27.0 | 20264 | 0 | 20264 | 0 |
| `update-5719` | 46 | 1078 | 7 | 1131 | 1 | 1131 | 22.6 | 1131 | 0 | 1132 | −1 |
| `draw-6000` | 593 | 13987 | 107 | 14687 | 15 | 979 | 19.6 | — | — | 14687 | 0 |

Every part equals step 3's probe baseline exactly on the five runs it had.
`draw-6000`'s first probe baseline equals step 4's measured replay part for
part. Nothing moved since step 3, so the deltas below are read against this
table with no shift to account for.

### `cart_draw`'s own cost

The baseline draw part brackets the one `cart_draw` call of each frame, so it
is `cart_draw`'s cost. The `call-twice-cart-draw` fixture measures the
method's bias: bias = draw delta ÷ baseline draw − 1.

| Run | Baseline draw (window) | Per frame | % of mean window frame | Call-twice draw | Draw delta | Bias | Update delta | Submit delta | Text | Game state |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| `start` | 465 | 465 | 4.9% | 956 | +491 | +5.6% | −409 | 0 | +16 bytes | **differs** (known, not used) |
| `draw-5564` | 19491 | 1299 | 96.2% | 40410 | +20919 | +7.3% | +1 | +1 | +16 bytes | same |
| `draw-6000` | 13987 | 932 | 95.2% | 27872 | +13885 | −0.7% | +1 | +1 | +16 bytes | same |

- `start` and `draw-5564` reproduce step 3's figures to the hblank (956 and
  40410 probe draw), with the same `start` mismatch: the second
  `help_screen_draw` ticks the RNG, p1 lands elsewhere, frame 12's update
  moves by −409.
- `draw-6000`, probed for the first time, is the first run where the second
  `cart_draw` costs less than the first (−102 hblanks over 15 frames). Its
  game state is the same, and its update and submit leaks are within the
  2-hblank floor. The children's call-twice deltas on this run are read
  against a bias near zero, not the +4–9% of the other runs.

### PC shares and calls per child

From `make frame-profile-coverage` on the same commit (`coverage ok`, every
profile `same-game yes`). Share is **PC time**: the child's inclusive time
over the window ÷ `cart_draw`'s, median of the K = 4 profiles of the run.
Calls per frame is the child's calls column on each window frame of
`tmp/frame-profile/coverage/<run>/<k>/frames.txt`, min–max over all window
frames of the four profiles.

| Child | `start` share | `start` calls / frame | `draw-5564` share | `draw-5564` calls / frame | `draw-6000` share | `draw-6000` calls / frame |
|---|---:|---:|---:|---:|---:|---:|
| `dispatch_context_mem` | 0.0% | 1–1 | 0.0% | 1–1 | 0.0% | 1–1 |
| `p8_cls` | 0.1% | 1–1 | 0.0% | 1–1 | 0.0% | 1–1 |
| `hud_select_palette_block_default` | 0.0% | 1–1 | 0.0% | 1–1 | 0.0% | 1–1 |
| `draw_layers#1` | 99.6% | 1–1 | 75.4% | 1–1 | 99.2% | 1–1 |
| `draw_layers#2` | 0.2% | 1–1 | 24.6% | 1–1 | 0.8% | 1–1 |
| `dispatch_context_gfx` | 0.0% | 1–1 | 0.0% | 1–1 | 0.0% | 1–1 |
| `demo_overlay_draw` | 0.0% | 1–1 | 0.0% | 1–1 | 0.0% | 1–1 |

The four profiles' `draw_layers#1` shares are 99.52 / 99.58 / 99.62 / 99.75%
on `start`, 69.53 / 75.25 / 75.48 / 75.87% on `draw-5564` and 98.99 / 99.11 /
99.20 / 99.22% on `draw-6000`. The medians move by at most 0.1 point from
step 4's coverage table (PC timer noise); this step uses its own run's.

### Probes run

Measured on commit `2458bad5a7372d2e88bb7fbeb25a01a2ebb54a1f` (its `src/`
and `psx/` are those of `e5d54ab`, so the baseline above is not stale), each
patch from `tmp/plan20-draw-l1/` with its own label:

```
make psx-probe PATCH=tmp/plan20-draw-l1/<child>-twice.patch PSX_SHORT_RUNS="start draw-5564 draw-6000" PSX_PROBE_LABEL=l1-<child>-twice
make psx-probe PATCH=tmp/plan20-draw-l1/draw-layers-1-twice.patch PSX_SHORT_RUNS="draw-5564 draw-6000" PSX_PROBE_LABEL=l1-draw-layers-1-twice
make psx-probe PATCH=tmp/plan20-draw-l1/draw-layers-1-delay.patch PSX_SHORT_RUNS="start draw-5564 draw-6000" PSX_PROBE_LABEL=l1-draw-layers-1-delay
```

Every probe is `valid` on every run, and every call-twice gives
`game-state same`: no child needed the fallback delay beyond `start`'s
`draw_layers#1`, which was never call-twice-probed there. Deltas are window
sums in hblanks over the probe baseline. A draw delta at or under 2 is
written "≤ 2" (per frame "≤ 2 / frames"); so is a negative one, which only
says the cost is under what the timer resolves. "Calls / frame" gives the PC
profile's min–max over the window frames, then the delay probe's PSX figure
where it ran. An update delta beyond max(2% of the draw delta, 2 hblanks) is
marked **leak**.

### `start` (1 window frame, `cart_draw` 465, mean window frame 9451, 2% = 189.0)

| Child | Method | Draw delta | Cost / frame | % of `cart_draw` | % of mean window frame | Calls / frame (PC; PSX) | Update delta | Submit delta | Game state | Text |
|---|---|---:|---:|---:|---:|---|---:|---:|---|---:|
| `dispatch_context_mem` | call-twice | 0 | ≤ 2 | ≤ 0.4% | ≤ 0.1% | 1–1 | 0 | 0 | same | +8 |
| `p8_cls` | call-twice | 0 | ≤ 2 | ≤ 0.4% | ≤ 0.1% | 1–1 | +1 | 0 | same | +8 |
| `hud_select_palette_block_default` | call-twice | −1 | ≤ 2 | ≤ 0.4% | ≤ 0.1% | 1–1 | +1 | 0 | same | +8 |
| `draw_layers#1` | fixed delay, **PC-derived** cost | +18 (delay) | **463** (PC: 99.6% × 465; residual 465) | 99.6% | 4.9% | 1–1; 1.00 | +1 | +1 | same | +456 |
| `draw_layers#2` | call-twice | +1 | ≤ 2 | ≤ 0.4% | ≤ 0.1% | 1–1 | 0 | 0 | same | +8 |
| `dispatch_context_gfx` | call-twice | 0 | ≤ 2 | ≤ 0.4% | ≤ 0.1% | 1–1 | 0 | 0 | same | +8 |
| `demo_overlay_draw` | call-twice | 0 | ≤ 2 | ≤ 0.4% | ≤ 0.1% | 1–1 | 0 | 0 | same | +24 |

- `draw_layers#1`'s delay probe: +18 draw over the window for 20 × 1
  expected (within the 2-hblank floor); over the whole run, delta ÷ delay =
  100.75 for 100 calls (+0.8%), 1.00 a window frame. Its cost is PC-derived:
  the median PC share 99.6% (K = 4) times `cart_draw`'s 465. It must not be
  reused by step 10 as a PSX measurement.
- Residual: 465 − Σ measured children (0) = 465, so the PC-derived 463 is
  −0.4% off the residual. Since each of the six measured children is only
  known to be ≤ 2, the residual is somewhere in 453–465; the derived figure
  sits inside that range.

### `draw-5564` (15 window frames, `cart_draw` 19491, mean window frame 1351, 2% = 27.0)

| Child | Method | Draw delta | Cost / frame | % of `cart_draw` | % of mean window frame | Calls / frame (PC; PSX) | Update delta | Submit delta | Game state | Text |
|---|---|---:|---:|---:|---:|---|---:|---:|---|---:|
| `dispatch_context_mem` | call-twice | −1 | ≤ 2 / 15 | < 0.1% | < 0.1% | 1–1 | +2 | −1 | same | +8 |
| `p8_cls` | call-twice | 0 | ≤ 2 / 15 | < 0.1% | < 0.1% | 1–1 | +2 | −1 | same | +8 |
| `hud_select_palette_block_default` | call-twice | +2 | ≤ 2 / 15 | < 0.1% | < 0.1% | 1–1 | +1 | −2 | same | +8 |
| `draw_layers#1` | call-twice | +16225 | 1082 | 83.2% | 80.1% | 1–1; 1.00 | +1 | 0 | same | +8 |
| `draw_layers#2` | call-twice | +4686 | 312 | 24.0% | 23.1% | 1–1 | +2 | 0 | same | +8 |
| `dispatch_context_gfx` | call-twice | −1 | ≤ 2 / 15 | < 0.1% | < 0.1% | 1–1 | +2 | −1 | same | +8 |
| `demo_overlay_draw` | call-twice | 0 | ≤ 2 / 15 | < 0.1% | < 0.1% | 1–1 | +1 | 0 | same | +24 |

- `draw_layers#1`'s delay probe (PSX call count only; the call-twice delta
  stays the cost): +298 draw over the window for 300 expected (tolerance 6);
  over the run 53.80 for 54 calls (−0.4%), 1.00 a window frame; update +2,
  submit 0, text +456 bytes.
- PSX shares against PC time: `draw_layers#1` 83.2% against 75.4%,
  `draw_layers#2` 24.0% against 24.6%. The excess sits on `draw_layers#1`
  and is the size of the method's +7.3% bias on this run (see the check
  below); separating the two is step 10's comparison, not this step's.

### `draw-6000` (15 window frames, `cart_draw` 13987, mean window frame 979, 2% = 19.6)

| Child | Method | Draw delta | Cost / frame | % of `cart_draw` | % of mean window frame | Calls / frame (PC; PSX) | Update delta | Submit delta | Game state | Text |
|---|---|---:|---:|---:|---:|---|---:|---:|---|---:|
| `dispatch_context_mem` | call-twice | −4 | ≤ 2 / 15 | < 0.1% | < 0.1% | 1–1 | +3 **leak** | +2 | same | +8 |
| `p8_cls` | call-twice | 0 | ≤ 2 / 15 | < 0.1% | < 0.1% | 1–1 | +3 **leak** | 0 | same | +8 |
| `hud_select_palette_block_default` | call-twice | −1 | ≤ 2 / 15 | < 0.1% | < 0.1% | 1–1 | +4 **leak** | 0 | same | +8 |
| `draw_layers#1` | call-twice | +13642 | 909 | 97.5% | 92.9% | 1–1; 1.00 | +7 | 0 | same | +8 |
| `draw_layers#2` | call-twice | +233 | 16 | 1.7% | 1.6% | 1–1 | +4 | 0 | same | +8 |
| `dispatch_context_gfx` | call-twice | −4 | ≤ 2 / 15 | < 0.1% | < 0.1% | 1–1 | +3 **leak** | +2 | same | +8 |
| `demo_overlay_draw` | call-twice | −5 | ≤ 2 / 15 | < 0.1% | < 0.1% | 1–1 | +2 | +2 | same | +24 |

- `draw_layers#1`'s delay probe: +295 draw over the window for 300 expected
  (tolerance 6); over the run 123.25 for 123 calls (+0.2%), 1.00 a window
  frame; update +2, submit +3, text +456 bytes.
- The negative draw deltas and the four update leaks are a code-layout
  effect, not a cost: `dispatch_context_mem` and `dispatch_context_gfx`
  insert different calls at different places but both add 8 bytes of text,
  and give the very same deltas on all three runs (−4 draw, +3 update, +2
  submit here; −1, +2, −1 on `draw-5564`; 0, 0, 0 on `start`). Every
  `cart_draw` patch shifts the code linked after `cart.c`, and `draw-6000`
  answers with a few hblanks either way over 15 frames. Each leak is at most
  4 hblanks over the window (0.3 a frame), two orders of magnitude under the
  2% threshold, so no reading below changes.

### Additivity check

Σ children is the sum of the draw deltas above (measured children at their
raw value, `start`'s `draw_layers#1` at its PC-derived 463). Gap =
(Σ − reference) ÷ reference.

| Run | Σ children | `cart_draw`'s own | Gap | Call-twice `cart_draw` delta | Gap | Verdict |
|---|---:|---:|---:|---:|---:|---|
| `start` (partly PC-derived) | 463 | 465 | −0.4% | 491 (game state differs) | −5.7% | within 10% |
| `draw-5564` | 20911 | 19491 | +7.3% | 20919 | −0.0% | within 10% |
| `draw-6000` | 13861 | 13987 | −0.9% | 13885 | −0.2% | within 10% |

- Every run is within 10%; no gap needs an explanation beyond this one.
- `draw-5564`'s +7.3% is the method's bias, not missing or double-counted
  cost: the same sum against the call-twice `cart_draw` delta is −8 hblanks
  over 20919 (−0.04%). A second call of a draw child costs as much more than
  the first as a second `cart_draw` does, and the children's second calls
  add up to the second `cart_draw` to the hblank. So `cart_draw`'s own body
  (its prologue and locals) costs nothing measurable, as expected.
- `draw-6000`, whose `cart_draw` bias is −0.7%, sums to −0.9% against the
  baseline draw and −0.2% against the call-twice delta: the same agreement.
- `start`'s check is partly PC-derived: `draw_layers#1`'s 463 decides the
  sum. Its independent cross-check is the residual (465, range 453–465 given
  the six children's ≤ 2 bounds), which the derived figure matches within
  0.4%.

### Patches

Generated from `cart.c` of `2458bad` with `sed` and `diff -u` into
`tmp/plan20-draw-l1/` (git-ignored), used once and discarded; none is
committed. Call sites are `cart_draw` in `src/game/cart.c` (lines 335–341 of
that commit).

| Probe label | Call site | Inserted statement |
|---|---|---|
| `l1-dispatch-context-mem-twice` | before `p8_vram_t *mem = dispatch_context_mem();` | `(void)dispatch_context_mem();` |
| `l1-p8-cls-twice` | after `p8_cls(mem);` | `p8_cls(mem);` |
| `l1-hud-select-palette-block-default-twice` | after `hud_select_palette_block_default(mem);` | `hud_select_palette_block_default(mem);` |
| `l1-draw-layers-1-twice` | after `draw_layers(ENTITY_MESSAGE_DRAW);` | `draw_layers(ENTITY_MESSAGE_DRAW);` (runs `draw-5564`, `draw-6000` only) |
| `l1-draw-layers-2-twice` | after `draw_layers(ENTITY_MESSAGE_DRAW_OVERLAY);` | `draw_layers(ENTITY_MESSAGE_DRAW_OVERLAY);` |
| `l1-dispatch-context-gfx-twice` | before `demo_overlay_draw(dispatch_context_gfx(), mem, g_demo_frame);` | `(void)dispatch_context_gfx();` |
| `l1-demo-overlay-draw-twice` | after `demo_overlay_draw(dispatch_context_gfx(), mem, g_demo_frame);` | the same statement again |
| `l1-draw-layers-1-delay` | before `draw_layers(ENTITY_MESSAGE_DRAW);`, plus `#include "platform/psx_probe.h"` after `#include "game/background.h"` | `psx_probe_delay(20);` |

### Hand-off to step 7

Children above 2% of the run's mean window frame are the branches step 7
goes down; the others are closed at level 1.

| Run | Above 2% (step 7 goes down) | Closed at level 1 |
|---|---|---|
| `start` | `draw_layers#1` (463 a frame, PC-derived, 4.9%) | the other six, each ≤ 2 |
| `draw-5564` | `draw_layers#1` (1082 a frame, 80.1%), `draw_layers#2` (312 a frame, 23.1%) | the other five, each ≤ 2 / 15 |
| `draw-6000` | `draw_layers#1` (909 a frame, 92.9%) | `draw_layers#2` (16 a frame, 1.6%, under the 19.6 threshold), the other five, each ≤ 2 / 15 |

So step 7 starts from `draw_layers(ENTITY_MESSAGE_DRAW)` on all three runs
and from `draw_layers(ENTITY_MESSAGE_DRAW_OVERLAY)` on `draw-5564` only.
`dispatch_context_mem`, `p8_cls`, `hud_select_palette_block_default`,
`dispatch_context_gfx` and `demo_overlay_draw` are closed everywhere. On
`start`, `draw_layers#1` is the help screen's draw, so step 7 keeps the fixed
delay below it wherever a call-twice would tick the RNG.

## Draw tree

Plan 20 step 7: below `cart_draw`'s level-1 branches, every node above 2%
of the run's mean window frame is opened one level deeper, down to a leaf
or a node under 2%. This section holds level 2 (the children of
`draw_layers#1` and `draw_layers#2`); levels 3 and below follow.

Measured on commit `93c106491d24e07a079032bfe2e0f6e19159c513` (pcsx-redux,
probe build, asserts off) with:

```
make psx-probe-baseline
make frame-profile-subtree ROOT=draw_layers PSX_SHORT_RUNS="start draw-5564 draw-6000"
make psx-probe PATCH=tmp/plan20-draw-tree/<probe>.patch PSX_SHORT_RUNS="<runs>" PSX_PROBE_LABEL=l2-<probe>
```

Method, per node opened on a run:

- **Map.** `make frame-profile-subtree ROOT=<function>` lists the node's
  children (direct and function-pointer calls, by call site) with their PC
  calls per window frame and their median PC share of the root (K = 4).
  The map never closes a branch.
- **Call-twice** (default): a method reached only through `draw_layers` is
  doubled in its own file (body renamed `<method>_once`, the method calls
  it twice: *wrapper*); a direct call is duplicated at its call site
  (*site*). The node's cost is the window draw delta. An update delta
  beyond max(2% of the draw delta, 2 hblanks) is marked **leak**.
- **Fixed delay + PC share** where a second call is not clean: the
  help screen (RNG tick), bookkeeping (list adds, frame-zone allocations,
  `apply_camera`), and methods whose second call spawns or ages dots and
  particles (it changes what later frames draw). `psx_probe_delay(d)` at the
  method's entry or the call site, d = clamp(⌊200 ÷ PC calls per window
  frame⌋, 1, 20), gives the PSX call count; the cost is the node's PC share
  of its parent times the parent's PSX cost, marked **PC-derived**.
- **Groups.** Children estimated under 2% by PC (PC share × parent's PSX
  cost) are doubled together in one patch. A group delta under 2% closes
  every member at "≤ group delta"; a group at or above 2% is split in two
  by PC estimate until each piece is under 2% or one function. PC only
  chooses the groups.
- **Leaves:** a PICO-8 runtime call (`p8_` prefix), or a game function with
  no instrumented child. **Self** = node cost − Σ measured children, a
  difference, flagged when above 2%.

Pre-checks (step 6's hypotheses H-6-1 to H-6-9), all held: "Draw level 1"
is committed with its hand-off table and is the last section; the manifest
still has six runs and step 6 probed `start`, `draw-5564`, `draw-6000`; the
branches above 2% are `draw_layers#1` on the three runs and
`draw_layers#2` on `draw-5564` only; `draw_layers#1` on `start` is
PC-derived (463 a frame); step 6's additivity held within 10% on every run,
no open item; `psx/probes/` holds only the three sanity fixtures. Since
step 6's probes (`2458bad`), the only commits under `src/` are
`e97bb51` (`src/cmd/frame_profile.c`, `src/devtools/call_profile.h`, PC
profile tooling, not compiled into the PSX build) and `93c1064` (nothing
under `src/` or `psx/`): `src/` has the same tree id at both, so the
baseline below, built at `e97bb51`, carries `HEAD`'s fingerprint.

### Rebuilt probe baseline

Window sums in hblanks, every boot `valid` and `game-state same`; text is
the probe build's ELF text size.

| Run | Update | Draw | Submit | Total | Text | Step 6 total | Gap | Mean window frame | 2% of it |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| `start` | 8980 | 465 | 6 | 9451 | 319192 | 9451 | 0 | 9451 | 189.0 |
| `update-4737` | 118 | 706 | 8 | 832 | 322128 | 832 | 0 | 832 | 16.6 |
| `update-5500` | 1409 | 714 | 7 | 2130 | 322308 | 2130 | 0 | 2130 | 42.6 |
| `draw-5564` | 655 | 19491 | 118 | 20264 | 322796 | 20264 | 0 | 1351 | 27.0 |
| `update-5719` | 46 | 1078 | 7 | 1131 | 322972 | 1131 | 0 | 1131 | 22.6 |
| `draw-6000` | 593 | 13987 | 107 | 14687 | 324024 | 14687 | 0 | 979 | 19.6 |

Every part equals step 6's baseline exactly. Over the 15-frame windows the
2% threshold is 405 hblanks on `draw-5564` and 294 on `draw-6000`.

### Map of `draw_layers` (PC)

`draw_layers` is one function called twice a frame; the subtree root sums
both calls, so the profile's shares are of `draw_layers#1` + `#2`. A draw
method runs only under `#1`, an overlay method only under `#2`. The
compiler split the dispatch loop by message, so `entity_has` and
`entity_pos` show one call site per pass (`#1` draw, `#2` overlay);
`entity_list_add`, `p8_fixed_flr_int`, `frame_zone` and `entity_list_init`
are shared and split by call count (the bucket sizes; 15 each for the two
init calls). Share of `#1` = share of the root ÷ `#1`'s share of
`#1` + `#2` (step 6's coverage medians: 99.84% `start`, 75.42%
`draw-5564`, 99.23% `draw-6000`). Calls per window frame, min–max.

| Child | Pass | `start` calls | `start` share of root | `draw-5564` calls | `draw-5564` share | `draw-6000` calls | `draw-6000` share |
|---|---|---:|---:|---:|---:|---:|---:|
| `background_draw_method` | #1 | 1 | 64.6% | 1 | 31.2% | 1 | 40.5% |
| `planet_draw_method` | #1 | 90 | 17.0% | 70 | 9.0% | 72 | 16.3% |
| `score_panel_draw_method` | #1 | 1 | 9.8% | 1 | 4.7% | 1 | 6.3% |
| `button_draw_method` | #1 | 4 | 2.5% | 11 | 14.6% | 4–6 | 4.3% |
| `slipway_draw_method` | #1 | — | — | 38 | 8.7% | 40 | 17.7% |
| `ascension_draw_method` | #1 | — | — | 3 | 2.6% | 4 | 5.9% |
| `emitter_update_method` | #1 | — | — | 1 | 0.8% | 1 | 1.6% |
| `structure_icon_draw_method` | #1 | 7 | 0.4% | 7 | 0.4% | 7 | 0.7% |
| `camera_apply_method` | #1 | 5 | 0.1% | 7 | 0.1% | 7–8 | 0.1% |
| `game_data_fixed_seq_at` | #1 | 5 | 0.0% | 7 | 0.0% | 7–8 | 0.0% |
| `entity_has#1`, `entity_pos#1` | #1 | 103 | 0.9% | 132 | 0.7% | 130–132 | 0.9% |
| `message_box_draw_overlay_method` | #2 | — | — | 1 | 21.3% | — | — |
| `button_draw_overlay_method` | #2 | 4 | 0.0% | 11 | 1.8% | 4–6 | 0.0% |
| `slipway_draw_overlay_method` | #2 | — | — | 38 | 0.3% | 40 | 0.4% |
| `targeting_tooltip_method` | #2 | — | — | 11 | 0.1% | 14 | 0.1% |
| `ui_draw_overlay_method` | #2 | 1 | 0.0% | 1 | 0.0% | 1 | 0.0% |
| `entity_has#2`, `entity_pos#2` | #2 | 5 | 0.1% | 62 | 0.4% | 59–61 | 0.5% |
| `entity_list_add`, `p8_fixed_flr_int` | both | 108 | 1.1% | 194 | 1.0% | 189–193 | 1.3% |
| `frame_zone`, `entity_list_init` | both | 30 | 0.3% | 30 | 0.1% | 30 | 0.2% |
| `entities_by_message` (#1, #2) | both | 1 + 1 | 0.0% | 1 + 1 | 0.0% | 1 + 1 | 0.0% |
| self | both | | 2.7% | | 2.6% | | 3.3% |

`help_screen_draw_method` is not called on `start`'s window frame (12): the
help screen draws on run frames 0–11 only. No level-2 child on `start`
runs it, and every wrapper probe there doubles its method over the whole
run with `game-state same`, so step 1.3's RNG exclusion applied to no
level-2 node.

### Probes run (level 2)

Every probe `valid` on every run, every game state `same`. Deltas are
window sums over the probe baseline. A delay probe's "PSX calls / frame" is
the tool's window figure (delta ÷ d ÷ frames).

| Probe | Run | Draw delta | Update | Submit | Text | Delay check |
|---|---|---:|---:|---:|---:|---|
| `background-draw-method-twice` | `start` / `draw-5564` / `draw-6000` | +251 / +3463 / +3503 | 0 / +1 / +5 | 0 / −3 / 0 | +36 | |
| `planet-draw-method-twice` | same | +93 / +2203 / +3049 | 0 / +5 / +6 | 0 / −2 / +1 | +68 | |
| `score-panel-draw-method-twice` | same | +84 / +1303 / +1296 | 0 / −2 / +4 | 0 / −1 / −1 | +32 | |
| `button-draw-method-twice` | same | +12 / +4603 / +897 | +1 / +1 / 0 | 0 / −2 / −1 | +68 | |
| `group-draw-layers-1-1` | same | +6 / +179 / +217 | +1 / +2 / +3 | 0 / −2 / +1 | +328 | |
| `slipway-draw-method-twice` | `draw-5564` / `draw-6000` | +2422 / +2668 (not used) | +2 / 0 | −1 / +1 | +40 | |
| `slipway-draw-method-delay` (d 5) | same | +2869 / +3020 | +2 / +3 | 0 / +1 | +460 | 38.28 / 40.32 a frame (PC 38 / 40); run +0.7% vs counted calls |
| `ascension-draw-method-twice` | same | +1418 / +860 (not used) | +2 / +3 | −2 / +2 | +164 | |
| `ascension-draw-method-delay` (d 20) | same | +899 / +1200 | +1 / +3 | +1 / 0 | +460 | 3.00 / 4.01 (PC 3 / 4); +0.2% |
| `emitter-update-method-delay` (d 20) | same | +299 / +302 | +2 / +2 | −1 / −2 | +460 | 1.00 / 1.01 (PC 1 / 1); ≤ +0.1% |
| `entity-list-add-1-delay` (d 1) | same | +2049 / +2043 | +5 / +3 | −3 / 0 | +476 | 136.73 / 136.40 (PC 132 / 130–132); +3.5% / +3.6% |
| `entity-list-init-frame-zone-1-delay` (d 13) | same | +2931 / +2932 | +6 / +4 | 0 / −1 | +500 | 15.06 / 15.05 (PC 15 / 15); +0.3% |
| `camera-apply-method-delay` (d 20) | same | +2106 / +2342 | +2 / +3 | −3 / +3 | +476 | 7.02 / 7.83 (PC 7 / 7–8); +0.2% |
| `message-box-draw-overlay-method-twice` | `draw-5564` | +4016 | +2 | 0 | +40 | |
| `group-draw-layers-2-1` | `draw-5564` | +564 (≥ 405: split) | +8 | −1 | +484 | |
| `button-draw-overlay-method-twice` | `draw-5564` | +428 | +9 **leak** | −1 | +32 | |
| `group-draw-layers-2-3` | `draw-5564` | +133 | 0 | −1 | +452 | |

- Every delay probe counts the PC calls on the PSX. The 1-hblank delay
  over-runs by 3.5% on both runs and over the whole run as well (5641
  hblanks for 5448 counted calls on `draw-5564`): the call's own overhead
  at the shortest delay, not extra calls. The tables below divide it out
  (132.1 and 131.6 calls a frame).
- `button-draw-overlay-method-twice`'s update delta (+9) is just over its
  bound (8.6); its group (`group-draw-layers-2-1`, +8 for a bound of 11) and
  every other probe stay within theirs. Like step 6's `draw-6000` leaks, it
  is a few hblanks over 15 frames (0.6 a frame), far under the threshold.
- **Call-twice not used for the spawning methods.** `slipway_draw`
  (`src/game/slipway_display.c:88`) ages its transit dots, frees the expired
  ones and spawns two more on every call; `ascension_draw`
  (`src/game/planet_family.c:170`) runs `emitter_update`, which advances and
  spawns particles, as `emitter_update_method` does alone. Their second call
  changes what the following frames draw, so they take the delay and a PC
  share. Their call-twice deltas are kept above, unused: 2422 against a
  PC-derived 1872 on `draw-5564` (the dots, not saved by the resume, build
  up faster when doubled), and 1418 against 550 for `ascension`.

### `start` (1 window frame, `draw_layers#1` 463 PC-derived, mean window frame 9451, 2% = 189.0)

Rooted on a PC-derived cost (step 6). Statuses: **open** = above 2%,
opened at level 3; **under** = closed under 2%; **≤ group** = closed by
the group bound.

| Node | Method | Calls / frame (PC; PSX) | Cost / frame | % of parent | % of mean window frame | Status |
|---|---|---|---:|---:|---:|---|
| `draw_layers#1` | fixed delay, PC-derived (step 6) | 1; 1.00 | 463 | — | 4.9% | explored |
| · `background_draw_method` | wrapper call-twice | 1 | 251 | 54.2% | 2.7% | **open** |
| · `planet_draw_method` | wrapper call-twice | 90 | 93 | 20.1% | 1.0% | under |
| · `score_panel_draw_method` | wrapper call-twice | 1 | 84 | 18.1% | 0.9% | under |
| · `button_draw_method` | wrapper call-twice | 4 | 12 | 2.6% | 0.1% | under |
| · `structure_icon_draw_method`, `entities_by_message#1`, `entity_has#1`, `p8_fixed_flr_int`, `game_data_fixed_seq_at`, `entity_pos#1` | group `group-draw-layers-1-1` | 7; 1; 103; 108; 5; 103 | ≤ 6 | ≤ 1.3% | ≤ 0.1% | ≤ group |
| · `entity_list_add`, `frame_zone`, `entity_list_init`, `camera_apply_method` | bookkeeping, left in self (PC: 0.9% of `#1`, about 4) | 103; 15; 15; 5 | in self | | | under |
| · self | difference | | 17 | 3.7% | 0.2% | under |

Σ measured children 446 against the node's 463: gap −3.7%, within 10%.

### `draw-5564` (15 window frames, mean window frame 1351, 2% = 27.0)

`draw_layers#1` (call-twice, 16225 over the window, 1082 a frame):

| Node | Method | Calls / frame (PC; PSX) | Window | Cost / frame | % of parent | % of mean window frame | Status |
|---|---|---|---:|---:|---:|---:|---|
| · `button_draw_method` | wrapper call-twice | 11 | 4603 | 307 | 28.4% | 22.7% | **open** |
| · `background_draw_method` | wrapper call-twice | 1 | 3463 | 231 | 21.3% | 17.1% | **open** |
| · `planet_draw_method` | wrapper call-twice | 70 | 2203 | 147 | 13.6% | 10.9% | **open** |
| · `slipway_draw_method` | delay 5, **PC-derived** (11.54% × 16225) | 38; 38.28 | 1872 | 125 | 11.5% | 9.2% | **open** |
| · `score_panel_draw_method` | wrapper call-twice | 1 | 1303 | 87 | 8.0% | 6.4% | **open** |
| · `ascension_draw_method` | delay 20, **PC-derived** (3.39%) | 3; 3.00 | 550 | 37 | 3.4% | 2.7% | **open** |
| · `emitter_update_method` | delay 20, **PC-derived** (1.08%) | 1; 1.00 | 175 | 12 | 1.1% | 0.9% | under |
| · `structure_icon_draw_method` + `#1` getters (as on `start`) | group `group-draw-layers-1-1` | 7; 132 | ≤ 179 | ≤ 12 | ≤ 1.1% | ≤ 0.9% | ≤ group |
| · `entity_list_add` (`#1` part) | delay 1, **PC-derived** (0.47%) | 132; 132.1 | 76 | 5 | 0.5% | 0.4% | under |
| · `entity_list_init`, `frame_zone` (`#1` part) | delay 13, **PC-derived** (0.04%, 0.05%) | 15; 15.06 | 7 + 8 | 1 | 0.1% | 0.1% | under |
| · `camera_apply_method` | delay 20, **PC-derived** (0.08%) | 7; 7.02 | 13 | 1 | 0.1% | 0.1% | under |
| · self | difference | | 1773 | 118 | 10.9% | 8.7% | **flagged** |

`draw_layers#2` (call-twice, 4686 over the window, 312 a frame):

| Node | Method | Calls / frame | Window | Cost / frame | % of parent | % of mean window frame | Status |
|---|---|---|---:|---:|---:|---:|---|
| · `message_box_draw_overlay_method` | wrapper call-twice | 1 | 4016 | 268 | 85.7% | 19.8% | **open** |
| · `button_draw_overlay_method` | wrapper call-twice (split out of `group-draw-layers-2-1`, +564) | 11 | 428 | 29 | 9.1% | 2.1% | **open** |
| · `slipway_draw_overlay_method`, `targeting_tooltip_method`, `ui_draw_overlay_method`, `entities_by_message#2`, `entity_has#2`, `p8_fixed_flr_int`, `entity_pos#2` | group `group-draw-layers-2-3` | 38; 11; 1; 1; 62; 62; 62 | ≤ 133 | ≤ 9 | ≤ 2.8% | ≤ 0.7% | ≤ group |
| · `entity_list_add`, `frame_zone`, `entity_list_init` (`#2` part) | bookkeeping, left in self (PC: about 44) | 62; 15; 15 | in self | | | | under |
| · self | difference | | 109 | 7 | 2.3% | 0.5% | under |

Gaps, Σ measured children against the node: `#2` 4577 vs 4686, −2.3%.
`#1` 14452 vs 16225, **−10.9%**, explained: the three PC-derived spawning
methods carry PC proportions, not PSX costs. With their call-twice deltas
instead (unused, upward-biased), Σ is 15870 (−2.2%) and the self 355 (24 a
frame). PC time puts `draw_layers`' own self at 3.49% of `#1`, about 566
(38 a frame). The PSX self is therefore somewhere between about 355 and 1773
over the window; it stays flagged, and step 7's level 3 replaces the
PC-derived terms with measured children of `slipway_draw_method` and
`ascension_draw_method`.

### `draw-6000` (15 window frames, mean window frame 979, 2% = 19.6)

`draw_layers#1` (call-twice, 13642 over the window, 909 a frame); `#2` is
closed at level 1 (16 a frame).

| Node | Method | Calls / frame (PC; PSX) | Window | Cost / frame | % of parent | % of mean window frame | Status |
|---|---|---|---:|---:|---:|---:|---|
| · `background_draw_method` | wrapper call-twice | 1 | 3503 | 234 | 25.7% | 23.9% | **open** |
| · `planet_draw_method` | wrapper call-twice | 72 | 3049 | 203 | 22.4% | 20.8% | **open** |
| · `slipway_draw_method` | delay 5, **PC-derived** (17.84% × 13642) | 40; 40.32 | 2434 | 162 | 17.8% | 16.6% | **open** |
| · `score_panel_draw_method` | wrapper call-twice | 1 | 1296 | 86 | 9.5% | 8.8% | **open** |
| · `button_draw_method` | wrapper call-twice | 4–6 | 897 | 60 | 6.6% | 6.1% | **open** |
| · `ascension_draw_method` | delay 20, **PC-derived** (5.99%) | 4; 4.01 | 818 | 55 | 6.0% | 5.6% | **open** |
| · `emitter_update_method` | delay 20, **PC-derived** (1.57%) | 1; 1.01 | 214 | 14 | 1.6% | 1.5% | under |
| · `structure_icon_draw_method` + `#1` getters | group `group-draw-layers-1-1` | 7; 130–132 | ≤ 217 | ≤ 14 | ≤ 1.6% | ≤ 1.5% | ≤ group |
| · `entity_list_add` (`#1` part) | delay 1, **PC-derived** (0.47%) | 130–132; 131.6 | 64 | 4 | 0.5% | 0.4% | under |
| · `entity_list_init`, `frame_zone` (`#1` part) | delay 13, **PC-derived** (0.04%, 0.05%) | 15; 15.05 | 6 + 6 | 1 | 0.1% | 0.1% | under |
| · `camera_apply_method` | delay 20, **PC-derived** (0.09%) | 7–8; 7.83 | 12 | 1 | 0.1% | 0.1% | under |
| · self | difference | | 1126 | 75 | 8.3% | 7.7% | **flagged** |

Gap: Σ 12516 vs 13642, −8.3%, within 10%. With the unused call-twice
deltas of the two spawning methods, Σ is 12792 and the self 850 (57 a
frame); PC time puts the self at 3.36% of `#1`, about 458 (31 a frame).
Either way it is above the 294 threshold.

### `draw_layers`: "do less" candidates

Explored on the three runs (`#1`) and on `draw-5564` (`#2`). Gains are
upper bounds per window frame, from the measured figures above.

1. **Recomputation: the fifteen layer lists rebuilt on every call**
   (`src/engine/entity_registry.c:199`). Each call allocates 15 lists from
   the frame zone and walks its whole message bucket (103 / 132 / 130–132
   entities for the draw pass, 5 / 62 / 59–61 for the overlay pass) to
   re-sort it by `flr(layer)`, then throws the lists away. An entity's layer
   is set at construction and rarely changes, and the buckets change only on
   register/unregister. Lists kept sorted by layer at registration (and
   re-sorted on a layer change) would remove the bucket pass entirely.
   Bound: the self plus the bookkeeping children plus the getters' group
   bound (which also holds `structure_icon_draw_method`), per frame:
   `start` ≤ 17 + 6; `draw-5564` `#1` ≤ 118 + 7 + 12 = 137 and `#2` ≤ 7 + 9
   = 16; `draw-6000` `#1` ≤ 75 + 6 + 14 = 95. The self also holds the
   dispatch loop, which stays. Pixel risk: identical, provided the per-layer
   order stays the registration order.
2. **Repeated calls: the overlay pass sorts again what the draw pass just
   sorted.** On `draw-5564`, 62 overlay entities (planets' tooltips,
   buttons, slipways) are bucketed a second time each frame. Covered by (1)
   when the sorted lists are per message; listed apart because it alone
   halves the bucket work if only one structure is kept. Bound: `#2`'s
   self and group, ≤ 16 a frame on `draw-5564`, ≤ 2 elsewhere (`#2` closed
   at level 1). Pixel risk: identical.
3. **Recomputation: the parallax camera recomputed per non-empty layer.**
   `camera_apply_method` (`src/game/scroll.c:21`) runs 5 / 7 / 7–8 times a
   frame, each time looking up the layer's factor in `layer_fades_table`
   and multiplying the camera position by it, though both only change when
   the camera moves. Bound: ≤ 1 a frame (PC-derived). Pixel risk:
   identical. Too small to matter; recorded for completeness.
4. **Repeated calls: every entity of a pass is dispatched whether or not it
   is on screen.** `planet_draw_method` (70–90 calls), `slipway_draw_method`
   (38–40) and `ascension_draw_method` draw every entity each frame and
   leave clipping to the primitives. Skipping entities whose bounds miss the
   screen saves their whole call. Bound: those methods' measured cost times
   the off-screen fraction, which this step does not measure; at most
   147 + 125 + 37 a frame on `draw-5564`, 203 + 162 + 55 on `draw-6000`,
   93 on `start`. Pixel risk: identical pixels, but `planet_draw` draws
   from `visual_random` (`src/game/planet_display.c:367`), so a skipped
   planet shifts it: `visual_random` only.

The known items are not at level 2: `hud_print_outlined`'s 9 prints sit
below the methods that print text (opened at level 3), and the draw meter
sits inside every runtime call (step 8).

### Level 2 hand-off

| Run | Opened at level 3 | Flagged self | Closed at level 2 |
|---|---|---|---|
| `start` | `background_draw_method` | — | `planet_draw_method`, `score_panel_draw_method`, `button_draw_method`, the group, the bookkeeping, self 17 |
| `draw-5564` | `#1`: `button_draw_method`, `background_draw_method`, `planet_draw_method`, `slipway_draw_method`, `score_panel_draw_method`, `ascension_draw_method`; `#2`: `message_box_draw_overlay_method`, `button_draw_overlay_method` | `draw_layers#1` (118 a frame, candidates 1–2) | `emitter_update_method`, the two groups, the bookkeeping, `#2`'s self 7 |
| `draw-6000` | `background_draw_method`, `planet_draw_method`, `slipway_draw_method`, `score_panel_draw_method`, `button_draw_method`, `ascension_draw_method` | `draw_layers#1` (75 a frame, candidates 1–2) | `emitter_update_method`, the group, the bookkeeping |

### Patches (level 2)

Generated from `93c1064` by a throwaway script into `tmp/plan20-draw-tree/`
(git-ignored), used once and discarded; none is committed. Wrapper: the
method's body renamed `<method>_once` (static), the method made to call it
twice with the same arguments. Delay: `#include "platform/psx_probe.h"`
after the file's first include, plus the statement.

| Probe label | Node(s) | Shape | Inserted statement |
|---|---|---|---|
| `l2-background-draw-method-twice` | `background_draw_method` (`src/game/background.c`) | wrapper | `background_draw_method_once(self, screen_pos);` twice |
| `l2-planet-draw-method-twice` | `planet_draw_method` (`src/game/planet_display.c`) | wrapper | idem |
| `l2-score-panel-draw-method-twice` | `score_panel_draw_method` (`src/game/score_panel.c`) | wrapper | idem |
| `l2-button-draw-method-twice` | `button_draw_method` (`src/game/ui_button.c`) | wrapper | idem |
| `l2-slipway-draw-method-twice` | `slipway_draw_method` (`src/game/slipway_display.c`) | wrapper (not used) | idem |
| `l2-ascension-draw-method-twice` | `ascension_draw_method` (`src/game/planet_family.c`) | wrapper (not used) | idem |
| `l2-message-box-draw-overlay-method-twice` | `message_box_draw_overlay_method` (`src/game/message_box.c`) | wrapper | idem |
| `l2-button-draw-overlay-method-twice` | `button_draw_overlay_method` (`src/game/ui_button.c`) | wrapper | idem |
| `l2-slipway-draw-method-delay` | `slipway_draw_method` | delay 5 at entry | `psx_probe_delay(5);` |
| `l2-ascension-draw-method-delay` | `ascension_draw_method` | delay 20 at entry | `psx_probe_delay(20);` |
| `l2-emitter-update-method-delay` | `emitter_update_method` (`src/game/particle.c`) | delay 20 at entry | `psx_probe_delay(20);` |
| `l2-entity-list-add-1-delay` | `entity_list_add` in `draw_layers` (`src/engine/entity_registry.c`) | delay 1 at site | `if (is_draw) psx_probe_delay(1);` before `entity_list_add(&layers[layer - 1], qc);` |
| `l2-entity-list-init-frame-zone-1-delay` | `entity_list_init`, `frame_zone` in `draw_layers` | delay 13 at site | `if (is_draw) psx_probe_delay(13);` before `entity_list_init(&layers[index], frame_zone());` |
| `l2-camera-apply-method-delay` | `camera_apply_method` via `apply_camera` in `draw_layers` | delay 20 at site | `psx_probe_delay(20);` before `g_main_camera->class->apply_camera(g_main_camera, ci);` |
| `l2-group-draw-layers-1-1` | `structure_icon_draw_method` (wrapper) and the `#1` getters | group | wrapper, plus in `draw_layers` under `if (is_draw)`: a second `entities_by_message`, `entity_has`, `p8_fixed_flr_int`, `game_data_fixed_seq_at` and `entity_pos` call on `volatile` copies of the arguments into `volatile` results, each before the original |
| `l2-group-draw-layers-2-1` | `button_draw_overlay_method`, `slipway_draw_overlay_method` (`src/game/slipway.c`), `targeting_tooltip_method` (`src/game/probe.c`), `ui_draw_overlay_method` (`src/game/ui.c`) (wrappers) and the `#2` getters | group | wrappers, plus the getters under `if (!is_draw)` (no `game_data_fixed_seq_at`, draw pass only) |
| `l2-group-draw-layers-2-3` | `group-draw-layers-2-1` without `button_draw_overlay_method` | group (split) | idem |

The `volatile` copies keep the compiler from merging the doubled
`static inline` getters with the originals.
