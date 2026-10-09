# PSX game build: the core replayed on MIPS

The game's C core builds, unchanged, as a PS-EXE that replays an e2e recording
inside the headless emulator and prints its `make state` text and VRAM digests
over TTY, and as a second PS-EXE that replays it through the GPU drawing
backend. `make psx-check` proves they play the same game as the PC, that the
software one draws the same pictures, that the GPU one receives the same draw
calls and fits in RAM, and that scripted pad input reaches a PS-EXE. Toolchain, emulator, setups and the plain boot mode are
in `docs/psx-hello-world.md`. The Makefile legend and the usage header of each
`tools/psx_*.sh` script hold the per-option detail.

## Build

`make psx-build [EVENTS=<events>]` builds three PS-EXEs in `tmp/psx/build`
(`PSX_BUILD_DIR`), then prints their ELF sizes:

- `psx-replay.{elf,exe}`: the core replaying `EVENTS` (default
  `tests/e2e/pad_slipway.json`), drawing into the RAM framebuffer;
- `psx-gpu-replay.{elf,exe}`: the same replay drawing through the GPU backend;
- `psx-pad-probe.{elf,exe}`: the pad probe.

`psx/CMakeLists.txt` is a PSn00bSDK project. The Makefile hands it the source
lists, nothing is globbed: `CORE_SRCS` (which holds the memory save storage
and the embedded sprite sheet, `src/p8/sprite_sheet.c`), the shared state
formatter (`src/devtools/state_format.c`), the shared VRAM digest
(`src/devtools/vram_digest.c`), the format self-test,
the PSX platform files of `src/platform/` (`psx_exit`, `psx_tty`, `fatal_psx`,
`assert_psx`, `format_psx`, `psx_pad`), `src/p8/format.c` and the generated
replay source. The GPU replay swaps the VRAM digest for the GPU backend
(`src/platform/psx_gpu.c`) and the VRAM layout
(`src/platform/psx_vram_layout.c`). A file added to `CORE_SRCS` is in the PSX
build at once.

It is built `GPREL`, Release (`-O2`), with `-UNDEBUG`: asserts stay compiled
in, as in the PC `-O0` binary. `snprintf`/`vsnprintf` are wrapped onto
`src/p8/format.c`, and `_assert_abort` onto the PSX assert handler. A missing
SDK fails naming `make psn00bsdk-toolchain-setup`.

## Embedded assets

No asset is read from a file, on PC or PSX. The sprite sheet is
`src/p8/sprite_sheet_packed.h`, 8,192 bytes at 4 bits per pixel (the PSX GPU's
4-bit texture layout), regenerated from `pico8/gfx.json` by `make sprite-sheet`;
`p8_sprite_sheet_load` unpacks it into `p8_vram_t.sprite_sheet`. The map has no
data: `background.c` builds it at runtime with `p8_mset`. The font is the
`static const` glyph tables of `src/p8/gfx.c`. No JSON loader
(`src/p8/assets.c`, `src/devtools/input_json.c`) is in `CORE_SRCS`, and
`psx-check`'s `no-json` checks the ELF for it.

## Embedded replay

The recording is compiled into the PS-EXE: no file system, CD image or JSON
parser on PSX. `tmp/embed_replay <events> <name> <out.c>` (`src/cmd/embed_replay.c`)
reads it through the PC replay's own loader and writes a C source holding the
frames (`p8_input_frame_t`, 16 bytes a frame), the `initial_save` bytes, the
name and the frame count (`src/devtools/embedded_replay.h`). It refuses a mouse
recording, an unreadable or empty file, and writes nothing then. One PS-EXE per
recording: only the generated source changes.

`src/cmd/psx_replay.c` runs the PC replay loop (input step, `cart_update60`,
`cart_draw`, input advance) into the in-RAM framebuffer, never shown, after
loading the embedded sprite sheet. `cart_draw` stays: drawing spends random
draws. Its TTY:

