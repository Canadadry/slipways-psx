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

`make psx-build [EVENTS=<events>]` builds four PS-EXEs and the game's CD
image in `tmp/psx/build` (`PSX_BUILD_DIR`), then prints the ELF sizes and the
image paths:

- `psx-replay.{elf,exe}`: the core replaying `EVENTS` (default
  `tests/e2e/pad_slipway.json`), drawing into the RAM framebuffer;
- `psx-gpu-replay.{elf,exe}`: the same replay drawing through the GPU backend;
- `psx-pad-probe.{elf,exe}`: the pad probe;
- `psx-game.{elf,exe}`: the game read from the pad, the EXE that ships (below);
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
family filter or capture code. A file added to `CORE_SRCS` is in the PSX
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
pattern textures, no software fallback** (decided; implemented in step 4).

- `spr` reads its texture through a CLUT built from `mem->draw_palette` at
  the call: entry `c` is 0x0000 (transparent) when bit 0x80 is set, else the
  RGB555 of `P8_PALETTE_RGB[draw_palette[c] & 0x0F]` with the mask bit 0x8000,
  so black stays opaque. CLUTs are cached in VRAM, keyed by the 16 bytes; a
  palette `hud.c` copied with `memcpy` needs nothing more.
- A `rectfill` or `circfill` span under a non-zero pattern is a doubled
  `SPRT` of a 16x16 4-bit x2 tile (the 8x8 tile of the 4x4 pattern, texel 1
  for a set bit, upscaled x2) repeated by a 16-texel texture window, with a
  2-entry CLUT: `col & 0x0F`, then `(col >> 4) & 0x0F` or 0x0000 under
  `fillp_transparent`. The pattern is indexed by world coordinates, before
  the camera (U/V origin = twice the world coordinate mod 8), as the
  software `fillp_color` does. A span whose U would cross 256 is cut in two
  `SPRT`s (the second from U 0, the same phase under the window), so no
  `SPRT` relies on the 8-bit UV wrap; the cost model counts the pieces. A slot is keyed by
  pattern, colour byte and transparent flag. A span without pattern is a
  `TILE`. Any change of texture source (the sprite sheet, the font, a
  pattern slot) sends the `DR_TPAGE` + `DR_TWIN` pair the cost model
  meters, both starting a frame on the sheet with no window.
- Rejected: rasterising those calls in RAM would need the framebuffer
  uploaded every frame, the copy the GPU path replaces.

**GPU primitive buffers: counted on PC, sized at peak + 1/8, outside
`ZONE_BUDGET_BYTES`, checked against the MIPS RAM.** `src/p8/draw_cost.c`
gives each call's GPU bytes and primitives (PSn00bSDK 0.24 packet sizes, an
upper bound; a call whose screen box misses 160x120 costs 0; a `line` costs
exactly one `TILE` per on-screen pixel, from the walk the backend sends, so 0
when none shows), and the draw
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

- init: NTSC 320x240 display on two display buffers; the packed sprite sheet
  (`p8_sprite_sheet_packed`, already the 4-bit texture layout) and the font
  texture uploaded upscaled x2 by `psx_gpu_textures_upscale` (host-tested by
  `test_psx_gpu_textures`), in chunks of 16 x2 lines through one 2 KiB
  buffer;
- each frame: draw area and offset on the 320x240 display buffer the frame
  will be shown from (the other one is on screen), the sheet's texture page,
  no texture window, then the primitives in call order (no depth sort) in a
  bump allocator over that frame's static buffer of
  `GPU_FRAME_BUFFER_BYTES`; the frame's one clear is the `cls` that opens
  every `cart_draw`, so a frame without one would show the buffer's frame
  N-2; the frame end appends nothing: it waits for the previous frame's
  `DrawSync`, shows it and submits this one, with no `VSync` wait;
- overflow of the buffer is a `p8_fatal` naming the frame and the bytes
  asked; a frame that sent more bytes than the meter counted is a `p8_fatal`
  naming the frame and both figures;
