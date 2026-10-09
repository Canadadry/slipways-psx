---
title: "The core compiles for MIPS and replays an e2e in the emulator"
description: "Nothing proves yet that the game's C core runs on the PSX: build it for MIPS, replay a recorded game inside the emulator, and show that its state matches the PC's line for line."
status: needs-triage
---

## Problem Statement

The game runs on PC (raylib, software rendering), and the e2e replays plus `make state` describe exactly what it does. Plan 9 left a toolchain (PSn00bSDK 0.24, `mipsel-none-elf` GCC 12.3.0) and a headless emulator (PCSX-Redux with OpenBIOS) that boot `psx-hello` and capture its TTY and VRAM. Nothing links the two. Nobody knows whether the core builds for MIPS, whether it fits, or whether it plays the same game on a different machine (32-bit pointers and `long`, a freestanding libc with no `FILE`, no `atexit`, asserts compiled out in release builds). Every later step of plan 10 (assets, the GPU backend, the controller, the `.bin/.cue`) assumes it does. Plan 11 needs MIPS measurements and a way to replay an `events.json` in the emulator. Today there is no PSX check target for the game, so no later step can prove anything.

## Solution

Build the core, unchanged, as a PS-EXE that replays an e2e recording embedded at build time. It runs the same per-frame loop as the PC replay driver, rendering into the in-RAM software framebuffer and never showing it. At the end it prints the `make state` text over TTY and leaves the emulator with an exit code. A new check target, `make psx-check`, builds the PS-EXE, boots it headless and compares the game lines of its state with the PC's for the same recording. It writes the MIPS `zone`/`cap` lines and the executable's size as a first measurement for plan 11. A small probe PS-EXE driven by a Lua pad script proves that the emulator accepts scripted pad input. `make psx-check` is the plan-10 check target. Every later step adds its own proof to it.

The minimum proof is `pad_slipway.json`, a 401-frame e2e that reaches gameplay with 90 planets, two colonies and one slipway. Then `worst-case/full_game.json` is run through its own target. If it cannot finish yet (too slow under the interpreter, or a zone full), it is recorded as pending for plan 11, with the reason measured.

## User Stories

