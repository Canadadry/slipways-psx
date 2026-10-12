---
title: "Full-game frame budget conditions on the GPU path"
description: "Nobody knows how long each frame of a full game takes on the GPU path, or which part of a frame is to blame, so step 4 of plan 11 has nothing to optimize against."
status: needs-triage
---

## Problem Statement

Plan 11 wants a `.bin/.cue` in which a full game plays at the game's frame rate (one frame a vsync, 60 Hz) without running out of memory. Today the only timing figure on PSX is a total: each replay's `done` line prints the vsyncs of the whole run. Since step 1, `make psx-full-game-check` boots the GPU replay of `worst-case/full_game.json` to its end: `done full_game 6059 vsyncs 20540`, so 3.4 vsyncs a frame on average. The CD boot of the game averages ~1.7 (342 vsyncs for 199 frames). Nobody can say which frames are too slow, by how much, or whether the time goes to the game update, to building the draw calls, or to submitting to the GPU and waiting for it. The MIPS state prints the zone, cap and primitive buffer peaks, and `measurements.txt` collects them, but no condition compares them with their sizes over a full game on the GPU path. Step 4 ("optimize only what the measurement points to") cannot start until a measurement points somewhere. It also needs thresholds that it is not allowed to move.

## Solution

`make psx-full-game-check` gains frame-budget and size conditions on its GPU replay boot. Step 1 built that boot inside this target (`tools/psx_full_game_check.sh`, empty capture script, `gpu-replay-done` and `gpu-state-game-lines`). This step adds no new target. The check stays outside `make test-all`. After its existing seven conditions, it:

- prints one line per frame for all 6,059 frames, with the time spent in each of the three parts (game update, draw calls, GPU submit and wait), their total, and the vsyncs that went by during the frame;
- prints a summary: how many frames exceed one vsync, by how much (the worst frame, the mean overrun, the overrun distribution), and which part dominates the over-budget frames;
- prints each zone's peak against its size and each cap's peak against its size. That includes the GPU primitive buffer's peak (`zone gpu_frame`) against `GPU_FRAME_BUFFER_BYTES`, and the GPU replay ELF's RAM fit (`gpu-ram-room`);
- fails if any frame exceeds one vsync, if a zone, a cap or the zone budget exceeds its size, or if the GPU replay ELF does not fit in RAM. Every condition is printed before the check concludes.

The GPU replay gains a hardware frame timer: PSX root counter 1, counting horizontal blanks. The replay reads it at the boundaries of the three parts of every frame. It keeps the measurements in RAM during the run and writes them after the loop, so writing them never changes the frames they measure.

The `frame-budget` condition is expected red at this step, so `make psx-full-game-check` goes red until step 4 turns it green. The step closes when one run of the target ends on its own, with:

- the seven step-1 conditions green;
- the new `timer` and `gpu-ram-room` conditions green, which proves the measurement is valid;
- every new condition evaluated, and its figures reported for the step note.

## User Stories

