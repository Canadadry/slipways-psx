---
title: "Drawing API split from its backend: software for PC, GPU skeleton for PSX"
description: "Every PICO-8 draw call writes pixels straight into the 160x120 RAM framebuffer, so the PSX build cannot draw through its GPU without either changing the game or copying that framebuffer each frame."
status: needs-triage
---

## Problem Statement

The PICO-8 drawing API in the runtime (`cls`, `pset`, `line`, `rect`, `rectfill`, `circ`, `circfill`, `spr`, `map`, `print`, plus the state calls `camera`, `fillp`, `palt`, `mset`) is one module. Each call rasterises into `p8_vram_t.screen`, the 160×120 byte framebuffer in RAM. The PC shows that buffer (raylib) and hashes it (the e2e captures). Plan 10 needs the PSX to draw the same calls as GPU primitives instead of copying a framebuffer. Nothing in the code separates "what the game asked to draw" from "how a machine draws it":

- The game calls the API from about fifteen files in `src/game/`, always with a graphics state and the VRAM struct. Plan 10's rule is that the core does not move, so the split must happen under that API, in `src/p8/`, with the PSX side in `src/platform/`.
- The software routines call each other through the public API (`rect`, `line`, `circ` and `print` call the public `pset`; `map` calls the public `spr`). A backend placed under the API would see those internal calls as game calls.
- `cls` takes only the VRAM struct. `hud.c` writes the draw palette with a `memcpy` of a 16-byte block, not through `palt`. A backend has to find the draw state where the game leaves it.
- Two open questions of the plan must be settled before the split, because the split must allow their answers. (a) How `fillp`, `palt` and the draw palette are drawn on the GPU. (b) Whether the GPU primitive buffers fit. Nobody has counted the primitives a frame would need, and the only room `make state` reports is the space left under `ZONE_BUDGET_BYTES`.

## Solution

The drawing API keeps its names and signatures. Underneath it, a small backend interface takes each draw call. A process-wide setter selects the backend, and the software backend is the default. The software backend is today's rasteriser moved unchanged. It writes `p8_vram_t.screen`, so the PC game, `make e2e`, `make state` and every unit test keep the same pixels, and every `*.sha256` stays unchanged. State calls (`camera`, `fillp`, `palt`, `mset`, the draw palette) stay in the front end and its structs. A backend reads them when a draw call reaches it.

The two open questions are decided here:

- **(a) Per-texture CLUT and pattern textures, no software fallback.** `spr` and `map` get their remap and transparency from a CLUT built from the 16-byte draw palette as it is when `spr` is called (transparent bit → texel 0x0000). `fillp` on `rectfill`/`circfill` becomes a textured sprite: a 4-bit 8×8 pattern texture repeated by the GPU texture window, with a 2-entry CLUT (clear bit → low colour, set bit → high colour, or 0x0000 for `fillp_transparent`). The backend interface passes the graphics state and the VRAM struct to every draw call, so the GPU backend sees the camera, the fill pattern, the full colour byte and the current draw palette at draw time.
- **(b) Counted on PC, sized at peak + 1/8, outside the zone budget, checked against the real MIPS RAM room.** A cost model gives the GPU bytes and primitives each draw call will cost. A meter adds them up per frame. `make state` reports the per-frame peak over every e2e and worst-case game, as a `zone` line judged by its 1/8 margin like any zone. It also reports the peaks of distinct draw palettes and of fill patterns per frame, which size the VRAM caches. The two primitive buffers are a static reservation of the PSX GPU backend. They do not come out of `ZONE_BUDGET_BYTES`: the room `make state` reports there is about 2 KB, smaller than one frame of the background map. `make psx-check` checks that they fit on the actual GPU PS-EXE: kernel area + ELF text/data/bss (buffers included) + stack reserve ≤ 2 MiB.

On PSX, a GPU backend skeleton is added in `src/platform/`. It sets up the display, the 160×120 draw area and the VRAM layout, uploads the sprite sheet, and manages the double-buffered primitive buffers and the frame lifecycle. It implements `cls`, the smallest call, as the pipeline's proof. Every other draw call is accepted and draws nothing yet: step 4 fills them in one family at a time. A new PS-EXE replays the e2e recording through the GPU backend. `make psx-check` gains conditions proving that it runs to the end without overflowing its buffers, leaves the game state identical to the PC's, sends the backend the same command stream (equal meter lines on MIPS and PC), and fits in RAM.

