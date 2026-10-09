---
title: "The game's .bin/.cue, built by make and proven on the CD boot"
description: "The controller-driven game exists only as a PS-EXE side-loaded into the emulator, so nothing builds the CD image the plan ships, nor proves that this image boots, shows the title screen and passes it under the controller."
status: needs-triage
---

## Problem Statement

Plan 10, phase 1, step 6, verbatim:

> 6. **The game's `.bin/.cue`** produced by a `make` target. The step-5 EXE goes into the CD image with the `psx-hello-cd` tooling (its `cue` condition, never hashing the `.bin`), and the step-5 proof is repeated on the `.cue`.

Plan 10's goal is "the full game compiled for MIPS, in a `.bin/.cue` that boots in the emulator, shows the title screen and is driven by the controller". Its first "Done when" item is the same sentence about the `.bin/.cue`.

After step 5 (see "Step 5 as committed"), the shipped game is a PS-EXE, `psx-game.exe`, and every proof boots it with `-loadexe`: the emulator side-loads it into RAM and jumps to it. Three things are therefore unproven:

- **No CD image of the game exists.** Only `psx-hello` has one (`psx-hello/iso.xml`, `psx-hello/system.cnf`, `make psx-hello-cd`). A player, or plan 11's "Ship" step, needs a `.bin/.cue`, not an EXE.
- **The BIOS boot path is not exercised for the game.** On a CD boot, the BIOS reads `SYSTEM.CNF`, loads the EXE from the ISO 9660 file system and sets the stack from `STACK=`. A wrong 8.3 name, a wrong `BOOT=` line or a missing file does not fail the emulator: it lands in the OpenBIOS shell (`docs/psx-hello-world.md`, "Traps").
- **The EXE in the image is not tied to the EXE under check.** Step 5's static conditions (`magic`, `no-json`, `game-ram-room`) read `psx-game.exe`/`.elf` on disk. Nothing says the image holds those exact bytes.

## Solution

`make psx-build` now also writes the game's CD image, `slipways.bin` and `slipways.cue`, next to the PS-EXEs in `tmp/psx/build`. It uses `psx-hello`'s tooling: PSn00bSDK's `psn00bsdk_add_cd_image` over a committed `psx/iso.xml` and `psx/system.cnf`, which boot `SLIPWAYS.EXE`, a byte copy of `psx-game.exe`. `make psx-game-cd` is the named target: it builds and names the two files.

`make psx-check` keeps five boots. The game boot of step 5 is **moved** from `--exe psx-game.exe` to `--iso slipways.cue`: same pad script, same end word, same post-boot checks. One CD boot carries the whole step-5 proof and the CD proof, so no boot is added (plan rule "PSX check cost"). Three conditions are added:

- `game-cue`: `psx-hello`'s `cue` condition, now a shared script. The `.cue` names a `.bin` of whole 2352-byte sectors.
- `game-cd-exe`: the `SLIPWAYS.EXE` that `dumpsxiso` (shipped with the SDK) extracts from the image is byte-identical to `psx-game.exe`. The EXE checked is the EXE shipped.
- `game-gpu-cost`: the CD game's draw-meter lines equal the PC's for the same input. The GPU backend received the PC's exact draw calls, title frames included.

The `.bin` is never hashed, and no capture of the CD boot is compared to a reference: mkpsxiso stamps the build time into the `.bin`, and the proof is state-based, as in step 5.

`make psx-game-check` (step 5) keeps booting the EXE, for fast controller work. A new `make psx-game-cd-check` runs the CD boot and its conditions alone.

## Step 5 as committed

Step 5 is done (HEAD `eb762ae`; see the "Step 1.5" note of `docs/plan-10.md` and `docs/psx-game.md`). The facts below were checked against the committed code. The H-5-k labels are kept so that the references elsewhere in this PRD stay traceable.

- **H-5-1. The game EXE.** `make psx-build` builds `psx-game.{elf,exe}` in `PSX_BUILD_DIR` (`tmp/psx/build`). It is the `psx-game` target of `psx/CMakeLists.txt`, built from the required `PSX_GAME_SOURCES` list (`PSX_GAME_SRCS` in the Makefile). It has the same flags as the other targets: `GPREL`, `-UNDEBUG` and the `--wrap` link options. There is no check-only build flag: the EXE under check is the one that ships. `psx-game.exe` is 362,496 bytes. The ELF has text 357,128, data 2,856 and bss 1,113,612.
- **H-5-2. Load independence.** `src/cmd/psx_game.c`'s `main(void)` reads no arguments. After `psx_pad_start()`, it clears the scratchpad end word, then raises the pad handshake word itself. So nothing it does depends on `-loadexe`. The BIOS loading it from CD with `STACK=801FFFF0` runs the same program. `psx_pad_start` now fills the pad buffers with 0xff, a step-5 fix: zeroed buffers read as every button held until the BIOS first polled. A slower CD start therefore cannot log phantom presses.
- **H-5-3. Its TTY and ending.**
  - It prints `psx-game: ready`, then `psx-game: pad <frame> <%03x>` on frame 0 and on each change.
  - It runs at most one frame per vsync.
  - The loop ends once the end word holds the end magic. It then prints `psx-replay: done game <frames> vsyncs <n>` and the `psx-state:` block (`state_format` through `psx_tty_state_sink`), and exits with `PSX_EXIT_OK`.
  - A `p8_fatal` prints the stopped state through the exit hook.
