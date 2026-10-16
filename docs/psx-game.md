# PSX game build: the core on MIPS

The game's C core builds, unchanged, as a PS-EXE that replays an e2e recording
inside the headless emulator and prints its `make state` text and VRAM digests
over TTY, as a second PS-EXE that replays it through the GPU drawing backend,
and as the game itself, `psx-game`, driven by the controller and packed into
the game's CD image, `slipways.bin`/`.cue`. `make psx-check`
proves the replays play the same game as the PC, that the software one draws
the same pictures, that the GPU one receives the same draw calls and fits in
RAM, that its frame captures are deterministic and within their recorded
divergence from the PC, that scripted pad input reaches a PS-EXE, and that the
game, booted from its CD image and driven by a pad script, leaves the title
screen and plays what the PC plays from the same input. Toolchain, emulator, setups and the plain boot mode are
in `docs/psx-hello-world.md`. The Makefile legend and the usage header of each
`tools/psx_*.sh` script hold the per-option detail.

## Build

`make psx-build [EVENTS=<events>]` builds five PS-EXEs and the game's CD
image in `tmp/psx/build` (`PSX_BUILD_DIR`), then prints the ELF sizes and the
image paths:

- `psx-replay.{elf,exe}`: the core replaying `EVENTS` (default
  `tests/e2e/pad_slipway.json`), drawing into the RAM framebuffer;
- `psx-gpu-replay.{elf,exe}`: the same replay drawing through the GPU backend;
- `psx-pad-probe.{elf,exe}`: the pad probe;
- `psx-game.{elf,exe}`: the game read from the pad, the EXE that ships (below);
- `psx-largest-slots.{elf,exe}`: the largest-case scenarios of
  `src/devtools/largest_slots.c` on MIPS (below);
- `slipways.{bin,cue}`: the CD image holding `psx-game.exe` ("CD image" below).

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
(`src/devtools/draw_filter.c`). The game takes `CORE_SRCS`, the state
formatter, the GPU backend (`PSX_GPU_BACKEND_SRCS`, shared with the GPU
replay), `psx_pad` and `psx_pad_map`, and no embedded replay, format self-test,
family filter or capture code. `psx-largest-slots` takes `CORE_SRCS`, the
state formatter, `src/devtools/largest_slots.c` and the PSX platform files,
and no embedded replay, VRAM digest or GPU backend. A file added to
`CORE_SRCS` is in the PSX build at once.

It is built `GPREL`, Release, with `-UNDEBUG`. `psx-game` and the two GPU
replays (`psx-gpu-replay`, `psx-gpu-replay-breakdown`) are built at `-O3`:
the CMake options `PSX_GAME_OPT` and `PSX_MEASURED_REPLAY_OPT` (`O2`, `O3`
or `Os`, default `O3`; any other value fails the configure) link one
interface library per level, `psx-opt-<level>`, after the SDK's, because a
target's own options come before the SDK interface's trailing `-O2` and lose
to it; `O2` links nothing. `psx-build` sets both from `PSX_BUILD_OPT`
(default `O3`) on every configure, then reads the three targets' levels with
`tools/psx_opt_level.sh`, prints `psx-build: opt psx-game <l> psx-gpu-replay
<l> psx-gpu-replay-breakdown <l>` and fails naming a target at another level.
Every other target (`psx-replay`, the pad probe, `psx-largest-slots`) keeps
the SDK's `-O2`, and the prebuilt libpsn00b libraries keep their level.
`-O3`'s gain rests on pcsx-redux's timing model, which charges no
instruction-cache miss or data wait, so its larger text is not promised to
pay on real hardware. Asserts stay compiled
in, as in the PC `-O0` binary, in every target but `psx-game`. `psx-game`, the
program on the CD image, is built without asserts: the CMake option
`PSX_GAME_ASSERTS` (default `OFF`; Makefile `PSX_BUILD_GAME_ASSERTS=ON|OFF`,
passed by `psx-build` on every configure) drops `-UNDEBUG` from it, so the
SDK's `NDEBUG` applies, and keeps `unused-variable`,
`unused-but-set-variable`, `unused-function` and `unused-parameter` warnings,
not errors; `ON` brings its asserts back. Every `assert` stays in the source. `snprintf`/`vsnprintf` are wrapped onto
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
`static const` glyph tables of `src/p8/gfx_software.c`, read by both
backends through `p8_font_glyph_rows`. No JSON loader
(`src/p8/assets.c`, `src/devtools/input_json.c`) is in `CORE_SRCS`, and
`psx-check`'s `no-json` checks the ELF for it.

## Embedded replay

The recording is compiled into the PS-EXE: no file system, CD image or JSON
parser on PSX. `tmp/embed_replay <events> <name> <out.c>` (`src/cmd/embed_replay.c`)
reads it through the PC replay's own loader and writes a C source holding the
frames (`p8_input_frame_t`, 16 bytes a frame), the `initial_save` bytes, the
name, the frame count and the `attract_after` threshold
(`src/devtools/embedded_replay.h`). It refuses a mouse recording, an
unreadable or empty file, a `"start": "resume"` file and a `"bindings"`
member (the PSX replays start on the title with `bindings_default`), and
writes nothing then. One PS-EXE per
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
- the state, each line behind `psx-state: `, ending like the PC's `STATE=`
  file with `cheat activations <n>` (the frames `input_step_run` fired the
  cheat on); the three PSX drivers print it

It also installs the draw meter (below), so its `zone gpu_*` and `cap
gpu_*` lines are measured like the PC's. It then leaves the emulator with
code 0. A `p8_fatal` prints its message,
`stopped p8_fatal frame <n>` and the state, then exits with 3; a failed assert
prints `file:line expr` and exits with 4.

## Drawing backend

The drawing API of `src/p8/gfx.h` keeps its names and signatures. Each draw
call (`cls`, `pset`, `line`, `rect`, `rectfill`, `circ`, `circfill`, `spr`,
`print`) goes to the active `p8_draw_backend_t` (`src/p8/draw_backend.h`),
with the graphics state and the VRAM struct as they are at call time and the
API's own arguments (world coordinates, full colour byte). `map` (one `spr` per
non-empty cell whose 8×8 box meets the screen after the camera) and
`spr_default` stay in the front end, as do the state calls (`camera`, `fillp`,
`palt`, `mset`, the draw palette): a backend reads them, never writes them. `p8_draw_backend_set` installs one process-wide and returns
the previous one; NULL, the default, is the software backend
(`src/p8/gfx_software.c`), which rasterises into `p8_vram_t.screen` for the
PC game, the e2e and every test.

**Image cache** (`src/p8/image.h`). `p8_image_build` runs a draw function once
per 160×120 window of an image slot, with the camera at
the window's origin, through the backend's `image_begin`, `image_window` and
`image_end`; `p8_image_draw` copies the last build at a world position under
the camera, pixels the build never wrote staying transparent (`image_draw`).
`p8_mset` stamps a process-wide map version into the vram, so a caller can
tell the map changed. The software backend keeps each slot as colour indices
in a static store (64 KiB for slot 0, linked into every ELF). The GPU
backend keeps each slot as a 15-bit rectangle of VRAM (below). The game's
slots (`src/game/image_slot.h`, checked against the runtime count): slot 0,
256×256, the background map; 1–32, 48×8, planet rows (`row_image.c`); 33–64,
128×8, outlined text (`text_image.c`); 65, 16×16, the planet disc; 66–69,
16×16, one colony ring per ring colour, built once on the first planet drawn
with no fill pattern (a planet drawn under a fill pattern keeps its
`circfill`/`circ`). The meter, filter, breakdown timing and call
profile follow the copy as one `spr` and the build's calls under their own
families.

**`fillp`, `palt` and the draw palette on the GPU: per-texture CLUT and
pattern textures, no software fallback** (decided; implemented in step 4).

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
  `TILE`. The window change is one `DR_TWIN`, not the `DR_TPAGE` +
  `DR_TWIN` pair the cost model meters: every textured call reads the one
  page `psx_gpu_frame_begin` sets.
- Rejected: rasterising those calls in RAM would need the framebuffer
  uploaded every frame, the copy the GPU path replaces.

