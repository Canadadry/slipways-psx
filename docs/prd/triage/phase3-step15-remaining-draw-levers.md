---
title: "The remaining draw levers above 2% of the mean frame: D2, D3, D10, D9, D13, L3, G4"
description: "Seven small or partly blocked draw levers are left in plan 21's phase 3; the step measures what each can still remove on today's build, skips every one under 10.6 hblanks a frame or covered, builds the text of buttons, tooltips and the message box through the image cache only in a form that keeps every raw capture reference, and implements the others that clear the threshold."
status: needs-triage
---

## Problem Statement

Plan 21 step 15 asks for "the remaining draw levers above 2% of the judge's
mean frame, in the report's order: D2, D3, D10, D9, D13, L3, G4 (D2 and D3
through step 8's image cache)", each skipped and noted when covered or
under 2%. The 2% is of the mean frame in the latest `psx-full-game-check`
summary: step 7's (`### Step 2.7`), mean **530.2** hblanks, so the
threshold is **10.6 hblanks a frame** (530.2 × 0.02 = 10.60).

The report (`docs/psx-perf-levers.md`, "Lever entries") priced these levers
at O2, with the draw meter and the draw filter compiled in and the outlined
font drawn in nine passes. All four have gone since (B1, F1, G2, D1/B4).
Per window frame, `draw-5564` / `draw-6000`:

| Lever | Where | Report | Since then |
|---|---|---|---|
| D2 | `button_draw` rows and side tooltip, `tooltip_draw_at` rows | 313 / 49 | step 3.9 residual probe −43.9 / −8.9: not covered |
| D3 | `message_box_draw_overlay` articles | 268 / 0 | step 3.9 residual probe −36.1 / 0: not covered |
| D10 | `slipway_draw_line`'s two end points, computed for each of its two calls | 14 / 12 | game code; only G2 touched it |
| D9 | off-screen planets dispatched by `draw_layers` and returning early in `planet_draw` | 10 / 10 | L1 changed the dispatch loop (step 3.10) |
| D13 | `slipway_draw`'s two `entity_geom_is_on_screen` tests | 8 / 8 | game code; only G2 touched it |
| L3 | `camera_apply_method` once per drawn layer | ≤ 1 / ≤ 1 | as D13 |
| G4 | `upload_slots` in the PSX frame end (submit) | 6.7 / 6.0 (the whole upload piece) | submit −3.2% from G2 |

Two of them are also blocked in the form the plan names:

- **Step 3.11's lesson.** A copy from the image cache writes each texel raw,
  bit 15 included, and the cache builds with the mask bit forced on (black
  must carry bit 15 to stay opaque in a copy). An untextured `rect`,
  `rectfill`, fill pattern bar, line or circle drawn directly writes bit 15
  clear. Caching any of them changes the raw-VRAM reference hash of every
  capture that shows it, invisible on screen, and accepting a reference is
  the human's command only (`make psx-gpu-capture-accept`). D8 was
  reverted for exactly this.
- **D2 and D3 hold untextured content.** A button is a `rect`, two
  `rectfill` and a layout row; a side tooltip adds a `rectfill`; a tooltip
  is a fill-pattern bar and rows; a message box article is a bar
  (`hud_draw_bar`), a help frame, a sprite, an outlined text or a layout
  row. The HUD buttons (tech, score, year skip, pause) are on screen in
  every in-game frame, so the three `all` captures of `pad_slipway` (100,
  188, 265) show buttons. A whole-button or whole-box image would move
  their references: **D2 and D3 as "one image per button / per box" are
  skipped on references**, before any build.
- **Families.** The draw filter passes a copy only when the capture's mask
  holds every family its image was built from (step 3.8). An image holding
  both `spr` and `print` texels would be dropped whole from `spr 234
  cls,spr` and `print 275 cls,print`, where the direct path draws half of
  it: another reference change. A cached image must hold one family.