- **H-5-4. Its meter lines.** It installs the draw meter around the GPU backend (`p8_draw_meter_install()` after `p8_draw_backend_set(psx_gpu_backend())`), so its `psx-state:` block holds the meter's `zone gpu_frame`, `cap gpu_cluts` and `cap gpu_patterns` lines. `psx_game_check.sh` replays the converted log on PC with `STATE=`, which installs the meter too, so its `pc_state.txt` holds the same kinds of lines. Nothing compares them yet: `gpu-cost` reads only the GPU replay.
- **H-5-5. Post-boot checks.** `tools/psx_game_check.sh <tty> <boot-status> <boot-output> <pad-script> <pad-log-tool> <replay-tool> <difficulty> <out-dir>` prints five `psx-game-check: <condition>: ok[, detail]` lines: `game-done`, `game-pad-log`, `game-pad-states`, `game-state-game-lines` and `game-title-left`. Each failure goes to stderr, and the script exits 1 if any condition failed. Into `<out-dir>`, it writes:
  - `tty.txt`;
  - `game_events.json`;
  - `pc_state.txt` and `pc.dump.txt`;
  - `game-state/`, the outputs of `psx_state_compare.sh`. The PSX state is in `game-state/tty.txt`, behind `psx-state: `.

  `test_psx_game_check` drives it on crafted files in `make psx-tools-test`.
- **H-5-6. `make psx-check`.** `tools/psx_check.sh` takes **31 positional arguments**. The last eight are `<game-exe> <game-script> <game-ceiling> <game-timeout> <game-difficulty> <pad-log-tool> <replay-tool> <out-dir>`.
  - The game boot comes after the pad probe: `psx_boot.sh --until-exit --pad-script <game-script> --exe <game-exe> <game-ceiling> <out-dir>/game`, under `PSX_BOOT_TIMEOUT=<game-timeout>`.
  - It runs `psx_game_check.sh` into `<out-dir>/game`, writing `game-check.out.txt` and `game-check.err.txt`, and relays the five lines. It adds `game-ram-room` through its `check_ram_room` function.
  - `magic` and `no-json` cover the game EXE and ELF.
  - That makes 27 conditions over five boots (replay, GPU capture, GPU determinism, pad probe, game). The run takes about 96 s and ends with `psx-check: all twenty-seven conditions hold`.
  - `measurements.txt` gains `game vsyncs`, `game frames`, `game ram-room` and `game` size lines.
  - The `gpu-cost` comparison is inline code, keyed by `meter_lines='^(zone gpu_frame|cap gpu_cluts|cap gpu_patterns) '`.
- **H-5-7. Standalone target.** `make psx-game-check` (~8 s, outside `test-all`) depends on `psx-tools-test`, `psx-build`, `tmp/replay` and `tmp/psx_pad_log`. It boots `psx-game.exe` with `PSX_GAME_SCRIPT` into `tmp/psx/game/run`, then runs `psx_game_check.sh` into `tmp/psx/game` (`PSX_GAME_OUT`). Its variables are:
  - `PSX_GAME_SCRIPT ?= psx/game_title_script.txt`;
  - `PSX_GAME_DIFFICULTY ?= reasonable`;
  - `PSX_GAME_CEILING ?= 545` and `PSX_GAME_TIMEOUT ?= 10`. The measured game boot takes 363 vsyncs.
