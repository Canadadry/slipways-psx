---
title: "Full-game replay finishes in the emulator"
description: "The emulator aborts near the end of the full-game PSX replay, so nothing measured on the whole game can be trusted until the abort is reproduced in a short run, pinned to the guest or the emulator, and either fixed or worked around."
status: needs-triage
---

## Problem Statement

`make psx-full-game-check` is red. The PS-EXE replaying `worst-case/full_game.json` builds, and the PC replays the 4,778 frames, but after about 9 minutes the emulator dies: exit 134, `An unknown exception occured` and `terminate called without an active exception` in `run.log`. This happens before the vsync ceiling and before the timeout. The last line that reached the TTY was `psx-replay: frame 4608`, with no `p8_fatal` line, and the `done` line never came.

The rest of plan 11 sits on this check. The step 2 measurements, the step 4 optimizations and the step 5 ship all need a full game that runs to its end in the emulator, on the software path and on the GPU path. Today nobody knows whether the abort is a bug in the game's C core on MIPS (the guest) or in pcsx-redux (the emulator). A 9-minute cycle per attempt is too slow to find out by trial.

What the evidence already says, and what it does not:

- `frame 4608` is not the abort frame. Progress lines come every 256 frames, so 4608 is the last one possible before the 4,778-frame end. The abort lies anywhere from frame 4608 to the `done` line, and that range includes the after-loop `map` digest.
- `psx_boot.sh` leaves the emulator's stdout block-buffered, so an abort loses the end of the TTY. Only a manual rerun under `stdbuf -o0` showed `frame 4608`. The boot then fails as "emulator exited with 134 without a result", which says nothing about a signal or how far the run got.
- Every digest that reached the TTY matches the PC's: the sprite sheet, and the screens up to frame 2464 in the buffered run.
- The full-game check boots only the software replay. Nothing boots the GPU replay, the path `psx-game` ships and step 2 instruments, on `full_game.json`.

## Solution

Turn the abort from a 9-minute mystery into a short run that reproduces it, find its cause, and make both full-game replays reach their `done` line with the PC's frame count:

1. Make the boot tell the truth about an abort. The TTY survives it, the failure names the abort and its signal, and the boot leaves behind the last emulated vsync it reached.
2. Rule out the cheap guest causes first, on the host, in seconds. The sanitized PC replay runs `full_game.json`.
3. Take one long background run of each replay with the new reporting, to learn the exact vsync, wall time and last digest of the abort.
4. Reproduce the abort in a short run. Save an emulator state a little before the abort vsync, then boot from that state. The result also tells the two kinds of cause apart:
   - If the abort follows the loaded state, it is driven by what the guest does at that point.
   - If it does not, it comes from host or emulator state built up over the long run.
5. Fix the cause. A guest bug is fixed in the C core, with a test. An emulator bug is fixed from the repository if it can be: a patch applied by the emulator setup, or a pin bump. If it cannot, the step closes on the recorded cause plus a way that lets full-game runs reach the end.
6. Extend `make psx-full-game-check` so that it boots the GPU replay of `full_game.json` as well, and is green on both.
7. Record the cause in `docs/psx-game.md` and in the plan's step note.

If the abort cannot be reproduced in a short run, or cannot be pinned to the guest or the emulator, the PRD run stops as a failure (`FAILED:`). It reports to the human what was tried, what was ruled out and the last frame reached. Steps 2 to 5 wait on that decision.

## User Stories