1. As the step 4 implementer, I want the full-game check to measure every frame of a whole game on the GPU path, so that I know which frames are over budget before I touch any code.
2. As the step 4 implementer, I want each frame split into game update, draw calls, and GPU submit and wait, so that I optimize the part that actually takes the time.
3. As the step 4 implementer, I want the summary to name the dominant part of the over-budget frames, so that I can pick a lead without reading 6,059 lines.
4. As the step 4 implementer, I want the worst frame named with its parts, so that I can build a short GPU-path run around it.
5. As the step 4 implementer, I want an overrun distribution (how many frames are a little over, how many a lot over), so that I can tell a few spikes apart from a frame rate that is too slow throughout.
6. As the step 4 implementer, I want the frame budget fixed as a named constant at this step, so that I cannot pass the check by moving the threshold.
7. As the step 4 implementer, I want the full-game check to turn green exactly when every frame fits one vsync and every zone and cap fits its size, so that "step 2's check is green" is a precise done condition.
8. As the step 3 implementer, I want the MIPS zone and cap peaks of a full game on the GPU path printed against their sizes, so that I can re-size the zones from MIPS figures.
9. As the plan pilot, I want the new conditions on the existing full-game target, outside `make test-all`, so that the fast loop does not get slower and the full game is booted once, not twice.
10. As the plan pilot, I want the step-1 conditions, `timer` and `gpu-ram-room` to stay distinct from `frame-budget`, `zone-sizes` and `cap-sizes` in `conditions.txt`, so that an expected red budget is never mistaken for a broken run.
11. As the plan pilot, I want every condition printed before the check concludes, so that one red condition does not hide the others.
12. As the plan pilot, I want the figures for the step note (frames over one vsync, by how much, the dominant part) printed in a few summary lines, so that the note is copied, not computed.
13. As a developer, I want the per-frame measurements also written to a file in the output directory, so that I can diff two runs or plot them.
14. As a developer, I want the frame lines written after the loop, not during it, so that writing them over the TTY never shortens or lengthens the frames they measure.
15. As a developer, I want each frame to carry the vsyncs that went by during it, so that a frame longer than the 16-bit timer can count is detected instead of silently wrapping.
16. As a developer, I want the timer calibrated once at the start of the run (horizontal blanks counted over one vsync), so that a timer that does not count, or counts the wrong source, fails a condition instead of producing plausible zeros.
17. As a developer, I want the frame lines to use their own `psx-frame:` prefix, so that `psx_boot.sh`'s last progress line, the state comparison and the done-line parsing never mistake them for their own lines.
18. As a developer, I want a replay too long for the frame table to stop with a `p8_fatal` naming both counts, so that a longer recording never overwrites memory.
19. As a developer, I want a `p8_fatal` in the middle of a run (a zone or the primitive buffer overflowing) to still write the frames measured so far before the stopped state, so that I can see what led up to it.
20. As a developer, I want the instrumentation always on in the GPU replay, with no build flag, so that the EXE measured is the EXE `psx-check` proves.
21. As a developer, I want `make psx-check` to stay green with the instrumented GPU replay, so that the pad_slipway proofs are untouched.
22. As a developer, I want the frame-time arithmetic (16-bit wrap, part deltas, line format) host-tested in `make test`, so that a mistake there is caught without the emulator.
23. As a developer, I want the frame analysis host-tested on hand-written TTY files in `make psx-tools-test`, so that its verdicts and summary are proven without a six-minute boot.
24. As a developer, I want the GPU boot's ceiling and timeout rechecked against the first instrumented run, so that the extra output never makes a good run fail at the ceiling.
25. As a developer, I want the GPU primitive buffer's peak judged against its size in the same check, so that the plan's "primitive buffer size" is part of the verdict.
26. As a developer, I want the zone budget line (`budget zones <used> <total>`) judged too, so that the sum of the zones cannot outgrow its budget unnoticed.
27. As a developer, I want the frame table's RAM cost checked on the full-game GPU ELF, so that the measurement never makes the measured EXE run out of RAM.
28. As the human, I want `docs/psx-game.md` to describe the new conditions and lines, so that I can read a run without opening the scripts.
29. As the human, I want the red budget recorded as expected in the docs' "Still to do" until step 4, so that nobody "fixes" it by raising thresholds.
30. As the step 4 implementer, I want the measurement to be the GPU replay of the PC's own recording, so that the measured frames are the frames of the e2e source of truth.

## Implementation Decisions

**Facts from step 1 this PRD builds on** (note `### Step 1.1` of `docs/plan-11.md`, commits up to 7a826ee):
- The full-game abort came from the emulator: a Lua listener leaked stack, and the vsync listener overflowed it after ~32,700 vsyncs. `psx-emulator-setup` applies the fix as a patch, so every `psx_boot.sh` boot gets it. TTY volume played no part, and nothing in the guest changed. The GPU replay's frame loop keeps its order: input step, `cart_update60`, `cart_draw`, meter frame end, filter frame end, `psx_gpu_frame_end`, optional capture, `psx_gpu_frame_begin`, input advance, a progress line every 256 frames.
- `make psx-full-game-check` builds `full_game.json` (6,059 frames) into `tmp/psx/full-game/build` and writes the PC state.
  - It boots the software replay (ceiling 71,834 vsyncs, timeout 1,197 s; first run 47,889 vsyncs in 798 s).
  - It then boots the GPU replay with an empty capture script (`PSX_FULL_GAME_GPU_CEILING` 30,810 and `PSX_FULL_GAME_GPU_TIMEOUT` 515 s; first run 20,540 vsyncs in 343 s).
  - It checks seven conditions through `tools/psx_full_game_check.sh` and the shared `tools/psx_conditions.sh` (`pass`/`report`, one line each, and `run/conditions.txt`).
  - It writes `gpu ` lines (vsyncs, `zone gpu_frame`, `cap gpu_*`, ELF size) into `run/measurements.txt`. The whole target takes ~20 min.
