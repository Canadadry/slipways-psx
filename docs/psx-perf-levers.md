# PSX performance levers

This file will become plan 20's ranked inventory of the ways to make each PSX
frame do less work. For now it holds the baseline every lever is
measured against, the judge's asserts-off full-run baseline, the probe
tool's sanity checks and the PC frame profile's draw coverage. The full-run
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