1. As the developer, I want `make psx-full-game-check` to go green, so that the PSX build is proven to play a whole game the way the PC plays it.
2. As the developer, I want the software replay's `done` line to carry the PC's frame count (4,778), so that a run that stopped early can never pass.
3. As the developer, I want the GPU replay of `full_game.json` to reach its `done` line with the same count, so that the path that ships is proven on the whole game, and step 2 has an EXE to instrument.
4. As the developer, I want the GPU replay's game state lines to equal the PC's, so that "reaches the end" also means "plays the same game".
5. As the developer, I want the cause of the abort written down, so that nobody hunts it again and the later plan steps know the limits of the emulator.
6. As an agent running a long boot, I want the TTY to survive an emulator abort, so that the last progress line in the failure message is the real one and not whatever the buffer had flushed.
7. As an agent, I want a boot that dies on a signal to say so (status, signal name, last progress line, last vsync), so that I can tell an abort from a guest exit code, a ceiling or a timeout without opening the logs.
8. As an agent, I want the boot to leave the last emulated vsync it reached in a file, so that I know when the abort happened even when the guest printed nothing near it.
9. As an agent, I want to save an emulator state at a chosen vsync in one long background run, so that I pay the 9 minutes once.
10. As an agent, I want to boot from a saved state and run to the program's exit, so that I can retry the end of the full game in seconds or a minute, not 9 minutes.
11. As an agent, I want the state-loaded run to use the same ceiling, timeout, abort reporting and result protocol as a normal until-exit boot, so that its result reads the same way.
12. As an agent, I want a bad save-state argument (a missing file, a vsync of 0 or a non-number, a load and a save in the same boot) refused, naming the argument, before the emulator starts, so that a typo does not cost a long run.
13. As an agent, I want the sanitized PC replay run on `full_game.json` first, so that an out-of-bounds access or an undefined behavior in the core is found in seconds rather than through the emulator.
14. As an agent, I want each rung of the investigation and its result recorded, so that if the step stops, the human gets a complete "tried / ruled out / last frame" report.
15. As an agent, I want to know whether the abort follows the guest's state or the emulator's run time, so that I can pin it to the guest or the emulator with evidence, not a guess.
16. As an agent who has pinned the abort to the guest, I want a host test that fails on the bug before the fix, so that the fix follows `tdd` and the bug cannot come back.
17. As an agent who has pinned the abort to the emulator, I want to fix it from the repository when I can (a patch applied by `psx-emulator-setup`, or a newer pinned commit), so that the fix is reproducible by every later setup.
18. As an agent who has pinned the abort to an emulator bug I cannot fix, I want a documented way for full-game runs to reach the end, so that steps 2 to 5 are not blocked.
19. As the human, I want a stopped step to tell me what was tried, what was ruled out and the last frame reached, so that I can make the call the plan reserves for me.
20. As the developer, I want the full-game ceilings and timeouts set from the first green run plus half again, as every other PSX target's are, so that a later slowdown fails loudly but a normal run never trips them.
21. As the developer, I want the full-game check to stay out of `make test-all` and `make psx-check`, so that the fast loop stays fast.
22. As the developer, I want every `*.sha256` in `tests/e2e/` unchanged, so that the game itself does not change.
23. As an agent, I want every full-game boot run in the background and waited on until it exits, so that a run killed by my own command limit is never read as the abort.
24. As the developer, I want `measurements.txt` of the full-game check to carry the GPU replay's vsyncs and meter lines next to the software replay's, so that step 2 starts from a baseline.
25. As the developer, I want `docs/psx-game.md` to lose its "full-game check is red" entry and describe the check as it now is, so that the docs only say what is true.
26. As a later step's implementer, I want the save-state boot to be a supported option of the boot script, so that step 4's "short GPU-path run holding the worst frames" can reuse it.

## Implementation Decisions

### Boot reporting (`psx_boot.sh`, `psx_until_exit.lua`, `psx_portable.sh`)

