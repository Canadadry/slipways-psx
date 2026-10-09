---
title: "Sprites, map and font embedded, with no JSON parsed at startup"
description: "The game reads its sprite sheet from pico8/gfx.json at startup with a JSON parser and fopen, which the PSX build has neither of, so the PS-EXE draws every sprite from an empty sheet."
status: needs-triage
---

## Problem Statement

The game has three kinds of graphic asset: the sprite sheet, the map and the font. Only the sprite sheet comes from a file. Both PC binaries (`replay` and the raylib `game`) call `p8_load_gfx_json(&memory, "pico8/gfx.json")` at startup. That loader opens the file with `fopen`, reads it whole into a `malloc` buffer and scans it with `strstr`/`strtol`. The PSX has no file system for the game, and its libc has no `FILE` and no `fopen`. The JSON loader (`src/p8/assets.c`) is also deliberately left out of the core source list (`CORE_SRCS`) that the PSX build uses. So the step-1 PS-EXE replays the game with an all-zero sprite sheet. Every `spr` and every `map` cell draws nothing, and nothing proves that the game would draw the same pictures on MIPS as on PC. Steps 3 and 4 (the GPU backend, sprites and the font in VRAM) need the sheet in the executable, in a form they can upload to VRAM.

The other two assets need no loading, which this PRD records as findings:

- **Map.** No map data exists. `background.c` builds the whole 32×32 map at runtime with `p8_mset`, from the game's random walk, and `p8_map` draws its cells through `p8_spr`. `p8-guard` already bans `map.json` from `src/` and `tests/`.
- **Font.** The 3×5 glyphs are `static const` tables compiled into `src/p8/gfx.c` (`font_glyph_rows`). Nothing is read at startup.

## Solution

The sprite sheet becomes a committed, generated C table of 8,192 bytes, packed 4 bits per pixel. A host tool regenerates it from `pico8/gfx.json` with the existing JSON loader, behind a `make` target, as `make sin-table` regenerates the sine table. A small loader in `src/p8/` unpacks it into `p8_vram_t.sprite_sheet`, with no file, no allocation and no parsing. That loader is part of the core, so it reaches the PSX build. Both PC binaries switch to it. `make e2e` keeps every `*.sha256` unchanged, which proves the embedded sheet is the JSON's pixel for pixel. A unit test in `make test` checks the same thing directly.

On PSX, the replay driver loads the embedded sheet before `cart_load`, as the PC replay does. Both drivers then write the same digest lines: the sprite sheet after loading, the screen at sampled frames, and the map at the end. `make psx-check` gains the step-2 conditions. The MIPS sprite sheet, every sampled screen and the final map must equal the PC's for the same recording. The PS-EXE must also hold no JSON loader. The framebuffer is the software one in RAM, still never shown. Equal screens prove that the sprites, the generated map and the font all draw identically on MIPS.

## Hypotheses on step 1

Step 1 is being implemented while this PRD is written, from `docs/prd/triage/phase1-step1-core-on-mips.md`. Everything below is assumed about its outcome. **Rule for the implementer:** where a hypothesis turns out wrong, follow what step 1 actually committed (its names, targets, scripts and file layout) and keep the meaning of the step-2 proof unchanged. Record each corrected hypothesis in the step note.