- Step 1 added no per-frame TTY output to the GPU replay.

**What is measured, and on which EXE.**
- The instrumented EXE is the GPU replay (`psx-gpu-replay`) that `psx-full-game-check` already builds on `full_game.json` and boots. It draws through the backend that `psx-game` ships. It runs frames back to back with no vsync wait, so a frame's measured time is the time its work takes: the time one frame of `psx-game` must fit into. `psx-game` itself is not instrumented at this step.
- The instrumentation is always compiled into the GPU replay, with no build flag. `psx-check`'s GPU replay boots (pad_slipway) also write frame lines, which `psx-check` ignores.

**The timer.**
- PSX root counter 1, clocked by the horizontal blank, free-running, 16 bits, started once before the frame loop. The unit is one horizontal blank, ~63.6 µs: one NTSC vsync is 263 lines in the non-interlaced mode the backend sets. Rejected: counter 2 at system clock / 8 wraps every ~15.5 ms, which is shorter than one vsync, so every over-budget frame would alias.
- Calibration: before the frame loop, the replay waits for a vsync, reads the counter, waits for the next vsync, reads it again, and writes `psx-frame: hblanks-per-vsync <n>`. This adds at most two vsyncs before `start_vsync`, which are not counted in the `done` line.
- A part's time is the difference of two 16-bit readings modulo 65,536. A frame longer than 65,535 lines (~4.2 s, ~249 vsyncs) would alias. To catch that, each frame also records `VSync(-1)` at its start and its end, and the analysis fails `timer` on any frame that spans 249 vsyncs or more.