- The emulator runs with stdout and stderr unbuffered. Use `stdbuf -o0 -e0` when the host has it: GNU coreutils has it, macOS without coreutils does not. `psx_portable.sh` gains the helper that picks it, and falls back to the current call when it is missing. The TTY is small, so unbuffered output costs nothing measurable.
- An until-exit boot that ends with no result file and a status above 128 fails as `psx-boot: <input>: emulator aborted with status <n> (signal <name or number>); last progress line: ...; last vsync: <v> (see <log>)`. The cases with a result file (exit code, ceiling, error) and the timeout keep their current messages.
- `psx_until_exit.lua` writes the emulated vsync count to `<out-prefix>.progress` every 60 vsyncs. It overwrites the file and closes it each time, so it survives a host abort. `psx_boot.sh` passes the path the same way as the result file, and reads it only for the abort and timeout messages. It removes a stale one before the boot, as it already does for `.result`. The ceiling listener stays the single place that counts vsyncs.
- The guest TTY protocol does not change. No finer progress line is added to `psx_replay.c` or `psx_gpu_replay.c`: `psx-check`'s comparisons and its tests rely on the current lines. The vsync heartbeat and the screen digests (every 32 frames on the software path) are enough to place the abort.

### Save-state boot (`psx_boot.sh`, `psx_until_exit.lua`)

- Two new until-exit options:
  - `--save-state-at <vsync> <file>` writes the emulator's save state to `<file>` when the vsync count reaches `<vsync>`, then the boot carries on.
  - `--load-state <file>` loads that state before the first vsync of the boot, then runs to the program's exit under the usual ceiling and timeout.

  They are mutually exclusive, both need `--until-exit`, and they are refused naming the argument when the vsync is not a positive integer or the file to load is missing. Use the pcsx-redux Lua save-state API of the pinned commit (`PCSX.createSaveState` and `PCSX.loadSaveState`, or what that commit calls them). The implementer checks the names against the installed emulator before writing the Lua.
- A boot from a loaded state has its vsync count start at 0, so its ceiling and `done` vsyncs count from the load. Its `done` line's vsync figure is not compared with anything. Its frame count is still the full 4,778, because the guest's own counter came back with the state.
- Fallback when the pinned commit has no usable save-state API: the short run is the same boot with `-dynarec` in place of `-interpreter`. It is a separate option, `--dynarec`, for investigation only. Checks keep the interpreter, because their captures depend on its determinism. If neither gives a run much shorter than 9 minutes that reproduces the abort, the step stops (see "Stop condition").

### Investigation order (recorded in the note, rung by rung)

1. Host probes, seconds:
   - the sanitized PC replay (`replay_sanitize`, ASan and UBSan) on `full_game.json`;
   - the existing PC replay with `STATE=`, to confirm that the PC itself reaches 4,778 cleanly.

   A sanitizer finding is a guest bug: fix it through `tdd`, then go on to rung 5.
2. One background boot of each full-game PS-EXE (software and GPU) with the new reporting. Record:
   - for an abort: the abort vsync (from `.progress`), the wall time, the last progress line, the last `screen` digest frame, and the tail of `run.log`;
   - for a GPU boot that does not abort: that fact, its vsyncs and its exit.
3. Short repro: one background boot with `--save-state-at` at about 600 vsyncs before the recorded abort vsync, then `--load-state` boots from that file. The repro counts as short when it runs well under a minute of wall time.
4. Pin the abort:
   - **Abort follows the loaded state:** the guest's actions at that point trigger it. Narrow it with repeated short boots, for example by saving states closer to the abort and checking the guest PC and the GPU and DMA activity through the emulator's own logging. Then decide:
     - guest bug: a bad pointer, an out-of-range GPU or DMA command, stack overrun, or memory past `ram-room`;
     - emulator bug: a legitimate guest action that makes pcsx-redux throw.
   - **Abort does not follow the loaded state:** host or emulator state built up over the run causes it, such as host memory growth, a host-side counter, or something in the Lua listeners. Sample the emulator's resident memory during a long background boot, and compare with a `-dynarec` boot. That places it in the emulator, or in the repository's Lua.