What is left of D2 and D3 is their text. A layout row's text item and a
message box's outlined text are drawn by `hud_print_outlined` →
`p8_print_outlined`: one textured `SPRT` a glyph from the outlined font,
through a CLUT built by `psx_colour_rgb555`, which sets bit 15 on every
opaque entry (the outline's black included). These are texels of the same
class as step 3.12's sprite rows, which kept every reference. Text was the
largest part of both levers in the report (D2: rows 4,268 of 4,689).
Whether outlined glyph texels really survive a build and a copy bit for bit
is not proven yet: step 3.12 proved it for sprites only.

## Solution

The step opens with one chunk of throwaway probes and host-only counts, no
committed source change, that decides every lever:

1. Every assumption about step 14's outcome is checked (H-14-1 to H-14-10,
   below).
2. A gross-target probe per lever, on the six short runs, measures what the
   lever can remove at most on today's build.
3. A lever under 10.6 a window frame on both `draw-5564` and `draw-6000`
   is skipped as "under 2% of the mean frame", with its figure; nothing is
   built for it.
4. For D2 and D3, a throwaway reference spike answers the one open
   question (do outlined glyphs keep their raw bits through the cache?)
   before anything is built in full.

Then each standing lever is built, measured with `make psx-probe` on its own
diff and kept or reverted on its draw (or submit) delta, in the report's
order. D2 and D3 are built only in their reference-safe form: **the text of
a button, a tooltip or a message box drawn from a content-keyed text image;
every fill, frame, bar, line and sprite drawn as today.**

Whatever is kept leaves the picture pixel-identical on PC and PSX, every
e2e hash, `make end-state` and every capture reference unchanged.

## User Stories