1. As the plan-pilot run, I want the plan-9 gate (`make psx-hello-e2e`) checked before anything else, so that a broken toolchain or emulator stops the run at once instead of being worked around.
2. As the plan-pilot run, I want the run to stop with a report when that gate is red, so that no step is built on an emulator that cannot capture.
3. As a developer, I want one make target that builds the game's core as a PS-EXE for a given recording, so that the MIPS build is reproducible from a clean checkout once both setups are installed.
4. As a developer, I want the PS-EXE built from the same source list as the PC engine closure, so that a file added to the core cannot be silently missing from the PSX build.
5. As a developer, I want the PSX build to fail with a message naming `make psn00bsdk-toolchain-setup` when the SDK is missing, so that the PSX build fails the same way the psx-hello targets do.
6. As a developer, I want the recording embedded in the PS-EXE at build time, so that the emulator needs no file system, no CD image and no JSON parser to replay it.
7. As a developer, I want the embedded frames produced on PC by the same input loader the PC replay uses, so that both machines replay exactly the same frames.
8. As a developer, I want the generator to embed a recording's `initial_save` and seed the in-memory save storage with it as the PC replay does, so that replays that resume a save behave identically.
9. As a developer, I want the generator to refuse a mouse recording or an unreadable file with a message naming it, so that a wrong input never becomes a silently empty replay.
10. As a developer, I want the PSX replay loop to be the PC replay loop (input step, `cart_update60`, `cart_draw`, input advance), so that the state can only differ through the machine, not through the driver.
11. As a developer, I want `cart_draw` kept in the MIPS replay even with no display, so that the random draws spent by drawing (the slipway drag spends some) keep the galaxy identical to the PC's.
12. As a developer, I want the state text produced by one formatter shared by PC and PSX, so that a difference in the comparison is a difference in the game, not in two printers.
13. As a developer, I want the PC `make state` output byte-identical after that formatter is shared, so that every existing state file, test and note stays valid.
14. As a developer, I want each state line printed over TTY behind a fixed prefix, so that it can be pulled out of a TTY file that also holds emulator log lines.
15. As a developer, I want the PSX replay to print a completion line with the number of frames run, so that a truncated or hung replay can never look like a match.
16. As a developer, I want a `p8_fatal` on PSX to print its message, then the `stopped p8_fatal frame <n>` line and the state, then end the boot with a non-zero code, so that a MIPS run that stops reads the same as a PC run that stops.
17. As a developer, I want a failed `assert` on PSX to print its location over TTY and end the boot with a non-zero code, so that a broken invariant ends the boot at once instead of hanging until the timeout.
18. As a developer, I want asserts compiled in on the PSX build and the PS-EXE to print that they are, so that the PSX build checks the same invariants as the PC `-O0` binary.
19. As a developer, I want the PS-EXE to leave the emulator itself when the replay ends, so that a boot lasts as long as the replay and not a fixed number of vsyncs guessed in advance.
20. As a developer, I want `make psx-boot` to keep its current behaviour for psx-hello, so that plan 9's targets and reference stay green.
21. As a developer, I want a boot in "until exit" mode to fail when it reaches its vsync ceiling or its timeout, naming the input and the last progress line, so that a slow or hung replay is diagnosed, not just reported as red.
22. As a developer, I want progress lines (frame count) and the vsync count at the end printed by the replay, so that the CPU cost per game frame on MIPS is measured for plan 11.
23. As a developer, I want the libc calls the core needs (`snprintf`, `strlen`, `memcpy`, `memset`, `strcmp`, `strcpy`, `memcmp`, 64-bit division helpers) either taken from PSn00bSDK or libgcc, or provided in `src/p8/` or the PSX platform when missing, so that the core links without changes.
24. As a developer, I want the PS-EXE to print a formatting self-test of every `printf` conversion the core and the state use, compared with the PC's output, so that a PSn00bSDK formatter difference is named directly and not found three layers down in a planet line.
25. As a developer, I want the comparison to cover every game line of the state (clock, difficulty, resources, techs, camera, each planet, each slipway, each structure, counts), so that "matches the PC" means the whole game state, not a sample.
26. As a developer, I want the `zone`, `budget` and `cap` lines left out of the comparison, so that a 32-bit pointer size does not count as a game difference.
27. As a developer, I want the MIPS `zone`, `budget` and `cap` lines, the executable's text/data/bss sizes and the vsync count written to a measurement file next to the check outputs, so that plan 11 starts from a measured first figure.
28. As a developer, I want a mismatch to name the first differing game line from both sides, so that I can start debugging from it.
29. As a developer, I want the PC reference produced by the check itself from the same recording with the current PC replay binary, so that a stale `tmp/state/` file can never be compared.
30. As a developer, I want the emulator shown to accept scripted pad input through a Lua pad script, so that steps 5 and plan 11 can drive the game with the controller headless.
31. As a developer, I want the pad proof to compare the ordered sequence of pad states seen by a PS-EXE with the script's sequence, so that a pad that is ignored, stuck or misordered fails.
32. As a developer, I want the pad script to start from a handshake with the probe and not from a fixed vsync, so that a change in boot length does not shift the proof.
33. As a developer, I want `make psx-check` to evaluate every condition before concluding and print one line per condition, as `psx-hello-check` does, so that one run shows everything that is wrong.
34. As a developer, I want `make psx-check` to stay out of `make test-all`, so that `make test` and `make e2e` do not get slower.
35. As a developer, I want `full_game.json` replayed by its own target, with the outcome (green, or pending with its measured reason) recorded in the step note, so that plan 11 knows where it starts.
36. As a later plan-10 step, I want a check target that I extend with my own proof, so that my step is done only when the target is green.
37. As a reviewer, I want the docs to describe the PSX game build, the embedded replay, the boot mode and `make psx-check` as they are, so that the next session can run them without reading this PRD.
38. As a human, I want the emulator never opened in a window, so that verification stays headless and agents never launch a GUI.

## Implementation Decisions

### Gate

