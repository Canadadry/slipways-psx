---
title: "GPU primitives family by family, each captured in the emulator and compared with the PC framebuffer"
description: "The PSX GPU backend draws only cls, and nothing yet captures a GPU frame in the emulator and compares it with the PC software frame, so no draw family can be implemented or judged."
status: needs-triage
---

## Problem Statement

After step 3, the drawing API reaches a backend, the PC draws through the software backend, and the PSX has a GPU backend skeleton. That skeleton draws `cls` and nothing else. The other eight entries (`pset`, `line`, `rect`, `rectfill`, `circ`, `circfill`, `spr`, `print`; `map` reaches `spr` through the front end) accept their call and draw nothing. Patterned fills (`fillp`) and palette-remapped sprites (`palt`, the draw palette) are decided on paper only.

Plan 10's step 4 asks for each family to be implemented and checked by a VRAM capture in the emulator, compared with the PC software framebuffer of the same frame. Today, several pieces are missing:

- The plan-9 capture (`tools/psx_capture.lua`) is taken at a fixed vsync count, in the plain boot mode. The GPU replay runs in until-exit mode with no `VSync` wait. Nothing ties a vsync to a replay frame, and until-exit mode writes no capture at all.
- A full frame mixes every family. A missing or wrong family hides inside the others, and a frame compared while only some families exist shows mostly "missing" pixels.
- Nothing turns a capture (320×240 RGB555, upscaled ×2) back into PICO-8 colour indices, and nothing measures how far it is from the PC frame.
- The plan accepts a divergence of a few pixels (circles, lines), to be judged by a human when it appears. No agent can judge it. The step must therefore produce, for each family, a capture and a comparison a human can review, and assert only what an agent can prove: the captures exist, are well-formed and are deterministic, and the reports are written. References are frozen only after the human's review, which is outside this step.

## Solution

Each family is implemented in the PSX GPU backend, one at a time. The primitive choices follow step 3's cost model and its decision (a). Each family comes with at least one **family capture**: one frame of the PSX check replay (`pad_slipway`), drawn with a **family mask** that lets through `cls` and the family under test only. The same frame and mask are drawn on the PC by the software backend. A comparator turns both into 160×120 colour-index images and writes a diff report and a side-by-side image for the human.

The pieces:

- **A family filter**, a backend wrapper shared by PC and PSX. It drops the draw calls whose family is outside the mask, and it counts on-screen calls per family. It sits under the meter, so step 3's cost lines stay byte-identical.
- **A capture list**, one committed text file. It names, for each family capture, its family, its frame and its mask. The PC replay reads it to write the reference frames and the per-family call counts. It also converts the list into a numeric capture script for the emulator.
- **A frame-locked capture in until-exit mode.** The GPU replay and a new Lua script do a scratchpad handshake, as step 1's pad probe does. The Lua hands the program the next frame and its mask. At that frame, the program draws with the mask, settles the GPU, shows the frame and waits. The Lua takes the plan-9 capture (`psx-vram` header + `takeScreenShot()` bytes), then releases the program.
- **A comparator.** A pure module plus a host tool. It checks the capture's format and the exact ×2 upscale. It maps each pixel back to a PICO-8 colour, and counts the differing pixels by distance to the nearest PC pixel of the same colour. It writes one report line, a text diff map and a PPM image (PC | PSX | diff) per capture.
- **`make psx-check` conditions that do not require a match.** The list is valid and each family's frame really draws that family. Every capture exists and is well-formed. All family captures come from one boot. A second boot gives byte-identical captures. The upscale is exact and every pixel is a palette colour. A report exists for every capture. Until a capture is accepted, it is guarded mechanically:
  - its differing-pixel count must not exceed the ceiling recorded for it in a committed file, `psx/gpu_capture_ceilings.txt`, which the step note quotes;
  - a count that rises fails the check;
  - such a capture is listed as **pending human acceptance**.

  A capture with a committed reference must match it byte for byte. No reference is written in this step.
- **One command for the human.** `make psx-gpu-capture-accept LABEL=<label>` is ready to run. It turns the accepted captures of a label into committed references.

The step is done when every family has its capture(s), report(s) and recorded ceiling, `make psx-check` is green, and the step note lists each family's capture and comparison (with its ceiling) as pending human acceptance. The step note also gives `make psx-check`'s run time.

## Step 3 as committed

Step 3 is done (see the "Step 1.3" note of `docs/plan-10.md` and `docs/psx-game.md`). This PRD builds on the facts below. The H-3-k labels are kept so that the references elsewhere in this PRD stay traceable.

- **H-3-1. Backend interface.** `src/p8/draw_backend.h` defines `p8_draw_backend_t`, nine entries: `cls`, `pset`, `line`, `rect`, `rectfill`, `circ`, `circfill`, `spr`, `print`.
  - Each entry takes the graphics state and the VRAM struct (`cls` takes only the VRAM struct) and the API's arguments, in world coordinates, with the full colour byte.
  - `p8_draw_backend_set` returns the previous backend (never NULL); NULL installs software. `p8_draw_backend_get` never returns NULL.
  - `map` and `spr_default` stay in the front end and reach `spr`.
- **H-3-2. Software backend.** `src/p8/gfx_software.c`, today's rasteriser moved unchanged, is the default. `p8_gfx_software_backend()` returns its table.
- **H-3-3. Cost model and meter.**
  - `src/p8/draw_cost.{h,c}` holds the PSn00bSDK 0.24 sizes as `P8_GPU_*_BYTES`: `FILL` 16, `TILE_1` 12, `TILE` 16, `LINE_F2` 16, `SPRT` 20, `DR_TPAGE` 8, `DR_TWIN` 8, `DR_ENV` 36, `POLY_FT4` 40.
  - It has one `p8_draw_cost_<call>` function per entry, which take no VRAM argument and cost 0 off screen. `rectfill`, `circfill`, `spr` and `print` also take a `p8_draw_cost_window_t *` that tracks the texture window. `p8_draw_cost_frame_begin` resets it and returns the frame overhead. `p8_draw_cost_pattern_key(gfx, col)` keys a pattern slot.
  - The primitive mapping is the step-3 table: `pset` → `TILE_1`, `line` → `LINE_F2`, `rect` → 4 `TILE`, `rectfill` → `TILE` (`SPRT` with a pattern), `circ` → 8 `TILE_1` per midpoint iteration, `circfill` → 4 spans per iteration, `spr` → 1 `SPRT`, `print` → 1 `SPRT` per lit glyph, plus `DR_TPAGE` + `DR_TWIN` when the window changes.
  - `src/p8/draw_meter.{h,c}` has `p8_draw_meter_install()`, which wraps the active backend and returns it, `p8_draw_meter_frame_end()`, `p8_draw_meter_last_frame()` and `p8_draw_meter_peak()` (bytes, primitives, cluts, patterns). The meter counts the CLUTs of culled `spr` calls too, so it can only over-count.