1. As the plan pilot, I want the 2% threshold stated with its source (step 2.7's mean 530.2, 10.6 hblanks), so that every skip in this step cites the same figure.
2. As the plan pilot, I want each of the seven levers' gross target measured by a throwaway probe on today's build, so that "under 2%" is decided on today's cost, not on the report's O2-with-meter figure.
3. As the plan pilot, I want each probe to leave the picture, the game state and the visual random stream untouched, so that its short runs are valid.
4. As the plan pilot, I want each probe's figure cited next to the report's figure and next to an estimate from step 3.9's unit costs, so that the note follows the plan's "cite both figures" rule.
5. As the plan pilot, I want a lever under 10.6 on both draw runs skipped with no source change, so that no effort goes below the plan's threshold.
6. As the plan pilot, I want D2 and D3's whole-element forms skipped on references with the reason written, so that nobody rebuilds a button or a box image that moves a capture reference.
7. As the plan pilot, I want the one class question (outlined glyph texels through a build and a copy) answered by a throwaway spike and `psx-check` before D2 is built in full, so that a reference change costs one throwaway build, not a whole chunk.
8. As the plan pilot, I want the spike's coverage proven (at least one capture frame draws a text item through it), so that a green spike is not a vacuous one.
9. As the plan pilot, I want a capture added at 0 tolerance when no existing capture shows a cached text item, so that the plan's "a capture shows the cached element" rule holds.
10. As the plan pilot, I want every assumption about step 14 written as a hypothesis and checked before chunk 0, so that no decision rests on an outcome that did not happen.
11. As the plan pilot, I want each built lever kept or reverted on its own measured delta, without the resumed run's first frame where it builds images, so that the image-cache rule of the plan applies.
12. As the plan pilot, I want each one-off build cost recorded with the judge frames it lands on, so that phase 4's spike list gets it.
13. As the plan pilot, I want D3 judged after D2 and D13 after D9, so that a lever is not credited with work the one before it removed.
14. As a player on PC, I want buttons, tooltips, the message box, slipways and planets to look exactly as before, so that the change is invisible.
15. As a player on PSX, I want the same pixels as on PC, raw bit for raw bit, so that the platforms stay identical and every capture reference holds.
16. As a player, I want a button label that changes (a price, a count) to show its new text on the frame it changes, so that the cache never shows a stale label.
17. As a player, I want a label drawn under a remapped draw palette (a faded or highlighted row) to show the remapped colours, so that the cache never shows the wrong colour.
18. As a player, I want a sliding menu's labels to slide with their buttons, so that a cached label is placed where the direct one would be.
19. As a player, I want the message box's per-article shake kept, so that a cached article line moves as before.
20. As a game developer, I want one text-image module with the same contract as `hud_print_outlined` (same pixels, same draw palette after), so that every caller can switch with one line.
21. As a game developer, I want the text image to fall back to the direct draw whenever it cannot copy exactly (string too long or too wide, clip not the full screen, fill pattern set, no free slot), so that correctness never depends on the cache.
22. As a game developer, I want a slot used earlier in the same frame never rebuilt in that frame, so that a copy never reads an image rebuilt under it.
23. As a game developer, I want the text image keyed on content (string, colour, resolved draw palette entries), so that equal labels share a slot and changed labels rebuild.
24. As a game developer (D10), I want `slipway_draw` to compute the two end points once and give them to both line passes, so that the fixed-point trigonometry runs once per slipway.
25. As a game developer (D9), I want off-screen planets skipped before their draw method is called, through an engine-level class flag and not a planet name in the engine, so that the engine stays planet-free.
26. As a game developer (D13), I want a planet's on-screen answer computed once per draw frame and reused by the slipways that end on it, so that the test is not repeated.
27. As a game developer (L3), I want the parallax camera offset of a layer reused while the camera and the parallax are unchanged, so that the computation runs once per move.
28. As a game developer (G4), I want the PSX frame end to skip uploading a CLUT or pattern slot whose VRAM already holds the same content, so that only changed slots are uploaded.
29. As a maintainer of the image cache, I want new slots declared in the runtime's slot table and the game's slot enum, placed in VRAM by the layout module with a test, so that the cache stays one facility.
30. As a maintainer of the draw filter, I want a text image to carry the `print` family only, so that `print 275` and the `all` captures pass it and `spr 234` drops it exactly as the direct glyphs.
31. As a maintainer of `make state`, I want the `gpu_build` peak checked after D2 and D3, and the build buffer raised only if the peak needs it, so that RAM is spent only when proven necessary.
32. As a maintainer of the e2e replays, I want every `*.sha256` and `make end-state` unchanged, so that the game is proven untouched.
33. As the human who alone accepts capture references, I want every existing reference unchanged at the raw hash, and a new capture left pending with its pre-lever hash recorded in the note, so that I accept nothing I have not reviewed.
34. As a developer running `make test` and `make e2e`, I want them no slower, so that the fast loop stays fast.
35. As a maintainer, I want unit tests comparing a cached and a direct draw on the software backend at fractional positions, edges, remapped palettes and every fallback, so that the text image's exactness is proven on PC.
36. As a maintainer, I want unit tests for each of D10, D9, D13, L3 and G4 that pin the unchanged output, so that each is proven pixel- or upload-identical without an emulator.
37. As the PSX RAM budget's owner, I want `ram-room`, `game-ram-room` and `gpu-ram-room` recorded after the step, so that a growth from new slots is visible.
38. As the plan pilot, I want the note to list for each of the seven levers which rule decided it and with which figures, so that step 16 and the human see why the step delivered what it did.
39. As the human merging `claude/plan-21-d8-pending` later, I want new image slots appended after the row slots, so that rebasing D8's five slots stays a mechanical change.

## Step 14's outcome (hypotheses, checked before chunk 0)

Step 14 (B2, or D11 + D15; D12 skipped; PRD
`docs/prd/triage/phase3-step14-offscreen-calls-or-batched-calls.md`) is
running when this PRD is written. Chunk 0 checks each hypothesis against
`docs/plan-21.md` (its `### Step 3.14` note) and HEAD, and records the
outcome. If one fails, the decision it names is adapted as said; nothing
else changes.

- **H-14-1.** Step 14 is finished and `### Step 3.14` is written at the HEAD
  this step starts from. Every probe takes its baseline there (`make
  psx-probe-baseline`; the fingerprint refuses a stale one).
- **H-14-2.** Step 14 kept B2, or D11 and/or D15, or nothing. Either way
  this PRD holds:
  - If **D11** was kept, `slipway_draw_line` fills a buffer of segments and
    issues one batched call. D10's probe and change apply to the end-point
    computation ahead of that buffer, unchanged in substance.
  - If **B2** was kept, the runtime rejects wholly off-screen `pset`,
    `line`, `rect`, `rectfill`, `spr` and image copies before the backend,
    and the call profile and draw timing no longer count those calls. A text
    image copy off screen is then rejected in the runtime too; nothing here
    depends on it.
  - If **D15** was kept, nothing here changes.
- **H-14-3.** The per-family unit-cost table is step 3.9's (`print` 1.522 a
  call and 0.268 a glyph, `spr` 0.448, `rectfill` 0.190, `rect` 0.314).
  If `### Step 3.14` gives a newer one, it is cited instead.