**The parts and the marks.** The GPU replay reads four marks in each iteration of its loop:
- m0 at the top of the iteration, before the input step;
- m1 after `cart_update60`: **update** = m1 − m0 (input step and game update);
- m2 after `cart_draw`: **draw** = m2 − m1 (the game's draw code and the backend building primitives, under the filter and meter wrappers);
- m3 after `psx_gpu_frame_begin`: **submit** = m3 − m2. It covers the meter and filter frame ends and `psx_gpu_frame_end` (CLUT and pattern uploads, the `DrawSync` wait on the previous frame, the display switch, `DrawOTag`), then `psx_gpu_frame_begin`.

The input advance, the capture handshake and the progress line come after m3 and before the next m0, outside every part. A frame's **total** is update + draw + submit. The progress line every 256 frames runs while the GPU draws the submitted frame, so the next frame's submit may wait a little less. This is accepted: it affects 23 of 6,059 frames, and the docs note it.

**The frame table**: a new pure module in `devtools/` (`frame_times`), built for the host and for MIPS.
- Interface:
  - init a table;
  - record one frame from its four 16-bit marks and its vsync count;
  - read a frame's record back (update, draw, submit, total, vsyncs);
  - write every recorded frame through a `state_sink_t` as `frame <n> update <u> draw <d> submit <s> total <t> vsyncs <v>`. That is the line sink the state already uses, so the TTY prefix stays the sink's business;
  - write the calibration line `hblanks-per-vsync <n>`.
- Storage: a static table of 8 bytes a frame (three 16-bit part times and a 16-bit vsync count). Its capacity is `FRAME_TIMES_CAP` = 6,848 frames: full_game's 6,059 plus 1/8, rounded up to a multiple of 64. That is 54,784 bytes of bss, in the GPU replay only.
- Recording past the capacity is a `p8_fatal` naming the frame and the capacity. Before the loop, the GPU replay checks `embedded_replay_frame_count` against the capacity and calls `p8_fatal` naming both, so a recording that is too long stops before frame 0.
- No `malloc` and no floating point, per the PSX build rules.
- The hardware reads (counter start, counter read, the vsync count) live in a small platform file (`psx_frame_timer`), not in the pure module.

**RAM cost.** The table adds 54,784 bytes to the GPU replay ELF, and the shipped `psx-game` ELF does not change. `ram-room` (kernel area + text + data + bss + `PSX_STACK_RESERVE` ≤ 2 MiB) is today a condition of `psx-check` only, on the pad_slipway GPU ELF. The full-game GPU ELF carries 6,059 embedded frames (~97 KB of constant data), and no condition checks its fit yet. So:
- `check_ram_room` moves from `tools/psx_check.sh` into `tools/psx_conditions.sh` (with its kernel-area, stack-reserve and RAM-size parameters), unchanged for `psx-check`.
- `psx_full_game_check.sh` checks a new condition, `gpu-ram-room`, on the full-game GPU ELF, and appends `gpu ram-room <used> <limit> <left>` to `measurements.txt`.
- The step's done condition needs `gpu-ram-room` green. If it is red, the step stops and reports the shortfall rather than shrinking the table (the table cannot shrink below 6,059 frames without dropping frames).

**Writing the lines.** A new TTY sink writes each line behind `psx-frame: `, next to `psx_tty_state_sink`. The GPU replay:
- writes the calibration line before frame 0;
- takes the `done` line's vsync count right after the loop, as today, then writes every frame line, then the `done` line;
- writes the frames recorded so far before the stopped state in its `p8_fatal` exit hook.

**The frame analysis**: `tools/psx_frame_budget.sh <gpu-tty> <gpu-psx-state> <expected-frames> <budget-hblanks> <out-dir>`. It is a pure analyzer of a finished boot's TTY and MIPS state, with no boot and no prefix of its own.
- It writes `<out-dir>/frames.txt` (a header line naming the columns, then one line a frame) and `<out-dir>/summary.txt`.
- It writes `<out-dir>/verdicts.txt`, one line per condition: `<condition> ok[ <detail>]` or `<condition> failed <reason>`. It always exits 0 once it has written the three files; a usage error exits 1.
- Conditions:
  - `timer`: the calibration line is present with n in 255..271; there is one frame line per frame from 0 to frames−1, in order; no frame spans 249 vsyncs or more. A failure names the first offending figure or frame.
  - `frame-budget`: no frame's total is above `<budget-hblanks>`. A failure names the count of frames over and the worst frame.
  - `zone-sizes`: every MIPS `zone <name> <current> <peak> <limit>` line, `gpu_frame` (the primitive buffer) included, has peak ≤ limit, and the `budget zones <used> <total>` line has used ≤ total. "Exceeds its size" means peak > limit. The 1/8 margin is the sizing rule of steps 3 and 4 (and of `make state` on PC), not a failure of this check, so the analyzer only prints it, as information.
  - `cap-sizes`: every `cap <name> <live|-> <peak> <limit>` line has peak ≤ limit. That covers the UI and particle slots, the exit queue, the entity table, the element and byte peaks, `gpu_cluts` and `gpu_patterns`.
- The summary, in integer arithmetic (`awk`), one figure per line:
  - `frames <n> budget <h> hblanks`;
  - `over <count> frames (<per mille>‰), worst frame <f> total <t> (+<t−h>) update <u> draw <d> submit <s>`;
  - `overrun mean <m> hblanks over the over-budget frames`;
  - `overrun distribution <=1/4 vsync <a> <=1/2 <b> <=1 <c> <=2 <d> >2 <e>`, overruns relative to 263 lines. The `<=2` and `>2` buckets are there because the average frame is already 3.4 vsyncs;
  - `parts over-budget frames update <U> draw <D> submit <S> dominant <part>`: sums over the over-budget frames, and the dominant part is the largest sum. It reads `dominant -` when no frame is over;
  - `parts all frames update <U> draw <D> submit <S> mean total <m>`;
  - every zone, budget and cap line with `ok` or `over` and the margin left in eighths.

**`tools/psx_full_game_check.sh` and the target.**
- After the GPU boot's two conditions, the script runs `psx_frame_budget.sh` on the GPU TTY and the GPU state extracted by `psx_state_compare.sh` (`gpu-state/`). It turns each `verdicts.txt` line into `pass`/`report` of `psx_conditions.sh`, in the order `timer`, `frame-budget`, `zone-sizes`, `cap-sizes`, then checks `gpu-ram-room`. That makes five conditions after the seven of step 1, twelve in all, every one checked before the script concludes.
- It prints every frame line on stdout behind `psx-full-game-check: frame ` and the summary behind `psx-full-game-check: `. It appends the summary to `measurements.txt` behind `gpu frame `. The last line becomes `all twelve conditions hold`.
- New arguments: the budget in hblanks, the kernel area, the stack reserve, the RAM size. The Makefile recipe passes `PSX_FRAME_BUDGET_HBLANKS := 263` (one NTSC vsync, fixed at this step: by the plan's rule, step 4 never changes it) and the `ram-room` values `psx-check` already uses.
- A full run already takes ~20 min. The plan's rule applies: it runs in the background and is waited on until it exits. Only a run that ended on its own counts.

**Ceilings and timeouts.** Each frame now costs four counter reads and two `VSync(-1)` reads, a few dozen cycles. The `done` line's 20,540 vsyncs should therefore barely move. The cost is after the loop: ~6,060 frame lines (~55 characters each, ~330 KB) go out through the BIOS `putchar` with IRQs masked, and the ceiling counts those vsyncs and the timeout counts that wall time. The boot today has 10,270 vsyncs and 172 s of headroom, so the first instrumented run keeps `PSX_FULL_GAME_GPU_CEILING` = 30,810 and `PSX_FULL_GAME_GPU_TIMEOUT` = 515 s. Afterwards:
- if the boot's whole vsync count (the `.progress` heartbeat or the boot log's last figure) or its wall time exceeds two thirds of its limit, both are reset to that run plus half again, the PSX convention, and the docs' figures are updated in the same commit;
- otherwise they stay, and the step note says so.
- A run that hits the ceiling is not evidence. The step then raises the ceiling and timeout to twice the current values, reruns, and sets them from that run.

**Docs.** `docs/psx-game.md` changes as follows:
- the GPU replay section gains the `psx-frame:` lines and the timer;
- the `psx-full-game-check` paragraph and its table row list the twelve conditions and the `gpu frame ` measurement lines;
- `check_ram_room` is listed among the shared conditions;
- "Still to do" records that `psx-full-game-check` is red on `frame-budget` until plan 11 step 4, with the first run's figures, and that the table's 54,784 bytes are counted in the GPU replays' `ram-room`, not in `game-ram-room`.

The Makefile legend for `psx-full-game-check` names the new conditions.

**Expected verdict at this step.** To close the step, these must be green: `replay-done`, `state-game-lines`, `sprite-sheet`, `screen`, `map`, `gpu-replay-done`, `gpu-state-game-lines`, `timer` and `gpu-ram-room`. `frame-budget` is expected red. `zone-sizes` and `cap-sizes` are whatever they measure. The implementer runs the target once to its end in the background and reports, for the step note:
- the number of frames over one vsync;
- the worst frame and its overrun;
- the overrun distribution and the dominant part;
- any zone or cap over its size;
- the `gpu-ram-room` bytes left;
- the boot's vsyncs and wall time, and whether the ceiling and timeout moved.

## Implementation Chunks

1. **Frame table module** (`devtools/frame_times`) and its host unit test in `make test`: recording from marks with a 16-bit wrap, reading back, refusing past capacity, the frame and calibration line formats through a sink.
2. **GPU replay instrumentation**: the hardware timer file, the `psx-frame:` TTY sink, calibration, the four marks, the capacity check before the loop, the lines after the loop and in the fatal hook. Verified by `make psx-build` and a green `make psx-check`: its GPU replay boots now carry frame lines, its `ram-room` still holds, and nothing else changes.
3. **Frame analysis** `tools/psx_frame_budget.sh` and its host test in `make psx-tools-test`, on hand-written TTY and state files. The test covers every verdict ok, then each verdict failed on its own case:
   - the calibration line missing or out of range;
   - a frame line missing or out of order;
   - a frame spanning 249 vsyncs;
   - one frame over budget;
   - a zone over its size, the budget over its total, a cap over its size.

   It also checks the summary's figures, buckets and dominant part on a known fixture.
4. **Full-game check wiring**: `check_ram_room` moved into `psx_conditions.sh` (with `psx-check` unchanged, `test_psx_game_check` and the psx-check tests still green), the five new conditions in `psx_full_game_check.sh`, the Makefile recipe and legend. `tests/test_psx_full_game_check.c` gets stub TTYs with `psx-frame:` lines and a stub `size` tool. It checks that twelve conditions are printed, that an over-budget frame turns only `frame-budget` red while the other conditions are still printed, that `gpu-ram-room` goes red on an oversized ELF, and that the `gpu frame ` measurement lines are written.
5. **Docs and the first run**: `docs/psx-game.md`, then one full `make psx-full-game-check` in the background to its end, the ceiling and timeout decision above, and the figures reported for the step note.

## Testing Decisions

- A good test drives the public interface and checks external behaviour. For the frame table, that is the records and lines it produces from given marks. For the scripts, it is their condition lines, exit codes, output files and summary on given TTY, state and stub files. No test reaches into a script's temporary files or the module's storage layout.
- `tests/test_frame_times.c` (one test per module, in `make test`, a few milliseconds; `make test` already measures ~10 s, and this adds nothing perceptible) covers:
  - part deltas across a counter wrap (65,530 → 4 is 10);
  - a frame whose total exceeds 16 bits as a sum of parts;
  - the vsync count carried through;
  - the refusal past capacity through `p8_fatal`: exit 3 and its message, checked in a child process as `tests/test_exit_queue.c` does for a full exit queue;
  - the exact line format.
- `tests/test_psx_frame_budget.c` is added to `PSX_TOOLS_TESTS` (`make psx-tools-test`, outside `make test-all`). Prior art: `tests/test_psx_state_compare.c` (hand-written TTY and state files in a work directory under `tmp/`, the script run through `system`, its output and files asserted).
- `tests/test_psx_full_game_check.c` is extended on its existing stub install, which prints a crafted TTY per PS-EXE and never boots an emulator.
- The hardware timer file and the GPU replay loop are not host-tested. The `timer` condition proves them on the emulator run (calibration in range, one line per frame), and `make psx-check` staying green proves the rest.
- No e2e changes: the game does not change, every `*.sha256` in `tests/e2e/` stays identical, and `make test-all` passes on every commit.

## Out of Scope

- Any optimization, any resizing of a zone or cap, and any change of a threshold (steps 3 and 4).
- Instrumenting `psx-game` (the shipped EXE), or measuring the CD boot's frame pacing.
- Splitting the submit part further (CLUT uploads versus the `DrawSync` wait), GPU-side timing, or per-draw-family costs.
- Making `make psx-full-game-check` green again (step 4), or adding it to `psx-check` or `make test-all`.
- Any change to the software replay boot of `psx-full-game-check` or to its conditions.
- Audio (out of scope for the whole port).

## Further Notes

- The budget is a line count, not a vsync count. The GPU replay never waits for a vsync, so the vsyncs a frame spans only bound its time to within one vsync, which is too coarse to say "by how much".
- The 20,540 vsyncs for 6,059 frames from step 1 already say the average frame takes ~3.4 vsyncs on the GPU replay. The summary's buckets above 1 vsync exist for that, and `frame-budget` will be red on most frames. A measured average far from 3.4 × 263 ≈ 890 hblanks means the timer or the marks are wrong: check `timer` and the calibration before reporting.
- If the run hits a `p8_fatal` (a zone or the primitive buffer overflowing on MIPS), `gpu-replay-done` fails, and the frames up to the stop are still in the TTY. The step then stops and reports instead of raising a size: by the plan's rule, a saturation is first treated as a leak.