## Hypotheses on step 2

Step 2 is being implemented while this PRD is written, from `docs/prd/triage/phase1-step2-embedded-assets.md`. Its first chunk was committed as this PRD was written (60b73cb). Everything below is assumed about its final outcome. **Rule for the implementer:** where a hypothesis turns out wrong, follow what step 2 actually committed (names, targets, scripts, file layout) and keep the meaning of the step-3 proof unchanged. Record each corrected hypothesis in the step note.

- **H-2-1.** The sprite-sheet module in `src/p8/` exports `p8_sprite_sheet_load(p8_vram_t *)`, which writes only `sprite_sheet`, and `p8_sprite_sheet_packed()`, which returns 8,192 static bytes. A pixel `(x, y)` sits in byte `y * 64 + x / 2`, in the low nibble for an even `x`. This is the PSX 4-bit texture layout. The module is in `ENGINE_SRCS` and in `CORE_SRCS`.
- **H-2-2.** Both PC drivers (`replay`, the raylib `game`) load the sheet through that module. Neither parses `pico8/gfx.json` at startup.
- **H-2-3.** The software PSX replay driver loads the embedded sheet before `cart_load`.
- **H-2-4.** A shared VRAM digest module exists in `src/devtools/`. The PC replay writes digest lines when `DIGEST=<file>` is set. The PSX replay prints them behind `psx-digest: `. `make psx-check` has the step-2 conditions `sprite-sheet`, `screen`, `map` and `no-json`.
- **H-2-5.** `make psx-check` (`tools/psx_check.sh`) still evaluates every condition before concluding, prints one line per condition, and is extended by adding conditions and arguments.
- **H-2-6.** The replay ceiling and timeout and the measurement file follow step 1's recipe (first measured run plus half again; MIPS `zone`/`budget`/`cap` lines, vsyncs, ELF sizes). They may have been re-measured by step 2.
- **H-2-7.** Step 2 changes no drawing routine in `src/p8/gfx.c` and does not change the layout of `p8_vram_t` or `p8_graphics_t`.
- **H-2-8.** Step 2 is done and `make psx-check` is green on the plan branch when step 3 starts. If it is not, step 3 stops at once with a report. It does not repair step 2.

## User Stories

