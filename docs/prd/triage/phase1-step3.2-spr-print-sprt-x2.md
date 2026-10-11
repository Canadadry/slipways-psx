---
title: "Plan 19 step 3.2: spr and print as x2 SPRTs from an x2 sprite sheet and font"
description: "After steps 2 and 3.1 every untextured call draws at x2 in the display buffer, but spr and print still send 1x SPRTs read from the 1x sheet and font, so sprites and text land in the top-left quarter at a quarter of their size."
status: needs-triage
---

## Plan context (verbatim from docs/plan-19.md, § 3)

Step 3's common text:

> **Primitives at x2, one family per step.** Each family comes with its cost-model change, its unit tests, and its capture labels passing the same conditions. It also re-sizes `GPU_FRAME_BUFFER_BYTES` (and the CLUT and pattern slots) by the existing rule in its own commit, with `make state` passing and `ram-room` staying positive. Step 2 does the same for its family. In this order:

This sub-step's bullet:

> - `spr`/`print`: `SPRT` with doubled position, size and UV, together with uploading the packed sprite sheet (128x128 4-bit) and the font's glyph cells (4x6 per code) upscaled x2, in the layout step 2 settled.

## What step 2 settled (from the plan's step notes, committed)

- The frame draws straight into its 320x240 display buffer. `cls`, `rect`, `rectfill` and `circfill` spans are `TILE`s doubled by `psx_tiles_double`; the families not yet converted draw at 1x into the top-left quarter until their own step.
- Frame overhead: `DR_ENV` + `DR_TPAGE` + `DR_TWIN` (52 bytes). Today the backend sends only a `DR_TWIN` on a window change, while the cost model already meters the `DR_TPAGE` + `DR_TWIN` pair for it.
- The x2 VRAM layout, in 16-bit units: the x2 sheet fills the whole 4-bit page at (512, 256) (64x256 units, 256x256 texels); the x2 font is 64x96 units at (576, 256), page (576, 256), v 0..95 (256 codes in 8x12-texel cells, 32 a row, 8 rows); the x2 pattern slots will share the font's page at v 96..255 (step 3.3); the CLUTs stay at (640, 0). The 1x textures stay in the page at (512, 0) until the step that replaces them.
- So from this step the textured calls read more than one page, and **the cost model must count a page switch: a change of (page, window) costs the `DR_TPAGE` + `DR_TWIN` pair.**

## Hypotheses on step 3.1's outcome

Step 3.1 ("`pset` and `circ`: a 2x2 `TILE` instead of `TILE_1`") is not done when this PRD is written. Its PRD exists, but its code is not committed, so this PRD reads the code at the commit that closed step 2 and reads step 3.1 only from its PRD. Every assumption about what step 3.1 leaves behind is listed here. Before starting, the implementer checks each one against the code and the plan's step notes. When one fails, its "if it fails" line says what changes in this PRD.

- **H-3.1-1: step 3.1 is committed first.** Step 3.1's behaviour commit and its re-size commit are on the branch: `gpu_pset` and `gpu_circ` send doubled 2x2 `TILE`s through the shared tile emitter, and `P8_GPU_TILE_1_BYTES` and the backend's `TILE_1` size check are gone.
  *If it fails* (3.1 not committed, or only partly): this step does not start and does not fold any of 3.1 in, because the plan sets the order. The implementer reports that 3.1 is missing and stops.
- **H-3.1-2: step 3.1 left the textured path as step 2 left it.** These still have the shape they had at the commit that closed step 2: `add_sprt`, `add_textured_sprite`, `use_texture_window` and its `window_slot` state, `upload_sprite_sheet`, `upload_font`, the `DR_TPAGE` + `DR_TWIN` that `psx_gpu_frame_begin` sends, `p8_draw_cost_window_t` (`patterned`, `pattern_key`), the `spr`/`print`/patterned-fill costs, `psx_vram_layout`, `psx_gpu_textures`, `psx_sprite`, `psx_text` and `psx_pattern`.
  *If it fails* (3.1 touched one of them): the decisions below still hold as contracts (what the frame sends, what the meter counts, where each texture lives). The implementer applies them to the code as 3.1 left it, and the step note names what differed.