- **H-5-8. Boot tooling.** `psx_boot.sh`'s last-progress pattern is `^psx-(replay|pad|game): `. Step 5 left `tools/psx_pad_play.lua` and `tools/psx_until_exit.lua` unchanged. The pad player starts on the handshake word, whatever loaded the program. The pad words are shared in `src/platform/psx_pad_protocol.h` and checked against the Lua in `make psx-tools-test`.
- **H-5-9. Green at the end of step 5.** The "Step 1.5" note records `make psx-check` green with 27 conditions, ~96 s and five boots. The game run was 207 frames, 11 pad states and 102 matching game lines, and it left the title on `difficulty reasonable`. `game-ram-room` left 492,484 bytes.
- **H-5-10. Docs.** `docs/psx-game.md` describes the game EXE, its pad log, how it ends under check, the 27 conditions and `make psx-game-check`. Its "Still to do" starts with "Plan 10 step 6: pack `psx-game.exe`, as built, into the game's `.bin/.cue` and repeat the controller proof on the CD image."
- **H-5-11. Shell-driven tests.** Every shell-driven test is in `PSX_TOOLS_TESTS` (`make psx-tools-test`), outside `make test` and `make test-all`, and is a prerequisite of `psx-check` and `psx-game-check`. Step 6's `test_psx_cue_check` and `test_psx_cd_check` go there.

## User Stories