1. As the plan-pilot run, I want step 3 to start by checking that `make psx-check` is green, so that the backend split is never built on a broken step-2 proof (H-2-8).
2. As a developer, I want the drawing API's names and signatures unchanged, so that no file of `src/game/` or `src/engine/` changes in this step.
3. As a developer, I want one small backend interface under the API, with one entry per draw call, so that a machine only has to implement nine functions to draw the whole game.
4. As a developer, I want `map` and `spr_default` handled in the front end (`map` as one `spr` per non-empty cell, `spr_default` as a 1×1 `spr`), so that a backend never reimplements them.
5. As a developer, I want the state calls (`camera`, `camera_reset`, `fillp`, `fillp_transparent`, `fillp_reset`, `palt`, `mset`, `sget`, `gfx_is_transparent`) to stay in the front end and its structs, so that a backend only reads state and never owns it.
6. As a developer, I want every backend entry to receive the graphics state and the VRAM struct as they are at call time, so that a backend sees the camera, the fill pattern, the full colour byte and the draw palette the game set, including a palette that `hud.c` copied in with `memcpy`.
7. As a developer, I want `cls` to reach the backend even though it takes no graphics state, so that the GPU can clear its own draw area.
8. As a developer, I want the software backend to be the default with nothing to set up, so that the PC game, the replay, every test harness and every unit test draw exactly as before.
9. As a developer, I want the software backend to be today's code moved unchanged, its internal pixel writes kept internal, so that its output is identical and an internal `pset` is never counted as a game call.
10. As a developer, I want one setter that installs a backend and returns the previous one, so that a driver or a test can install a backend and restore the previous one.
11. As a developer, I want every `*.sha256` in `tests/e2e/` and every `make state` game line unchanged by the split, so that the split is proven to change no pixel.
12. As a developer, I want `make test` and `make e2e` no slower, so that the one indirect call per draw call never costs the fast loop.
13. As a developer, I want a unit test that installs a recording backend and checks which entry each API call reaches, with which arguments, so that the routing is tested through the public API and not through the software pixels.
14. As a developer, I want the plan's open question (a) decided and written down: per-texture CLUT for the draw palette and `palt`, a texture-window pattern for `fillp`, no software fallback, so that step 4 implements the families without reopening it.
15. As the step-4 implementer, I want the CLUT rule stated: entry `c` is the colour `draw_palette[c] & 0x0F` with the mask bit set, or 0x0000 when the transparent bit is set, so that black stays opaque and transparency matches the software `spr`.
16. As the step-4 implementer, I want the fill-pattern rule stated: the pattern is indexed by world coordinates, before the camera, as the software `fillp_color` does, so that the GPU pattern has the same phase as the PC's under a moving camera.
17. As the step-4 implementer, I want the VRAM layout fixed: display buffers, the 160×120 draw area, the sprite sheet, the font, the pattern slots and the CLUT region, so that each family has its place and the layout lives in one module.
18. As the step-4 implementer, I want the sprite sheet, font and pattern textures in one 4-bit texture page, so that switching between `spr`, `print` and a fill pattern never needs a texture-page change.
19. As the step-4 implementer, I want the game drawn 1:1 into a 160×120 draw area and scaled ×2 to a 320×240 display by one textured quad per frame, so that every primitive stays in PICO-8 pixels and a capture compares directly with the PC framebuffer.
20. As a developer, I want a cost model that gives, for each draw call and the current state, the GPU primitives and bytes the GPU backend may emit, so that the buffers are sized before the families exist.
21. As a developer, I want the cost model to cull calls whose screen bounding box is fully outside 160×120, as the GPU backend will, so that the off-screen half of the 32×32 map is not counted.
22. As a developer, I want the cost model's byte sizes checked against the PSn00bSDK primitive structs at PSX compile time, so that a wrong size fails the build instead of undersizing a buffer.
23. As a developer, I want the cost model to be an upper bound that the GPU backend checks each frame (real bytes ≤ metered bytes, else `p8_fatal` naming the frame and both numbers), so that step 4 can never silently exceed what was measured.
24. As a developer, I want a meter that wraps the active backend, adds up each frame's cost and keeps the per-frame peaks, so that counting needs no change to the front end or to any backend.
25. As a developer, I want the meter to count distinct draw-palette contents and distinct fill patterns per frame, so that the CLUT cache and the pattern slots are sized on measurement too.
26. As a developer, I want the PC replay to install the meter only when `STATE=` is set, so that `make e2e` and `make test` pay nothing for it.
27. As a developer, I want `make state` to report the per-frame GPU byte peak as a `zone gpu_frame` line and the cache peaks as `cap gpu_cluts` and `cap gpu_patterns` lines, so that the existing peak-and-margin machinery covers every e2e and every worst-case game with no new script.
28. As a developer, I want the buffer size and the cache slot counts to be constants sized at measured peak + 1/8, so that they follow plan 11's sizing rule and `make state` fails the day a change eats the margin.
29. As a developer, I want the plan's open question (b) decided and written down: two buffers, sized by `make state`, reserved outside `ZONE_BUDGET_BYTES`, checked against the MIPS RAM room, so that nobody tries to fit them in the ~2 KB left under the zone budget.
30. As a developer, I want `make state` to print the buffers' total next to the bytes left under `ZONE_BUDGET_BYTES`, so that both figures are visible in one place at every run.
31. As a developer, I want a GPU backend skeleton on PSX that initialises the GPU, the display and the VRAM layout, uploads the packed sprite sheet (H-2-1), and runs the double-buffered frame lifecycle, so that step 4 only adds primitives.
32. As a developer, I want the skeleton to implement `cls` and nothing else, the other entries accepting their call and drawing nothing, so that the pipeline is proven without starting step 4's families.
33. As a developer, I want a buffer overflow on PSX to stop with `p8_fatal` naming the frame and the bytes asked, so that an undersized buffer is never a silent corruption.
34. As a developer, I want a new PS-EXE that replays the e2e recording through the GPU backend with the same TTY protocol as the software replay, so that the existing boot and comparison scripts work on it unchanged.
35. As a developer, I want `make psx-check` to gain `gpu-replay-done`, `gpu-state-game-lines`, `gpu-cost` and `ram-room`, one line each, so that the step-3 proof sits beside steps 1 and 2.
36. As a developer, I want `gpu-cost` to require the MIPS meter lines to equal the PC's for the same recording, so that the backend is proven to receive the PC's exact command stream on MIPS.
37. As a developer, I want `ram-room` to read the GPU ELF's sizes with the toolchain's `size` and print the bytes left, so that the fit is measured on the binary at that moment and not taken from a figure in the plan.
38. As a developer, I want the measurement file to record the GPU PS-EXE's sizes, vsyncs and meter lines, so that plan 11 sees what the GPU path costs.
39. As a developer, I want the software PSX replay and its step-1 and step-2 conditions unchanged, so that the state and pixel proofs on MIPS keep running on the software renderer.
40. As a developer, I want the GPU backend's pure parts (VRAM layout constants, the cost model and its sizes) host-compiled and unit-tested, so that only the SDK calls are left untested on PC.
41. As a reviewer, I want the docs and the Makefile legend to describe the backend interface, the meter lines of `make state`, the GPU PS-EXE, the VRAM layout and the new `psx-check` conditions as they are, so that the next session can use them without reading this PRD.
42. As a reviewer, I want the step note to record both decisions, the measured peaks with the replay that holds each, the chosen constants, the RAM room, the new vsyncs and ceiling, and every corrected step-2 hypothesis, so that step 4 and plan 11 start from facts.
43. As a human, I want neither the raylib window nor the emulator opened in a window, so that verification stays headless.

