---
title: "Planet row icons copied from cached images (D6)"
description: "Every on-screen planet redraws its resource row (5 to 10 one-tile sprites, each wrapped in two palette-block copies) every frame, although the row changes only when the economy rebuilds it; each row should be drawn once into step 8's image cache and copied."
status: needs-triage
---

## Problem Statement

`planet_draw` ends by drawing the planet's resource row (needs, then
outputs) under the planet, through the row's `draw_at` method:
`layout_row_draw_at` walks the row's placed items and, for each sprite item
(`row_draw_at_item_draw`, "path 3" in the report):

1. copies the item's palette block into the draw palette
   (`hud_select_palette_block`, 16 bytes);
2. draws one 1×1 `p8_spr_default` at the item's offset;
3. copies block 0 back (16 bytes).

The row's pixels depend only on its items (sprite, palette block, offset)
and the sprite sheet. The row is rebuilt by `planet_refresh` when the
economy recomputes, and its content mostly comes back identical. Between
two rebuilds it is redrawn identically every frame; only its position
moves with the camera and the planet's shake.

The report (`docs/psx-perf-levers.md`, D6, "estimated", group `rowicons`)
puts this item draw at 830 hblanks a window on `draw-5564` (55 a frame,
43 sprites a frame) and 1,212 on `draw-6000` (81 a frame, 64.18 sprites a
frame), at the report's costs (spr 0.646 a call). Step 3.9's re-measured
unit costs (O3, asserts off, no meter, no filter) put spr at 0.448 a call.
Even at that cost the sprites alone are 19.3 / 28.8 a frame, above 2% of
the mean frame (10.6 hblanks, step 2.7).

## Solution

Each planet row made only of sprites is drawn once into a small image slot
of step 8's image cache, and each frame `planet_draw` copies that slot
where the row's first pixel would land. The cache is content-keyed: a row
rebuilt by the economy with the same items finds its old image and costs
no build. A pool of fixed-size slots holds the images; when it is full the
least recently used image is replaced.

A row that cannot be copied exactly (one holding a text item, a fractional
or negative offset, or wider than a slot) is drawn as today. The result is
pixel-identical by construction, and proven by unit tests, unchanged e2e
hashes and unchanged `psx-check` captures. Row images hold sprite texels
only, so even the raw capture references (bit 15 included) stay the same;
if one moves, the lever is reverted.

The step opens with a throwaway probe of the row's gross cost, decides
coverage on it, and is kept or reverted on a measured short-run delta
without the resume frame.

## User Stories

