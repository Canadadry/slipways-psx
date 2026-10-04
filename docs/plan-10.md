# Plan 10 — PSX backend: GPU rendering and controller

Draft: run it through `plan-converge` before executing anything. Fourth of the five plans toward a PSX `.bin/.cue` (plans 7 to 11).

## 1. Goal

The full game compiled for MIPS, in a `.bin/.cue` that boots in the emulator, shows the title screen and is driven by the controller.

## 2. Approach

- **GPU rendering.** Each PICO-8 draw call (`cls`, `pset`, `line`, `rect`, `rectfill`, `circ`, `circfill`, `spr`, `map`, `print`, `fillp`, `palt`) becomes GPU primitives, instead of copying a 160×120 framebuffer. Sprites and the font live in VRAM.
- **Accepted divergence.** PSX rendering may differ from PC by a few pixels (circles, lines). The difference will be judged when we get there, not before.
- **Controller.** The button → mouse layer (`src/engine/button_cursor`), wired to the controller driver.
- **The core doesn't move.** Only `src/p8/` and a new PSX `src/platform/` adapt.
- **Prerequisites.** Plans 7 to 9 done. Plan 7 left 1,980 bytes under the 768 KB of zones; the GPU primitive buffers live in the half it freed. Plan 9 produces a VRAM capture, not just TTY output: step 4 is checked against it.
- **Invariant for every commit.** `make test-all` passes, the `*.sha256` in `tests/e2e/` don't change, `make test` and `make e2e` don't get slower; `make sanitize-net`, `make full-game` and `make state` (with its 1/8 margin) pass too.

## 3. Steps

1. **The core compiles for MIPS.** Without rendering, it runs N frames, and the remaining libc calls (`fopen`, `snprintf`, `__divdi3`…) are replaced or provided. Done when the state after N frames, printed over TTY in the emulator, matches the PC's (`make state`) on the game lines (clock, planets, slipways, structures). The `zone` and `cap` lines are recorded as a first MIPS measurement for plan 11.
2. **Assets.** Sprites, map and font embedded without parsing JSON at startup.
3. **Separate the drawing API from its backend.** The current software renderer stays for PC and the e2e, and the GPU one is added for PSX. The PC e2e don't change.
4. **GPU primitives,** one family at a time, each checked by a VRAM capture in the emulator (plan 9).
5. **Controller** wired, then the title screen passed in the emulator through scripted input.
6. **The game's `.bin/.cue`** produced by a `make` target.

## 4. Open questions

- `fillp`, `palt` and the draw palette (blocks copied into the draw palette by `hud.c`) on the GPU: a pattern or a per-texture CLUT, or fall back to software for these cases. Decided at the start of step 4.
- Replaying an `events.json` in the emulator, to reuse the e2e. Decided before step 5, which needs it.
