# PSX game build: the core replayed on MIPS

The game's C core builds, unchanged, as a PS-EXE that replays an e2e recording
inside the headless emulator and prints its `make state` text and VRAM digests
over TTY, and as a second PS-EXE that replays it through the GPU drawing
backend. `make psx-check` proves they play the same game as the PC, that the
software one draws the same pictures, that the GPU one receives the same draw
calls and fits in RAM, that its frame captures are deterministic and within
their recorded divergence from the PC, and that scripted pad input reaches a
PS-EXE. Toolchain, emulator, setups and the plain boot mode are
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
(`src/platform/psx_gpu.c`), the VRAM layout
(`src/platform/psx_vram_layout.c`) and the draw family filter
(`src/devtools/draw_filter.c`). A file added to `CORE_SRCS` is in the PSX
build at once.

It is built `GPREL`, Release (`-O2`), with `-UNDEBUG`: asserts stay compiled
in, as in the PC `-O0` binary. `snprintf`/`vsnprintf` are wrapped onto
`src/p8/format.c`, and `_assert_abort` onto the PSX assert handler. A missing
SDK fails naming `make psn00bsdk-toolchain-setup`. `psx_tty_write` masks
IRQs around each BIOS `putchar`: an IRQ taken on the jump into it makes
PCSX-Redux log that character twice, which broke a state line.

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
pattern textures, no software fallback** (decided; step 4 implements it,
`spr` done).

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
wrapped by the draw family filter, itself wrapped by the draw meter (so the
meter sees every call); each frame ends with the meter's and the filter's
frame ends, then `psx_gpu_frame_end` with the metered bytes, then
`psx_gpu_frame_begin`. It is driven by a capture script (below): without one
it waits forever before its first frame.

The GPU backend skeleton (`src/platform/psx_gpu.c`, PSn00bSDK, no floating
point, no `malloc`):

- init: NTSC 320x240 display on two display buffers, the packed sprite sheet
  (`p8_sprite_sheet_packed`, already the 4-bit texture layout) uploaded;
- each frame: draw area and offset on the 160x120 draw area, the texture
  page, no texture window, then the primitives in call order (no depth sort)
  in a bump allocator over that frame's static buffer of
  `GPU_FRAME_BUFFER_BYTES`; the frame ends on a black `FILL` of the frame's
  display buffer and the x2 upscale quad into it (the quad skips 0x0000
  texels, which black `FILL`s and `TILE`s write, so the clear keeps the
  previous frame from showing through), waits for the previous frame's
  `DrawSync`, shows it and submits this one, with no `VSync` wait;
- overflow of the buffer is a `p8_fatal` naming the frame and the bytes
  asked; a frame that sent more bytes than the meter counted is a `p8_fatal`
  naming the frame and both figures;
- `cls` is a `FILL` of the draw area; `pset` is one `TILE_1`, `rect` up to
  four `TILE`s (its edges, or one when it has no inside) and a plain
  `rectfill` one `TILE`, after the camera and clipped to the screen
  (`src/platform/psx_tiles.c`, host-tested against the software backend by
  `test_psx_tiles`), coloured with the 8-bit components of
  `P8_PALETTE_RGB[col & 0x0F]`; `line` is one `LINE_F2` between its
  endpoints after the camera, unclipped (the GPU clips to the draw area),
  sent when its box meets the screen, so its stepping is the GPU's, not
  PICO-8's (the plan's accepted divergence, measured by its capture);
  `circ` is one `TILE_1` per on-screen pixel and a plain `circfill` one
  `TILE` span per screen row, both from the software backend's midpoint
  loop (`psx_tiles.c`, host-tested pixel for pixel), so circles match the
  PC exactly; `spr` (and `map` through it) is one `SPRT` from the sheet's
  4-bit page, after the camera and clipped to the screen
  (`src/platform/psx_sprite.c`, host-tested pixel for pixel by
  `test_psx_sprite`; a sprite reading past the sheet is an assert, which no
  replay reaches), raw texture, through the CLUT of the call's draw palette;
  a frame's CLUTs are cached by their 16 draw-palette bytes
  (`src/platform/psx_clut_cache.c`, built by `psx_gpu_textures_clut`, one
  cache per primitive buffer), and uploaded to their palette CLUT lines
  with `LoadImage` after the previous frame's `DrawSync` and before this
  frame's `DrawOTag`, so no line changes while a frame reads it; more than
  `GPU_CLUT_SLOTS` distinct palettes in a frame is a `p8_fatal` naming the
  frame; a patterned `rectfill` or `circfill` and `print` draw nothing yet;
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