1. As the plan pilot, I want the gross cost of the planet rows measured on the six short runs before any code, so that coverage is decided on today's cost as the plan's rules require.
2. As the plan pilot, I want the gross figure cited next to the report's (55 / 81 a frame) and next to an estimate from the latest unit costs, so that the note follows the "cite both figures" rule.
3. As the plan pilot, I want a written rule for "covered" (gross under 10.6 hblanks a window frame on both draw runs), so that the decision is mechanical and matches steps 8 to 11.
4. As the plan pilot, I want the probe to leave the picture and the game state untouched, so that its runs are valid.
5. As the plan pilot, I want it checked before any code that a `psx-check` capture shows a planet row at 0 tolerance, so that the plan's "capture showing the cached element" rule holds.
6. As the plan pilot, I want every assumption about step 11's outcome checked against its note and HEAD, and recorded, so that no decision rests on an outcome that did not happen.
7. As the plan pilot, I want the largest row and the most rows drawn in one frame measured on PC before the slots are sized, so that the pool is sized on data rather than a guess.
8. As a player on PC, I want the resource icons under each planet to look exactly as before, so that the change is invisible.
9. As a player on PSX, I want the same icons as on PC, pixel for pixel, so that the platforms stay identical.
10. As a player, I want a row half off the screen edge to be cut exactly as before, so that edges show no seam or missing pixel.
11. As a player, I want a shaking planet's row to shake exactly as before, so that the fractional position rounds the same way.
12. As a player, I want greyed-out icons (palette block 7) to stay grey and lit icons to stay lit, so that unmet needs and spent outputs read as before.
13. As a player, I want icons that overlap inside a row (outputs drawn 2 or 3 pixels apart) to overlap as before, so that stacked outputs read the same.
14. As a player, I want a row holding a number ("x6") to look as before, so that large outputs still read.
15. As a player, I want a row to change on the very frame its content changes, so that a new route or a met need shows at once.
16. As a game developer, I want one function with the same contract as `layout_row_draw_at` that draws a row through the cache, so that `planet_draw` changes one call and nothing else.
17. As a game developer, I want that function to fall back to `layout_row_draw_at` for any row it cannot copy exactly, so that correctness never depends on the row's shape.
18. As a game developer, I want the image built by `layout_row_draw_at` itself, so that the item drawing code exists once.
19. As a game developer, I want images keyed by content, not by row object, so that the economy's frequent rebuilds of identical rows cost no image build.
20. As a game developer, I want the draw palette left as `layout_row_draw_at` leaves it, so that whatever is drawn next sees the same state.
21. As a game developer, I want the build to read no game state and draw no visual random number, so that it obeys the image cache's draw contract.
22. As a game developer, I want a game-side slot enum, created by this step, listing the background's slot and the row slots, so that no two modules can claim the same slot.
23. As a game developer, I want a compile-time check that the game's slot list and the runtime's slot count agree, so that adding a slot in one place only fails the build.
24. As a maintainer of the runtime, I want the new slots added through the existing slot list, each 48 wide (a multiple of the PSX FILL's 16), so that the image cache needs no new API.
25. As a maintainer of the PSX backend, I want the new slots packed in the free part of the (704, 256) half, on 16-unit bounds, each inside one 15-bit texture page, so that they overlap nothing and the copy code works unchanged.
26. As a maintainer of the PSX backend, I want the layout tests to prove the new slots lie inside the image region, overlap no other slot and fit their page, so that a later slot cannot collide with them.
27. As a maintainer of the PSX backend, I want a copy from a 48×8 slot that does not start on its page's left edge tested for its texture coordinates and its clipping, so that it is known right without an emulator.
28. As a maintainer of the PSX frame budget, I want `make state`'s `zone gpu_build` peak read after the change, and `GPU_BUILD_BUFFER_BYTES` raised by its own sizing rule if needed, so that a frame building the map and many rows cannot overflow.
29. As a maintainer of `psx-check`'s masked captures, I want each row image to hold only the spr family, so that the draw filter passes or drops each copy exactly where it passed or dropped the sprites it replaces.
30. As a maintainer of the e2e replays, I want every `*.sha256` and `make end-state` unchanged, so that the game is proven untouched.
31. As the maintainer of `psx-check`, I want every capture unchanged at 0 tolerance, so that PSX pixels are proven identical.
32. As the plan pilot, I want the lever's delta per part on all six short runs, with asserts mode and opt level as the runs report them, so that the note has the measured gain against the estimate.
33. As the plan pilot, I want the lever judged on the draw runs without their resume frame, so that the plan's one-off rule is applied.
34. As the plan pilot, I want the resume frame's build cost and the judge frame it lands on recorded, so that phase 4's spike list has it.
35. As the plan pilot, I want the recurring builds (rows whose content changed, rows coming on screen) counted per window and on the judge's spike frames, so that phase 4 knows what this lever adds to update-heavy frames.
36. As the PSX RAM budget's owner, I want the static cost (the software store's growth and the pool's keys) recorded with `ram-room`, `game-ram-room` and `gpu-ram-room`, so that the cost is visible.
37. As a developer running `make test` and `make e2e`, I want them no slower, so that the fast loop stays fast.
38. As a maintainer, I want a unit test that draws many rows both ways (cached and direct) across the screen edges and compares framebuffers, so that pixel identity on the software backend is proven without a replay.
39. As a maintainer, I want a unit test that counts image builds at the backend boundary, so that "same content, no rebuild" and "changed content, one rebuild" are proven.
40. As a maintainer, I want a unit test that drives more distinct rows than the pool holds, so that the eviction path is proven pixel-identical.
41. As a maintainer, I want a unit test proving the largest planet row the game can build fits a slot, so that the common case never falls back silently.
42. As the human who alone accepts capture references, I want every row image to hold sprite texels only, and the lever reverted if any raw capture reference changes, so that this step never needs a reference I have not accepted.
43. As a maintainer of the PSX layout, I want each row slot on its own texture page by the existing `x − x mod 64` rule, tested to lie wholly inside that page, so that slots past `x` 768 copy right.

## Step 11's outcome (H-11 checked)

Step 11 (D8) is done: its note is `### Step 3.11` in `docs/plan-21.md`, at
HEAD `3faa054`. D8 was built and measured (−63.6 draw a frame, pooled), then
**skipped and fully reverted** (`cc440fa`): the five 16×16 slots, the
game-side slot header and the `planet_draw` copies are gone. Every source
file at HEAD is as at `c6fb3b2`. Each hypothesis this PRD was written
against is recorded below with its outcome; the decisions already take
the outcome into account.

- **H-11-1 holds.** Step 11 is finished and its note is written. Every
  probe of this step takes its baseline at the HEAD it runs on.
- **H-11-2 does not hold** (D8 not kept): the runtime's slot list holds
  the map's `SLOT(256, 256)` only, and there is no game-side slot enum.
- **H-11-3 holds.** This step creates the game-side slot enum itself: the
  background's slot (today a `#define` of 0 in `background.c`) moves into
  it, with a compile-time check against `P8_IMAGE_SLOT_COUNT`. The row
  slots start at slot 1. Every slot is referred to by its enum name.
- **H-11-4 holds, (704, 256) branch.** The (704, 256) half is empty; the
  row slots' band starts at (704, 256). The page rule at HEAD is back to
  `x − x mod 64` for each slot (see "Texture pages" below).
- **H-11-5 holds.** `planet_draw` at HEAD is unchanged since `c6fb3b2`:
  the row is drawn after the pad circle, through the row's `draw_at`
  method at `shaken_pos + (0, 7)`, alignment 0.5.
- **H-11-6 holds.** E2e hashes, end states and capture references are
  unchanged at HEAD (the revert restored the code that produced them).
- **H-11-7, settled.** `GPU_BUILD_BUFFER_BYTES` is **10,240** at HEAD. Step
  11 needed 12,288 for the map and five 16×16 images on the same frame;
  this step's rows build more bytes on that frame, so a raise is
  expected, by the sizing rule below.
- **H-11-8 holds.** Step 11's note gives no new unit-cost table: step
  3.9's (spr 0.448 a call) is the one cited.
- **H-11-9 holds.** The threshold stays 10.6 hblanks.
- **H-11-10 holds.** The image cache's API, its filter rule and its copy
  code are as step 8 left them.
- **H-11-11 holds.** No newer `warn-review`; no prerequisite chunk.
- **H-11-12 and H-11-13** are checked when chunk 0 runs (probe tooling as
  in steps 8 to 11; `representative` still failing is recorded, not a
  blocker).
- **New constraint from step 11's note: the mask bit.** A copy writes each
  image texel raw, bit 15 included. D8's images held untextured black and
  circle pixels, which the build stamps with bit 15 where the direct
  primitives leave it clear: invisible on screen, but seven raw-VRAM
  capture references changed, and accepting references
  (`make psx-gpu-capture-accept`) is the human's command only. Sprite
  texels already carry bit 15 when drawn directly, so a sprite-only image
  copies the same raw values. D5's map image is sprite-only and changed
  no reference, which confirms it. See "Hard requirement" below.

## Implementation Decisions

### Coverage

- **D6 against the levers already taken:** F1, G1, B1, G2, B3, D5, D1/B4,
  and L1 (D8 was skipped and reverted in step 11). None removes or cheapens the row's
  sprites or its palette selects: D5 is the map, D1/B4 the outlined text
  (the row's rare text item only), L1 the dispatch walk, D8 the disc and
  ring. The report pairs D6 with no alternative; D7 was dropped by the
  report as covered by D2, D6 and D3. So D6 is expected not to be
  covered, and the measurement decides.
- **Gross target probe** (throwaway, uncommitted), `row-twice`: in
  `planet_draw`, the row's `draw_at` call is issued twice in a row with the
  same arguments. Drawing the same row twice gives the same pixels: every
  final pixel is the one the row's last writer of that pixel puts there,
  in both passes alike. The palette ends at block 0 both times. No visual
  random number is drawn. The delta is the row's whole cost (sprites,
  palette copies, the walk, and the rare text item).
  - Run with `make psx-probe` on all six short runs, against one
    `make psx-probe-baseline` at the HEAD the step starts from.
  - Prior art: `psx/probes/call-twice-cart-draw.patch`.
- **Rule** (steps 8 to 11's, recorded as a decision): D6 is covered when
  the gross is under 10.6 hblanks a window frame on both `draw-5564` and
  `draw-6000`. If covered, the step ends at chunk 0 with its note.
- **Figures cited next to the gross target:**
  - the report's: 830 / 1,212 a window, 55 / 81 a frame (`draw-5564` /
    `draw-6000`), at spr 0.646;
  - the unit-cost estimate, by step 3.9's table (step 11 gave none newer):
    gross at least 0.448 × sprites (19.3 / 28.8 a frame at the report's 43
    and 64.18 sprites; the palette copies and the walk are not in the
    table); net = gross − 0.448 × copies, one copy a cached row (the call
    profile files `image_draw` under spr). The sprite and row counts per
    run come from chunk 0's PC count or from `make frame-profile` on the
    same windows.

### Sizing count (chunk 0, before the slots)

- A throwaway PC-only patch, run on the PC replay of
  `worst-case/full_game.json` and of every `tests/e2e/` replay, counts per
  frame the planet rows drawn, how many of them hold only sprites with
  whole, non-negative offsets, and the widest such row's sprite extent
  (largest offset + 8). It writes only to the scratch output, nothing
  tracked.
- **Slot size: 48 × 8.** The largest planet row the game can build
  (`largest_slots.c`'s case: two wants, luxury output so two outputs,
  amount 5) has its last sprite at offset 36, so an extent of 44. Rows
  with an amount above 5 hold a text item and are never cached. Every
  item sits at `y` offset 0 and every sprite is 8 tall. If the count finds
  a wider cacheable row, the slot width becomes the next multiple of 16
  above the widest, and the note says so.
- **Pool: 32 slots.** It must hold every row drawn in one frame, with
  room, so that a frame never evicts what it just built. If the count's
  most cacheable rows in one frame exceed 24, the pool becomes the next
  multiple of 8 at or above 1.25 × that maximum, and the note says so.
- Memory at 32 × 48 × 8: 12,288 bytes of software store in every ELF
  that links the software backend (the PSX ones included); a key array
  of about 2.5 KB; VRAM 48 × 8 units a slot.

### Capture check (before any code)

- Expected to hold already: planets with an output carry a row, and
  `tests/e2e/pad_slipway.txt` shows planets at f100, f188 and f265, so
  captures `all 100`, `all 188` and `all 265` (mask `all`) and `spr 234`
  (mask `cls,spr`) should show rows.
- Chunk 0 confirms it from the PC dump of those frames: icon pixels at
  `y + 7` under a planet's centre. The note names the captures that show
  a row, and whether one shows a grey (block 7) icon.
- If none shows a row, a capture at a `pad_slipway` frame that shows one
  is added to `psx/gpu_captures.txt` at 0 tolerance (mask `all`), with its
  reference recorded, in its own commit, before chunk 1.

### Hard requirement: sprite texels only, capture references unchanged

- Every row image holds **only sprite texels**: it is built by
  `layout_row_draw_at` on a row whose items are sprites and nothing
  items, so the only draw calls inside a build are `p8_spr_default`
  (palette selects draw nothing). No fill, line, circle, rectangle, point
  or print ever enters a row image. The PSX build's clear (a black FILL
  to 0x0000) only leaves transparent texels the copy skips.