- `psx-replay: asserts on|off`
- `psx-replay: format <self-test string>` (`src/devtools/format_self_test.c`;
  `tmp/format_self_test` prints the PC's)
- the VRAM digest lines of `src/devtools/vram_digest.h`, each behind
  `psx-digest: `, at the points where the PC replay writes them with
  `DIGEST=<file>`: `sprite-sheet <fnv1a>` once loaded, `screen <frame> <fnv1a>`
  after `cart_draw` on every 32nd frame from 0 and on the last, `map <fnv1a>`
  after the loop
- `psx-replay: frame <n>` every 256 frames
- `psx-replay: done <name> <frames> vsyncs <n>`
- the state, each line behind `psx-state: `

It also installs the draw meter (below), so its `zone gpu_frame` and `cap
gpu_*` lines are measured like the PC's. It then leaves the emulator with
code 0. A `p8_fatal` prints its message,
`stopped p8_fatal frame <n>` and the state, then exits with 3; a failed assert
prints `file:line expr` and exits with 4.

## Drawing backend

The drawing API of `src/p8/gfx.h` keeps its names and signatures. Each draw
call (`cls`, `pset`, `line`, `rect`, `rectfill`, `circ`, `circfill`, `spr`,
`print`) goes to the active `p8_draw_backend_t` (`src/p8/draw_backend.h`),
with the graphics state and the VRAM struct as they are at call time and the
API's own arguments (world coordinates, full colour byte). `map` (one `spr` a
non-empty cell) and `spr_default` stay in the front end, as do the state calls
(`camera`, `fillp`, `palt`, `mset`, the draw palette): a backend reads them,
never writes them. `p8_draw_backend_set` installs one process-wide and returns
the previous one; NULL, the default, is the software backend
(`src/p8/gfx_software.c`), which rasterises into `p8_vram_t.screen` for the
PC game, the e2e and every test.

**`fillp`, `palt` and the draw palette on the GPU: per-texture CLUT and
pattern textures, no software fallback** (decided; step 4 implements it).

- `spr` reads its texture through a CLUT built from `mem->draw_palette` at
  the call: entry `c` is 0x0000 (transparent) when bit 0x80 is set, else the
  RGB555 of `P8_PALETTE_RGB[draw_palette[c] & 0x0F]` with the mask bit 0x8000,
  so black stays opaque. CLUTs are cached in VRAM, keyed by the 16 bytes; a
  palette `hud.c` copied with `memcpy` needs nothing more.
- A `rectfill` or `circfill` span under a non-zero pattern is a `SPRT` of an
  8x8 4-bit texture (the 4x4 pattern tiled, texel 1 for a set bit) repeated
  by the texture window, with a 2-entry CLUT: `col & 0x0F`, then
  `(col >> 4) & 0x0F` or 0x0000 under `fillp_transparent`. The pattern is
  indexed by world coordinates, before the camera (U/V origin = world
  coordinate mod 8), as the software `fillp_color` does. A slot is keyed by
  pattern, colour byte and transparent flag. A span without pattern is a
  `TILE`.
- Rejected: rasterising those calls in RAM would need the framebuffer
  uploaded every frame, the copy the GPU path replaces.

**GPU primitive buffers: counted on PC, sized at peak + 1/8, outside
`ZONE_BUDGET_BYTES`, checked against the MIPS RAM.** `src/p8/draw_cost.c`
gives each call's GPU bytes and primitives (PSn00bSDK 0.24 packet sizes, an
upper bound; a call whose screen box misses 160x120 costs 0), and the draw
meter (`src/p8/draw_meter.c`) wraps the active backend and adds them up a
frame, with the distinct draw palettes and fill patterns. The PC replay
installs it under `STATE=`, so `make state` reports `zone gpu_frame` and `cap
gpu_cluts`/`cap gpu_patterns` over every e2e and worst-case game, judged by
the 1/8 margin. `src/p8/gpu_sizes.h` holds the sizes:
`GPU_FRAME_BUFFER_BYTES`, `GPU_CLUT_SLOTS`, `GPU_PATTERN_SLOTS`. The GPU
backend reserves two buffers (the CPU fills one while the GPU draws the
other) as static data, counted in no zone; `make state` prints their total
after the bytes left under `ZONE_BUDGET_BYTES`, and `psx-check`'s `ram-room`
checks they fit.