`tools/psx_boot.sh <install-dir> --until-exit [--pad-script <file>] [--capture-script <file>] --exe <exe> <ceiling> <out-prefix>`
adds `-testmode`, so the program's write to the PCSX exit register quits the
emulator with its code. No capture is written, except the frame-locked ones of
`--capture-script` (below). `<ceiling>` is a vsync ceiling:
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

## GPU family captures

Each GPU draw family is checked by a capture of one frame of the check replay,
drawn with a family mask, against the PC software frame of the same frame and
mask.

- **Family filter** (`src/devtools/draw_filter.c`, PC and PSX): a backend
  wrapper that drops the calls outside its mask and counts, per family, the
  calls the cost model costs above 0. The mask bits, in order: `cls pset line
  rect rectfill circ circfill spr print pattern`; `cls` is always set, and a
  patterned `rectfill`/`circfill` needs its shape bit and `pattern`. The mask
  applies only on a captured frame, "all" otherwise.
- **Capture list** `psx/gpu_captures.txt`: `replay <name>`, then
  `<label> <frame> <mask>` a capture (parser `src/devtools/capture_list.c`).
  With `CAPTURE_LIST=` and `CAPTURE_DIR=`, `tmp/replay` refuses a bad list
  naming its line, and writes `pc_frames.bin` (the listed frames, screencast
  format), `counts.txt` (`<frame> <family> <calls>`) and `capture_script.txt`
  (`<frame> <mask hex>`).
- **Frame-locked capture**: the GPU replay and `tools/psx_capture_frames.lua`
  hand over on three scratchpad words below the pad script's
  (`src/platform/psx_capture_protocol.h`; `test_psx_capture_protocol` checks
  the Lua's constants equal the header's). The Lua hands the next frame and
  mask; at that frame the program draws with the mask, ends the frame,
  `psx_gpu_frame_show_last()` settles the GPU and shows it, prints
  `psx-replay: capture <frame> <mask>` and waits; one vsync later the Lua
  writes `<out-prefix>.f<frame>.vram` (`psx-vram 320 240 16` and the
  `takeScreenShot()` bytes) and releases it.
- **Comparator** (`src/devtools/capture_compare.c`, host tool
  `tmp/psx_capture_compare`, run by `tools/psx_capture_report.sh`): checks the
  header and size, that each 2x2 block is uniform (`nonuniform`), maps each
  pixel back to a PICO-8 colour by exact RGB555 equality
  (`src/platform/psx_colour.c`, `offpalette` otherwise), and writes one report
  line `<label> f<frame> mask <names> calls <n> differ <n> (<p>‰) near1 <n>
  near2 <n> far <n> bbox <x0>,<y0>-<x1>,<y1> offpalette <n> nonuniform <n>`,
  a text diff map `<label>-f<frame>.diff.txt` (`.` equal, the PSX colour's hex
  digit or `?` where not) and an image `<label>-f<frame>.ppm` (PC, PSX and
  diff side by side). `near1`/`near2` count the differing pixels whose PSX
  colour is in the PC frame within 1 or 2 pixels, `far` the others. The `rect`
  and `rectfill` captures, on real colours, confirm the channel order (red in
  bits 0-4) and that the x2 upscale is exact: zero `offpalette`, zero
  `nonuniform`.
- **Ceilings and references**: until a human accepts a family, its captures'
  `differ` may not exceed their line in `psx/gpu_capture_ceilings.txt`,
  written only by `make psx-gpu-capture-ceilings LABEL=<label>` from the last
  `make psx-check` run. `make psx-gpu-capture-accept LABEL=<label>` writes the
  sha256 of a label's captures into `psx/gpu_capture_references.txt`; a capture
  with a reference must match it byte for byte. Only a human runs it.

## Checks

| Target | What it proves | Time |
|---|---|---|
| `make psx-check` | the twenty-one conditions below, on `pad_slipway.json` | ~75 s |
| `make psx-gpu-capture-report` | the GPU captures and their reports alone (outputs in `tmp/psx/gpu-capture`) | ~13 s |
| `make psx-replay-check` | the replay proof alone (game lines and digests), on `PSX_EVENTS` (outputs in `tmp/psx/replay`) | ~45 s |
| `make psx-pad-check` | the pad proof alone (outputs in `tmp/psx/pad`) | ~5 s |
| `make psx-full-game-check` | the replay proof on `worst-case/full_game.json` (outputs in `tmp/psx/full-game`) | minutes |