- The step starts by running `make psx-hello-e2e` (the plan-9 check target: CD image, five-condition proof, capture against the committed sha256). Green, with `run1.vram` written, is the gate. Red, or a missing setup, stops the run with a report. Nothing is patched, re-baselined or worked around. The toolchain and the emulator are being installed in this container: the gate waits for both stamps and never runs a setup itself.

### Open question decided: replaying an `events.json` in the emulator, and scripted pad input

- **Replay: embedded at build time, not injected by the emulator.** A PC tool reads a recording through the existing replay source (the loader `make replay` uses, so the same frames and the same `initial_save`). It writes a generated C source holding the frame array (`p8_input_frame_t`, mouse fields zero) and the seed save bytes, plus the recording's name and frame count. The PSX build links that source. Consequences: no `fopen`, no JSON parsing and no CD file system on PSX, and one PS-EXE per recording (the core is compiled once, only the generated source differs). The frame array costs 16 bytes a frame (~76 KB for `full_game.json`'s 4,778 frames), counted in the measurement file. Packing it to the 16-bit pad word is left to plan 11 if memory asks for it.
- Why not feed the recording through the emulated pad: the game reads `pad_buttons` from the input frame, and the PSX pad driver that fills it is step 5. Driving the replay through the pad would bring step 5 into this step, and it would make the replay depend on vsync timing, while the PC replay is frame-exact.
- **Scripted pad input: PCSX-Redux Lua pad overrides.** At the pinned commit, `PCSX.SIO0.slots[1].pads[1].setOverride(<button>)` / `clearOverride(<button>)` (the pad API in the emulator's pad module, button indices in hardware bit order) are called from the existing `GPU::Vsync` listener. The trial recorded in `docs/psx-hello-world.md` showed this works headless and deterministically. A pad script is a text file of steps, each a vsync offset and a set of button names. The Lua side plays it relative to a handshake: the probe PS-EXE writes a magic word into the last word of the scratchpad once its pad driver runs, and the Lua polls it each vsync through `PCSX.getScratchPtr()`.
- **Fallback, if overrides do not reach the BIOS pad driver at the pin:** the pad proof is recorded in the step note as failing with its evidence. Step 5 then drives the title through embedded pad frames fed at the input-frame level (the same mechanism as the replay). This step is still done on the replay proof, and the gap is recorded as pending for step 5. No other emulator or pin is tried in this step.

### Modules

- **State text formatter (devtools, shared).** The `make state` text is produced through a small line sink (a write callback and its context), using only `snprintf`/`vsnprintf` into a local buffer. It has no `FILE`, no `atexit` and no `stdio.h` in its interface. The PC keeps today's API (write to a `FILE`, write-on-exit through `atexit`, the save summary) as a thin adapter over the formatter, so every caller (replay driver, save check, input-step test) is unchanged and its output byte-identical. The PSX adapter prefixes each line with `psx-state: ` and prints it over TTY.
- **Embedded replay generator (PC binary, one file in `src/cmd/`).** Its input is a recording path and a name, and its output is the generated C source. It refuses a mouse recording, a load failure or an empty recording, exits non-zero naming the file, and writes nothing on failure.
- **PSX replay driver (one file in `src/cmd/`, PSX only).** It sets up the input, memory and graphics contexts as the PC driver does, without loading the sprite sheet, then seeds the memory save storage from the embedded save, runs `cart_load`/`cart_init` and the frame loop, and prints over TTY:
  - `psx-replay: asserts on|off`
  - `psx-replay: format <self-test string>`
  - `psx-replay: frame <n>` every 256 frames
  - `psx-replay: done <name> <frames> vsyncs <n>` at the end
  - the `psx-state: ` block

  It then leaves through the exit module with code 0. The sprite sheet is not loaded in this step: no game line depends on it (the core never reads sprite pixels or framebuffer pixels back), and embedding assets is step 2.
- **PSX platform (new files in `src/platform/`, PSX only):**
  - an exit module: an optional one-shot hook, then the PCSX exit register, honoured under `-testmode`, then a spin;
  - the PSX `p8_fatal`: TTY message, then the hook that prints `stopped p8_fatal frame <n>` and the state, then exit code 3;
  - the assert handler: TTY `file:line expr`, exit code 4, overriding PSn00bSDK's `_assert_abort` (if the static link refuses the override, `-Wl,--wrap` is used);
  - the TTY line sink for the state formatter.

  The pad probe's raw pad read (BIOS `InitPAD`/`StartPAD`, the active-low button word inverted) also lives here. Step 5 will map it to `pad_buttons`, and this step does not.
- **Pad probe (one file in `src/cmd/`, PSX only).** It starts the pad driver, sets the scratchpad handshake, then prints `psx-pad: <hex>` each time the pressed-button word changes. It ends when it has seen the script's final state (no button after the last step) or a vsync ceiling, and leaves with code 0 or non-zero.
- **PSX game build.** A CMake project for the PSn00bSDK toolchain file, like `psx-hello`. The Makefile hands it the source list: the core sources (`CORE_SRCS`, which already leaves out the host tools `assets.c`, `dump.c`, `input_json.c` and `fatal_pc.c`), the memory save storage, the shared state formatter, the PSX platform files and the generated replay source. It builds two PS-EXEs: the replay for `EVENTS=<recording>` and the pad probe. Includes are relative to `src/`. It is built `GPREL` at `-O2` with `NDEBUG` removed, so asserts stay on. The `asserts on` TTY line proves it, and the check fails on `off`. 64-bit division comes from libgcc, which the SDK already links. A missing libc symbol is provided in `src/p8/` (portable C, also built on PC) or in the PSX platform, never by changing a call site in the core.
- **Boot script.** `psx_boot.sh` gains an "until exit" mode that adds `-testmode`. In it the frame count is a vsync ceiling, and reaching it fails naming the input and the last `psx-replay:` or `psx-pad:` line. A program exit with code 0 passes with no capture required, and a non-zero code fails naming it. The mode also takes an optional pad script that the Lua plays. The existing mode, its capture format and every psx-hello target are unchanged. The ceiling and the timeout for each proof are set from the first measured run plus half again, and recorded in the step note.
- **Game-line comparison script (`tools/`).** Its inputs are a TTY file, a PC state file and an output directory. It extracts the `psx-state: ` lines and requires the `done` line with the expected frame count and no `stopped p8_fatal`. It compares every line except `zone`, `budget` and `cap` with the PC's, in order, and names the first differing line from both sides. It writes the MIPS `zone`/`budget`/`cap` lines to the measurement file.
- **`make psx-check` (the plan-10 check target, outside `make test-all`).** It builds both PS-EXEs (replay of `tests/e2e/pad_slipway.json`), produces the PC reference with `STATE=` and the PC replay binary, and boots the replay and the probe. It evaluates and prints, one line each:
  - `magic`: both files start with `PS-X EXE`;
  - `asserts-on`;
  - `format`: the self-test string equals the PC's;
  - `replay-done`;
  - `state-game-lines`;
  - `pad-script`: the probe's ordered states equal the script's.

  It fails if any condition fails. The measurement file gets the `zone`/`cap` lines, the vsync count and `size` of the ELF. The psx-hello targets stay as the toolchain smoke test, outside `psx-check`.
- **`make psx-full-game-check`.** The same replay proof on `worst-case/full_game.json`, with its own ceiling and timeout. It is not a prerequisite of `psx-check`: at minutes a run it would be paid again by every later step. Its outcome is recorded in the step note: green with its measurement, or pending for plan 11 with the reason measured (frames reached against vsyncs, or the `p8_fatal` line).
- **Core changes.** The plan's rule is that the core does not move. A MIPS divergence whose cause is in `src/game/` or `src/engine/` (undefined behaviour, a 64-bit `long` or pointer-size assumption, an order that depends on addresses) is fixed there only if the fix keeps every `*.sha256` and `make state` output identical. It is recorded in the step note as a portability fix. A divergence that cannot be fixed that way stops the step with a report.
- **Docs.** A new doc describes the PSX game build, the embedded replay, the until-exit boot mode, the pad script and `make psx-check` as they are. The "Still to do" items of `docs/psx-hello-world.md` that this step closes (scripted pad input) move there or are removed. The Makefile legend gets the new targets.

## Implementation Chunks

1. **Gate.** Run `make psx-hello-e2e`, then stop on red and record the result in the step note.
2. **Shared state formatter** (PC only). Move the state text behind a line sink, with the PC `FILE` adapter unchanged for callers. `make test-all`, `make state` and every hash stay identical.
3. **Embedded replay generator** (PC only), with its host test.
4. **Game-line comparison script**, with its host test.
5. **PSX build and platform.** The CMake project, the Makefile source list, the exit/fatal/assert/TTY platform and the PSX replay driver. Done when the replay PS-EXE for `pad_slipway.json` links, with libc gaps provided, and the probe links.
6. **Until-exit boot mode and the replay proof.** Boot the `pad_slipway` PS-EXE, fix divergences until the game lines match, and set the ceiling from the measurement.
7. **Pad probe and Lua pad script**, the `pad-script` condition, and the fallback recorded if needed.
8. **`make psx-check`** assembling every condition, `make psx-full-game-check` run once with its outcome recorded, the measurement file, then the docs and the Makefile legend.

## Testing Decisions

- A good test checks behaviour seen from outside: the bytes a tool writes, the lines a script prints and its exit code. It never checks the formatter's internals or the generated source's layout beyond what a compiler and the replay consume.
- **Shared state formatter.** The existing `test_state`, `test_input_step`, save-check tests and `make state` are the net, and they must stay byte-identical. One new unit test replays a short fixture and checks that the sink output equals the `FILE` output, byte for byte.
- **Embedded replay generator.** A unit test runs the generator on existing fixtures (a pad recording, a recording with `initial_save`, the refused mouse recording), compiles the generated source with the host compiler into the test, and checks three things. Its frames equal what `p8_input_load` returns, its save bytes equal the replay source's, and the mouse recording exits non-zero with no output file. Prior art: `test_replay_source`, `test_input`.
- **Game-line comparison script.** A unit test drives the script on crafted files, as `test_frames_check` drives `frames_check.sh`, and checks six cases:
  - a match with different `zone` lines passes;
  - one differing planet line fails, naming both lines;
  - a TTY interleaved with emulator log lines passes;
  - a missing `done` line fails;
  - a wrong frame count fails;
  - a `stopped p8_fatal` fails.
- The emulator-side proofs (`psx-check`, `psx-full-game-check`) are integration checks outside `make test-all`. Their conditions are the tests of the PSX build, platform, boot mode and pad probe. No unit test runs the toolchain or the emulator.
- `make test` and `make e2e` must not get slower. The three new unit tests are millisecond-scale host binaries and add no replay of their own beyond one short fixture.
- Every commit keeps `make test-all`, `make sanitize-net`, `make full-game` and `make state` (with its 1/8 margin) green, and leaves every `*.sha256` unchanged.

## Out of Scope

- Any GPU rendering, display output or VRAM capture of the game (steps 3 and 4). The software framebuffer is drawn into but never shown or compared.
- Embedding the sprite sheet, map and font (step 2).
- Wiring the PSX pad driver into `pad_buttons` or passing the title screen with the controller (step 5). The pad probe only proves that scripted input reaches a PS-EXE.
- The game's `.bin/.cue` and CD boot (step 6). This step boots PS-EXEs only.
- Optimizing CPU time or memory on MIPS, resizing zones or caps, or packing the embedded frames (plan 11).
- Audio.
- Memory-card save storage: the memory save backend is used.

## Further Notes

- The PSn00bSDK release config defines `NDEBUG`, and its `stdio.h` has no `FILE`, `fopen` or `atexit`. Those three facts drive the formatter split, the exit module and the asserts-on proof.
- PCSX-Redux's software exit register (write a 16-bit code at `0x1f802082`) quits only under `-testmode`. Without it the emulator pauses and the boot runs until its timeout.
- `pad_slipway.txt` documents that drawing the slipway drag spends random draws, which is why `cart_draw` stays in the MIPS loop.
- The MIPS `zone` figures will be lower than the PC's (4-byte pointers). They are a first measurement, not a limit check: `make state`'s 1/8 margin stays the PC's job until plan 11 re-measures on MIPS.
- The step note should record the gate result, which libc symbols were provided, every portability fix, and the measurement file's figures. It should also record the `full_game.json` outcome and whether the pad overrides worked or the fallback applies.
