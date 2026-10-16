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
- Phase 3 opens with the per-call costs re-measured: short runs at the
  current setup (O3, asserts off, no meter, no filter), then
  `make frame-profile-unit-cost` for today's per-family unit costs, the
  table in step 8's note. Every later "covers", "cheaper" or "under 2%"
  choice, and every "measured against estimate" line, uses those unit
  costs and cites both figures (step 2.6 found one report cost 4× high).
  Each lever step also opens with a throwaway probe of its gross target
  cost.
- A lever that adds a one-off cost on a short run's resume frame (an
  image-cache build) is kept or reverted on its delta without that frame.
  The one-off cost is recorded separately, with the judge frames it lands
  on, as input to phase 4's spike list.
- When a step extends code the latest `warn-review` flagged, its first
  chunk adds the missing unit test, or derives the hand-copied figure, in
  that code only. Nothing else is refactored.
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
   15's D2/D3 reuse it. It opens with a throwaway, uncommitted spike that
   builds the map into the VRAM region and copies it, hard-wired, compared
   at 0 tolerance against a map capture, before any PC-side wiring. If D5 itself is skipped, the step still delivers
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
    `psx-full-game-check`, with its summary, the frames still over split
    into three groups with count and overrun each: (a) those in
    `spike-frames.txt`, (b) the image-build frames from steps 8, 11, 12, 15
    and 16's notes, (c) every other frame;
    steps 19–20 spread those
    frames, not the report's estimates.
19. Spreading mechanism, if frames are still over: the means to spread
    work over the following frames, on PC and PSX alike, landed inert —
    nothing uses it yet. It also fixes how input that arrives during a
    spread frame is treated, and how the recordings (e2e replays,
    `worst-case/full_game.json`, the short-run windows, the capture frames)
    follow the added frames. The accepted GPU capture references and
    ceilings are re-keyed to their new frames by the run itself, hash and
    ceiling unchanged: the human's decision, an exception to
    `psx-gpu-capture-accept` being theirs alone. Verified when every e2e
    hash is unchanged and
    a test drives the mechanism on stand-in work that registers and
    unregisters an entity and changes a layer while a spread is pending,
    with no registry-view or `mem` address carried across frames: input
    during a spread
    frame follows the settled rule, and a stretched recording reaches the
    same end state.
20. Spreading applied: the spike work that does not fit is spread with
    step 19's mechanism, per the report's "Spikes and spreading" classes,
    over step 18's groups (a) and (b) only; the note says what is left in
    (c).
    Galaxy generation (frame 12) shows as a loading screen that ignores
    input. The e2e hashes are re-recorded once, in this step's own commit,
    with each changed screen checked. Verified when `psx-check` is green,
    `gpu-capture-references` and `gpu-capture-ceilings` pass with every
    hash and ceiling as before and only their frames moved, the PC and PSX
    play the same game, each changed replay reaches the same
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
- `make test-all` and `psx-check` are green. Every capture is unchanged (in step 20,
  only its frame number moves).
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

### Step 2.6

- B3 not covered by any lever taken (F1, G1, B1, G2); D5 comes in step 8. `p8_map` now loops over the visible cell ranges only, computed once a call with the cost model's 1×1-sprite test, which matches both backends' clipping. Contract line in `gfx.h`; tests in `test_draw_backend.c` (camera sweep against the old loop filtered by `p8_draw_cost_spr`) and `test_gfx.c` (pixels).
- E2e hashes, `make end-state` (25 replays) and every `psx-check` capture unchanged. `make state` peaks unchanged (`cap gpu_cluts` did not drop). `ram-room` 325,948.
- `spr` calls removed equal the report's exactly: `draw-5564` 4,935 of 7,541, `draw-6000` 4,830 of 7,707; one-frame runs 274, 309, 318, 209.
- Short runs at O3, draw: `draw-5564` −38.9 a frame (−4.3%), `draw-6000` −38.0 (−6.5%); one-frame runs −28 to −38. Report: 146–162 a frame. **Under a quarter of the estimate**: 0.12 hblank per skipped call against 0.45–0.49, the report having measured at O2 with meter and filter compiled in. Pooled six-run total −3.7%. Kept (a gain); no gate on the shortfall.

### Step 2.7