## GPU replay

`src/cmd/psx_gpu_replay.c` is the replay loop and TTY protocol of
`psx_replay.c`, plus `psx-replay: backend gpu` after the format line, without
`psx-digest:` lines (the RAM screen is not drawn). It still loads the RAM
sprite sheet, so the game sees the same memory. It installs the GPU backend
wrapped by the draw meter; each frame ends with the meter's frame end, then
`psx_gpu_frame_end` with the metered bytes, then `psx_gpu_frame_begin`.

The GPU backend skeleton (`src/platform/psx_gpu.c`, PSn00bSDK, no floating
point, no `malloc`):

- init: NTSC 320x240 display on two display buffers, the packed sprite sheet
  (`p8_sprite_sheet_packed`, already the 4-bit texture layout) uploaded;
- each frame: draw area and offset on the 160x120 draw area, the texture
  page, no texture window, then the primitives in call order (no depth sort)
  in a bump allocator over that frame's static buffer of
  `GPU_FRAME_BUFFER_BYTES`; the frame ends on the x2 upscale quad into the
  frame's display buffer, waits for the previous frame's `DrawSync`, shows it
  and submits this one, with no `VSync` wait;
- overflow of the buffer is a `p8_fatal` naming the frame and the bytes
  asked; a frame that sent more bytes than the meter counted is a `p8_fatal`
  naming the frame and both figures;
- `cls` is a `FILL` of the draw area; the eight other entries draw nothing
  yet;
- the cost model's byte sizes are checked against the SDK's `sizeof` at
  compile time.

VRAM layout (`src/platform/psx_vram_layout.c`, host-tested by
`test_psx_vram_layout`; a region moves by changing that module only), in
16-bit units:

| region | x, y | size | notes |
|---|---|---|---|
| display buffers | (0, 0), (0, 240) | 320x240 each | |
| draw area | (320, 0) | 160x120 | 15-bit texture page (320, 0) for the upscale |
| sprite sheet | (512, 0) | 32x128 | 4-bit page (512, 0): u 0..127, v 0..127 |
| font | (512, 128) | 32x128 | same page, v 128..255 |
| pattern slots | (544, 0) | 32x256, 2x8 a slot | same page, u 128..255 |
| CLUTs | (640, 0) | 16x64, 16x1 each | `GPU_CLUT_SLOTS` draw palettes, 16 font colours, `GPU_PATTERN_SLOTS` pattern CLUTs |

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
| `make psx-check` | the fourteen conditions below, on `pad_slipway.json` | ~60 s |
| `make psx-replay-check` | the replay proof alone (game lines and digests), on `PSX_EVENTS` (outputs in `tmp/psx/replay`) | ~45 s |
| `make psx-pad-check` | the pad proof alone (outputs in `tmp/psx/pad`) | ~5 s |
| `make psx-full-game-check` | the replay proof on `worst-case/full_game.json` (outputs in `tmp/psx/full-game`) | minutes |

`make psx-check` is the plan-10 check target: a later step adds its own
condition to it. It builds the PS-EXEs, writes the PC references with
`STATE=` and `DIGEST=` in one run of the current `tmp/replay`, boots the
replay, the GPU replay and the probe until they exit, then checks every
condition before concluding, one line each (`tools/psx_check.sh`, outputs in
`tmp/psx/game-check`):

- `magic`: the three `.exe` start with `PS-X EXE`;
- `asserts-on`: the replay printed `asserts on`;
- `format`: its self-test string equals the PC's;
- `replay-done`: the boot exited with 0, its `done` line has the PC's frame
  count, and no `stopped p8_fatal`;