1. As the plan-pilot run, I want step 6 to start by re-running `make psx-check` and re-reading H-5-1 to H-5-11 against the code, so that the CD image is not built on a step 5 that has drifted since its note.
2. As the plan-pilot run, I want a red `make psx-check` at the start, or a missing `psx-game` EXE, to stop the step with a report, so that a broken step 5 is never papered over by step 6.
3. As a player, I want a `.bin/.cue` of the game, so that I can load it in a PSX emulator as a disc, the way a PSX game is played.
4. As a player, I want the disc to boot straight into the game, with no shell and no file to pick, so that it behaves like a PSX game disc.
5. As a player, I want the game on the disc to be exactly the controller-driven game of step 5, so that what I play is what was checked.
6. As a developer, I want the CD image built by `make psx-build` itself, next to the PS-EXEs, so that every build that produces the game also produces its disc and they cannot get out of step.
7. As a developer, I want a named target, `make psx-game-cd`, that builds the image and fails naming the missing file when the `.bin` or the `.cue` was not written, so that "the `make` target that produces the `.bin/.cue`" is one command, as `make psx-hello-cd` is for `psx-hello`.
8. As a developer, I want the image built with `psx-hello`'s tooling (`psn00bsdk_add_cd_image`, an `iso.xml`, a `system.cnf`), so that the game uses the one CD path the toolchain checks already proved.
9. As a developer, I want the image's `iso.xml` and `system.cnf` committed under `psx/`, next to the game's CMake project, so that the disc layout is reviewed like code.
10. As a developer, I want every file name on the disc in 8.3 form (`SYSTEM.CNF`, `SLIPWAYS.EXE`), so that the BIOS's ISO 9660 reader finds them (`psx-hello/iso.xml`'s trap).
11. As a developer, I want `system.cnf` to keep `psx-hello`'s `TCB`, `EVENT` and `STACK` values, so that the game's stack is where `ram-room` assumes it is (top of RAM).
12. As a developer, I want the image to carry no license data and no region data, as `psx-hello`'s does, so that the pinned emulator and OpenBIOS, which check neither, boot it, and the gap is written down rather than faked.
13. As a developer, I want the `.cue` and `.bin` paths in Makefile variables (`PSX_GAME_CUE`, `PSX_GAME_BIN`), so that `make psx-boot PSX_ISO=$(PSX_GAME_CUE)` and the checks name one path.
14. As a reviewer, I want the `.bin` never hashed and never compared, by any target or script, so that mkpsxiso's build-time stamp never makes a check flaky.
15. As a developer, I want `psx-hello`'s `cue` condition moved into one shared script, used by both `psx_hello_check.sh` and `psx_check.sh`, so that the game's disc is checked by the exact code that checks `psx-hello`'s.
16. As a developer, I want `psx_hello_check.sh`'s output and exit codes unchanged by that move, so that `make psx-hello-cd-check` and `make psx-hello-e2e` keep their meaning.
17. As the check, I want `game-cue`, which passes only when the game's `.cue` has a `FILE "<bin>"` line naming an existing, non-empty `.bin` of whole 2352-byte sectors, so that a broken image fails before its boot is blamed.
18. As the check, I want `game-cd-exe`, which passes only when the `SLIPWAYS.EXE` that `dumpsxiso` extracts from the `.bin` is byte-identical to `psx-game.exe`, so that `magic`, `no-json` and `game-ram-room`, which read the EXE file, are proven to cover the shipped disc.
19. As the check, I want `game-cd-exe` to name what went wrong (`dumpsxiso` failed, no `SLIPWAYS.EXE` on the disc, or the first differing byte offset), so that a failure points at its cause.
20. As the check, I want the step-5 game boot of `make psx-check` to boot the `.cue` with `--iso` instead of the EXE with `--exe`, so that the step-5 proof is repeated on the `.cue` without a sixth boot.
21. As the check, I want `game-done`, `game-pad-log`, `game-pad-states`, `game-state-game-lines` and `game-title-left` to read the CD boot's TTY unchanged, so that "boots, shows the title screen and passes it through scripted controller input" is proven on the disc.
22. As the check, I want `game-gpu-cost`, which passes only when the CD game's `zone gpu_frame`, `cap gpu_cluts` and `cap gpu_patterns` lines equal those of the PC replay of its own log, so that the GPU backend is proven to receive the PC's exact draw calls over the run, title screen included. Step 4's captures pin how those calls are drawn.
23. As the check, I want a CD boot that lands in the OpenBIOS shell to fail on `game-done` with the boot's own message (ceiling reached, last progress line none), so that a missing or misnamed EXE on the disc is caught and not mistaken for a slow game.
24. As a reviewer, I want `make psx-check` to keep exactly five boots, and the step note to record its run time and boot count, so that the plan's "PSX check cost" rule is visibly kept.
25. As a developer, I want the CD boot's ceiling and timeout in their own variables (`PSX_GAME_CD_CEILING`, `PSX_GAME_CD_TIMEOUT`), measured as the first run plus half again, so that the BIOS and CD loading time is budgeted without loosening the EXE boot's limits.
26. As a developer, I want `make psx-game-check` to keep booting the EXE, with its step-5 limits, so that controller work iterates without the disc in the loop.
27. As a developer, I want a standalone `make psx-game-cd-check` that builds, boots the `.cue` with the game script and checks `game-cue`, `game-cd-exe` and the game conditions alone, so that I can iterate on the disc without paying for the whole `psx-check`.
28. As a developer, I want the CD conditions in one post-boot script, `tools/psx_cd_check.sh`, driven by a `make psx-tools-test` test with a stub `dumpsxiso` and crafted files, so that each pass and fail path is proven without the toolchain or the emulator.
29. As a developer, I want the shared cue script tested in `make psx-tools-test` on crafted `.cue`/`.bin` pairs, so that its four outcomes (ok, no `FILE` line, missing or empty `.bin`, partial sector) are proven without the emulator.
30. As a developer, I want a missing `dumpsxiso` to fail naming `make psn00bsdk-toolchain-setup`, as a missing SDK does, so that the cause is obvious.
31. As a reviewer, I want `measurements.txt` to gain the `.bin`'s size in bytes and sectors and the CD boot's vsyncs to exit, each behind `game cd `, so that the disc's size and the BIOS loading cost are visible without hashing anything.
32. As a reviewer, I want the extracted disc files and the CD boot's TTY kept in the check's output directory, so that a failure can be inspected after the run.
33. As a human, I want `make psx-boot PSX_ISO=tmp/psx/build/slipways.cue` to keep working with the plain capture mode, so that I can look at a frame of the disc's title screen without a new target.
34. As a human, I want to play the `.bin/.cue` in a windowed emulator myself, so that the "driven by the controller" goal is also judged by hand. No agent opens a window.
35. As the plan-pilot run, I want the end of the step to check each of plan 10's "Done when" items and write the result in the step note, so that the plan's completion is a recorded fact.
36. As the plan-pilot run, I want the human-acceptance item reported as "awaiting the human", listing the pending labels that `gpu-capture-references` prints, so that it is never reported as a failure, and never "fixed" by an agent running the acceptance command.
37. As a developer, I want every `*.sha256` in `tests/e2e/`, every `make state` line, `make full-game` and `make sanitize-net` unchanged, so that the PC is proven untouched.
38. As a developer, I want `make test` and `make e2e` no slower, with every new shell-driven test in `make psx-tools-test`, so that the fast loop stays fast.
39. As a reviewer, I want `docs/psx-game.md` to describe the disc, its targets, its three conditions and the moved boot, and `docs/psx-hello-world.md` to point at the shared cue script, so that the docs state what is true.
40. As a reviewer, I want `docs/psx-game.md`'s "Still to do" to drop step 6 and name what the disc still lacks for real hardware (license data, region), so that the next reader knows the disc is emulator-proven only.

## Implementation Decisions

### Start

- Step 6 starts by running `make psx-check` and `make psx-game-check`, and by re-reading H-5-1 to H-5-11 against the code.
  - If a check is red, `psx-game.exe` is missing, or a setup is missing, the run stops with a report.
  - Any fact found different is adapted to, and recorded in the step note under its H-5-k label.

### CD image layout