`make psx-check` is the plan-10 check target: a later step adds its own
condition to it. It builds the PS-EXEs, writes the PC references with
`STATE=`, `DIGEST=` and `CAPTURE_LIST=` in one run of the current
`tmp/replay`, boots the replay, the GPU replay (twice, both with the capture
script) and the probe until they exit, then checks every condition before
concluding, one line each (`tools/psx_check.sh`, outputs in
`tmp/psx/game-check`, conditions also in `run/conditions.txt`):

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
- `gpu-capture-list`: the list's replay is the PS-EXE's, the PC run wrote
  its frames to the capture script, and each label but `all` has an
  on-screen call of its family at its frame on PC;
- `gpu-captures`: every listed frame has a well-formed `gpu.f<frame>.vram`,
  and the TTY's capture lines are exactly the list's frames, in order;
- `gpu-capture-deterministic`: the second boot's captures are
  byte-identical, else the first differing frame is named;
- `gpu-capture-clean`: zero `nonuniform` and `offpalette` in every report;
- `gpu-capture-report`: a report line, diff map and image per capture;
- `gpu-capture-ceilings`: every capture without a matching reference has a
  ceiling and its `differ` is not above it (`below ceiling: ...` is printed
  when lower); a ceiling for an unlisted capture fails;
- `gpu-capture-references`: a capture with a reference matches it; one
  without prints `pending human acceptance: <label> f<frame>`; a reference
  for an unlisted capture fails;
- `no-json`: the toolchain's `nm` (`PSX_NM`) finds no definition of
  `p8_load_gfx_json`, `p8_input_load`, `p8_input_save`, `p8_input_append` or
  `p8_input_free` in either replay ELF;
- `pad-script`: the probe's ordered pad states equal the script's
  (`tools/psx_pad_compare.sh`).

`measurements.txt` (in `tmp/psx/game-check/run`, and in the output directory of
`psx-replay-check` and `psx-full-game-check`) holds the MIPS `zone`, `budget`
and `cap` lines, `vsyncs <n>`, and `size` of the replay ELF; `psx-check`'s
adds the GPU replay's vsyncs, meter lines, `ram-room <used> <limit> <left>`
and `size`, each behind `gpu `, then the capture report lines behind
`gpu capture `. The MIPS figures
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

- Step 4 of plan 10: the GPU families, one at a time, each with its capture
  line in `psx/gpu_captures.txt`: `print` (with the font texture
  and its 16 CLUTs), the fill patterns of `rectfill` and `circfill` (with the
  pattern slots), then the `all` captures.

- Pending human acceptance (review the report line and
  `tmp/psx/game-check/run/gpu-captures/<label>-f<frame>.ppm` after `make
  psx-check`, then accept with `make psx-gpu-capture-accept LABEL=<label>`):
  - `cls` f1: `cls f1 mask cls calls 1 differ 0 (0.0‰) near1 0 near2 0 far 0
    bbox - offpalette 0 nonuniform 0`, ceiling 0.
  - `rectfill` f11: `rectfill f11 mask cls,rectfill calls 8 differ 0 (0.0‰)
    near1 0 near2 0 far 0 bbox - offpalette 0 nonuniform 0`, ceiling 0.
  - `pset` f131: `pset f131 mask cls,pset calls 65 differ 0 (0.0‰) near1 0
    near2 0 far 0 bbox - offpalette 0 nonuniform 0`, ceiling 0.
  - `line` f194: `line f194 mask cls,line calls 8 differ 2 (0.1‰) near1 2
    near2 0 far 0 bbox 72,119-73,119 offpalette 0 nonuniform 0`, ceiling 2:
    two bottom-row pixels the PC lights stay black on the PSX, where the
    dragged slipway enters from below the screen (GPU stepping).
  - `circfill` f217: `circfill f217 mask cls,line,circfill calls 17 differ 0
    (0.0‰) near1 0 near2 0 far 0 bbox - offpalette 0 nonuniform 0`, ceiling
    0. Every plain `circfill` of the game is the colour-0 disc behind a
    planet, invisible on a cleared screen: the mask keeps `line` so the
    discs show as the 14 pixels they cut from the slipways.
  - `spr` f234: `spr f234 mask cls,spr calls 191 differ 0 (0.0‰) near1 0
    near2 0 far 0 bbox - offpalette 0 nonuniform 0`, ceiling 0: the
    background map and the planet sprites, 1,134 lit pixels in 14 colours,
    under 4 distinct draw palettes (the meter's count at that frame).
  - `rect` f322: `rect f322 mask cls,rect calls 8 differ 0 (0.0‰) near1 0
    near2 0 far 0 bbox - offpalette 0 nonuniform 0`, ceiling 0.
  - `circ` f394: `circ f394 mask cls,circ calls 2 differ 0 (0.0‰) near1 0
    near2 0 far 0 bbox - offpalette 0 nonuniform 0`, ceiling 0: the
    colour-7 radius-9 pad circle round p61 and a colour-2 planet ring.

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
