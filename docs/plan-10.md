# Plan 10 — PSX backend: GPU rendering and controller

Fourth of the five plans toward a PSX `.bin/.cue` (plans 7 to 11).

## 1. Goal

The full game compiled for MIPS, in a `.bin/.cue` that boots in the emulator, shows the title screen and is driven by the controller.

## 2. Rules

- **GPU rendering.** Each PICO-8 draw call (`cls`, `pset`, `line`, `rect`, `rectfill`, `circ`, `circfill`, `spr`, `map`, `print`, `fillp`, `palt`) becomes GPU primitives, instead of copying a 160×120 framebuffer. Sprites and the font live in VRAM.
- **Accepted divergence.** PSX rendering may differ from PC by a few pixels (circles, lines). The difference will be judged when we get there, not before.
- **Controller.** The PSX controller driver fills the PSX buttons of the input frame (`pad_buttons` in `p8_input_frame_t`, `src/p8/input.h`), as `keymap.cfg` does on PC. The pad navigation (`src/game/pad_nav.c`, with `src/game/pad_panel.c`) turns them into the game's input, unchanged.
- **The core doesn't move.** Only `src/p8/` and a new PSX `src/platform/` adapt.
- **Prerequisites.** Plans 7 to 9 done. Plan 7 left 1,980 bytes under the 768 KB of zones; the GPU primitive buffers live in the half it freed. Plan 9 produces a VRAM capture, not just TTY output: step 4 is checked against it.
- **Invariant for every commit.** `make test-all` passes, the `*.sha256` in `tests/e2e/` don't change, `make test` and `make e2e` don't get slower; `make sanitize-net`, `make full-game` and `make state` (with its 1/8 margin) pass too.

## 3. Phases and steps

### Phase 1 — PSX backend

1. **The core compiles for MIPS.** Gate first: plan 9's check target is green on `psx-hello` and produces a VRAM capture; if not, the run stops, with no workaround. Without rendering, it replays an `events.json` in the emulator, and the remaining libc calls (`fopen`, `snprintf`, `__divdi3`…) are replaced or provided. Done when the state after a real replay (ideally `full_game.json`), printed over TTY in the emulator, matches the PC's (`make state`) on the game lines (clock, planets, slipways, structures). An idle run on the title screen doesn't count: the minimum is a short e2e replay that reaches gameplay with planets and slipways, then `full_game.json`, or `full_game.json` recorded as pending for plan 11 if it can't run yet. The `zone` and `cap` lines are recorded as a first MIPS measurement for plan 11. The emulator is also shown to accept scripted pad input. Plan 9's check target switches from `psx-hello` to the game and checks this proof. Every later step adds its own proof to that target, and is done only when the target is green.
2. **Assets.** Sprites, map and font embedded without parsing JSON at startup.
3. **Separate the drawing API from its backend.** The current software renderer stays for PC and the e2e, and the GPU one is added for PSX. The PC e2e don't change.
4. **GPU primitives,** one family at a time, each checked by a VRAM capture in the emulator (plan 9), compared against the PC software framebuffer of the same frame. Done when each family's capture and comparison are listed in the step note as pending human acceptance; references are frozen in the check target only after that review.
5. **Controller** wired, then the title screen passed in the emulator through scripted input.
6. **The game's `.bin/.cue`** produced by a `make` target.

## 4. Done when

- The game's `.bin/.cue` boots in the emulator, shows the title screen and passes it through scripted controller input.
- The PSX check target is green.
- The human has reviewed the step 4 divergences family by family, and the accepted captures are frozen as references.

## 5. Open questions

- `fillp`, `palt` and the draw palette (blocks copied into the draw palette by `hud.c`) on the GPU: a pattern or a per-texture CLUT, or fall back to software for these cases. Decided before step 3, whose API split must allow the chosen option.
- Replaying an `events.json` in the emulator, to reuse the e2e. Decided at the start of step 1, which needs it, together with how scripted pad input reaches the emulator (or the fallback if it can't).
- Whether the GPU primitive buffers fit: primitives per frame counted headless on PC over the e2e and worst-case games, peak plus margin checked against the room `make state` measures at that moment, not a figure written in this plan. Decided before step 3.

## 6. Step notes

### Step 1.1

- Gate: `make psx-hello-e2e` green after both setups (five conditions, capture matches its sha256).
- Open question decided (PRD): the recording is embedded at build time (`tmp/embed_replay` writes a C source from the PC replay loader; one PS-EXE per recording), no `fopen`/JSON/CD on PSX. Scripted pad input goes through PCSX-Redux Lua `setOverride`/`clearOverride`, started on a scratchpad handshake; it works at the pin, no fallback.
- `make psx-build [EVENTS=]` builds the replay and the pad probe (`psx/CMakeLists.txt`, `src/platform/psx_*`, `src/cmd/psx_replay.c`, `psx_pad_probe.c`), `-O2` GPREL, asserts kept (`-UNDEBUG`, `--wrap=_assert_abort`). The state text moved behind a line sink (`src/devtools/state_format.c`); PC output byte-identical.
- libc: nothing missing (`__divdi3` from libgcc). One portability fix, outside the core: PSn00bSDK 0.24 `vsnprintf` overflows from 276447232 up, so `snprintf`/`vsnprintf` are wrapped onto a portable `p8_vsnprintf` (`src/p8/format.c`). No `src/game`/`src/engine` change.
- Proof: `make psx-check` (the plan 10 check target, ~50 s, outside `test-all`): magic, asserts-on, format, replay-done, state-game-lines (all 103 game lines of `pad_slipway.json` match), pad-script (9 states, 14 buttons). psx-hello targets stay as the toolchain smoke test.
- First MIPS measurement (pad_slipway): 401 frames in 2584 vsyncs (~6.4/frame, real-time emulator); text 331996, data 2868, bss 977224; zones game 52912, load 1904, frame 8232, ui 60096, particles 7325 peak; budget zones 793195/794584. Ceilings: replay 3906 vsyncs / 66 s, pad 306 vsyncs / 6 s (tight: emulator start is most of the 4 s).
- `full_game.json`: pending for plan 11. `make psx-full-game-check` exists; the emulator aborts (exit 134, "unknown exception") after ~9 min. With unbuffered stdout it reached frame 4608 of 4778, no `p8_fatal`, then aborted before `done`. Cause unknown; recorded in `docs/psx-game.md`.
- Deviations: the comparison script takes the expected frame count as a 4th argument; the probe ends on a Lua end word instead of "final state seen"; a `tmp/format_self_test` host tool is the PC side of `format`.