- **New committed files `psx/iso.xml` and `psx/system.cnf`**, copied from `psx-hello`'s and changed only where named here.
- `psx/system.cnf`:

  ```
  BOOT=cdrom:\SLIPWAYS.EXE;1
  TCB=4
  EVENT=10
  STACK=801FFFF0
  ```

  `TCB`, `EVENT` and `STACK` are `psx-hello`'s values. `STACK=801FFFF0` is the top of the 2 MiB RAM, where `ram-room`'s `PSX_STACK_RESERVE` is counted (H-5-2).
- `psx/iso.xml` is one data track:
  - identifiers: `system` and `application` `PLAYSTATION`; `volume` and `volume_set` `SLIPWAYS`; `publisher` `SLIPWAYS`; `data_preparer` `PSN00BSDK ${PSN00BSDK_VERSION}`;
  - directory tree: `SYSTEM.CNF` from `${PROJECT_SOURCE_DIR}/system.cnf`, `SLIPWAYS.EXE` from `psx-game.exe` (the build directory), and `<dummy sectors="1024"/>`;
  - `psx-hello`'s comment on 8.3 names is kept: it is a trap someone would reintroduce.
- **No license data, no region data.** PSn00bSDK ships none, and the pinned OpenBIOS does not check it (`docs/psx-hello-world.md`). The emulator is the plan's target. A disc for real hardware or a Sony BIOS needs license data: this goes in "Still to do", not in this step. The display mode stays the game's NTSC 320×240.
- **Image name `slipways`**: `slipways.bin` and `slipways.cue` in `PSX_BUILD_DIR`. On the disc, the EXE is named `SLIPWAYS.EXE`, not `PSXGAME.EXE`: it is the game a player sees in a disc listing.

### Build and the `make` target

- `psx/CMakeLists.txt` gains, after the executables:

  ```
  psn00bsdk_add_cd_image(psx-game-iso slipways iso.xml DEPENDS psx-game system.cnf)
  ```

  This is the same call shape as `psx-hello/CMakeLists.txt`. PSn00bSDK 0.24 adds the image target to `ALL` (read from the installed `internal_setup.cmake`). `make psx-build`'s plain `cmake --build` therefore writes the image every time, after `psx-game.exe`. The project `DESCRIPTION` names the game and its disc.
- `make psx-build` prints, after the ELF sizes, the two image paths. It also fails naming them when either is missing.
- **`make psx-game-cd`** (new, outside `test-all`): depends on `psx-build`, then checks that `$(PSX_GAME_BIN)` and `$(PSX_GAME_CUE)` exist and prints them. It is the plan's "`make` target that produces the `.bin/.cue`", named after `psx-hello-cd`. It needs no `--target`: the image is in `ALL`.
- New variables: `PSX_GAME_CUE ?= $(PSX_BUILD_DIR)/slipways.cue`, `PSX_GAME_BIN := $(PSX_BUILD_DIR)/slipways.bin`, `PSX_DUMPSXISO = $(PSN00BSDK_DIR)/bin/dumpsxiso`.
- `psx-full-game-check` builds in its own `PSX_BUILD_DIR`, so it writes its own image there. That is harmless, and nothing reads it.
- mkpsxiso's time stamp makes the `.bin` differ on every build. Nothing hashes or compares it. A comment at the place that prints the `.bin` size repeats the trap, as `psx_hello_check.sh` does.

### The shared cue condition (`tools/psx_cue_check.sh`, new)

- Usage: `psx_cue_check.sh <cue>`. It is the `cue_bin` function and the `cue` branch of `psx_hello_check.sh`, moved unchanged:
  - the `.bin` is resolved next to the `.cue` from the first `FILE "<name>"` line;
  - on success it prints the `.bin` path on stdout and exits 0;
  - otherwise it prints the same reasons as today on stdout (`<cue> has no FILE "<bin>" line`, `<bin>, named by <cue>, is missing or empty`, `<bin> is <n> bytes, not a whole number of 2352-byte sectors`) and exits 1.
- `psx_hello_check.sh` calls it for `--iso` and keeps printing `psx-hello-check: cue: ok` or its `report cue` line, and its final `.bin` sha256 information line. Its five conditions, messages and exit codes are unchanged.

### The CD conditions (`tools/psx_cd_check.sh`, new)

- Usage: `psx_cd_check.sh <cue> <exe> <disc-name> <dumpsxiso> <out-dir>`. It is a post-boot script that needs neither the emulator nor the boot, so it is testable on crafted files.
- It prints one `psx-cd-check: <condition>: ok[, detail]` line per condition, or the failure on stderr. Both conditions are evaluated before concluding. It exits 1 if any failed, and 2 on usage.
  - `cue`: through `psx_cue_check.sh`. The detail is `<bin>, <sectors> sectors`.
  - `cd-exe`: `<dumpsxiso>` extracts the `.bin` into `<out-dir>/disc` (fresh directory). Then `<out-dir>/disc/<disc-name>` must exist and `cmp` equal to `<exe>`. Failures are named: `dumpsxiso` missing or exiting non-zero (its stderr quoted), the disc file absent (the extracted names listed), or `cmp`'s first differing byte. Skipped, and failed, when `cue` failed.
