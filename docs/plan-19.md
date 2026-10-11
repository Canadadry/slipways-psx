# Plan 19 — Draw the PSX frame at display resolution

Follows plan 10 (PSX GPU backend), before plan 11.

## 1. Goal

The PSX disc looks the same under every renderer of an emulator, hardware ones included, as it already does under a software renderer.

Today `src/platform/psx_gpu.c` draws each frame into a 160x120 draw area in VRAM, at (320, 0). The frame then ends on one `POLY_FT4` that reads that area as a 15-bit texture and stretches it x2 onto the 320x240 display buffer. PCSX-Redux's software GPU and, by design, a real console sample that quad exactly. DuckStation's hardware renderer (Metal, macOS) does not. Some texels come out 1 pixel wide and others 3, so glyph outlines have uneven widths, the title logo is broken up and its particles become big squares. The human saw it on the title screen; the Software renderer fixes it.

The fix removes the render-to-texture step: every primitive is drawn straight into the display buffer at x2, so no texture is ever stretched.

## 2. Rules

- The game doesn't change: every `*.sha256` in `tests/e2e/` stays identical, and `make test-all` passes on every commit, no slower.
- The PSX image stays exactly the PC image upscaled x2: 2x2 uniform blocks, palette colours only. The `psx-check` conditions keep proving it (`offpalette 0`, `nonuniform 0`, `differ` against the PC capture).
- Every primitive keeps an integer x2 mapping: no primitive scales a texture. `SPRT` reads pre-scaled textures, `TILE` gets doubled coordinates. No `POLY_FT4` is left in a frame.
- The cost model (`p8_draw_cost`, `p8_draw_meter`, `gpu_sizes.h`) is updated in the same step as the primitive it counts; `GPU_FRAME_BUFFER_BYTES` is re-sized by the existing rule (measured peak + 1/8).
- Until the `line` step, `make psx-check` fails by design. Steps 2 and 3 are gated on its `conditions.txt` instead: every condition is `ok` except `gpu-capture-clean` and `gpu-capture-ceilings`, and those two fail only on labels not yet due. From the `line` step on, `make psx-check` exits green.
- The 14 accepted captures in `psx/gpu_capture_references.txt` are invalidated by design. They go back to human acceptance once, at the end, never one by one along the way.

## 3. Steps

1. **Reproduce the bug headless.** Find an emulator renderer the agent can run without a window that shows the bug. Try these candidates, in order: DuckStation's headless regression runner with a hardware renderer and its internal resolution above 1x (the case the human saw), then PCSX-Redux's OpenGL GPU (without `-softgpu`) on a software GL (Mesa llvmpipe). A candidate counts only if its check fails on today's disc, while the same capture and check pass on that disc under the same emulator's software renderer (the control). Its capture is the renderer's output at its internal resolution, not emulated VRAM or a 320x240 readback, and the checks judge blocks of 2 x that scale. A candidate is declared exhausted only once its step note records either a capture taken at that resolution, or the concrete blocker that keeps it from producing one (command and error). Capture the title screen and the frames of `psx/gpu_captures.txt` with it, and add a check that fails on that disc: an image that is not the PC image upscaled x2 (same `offpalette`/`nonuniform` conditions). From step 2 on, each family's labels must also pass under that renderer. Once both candidates are exhausted, write it down and go to step 2; the proof then falls to the human with DuckStation (step 6).
2. **Draw into the display buffer.** First, lay out the 1024x512 VRAM at x2 on paper: the two display buffers, the x2 sprite sheet, the x2 font, the x2 pattern slots and the CLUTs. Write into this step's note how many texture pages the textured calls use and whether the cost model must count a page switch. Also record the frame-buffer peak and RAM room each `line` option would cost on `make state` (PC only). The x2 sheet alone fills a whole 4-bit page, so the single-page rule of today's `frame_begin` does not hold. Then the frame draws straight into its display buffer and clears it once, at the start. Remove the 160x120 draw area (and its place in `psx_vram_layout`), the end-of-frame black `FILL`, the `POLY_FT4` and the 0x0000-transparency workaround they needed. In the same step, `cls`/`rectfill`/`rect`/circle spans become `TILE` with doubled coordinates. In that step's commit, set all 14 entries of `psx/gpu_capture_references.txt` to pending (`-`), so that `psx-check` can still gate each step. Proof: the capture labels of this family pass `offpalette 0`, `nonuniform 0` and their `differ` ceiling. A label is proven at the step that converts the last family in its capture mask (`psx/gpu_captures.txt`): `circfill 263` also draws `line`, so it is proven at the `line` step, and the `all` labels at step 4. Each step names the labels it must turn green. Any other label may still fail.
3. **Primitives at x2, one family per step.** Each family comes with its cost-model change, its unit tests, and its capture labels passing the same conditions. It also re-sizes `GPU_FRAME_BUFFER_BYTES` (and the CLUT and pattern slots) by the existing rule in its own commit, with `make state` passing and `ram-room` staying positive. Step 2 does the same for its family. In this order:
   - `pset` and `circ`: a 2x2 `TILE` instead of `TILE_1`.
   - `spr`/`print`: `SPRT` with doubled position, size and UV, together with uploading the packed sprite sheet (128x128 4-bit) and the font's glyph cells (4x6 per code) upscaled x2, in the layout step 2 settled.
   - Patterned fills: the texture window at x2, together with uploading the fill-pattern tiles at x2.
   - `line`: a `LINE_F2` is 1 pixel thick whatever its coordinates. Draw it either from `gfx_software.c`'s pixel list as 2x2 tiles (exact, more bytes) or from two offset `LINE_F2`s if the capture proves them exact. Decide from the `line` capture's `differ` and from the frame-buffer bytes and RAM room each option costs. The two-`LINE_F2` option is admissible only if the step 1 reproducer exists and the `line` labels pass under it; otherwise take the pixel list. If the pixel list does not fit in RAM, merge its pixel runs into wider `TILE`s, which is still exact; if that does not fit either, record the blocker and hand it to step 6.
