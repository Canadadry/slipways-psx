# Plan 11 — Measure, optimize, ship

Last of the five plans toward a PSX `.bin/.cue` (plans 7 to 11).

## 1. Goal

A final `.bin/.cue` in which a full game plays in the emulator at the game's frame rate, without running out of memory.

## 2. Rules

- The game doesn't change: every `*.sha256` in `tests/e2e/` stays identical, and `make test-all` passes on every commit.
- A saturation is first treated as a leak; a limit is raised only once a leak is ruled out.
- Size of a zone or cap = measured peak + 1/8, rounded up to a whole cell or slot. A ceiling proven by a largest-case test is sized on that ceiling, with no margin.
- The emulator replay is a dedicated target, outside `make test-all`, which must not get slower.
- Full-game emulator runs and the PSX setups outlast an agent's command time limit: they run in the background and are waited on until they exit. A run counts as evidence only if it ended on its own (the emulator's exit, or `psx_boot.sh`'s ceiling or timeout); a run the agent's command limit killed is never read as the abort and never closes a step.
- A step's done condition is verified on the targets it names, even outside `make test-all`. Where a step says it "stops and reports to the human", it ends as a failure of its PRD run (the verifier reports `FAILED:`), never as DONE: that is the stop plan-pilot halts on, and no later step starts on it.
- Measurements that judge the shipped game (frame time, primitive buffer) are taken on the GPU drawing path `psx-game` ships, not on the software `psx-replay`.

## 3. Phases and steps

### Phase 1 — Measure, optimize, ship