**GPU primitive buffers: counted on PC, sized at peak + 1/8, outside
`ZONE_BUDGET_BYTES`, checked against the MIPS RAM.** `src/p8/draw_cost.c`
gives each call's GPU bytes and primitives (PSn00bSDK 0.24 packet sizes, an
upper bound; a call whose screen box misses 160x120 costs 0), and the draw
meter (`src/p8/draw_meter.c`) wraps the active backend and adds them up a
frame, with the distinct draw palettes and fill patterns. The PC replay
installs it under `STATE=`, so `make state` reports `zone gpu_frame`, `zone
gpu_build` (an image build's own buffer) and `cap
gpu_cluts`/`cap gpu_patterns` over every e2e and worst-case game, judged by
the 1/8 margin. `src/p8/gpu_sizes.h` holds the sizes:
`GPU_FRAME_BUFFER_BYTES`, `GPU_BUILD_BUFFER_BYTES`, `GPU_CLUT_SLOTS`,
`GPU_PATTERN_SLOTS`. The GPU
backend reserves two frame buffers and two build buffers (the CPU fills one
of each while the GPU draws the others) as static data, counted in no zone;
`make state` prints the frame buffers' total after the bytes left under
`ZONE_BUDGET_BYTES`, and `psx-check`'s `ram-room` checks they all fit.

## GPU replay

`src/cmd/psx_gpu_replay.c` is the replay loop and TTY protocol of
`psx_replay.c`, plus `psx-replay: backend gpu`, `psx-replay: draw-filter
on|off` and `psx-replay: draw-meter on|off` after the format line, and
`psx-replay: gpu-peak bytes <b> cluts <c> patterns <p> build <k>` (the
backend's own peaks, `psx_gpu_peaks`) before the done line, never on a `p8_fatal`; it has
no `psx-digest:` lines (the RAM screen is not drawn). It still loads the RAM
sprite sheet, so the game sees the same memory. It installs the GPU backend;
each frame ends with `psx_gpu_frame_end`, then `psx_gpu_frame_begin`. It is
driven by a capture script (below): without one it waits forever before its
first frame.

**Draw meter build.** The draw meter wraps the GPU backend (outermost, so it
sees every call) only when the replay is compiled with
`PSX_GPU_REPLAY_DRAW_METER` (CMake option of the same name, off by default,
set for both `psx-gpu-replay` and `psx-gpu-replay-breakdown`, independent of
the draw filter); each frame then ends with the meter's frame end, and
`psx_gpu_frame_end` gets the metered bytes as its limit, so `psx-check`
proves frame by frame on MIPS that the cost model bounds the real buffer.
Without it, the limit is `GPU_FRAME_BUFFER_BYTES` and the state's `zone
gpu_frame` and `cap gpu_*` lines are zeros. `psx-build` passes it from
`PSX_BUILD_DRAW_METER` (default `ON`: the build of `psx-check`, whose
`gpu-cost` reads the meter); `psx-full-game-check`, `psx-short-runs` and the
probe builds always pass `OFF`, so they time the shipped draw.

**Draw filter build.** The draw family filter, which the shipped game never
has, sits on the GPU backend (between it and the meter when both are built)
only when the replay is
compiled with `PSX_GPU_REPLAY_DRAW_FILTER` (CMake option of the same name,
off by default, set for both `psx-gpu-replay` and
`psx-gpu-replay-breakdown`); the filter then sets each frame's mask and ends
its frame after the meter's, if any. `psx-build` passes it from
`PSX_BUILD_DRAW_FILTER` (default `ON`: the build of `psx-check` and
`psx-gpu-capture-report`, whose masked captures need it);
`psx-full-game-check`, `psx-short-runs` and the probe builds always pass
`OFF`, so they time the shipped draw. Without it, a capture request whose
mask is not "all" stops the replay with a `p8_fatal` naming the define; an
"all" capture, or the empty script's `NO_FRAME`, proceeds.

**Frame timer.** The GPU replay times every frame on root counter 1, clocked
by the horizontal blank (~63.6 µs; 263 lines a vsync in the backend's NTSC
non-interlaced mode), free-running over 16 bits
(`src/platform/psx_frame_timer.c`). Counter 2 (system clock / 8) was
rejected: it wraps every ~15.5 ms, under one vsync. Before the loop the
replay counts the hblanks between two vsyncs and prints `psx-frame:
hblanks-per-vsync <n>` (at most two vsyncs before `start_vsync`, not counted
in the `done` line). Each frame reads four marks: before the input step,
after `cart_update60`, after `cart_draw`, after `psx_gpu_frame_begin`.
**update**, **draw** and **submit** are their differences modulo 65,536;
submit covers the meter's and the filter's frame ends (in the builds that
have them), `psx_gpu_frame_end`
(uploads, the `DrawSync` wait on the previous frame, `DrawOTag`) and
`psx_gpu_frame_begin`. The capture handshake's time and vsyncs are taken out
of submit and of the frame's vsyncs. The input advance and the progress line
fall outside every part; the progress line runs while the GPU draws, so the
next frame's submit may wait a little less (23 of `full_game`'s 6,059
frames). Each frame also records the vsyncs it spanned, so a frame of 249
vsyncs or more (where the 16-bit counter may wrap) is detected.

The records go to a static table (`src/devtools/frame_times.c`, pure,
host-tested by `test_frame_times`): 8 bytes a frame, `FRAME_TIMES_CAP` =
6,848 frames, 54,784 bytes of bss in the GPU replay only. A replay longer than
the table stops with a `p8_fatal` naming both counts before frame 0. Nothing
is printed during the loop: after it, the replay prints `psx-frame: frame <n>
update <u> draw <d> submit <s> total <t> vsyncs <v>` for every frame, then the
`done` line; on a `p8_fatal`, the frames recorded so far come before the
stopped state. The instrumentation has no build flag: `psx-check`'s GPU
boots print frame lines too, and ignore them.

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
  asked; a frame that sent more bytes than its caller's limit (the metered
  bytes in the metered GPU replay, `GPU_FRAME_BUFFER_BYTES` elsewhere) is a
  `p8_fatal` naming the frame and both figures; `psx_gpu_peaks` returns the
  peaks since `psx_gpu_init` of each frame's bytes, CLUT slots, pattern
  slots and image build bytes;
- an image build (`image_begin` to `image_end`) writes its packets into a
  second buffer of `GPU_BUILD_BUFFER_BYTES` per frame parity, chained into
  the frame's list in call order (`packet()` swaps buffer, count and limit
  at the build's begin and end, and never branches on it); overflow is a
  `p8_fatal` naming the frame and the slot. The build is a `FILL` of
  0x0000 over the slot, the mask bit set while drawing (`DR_STP`, so an
  untextured black is 0x8000 and stays opaque), per window the draw area
  and offset of `psx_image_window_area`, then the frame's draw area and
  offset, the mask bit off and a texture cache flush. The copy is no
  texture window, a `DR_TPAGE` to the slot's 15-bit page, one raw `SPRT`
  placed and clipped by `psx_image_copy_place`
  (`src/platform/psx_image.c`, host-tested by `test_psx_image`), and a
  `DR_TPAGE` back to the 4-bit page; nothing when it misses the screen;
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
  frame; `print` is one `SPRT` per glyph with a lit texel that shows, walked
  as the software walks the string, after the camera and clipped to the
  screen (`src/platform/psx_text.c`, host-tested pixel for pixel by
  `test_psx_text`), and `print_outlined` is one 5x7 `SPRT` per such glyph,
  placed one pixel up and left of it, from the outlined font; both fonts
  are in the one font texture `psx_gpu_init` uploads once: the 256
  character codes' glyphs from `p8_font_glyph_rows`, each in a 4x6 cell
  (texel 1 lit) on lines 0..47, then each again in a 5x7 outlined cell
  (texel 1 lit, texel 2 an unlit texel with a lit 8-neighbour) on lines
  48..124; and the 16 font CLUTs (entry 1 the colour, entry 2 the black a
  colour-0 `print` writes, every other entry transparent), built by
  `psx_gpu_textures_font` and `psx_gpu_textures_font_clut` (host-tested by
  `test_psx_gpu_textures`); the CLUT is the font line of `col & 0x0F`; a
  patterned `rectfill` box or
  `circfill` span is a raw-textured `SPRT` whose (u, v) is its top-left
  pixel's world coordinates mod 8, under a texture window repeating its
  pattern slot's 8x8 tile (`src/platform/psx_pattern.c`, host-tested pixel
  for pixel by `test_psx_pattern`), a `DR_TWIN` sent only when the window
  changes (a pattern slot, or none for `spr` and `print`); a frame's
  pattern tiles and CLUTs are cached by `p8_draw_cost_pattern_key`
  (`src/platform/psx_pattern_cache.c`, built by `psx_gpu_textures_pattern`
  and `psx_gpu_textures_pattern_clut`, one cache per primitive buffer) and
  uploaded with the spr CLUTs to the pattern slots and pattern CLUT lines;
  more than `GPU_PATTERN_SLOTS` in a frame is a `p8_fatal` naming the
  frame;
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
| images | (704, 0) | 256x512 | 15-bit image slots, each 16-aligned on its own page: slot 0 is (704, 0) 256x256, page (704, 0); the 48x8 slots five a band from (704, 256); the 128x8 slots two a band from (704, 312); the five 16x16 slots in a row from (704, 440); free: (944, 256) 16x56, (800, 304) 144x8, (784, 440) 176x16 and (704, 456) 256x56 |

## Until-exit boot mode

`tools/psx_boot.sh <install-dir> --until-exit [--pad-script <file>] [--capture-script <file>] [--save-state-at <vsync> <file> | --load-state <file>] --exe <exe> <ceiling> <out-prefix>`
adds `-testmode`, so the program's write to the PCSX exit register quits the
emulator with its code. No capture is written, except the frame-locked ones of
`--capture-script` (below). `<ceiling>` is a vsync ceiling:
reaching it, or `PSX_BOOT_TIMEOUT` seconds, fails naming the input and the last
`psx-replay:`, `psx-pad:` or `psx-game:` line. Code 0 passes, any other fails naming it.
`tools/psx_until_exit.lua` overwrites `<out-prefix>.progress` with
`vsync <n>` every 60 vsyncs. An emulator that dies on a signal without a
result fails as `emulator aborted with status <n> (signal <name>)`, naming the
last progress line and the last vsync of `.progress`; the timeout names that
vsync too. The emulator runs under `stdbuf -o0 -e0` when the host has it, so
an abort keeps the whole TTY.
`--save-state-at <vsync> <file>` writes the emulator's save state
(`PCSX.createSaveState`) to `<file>` when the vsync count reaches `<vsync>`,
then the boot carries on; the boot fails if the program exits first.
`--load-state <file>` loads that state (`PCSX.loadSaveState`) at the first
vsync and runs to the program's exit: the boot's vsync count, so its ceiling
and its `exited with 0 after <n> vsyncs`, starts at the load, while the
guest's own counters, so the `done` line's frame count and vsyncs, come back
with the state. The two options exclude each other, need `--until-exit`, and are
refused naming the argument before the emulator starts when the vsync is not a
positive integer below the ceiling or the file to load is missing.
Without `--until-exit`, `psx_boot.sh` behaves as `docs/psx-hello-world.md`
describes.

## Pad script

`--pad-script` plays a text file on port 1 through the PCSX-Redux pad
overrides (`tools/psx_pad_play.lua`, loaded by `tools/psx_until_exit.lua`).
A line is `<vsync offset> <button>...` or `<vsync offset> -` (nothing held);
offsets increase, the last step is `-`, blank and `#` lines are skipped.
Buttons: `SELECT START UP RIGHT DOWN LEFT L2 R2 L1 R1 TRIANGLE CIRCLE CROSS
SQUARE`. Offsets count from a handshake, not from boot: the program writes a
magic word into the last scratchpad word once its pad driver runs, and the Lua
polls it each vsync. Ten vsyncs after the last step, the Lua writes an end word
just below it and the program exits. Both words and magics are in
`src/platform/psx_pad_protocol.h`, shared by the probe and the game;
`test_psx_pad_protocol` (`make psx-tools-test`) checks the Lua's constants
equal them. `psx_pad_start` fills the BIOS pad buffers with 0xff ("no pad")
after starting the driver: until the first poll at the next vsync they hold
zero, which reads as every button held.

`src/cmd/psx_pad_probe.c` prints `psx-pad: <hex4>` (hardware bit order,
1 = pressed) each time the pressed buttons change, then `psx-pad: done`.
`psx/pad_probe_script.txt` is the probe's script in `make psx-check`.

## Game EXE

`src/cmd/psx_game.c` is the GPU replay's frame (input step, `cart_update60`,
`cart_draw`, `psx_gpu_frame_end` with `GPU_FRAME_BUFFER_BYTES`,
`psx_gpu_frame_begin`) driven by port 1 instead of a recording, with no draw
meter, no family filter and no capture handshake. It starts with an erased save
storage and a one-frame zero input during `cart_load` and `cart_init`, as a
PC replay of a pad recording without `initial_save`. It runs at most one frame
a vsync: a frame that starts on the vsync the previous one started on waits
for the next. Its state's `zone gpu_frame` and `cap gpu_*` lines are zeros
(no meter): before its done line it prints `psx-game: gpu-peak bytes <b>
cluts <c> patterns <p> build <k>`, the backend's own peaks, which `game-gpu-cost`
bounds by the PC's metered peaks.