- `cls`, `pset`, `line`, `rect`, plain `rectfill`, `circ` and plain
  `circfill` spans are drawn at x2: each is computed as a 160x120 tile, then doubled by
  `psx_tiles_double` (x, y, w and h times `PSX_TILES_SCALE`) into a `TILE`
  of the display buffer, so each PICO-8 pixel is a uniform 2x2 block; `cls`
  is one black `TILE` of the whole 320x240 buffer; `rect` is up to four
  `TILE`s (its edges, or one when it has no inside) and a plain `rectfill`
  one `TILE`, `pset`, `line` and `circ` one 2x2 `TILE` at doubled
  coordinates per on-screen pixel (`line` from the software backend's line
  walk, `p8_draw_cost_line_pixels`, and `circ` from its midpoint loop), and a
  plain `circfill` one `TILE` span per screen row from the same loop, after the camera and clipped to the
  screen (`src/platform/psx_tiles.c`, host-tested against the software
  backend pixel for pixel, at 1x and upscaled x2, by `test_psx_tiles`),
  coloured with the 8-bit components of `P8_PALETTE_RGB[col & 0x0F]`;
- `spr` and `print` are placed and clipped at 1x, then doubled by
  `psx_sprite_double` (x, y, u, v, w and h times `PSX_TILES_SCALE`) into
  one `SPRT` of the display buffer reading the x2 texture in its own page,
  so each texel is a uniform 2x2 block (host-tested at 1x and upscaled x2
  by `test_psx_sprite` and `test_psx_text`); a doubled UV past its 256x256
  page is an assert;
- a patterned `rectfill` box or `circfill` span is placed and clipped at 1x,
  its (u, v) its top-left pixel's world coordinates mod 8, then doubled by
  `psx_sprite_double` and cut where its U would cross 256
  (`psx_pattern_sprites_x2`, at most 2 pieces): each piece is a raw-textured
  `SPRT` under a texture window (mask 0x1E, a 16-texel repeat) over its
  pattern slot's x2 tile in the font's page (`src/platform/psx_pattern.c`,
  host-tested against the software backend upscaled x2 by
  `test_psx_pattern`); a frame's x2 tiles and CLUTs are cached by
  `p8_draw_cost_pattern_key` (`src/platform/psx_pattern_cache.c`, built by
  `psx_gpu_textures_pattern_x2` and `psx_gpu_textures_pattern_clut`, one
  cache per primitive buffer) and uploaded with the spr CLUTs to the pattern
  slots and pattern CLUT lines; more than `GPU_PATTERN_SLOTS` in a frame is a
  `p8_fatal` naming the frame;
- `spr` (and `map` through it) is one `SPRT` from the x2 sheet's
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
  `test_psx_text`), from the x2 font texture `psx_gpu_init` uploads once:
  the 256 character codes' glyphs from `p8_font_glyph_rows`, each in a 4x6
  cell (texel 1 lit) upscaled to 8x12, and the 16 font CLUTs (entry 1 the colour, every other
  entry transparent), built by `psx_gpu_textures_font` and
  `psx_gpu_textures_font_clut` (host-tested by `test_psx_gpu_textures`);
  the CLUT is the font line of `col & 0x0F`;
- the cost model's byte sizes are checked against the SDK's `sizeof` at
  compile time.

VRAM layout (`src/platform/psx_vram_layout.c`, host-tested by
`test_psx_vram_layout`; a region moves by changing that module only), in
16-bit units:

| region | x, y | size | notes |
|---|---|---|---|
| display buffers | (0, 0), (0, 240) | 320x240 each | |
| sprite sheet | (512, 256) | 64x256 | x2, 256x256 texels 4-bit: the whole page (512, 256) |
| font | (576, 256) | 64x96 | x2, page (576, 256), v 0..95: 256 codes in 8x12-texel cells, 32 a row, 8 rows |
| pattern slots | (576, 352) | 64x160, 4x16 a slot | x2, the font's page (576, 256), v 96..255: 16x16 texels on 16-texel bounds for the texture window, 16 a row, up to 160 slots |
| CLUTs | (640, 0) | 16x64, 16x1 each | `GPU_CLUT_SLOTS` draw palettes, 16 font colours, `GPU_PATTERN_SLOTS` pattern CLUTs |