- Judge run on `fdf3028` (asserts off, O3, no filter, no meter, 823 s): every condition ok but `frame-budget`. Table replaced; `docs/psx-game.md` "Still to do" updated.
- Frames over **6,046** of 6,059 (step 4: 6,047; frame 3890 now at exactly 263). Worst: frame 12 at **7,853** (update 7,616, draw 231) vs 9,268. Mean **530.2** (604.1); overrun mean 267 (341). Totals: update −14.8%, draw −11.9%, submit −2.8%. Spikes 4737 / 5500 / 5719: 2,131 / 1,956 / 2,021.
- **2% of the mean frame, for step 15: 10.6 hblanks.**
- `gpu-ram-room` 315,792; zones and caps unchanged (`gpu_frame` 65,028 of 73,728).
- Phase 2 against plan 20: mean 857 → 530 (−38%), but frames over 6,047 → 6,046. Every frame but 13 still needs about twice the budget.
- `warn-review` (phases 1–2): 14 warnings, none on pointer rules — duplicated `asserts_enabled()`, the O3 default in five places, hand-copied condition counts, `#ifdef` stub blocks in `psx_gpu_replay.c`, no unit test for `psx_gpu_peaks`, a 3,000-line Makefile. Fed to `plan-review`; only those step 8 extends are acted on (rule below). The rest stay open.
- `plan-review` (accepted, method only): re-measure per-call costs before phase 3 and judge every choice on them; keep or revert cached levers without the resume-frame build; step 8 opens with a GPU copy spike; a step extending flagged code first adds its test or derived figure.

### Step 3.8

- **Unit costs re-measured** (O3, asserts off, no meter, no filter; pooled cost per call today / report): cls 0.109/0.188, pset 0.114/0.279, line 0.162/0.400, rect 0.327/0.640, rectfill 0.172/0.453, circ 3.567/4.096, circfill 3.160/3.139, spr 0.438/0.646 (per unit 0.468/1.880), print 1.514/1.892 (0.267 a glyph), pattern 0.240/0.601. Details in `be39841`'s message. Circles and print kept most of their cost; the rest fell to 40–70%.
- **Coverage:** no-map probe, gross target −32.1 (`draw-5564`) / −35.4 (`draw-6000`) a frame against the report's 221/224, above 10.6: D5 not covered by B3. The map issues 71–78 on-screen `spr` a frame (the PRD guessed 174–192). Map already shown at 0 tolerance by captures all 100, all 188, spr 234, all 265; none added.
- **warn-review prerequisites:** GPU peak bookkeeping moved to `psx_gpu_peaks.c` with a PC test; `psx-check` and `psx-full-game-check` print their condition count from `conditions.txt`.
- **GPU spike** (throwaway): pixel-identical at the first try. Findings in `3ea80c9`: mask bit (GP0 E6) needed to keep untextured black opaque; `packet()` must swap its target rather than branch (the branch cost ~10 a frame; the swap made `packet()` cheaper than before).
- **Image cache delivered** (reusable for steps 12, 13, 15): runtime API `p8_image_build`/`p8_image_draw`, map version stamped by `p8_mset`, software store (64 KiB, paid by the PSX ELFs too), PSX region (704,0) on its own 15-bit page with a double build buffer, meter `zone gpu_build`, filter rule (a copy passes only when the mask holds every family of its image). Pixel-identical on every capture.
- **D5 kept:** steady draw a frame without the build frame `draw-5564` −31.3, `draw-6000` −34.8, pooled **−33.0** (report 221/224; about 0.15 of it). One-off build about +130–150 draw (≈180 gross), landing on **judge frame 12 only** (galaxy generation; draw 231 → ≈358): input for phase 4's spike list. `GPU_BUILD_BUFFER_BYTES` 10,240 (make state peak 8,800). `zone gpu_frame` peak 63,652.
- RAM: `ram-room` 229,460, `game-ram-room` 376,244.

### Step 3.9