- **H-3-4. GPU backend skeleton** (`src/platform/psx_gpu.{h,c}`).
  - Its API: `psx_gpu_init()` (after `ResetGraph`, which stays in the driver's `main`), `psx_gpu_backend()`, `psx_gpu_frame_begin(int frame)` and `psx_gpu_frame_end(unsigned long metered_bytes)`.
  - `frame_begin` targets the draw area, sets the 4-bit texture page with dithering off (`setDrawTPage(…, 0, 0, …)`) and sets no texture window.
  - `frame_end` appends the ×2 upscale (`POLY_FT4`, raw texture through `setShadeTex`, 15-bit page) into the current display buffer. It then stops with `p8_fatal` if the real bytes exceed the metered bytes, calls `DrawSync(0)`, shows the *other* display buffer (the previous frame), submits this frame and swaps. There is no `VSync` wait. After `frame_end(N)`, the screen therefore shows frame N−1, and frame N is still being drawn.
  - The two primitive buffers are static (`GPU_FRAME_BUFFER_BYTES` each), with a bump allocator that stops with `p8_fatal` on overflow.
  - Only `cls` is implemented (a `FILL`). The other eight entries draw nothing.
- **H-3-5. VRAM layout and sizes.** `src/platform/psx_vram_layout.{h,c}` is host-compiled and tested by `test_psx_vram_layout`. All units are 16-bit VRAM units.

  | region | rectangle |
  |---|---|
  | `PSX_VRAM_DISPLAY_0` | (0, 0, 320, 240) |
  | `PSX_VRAM_DISPLAY_1` | (0, 240, 320, 240) |
  | `PSX_VRAM_DRAW_AREA` | (320, 0, 160, 120) |
  | `PSX_VRAM_SPRITE_SHEET` | (512, 0, 32, 128) |
  | `PSX_VRAM_FONT` | (512, 128, 32, 128) |
  | `PSX_VRAM_PATTERNS` | (544, 0, 32, 256), 2×8 units per slot |
  | `PSX_VRAM_CLUTS` | (640, 0, 16, 64), one 16×1 CLUT per line |

  - The CLUT lines are: palette slots 0..5 (`psx_vram_layout_palette_clut`), font colours 6..21 (`psx_vram_layout_font_clut`), pattern CLUTs 22..25 (`psx_vram_layout_pattern_clut`). The pattern textures are reached through `psx_vram_layout_pattern_slot`. One 4-bit texture page at (512, 0) holds the sheet, the font and the patterns (`psx_vram_layout_texture_page`).
  - The sizes live in `src/p8/gpu_sizes.h`, not in the layout module: `GPU_FRAME_BUFFER_BYTES` 62,464, `GPU_CLUT_SLOTS` 6 and `GPU_PATTERN_SLOTS` 4. They are measured peaks plus 1/8: 54,700 bytes a frame (`pad_bankrupt`), 5 CLUTs and 3 patterns (`full_game`).
  - **No function gives a palette index's RGB555 yet.** Step 4 adds it in chunk 1 (below), because the comparator needs it before any family uses it.
- **H-3-6. GPU replay driver** (`src/cmd/psx_gpu_replay.c`, PS-EXE `psx-gpu-replay`, built by `make psx-build`).
  - It keeps `psx_replay.c`'s TTY protocol and adds `psx-replay: backend gpu`. It prints no digest lines.
  - Its `main` calls `ResetGraph(0)`, loads the RAM sprite sheet and calls `psx_gpu_init()`.
  - It then sets the GPU backend, installs the meter around it, calls `psx_gpu_frame_begin(0)` (before `cart_load`, which may draw), then `cart_load` and `cart_init`.
  - Each frame runs: `cart_update60`, `cart_draw`, `p8_draw_meter_frame_end()`, `psx_gpu_frame_end(p8_draw_meter_last_frame().bytes)`, then `psx_gpu_frame_begin(frame + 1)`.
  - The software PSX replay also installs the meter.
- **H-3-7. `make psx-check`.**
  - It has fourteen conditions. The step-3 ones are `gpu-replay-done`, `gpu-state-game-lines`, `gpu-cost` and `ram-room`. In the step-3 run, `ram-room` left 508,060 bytes and `gpu-cost` showed `zone gpu_frame 14468 32060 62464`.
  - `tools/psx_check.sh` takes 19 arguments (`<gpu-exe>`, `<gpu-ceiling>`, `<gpu-timeout>` and `<stack-reserve>` among them). It checks every condition before concluding and writes `measurements.txt` with the GPU lines behind `gpu `.
  - The GPU replay is booted until it exits with `PSX_GPU_CEILING` 861 vsyncs and `PSX_GPU_TIMEOUT` 17 s; one run takes 553 vsyncs. The full target takes about 60 s.
- **H-3-8. PC reference run.** `make psx-check` writes the PC references with one `tmp/replay` run under `STATE=` and `DIGEST=`. The replay installs the meter only under `STATE=`, and calls `p8_draw_meter_frame_end()` after each `cart_draw`.
- **H-3-9. Font tables.** The glyph tables are `static const` arrays in `src/p8/gfx_software.c` (`FONT_ROWS_*`, selected per character). They are not reachable from `src/platform/`.
- **H-3-10. Start gate.** `make psx-check` was green at the end of step 3. Step 4 still re-runs it first (below).

## User Stories

1. As the plan-pilot run, I want step 4 to start by checking that `make psx-check` is green (H-3-10), so that no family is built on a broken step-3 pipeline.
2. As the step-4 implementer, I want the families implemented one at a time, each in its own commit with its own capture, so that a regression points at one family.
3. As a developer, I want a family filter that drops the calls outside a mask and passes the rest unchanged, so that one family can be drawn alone on both machines.
4. As a developer, I want the filter to count, per family, the calls of the current frame whose screen bounding box meets the screen, before it applies the mask, so that a capture's frame can be proven to draw its family.
5. As a developer, I want `cls` always in every mask, so that a filtered frame starts from the same cleared draw area on both machines.
6. As a developer, I want a `pattern` bit in the mask, so that patterned `rectfill`/`circfill` calls are drawn only in the captures meant for them and plain fills can be reviewed first.
7. As a developer, I want the filter installed under the meter, so that step 3's `gpu-cost` lines and the real ≤ metered check are unchanged by any mask.
8. As a developer, I want the mask applied only on the captured frame and "all families" on every other frame, so that the game, its state and its random draws are unaffected and `gpu-state-game-lines` stays green.
9. As a developer, I want one committed capture list naming the replay and, per capture, a family label, a frame and a mask, so that PC and PSX read the same choice from one place.
10. As a developer, I want the list parser to reject an unknown family, a mask without `cls`, a duplicate or unsorted frame, a frame past the replay's end and a replay other than the one built, each named with its line, so that a bad list never boots the emulator.
11. As a developer, I want each family's capture to include only `cls` and that family (plus `rectfill`/`circfill` for the pattern captures), so that a family's report does not change when a later family is added.
12. As a reviewer, I want a few captures with every family on, so that interactions (sprite transparency over the grid, text over fills, draw order) are reviewed too.
13. As a developer, I want the PC replay, given `CAPTURE_LIST=` and `CAPTURE_DIR=`, to write the filtered software frame of each listed frame, so that the reference is the PC software framebuffer of the same frame and mask.
14. As a developer, I want the PC replay to also write the per-family call counts of each listed frame, so that the check can prove each family's frame draws that family.
15. As a developer, I want the PC replay to write the numeric capture script (`<frame> <mask hex>` per line) for the emulator, so that the Lua never knows the family names or their bit order.
16. As a developer, I want the PC replay to write nothing new and pay nothing when `CAPTURE_LIST=` is unset, so that `make e2e` and `make test` are not slower.
17. As a developer, I want the GPU replay to stop at each scripted frame, after drawing it with its mask, until the emulator has captured it, so that the capture is locked to a replay frame and not to a vsync count.
18. As a developer, I want the handshake on scratchpad words, as the pad probe's, with its addresses named once on each side and checked equal by a static check, so that the program and the Lua never drift apart.
19. As a developer, I want the GPU to be settled (`DrawSync`) and the captured frame's display buffer shown before the program signals it, so that the screenshot holds that frame and nothing half-drawn.
20. As a developer, I want the Lua to wait one more vsync after the signal before taking the screenshot, so that the display switch has happened.
21. As a developer, I want each capture written in the plan-9 format (`psx-vram <w> <h> <depth>` and the raw `takeScreenShot()` bytes), named after its frame, so that every existing capture tool still reads it.
22. As a developer, I want the GPU replay to print `psx-replay: capture <frame> <mask hex>` at each captured frame, so that the TTY shows which frames were captured and in which order.
23. As a developer, I want `psx_boot.sh --until-exit` to accept `--capture-script <file>`, and to fail naming the line on a bad script, so that the capture mode is a boot option like `--pad-script`.
24. As a developer, I want the capture boot to be step 3's GPU boot itself, so that `make psx-check` pays only one more boot (the determinism boot).
25. As a developer, I want a comparator that rejects a capture whose header is not `psx-vram 320 240 16` or whose size does not match, naming both, so that a display-mode change is caught before any pixel is compared.
26. As a developer, I want the comparator to check that every 2×2 block of the capture is uniform and to count the blocks that are not, so that step 3's open point on the exactness of the ×2 upscale is settled by measurement.
27. As a developer, I want each downscaled pixel mapped back to a PICO-8 colour by exact RGB555 equality with the GPU's own colour rule, and the pixels matching none counted as off-palette, so that dithering, blending or a wrong channel order is never mistaken for a geometry divergence.
28. As a reviewer, I want each report line to give the differing pixels (count and per mille of 19,200), how many of them have the PSX colour within 1 px and within 2 px in the PC frame, how many have it further away, and their bounding box, so that "a few pixels off" can be told from "wrong" at a glance.
29. As a reviewer, I want each report line to give the family's on-screen call count at that frame, so that I know how much drawing the numbers are about.
30. As a reviewer, I want one PPM image per capture (PC, PSX and diff side by side, ×2), so that I can see the divergence without any project tool.
31. As a reviewer, I want one text diff map per capture (120 lines of 160 characters, `.` where equal and the PSX colour's hex digit where not), so that an agent and a human can both read and diff it.
32. As the check target, I want `gpu-capture-list` to pass only when the list parses and each family capture's family has at least one on-screen call at its frame on PC, so that a frame choice that draws nothing of its family fails.
33. As the check target, I want `gpu-captures` to pass only when every listed frame has a well-formed capture and its TTY line, in order, so that a missed handshake fails.
34. As the check target, I want `gpu-capture-deterministic` to pass only when a second boot of the same PS-EXE gives byte-identical captures, so that a reference frozen later can be trusted.
35. As the check target, I want `gpu-capture-clean` to pass only when every capture has zero non-uniform blocks and zero off-palette pixels, so that the decoding is proven before any divergence is judged.
36. As the check target, I want `gpu-capture-report` to pass when a report line, a diff map and an image exist for every capture, whatever the numbers say, so that a divergence never fails the check before the human has judged it.
37. As the check target, I want `gpu-capture-ceilings` to fail any capture without a reference whose differing-pixel count is above its committed ceiling, or that has no ceiling at all, naming the recorded and the new count, so that a family cannot get worse unnoticed before the human has reviewed it.
38. As a developer, I want the ceiling of a label written only by `make psx-gpu-capture-ceilings LABEL=<label>` from the last `make psx-check` run, never by hand, and touching that label's lines only, so that a chunk can record its own family's count but never quietly raise another family's.
39. As the check target, I want `gpu-capture-references` to list every capture without a committed reference as `pending human acceptance` and pass, and to fail a capture whose committed reference differs, so that acceptance later only adds references and the target freezes them.
40. As a human, I want a ready-to-run target that writes the references of one family's captures after a green `make psx-check`, so that accepting a family is one command once I have looked at it.
41. As a human, I want no agent to run that acceptance target, so that a reference is never frozen without my review.
42. As a reviewer, I want all family captures taken in one GPU boot, plus one boot only for determinism, and `make psx-check`'s run time recorded in the step note, so that the check's cost stays visible and bounded (plan rule "PSX check cost").
43. As the step-4 implementer, I want `rect` and `rectfill` drawn as `TILE`s with PICO-8's inclusive bounds, so that plain boxes match the PC exactly.
44. As the step-4 implementer, I want `pset` drawn as a `TILE_1`, so that single pixels match exactly.
45. As the step-4 implementer, I want `line` drawn as a `LINE_F2` between the camera-applied endpoints, with no attempt to copy PICO-8's stepping, so that the line divergence the plan expects is measured instead of hidden.
46. As the step-4 implementer, I want `circ` and plain `circfill` drawn with the software's own midpoint loop, emitting `TILE_1`s and spans, so that circles match the PC pixel for pixel when the loop matches.
47. As the step-4 implementer, I want `spr` drawn as one `SPRT` per call from the sprite sheet page, with a CLUT built from the draw palette at call time, so that `palt` and `hud.c`'s `memcpy` palettes work with no game change.
48. As the step-4 implementer, I want the CLUTs of a frame cached by content and uploaded after the previous frame's `DrawSync` and before this frame's submission, so that a slot is never rewritten while a frame still reads it.
49. As the step-4 implementer, I want the font packed at init into the 4-bit font region from the same glyph tables the software backend uses, so that the PSX and PC fonts cannot drift.
50. As the step-4 implementer, I want `print` drawn as one `SPRT` per glyph with a lit row, through one of 16 fixed font CLUTs, so that text costs what step 3 measured.
51. As the step-4 implementer, I want patterned `rectfill`/`circfill` drawn as `SPRT`s textured by an 8×8 pattern tile under a texture window, phased on world coordinates, so that the grid, the menu bar and the transparent disc match the PC phase under a moving camera.
52. As the step-4 implementer, I want each family's real bytes kept under step 3's metered bytes, so that the per-frame check never fires. If a family needs more, the cost model is raised and `make state` is re-run in that family's commit.
53. As a developer, I want the pure builders (CLUT from a draw palette, pattern tile and its CLUT, font texture, font CLUTs) host-compiled and unit-tested, so that only SDK calls stay untested on PC.
54. As a developer, I want every `*.sha256` in `tests/e2e/` and every `make state` game line unchanged, so that the PC is proven untouched.
55. As a developer, I want `make test` and `make e2e` no slower, so that the fast loop stays fast.
56. As a reviewer, I want `docs/psx-game.md` to describe the capture list, the handshake, the comparator's metrics, the new conditions and the acceptance target, and to list under "Still to do" each family pending human acceptance, so that the human knows what to review and how.
57. As a reviewer, I want the step note to list, per family, its frame, mask, report line and image path, marked pending human acceptance, so that the plan's "done when" is checked against facts.
58. As a human, I want neither the raylib window nor the emulator opened in a window, so that verification stays headless.

## Implementation Decisions

### Start

- Step 4 starts by running `make psx-check` (H-3-10). If it is red, or a setup is missing, the run stops with a report.

### Families

| family | primitive (step 3 cost model) | mask bit | expected result |
|---|---|---|---|
| `cls` | `FILL` of the draw area (step 3) | always on | exact |
| `rect` | 4 `TILE`, one per edge, bounds inclusive | `rect` | exact |
| `rectfill` | 1 `TILE`, bounds normalised and inclusive | `rectfill` | exact |
| `pset` | 1 `TILE_1` | `pset` | exact |
| `line` | 1 `LINE_F2` | `line` | divergence expected (GPU line stepping) |
| `circ` | 8 `TILE_1` per iteration of the software's midpoint loop | `circ` | exact if the loop is the software's |
| `circfill` | 4 `TILE` spans per iteration of the same loop | `circfill` | exact if the loop is the software's |
| `spr` (`map` through it) | 1 `SPRT` from the sheet page, CLUT from the draw palette | `spr` | exact |
| `print` | 1 `SPRT` per glyph with a lit row, font CLUT for `col & 0x0F` | `print` | exact |
| patterned fills | `SPRT` per box or span, 8×8 pattern tile, texture window, pattern CLUT | `pattern` (with `rectfill`/`circfill`) | exact |

- The mask has ten bits, in this order: `cls pset line rect rectfill circ circfill spr print pattern`. A patterned `rectfill`/`circfill` (non-zero `fillp_pattern`) passes only when both its shape bit and `pattern` are set. A plain one ignores `pattern`. "All" is every bit set.
- Dithering stays off: step 3's frame begin already sets the texture page with dithering off, and any `DR_TPAGE` a family adds does the same. Textured primitives use raw-texture mode (`setShadeTex(…, 1)`, as step 3's upscale quad does) and no semi-transparency. Every displayed pixel is then a palette colour, and `gpu-capture-clean` can demand it.
- **Colour rule (new, chunk 1).** A pure, host-compiled module, `src/platform/psx_colour.{h,c}`, gives:
  - the RGB555 of a palette index, from `P8_PALETTE_RGB`, each component shifted right by 3 and the mask bit (0x8000) set;
  - a draw-palette byte's CLUT entry: 0x0000 when bit 0x80 is set, else the RGB555 of its low nibble.

  Flat primitives set their colour with the 8-bit components of `P8_PALETTE_RGB`, which the GPU truncates to the same 5 bits. CLUTs use the module directly. The comparator uses the same module to map pixels back. It is added in chunk 1 because the comparator needs it before any family does, and the GPU backend uses it from chunk 2 onward.
- Camera: each entry subtracts the camera from world coordinates, as the software does. Patterns are phased on world coordinates (step 3's rule): the U/V origin of a patterned box is (world x mod 8, world y mod 8).
- `circ`/`circfill`: the GPU backend runs the same integer midpoint loop as the software backend, so step 3's cost model counts its iterations exactly. If the loop is shared code, it is shared through a pure helper in `src/p8/` that yields spans and points; the software backend's output must stay byte-identical.
- `spr`: the texture is the 4-bit sheet page; u = (n mod 16) × 8 and v = (n div 16) × 8; size w×8 by h×8, clipped by the GPU to the draw area. The CLUT key is the 16 draw-palette bytes at call time. Entry rule as in step 3: 0x0000 when bit 0x80 is set, else RGB555 of `draw_palette[c] & 0x0F` with the mask bit, so that black stays opaque.
- **CLUT and pattern slots.** These are frame-local caches keyed by content:
  - draw palettes in the `GPU_CLUT_SLOTS` (6) palette CLUT lines;
  - patterns in the `GPU_PATTERN_SLOTS` (4) pattern textures and their CLUT lines (H-3-5), keyed by `p8_draw_cost_pattern_key`, so that the backend and the meter agree on what a distinct pattern is.

  Their contents are staged in RAM during the frame. In `psx_gpu_frame_end`, after its existing `DrawSync(0)` and before `DrawOTag`, they are uploaded with `LoadImage`. The previous frame is finished at that point, so no slot is rewritten while a frame reads it. A frame needing more slots than exist stops with `p8_fatal` naming the frame, the cache and the count. The meter's peaks size the caches, so this is a sizing failure, not a fallback.
- **Font.** The glyph tables become reachable from the PSX backend through a read-only accessor in `src/p8/`, returning a glyph's five row bytes for a character (H-3-9). The software backend keeps its output identical. In `psx_gpu_init`, a pure packer writes every glyph into `PSX_VRAM_FONT` (128×128 texels) as a 4×6 cell (texel 1 lit, 0 clear). The 16 font CLUTs (entry 0 = 0x0000, entry 1 = the RGB555 of colour c) go into the CLUT lines given by `psx_vram_layout_font_clut`. `print` walks the string exactly as the software does (advance, newline, unknown characters drawn as the placeholder), through one shared pure walker if the software's walk can be extracted without changing its output. Otherwise the GPU copies the walk and the `print` capture proves it.
- **Patterns.** A pattern slot is an 8×8 4-bit tile (the 4×4 pattern repeated, texel 1 for a set bit) and a 2-entry CLUT: entry 0 = colour `col & 0x0F`, entry 1 = `(col >> 4) & 0x0F`, or 0x0000 when `fillp_transparent` is set (step 3's decision). A `DR_TWIN` of 8×8 at the slot repeats it. The `DR_TPAGE` + `DR_TWIN` pair is emitted only when the window changes, as the cost model counts it.
- **Cost.** After each family, `make state` must stay green, and the GPU's real bytes stay ≤ the metered bytes on every frame of the check replay. If a family needs more than the model gives, the model is raised in that family's commit, and `make state` is re-run with its limits re-sized at peak + 1/8. It is never lowered below what a family emits.

### Family filter (`src/devtools/`, new, PC and PSX)

- It is a backend that wraps the backend active when it is installed (H-3-1), like `p8_draw_meter_install`. Its interface:
  - install;
  - set mask, which applies from the next call;
  - frame end, which resets the counts;
  - a count getter per family for the current frame.
- An on-screen call is one the step-3 cost model costs more than 0 (H-3-3). The filter counts it before applying the mask. A patterned fill counts under its shape and under `pattern`.
- The chain on both drivers is meter → filter → backend (software on PC, GPU on PSX). The driver sets the backend, then installs the filter, then installs the meter. The meter sees every call, so its lines and the real ≤ metered check are unchanged.
- The on-screen test calls the matching `p8_draw_cost_*` function with the filter's own `p8_draw_cost_window_t`, reset at frame end, and counts a call when its bytes are above 0. `cls` always counts.
- It is in the PSX GPU PS-EXE's source list and in the PC replay. It is not in `CORE_SRCS`.

### Capture list (`psx/gpu_captures.txt`, committed) and its parser (`src/devtools/`, PC only)

- First line: `replay <name>`. Then one capture per line: `<label> <frame> <mask>`. The label is a family name or `all`. The mask is a comma list of family names, `cls` mandatory, or `all`. Blank and `#` lines are skipped. Frames are unique and strictly increasing. The parser returns the captures or an error naming the line.
- The guard: for a label other than `all`, every family named in the label must have at least one on-screen call at that frame on PC. For the label `pattern`, that means at least one patterned call.
- The initial list, all in `pad_slipway` (401 frames; frames chosen from `tests/e2e/pad_slipway.txt`):

  | label | frame | mask | why this frame | chunk |
  |---|---|---|---|---|
  | `cls` | 1 | `cls` | title screen; proves the pipeline | 1 |
  | `rectfill` | 11 | `cls,rectfill` | title rows, one filled as hovered | 2 |
  | `all` | 100 | `all` | free mode just after the camera centres on p14 | 9 |
  | `pset` | 131 | `cls,pset` | the colour-7 crosshair (`ui.c`) | 3 |
  | `all` | 188 | `all` | p61's menu sliding open over the map | 9 |
  | `pattern` | 190 | `cls,rectfill,circfill,pattern` | planet menu open: the 0x0F0F bar and the 0xA5A5 grid | 8 |
  | `line` | 194 | `cls,line` | the dragged slipway from the bottom edge | 4 |
  | `circfill` | 217 | `cls,circfill` | gameplay planets | 5 |
  | `spr` | 234 | `cls,spr` | the background map and planet sprites, p12 under the centre | 6 |
  | `pattern` | 257 | `cls,rectfill,circfill,pattern` | the transparent 0xAAAA disc on p12 | 8 |
  | `print` | 299 | `cls,print` | "-too far-" | 7 |
  | `rect` | 322 | `cls,rect` | the grid's outer edge and HUD boxes | 2 |
  | `all` | 391 | `all` | two colour-2 slipways along one path, the forbidden sign | 9 |
  | `circ` | 400 | `cls,circ` | the colour-7 circle of radius 9 round p61 | 5 |

  Each frame can carry one mask only, since a frame is drawn once. If the guard fails for a frame, the implementer moves that capture to the nearest frame of the same scene that passes, and records the change in the step note. The list grows chunk by chunk: each chunk adds only its own lines.
- A label repeated on several frames (`pattern`, `all`) gives several captures of one family. They are accepted together.

### PC reference (`src/cmd/` replay, modified)

- With `CAPTURE_LIST=<list>` and `CAPTURE_DIR=<dir>`, the replay parses the list (refusing a replay name other than its recording's basename), installs the filter before `STATE=`'s meter (so the meter wraps it), and sets the mask before each listed frame's `cart_draw` (all otherwise). It writes:
  - `<dir>/pc_frames.bin`: the listed frames' screens in list order, in the screencast frame format (`src/devtools/screencast.h`);
  - `<dir>/counts.txt`: `<frame> <family> <calls>` for the ten families of each listed frame;
  - `<dir>/capture_script.txt`: `<frame> <mask as 4 hex digits>` per capture.
- Without `CAPTURE_LIST=`, nothing is installed and nothing is written. These outputs come from the same `tmp/replay` run as the `STATE=`/`DIGEST=` references (H-3-8), so `make psx-check` runs the PC replay once.

### Frame-locked capture

- **Protocol words.** Three 32-bit words at the scratchpad: state, frame and mask. They sit below the two words the pad protocol uses (the last word and the one under it), and are named in one PSX header and as constants in the Lua. A `make test` static check (a grep script, millisecond-scale) fails when the two disagree.
- **Program side (GPU replay driver, modified).** At start, it writes state = `PROGRAM_READY` and spins until state = `NEXT_READY`; the frame word then holds the first scripted frame, or 0xFFFFFFFF for none. Before each frame's `cart_draw`, it sets the filter mask from the mask word if the frame is the scripted one, and to "all" otherwise. After that frame's `psx_gpu_frame_end`, the screen still shows the previous frame and this one may still be drawing (H-3-4). So the driver:
  - calls a new function, `psx_gpu_frame_show_last()`. It does `DrawSync(0)`, then shows the display buffer just drawn, and leaves the swap state as `psx_gpu_frame_end` set it. The next `psx_gpu_frame_end` shows the other buffer as usual;
  - prints `psx-replay: capture <frame> <mask>`;
  - writes state = `FRAME_SHOWN` and spins until state = `NEXT_READY`.

  The filter's mask is reset to "all" before the next frame.
- **Emulator side (`tools/psx_capture_frames.lua`, new).** It is loaded by `tools/psx_until_exit.lua` when `PSX_CAPTURE_SCRIPT` is non-empty, as the pad player is. It parses the script, and a bad line ends the boot with `error <message>` naming the line. On each vsync it acts on the state word:
  - on `PROGRAM_READY`, it writes the first entry and `NEXT_READY`;
  - on `FRAME_SHOWN`, it waits one vsync, then writes `<prefix>.f<frame>.vram` in the plan-9 format, then the next entry (or none) and `NEXT_READY`.

  A `FRAME_SHOWN` for a frame other than the expected one ends the boot with `error`.
- **`psx_boot.sh`.** It gains `--capture-script <file>`, which needs `--until-exit`, can be combined with `--pad-script`, and is passed through `PSX_CAPTURE_SCRIPT`. The plain capture mode is unchanged.

### Comparator (`src/devtools/` pure module, `src/cmd/` host tool `tmp/psx_capture_compare`)

- Input: one capture file, the PC frame (screencast reader, by index), and its label, frame, mask and call count. Output: one report line, a text diff map and a PPM. Exit 0 when the report is written, whatever the pixels say. Exit 1 only on unreadable or malformed input, naming it.
- Decoding:
  - the header must be `psx-vram 320 240 16` and the size must be header + 153,600;
  - pixels are 16-bit little-endian in PSX VRAM order (red in bits 0–4, green in 5–9, blue in 10–14, bit 15 ignored);
  - each 2×2 block must be uniform, and its top-left pixel is the downscaled pixel;
  - the colour is the index whose RGB555 under the colour rule equals it (mask bit ignored), or off-palette.

  Colours map through the colour rule (`src/platform/psx_colour`, compiled into the host tool). The first capture with colours (the `rectfill` chunk) confirms the channel order. If every pixel is off-palette there, the decoder is fixed and the fix recorded.
- **Metrics per capture.**
  - `differ`: pixels whose index differs (an off-palette pixel always differs), with the per mille of 19,200;
  - `near1`, `near2`, `far`: the differing pixels whose PSX colour appears in the PC frame within Chebyshev distance 1, then 2, then neither;
  - `bbox` of the differing pixels;
  - `offpalette`, `nonuniform`;
  - `calls`: the family's on-screen calls.
- **Report line**: `<label> f<frame> mask <names> calls <n> differ <n> (<p>‰) near1 <n> near2 <n> far <n> bbox <x0>,<y0>-<x1>,<y1> offpalette <n> nonuniform <n>`. With no difference, `bbox -`.
- **Diff map** `<label>-f<frame>.diff.txt`: 120 lines of 160 characters, `.` where equal, the PSX index as a hex digit where different, `?` where off-palette.
- **Image** `<label>-f<frame>.ppm`: binary P6, 960×240. PC, PSX and diff side by side, each 160×120 scaled ×2. The diff panel is dark grey where equal and the PSX colour where different.
- No threshold is applied. Judging a divergence is the human's job (plan rule "Accepted divergence").

### `make psx-check` (H-3-7)

- The PC run gains `CAPTURE_LIST=psx/gpu_captures.txt` and `CAPTURE_DIR`. Step 3's GPU boot gains `--capture-script`. A second GPU boot with the same script runs in its own subdirectory. Then the comparator runs for every capture into `<out-dir>/gpu-captures/`, with `report.txt` holding the report lines in list order.
- New conditions, one line each, evaluated with all the others before concluding:
  - `gpu-capture-list`: the list parses, its replay is the PS-EXE's, and the guard holds;
  - `gpu-captures`: each listed frame has `run.f<frame>.vram`, well-formed, and a `psx-replay: capture` TTY line, in list order, with no extra one;
  - `gpu-capture-deterministic`: the second boot's captures are byte-identical to the first's, and the first differing frame is named;
  - `gpu-capture-clean`: zero `nonuniform` and zero `offpalette` in every report line;
  - `gpu-capture-report`: one report line, diff map and image per capture;
  - `gpu-capture-ceilings`: applies to every capture without a matching reference. Its `differ` must be ≤ its line in `psx/gpu_capture_ceilings.txt` (`<label> <frame> <differ>`). The condition fails in three cases:
    - a capture has no ceiling line, which names it;
    - a count is above its ceiling, which names the capture, the ceiling and the new count;
    - a ceiling line is left for a capture no longer in the list.

    A count below its ceiling passes and prints `below ceiling: <label> f<frame> <ceiling> -> <count>`, so that the ceiling can be lowered. This is the mechanical guard the plan asks for until the references are frozen. A capture with a reference is guarded by the reference instead, and its ceiling line is ignored.
  - `gpu-capture-references`: for each capture, the reference file `psx/gpu_capture_references.txt` (`<label> <frame> <sha256 of the capture file>`; absent or empty means no reference) either matches, or has no entry. In that case the condition prints `pending human acceptance: <label> f<frame>` and still passes. A reference for a frame no longer in the list fails, naming it.
- The condition logic sits in a new script, `tools/psx_capture_check.sh`, called by `psx_check.sh` as it calls the compare scripts. The script is driven on crafted files by a test.
- `PSX_GPU_CEILING`/`PSX_GPU_TIMEOUT` (861 vsyncs / 17 s, from 553 vsyncs) are re-measured with the capture pauses included (first measured run plus half again), and apply to both GPU boots. `tools/psx_check.sh` gains the capture script, the PC capture directory and the comparator tool as arguments. The measurement file gains the report lines.
- **Ceiling target** `make psx-gpu-capture-ceilings LABEL=<label>`:
  - it reads `report.txt` from the last green-but-for-ceilings `make psx-check` run, meaning every other condition passed;
  - it writes the `differ` of that label's captures into `psx/gpu_capture_ceilings.txt` and leaves every other line untouched;
  - it refuses when the run is missing or another condition failed.

  It is the only writer of that file. Each family chunk runs it for its own label, after reading its report. The ceilings committed by the chunk are the counts quoted in the step note. Lowering a ceiling later (a family improved) uses the same target. Raising one is a regression to explain in its commit, and is never done for another chunk's label.
- **Acceptance target** `make psx-gpu-capture-accept LABEL=<label>`, the human's one command: after a green `make psx-check`, it writes or replaces the references of that label's captures from the last run. It is the only writer of the reference file, as `psx-hello-e2e-baseline` is of its reference. It is a human's command: no agent and no skill runs it. `docs/psx-game.md` names it, under "Still to do", as the one command that accepts a reviewed family, and the Makefile legend says the same. It is ready to run when this step ends and is tested on crafted files (below). This step commits no reference.
- **Boots.** The PC run is single. All captures come from the one GPU boot that step 3 already runs, now with `--capture-script`. The only boot added is the determinism boot: one boot cannot prove that two boots agree (plan rule "PSX check cost").
- `make psx-gpu-capture-report` runs the PC references, one GPU capture boot and the comparator, without the other conditions, into `tmp/psx/gpu-capture`. It is used to iterate on one family. It is not in `make test-all`.

### What stays put

- No file of `src/game/` or `src/engine/` changes. No API signature changes. The software backend's output is byte-identical.
- The software PSX replay and every step-1/2/3 condition are unchanged, apart from re-measured GPU ceilings.
- Nothing PSX enters `make test-all`.

## Implementation Chunks

1. **Capture pipeline, with `cls`.** First check that `make psx-check` is green (H-3-10). Then build:
   - the family filter and its unit test;
   - the capture-list parser and its unit test, and the `CAPTURE_LIST=`/`CAPTURE_DIR=` outputs of the PC replay;
   - the colour rule (`src/platform/psx_colour`) and its unit test;
   - the protocol header, the Lua and its static check, and `--capture-script` in `psx_boot.sh`;
   - the handshake in the GPU replay and `psx_gpu_frame_show_last`;
   - the comparator module, its unit test and its host tool;
   - `tools/psx_capture_check.sh` with its test, the seven conditions in `make psx-check`, the reference file (empty), the ceiling file with the `cls` line (written by the ceiling target), the ceiling target, the acceptance target and the report target;
   - the list with the `cls` line only.

   Done when `make psx-check` is green with `cls` pending human acceptance, and `make test-all`, `make state`, `make full-game` and `make sanitize-net` are green.
2. **`rect` and `rectfill`** (plain `TILE`s), with their capture lines. This chunk confirms the colour decoding and the exact ×2 upscale on real colours.
3. **`pset`**, with its capture line.
4. **`line`**, with its capture line. Its divergence is reported, not fixed: the GPU line is the decision.
5. **`circ` and `circfill`** (plain), on the software's midpoint loop, with their capture lines.
6. **`spr` (and `map`)**: the host-tested CLUT builder, the CLUT cache and its frame-end upload, with its capture line.
7. **`print`**: the glyph accessor, the host-tested font packer and font CLUTs, the font upload at init, with its capture line.
8. **Patterned fills**: the host-tested pattern tile and CLUT builder, the pattern slots, `DR_TWIN`, with both `pattern` lines.
9. **Integration and hand-off**: add the three `all` captures and re-measure the ceilings. Update `docs/psx-game.md`:
   - the capture pipeline, the metrics, the conditions and the acceptance target;
   - under "Still to do", one line per label pending human acceptance with its report line.

   Update the Makefile legend.

Each family chunk also: keeps the real ≤ metered check quiet (raising the cost model and re-running `make state` if needed), runs `make psx-check` green, and runs `make psx-gpu-capture-ceilings LABEL=<its label>` after reading its report, commits the ceiling lines with the family, and adds the family's report line, its ceiling and `make psx-check`'s run time to the step-note draft.

## Testing Decisions

- A good test drives a public interface and checks what leaves it: the calls a recording backend receives, the counts a getter returns, the bytes a pure builder writes, the lines and exit code of a script on crafted files. It never checks how a cache stores its slots.
- **Family filter (new `test_draw_filter`, `make test`).** A recording backend under the filter. Each family is passed or dropped by its mask bit. A patterned fill needs `pattern` and its shape bit, and a plain fill ignores `pattern`. Counts are per family, before the mask, on-screen calls only (an off-screen call counts 0), and frame end resets them. "All" forwards every call with the same arguments and pointers. The filter writes no graphics or VRAM state.
- **Capture list (new `test_capture_list`, `make test`).** Valid list, blank and `#` lines, the `all` keyword. Each refusal names its line: unknown family, mask without `cls`, duplicate or decreasing frame, frame past the end, missing or wrong `replay` line. The script line written for a mask (bit order fixed).
- **Comparator (new `test_capture_compare`, `make test`).** Crafted in-memory captures:
  - bad header, size or depth refused;
  - a uniform capture of each palette colour maps to that index;
  - one off-palette pixel counted;
  - one non-uniform block counted;
  - a one-pixel shift counted as `near1`, a two-pixel one as `near2`, and a wrong colour as `far`;
  - bbox, and `-` when equal;
  - the diff map's size and characters;
  - the PPM header and size.
- **Colour rule (new `test_psx_colour`, host-compiled, `make test`, chunk 1).** The RGB555 of each of the 16 palette colours, with the mask bit set and black at 0x8000. A draw-palette byte with bit 0x80 maps to 0x0000. A byte with a high nibble set but not 0x80 uses its low nibble.
- **PSX pure builders (new `test_psx_gpu_textures`, host-compiled, `make test`).**
  - The CLUT from a draw palette: transparent entry 0x0000, black opaque 0x8000, remap.
  - The pattern tile for 0xA5A5, 0x0F0F and 0xAAAA, against the software's `fillp` bit at the same world phase, and its CLUT, transparent or not.
  - The font texture against the glyph accessor, glyph by glyph, and the 16 font CLUTs.
- **Capture check script (new `test_psx_capture_check`, `make test`).** Crafted directories:
  - a missing capture, a malformed one and an extra TTY line;
  - two runs differing in one byte, with the frame named;
  - a non-zero `nonuniform`/`offpalette`;
  - no reference (passes, `pending` printed), a matching reference, a differing reference, and a stale reference;
  - a count equal to its ceiling, below it (passes, `below ceiling` printed), above it (fails, both counts named); a missing ceiling line; a stale one; a capture with a reference whose ceiling is exceeded (passes).
- **Ceiling and acceptance targets.** Their writer scripts are driven on crafted run directories by the same test. The ceiling writer touches only the given label's lines, and refuses a run whose other conditions failed. The acceptance writer writes the sha256 of each of the label's captures and leaves the other labels untouched.
- **Protocol constants.** The static check runs in `make test`.
- **Software backend.** `test_gfx`, the e2e and `make state` are the net. Any helper extracted for circles or print must leave every `*.sha256` unchanged.
- **PSX side.** The seven conditions of `make psx-check` are the integration tests of the families, the handshake and the Lua. No unit test runs the toolchain or the emulator.
- The new tests are millisecond-scale, and the PC replay pays for the filter only under `CAPTURE_LIST=`. `make test` and `make e2e` keep their timing.
- Prior art:
  - `test_draw_backend` and `test_draw_meter` from step 3 (a recording backend under a wrapper);
  - `test_psx_state_compare` and `test_frames_check` (scripts on crafted files);
  - `test_psx_vram_layout` from step 3 (host-compiled PSX pure code);
  - `test_gfx` (the software reference);
  - `tools/psx_reference_check.sh` and `psx-hello-e2e-baseline` (sha256 references and their single writer).

## Out of Scope

- **Freezing references.** No reference is written by this step. The human reviews each family and runs the acceptance target.
- **Judging or fixing a divergence the comparator reports.** That includes the `line` stepping, unless the divergence is plainly a bug (off-palette, shifted by the camera, wrong colour everywhere), which is fixed.
- **The controller and the title passed through scripted input** (step 5). **The game's `.bin/.cue`** (step 6).
- **Memory savings.** Removing the RAM framebuffer or RAM sheet from the GPU build, or shrinking the caches below their measured peak (plan 11).
- **`make psx-full-game-check` on the GPU PS-EXE.** It stays pending for plan 11.
- **Captures of replays other than the check replay.** Changing `src/game/`, `src/engine/` or any API signature. Audio.

## Further Notes

- **Why masks per family.** A family's capture shows only `cls` and that family, so its report does not move when a later family lands. The human can review families in any order. Interactions (transparency over the grid, text over fills, draw order) are covered by the three `all` captures.
- **What the metrics mean.** `near1`/`near2` separate a pixel drawn one or two pixels away from where the PC draws it (the plan's "accepted divergence" kind) from a pixel that has no business there (`far`). A large `far` count on an "exact" family is a bug to fix in its chunk, not a divergence to hand over.
- **Cost of the check.** `make psx-check` (about 60 s today) gains one GPU boot for determinism, about 10 s (553 vsyncs plus the pauses), and a few vsyncs per capture. It stays outside `make test-all`. Each step note records the measured run time.
- **Step note contents.**
  - The start check's result.
  - A table: label, frame, mask, report line, committed ceiling, image path, "pending human acceptance".
  - `make psx-check`'s run time (plan rule "PSX check cost"), with the number of boots.
  - The human's command to accept a label: `make psx-gpu-capture-accept LABEL=<label>`.
  - Every frame moved by the guard.
  - The channel-order and upscale findings of the `rectfill` chunk.
  - Every raise of the cost model, with the new `make state` peaks and constants.
  - The new GPU vsyncs, ceiling and timeout.
  - Any fact of "Step 3 as committed" found to differ from the code, and how the step adapted.