**Pad map** (`src/platform/psx_pad_map.c`, pure, host-tested by
`test_psx_pad_map`). Each frame reads `psx_pad_read_pressed()` once, before
the input step, and `psx_pad_map_buttons` turns it into `pad_buttons`: each
controller button sets the button (1 to 12, `P8_PAD_BUTTON_<n>`) that
`keymap.default.cfg` binds to its raylib gamepad name (CROSS is
`RIGHT_FACE_DOWN`, START `MIDDLE_RIGHT`...), so the game reads on PSX what
the PC reads with the default keymap; `test_psx_pad_map` checks the table
against that file. `psx_pad.c` checks the hardware bits against the SDK's
`PAD_*` at compile time. No keymap file is read on PSX, and the PSX game
plays `bindings.default.cfg` (`bindings_default`).

| hardware bit | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| button | SELECT | L3 | R3 | START | UP | RIGHT | DOWN | LEFT | L2 | R2 | L1 | R1 | TRIANGLE | CIRCLE | CROSS | SQUARE |
| `pad_buttons` button | 12 | - | - | 11 | 1 | 4 | 2 | 3 | - | - | 9 | 10 | 8 | 6 | 5 | 7 |

**Pad log.** Its TTY is `psx-game: asserts on|off` (the mode it was built
in), then `psx-game: ready` once the handshake is raised, then
`psx-game: pad <frame> <pad_buttons as %03x>` on frame 0 and on every frame
where `pad_buttons` changes: a run-length log of every frame's input, each
line carrying its frame so a lost line is detected.
`tmp/psx_pad_log <tty> <pad-script> <events.json>` (`src/cmd/psx_pad_log.c`,
parser `src/devtools/pad_log.c`, tested by `test_pad_log`) refuses a malformed
log naming its line (no frame 0, a frame not increasing, bad hex, a bit outside
the twelve `P8_PAD_*`, no done line or two, a pad line at or after the done
frame count), writes the log as an `events.json` through `p8_input_save` with
no `initial_save`, and compares its ordered distinct states with the script's,
mapped by the same table:
`psx-pad-log: log: ok, <frames> frames` and `psx-pad-log: states: ok, <n>
states`, or the first difference; exit 0, 1, or 2 on usage.