- Why it matters (step 3.11's note): a copy writes texels raw, bit 15
  included. A sprite texel already carries bit 15 when drawn directly, so
  the copy writes the very value the direct sprite wrote; an untextured
  pixel would gain bit 15 and change a raw-VRAM capture reference even
  with 0 differing pixels.
- The module enforces it: the cacheable check accepts only sprite and
  nothing items, and the build installs nothing else. A unit test draws
  a building row through a backend that records every call and asserts
  that each call inside the build is `spr`.
- **If any `psx-check` capture reference changes** (its raw hash, not
  only its pixels), the lever is **skipped and reverted** (chunks 3, 2
  and 1), and the note says which references moved. Accepting new
  references (`make psx-gpu-capture-accept`) is the human's command only;
  this step never runs it and never stages a reference file.

### What is cached, and what is not

- **Cached:** a row whose placed items are only sprite items and nothing
  items, with at least one sprite, every sprite at a whole, non-negative
  `x` offset and a `y` offset of 0, and whose extent fits the slot.
- **Not cached, drawn as today:** a row holding a text item (an amount
  above 5: "x6"), a row with a fractional or negative offset, a row wider
  than a slot. Keeping text out keeps every image to the spr family, so
  the draw filter passes or drops a copy exactly where it passed or
  dropped the row's sprites; a mixed image would be dropped whole under
  `cls,spr` or `cls,print` and change a masked capture.
- **Only planet rows.** Button rows, side tooltips, tooltips and message
  box articles keep `layout_row_draw_at`: they are D2's and D3's (step 15)
  and hold text.

### Why the copy is pixel-identical

- **Position.** `layout_row_draw_at` draws item `i` at
  `flr(p.x − a·w + offset_i)`, `flr(p.y)`. A whole offset adds exactly in
  fixed point, so that equals `flr(p.x − a·w) + offset_i`. The image holds
  item `i` at `(offset_i, 0)` and is copied at
  `(flr(p.x − a·w), flr(p.y))`, `a·w` computed with the same fixed-point
  multiply as `layout_row_draw_at`.
- **Build.** The image is built by calling `layout_row_draw_at` on the
  row itself with `p = (a·w, 0)` and the same `a`, so every sprite lands at
  `(offset_i, 0)` through the very code that draws it today. There is one
  item-drawing routine.
- **Palette.** Each sprite item selects its own palette block (whose
  entries carry the transparency bits) and restores block 0; nothing of
  the entry palette reaches a sprite. So the image depends only on the
  items and the sprite sheet. After a copy, the function selects block 0,
  as `layout_row_draw_at` leaves it after a row with a sprite.
- **Overlap.** Outputs sit 2 or 3 pixels apart and overlap; the build
  draws them in the same order, so the image holds the same final pixels.
  Pixels no sprite writes stay transparent in the copy; a black sprite
  pixel is written, so it stays opaque (the PSX mask bit, step 8).
- **Clipping and camera.** The copy is clipped to the screen and drawn
  under the camera, like each sprite. Step 8 proved this on both backends;
  this step's unit test proves it for rows on the software backend.
- **Fill pattern.** `spr` ignores the fill pattern on the software
  backend (it reads only the draw palette); the PSX backend matches (the
  captures). The images are therefore independent of the pattern. Chunk 2's
  test draws with a pattern in force to pin this; if it fails, a
  pattern in force sends the row to the direct path.
- **Sprite sheet.** Loaded once and never written by the game. The key
  includes the vram the image was built from, as the background's does.

### Module: row images (game, new)

- A new game module owns the pool and exposes one draw function with the
  contract of `layout_row_draw_at` (same arguments, same pixels, same draw
  palette after), taking a non-const graphics state because a build moves
  the camera. It draws through a slot when the row is cached, and calls
  `layout_row_draw_at` otherwise.
- It also exposes a predicate saying whether a row can be cached, used by
  its tests (largest row) and by chunk 0's count if convenient.
- **Key**, computed at each draw from the row's placed items (not stored
  in the row): the vram pointer, then one packed entry per sprite item
  (sprite number, palette block, `x` offset), in order. A 32-bit hash of
  it rejects most slots fast; a full compare confirms a hit. Rejected:
  storing the key in the row at construction. The economy rebuilds every
  planet's row on a recompute, so that would add work to the spike frames
  phase 4 must shrink, for a saving of a fraction of a hblank a row.