- **H-3.1-3: the sizes step 3.1 set are recorded.** Plan 19's step notes have a "Step 3.1" entry with the `make state` peak, measured with `pset`/`circ` at 16-byte `TILE`s. It also gives the `GPU_FRAME_BUFFER_BYTES` that 3.1 set: expected 75776, from step 2's measured 67316 for the single-`LINE_F2` option, though 73728 is possible if the peak moved. `GPU_CLUT_SLOTS` stays 6, `GPU_PATTERN_SLOTS` stays 4, and `ram-room` and `game-ram-room` are positive.
  *If it fails* (no entry, or no figure): before changing anything, the implementer runs `make state` once on 3.1's last commit and writes that baseline peak in this step's note, so that this step's growth is told apart from 3.1's.
- **H-3.1-4: the gate state step 3.1 leaves.** In `make psx-check`'s `conditions.txt`, every condition is `ok` except `gpu-capture-clean` and `gpu-capture-ceilings`. Those two fail only on `spr 234`, `print 275`, `pattern 190`, `pattern 257`, `circfill 263`, `line 276`, `all 100`, `all 188` and `all 265`. `cls 1`, `rectfill 11`, `rect 300`, `pset 131` and `circ 314` are green.
  *If it fails* (another condition or a label due by 3.1 is red): this step does not fix it, since that would fold steps together. The step still makes `spr 234` and `print 275` green and keeps every label that was green still green. It stops before the re-size commit, records the inherited red condition or label and its reason in the step note, and hands it back through the plan's normal failure path.
- **H-3.1-5: references pending, ceilings untouched.** All 14 entries of `psx/gpu_capture_references.txt` are `-`. In `psx/gpu_capture_ceilings.txt`, `spr 234` and `print 275` still hold 0.
  *If it fails*: this step accepts no capture, because acceptance happens once, at step 6. It sets any remaining entry to `-` in its own commit and records that in the step note. It does not raise a ceiling: `spr` and `print` must be exact.