- The `dumpsxiso` command line is the one mkpsxiso 2.03 documents (extract all files of a `.bin` into a directory). The implementer reads it from the tool's own usage text, once, when writing the script.
- `<disc-name>` is `SLIPWAYS.EXE`, passed by the Makefile as `PSX_GAME_DISC_EXE`, so the script stays generic.

### `make psx-check` (plan rule "PSX check cost")

- **The game boot moves to the disc; no boot is added.** `psx_check.sh`'s step-5 game boot becomes `--until-exit --pad-script $(PSX_GAME_SCRIPT) --iso $(PSX_GAME_CUE)`, with `PSX_GAME_CD_CEILING` and `PSX_GAME_CD_TIMEOUT`.
  - Why one boot suffices: the BIOS loads the same EXE bytes (`game-cd-exe`), and the program does not depend on how it was loaded (H-5-2). So the CD boot proves everything the EXE boot proved, plus the boot path.
  - `make psx-check` keeps five boots: replay, GPU capture, GPU determinism, pad probe, game from disc.
- `psx_check.sh` goes from 31 to 34 arguments (H-5-6). It gains `<game-cue>`, `<dumpsxiso>` and `<game-disc-exe>` (`PSX_GAME_DISC_EXE`), inserted after `<game-difficulty>` and before `<pad-log-tool>`. The usage line and the header comment name them. Its `<game-ceiling>` and `<game-timeout>` arguments now receive `PSX_GAME_CD_CEILING` and `PSX_GAME_CD_TIMEOUT`. The game EXE argument stays: `magic`, `no-json`, `game-ram-room` and `game-cd-exe` read it.
- After the game's post-boot checks, `psx_check.sh` runs `psx_cd_check.sh <cue> <game-exe> <disc-name> <dumpsxiso> <out-dir>/game-cd` and relays its two lines as `game-cue` and `game-cd-exe`. It also adds `game-gpu-cost`:
  - the `^(zone gpu_frame|cap gpu_cluts|cap gpu_patterns) ` lines of the game's PSX state (`<out-dir>/game/game-state/tty.txt`, H-5-5, with `psx-state: ` stripped) must equal those of `<out-dir>/game/pc_state.txt`;
  - the failure quotes both sides, as `gpu-cost` does. `gpu-cost`'s inline code (H-5-6) becomes one shell function, `check_meter <condition> <psx-state-tty> <pc-state>`, used by both conditions, so the two cannot drift. `gpu-cost`'s output is unchanged.
- 30 conditions in all (27 + `game-cue`, `game-cd-exe`, `game-gpu-cost`). The header comment, `conditions.txt` and the final line (`psx-check: all thirty conditions hold`) name the new total.
- `measurements.txt` gains, each behind `game cd `:
  - `bin-bytes <n> sectors <n>`;
  - `boot-vsyncs <n>`, read from `psx_boot.sh`'s `exited with 0 after <n> vsyncs` line. This is the vsyncs from power-on, BIOS and CD load included, next to step 5's `game` vsyncs from the first frame.
- **Limits.** `PSX_GAME_CD_CEILING` and `PSX_GAME_CD_TIMEOUT` are measured on the first green run as that run plus half again, like every other boot, and recorded in the step note. Until then, the implementer's first run uses the step-5 game limits plus 600 vsyncs and 10 s (1,145 vsyncs, 20 s). That is generous for an OpenBIOS CD load of the 362,496-byte EXE (H-5-1), about 177 sectors. `PSX_GAME_CEILING`/`PSX_GAME_TIMEOUT` (EXE boot) are unchanged, used by `make psx-game-check` only.
- **Expected cost.** The step-5 run time (~96 s, H-5-9) plus the BIOS and CD loading of one boot (a few seconds) plus `dumpsxiso` (well under a second). The step note records the measured `make psx-check` time and its five boots.

### `make psx-game-cd-check` (new, outside `test-all`)