The textured calls read two pages: the sheet's (512, 256), and the one the
font and the pattern slots share (576, 256). The page at (512, 0) is free.

## Until-exit boot mode

`tools/psx_boot.sh <install-dir> --until-exit [--pad-script <file>] [--capture-script <file>] --exe <exe> <ceiling> <out-prefix>`
adds `-testmode`, so the program's write to the PCSX exit register quits the
emulator with its code. No capture is written, except the frame-locked ones of
`--capture-script` (below). `<ceiling>` is a vsync ceiling:
reaching it, or `PSX_BOOT_TIMEOUT` seconds, fails naming the input and the last
`psx-replay:`, `psx-pad:` or `psx-game:` line. Code 0 passes, any other fails naming it.
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
`cart_draw`, the meter's frame end, `psx_gpu_frame_end` with the metered
bytes, `psx_gpu_frame_begin`) driven by port 1 instead of a recording, with
no family filter and no capture handshake. It starts with an erased save
storage and a one-frame zero input during `cart_load` and `cart_init`, as a
PC replay of a pad recording without `initial_save`. It runs at most one frame
a vsync: a frame that starts on the vsync the previous one started on waits
for the next. The draw meter wraps the GPU backend, so a frame sending more
bytes than metered is a `p8_fatal`, as in the GPU replay.

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

**Pad log.** Its TTY is `psx-game: ready` once the handshake is raised, then
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

- **Family filter** (`src/devtools/draw_filter.c`, PC and PSX): a backend
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
  bits 0-4) and that the doubled `TILE`s are exact: zero `offpalette`, zero
  `nonuniform`.
- **Ceilings and references**: until a human accepts a family, its captures'
  `differ` may not exceed their line in `psx/gpu_capture_ceilings.txt`,
  written only by `make psx-gpu-capture-ceilings LABEL=<label>` from the last
  `make psx-check` run whose other conditions all passed, for that label's
  lines only. A count below its ceiling prints `below ceiling`, so the
  ceiling can be lowered with the same target; raising one is a regression
  to explain in its commit.
- **Acceptance**: `make psx-gpu-capture-accept LABEL=<label>` is the human's
  one command. After a green `make psx-check` and a review of the label's
  report lines and images, it writes the sha256 of every capture of that
  label (all of them for `pattern` and `all`) into
  `psx/gpu_capture_references.txt`, other labels untouched; from then on a
  capture with a reference must match it byte for byte, and its ceiling line
  is ignored. No agent and no skill runs it.

## Checks

| Target | What it proves | Time |
|---|---|---|
| `make psx-check` | the thirty conditions below, on `pad_slipway.json` and the title script, the game booted from its CD image; five boots | ~105 s |
| `make psx-game-check` | the five `game-*` conditions alone, the game EXE booted on `PSX_GAME_SCRIPT` (outputs in `tmp/psx/game`) | ~10 s |
| `make psx-game-cd-check` | the five `game-*` conditions on the CD boot, then `cue` and `cd-exe` (outputs in `tmp/psx/game-cd`) | ~15 s |
| `make psx-game-cd` | builds the CD image and prints its paths | build |
| `make psx-gpu-capture-report` | the GPU captures and their reports alone (outputs in `tmp/psx/gpu-capture`) | ~20 s |
| `make psx-replay-check` | the replay proof alone (game lines and digests), on `PSX_EVENTS` (outputs in `tmp/psx/replay`) | ~45 s |
| `make psx-pad-check` | the pad proof alone (outputs in `tmp/psx/pad`) | ~5 s |
| `make psx-full-game-check` | the replay proof on `worst-case/full_game.json` (outputs in `tmp/psx/full-game`) | minutes |