- **H-1-1.** `make psx-check` exists. It is the plan-10 check target, outside `make test-all`. It evaluates every condition before concluding, prints one line per condition, fails if any fails, and a later step extends it by adding conditions.
- **H-1-2.** The PSX game build is a CMake project for the PSn00bSDK toolchain file. The Makefile hands it a source list derived from `CORE_SRCS`, plus the memory save storage, the shared state formatter, the PSX platform files and the generated replay source. A file added to `ENGINE_SRCS` and not filtered out of `CORE_SRCS` reaches the PS-EXE with no change to the CMake project.
- **H-1-3.** `CORE_SRCS` still leaves out `src/p8/assets.c`, `src/devtools/input_json.c`, `src/devtools/dump.c` and `src/platform/fatal_pc.c`. The PS-EXE therefore links no JSON parser today.
- **H-1-4.** A PSX replay driver (one file in `src/cmd/`, PSX only) sets up the input, memory and graphics contexts and does **not** load the sprite sheet. It seeds the memory save storage, runs `cart_load`/`cart_init`, then the PC frame loop (input step, `cart_update60`, `cart_draw`, input advance). It prints `psx-replay:` lines, then the `psx-state: ` block, and leaves through the exit module with code 0.
- **H-1-5.** The PSX platform provides a TTY line sink: a write callback and its context that print one line, optionally behind a fixed prefix. It is the sink used for the `psx-state: ` block.
- **H-1-6.** An embedded replay generator (a PC binary in `src/cmd/`) turns a recording into a generated C source. `make psx-check` builds the replay PS-EXE for `tests/e2e/pad_slipway.json` (401 frames).
- **H-1-7.** `make psx-check` produces the PC reference itself. It runs the current PC replay binary on the same recording with `STATE=<file>`, in the check's output directory.
- **H-1-8.** `psx_boot.sh` has an "until exit" mode (`-testmode`, PCSX exit register). Its vsync ceiling and timeout for the replay proof were set from the first measured run plus half again, and recorded in the step-1 note.
- **H-1-9.** `make psx-check` writes a measurement file next to its outputs. The file holds the MIPS `zone`/`budget`/`cap` lines, the vsync count and the ELF's text/data/bss sizes from the toolchain's `size`.
- **H-1-10.** The PSn00bSDK prebuilt GCC ships binutils next to `size`, including the `mipsel-none-elf-` `nm`, under `PSN00BSDK_DIR`.
- **H-1-11.** `make psx-full-game-check` exists: the same replay proof on `worst-case/full_game.json`, not a prerequisite of `psx-check`. Its outcome (green, or pending for plan 11) is in the step-1 note.
- **H-1-12.** Step 1 is done: `make psx-check` is green on the plan branch when step 2 starts. If it is not, step 2 stops at once with a report. It does not repair step 1.

## User Stories