## Implementation Decisions

### Start

- Step 3 starts by running `make psx-check` (H-2-8). If it is red, or a setup is missing, the run stops with a report. Nothing is patched or re-baselined to make it green.

### Decision (a): `fillp`, `palt` and the draw palette on the GPU

What the code shows: the draw palette is used only by `spr` (and `map` through it), for transparency (bit 0x80) and remapping (low nibble). `hud.c` replaces all 16 entries at once from its blocks, and `palt` toggles the transparency of colours 0 and 3. `fillp` affects only `rectfill` and `circfill`. The game uses three patterns: 0xA5A5 for the grid lines of the background, 0x0F0F for the message box bar, and 0xAAAA as a transparent pattern on the planet circle. The pattern bit is taken at world coordinates, before the camera is subtracted.

Decision:

- **Draw palette and `palt`: per-texture CLUT.** On each `spr`, the GPU backend uses a CLUT built from `mem->draw_palette` as it is at that moment. Entry `c` is 0x0000 (a transparent texel) when bit 0x80 is set. Otherwise it is the RGB555 of `P8_PALETTE_RGB[draw_palette[c] & 0x0F]` with the mask bit (0x8000) set, so that black stays opaque. CLUTs are cached in VRAM, keyed by the 16 bytes, with a slot count sized by the `gpu_cluts` peak (below). No call site changes, and `hud.c`'s `memcpy` needs nothing, because the backend reads the palette at draw time.
- **`fillp`: pattern texture.** A `rectfill` or `circfill` span drawn with a non-zero pattern becomes a textured sprite. Its texture is the 4×4 pattern tiled into an 8×8 4-bit texture, with texel 0 for a clear bit and 1 for a set bit, repeated by the GPU texture window (whose unit is 8 pixels). Its CLUT has entry 0 = `col & 0x0F` and entry 1 = `(col >> 4) & 0x0F`, or 0x0000 when `fillp_transparent` is set. The U/V origin is the world coordinate modulo 8. A pattern slot (texture + CLUT) is keyed by (pattern, colour byte, transparent flag), and the slot count is sized by the `gpu_patterns` peak. A span without a pattern is a plain `TILE`.
- **Rejected: software fallback.** Rasterising patterned or palette-remapped calls into RAM would need the RAM framebuffer uploaded every frame (38,400 bytes at 16 bits). That is the framebuffer copy the plan's GPU rule replaces. Sending each patterned pixel as a 1×1 tile would cost one primitive per pixel on the background grid.
- **What the split must therefore allow:** every backend entry gets the `const p8_graphics_t *` (camera, `fillp_pattern`, `fillp_transparent`) and the `p8_vram_t *` (draw palette; the screen for the software backend), and the colour argument as the game passed it, high nibble included. The front end resolves nothing to pixels.

### Decision (b): whether the GPU primitive buffers fit

