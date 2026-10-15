# PSX performance levers

This file will become plan 20's ranked inventory of the ways to make each PSX
frame do less work. For now it holds the baseline every lever is
measured against, the judge's asserts-off full-run baseline, the probe
tool's sanity checks, the PC frame profile's draw coverage, the split of
`cart_draw` among its direct children ("Draw level 1"), the draw tree
below them ("Draw tree"), the backend's cost per call and per unit of
every draw family ("Backend: per-unit costs"), the update spikes with
the calibration of PC update proportions ("Update spikes") and update on
the frames outside the spikes ("Ordinary update"). The full-run
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
`draw_layers#1` and `draw_layers#2`), levels 3 and below on `draw-5564`,
then on `draw-6000` and `start`, and the verification across the three
runs.

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

### Levels 3 and below, `draw-5564`

Every node opened on `draw-5564` at level 2 (the eight in the level-2
hand-off), down to a PICO-8 runtime leaf or a node under 2% (405 hblanks
over the window, 27.0 a frame). Measured on commit `b490c78` (same `src/`
and `psx/` tree ids as the rebuilt baseline, so no probe was refused as
stale) with:

```
make frame-profile-subtree ROOT=<function> PSX_SHORT_RUNS=draw-5564
make psx-probe PATCH=tmp/plan20-draw-tree/<probe>.patch PSX_SHORT_RUNS=draw-5564 PSX_PROBE_LABEL=l3-<probe>
```

Every probe of this chunk carries the `l3-` prefix whatever its level: the
prefix keeps the chunk's outputs apart from level 2's. 102 valid probes,
about 20 s each on one run; nine builds that failed on a missing
declaration in the patch were fixed and re-run, none is a figure. Method additions to the ones above:

- **Pass-through methods.** Each of the eight methods is one line,
  `X_draw(dispatch_context_gfx(), dispatch_context_mem(), self)`; the map
  gives `X_draw` 91–99.9% of the method on PC. Level 3 is therefore
  `X_draw` (site call-twice, the two getters read once into locals) plus
  the getters, closed together for the eight methods by one group.
- **Path flag.** A function reached from several parents
  (`layout_row_draw_at`, `row_draw_at_item_draw`, `hud_print_outlined`,
  `draw_rows_dispatch_one`, `emitter_update`, `planet_draw`) is doubled at
  its site only under one parent: the parent's call is wrapped in
  `probe_path = <code>` (saved and restored), the doubled call runs under
  `if (probe_path == <code>)`. Codes: 1 a button's label row, 2 a button's
  side row, 3 `planet_draw_method`, 4 the score panel's rows, 5 a message
  box article, 6 a button's tooltip, 7 `emitter_update` under
  `ascension_draw`, 8 `emitter_update_method`. `slipway_draw_line`'s two
  sites are told apart by their step (`s` = 3 border, 1 line), and
  `planet_draw` under its method by a static flag set in the method.
- **Cross-parent groups.** The small children of the shared functions
  (getters, `p8_fixed_*`, `vec_*`, palette selects) were doubled under all
  their parents at once: a group delta is a sum of non-negative costs, so
  it bounds each member under each parent. A node's self row includes its
  members of these groups.
- **Count probes.** `psx_probe_delay(20)` under a path flag, before the
  print or the sprite of `row_draw_at_item_draw`, gives a path's PSX calls
  a frame where PC cannot split them by parent. Count only, never a cost.
- **PC calls** are the subtree map's, from the function as root: for a
  shared function they add up every parent, so the PSX count is given
  where a path needs its own.
- **Bias.** A call-twice child often costs a little more than its parent's
  call-twice (up to +8% here, +17% on the 28-a-frame tooltip): second-call
  bias plus the path flag's load and test. Those selves are negative,
  labelled "bias", never corrected.

Every probe `valid`, every game state `same`. The update delta stayed
within max(2% of the draw delta, 2 hblanks) except on sixteen probes, all
by 3 to 11 hblanks over the window (under 1 a frame), the tooltip ones
among them as at level 2; they are marked in the probe list.

#### `draw_layers#1`'s branches

`slipway_draw_method` and `ascension_draw_method` keep their level-2
PC-derived figures as roots; `planet_draw`'s PC calls are the subtree's
(73 calls, both parents: 70 from its method, 3 from `ascension_draw`).

| Node | Method | Calls / frame (PC; PSX) | Window | Cost / frame | % of parent | % of mean window frame | Status |
|---|---|---|---:|---:|---:|---:|---|
| `button_draw_method` | level 2 | 11 | 4603 | 307 | — | 22.7% | explored |
| · `button_draw` | site call-twice | 11 | 4600 | 307 | 99.9% | 22.7% | explored |
| · · `layout_row_draw_at_default` (label row) | site call-twice | 11 | 3564 | 238 | 77.5% | 17.6% | explored |
| · · · `layout_row_draw_at` | site call-twice, path 1 | 11 | 3562 | 237 | 99.9% | 17.6% | explored |
| · · · · `row_draw_at_item_draw` | site call-twice, path 1 | 17; 17.05 | 3605 | 240 | 101.2% | 17.8% | explored |
| · · · · · `hud_print_outlined_default` | site call-twice, path 1 | 7; 7.03 | 3367 | 224 | 93.4% | 16.6% | explored |
| · · · · · · `hud_print_outlined` | site call-twice, path 1 | 7; 7.03 | 3362 | 224 | 99.9% | 16.6% | explored |
| · · · · · · · `p8_print` | site call-twice, path 1 | 63; 63.3 | 3355 | 224 | 99.8% | 16.6% | **leaf** |
| · · · · · · · self (incl. `group-l8-small`) | difference |  | 7 | 0 | 0.2% | 0.0% | under |
| · · · · · · self | difference |  | 5 | 0 | 0.1% | 0.0% | under |
| · · · · · `p8_spr_default` | site call-twice, path 1 | 10; 10.02 | 137 | 9 | 3.8% | 0.7% | leaf, under |
| · · · · · self (incl. palette selects, `group-l7-small`) | difference |  | 101 | 7 | 2.8% | 0.5% | under |
| · · · · self | difference |  | −43 | −3 | −1.2% | −0.2% | under (bias) |
| · · · self | difference |  | 2 | 0 | 0.1% | 0.0% | under |
| · · `layout_row_draw_at_default` (side row) | site call-twice | 5 | 704 | 47 | 15.3% | 3.5% | explored |
| · · · `layout_row_draw_at` | site call-twice, path 2 | 5 | 706 | 47 | 100.3% | 3.5% | explored |
| · · · · `row_draw_at_item_draw` | site call-twice, path 2 | 10; 10.05 | 758 | 51 | 107.4% | 3.7% | explored |
| · · · · · `hud_print_outlined_default` | site call-twice, path 2 | 5; 5.02 | 605 | 40 | 79.8% | 3.0% | explored |
| · · · · · · `hud_print_outlined` | site call-twice, path 2 | 5; 5.02 | 602 | 40 | 99.5% | 3.0% | explored |
| · · · · · · · `p8_print` | site call-twice, path 2 | 45; 45.2 | 614 | 41 | 102.0% | 3.0% | **leaf** |
| · · · · · `p8_spr_default` | site call-twice, path 2 | 5; 5.03 | 73 | 5 | 9.6% | 0.4% | leaf, under |
| · · · · · self | difference |  | 80 | 5 | 10.6% | 0.4% | under |
| · · `p8_rect` | site call-twice | 11 | 99 | 7 | 2.2% | 0.5% | leaf, under |
| · · `p8_rectfill` (top line) | site call-twice | 11 | 73 | 5 | 1.6% | 0.4% | leaf, under |
| · · `p8_rectfill` (body) | site call-twice | 11 | 70 | 5 | 1.5% | 0.3% | leaf, under |
| · · `p8_rectfill` (side background) | site call-twice | 5 | 34 | 2 | 0.7% | 0.2% | leaf, under |
| · · `ui_to_int`, `main_ui`, `ui_pad_tab`, `vec_*`, `p8_fixed_*` | group `group-bd-small` | 54; 20; 10; ~110 | ≤ 40 | ≤ 3 | ≤ 0.9% | ≤ 0.2% | ≤ group |
| · · self | difference |  | 16 | 1 | 0.3% | 0.1% | under |
| · `dispatch_context_gfx`, `dispatch_context_mem` | group `group-method-getters` (the 8 methods) | 11 + 11 | ≤ 32 | ≤ 2 | ≤ 0.7% | ≤ 0.2% | ≤ group |
| · self | difference |  | 3 | 0 | 0.1% | 0.0% | under |
| `background_draw_method` | level 2 | 1 | 3463 | 231 | — | 17.1% | explored |
| · `background_draw` | site call-twice | 1 | 3458 | 231 | 99.9% | 17.1% | explored |
| · · `p8_map` | site call-twice | 1 | 3319 | 221 | 96.0% | 16.4% | **leaf** |
| · · `p8_rectfill` (vertical grid) | site call-twice | 9 | 67 | 4 | 1.9% | 0.3% | leaf, under |
| · · `p8_rectfill` (horizontal grid) | site call-twice | 9 | 62 | 4 | 1.8% | 0.3% | leaf, under |
| · · `p8_rect`, `p8_fillp`, `p8_fillp_reset` | group `group-bg-small` | 1; 1; 1 | ≤ 3 | ≤ 0 | ≤ 0.1% | ≤ 0.0% | ≤ group |
| · · self | difference |  | 7 | 0 | 0.2% | 0.0% | under |
| · self (incl. getters) | difference |  | 5 | 0 | 0.1% | 0.0% | under |
| `planet_draw_method` | level 2 | 70 | 2203 | 147 | — | 10.9% | explored |
| · `planet_draw` | site call-twice | 70 | 2191 | 146 | 99.5% | 10.8% | explored |
| · · `layout_row_draw_at_method` (`draw_at`) | site call-twice, method path | 10 | 764 | 51 | 34.9% | 3.8% | explored |
| · · · `layout_row_draw_at` | site call-twice, path 3 | 10 | 768 | 51 | 100.5% | 3.8% | explored |
| · · · · `row_draw_at_item_draw` | site call-twice, path 3 | 43; 43.15 | 830 | 55 | 108.1% | 4.1% | explored |
| · · · · · `p8_spr_default` | site call-twice, path 3 | 43; 43.15 | 556 | 37 | 67.0% | 2.7% | **leaf** |
| · · · · · `hud_print_outlined_default` | site call-twice, path 3 | 0 | 15 | 1 | 1.8% | 0.1% | under |
| · · · · · self (palette selects, `group-l7-small`) | difference |  | 259 | 17 | 31.2% | 1.3% | under |
| · · · · self | difference |  | −62 | −4 | −8.1% | −0.3% | under (bias) |
| · · · self | difference |  | −4 | 0 | −0.5% | 0.0% | under |
| · · `p8_circfill` (disc) | site call-twice, method path | 13 | 536 | 36 | 24.5% | 2.6% | **leaf** |
| · · `p8_circ` (colony ring) | site call-twice, method path (split out of `group-pd-small`) | 6 | 388 | 26 | 17.7% | 1.9% | leaf, under |
| · · `p8_spr` (planet) | site call-twice, method path | 13 | 157 | 10 | 7.2% | 0.8% | leaf, under |
| · · `entity_geom_is_on_screen` | site call-twice, method path | 73 | 144 | 10 | 6.6% | 0.7% | under |
| · · `p8_spr_default` (tier) | site call-twice, method path (split) | 6 | 79 | 5 | 3.6% | 0.4% | leaf, under |
| · · palette selects, `p8_palt` ×2, `planet_is_colonized`, ring colour lookup, `main_ui`, `ui_pad_circle`, `entity_get`, `vec_*`, `p8_fixed_*` | group `group-pd-small-b` (split) | 13 each; 6 | ≤ 85 | ≤ 6 | ≤ 3.9% | ≤ 0.4% | ≤ group |
| · · self (incl. `entity_geom_shake`, which decays the shake) | difference | 13 | 38 | 3 | 1.7% | 0.2% | under |
| · self | difference |  | 12 | 1 | 0.5% | 0.1% | under |
| `slipway_draw_method` | level 2, **PC-derived** | 38 | 1872 | 125 | — | 9.2% | explored |
| · `slipway_draw` | delay 5, **PC-derived** (95.2%) | 38; 38.28 | 1782 | 119 | 95.2% | 8.8% | explored |
| · · `slipway_draw_line` (border, s = 3) | site call-twice | 15 | 525 | 35 | 29.5% | 2.6% | explored |
| · · · `p8_line` | site call-twice, s = 3 only | 60 | 355 | 24 | 67.6% | 1.8% | leaf, under |
| · · · `to_int`, `p8_fixed_*`, `vec_from_polar` ×2, `vec_add`, `slipway_end_pos`, `slipway_start`/`end` | group `group-sl1-rest` (split out of `group-sl1`, +547) | 120; ~300; 30; … | ≤ 207 | ≤ 14 | ≤ 39.4% | ≤ 1.0% | ≤ group |
| · · · self | difference |  | −37 | −2 | −7.0% | −0.2% | under (bias) |
| · · `slipway_draw_line` (line, s = 1) | site call-twice | 15 | 536 | 36 | 30.1% | 2.6% | explored |
| · · · `p8_line` | site call-twice, s = 1 only | 60 | 363 | 24 | 67.7% | 1.8% | leaf, under |
| · · · same small children | group `group-sl2-rest` (split out of `group-sl2`, +557) |  | ≤ 207 | ≤ 14 | ≤ 38.6% | ≤ 1.0% | ≤ group |
| · · · self | difference |  | −34 | −2 | −6.3% | −0.2% | under (bias) |
| · · `p8_rectfill` (transit dot) | site call-twice | 29–44 | 217 | 14 | 12.2% | 1.1% | leaf, under |
| · · `p8_pset` (transit dot) | site call-twice | 29–44 | 134 | 9 | 7.5% | 0.7% | leaf, under |
| · · `entity_geom_is_on_screen` ×2 | site call-twice | 38 + 28 | 126 | 8 | 7.1% | 0.6% | under |
| · · `slipway_spawn_transit_dot` ×2 | delay 6, **PC-derived** (1.7%) | 30; 30.17 | 30 | 2 | 1.7% | 0.1% | under |
| · · `to_int`, `vec_add`, `slipway_start`/`end`, `targeting_target`, `entity_get`, `int_seq_at`, `entity_has`, `particle_pixels_hidden`, `p8_fixed_*` | group `group-sd-small` | 132–177; … | ≤ 109 | ≤ 7 | ≤ 6.1% | ≤ 0.5% | ≤ group |
| · · `pointer_list_del`, `allocator_release`, `particle_zone` | bookkeeping, left in self | 0–2 | | | | | under |
| · · self (dot ageing loop) | difference with a PC-derived node |  | 105 | 7 | 5.9% | 0.5% | under |
| `score_panel_draw_method` | level 2 | 1 | 1303 | 87 | — | 6.4% | explored |
| · `score_panel_draw` | site call-twice | 1 | 1301 | 87 | 99.8% | 6.4% | explored |
| · · `draw_functions_draw_rows_default` | site call-twice | 1 | 1300 | 87 | 99.9% | 6.4% | explored |
| · · · `draw_rows_dispatch_one` | site call-twice, path 4 | 12 | 1296 | 86 | 99.7% | 6.4% | explored |
| · · · · `print_outlined_entry` (readouts) | site call-twice of `fn(e)`, path 4 | 7 | 1145 | 76 | 88.3% | 5.7% | explored |
| · · · · · `hud_print_outlined_default` | site call-twice, path 4 | 7 | 1094 | 73 | 95.5% | 5.4% | explored |
| · · · · · · `hud_print_outlined` | site call-twice, path 4 | 7 | 1097 | 73 | 100.3% | 5.4% | explored |
| · · · · · · · `p8_print` | site call-twice, path 4 | 63 | 1101 | 73 | 100.4% | 5.4% | **leaf** |
| · · · · · `number`, `is_number`, `draw_functions_arg`, `p8_fixed_flr_int` | in `group-l7-small` | 7 each | | | | | ≤ group |
| · · · · · self (incl. `p8_tostr_fixed` + `frame_zone`, bookkeeping) | difference | 2 + 2 | 51 | 3 | 4.5% | 0.3% | under |
| · · · · `draw_help_frame_entry` (frames) | site call-twice of `fn(e)`, path 4 | 3 | 135 | 9 | 10.4% | 0.7% | under |
| · · · · `draw_bar_entry` (bars) | site call-twice of `fn(e)`, path 4 | 2 | 21 | 1 | 1.6% | 0.1% | under |
| · · · · self | difference |  | −5 | 0 | −0.4% | 0.0% | under |
| · · · self | difference |  | 4 | 0 | 0.3% | 0.0% | under |
| · · `score_panel_frame_rows`, `score_panel_readouts` | in `group-l4-small` | 1; 1 | | | | | ≤ group |
| `ascension_draw_method` | level 2, **PC-derived** | 3 | 550 | 37 | — | 2.7% | explored |
| · `ascension_draw` | delay 20, **PC-derived** (98.3%) | 3; 3.00 | 541 | 36 | 98.4% | 2.7% | explored |
| · · `emitter_update` | children measured (delay 20: PC-derived 289, rejected) | 3; 2.99 | ≥ 512 | ≥ 34 | ≥ 94.6% | ≥ 2.5% | explored |
| · · · `p8_pset` | site call-twice, path 7 | 81–123 (both parents) | 293 | 20 | 57.2% | 1.4% | leaf, under |
| · · · `emitter_of`, `emitter_host_layer_pos`, `entity_geom_is_on_screen`, `vec_add`, `p8_fixed_*`, `particle_to_int`, `int_seq_at` | group `p7-emitter-arith` (split out of `p7-emitter-children`, +513) | 81–123 each (both parents) | ≤ 219 | ≤ 15 | ≤ 42.8% | ≤ 1.1% | ≤ group |
| · · · `emitter_spawn` | delay 20 (PSX count only) | 2; 1.99 | | | | | under (PC 1.9%) |
| · · · self (particle ageing loop) | not measurable |  | | | | | open (PC 45.7%) |
| · · `planet_draw` | site call-twice | 3 | 243 | 16 | 44.9% | 1.2% | under |

#### `draw_layers#2`'s branches

| Node | Method | Calls / frame (PC; PSX) | Window | Cost / frame | % of parent | % of mean window frame | Status |
|---|---|---|---:|---:|---:|---:|---|
| `message_box_draw_overlay_method` | level 2 | 1 | 4016 | 268 | — | 19.8% | explored |
| · `message_box_draw_overlay` | site call-twice | 1 | 4016 | 268 | 100.0% | 19.8% | explored |
| · · `message_box_draw_article` | site call-twice | 12 | 4017 | 268 | 100.0% | 19.8% | explored |
| · · · `layout_row_draw_at` (layout-row articles) | site call-twice, path 5 | 7 | 3952 | 263 | 98.4% | 19.5% | explored |
| · · · · `row_draw_at_item_draw` | site call-twice, path 5 | 23; 23.08 | 4008 | 267 | 101.4% | 19.8% | explored |
| · · · · · `hud_print_outlined_default` | site call-twice, path 5 | 13; 13.05 | 3756 | 250 | 93.7% | 18.5% | explored |
| · · · · · · `hud_print_outlined` | site call-twice, path 5 | 13; 13.05 | 3746 | 250 | 99.7% | 18.5% | explored |
| · · · · · · · `p8_print` | site call-twice, path 5 | 117; 117.5 | 3726 | 248 | 99.5% | 18.4% | **leaf** |
| · · · · · · · self | difference |  | 20 | 1 | 0.5% | 0.1% | under |
| · · · · · · self | difference |  | 10 | 1 | 0.3% | 0.0% | under |
| · · · · · `p8_spr_default` | site call-twice, path 5 | 10; 10.03 | 139 | 9 | 3.5% | 0.7% | leaf, under |
| · · · · · self | difference |  | 113 | 8 | 2.8% | 0.6% | under |
| · · · · self | difference |  | −56 | −4 | −1.4% | −0.3% | under (bias) |
| · · · `draw_functions_draw_row` (shared-row articles: bars) | site call-twice, path 5 | 5 | 50 | 3 | 1.2% | 0.2% | under |
| · · · self | difference |  | 15 | 1 | 0.4% | 0.1% | under |
| · · `p8_camera` ×12, `p8_camera_reset`, `p8_fixed_*` | in `group-l4-small` | 12; 1; ~20 | | | | | ≤ group |
| · · self | difference |  | −1 | 0 | 0.0% | 0.0% | under |
| · self | difference |  | 0 | 0 | 0.0% | 0.0% | under |
| `button_draw_overlay_method` | level 2 | 11 | 428 | 29 | — | 2.1% | explored |
| · `button_draw_overlay` | site call-twice | 11 | 429 | 29 | 100.2% | 2.1% | explored |
| · · `tooltip_draw_at` | site call-twice | 0–1 | 421 | 28 | 98.1% | 2.1% | explored |
| · · · `layout_row_draw_at` | site call-twice, path 6 | 0–2 | 419 | 28 | 99.5% | 2.1% | explored |
| · · · · `row_draw_at_item_draw` | site call-twice, path 6 | 0–2; 0.88 | 491 | 33 | 117.2% | 2.4% | explored |
| · · · · · `hud_print_outlined_default` | site call-twice, path 6 | 0–2; 0.88 | 405 | 27 | 82.5% | 2.0% | under (at 2%, not above) |
| · · · · · `p8_spr_default` | site call-twice, path 6 | 0 | 18 | 1 | 3.7% | 0.1% | under |
| · · · · · self | difference |  | 68 | 5 | 13.8% | 0.3% | under |
| · · · · self | difference |  | −72 | −5 | −17.2% | −0.4% | under (bias) |
| · · · `hud_draw_bar` | site call-twice, path 6 | 0–1 | 3 | 0 | 0.7% | 0.0% | under |
| · · · `p8_fillp`, `p8_fillp_reset`, `p8_fixed_*`, `vec_make` | in `group-l5-small` | 0–1; 0–2 | | | | | ≤ group |
| · · · self | difference |  | −1 | 0 | −0.2% | 0.0% | under |
| · · `p8_fixed_from_int` ×2 | in `group-l4-small` | 0–1 | | | | | ≤ group |
| · · self | difference |  | 8 | 1 | 1.9% | 0.0% | under |
| · self | difference |  | −1 | 0 | −0.2% | 0.0% | under |

Cross-parent groups (all `valid`, game state `same`):

| Group | Members, doubled under | Window delta |
|---|---|---:|
| `group-method-getters` | `dispatch_context_gfx`, `dispatch_context_mem` in the eight level-2 methods | 32 |
| `group-l4-small` | `score_panel_frame_rows`, `score_panel_readouts`; `message_box_draw_overlay`'s `p8_camera`, `p8_camera_reset` and fixed-point arithmetic; `button_draw_overlay`'s and `ascension_draw`'s `p8_fixed_from_int` | ≤ 2 over the window |
| `group-l5-small` | `p8_fixed_from_int` in `layout_row_draw_at_default` (paths 1, 2); the getters of `layout_row_draw_at_method` (3); `tooltip_draw_at`'s `p8_fillp`, `p8_fillp_reset`, arithmetic and `vec_make` (6) | 21 |
| `group-l6-small` | `layout_row_draw_at`'s `vec_sub`, `vec_make`, `p8_fixed_mul`, `p8_fixed_from_int` (paths 1, 2, 3, 5, 6) | 54 |
| `group-l7-small` | `row_draw_at_item_draw`'s `vec_add` and its two palette selects around each sprite (1, 2, 3, 5, 6); `print_outlined_entry`'s `number`, `is_number`, `draw_functions_arg`, `p8_fixed_flr_int` (4) | 379 |
| `group-l8-small` | `hud_print_outlined_default`'s `p8_fixed_from_int`; `hud_print_outlined`'s `to_int`, `p8_fixed_mul`, `strlen` (1, 2, 4, 5) | 65 |