1. As the plan-pilot run, I want step 2 to start by checking that `make psx-check` is green, so that the asset proof is never built on a broken step-1 proof.
2. As a developer, I want the sprite sheet compiled into the executable, so that neither the PC nor the PSX needs a file, `fopen` or a JSON parser to draw a sprite.
3. As a developer, I want the embedded sheet generated from `pico8/gfx.json` by the existing JSON loader, so that it holds exactly the bytes the game has used until now.
4. As a developer, I want the generated table committed, so that the PSX build needs no host tool run before CMake, and a clean checkout builds as it is.
5. As a developer, I want one `make` target that regenerates the table, as `make sin-table` does for the sine table, so that regenerating it is one command and never a hand edit.
6. As a developer, I want the generator to refuse a sheet whose load fails or that holds a value above 15, naming the file and exiting non-zero without touching the committed table, so that a bad input never becomes a silently wrong sheet.
7. As a developer, I want the generated file to say that it is generated and by which target, so that nobody edits it by hand.
8. As a developer, I want the sheet packed 4 bits per pixel, the left pixel in the low nibble, so that it costs 8 KB instead of 16 KB in the executable.
9. As the step-4 implementer, I want the packed sheet in the PSX GPU's 4-bit texture layout (four pixels per 16-bit word, leftmost in the lowest nibble, 128 pixels = 64 bytes per row), so that I can upload it to VRAM as it is.
10. As the step-4 implementer, I want read-only access to the packed bytes through the sprite-sheet module, with static storage that is never freed, so that I do not unpack and repack the sheet.
11. As a developer, I want one loader call that unpacks the embedded sheet into `p8_vram_t.sprite_sheet`, so that both drivers and every platform set up the sheet the same way.
12. As a developer, I want that loader to touch nothing in `p8_vram_t` but the sprite sheet, so that the draw palette, the map and the screen keep their initial state.
13. As a developer, I want that loader to allocate nothing and use no floating point, so that it passes `alloc-guard`, `float-guard` and `float-grep` as part of the core.
14. As a developer, I want the loader in the core source list, so that it reaches the PSX build through `CORE_SRCS` without the CMake project being edited (H-1-2).
15. As a developer, I want `make replay`, `make state` and `make e2e` to use the embedded sheet, so that the e2e hashes prove on every commit that the embedded sheet draws exactly what the JSON sheet drew.
16. As a developer, I want every `*.sha256` in `tests/e2e/` and every `make state` output unchanged by this step, so that the switch is proven to be pixel-exact.
17. As a player on PC, I want the raylib game to start without reading `pico8/gfx.json`, so that it no longer depends on the directory it is launched from to show its sprites.
18. As a developer, I want a unit test in `make test` that checks the embedded sheet equals the sheet the JSON loader reads from `pico8/gfx.json`, byte for byte, so that a stale table is caught in the fast loop and not only by the e2e.
19. As a developer, I want a unit test that checks the nibble order against known pixels, so that a swapped nibble is named directly and not found as a shifted sprite in an e2e capture.
20. As a developer, I want the JSON loader and its own test kept, so that the generator and the existing unit tests that read `pico8/gfx.json` keep working unchanged.
21. As a developer, I want the PSX replay driver to load the embedded sheet before `cart_load`, as the PC replay does, so that the MIPS game draws with the real sprites.
22. As a developer, I want a digest of the sprite sheet printed by both drivers right after loading, so that a sheet that differs on MIPS (wrong packing, wrong section, wrong copy) is named before any frame runs.
23. As a developer, I want a digest of the software screen printed by both drivers at sampled frames, so that I know whether sprites, the generated map and the font draw identically on MIPS and on PC.
24. As a developer, I want the screen sampled at a fixed set of frames (every 32nd frame from frame 0, plus the last), so that the title screen's text and the gameplay screens are both covered without hashing every frame on a 33 MHz CPU.
25. As a developer, I want each screen digest line to carry its frame index, so that a mismatch names the first frame where the pictures differ.
26. As a developer, I want a digest of the map cells printed at the end of the replay, so that a difference in the generated map is told apart from a difference in drawing.
27. As a developer, I want one digest formatter shared by the PC and the PSX, writing through a line callback, so that a difference in the comparison is a difference in the game, not in two printers.
28. As a developer, I want the digest hex written by the formatter itself and not by `printf`, so that a PSn00bSDK formatting quirk can never fake a mismatch.
29. As a developer, I want the PC replay to write the digest lines to a file only when `DIGEST=<file>` is set, as `STATE=` does, so that `make e2e` and `make test` get no slower.
30. As a developer, I want the PSX digest lines printed over TTY behind a fixed prefix (`psx-digest: `), so that they can be pulled out of a TTY file that also holds emulator log lines.
31. As a developer, I want a comparison script that compares the PSX digest lines with the PC's, in order, and names the first differing line from both sides, so that I can start debugging from it.
32. As a developer, I want that script to report the sprite sheet, the screens and the map as three separate results, so that one run says which asset diverged.
33. As a developer, I want that script to fail when either side has fewer lines than the other, so that a truncated TTY can never look like a match.
34. As a developer, I want `make psx-check` to gain the conditions `sprite-sheet`, `screen`, `map` and `no-json`, one line each, so that the step-2 proof sits beside step 1's (H-1-1).
35. As a developer, I want the `no-json` condition to check with the toolchain's `nm` that the replay ELF defines no symbol from the JSON loaders, so that "no JSON parsed at startup" is proven on the binary and not assumed from a source list.
36. As a developer, I want the PC reference digests produced by `make psx-check` itself, in the same PC replay run as the state reference, so that a stale file can never be compared (H-1-7).
37. As a developer, I want the measurement file to record the new ELF sizes and the vsync count after the sheet is embedded and the digests are added, so that plan 11 sees what the 8 KB table and the hashing cost.
38. As a developer, I want the replay boot's vsync ceiling and timeout re-measured and raised if the digests need it, so that the check does not go red for a reason that has nothing to do with assets.
39. As a developer, I want `make psx-full-game-check` to check the same digest conditions when it runs green, so that the long game's pictures are covered too, with the outcome recorded in the step note and not a done criterion.
40. As a developer, I want a screen divergence on MIPS treated like a state divergence in step 1 (fixed in the core only if every `*.sha256` and every `make state` output stays identical, otherwise the step stops with a report), so that the software renderer is never made to differ between machines.
41. As a reviewer, I want the docs and the Makefile legend to describe the embedded sheet, `make sprite-sheet`, `DIGEST=` and the new `psx-check` conditions as they are, so that the next session can use them without reading this PRD.
42. As a reviewer, I want the PRD's finding that the map and the font need no loading recorded in the step note, so that nobody looks for a map or font asset file later.
43. As a human, I want the emulator never opened in a window, so that verification stays headless.