**End word.** The game clears the pad end word before raising the handshake
and tests it at the top of each frame, before reading the pad. Under check,
`tools/psx_pad_play.lua` writes it ten vsyncs after the script's last step;
the game then prints the replays' end protocol, `psx-replay: done game
<frames> vsyncs <n>` (vsyncs since the first frame; the log's frames are 0 to
`<frames>` − 1) and the `psx-state:` block, and exits with code 0. On a
console nothing writes that word, so the game never ends: the EXE checked is
the EXE that ships, with no build flag. A `p8_fatal` prints `psx-state:
stopped p8_fatal frame <n>` and the state, code 3; a failed assert, code 4.

**Title script** `psx/game_title_script.txt` walks the title screen as
`tests/e2e/pad_title.json` does: idle 90 vsyncs while the title rows slide in,
Down, Down, Right, Up, then ✕ on "reasonable", each press and release held 20
vsyncs, then 60 idle vsyncs into the first month. Pad navigation acts on
edges, so a press held several frames moves the focus once. Its states are
`000 002 000 002 000 008 000 001 000 010 000`; a state shorter than one frame
is fixed by lengthening the offsets.

## CD image

`psx/CMakeLists.txt`'s `psn00bsdk_add_cd_image(psx-game-iso slipways iso.xml
DEPENDS psx-game system.cnf)`, the call `psx-hello` uses, is in `ALL`: every
`make psx-build` writes `slipways.bin` and `slipways.cue`
(`PSX_GAME_BIN`, `PSX_GAME_CUE`) after `psx-game.exe`, and fails naming
either when it is missing. `make psx-game-cd` is the named target: it runs
`psx-build` and prints the two paths.

- **Layout** (`psx/iso.xml`, one data track): system and application
  `PLAYSTATION`, volume, volume set and publisher `SLIPWAYS`; files
  `SYSTEM.CNF` and `SLIPWAYS.EXE` (`PSX_GAME_DISC_EXE`, a byte copy of
  `psx-game.exe`), then 1,024 dummy sectors. Names on the disc are 8.3: the
  BIOS's ISO 9660 reader finds nothing else.
- **`psx/system.cnf`**: `BOOT=cdrom:\SLIPWAYS.EXE;1`, `TCB=4`, `EVENT=10`,
  `STACK=801FFFF0` (top of RAM, where `ram-room` counts
  `PSX_STACK_RESERVE`).
- **No license data, no region**: PSn00bSDK ships none, and the pinned
  OpenBIOS checks neither. The image boots in the emulator, not on a console
  or under a Sony BIOS ("Still to do").
- The image is 2,890,608 bytes, 1,229 sectors of 2,352. The CD boot exits
  after 763 vsyncs from power-on (the EXE boot: 363), the game's own 342 from
  its first frame included (199 frames).
- **Never hash the `.bin`**: mkpsxiso stamps the build time into it. Its size
  is measured; the proof is state-based, and no capture of the CD boot is
  compared to a reference.
- `make psx-boot PSX_ISO=tmp/psx/build/slipways.cue` boots it in the plain
  capture mode, to look at a frame of the disc's title screen.

`tools/psx_cd_check.sh <cue> <exe> <disc-name> <dumpsxiso> <out-dir>` checks
an image after the build, without the emulator, one `psx-cd-check:` line per
condition:

- `cue`: `tools/psx_cue_check.sh` (shared with `psx-hello`): the `.cue`'s
  first `FILE` line names a non-empty `.bin`, next to it, of whole 2,352-byte
  sectors;
- `cd-exe`: `dumpsxiso -x <out-dir>/disc <bin>` (`PSX_DUMPSXISO`, shipped
  with the SDK) extracts the disc, and `<disc-name>` is byte-identical to
  `<exe>`: the EXE `magic`, `no-json` and `game-ram-room` read is the EXE
  shipped. A failure names `dumpsxiso`'s error, the extracted names, or the
  first differing byte.

The game itself does not depend on how it was loaded (it reads no argument
and raises the pad handshake itself), so `make psx-check` boots the disc
instead of the EXE: one boot carries the controller proof and the CD proof.
`make psx-game-check` keeps booting the EXE, for quicker controller work.

## GPU family captures

Each GPU draw family is checked by a capture of one frame of the check replay,
drawn with a family mask, against the PC software frame of the same frame and
mask.

- **Family filter** (`src/devtools/draw_filter.c`, PC and the PSX draw filter build): a backend
  wrapper that drops the calls outside its mask and counts, per family, the
  calls the cost model costs above 0. The mask bits, in order: `cls pset line
  rect rectfill circ circfill spr print pattern`; `cls` is always set, and a
  patterned `rectfill`/`circfill` needs its shape bit and `pattern`. The mask
  applies only on a captured frame, "all" otherwise.
- **Capture list** `psx/gpu_captures.txt`: `replay <name>`, then
  `<label> <frame> <mask>` a capture (parser `src/devtools/capture_list.c`).
  A family label's mask holds `cls` and that family (the `pattern` label adds
  `rectfill` and `circfill`, the `circfill` label `line`, since its colour-0
  discs only show where they cut a slipway), so its report does not move
  when another family changes. Three `all` captures (f100, f188, f265) draw
  every family together, for the interactions: sprites over the grid, text
  over fills, draw order. No captured frame is one the `DIGEST=` screen
  lines sample (every 32nd and the last): the PC draws a captured frame
  filtered.
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
  `make psx-check` run whose other conditions all passed, for that label's
  lines only. A count below its ceiling prints `below ceiling`, so the
  ceiling can be lowered with the same target; raising one is a regression
  to explain in its commit.
- **Acceptance**: `make psx-gpu-capture-accept LABEL=<label>`, after a green
  `make psx-check`, writes the sha256 of every capture of that label (all of
  them for `pattern` and `all`) into `psx/gpu_capture_references.txt`, other
  labels untouched; from then on a capture with a reference must match it
  byte for byte, and its ceiling line is ignored. An agent may run it only
  when every capture of the label reports `differ 0` against the PC; any
  other label waits for the human's review of its report lines and images.

## Checks

| Target | What it proves | Time |
|---|---|---|
| `make psx-check` | every condition below, on `pad_slipway.json` and the title script, the game booted from its CD image; six boots | ~110 s |
| `make psx-game-check` | the seven `game-*` conditions alone, the game EXE booted on `PSX_GAME_SCRIPT` (outputs in `tmp/psx/game`) | ~10 s |
| `make psx-game-cd-check` | the seven `game-*` conditions on the CD boot, then `cue` and `cd-exe` (outputs in `tmp/psx/game-cd`) | ~15 s |
| `make psx-game-cd` | builds the CD image and prints its paths | build |
| `make psx-gpu-capture-report` | the GPU captures and their reports alone (outputs in `tmp/psx/gpu-capture`) | ~20 s |
| `make psx-replay-check` | the replay proof alone (game lines and digests), on `PSX_EVENTS` (outputs in `tmp/psx/replay`) | ~45 s |
| `make psx-pad-check` | the pad proof alone (outputs in `tmp/psx/pad`) | ~5 s |
| `make psx-full-game-check` | its conditions on `worst-case/full_game.json`, the replay and the GPU replay booted (the GPU replay without asserts unless `PSX_FULL_GAME_ASSERTS=on`), the GPU replay's frame times and MIPS sizes judged (outputs in `tmp/psx/full-game/run`) | ~20 min |
| `make psx-real-game-budget [FRAMES=] [SITUATIONS=]` | `real-game-budget`'s figures and verdict on a committed frame table (defaults `psx/perf/full-run-frames.txt` and `psx/perf/full-run-situations.txt`), fails when red; boots nothing | ~1 s |
| `make psx-short-runs-cut` | the five short runs cut out of `full_game.json`, each embedded and replayed on PC, the game match reported (outputs in `tmp/psx/short-runs`) | ~25 s |
| `make psx-short-runs` | the short runs' GPU replays booted, judged and their draw split per family (outputs in `tmp/psx/short-runs`) | ~4 min |
| `make psx-probe-baseline` | the probe build of unpatched `HEAD` booted on the short runs, as the probes' baseline (outputs in `tmp/psx/probe/baseline`) | ~2 min |
| `make psx-probe PATCH=<file>` | a throwaway patch measured on the short runs against the probe baseline, built in a temporary worktree (outputs in `tmp/psx/probe/<label>`) | ~2 min |
| `make situations [EVENTS=] [SITUATIONS_OUT=]` | writes the situation table of a replay (default `full_game.json`, into `tmp/situations-table/`), host only; see "Situations" | ~2 s |
| `make situations-check` | the full game's regenerated situation table equals `psx/perf/full-run-situations.txt`, else the first differing frame is named | ~2 s |
| `make situation-report [FRAMES=] [SITUATIONS=]` | per-situation frames, overruns and means, one-off occurrences and runs, verdicts `frame-count`, `families`, `one-off-runs` | ~1 s |

`make psx-check` is the PSX check target: new PSX work adds its own
condition to it. It builds the PS-EXEs, writes the PC references with
`STATE=`, `DIGEST=` and `CAPTURE_LIST=` in one run of the current
`tmp/replay`, boots the replay, the GPU replay (twice, both with the capture
script), the probe, the game's CD image (with `PSX_GAME_SCRIPT`) and
`psx-largest-slots` until they exit, six boots, then checks every condition before
concluding, one line each (`tools/psx_check.sh`, outputs in
`tmp/psx/game-check`, conditions also in `run/conditions.txt`):

- `magic`: the five `.exe` start with `PS-X EXE`;
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
- `gpu-cost`: its `zone gpu_frame`, `zone gpu_build`, `cap gpu_cluts` and `cap
  gpu_patterns` lines equal the PC's: the backend gets the PC's exact draw calls on MIPS;
- `ram-room`: 65,536 (kernel area) + text + data + bss of the GPU ELF (the
  toolchain's `size`, `PSX_SIZE`; the zones and both primitive buffers
  included) + `PSX_STACK_RESERVE` (64 KiB) <= 2,097,152; the line prints the
  bytes left (`check_ram_room` of `tools/psx_conditions.sh`, shared with
  `game-ram-room` and `psx-full-game-check`'s `gpu-ram-room`);
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
  `p8_input_free` in either replay ELF or the game ELF;
- `pad-script`: the probe's ordered pad states equal the script's
  (`tools/psx_pad_compare.sh`);
- the seven game conditions, on the CD boot, from `tools/psx_game_check.sh` (outputs in
  `run/game`: `game_events.json`, a pad recording `make replay EVENTS=` can
  dump, its PC state `pc_state.txt` and `game-state/`):
  - `game-done`: the game boot exited with 0, printed one `psx-replay: done
    game <frames> vsyncs <n>` line and no `stopped p8_fatal`;
  - `game-pad-log`: `tmp/psx_pad_log` turned the TTY's pad log into
    `game_events.json`, with the done line's frame count;
  - `game-pad-states`: the log's ordered pad states equal the script's,
    mapped by `psx_pad_map` (the mapping proven on MIPS); a difference names
    the first differing state;
  - `game-state-game-lines`: `tmp/replay` replays `game_events.json` with
    `STATE=` for the same frame count, and `tools/psx_state_compare.sh` finds
    every game line equal to the PSX state's;
  - `game-title-left`: the PSX state's `difficulty` line is `difficulty
    PSX_GAME_DIFFICULTY` (`reasonable`) and its `clock` line is not `clock
    year - month -`;
  - `game-asserts`: the TTY's first `psx-game: asserts` line says the mode
    `psx-build` asked for (`off` unless `PSX_BUILD_GAME_ASSERTS=ON`); a
    failure names the line read, or `psx-game: asserts (no line)`;
  - `game-gpu-cost`: the TTY's `psx-game: gpu-peak` bytes, CLUTs, patterns
    and build bytes are no larger than the `zone gpu_frame`, `cap
    gpu_cluts`, `cap gpu_patterns` and `zone gpu_build` peaks of the PC replay of its pad log
    (`run/game/pc_state.txt`, title frames included); a failure names both
    figures, `psx-game: gpu-peak (no line)`, or `skipped` when the PC replay
    did not run;
- `game-cue`, `game-cd-exe`: `cue` and `cd-exe` of `tools/psx_cd_check.sh` on
  `PSX_GAME_CUE` and `psx-game.exe` (extracted disc in `run/game-cd/disc`);
  a disc that lands in the OpenBIOS shell fails `game-done` first, with "last
  progress line: (none)";
- `game-ram-room`: `ram-room` on the game ELF;
- `largest-slots`: the `psx-largest-slots` boot exited with 0, printed
  `psx-largest: asserts on`, `psx-largest: done` and no `stopped p8_fatal`,
  and its `psx-largest: bytes_button <peak> <size>` and `bytes_row` lines
  each have a peak no larger than the size: the largest button and row the
  game builds fit their slot on MIPS (`check_largest_slots` of
  `tools/psx_conditions.sh`); a failure names the missing line or the first
  slot over its size.

`src/cmd/psx_largest_slots.c` loads the sprite sheet and the cart, runs
`largest_slots_run` (the scenarios `test_button` and `test_layout_row` run on
the host), prints the `largest_slots_format` lines behind `psx-largest: `,
then `psx-largest: done`, and exits with 0. A `p8_fatal` prints `psx-state:
stopped p8_fatal frame 0` and the state, then exits with 3; a failed assert
exits with 4.

`make psx-full-game-check` builds the PS-EXEs of `worst-case/full_game.json`
in `tmp/psx/full-game/build`, writes the PC state and digest with `STATE=` and
`DIGEST=`, boots the replay, then the GPU replay with an empty capture script
(the capture handshake runs, `psx_capture_frames.lua` hands `NO_FRAME` at once,
nothing is captured), until they exit, and checks its conditions
(`tools/psx_full_game_check.sh`, outputs in `tmp/psx/full-game/run`, conditions
also in `run/conditions.txt`), every one before concluding.
`PSX_FULL_GAME_ASSERTS=on|off` (default `off`; any other value fails before
anything builds) picks the asserts mode: `off` has `psx-build` configure
`PSX_MEASURED_REPLAY_ASSERTS=OFF`, so the measured GPU replays
(`psx-gpu-replay`, `psx-gpu-replay-breakdown`) build without asserts, while
the CPU-path replay and every other target but `psx-game` keep them; `on`
keeps them in the measured GPU replays too. Either way it has
`psx-build` configure `PSX_GPU_REPLAY_DRAW_FILTER=OFF` and
`PSX_GPU_REPLAY_DRAW_METER=OFF`, and always passes `PSX_BUILD_OPT=O3`, so
the judge measures at `O3` whatever the caller's environment and
`psx-build`'s `opt` line shows it. `psx-build` passes
these options and `PSX_GAME_ASSERTS` on every configure (`ON`, and `OFF` for
`PSX_GAME_ASSERTS`, unless asked), so a cached value never carries into the
next run:

- `replay-done`, `state-game-lines`, `sprite-sheet`, `screen` and `map` on
  the replay, `gpu-replay-done` and `gpu-state-game-lines` on the GPU replay,
  as `psx-check` defines them (both scripts source these conditions, and
  `check_ram_room`, from `tools/psx_conditions.sh`);
- `gpu-draw-filter`: the GPU boot's TTY has `psx-replay: draw-filter off`
  (the judge never measures a replay built with the draw family filter); a
  failure quotes its `draw-filter` line, or `(no line)`;
- `gpu-draw-meter`: the GPU boot's TTY has `psx-replay: draw-meter off` (the
  judge never measures a metered replay); a failure quotes its `draw-meter`
  line, or `(no line)`;
- `timer`, `frame-budget`, `zone-sizes` and `cap-sizes`, the verdicts of
  `tools/psx_frame_budget.sh` on the GPU boot's TTY and
  `run/gpu-state/judged_state.txt`: its MIPS state with the zero `zone
  gpu_frame`, `zone gpu_build` and `cap gpu_*` lines replaced by the PC
  state's (below);
- `real-game-budget`: plan 22's judge of a 60-minute game (216,000 vsyncs),
  from `run/frame-budget/frames.txt` and the situation table
  `PSX_FULL_GAME_SITUATIONS` (default `psx/perf/full-run-situations.txt`,
  see "Situations"): no `steady` frame over 263 hblanks, and the `one-off`
  frames' vsyncs over 263 at most `PSX_REAL_GAME_ONE_OFF_VSYNCS` (10,800, 5%
  of 216,000; below);
- `gpu-ram-room`: `ram-room` on the full-game GPU ELF (its 6,059 embedded
  frames and the frame table included).

`tools/psx_frame_budget.sh <gpu-tty> <gpu-psx-state> <expected-frames>
<budget-hblanks> <out-dir>` analyses a finished boot, with no boot of its own
(host-tested by `test_psx_frame_budget` in `make psx-tools-test`). It writes
`frames.txt` (a header, then one line a frame), `summary.txt` and
`verdicts.txt` into `run/frame-budget`:

- `timer`: the calibration line is there with 255..271 hblanks a vsync, one
  frame line per frame from 0 to frames − 1 in order, no frame spanning 249
  vsyncs or more;
- `frame-budget`: no frame's total above `PSX_FRAME_BUDGET_HBLANKS` (263, one
  NTSC vsync; fixed, plan 11's step 4 never moves it); a failure names the
  count over and the worst frame;
- `zone-sizes`: every `zone` line's peak (`gpu_frame`, the primitive buffer
  against `GPU_FRAME_BUFFER_BYTES`, and `gpu_build` against
  `GPU_BUILD_BUFFER_BYTES` included) at most its limit, and `budget
  zones <used> <total>` with used at most total;
- `cap-sizes`: every `cap` line's peak at most its limit.

The summary, integer arithmetic: `frames <n> budget <h> hblanks`; `over
<count> frames (<‰>), worst frame <f> total <t> (+<overrun>) update <u> draw
<d> submit <s>`; `overrun mean <m> hblanks over the over-budget frames`;
`overrun distribution <=1/4 vsync <a> <=1/2 <b> <=1 <c> <=2 <d> >2 <e>`
(overruns against 263 lines); `parts over-budget frames update <U> draw <D>
submit <S> dominant <part>` (the largest sum, `-` when no frame is over);
`parts all frames update <U> draw <D> submit <S> mean total <m>`; then each
zone, budget and cap line, `ok margin <n>/8` or `over by <n>`. The 1/8 margin
is information here, not a failure. The check prints every frame line behind
`psx-full-game-check: frame ` and the summary behind `psx-full-game-check: `.

`tools/psx_real_game_budget.sh <frames-table> <situations-table>
<budget-hblanks> <one-off-allowance-vsyncs>` judges `real-game-budget`, for
the check and for `make psx-real-game-budget` (host-tested by
`test_psx_real_game_budget` in `make psx-tools-test`). It joins the two tables
line by line and prints:

```
real-game steady-over <n> frames, worst steady frame <f> total <t> situation <name>
real-game one-off-over <n> frames, vsyncs <sum> of <allowance>
real-game-budget ok|failed <reason>
```

A frame is over when its `total` is above the budget. A one-off frame over
counts `ceil(total / 263)` vsyncs, the vsyncs it holds the screen on a
vsync-locked display; the measured `vsyncs` column is not used, it depends on
where the frame started. With no steady frame over, the first line ends
`worst steady frame -`. It is `failed` when a steady frame is over (`<n>
steady frames over 263, one-off vsyncs <sum> of <allowance>`), when the sum
is above the allowance (`one-off vsyncs <sum> over <allowance>`; both joined
with `; `), or when the join fails, the figures then `-`: a missing table,
a header other than `frame update draw submit total vsyncs` or `frame
situation family`, `frame count <n> vs <m>`, `frame <k>: frame table has
<a>, situation table has <b>`, a family neither `steady` nor `one-off`, or
`no frame line`. It exits 0 once the verdict is printed, 1 on a usage error.
The check prints the two figure lines behind `psx-full-game-check: ` and
adds them to `measurements.txt` behind `gpu `. `frame-budget` is unchanged
and still gating: plan 22's verdicts read `real-game-budget`.

The unpatched pinned emulator aborts any boot at about 32,700 vsyncs (exit
134, `An unknown exception occured`): each Lua event listener left two slots
on its Lua thread per event, and the `GPU::Vsync` listener's thread
overflowed LuaJIT's 65,500-slot stack outside a protected call. An idle boot
with no program aborted at the same vsync, and a state saved at vsync 32,100
and loaded in a fresh emulator ran on past it, so the guest is not involved.
`tools/pcsx-redux-event-listener-stack.patch`, applied by
`psx-emulator-setup`, restores the thread's stack after each call; an install
made before it must be removed and set up again.

`measurements.txt` (in `tmp/psx/game-check/run`, in the output directory of
`psx-replay-check` and in `tmp/psx/full-game/run`) holds the MIPS `zone`,
`budget` and `cap` lines, `vsyncs <n>`, and `size` of the replay ELF;
`psx-full-game-check`'s adds the GPU replay's `vsyncs`, the PC state's `zone
gpu_frame`, `zone gpu_build` and `cap gpu_*` lines, the GPU replay's `gpu-peak bytes <b> cluts
<c> patterns <p> build <k>`, `ram-room <used> <limit> <left>` and its ELF `size`, each
behind `gpu `, then the frame summary behind `gpu frame `, then the two `real-game` figure
lines behind `gpu `; `psx-check`'s
adds the GPU replay's vsyncs, meter lines, `ram-room <used> <limit> <left>`
and `size`, each behind `gpu `, then the capture report lines behind
`gpu capture `, then the game's `vsyncs`, `frames`, `ram-room` and `size`,
behind `game `, then `bin-bytes <n> sectors <n>` and `boot-vsyncs <n>`
(from power-on, BIOS and CD load included) behind `game cd `, then the
`largest-slots` boot's `bytes_<slot> <peak> <size>` lines behind `largest `.
The MIPS figures
are lower than the PC's (4-byte pointers): in `measurements.txt` a
measurement, not a limit check; `psx-full-game-check`'s `zone-sizes` and
`cap-sizes` judge the GPU replay's against their sizes (the `gpu_*` ones
taken from the PC state).

**UI and particle slot sizes are per build, picked by pointer width.** Each
is either a `ZONE_BYTES_BY_WIDTH(<PC>, <MIPS>)` pair (`src/engine/zone.h`:
the MIPS column must be a whole 8-byte cell and no larger than the PC
column, checked at compile time) or computed from the real `sizeof`s
(`particles`, `particles_help_screen`, `ephemerals`: `ARENA_LIST_BYTES` of
`src/engine/allocator.h`). Slot counts are shared. The MIPS column comes from:

| Slot | PC | MIPS | MIPS source |
|---|---|---|---|
| `button` | 6,056 | 3,784 | `largest-slots` peak, no margin |
| `row` | 1,184 | 616 | `largest-slots` peak, no margin |
| `modal_backdrop` | 280 | 160 | MIPS `cap bytes_*` peak 136 + 1/8 |
| `message_box` | 5,128 | 3,072 | MIPS peak 2,728 + 1/8 |
| `toast` | 344 | 192 | MIPS peak 168 + 1/8 |
| `burst` | 648 | 424 | MIPS peak 376 + 1/8 |
| `particles` | 1,384 | 696 | computed |
| `particles_help_screen` | 5,344 | 2,680 | computed |
| `ephemerals` | 152 | 96 | computed |
| `particle`, `transit_dots` | 32, 24 | 32, 24 | struct size, compile-time guard |

The other MIPS sizes are the highest `cap bytes_<slot>` over MIPS replays
of the five worst-case games plus 1/8, rounded up to a cell (the four
`full_game*` games gave the same peaks; `title_idle` never opens these
slots). `largest-slots` keeps `button` and `row` honest, and
`psx-full-game-check`'s `cap-sizes` the others on `full_game.json`. The
other four worst-case games are in no PSX target: to re-measure one, run
`make psx-replay-check PSX_EVENTS=worst-case/<game>.json` with
`PSX_REPLAY_CEILING` at frames × 12 and `PSX_REPLAY_TIMEOUT` at frames ×
0.2 s (and its own `PSX_BUILD_DIR` and `PSX_REPLAY_OUT`), in the
background, about 13 minutes each.

Ceilings and timeouts are the first measured run plus half again:
`PSX_REPLAY_CEILING`/`PSX_REPLAY_TIMEOUT` for `pad_slipway`,
`PSX_GPU_CEILING`/`PSX_GPU_TIMEOUT` for the GPU replay of `pad_slipway`,
`PSX_PAD_CEILING`/`PSX_PAD_TIMEOUT` for the probe,
`PSX_GAME_CEILING`/`PSX_GAME_TIMEOUT` for the game EXE with the title script
(`psx-game-check`), `PSX_GAME_CD_CEILING`/`PSX_GAME_CD_TIMEOUT` (1,140 vsyncs,
20 s) for the CD boot (`psx-game-cd-check`, `psx-check`),
`PSX_LARGEST_SLOTS_CEILING`/`PSX_LARGEST_SLOTS_TIMEOUT` (531 vsyncs, 10 s) for
`psx-largest-slots` (first run 354 vsyncs in 6.5 s),
`PSX_FULL_GAME_CEILING`/`PSX_FULL_GAME_TIMEOUT` (71,834 vsyncs, 1,197 s) for
the replay of `full_game` and `PSX_FULL_GAME_GPU_CEILING`/
`PSX_FULL_GAME_GPU_TIMEOUT` (31,500 vsyncs, 525 s) for its GPU replay: the
first green run's 47,889 vsyncs in 798 s for the replay, and for the GPU
replay the first run with frame lines, 21,000 vsyncs (`.progress`, the frame
lines printed after the loop included; `done` line 20,543) in 350 s, both
`done` lines at 6,059 frames, the PC's count.
`PSX_SHORT_RUN_CEILING`/`PSX_SHORT_RUN_TIMEOUT` (1080 vsyncs, 20 s) for every
boot of `psx-short-runs`, the probe tool's values: twice its slowest boot
seen, `draw-6000`'s breakdown boot with asserts off at 542 vsyncs; a
ceiling only caps a boot that never exits, so a passing run is not slower.
`psx-full-game-check` builds in its own directory and is not part of
`psx-check`: at minutes a run, every later step would pay it again.

## Short runs

`make psx-short-runs` measures the GPU replay on a few short recordings that
hold the worst frames of `full_game.json`, in minutes instead of the full
check's 20, from the program under test (an emulator save state would hold
the old code). It judges no zone or cap: `psx-full-game-check` stays the only
judge of sizes.

**Manifest.** `psx/short_runs.txt`, one run a line, frames of
`full_game.json`: `<name> prefix <first> <last> <end>` (its first `end + 1`
frames) or `<name> resume <first> <last>` (a resume before the window
`first..last`). The six runs: `start` (frames 0–99, the start spike at 12),
`update-4737`, `update-5500`, `update-5719` (update spikes), `draw-5564`
(the draw cluster 5564–5578) and `draw-6000` (frames 6000–6014, where
`pset` is above 2% of draw on PC: a resume before the `pset` peak at
5504–5552 starts without slipway transit dots, so it shows too little
`pset`).

**Steady runs.** Each steady situation with frames over 263 is to get a
resumed run named `<situation>-<first>` whose window lies in it.
`make steady-windows` (`tools/steady_windows.sh`, host-tested by
`test_steady_windows`) joins `psx/perf/full-run-frames.txt` with
`psx/perf/full-run-situations.txt` and ranks, per steady situation over, every
window of `min(15, stretch length)` frames inside one stretch of it (a
stretch: consecutive frames with that label) by frames over 263, then summed
total, then first frame; a window with no frame over is never a candidate.
It keeps up to three non-overlapping candidates: `candidate <situation>
<rank> <first> <last> frames <n> over <o> mean-update <u> mean-draw <d>`, then
`situation <name> frames-over <o> candidates <c>`. `make short-run-situations
[PSX_SHORT_RUNS=<names>]` (host only) runs the cut, labels each run's
`run.json` with `tmp/situations` into `<name>/situations.txt`, and
`tools/short_run_situations.sh` (host-tested by `test_short_run_situations`)
compares every window frame, through `map.txt`, with the committed table:
`short-run-situation <name> same <situation>`, `differs frame <full> run
<label> full <label>` (the first difference), `mixed <labels>` (the full
window itself spans several labels) or `not-steady <label>`; the target fails
unless every run is `same`. A run stands for its situation once it is `same`
and `psx-short-runs` against the full game's frame table gives
`representative ok`.

**Cut.** `make psx-short-runs-cut` (host only) replays `full_game.json` with
`tmp/short_run_cut` (`src/devtools/short_run_cut.c`, pure, host-tested by
`test_short_run_cut`), which observes every storage write and writes per run
into `tmp/psx/short-runs/<name>/`:

- `run.json`: for `resume`, an `initial_save` recording whose cut W is the
  last frame at or before `first − 9` where the game wrote a save and whose
  next frame W+1 has an idle pad; its frames are the 13-frame title resume of
  `tests/e2e/pad_resume_at_start.json`, then `full_game.json`'s frames W+1 to
  `last`. A cut with no eligible save, or longer than 1,024 frames (the
  breakdown table), is refused naming the run;
- `reference.json`: `full_game.json`'s frames up to the frame matching the
  run's last;
- `map.txt`: `cut <W> first-game-frame 13 source-first <W+1> window <first>
  <last>` (run frame 13 + k plays full frame W+1+k; run frames 0–12, the
  resume frame included, are never compared), or `cut none first-game-frame
  0 source-first 0 ...` for a prefix.

Both recordings must pass `tmp/embed_replay` and replay with `STATE=`; the
target then prints `short-run-match: <name> same|differs <map>` with the
differing lines of the two `tools/end_state.sh` states. A difference is
reported, never a failure: a save holds no camera, jump selection or
crosshair, so a resumed run can diverge from the full game, and the game is
not changed for it.

**Breakdown PS-EXE.** `psx-gpu-replay-breakdown` (built by `psx-build`, never
booted by `psx-check`) is the GPU replay compiled with
`PSX_GPU_REPLAY_BREAKDOWN`: `src/devtools/draw_timing.c` (host-tested by
`test_draw_timing`) wraps the backend outermost, after the meter if built, and times
every p8 draw call per family of `draw_filter.h` on root counter 2, system
clock / 8 (`src/platform/psx_fine_clock.c`), with counter 1's hblanks as the
coarse clock (a call of 200 hblanks or more flags its frame `wrapped`). A
patterned rectfill or circfill counts under its shape and again under
`pattern`, which no total adds. After the `psx-frame:` lines it prints
`psx-draw: clk8-per-64-hblanks <n>` then, per frame, `psx-draw: frame <n>`,
`<family> <ticks> <calls>` for the ten families and `wrapped <0|1>`. Without
the definition the GPU replay is the same program.

**Target.** `make psx-short-runs [PSX_SHORT_RUNS=<names>] [PSX_SHORT_RUN_ASSERTS=on|off] [PSX_SHORT_RUN_OPT=O2|O3|Os]`
runs the cut, then per run configures `psx/` in `<name>/build` with
`run.json` embedded and
builds only `psx-gpu-replay` and `psx-gpu-replay-breakdown`, without asserts
unless `PSX_SHORT_RUN_ASSERTS=on` (default `off`; any other value fails
before the cut). `off` sets the CMake option `PSX_MEASURED_REPLAY_ASSERTS`
to `OFF`: those two targets drop `-UNDEBUG`, so the SDK's `NDEBUG` applies,
and `unused-variable`, `unused-but-set-variable`, `unused-function` and
`unused-parameter` stay warnings, not errors (helpers, locals and parameters
that only feed an assert); `on` keeps `-UNDEBUG` in them. Every other
target but `psx-game` keeps asserts, and `psx-build` keeps them in the
measured GPU replays unless `psx-full-game-check` (default
`PSX_FULL_GAME_ASSERTS=off`) asks it not to. `PSX_SHORT_RUN_OPT` (default `O3`, the level
`psx-build` ships and measures at; `O2`, the SDK's level, and `Os` stay
accepted for a before/after comparison; any other value fails before the
cut) sets the CMake option `PSX_MEASURED_REPLAY_OPT` of those two targets
only. After the build, `tools/psx_opt_level.sh` (host-tested by
`test_psx_opt_level`) reads the last `-O` of each target's `flags.make`; a
level other than the one asked for fails the run before any boot, naming the
target and both levels. Each run prints `opt <level> psx-gpu-replay
<text>/<data>/<bss> psx-gpu-replay-breakdown <text>/<data>/<bss>`. Then it boots the plain one twice and the breakdown once with an empty capture script until they
exit (`PSX_SHORT_RUN_CEILING` vsyncs, `PSX_SHORT_RUN_TIMEOUT` s; each boot's
`exit ok|failed`, wall time and vsyncs in `boots.txt`; a failed boot, at the
ceiling, the timeout or a non-zero exit, fails its run), then runs `tools/psx_short_run.sh`
(host-tested by `test_psx_short_run` in `psx-tools-test`) into
`<name>/analysis`. One line per condition, `ok` or `failed <reason>`
(`verdicts.txt`):

- `done`: each boot printed its `done` line with the run's frame count, no
  stopped state, and after the `done` line its state dump through its last
  line, `psx-state: cheat activations <n>` (else `<boot> state dump
  incomplete`);
- `deterministic`: the two plain boots' `psx-frame:` lines are byte-identical;
- `representative`: over the window frames (mapped), the plain run's draw sum
  and update sum each within ±10% of `PSX_SHORT_RUN_REFERENCE`'s (default
  `tmp/psx/full-game/run/frame-budget/frames.txt`, written by
  `psx-full-game-check`); both runs' figures side by side in `window.txt`.
  An empty reference skips it, so a changed program can still use the runs;
  a set path that does not exist fails the target naming
  `make psx-full-game-check`;
- `breakdown`: the calibration in 15,500–19,000, one `psx-draw:` record per
  frame, no wrapped frame, and no frame's in-call time above the breakdown
  boot's own draw;
- `asserts`: each boot's first `psx-replay: asserts` line reports the
  mode `PSX_SHORT_RUN_ASSERTS` asked for, naming any boot that says
  otherwise or has no such line.

`summary.txt`, over the window frames: first the plain-1 boot's sums,
`window plain update <U> draw <D> submit <S>` (with or without a reference,
even when the breakdown boot failed; `window plain unavailable: ...` when
plain-1 lacks a window frame), then each family's hblanks, calls and share
of the breakdown boot's draw (`pattern` apart), `in-calls` (everything under
the p8 draw API: meter, filter and GPU backend) and `outside-calls` (the
game's own code in `cart_draw`); `overhead` (breakdown draw − plain draw);
then per frame and as the window's maximum the best cases on the first plain
boot's parts with the breakdown's in-call share s: `backend-free` (update +
submit + draw × (1 − s)), `game-draw-free` (update + submit + draw × s) and
`draw-free` (update + submit), each with the frames still over
`PSX_FRAME_BUDGET_HBLANKS`. The target fails after every run when a
boot or a condition failed. It is not part of `psx-check`.

**Probes.** `make psx-probe PATCH=<file> [PSX_SHORT_RUNS=<names>] [PSX_PROBE_LABEL=<label>]`
measures a throwaway patch (a unified diff against `HEAD`, what `git diff`
prints) on the short runs. It adds a detached git worktree of `HEAD` in
`tmp/psx/probe/<label>/worktree` (a leftover one is removed and pruned
first; it is removed on every exit), `git apply`s the patch there, and per
run builds the *probe build*: the same configuration as `psx-short-runs`
with the source directory, `SLIPWAYS_SRC_DIR` and every `src/` source taken
from the worktree (generated units and the run's `run.embedded.c` stay the
main tree's, so a patch that needs them regenerated is out of scope),
`-DPSX_PROBE=ON`, `-DPSX_MEASURED_REPLAY_ASSERTS=OFF` and, always,
`-DPSX_MEASURED_REPLAY_OPT=O3` (the build directory is kept and CMake caches
the option), target `psx-gpu-replay` only. After the build
`tools/psx_opt_level.sh` reads its level: anything but `O3` makes the run
`invalid opt <level>, not O3`, and it is not booted. `PSX_PROBE` doubles `GPU_FRAME_BUFFER_BYTES` (one
constant for the backend's overrun check and byte limit, the meter and the
`gpu_frame` zone, so calling even `cart_draw` twice fits; the probe build has
no meter) and links the delay helper
`platform/psx_probe.h`: `psx_probe_delay(hblanks)` busy-waits that many
hblanks (1..200, one value per build, otherwise `p8_fatal`) on the fine
clock calibrated at start, and counts its calls; the boot prints
`psx-probe: clk8-per-64-hblanks <n>` and, when it was called,
`psx-probe: delay <hblanks> calls <count>`. A patch uses it by including the
header and calling it at a function's entry. Each run boots once with an
empty capture script (`PSX_PROBE_CEILING` vsyncs, default 1080, and
`PSX_PROBE_TIMEOUT` s, default 20: twice the short run's) and
`tools/psx_probe.sh` (host-tested by `test_psx_probe`) compares it with the
run's probe baseline over the window frames: draw, update, submit and total,
baseline → probe, delta in hblanks and in % of the baseline window total;
`game-state same|differs` (every `psx-state:` line but zone, budget and cap,
first difference printed); the delay line (delta ÷ delay over the run, its
gap to the counted calls, and per window frame); then the ELF text sizes,
`text <baseline> -> <probe> (<delta> bytes)`. Outputs in
`tmp/psx/probe/<label>/<run>/` (`build.log`, `boot.tty.txt`,
`analysis/probe.txt`); the label defaults to the patch's base name.

A run is `invalid <reason>` instead, never a delta, when the patch does not
apply (git's message; nothing is built), the build fails, the run has no
baseline or a stale one, or either boot is missing, stopped (`p8_fatal`
message quoted), did not finish its frames, has asserts on, lacks a window
frame line, or has a bad calibration. Every run is measured; the target
exits 1 if one is invalid, 0 otherwise, differing game-state lines included.

`make psx-probe-baseline [PSX_SHORT_RUNS=<names>]` is the same pipeline
without a patch into `tmp/psx/probe/baseline/`: the deltas compare probe
builds with probe builds, never with `psx-short-runs`' measured replay,
whose asserts-off figures stay the absolute baseline. A valid run
gets `fingerprint.txt`: the tree ids of `src/` and `psx/`, the blob id of
`full_game.json` at `HEAD` and `opt O3`, so a baseline built at another
level is stale; `psx-probe` calls a run stale when its own
`HEAD` gives another, so a commit outside those paths keeps the baseline.
The sanity patches `psx/probes/call-twice-cart-draw.patch`,
`delay-update-and-draw.patch` (20 hblanks at the entry of `cart_update60`
and `cart_draw`) and `null.patch` (code linked next to `cart_draw`, never
run) are the tool's fixtures; when a commit moves their context they are
refreshed with it.

**PC frame profile.** `make frame-profile [EVENTS=] [REFERENCE=]` replays a
recording (default `full_game.json`, about 7.5 s) with `tmp/frame_profile`.
This build is `-O2` with asserts off, and the game, engine and PICO-8
runtime are compiled with `-finstrument-functions`. Per frame, it writes the
calls and PC time of each direct child of `cart_update60` and `cart_draw`
(keyed by call site: `draw_layers#1`, `#2`) and of each draw family, counted
as the breakdown counts them. It also splits each family per root child:
`frame <f> child-family <root> <child> <family> <calls> <ns>` for the calls
made while that child runs, at any depth, and `frame <f> self-family <root>
<family> <calls> <ns>` for the root's own calls, one line per family with at
least one call. These lines are kept only for the first 1,024 frames (a short
run's cap); a longer recording has none past that. Every mode of
`tools/frame_profile_check.sh` ignores them. The output goes to `tmp/frame-profile/<name>/`,
and the end state is checked (`same-game`). The binary takes an optional
fourth argument, a draw root other than `cart_draw` (for example
`draw_layers`), named on the `root draw <symbol>` header line; an unknown or
ambiguous symbol, or `cart_update60`, exits 1.

- `make frame-profile-breakdown-check [PSX_SHORT_RUNS=]` (default
  `draw-5564`) checks those call counts against each run's last breakdown
  boot, frame by frame over its window. It never boots.
- `make frame-profile-coverage [FRAME_PROFILE_THRESHOLD=] [FRAME_PROFILE_REPEATS=]`
  profiles the full game and the `start` and `draw-*` runs K times (default
  4, about 1 min). A full-run frame counts above the threshold only if it is
  above in all K profiles. A window's share is the median of its K shares.
  For each `cart_draw` child and family, it compares the share of draw over
  the full game with the share in each window, and fails while an item above
  2% on at least 15 frames is in no window.
- `make frame-profile-subtree ROOT=<symbol> [PSX_SHORT_RUNS=] [FRAME_PROFILE_REPEATS=]`
  profiles the `start` and `draw-*` runs (by default) K times with that
  function as the draw root, into `tmp/frame-profile/subtree/<symbol>/`.
  Its `summary.txt` gives, per run, the root's PC time per window frame,
  then each child called in the window with its calls per window frame
  (min–max) and its median share of the root's PC time, by decreasing
  share, then the root's self share. It never boots.
- Its tests are in `make frame-profile-test`.

None of these targets is in `make test-all`, and none runs a setup.

## Situations

Every frame of a replay gets a **situation** and a **family**: `steady`
(what the player stays in) or `one-off` (an event and its bounded tail). The
label is read from the game state only, never from input buttons or frame
times. This is step 2's rule of plan 22, **frozen**: the rule and the
committed table change only through step 6's renumbering (the loading screen
frame).