- **H-14-4.** No `psx-full-game-check` ran since step 7: the threshold stays
  **10.6**. (Step 16 runs the next one.)
- **H-14-5.** E2e hashes, `make end-state` and every capture reference at
  HEAD are those step 14 kept unchanged (its hard requirement), so this
  step's baselines are valid.
- **H-14-6.** Step 14 left `button_draw`, `tooltip_draw_at`,
  `layout_row_draw_at`, `message_box_draw_overlay`, `draw_functions`,
  `hud_print_outlined`, `planet_draw`, `draw_layers`, `camera_apply`,
  `upload_slots`, the image runtime, the image slot enum and the VRAM
  layout unchanged. If one changed, chunk 0 re-reads it before writing a
  probe.
- **H-14-7.** The draw backend table may have gained `lines` and/or
  `psets` entries. This step adds no backend entry (the text image uses the
  existing image entries), so no wrapper table changes.
- **H-14-8.** No `warn-review` ran since step 2.7's. None of the code it
  flagged (`asserts_enabled()` duplicates, the O3 default, hand-copied
  condition counts, the GPU replay's `#ifdef` stubs, the Makefile) is
  extended here: no prerequisite chunk. Adding a line to
  `psx/gpu_captures.txt` and its ceiling uses the existing
  conditions-from-file mechanism (step 3.8).
- **H-14-9.** The probe tooling (`make psx-probe`, `make
  psx-probe-baseline`, `psx/probes/`) works as in steps 8 to 14;
  `representative` still fails on every short run, recorded, not a
  blocker.
- **H-14-10.** The image slot enum still ends with the 32 row slots, and
  step 14 added no slot. D8's pending branch is not merged.

## Implementation Decisions

### The threshold and the rules

- **Threshold: 10.6 hblanks a window frame** (2% of step 2.7's mean frame,
  530.2). The note says "10.6, step 2.7".
- **Under 2%:** a lever whose gross-target probe is under 10.6 on both
  `draw-5564` and `draw-6000` is skipped (steps 8 to 14's rule). The
  one-frame runs (`start`, `update-*`) are recorded, not used. For G4 the
  part read is submit, for the others draw.
- **Covered:** a lever whose target an earlier lever of this step removed is
  re-probed after that lever and judged on the residual. This applies to
  D3 after D2 (only if D2 changed code that D3's articles reach; with the
  site split below it does not, so D3's chunk-0 probe stands) and to D13
  after D9 (D9 removes the off-screen planets' tests, not the slipways'; D13's
  probe stands). The report's alternatives (D1/B4 against D2 and D3) were
  already settled by step 3.9's residuals; they are not reopened.
- **Keep:** a built lever is measured with `make psx-probe` on its own diff
  against a baseline at the HEAD before it, all six runs. It is kept when the
  pooled `draw-5564` + `draw-6000` delta a frame of its part is a gain,
  **without the resumed run's first frame** where it builds images (plan
  rule). Otherwise it is reverted and noted.
- **References:** a lever that moves any existing capture's raw reference is
  reverted whatever its gain. This step never runs `make
  psx-gpu-capture-accept` and never edits `psx/gpu_capture_references.txt`.
- **Order:** D2, D3, D10, D9, D13, L3, G4, each built only if standing.

### Chunk 0: probes (throwaway, uncommitted)

Each probe is a patch run with `make psx-probe` on the six short runs
against one `make psx-probe-baseline` at the starting HEAD. None moves the
picture, the game state or `visual_random`. Prior art:
`psx/probes/call-twice-cart-draw.patch`; steps 3.11 and 3.12's "drawn
twice" probes. "Twice" draws the same pixels over themselves (idempotent).

- **`d2-text-twice`:** in `button_draw` (its row and its side tooltip's row)
  and in `tooltip_draw_at` (its rows), every text item of the row drawn a
  second time with the same call. The delta is the text's whole cost: D2's
  gross target in its reference-safe form.
- **`d3-text-twice`:** in the message box's articles, every outlined text
  (layout-row articles' text items and the draw functions'
  `print_outlined_entry` reached from a message box article) drawn twice.
  D3's gross target in its reference-safe form.
- **`d10-endpoints-twice`:** in `slipway_draw_line`, the two end points
  computed a second time into a `volatile` sink. The delta is one
  computation per call: what D10 removes (one of the two calls'
  computations a slipway).
- **`d9-offscreen-dispatch-twice`:** in `draw_layers`, an entity whose class
  draw method is the planet's and which is off screen
  (`entity_geom_is_on_screen` false) is dispatched a second time
  (`entity_pos` and the indirect call; the planet returns at its first
  test). An upper bound of D9 (the probe pays one extra on-screen test the
  lever keeps).
- **`d13-visibility-twice`:** in `slipway_draw`, the two
  `entity_geom_is_on_screen` tests evaluated a second time into a
  `volatile` sink. D13's gross target.
- **`l3-camera-twice`:** in `draw_layers`, `apply_camera` called twice per
  drawn layer. L3's gross target.
- **`g4-upload-twice`:** in the PSX frame end, `upload_slots` called twice.
  The submit delta is the whole upload piece: an upper bound of G4, which
  removes only the uploads of unchanged slots.

The note gives, per lever, the two draw runs' figures a window frame, the
report's figure, and an estimate from the unit costs where one applies (for
D2 and D3: text items × (`print` call 1.522 + 0.268 a glyph) from the PC
counts below). The report's figures make most of D10, D9, D13, L3 and G4
likely to fall under 10.6; the probes decide.

### Chunk 0: host-only counts (PC, nothing tracked written)

From a throwaway instrumented PC build replaying `worst-case/full_game.json`
(the short-run windows and the whole game) and every e2e replay, output in
`tmp/`:

- D2 and D3 sites: text items a frame, distinct strings a frame, the widest
  item in pixels, the longest string, the maximum number of text items in
  one frame; how many are drawn under a non-identity draw palette, a clip
  or a fill pattern.
- Which `pad_slipway` capture frames (100, 188, 234, 263, 265, 275, 276,
  300, 314) draw at least one text item at a D2 site (D3: at a message box
  article).
- Off-screen planets dispatched a frame, slipways drawn a frame (to turn D9
  and D13 into per-call figures).

### D2 and D3: what is skipped and what can be built

- **Skipped on references, without a build:** a whole button, tooltip or
  message box drawn from one image (the plan's literal D2/D3), and any
  image holding a `rect`, `rectfill`, fill-pattern bar, help frame or
  sprite. The note cites step 3.11.
- **Buildable:** each outlined text item at D2's and D3's sites drawn from a
  text image, everything else at those sites drawn as today, in the same
  order. Sprite items of a mixed row stay direct `spr` (one `SPRT` each;
  a mixed-family image would be dropped whole by masked captures).
- **Sites.** D2: the layout rows drawn by `button_draw` (the button's row
  and its side tooltip's row) and by `tooltip_draw_at`. D3: the message box
  articles' outlined text (layout-row articles' text items, and
  `print_outlined_entry` when reached from a message box article). No other
  caller of `layout_row_draw_at`, `draw_functions` or `hud_print_outlined`
  changes: D4 (score panel readouts) stays skipped, covered (step 3.13), and
  routing it now would be a lever outside this step.
- **How the sites switch.** `layout_row_draw_at` gains a sibling with the
  same contract whose text items go through the text image; the D2 sites
  and the message box's layout-row article call it. `draw_functions` gains
  a row-drawing sibling whose `print_outlined_entry` goes through the text
  image; `message_box_draw_article` calls it. No module-level flag selects
  the path.

### D2 and D3: the reference spike (before building in full)

Decision on how to verify cheaply, recorded as asked: **a throwaway build
and one `psx-check`, before any committed code.**

- **Precondition (host, free):** chunk 0's counts show at least one capture
  frame drawing a text item at a D2 site. HUD buttons are on screen in every
  in-game frame; the planet menu (a message box of buttons) is open over
  `pad_slipway` f231–f237 (`spr 234`). If no frame qualifies, chunk 1 first
  adds `all 233 all` (menu open, between `pattern 190` and `spr 234`) with
  ceiling 0. Its reference stays pending human acceptance; the spike and
  every later run compare its raw hash with the one taken at the starting
  HEAD, which the note records.
- **Spike patch (uncommitted):** at D2's sites, each text item built into a
  free row slot (48×8) and copied, rebuilt at every use, with no key and no
  reuse; items wider than a slot, or beyond the 32 slots in a frame, drawn
  direct. Built with the GPU replay's draw filter on, as `psx-check`
  requires.
- **Verdict:** `psx-check` green with `gpu-capture-ceilings` and
  `gpu-capture-references` ok (and the added capture's raw hash equal to
  its pre-spike hash) → outlined glyph texels survive a build and a copy;
  D2 and D3 are built. Any reference change → D2 and D3 are both skipped on
  references, citing the spike, and the step moves to D10.
- The spike also shows whether the build buffer holds a burst of text
  builds (`gpu_build` peak in the PC `make state`, and no `packet`
  overflow).

### The text image (D2, then reused by D3)

A game module next to `row_image`, deep and small:

- **Interface:** one call with `hud_print_outlined`'s integer contract:
  string, integer x and y (the position `hud_print_outlined` passes to
  `p8_print_outlined`, alignment already applied), colour. Same pixels and
  same draw palette after as the direct call. A query "would this be
  copied" for tests, as `row_image_cacheable`.
- **Key:** the string's bytes (copied into the slot record, at most a fixed
  length), the colour, and the resolved draw palette entries of that colour
  and of 0 (the outline). Equal keys share a slot; a changed label misses
  and rebuilds on the frame it changes.
- **Image:** built at the text's own origin minus the outline's one pixel,
  so the copy's top-left is the direct draw's top-left; built through
  `p8_image_build`, family `print` only.
- **Fallback to the direct draw** when: the string is longer than the key
  allows or wider than a slot; the clip rectangle is not the full screen;
  a fill pattern is set; or every slot was used this frame.
- **Eviction:** least recently used among slots **not used in the current
  frame**. A slot used earlier in the frame is never rebuilt in it (a copy
  must not read an image rebuilt under it). The frame is told by a counter
  the cart's draw advances, not by `g_frame` reads inside the module.
- **Slots:** appended after the 32 row slots in the runtime's slot table
  and the game's enum (`IMAGE_SLOT_TEXT_FIRST`, `IMAGE_SLOT_TEXT_COUNT`).
  Count and width come from chunk 0's counts: count = the largest number of
  text items in one frame at D2 + D3's sites, rounded up to a multiple of
  8, at most 32; width = the widest item's image width rounded up to 16, at
  most 128 (wider items draw direct); height 8. The note records the
  chosen figures and the share of text draws that fall back.
- **VRAM and RAM:** placed by the PSX VRAM layout module, each slot on one
  texture page, with its existing test extended; the software store grows by
  count × width × 8 bytes (both PSX ELFs pay it, as step 8 decided). The
  note records `ram-room` and `game-ram-room`; if either would fall under
  64 KiB, the count is halved and the fallback share recorded.
- **Build buffer:** `GPU_BUILD_BUFFER_BYTES` is raised only if `make
  state`'s `gpu_build` peak over the full game and every e2e exceeds it,
  to the peak rounded up to 1 KiB.

### D10 (if standing)

`slipway_draw` computes the two end points (the end positions plus their
`vec_from_polar` wobble from `state_time`) once and passes them to both
line passes; `slipway_draw_line` takes them instead of recomputing. Its
public contract (same lines, same order, same colours) stays; its callers
are `slipway_draw` and tests. If step 14 kept D11, the end points feed its
segment buffer the same way (H-14-2).

### D9 (if standing)

- The engine stays planet-free: `entity_class_t` gains a flag "skip at
  dispatch when off screen"; `draw_layers` tests it and calls
  `entity_geom_is_on_screen` before the indirect call, for the draw message
  only.
- Every planet-family class (the 0x60000 layer archetypes: planet,
  structure, trading hub, processor, ascension, protostar, synthesizer)
  sets it; no other class does (L4, not measured, stays out of scope).
- `planet_draw` keeps its own test as its public contract for direct
  callers. To avoid paying the test twice on on-screen planets, the class's
  draw method calls an internal path that trusts the engine's cull; the
  report's risk holds (the early return already precedes `visual_random`).

### D13 (if standing)

- A planet's on-screen answer is memoised for the current draw frame (a
  frame stamp and a flag on the planet, set at the first query in the
  frame). Slipways are on layer 2 and planets on layer 6, so a slipway
  queries first and the planet's own test (or D9's cull) reuses the answer.