- **H-3.1-6: tiles have one doubling point.** `psx_tiles_double` and `PSX_TILES_SCALE = 2` are still the only doubling in the backend, and 3.1 added no generic scale helper.
  *If it fails* (3.1 added a shared doubling helper that also fits a sprite's six fields): `psx_sprite_double` below is built on that helper instead of next to it. Its contract and its test do not change.
- **H-3.1-7: the cost model's window tests are as step 2 left them.** 3.1 changed only the `pset`/`circ` cases of `test_draw_cost`, `test_draw_meter` and `test_psx_tiles`. The window tests (`test_patterned_fill_is_a_sprt_with_its_window_pair_once`, `test_spr_and_print_reset_the_window_after_a_pattern`, `test_untextured_and_culled_calls_keep_the_window`, `test_frame_begin_returns_the_overhead_and_resets_the_window`) are unchanged.
  *If it fails*: the implementer applies the new expectations below (spr and print are different textures) to whatever those tests became.
- **H-3.1-8: step 3.1 edited only its own lines of the PSX game doc.** In the doc, 3.1 rewrote only the `pset`/`circ` wording. The VRAM table, the "Still to do (steps 3.2 and 3.3)" table and the paragraph saying "The window change is one `DR_TWIN`, not the `DR_TPAGE` + `DR_TWIN` pair" are as step 2 left them.
  *If it fails*: the doc edits below apply to whatever text now describes the same facts.

## Problem Statement

A player boots the PSX disc after steps 2 and 3.1. Every solid shape and every dot is now drawn at display resolution, but every sprite (`spr`, and `map` through it) and every piece of text (`print`) is still a 1x `SPRT` at its 160x120 position, read from the 1x sprite sheet and the 1x font. So the ships, the planets' icons, the title logo and all the HUD text sit in the top-left quarter of the screen at a quarter of their size, apart from the shapes drawn around them. On `make psx-check`, the `spr 234` and `print 275` captures fail their `offpalette`/`nonuniform`/`differ` conditions.

The textured calls also cannot all move to x2 on the one texture page today's frame sets. The x2 sheet alone fills a whole 4-bit page, and the x2 font sits on another. For every change of page, the backend must send a `DR_TPAGE`, and the cost model must price it. Otherwise a frame that alternates sprites and text could send more bytes than metered, and stop on the backend's fatal.

## Solution

At boot, the backend uploads the packed sprite sheet upscaled x2 (256x256 texels, every texel doubled in both directions) to the page at (512, 256). It uploads the font texture upscaled x2 (8x12-texel cells, 96 lines) to (576, 256). Both go in short chunks through one pure, host-tested x2 upscaler. Each `spr` and each lit glyph of a `print` stays placed and clipped at 1x, exactly as today. It is then doubled (position, size, UV) by one pure function and sent as one `SPRT` reading the x2 texture in its own page. So every PICO-8 pixel of a sprite or a glyph is a uniform 2x2 block of a palette colour, read 1:1 with no scaling.

Any change of texture source (sprite sheet, font, or a pattern slot) sends the `DR_TPAGE` + `DR_TWIN` pair. The cost model meters that pair for the same changes, now telling the sheet and the font apart. The 1x sheet and font regions leave the VRAM layout, because nothing reads them any more. The 1x pattern slots stay where they are until step 3.3. `GPU_FRAME_BUFFER_BYTES` and the CLUT and pattern slots are re-sized by the existing rule. `spr 234` and `print 275` then pass `offpalette 0`, `nonuniform 0` and `differ` 0. The PC game, its replays and its timing do not change.

## User Stories

1. As a player on the PSX disc, I want every sprite drawn at its PC position on the 320x240 screen, so that ships, planets and icons are where the PC draws them.
2. As a player, I want every sprite pixel to be a 2x2 block, so that sprites have the same size as every other PICO-8 pixel.
3. As a player, I want the map (drawn through `spr`) at x2, so that the background matches the PC.
4. As a player, I want every glyph of every `print` drawn at its PC position and at x2, so that the HUD, the menus and the title text are readable and in place.
5. As a player, I want glyph outlines of even width, so that text no longer looks uneven under a hardware renderer.
6. As a player, I want the title logo drawn from the x2 sheet with no stretched texture, so that it is no longer broken up under DuckStation's hardware renderer.
7. As a player, I want transparent sprite texels (draw palette bit 0x80) to stay transparent at x2, so that sprites keep their shapes over the background.
8. As a player, I want black remapped onto another colour to stay opaque at x2, so that the draw palette behaves as on PC.
9. As a player, I want a sprite or a glyph that is partly off screen clipped exactly as on PC, so that nothing spills into the other display buffer.
10. As a player, I want a sprite or a string that is entirely off screen to draw nothing, so that the PSX frame matches the PC frame.
11. As a player, I want a `print` with the camera set to place its text as the PC does, so that world labels follow the camera.
12. As a player, I want sprites `palt`/`pal`-remapped within one frame to keep their own colours, so that the per-call CLUT still works at x2.
13. As a player, I want a frame that alternates sprites and text to never stop on a frame-buffer overflow, so that a long game still runs to the end.
14. As a player, I want patterned fills to keep working while sprites and text move to x2, so that this step does not break the step 3.3 family.
15. As a maintainer, I want one pure x2 upscaler for 4-bit textures, used for both the sheet and the font, so that "upscaled x2" has one definition with one test.
16. As a maintainer, I want the upscaler to produce any band of x2 lines from any first line, so that the backend can upload in small chunks and keep its RAM use low.
17. As a maintainer, I want a unit test that checks every texel of the x2 sheet against the packed 1x texel at (u/2, v/2), so that a wrong nibble order or a missed line fails `make test`.
18. As a maintainer, I want a unit test that checks every texel of the x2 font against the 1x font texel at (u/2, v/2), and that all 256 codes fit the 96 x2 lines, so that no glyph is cut.
19. As a maintainer, I want the chunked upscale to equal a single-pass upscale whatever the chunk size or the first line, so that chunk boundaries cannot shift a line.
20. As a maintainer, I want one pure function that doubles a placed sprite's position, size and UV, so that `spr` and `print` share one mapping that is tested on the host.
21. As a maintainer, I want `psx_sprite_place`, `psx_sprite_clip` and `psx_text_place` to keep their 1x contract, so that their existing pixel-for-pixel tests against the software backend stay valid.
22. As a maintainer, I want a host test that places, doubles and paints a sprite from the x2 sheet into a 320x240 buffer and compares it with the software screen upscaled x2, so that the whole host-side `spr` path is proven exact at x2.
23. As a maintainer, I want the same x2 test for `print` from the x2 font, so that the whole host-side `print` path is proven exact at x2.
24. As a maintainer, I want the VRAM layout to name the x2 sheet and the x2 font as the sheet and font regions, so that the layout lists only regions something reads.
25. As a maintainer, I want the 1x sheet and font regions freed only because nothing reads them, while the 1x pattern slots stay in their page until step 3.3, so that patterned fills keep their texture.
26. As a maintainer, I want the layout to give the 4-bit page that holds a given region, so that each textured call selects its own page and no caller hard-codes one.
27. As a maintainer, I want a layout test that checks the sheet, the font and the patterns each fit inside one page, and that the x2 sheet fills its page alone, so that a moved region that straddles a page fails `make test`.
28. As a maintainer, I want the cost model's texture state to tell the sprite sheet, the font and each pattern apart, so that a sheet-to-font switch is priced.
29. As a maintainer, I want any change of texture state to cost the `DR_TPAGE` + `DR_TWIN` pair in both the meter and the backend, so that the meter matches the frame byte for byte.
30. As a maintainer, I want the frame's first state, in the meter and in the backend, to be the sprite sheet with no window, so that the first `spr` of a frame is free in both.
31. As a maintainer, I want culled calls to leave the texture state alone in both, so that the backend never switches where the meter does not.
32. As a maintainer, I want the backend's "sent more than metered" fatal to stay unreachable, so that the meter remains an upper bound for every frame.
33. As a maintainer, I want the PC and PSX `zone gpu_frame` meter lines to stay equal (`gpu-cost`, `game-gpu-cost`), so that the meter is proven to describe the PSX build.
34. As a maintainer, I want the backend to assert that a doubled `SPRT`'s UV stays inside its 256x256 page, so that an 8-bit UV wrap is caught where it happens.
35. As a maintainer, I want the boot-time checks that a region holds its texture to compare against the x2 sizes, so that a region too small for the x2 texture stops at boot with a clear message.
36. As a maintainer, I want `GPU_FRAME_BUFFER_BYTES` re-sized by the existing rule (measured `make state` peak + 1/8, rounded up to 1 KiB) in its own commit, so that the extra page switches keep their margin and a bisect tells the size apart from the behaviour.
37. As a maintainer, I want `GPU_CLUT_SLOTS` and `GPU_PATTERN_SLOTS` re-checked by the same rule in that commit, so that the plan's re-size clause is met.
38. As a maintainer, I want `make state` to pass and `ram-room`/`game-ram-room` to stay positive after the larger upload chunk and buffers, so that the replay and the game still fit in 2 MiB.
39. As a maintainer, I want `spr 234` and `print 275` to pass `offpalette 0`, `nonuniform 0` and `differ` 0, so that this family is proven exact at x2.
40. As a maintainer, I want every label that was green before this step to stay green, so that the step regresses nothing.
41. As a maintainer, I want the remaining red labels to be only those due later, so that the step is gated as the plan's Rules require.
42. As a maintainer, I want no capture reference accepted and no ceiling changed, so that human acceptance stays one pass at step 6.
43. As a maintainer, I want every `*.sha256` in `tests/e2e/` unchanged and `make test`/`make e2e` no slower, so that the PC game is proven untouched and the loop stays fast.
44. As a maintainer, I want the PSX game doc to describe the x2 sheet and font, their pages and the page-switch pair, so that `docs/` says what is true.
45. As a maintainer, I want plan 19's step notes to record the new peak, the buffer size, the slots, the RAM room, the two labels' figures and the remaining red labels, so that step 3.3 starts from known figures.
46. As step 3.3's implementer, I want the cost model's texture state to already treat a pattern slot as its own source, so that moving the patterns to the font's page needs no cost-model change.
47. As a reviewer, I want the PR to touch only `spr`, `print`, their textures, the texture state, the layout, their tests, the sizes and the doc, so that I can check it against the plan bullet in one read.

## Implementation Decisions

- **x2 upscaler (`psx_gpu_textures`), pure.** It gets one new function. It takes a 4-bit source texture (its width in texels, its lines), a first x2 line and a line count. It writes those x2 lines, each `2 x width` texels, into the caller's buffer. x2 texel (u, v) is source texel (u/2, v/2). Source and output use the byte layout of `p8_sprite_sheet_packed`: texel u of a line is in byte u/2, low nibble for an even u. That is the layout `LoadImage` takes for a 4-bit texture.
- **Font bytes are little-endian.** The font texture (`psx_gpu_textures_font`, `uint16_t` units, leftmost texel in the low nibble) is handed to the upscaler as its bytes. This is the same layout only because both the PSX and the host are little-endian. The decision states this as an assert-with-message trap in the upscaler's test, which checks the font's byte view texel for texel against `font_texel`. The font texture itself stays 1x and unchanged: it is the input of the x2 font, and `test_psx_text`'s 1x oracle.
- **Font extent.** A new constant in `psx_gpu_textures` gives the lines of the 1x font that hold a cell (256 codes / 32 a row x 6 = 48). It is checked at compile time against the cell layout. The x2 font region's height must be twice it (96).
- **Sprite doubling (`psx_sprite`), pure.** It gets one new function next to `psx_sprite_clip`, which returns the sprite with x, y, u, v, w and h multiplied by `PSX_TILES_SCALE`. It is applied after placement and clipping at 1x, so a doubled sprite always lies inside 320x240, and a clipped edge stays on an even texel of the x2 texture. `psx_sprite_place`, `psx_sprite_clip` and `psx_text_place` keep their 1x contracts. `psx_gpu_textures_font_cell` keeps returning 1x texels.
- **VRAM layout (`psx_vram_layout`).** `PSX_VRAM_SPRITE_SHEET` becomes (512, 256), 64x256. `PSX_VRAM_FONT` becomes (576, 256), 64x96. The 1x sheet (512, 0) and the 1x font (512, 128) leave the table, because only `gpu_spr` and `gpu_print` read them, and both move to x2. The other reads of the sheet (`sprite-sheet` digest, `p8_sprite_sheet_load`) are RAM, not VRAM. `PSX_VRAM_PATTERNS` stays (544, 0), 32x256 until step 3.3. `psx_vram_layout_texture_page(void)` becomes `psx_vram_layout_texture_page(region)`: the 64x256 4-bit page that holds that region. The function asserts that the region does not straddle a page. `psx_pattern_window` uses the page of `PSX_VRAM_PATTERNS`. The doc's x2 pattern slots, (576, 352) at v 96..255, are not added here, because they belong to step 3.3.
- **Texture state in the cost model (`draw_cost`).** `p8_draw_cost_window_t` replaces `patterned` with `texture`, one of sheet, font or pattern, and keeps `pattern_key`, which is meaningful only for pattern. `spr` switches to (sheet, 0), `print` to (font, 0) and a patterned span to (pattern, key). Any change of the pair costs `P8_GPU_DR_TPAGE_BYTES + P8_GPU_DR_TWIN_BYTES`, two primitives, as the window switch costs today. `p8_draw_cost_frame_begin` resets the state to (sheet, 0). The frame overhead stays `DR_ENV` + `DR_TPAGE` + `DR_TWIN` (52 bytes). The visibility rules and the `SPRT` counts do not change: one per `spr`, one per non-space glyph. The source is not reduced to the VRAM page. In step 3.3, font and patterns share a page, and a font-to-pattern change will still be metered as a pair. That is a sound upper bound, and the backend will send exactly that, so step 3.3 needs no cost-model change for pages.
- **Backend texture state (`psx_gpu`).** `window_slot` becomes a state of (texture region, pattern slot or no window). One function sets it. When either part differs from the current state, it sends a `DR_TPAGE` for the page holding the region (same draw-mode flags as today's `frame_begin`), then a `DR_TWIN` (the slot's window, or none). That is exactly the pair the meter counts for the same change. `psx_gpu_frame_begin` sends the draw target, then forces the (sheet, no window) state: the same 16 bytes it sends today, in place of the 1x page. `gpu_spr` sets (sheet, none). `add_glyph` sets (font, none). Patterned spans set (patterns, slot), and their `SPRT`s stay 1x until step 3.3.
- **Why the bound still holds.** The backend only switches state when it emits a `SPRT`, and that implies the meter's box test passed for that call. So the backend's sequence of state changes is a sequence over a subset of the calls the meter switched on, both starting from (sheet, none). A subsequence never has more adjacent changes than the sequence it comes from. This is the argument that already holds for the pattern windows today, and the per-frame "sent > metered" fatal stays.
- **Textured `SPRT` at x2.** `add_textured_sprite` doubles the 1x sprite. It adds the region's offset in its page (`(region.x - page.x) x 4` texels, `region.y - page.y` lines; 0 for both x2 regions) to the doubled UV. It asserts that `u + w <= 256` and `v + h <= 256`, and then sends one `SPRT` with the same CLUT as today: the draw palette slot for `spr`, the font colour line for `print`. The sheet's existing assert (no read past the sheet at 1x) guarantees the x2 bound for `spr`, and the font extent guarantees it for `print`.
- **Uploads at boot (`psx_gpu_init`).** One static chunk buffer of 16 x2 lines is shared by both uploads. It is word-aligned for `LoadImage`: 16 x 256 texels / 2 = 2 KiB, against 1 KiB today. The sheet is uploaded as 16 chunks of 16 x2 lines, upscaled from `p8_sprite_sheet_packed`. The font is built 1x into its existing static buffer, then uploaded as 6 chunks of 16 x2 lines from its first 48 lines. Each chunk is a `LoadImage` followed by `DrawSync`, as today. The boot checks become: the sheet region is `4 x P8_SPRITE_SHEET_PACKED_SIZE` bytes and a whole number of chunks high. The font region is `2 x PSX_GPU_TEXTURES_FONT_SIZE / 4` units wide and twice the font extent high. A mismatch is a `p8_fatal` naming the region, as today. The 16 font CLUTs and the CLUT layout do not change.
- **No RAM growth beyond the chunk.** No 32 KiB x2 sheet or 12 KiB x2 font is ever held whole, so BSS grows by 1 KiB plus the re-size.
- **Re-size (`gpu_sizes`), own commit.** After the behaviour commit, run `make state` and take the `zone gpu_frame` peak from `tmp/state.txt`. Set `GPU_FRAME_BUFFER_BYTES` to `peak x 9 / 8`, rounded up to a multiple of 1024. If that equals the current value, skip the commit and write the figures in the step note. `GPU_CLUT_SLOTS` and `GPU_PATTERN_SLOTS` are recomputed from the same run by the same rule. This family's CLUT counting does not change, so they should stay 6 and 4, and the note says why if not. `make state` must exit 0 afterwards. Each extra 1 KiB of buffer costs 2 KiB of BSS, and `ram-room` and `game-ram-room` must stay positive. If either goes negative, the step stops before committing the re-size and records the blocker.
- **Gate.** `make psx-check`'s `conditions.txt` must have every condition `ok` except `gpu-capture-clean` and `gpu-capture-ceilings`. Those two must fail only on labels not yet due: `pattern 190`, `pattern 257`, `circfill 263` (its mask includes `line`), `line 276`, `all 100`, `all 188` and `all 265`. By the mask rule in `psx/gpu_captures.txt`, this step must turn green `spr 234` (mask `cls,spr`) and `print 275` (mask `cls,print`), each with `offpalette 0`, `nonuniform 0` and `differ` <= 0. `cls 1`, `rectfill 11`, `rect 300`, `pset 131` and `circ 314` stay green (H-3.1-4). `make psx-check` itself still exits 1 by design, until the `line` step.
- **Ceilings and references untouched** (H-3.1-5).
- **Documentation (PSX game doc).** The VRAM table gets the x2 sheet and font rows, at their x2 positions and in their pages. The 1x sheet and font rows go. The pattern row says it stays 1x in page (512, 0) until step 3.3. The "Still to do" table keeps only the x2 pattern slots row, now labelled step 3.3. The fill paragraph's "the window change is one `DR_TWIN`, not the pair" becomes: any change of texture source (sheet, font, pattern slot) sends the `DR_TPAGE` + `DR_TWIN` pair the cost model meters. The `spr` and `print` bullets of the backend description say: placed and clipped at 1x, doubled, one `SPRT` from the x2 texture, uploaded x2 in 16-line chunks at boot.
- **Step note.** Plan 19's step notes get a "Step 3.2" entry. It records the `make state` peak and the replay that holds it, the old and new `GPU_FRAME_BUFFER_BYTES`, the CLUT/pattern slot figures, `ram-room` and `game-ram-room`, the `spr 234`/`print 275` figures, the remaining red labels, and which H-3.1-k held.

## Implementation Chunks

1. **Texture state in the cost model (host only).** `p8_draw_cost_window_t` gets `texture` (sheet, font, pattern), and frame begin resets it to sheet. `spr`, `print` and the patterned fills switch to their own state. Update `test_draw_cost`'s window tests; add a test that alternating `spr` and `print` costs a pair each switch, and that a culled `spr` or `print` changes nothing. The backend still sends only a `DR_TWIN`, which the meter's extra pairs bound from above, so this commit is safe alone. `make test` is green, e2e unchanged.
2. **Pure x2 helpers (host only).** Add the x2 upscaler and the font extent constant to `psx_gpu_textures`, and `psx_sprite_double` to `psx_sprite`, with their tests. Add the x2 paint-and-compare tests to `test_psx_sprite` and `test_psx_text`. Nothing on the PSX uses them yet.
3. **Layout, uploads and backend (one commit, because the layout and the backend must agree to build).** Move the sheet and font regions to x2, add `psx_vram_layout_texture_page(region)`, and update `psx_pattern` and its test, and `test_psx_vram_layout`. In the backend: the x2 uploads through the chunk buffer, the texture-state function sending the pair, doubled textured `SPRT`s with the UV asserts, and the frame begin state (sheet, none). Edit the doc. `make test` is green, and `make psx-build` compiles and links.
4. **Re-size (own commit).** Run `make state`, apply the rule, and run `make state` again until it is green.
5. **Gate.** Run `make psx-check` once per attempt (the emulator is shared). Read `conditions.txt` and the capture reports. Check the two labels, the green set and the remaining red set, then write the step note.

## Testing Decisions

- Good tests here check what a caller sees: the cost a call reports given the previous calls, the texels an upscale produces, the pixels a placed and doubled sprite paints compared with the software backend. They do not inspect packet bytes or the backend's statics.
- **Cost model (`test_draw_cost`).** The existing window tests move to the new expectations. `spr` after `print`, and `print` after `spr`, each cost the pair. `spr` after `spr` and `print` after `print` are free. A pattern after either one costs the pair, and so does either one after a pattern. Frame begin resets to sheet: the first `spr` is free, the first `print` costs the pair. Culled calls keep the state. Prior art: `assert_cost` and `WINDOW_PAIR_BYTES` in the same file.
- **Meter (`test_draw_meter`).** Its expected frame totals move only where a test mixes `spr` and `print`. No new test.
- **x2 upscaler (`test_psx_gpu_textures`).** Upscale the whole packed sheet and check all 256x256 x2 texels against `p8_sprite_sheet_packed` at (u/2, v/2). Upscale the font's 48 lines and check all 256x96 texels against the 1x font. Produce the same 256 lines in 16-line chunks, in 7-line chunks, and from an odd first line, and check that the result is identical to the single pass. Check that the font's byte view equals `font_texel` (the little-endian trap). Check that every code's x2 cell, twice `psx_gpu_textures_font_cell`, plus 8x12, fits inside 256x96.
- **Sprite doubling (`test_psx_sprite`).** `psx_sprite_double` of {0,0,0,0,1,1} gives {0,0,0,0,2,2}, and of a sprite at (152,112) with UV (120,120) and size 8x8 gives (304,224), UV (240,240), 16x16. That is the last cell, which ends exactly on the 320x240 buffer and the 256x256 page. New x2 test: for the existing placement cases (clipped on each side, camera, multi-cell), place, double and paint from the x2 sheet into a 320x240 buffer through the same CLUT rule, and compare with the software screen upscaled x2.
- **Text (`test_psx_text`).** The existing 1x tests are kept. New x2 test, the same way: place, double each glyph, and paint from the x2 font into 320x240. The result equals the software `print` upscaled x2, for the existing strings, the camera case and the clipped cases.
- **Layout (`test_psx_vram_layout`).** The "share one page" test becomes: the sheet, the font and the patterns each lie inside `psx_vram_layout_texture_page(region)`, every page is aligned (x % 64, y % 256, 64x256), and the sheet's page equals the sheet region. The no-overlap test and the CLUT tests are unchanged.
- **Patterns (`test_psx_pattern`).** It uses the patterns' page, and its results are unchanged.
- **Not unit-tested.** `psx_gpu.c` builds only for the PSX. Its behaviour is proven by `spr 234` and `print 275` in `make psx-check` (the emulator's display buffer against the PC capture x2), and by the equal meter lines.
- **Speed.** No new test file, no new e2e file, no slow check. The added loops are a few hundred thousand texel comparisons, well within `make test`'s budget.

## Out of Scope

- Patterned fills at x2, the x2 pattern slots and their upload, and the texture window at x2 (step 3.3). The 1x pattern region stays, in page (512, 0).
- `line` (the `line` step), and `cls`, `rect`, `rectfill`, `circfill`, `pset` and `circ` (steps 2 and 3.1).
- The frame overhead, the draw target, and the removal of `POLY_FT4` (step 2).
- Accepting or editing capture references, or changing capture ceilings.
- Making `make psx-check` exit green (from the `line` step on), the "no stretch left" check (step 4), and the boot ceilings (step 5).
- Any headless reproducer (step 1 found none) and any DuckStation check (step 6).
- Audio, the PC raylib backend, and the e2e replays.

## Further Notes

- After this step three pages are in use: the x2 sheet (512, 256), the x2 font (576, 256) and the 1x patterns (512, 0). From step 3.3 there are two, as step 2's note planned. The texture-state design does not change with that.
- The byte growth comes only from texture switches (16 bytes each). The `SPRT` count and size do not change. A HUD frame that alternates icons and labels is the likely peak, so the re-size is measured, not estimated.
- `SPRT` UV is 8-bit, so a doubled `u + w` of exactly 256 is valid. The GPU reads texels u..255 of the page and never wraps. The asserts allow `<= 256`.
- PSX targets run only in chunk 5's gate, once per attempt.