#### Children-sum gaps

Σ measured children (individual probes and per-node groups) against each
explored node's cost:

| Node (path) | Node | Σ children | Gap |
|---|---:|---:|---:|
| `button_draw` | 4600 | 4584 | −0.3% |
| `background_draw` | 3458 | 3451 | −0.2% |
| `planet_draw` (method) | 2191 | 2153 | −1.7% |
| `slipway_draw` | 1782 PC-derived | 1677 | −5.9% |
| `ascension_draw` | 541 PC-derived | ≥ 755 | **≥ +39.6%** |
| `message_box_draw_article` | 4017 | 4002 | −0.4% |
| `tooltip_draw_at` | 421 | 422 | +0.2% |
| `slipway_draw_line` s = 3 / s = 1 | 525 / 536 | 562 / 570 | +7.0% / +6.3% |
| `layout_row_draw_at` (1 / 2 / 3 / 5) | 3562 / 706 / 768 / 3952 | 3605 / 758 / 830 / 4008 | +1.2% / +7.4% / +8.1% / +1.4% |
| `layout_row_draw_at` (6) | 419 | 491 | **+17.2%** |
| `draw_rows_dispatch_one` | 1296 | 1301 | +0.4% |
| `row_draw_at_item_draw` (1 / 2 / 5) | 3605 / 758 / 4008 | 3504 / 678 / 3895 | −2.8% / −10.6% / −2.8% |
| `row_draw_at_item_draw` (3) | 830 | 571 | **−31.2%** |
| `row_draw_at_item_draw` (6) | 491 | 423 | **−13.8%** |
| `print_outlined_entry` | 1145 | 1094 | −4.5% |
| `hud_print_outlined_default` (1 / 2 / 4 / 5) | 3367 / 605 / 1094 / 3756 | 3362 / 602 / 1097 / 3746 | −0.1% / −0.5% / +0.3% / −0.3% |
| `hud_print_outlined` (1 / 2 / 4 / 5) | 3362 / 602 / 1097 / 3746 | 3355 / 614 / 1101 / 3726 | −0.2% / +2.0% / +0.4% / −0.5% |
| the pass-through methods and score panel links | | | 0.0% to −0.5% |

Gaps above 10%:

- **`ascension_draw`, ≥ +39.6%**: its parent's cost is PC-derived and too
  low; see the open item below.
- **`layout_row_draw_at` (6), +17.2%**: second-call bias on a node of 28 a
  frame drawn on some frames only (the hovered button's tooltip); the
  doubled `row_draw_at_item_draw` costs 72 more than the doubled
  `layout_row_draw_at` above it, which no self can produce.
- **`row_draw_at_item_draw` (3), −31.2%**, and **(2), −10.6%**, **(6),
  −13.8%**: self time. The item's own work is the two palette selects
  around each sprite (`hud_select_palette_block`, a 16-byte `memcpy` each),
  three `p8_fixed_flr_int` and a `vec_add`, all in `group-l7-small` (379);
  path 3 draws 43 of the 68 row sprites a frame, so most of the group is
  its own. The selves stay under 2% (17, 5 and 5 a frame).

#### Open item from level 2: the −10.9% on `draw_layers#1`

Level 2 left `draw_layers#1`'s gap to the PC-derived costs of
`slipway_draw_method` (1872) and `ascension_draw_method` (550), whose
call-twice deltas (2418, 1416) are not used because their second call
ages and spawns dots and particles. Their children are clean and were
measured:

- **`slipway_draw_method`: the PC-derived figure holds.** `slipway_draw`'s
  measured children add up to 1647, plus 30 PC-derived for the two
  `slipway_spawn_transit_dot`, so 1677, within 10% of the PC-derived 1782
  (1872 for the method); the difference is the dot-ageing loop's own
  stores, list deletes and releases, which no clean probe reaches. The
  call-twice 2418 would need a 741 self: it is the outlier (doubled
  spawning builds up the dots, as level 2 said).
- **`ascension_draw_method`: the PC-derived figure is too low.**
  `planet_draw` under it costs 243 and `emitter_update`'s clean children
  alone 512 (`p8_pset` 293, the per-particle arithmetic 219), so its cost
  is at least 755 before `emitter_update`'s particle-ageing self (45.7% of
  `emitter_update` on PC) and `emitter_spawn` (two calls a frame). The
  delay probe's PC-derived 289 for `emitter_update` is rejected. The
  call-twice 1416 stays the upper side.
- **Outcome.** With 755 for `ascension_draw_method`, Σ of `draw_layers#1`'s
  children is 14657 against 16225: gap −9.7%, within 10%. The −10.9% was
  that PC-derived term. `draw_layers#1`'s self is at most 1568 (105 a
  frame), and that still holds the unmeasured particle-ageing self; it is
  at least 361 only with both rejected call-twice figures. It stays
  **flagged**: about 1100 (74 a frame) if `emitter_update`'s self is in its
  PC proportion, PC's own figure for `draw_layers`' self being 566.
- **Consequence for a level-2 closure.** `emitter_update_method` was closed
  at level 2 on a PC-derived 175. Its clean children alone measure 241
  (`p8-emitter-method-children`): still under 405, but in PC proportion its
  particle-ageing self would bring it to about 460. It is recorded here as
  **explored** on `draw-5564` (children `p8_pset` and arithmetic, both
  under 2%; self not measurable), with `emitter_update`'s candidates;
  chunk 5's verification table takes it from here.

#### Where the known items sit

- **The 9× outlined print** is `hud_print_outlined`'s loop
  (`src/game/hud.c:85`), reached through four paths above 2%: button label
  rows (63 `p8_print` a frame, 3355), button side rows (45, 614), score
  panel readouts (63, 1101) and message box articles (117, 3726); the
  tooltip (8 a frame; 405 for its whole outlined print) sits at the
  threshold. 289 `p8_print` a frame cost 8796
  over the window, 586 a frame: **43.4% of the mean window frame**, 45% of
  `draw-5564`'s draw.
- **The draw meter** sits inside every runtime leaf; not separable here
  (step 8).

#### "Do less" candidates (levels 3 and below, `draw-5564`)

Gains are upper bounds over the window (per frame in brackets), from the
measured costs above; the eight pass-through methods and `X_draw` links
add at most 12 each and get none of their own.

1. **Repeated calls: nine `p8_print` per outlined text** —
   `hud_print_outlined` (`src/game/hud.c:85`), explored on paths 1, 2, 4,
   5. Eight prints draw the same glyphs in colour 0 at the eight
   neighbouring offsets, the ninth in the text colour. Removing the eight
   outline calls (an outline drawn once, from a pre-outlined font or a
   cached text image) bounds at 8/9 of the leaf: 2982 (199) labels, 546
   (36) side rows, 979 (65) readouts, 3312 (221) message box; 7819 (521)
   together. Pixel risk: identical only with an equivalent outline;
   dropping the outline would change pixels, ruled out by the plan unless
   reopened. The plan's "Already known" item; step 8 bounds it by
   experiment.
2. **Identical redraw: button labels and side rows every frame** —
   `button_draw` (`src/game/ui_button.c:254`, `:271`). Eleven buttons
   redraw their layout rows (7 texts and 10 sprites a frame, plus 5 texts
   and 5 sprites of side rows) though a row changes only when the button
   is refreshed. Drawn once into an off-screen image and copied each
   frame: bound 3564 + 704 = 4268 (285), less the copy. The hovered
   button's tooltip (`tooltip_draw_at`, `src/game/layout.c:285`) is the
   same case: 421 (28). Pixel risk: identical.
3. **Identical redraw: the open message box** — `message_box_draw_overlay`
   (`src/game/message_box.c:128`). Twelve articles (13 outlined texts, 10
   sprites, 5 bars a frame) redrawn unchanged while the box stays open;
   only the per-article camera shake (`p8_camera`, `:126`) moves them
   during the opening. Bound 4017 (268), less a copy. Pixel risk:
   identical if the shake offsets are kept.
4. **Identical redraw and recomputation: the score panel readouts** —
   `print_outlined_entry` (`src/game/draw_functions.c:54`). Seven outlined
   readouts each frame, changing only when a value changes; two of them
   are formatted again every frame by `p8_tostr_fixed` into the frame zone
   (`:56`). Bound 1145 (76) for the redraw, ≤ 51 (3) for the formatting
   (it is in the self). Pixel risk: identical.
5. **Identical redraw: the background map** — `background_draw`
   (`src/game/background.c:143`). `p8_map` draws the same 32×32 tiles
   every frame; only the camera moves. Kept as a pre-built background in
   VRAM and copied with the camera offset: bound 3319 (221), less the
   copy. Pixel risk: identical while the map memory does not change.
   `p8_map`'s inside is step 8's.
6. **Identical redraw: planet row icons** — `row_draw_at_item_draw` on
   path 3 (`src/game/layout.c:196`). 43 `p8_spr_default` a frame (556) for
   the resource icons under the planets, unchanged until the row is
   rebuilt. Bound: the item draw, 830 (55). Pixel risk: identical.
7. **Repeated calls: palette block selected around every row sprite** —
   `row_draw_at_item_draw` (`src/game/layout.c:195`, `:199`). Each sprite
   copies its palette block, draws, and copies block 0 back: two 16-byte
   copies per sprite, 68 sprites a frame, though consecutive icons mostly
   share a block. Skipping a select that changes nothing: bound ≤ 379 (25),
   the group that holds them. Pixel risk: identical.
8. **Identical redraw: a black disc and a ring behind each planet** —
   `planet_draw` (`src/game/planet_display.c:375`, `:393`). 13
   `p8_circfill` (536) and 6 `p8_circ` (388) a frame, 2.7 and 4.3 hblanks a
   call, redrawn identically while the camera stands still. Baked into the
   planet sprite they would cost one sprite each: bound 924 (62), less the
   larger sprites. Pixel risk: identical only if the baked sprite
   reproduces the disc exactly, otherwise would change pixels.
9. **Correction to `draw_layers` candidate 4 (off-screen dispatch).**
   `planet_draw` already returns early off screen
   (`src/game/planet_display.c:358`): of 73 calls a frame, 13 draw. What a
   culled planet still costs is its dispatch and `entity_geom_is_on_screen`
   (144, 10). The bound of that candidate shrinks accordingly for planets;
   slipways also test both end planets (below).
10. **Recomputation: slipway end points computed twice** —
    `slipway_draw_line` (`src/game/slipway_display.c:72`, `:74`). The
    border and the line pass each recompute both wobbling end points
    (`vec_from_polar`, a sine and a cosine each) for the same `t`.
    Computed once per slipway: bound half the two small groups, 207 (14).
    Pixel risk: identical.
11. **Repeated calls: four lines per slipway pass** — `slipway_draw_line`
    (`src/game/slipway_display.c:83`). Each pass draws its line four times
    at 2×2 offsets: 8 `p8_line` a slipway, 120 a frame (718). Bound 3/4 of
    it, 539 (36), if one primitive covered the four. Pixel risk: identical
    if the backend batches the same four lines; would change pixels as a
    thick line.
12. **Repeated calls: two primitives per transit dot** — `slipway_draw`
    (`src/game/slipway_display.c:162`). A 2×2 black `p8_rectfill` then a
    `p8_pset` for each of 29–44 dots a frame (217 + 134). One 2×2 sprite
    with the same pixels: bound 351 (23), less the sprite. Pixel risk:
    identical.
13. **Recomputation: planet visibility tested again by each slipway** —
    `slipway_draw` (`src/game/slipway_display.c:99`). 66 calls a frame to
    `entity_geom_is_on_screen` on end planets whose visibility
    `planet_draw` computes too. Bound 126 (8). Pixel risk: identical.
14. **`emitter_update` (under `ascension_draw` and its own method): no
    candidate that keeps pixels.** Every particle moves every frame; the
    per-particle matrix step (219) and one `p8_pset` each (293) are the
    work itself. The particle simulation runs in a draw method, which is
    why call-twice could not cost it; moving it to the update is not a
    "do less".

#### Verification, `draw-5564` levels 3 and below

Every node above 2% of `draw-5564`'s mean window frame below level 2:

| Node (path) | Cost / frame | Status | Candidates |
|---|---:|---|---|
| `button_draw` | 307 | explored | 2 |
| · `layout_row_draw_at_default` → `layout_row_draw_at` → `row_draw_at_item_draw` (1) | 238 / 237 / 240 | explored | 2, 7 |
| · `hud_print_outlined_default` → `hud_print_outlined` (1) | 224 / 224 | explored | 1 |
| · `p8_print` (1) | 224 | leaf | 1 |
| · `layout_row_draw_at_default` → `layout_row_draw_at` → `row_draw_at_item_draw` (2) | 47 / 47 / 51 | explored | 2, 7 |
| · `hud_print_outlined_default` → `hud_print_outlined` (2) | 40 / 40 | explored | 1 |
| · `p8_print` (2) | 41 | leaf | 1 |
| `background_draw` | 231 | explored | 5 |
| · `p8_map` | 221 | leaf | 5 |
| `planet_draw` | 146 | explored | 8, 9 |
| · `layout_row_draw_at_method` → `layout_row_draw_at` → `row_draw_at_item_draw` (3) | 51 / 51 / 55 | explored | 6, 7 |
| · `p8_spr_default` (3) | 37 | leaf | 6 |
| · `p8_circfill` | 36 | leaf | 8 |
| `slipway_draw` | 119 (PC-derived) | explored | 11, 12, 13 |
| · `slipway_draw_line` s = 3, s = 1 | 35, 36 | explored | 10, 11 |
| `score_panel_draw` → `draw_functions_draw_rows_default` → `draw_rows_dispatch_one` | 87 / 87 / 86 | explored | 4 |
| · `print_outlined_entry` → `hud_print_outlined_default` → `hud_print_outlined` (4) | 76 / 73 / 73 | explored | 1, 4 |
| · `p8_print` (4) | 73 | leaf | 1 |
| `ascension_draw` | 36 (PC-derived; ≥ 50 measured) | explored | 14 |
| · `emitter_update` (7) | ≥ 34 | explored | 14 |
| `message_box_draw_overlay` → `message_box_draw_article` | 268 / 268 | explored | 3 |
| · `layout_row_draw_at` → `row_draw_at_item_draw` (5) | 263 / 267 | explored | 3, 7 |
| · `hud_print_outlined_default` → `hud_print_outlined` (5) | 250 / 250 | explored | 1, 3 |
| · `p8_print` (5) | 248 | leaf | 1 |
| `button_draw_overlay` → `tooltip_draw_at` | 29 / 28 | explored | 2 (same rows), 7 |
| · `layout_row_draw_at` → `row_draw_at_item_draw` (6) | 28 / 33 | explored | 1, 7 |
| `emitter_update_method` (level 2, re-examined) | ≥ 16 measured | explored | 14 |
| the eight level-3 methods' `X_draw` links | as their method | explored | — (pass-through) |

No self below level 2 is above 2% among those measured; the unmeasured
ones are `emitter_update`'s particle-ageing loops and `slipway_draw`'s
dot-ageing loop (105 against a PC-derived node).

#### Patches (levels 3 and below, `draw-5564`)

Generated from `b490c78` by throwaway scripts (`gen2.py`, `pp.py`,
`l4.py` to `l8.py`, `count.py`, `l2emit.py` in `tmp/plan20-draw-tree/`,
git-ignored), each used once and discarded; none is committed. "Path K"
wraps the parent's call in `{ int probe_saved = probe_path; probe_path =
K; <call> probe_path = probe_saved; }`, with `int probe_path;` defined in
`src/game/hud.c` and declared `extern` where used; "under K" is `if
(probe_path == K) { <statement> }` right after the original. Labels are
`l3-<probe>`.

| Probes | Node(s) | Shape | Inserted statement |
|---|---|---|---|
| `button-draw-twice`, `background-draw-twice`, `planet-draw-twice`, `score-panel-draw-twice`, `message-box-draw-overlay-twice`, `button-draw-overlay-twice`, `slipway-draw-twice` (not used), `ascension-draw-twice` (not used) | `X_draw` in its method | site | the getters read into `probe_gfx`, `probe_mem`, then `X_draw(probe_gfx, probe_mem, self…)` twice |
| `slipway-draw-delay`, `ascension-draw-delay` | `slipway_draw`, `ascension_draw` | delay 5, 20 at site | `psx_probe_delay(d);` before the call |
| `group-method-getters` | getters of the eight methods | group | a `volatile` second read of both getters before each call |
| `bd-row-twice`, `bd-side-row-twice`, `bd-fill-twice`, `bd-top-twice`, `bd-side-fill-twice`, `bd-rect-twice` | `button_draw`'s children (`src/game/ui_button.c`) | site | the statement duplicated |
| `group-bd-small` | `button_draw`'s arithmetic and getters | group | a block recomputing them from `volatile` copies of `draw_pos` and `self` |
| `bg-map-twice`, `bg-vlines-twice`, `bg-hlines-twice`, `group-bg-small` | `background_draw`'s children | site / group | the statement(s) duplicated |
| `pd-circfill-twice`, `pd-row-twice`, `pd-spr-twice`, `pd-onscreen-twice`, `pd-circ-twice`, `pd-sprdef-twice`, `group-pd-small` (+556, split into `-a` +468, then `pd-circ`/`pd-sprdef`, and `-b`) | `planet_draw`'s children under its method | site | `if (probe_from_method) <statement>` after the original; `probe_from_method` set around the call in `planet_draw_method` |
| `sd-line1-twice`, `sd-line2-twice`, `sd-dot-rectfill-twice`, `sd-dot-pset-twice`, `sd-onscreen-twice`, `group-sd-small` | `slipway_draw`'s children (`src/game/slipway_display.c`) | site / group | the statement duplicated, or recomputed from `volatile` copies |
| `sd-spawn-delay` | `slipway_spawn_transit_dot` ×2 | delay 6 at site | `psx_probe_delay(6);` before each call |
| `group-sl1`, `group-sl2` (+547, +557: split), `sl1-line-twice`, `sl2-line-twice`, `group-sl1-rest`, `group-sl2-rest` | `slipway_draw_line`'s children, one site | site / group | under `if (s == p8_fixed_from_int(3))` (or 1): `p8_line` again, or the end points and line coordinates recomputed from `volatile` copies |
| `sp-rows-twice`, `mb-article-twice`, `bo-tooltip-twice`, `ad-planet-twice` | the single heavy child of `score_panel_draw`, `message_box_draw_overlay`, `button_draw_overlay`, `ascension_draw` | site | the statement duplicated |
| `ad-emitter-delay` | `emitter_update` under `ascension_draw` | delay 20 at site | `psx_probe_delay(20);` before the call |
| `group-l4-small` … `group-l8-small` | cross-parent groups (table above) | group | `volatile` recomputation or the statement under its paths |
| `p1-lrd-twice`, `p2-lrd-twice`, `p3-lrm-twice`, `p5-layout-twice`, `p6-layout-twice`, `p5-row-twice`, `p6-bar-twice`, `p4-dispatch-twice` | children of `layout_row_draw_at_default`, `layout_row_draw_at_method`, `message_box_draw_article`, `tooltip_draw_at`, `draw_functions_draw_rows_default` | site, path K | the call again under K |
| `p{1,2,3,5,6}-item-twice` | `row_draw_at_item_draw` in `layout_row_draw_at` | site, path K | `row_draw_at_item_draw(gfx, mem, &self->placed_items[i], p);` under K |
| `p{1,2,3,5,6}-print-twice`, `p{1,2,3,5,6}-spr-twice`, `p4-print-twice` | `hud_print_outlined_default`, `p8_spr_default` in `row_draw_at_item_draw`; the outlined print in `print_outlined_entry` | site, path K | the call again under K |
| `p4-print-outlined-entry-twice`, `p4-help-frame-entry-twice`, `p4-bar-entry-twice` | `fn(e)` in `draw_rows_dispatch_one` | site, path 4 | `fn(e);` under `probe_path == 4 && e->draw_function == <kind>` |
| `p{1,2,4,5}-hpo-twice`, `p{1,2,4,5}-p8print-twice` | `hud_print_outlined` in `hud_print_outlined_default`; `p8_print` in `hud_print_outlined`'s loop | site, path K | the call again under K |
| `p{1,2,5,6}-print-count`, `p{1,2,3,5}-spr-count` | per-path counts | delay 20, count only | `psx_probe_delay(20);` under K before the print or sprite |
| `p7-emitter-children` (+513: split), `p7-emitter-pset-twice`, `p7-emitter-arith`, `p7-emitter-spawn-delay`, `p8-emitter-method-children` | `emitter_update`'s children under `ascension_draw` (7) or its method (8) | site / group / delay 20 | `p8_pset` again, the per-particle arithmetic recomputed from `volatile` copies, or `psx_probe_delay(20);` before `emitter_spawn`, under K |

Probes with an update delta beyond their bound (all 3–11 hblanks over the
window): `bd-rect-twice` (+6), `bo-tooltip-twice` (+11),
`button-draw-overlay-twice` (+9), `p6-layout-twice` (+10),
`p6-print-twice` (+9), `group-pd-small-b` (+8), `pd-sprdef-twice` (+5),
`ad-planet-twice` (+5), `p7-emitter-arith` (+5), `group-sl1-rest` and
`group-sl2-rest` (+5), `group-sd-small` (+4), `sd-onscreen-twice` (+3),
`p1-spr-twice` (+3), `p5-spr-twice` (+3), `group-l8-small` (+3).

#### `draw-5564` hand-off

Below level 2, `draw-5564`'s window is held by four runtime leaves:
`p8_print` (586 a frame through the outlined print, 43.4% of the mean
window frame), `p8_map` (221), `p8_spr_default` (37 for the planet rows
alone) and `p8_circfill` (36). `p8_rectfill` is cheap on the PSX where PC
time made it look heavy: the message box's five full-width bars cost 50
over the window against 72% of the article on PC, and `button_draw`'s
rectangles 276 against 51% on PC. `draw-6000` and `start` follow, then
the verification across runs.

### Levels 3 and below, `draw-6000` and `start`

Every node opened at level 2 on `draw-6000` (`background_draw_method`,
`planet_draw_method`, `slipway_draw_method`, `score_panel_draw_method`,
`button_draw_method`, `ascension_draw_method`; `draw_layers#1`'s self
flagged) and on `start` (`background_draw_method`), down to a PICO-8
runtime leaf or a node under 2% (294 hblanks over `draw-6000`'s window,
19.6 a frame; 189 over `start`'s one window frame). Measured on commit
`976c13a` (same `src/` and `psx/` tree ids as the rebuilt baseline) with:

```
make frame-profile-subtree ROOT=<function> PSX_SHORT_RUNS="start draw-6000"
make psx-probe PATCH=tmp/plan20-draw-tree/<probe>.patch PSX_SHORT_RUNS=draw-6000 PSX_PROBE_LABEL=l3c5-<probe>
```

The nodes have the same structure as on `draw-5564`, so chunk 4's patches
were re-run unchanged on `draw-6000` (and `start` for the background);
every cost below is this run's own delta. 86 probe invocations (92 boots),
every one `valid`, every game state `same`; labels carry the `l3c5-`
prefix. Where a group crossed 2% here though it had not on `draw-5564`, it
was split by the same rule, in new patches: `group-sl1-rest` and
`group-sl2-rest` (+305 each) into the end points and the line coordinates,
`p7-emitter-arith` (+444) into the per-particle motion and colour,
`group-l7-small` (+367) into path 3 and the rest, then path 3 into
`vec_add` and the palette selects. `emitter_update_method`'s clean children
measure 447 on `draw-6000` (`p8-emitter-method-children`), above 2%, so it
is opened there too (below).

#### `start` (1 window frame, mean window frame 9451, 2% = 189.0)

| Node | Method | Calls / frame (PC) | Cost / frame | % of parent | % of mean window frame | Status |
|---|---|---|---:|---:|---:|---|
| `background_draw_method` | level 2 | 1 | 251 | — | 2.7% | explored |
| · `background_draw` | site call-twice | 1 | 251 | 100.0% | 2.7% | explored |
| · · `p8_map` | site call-twice | 1 | 241 | 96.0% | 2.6% | **leaf** |
| · · `p8_rectfill` (vertical grid) | site call-twice | 9 | 4 | 1.6% | 0.0% | leaf, under |
| · · `p8_rectfill` (horizontal grid) | site call-twice | 9 | 4 | 1.6% | 0.0% | leaf, under |
| · · `p8_rect`, `p8_fillp`, `p8_fillp_reset` | group `group-bg-small` | 1; 1; 1 | ≤ 2 over the window | | | ≤ group |
| · · self | difference | | 2 | 0.8% | 0.0% | under |
| · `dispatch_context_gfx`, `dispatch_context_mem` | group `group-method-getters` | 1 + 1 (other methods: 90, 4, 1 …) | ≤ 2 over the window | | | ≤ group |
| · self | difference | | 0 | 0.0% | 0.0% | under |

`start`'s tree stops there: rooted on a PC-derived `draw_layers#1` (463),
its only branch above 2% is the background, and the background is the map.

#### `draw-6000` (15 window frames, mean window frame 979, 2% = 19.6)

`slipway_draw_method` and `ascension_draw_method` keep their level-2
PC-derived figures as roots; PC calls are the subtree map's (for a shared
function, every parent's calls together; PSX counts where a path needs its
own).