- The memo is read only during the draw (the camera does not move inside a
  draw); update-time callers (`planet.c`, `particle.c`, state dumps) keep
  the plain test.

### L3 (if standing)

`camera_apply` reuses the last computed parallax offset for a layer when
the camera position, its shake and the parallax are unchanged since it was
computed; 15 cached offsets. Same `p8_camera` calls as today.

### G4 (if standing)

The PSX backend remembers, per CLUT and pattern VRAM slot and per frame
buffer of the double-buffered caches, the content it last uploaded, and
skips `queue_load` for a slot whose content is unchanged. The comparison
and the record are a pure function tested on PC. A wrong skip shows a wrong
palette, which the captures catch; any capture change reverts it.

### What is not changed

- The cost model, the draw filter's rule, the image runtime's contract,
  `row_image` and the 32 row slots.
- `circ`, `circfill`, lines, fills, bars and frames at D2's and D3's sites.
- Line numbers in the report are the report's; the functions named here
  are what counts.

## Implementation Chunks

0. **Hypotheses, probes, counts, decision** (no committed source change).
   - Check H-14-1 to H-14-10; record each outcome.
   - `make psx-probe-baseline` at the starting HEAD; the seven probes.
   - The host counts.
   - Per lever: figure against 10.6, report's figure, unit-cost estimate;
     standing or skipped.
   - If D2 or D3 stands: the reference spike (and, first, the added capture
     if the counts require it; that capture is chunk 1's commit).
   - Figures go in the progress commit's message, as steps 8 to 14 did.
     If nothing stands, the step goes to chunk 6.
1. **Capture** (only if the counts showed no capture frame with a D2 text
   item): `all 233 all` in `psx/gpu_captures.txt` with ceiling 0, its
   pre-lever raw hash in the commit message; `psx-check` green with the
   reference pending.
2. **Text image and D2** (if D2 stands and the spike passed): the module,
   its slots in the runtime, the game enum and both backends' layouts, the
   row-drawing sibling, D2's three sites switched; tests; `make test-all`,
   `make end-state`, `make state`, `psx-check` (every reference unchanged);
   `make psx-probe` on the diff; keep or revert; one-off builds on the judge
   (PC count of build frames over `full_game.json`) recorded.
3. **D3** (if D3 stands, after chunk 2 kept the module; if chunk 2 was
   reverted for a gain reason, D3 brings the module itself; if for a
   reference reason, D3 is skipped citing it): the draw-functions sibling,
   the message box's articles switched; same gates and measurement.
4. **D10, D9, D13** (each only if standing; one commit each, in this order).
   Same gates; `make psx-probe` per diff.
5. **L3, G4** (each only if standing; one commit each). Same gates.
6. **Close.** If a lever was kept: `make psx-short-runs` at the harness
   defaults, `ram-room`, `game-ram-room`, `gpu-ram-room`. The step note in
   `docs/plan-21.md` under `### Step 3.15`: the threshold and its source,
   each lever's decision with its figures, the hypotheses' outcomes, the
   spike's verdict, one-off build frames for phase 4.

## Testing Decisions

- **What makes a good test here:** the output is pixels, and on PSX the
  primitives and uploads emitted. Tests draw through public calls on the
  software backend and compare framebuffers, or call PSX placement and
  layout functions compiled on PC. No test reads a module's internals.
- **Text image (`tests/test_text_image.c`, new; prior art
  `tests/test_row_image.c`):** cached and direct draws give the same
  framebuffer at fractional and negative positions, at every screen edge,
  under a non-zero camera, over a non-black background, under a remapped
  draw palette; a changed string or colour rebuilds; equal keys share a
  slot; each fallback (too long, too wide, clip set, fill pattern set,
  every slot used this frame) draws direct and gives the same pixels; a slot
  used this frame is never rebuilt in it; the draw palette after equals the
  direct call's; nothing touches `visual_random`.
- **Sites (`tests/test_layout_row.c`, `tests/test_message_box.c`, and a
  button test next to them):** a button
  with a mixed row and a side tooltip, a tooltip, and a message box with
  every article kind give the same framebuffer through the new path as
  through today's calls drawn from the same state.
- **Slots and layout (`tests/test_image.c`, `tests/test_psx_image.c`,
  `tests/test_psx_vram_layout.c`):** the new slots have their declared sizes, each
  lies on one texture page and overlaps nothing.
- **D10 (`tests/test_slipway.c`):** a slipway drawn at several
  `state_time`s gives the same framebuffer as today's code path reproduced
  in the test.
- **D9 (`tests/test_entity_registry.c` and a planet test):** an off-screen
  planet's draw method is not called; an on-screen one is called once with
  its position; a class without the flag is always dispatched; the frame
  and `visual_random` are unchanged.
- **D13:** the memoised answer equals `entity_geom_is_on_screen` across
  camera moves between frames; a new frame recomputes.
- **L3 (`tests/test_camera.c`, `tests/test_draw_layers_camera.c`):** the `p8_camera`
  offsets set per layer equal today's across camera moves and shakes.
- **G4 (a PSX-side test compiled on PC, prior art
  `tests/test_psx_gpu_peaks.c`):** the upload record skips an unchanged slot
  per buffer and uploads a changed one, the first use of a slot, and after
  a reset.
- **Whole-game proof:** every `tests/e2e/*.sha256`, `make end-state` and
  `make state`'s zones unchanged (`gpu_build` and `gpu_frame` peaks
  recorded); `psx-check` green, every capture at 0 tolerance and every
  existing raw reference unchanged.