- `state-game-lines`: every state line but `zone`, `budget` and `cap` equals
  the PC's, in order (`tools/psx_state_compare.sh`); a mismatch names the
  first differing line from both sides;
- `sprite-sheet`, `screen`, `map`: its digest lines of that kind equal the
  PC's, in order (`tools/psx_digest_compare.sh`); a missing or extra line
  fails, a mismatch names the first differing line from both sides. Equal
  screens prove the sprites, the generated map and the font draw the same on
  MIPS;
- `gpu-replay-done`, `gpu-state-game-lines`: `replay-done` and
  `state-game-lines` on the GPU replay;
- `gpu-cost`: its `zone gpu_frame`, `cap gpu_cluts` and `cap gpu_patterns`
  lines equal the PC's: the backend gets the PC's exact draw calls on MIPS;
- `ram-room`: 65,536 (kernel area) + text + data + bss of the GPU ELF (the
  toolchain's `size`, `PSX_SIZE`; the zones and both primitive buffers
  included) + `PSX_STACK_RESERVE` (64 KiB) <= 2,097,152; the line prints the
  bytes left;
- `no-json`: the toolchain's `nm` (`PSX_NM`) finds no definition of
  `p8_load_gfx_json`, `p8_input_load`, `p8_input_save`, `p8_input_append` or
  `p8_input_free` in either replay ELF;
- `pad-script`: the probe's ordered pad states equal the script's
  (`tools/psx_pad_compare.sh`).

`measurements.txt` (in `tmp/psx/game-check/run`, and in the output directory of
`psx-replay-check` and `psx-full-game-check`) holds the MIPS `zone`, `budget`
and `cap` lines, `vsyncs <n>`, and `size` of the replay ELF; `psx-check`'s
adds the GPU replay's vsyncs, meter lines, `ram-room <used> <limit> <left>`
and `size`, each behind `gpu `. The MIPS figures
are lower than the PC's (4-byte pointers): a measurement, not a limit check.

Ceilings and timeouts are the first measured run plus half again:
`PSX_REPLAY_CEILING`/`PSX_REPLAY_TIMEOUT` for `pad_slipway`,
`PSX_GPU_CEILING`/`PSX_GPU_TIMEOUT` for the GPU replay of `pad_slipway`,
`PSX_PAD_CEILING`/`PSX_PAD_TIMEOUT` for the probe,
`PSX_FULL_GAME_CEILING`/`PSX_FULL_GAME_TIMEOUT` for `full_game`.
`psx-full-game-check` builds in its own directory and is not part of
`psx-check`: at minutes a run, every later step would pay it again.

None of these targets is in `make test-all`, and none runs a setup.

## Still to do

- Step 4 of plan 10: the GPU families, one at a time, each checked by a VRAM
  capture against the PC framebuffer of the same frame: `pset`, `line`,
  `rect`, `rectfill` and `circfill` (with their fill patterns and the pattern
  slots), `circ`, `spr` and `map` (with the draw-palette CLUT cache), `print`
  (with the font texture and its 16 CLUTs). Whether the x2 upscale samples
  exactly 2x2 a texel is checked by the first capture.

- `make psx-full-game-check` is red, pending for plan 11. The PS-EXE builds
  (text 412552, data 2868, bss 977224) and the PC replay runs its 4,778
  frames, but the emulator aborts after about 9 minutes (exit 134,
  `An unknown exception occured`, `terminate called without an active
  exception` in `run.log`), before the ceiling or the timeout. A rerun with
  the emulator's stdout unbuffered (`stdbuf -o0`; `psx_boot.sh` leaves it
  block-buffered, so an abort loses the TTY) reached `psx-replay: frame 4608`
  with no `p8_fatal` line, then aborted the same way before the `done` line.
  The cause, in the guest or in the emulator, is not found yet. The digest
  lines that reach the TTY before the abort equal the PC's: the sprite sheet
  and every screen up to frame 2464.