`tmp/situations` (`src/cmd/situations.c`, `src/devtools/situation.c`)
replays an events file headless on PC, title or resume start, never booting
the PSX. After each frame's `cart_update60()` it reads a snapshot: the screen
mode (pad-metrics' `title`, `pause`, `placement`, `bar`, `tech`, `score`,
`menu`, `jump`, `aim`, `slipway`) and whether the attract demo plays; whether
a game world (economy) exists and has its end status; the year; the counts of
colonised planets, built structures (protostars and structures in placement
excluded), placed slipways, invented techs and probes; the camera position
before shake, floored to pixels; the hovered and pad-focused entities; the
open panel's layout (the focused item and every item reachable from it
through pad neighbours, with their focus points; empty without a focus); and
whether a shake runs (the camera's, or an on-screen planet's).

For each frame, `before` is the previous frame's snapshot (frame 0: the
snapshot after `cart_init()`, or after the resume), `after` this frame's:

1. **Event frame.** The first that holds, in this order, labels the frame
   with that one-off event:
   1. `galaxy-generation`: no game world in `before`, one in `after`;
   2. `return-to-title`: a game world in `before`, none in `after`;
   3. `game-end`: the end status appears;
   4. `new-year`: the year increased;
   5. `research`: the invented tech count increased;
   6. `colonise`: the colonised planet count increased;
   7. `structure-built`: the built structure count increased;
   8. `slipway-built`: the placed slipway count increased;
   9. `probe`: the probe count changed;
   10. `screen-change`: the screen mode or the attract flag changed.