- No test is added for a lever chunk 0 skips. A unit test that froze old
  behaviour and breaks while every e2e hash holds is updated (CLAUDE.md).

## Out of Scope

- D2 and D3 as whole-element images, and any cached image holding
  untextured content or more than one family (references; step 3.11).
- Routing any other outlined text through the text image: D4 stays skipped
  (step 3.13); the HUD, score panel and planet labels are not touched.
- Caching the sprite items of mixed rows; changing `row_image`.
- L4 (culling other classes at dispatch), L2, D7, D11, D12, D14, D15, B2:
  other steps or skipped by the report.
- Accepting capture references; merging `claude/plan-21-d8-pending`.
- Running `psx-full-game-check`: step 16 does it.
- Measuring on the Miyoo or PCSX-ReARMed; audio.

## Further Notes

- **Expected outcome, not a decision:** at step 3.9's unit costs, D2's text
  on `draw-5564` (eleven labelled buttons) is of the order of its residual
  −43.9 and stands; on `draw-6000` it is under 10.6. D3's text is a part of
  its −36.1 and likely stands on `draw-5564` alone. D10 (report 14 / 12)
  is borderline after G2; D9 (10 / 10), D13 (8 / 8), L3 (≤ 1) and G4
  (≤ 6.7 submit, an upper bound) are likely under 10.6 and skipped after
  their probes. A step that builds only D2 and D3, or nothing, is a valid
  outcome.