1. **Make the full-game replay finish in the emulator.** `make psx-full-game-check` is red: the emulator aborts around frame 4608 of 4,778 (see `docs/psx-game.md`, "Still to do"), cause unknown, guest or emulator. First move: reproduce the abort in a run much shorter than 9 minutes. Done when the target is green, its `done` line carries the PC's frame count, a GPU-path replay of `full_game.json` (the EXE step 2 instruments) also reaches its `done` line with that count, and the cause of the abort is found and recorded. If the cause is in the emulator and cannot be fixed from the repository, the step closes on the recorded cause plus a way that lets the later full-game runs reach the end. If the abort cannot be reproduced in a short run, or cannot be pinned to the guest or the emulator, the step stops and reports to the human what was tried, what was ruled out and the last frame reached; steps 2 to 5 wait on that decision.
2. **Measure the full game on the GPU path**, to the end: per-frame CPU time split into a few coarse parts (game update, draw calls, GPU submit and wait; new instrumentation: only total vsyncs exist today), each zone's peak, primitive buffer size. Delivered as a repeatable check on the GPU path (a target, or conditions on the full-game check, outside `make test-all`) that prints the per-frame measurements for the whole of `full_game.json` and fails when a frame exceeds one vsync or a zone or cap exceeds its size. Red is expected at this step (the game's CD boot already averages ~1.7 vsyncs a frame); step 4 turns it green. The note records how many frames exceed one vsync, by how much, and which part dominates.
3. **Re-measure the zones on MIPS**: slot sizes of the UI and particle zones. `button`, `row` and `particles_help_screen` are sized on their proven ceiling, with no margin. The largest-case tests are host-only today: the step states which figures come from the host tests and which from the MIPS `zone`/`cap` lines, or builds a MIPS run of those tests as a named part of the step. Sizes are shared by both builds today and MIPS peaks are lower (4-byte pointers): a size taken from MIPS must leave the PC build green on its own larger peak; the PRD settles how (sized per build, or expressed to follow pointer width) and the note records it.
4. **Optimize only what the measurement points to,** within the RAM left (`psx-check`'s `ram-room`). The measurement points to a frame that takes longer than 1/60 s, or a zone that exceeds its size. Leads noted, not committed to: the `p8_quarter_wave` table (64 KB), the `assert` strings, the number of primitives per frame. A memory lead, not measured: replace stored pointers with 16-bit indexes local to the dynamic array the object comes from (most references point into a bounded-size list or arena), instead of 32-bit addresses. Known reserves: `row` slots 1,176 → 1,120 reachable (~4 KB); `button` 14 → 12 slots if a test proves 12 in play (~12 KB). Accepted risk: the output queue, transit points, particles and registry reserves follow the size of `full_game.json`. Its PRD is not pre-written in parallel with step 3: before it, plan-pilot consults `plan-review` with the measurements of the step 2 and 3 notes, which may split this step into one step per flagged item. Each optimization is tried on a short GPU-path run holding the worst frames step 2 found; the full-game check runs to close the step. Step 2's thresholds are fixed at step 2; this step never changes them. Done when step 2's check is green; if the measurement flags nothing, the step closes with a note saying so. If the check is still red once the flagged items are done, the step stops and reports to the human the remaining over-budget frames and their dominant part, against step 2's note, without opening unflagged leads; step 5 waits on that decision.
5. **Ship.** The final `.bin/.cue`, with `make psx-check`, `make psx-full-game-check` and step 2's check green.

## 4. Done when

- `make test-all` passes, every `*.sha256` in `tests/e2e/` is unchanged, and `make psx-check`, `make psx-full-game-check` and step 2's check are green on the final `.bin/.cue`.
- Waits on the human: they finish a game in the emulator with the controller. The rendering differences they find are accepted or fixed one by one; a slowdown or a crash they find becomes a bug for `fix-bug`, or a new plan.

## 5. Open questions

## 6. Step notes

### Step 1.1

- Cause, in the emulator: each Lua event listener of the pinned pcsx-redux left two values on its Lua thread per event (`src/core/eventslua.cc`, `createListener`); the vsync listener overflowed LuaJIT's stack at ~32,700 vsyncs, outside a protected call → `An unknown exception occured`, exit 134. A BIOS-only boot aborts at the same vsync. Upstream HEAD still leaks and the next commits move to SDL3, so no re-pin: `tools/pcsx-redux-event-listener-stack.patch`, applied by `install_psx_emulator.sh` and recorded in the stamp (an older install must be set up again).
- Rungs: (1) sanitized PC replay on `full_game.json` clean; (2) old emulator: software boot aborted at vsync 32,700 (last progress `frame 4096`, last screen digest 4256, 546 s), GPU boot exited 0 at 20,568 vsyncs (under the limit); (3) a state saved at vsync 32,100 and loaded ran past the abort in 24 s, screen digests equal to the PC's; (4) the abort follows the emulator's own vsync count, not the guest; (5) patched build: BIOS-only boot reaches 40,000 vsyncs.
- Deviation: `full_game.json` is 6,059 frames, not 4,778 (it grew in later commits). Green `make psx-full-game-check`: software `done full_game 6059 vsyncs 47889` (798 s), GPU `done full_game 6059 vsyncs 20540` (343 s). Ceilings/timeouts: software 71,834 / 1,197 s, GPU 30,810 / 515 s.
- Added on the way: abort reporting with signal, last progress line and `.progress` vsync heartbeat; `psx_boot.sh --save-state-at`/`--load-state` (state loads at the first vsync, written with plain `io`: `Support.File` writes on another thread and truncated it); the GPU replay boot inside `psx-full-game-check` (seven conditions, shared `tools/psx_conditions.sh`, `gpu ` measurement lines). `make psx-check` green.
- `make test` measures ~10 s on this 4-core container, above CLAUDE.md's ~5 s; no step-1 test runs in it (all in `psx-tools-test`).

### Step 1.2

- Delivered as five conditions on `make psx-full-game-check` (twelve in all), not a new target: `timer`, `frame-budget`, `zone-sizes`, `cap-sizes`, `gpu-ram-room`. The GPU replay times each frame on root counter 1 (hblanks, calibrated at 263 a vsync) in three parts: update (input + `cart_update60`), draw (`cart_draw`), submit (frame end, `DrawSync`, frame begin; the capture handshake is subtracted). Lines `psx-frame:` printed after the loop; `tools/psx_frame_budget.sh` judges them. Budget `PSX_FRAME_BUDGET_HBLANKS` 263, fixed. Frame table 6,848 frames, 54,784 bytes, GPU replays only.
- First full run (ended on its own, ~20 min): only `frame-budget` red. 6,047 of 6,059 frames over one vsync; only frames 0–11 fit. Mean frame 891 hblanks (~3.4 vsyncs), mean overrun 629. Overruns: 2,210 frames 1–2 vsyncs over, 2,889 2–3, 733 3–4, 215 >4. Worst frame 12: 9,558 hblanks, update 9,074 (a one-off start spike over 36 vsyncs); next worst 4737 (2,876, draw 2,001), 5719 (2,773), 5500 (2,699).
- Dominant part: draw — 87% of over-budget time (update 637,892, draw 4,714,765, submit 46,380 hblanks), largest part in 5,956 frames, update in 91, submit in none.
- Zones and caps all fit on MIPS. Tightest: `budget zones` 793,195 / 794,584 (1,389 bytes left), `zone gpu_frame` 65,028 / 73,728, `cap particles_help_screen` 1/1, `elements_particles` 140/158, `elements_transit_dots` 19/24, `elements_ephemerals` 7/8, `gpu_cluts` 5/6, `button` 11/15, `gpu_patterns` 3/4. `gpu-ram-room` 175,928 bytes left.
- Boots: software 47,889 vsyncs, ~798 s; GPU `done` 20,543 vsyncs (timer +3), 21,000 boot vsyncs with the frame lines, ~350 s. The GPU boot crossed two thirds of its limits, so its ceiling/timeout reset to 31,500 / 525 s (supersedes the step 1.1 figures). The software boot sits at two thirds of its own (71,834 / 1,197 s).
- Decision (implementer, accepted): kernel area and RAM size defined once in `tools/psx_conditions.sh`, with `check_ram_room`, shared by `psx-check` and the full-game check.