- **What is counted.** A pure cost model (below) gives the bytes and primitives of each draw call. A meter adds them up per frame. On PC, `make state` runs the meter over every replay it already runs: the worst-case games and every e2e. This is "counted headless on PC over the e2e and worst-case games".
- **Peak plus margin.** One buffer holds one frame. `GPU_FRAME_BUFFER_BYTES` is set to the measured peak + 1/8, rounded up to 1 KiB. The `zone gpu_frame <current> <peak> <limit>` line makes `make state` fail when 9 × peak > 8 × limit, as for any zone. Two buffers are reserved: the CPU fills one while the GPU draws the other.
- **Which room.** `make state`'s existing room is the bytes left under `ZONE_BUDGET_BYTES` (plan 10's rule cites 1,980; step 1 measured 1,389 on MIPS). It is the zones' own margin, and it is smaller than one frame of the background map alone (up to 21 × 16 visible cells × 20 bytes ≈ 6.7 KB). Plan 10's rule places the buffers in the half plan 7 freed, which is outside `ZONE_BUDGET_BYTES`. So the buffers are a static reservation of the GPU backend, counted in no zone. `make state` prints their total (2 × `GPU_FRAME_BUFFER_BYTES`) on the line after the zone room, so both are measured at every run. The fit is checked against the room that actually bounds them: the MIPS RAM of the GPU PS-EXE that holds them. That check is the `ram-room` condition of `make psx-check`, and its figures come from the toolchain's `size` at that moment.
- **`ram-room` formula.** The condition passes when 65,536 (kernel area below the load address) + text + data + bss (the zones, `p8_vram_t` and the two buffers included) + `PSX_STACK_RESERVE` ≤ 2,097,152. `PSX_STACK_RESERVE` is 64 KiB, a make variable that plan 11 re-measures. The condition prints the bytes left. If it fails, step 3 stops with a report: the buffers are not shrunk below their measured peak to make them fit.
- **The caches too.** `cap gpu_cluts - <peak> <limit>` and `cap gpu_patterns - <peak> <limit>` give the per-frame peaks of distinct draw-palette contents used by `spr` and of distinct pattern keys. `GPU_CLUT_SLOTS` and `GPU_PATTERN_SLOTS` are sized at peak + 1/8, rounded up to a whole slot, and judged by the same margin rule.

### Modules

