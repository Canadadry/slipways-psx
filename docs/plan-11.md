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
