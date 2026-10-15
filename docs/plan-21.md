# Plan 21 — Bring the PSX build to 60 fps, lever by lever

## 1. Goal

`frame-budget` green in `make psx-full-game-check`: every frame of
`worst-case/full_game.json` within 263 hblanks (one NTSC vsync) in
pcsx-redux, on the GPU path.

Plan 20 found and ranked the levers without implementing any. Its report,
`docs/psx-perf-levers.md`, is this plan's reference. The sections that
matter here are "Verdict and stop condition", "Levers ranked" and "Lever
entries": each lever there has a where (file:line), a gain, a cost and a
risk. Its verdict, on 6,059 judge frames:

- measured levers alone leave 6,047 frames over 263;
- measured plus estimated levers leave 2 over (on `draw-6000`'s lever set)
  or 0 (on `draw-5564`'s);
- with every gain cut by a fifth, 245 frames stay over.

So 263 is reachable on paper only if the estimated levers deliver. This
plan implements them in a fixed order, measures each one, and stops at
green or when the list runs out.

The target device is a Miyoo Mini Flip running PCSX-ReARMed at default
settings. It is never measured: the human will not run anything on it.
The contract stays pcsx-redux at 263.

## 2. Rules

### Locked decisions (plan 20's grilling, carried over)

- **Strict 60 fps.** Every frame, update + draw + submit, within 263
  hblanks. No percentile and no tolerance. A frame that cannot fit is split
  across frames.
- **Judge: pcsx-redux at 263, no margin.** The Miyoo is never measured,
  and the README and `docs/psx-game.md` say so. Raising PCSX-ReARMed's CPU
  clock option is rejected.
- **Spreading work across frames is allowed on PC and PSX alike**, with
  the `tests/e2e/*.sha256` files re-recorded once, explicitly. The game
  stays identical on both platforms; a PSX-only divergence is rejected.
  Galaxy generation, if spread, shows as a loading screen during which
  input is ignored.
- **The PSX picture stays pixel-identical to the PC's.** Every capture
  keeps its 0-pixel tolerance. A lever that cannot be made pixel-identical
  is skipped and noted. Exception: a change that only alters how much
  `visual_random` is consumed may change e2e hashes, once each changed
  screen is checked to differ from the old one by only a few pixels of
  visual noise, with the game state unchanged.
- **Asserts off** in `psx-game` and in the measured GPU replays, on in the
  PC build and the tests.

### Decisions of this plan's grilling

- **Order: safe gains first.** Phase 2 holds the measured, cheap levers:
  asserts off, no meter, `-O3`, and culling the map's off-screen cells.
  Then the draw levers in impact order, then update. Implementing in impact
  order from the start was rejected: the human wants certain gains before
  the large estimated jobs.
- **No checkpoint with the human.** The plan runs every step unattended. A
  lever that delivers less than its estimate is recorded in its step note,
  and the plan goes on. A "stop and ask if under half its estimate" rule
  was proposed and rejected.
- **The draw filter goes behind a compile-time define.**
  `draw_filter_install` (`src/cmd/psx_gpu_replay.c:199`) runs the cost
  model a second time on every draw call. The shipped `psx-game` does not
  install it, so the judge reads about 16% of draw high. It is compiled
  only under a define. The define is off in the measured replay and
  `psx-full-game-check`, and on only where `psx-check`'s captures need the
  filter's mask.
- **End of list.** When every step is done and `frame-budget` is still
  red, the plan stops and reports how many frames are over and by how
  much. Searching for new levers without limit was rejected.

### How each lever is implemented and judged

- Before a lever, check whether a lever already taken covers it. The
  report names these alternatives:
  - D5 replaces B3;
  - D11, D12 and D15 against B2;
  - L1 against L2;
  - O1 against O2;
  - D1/B4 against D2, D3 and D4.

  A covered lever is skipped, and its note says by which. These pairs
  are the report's estimator rule, not proof of coverage. "Covered", like
  step 15's "under 2%", is decided on the lever's target cost as it still
  stands after the levers already taken, measured on the short runs. The
  note records that figure.
- Each lever is measured on the short runs: asserts off, baseline before,
  run after, delta per part (`make psx-short-runs`, with the probe tool for
  baselines as in plan 20). The note gives the measured gain against the
  report's estimate, and the asserts mode and opt level the runs
  reported; from step 5 on, a figure not taken asserts off at O3 is
  invalid.
  A short run's `representative` condition compares the window with the
  last `psx-full-game-check`, so it is expected to fail once levers have
  moved the window by more than 10% since that run. That failure does not
  void the before/after figure; the note records it, and the other
  short-run conditions must hold.
- A lever whose measured delta on its target part is not a gain is
  reverted and noted. A reusable facility its step delivers stays (step
  8's image cache stays even if D5 goes).
- `make psx-full-game-check` runs at the end of each phase. Its
  `frame-budget` summary (frames over, worst, mean) goes in the step note.
  If `frame-budget` is green, the plan goes to its last step and skips the
  levers left. Every other `psx-full-game-check` condition must hold at
  each of these runs, and the note says so. Each run replaces
  `psx/perf/full-run-frames.txt`, so the next lever is measured against
  the run just before it.
- `make test-all`, `make e2e` and `psx-check` stay green after every
  lever, and every `*.sha256` stays unchanged, except in step 20.
- A lever whose risk is "e2e decides" (O1, O2, O3, U1, U2, U3) must leave
  every e2e hash unchanged. Otherwise it changed the game: revert it and
  note it.
- The report's timing-model caveat applies: pcsx-redux charges a flat 2
  cycles per instruction, with no instruction-cache miss or data wait. A
  gain that rests on code size or caches (labelled in the report, e.g.
  `-O3`) is real on the judge only. That is accepted.

### Out of scope

- Measuring on the Miyoo or PCSX-ReARMed.
- Audio.
- Changing the 263 budget.
- Any lever not in plan 20's report.

## 3. Phases and steps

### Phase 1 — Measure what ships

1. The draw filter is compiled only under a define, off by default in the
   measured GPU replay and `psx-full-game-check`, and on for `psx-check`'s
   capture replays. Verified when:
   - `psx-check` holds all its conditions with its captures unchanged;
   - a short run without the filter shows draw lower, by at most the
     report's upper bound for F1 (the no-meter delta: about −211 a frame on
     `draw-5564`, −198 on `draw-6000`). The note records the measured delta
     as F1's actual figure.
2. A new judge baseline: one asserts-off `make psx-full-game-check` with
   the filter off. Its frame table replaces `psx/perf/full-run-frames.txt`.
   Verified when the step note gives the frames over, the worst and the
   mean frame against plan 20's (6,047 / 9,451 / 857).

### Phase 2 — Safe gains

3. Asserts off in the shipped `psx-game` (G1), as already done for the
   measured replays (`PSX_MEASURED_REPLAY_ASSERTS`, `psx/CMakeLists.txt`).
   The measuring harness's defaults (short runs, full-game check, probe
   baseline) switch to asserts off with it. Verified when `psx-game` builds
   without `-UNDEBUG`, and `psx-check`'s
   scripted pad boot still plays what the PC plays.
4. The draw meter leaves `psx-game` and the measured replays (B1:
   `p8_draw_meter_install`, `src/cmd/psx_game.c:68`).
   `psx_gpu_frame_end` gets the buffer size instead of the metered bytes,
   and the backend's own overflow check stays. The PC replay keeps the
   meter for `make state`. `psx-check`'s zone and cap lines that came from
   the PSX meter get another source, and so do `psx-full-game-check`'s.
   Verified when `psx-check` and every `psx-full-game-check` condition but
   `frame-budget` are green
   with the same zone and cap figures, and the short runs show draw lower
   by about the measured no-meter delta (−211 a frame on `draw-5564`).
5. `-O3` for `psx-game` and the measured replays (G2). It reaches the
   compiler after the SDK's trailing `-O2`, as `PSX_MEASURED_REPLAY_OPT`
   does. The measuring harness's defaults switch to O3 with it. Verified
   when `tools/psx_opt_level.sh` reads O3 on every shipped
   and measured target, `psx-check`'s `ram-room` still holds, and the short
   runs show the measured −5.9%.
6. `p8_map` skips cells outside the screen (B3: `src/p8/gfx.c:133`, from
   `src/game/background.c:143`). Verified when every e2e hash is unchanged,
   every capture is unchanged, and the short runs show the `spr` calls
   drop (about 4,900 of 7,600 a window).
7. End of phase: `psx-full-game-check`, with its summary in the note.

### Phase 3 — Draw, in impact order

Each step implements one lever from the report's "Lever entries", at the
location given there. Before a lever that draws through the image cache,
check that a `psx-check` capture shows the cached element; if none does,
add one at 0 tolerance first. Each is verified by pixel-identical captures,
unchanged e2e hashes, and a measured short-run delta in its note. Order,
by judge-wide gain (the report's `draw-6000` figure):

8. D5 — pre-built background copied each frame (replaces B3). This step
   builds the off-screen image cache (runtime and both backends) as a
   reusable facility, proven pixel-identical here, before steps 11–13 and
   15's D2/D3 reuse it. If D5 itself is skipped, the step still delivers
   the cache; if the cache cannot be made pixel-identical, the note says
   so and the levers that need it are skipped citing that note.
9. D1/B4 — outlined font, one primitive per glyph (`hud_print_outlined`,
   `src/game/hud.c:85`). Taken instead of D2+D3+D4 if it covers them.
10. L1 — layer lists kept sorted at registration (`draw_layers`,
    `src/engine/entity_registry.c:199`).
11. D8 — disc and ring baked into the planet sprite.
12. D6 — planet row icons drawn once, through step 8's image cache.
13. D4 — score panel readouts drawn once, through step 8's image cache
    (skipped if step 9 covers it).
14. B2 — reject off-screen calls at the first test, or D12 + D11 + D15 if
    those are cheaper for the same calls.
15. The remaining draw levers above 2% of the judge's mean frame, in the
    report's order: D2, D3, D10, D9, D13, L3, G4 (D2 and D3 through step
    8's image cache). The 2% is of the mean frame in the latest
    `psx-full-game-check` summary (step 7's), and the note records the
    figure used. Each one is skipped and noted when covered or under 2%.
16. End of phase: `psx-full-game-check`, with its summary in the note.

### Phase 4 — Update

17. O2 — ask "is a panel open?" once per pad step (the twelve
    `pad_nav_panel_open()` sites of `src/game/pad_nav.c`), or O1 if cheaper
    (`pad_panel_open`, `src/game/pad_panel.c:51`). Then O3, hit-testing only
    the planets whose box can hold the cursor (`src/game/ui.c:345`), and
    O5. Each keeps every e2e hash.
18. Spike levers that keep the game state: U4 (table-driven CRC-32,
    `src/game/save_state.c:601`), U6, U2, U1, U3. Each keeps every e2e hash
    or is reverted and noted. Most spike classes appear in no short run,
    so these levers are measured class by class on the judge's spike frames
    (`psx/perf/spike-frames.txt`), each against the full run just before
    it; the short runs serve only where they hold the class. Then
    `psx-full-game-check`, with its summary
    and the spike frames still over in the note: steps 19–20 spread those
    frames, not the report's estimates.
19. Spreading mechanism, if frames are still over: the means to spread
    work over the following frames, on PC and PSX alike, landed inert —
    nothing uses it yet. It also fixes how input that arrives during a
    spread frame is treated, and how the recordings (e2e replays,
    `worst-case/full_game.json`, the short-run windows, the capture frames)
    follow the added frames. Verified when every e2e hash is unchanged and
    a test drives the mechanism on stand-in work: input during a spread
    frame follows the settled rule, and a stretched recording reaches the
    same end state.
20. Spreading applied: the spike work that does not fit is spread with
    step 19's mechanism, per the report's "Spikes and spreading" classes.
    Galaxy generation (frame 12) shows as a loading screen that ignores
    input. The e2e hashes are re-recorded once, in this step's own commit,
    with each changed screen checked. Verified when `psx-check` is green,
    the PC and PSX play the same game, each changed replay reaches the same
    end state as before, `full_game.json` still plays to its end, and the
    note lists the re-recorded hashes.
21. End of phase: `psx-full-game-check`, with its summary in the note.

### Phase 5 — Close

22. Close the plan:
    - Write down the result: `frame-budget` green, or the frames still over
      with their worst and mean.
    - Update `docs/psx-game.md`'s "Still to do" accordingly.
    - Say in the README that the Miyoo was never measured.
    - Delete `docs/psx-perf-levers.md`, which this plan used as its lever
      reference.

    Verified when `make test-all` and `psx-check` are green and the docs
    describe only what is true.

## 4. Done when

- `frame-budget` is green in `psx-full-game-check`, or every step is done
  and the note gives the frames still over, the worst and the mean.
- `make test-all` and `psx-check` are green. Every capture is unchanged.
- E2e hashes changed only in step 20, explicitly.
- `docs/psx-perf-levers.md` is removed and `docs/psx-game.md` is up to
  date.

## 5. Open questions

(none)

## 6. Step notes

### Step 1.1

- The GPU replay's draw filter compiles only under `PSX_GPU_REPLAY_DRAW_FILTER` (CMake option, default OFF, on the two GPU replays only). `psx-build` turns it ON for `psx-check`'s captures; `psx-full-game-check`, the short runs and the probe build it OFF. Without it a masked capture is a `p8_fatal`, and the replay prints `psx-replay: draw-filter on|off`.
- `psx-full-game-check` gained a thirteenth condition, `gpu-draw-filter`, requiring `off` (decided by the PRD agent; `psx-check` got no new condition, its masked captures already prove the filter is in).
- `psx-check` green, 31 conditions, every capture unchanged.
- F1 measured (asserts off, O2), filter removed, per frame: `draw-5564` draw −136.7 (−11.3%), `draw-6000` −121.3 (−14.1%); update and submit within ±0.3. Both under the report's upper bounds (−211.3 / −198.5). Game state same, runs valid.
- Deviation: chunk 2 was committed before the probe, so F1 was measured in reverse (baseline HEAD without the filter, a patch restoring it), sign flipped.
- The PSn00bSDK toolchain and pcsx-redux were installed in this session (`make psn00bsdk-toolchain-setup`, `make psx-emulator-setup`).

### Step 1.2

- Judge run `make psx-full-game-check PSX_FULL_GAME_ASSERTS=off` on `ad152e8`: asserts off, O2, draw filter off, 1,132 s. Every condition ok but `frame-budget` (13 conditions, `gpu-draw-filter` ok).
- Frames over 263: **6,047** of 6,059 (plan 20: 6,047). Worst: frame 12 at **9,392** (update 8,981, draw 404) vs 9,451. Mean frame **765** vs 857; overrun mean 503 vs 595.
- Draw total 3,994,334 vs 4,550,826: −12.2%, −91.8 a frame (F1 on the judge). Update and submit unchanged within timer jitter (≤3 hblanks a frame). The ten worst frames keep plan 20's order; the four spikes (12, 4737, 5500, 5719) only lost draw.
- `psx/perf/full-run-frames.txt` is this run's table. Plan 20's table moved to `psx/perf/report-judge-frames.txt`, read by `lever-estimate` and `lever-estimate-check` through `ESTIMATE_JUDGE_FRAMES` (PRD decision: their figures are plan 20's and break on a table whose update moved by a hblank). Delete it with the report in step 22.
- Zones and caps all ok; `gpu_frame` 65,028 of 73,728 (margin 0/8). `gpu-ram-room` 351,872 bytes left.

### Step 2.3

- **Human decision** (mid-run, overriding the PRD's "asserts off unconditionally"): `psx-game`'s asserts sit behind a switch, CMake `PSX_GAME_ASSERTS` / Makefile `PSX_BUILD_GAME_ASSERTS`, default `OFF`. Every `assert` stays in the source.
- `psx-game` prints `psx-game: asserts on|off`; `psx-check` gained `game-asserts` (32 conditions), which expects the mode `psx-build` was asked for (`PSX_GAME_ASSERTS` env var to the check scripts). `psx-check` green both ways, captures unchanged. No assert in `psx-game`'s own sources had a side effect.
- `psx-game` text 382,748 → 313,664 (−69,084, report: about −70 KB); `game-ram-room` 435,908 → 504,996 bytes left. No judge delta: the measured replays were already asserts off.
- `PSX_FULL_GAME_ASSERTS` and `PSX_SHORT_RUN_ASSERTS` default to `off`. One short run, no variable: asserts off in every boot, O2; done, deterministic, breakdown hold everywhere; `representative` fails on update-4737, update-5500, draw-5564, update-5719 (red before this step too: resumed runs lose the update spikes and draw lower).

### Step 2.4

- `psx-game` dropped the meter: each frame ends against `GPU_FRAME_BUFFER_BYTES`, `packet`'s overflow check stays. The backend keeps per-frame peaks (`psx_gpu_peaks`); `game-gpu-cost` now requires them within the PC replay's metered peaks for the same pad log (18,280 of 18,288 bytes, CLUTs 3 of 3, patterns 1 of 1). `psx-check` stays at 32 conditions.
- The GPU replay's meter compiles only under `PSX_GPU_REPLAY_DRAW_METER` (on in `psx-build` for `psx-check`'s captures, which keep the frame-by-frame proof that the model bounds the buffer; off in the judge, short runs, probe). The judge gained `gpu-draw-meter` (14 conditions) and takes its `gpu_*` zone and cap lines from the PC state (PRD decision: the backend sends fewer bytes than the model, so its own counts cannot give "the same figures").
- Short runs (asserts off, O2), draw per frame: `draw-5564` −210.9 (report −211.3), `draw-6000` −198.5 (−198.5); update and submit unchanged. `representative` fails on every run (draw moved >10% against step 2's table), other conditions hold.
- Judge (asserts off, no meter, no filter): all conditions ok but `frame-budget`; zones and caps equal step 2's; backend peak 62,328 of 65,028 metered bytes. Frames over **6,047** (unchanged); worst frame 12 at **9,268** (update 8,981, draw 281); mean **604** (765); overrun mean 341. Draw total 3,019,047, −161 a frame. Table replaced.
- `psx-game` text 313,664 → 309,548; `game-ram-room` 509,352 bytes left. The meter's code stays linked in the replays (`state_format` reads it), so their text shrank 4,052, not the report's 2,152-and-more.

### Step 2.5

- `psx-build` takes `PSX_BUILD_OPT` (O2/O3/Os, default O3) for `psx-game` (new `PSX_GAME_OPT`) and both GPU replays (`PSX_MEASURED_REPLAY_OPT`, default O3 now), through one `psx-opt-<level>` interface library linked after the SDK; `psx-build` reads every one back with `psx_opt_level.sh` and fails on a mismatch. `-Wno-error` dropped; O3 raised no new warning. Other targets stay O2.
- Harness: `PSX_SHORT_RUN_OPT` defaults to O3, `psx-full-game-check` passes O3, the probe builds O3 explicitly (another level makes a run invalid; fingerprint gains `opt O3`). New O3 probe baseline taken.
- `psx-check` green at O3, captures unchanged. RAM: `ram-room` 371,160 → 326,412, `game-ram-room` 509,352 → 469,652; full-game GPU replay at O3 (built, not booted) 316,256 bytes left. Text +40 KB per binary.
- Short runs, asserts off, O2 → O3, pooled window totals **−7.6%** (report −5.9%): update −13.8%, draw −4.7%, submit −3.2%. Draw per frame: `draw-5564` −42.4 (report −45.5), `draw-6000` −31.2 (−36.3); `start` −1,381 (−1,382). `representative` fails on every run, other conditions hold.