## Implementation Decisions

### Start

- Step 2 starts by running `make psx-check` (H-1-12). Red, or a missing setup, stops the run with a report. Nothing is patched or re-baselined to make it green.

### Scope of "assets"

- **Map: nothing to embed.** The map is generated at runtime by the background with `p8_mset` and drawn through `p8_spr`. The step proves it on MIPS through the `map` and `screen` digests and changes no map code.
- **Font: nothing to embed.** The glyph tables are already `static const` data in the core. The step proves them on MIPS through the `screen` digests, since the title screen and the HUD print text, and changes no font code. Turning the glyphs into a VRAM texture is part of the GPU `print` family in step 4.
- **Sprite sheet: embedded.** It is the only asset read from a file, and this step's work.

### Modules

- **Sprite-sheet module (`src/p8/`, core, new).** A small interface with two exports:
  - a load call that takes a `p8_vram_t *` and unpacks the embedded sheet into `sprite_sheet`, touching no other field;
  - an accessor that returns the packed bytes (static storage, never freed, `P8_SPRITE_SHEET_WIDTH * P8_SPRITE_SHEET_HEIGHT / 2` bytes).

  Names follow `module_verb[_object]` with the `p8_` prefix, for example `p8_sprite_sheet_load` and `p8_sprite_sheet_packed`. The file is added to `ENGINE_SRCS` and not filtered out of `CORE_SRCS`, so it reaches the PSX build (H-1-2). It allocates nothing and uses no floating point.
- **Packed layout.** Row-major, 64 bytes per row, 128 rows. The pixel at `(x, y)` sits in byte `y * 64 + x / 2`: the low nibble when `x` is even, the high nibble when `x` is odd. This is PICO-8's own sprite memory layout and the PSX GPU's 4-bit texture layout, so step 4 can upload the bytes unchanged.
- **Generated table (committed, `src/p8/`).** A header holding the packed bytes as a `static const unsigned char` array, included only by the sprite-sheet module, as `quarter_wave.h` is included by the fixed-point code. Its first line says it is generated by `make sprite-sheet` from `pico8/gfx.json` and must not be edited. This one-line comment is allowed by the comment rules, because it warns about a trap.
- **Generator (`tools/`, host only).** A C program like `tools/gen_quarter_wave.c`. It links the existing JSON loader and `p8_vram_init`, loads `pico8/gfx.json` into a `p8_vram_t`, and checks that the load succeeded and that every pixel is between 0 and 15. It then writes the packed header to stdout. On failure it exits non-zero naming the file and writes nothing.
- **`make sprite-sheet` (outside the suite).** It builds the generator, writes into `tmp/`, and moves the result over the committed header only when the generator succeeded. This follows the `sin-table` recipe.
- **PC drivers.** The PC replay driver and the raylib game call the sprite-sheet load in place of `p8_load_gfx_json`. Neither reads `pico8/gfx.json` any longer. The JSON loader stays where it is, outside `CORE_SRCS`, for the generator and the unit tests that read the JSON today. Those tests are not migrated in this step.
- **VRAM digest (`src/devtools/`, shared by PC and PSX, new).** It uses 32-bit FNV-1a over bytes with `uint32_t` arithmetic only, and writes lines through a callback (line text, context). It formats the hex itself and uses `snprintf` only for the decimal frame index. It produces three kinds of line, without a prefix:
  - `sprite-sheet <8 lowercase hex>`: the 16,384 bytes of `sprite_sheet`;
  - `screen <frame> <8 lowercase hex>`: the 19,200 bytes of `screen`, for frame indices `i` with `i % 32 == 0`, and for the last frame;
  - `map <8 lowercase hex>`: the 1,024 bytes of `map_cells`.

  The sampling rule is part of the module, as a call that takes the frame index and the frame count, so both drivers sample the same frames. The module is not part of `CORE_SRCS` (it is devtools). The PSX source list gets it the way it gets the shared state formatter (H-1-2).