`make psx-check` is the PSX check target: new PSX work adds its own
condition to it. It builds the PS-EXEs, writes the PC references with
`STATE=`, `DIGEST=` and `CAPTURE_LIST=` in one run of the current
`tmp/replay`, boots the replay, the GPU replay (twice, both with the capture
script), the probe and the game's CD image (with `PSX_GAME_SCRIPT`) until
they exit, five boots, then checks every condition before
concluding, one line each (`tools/psx_check.sh`, outputs in
`tmp/psx/game-check`, conditions also in `run/conditions.txt`):

- `magic`: the four `.exe` start with `PS-X EXE`;
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
  `p8_input_free` in either replay ELF or the game ELF;
- `pad-script`: the probe's ordered pad states equal the script's
  (`tools/psx_pad_compare.sh`);
- the five game conditions, on the CD boot, from `tools/psx_game_check.sh` (outputs in
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
- `game-gpu-cost`: `gpu-cost` on the CD game: its meter lines equal those of
  the PC replay of its pad log (`run/game/pc_state.txt`), title frames
  included; both use one function, so they cannot drift;
- `game-cue`, `game-cd-exe`: `cue` and `cd-exe` of `tools/psx_cd_check.sh` on
  `PSX_GAME_CUE` and `psx-game.exe` (extracted disc in `run/game-cd/disc`);
  a disc that lands in the OpenBIOS shell fails `game-done` first, with "last
  progress line: (none)";
- `game-ram-room`: `ram-room` on the game ELF.

`measurements.txt` (in `tmp/psx/game-check/run`, and in the output directory of
`psx-replay-check` and `psx-full-game-check`) holds the MIPS `zone`, `budget`
and `cap` lines, `vsyncs <n>`, and `size` of the replay ELF; `psx-check`'s
adds the GPU replay's vsyncs, meter lines, `ram-room <used> <limit> <left>`
and `size`, each behind `gpu `, then the capture report lines behind
`gpu capture `, then the game's `vsyncs`, `frames`, `ram-room` and `size`,
behind `game `, then `bin-bytes <n> sectors <n>` and `boot-vsyncs <n>`
(from power-on, BIOS and CD load included) behind `game cd `. The MIPS figures
are lower than the PC's (4-byte pointers): a measurement, not a limit check.

Ceilings and timeouts are the first measured run plus half again:
`PSX_REPLAY_CEILING`/`PSX_REPLAY_TIMEOUT` for `pad_slipway`,
`PSX_GPU_CEILING`/`PSX_GPU_TIMEOUT` for the GPU replay of `pad_slipway`,
`PSX_PAD_CEILING`/`PSX_PAD_TIMEOUT` for the probe,
`PSX_GAME_CEILING`/`PSX_GAME_TIMEOUT` for the game EXE with the title script
(`psx-game-check`), `PSX_GAME_CD_CEILING`/`PSX_GAME_CD_TIMEOUT` (1,140 vsyncs,
20 s) for the CD boot (`psx-game-cd-check`, `psx-check`),
`PSX_FULL_GAME_CEILING`/`PSX_FULL_GAME_TIMEOUT` for `full_game`.
`psx-full-game-check` builds in its own directory and is not part of
`psx-check`: at minutes a run, every later step would pay it again.

None of these targets is in `make test-all`, and none runs a setup.

## Still to do

- A disc for real hardware or a Sony BIOS: `slipways.bin` carries no license
  data and no region string (PSn00bSDK ships none); the pinned OpenBIOS
  checks neither, so the disc is proven in the emulator only.

- The human's play: load `tmp/psx/build/slipways.cue` in a windowed
  emulator and judge the controller by hand. No agent opens a window.

- Pending human acceptance, all 14 labels once, at the end of
  `docs/plan-19.md` (its step 6): run `make
  psx-check`, review each capture's report line and
  `tmp/psx/game-check/run/gpu-captures/<label>-f<frame>.ppm` (PC | PSX |
  diff), then accept each label with the one command
  `make psx-gpu-capture-accept LABEL=<label>` and commit
  `psx/gpu_capture_references.txt`. The report lines below are those of the
  x2 backend:
  - `cls` f1: `cls f1 mask cls calls 1 differ 0 (0.0‰) near1 0 near2 0 far 0
    bbox - offpalette 0 nonuniform 0`, ceiling 0.
  - `rectfill` f11: `rectfill f11 mask cls,rectfill calls 8 differ 0 (0.0‰)
    near1 0 near2 0 far 0 bbox - offpalette 0 nonuniform 0`, ceiling 0.
  - `pset` f131: `pset f131 mask cls,pset calls 65 differ 0 (0.0‰) near1 0
    near2 0 far 0 bbox - offpalette 0 nonuniform 0`, ceiling 0.
  - `pattern` f190: `pattern f190 mask cls,rectfill,circfill,pattern calls 9
    differ 0 (0.0‰) near1 0 near2 0 far 0 bbox - offpalette 0 nonuniform
    0`, ceiling 0: the patterned HUD panels in aim mode, 1,984 lit pixels
    in 4 colours.
  - `pattern` f257: `pattern f257 mask cls,rectfill,circfill,pattern calls 9
    differ 0 (0.0‰) near1 0 near2 0 far 0 bbox - offpalette 0 nonuniform
    0`, ceiling 0: the background grid under the HUD panels in slipway
    mode, 2,028 lit pixels in 4 colours. Both `pattern` captures are
    accepted together.
  - `spr` f234: `spr f234 mask cls,spr calls 106 differ 0 (0.0‰) near1 0
    near2 0 far 0 bbox - offpalette 0 nonuniform 0`, ceiling 0: the
    background map and the planet sprites, 829 lit pixels in 14 colours.
  - `circfill` f263: `circfill f263 mask cls,line,circfill calls 12 differ 0
    (0.0‰) near1 0 near2 0 far 0 bbox - offpalette 0 nonuniform 0`, ceiling
    0. Every plain `circfill` of the game is the colour-0 disc behind a
    planet, invisible on a cleared screen: the mask keeps `line` so the
    discs show as the 36 pixels they cut from the colour-2 lines of the
    already linked p47.
  - `print` f275: `print f275 mask cls,print calls 81 differ 0 (0.0‰) near1
    0 near2 0 far 0 bbox - offpalette 0 nonuniform 0`, ceiling 0: "-too
    far-", the HUD figures and labels, 246 lit pixels in 4 colours.
  - `line` f276: `line f276 mask cls,line calls 16 differ 0 (0.0‰) near1 0
    near2 0 far 0 bbox - offpalette 0 nonuniform 0`, ceiling 2 (the 1x
    `LINE_F2`'s GPU stepping; `psx-check` prints `below ceiling: line f276
    2 -> 0`).
  - `rect` f300: `rect f300 mask cls,rect calls 8 differ 0 (0.0‰) near1 0
    near2 0 far 0 bbox - offpalette 0 nonuniform 0`, ceiling 0.
  - `circ` f314: `circ f314 mask cls,circ calls 2 differ 0 (0.0‰) near1 0
    near2 0 far 0 bbox - offpalette 0 nonuniform 0`, ceiling 0: the
    colour-7 hovered circle round p28 and a colour-2 planet ring.
  - `all` f100, f188, f265 (accepted together): `all f100 mask all calls
    357 differ 0 (0.0‰) near1 0 near2 0 far 0 bbox - offpalette 0
    nonuniform 0`, `all f188 mask all calls 298 differ 0 (0.0‰) near1 0
    near2 0 far 0 bbox - offpalette 0 nonuniform 0`, `all f265 mask all
    calls 327 differ 0 (0.0‰) near1 0 near2 0 far 0 bbox - offpalette 0
    nonuniform 0`, ceilings 0: every family at once, aim mode entered from
    p47 (f100), aim mode moving to the fold (f188), the aim on the already
    linked p47 in colour 2 with the forbidden sign (f265).

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