4. **No stretch left.** Check that no `POLY_FT4` or other scaled texture is left in a frame. Every capture label passes, and only the 14 pending references remain. If step 1 produced a headless reproducer, it now passes on the fixed disc. It stays a separate target, outside `make test-all`.
5. **Re-measure.** A final check of the sizes the family steps set, and a re-size of the boot ceilings, on `make psx-check`. `make psx-full-game-check` is not run: it links no GPU backend, and its red (the emulator abort) belongs to plan 11. The RAM room must stay positive.
6. **Human acceptance.** `make psx-check` green, then the human compares the 14 captures and re-accepts them with `make psx-gpu-capture-accept`. Then they boot the disc in DuckStation with the hardware renderer (and its internal resolution above 1x) and check the title screen and a game.

## 4. Done when

The title screen and a game look the same in DuckStation with the hardware renderer as with the Software renderer, and `make psx-check` is green with the 14 captures re-accepted. The step 1 reproducer passes, if step 1 produced one.

## 5. Open questions

- Does `line` at x2 stay within its current `differ` ceiling with two `LINE_F2`s, or does it need the pixel list?
- Can step 1 run in this container at all, with no GPU? No: both candidates are exhausted, so the proof relies on step 6 alone (see § 6, Step 1).

## 6. Step notes

### Step 1

Probed 2026-10-11 in this container (no GPU, no `/dev/dri`, no X server; Mesa llvmpipe gives a surfaceless GL 4.5 core context).

- **DuckStation regression runner: exhausted.** Blocker: license and owner instruction, not code. Releases ship only the Qt AppImage, so `duckstation-regtest` would need a build from source (`curl https://api.github.com/repos/stenzek/duckstation/releases/latest` is now refused by this session's proxy with 403, so the asset list was not re-read; the PRD recorded it). The root and `src/duckstation-regtest/CMakeLists.txt` read: "SPDX-License-Identifier: CC-BY-NC-ND-4.0 + Packaging Restriction … you may not use this file to create packages or build recipes without explicit permission from the copyright holder". The repository's `CLAUDE.md` says the codebase "is not authorized for use with AI coding assistants" and tells them not to operate on it. Not cloned, not built.
- **PCSX-Redux OpenGL GPU, `-cli`: exhausted.** `env -u DISPLAY -u WAYLAND_DISPLAY HOME=<tmp> timeout 60 third_party/pcsx-redux/pcsx-redux -cli -stdout -openglgpu -interpreter -run -iso tmp/psx/build/slipways.cue -dofile <120-vsync screenshot lua>` exits 139 (SIGSEGV) with empty stderr, no capture. gdb: `#0 0x0000000000000000`, `#1 OpenGL::VertexBuffer::create` (`src/support/opengl.h:345`), `#3 OpenGL_GPU::initBackend` (`src/core/OpenGL_GPU/gpu_opengl.cc:84`), `#4 pcsxMain` (`src/main/main.cc:378`): the text UI never creates a GL context or loads GL entry points.
- **PCSX-Redux OpenGL GPU, GUI front end: exhausted.** The same command without `-cli -stdout` exits 255 with `Failed to initialize GLFW` on stderr, no capture.
- **PCSX-Redux, structural:** its OpenGL GPU allocates VRAM at native 1024x512 (`vramWidth`/`vramHeight` in `gpu_opengl.h`, MSAA the only option) and has no internal-resolution scale, so even with a context it could not render the case the human saw.

Consequences: no headless reproducer exists. Step 3's two-`LINE_F2` option for `line` is not admissible (`line` takes the pixel list, or merged `TILE` runs). Step 4's reproducer clause is void. The bug's proof falls to the human in step 6.