| Node | Method | Calls / frame (PC; PSX) | Window | Cost / frame | % of parent | % of mean window frame | Status |
|---|---|---|---:|---:|---:|---:|---|
| `background_draw_method` | level 2 | 1 | 3503 | 234 | — | 23.9% | explored |
| · `background_draw` | site call-twice | 1 | 3508 | 234 | 100.1% | 23.9% | explored |
| · · `p8_map` | site call-twice | 1 | 3354 | 224 | 95.6% | 22.8% | **leaf** |
| · · `p8_rectfill` (vertical grid) | site call-twice | 9 | 67 | 4 | 1.9% | 0.5% | leaf, under |
| · · `p8_rectfill` (horizontal grid) | site call-twice | 9 | 64 | 4 | 1.8% | 0.4% | leaf, under |
| · · `p8_rect`, `p8_fillp`, `p8_fillp_reset` | group `group-bg-small` | 1; 1; 1 | ≤ 2 over the window | | | | ≤ group |
| · · self | difference | | 21 | 1 | 0.6% | 0.1% | under |
| · self (incl. getters) | difference | | −5 | 0 | −0.1% | 0.0% | under |
| `planet_draw_method` | level 2 | 72 | 3049 | 203 | — | 20.8% | explored |
| · `planet_draw` | site call-twice | 72 | 3035 | 202 | 99.5% | 20.7% | explored |
| · · `layout_row_draw_at_method` (`draw_at`) | site call-twice, method path | 15 | 1145 | 76 | 37.7% | 7.8% | explored |
| · · · `layout_row_draw_at` | site call-twice, path 3 | 15 | 1142 | 76 | 99.7% | 7.8% | explored |
| · · · · `row_draw_at_item_draw` | site call-twice, path 3 | 102–107 (all paths) | 1212 | 81 | 106.1% | 8.3% | explored |
| · · · · · `p8_spr_default` | site call-twice, path 3 | 73–74 (all paths); 64.18 | 820 | 55 | 67.7% | 5.6% | **leaf** |
| · · · · · palette selects (`hud_select_palette_block`, `_default`) | group `l7-p3-select` (split out of `group-l7-path3`, +332, out of `group-l7-small`, +367) | 64 + 64 | 257 | 17 | 21.2% | 1.7% | ≤ group |
| · · · · · `vec_add` | `l7-p3-vecadd` (split) | 102–107 (all paths) | 90 | 6 | 7.4% | 0.6% | under |
| · · · · · `hud_print_outlined_default` | site call-twice, path 3 | 0–3 (all paths) | 16 | 1 | 1.3% | 0.1% | under |
| · · · · · self (three `p8_fixed_flr_int`) | difference | | 29 | 2 | 2.4% | 0.2% | under |
| · · · · self | difference | | −70 | −5 | −6.1% | −0.5% | under (bias) |
| · · · self | difference | | 3 | 0 | 0.3% | 0.0% | under |
| · · `p8_circfill` (disc) | site call-twice, method path | 19 (both parents) | 758 | 51 | 25.0% | 5.2% | **leaf** |
| · · `p8_circ` (colony ring) | site call-twice, method path (split out of `group-pd-small`, +703, then `-a`, +574) | 8 | 472 | 31 | 15.6% | 3.2% | **leaf** |
| · · `p8_spr` (planet) | site call-twice, method path | 19 | 206 | 14 | 6.8% | 1.4% | leaf, under |
| · · `entity_geom_is_on_screen` | site call-twice, method path | 76 (both parents) | 150 | 10 | 4.9% | 1.0% | under |
| · · `p8_spr_default` (tier) | site call-twice, method path (split) | 8 | 104 | 7 | 3.4% | 0.7% | leaf, under |
| · · palette selects, `p8_palt` ×2, `planet_is_colonized`, ring colours, `main_ui`, `ui_pad_circle`, `entity_get`, `vec_*`, `p8_fixed_*` | group `group-pd-small-b` (split) | 19 each; 8 | 128 | 9 | 4.2% | 0.9% | ≤ group |
| · · self (incl. `entity_geom_shake`) | difference | 19 | 72 | 5 | 2.4% | 0.5% | under |
| · self | difference | | 14 | 1 | 0.5% | 0.1% | under |
| `slipway_draw_method` | level 2, **PC-derived** | 40 | 2434 | 162 | — | 16.6% | explored |
| · `slipway_draw` | delay 5, **PC-derived** (96.7%) | 40; 40.32 | 2354 | 157 | 96.7% | 16.0% | explored |
| · · `slipway_draw_line` (border, s = 3) | site call-twice | 22 | 755 | 50 | 32.1% | 5.1% | explored |
| · · · `p8_line` | site call-twice, s = 3 only | 88 | 503 | 34 | 66.6% | 3.4% | **leaf** |
| · · · end points (`slipway_start`/`end`, `slipway_end_pos`, `vec_from_polar` ×2, `vec_add`, `p8_fixed_div`, `p8_fixed_from_int`) | group `group-sl1-ends` (split out of `group-sl1-rest`, +305, out of `group-sl1`, +782) | 22 each | 183 | 12 | 24.2% | 1.2% | ≤ group |
| · · · line coordinates (`to_int` ×4, `p8_fixed_add` ×5) | group `group-sl1-coords` (split) | 88 each | 119 | 8 | 15.8% | 0.8% | ≤ group |
| · · · self | difference | | −50 | −3 | −6.6% | −0.3% | under (bias) |
| · · `slipway_draw_line` (line, s = 1) | site call-twice | 22 | 756 | 50 | 32.1% | 5.1% | explored |
| · · · `p8_line` | site call-twice, s = 1 only | 88 | 505 | 34 | 66.8% | 3.4% | **leaf** |
| · · · end points | group `group-sl2-ends` (split out of `group-sl2-rest`, +305, out of `group-sl2`, +785) | 22 each | 183 | 12 | 24.2% | 1.2% | ≤ group |
| · · · line coordinates | group `group-sl2-coords` (split) | 88 each | 119 | 8 | 15.7% | 0.8% | ≤ group |
| · · · self | difference | | −51 | −3 | −6.7% | −0.3% | under (bias) |
| · · `p8_rectfill` (transit dot) | site call-twice | 88–99 | 554 | 37 | 23.5% | 3.8% | **leaf** |
| · · `p8_pset` (transit dot) | site call-twice | 88–99 | 353 | 24 | 15.0% | 2.4% | **leaf** |
| · · `entity_geom_is_on_screen` ×2 | site call-twice | 40 + 24 | 123 | 8 | 5.2% | 0.8% | under |
| · · `slipway_spawn_transit_dot` ×2 | delay 6, **PC-derived** (1.0%) | 44; 44.24 | 24 | 2 | 1.0% | 0.2% | under |
| · · `to_int`, `vec_add`, `slipway_start`/`end`, `targeting_target`, `entity_get`, `int_seq_at`, `entity_has`, `particle_pixels_hidden`, `p8_fixed_*` | group `group-sd-small` | 330–363; … | 202 | 13 | 8.6% | 1.4% | ≤ group |
| · · `pointer_list_del`, `allocator_release`, `particle_zone` | bookkeeping, left in self | 0–5 | | | | | under |
| · · self (dot ageing loop) | difference with a PC-derived node | | −413 | −28 | −17.5% | −2.8% | see gaps |
| `score_panel_draw_method` | level 2 | 1 | 1296 | 86 | — | 8.8% | explored |
| · `score_panel_draw` | site call-twice | 1 | 1296 | 86 | 100.0% | 8.8% | explored |
| · · `draw_functions_draw_rows_default` | site call-twice | 1 | 1294 | 86 | 99.8% | 8.8% | explored |
| · · · `draw_rows_dispatch_one` | site call-twice, path 4 | 12 | 1293 | 86 | 99.9% | 8.8% | explored |
| · · · · `print_outlined_entry` (readouts) | site call-twice of `fn(e)`, path 4 | 7 | 1144 | 76 | 88.5% | 7.8% | explored |
| · · · · · `hud_print_outlined_default` | site call-twice, path 4 | 7 | 1103 | 74 | 96.4% | 7.5% | explored |
| · · · · · · `hud_print_outlined` | site call-twice, path 4 | 7 | 1092 | 73 | 99.0% | 7.4% | explored |
| · · · · · · · `p8_print` | site call-twice, path 4 | 63 | 1078 | 72 | 98.7% | 7.3% | **leaf** |
| · · · · · · · self (incl. `group-l8-small`) | difference | | 14 | 1 | 1.3% | 0.1% | under |
| · · · · · · self | difference | | 11 | 1 | 1.0% | 0.1% | under |
| · · · · · `number`, `is_number`, `draw_functions_arg`, `p8_fixed_flr_int` | in `group-l7-rest` | 7 each | | | | | ≤ group |
| · · · · · self (incl. `p8_tostr_fixed` + `frame_zone`, bookkeeping) | difference | 2 + 2 | 41 | 3 | 3.6% | 0.3% | under |
| · · · · `draw_help_frame_entry` (frames) | site call-twice of `fn(e)`, path 4 | 3 | 134 | 9 | 10.4% | 0.9% | under |
| · · · · `draw_bar_entry` (bars) | site call-twice of `fn(e)`, path 4 | 2 | 17 | 1 | 1.3% | 0.1% | under |
| · · · · self | difference | | −2 | 0 | −0.2% | 0.0% | under |
| · · `score_panel_frame_rows`, `score_panel_readouts` | in `group-l4-small` | 1; 1 | | | | | ≤ group |
| `button_draw_method` | level 2 | 4–6 | 897 | 60 | — | 6.1% | explored |
| · `button_draw` | site call-twice | 4–6 | 897 | 60 | 100.0% | 6.1% | explored |
| · · `layout_row_draw_at_default` (label row) | site call-twice | 4–6 | 607 | 40 | 67.7% | 4.1% | explored |
| · · · `layout_row_draw_at` | site call-twice, path 1 | 4–6 | 607 | 40 | 100.0% | 4.1% | explored |
| · · · · `row_draw_at_item_draw` | site call-twice, path 1 | 102–107 (all paths) | 649 | 43 | 106.9% | 4.4% | explored |
| · · · · · `hud_print_outlined_default` | site call-twice, path 1 | 0–3 (all paths); 1.61 | 511 | 34 | 78.7% | 3.5% | explored |
| · · · · · · `hud_print_outlined` | site call-twice, path 1 | 1.61 | 510 | 34 | 99.8% | 3.5% | explored |
| · · · · · · · `p8_print` | site call-twice, path 1 | 14.5 (9 × 1.61) | 509 | 34 | 99.8% | 3.5% | **leaf** |
| · · · · · `p8_spr_default` | site call-twice, path 1 | 4.83 | 70 | 5 | 10.8% | 0.5% | leaf, under |
| · · · · · self (incl. palette selects, `group-l7-rest`) | difference | | 68 | 5 | 10.5% | 0.5% | under |
| · · · · self | difference | | −42 | −3 | −6.9% | −0.3% | under (bias) |
| · · `layout_row_draw_at_default` (side row) | site call-twice | 0–1 | 124 | 8 | 13.8% | 0.8% | under |
| · · `p8_rect` | site call-twice | 4–6 | 55 | 4 | 6.1% | 0.4% | leaf, under |
| · · `p8_rectfill` (body) | site call-twice | 4–6 | 37 | 2 | 4.1% | 0.3% | leaf, under |
| · · `p8_rectfill` (top line) | site call-twice | 4–6 | 35 | 2 | 3.9% | 0.2% | leaf, under |
| · · `p8_rectfill` (side background) | site call-twice | 0–1 | 1 | 0 | 0.1% | 0.0% | leaf, under |
| · · `ui_to_int`, `main_ui`, `ui_pad_tab`, `vec_*`, `p8_fixed_*` | group `group-bd-small` | 16–27; 4–5; … | 17 | 1 | 1.9% | 0.1% | ≤ group |
| · · self | difference | | 21 | 1 | 2.3% | 0.1% | under |
| `ascension_draw_method` | level 2, **PC-derived** | 4 | 818 | 55 | — | 5.6% | explored |
| · `ascension_draw` | delay 20, **PC-derived** (98.7%) | 4; 4.01 | 807 | 54 | 98.7% | 5.5% | explored |
| · · `emitter_update` | children measured (delay 20, count 4.02; PC-derived 480, rejected) | 4; 4.02 | ≥ 911 | ≥ 61 | ≥ 112.9% | ≥ 6.2% | explored |
| · · · `p8_pset` | site call-twice, path 7 | 190–195 (both parents) | 469 | 31 | 58.1% | 3.2% | **leaf** |
| · · · motion (`emitter_of`, `emitter_host_layer_pos`, `entity_geom_is_on_screen`; per particle `vec_add`, `p8_fixed_mul` ×4, `p8_fixed_add` ×2, `p8_fixed_sub`, `p8_fixed_from_int`) | group `p7-emitter-motion` (split out of `p7-emitter-arith`, +444, out of `p7-emitter-children`, +907) | 194–198 each (both parents) | 288 | 19 | 35.7% | 2.0% (288 < 294) | ≤ group |
| · · · colour (`particle_to_int` ×3, `p8_fixed_add` ×2, `p8_fixed_flr`, `int_seq_at`) | group `p7-emitter-colour` (split) | 190–195 each | 154 | 10 | 19.1% | 1.0% | ≤ group |
| · · · `emitter_spawn` | delay 20 (PSX count only) | 4 (both parents); 3.01 | | | | | under (PC 0.8%) |
| · · · self (particle ageing loop) | not measurable | | | | | | open (PC 46.4%) |
| · · `planet_draw` | site call-twice | 4 | 276 | 18 | 34.2% | 1.9% | under |
| `emitter_update_method` | level 2 (PC-derived 214, under), re-opened | 1 | ≥ 458 | ≥ 31 | — | ≥ 3.1% | explored |
| · `emitter_update` → `p8_pset` | site call-twice, path 8 (split out of `p8-emitter-method-children`, +447) | 190–195 (both parents) | 255 | 17 | | 1.7% | leaf, under |
| · `emitter_update` → motion and colour | group `p8-emitter-arith` (split) | 194–198 each (both parents) | 203 | 14 | | 1.4% | ≤ group |
| · `emitter_update`'s self (particle ageing loop) | not measurable | | | | | | open (PC 46.4%) |

Cross-parent groups on `draw-6000` (all `valid`, game state `same`):

| Group | Window delta |
|---|---:|
| `group-method-getters` | 25 |
| `group-l4-small` | ≤ 2 over the window |
| `group-l5-small` | 17 |
| `group-l6-small` | 33 |
| `group-l7-small` (split: `group-l7-path3` 332, then `l7-p3-select` 257 and `l7-p3-vecadd` 90; `group-l7-rest` 55) | 367 |
| `group-l8-small` | 12 |

#### Children-sum gaps, `draw-6000` and `start`

| Node (path) | Node | Σ children | Gap |
|---|---:|---:|---:|
| `background_draw` (`start` / `draw-6000`) | 251 / 3508 | 249 / 3487 | −0.8% / −0.6% |
| `planet_draw` (method) | 3035 | 2963 | −2.4% |
| `layout_row_draw_at` (3) | 1142 | 1212 | +6.1% |
| `row_draw_at_item_draw` (3) | 1212 | 1183 | −2.4% |
| `slipway_draw` | 2354 PC-derived | 2767 | **+17.5%** |
| `slipway_draw_line` s = 3 / s = 1 | 755 / 756 | 805 / 807 | +6.6% / +6.7% |
| `draw_rows_dispatch_one` | 1293 | 1295 | +0.2% |
| `print_outlined_entry` | 1144 | 1103 | −3.6% |
| `hud_print_outlined_default` / `hud_print_outlined` (4) | 1103 / 1092 | 1092 / 1078 | −1.0% / −1.3% |
| `button_draw` | 897 | 876 | −2.3% |
| `layout_row_draw_at` (1) | 607 | 649 | +6.9% |
| `row_draw_at_item_draw` (1) | 649 | 581 (+ ≤ 55 in `group-l7-rest`) | −10.5% (−2.0% with the group) |
| `hud_print_outlined_default` / `hud_print_outlined` (1) | 511 / 510 | 510 / 509 | −0.2% / −0.2% |
| `ascension_draw` | 807 PC-derived | ≥ 1187 | **≥ +47.1%** |
| `emitter_update_method` | 214 PC-derived | ≥ 458 | **≥ +114%** |
| the pass-through methods and score panel links | | | 0.0% to −0.5% |

Gaps above 10%:

- **`slipway_draw`, +17.5%**: on `draw-6000` the PC-derived figure is too
  low, unlike `draw-5564` (−5.9%). Its clean children add up to 2767, more
  than the method's PC-derived 2434 and close to its unused call-twice
  (2668, −3.6%). The sum carries each child's second-call bias (+4–9% each,
  never corrected), so `slipway_draw_method`'s PSX cost is about 2540–2770
  (169–184 a frame); the dot ageing loop's own work is inside that range,
  not separable. PC proportions under-weigh the slipways here: 88 lines
  and 88–99 dots a frame cost the PSX more per call than the PC's share
  says.
- **`ascension_draw`, ≥ +47.1%**, and **`emitter_update_method`,
  ≥ +114%**: as on `draw-5564`, the PC-derived figures are too low; the
  measured children are lower bounds (the particle-ageing selves, 46.4% of
  `emitter_update` on PC, are not in them). Here the unused call-twice of
  `ascension_draw_method` (860) is even below its children (1187): its
  second call ages every particle again, so particles die twice as fast and
  both calls draw fewer of them. The measured lower bounds are used:
  `ascension_draw_method` ≥ 1187 (79 a frame), `emitter_update_method`
  ≥ 458 (31 a frame), both above 2%.
- **`row_draw_at_item_draw` (1), −10.5%**: the item's own work (palette
  selects, `vec_add`, three `p8_fixed_flr_int`), bounded by
  `group-l7-rest` (55); within 10% with it.

#### `draw_layers#1`'s self on `draw-6000`

With the measured lower bounds in place of the three PC-derived terms
(`slipway_draw_method` 2767, `ascension_draw_method` 1187,
`emitter_update_method` 458), Σ of `draw_layers#1`'s children is 13462
against 13642: gap −1.3%, and the self is 180 over the window (12 a frame),
under the 294 threshold. By the rule the flag of level 2 is withdrawn on
`draw-6000`. It stays fragile: the children's uncorrected second-call bias
(+4–9% of about 13000) and PC's own figure for the self (458, 31 a frame)
put it plausibly above 2%, while the unmeasured particle-ageing selves push
it lower. `draw_layers` candidates 1–2 keep their level-2 bound (≤ 95 a
frame).

#### Where the known items sit, `draw-6000`

- **The 9× outlined print** is reached through two paths above 2%: score
  panel readouts (63 `p8_print` a frame, 1078) and button labels (14.5,
  509); the planet rows print almost nothing (16) and no message box is
  open. 1587 over the window, 106 a frame: **10.8% of the mean window
  frame**, against 43.4% on `draw-5564`.
- **The draw meter** sits inside every runtime leaf; not separable here
  (step 8).

#### "Do less" candidates, `draw-6000` and `start`

The fourteen candidates of `draw-5564` hold on `draw-6000` with the same
code and evidence; their bounds on this run, over the window (per frame),
from the measured costs above:

| `draw-5564` candidate | Where it applies on `draw-6000` | Bound on `draw-6000` | Pixel risk |
|---|---|---:|---|
| 1. Nine `p8_print` per outlined text | readouts (4), button labels (1) | 8/9 of the leaves: 958 (64) + 452 (30) = 1410 (94) | identical only with an equivalent outline |
| 2. Button rows redrawn every frame | label rows, side rows | 607 + 124 = 731 (49), less a copy | identical |
| 3. Open message box redrawn | no message box open | — | — |
| 4. Score panel readouts redrawn and reformatted | `print_outlined_entry` | 1144 (76) redraw; ≤ 41 (3) formatting | identical |
| 5. Background map redrawn | `p8_map` | 3354 (224); on `start` 241 (241, one frame) | identical |
| 6. Planet row icons redrawn | `row_draw_at_item_draw` (3), 64.18 sprites a frame | 1212 (81) | identical |
| 7. Palette block selected around every row sprite | paths 3 and 1 | ≤ 257 + 55 = 312 (21) | identical |
| 8. Disc and ring behind each planet | `p8_circfill` 19 a frame, `p8_circ` 8 | 758 + 472 = 1230 (82), less larger sprites | identical only if baked exactly |
| 9. Off-screen planets still tested | `entity_geom_is_on_screen` under `planet_draw` | 150 (10) | identical |
| 10. Slipway end points computed twice | the two end-point groups | one of them, 183 (12) | identical |
| 11. Four lines per slipway pass | `p8_line`, 176 a frame | 3/4 of 1008 = 756 (50) | identical if batched; would change pixels as a thick line |
| 12. Two primitives per transit dot | `p8_rectfill` + `p8_pset`, 88–99 dots a frame | 554 + 353 = 907 (60), less the sprite | identical |
| 13. Planet visibility retested by each slipway | 64 calls a frame | 123 (8) | identical |
| 14. `emitter_update`: no candidate that keeps pixels on the simulation | under `ascension_draw` and its method | — | — |

One candidate is new, from the particle leaf now above 2%:

15. **Repeated calls: one runtime call per particle** — `emitter_update`
    (`src/game/particle.c:190`). Each live particle is one `p8_pset` call,
    190–195 a frame over five emitters (469 under `ascension_draw`, 255
    under `emitter_update_method`, about 0.25 hblank a call), each going through
    the runtime's clip, palette and draw-state checks for one pixel. A point
    list handed to the runtime once per emitter keeps the pixels and saves
    the per-call part, which only step 8's per-unit cost can size: bound
    ≤ 724 (48) on `draw-6000`; on `draw-5564` 293 (20) under
    `ascension_draw` plus at most 241 (16) under the method; the whole leaf.
    Pixel risk: identical. The transit dots' `p8_pset` (candidate 12) is
    the same case.

#### Verification across runs

Every node above 2% of its run's mean window frame, on any run, with its
cost a frame per run (— where it is not above 2% or not run; PC-derived
roots marked), its status and its candidates (`DL` = the `draw_layers`
candidates of level 2, numbers = the "do less" lists below level 2):