2. **Continuation.** Otherwise, if the previous frame was one-off and the
   screen is still settling (the layout differs between `before` and
   `after`, or a shake runs in `after`), the frame keeps the previous frame's
   label. Frame 0's previous label is the steady label of the start
   snapshot, so it never continues.
3. **Steady.** Otherwise, by `after`'s screen: `attract` while the demo
   plays; `title`; `pause`; `build` (bar or placement); `tech-panel`;
   `score-panel`; `menu` (any other open panel or box); `slipway-aim`
   (slipway started, target not chosen); on the map (jump or aim mode)
   `map-move` when the camera position, the hovered or the focused entity
   differs between `before` and `after`, else `map-rest`.

The ten steady labels and the ten event labels are fixed families, never
decided per frame.

**Table.** `psx/perf/full-run-situations.txt`, the labels of
`worst-case/full_game.json`: a `frame situation family` header, then
`<frame> <situation> <family>` per frame, frames 0..6058 numbered like the
frame table `psx/perf/full-run-frames.txt`. `test_situation_table` checks
its shape and a list of hand-picked frames (frame 12 is
`galaxy-generation`); `make situations-check` regenerates it and fails on any
difference.

**Report.** `make situation-report` (`tools/situation_report.sh`) joins a
frame table and a situation table line by line. Per situation: frames,
frames over 263 hblanks (total), mean update and draw. Per one-off
situation: occurrences and longest run. A run is a stretch of consecutive
frames with the same one-off label, counted as one occurrence: the table
does not mark event frames, so two back-to-back events of one label make one
longer run, and the check can only over-flag. A run longer than **31
frames** is marked `long`: a planet project's burst sets a camera shake of 3,
decaying 0.1 a frame for 30 frames, plus the event frame. Verdicts:
`frame-count` (same count and numbering), `families` (known names, one
family each), `one-off-runs` (no `long` run).