5. Fix:
   - **Guest:** the fix in the core or the PSX platform, with a test that fails before it (host unit test where the bug is host-reproducible). Every e2e `*.sha256` stays unchanged.
   - **The repository's Lua or boot script:** fix it there.
   - **Emulator, fixable from the repository:** one of two forms, in order of preference.
     - (a) A newer pinned commit in `psx_sources.txt` that fixes it. Its stamp changes, so `psx-emulator-setup` rebuilds, run in the background.
     - (b) A patch file in the repository applied by `install_psx_emulator.sh` after the headless fix. It must apply cleanly or the setup fails naming it, and it is part of the stamp.
   - **Emulator, not fixable from the repository:** a workaround that lets the full-game runs reach their end, documented with the cause. Preferred: an engine switch for the full-game boots only, such as `-dynarec`, if it does not abort. Otherwise: the full-game boot split at a vsync by save and load into two emulator processes, with the two TTYs concatenated before the comparison. The guest still prints its own `done` line and state.

### Stop condition

- The PRD run ends as `FAILED:`, not DONE, when either of these happens:
  - rung 3 does not reproduce the abort in a short run with either method;
  - rung 4 cannot pin it to the guest or the emulator with evidence.
- The report lists each rung tried and its result, what was ruled out, and the last frame reached (last progress line, last screen digest, abort vsync).
- The changes to boot reporting and save state are still committed: they are verified by their own tests and help the human's decision.

### Full-game check (`make psx-full-game-check`)

- It keeps building in its own directory and writing to its own output directory. It now boots two PS-EXEs of `full_game.json` from that build:
  - the software replay, as today;
  - the GPU replay, with a capture script that holds no entries, so that the capture handshake runs but nothing is captured. The implementer confirms that `psx_capture_frames.lua` hands `NO_FRAME` on an empty script. If it does not, the Lua is fixed so that it does, with its test.
- Conditions, all checked before concluding, one line each, fail if any fails:
  - `replay-done` and `gpu-replay-done`: exit 0, `done` frame count equals the PC's, no `stopped p8_fatal`;
  - `state-game-lines` and `gpu-state-game-lines`;
  - `sprite-sheet`, `screen` and `map` digests on the software path.

  Reuse the condition functions `psx_check.sh` already has, without duplicating them: lift them into a shared sourced file or a script that both call.
- `measurements.txt` gains the GPU replay's `vsyncs`, its `zone gpu_frame` and `cap gpu_*` lines and its ELF `size`, behind `gpu `, as `psx-check`'s does.
- Separate ceilings and timeouts: `PSX_FULL_GAME_CEILING` and `PSX_FULL_GAME_TIMEOUT` for the software replay, and new `PSX_FULL_GAME_GPU_CEILING` and `PSX_FULL_GAME_GPU_TIMEOUT` for the GPU replay. They keep generous values until the first green run, then are set to that run's measurements plus half again. A run that hits the agent's command limit is not a measurement.
- The two boots run one after the other within the target. The agent launches the whole target in the background and waits for it to exit. It stays out of `make psx-check` and `make test-all`.

### Docs

- `docs/psx-game.md`:
  - "Checks": the full-game row and the until-exit section describe the two boots, the new conditions and the new options (abort reporting, `.progress`, save and load state, and `--dynarec` if it was added).
  - "Still to do": the red-check entry is replaced by the recorded cause in one short paragraph. If a workaround was needed, it says which and why, and the entry stays as an open emulator issue.
- The Makefile legend for `psx-full-game-check` and the usage headers of `psx_boot.sh` and `psx_until_exit.lua` are updated to match.
- The step note in `docs/plan-11.md` records the cause, the investigation's rungs and results, both boots' frame and vsync counts, and the ceilings chosen.

## Implementation Chunks

1. **Boot reporting.** Unbuffered emulator output, the abort message with signal and last vsync, and the `.progress` heartbeat. Tested on a stub emulator, with no real boot.
2. **Save-state boot.** `--save-state-at` and `--load-state` (plus `--dynarec` only if the API is missing), with argument validation tested on the stub. Real use is checked by one short real boot of `pad_slipway`, run in the background: save at a vsync, then load, and the program still exits 0 with its `done` line.
3. **Full-game check with the GPU boot.** The shared condition functions, the GPU boot with an empty capture script, the new conditions and measurements, and the new ceiling and timeout variables. Tested on crafted TTYs, no emulator.
4. **Investigation and fix.** Rungs 1 to 5 above. The fix (guest, Lua, or emulator patch or pin) lands with its test where one is possible. Then the full-game check runs green in the background, the ceilings are set from it, and the docs and step note are written. If the stop condition hits, this chunk ends the run as `FAILED:` with the report, and chunks 1 to 3 stay committed.