| Node (path) | `start` | `draw-5564` | `draw-6000` | Status | Candidates |
|---|---:|---:|---:|---|---|
| `draw_layers#1` | 463 (PC-derived) | 1082 | 909 | explored | DL 1–4 |
| `draw_layers#1` self | — (17) | ≤ 105, flagged | — (12, flag withdrawn) | explored (self) | DL 1–2 |
| `draw_layers#2` | — | 312 | — | explored | DL 1–2 |
| `background_draw_method` → `background_draw` | 251 / 251 | 231 / 231 | 234 / 234 | explored | 5 |
| · `p8_map` | 241 | 221 | 224 | leaf | 5 |
| `planet_draw_method` → `planet_draw` | — | 147 / 146 | 203 / 202 | explored | 8, 9 |
| · `layout_row_draw_at_method` → `layout_row_draw_at` → `row_draw_at_item_draw` (3) | — | 51 / 51 / 55 | 76 / 76 / 81 | explored | 6, 7 |
| · · `p8_spr_default` (3) | — | 37 | 55 | leaf | 6 |
| · `p8_circfill` | — | 36 | 51 | leaf | 8 |
| · `p8_circ` | — | — | 31 | leaf | 8 |
| `slipway_draw_method` → `slipway_draw` | — | 125 / 119 (PC-derived) | 162 / 157 (PC-derived; ≥ 184 measured) | explored | 11, 12, 13 |
| · `slipway_draw_line` s = 3, s = 1 | — | 35, 36 | 50, 50 | explored | 10, 11 |
| · · `p8_line` s = 3, s = 1 | — | — | 34, 34 | leaf | 11 |
| · `p8_rectfill`, `p8_pset` (transit dots) | — | — | 37, 24 | leaf | 12, 15 |
| `score_panel_draw_method` → `score_panel_draw` → `draw_functions_draw_rows_default` → `draw_rows_dispatch_one` | — | 87 / 87 / 87 / 86 | 86 / 86 / 86 / 86 | explored | 4 |
| · `print_outlined_entry` → `hud_print_outlined_default` → `hud_print_outlined` (4) | — | 76 / 73 / 73 | 76 / 74 / 73 | explored | 1, 4 |
| · · `p8_print` (4) | — | 73 | 72 | leaf | 1 |
| `button_draw_method` → `button_draw` | — | 307 / 307 | 60 / 60 | explored | 2 |
| · `layout_row_draw_at_default` → `layout_row_draw_at` → `row_draw_at_item_draw` (1) | — | 238 / 237 / 240 | 40 / 40 / 43 | explored | 2, 7 |
| · · `hud_print_outlined_default` → `hud_print_outlined` (1) | — | 224 / 224 | 34 / 34 | explored | 1 |
| · · · `p8_print` (1) | — | 224 | 34 | leaf | 1 |
| · `layout_row_draw_at_default` → `layout_row_draw_at` → `row_draw_at_item_draw` (2) | — | 47 / 47 / 51 | — | explored | 2, 7 |
| · · `hud_print_outlined_default` → `hud_print_outlined` (2) | — | 40 / 40 | — | explored | 1 |
| · · · `p8_print` (2) | — | 41 | — | leaf | 1 |
| `ascension_draw_method` → `ascension_draw` | — | 37 / 36 (PC-derived; ≥ 50 measured) | 55 / 54 (PC-derived; ≥ 79 measured) | explored | 14, 15 |
| · `emitter_update` (7) | — | ≥ 34 | ≥ 61 | explored | 14, 15 |
| · · `p8_pset` (7) | — | — | 31 | leaf | 15 |
| `emitter_update_method` (closed at level 2 on PC-derived 12 / 14 a frame) | — | ≥ 16 measured, re-examined | ≥ 31 measured | explored | 14, 15 |
| `message_box_draw_overlay_method` → `message_box_draw_overlay` → `message_box_draw_article` | — | 268 / 268 / 268 | — | explored | 3 |
| · `layout_row_draw_at` → `row_draw_at_item_draw` (5) | — | 263 / 267 | — | explored | 3, 7 |
| · · `hud_print_outlined_default` → `hud_print_outlined` (5) | — | 250 / 250 | — | explored | 1, 3 |
| · · · `p8_print` (5) | — | 248 | — | leaf | 1 |
| `button_draw_overlay_method` → `button_draw_overlay` → `tooltip_draw_at` | — | 29 / 29 / 28 | — | explored | 2, 7 |
| · `layout_row_draw_at` → `row_draw_at_item_draw` (6) | — | 28 / 33 | — | explored | 1, 7 |
| the level-3 methods' `X_draw` links | as their method | as their method | as their method | explored | — (pass-through) |

Every node above 2% on every run is explored or a leaf. What stays open is
not a node but three selves no clean probe reaches, all inside explored
nodes: `emitter_update`'s particle-ageing loop (PC 46.4% of
`emitter_update`), `slipway_draw`'s dot-ageing loop, and, by extension,
`draw_layers#1`'s self on `draw-5564`, which holds the former.

#### Patches (`draw-6000` and `start`)

Chunk 4's patches (its table above) re-run unchanged with
`PSX_PROBE_LABEL=l3c5-<probe>`: on `start` and `draw-6000`,
`background-draw-twice`, `bg-map-twice`, `bg-vlines-twice`,
`bg-hlines-twice`, `group-bg-small`, `group-method-getters`; on
`draw-6000`, `planet-draw-twice`, `pd-*-twice`, `group-pd-small`,
`group-pd-small-a`, `group-pd-small-b`, `p3-lrm-twice`, `p3-item-twice`,
`p3-spr-twice`, `p3-print-twice`, `p3-spr-count`, `slipway-draw-delay`,
`sd-*-twice`, `sd-spawn-delay`, `group-sd-small`, `group-sl1`,
`group-sl2`, `sl1-line-twice`, `sl2-line-twice`, `group-sl1-rest`,
`group-sl2-rest`, `score-panel-draw-twice`, `sp-rows-twice`,
`p4-dispatch-twice`, `p4-print-outlined-entry-twice`,
`p4-help-frame-entry-twice`, `p4-bar-entry-twice`, `p4-print-twice`,
`p4-hpo-twice`, `p4-p8print-twice`, `button-draw-twice`, `bd-*-twice`,
`group-bd-small`, `p1-lrd-twice`, `p1-item-twice`, `p1-print-twice`,
`p1-hpo-twice`, `p1-p8print-twice`, `p1-spr-twice`, `p1-print-count`,
`p1-spr-count`, `p2-lrd-twice`, `ascension-draw-delay`, `ad-planet-twice`,
`ad-emitter-delay`, `p7-emitter-children`, `p7-emitter-pset-twice`,
`p7-emitter-arith`, `p7-emitter-spawn-delay`,
`p8-emitter-method-children`, `group-l4-small` to `group-l8-small`.

New patches, generated from `976c13a` by throwaway scripts (`l7b.py` and
inline hunk splits in `tmp/plan20-draw-tree/`, git-ignored), used once and
discarded; none is committed:

| Probe label | Node(s) | Shape | Inserted statement |
|---|---|---|---|
| `l3c5-group-sl1-ends`, `l3c5-group-sl2-ends` | `slipway_draw_line`'s end points, one site | group | under `if (s == p8_fixed_from_int(3))` (or 1), before the loop: both end points recomputed from `volatile` copies of `self` and `t` |
| `l3c5-group-sl1-coords`, `l3c5-group-sl2-coords` | `slipway_draw_line`'s line coordinates, one site | group | under the same test, after `p8_line`: the four `to_int(p8_fixed_add(…))` and the step recomputed from `volatile` copies |
| `l3c5-p7-emitter-motion` | `emitter_update`'s per-emitter calls and per-particle motion, path 7 | group | under `probe_path == 7`: `emitter_of`, `emitter_host_layer_pos`, `entity_geom_is_on_screen`, then per particle `vec_add`, the matrix step and the life step on `volatile` copies |
| `l3c5-p7-emitter-colour` | `emitter_update`'s per-particle colour, path 7 | group | under `probe_path == 7`: `px`, `py`, the colour index and `int_seq_at` recomputed from `volatile` copies |
| `l3c5-p8-emitter-pset-twice` | `p8_pset` in `emitter_update`, path 8 | site | `p8_pset(graphics, memory, px, py, col);` under `probe_path == 8`; path 8 set around `emitter_update_method`'s call |
| `l3c5-p8-emitter-arith` | motion and colour, path 8 | group | `p8-emitter-method-children` without the doubled `p8_pset` |
| `l3c5-group-l7-path3` | `row_draw_at_item_draw`'s `vec_add` and palette selects, path 3 | group | `group-l7-small`'s layout statements under `probe_path == 3` only |
| `l3c5-group-l7-rest` | the same under paths 1, 2, 5, 6, and `print_outlined_entry`'s arithmetic (4) | group | `group-l7-small` with `probe_path != 0 && probe_path != 3 && probe_path != 4` in the layout |
| `l3c5-l7-p3-vecadd`, `l3c5-l7-p3-select` | `vec_add`; the two palette selects, path 3 | group (split) | the corresponding statements of `group-l7-path3` |

Probes with an update delta beyond their bound, all 3–9 hblanks over the
window (under 1 a frame): `bd-rect-twice` (+5), `bd-side-fill-twice`
(+6), `bg-vlines-twice` (+6), `bg-hlines-twice` (+3), `group-bd-small`
(+5), `group-l4-small` (+3), `group-l6-small` (+3), `group-l7-rest` (+8),
`group-l8-small` (+3), `group-method-getters` (+9), `group-pd-small-b`
(+4), `group-sl1-ends`, `group-sl2-ends`, `group-sl1-coords`,
`group-sl2-coords` (+4 each), `l7-p3-vecadd` (+4), `p2-lrd-twice` (+3),
`p4-bar-entry-twice` (+3), `p7-emitter-colour` (+4), `pd-spr-twice` (+7),
`pd-sprdef-twice` (+6), `sd-onscreen-twice` (+5), all on `draw-6000`.

#### Hand-off to step 8

Step 7 ends with every branch above 2% explored or a leaf on `start`,
`draw-5564` and `draw-6000`. The leaves step 8 costs per unit, heaviest
first, per frame of the window:

