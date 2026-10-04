# Plan 11 — Measure, optimize, ship

Draft: run it through `plan-converge` before executing anything. Last of the five plans toward a PSX `.bin/.cue` (plans 7 to 11).

## 1. Goal

A final `.bin/.cue` in which a full game plays in the emulator at the game's frame rate, without running out of memory.

## 2. Rules

- The game doesn't change: every `*.sha256` in `tests/e2e/` stays identical, and `make test-all` passes on every commit.
- A saturation is first treated as a leak; a limit is raised only once a leak is ruled out.
- Size of a zone or cap = measured peak + 1/8, rounded up to a whole cell or slot. A ceiling proven by a largest-case test is sized on that ceiling, with no margin.
- The emulator replay is a dedicated target, outside `make test-all`, which must not get slower.

## 3. Steps

1. **Replay `full_game.json` in the emulator** to the end, measuring per frame: CPU time, each zone's peak, primitive buffer size. If plan 10 didn't deliver `events.json` replay in the emulator, that is the first job.
2. **Re-measure the zones on MIPS**: slot sizes of the UI and particle zones. `button`, `row` and `particles_help_screen` are sized on their proven ceiling, with no margin: rerun their largest-case tests on MIPS.
3. **Optimize only what the measurement points to,** within the margin plan 7 freed. The measurement points to a frame that takes longer than 1/60 s, or a zone that exceeds its size. Leads noted, not committed to: the `p8_quarter_wave` table (64 KB), the `assert` strings, the number of primitives per frame. A memory lead, not measured: replace stored pointers with 16-bit indexes local to the dynamic array the object comes from (most references point into a bounded-size list or arena), instead of 32-bit addresses. Known reserves: `row` slots 1,176 → 1,120 reachable (~4 KB); `button` 14 → 12 slots if a test proves 12 in play (~12 KB). Accepted risk: the output queue, transit points, particles and registry reserves follow the size of `full_game.json`.
4. **Ship.** The final `.bin/.cue`, and a full game played by a human with the controller.

## 4. Done when

The human finishes a game in the emulator. The rendering differences they find are accepted or fixed one by one; a slowdown or a crash sends us back to step 1.