## Testing Decisions

- A good test drives the boot and check scripts through their command line and checks what they print and their exit status, on crafted inputs. Tests never boot the emulator. The real boots are the target runs, in the background.
- New `test_psx_boot`, in `PSX_TOOLS_TESTS` (`make psx-tools-test`, outside `make test-all`). It builds a stub install directory: the stamp, the OpenBIOS file and an executable `pcsx-redux` shell stub driven by an environment variable. It checks that:
  - a stub that prints `psx-replay: frame 4608`, writes a `.progress` of `vsync 7000` and kills itself with SIGABRT makes the boot fail with the abort message naming signal, last progress line and last vsync;
  - a stub that writes `exit 120` to the result file and exits 0 passes, as today;
  - `ceiling`, `error` and timeout keep their messages;
  - each bad `--save-state-at` and `--load-state` form is refused naming the argument, and the stub never runs;
  - the arguments the stub receives include the save or load request in the form the Lua reads.
- Prior art: `test_psx_game_check`, `test_psx_state_compare` and `test_psx_capture_check`, which are C tests running the shell scripts on crafted TTY files in a `tmp/test_*_work` directory.
- The full-game conditions are tested like `test_psx_game_check`. A crafted software TTY and a crafted GPU TTY:
  - both with a `done` line of the right count pass;
  - a missing GPU `done` line, a wrong count or a `stopped p8_fatal` fails, naming the condition;
  - after the shared functions are lifted, `psx_check.sh`'s existing tests stay green unchanged.
- A guest fix gets a host unit test in `tests/test_<module>.c` that fails before the fix. Where the bug only shows on MIPS, the test reproduces its precondition on the host, for example a pointer-width or alignment case. The full-game check is the proof on MIPS.
- `make test-all` passes on every commit and is not slower. The new tests go in `psx-tools-test` only.
- The step is verified on the targets it names:
  - `make psx-full-game-check` green, both `done` lines showing 4,778 (the PC's count from its `pc.log`);
  - `make psx-check` still green, because the boot script and condition functions changed under it.

  Both run in the background and count only if they exit on their own.

## Out of Scope

- Per-frame timing, zone peaks and primitive buffer measurement on the GPU path (step 2). This step only makes the GPU replay reach its end and records its total vsyncs and meter lines.
- Re-sizing zones from MIPS peaks (step 3), any optimization (step 4), and the final `.bin/.cue` (step 5).
- Making the full-game runs faster than they need to be to finish. A slow run that finishes is green.
- Changing the replays' TTY protocol or the PC replay.
- Audio, the hardware renderers, and a Sony-BIOS disc.
- Running any setup from the PRD writer. The implementer may rerun `psx-emulator-setup` in the background only when the fix is an emulator patch or pin.

## Further Notes

- The abort reads like a C++ exception escaping pcsx-redux's main loop: `An unknown exception occured`, then `std::terminate` on a joinable thread at teardown. That points to the emulator's host code, but the trigger can still be a guest action, such as a malformed DMA or GPU command, or a jump into garbage that the interpreter handles badly. The save-state test exists to separate these. Do not conclude from the message alone.
- A 32-bit cycle counter would wrap at about 7,600 vsyncs, far earlier than this run gets, so a plain 32-bit wrap is unlikely. Record the abort vsync anyway and compare it with powers of two.
- The `pad_slipway` software replay measured about 2,670 vsyncs for 318 frames (its ceiling is 4,001, the first run plus half again). If that rate holds, the full game runs about 38,000 vsyncs, under the 60,000 ceiling. Use the abort vsync, not this estimate.
- The plan's rule holds throughout. Full-game and setup runs go in the background and are waited on until they exit. A run that the agent's command limit killed is never evidence.