| Runtime family | `start` | `draw-5564` | `draw-6000` |
|---|---:|---:|---:|
| `p8_print` (through `hud_print_outlined`) | — | 586 (289 calls) | 106 (78 calls) |
| `p8_map` | 241 | 221 | 224 |
| `p8_circfill` + `p8_circ` | — | 36 (+ 26 under) | 82 |
| `p8_pset` (particles and transit dots) | — | 20 + 9, each under (+ the method's particles, ≤ 16) | 72 |
| `p8_line` | — | 48 (two sites, 24 each, under) | 67 |
| `p8_spr_default` (row icons) | — | 37 (+ 14 under) | 55 (+ 5 under) |
| `p8_rectfill` (transit dots) | — | 14 (under) | 37 |

`p8_print` holds `draw-5564` (the message box and the button labels), the
geometry primitives hold `draw-6000` (map, discs, rings, lines, points),
and `start` is the map alone. `p8_rectfill` and `p8_rect` stay cheap on
the PSX wherever PC time made them look heavy (`button_draw`'s rectangles,
the message box bars, the score panel's help frames: 134 against 39.8% of
`draw_rows_dispatch_one` on PC).

## Backend: per-unit costs

Plan 20 step 8: what one call, and one glyph or primitive, of each draw
family costs inside the backend, and what the two backend-side levers (the
9× outlined print, the shipped draw meter) are worth: the per-unit tables,
the one-print and no-meter experiments, the backend and meter "do less"
candidates, the draw-filter finding and the verification table.

Measured on commit `17c2602dab38e4e369c54e579c6b9c96f2e9bbb1` (pcsx-redux,
asserts off) with:

```
make psx-probe-baseline
make psx-short-runs PSX_SHORT_RUN_ASSERTS=off PSX_SHORT_RUN_REFERENCE=
make frame-profile-unit-cost
```

Method: time and calls per family come from each run's breakdown boot
(`psx-gpu-replay-breakdown`, window frames only, fine-clock ticks converted
to hblanks with the boot's calibration line); units come from the PC frame
profile of the same `run.json`, counted with the draw meter's cost model
(`p8_draw_cost_*`). `make frame-profile-unit-cost` joins the two per window
frame and refuses a frame where PC and PSX calls differ: it accepted all six
runs, and the PC replay reached the same game on each (`same-game yes`).

- **Unit, per family:** `cls` 1 fill; `pset` 1 pixel; `line` 1 line;
  `rect` 4 tiles; `rectfill` 1 tile or patterned sprite; `circ` 8 ×
  midpoint iterations; `circfill` 4 × midpoint iterations (spans); `spr` 1
  sprite; `print` 1 glyph (non-space character); `pattern` the spans of
  patterned fills, inside `rectfill` and `circfill` and never added to them.
  A call off screen is 1 call and 0 units. A **switch** is the
  texture-window pair (`DR_TPAGE` + `DR_TWIN`) the model charges when a call
  changes window; switches are counted, not priced, and their time is inside
  the family that causes them.
- **Per call and per unit are two divisions of one total**, never to be
  added. Per unit is the family's whole time over its units, so off-screen
  calls (time, no unit) push it up: on `spr`, only a third of the calls are
  on screen (below).
- **Units are an upper bound** of what the GPU backend emits (it clips per
  glyph, span and rect edge; the model counts whole on-screen calls), so
  the per-unit figures are slight lower bounds.
- **Timer bias:** the breakdown boot's in-call time includes the timer
  wrapper's own per-call work. `cls` (one fill packet, the cheapest call)
  is the floor reference at 0.186 per call; no correction is applied. The
  breakdown-vs-plain draw overhead is 0.129 to 0.133 hblanks per backend
  call on every run (table below), split between in-call and outside-call
  time.
- **Wrappers inside every figure:** breakdown timer (breakdown boot only),
  draw meter, draw filter (not in `psx-game`), GPU backend.

Pre-checks (H-7-1 to H-7-8), all held:

- H-7-1, H-7-6: "Draw tree" is committed (`6aad8ab`, plan note `b6fd2f8`)
  and was the last section; this one follows it.
- H-7-2: step 7's tooling is the subtree root (`e97bb51`, `93c1064`); this
  step's tooling (`1e4015a`, `17c2602`) appends `units <u> switches <s>` to
  the family lines and adds the `unit-cost` mode and target.
- H-7-3, H-7-4: step 7's hand-off locates `p8_print` through
  `hud_print_outlined` (289 calls, 586 hblanks a frame on `draw-5564`; 78
  and 106 on `draw-6000`) and names the leaves and their parents.
- H-7-5: since `93c1064`, the only commit under `src/` or `psx/` is
  `1e4015a` (`src/cmd/frame_profile.c`, `src/devtools/call_profile.*`), none
  compiled by `psx/CMakeLists.txt`; the probe tool and the six-run manifest
  are unchanged.
- H-7-7: step 7 recorded no per-unit cost and ran no backend experiment.
- H-7-8: `psx/probes/` still holds only the three sanity fixtures
  (`bb5f9be`).

### Rebuilt probe baseline and short runs

Window sums in hblanks. Every probe boot `valid`, `game-state same`, text
size equal to step 7's. Short-run verdicts: `done`, `deterministic`,
`breakdown` and `asserts` ok on all six runs, `representative` skipped (no
reference, on purpose).

| Run | Probe update | Probe draw | Probe submit | Probe total | Step 7 total | Gap | Short-run update | Short-run draw | Short-run submit | Short-run total | Step 2/4 total | Gap |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| `start` | 8980 | 465 | 6 | 9451 | 9451 | 0 | 8980 | 464 | 7 | 9451 | 9451 | 0 |
| `update-4737` | 118 | 706 | 8 | 832 | 832 | 0 | 118 | 706 | 8 | 832 | 832 | 0 |
| `update-5500` | 1409 | 714 | 7 | 2130 | 2130 | 0 | 1409 | 714 | 7 | 2130 | 2130 | 0 |
| `draw-5564` | 655 | 19491 | 118 | 20264 | 20264 | 0 | 657 | 19489 | 118 | 20264 | 20264 | 0 |
| `update-5719` | 46 | 1078 | 7 | 1131 | 1131 | 0 | 46 | 1078 | 8 | 1132 | 1132 | 0 |
| `draw-6000` | 593 | 13987 | 107 | 14687 | 14687 | 0 | 593 | 13987 | 107 | 14687 | 14687 | 0 |

Every part equals the previous figures exactly, as expected from a tooling
commit that changes nothing the PSX build compiles.

`draw-6000`'s breakdown boot reached the default ceiling of 540 vsyncs
after its `done` line (476 vsyncs) and before exiting, cutting the end of
its state dump; its 123 breakdown records were complete and the verdicts
ok. It was re-run alone with `PSX_SHORT_RUN_CEILING=720
PSX_SHORT_RUN_TIMEOUT=20`: exit after 542 vsyncs, `analysis/summary.txt`
identical to the first boot's.

### Breakdown boot per run

| Run | Window frames | Backend calls | In-calls | Outside-calls | Breakdown draw | Plain draw | Overhead | Overhead per call |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| `start` | 1 | 549 | 422 (78.6%) | 115 | 537 | 464 | 73 | 0.133 |
| `update-4737` | 1 | 833 | 583 (71.5%) | 232 | 815 | 706 | 109 | 0.131 |
| `update-5500` | 1 | 856 | 577 (69.8%) | 249 | 826 | 714 | 112 | 0.131 |
| `draw-5564` | 15 | 17769 | 17210 (79.0%) | 4575 | 21785 | 19489 | 2296 | 0.129 |
| `update-5719` | 1 | 1072 | 926 (76.0%) | 293 | 1219 | 1078 | 141 | 0.132 |
| `draw-6000` | 15 | 18469 | 11252 (68.8%) | 5114 | 16365 | 13987 | 2378 | 0.129 |

Backend calls are the nine families' calls (`pattern` is inside its
shapes). Percentages are of the breakdown draw.

### Per-unit costs, `draw-5564` (15 window frames)

Hblanks summed over the window; "per frame" is hblanks ÷ 15.

| Family | Calls | Units | Switches | Hblanks | Per frame | Per call | Per unit | Note |
|---|---:|---:|---:|---:|---:|---:|---:|---|
| `cls` | 15 | 15 | 0 | 2.8 | 0.2 | 0.185 | 0.185 | low count; floor |
| `pset` | 2083 | 1689 | 0 | 607.7 | 40.5 | 0.292 | 0.360 | |
| `line` | 1800 | 1732 | 0 | 741.4 | 49.4 | 0.412 | 0.428 | |
| `rect` | 225 | 900 | 0 | 143.7 | 9.6 | 0.639 | 0.160 | |
| `rectfill` | 1383 | 1075 | 20 | 647.1 | 43.1 | 0.468 | 0.602 | |
| `circ` | 90 | 2880 | 0 | 388.0 | 25.9 | 4.311 | 0.135 | |
| `circfill` | 195 | 3600 | 0 | 648.2 | 43.2 | 3.324 | 0.180 | |
| `spr` | 7541 | 2471 | 15 | 4813.4 | 320.9 | 0.638 | 1.948 | 32.8% of calls on screen |
| `print` | 4437 | 28215 | 5 | 9218.1 | 614.5 | 2.078 | 0.327 per glyph | 6.4 glyphs a call |
| `pattern` | 275 | 125 | 20 | 166.9 | 11.1 | 0.607 | 1.335 | inside its shapes |

### Per-unit costs, `draw-6000` (15 window frames)

| Family | Calls | Units | Switches | Hblanks | Per frame | Per call | Per unit | Note |
|---|---:|---:|---:|---:|---:|---:|---:|---|
| `cls` | 15 | 15 | 0 | 2.8 | 0.2 | 0.186 | 0.186 | low count; floor |
| `pset` | 4327 | 2411 | 0 | 1177.9 | 78.5 | 0.272 | 0.489 | 55.7% of calls on screen |
| `line` | 2640 | 2196 | 0 | 1030.9 | 68.7 | 0.390 | 0.469 | |
| `rect` | 144 | 576 | 0 | 92.9 | 6.2 | 0.645 | 0.161 | |
| `rectfill` | 1959 | 1299 | 15 | 857.2 | 57.1 | 0.438 | 0.660 | |
| `circ` | 123 | 3984 | 0 | 495.8 | 33.1 | 4.031 | 0.124 | |
| `circfill` | 285 | 4800 | 0 | 862.7 | 57.5 | 3.027 | 0.180 | |
| `spr` | 7707 | 2652 | 15 | 4991.3 | 332.8 | 0.648 | 1.882 | 34.4% of calls on screen |
| `print` | 1269 | 4779 | 0 | 1740.0 | 116.0 | 1.371 | 0.364 per glyph | 3.8 glyphs a call |
| `pattern` | 270 | 135 | 15 | 160.6 | 10.7 | 0.595 | 1.189 | inside its shapes |

### Per-unit costs, one-frame runs

Per call / per unit; "low" marks a family under 30 window calls on that run
(its ratio is a few hblanks over a few calls).

| Family | `start` | `update-4737` | `update-5500` | `update-5719` |
|---|---|---|---|---|
| `cls` | 0.186 / 0.186 (1, low) | 0.186 / 0.186 (1, low) | 0.186 / 0.186 (1, low) | 0.186 / 0.186 (1, low) |
| `pset` | — (0) | 0.301 / 0.332 (32) | 0.277 / 0.416 (51) | 0.277 / 0.438 (68) |
| `line` | — (0) | 0.420 / 0.420 (160) | 0.406 / 0.451 (160) | 0.407 / 0.446 (184) |
| `rect` | 0.630 / 0.158 (8, low) | 0.630 / 0.158 (8, low) | 0.630 / 0.158 (8, low) | 0.630 / 0.157 (8, low) |
| `rectfill` | 0.563 / 0.793 (31) | 0.498 / 0.668 (47) | 0.487 / 0.715 (47) | 0.463 / 0.613 (86) |
| `circ` | — (0) | 3.984 / 0.140 (9, low) | 3.398 / 0.142 (8, low) | 3.617 / 0.126 (10, low) |
| `circfill` | 3.046 / 0.185 (17, low) | 3.035 / 0.175 (15, low) | 3.181 / 0.188 (13, low) | 3.018 / 0.174 (15, low) |
| `spr` | 0.645 / 1.796 (429) | 0.646 / 1.883 (498) | 0.640 / 1.946 (496) | 0.730 / 1.292 (520) |
| `print` | 1.124 / 0.393 (63) | 1.176 / 0.392 (63) | 1.163 / 0.388 (72) | 1.811 / 0.335 (180) |
| `pattern` | 0.642 / 1.285 (18, low) | 0.595 / 1.190 (18, low) | 0.577 / 1.297 (18, low) | 0.595 / 1.190 (18, low) |

### Per-unit costs, pooled over the six runs (34 window frames)

From `tmp/frame-profile/unit-cost/pooled.txt` (sums of the six runs, then
the ratios).

| Family | Calls | Units | Switches | Hblanks | Per call | Per unit | Runs' per-call range |
|---|---:|---:|---:|---:|---:|---:|---|
| `cls` | 34 | 34 | 0 | 6.4 | 0.188 | 0.188 | 0.185–0.186 |
| `pset` | 6561 | 4206 | 0 | 1828.1 | 0.279 | 0.435 | 0.272–0.301 |
| `line` | 4944 | 4400 | 0 | 1979.4 | 0.400 | 0.450 | 0.390–0.420 |
| `rect` | 401 | 1604 | 0 | 256.6 | 0.640 | 0.160 | 0.630–0.645 |
| `rectfill` | 3553 | 2528 | 39 | 1607.8 | 0.453 | 0.636 | 0.438–0.563 |
| `circ` | 240 | 7600 | 0 | 983.1 | 4.096 | 0.129 | 3.398–4.311 |
| `circfill` | 540 | 9420 | 0 | 1694.9 | 3.139 | 0.180 | 3.018–3.324 |
| `spr` | 17191 | 5905 | 34 | 11100.3 | 0.646 | 1.880 | 0.638–0.730 |
| `print` | 6084 | 34551 | 5 | 11512.7 | 1.892 | 0.333 per glyph | 1.124–2.078 |
| `pattern` | 617 | 295 | 39 | 370.9 | 0.601 | 1.257 (inside its shapes) | 0.577–0.642 |

In-calls pooled: 30969.5 hblanks.

### Reading the tables

- **`print` is priced by the glyph.** Per glyph is steady (0.327 to 0.393
  over the six runs); per call follows the text length (6.4 glyphs a call
  on `draw-5564`, 3.8 on `draw-6000`, 2.9 on the title screen). A lever
  that removes whole calls is priced with the run's own per-call figure, a
  lever that removes glyphs with the per-glyph one; never both.
- **`spr` is priced by the call.** Per call is flat (0.64, 0.73 on
  `update-5719`), while on the two draw runs two thirds of the calls draw
  nothing on screen (43% on `update-5719`): per sprite drawn is about 1.9 because the off-screen calls' time lands on
  the third that is drawn. `p8_map` (`src/p8/gfx.c:133`) issues one
  `p8_spr` per non-empty cell of the whole 32×32 map
  (`src/game/background.c:143`), wherever the camera is, which is the
  likely source of the off-screen calls; step 7 measured `p8_map` at 221
  hblanks a frame on `draw-5564`, 224 on `draw-6000`.
- **`pset` and `line`** cost 0.27–0.30 and 0.39–0.42 per call; `pset`'s
  off-screen calls (44% on `draw-6000`) raise its per-pixel figure the same
  way.
- **`circ` and `circfill`** are the dearest calls (about 4.1 and 3.1), in
  line with their many spans; per model unit they are the cheapest (0.13,
  0.18).
- **`rect` and `rectfill`** are cheap per call (0.64, 0.45) and per tile
  (0.16, 0.64), which confirms step 7's finding that their PC weight does
  not carry over to the PSX.
- **Switches** are rare (34 on `spr`, 39 on patterned fills, 5 on `print`
  over 34 window frames): texture-window changes are not a lever here.
- **Floor:** `cls` at 0.186 a call is the cheapest call measured, with the
  timer's per-call work inside it; every family's per-call figure reads high
  by part of that floor and of the 0.13-per-call breakdown overhead.

### Experiments

Both run with the probe tool on all six runs, on `cbd0c4f` (same `src/` and
`psx/` as the measured commit: the probe baseline's fingerprint held):

```
make psx-probe PATCH=tmp/plan20-backend/b-one-print.patch PSX_PROBE_LABEL=b-one-print
make psx-probe PATCH=tmp/plan20-backend/b-no-meter.patch PSX_PROBE_LABEL=b-no-meter
```

Every boot `valid`, `game-state same` on every run. Deltas are window sums
in hblanks (per frame on the 15-frame runs); update and submit moved by
−2 to +7 on every run, layout noise of the rebuilt binary, and are left out
of the tables below.

#### one-print: `hud_print_outlined` draws only its last pass

Patch: in `hud_print_outlined` (`src/game/hud.c:85`), the loop over the
nine outline offsets starts at 8, so only the text pass (offset 0,0, colour
`15 & c`) is drawn. Text −156 bytes.

Removed calls and glyphs: the PC profile of each run's `run.json` on the
patched tree, in a throwaway worktree (same game on all six runs), minus
HEAD's units. On every run exactly 8/9 of `print`'s window calls and glyphs
go: every `print` call of the six windows is an outlined pass (step 7's
289 a frame on `draw-5564` are its tree path's; the profile's 295.8 are all
callers'). Prediction: removed calls × the run's own per-call figure, and
removed glyphs × its per-glyph figure; both are 8/9 of the run's `print`
hblanks, so they are equal.

| Run | Draw delta | Of window total | Removed calls | Removed glyphs | Per call | Per glyph | Predicted | Gap |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| `start` | −64 | −0.68% | 56 | 160 | 1.124 | 0.393 | −62.9 | +1.7% |
| `update-4737` | −65 | −7.81% | 56 | 168 | 1.176 | 0.392 | −65.9 | −1.3% |
| `update-5500` | −74 | −3.47% | 64 | 192 | 1.163 | 0.388 | −74.4 | −0.5% |
| `draw-5564` | −8134 (−542.3) | −40.14% | 3944 | 25080 | 2.078 | 0.327 | −8193.9 | −0.7% |
| `update-5719` | −287 | −25.38% | 160 | 864 | 1.811 | 0.335 | −289.8 | −1.0% |
| `draw-6000` | −1532 (−102.1) | −10.43% | 1128 | 4248 | 1.371 | 0.364 | −1546.7 | −0.9% |

Gap = (measured − predicted) ÷ predicted. All six within 15% (−1.3% to
+1.7%): the per-unit table predicts a call-removing lever to about 2%. The
prediction carries the breakdown timer's per-call bias (reads high) and
misses the eight removed passes' outside-call work (the loop, the argument
setup, `p8_print`'s dispatch; reads low); on these runs the two nearly
cancel. With the pooled figures instead of the run's own, `draw-5564`
reads −7462 per call (+9.0% gap) and −8352 per glyph (−2.6%): for a lever
priced on another run, the pooled per-glyph figure is the one to use.

Reading: an **upper bound of the outlined-font lever** (one SPRT per glyph
from a pre-drawn outlined font): that lever keeps one primitive per glyph,
larger (5×7 instead of 3×5) and a CLUT per text colour, which the stand-in
does not pay. The stand-in itself changes pixels (no outline); the lever
does not. Step 7's candidate 1 bounded the same nine passes at 1410 (94) on
`draw-6000` from its tree; measured 1532 (102).

#### no-meter: the measured replay without the draw meter

Patch: in `src/cmd/psx_gpu_replay.c`, `p8_draw_meter_install()` removed
and `psx_gpu_frame_end` handed `GPU_FRAME_BUFFER_BYTES` (`p8/gpu_sizes.h`
included) instead of the meter's last frame bytes, so its overrun check is
vacuous; `p8_draw_meter_frame_end` stays (it copies a struct), the draw
filter stays, and the backend's own packet overflow check (`packet`,
`src/platform/psx_gpu.c:58`) still ends in `p8_fatal`. Text −2152 bytes.

| Run | Draw delta | Of window total | Backend calls | Per backend call |
|---|---:|---:|---:|---:|
| `start` | −124 | −1.31% | 549 | 0.226 |
| `update-4737` | −165 | −19.83% | 833 | 0.198 |
| `update-5500` | −164 | −7.70% | 856 | 0.192 |
| `draw-5564` | −3170 (−211.3) | −15.64% | 17769 | 0.178 |
| `update-5719` | −199 | −17.60% | 1072 | 0.186 |
| `draw-6000` | −2977 (−198.5) | −20.27% | 18469 | 0.161 |
| pooled | −6799 | | 39548 | 0.172 |

No per-unit prediction: the meter sits inside every family's time. Its
expected share is one cost-model call per backend call (plus the CLUT and
pattern tables on `spr` and the fills), so the delta is read per backend
call: 0.16 to 0.23 hblanks, about the `cls` floor (0.186) — the meter costs
as much per call as the cheapest whole call. No surprise against the code:
it walks every `print` string and runs every circle's midpoint loop, and
`spr`'s `count_clut` compares the palette with up to 12 entries on every
call, off screen included (`src/p8/draw_meter.c:21`, `:96`). The lever is
"drop the meter from `psx-game`", where it sits at `src/cmd/psx_game.c:68`
and `:95`–`:96` behind the same backend: the measured replay is the plan's
stand-in for it.

#### Overlap

The one-print delta contains the meter's and the filter's work on the 3944
(`draw-5564`) and 1128 (`draw-6000`) removed calls, each of which ran the
cost model's string walk twice; the no-meter delta contains the meter's work
on those same calls. Both gains are not to be added in full; step 12
settles the overlap.

### "Do less" inside the backend and the meter

The per-call path of the measured replay: runtime `p8_<family>` (one
indirect dispatch, `src/p8/gfx.c`) → breakdown timer (breakdown boot only)
→ draw meter (cost model, CLUT and pattern tables, `src/p8/draw_meter.c`)
→ draw filter (cost model, `src/devtools/draw_filter.c`) → GPU backend
(placement and clip in `psx_tiles.c`, `psx_sprite.c`, `psx_text.c`; CLUT
and pattern caches; texture-window check; one packet per primitive,
`src/platform/psx_gpu.c`). `psx-game` has the meter and the backend only.

Per family, on top of the dispatch and the two cost-model calls: `cls` one
FILL; `pset` clip, one TILE_1; `line` clip, one LINE_F2; `rect` up to 4
TILEs; `rectfill` one TILE, or pattern slot, window check and one SPRT;
`circ` the midpoint loop (third time, after the meter's and the filter's),
one TILE_1 per point; `circfill` the same loop, one TILE or SPRT per span;
`spr` placement and clip, CLUT lookup (`memcmp` over up to 6 slots, after
the meter's over up to 12), window check, page and region lookups, one SPRT;
`print` per character a lit-row test (up to 5 rows) and a font-cell lookup,
clip, window check, page and region lookups, one SPRT per glyph.

An `spr` call costs about 0.94–1.02 on screen and 0.45–0.49 off screen
(estimated: a two-term least-squares fit of each run's `spr` hblanks on its
on-screen and off-screen calls, all six runs / without `update-5719`; the
fit is within 2% of every draw run). Bounds below are window sums, per frame
in brackets, `draw-5564` then `draw-6000`.

1. **Recomputation: the meter in the shipped game** — `p8_draw_meter_install`
   (`src/cmd/psx_game.c:68`), `psx_gpu_frame_end(p8_draw_meter_last_frame().bytes)`
   (`:96`). The meter runs the cost model on every call only to check, at
   frame end, that the backend sent no more than the model; the backend's
   `packet` already refuses an overflow. Gain **measured**: 3170 (211),
   2977 (198); 124 to 199 on the one-frame runs. Pixel risk: identical. Cost
   outside pixels: the `zone gpu_frame` and `cap gpu_*` state lines that
   `tools/psx_check.sh` and `tools/psx_full_game_check.sh` read come from
   the meter (`src/devtools/state_format.c:282`) and would need the
   backend's own `used` and cache counts.
2. **Repeated calls: off-screen calls go all the way down** — `meter_*`
   (`src/p8/draw_meter.c:58`–`:104`) and the runtime's `p8_<family>`
   (`src/p8/gfx.c`). A call the cost model rates free (off screen) still
   reaches the backend, which places and clips it again to emit nothing; in
   the judge's replay it is also tested by the filter. Rejecting it once,
   at the first test, keeps the pixels. Off-screen calls outside the map
   (candidate 3): `pset` 394, `line` 68, `rectfill` 308, `spr` 135 on
   `draw-5564`; 1916, 444, 660, 225 on `draw-6000` (calls − units; `circ`,
   `circfill`, `print` not counted). Gain **estimated**, ≤ those calls ×
   their family's per call: ≤ 373 (25), ≤ 1129 (75), less the one test
   kept. Pixel risk: identical.
3. **Repeated calls: the map's off-screen cells** — `p8_map`
   (`src/p8/gfx.c:133`) from `background_draw`
   (`src/game/background.c:143`), runtime side just above the backend.
   `p8_map` issues one `p8_spr` per non-empty cell of the whole 32×32 map
   (400 a frame on `draw-5564`), wherever the camera is. Evidence: a PC-only
   throwaway that skips cells whose 8×8 box is off screen (same game on the
   six runs, units and switches unchanged on every run, the 20 e2e replays'
   `*.sha256` unchanged) removes 4935 of `draw-5564`'s 7541 `spr` calls and
   4830 of `draw-6000`'s 7707 (274, 309, 318, 209 on the one-frame runs):
   the source of two thirds of the off-screen `spr` calls. Gain
   **estimated**: removed calls × 0.45–0.49 = 2240–2433 (149–162),
   2193–2381 (146–159); 95 to 157 on the one-frame runs. Cross-check: step
   7's `p8_map` (3319, 3354) less its on-screen sprites (1065 and 1170 ×
   0.94–1.02) leaves 2233–2318 and 2161–2254. Pixel risk: identical.
   Overlaps step 7's candidate 5 (pre-built background), which removes all
   of `p8_map`.
4. **Repeated calls: nine placements per outlined text** —
   `hud_print_outlined` (`src/game/hud.c:85`) over `gpu_print`
   (`src/platform/psx_gpu.c:326`). Each pass walks the string in the
   filter, in the meter and in `psx_text_place`, for the same glyphs moved
   by one pixel. An outlined font (one SPRT per glyph) has the **measured**
   upper bound 8134 (542), 1532 (102); a backend entry that places each
   glyph once and emits its nine SPRTs keeps the packets and saves part of
   it (not separable here). Pixel risk: identical only if the outlined
   glyphs reproduce the nine passes (the outline colour is `0 & c`, always
   0; neighbours' outlines meet in the gap column only).
5. **Recomputation: lit-row test and font-cell lookup per glyph** —
   `glyph_lit`, `psx_text_place` (`src/platform/psx_text.c:5`, `:18`–`:21`).
   Up to 5 row reads per character to learn whether it has a lit row, and a
   division for its cell, recomputed on every pass of every text; a 256-entry
   table holds both. Gain **estimated**, not separable: part of the
   0.327–0.393 per glyph, inside `print`'s 9218 (615), 1740 (116); overlaps
   4. Pixel risk: identical.
6. **Recomputation: CLUT lookup per sprite** — `clut_slot`
   (`src/platform/psx_gpu.c:301`), `psx_clut_cache_slot`
   (`src/platform/psx_clut_cache.c:10`), and `count_clut` in the meter. A
   16-byte `memcmp` per slot on every on-screen `spr`, though the palette
   changes rarely between calls; a "same palette as last call" path skips
   it. Gain **estimated**, not separable: inside what a drawn sprite adds
   over an off-screen one (0.45–0.56 with placement and packet), loose
   ceiling 1102–1389 (73–93), 1183–1490 (79–99); the meter's part is in 1.
   Pixel risk: identical.
7. **Recomputation: page and region lookups per textured SPRT** —
   `add_textured_sprite`, `add_glyph` (`src/platform/psx_gpu.c:294`,
   `:321`). `psx_vram_layout_texture_page()` and
   `psx_vram_layout_region(...)` are called per sprite and glyph for values
   fixed at boot; hoisting them is a micro-optimization. Gain
   **estimated**, not separable: part of `spr`'s and `print`'s per unit.
   Pixel risk: identical.
8. **Recomputation: the midpoint loop three times per circle** —
   `circle_iterations` (`src/p8/draw_cost.c:97`) in the filter and the
   meter, then `psx_tiles_circ`/`psx_tiles_circfill` in the backend.
   Candidate 1 removes one, the filter is absent from `psx-game`; the model's
   count is a function of the radius alone (a small table). Gain
   **estimated**, ceiling `circ` + `circfill`: 1036 (69), 1359 (91), of
   which the model loops are a part. Pixel risk: identical.

Examined and dropped: the per-call texture-window check (`use_texture_window`,
`src/platform/psx_gpu.c:90`) is one compare, and switches are 34 on `spr`
and 39 on the fills over 34 window frames; nothing to remove.

**Finding, not measured: the draw filter.** `draw_filter_install`
(`src/cmd/psx_gpu_replay.c:199`) wraps every call of the judge's replay and
of the short runs and runs the cost model once more per call (it counts
per family for the capture masks); `psx-game` has no filter. Its cost is
not measured in this step (a third experiment is out of scope); it does the
same cost-model calls as the meter without the CLUT and pattern tables, so
it is likely of the order of, and below, the no-meter delta (0.16–0.23 a
call). Every figure of this report, and the judge's frames, read high by
it against `psx-game`; step 12 and the human should discount it.

### Verification

| Item | Figure | Runs |
|---|---|---|
| `cls` | 0.188 a call (pooled) | 0.185–0.186, low count on every run |
| `pset` | 0.279 a call, 0.435 a pixel | 0.272–0.301 a call |
| `line` | 0.400 a call, 0.450 a line | 0.390–0.420 |
| `rect` | 0.640 a call, 0.160 a tile | 0.630–0.645, low count on the one-frame runs |
| `rectfill` | 0.453 a call, 0.636 a tile or sprite | 0.438–0.563 |
| `circ` | 4.096 a call, 0.129 a point | 3.398–4.311, low count on the one-frame runs |
| `circfill` | 3.139 a call, 0.180 a span | 3.018–3.324, low count on the one-frame runs |
| `spr` | 0.646 a call, 1.880 a drawn sprite | 0.638–0.730 |
| `print` | 1.892 a call, 0.333 a glyph | 0.327–0.393 a glyph |
| `pattern` | 0.601 a call, 1.257 a span, inside its shapes | 0.577–0.642 |
| one-print | measured: −8134 (`draw-5564`), −1532 (`draw-6000`), −64 to −287 one-frame | six runs `valid`, `game-state same`, gaps −1.3% to +1.7% |
| no-meter | measured: −3170 (`draw-5564`), −2977 (`draw-6000`), −124 to −199 one-frame | six runs `valid`, `game-state same` |

Every family has a per-unit cost, each experiment a measured delta.

### Patches (backend)

Written by hand on `cbd0c4f` into `tmp/plan20-backend/` (git-ignored),
used once and discarded; none is committed.

| Patch | Use | Change |
|---|---|---|
| `b-one-print.patch` | probe, label `b-one-print`; PC profile on the patched tree | `src/game/hud.c`: `for (i = 0; i < 9; i++)` → `for (i = 8; i < 9; i++)` |
| `b-no-meter.patch` | probe, label `b-no-meter` | `src/cmd/psx_gpu_replay.c`: `p8_draw_meter_install();` removed, `psx_gpu_frame_end(GPU_FRAME_BUFFER_BYTES)`, `#include "p8/gpu_sizes.h"` |
| `pc-map-cull.patch` | PC profile and `make e2e` only, never booted (candidate 3's evidence) | `src/p8/gfx.c` `p8_map`: `continue` when the cell's 8×8 box less the camera is off the screen |

## Update spikes

Plan 20 step 9: the judge's 166 frames whose update alone is over 263
hblanks (42 of them over 1000), which `cart_update60` descendant each comes
from, why it runs then, how far the PC's update proportions hold on the
PSX, and how many frames each spike's work would take if spread out. It
opens with the calibration on update code, then the classification, the
spreading, the fallback full run that settles C13, and the verification.

Measured on commit `3fe4b8bbcf62f611247fcd853b0b6da31b2c56fe` (pcsx-redux,
asserts off), with no change to the game.

Pre-checks, all held:

- "Backend: per-unit costs" was the last section; this one follows it.
- Between `b6fd2f8` and `3fe4b8b`, `src/` changed only in
  `src/cmd/frame_profile.c` and `src/devtools/call_profile.*`; `psx/` did
  not change. The probe baseline's fingerprint matches `HEAD`, and its
  window parts are still `start` 8980 / 465 / 6 and `update-5500`
  1409 / 714 / 7 (update / draw / submit).
- **Save storage:** the measured GPU replay (`PSX_GPU_REPLAY_SRCS`, through
  `CORE_SRCS`) and the PC frame profile (`ENGINE_SRCS`) both link
  `src/devtools/save_storage_memory.c`, so the PSX replay saves to the same
  in-memory storage as the PC. No divergence is expected on
  `save_at_frame_end` from storage, and none large is measured (below).

### Calibration on update code

Commands:

```
make psx-probe PATCH=tmp/plan20-update/u-calibrate.patch PSX_SHORT_RUNS="start update-5500" PSX_PROBE_LABEL=u-calibrate
for r in start update-5500; do for k in 1 2 3 4; do
  d=tmp/frame-profile/calibration/$r/$k; mkdir -p $d
  STATE=$d/state.txt ./tmp/frame_profile tmp/psx/short-runs/$r/run.json $d tmp/frame_profile.syms
  sh tools/end_state.sh check tmp/psx/short-runs/$r/run.json $d/state.txt tmp/psx/short-runs/$r/run.end-state
done; done
```

Method: the throwaway patch wraps each of `cart_update60`'s five
statements in a span that reads the fine clock and the hblank frame
counter at both ends, the breakdown boot's clock pair. A span of 200
hblanks or more counts the coarse delta in whole hblanks, otherwise fine
ticks converted with the boot's `clk8-per-64-hblanks` line (17170 on
`start`, 17168 on `update-5500`). The `ui_update` span includes its
`dispatch_context_mem` and `main_ui` arguments, and the `save` span
includes the `g_save_cut` test. Per-frame records are printed after the
last frame, so printing is outside every frame. On PC, each run's
`run.json` was profiled 4 times (all `same-game yes`); a child's PC time is
its per-frame minimum over the 4 profiles, with `dispatch_context_mem` and
`main_ui` added to `ui_update`, and the denominator is `cart_update60`'s
per-frame minimum (`start` 7,965,788 ns, `update-5500` 657,248 ns).

Validity: both boots `valid`, `game-state same`. The spans' overhead
(probe update minus baseline update over the window frame) is +2 hblanks
on `start` (8980 → 8982) and +1 on `update-5500` (1409 → 1410). The five
spans sum to 8975.94 and 1371.09, under the window update less the
overhead (8980 and 1409), so no span is missing or counted twice.

Window frame (run frame 12 of `start` = full-run frame 12; run frame 24 of
`update-5500` = full-run frame 5500). PSX share is of the window update
(less the overhead), PC share of `cart_update60`'s PC time; ρ = PSX share
÷ PC share; gap = PSX share − PC share, in points.

| Run | Child | PSX hblanks | PSX share | PC share | ρ | Gap |
|---|---|---|---|---|---|---|
| `start` | `frame_zone_reset` | 0.05 | 0.0% | 0.0% | below | 0.0 |
| `start` | `cart_flush_exit_queue` | 0.06 | 0.0% | 0.0% | below | 0.0 |
| `start` | `update_entities` | 1.77 | 0.0% | 0.0% | below | 0.0 |
| `start` | `ui_update` | 8974.00 | 99.9% | 100.0% | 1.00 | −0.0 |
| `start` | `save_at_frame_end` | 0.06 | 0.0% | 0.0% | below | 0.0 |
| `update-5500` | `frame_zone_reset` | 0.05 | 0.0% | 0.0% | below | −0.0 |
| `update-5500` | `cart_flush_exit_queue` | 0.06 | 0.0% | 0.0% | below | −0.0 |
| `update-5500` | `update_entities` | 6.98 | 0.5% | 0.9% | below | −0.4 |
| `update-5500` | `ui_update` | 888.00 | 63.0% | 68.5% | 0.92 | −5.5 |
| `update-5500` | `save_at_frame_end` | 476.00 | 33.8% | 30.6% | 1.11 | +3.2 |

| Run | Window update | Overhead | Spans' sum | Outside the spans | PSX hblanks per PC µs |
|---|---|---|---|---|---|
| `start` | 8980 | +2 | 8975.94 | 4.06 | 1.127 |
| `update-5500` | 1409 | +1 | 1371.09 | 37.91 | 2.144 |

- **The PC proportions hold on update code.** Both applicable ratios are
  in 0.85–1.15 (high); on frame 5500 the PC overstates `ui_update`'s
  share by 5.5 points and understates `save_at_frame_end`'s by 3.2. Against the spans' sum
  instead of the window update (that is, against `cart_update60` itself on
  both sides), ρ is 1.00 for `ui_update` on `start`, 0.95 and 1.14 on
  `update-5500`.
- **Outside the spans is not `cart_update60`'s self:** `cart_update60` has
  nothing between its statements. The remainder is the replay's
  `input_step_run` (pad navigation), timed in update but outside
  `cart_update60` and so outside every PC profile rooted there. It is
  about 4 hblanks on title frames and about 40 on every in-game frame of
  both runs (frames 16–96 of `start`, 16–24 of `update-5500`), so it is a
  steady cost, not a spike.
- **PSX hblanks per PC µs differs by 1.9× between the runs** (1.127 on
  galaxy generation, 2.144 on frame 5500), so a PC time converts to PSX
  hblanks only per kind of work; shares, not times, are what carries over.
- `ui_update`, `save_at_frame_end` and the spans of 200 hblanks or more
  are counted in whole hblanks (±1); `update_entities` is "below" on both
  runs, so a class under it is uncalibrated.
- **For step 10:** this table is the calibration that corrects step 10's
  PC-derived update costs ("PC proportions corrected by step 9's
  calibration").

Patch, written by hand on `3fe4b8b` into `tmp/plan20-update/`
(git-ignored), used once and discarded; not committed:

| Patch | Use | Change |
|---|---|---|
| `u-calibrate.patch` | probe, label `u-calibrate`, `start` and `update-5500` | `src/game/cart.c`: each statement of `cart_update60` in a `SPAN(i, …)` under `PSX_PROBE`; `src/platform/psx_probe.[ch]`: span begin/end on the fine clock and frame timer, per-frame records (1024 frames), `probe-update:` lines; `src/cmd/psx_gpu_replay.c`: closes the record after `frame_times_record`, writes the lines in `probe_write` |

### Classification

Commands (at `42c5f2c`; the game is unchanged since `3fe4b8b`):

```
make frame-profile-spikes FRAME_PROFILE_REPEATS=8
```

K = 8, not the PRD's 4. At K = 4 (18 m 45 s) one class maximum read
119.3% (frame 2558, the wormhole class): `cart_update60`'s minimum over 4
profiles was 210,643 ns while the minimum of `economy_recompute` in
`economy_advance_clock`'s set was 251,365 ns, the same frame read 2× apart
between runs. At K = 8 (31 m 03 s) that class's maximum is 87.7%. The one
share still above 100% is frame 12's `nearest_step` at 106.4%, and more
profiles would not move it: it is the profiler, not timer noise. Rooted at
`entity_geom_nearest`, the frame reads 12.9 ms against 6.4 ms for the same
calls inside `galaxy_populate`'s set, because its 51,499 `nearest_step`
calls become direct children, each booked by call site. Frame 12's
responsible function is therefore read one level up (below).

Result: 166 spike frames, 42 over 1000, 19 classes, none open, nothing
unresolvable. Between K = 4 and K = 8, 58 frames changed path, always
deeper or shallower on the same branch, never to another branch. A frame
whose branch holds 50–55% flips depth with noise, but the event behind it
stays the same.

Columns: frames, frames over 1000, summed judge update (hblanks), PC share
of update min / median / max, calibrated interval (`p_median × ρ`, capped at
100%), label, `runs-on` / `spike-runs`, median PSX hblanks per PC µs.
Abbreviations: `U` = `cart_update60/ui_update/ui_handle_button`, `B` =
`U/ui_send_to_hovered#1/button_left_press_method/button_left_press`.
Ratios: `ui_update` 1.00 (`start`) and 0.92 (`update-5500`),
`save_at_frame_end` 1.11 (`update-5500`; "below" on `start`),
`update_entities` "below" on both runs.

| Class | Path | Responsible function | Fr. | >1000 | Update | PC share | Calibrated | Label | Runs-on / spike | hb/µs | Flags |
|---|---|---|---:|---:|---:|---|---|---|---|---:|---|
| C1 | `U/slipway_drop_method/slipway_drop` | `slipway_drop`, `src/game/slipway.c:157` | 36 | 33 | 45523 | 51.8 / 59.2 / 69.8% | 54.5% (ρ 0.92, has 5500) | high | 97 / 40 | 2.178 | — |
| C2 | `cart_update60/ui_update` | `ui_update`, `src/game/ui.c:266` | 16 | 0 | 12964 | 52.3 / 53.9 / 54.8% | 49.6–53.9% | high | 6059 / 166 | 2.092 | — |
| C3 | `U` | `ui_handle_button`, `src/game/ui.c:387` | 14 | 1 | 11806 | 50.3 / 53.9 / 61.0% | 49.6–53.9% | high | 6059 / 166 | 2.186 | — |
| C4 | `U/wormhole_drop_method/wormhole_drop/targeting_confirm/economy_advance_clock/economy_recompute` | `economy_recompute`, `src/game/economy.c:193` | 33 | 0 | 11728 | 69.6 / 76.6 / 87.7% | 70.5–76.6% | high | 110 / 108 | 1.852 | — |
| C5 | `cart_update60/save_at_frame_end` | `save_at_frame_end`, `src/game/cart.c:312` | 18 | 0 | 9961 | 58.4 / 82.1 / 87.9% | 91.1% | high | 6059 / 166 | 2.767 | — |
| C6 | `B/cart_start_game/galaxy_populate/entity_geom_nearest/nearest_step` | `entity_geom_nearest`, `src/engine/entity_geom.c:74` (tool leaf `nearest_step`, :62) | 1 | 1 | 8980 | 106.4% (tool leaf); 96.2% at `entity_geom_nearest` | 96.2% (ρ 1.00, `start`) | high | 1 / 1 | 1.353 | multi-site |
| C7 | `cart_update60/save_at_frame_end/save_state_encode` | `save_state_encode`, `src/game/save_state.c:243` | 13 | 0 | 6929 | 50.0 / 54.1 / 58.1% | 60.1% | high | 125 / 125 | 3.082 | — |
| C8 | `U/ui_send_to_hovered#1` | `ui_send_to_hovered`, `src/game/ui.c:434` | 8 | 1 | 6719 | 50.5 / 51.7 / 55.2% | 47.6–51.7% | high | 343 / 52 | 2.255 | multi-site |
| C9 | `B/planet_projects_panel_on_click_method/planet_apply_project/economy_advance_clock` | `economy_advance_clock`, `src/game/economy.c:95` | 4 | 2 | 3732 | 51.1 / 62.7 / 86.2% | 57.7–62.7% | high | 24 / 24 | 2.046 | multi-site |
| C10 | `B/planet_projects_panel_on_click_method/planet_apply_project` | `planet_apply_project`, `src/game/planet_display.c:474` | 3 | 3 | 3331 | 59.8 / 59.8 / 59.9% | 55.0–59.8% | high | 24 / 24 | 1.929 | multi-site |
| C11 | `U/slipway_drop_method` | `slipway_drop_method`, `src/game/slipway.c:325` | 3 | 0 | 2731 | 52.4 / 53.0 / 56.7% | 48.8–53.0% | high | 58 / 40 | 2.149 | — |
| C12 | `B/planet_projects_panel_on_click_method/planet_apply_project/economy_advance_clock/economy_recompute` | `economy_recompute`, `src/game/economy.c:193` | 5 | 0 | 2459 | 51.2 / 79.9 / 84.1% | 73.5–79.9% | high | 110 / 108 | 2.135 | multi-site |
| C13 | `cart_update60` (PC); `cart_update60/save_at_frame_end` (full run) | none on PC (no child reaches 50%); `save_at_frame_end`, `src/game/cart.c:312`, measured | 3 | 0 | 2006 | 100.0% on PC; measured 50.3 / 58.4 / 59.8% | 50.3–59.8% (measured on the full run) | high (measured) | 6059 / 166 | 2.167 | unsettled on PC, settled by the fallback |
| C14 | `U/ui_send_to_hovered#1/button_left_press_method` | `button_left_press_method`, `src/game/ui_button.c:52` | 2 | 0 | 1649 | 51.0 / 52.5 / 53.9% | 48.3–52.5% | high | 120 / 51 | 2.148 | multi-site |
| C15 | `B/galaxy_menu_button1_on_click_method/economy_new_year/economy_recompute` | `economy_recompute`, `src/game/economy.c:193` | 2 | 0 | 1468 | 52.6 / 68.2 / 83.7% | 62.7–68.2% | high | 25 / 25 | 2.091 | multi-site |
| C16 | `B` | `button_left_press`, `src/game/ui_button.c:276` | 2 | 0 | 1430 | 51.5 / 52.5 / 53.5% | 48.3–52.5% | high | 120 / 51 | 2.220 | multi-site |
| C17 | `U/targeting_confirm_method/targeting_confirm/economy_advance_clock/economy_recompute` | `economy_recompute`, `src/game/economy.c:193` | 1 | 1 | 1130 | 58.1% | 53.5–58.1% | high | 110 / 108 | 2.236 | — |
| C18 | `B/galaxy_menu_button1_on_click_method/economy_new_year` | `economy_new_year`, `src/game/economy.c:118` | 1 | 0 | 879 | 50.2% | 46.2–50.2% | high | 17 / 17 | 2.223 | multi-site |
| C19 | `cart_update60/update_entities` | `update_entities`, `src/engine/bootstrap.c:53` | 1 | 0 | 266 | 72.6% | 72.6% (no ratio) | uncalibrated | 6059 / 166 | 4.530 | — |

Frames per class:

- C1: 383, 512, 1244, 1258, 1449, 1751, 1860, 2074, 2176, 2190, 2365, 2377, 2395, 2434, 3231, 3243, 3275, 3313, 3525, 3618, 3634, 4238, 4250, 4374, 4709, 4721, 4737, 5022, 5086, 5098, 5110, 5133, 5387, 5500, 5719, 5727
- C2: 145, 352, 741, 1786, 2312, 2802, 2980, 3150, 3224, 4145, 5303, 5422, 5579, 5610, 5667, 5842
- C3: 159, 1486, 2228, 2260, 2358, 3053, 3338, 3386, 4696, 5077, 5168, 5810, 5904, 5944
- C4: 320, 433, 553, 604, 658, 710, 951, 996, 1043, 1098, 1145, 1320, 1568, 1620, 1661, 1928, 1977, 2558, 2642, 2715, 2771, 3022, 3449, 3720, 3817, 3907, 4001, 4067, 4106, 4483, 4521, 4590, 4627
- C5: 58, 230, 461, 978, 1173, 1595, 1689, 2005, 2742, 3050, 3476, 3747, 3844, 3934, 4028, 4134, 4549, 4655
- C6: 12
- C7: 347, 582, 632, 685, 737, 1024, 1071, 1125, 1347, 1955, 2585, 2669, 2798
- C8: 1413, 2414, 2471, 2924, 3303, 4912, 4929, 5470
- C9: 229, 2147, 3265, 4300
- C10: 4952, 5339, 5488
- C11: 245, 259, 1386
- C12: 456, 1165, 1349, 1684, 3475
- C13: 113, 825, 901
- C14: 2006, 3567
- C15: 5537, 6043
- C16: 1226, 2108
- C17: 5781
- C18: 4774
- C19: 91

Every applicable ratio is in 0.85–1.15, so every calibrated class is
high. Seven of them (C2, C3, C8, C11, C14, C16, C18) have an interval that
straddles 50%, but a high label does not trigger the "cannot settle" rule.
The same ratio is applied at every depth: it is the best measured
correction for the child the path goes through, not a measurement of the
deeper function.

**PC cannot settle C13 only** (`unsettled`, frames 113, 825, 901). That
leaves C13 to the fallback, which settles it ("Fallback" below). On PC
each of its frames is an event branch under `ui_update` (a project on
113, a year skip on 825, a tech-tree invention on 901) beside the save,
at 43–48% each. C19 is
uncalibrated, but its 72.6% point share does not straddle 50%, so it is
settled.

### What triggers each class

The tool's classes are kept as they are. Six merges, each of the same
cause, name their paths and keep each class's figures above. Below, "event
branch" is the largest child at each level of the descent, without the 50%
rule: a reading aid for the shallow classes, not a split. One thing is
common to all but C6 and C19: a player action that changes the economy
runs `economy_recompute` (`src/game/economy.c:193`) once or more, and
`economy_recompute` walks every planet (79 `economy_score_net_income` and
79 `planet_refresh_method` calls on frame 2558). The action also sets
`save_pending`, so `save_at_frame_end` (`src/game/cart.c:312`) saves the
game in the same frame, unless a wormhole is live.

**Slipway drop: C1 + C11** (39 frames, 33 over 1000, 48,254 hblanks).
The player drags a slipway from a settled planet and releases it on
another. On release, `ui_handle_button` calls the held slipway's `drop`.
`slipway_drop` first calls `targeting_confirm`, which charges the cost and
runs `economy_advance_clock` with the slipway's 0 clock ticks. That
recomputes the economy before the slipway is attached. Then
`slipway_drop` attaches both ends and calls `economy_advance_clock(1)`
(`src/game/slipway.c:190`), which recomputes again. On 5 frames (512,
1244, 3313, 3634, 4737) that tick crosses the year, and `economy_new_year`
recomputes a third time. On frame 4737, `targeting_confirm` takes 21.1%
of update and the second `economy_advance_clock` 47.1%, of which
`economy_new_year` is 22.9%. The save runs in the same frame at 21–34%,
the class's second-largest node. These are the heaviest spikes: 33 of the
42 frames over 1000. `slipway_drop` runs on 97 frames, 40 of them spikes:
a refused drop (no planet under the cursor) is cheap. C3's frame 159 is a
slipway drop too (event branch), kept in C3.

**Wormhole launch: C4** (33 frames, 0 over 1000, 11,728 hblanks).
Releasing a wormhole after its 15th tick runs `wormhole_drop` →
`targeting_confirm`, which charges 3 credits and calls
`economy_advance_clock` with 1 tick, so the whole economy is recomputed
(`economy_recompute` 69.6–87.7% of update). No save runs in this frame:
`save_at_frame_end` waits while `wormhole_live_count()` is non-zero. In
the recording, a wormhole is launched about every 50 frames. The C4
frames are the small regular spikes of 266–450 hblanks that the PRD
expected as "periodic". `economy_recompute` runs on 110 frames, 108 of
them spikes.

**Deferred save: C5 + C7** (31 frames, 0 over 1000, 16,890 hblanks).
These are `save_at_frame_end` (C5) and its `save_state_encode` (C7).
30 of the 31 frames come 27–29 frames after a wormhole launch (202 → 230,
553 → 582, 604 → 632, 658 → 685, 710 → 737, …): the save the launch made
pending runs on the first frame without a live wormhole. Frame 58 is a
structure placement whose save runs in the same frame (save 58%). The save
is `save_state_capture` plus `save_state_encode`. The encoding's
`save_state_crc32` (`src/game/save_state.c:601`, bit by bit, 8 steps a
byte) takes 11–27% of update on these frames. `save_state_encode` runs
on 125 frames, all spikes: no save fits in 263.

**Project on a planet: C9 + C10 + C12** (12 frames, 5 over 1000, 9,522
hblanks). Clicking a project in a known planet's projects panel runs
`planet_projects_panel_on_click_method` → `economy_spend` →
`planet_apply_project`. That calls `economy_advance_clock` with 2 or 3
ticks (`src/game/planet_display.c:512`), and on 2147 and 4300
`economy_new_year` as well. The save follows in the same frame (0–41%),
except on 229, where a wormhole launched on 202 was still live. Its save
is frame 230 (C5).

**Year skip: C15 + C18** (3 frames, 2,347 hblanks). HUD button 1
(`galaxy_menu_button1_on_click_method`, `src/game/menu.c:187`) sets the
year's ticks and calls `economy_new_year`, which recomputes and sends
`new_year` to every listener. The save follows in the same frame.

**Structure placement: C17** (frame 5781, 1,130 hblanks). Dropping a
structure runs `targeting_confirm_method` → `economy_advance_clock` with
2 ticks → `economy_recompute` (58.1%). The save follows (36%).

**Event and save in one frame: C2, C3, C8, C14, C16** (42 frames,
2 over 1000, 34,568 hblanks), **and C13** (3 frames). The event branches
are 11 project clicks, 13 year skips, 8 tech-tree inventions
(`tech_tree_screen_row_on_click_method` → `economy_invent` →
`economy_recompute`), 9 structure placements and 1 slipway drop, all
under `ui_update`. Each is followed in the same frame by the save at
29–46% of update. The event's branch (`ui_update`) holds 52–69%, so the 50% rule stops
wherever that branch first drops under half, and the depth flickers with
noise. When neither half reaches 50%, the frame is C13.

**Game start: C6** (frame 12, 8,980 hblanks). The title's "new game"
press runs `cart_start_game` → `galaxy_populate`
(`src/game/galaxy.c:14`). That places 90 planets by rejection sampling: a
random point is redrawn while `entity_geom_nearest` finds a planet within
0.05. Here `entity_geom_nearest` runs 727 times, for 51,499
`nearest_step` calls. `galaxy_populate` is 86.8% of update in
`cart_start_game`'s set, and `entity_geom_nearest` 96.2% in
`galaxy_populate`'s set. Accepted points account for 4,005 of the steps
(0 + 1 + … + 89), so 92.2% of the steps are spent on points that are
rejected.

**Entities: C19** (frame 91, 266 hblanks, 3 over). On this early in-game
frame, `update_entities` runs `score_panel_update_method` (36% of update)
and a wormhole's `wormhole_update_unfolding_method` (30%). Both run every
frame; this is their sum crossing the budget by 3 hblanks, not an event.

### Do-less candidates

Bounds are judge hblanks summed over the named frames, from per-frame PC
shares (K = 8 minimum) times the judge update, labelled **estimated**. A
node profiled as its own root can read high (see frame 12 above), so
these are upper bounds. They overlap and are not ranked (step 12's job).
Every class also gets the spread candidate, whose count is in the
spreading table below.

| # | Classes | Kind | Location | Evidence | Upper bound (estimated) | Risk |
|---|---|---|---|---|---|---|
| U1 | C1, C11 | recomputation | `slipway_drop`'s `targeting_confirm`, `src/game/slipway.c:165` → `src/game/probe.c:322` | the first `economy_advance_clock` (0 ticks) recomputes the economy before the slipway exists; the second (`slipway.c:190`) recomputes it again; `targeting_confirm` is 20–33% of update (median 29%) | 13,479 on 39 frames | game state unless the first recompute's side effects (tier increases, refreshed rows) are also reached by the second; e2e decides |
| U2 | C1, C4, C9–C12, C15, C17, C18, and the event branches of C2, C3, C8, C13, C14, C16 | recomputation | `planet_refresh_method` in `economy_recompute`, `src/game/economy.c:257` → `planet_refresh`, `src/game/planet_display.c:41` | every recompute rebuilds the layout row of all 79 planets; 12–37% of update on these frames | 24,576 (slipway 11,398, wormhole 3,811, project 1,911, year skip 553, structure 202, shallow 6,392, C13 309) | timing only if a row is rebuilt only when its needs, output or routes changed, or built when shown (same pixels) |
| U3 | C4 | identical work | `wormhole_drop` → `targeting_confirm` → `economy_recompute` | a wormhole launch changes credits and one clock tick, not routes or production; the tiers, production and per-planet passes recompute the same figures | 10,891 on 33 frames (whole `economy_recompute`, less what the credit-dependent `end_status` test needs) | game state if any recomputed figure does change; e2e decides |
| U4 | C5, C7, and the save of every event frame | identical work | `save_state_crc32`, `src/game/save_state.c:601` | bitwise CRC-32, 8 shift steps per byte; 11–27% of update on save frames | 11,851 (save 3,557, slipway 3,459, shallow 3,688, project 579, others 568), bound of the whole CRC; a 256-entry table keeps about 1/8 of it | timing only (same bytes) |
| U5 | C1–C3, C8–C18 | spread across frames | `save_at_frame_end`, `src/game/cart.c:331` | the save shares the event's frame at 21–48% of update; it already waits for wormholes, so waiting one more frame splits the frame in two | 32,554 moved off the event frames (slipway 13,431, shallow 14,617, project 2,379, year skip 810, structure 411, C13 906, on the next frame) | e2e re-record (the save lands one frame later) |
| U6 | C6 | identical work | `entity_geom_nearest` in `galaxy_populate`, `src/game/galaxy.c:31` | the loop only tests "nearest ≤ 0.05", but each scan finds the true nearest; 92.2% of the 51,499 steps are spent on rejected points | 7,963 if a rejected scan stopped at once; about half (3,980) for a close planet met halfway on average | timing only (same draws, same planets) |
| U7 | C6 | spread across frames | `galaxy_populate`, `src/game/galaxy.c:14` | galaxy generation is one frame of 8,980 hblanks; spreading it is allowed by the plan | 104–285 frames (`step7`), 68–162 (`step7+8`), does-not-fit with no draw lever (spreading table) | e2e re-record |
| U8 | C19 | spread across frames | `update_entities`, `src/engine/bootstrap.c:53` | 3 hblanks over the budget, two always-running updates (36% + 30%) | 3 hblanks needed | e2e re-record |

### The four named frames

| Frame | Judge update | Class | Responsible function | PC share | Calibrated |
|---:|---:|---|---|---|---|
| 12 | 8980 | C6 game start | `entity_geom_nearest` (tool leaf `nearest_step`) | 96.2% (tool leaf 106.4%, a profiler artifact) | 96.2%, high |
| 4737 | 1783 | C1 slipway drop (with `economy_new_year`) | `slipway_drop` | 69.8% | 64.2%, high (ρ 0.92) |
| 5500 | 1416 | C1 slipway drop | `slipway_drop` | 53.7% | 49.4%, high (ρ 0.92) |
| 5719 | 1430 | C1 slipway drop | `slipway_drop` | 53.4% | 49.1%, high (ρ 0.92) |

Frames 4737, 5500 and 5719 are the same event: a slipway drop with two
economy recomputes and the save in the same frame. 4737 is the heaviest
of the three because its tick also crosses the year (a third recompute).
The class does not explain why the resumed short runs keep 5500's spike
and lose the other two: all three are C1.

### Spreading

Commands (at `94e6895`; `spikes.txt` is the K = 8 classification above):

```
R=ui_update=0.96,save_at_frame_end=1.11
make frame-profile-spread SPREAD_LABEL=none        DRAW_GAINS=default=0                 UPDATE_RATIOS=$R
make frame-profile-spread SPREAD_LABEL=step7-low   DRAW_GAINS=default=75.5,12-12=51.8   UPDATE_RATIOS=$R
make frame-profile-spread SPREAD_LABEL=step7-high  DRAW_GAINS=default=86.6,12-12=51.8   UPDATE_RATIOS=$R
make frame-profile-spread SPREAD_LABEL=step78-low  DRAW_GAINS=default=80.7,12-12=74.8   UPDATE_RATIOS=$R
make frame-profile-spread SPREAD_LABEL=step78-high DRAW_GAINS=default=88.8,12-12=74.8   UPDATE_RATIOS=$R
```

**Ratios:** the calibration's mean ratio per child: `ui_update` (1.00 +
0.92) / 2 = 0.96, `save_at_frame_end` 1.11 (its only ratio; "below" on
`start`). `update_entities` is "below" on both runs and left out (ratio
1), and so is C13's empty child (path `cart_update60` alone, share 100%).

**Draw gains,** by the PRD's rule: a run's gain is the sum of the
window bounds of step 7's candidates whose pixel risk is "identical" or
"identical if/only …", over the run's window draw (`start` 465,
`draw-5564` 19491, `draw-6000` 13987). The text group counts the larger
of candidate 1 and 2 + 3 + 4; 2 includes the tooltip (421 on
`draw-5564`); 4 includes the formatting (51, 41); 7 is dropped on both
runs (its rows are paths 1, 3 and 5, covered by 2, 6 and 3); 9 counts its
dispatch (144, 150); 11 counts its batched variant; 12 counts the dots,
15 the particles (534 = 293 + 241 on `draw-5564`); 14 counts 0; "less a
copy" is ignored. `start`'s tree has the map alone (candidate 5, 241).
`step7+8` replaces candidate 1 by step 8's measured one-print delta and
then combines the no-meter gain `m` as `1 − (1 − g)(1 − m)`.

| Run | Text group (1 against 2+3+4) | Other candidates | `step7` | One-print (step 8) | Text group, `step7+8` | No-meter `m` | `step7+8` |
|---|---:|---:|---:|---:|---:|---:|---:|
| `start` | 0 (no text above 2%) | 241 (5) | 51.8% | 64 | 64 | 26.7% | 74.8% |
| `draw-5564` | 9902 (2+3+4 > 7819) | 6974 (5 3319, 6 830, 8 924, 9 144, 10 207, 11 539, 12 351, 13 126, 15 534) | 86.6% | 8134 | 9902 (still 2+3+4) | 16.3% | 88.8% |
| `draw-6000` | 1916 (2+3+4 > 1410) | 8639 (5 3354, 6 1212, 8 1230, 9 150, 10 183, 11 756, 12 907, 13 123, 15 724) | 75.5% | 1532 | 1916 (still 2+3+4) | 21.3% | 80.7% |

Frame 12 gets `start`'s gain (`12-12=`); every other frame gets the
smaller in-game gain (`-low`, giving `N_high`) or the larger (`-high`,
giving `N_low`). Frame 12's own gain is the same in both halves of a
bracket; its count still moves because the frames after it take the
default gain. On the in-game runs step 8's one-print delta is smaller
than candidates 2 + 3 + 4, so `step7+8` differs from `step7` by the meter
alone; it reads optimistic by the meter's share of the calls the other
levers remove (counted twice, step 12 settles it). All these gains are
upper bounds, and step 8's estimated candidates are not in them.

**The spreading estimate is optimistic:** the work is assumed divisible
at any grain, with no cost to split it or to carry state between frames.
`frames` counts the spike frame itself.

Per class: work in hblanks (min / median / max, the same in every
scenario) and frames (min / median / max over the frames that fit,
`does-not-fit` count in brackets). `—` means no frame of the class fits.
Bracketed scenarios read `N_high` (smaller gain) then `N_low` (larger
gain).

| Class | Work | `none` | `step7`, gain 75.5% | `step7`, gain 86.6% | `step7+8`, gain 80.7% | `step7+8`, gain 88.8% |
|---|---|---|---|---|---|---|
| C1 | 474 / 722.5 / 1195 | — (36) | 16 / 57.5 / 220 (4) | 7 / 12 / 22 (0) | 10 / 24.5 / 67 (0) | 7 / 10 / 20 (0) |
| C2 | 321 / 392 / 518 | — (16) | 9 / 19 / 137 (6) | 4 / 6.5 / 14 (0) | 6 / 18 / 108 (0) | 4 / 5.5 / 9 (0) |
| C3 | 355 / 445 / 532 | — (14) | 11 / 43 / 160 (3) | 5 / 6.5 / 12 (0) | 6 / 11.5 / 55 (0) | 4 / 5.5 / 8 (0) |
| C4 | 189 / 245 / 377 | — (33) | 31 / 37 / 98 (0) | 2 / 6 / 22 (0) | 11 / 30 / 33 (0) | 2 / 5 / 11 (0) |
| C5 | 397 / 498.5 / 535 | — (18) | 7 / 22 / 94 (0) | 4 / 5 / 8 (0) | 5 / 8.5 / 22 (0) | 4 / 5 / 7 (0) |
| C6 | 8980 | — (1) | 285 (0) | 104 (0) | 162 (0) | 68 (0) |
| C7 | 291 / 316 / 351 | — (13) | 7 / 9 / 47 (0) | 4 / 4 / 6 (0) | 5 / 6 / 7 (0) | 4 / 4 / 5 (0) |
| C8 | 349 / 417 / 522 | — (8) | 26 / 55 / 174 (1) | 5 / 7 / 12 (0) | 11 / 26.5 / 54 (0) | 5 / 6 / 9 (0) |
| C9 | 415 / 537 / 795 | — (4) | 21 / 45.5 / 69 (0) | 5 / 9 / 13 (0) | 13 / 19.5 / 53 (0) | 5 / 8 / 10 (0) |
| C10 | 613 / 642 / 660 | — (3) | 58 (2) | 8 / 10 / 23 (0) | 25 / 36 / 46 (0) | 7 / 8 / 19 (0) |
| C11 | 434 / 475 / 508 | — (3) | 14 / 18 / 24 (0) | 7 / 8 / 9 (0) | 10 / 13 / 13 (0) | 7 / 8 / 8 (0) |
| C12 | 273 / 343 / 418 | — (5) | 17 / 69 / 98 (0) | 5 / 10 / 12 (0) | 10 / 15 / 19 (0) | 5 / 8 / 11 (0) |
| C13 (PC, superseded below) | 623 / 639 / 744 | — (3) | 14 / 64 / 87 (0) | 6 / 7 / 9 (0) | 8 / 16 / 22 (0) | 5 / 6 / 8 (0) |
| C14 | 376 / 414 / 452 | — (2) | 15 / 45.5 / 76 (0) | 5 / 5.5 / 6 (0) | 8 / 9.5 / 11 (0) | 5 / 5 / 5 (0) |
| C15 | 405 / 446.5 / 488 | — (2) | — (2) | 9 (1) | 134 (1) | 7 / 9 / 11 (0) |
| C16 | 347 / 360.5 / 374 | — (2) | 42 / 70 / 98 (0) | 6 / 7 / 8 (0) | 13 / 16.5 / 20 (0) | 5 / 5 / 5 (0) |
| C17 | 631 | — (1) | — (1) | 9 (0) | 20 (0) | 8 (0) |
| C18 | 424 | — (1) | 53 (0) | 7 (0) | 21 (0) | 6 (0) |
| C19 | 194 | — (1) | 77 (0) | 25 (0) | 28 (0) | 24 (0) |
| All | | 166 does-not-fit | 19 does-not-fit | 1 | 1 | 0 |

- **With no draw lever, no spike frame fits** (166 of 166 does-not-fit).
  Draw's median over the judge's frames is 717 hblanks, and no frame from
  12 onward has room under 263 on its own, so nothing can be spread into
  the frames that follow until draw shrinks. Step 8's two measured gains
  alone (one-print and no-meter combined: 36.8% `start`, 51.2%
  `draw-5564`, 30.0% `draw-6000`) are below the 64% threshold of the sweep
  below, so they leave every spike frame does-not-fit too.
- **C6 (frame 12)** uses the tool leaf's 106.4%, capped at 100%: work
  8980. At the report's 96.2% (`entity_geom_nearest`) the work is 8639
  and the counts drop to 273, 70, 156 and 64 (same columns).
- **Does-not-fit under a lever:** `step7` at 75.5% leaves 19 frames (C1 4,
  C2 6, C3 3, C8 1, C10 2, C15 2, C17 1); at 86.6% and at `step7+8` 80.7%
  only frame 6043 (C15), 16 frames before the run's end, where the
  end-of-game screen's draw (947–1235 a frame) leaves no room; at 88.8%
  it fits in 11.

Overlaps (spreads whose window holds another spike frame, so both cannot
count the same room as free), `spike -> other spikes in its window`:

- `step7+8`, gain 88.8% (13): 12->58, 91->113, 229->230, 456->461,
  1165->1173, 1347->1349, 1684->1689, 2005->2006, 3050->3053, 3475->3476,
  5098->5110, 5488->5500, 5719->5727.
- `step7`, gain 86.6% (18): 12->58,91,113, 91->113, 229->230, 456->461,
  1165->1173, 1347->1349, 1684->1689, 2005->2006, 2358->2365, 3050->3053,
  3224->3231, 3475->3476, 4238->4250, 5077->5086, 5086->5098, 5098->5110,
  5488->5500, 5719->5727.
- `step7+8`, gain 80.7% (72 frames) and `step7`, gain 75.5% (92 frames):
  the windows chain across most of the run (for example 5077 -> 5086,
  5098, 5110 and 5422 -> 5470, 5488, 5500 at 80.7%; 2312 -> 2358 … 2434
  and 5022 -> 5077 … 5168 at 75.5%); each frame's list is in
  `tmp/frame-profile/spikes/spread-step78-low.txt` and
  `spread-step7-low.txt`, reproduced by the commands above.

Most overlaps are a save frame right after its event frame (C5/C7 frames
27–29 frames after a wormhole, or the save following an event): spreading
both draws on the same room, so the later frame's count reads low.

**Addition beyond the PRD: the gain at which each class first fits.** The
same tool, uniform gain `default=<g>` for `g` = 0 … 100 in steps of 1
(frame 12 included), same ratios. First: the smallest gain at which one
frame of the class fits; all: at which every frame fits.

| Class | C1 | C2 | C3 | C4 | C5 | C6 | C7 | C8 | C9 | C10 | C11 | C12 | C13 (PC) | C14 | C15 | C16 | C17 | C18 | C19 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| First | 67 | 66 | 65 | 64 | 65 | 72 | 66 | 66 | 66 | 70 | 66 | 65 | 67 | 66 | 76 | 68 | 77 | 68 | 65 |
| All | 77 | 77 | 77 | 70 | 71 | 72 | 70 | 77 | 69 | 77 | 68 | 68 | 69 | 71 | 88 | 68 | 77 | 68 | 65 |

Over all 166 frames: none fits below 64%, 7 fit at 65%, 141 at 70%,
147 at 75%, 165 at 80%, and all 166 from 88% (frame 6043 last). Spreading
is therefore conditional on draw losing about two thirds of its cost
first; below that, only "do less" on the update side can bring a spike
frame under 263.

### Fallback: C13 on the full run

C13 is `unsettled` (no child reaches 50% on PC), so the PRD's rule
requires the fallback. One asserts-off full run on a throwaway patch,
made in a temporary worktree of `94e6895` (the game is unchanged since
`3fe4b8b`):

```
git apply tmp/plan20-update/u-fallback.patch
make psx-full-game-check PSX_FULL_GAME_ASSERTS=off PSN00BSDK_DIR=<main tree>/third_party/psn00bsdk PSX_EMULATOR_DIR=<main tree>/third_party/pcsx-redux
```

Suspects: C13's path is `cart_update60` alone, so its suspects are
`cart_update60`'s five children, timed by the calibration's span
mechanism (fine clock unwrapped by the hblank counter; `clk8-per-64-hblanks`
17170). The patch keeps a frame's record when its spans sum to 200
hblanks or more and prints the records after the last frame, outside every
frame. It is built into `psx-gpu-replay` only, without `PSX_PROBE`, so the
GPU frame buffer keeps its shipped size.

Conditions: 19 m 29 s. Eleven of the twelve conditions `ok`, among them
`gpu-replay-done` and `gpu-state-game-lines` (game state the same as the
PC); `frame-budget` failed, as on the judge run (6047 frames over 263,
worst frame 12 at 9451). Against the judge table, the boot's update
differs by −2 to +3 hblanks a frame (mean +0.38 over 6059 frames) and its
draw by −3 to +3: the spans' overhead is within the timer's resolution.
169 frames were recorded, 165 of the 166 spike frames among them (frame
91, C19, sums to under 200 in `cart_update60`: 266 with the replay's input
step).

Per C13 frame, PSX hblanks per child (`frame_zone_reset` and
`cart_flush_exit_queue` 0.05 and 0.06 on all three), the boot's update
(judge in brackets) and the PC shares (K = 8 minimum) for comparison:

| Frame | Update | `update_entities` | `ui_update` | `save_at_frame_end` | Outside the spans | PC `ui_update` / save | Responsible | Share of update |
|---:|---:|---:|---:|---:|---:|---|---|---:|
| 113 | 744 (744) | 121.21 | 239 | 374 | 9.68 | 47.9% / 44.5% | `save_at_frame_end` | 50.3% |
| 825 | 639 (639) | 5.43 | 229 | 373 | 31.46 | 49.4% / 43.1% | `save_at_frame_end` | 58.4% |
| 901 | 622 (623) | 6.55 | 231 | 372 | 12.34 | 48.0% / 48.0% | `save_at_frame_end` | 59.8% |

**C13 is settled: `save_at_frame_end` (`src/game/cart.c:312`) holds
50.3–59.8% of update on the PSX, measured on the full run, confidence
high.** The event branch under `ui_update` (a project on 113, a year skip
on 825, an invention on 901) holds 32.1–37.1%. On PC the two halves read
about equal; the PSX gives the save the larger part. Its do-less
candidates are U4 (the bitwise CRC) and U5 (the save one frame later),
both already listed with C13's figures.

C13's spread with the measured share (work 374, 373, 373; spikes file
with C13's path set to `cart_update60/save_at_frame_end` and its measured
shares, no ratio applied, same gains):

| Class | Work | `none` | `step7`, 75.5% | `step7`, 86.6% | `step7+8`, 80.7% | `step7+8`, 88.8% | First / all fit (sweep) |
|---|---|---|---|---|---|---|---|
| C13 (measured) | 373 / 373 / 374 | — (3) | 9 / 58 / 82 (0) | 5 / 6 / 7 (0) | 6 / 12 / 13 (0) | 4 / 5 / 6 (0) | 66 / 68 |

Overlaps: at 75.5%, 113's spread covers 145 and 159, and 901's covers 951
and 978; none at the other gains.

**Addition beyond the PRD: the calibration over every spike frame.** The
same run times the five children on 165 spike frames, so the calibration
ratio can be read per class, not only on frames 12 and 5500. ρ per frame
is PSX share ÷ PC share, both against `cart_update60` itself (the spans'
sum; the PC's K = 8 minimum), "below" when both are under 2%; min /
median / max over the class's frames, with the PSX's median shares:

| Class | Frames | ρ `ui_update` | ρ `save_at_frame_end` | PSX `ui_update` | PSX save |
|---|---:|---|---|---:|---:|
| C1 | 36 | 0.82 / 0.92 / 0.96 | 1.17 / 1.25 / 1.43 | 64.3% | 35.2% |
| C2 | 16 | 0.70 / 0.82 / 0.98 | 1.16 / 1.28 / 1.37 | 44.4% | 54.8% |
| C3 | 14 | 0.78 / 0.84 / 0.90 | 1.14 / 1.24 / 1.57 | 46.4% | 52.8% |
| C4 | 33 | 1.00 / 1.01 / 1.02 | below | 97.7% | 0.0% |
| C5 | 18 | 0.46 / 0.54 / 0.92 | 0.93 / 0.97 / 1.12 | 3.8% | 79.8% |
| C6 | 1 | 1.00 | below | 100.0% | 0.0% |
| C7 | 13 | 0.44 / 0.48 / 0.54 | 0.93 / 0.95 / 1.00 | 4.0% | 75.3% |
| C8 | 8 | 0.81 / 0.87 / 0.90 | 1.17 / 1.21 / 1.27 | 47.0% | 52.1% |
| C9 | 4 | 0.85 / 0.91 / 0.92 | 1.19 / 1.22 / 1.30 | 64.0% | 35.4% |
| C10 | 3 | 0.87 / 0.88 / 0.89 | 1.26 / 1.27 / 1.29 | 57.5% | 41.9% |
| C11 | 3 | 0.81 / 0.82 / 0.88 | 1.35 / 1.38 / 1.38 | 52.9% | 46.3% |
| C12 | 5 | 0.77 / 0.83 / 0.93 | 1.35 | 74.4% | 0.0% |
| C13 | 3 | 0.68 / 0.76 / 0.79 | 1.14 / 1.27 / 1.42 | 37.7% | 61.0% |
| C14 | 2 | 0.80 / 0.86 / 0.92 | 1.13 / 1.21 / 1.30 | 49.3% | 49.9% |
| C15 | 2 | 0.88 / 0.94 / 1.01 | 1.17 | 73.3% | 25.5% |
| C16 | 2 | 0.78 / 0.81 / 0.84 | 1.25 / 1.27 / 1.30 | 45.9% | 53.1% |
| C17 | 1 | 0.89 | 1.21 | 55.3% | 44.0% |
| C18 | 1 | 0.88 | 1.17 | 47.2% | 51.9% |

- On frames that hold an event and the save, the PC overstates the event
  (ρ `ui_update` 0.76–0.94 by class median) and understates the save
  (ρ 1.17–1.38). The calibration's two frames (12 with no save, 5500 at
  0.92 / 1.11) read inside 0.85–1.15 and gave every class "high"; over all
  spike frames the save's ratio is outside it, which would make those
  classes "medium".
- Where the event branch and the save are near half each, the PSX gives
  the majority to the save: C2, C3, C8, C16 and C18 (median save 51.9–54.8%),
  C14 at an even split. By the PRD's rule these classes are settled (high
  label) and keep their PC paths; on the PSX their responsible
  `cart_update60` child is the save, as for C13. Their spread work changes
  little (the event branch and the save are each about half), but a
  do-less ranking should read U4 and U5 as at least as heavy as the event
  branch on them. Step 12 decides; step 10 can use these per-class ratios
  in place of the two-frame ones.
- On save-only frames (C5, C7) the save's ratio is 0.95–0.97, and
  `ui_update` (4% of update) reads half its PC share, a small node.
- Wormhole (C4) and game start (C6) are exact: ρ 1.00–1.02.

### Verification

| Step 9 "verified when" | Where | Result |
|---|---|---|
| Calibration gap recorded | "Calibration on update code" | per child and run: PSX hblanks, shares, ρ (`ui_update` 1.00 and 0.92, `save_at_frame_end` 1.11), gap in points (−5.5 to +3.2); overhead +2 / +1; PSX hblanks per PC µs 1.127 / 2.144; hand-off line for step 10; plus the per-class ratios over 165 spike frames above (addition) |
| Every spike frame in a class | "Classification" | 166 frames (judge update > 263), 42 over 1000, each in exactly one of 19 classes (list per class); none open, none unresolvable; C13 unsettled on PC, settled by the full run |
| Per class: responsible function, share with confidence | class table | 18 classes from PC with calibrated interval and label (17 high; C19 uncalibrated but settled; C6 read one level up); C13 measured on the full run (50.3–59.8%, high) |
| Per class: do-less candidate | "Do-less candidates" | U1–U8 cover every class; every class also has the spread candidate |
| Per class: frames if spread under 263 after draw's estimated levers | "Spreading" | every class does-not-fit with no lever; per-class counts under `step7` and `step7+8` below |
| Fallback | "Fallback: C13 on the full run" | one asserts-off full run (the step's only one), game state same, C13 settled |

Spread count per class, the median frame count of each bracket
(`N_low` at the larger in-game gain – `N_high` at the smaller), with the
does-not-fit frames in brackets; with no draw lever every class is
does-not-fit:

| Class | Responsible function | Share (confidence) | Candidates | `step7` | `step7+8` |
|---|---|---|---|---|---|
| C1 | `slipway_drop` | 54.5%, high | U1, U2, U4, U5, spread | 12 – 57.5 (4) | 10 – 24.5 |
| C2 | `ui_update` | 49.6–53.9%, high | U2, U4, U5, spread | 6.5 – 19 (6) | 5.5 – 18 |
| C3 | `ui_handle_button` | 49.6–53.9%, high | U2, U4, U5, spread | 6.5 – 43 (3) | 5.5 – 11.5 |
| C4 | `economy_recompute` (wormhole) | 70.5–76.6%, high | U2, U3, spread | 6 – 37 | 5 – 30 |
| C5 | `save_at_frame_end` | 91.1%, high | U4, spread | 5 – 22 | 5 – 8.5 |
| C6 | `entity_geom_nearest` | 96.2%, high | U6, U7 | 104 – 285 | 68 – 162 |
| C7 | `save_state_encode` | 60.1%, high | U4, spread | 4 – 9 | 4 – 6 |
| C8 | `ui_send_to_hovered` | 47.6–51.7%, high | U2, U4, U5, spread | 7 – 55 (1) | 6 – 26.5 |
| C9 | `economy_advance_clock` (project) | 57.7–62.7%, high | U2, U4, U5, spread | 9 – 45.5 | 8 – 19.5 |
| C10 | `planet_apply_project` | 55.0–59.8%, high | U2, U4, U5, spread | 10 – 58 (2) | 8 – 36 |
| C11 | `slipway_drop_method` | 48.8–53.0%, high | U1, U2, U4, U5, spread | 8 – 18 | 8 – 13 |
| C12 | `economy_recompute` (project) | 73.5–79.9%, high | U2, U5, spread | 10 – 69 | 8 – 15 |
| C13 | `save_at_frame_end` (measured) | 50.3–59.8%, high (measured) | U4, U5, U2 (event branch), spread | 6 – 58 | 5 – 12 |
| C14 | `button_left_press_method` | 48.3–52.5%, high | U2, U4, U5, spread | 5.5 – 45.5 | 5 – 9.5 |
| C15 | `economy_recompute` (year skip) | 62.7–68.2%, high | U2, U4, U5, spread | 9 (1) – none fits (2) | 9 – 134 (1) |
| C16 | `button_left_press` | 48.3–52.5%, high | U2, U4, U5, spread | 7 – 70 | 5 – 16.5 |
| C17 | `economy_recompute` (structure) | 53.5–58.1%, high | U2, U4, U5, spread | 9 – none fits (1) | 8 – 20 |
| C18 | `economy_new_year` | 46.2–50.2%, high | U2, U4, U5, spread | 7 – 53 | 6 – 21 |
| C19 | `update_entities` | 72.6%, uncalibrated | U8 (spread) | 25 – 77 | 24 – 28 |

The one frame left does-not-fit under the larger gains (6043, C15) is 16
frames from the run's end; that is the run's end, not the class.

### Patches (update spikes)

Both written by hand into `tmp/plan20-update/` (git-ignored), used once
and discarded; neither is committed:

| Patch | Base | Use | Change |
|---|---|---|---|
| `u-calibrate.patch` | `3fe4b8b` | probe `u-calibrate`, `start` and `update-5500` | see "Calibration on update code" |
| `u-fallback.patch` | `94e6895` | one `psx-full-game-check PSX_FULL_GAME_ASSERTS=off`, temporary worktree | `psx/CMakeLists.txt`: `PSX_UPDATE_SPANS` and `platform/psx_update_spans.c` + `psx_fine_clock.c` on `psx-gpu-replay` only; `src/game/cart.c`: the five statements of `cart_update60` in `SPAN(i, …)`; new `src/platform/psx_update_spans.[ch]`: the calibration's spans, records of frames whose spans sum to 200 hblanks or more (512 max), `probe-update:` lines after the last frame; `src/cmd/psx_gpu_replay.c`: start after the frame timer, close a frame after `frame_times_record`, write before the done line |

## Ordinary update

Plan 20 step 10: update on the judge's 5,893 frames whose update is 263
hblanks or less, the complement of "Update spikes"' 19 classes (77% of the
run's update). This section is being written chunk by chunk; for now it
holds the calibration of PC update proportions on ordinary frames, its
transfer check against step 9, and the descent with each node's cost.

Measured on commit `078d9664b6349cef5e5d850860e465147bf954bc` (pcsx-redux,
asserts off), with no change to the game.

### Step 9, as committed

- The 166 classified frames are exactly the judge frames with update > 263,
  each in one of 19 classes; the ordinary frames are selected by
  update ≤ 263 on the judge table.
- PC profiles use K = 8 (step 9's deviation from 4; the Makefile's default
  stays 4).
- Step 9's ratios, against `cart_update60` itself: `ui_update` 1.00
  (frame 12) and 0.95 (frame 5500), the save 1.14 (5500);
  `update_entities`, `frame_zone_reset` and `cart_flush_exit_queue`
  "below" on both. Per class over 165 spike frames: on event-and-save
  frames `ui_update` 0.76–0.94 and the save 1.17–1.38, on save-only
  frames the save 0.95–0.97 and `ui_update` 0.48–0.54, wormhole and game
  start 1.00–1.02. Only shares carry over, not PC times.
- About 4 hblanks of a title frame's update and about 40 of an in-game
  frame's are outside `cart_update60`, in the replay's `input_step_run`,
  which the shipped `psx-game` runs every frame too.

### Method

- **Population:** the judge frames with update ≤ 263, plus the windows
  `draw-5564` and `draw-6000` (the only short runs with no class frame).
- **Two roots:** the update part is `input_step_run` plus
  `cart_update60`; on PC each is profiled as its own update root (draw
  root `cart_draw`), and `P(f)` is the sum of their per-frame minima.
- **Six top-level pieces:** `input_step_run`, `frame_zone_reset`,
  `cart_flush_exit_queue`, `update_entities`, `ui_update` (with its
  `dispatch_context_mem` and `main_ui` arguments) and
  `save_at_frame_end`; `cart_update60`'s PC self (hook overhead, no code)
  is a seventh piece at ratio 1.
- **Calibration:** the PSX times the six pieces on every frame of three
  short runs; the PC profiles the same recordings; a piece's ratio is its
  PSX share of update over its PC share of `P`.
- **Transfer check:** the ordinary ratios against step 9's.

### Calibration on ordinary frames

Commands:

```
make psx-probe-baseline PSX_SHORT_RUNS="start draw-5564 draw-6000"
make psx-probe PATCH=tmp/plan20-ordinary/u-ordinary-calibrate.patch PSX_SHORT_RUNS="start draw-5564 draw-6000" PSX_PROBE_LABEL=u-ordinary-calibrate
for r in start draw-5564 draw-6000; do for root in cart_update60 input_step_run; do for k in 1 2 3 4 5 6 7 8; do
  d=tmp/frame-profile/ordinary-calibration/$r/$root/$k; mkdir -p $d
  STATE=$d/state.txt ./tmp/frame_profile tmp/psx/short-runs/$r/run.json $d tmp/frame_profile.syms cart_draw $root
  sh tools/end_state.sh check tmp/psx/short-runs/$r/run.json $d/state.txt tmp/psx/short-runs/$r/run.end-state
done; done; done
for r in start draw-5564 draw-6000; do
  awk -f frames.awk tmp/psx/probe/u-ordinary-calibrate/$r/boot.tty.txt \
    tmp/frame-profile/ordinary-calibration/$r/cart_update60/*/frames.txt \
    tmp/frame-profile/ordinary-calibration/$r/input_step_run/*/frames.txt > $r.frames.txt
done
awk -v set=start-title -v frames=0-11 -f pool.awk start.frames.txt
awk -v set=start-game -v frames=13-99 -v skip=58,91 -f pool.awk start.frames.txt
awk -v set=draw-5564 -v frames=14-53 -f pool.awk draw-5564.frames.txt
awk -v set=draw-6000 -v frames=14-122 -v skip=52 -f pool.awk draw-6000.frames.txt
awk -v set=pooled -v frames=0-99999 -f pool.awk pooled.sel   # the three in-game selections concatenated
```

The probe baseline was rebuilt first: its fingerprint was stale (`src/`
changed in `src/cmd/frame_profile.c` only, which the PSX does not build),
and its window parts came back unchanged (`start` 8980 / 465 / 6,
`draw-5564` 655 / 19491 / 118, `draw-6000` 593 / 13987 / 107, update /
draw / submit).

`frames.awk` joins per frame the boot's `psx-frame:` update, the six
`probe-update:` spans and each PC piece's minimum over the 8 profiles of
its root (a child absent from a profile counts 0; `ui_update` is the sum
of its three call lines before the minimum). `pool.awk` sums a frame
selection: PSX share = Σ span ÷ Σ PSX update (the boot's own frame
lines), PC share = Σ piece ÷ Σ `P`, ρ = PSX share ÷ PC share, gap = PSX
share − PC share in points, "below" when both shares are under 2%; the
"against `cart_update60`" ratios use the five spans' sum and
`cart_update60`'s PC time instead. Both scripts are throwaway, kept with
the patch in `tmp/plan20-ordinary/` (git-ignored).

Calibration frames (run frames):

- `start-title`: 0–11; `start-game`: 13–99 less 58 (C5) and 91 (C19),
  checked on the judge table (`start` is a prefix run: these are the
  judge's own frames).
- `draw-5564`: 14–53, the frames after the resume's first game frame (13,
  from `map.txt`), lead-in and window.
- `draw-6000`: 14–122 less 52. **Deviation:** the PRD took every frame
  after the resume's first game frame as ordinary, but run frame 52 is
  full-run frame 5944, a C3 frame on the judge (update 1004), and the
  resumed run replays it (1004 on the probe boot). It is left out, so no
  class frame enters the ratios. No other lead-in or window frame of
  either run maps to a judge frame over 263.

Validity: the three boots `valid`, `game-state same`; all 48 PC profiles
`same-game yes`. The spans' overhead on the window update is +1 on `start`
(8980 → 8981, one frame), +9 on `draw-5564` (655 → 664, +0.6 a frame) and
+12 on `draw-6000` (593 → 605, +0.8 a frame); draw and submit move by
3 hblanks or less. On every calibration frame the six spans sum to the
frame's update within the frame timer's whole-hblank resolution: the
remainder (update − spans) is −0.57 to +1.46, mean 0.47 over the 233
in-game frames and 0.54 over the 12 title frames; the 27 frames a little
negative (−0.57 at worst) are the timer's truncation, never a whole hblank,
so no span is missing or counted twice.

Per run and piece, pooled over the run's calibration frames (PSX hblanks a
frame; PSX share of the PSX update; PC share of `P`; "self" is
`cart_update60`'s PC self):

| Run (frames) | Piece | PSX hblanks a frame | PSX share | PC share | ρ | Gap |
|---|---|---:|---:|---:|---|---:|
| `start-title` (12) | `input_step_run` | 4.21 | 46.8% | 45.8% | 1.02 | +1.0 |
| | `frame_zone_reset` | 0.05 | 0.6% | 1.0% | below | −0.4 |
| | `cart_flush_exit_queue` | 0.08 | 0.9% | 1.3% | below | −0.4 |
| | `update_entities` | 1.83 | 20.3% | 22.2% | 0.92 | −1.8 |
| | `ui_update` | 2.24 | 24.9% | 24.2% | 1.03 | +0.7 |
| | `save_at_frame_end` | 0.04 | 0.4% | 0.5% | below | −0.1 |
| | self | — | — | 5.0% | 1 (rule) | — |
| `start-game` (85) | `input_step_run` | 41.02 | 45.2% | 43.3% | 1.04 | +1.9 |
| | `frame_zone_reset` | 0.05 | 0.1% | 0.2% | below | −0.1 |
| | `cart_flush_exit_queue` | 0.17 | 0.2% | 0.4% | below | −0.2 |
| | `update_entities` | 21.97 | 24.2% | 13.2% | 1.84 | +11.0 |
| | `ui_update` | 27.01 | 29.8% | 41.8% | 0.71 | −12.0 |
| | `save_at_frame_end` | 0.07 | 0.1% | 0.1% | below | −0.0 |
| | self | — | — | 1.1% | 1 (rule) | — |
| `draw-5564` (40) | `input_step_run` | 28.12 | 46.5% | 37.8% | 1.23 | +8.7 |
| | `frame_zone_reset` | 0.05 | 0.1% | 0.2% | below | −0.1 |
| | `cart_flush_exit_queue` | 0.12 | 0.2% | 0.4% | below | −0.3 |
| | `update_entities` | 8.60 | 14.2% | 17.4% | 0.82 | −3.1 |
| | `ui_update` | 23.06 | 38.1% | 42.0% | 0.91 | −3.8 |
| | `save_at_frame_end` | 0.06 | 0.1% | 0.1% | below | −0.0 |
| | self | — | — | 2.1% | 1 (rule) | — |
| `draw-6000` (108) | `input_step_run` | 15.42 | 31.9% | 27.4% | 1.16 | +4.5 |
| | `frame_zone_reset` | 0.05 | 0.1% | 0.2% | below | −0.1 |
| | `cart_flush_exit_queue` | 0.07 | 0.1% | 0.3% | below | −0.1 |
| | `update_entities` | 7.77 | 16.1% | 17.7% | 0.91 | −1.6 |
| | `ui_update` | 24.53 | 50.7% | 52.8% | 0.96 | −2.1 |
| | `save_at_frame_end` | 0.06 | 0.1% | 0.1% | below | −0.0 |
| | self | — | — | 1.5% | 1 (rule) | — |
| pooled in-game (233) | `input_step_run` | 26.94 | 40.9% | 36.3% | 1.13 | +4.5 |
| | `frame_zone_reset` | 0.05 | 0.1% | 0.2% | below | −0.1 |
| | `cart_flush_exit_queue` | 0.11 | 0.2% | 0.3% | below | −0.2 |
| | `update_entities` | 13.09 | 19.9% | 15.6% | 1.28 | +4.3 |
| | `ui_update` | 25.18 | 38.2% | 46.0% | 0.83 | −7.8 |
| | `save_at_frame_end` | 0.07 | 0.1% | 0.1% | below | −0.0 |
| | self | — | — | 1.4% | 1 (rule) | — |

PSX update a frame over the selection: `start-title` 9.00, `start-game`
90.74, `draw-5564` 60.50, `draw-6000` 48.38, pooled 65.91. None of the
calibration frames is a save frame (`save_at_frame_end` ≥ 2% of `P`).

**The input step on the PSX** (span around `input_step_run`, hblanks a
frame, mean / min / max):

| Frames | Mean | Min | Max |
|---|---:|---:|---:|
| `start-title` (0–11) | 4.21 | 3.20 | 7.09 |
| `start-game` (85) | 41.02 | 11.91 | 117.62 |
| `draw-5564` (40), of which the window 5564–5578 | 28.12; 15.04 | 13.51; 13.53 | 99.83; 20.25 |
| `draw-6000` (108), of which the window 6000–6014 | 15.42; 12.60 | 7.01; 7.11 | 46.63; 35.16 |
| pooled in-game (233) | 26.94 | 7.01 | 117.62 |

Step 9's "about 40" is the early game: 39.7–40.6 on most of `start`'s
frames 14–86. Late in the game the input step is smaller, 12.6 and 15.0 a
frame on the two draw windows (28% and 34% of their update), with spikes
to 47–118 on single frames.

**`update_entities` reads differently by kind of frame.** `start-game`'s
ρ 1.84 comes from frames 87–99 (less 91), where `update_entities` runs
about 122 hblanks a frame on the PSX (63% of update) but 34% of `P` on
PC; on frames 13–86 it reads 0.92, on the draw runs 0.82 and 0.91:

| `start-game` subset | Frames | `input_step_run` ρ | `update_entities` ρ | `ui_update` ρ | PSX update a frame |
|---|---:|---|---|---|---:|
| 13–86 less 58 | 73 | 1.21 | 0.92 | 0.82 | 73.9 |
| 87–99 less 91 | 12 | 0.66 | 1.86 | 0.46 | 193.3 |

Frames 87–99 surround C19's frame 91 (`update_entities`, step 9) and run
the same entity work at a smaller size; on them the PC understates
`update_entities` by half and overstates the rest.

**Ratios in use** (per piece, the pooled in-game ratio; label from the
spread of `start-game`, `draw-5564`, `draw-6000` with step 9's thresholds:
all within 0.85–1.15 high, within 0.67–1.5 medium, otherwise low; title
frames use `start-title`'s where it is not "below"):

| Piece | In game | Per-run spread | Label | Title |
|---|---|---|---|---|
| `input_step_run` | 1.13 | 1.04 / 1.23 / 1.16 | medium | 1.02 |
| `frame_zone_reset` | 1 (below) | below everywhere | uncalibrated | 1 (below) |
| `cart_flush_exit_queue` | 1 (below) | below everywhere | uncalibrated | 1 (below) |
| `update_entities` | 1.28 | 1.84 / 0.82 / 0.91 | low | 0.92 |
| `ui_update` | 0.83 | 0.71 / 0.91 / 0.96 | medium | 1.03 |
| `save_at_frame_end` | 1 (below); on save frames step 9's 1.14 | below everywhere | uncalibrated (save frames: transfer band) | 1 (below) |
| `cart_update60` self | 1 (rule) | — | — | 1 (rule) |

For chunk 4's `UPDATE_RATIOS`:
`input_step_run=1.13,update_entities=1.28,ui_update=0.83:0.76-0.94,save_at_frame_end=1.14:1.38-0.95`
(intervals used on save frames only, from the transfer check below). The
mode pairs interval ends by position across pieces, so the save's ends are
written high-low: `ui_update` 0.76 goes with the save's 1.38 and 0.94 with
0.95, as the transfer check pairs them.

### Transfer check against step 9

Ratios against `cart_update60` alone (input step left out), as step 9
gave them:

| Piece | `start-title` | `start-game` | `draw-5564` | `draw-6000` | Pooled in-game | Step 9: frames 12 / 5500 | Step 9: event-and-save classes | Step 9: save-only classes | Pooled against step 9 |
|---|---|---|---|---|---|---|---|---|---|
| `ui_update` | 1.18 | 0.74 | 1.07 | 1.04 | 0.91 | 1.00 / 0.95 | 0.76–0.94 | 0.48–0.54 | below the two frames, inside the event-and-save band, above save-only |
| `save_at_frame_end` | below | below | below | below | below | — / 1.14 | 1.17–1.38 | 0.95–0.97 | not measurable on ordinary frames |
| `update_entities` | 1.05 | 1.92 | 0.97 | 0.98 | 1.39 | below / below | — | — | no step 9 reference |
| `input_step_run` | — | — | — | — | — | outside the spans (4.06 on frame 12, about 40 in game) | — | — | measured: 41.0 a frame on `start-game`, 4.2 on title |

- `ui_update` on ordinary frames reads 0.74–1.07 by run, wider than step
  9's two frames; the pooled 0.91 sits in step 9's event-and-save band.
  Both steps agree the PC tends to overstate `ui_update`.
- `update_entities`, "below" on step 9's frames, is a quarter to a third
  of `cart_update60` on ordinary frames and reads from 0.97 to 1.92: near
  1 on quiet frames, near 1.9 on entity-heavy ones. Its "low" label
  carries into chunk 4.
- The save does not run on any calibration frame, so its ordinary ratio
  stays step 9's 1.14, and on save frames the `ui_update` and save costs
  carry the interval of step 9's bands (`ui_update` 0.76–0.94 with the
  save 1.38 at one end, 0.94 with 1.17 at the other, and the save-only
  0.95–0.97, whichever bounds are wider: save 0.95–1.38).
- Cross-check, outside the calibration: `draw-6000`'s run frame 52 (judge
  5944, C3) timed on the PSX reads `ui_update` 0.88 and the save 1.17
  against `cart_update60`, inside step 9's C3 ranges (0.78–0.90 and
  1.14–1.57).