- Prerequisites: `psx-tools-test`, `psx-game-cd`, `tmp/replay`, `tmp/psx_pad_log`, the same as `psx-game-check`'s (H-5-7).
- Steps:
  1. the CD boot with `PSX_GAME_SCRIPT` and the CD limits into `tmp/psx/game-cd/run`, as `psx-game-check`'s recipe does for the EXE (H-5-7);
  2. `psx_game_check.sh` on its TTY into `tmp/psx/game-cd`;
  3. `psx_cd_check.sh` into `tmp/psx/game-cd/cd`.

  It fails if any line fails. It checks neither `game-gpu-cost` nor `game-ram-room`: those stay in `psx-check`.
- The Makefile legend describes `psx-game-cd`, `psx-game-cd-check` and the moved game boot of `psx-check`.

### What stays put

- No file of `src/` changes. `psx_game.c` and every PS-EXE are byte-for-byte what step 5 left. The disc only packs the EXE.
- `tools/psx_boot.sh`, `tools/psx_until_exit.lua` and `tools/psx_pad_play.lua` are unchanged. `--until-exit` with `--iso` is already accepted. The pad player starts on the handshake word whatever loaded the program (H-5-8).
- `psx-hello` and its reference sha256 are unchanged. Its targets keep proving the toolchain.
- PC: every e2e, `make state`, `make full-game`, `make sanitize-net` unchanged.
- No new capture, reference or ceiling file. `psx/gpu_capture_references.txt` is not touched by any agent.

### Checking plan 10's "Done when" at the end of the step

This is the last step of plan 10. The final chunk checks each "Done when" item and writes the result in the "Step 1.6" note:

1. *"The game's `.bin/.cue` boots in the emulator, shows the title screen and passes it through scripted controller input."* Done when the green `make psx-check` holds `game-cue`, `game-cd-exe`, `game-done`, `game-gpu-cost`, `game-pad-states`, `game-state-game-lines` and `game-title-left`, all on the CD boot. The note quotes them:
   - boots: the BIOS loaded `SLIPWAYS.EXE` from the disc, and it ran to its done line;
   - shows the title screen: the frames it ran, the title frames among them, sent the PC's exact draw calls to the GPU backend, whose drawing step 4's captures measure against the PC;
   - passes it: the pad states, the replayed game lines, and `difficulty reasonable` with a started clock.
2. *"The PSX check target is green."* Done when `make psx-check` ends with `psx-check: all thirty conditions hold`. The note records its run time and five boots.
3. *"The human has reviewed the step 4 divergences … frozen as references."* Reported as **awaiting the human**, not as a failure. The note lists the `pending human acceptance: <label> f<frame>` lines that `gpu-capture-references` prints in that run, and the one command per label (`make psx-gpu-capture-accept LABEL=<label>`). No agent runs that command, and no agent edits `psx/gpu_capture_references.txt`.

The note also records the invariants: `make test-all`, `make state`, `make full-game` and `make sanitize-net` green, and every `*.sha256` unchanged.

## Implementation Chunks

1. **Shared cue script and CD post-boot script (host only).**
   - Start gate and H-5-k re-read first.
   - Then `tools/psx_cue_check.sh` with `test_psx_cue_check`, and `psx_hello_check.sh` switched to it.
   - Then `tools/psx_cd_check.sh` with `test_psx_cd_check`, both tests in `make psx-tools-test`.

   Done when `make psx-tools-test` and `make test-all` are green. `make psx-hello-cd-check` is green with unchanged output, apart from timing and per-boot paths.
2. **The disc.**
   - `psx/iso.xml`, `psx/system.cnf`, the CMake call;
   - `psx-build`'s image check, `make psx-game-cd`, and the variables;
   - `make psx-game-cd-check`, with `PSX_GAME_CD_CEILING`/`PSX_GAME_CD_TIMEOUT` measured.

   Done when `make psx-game-cd-check` is green, and `make psx-check` and `make psx-game-check` are still green.