- **PC replay, `DIGEST=<file>`.** When set, the replay writes `sprite-sheet` after loading the sheet, `screen` after `cart_draw` on each sampled frame, and `map` after the loop, one line each, to that file. When unset, nothing is hashed, so `make e2e` and `make test` cost nothing more. The `make replay` and `make state` legends mention it.
- **PSX replay driver (H-1-4).** It loads the embedded sheet before `cart_load`, then prints the same digest lines at the same points through the TTY sink (H-1-5) behind `psx-digest: `. The `psx-state: ` block and the `psx-replay: done` line are unchanged, and the digest lines come before the state block.
- **Digest comparison script (`tools/`, new).** Its inputs are a TTY file, a PC digest file and an output directory. It extracts the `psx-digest: ` lines and compares them in order with the PC file's lines. It prints three result lines, `sprite-sheet`, `screen` and `map`, each `ok` or naming the first differing line from both sides. A missing or extra line counts as a difference in the kind where it occurs. It exits non-zero if any result fails. It is separate from step 1's game-line script, so it does not depend on that script's interface.
- **`make psx-check` (H-1-1, H-1-7).** Four conditions are added:
  - `sprite-sheet`, `screen` and `map`: the comparison script's results, against the PC digest file written by the same PC replay run that writes the state reference (`DIGEST=` beside `STATE=`);
  - `no-json`: the toolchain's `nm` (H-1-10) on the replay ELF finds none of the symbols defined by the JSON sprite loader and the JSON recording loader (for example `p8_load_gfx_json`).

  Step 1's conditions are unchanged. The measurement file (H-1-9) gets the new text/data/bss sizes and the vsync count.
- **Ceiling and timeout (H-1-8).** They are re-measured on the first green run. If the digests push the boot past the step-1 ceiling, it is reset to the new measurement plus half again, and the new figures are recorded in the step note.
- **`make psx-full-game-check` (H-1-11).** It gains the same digest conditions. Its outcome (green, or still pending with its reason) is recorded in the step note and is not a done criterion of this step.
- **Divergence policy.** The software renderer is the same C code on both machines, and plan 10's accepted divergence applies only to the GPU backend. A screen digest mismatch is therefore a portability bug. It is fixed in `src/p8/`, `src/game/` or `src/engine/` only if every `*.sha256` and every `make state` output stays identical, and it is recorded in the step note as a portability fix. A mismatch that cannot be fixed that way stops the step with a report naming the first differing frame. To find which draw call differs, the implementer may dump the MIPS screen of that frame over TTY, but no such dump is committed as a check.
- **Docs.** The PSX game doc that step 1 creates gains the embedded sheet, the digest lines and the new conditions. The Makefile legend gets `make sprite-sheet`, `DIGEST=` on `make replay`/`make state`, and the new `psx-check` conditions. No doc says the game loads `pico8/gfx.json` at startup any longer.

## Implementation Chunks

1. **Embedded sheet on PC.** Generator, `make sprite-sheet`, the committed packed header, the sprite-sheet module and its unit test. Then switch the PC replay and the raylib game to it. Done when `make test-all` is green with every `*.sha256` unchanged, `make state` gives identical output, and `make raylib` still links if the vendor setup is present.
2. **VRAM digest and `DIGEST=`** (PC only). The shared digest module with its unit test, and the PC replay writing the digest file. `make e2e` timing unchanged.
3. **Digest comparison script**, with its host test.
4. **PSX proof.** First check that `make psx-check` is green (H-1-12). Then the PSX replay driver loads the sheet and prints the digest lines. Add the four conditions to `make psx-check`, re-measure the ceiling, update the measurement file and run `make psx-full-game-check` once. Then the docs and the Makefile legend.