## Still to do

- `make psx-full-game-check` is red on `frame-budget`; nobody raises
  `PSX_FRAME_BUDGET_HBLANKS` to turn it green. Latest run (asserts off, O3,
  draw filter and draw meter off): 1,764 of 6,059 frames over 263 hblanks,
  worst frame 12 at 3,992 (galaxy generation; update 3,711), mean frame
  233.6 hblanks against 263, mean overrun 76. Draw still dominates the
  frames over (424,627 of their 598,793 hblanks), so the next work is draw.
  Spike work (galaxy generation, saves, slipway drops, launches, image
  builds) is not spread across frames. The thirteen other
  conditions held (`real-game-budget` did not exist yet). The judge is pcsx-redux at 263 hblanks; the Miyoo Mini
  Flip (PCSX-ReARMed) was never measured. Plan 22 judges by
  `real-game-budget` instead; on that table it is red: 1,133 steady frames
  over 263 (worst frame 4655, `build`, total 530), 631 one-off frames over
  for 1,391 vsyncs of 10,800.

- `make psx-short-runs` is red on `representative` for the resumed runs:
  the update spikes are not reproduced after the resume, and draw comes out
  lower than in the full run. `done`, `deterministic`, `breakdown` and
  `asserts` hold. The cut rule and the windows are not loosened. The first
  steady try, `map-rest-5538` (window 5538–5552, cut 5500), is labelled
  `same map-rest` but draws 27.5% less (3,465 vs 4,779; update −4.2%): on PC
  its window has half the `pset` calls (2,700 vs 5,386, no transit dots after
  the resume), 14 `circ` calls instead of 59 and fewer `rectfill` and `print`;
  it is not in the manifest.

- The frame table's 54,784 bytes are in the GPU replays' ELFs only, so in
  `psx-check`'s `ram-room` and `psx-full-game-check`'s `gpu-ram-room`, not in
  `game-ram-room`: the shipped `psx-game` is not instrumented.

- Hardware renderers: DuckStation's hardware renderer samples the x2 upscale
  quad unevenly (glyph outlines of uneven width, a broken-up title logo). Use
  its Software renderer until the frame is drawn at display resolution.

- A disc for real hardware or a Sony BIOS: `slipways.bin` carries no license
  data and no region string (PSn00bSDK ships none); the pinned OpenBIOS
  checks neither, so the disc is proven in the emulator only.