- D1/B4 taken first (no image cache needed, reaches every outlined string); D2/D3/D4 judged afterwards on their residual cost (PRD decision).
- Gross target (throwaway colour-pass-only probe): `draw-5564` −431.7, `draw-6000` −76.5 a frame. Captures all 100, all 188, all 265 already show the outline over non-black pixels; none added.
- `p8_print_outlined` (runtime, backend entry, software nine passes, cost model one SPRT a glyph over a box one pixel wider, every wrapper under `print`); PSX: outlined 5×7 cells on the font texture's unused lines 48–124, CLUT entry 2 black, `psx_text_place_outlined` tested on PC against the software output. `hud_print_outlined` makes one call.
- **Kept:** draw a frame `draw-5564` **−438.2**, `draw-6000` **−78.4**, pooled −258.3 (report −542.3 / −102.1; unit-cost estimate −398.1 / −113.9); one-frame runs −46 to −229. No one-off cost. `zone gpu_frame` peak 63,652 → 26,288. E2e hashes and every capture unchanged. `ram-room` 226,008, `game-ram-room` 373,596.
- Unit costs after (pooled): print 1.522 a call, 0.268 a glyph; others as step 8's within noise (cls 0.106, pset 0.129, line 0.157, rect 0.314, rectfill 0.190, circ 3.454, circfill 3.137, spr 0.448, pattern 0.253).
- Residuals (throwaway probes on top of the switch, `draw-5564` / `draw-6000`): **D2** −43.9 / −8.9, not covered; **D3** −36.1 / 0, not covered; **D4** −9.5 / −9.3, **covered** (under 10.6 on both): step 13 skips it citing this note.

### Step 3.10