## Testing Decisions

- A good test checks behaviour seen from outside: the bytes in `p8_vram_t` after a load, the packed bytes the accessor returns, the lines a formatter writes, and the lines and exit code of a script. It never checks how the table is laid out in the source file.
- **Sprite-sheet module (new `test_sprite_sheet`, in `make test`).**
  - Loading into a fresh `p8_vram_t` gives a `sprite_sheet` equal byte for byte to what `p8_load_gfx_json` reads from `pico8/gfx.json`. This is the drift check that replaces any regeneration step in the suite.
  - Every other field of `p8_vram_t` (draw palette, map cells, screen) equals a freshly initialised one after the load.
  - For known pixels, the accessor's byte at `y * 64 + x / 2` holds the value in the low nibble for even `x` and the high nibble for odd `x`. Use `(16, 0) = 3`, `(2, 64) = 1` and `(0, 127) = 3`, the values `test_asset_loading` already pins, plus one odd-`x` pixel taken from the JSON.
  - Prior art: `test_asset_loading`.
- **VRAM digest (new `test_vram_digest`, in `make test`).**
  - The FNV-1a value of a known byte string equals the published reference value.
  - The sampling rule selects frames 0, 32, 64… and the last frame, and no other.
  - The three line kinds come out in the documented text form through the callback, with hex in lowercase and zero-padded to 8 digits.
  - Changing one screen pixel changes the `screen` line.
- **Digest comparison script (new host test, as `test_frames_check` drives `frames_check.sh`).** It runs on crafted files and covers six cases:
  - identical lines inside a TTY interleaved with emulator log lines pass;
  - a differing `sprite-sheet` line fails that result only;
  - a differing `screen` line names its frame from both sides;
  - a differing `map` line fails that result only;
  - a TTY missing its last `screen` line fails;
  - an empty TTY fails all three.
- **PC switch.** No new test. `make e2e` (every `*.sha256` unchanged) and `make state` (identical output) are the net, as CLAUDE.md makes the e2e the source of truth.
- **PSX side.** `make psx-check` and `make psx-full-game-check` are integration checks outside `make test-all`. Their four new conditions are the tests of the PSX driver change. No unit test runs the toolchain or the emulator.
- `make test` and `make e2e` must not get slower. The new tests are millisecond-scale host binaries. The PC replay hashes nothing unless `DIGEST=` is set.
- Every commit keeps `make test-all`, `make sanitize-net`, `make full-game` and `make state` (with its 1/8 margin) green, and leaves every `*.sha256` unchanged.

## Out of Scope

- Any GPU rendering, VRAM upload of the sheet or the font, CLUTs, or VRAM capture of the game (steps 3 and 4). That includes the Open questions on `fillp`/`palt` on the GPU and on whether the primitive buffers fit, which are settled before step 3.
- A font texture or atlas: the glyphs stay compiled-in tables, and their VRAM form belongs to the GPU `print` family (step 4).
- Changing how the map is generated or stored.
- Removing the `p8_vram_t` RAM copy of the sheet (drawing straight from the const table) or any other memory saving on MIPS (plan 11).
- Migrating the existing unit tests off `p8_load_gfx_json`, or deleting the JSON loader.
- The PSX pad driver, the title screen through the controller (step 5) and the `.bin/.cue` (step 6).
- Audio.

## Further Notes

- `pico8/gfx.json` holds 128×128 = 16,384 values, all between 0 and 15 (7,833 of them non-zero), so packing to 4 bits loses nothing.
- The embedded table adds 8 KB of read-only data to the PS-EXE, and the `p8_vram_t` copy stays 16 KB in RAM as it is today. Plan 11 may draw straight from the packed table, since the accessor exposes it.
- Hashing the screen on 1 frame in 32 costs about 13 × 19,200 bytes for `pad_slipway.json`, small next to the replay itself. The vsync count in the measurement file shows the actual cost.
- The step note should record: the start check's result, the map and font findings, the digest results of `psx-check` and of `psx-full-game-check`, any portability fix, the new ELF sizes and vsync count, the new ceiling if it changed, and every corrected step-1 hypothesis.