- **Lookup and eviction.** A hit refreshes the slot's use stamp (a
  counter). A miss takes an empty slot, or else the least recently used
  one, builds the image, records the key. Rebuilding a slot already
  copied earlier in the same frame is safe on both backends: the software
  backend copies at once, and the PSX build packets are chained in order
  with the frame's packets (the copy reads the old texels before the
  build overwrites them). The pool is sized so that this does not happen
  in practice.
- **Build size.** Each build is `extent × 8`, not the whole slot, so the
  copy draws only the row's pixels.
- **Contract comment** on the build callback: it draws no visual random
  number and reads no game state (the image cache's rule).

### Image cache: slots (runtime, PSX layout, game enum)

- The runtime's slot list gains the pool's `SLOT(48, 8)` entries after
  the map's, as slots 1 to 32. No API change.
- PSX layout: five slots a band of 8 lines, at `x` = 704, 752, 800, 848,
  896 (all on 16-unit bounds), bands going down from (704, 256). 32 slots
  take 7 bands, lines 256 to 311.
- **Texture pages: one per slot, by HEAD's rule.** Each slot's page is
  `(x − x mod 64, y − y mod 256)`, 256 × 256: 704, 704, 768, 832, 896
  for the five columns, all at `y` 256. Step 11's chunk 1 found this rule
  puts a slot at `x` 768 or beyond on a page other than (704, 256); that
  is correct, not a fault: `gpu_image_draw` already switches to the
  copied slot's own page (`psx_vram_layout_image_page(slot)`) for every
  copy and back afterwards, so slots need not share a page. The rule is
  kept unchanged. What must hold, and is tested for every slot, is that
  the slot lies wholly inside its page (`x − page.x + w ≤ 256`,
  `y − page.y + h ≤ 256`) and that the page's left edge is a multiple of
  64. A shared page was rejected: only one 48-wide slot fits between 704
  and 768 per band, which would take 32 bands for no gain (each copy
  pays its page switch either way).
- **The game-side slot enum is created by this step** (D8's was reverted
  with it): the background's slot, then a contiguous range for the row
  slots, with a compile-time check that the range matches the pool size
  and that the whole enum matches `P8_IMAGE_SLOT_COUNT`. The
  background's `#define` goes. If the human later merges step 11's
  pending D8 branch, its five slots and its own enum must be rebased
  onto this one; recorded for the human in the note.
- The cost model, the meter, the draw filter, the call profile and the
  timing wrapper are already per slot and need no change beyond the
  larger count.

### Game: `planet_draw`

- The row call becomes the row-image draw function, with `planet_draw`'s
  own graphics and vram (which are the dispatch context's on every path
  that reaches it: the draw pass and `ascension_draw`), the same position
  (`shaken_pos + (0, 7)`) and alignment 0.5. The `draw_at` method stays
  on the row class for every other caller. Everything else in
  `planet_draw` is unchanged and in the same order.

### GPU build buffer

- Builds land on: the first frame drawing planets (with the map's build), every frame a row comes on screen with content
  not in the pool, and every frame a visible row's content changes.
- After chunk 3, `make state`'s `zone gpu_build` peak is read. If the
  sizing rule in `gpu_sizes.h` (peak + 1/8, rounded up to 1 KiB) gives
  more than HEAD's `GPU_BUILD_BUFFER_BYTES` (10,240), it is raised to that figure
  in the same chunk, and the RAM cost (twice the increase, one buffer a
  frame parity) goes in the note. A raise is expected: step 11 needed
  12,288 for the map plus five 16×16 images on one frame, and a frame's
  rows build more than that. The note also gives a worst-case bound
  (pool size × the largest row's build bytes, plus the map's) for
  phase 4; no build budget per frame is added in this step.

### Flagged code

- None (no `warn-review` since step 2.7's). By the plan's rule, no prerequisite test or derived
  figure.

### Measuring and verdict

- **Before (chunk 0):** H-11-12 and H-11-13, the capture check, the sizing
  count, the gross probe, the coverage verdict.
- **The lever's delta (chunk 3):** measured with `make psx-probe`, using
  chunk 3's diff (the `planet_draw` switch, and the build buffer resize if
  any) as the patch, against `make psx-probe-baseline` at chunk 2's HEAD,
  on all six short runs, before chunk 3 is committed. Chunks 1 and 2 add
  slots and a module nobody calls; their delta is not separated.
- **Resume frame:** a resumed short run builds every on-screen row on its
  first window frame. By the plan's rule, the lever is kept or reverted
  on the window without that frame. The one-off cost is that frame's draw
  delta minus the steady per-frame delta, recorded with the judge frame it
  lands on (the first frame drawing planets in `full_game.json`: frame
  12, galaxy generation, unless the title draws planets first; the note
  says which).
- **Recurring builds** stay inside the steady delta. The note counts them
  per window (PC call profile or a throwaway counter) and on the judge's
  spike frames (`psx/perf/spike-frames.txt`), as phase 4 input.
- **One-frame update runs** (`start`, `update-4737`, `update-5500`,
  `update-5719`): their single frame is a resume frame, so their delta is
  recorded as build cost, not used for the verdict.
- **Keep or revert:** first, any changed capture reference reverts the
  lever whatever its gain (hard requirement). Then: kept when the pooled `draw-5564` + `draw-6000` draw
  delta without the resume frame is a gain. Otherwise chunks 3, 2 and 1
  are reverted: the row slots and the module serve nothing else, so they
  are no reusable facility. The note says so.
- **Note contents:** step 11's outcome as used here; the captures showing a row; the
  sizing count and the slot size and pool chosen; the gross target per run
  against 10.6, the report's figures and the unit-cost estimate; the
  measured delta per part per run, with asserts mode and opt level as
  reported, against the estimate; the one-off build cost and its judge
  frame; the recurring builds; `representative` failures; `make state`'s
  `zone gpu_build` and `zone gpu_frame` peaks before and after, and
  `GPU_BUILD_BUFFER_BYTES` if it moved; `ram-room`, `game-ram-room` and
  `gpu-ram-room`; e2e, `make end-state` and `psx-check` status.

## Implementation Chunks

0. **Gross target, capture check, sizing count** (no committed source
   change).
   - Check H-11-12 and H-11-13 (the others are settled above).
   - Confirm from the PC dump that a capture shows a row; if none does,
     add one (its own commit) as decided above.
   - The PC sizing count; fix the slot size and the pool.
   - `make psx-probe-baseline`, then `row-twice` on the six short runs.
   - Record the figures against 10.6, the report and the unit-cost
     estimate. If covered, the step ends here with its note.
   - Figures go in the progress commit's message, as steps 8 to 11 did.
1. **Row image slots** (no behaviour change).
   - Runtime slot list, PSX layout entries, the new game-side slot enum
     (background's slot and the row range) with its compile-time checks.
   - Tests: runtime (a 48×8 slot holding sprites copies like the sprites
     drawn directly, clipped at every screen edge), PSX layout (inside the
     region, no overlap with any slot, 16-unit bound, wholly inside its
     own page, page edge a multiple of 64), PSX copy placement for slots
     at `x` 752 (page 704, `u` from 48) and 800 (page 768, `u` from 32).
   - `make test-all` green; e2e hashes unchanged by construction.
2. **The row-image module** (not yet called by the game).
   - The predicate, the key, the pool with lookup and eviction, the build
     through `layout_row_draw_at`, the copy, the palette reset.
   - The tests below.
   - `make test-all` green.
3. **`planet_draw` draws its row through the module.**
   - The one-call switch.
   - `make test-all`, `make end-state` and `psx-check` green, every hash
     and capture unchanged; `make state` peaks read, build buffer resized
     only if its rule says so.
   - Every capture reference unchanged (raw hash), else skip and revert
    (hard requirement).
  - The `psx-probe` measurement of this diff, then keep or revert.
4. **Close.**
   - `make psx-short-runs` at the harness defaults, the RAM figures.
   - The step note in `docs/plan-21.md`.

## Testing Decisions

- **What makes a good test here:** the output is pixels. Tests draw
  through public calls (the row-image draw function,
  `layout_row_draw_at`, `p8_image_build`, `p8_image_draw`, `p8_spr`) on
  the software backend and compare framebuffers and the draw palette. They
  do not read the pool, its keys or its stamps. Build counts are observed
  at the backend boundary, by a counting backend installed with
  `p8_draw_backend_set` in front of the software one, the way the image
  cache's own guard wraps it.
- **Runtime (`tests/test_image.c`):** a 48×8 slot built with overlapping
  sprites under two palette blocks, copied at the centre, across each edge
  and a corner, equals the same sprites drawn directly on a striped
  background. Prior art:
  `test_a_partial_image_clips_at_its_edge_and_leaves_the_rest_transparent`,
  `test_a_built_map_copies_like_the_map_drawn_directly`.
- **PSX layout and placement (`tests/test_psx_vram_layout.c`,
  `tests/test_psx_image.c`):** every slot lies inside the image region, on
  a 16-unit bound, overlaps no other slot, and lies wholly inside its own
  page (`x − x mod 64`, `y − y mod 256`); a copy from the slot at `x` 752
  (page 704) gets `u` from 48, one from the slot at `x` 800 (page 768) gets
  `u` from 32 and that page, and both clip at the screen edge. Run on PC,
  as today.
- **Row-image module (`tests/test_row_image.c`, new, flat):**
  - **Pixel identity:** for planet rows built by
    `research_display_planet_resource_row_keys` (the largest row, a
    crowded three-wants row, a row with grey items, a row with "nothing"
    cells), drawn at the centre, across each screen edge and at
    fractional positions (shake), with alignment 0 and 0.5 and a non-zero
    camera, the framebuffer and the draw palette after the row-image draw
    equal those after `layout_row_draw_at` on an identical vram.
  - **Fallback:** a row with a text item (amount 6), and a hand-built row
    with a fractional offset, give the same framebuffer as
    `layout_row_draw_at`, and cause no build.
  - **Content key:** the same content drawn from a second row object
    causes no build; one changed item (a block 0 turned 7) causes one.
  - **Eviction:** more distinct rows than the pool holds, drawn in two
    rounds, each draw compared with the direct one.
  - **Largest row:** the largest planet row (`largest_slots.c`'s case) is
    cacheable.
  - **Fill pattern:** a row drawn with a pattern in force equals the
    direct draw.
  - **No game state:** the game and visual random states are unchanged by
    a building draw. Prior art:
    `test_vanishing_flicker_draws_from_the_visual_stream`.
  - Prior art for building rows: `tests/test_layout_row.c`
    (`test_planet_resource_row`).
- **Whole-game proof:** every `tests/e2e/*.sha256` unchanged, `make
  end-state` unchanged, `psx-check` green with every capture at 0
  tolerance (the PSX copy is proven only there).
- An existing unit test that froze the old call sequence (for example one
  counting a planet draw's `spr` calls) is updated, since the e2e replays
  are the truth.

## Out of Scope

- Button rows, side tooltips, tooltips and message boxes (D2, D3, step
  15), and the score panel readouts (D4, covered per step 3.9).
- Baking the row's text item, or a row into the planet's own image.
- D7 (skipping palette selects that change nothing): covered by D6 for
  path 3, per the report.
- U2 (rebuilding a planet row only when it changed): an update lever,
  phase 4. The content key makes D6 independent of it.
- A per-frame build budget, a sub-rectangle copy or an atlas in the image
  cache.
- Running the judge (`psx-full-game-check`): step 16 does it.
- Any measurement on the Miyoo or PCSX-ReARMed; audio.

## Further Notes

- **Timing-model caveat** (the report's label on D6): the copy moves work
  from the CPU to the GPU. pcsx-redux charges no GPU-side time and no
  cache miss, so the gain is real on the judge. The plan accepts that.
- **Per-copy overhead on PSX:** each copy emits a texture page switch to
  the image page, one sprite, and a switch back (step 8's
  `gpu_image_draw`). It is inside the measured delta; batching switches
  across planets is not attempted here.
- **Code read at HEAD (`c6fb3b2`) when this PRD was written:** the row
  is drawn last in `planet_draw` but for the targeting circle, through
  `self->layout_row`'s `draw_at` at `shaken_pos + (0, 7)`, alignment 0.5;
  `row_draw_at_item_draw` selects the item's palette block around each
  `p8_spr_default`; planet row items use widths 2, 3, 5, 6 and spacings
  −4, −3, −1, 0, 1, 2, all whole; palette blocks 0 and 7; an amount above 5
  adds a text item; the transparency bits live in the draw palette and
  each block select overwrites them; the map's slot is `0`, a `#define`
  in `background.c`; the image region's (704, 256) half is empty at HEAD.
  HEAD `3faa054` (step 11 reverted) has the same sources as `c6fb3b2`.