- **Drawing front end (`src/p8/`, core, modified).** The public API keeps its names and signatures. It holds the state calls unchanged, and it holds the `map` loop and `spr_default`. Each draw call forwards to the active backend: `cls`, `pset`, `line`, `rect`, `rectfill`, `circ`, `circfill`, `spr`, `print`. The active backend is a file-scope pointer, and NULL means the software backend. It is reached through `p8_draw_backend_set` (it returns the previous backend; passing NULL restores the software default) and `p8_draw_backend_get`. It is process-wide because `cls` has no graphics argument and the game must not change.
- **Backend interface (`src/p8/`, core, new header).** `p8_draw_backend_t` is a const struct of nine function pointers, one per draw call. Each takes the graphics state and the VRAM struct (only the VRAM struct for `cls`) and the call's own arguments, as the API received them, in world coordinates. A backend applies the camera itself. The header states the contract: a backend reads the state and never writes it, and the tables are static and never freed.
- **Software backend (`src/p8/`, core, new file).** It is today's rasterising code moved verbatim, behind a const table returned by `p8_gfx_software_backend()`. Its internal plotting goes through a file-local pixel write, never through the public `pset`. The same applies to `spr` reading the sheet. The `fillp_color` comment about the dump assert moves with it. It is added to `ENGINE_SRCS`, so it reaches `CORE_SRCS` and the PSX build. Every Makefile link line that lists the front end also gets this file.
- **Cost model (`src/p8/`, core, new, pure).** It takes a draw call, the graphics state and the VRAM struct, and returns the GPU primitives and bytes it may emit. It is an upper bound for step 4. Bytes are the PSn00bSDK packet sizes on MIPS (tag included). The mapping:

  | call | cost when its screen bounding box meets 0..159 × 0..119 (else 0) |
  |---|---|
  | `cls` | 1 `FILL` |
  | `pset` | 1 `TILE_1` |
  | `line` | 1 `LINE_F2` |
  | `rect` | 4 `TILE` (one per edge) |
  | `rectfill` | 1 `TILE`, or with a pattern 1 `SPRT` |
  | `circ` | 8 `TILE_1` per iteration of the midpoint loop |
  | `circfill` | 4 spans per iteration: `TILE` each, or `SPRT` each with a pattern |
  | `spr` | 1 `SPRT` (w×h cells in one sprite) |
  | `print` | 1 `SPRT` per glyph with at least one lit row |

  A pair `DR_TPAGE` + `DR_TWIN` is added whenever the texture-window state needed by a textured call (none for `spr`/`print`, the pattern's window for a patterned fill) differs from the previous textured call of the frame. A fixed per-frame overhead covers the draw-environment setup and the ×2 upscale quad. The per-kind byte sizes and the overhead are named constants. On PSX the GPU backend checks each one against the `sizeof` of the SDK struct with a compile-time array-size check, as `sprite_sheet.c` checks its table. The model is integer-only and allocation-free.
- **Meter (`src/p8/`, core, new).** It is a backend that wraps another one. Every call adds its cost to the current frame, then forwards it. It also counts the distinct draw-palette contents seen by `spr` and the distinct pattern keys of the frame, in fixed tables of `GPU_CLUT_SLOTS` × 2 and `GPU_PATTERN_SLOTS` × 2 entries; a count past that is clamped and still reported as a peak above the limit. Its interface:
  - install, which wraps the backend active at that moment;
  - frame end, which folds the frame into the peaks and resets the frame;
  - getters for the last frame and for the peaks (bytes, primitives, CLUTs, patterns).

  Drivers call frame end after `cart_draw`. The sizes header next to it holds `GPU_FRAME_BUFFER_BYTES`, `GPU_CLUT_SLOTS` and `GPU_PATTERN_SLOTS`, as `zone_sizes.h` does for the zones.
- **State text (`src/devtools/`, modified).** After the existing zone lines it adds `zone gpu_frame <last> <peak> <GPU_FRAME_BUFFER_BYTES>`, and among the caps `cap gpu_cluts - <peak> <limit>` and `cap gpu_patterns - <peak> <limit>`. They start with `zone`/`cap`, so step 1's game-line comparison already excludes them and the measurement file already collects them.
- **`make state` (modified).** The peaks script needs no new parsing, because the new lines have the five fields it judges. After `N bytes left under ZONE_BUDGET_BYTES` it prints `state: GPU primitive buffers 2 x <B> = <total> bytes, outside ZONE_BUDGET_BYTES`, with B read from the `zone gpu_frame` limit.
- **PC replay (`src/cmd/`, modified).** When `STATE=` is set, it installs the meter around the software backend before `cart_load` and calls frame end after each `cart_draw`. Without `STATE=`, it changes nothing.
- **GPU backend skeleton (`src/platform/`, PSX only, new).** A PSn00bSDK module, `psx_gpu_*`. It does:
  - init: GPU reset, NTSC 320×240 display, two display buffers, the 160×120 draw area, and the VRAM layout below, with the packed sprite sheet uploaded from the H-2-1 accessor;
  - the backend table: `cls` implemented as a `FILL` of the draw area in the frame's buffer; the eight other entries draw nothing, each with a `TODO` naming step 4;
  - frame begin and frame end: append primitives in call order (painter's order, no depth sort), end the frame with the upscale quad, wait for the previous frame's `DrawSync`, submit, swap buffers. There is no `VSync` wait in replay mode;
  - a bump allocator over the frame's static buffer: overflow is `p8_fatal` naming the frame and the bytes asked;
  - the per-frame check that real bytes ≤ metered bytes.

  It uses no floating point and no `malloc`.
- **VRAM layout (one module of named constants, host-compilable).**

  | region | position (x, y in 16-bit units) | notes |
  |---|---|---|
  | display buffers | (0, 0) and (0, 240), 320×240 each | |
  | draw area | (320, 0), 160×120 | 15-bit texture page (320, 0) for the upscale |
  | sprite sheet | (512, 0), 32×128 | 4-bit page (512, 0), texels u 0..127, v 0..127 |
  | font (step 4) | (512, 128) | same page, v 128..255 |
  | pattern slots | (544, 0) onward, 2×8 each | same page, u 128..255 |
  | CLUT region | (640, 0) onward, 16×1 each | draw-palette cache, 16 font colours, pattern CLUTs |

  Step 4 may move a region if a family needs it, by changing this module only.
- **GPU replay driver (`src/cmd/`, PSX only, new).** It is the software PSX replay's loop and TTY protocol (same `psx-replay:` lines, then `psx-state: `), with three changes: it installs the GPU backend, wrapped by the meter; it calls the GPU frame begin/end around `cart_draw` and the meter's frame end after it; and it prints `psx-replay: backend gpu`. It prints no `psx-digest:` lines, because the RAM screen is not drawn. It still loads the RAM sheet (H-2-3), so the game sees the same memory. The software PSX replay also installs the meter, so that its measurement lines are comparable.
- **PSX build.** A third CMake target, `psx-gpu-replay`, with its own source list: the core, the GPU backend, the VRAM layout, the shared state formatter, the PSX platform files and the generated replay. `make psx-build` builds and sizes it too.
- **`make psx-check` (H-2-5).** It boots the GPU PS-EXE until it exits, in its own output subdirectory, with `PSX_GPU_CEILING`/`PSX_GPU_TIMEOUT` set from the first measured run plus half again. It adds four conditions:
  - `gpu-replay-done`: exit 0, its `done` line carries the PC's frame count, and there is no `stopped p8_fatal`;
  - `gpu-state-game-lines`: step 1's comparison script on the GPU TTY;
  - `gpu-cost`: the `zone gpu_frame`, `cap gpu_cluts` and `cap gpu_patterns` lines of the GPU run equal the PC reference's, which comes from the same `STATE=` run;
  - `ram-room`: the formula above, on the GPU ELF.

  The `no-json` condition (H-2-4) also covers the GPU ELF. The measurement file gains the GPU ELF's sizes, its vsyncs, its meter lines and the room. Existing conditions are unchanged.
- **Not in `make test-all`.** Nothing PSX enters the suite. `make full-game`, `make sanitize-net` and `make state` with its 1/8 margin stay green on every commit.
- **Docs.** `docs/psx-game.md` gains:
  - the GPU replay, the backend split and both decisions;
  - the VRAM layout and the new conditions;
  - under "Still to do", the families of step 4.

  The Makefile legend describes the new `make state` lines, the GPU PS-EXE in `psx-build` and the new `psx-check` conditions. Comments follow CLAUDE.md: contracts only (who owns a table, what NULL means for the setter).

## Implementation Chunks

1. **The split on PC.** Add the backend interface and the software backend moved verbatim, and route the front end through them. Add the setter and getter. Update the Makefile link lines and the recording-backend unit test. Done when `make test-all` is green with every `*.sha256` unchanged, `make state` gives identical output, `make e2e` timing is unchanged (best of several runs before and after), and `make raylib` still links if the vendor setup is present.
2. **Cost model, meter and `make state`.** Add the pure cost model and its unit test, then the meter and its unit test. Add the state lines and the `STATE=` hook in the PC replay. Run `make state` once with provisional limits, set `GPU_FRAME_BUFFER_BYTES`, `GPU_CLUT_SLOTS` and `GPU_PATTERN_SLOTS` at peak + 1/8, then run `make state` green. Record the peaks and the replays holding them in the step note.
3. **PSX GPU skeleton.** First check that `make psx-check` is green (H-2-8). Then add the VRAM layout module (host-tested), the GPU backend skeleton, the GPU replay driver, the CMake target, the four `psx-check` conditions, the ceiling and the measurements. Then the docs and the Makefile legend.

## Testing Decisions

- A good test drives the public API and checks what leaves it: the entry a recording backend received with its arguments, the cost a pure function returns, the lines a formatter writes, the result lines and exit code of a script. It never checks how a backend stores its tables.
- **Backend routing (new `test_draw_backend`, in `make test`).** A recording backend is installed through the setter and restored at the end. It checks:
  - each of the nine draw calls reaches its own entry once, with the same arguments and the same graphics and VRAM pointers;
  - `map` gives one `spr` per non-empty cell at the right screen position, and none for cell 0;
  - `spr_default` gives a 1×1 `spr`;
  - the state calls reach no entry;
  - a `memcpy` into the draw palette is visible to the next `spr` entry;
  - `set` returns the previous backend, and NULL restores software (a `pset` then lands in `screen`).
- **Software backend.** `test_gfx` and every unit test that reads `screen` stay as they are and pass unchanged; they now cover the software backend through the default route. `make e2e` is the net, as CLAUDE.md makes the replays the source of truth.
- **Cost model (new `test_draw_cost`, in `make test`).** It checks:
  - each call kind's cost against the table;
  - an off-screen call costs 0, judged after the camera;
  - a call straddling the edge costs in full;
  - `circ`/`circfill` iteration counts for radii 0, 1, 7 and 9;
  - `print` skips spaces;
  - a patterned fill costs a `SPRT` and adds the window pair once for consecutive calls with the same pattern, and again after a change.
- **Meter (new `test_draw_meter`, in `make test`).** It checks:
  - it forwards every call to the wrapped backend (a recording one);
  - frame end folds the frame into the peaks and resets the frame;
  - distinct draw palettes and pattern keys are counted per frame, repeats only once;
  - clamping past the table still reports a peak above the limit.
- **VRAM layout (new `test_psx_vram_layout`, in `make test`, host-compiled).** It checks:
  - no two regions overlap;
  - texture pages and CLUTs sit on their hardware alignments (pages on 64-unit x and 256-line y, CLUTs on 16-unit x);
  - the sheet, font and pattern slots share one 4-bit page;
  - the slot counts fit their regions.
- **State text and peaks.** `test_state_format` and `test_state_peaks` gain the new lines: their format, their exclusion by the game-line comparison (`test_psx_state_compare` gains one case), and a `zone gpu_frame` peak over its margin failing `make state`.
- **PSX side.** `make psx-check`'s four new conditions are the integration tests of the GPU backend skeleton and the driver. No unit test runs the toolchain or the emulator. The compile-time size checks run in every PSX build.
- `make test` and `make e2e` must not get slower. The new tests are millisecond-scale. The replay installs the meter only under `STATE=`, and the front end adds one indirect call per draw call. Chunk 1 records the `make e2e` timing before and after.
- Prior art: `test_gfx` (API through `screen`), `test_state_format` and `test_state_peaks` (state lines and margin), `test_psx_state_compare` and `test_frames_check` (scripts driven on crafted files), `test_sprite_sheet` (a compile-time size check beside a unit test).

## Out of Scope

- Implementing the GPU families (`pset`, `line`, `rect`, `rectfill`, `circ`, `circfill`, `spr`, `map`, `print`, and patterned and palette-remapped drawing), the font texture, the CLUT and pattern caches' contents, capture-at-frame and the VRAM comparison against the PC framebuffer: step 4. Step 3 fixes only their decisions, their layout and their budget.
- Changing any file of `src/game/` or `src/engine/`, or any API signature.
- Removing the RAM framebuffer or the RAM sprite sheet from `p8_vram_t` on the GPU build, or any other memory saving (plan 11).
- Tightening the cost model below what step 4 will need: step 4 may lower a family's cost and re-measure, and it never raises one without re-running `make state`.
- Embedded assets (step 2), the pad driver and the controller-driven title (step 5), the `.bin/.cue` (step 6).
- `make psx-full-game-check` for the GPU PS-EXE: it stays pending for plan 11 as step 1 recorded. The worst-case games are already counted on PC by `make state`.
- Audio.

## Further Notes

- **Size estimate.** The background map is the single largest known draw. At most 21 × 16 cells are visible, at 20 bytes per `SPRT`, about 6.7 KB. Particles cost one 12-byte `TILE_1` each, and an outlined `print` costs nine sprites per glyph. One frame is therefore expected to need tens of KB, and two buffers well under the ~700 KB that step 1's ELF sizes leave free in 2 MiB. Only the `make state` measurement and `ram-room` decide.
- The ×2 upscale is nearest-neighbour by construction. Whether the emulator's quad sampling is exactly 2×2 per texel is checked by step 4's first capture. If it is not, step 4 may show the draw area through a different display setup without touching this split.
- The meter on MIPS runs the same integer code as on PC. Equal `gpu-cost` lines therefore prove the command stream is identical, not that the model is right. Whether the model is right is guarded per frame by the real ≤ metered check, which bites in step 4.
- The step note should record:
  - the start check's result and both decisions, with a pointer to this PRD;
  - the peaks with the replays holding them, and the three constants;
  - the `make e2e` timing before and after;
  - the GPU ELF sizes, vsyncs, ceiling and RAM room;
  - every corrected step-2 hypothesis.