### Descent and costs

Profiled on commit `f173cf1195228dfedea8fd91666e97dbd73f4890` (no code
change since `078d966`; 40 profiles, all `same-game yes`, 6 min 22 s):

```
make frame-profile-ordinary FRAME_PROFILE_REPEATS=8 UPDATE_RATIOS=input_step_run=1.13,update_entities=1.28,ui_update=0.83:0.76-0.94,main_ui=0.83:0.76-0.94,dispatch_context_mem=0.83:0.76-0.94,save_at_frame_end=1.14:1.38-0.95
```

- `main_ui` and `dispatch_context_mem` are `cart_update60` children of
  their own in the profile (the arguments of the `ui_update` call). The
  calibration counted them inside the `ui_update` piece, so they get its
  ratio and interval; they weigh 0.1 and 0.0 hblanks a frame either way.
- The mode takes one ratio set: the 12 title frames at the run's start
  use the in-game ratios, not `start-title`'s. Their update is 8 to 9
  hblanks a frame, under 0.03% of the population's.
- The descent profiled `pad_nav_step`, `ui_update` and `ui_update_hover`
  as update roots (two levels), with nothing unresolvable, and ended
  `nodes-above 5 leaves 2 open 0`.

**Population** (judge table; threshold 2% of the mean total, the lower of
that and the short-run threshold for the windows):