- **Why the text image and not the row image:** a row image is one image
  per row; a button row mixes sprites and text, so as one image it would
  be dropped whole by `spr 234` and `print 275`. Per-item text images keep
  every masked capture's content equal.
- **Why D3 may lose its gain:** a message box article whose number changes
  every frame (a counting score) rebuilds its slot every frame; the build
  costs more than the direct draw. The measured delta decides, and the
  fallback does not try to predict it.
- **Timing-model caveat:** the copies are GPU-side work the judge does not
  charge (report risk for D2 and D3); D10, D9, D13, L3 and G4 remove
  instructions only.
- **Code read at HEAD `07d9906`** when this PRD was written: `button_draw`
  draws a `rect`, two `rectfill`, then its row and its side tooltip's
  `rectfill` and row through `layout_row_draw_at_default`; layout rows hold
  text items (drawn by `hud_print_outlined_default` at +2, +1) and sprite
  items; `message_box_draw_overlay` shakes each article by a camera offset
  and draws shared rows (`draw_functions_draw_row`), layout rows or scalar
  rows; `row_image` caches sprite-only rows in 32 slots of 48×8, LRU
  without a same-frame guard; `slipway_draw` tests both end planets on
  screen, then calls `slipway_draw_line` twice, each recomputing both end
  points; `draw_layers` walks the kept layer order and dispatches every
  entity; `planet_draw` returns first on an off-screen planet; slipways
  are on layer 2 and planets on layer 6; `upload_slots` queues every CLUT
  and pattern slot each frame.
