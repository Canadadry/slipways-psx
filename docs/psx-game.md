# PSX game build: the core replayed on MIPS

The game's C core builds, unchanged, as a PS-EXE that replays an e2e recording
inside the headless emulator and prints its `make state` text over TTY.
`make psx-check` proves it plays the same game as the PC, and that scripted pad
input reaches a PS-EXE. Toolchain, emulator, setups and the plain boot mode are
in `docs/psx-hello-world.md`. The Makefile legend and the usage header of each
`tools/psx_*.sh` script hold the per-option detail.

## Build

`make psx-build [EVENTS=<events>]` builds two PS-EXEs in `tmp/psx/build`
(`PSX_BUILD_DIR`), then prints their ELF sizes:

- `psx-replay.{elf,exe}`: the core replaying `EVENTS` (default
  `tests/e2e/pad_slipway.json`);
- `psx-pad-probe.{elf,exe}`: the pad probe.

`psx/CMakeLists.txt` is a PSn00bSDK project. The Makefile hands it the source
lists, nothing is globbed: `CORE_SRCS` (which holds the memory save storage),
the shared state formatter (`src/devtools/state_format.c`), the format self-test,
the PSX platform files of `src/platform/` (`psx_exit`, `psx_tty`, `fatal_psx`,
`assert_psx`, `format_psx`, `psx_pad`), `src/p8/format.c` and the generated
replay source. A file added to `CORE_SRCS` is in the PSX build at once.

It is built `GPREL`, Release (`-O2`), with `-UNDEBUG`: asserts stay compiled
in, as in the PC `-O0` binary. `snprintf`/`vsnprintf` are wrapped onto
`src/p8/format.c`, and `_assert_abort` onto the PSX assert handler. A missing
SDK fails naming `make psn00bsdk-toolchain-setup`.

## Embedded replay

The recording is compiled into the PS-EXE: no file system, CD image or JSON
parser on PSX. `tmp/embed_replay <events> <name> <out.c>` (`src/cmd/embed_replay.c`)
reads it through the PC replay's own loader and writes a C source holding the
frames (`p8_input_frame_t`, 16 bytes a frame), the `initial_save` bytes, the
name and the frame count (`src/devtools/embedded_replay.h`). It refuses a mouse
recording, an unreadable or empty file, and writes nothing then. One PS-EXE per
recording: only the generated source changes.

`src/cmd/psx_replay.c` runs the PC replay loop (input step, `cart_update60`,
`cart_draw`, input advance) into the in-RAM framebuffer, never shown, without
the sprite sheet. `cart_draw` stays: drawing spends random draws. Its TTY:

- `psx-replay: asserts on|off`
- `psx-replay: format <self-test string>` (`src/devtools/format_self_test.c`;
  `tmp/format_self_test` prints the PC's)
- `psx-replay: frame <n>` every 256 frames
- `psx-replay: done <name> <frames> vsyncs <n>`
- the state, each line behind `psx-state: `

It then leaves the emulator with code 0. A `p8_fatal` prints its message,
`stopped p8_fatal frame <n>` and the state, then exits with 3; a failed assert
prints `file:line expr` and exits with 4.

## Until-exit boot mode

`tools/psx_boot.sh <install-dir> --until-exit [--pad-script <file>] --exe <exe> <ceiling> <out-prefix>`
adds `-testmode`, so the program's write to the PCSX exit register quits the
emulator with its code. No capture is written. `<ceiling>` is a vsync ceiling:
reaching it, or `PSX_BOOT_TIMEOUT` seconds, fails naming the input and the last
`psx-replay:` or `psx-pad:` line. Code 0 passes, any other fails naming it.
Without `--until-exit`, `psx_boot.sh` behaves as `docs/psx-hello-world.md`
describes.

## Pad script

`--pad-script` plays a text file on port 1 through the PCSX-Redux pad
overrides (`tools/psx_pad_play.lua`, loaded by `tools/psx_until_exit.lua`).
A line is `<vsync offset> <button>...` or `<vsync offset> -` (nothing held);
offsets increase, the last step is `-`, blank and `#` lines are skipped.
Buttons: `SELECT START UP RIGHT DOWN LEFT L2 R2 L1 R1 TRIANGLE CIRCLE CROSS
SQUARE`. Offsets count from a handshake, not from boot: the probe writes a magic
word into the last scratchpad word once its pad driver runs, and the Lua polls
it each vsync. Ten vsyncs after the last step, the Lua writes an end word just
below it and the probe exits.

`src/cmd/psx_pad_probe.c` prints `psx-pad: <hex4>` (hardware bit order,
1 = pressed) each time the pressed buttons change, then `psx-pad: done`.
`psx/pad_probe_script.txt` is the script of `make psx-check`. The pad driver
does not feed `pad_buttons` yet: the game itself is not driven by the pad.

## Checks

| Target | What it proves | Time |
|---|---|---|
| `make psx-check` | the six conditions below, on `pad_slipway.json` | ~50 s |
| `make psx-replay-check` | the replay proof alone, on `PSX_EVENTS` (outputs in `tmp/psx/replay`) | ~45 s |
| `make psx-pad-check` | the pad proof alone (outputs in `tmp/psx/pad`) | ~5 s |
| `make psx-full-game-check` | the replay proof on `worst-case/full_game.json` (outputs in `tmp/psx/full-game`) | minutes |

`make psx-check` is the plan-10 check target: a later step adds its own
condition to it. It builds both PS-EXEs, writes the PC reference with
`STATE=` and the current `tmp/replay`, boots the replay and the probe until they
exit, then checks every condition before concluding, one line each
(`tools/psx_check.sh`, outputs in `tmp/psx/game-check`):

- `magic`: both `.exe` start with `PS-X EXE`;
- `asserts-on`: the replay printed `asserts on`;
- `format`: its self-test string equals the PC's;
- `replay-done`: the boot exited with 0, its `done` line has the PC's frame
  count, and no `stopped p8_fatal`;
- `state-game-lines`: every state line but `zone`, `budget` and `cap` equals
  the PC's, in order (`tools/psx_state_compare.sh`); a mismatch names the
  first differing line from both sides;
- `pad-script`: the probe's ordered pad states equal the script's
  (`tools/psx_pad_compare.sh`).

`measurements.txt` (in `tmp/psx/game-check/run`, and in the output directory of
`psx-replay-check` and `psx-full-game-check`) holds the MIPS `zone`, `budget`
and `cap` lines, `vsyncs <n>`, and `size` of the replay ELF. The MIPS figures
are lower than the PC's (4-byte pointers): a measurement, not a limit check.

Ceilings and timeouts are the first measured run plus half again:
`PSX_REPLAY_CEILING`/`PSX_REPLAY_TIMEOUT` for `pad_slipway`,
`PSX_PAD_CEILING`/`PSX_PAD_TIMEOUT` for the probe,
`PSX_FULL_GAME_CEILING`/`PSX_FULL_GAME_TIMEOUT` for `full_game`.
`psx-full-game-check` builds in its own directory and is not part of
`psx-check`: at minutes a run, every later step would pay it again.

None of these targets is in `make test-all`, and none runs a setup.

## Still to do

- `make psx-full-game-check` is red, pending for plan 11. Its first run built
  the PS-EXE (text 402028, data 2868, bss 977224) and the PC replay ran its
  4,778 frames, but the emulator aborted after about 9 minutes (exit 134,
  `An unknown exception occured`, `terminate called without an active
  exception` in `run.log`) before the ceiling or the timeout. Its TTY was
  empty: the emulator's stdout is block-buffered into the file and lost on an
  abort, so the frames reached are unknown. Line-buffering that stdout is the
  first thing to try.