3. **`make psx-check` on the disc, and hand-off.**
   - The moved game boot, the three conditions, the measurement lines, the new total and the Makefile legend.
   - Then `docs/psx-game.md`: a "CD image" section (layout, targets, the never-hash trap, the three conditions, the moved boot), the checks table, and "Still to do" (step 6 removed; license data and region for real hardware; the human's capture acceptance and play).
   - `docs/psx-hello-world.md`: its `cue` bullet points at `tools/psx_cue_check.sh`.
   - Then the step-note draft and the "Done when" check above.

   Done when `make psx-check` is green with 30 conditions and five boots.

## Testing Decisions

- A good test drives a script's or tool's public interface on crafted inputs and checks its printed lines and exit code. It never checks how the script is written.
- **`test_psx_cue_check`** (new, `make psx-tools-test`, crafted files in a temp directory):
  - a `.cue` naming a `.bin` of 2 × 2352 bytes passes and prints the `.bin` path;
  - no `FILE` line fails;
  - a missing `.bin`, and an empty `.bin`, fail naming the `.bin`;
  - a `.bin` of 2352 + 1 bytes fails naming the size;
  - a `FILE` line with a relative name resolves next to the `.cue`, not in the current directory.
- **`test_psx_cd_check`** (new, `make psx-tools-test`). It uses a stub `dumpsxiso`, a shell script written by the test that copies a chosen file into the extraction directory under a chosen name, or exits 1:
  - passing case: both lines `ok`, exit 0;
  - `cue` failing: `cd-exe` reported as skipped and failed, exit 1;
  - stub exiting 1: `cd-exe` quotes its stderr;
  - stub writing `PSXGAME.EXE` instead of `SLIPWAYS.EXE`: `cd-exe` lists the extracted names;
  - one byte changed in the extracted EXE: `cd-exe` names the offset;
  - a missing `dumpsxiso` path: names `make psn00bsdk-toolchain-setup`;
  - a wrong argument count: exit 2.
- **`psx_hello_check.sh`** has no host test today. Its unchanged behaviour is proven by `make psx-hello-cd-check` (and `psx-hello-e2e`) staying green, run once in chunk 1. That is a toolchain run, not part of `make test`.
- **PSX side.** `make psx-game-cd-check` and the three new conditions of `make psx-check`, with the five step-5 game conditions now on the CD boot, are the integration tests of the disc. No unit test runs mkpsxiso, `dumpsxiso` or the emulator.
- **PC net.** Every e2e `*.sha256`, `make state`, `make full-game` and `make sanitize-net` stay unchanged and green.
- Prior art:
  - `test_psx_capture_check` and `test_psx_digest_compare` (scripts on crafted files, in `make psx-tools-test`);
  - step 5's `test_psx_game_check` (H-5-5);
  - `psx_hello_check.sh`'s `cue` branch (moved, not rewritten).

## Out of Scope

- License data, region strings, a Sony BIOS, and booting on real hardware (recorded in "Still to do").
- CD audio, extra files on the disc, reading assets from the CD at run time: the game embeds everything (step 2).
- Memory-card saves: the game keeps the memory save storage of step 5.
- A screenshot or pixel reference of the CD boot. The proof is state-based, and step 4's captures cover drawing.
- Changing `psx-game.exe`, any `src/` file, `psx_boot.sh` or the Lua players.
- Judging or accepting step 4's GPU divergences: that is the human's job, reported as awaiting.
- `make psx-full-game-check` on the disc (plan 11), frame-rate and memory tuning (plan 11), and plan 11's final ship `.bin/.cue`.
- Retiring the EXE boot of `make psx-game-check`, the pad probe, or `psx-hello`.

## Further Notes

- **Why move the boot rather than add one.** The plan rule allows a new boot only when no existing boot can carry the proof. The CD boot carries step 5's proof in full: the same bytes run, and the pad player and the end word work from the handshake, whatever loaded the program. Keeping the EXE boot in `psx-check` as well would prove nothing new there. It stays in `make psx-game-check`, where its quicker turnaround helps controller work.
- **Why `dumpsxiso` rather than trusting the build graph.** `DEPENDS psx-game` orders the build. It does not prove the disc holds the EXE that `magic`, `no-json` and `game-ram-room` read. A byte comparison of the extracted file closes the "EXE checked is the EXE shipped" chain at no boot cost. `dumpsxiso` ships with the pinned SDK, next to `mkpsxiso`, so no new download or host is needed.
- **A disc that lands in the BIOS shell** reaches the ceiling without a `psx-game:` line. `psx_boot.sh` reports "last progress line: (none)", and `game-done` fails with that message. That is the CD-boot failure mode `docs/psx-hello-world.md` warns about, and `game-cue` and `game-cd-exe` usually name its cause first.
- **Risk: `-testmode` exit on a disc boot.** No check has yet left the emulator through the exit register after a disc boot: `psx-hello-cd-check` uses the plain capture mode. If the CD game boot reaches the ceiling although its TTY shows the done line and the state block, the step stops with a report. The boot is not switched back to `--exe`, since the proof must be on the `.cue`.
- **Step note contents ("Step 1.6").**
  - The start gate's result, and any H-5-k found different and how the step adapted.
  - The disc layout (volume, file names, `SYSTEM.CNF`).
  - The `.bin`'s size in bytes and sectors.
  - The CD boot's vsyncs (from power-on and from the first frame), `PSX_GAME_CD_CEILING` and `PSX_GAME_CD_TIMEOUT`.
  - `make psx-check`'s condition count (30), run time and boot count (five).
  - The "Done when" table, with item 3 as awaiting the human, and its pending labels.