| Population | Frames | Excluded | Mean total | Threshold | Mean update | Save frames | Their share |
|---|---:|---:|---:|---:|---:|---:|---:|
| ordinary | 5,893 | 166 | 838.0 | 16.8 | 78.3 | 0 | 0.0% |
| `draw-5564` (5564–5578) | 15 | 0 | 1557.6 | 27.0 | 45.3 | 0 | 0.0% |
| `draw-6000` (6000–6014) | 15 | 0 | 930.5 | 18.6 | 39.8 | 0 | 0.0% |

The figures match the PRD's (5,893 frames, 838.0, 78.3, 16.8). No ordinary
frame is a save frame: `save_at_frame_end` never holds 2% of `P` outside
the spike classes, so every interval collapses to its point estimate and
no label is lowered.

**Tree** (hblanks a frame on the judge's update, mean and p90; PC calls a
frame, mean–max, from the first profile of the parent's set; runs-on in
frames; share of the population's update). Above in at least one
population, plus the heaviest child under each leaf and the nearest
misses:

| Node | Ratio, label | Population | Cost | p90 | Share | Calls (PC) | Per call | Runs-on | Verdict |
|---|---|---|---:|---:|---:|---|---:|---:|---|
| `update/input_step_run` | 1.13, medium | ordinary | 34.2 | 54.2 | 43.7% | 1.0–1 | 34.2 | 5893 | above |
| | | `draw-5564` | 13.8 | 20.5 | 30.4% | 1.0–1 | 13.8 | 15 | under |
| | | `draw-6000` | 12.3 | 31.0 | 30.8% | 1.0–1 | 12.3 | 15 | under |
| `…/input_step_run/pad_nav_step` | 1.13, medium | ordinary | 32.9 | 52.4 | 42.0% | 1.0–1 | 32.9 | 5893 | above, leaf |
| | | `draw-5564` | 12.8 | 19.3 | 28.3% | 1.0–1 | 12.8 | 15 | under |
| | | `draw-6000` | 11.2 | 29.7 | 28.2% | 1.0–1 | 11.2 | 15 | under |
| `…/pad_nav_step/map_step` | 1.13, medium | ordinary | 8.8 | 15.8 | 11.2% | 0.9–1 | 9.5 | 5457 | under |
| | | `draw-5564` | 0.0 | 0.0 | 0.0% | 0.0–0 | — | 0 | under |
| | | `draw-6000` | 3.6 | 6.7 | 9.1% | 1.0–1 | 3.6 | 15 | under |
| `update/cart_update60` | pieces' own | ordinary | 44.1 | 84.4 | 56.3% | 1.0–1 | 44.1 | 5893 | above |
| | | `draw-5564` | 31.5 | 32.2 | 69.6% | 1.0–1 | 31.5 | 15 | above |
| | | `draw-6000` | 27.5 | 30.0 | 69.2% | 1.0–1 | 27.5 | 15 | above |
| `…/cart_update60/update_entities` | 1.28, low | ordinary | 16.2 | 43.6 | 20.6% | 1.0–1 | 16.2 | 5893 | under |
| | | `draw-5564` | 14.4 | 15.0 | 31.8% | 1.0–1 | 14.4 | 15 | under |
| | | `draw-6000` | 10.8 | 11.5 | 27.2% | 1.0–1 | 10.8 | 15 | under |
| `…/cart_update60/ui_update` | 0.83, medium | ordinary | 26.5 | 47.6 | 33.9% | 1.0–1 | 26.5 | 5893 | above |
| | | `draw-5564` | 16.1 | 16.7 | 35.5% | 1.0–1 | 16.1 | 15 | under |
| | | `draw-6000` | 15.6 | 16.9 | 39.1% | 1.0–1 | 15.6 | 15 | under |
| `…/ui_update/ui_update_hover` | 0.83, medium | ordinary | 18.7 | 26.2 | 23.9% | 1.0–1 | 18.7 | 5893 | above, leaf |
| | | `draw-5564` | 14.9 | 15.3 | 32.9% | 1.0–1 | 14.9 | 15 | under |
| | | `draw-6000` | 15.7 | 16.3 | 39.4% | 1.0–1 | 15.7 | 15 | under |
| `…/ui_update_hover/planet_hit_test_method` | 0.83, medium | ordinary | 16.7 | 23.3 | 21.3% | 77.7–90 | 0.2 | 5881 | under |
| | | `draw-5564` | 12.1 | 12.6 | 26.7% | 74.0–74 | 0.2 | 15 | under |
| | | `draw-6000` | 13.3 | 13.8 | 33.3% | 76.0–76 | 0.2 | 15 | under |
| `…/ui_update/ui_handle_button` | 0.83, medium | ordinary | 7.3 | 29.0 | 9.3% | 1.0–1 | 7.3 | 5893 | under |
| | | `draw-5564` | 0.1 | 0.1 | 0.3% | 1.0–1 | 0.1 | 15 | under |
| | | `draw-6000` | 0.1 | 0.1 | 0.3% | 1.0–1 | 0.1 | 15 | under |

PSX call counts come from the delay probes. Every other node of the tree
is at 3.7 hblanks a frame or less in every population: `frame_zone_reset` 0.1,
`cart_flush_exit_queue` 0.2, `save_at_frame_end` 0.1, `main_ui` 0.1,
`dispatch_context_mem` 0.0; under `pad_nav_step`, `switch_panels` 3.7,
`finish_step` 3.4, `pause_step` 3.3, `armed_panel_open` 3.2,
`foreign_panel_open` 3.2, `select_allowed#1` 2.9, `pad_nav_panel_open`
2.9; under `ui_update_hover`, `structure_icon_hit_test_method` 1.5 (7
calls), `button_hit_test_method` 1.0 (4.6 calls), `frame_zone` 0.9 and
`entity_id_list_init` 0.8 (16 calls each). Self costs are 0.8 hblanks a
frame at most (`input_step_run` and `cart_update60` on ordinary frames):
no node hides a heavy body.

Reading the tree:

- **Five nodes are above, all on the ordinary population:**
  `input_step_run` 34.2, `pad_nav_step` 32.9, `cart_update60` 44.1,
  `ui_update` 26.5, `ui_update_hover` 18.7; each runs once a frame on PC.
  On the windows only `cart_update60` is above (31.5 against 27.0, 27.5
  against 18.6). The two leaves, `pad_nav_step` and `ui_update_hover`,
  spread their cost over many children, none above.
- **The input step is 43.7% of ordinary update** (34.2 of 78.3), and
  `pad_nav_step` is 96% of it. Its weight is spread over a dozen
  per-frame state tests of about 3 hblanks each and `map_step` (8.8, not
  called in `draw-5564`'s window).
- **`ui_update_hover` is almost all `planet_hit_test_method`**: 77.7 calls
  a frame (74–90), 0.2 hblanks each, 16.7 a frame, 0.1 under the
  threshold. That is one hit test per planet each frame, whatever the
  cursor did.
- **Near misses.** `update_entities` (16.2, 0.6 under) carries the "low"
  label (per-run ρ 0.82–1.84) and a p90 of 43.6: at its `start-game` ρ it
  would be above. `planet_hit_test_method` (16.7) is 0.1 under. Neither
  is explored, by the rule; both are named for step 12.
- **`ui_update_hover`'s children overshoot their parent**: they sum to
  21.7 against its 18.7 on ordinary frames (17.0 against 14.9 and 17.4
  against 15.7 on the windows), where every other explored node's
  children are within 0.9 of it. The children are read from the set
  rooted at `ui_update_hover`, whose 135-odd instrumented calls a frame
  carry hook time that the parent's minimum, taken in `ui_update`'s set,
  does not. Its children's costs are an upper bound by about 16%.
- `ui_handle_button` is 7.3 on average but 29.0 at p90: a press-driven
  node, under in both windows.

### Patches (ordinary update)

Written by hand into `tmp/plan20-ordinary/` (git-ignored), used once and
discarded; not committed:

| Patch | Base | Use | Change |
|---|---|---|---|
| `u-ordinary-calibrate.patch` | `078d966` | probe `u-ordinary-calibrate`, `start`, `draw-5564`, `draw-6000` | step 9's `u-calibrate.patch` (applied unchanged) plus a sixth span: `src/cmd/psx_gpu_replay.c` wraps the frame loop's `input_step_run` call in `psx_probe_span_begin` / `psx_probe_span_end(5)` under `PSX_PROBE`; `src/platform/psx_probe.c`: six spans, lines `probe-update: frame <n> input <h> zone <h> exit-queue <h> entities <h> ui <h> save <h>` (hundredths of an hblank) |