- Gross target (throwaway probe sorting the layers twice, second copy in static scratch because the frame zone had only 3,920 bytes of room): draw-5564 +19.6, draw-6000 +18.9, start +9 a frame (report bound 153 / 95 / 23). **L2 skipped, covered by L1** (its overlay share 6.1 / 5.7).
- Every entity layer write goes through `entity_registry_set_layer` (12 constructors, the button key override, `slipway_inherit_infraspace`, the score panel's layer 13). The registry keeps one order per draw message (static, 512 entries, fatal beyond), updated at register / unregister / real layer change; `draw_layers` walks it. Two asserts (registry mutated during a draw; layer written behind the setter) never fired. Chunks 1 and 2 landed as one commit (same functions).
- **Kept:** probe, a frame: `draw-5564` draw −20.5 / total −20.8, `draw-6000` draw −20.1 / total −19.7; pooled draw −20.3, whole frame −20.2. Work moved into registration: update +27 on `start` (frame 12, whose total rises +16), +17 / +1 / +6 on the update runs: input for phase 4.
- `make state` (full game): `zone frame` peak 16,336 → 11,344. E2e hashes, end-state and every capture unchanged. `ram-room` 218,288, `game-ram-room` 365,892. Chunk 3's short runs were not rerun separately: the probe runs the same windows.

### Step 3.11

- Gross target (throwaway probes drawing each circle twice), a frame: disc +43.0 / +57.3, ring +21.6 / +26.3 on `draw-5564` / `draw-6000`, together 64.6 / 83.6 (report 62 / 82): not covered. Circles kept their cost, so the estimate held.
- Built: five 16×16 slots (disc, four rings), `planet_draw` copying them when no fill pattern is set, `GPU_BUILD_BUFFER_BYTES` 12,288 (map and planet images build on the same frame). Measured pooled steady draw **−63.6** a frame (−55.9 / −71.3), one-off build ≈ +25 on frame 12. Pixel-identical: differ 0 on every capture, e2e and end-state unchanged.
- **Skipped and reverted** (`guideline` decision): the copies set the PSX mask bit (bit 15) on the disc's black and the ring's pixels, invisible on screen but changing the raw-VRAM reference hash of seven captures (all 100, all 188, pattern 190, pattern 257, circfill 263, all 265, circ 314; with bit 15 masked all 14 hash as before). Black must carry bit 15 to stay opaque in a copy, so no copy-based D8 avoids it, and `make psx-gpu-capture-accept` is the human's command only. The working D8 is on branch `claude/plan-21-d8-pending`; the human takes it by merging it and accepting those seven references.
- **Consequence for steps 12 and 15:** a cached image whose content is not all sprite texels (untextured fills, lines, circles, print) will hit the same reference change. Sprite-only content (D6's rows) is safe: sprite texels already carry bit 15.

### Step 3.12

- Gross target (rows drawn twice): `draw-5564` +36.3, `draw-6000` +53.5 a frame (report 55 / 81): not covered. PC count over the full game and every e2e: widest row 39, at most 17 rows a frame, every row sprite-only.
- Built: 32 slots of 48×8 (PSX: five a band from (704,256), each on its own page by the unchanged `x − x mod 64` rule), a game-side `image_slot_t`, and `row_image_draw_at` (content-keyed, LRU over 32 slots, sprite-only rows, direct draw otherwise); `planet_draw` uses it. Every capture **and reference** unchanged (sprite texels already carry bit 15).
- **Kept:** draw a frame `draw-5564` −21.1, `draw-6000` −29.4, pooled **−25.2**; one-frame runs −25 to −28. Builds: ≈ +50 one-off on a resumed run's first frame; on the judge 81 builds on 58 frames, **all of them update-spike frames** (first at 113): input for phase 4. `gpu_build` peak 9,000 of 10,240 (no raise).
- RAM: `ram-room` 199,460, `game-ram-room` 347,804 (the software store grew 12,288 bytes).
- Merging `claude/plan-21-d8-pending` later now means rebasing D8's five slots onto this enum and layout.

### Step 3.13

- **Skipped:** D4 is covered by D1/B4. Step 3.9's residual probe left it at −9.5 / −9.3 a frame on `draw-5564` / `draw-6000`, under 10.6 on both. No source change.

### Step 3.14

- Ceilings (throwaway probes), draw a frame on `draw-5564` / `draw-6000`: B2 8.40 / 21.80, D11 7.30 / 10.75, D15 6.79 / 11.95 (report ≤25 / ≤75, 36 / 50, 36 / 48); none covered. **D12 skipped without a probe**: a sprite dot carries the PSX mask bit where `rectfill`/`pset` do not, so capture references would change. Pooled ceilings B2 30.2 vs D11 + D15 36.8: the D11 + D15 side built; B2 not built (fallback only if both failed).
- **D11 kept:** `p8_lines` + a `lines` backend entry; `slipway_draw_line` batches 4 segments. Draw a frame −0.93 / −1.73, pooled **−1.33** — a gain, so kept by the rule, but about 500 lines for one hblank: a candidate for the human to drop.
- **D15 kept:** `p8_psets` + a `psets` backend entry; `emitter_update` batches 32 points. Draw a frame −2.67 / −5.07, pooled **−3.87**.
- Both: e2e hashes, end-state, state peaks and every capture and reference unchanged. `ram-room` 195,188, `game-ram-room` 345,488.
- The batches keep one primitive and one clip per element, so they only remove per-call overhead; the probes' ceilings (cost of an extra rejected call) overstated that by 3–8×.

### Step 3.15

- Threshold **10.6** a window frame (2% of step 2.7's mean frame, 530.2). Gross targets (throwaway probes), a frame on `draw-5564` / `draw-6000` (report): D2 button text 29.0 / 4.3 (313 / 49); D3 message-box text 25.0 / 0.2 (268 / 0); D10 end points 16.5 / 24.4, halved 8.2 / 12.2 (14 / 12); D9 off-screen dispatch 18.3 / 18.1, upper bound (10 / 10); D13 7.4 / 8.2; L3 0.9 / 1.1; G4 submit 5.7 / 5.1. **D13, L3, G4 skipped** (under 10.6 on both).
- Hypotheses H-14-1 to H-14-10 rest on the `### Step 3.14` note (D11 and D15 kept, D12 skipped, B2 not built, no full-game check since step 2.7); chunk 0 did not itemise their outcomes.
- Spike: outlined glyph texels survive an image build and copy bit for bit, so text-only caching keeps the references. No capture showed D2 or D3 text: `all 233` added at ceiling 0 (five D2 labels, differ 0), its reference **pending the human's `make psx-gpu-capture-accept`**. **D3 skipped** (`guideline`): no `pad_slipway` frame shows message-box text.
- **D2 kept:** `text_image`, 32 slots of 128×8 keyed by string and colour; button rows, side tooltips and tooltip rows go through it. Draw a frame −13.5 / −0.9, pooled **−7.2**; build frame +11.
- **D10 kept:** `slipway_line_ends` once a draw. Draw a frame −7.5 / −11.2, pooled **−9.3**; update runs −10 to −12.
- **D9 kept:** `skip_draw_off_screen` on the six `planet_draw_method` classes (ascension classes and burst unflagged, overlays never culled). Draw a frame −4.1 / −4.0, pooled **−4.0** (bound 18.3 / 18.1).
- One-off builds for phase 4: on the judge 587 text builds on 167 frames, at most 12 a frame.
- All three: e2e hashes, end-state and every existing capture reference unchanged.
- Short runs (asserts off, O3): draw a frame `draw-5564` 326.9, `draw-6000` 362.0. Done, deterministic, breakdown and asserts hold; `representative` fails on every run (draw −30% to −68% against step 2.7's table), as expected. `ram-room` 156,548, `game-ram-room` 307,872; `gpu-ram-room` 154,480 (computed from the full-game GPU ELF, `psx-full-game-check` not run).

### Step 3.16

- Judge on a5c8d88 (asserts off, O3, filter and meter off): every condition holds but `frame-budget`. **3,942 of 6,059 frames over** (2.7: 6,046); worst frame 12 at 7,962 (7,853), update 7,659; mean frame **318.4** (530.2), mean overrun 102 (267). Totals: update +0.6%, draw −48.2%, submit −11.6%; draw still dominant over budget (1,009,267 of 1,439,214). Table replaced.
- New 2% of the mean frame: **6.4 hblanks**, for phase 4's "under 2%" calls.
- `gpu-ram-room` 154,480 (315,792); `gpu_build` 8,180 of 10,240; caps unchanged.
- Image builds (host count): map 1 on frame 12; rows 81 on 58 spike frames; text 587 on 167 frames, of which 166 are not in `spike-frames.txt` and 138 of those over 263, so phase 4's spike list must include them.
- H-15-6 failed, adapted: the tooling gained `zone gpu_build` in step 8; the condition count stays 14.

### Step 4.17

- Starting state S-1 to S-6 held (HEAD d16a6a3, threshold 6.4, no update unit-cost table, 12 `pad_nav_panel_open()` sites).
- Gross probes, update a frame on `draw-5564` / `draw-6000` (report, before G2): O2 0.87 / 4.40, pooled 2.63 (2.5 / 6.6); O1 0.00 / 3.27, pooled 1.63 (2.9 / 7.3); O3 12.5 / 12.6 (12.1 / 13.3); O5 0.53 / 0.53 (1.3 / 1.3). **O2 built first** (larger by 1.0, above the 0.5 tie margin).
- Host counts over `full_game.json`: 9.1 panel asks and 7.8 backdrop scans a pad step, 83% of steps with no panel open, against 0 and 3 such frames of 15 in the windows: judge-wide O2 is worth about 16 a frame (extrapolated). 53 in-step answer changes, all explained by the memo key.
- **O2 kept:** step-bracketed memo keyed by ui and economy ids, ephemerals count and a new registry generation. Pooled **−1.4** (+0.2 / −3.0); one-frame runs −16 on the update spikes.
- **O1 skipped, covered by O2:** residual 0.00 / 0.60.
- **O3 kept:** inline box test for planet hit tests in the hover pass, same bucket order. Pooled **−9.9** (−10.1 / −9.8).
- **O5 kept:** hover lists initialised on first write, one reader `ui_hovered_layer`. Pooled **−0.4** (−0.47 / −0.33).
- All: every e2e hash, end state, `make state` peak and capture reference unchanged; `all 233` still pending.
- Short runs (asserts off, O3): window update `draw-5564` 467 (621 at step 3.15), `draw-6000` 341 (538), i.e. −10.3 / −13.1 a frame for the step. Done, deterministic, breakdown and asserts hold; `representative` fails on every run, as expected. `ram-room` 154,940, `game-ram-room` 307,772.

### Step 4.18

- Starting state H-17-1 to H-17-12: H-17-3 failed in part (step 17 also changed `geom.h`/`geom.c`, `entity_geom.c`, `ui.h`, the Makefile); none of the functions this step touches changed, so U6's new function went next to `entity_geom_nearest` and left step 17's inline bodies alone. The rest held.
- Baseline judge on 74c0b4f (asserts off, O3, filter and meter off): **3,107 frames over** (3.16: 3,942), worst frame 12 at 7,961, mean **293.9** (318.4). Threshold stays 6.4.
- Host counts (all replays): 125 saves, one a frame, all on U4's frames; U6's early exit visits 30.2 of 70.8 planets a scan with the same answer on all 727 candidates; U2 70,608 equal signatures, all with equal keys, at most 4 needs; U1 175 drops, skipping the first advance changes no drop or end state; U3 152 launches, the recompute changes no figure or row. All five levers stood.
- Gains, judge update on the lever's spike frames against the run before it (host gross target; report allocated / bound):
  - **U4 kept** (table CRC-32): −15,697 on 131 frames, **−119.8** a frame (7,890; 10,375 / 11,851).
  - **U6 kept** (`entity_geom_any_within`): frame 12 **−3,960**, 7,961 → 4,000 (about 3,950; 3,980 / 7,963).
  - **U2 kept** (row signature, 32 bytes, capacity 4): −20,873 on 133 frames, **−156.9** a frame (30,545; 12,289 / 24,577).
  - **U1 kept** (`targeting_confirm_without_clock`): residual after U2 6,863, not covered; −6,428 on 39 frames, **−164.8** a frame (6,739 / 13,478).
  - **U3 kept** (`economy_tick_clock`; the refresh pass stays, the score panel readouts depend on it, which chunk 0's exactness check missed): residual after U2 5,854, not covered; −3,814 on 33 frames, **−115.6** a frame (5,446 / 10,892).
- All five: every e2e hash, end state and capture reference unchanged; `all 233` still pending.
- **Closing judge** on f14a576: every condition holds but `frame-budget`. **3,078 of 6,059 frames over**; worst frame 12 at 4,013, update 3,711; mean **285.4**, mean overrun 86. Update for the step −51,229 (−14.1%), draw and submit flat; draw dominant over budget (827,937 of 1,076,370).
- **Frames still over**, summed overrun 266,856:
  - (a) in `spike-frames.txt`: **141**, overrun 59,294, mean 420.5, largest 3,750 (frame 12); 25 of the 166 spike frames now fit.
  - (b) image-build frames: **119**, all text builds, overrun 12,061, mean 101.4, largest 300. List in `psx/perf/image-build-frames.txt`.
  - (c) the rest: **2,818**, overrun 195,501, mean 69.4, largest 286; mean draw 270.6 against update 55.5, beyond what spreading reaches.
- RAM: `ram-room` 152,528 (154,940), `game-ram-room` 305,800 (307,772), `gpu-ram-room` 152,408 (154,380).

### Step 4.19

- **Skipped, the human's decision** (after step 4.18). Of the 3,078 frames over, 2,818 are ordinary draw-bound frames that spreading cannot reach, and 107 of the 260 spike and image-build frames are over on draw + submit alone: steps 19–20 could bring at most about 153 frames under 263, and `frame-budget` would stay red. The human chose to close the plan and target draw in a later one. No source change; the e2e hashes were never re-recorded.

### Step 4.20

- **Skipped**, with step 4.19 (same decision).

### Step 4.21

- No source change since step 4.18's closing judge on f14a576, so that run stands as the phase 4 result: **3,078 of 6,059 frames over**, worst frame 12 at 4,013, mean 285.4, mean overrun 86; every other condition holds.

### Step 5.22

- Result: `frame-budget` still red, on the closing judge of step 4.18 (f14a576): **3,078 of 6,059 frames over 263**, worst frame 12 at **4,013** (galaxy generation; update 3,711), mean frame **285.4** (plan 20: 6,047 / 9,451 / 857), mean overrun 86. 2,818 of them are ordinary draw-bound frames (mean draw 270.6, update 55.5): the next work is draw. Spreading (steps 19–20) was not done, the human's decision.
- `docs/psx-game.md`: "Still to do" gives that result and says the Miyoo Mini Flip was never measured; the `psx-short-runs` item is cut to what still holds; the pending-acceptance list is cut to `all` f233; the links to `docs/psx-perf-levers.md` and the missing `docs/plan-19.md` are gone.
- README: a "PlayStation build" section; the contract is pcsx-redux at 263 hblanks, the Miyoo Mini Flip (PCSX-ReARMed) never measured.
- `docs/psx-perf-levers.md` deleted. Still citing it, left for a code change: the `lever-estimate` and `lever-estimate-check` targets with `psx/perf/report-judge-frames.txt` and `gains.txt`, `tools/lever_estimate_check.sh`, three `tests/fixtures/lever_estimate/` files.
- `make test-all` green; `psx-check` green, 32 conditions, `all 233` pending the human's `make psx-gpu-capture-accept LABEL=all`.
