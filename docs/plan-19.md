# Plan 19 — Draw the PSX frame at display resolution

Draft: run it through `plan-converge` before executing anything. Follows plan 10 (PSX GPU backend), before plan 11.

## 1. Goal

The PSX disc looks the same under every renderer of an emulator, hardware ones included, as it already does under a software renderer.

Today `src/platform/psx_gpu.c` draws each frame into a 160x120 draw area in VRAM, at (320, 0). The frame then ends on one `POLY_FT4` that reads that area as a 15-bit texture and stretches it x2 onto the 320x240 display buffer. PCSX-Redux's software GPU and, by design, a real console sample that quad exactly. DuckStation's hardware renderer (Metal, macOS) does not. Some texels come out 1 pixel wide and others 3, so glyph outlines have uneven widths, the title logo is broken up and its particles become big squares. The human saw it on the title screen; the Software renderer fixes it.

The fix removes the render-to-texture step: every primitive is drawn straight into the display buffer at x2, so no texture is ever stretched.

## 2. Rules

- The game doesn't change: every `*.sha256` in `tests/e2e/` stays identical, and `make test-all` passes on every commit, no slower.
- The PSX image stays exactly the PC image upscaled x2: 2x2 uniform blocks, palette colours only. The `psx-check` conditions keep proving it (`offpalette 0`, `nonuniform 0`, `differ` against the PC capture).
- Every primitive keeps an integer x2 mapping: no primitive scales a texture. `SPRT` reads pre-scaled textures, `TILE` gets doubled coordinates. No `POLY_FT4` is left in a frame.
- The cost model (`p8_draw_cost`, `p8_draw_meter`, `gpu_sizes.h`) is updated in the same step as the primitive it counts; `GPU_FRAME_BUFFER_BYTES` is re-sized by the existing rule (measured peak + 1/8).
- The 14 accepted captures in `psx/gpu_capture_references.txt` are invalidated by design. They go back to human acceptance once, at the end, never one by one along the way.

## 3. Steps

1. **Reproduce the bug headless.** Find an emulator renderer the agent can run without a window that shows the bug: PCSX-Redux's OpenGL GPU (without `-softgpu`) on a software GL (Mesa llvmpipe), or another headless emulator with a hardware-style renderer. Capture the title screen with it, and add a check that fails on today's disc: an image that is not the PC image upscaled x2. If no such renderer can be run, write it down, and the proof falls to the human with DuckStation (step 6).
2. **Textures at x2 in VRAM.** The packed sprite sheet (128x128 4-bit), the font's glyph cells (4x6 per code) and the fill-pattern tiles get uploaded upscaled x2. Rework the VRAM layout (`psx_vram_layout`) without the 160x120 draw area. Unit tests on the upscaled uploads.
3. **Primitives at x2.** In order, each with its cost-model change and tests:
   - `cls`/`rectfill`/`rect`/circle spans: `TILE` with doubled coordinates.
   - `pset` and `circ`: a 2x2 `TILE` instead of `TILE_1`.
   - `spr`/`print`: `SPRT` with doubled position, size and UV.
   - Patterned fills: the texture window, at x2.
   - `line`: a `LINE_F2` is 1 pixel thick whatever its coordinates. Draw it from `gfx_software.c`'s pixel list as 2x2 tiles (exact, more bytes), or from two offset `LINE_F2`s if the capture proves them exact. Decide on the `line` capture's `differ`.
4. **Drop the upscale.** Remove the draw area, the end-of-frame black `FILL` and the `POLY_FT4`, together with the 0x0000-transparency workaround they needed. The frame clears its display buffer once, at the start.
5. **Re-measure.** Re-size `GPU_FRAME_BUFFER_BYTES`, the CLUT and pattern slots and the boot ceilings on `make psx-check` and `make psx-full-game-check`. The RAM room must stay positive.
6. **Human acceptance.** `make psx-check` green, then the human compares the 14 captures and re-accepts them with `make psx-gpu-capture-accept`. Then they boot the disc in DuckStation with the hardware renderer (and its internal resolution above 1x) and check the title screen and a game.

## 4. Done when

The title screen and a game look the same in DuckStation with the hardware renderer as with the Software renderer, and `make psx-check` is green with the 14 captures re-accepted.

## 5. Open questions

- Does `line` at x2 stay within its current `differ` ceiling with two `LINE_F2`s, or does it need the pixel list?
- What does VRAM layout and per-frame byte cost look like with x2 textures? Measure before committing to the layout.
- Can step 1 run in this container at all, with no GPU? If not, the plan loses its automated proof and relies on step 6 alone.
