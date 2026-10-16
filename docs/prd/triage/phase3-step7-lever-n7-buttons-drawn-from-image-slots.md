---
title: "Lever N7: buttons drawn from image slots"
description: "Every button costs a rect, two rectfills, its sprites and its label's text copies each frame, although its look changes only on hover, enable or a refresh. On the tech panel and the menu that is a measurable share of the frames over 263. Each button's look should be built once into an image slot and copied each frame, with the same pixels, the same picture and every e2e hash unchanged."
status: needs-triage
---

## Plan step (verbatim)

Plan 22, phase 3, step 7 (`docs/plan-22.md`, section "3. Phases and steps",
as committed at `3d3daca`):

> 7. Each lever of step 5's ranked list, in its order, one step note each:
>    measured gain against its estimate, pixel-identical captures, unchanged
>    e2e hashes. After each lever, its measured gain replaces its estimate
>    in `docs/psx-levers.md` and, once two levers of the same estimation
>    basis (`calls` or `share`) are measured, the median of their
>    measured-to-estimated ratios scales the remaining estimates of that
>    basis before re-ranking (the ratio and the levers it rests on are in
>    the note); the projected steady frames still over are
>    recomputed (step 5's calculation) and the remaining levers re-ranked
>    if the order changes. Whenever a `psx-full-game-check` replaces
>    `psx/perf/full-run-frames.txt`, the projection is rebased on that
>    table: every lever already in the measured code is marked `in-base` in
>    the lever table (kept for the record, no gain applied), and the note
>    gives the measured steady frames over next to the last projection,
>    before and after the rebase. When the projection reaches zero steady frames
>    over, `psx-full-game-check` confirms `real-game-budget` green before the
>    phase stops early. (plan-pilot: one PRD per lever, or per group of
>    small levers that touch the same code.) Each lever's PRD lists the
>    open warn-review warnings (phases 1 and 2) that fall in the files it
>    changes, and decides each one: acted on in that PRD, or left with its
>    reason. The step note records which. A lever measured on
>    `psx-full-game-check` is first probed (`psx-probe`) on a run that
>    draws its target function — a representative run of another situation
>    if one does, else its best-try run. If that probe shows no gain on the
>    target function, the lever is reverted with no full-game check.
>    Consecutive levers of this kind in rank order then share one
>    `psx-full-game-check`, at most three per check, the last group sharing
>    step 8's. Each situation's mean update and draw are compared with the
>    previous full-game table (`make situation-report` on both). A lever's
>    measured gain is that per-situation delta, split between the group's
>    levers in proportion to their probe gains. If a group shows no gain in
>    the situation its levers target, it is split and re-measured one lever
>    at a time.

This PRD covers **only lever N7**, "buttons drawn once through the image
cache" in `docs/psx-levers.md`. At `3d3daca` the ranked order is N1, N2,
N3, D3, G4 (all taken and measured), then N7, then N6. N5 was skipped
(under 2%, step note `### Step 3.7 — N5`). Everything before N7 is
committed, so this PRD has no hypotheses on an earlier lever. N6 gets its
own PRD.

N7 is the **third `calls`-basis lever to be measured**, after D3 (ratio
0.42) and N3 (0.77). Their median, 0.60, is the current `scale calls` line,
and it scales N7's estimate alone. Once N7 is measured, the line becomes
the median of three ratios. No `estimated` `calls` lever remains after N7,
so the new line scales nothing. It is still written, for the record (G4
re-derived `scale share` the same way).

## Problem Statement

A button (`button_draw`, `src/game/ui_button.c`) is drawn from scratch every
frame:

- one `rect` (the colour-1 border);
- two `rectfill`s (the body: colour 12 when hovered or on the pad tab, 13
  when clickable, 5 otherwise; then its top line, colour 6 or 5);
- its layout row: each sprite item a `spr` under its palette block, each
  text item a copy from the text image cache;
- for a button with a side tooltip, one more `rectfill` and that row too.

The look changes only when the button is hovered, its row is rebuilt by a
refresh, or the button is created again. Yet the game pays for every call
in every frame, and on PSX each call is a trip through the backend, the
clip and the packet allocator.

The game draws buttons on almost every frame:

- the four HUD icon buttons (tech, score, year skip, pause; 11×11, one
  sprite each) on every in-game frame;
- the four title buttons;
- up to seven text buttons more on the tech panel, the planet panel and
  the menus, many with a side tooltip.

At most 11 buttons are drawn in one frame (full game and three e2e
replays), with at most 7 text buttons and at most 5 icon buttons.

`docs/psx-levers.md` sizes `button_draw_method` at 7.3 h per steady frame:
25.6 h a frame on `tech-panel-5895`, 11.8 h on `menu-5954`, 5.4 h on
`map-move-1057`. Its estimate, from call counts, is: tech-panel 11.1 h
(3.4% of draw), menu 4.0 h (1.4%), map-move 2.3 h (1.0%), map-rest, build
and score-panel 2.1 h (0.8%). Scaled by 0.60, it removes 12 steady frames
over 263 by itself.

For the player, the menu and the tech panel stutter partly because the
console redraws every button piece by piece on every frame.

## Solution

Each button's look is built once into an image slot and copied each frame:

- a button whose look can be copied exactly is drawn through a new
  **button image cache**: it is built into a slot on the first frame it is
  seen, then copied as one image per frame;
- the look is keyed by what decides it: the identity of the button's row
  and side-tooltip row contents, its body colour and its top-line colour.
  A hover change or a refresh is a new key, so it gets its own build;
- anything the copy cannot reproduce exactly (a fill pattern in force, a
  remapped draw palette, a fractional width or offset, a box too large for
  a slot, no free slot this frame) draws directly, exactly as today.

The pixels are the same by construction. The image holds exactly the pixels
the direct draw writes, translated to the button's floored position, and
pixels it never wrote stay transparent. On PSX the copied rects and fills
carry the mask bit, as every image copy does. So the raw VRAM of a capture
can change while every pixel's colour stays the same. That is the lever's
known "capture reference" risk.

Before the change, `psx-check` gains two captures, accepted at `differ 0` on
today's code:

- a `rectfill` capture on an in-game frame, since the current one (f11)
  draws only button fills, which N7 moves into images;
- an `all` capture of the tech panel on `pad_message_box`, so the screen-space
  text buttons with side tooltips are proven on the GPU.

After N7, every capture must still be at `differ 0`. The references of the
labels whose raw capture changes are accepted again under the agent rule.

N7 is measured with `make psx-probe` against a fresh baseline, on its three
representative runs, with **`tech-panel-5895` as its measuring run**
(instead of `map-move-1057`). Its measured gain replaces the estimate, the
`scale calls` line is re-derived, and the projection is recomputed. If N7
is not a gain, it is reverted and noted.

The player sees the same game, the same pixels, and fewer dropped frames on
the tech panel and in the menus.

## User Stories

1. As a player with the tech panel open, I want its buttons drawn as one copy each, so that tech-panel frames fit 263 hblanks more often.
2. As a player in the pause menu, I want the menu's buttons copied rather than redrawn, so that the menu stays at 60 fps.
3. As a player moving the camera, I want the four HUD buttons to cost one copy each, so that the map-move frames gain what they can.
4. As a player hovering a button with the pad, I want it to light up on the same frame as today, so that the feedback is unchanged.
5. As a player, I want a button whose label changes after a year end to show its new label on the same frame as today, so that no stale button is ever drawn.
6. As a player, I want a button with a side tooltip (a cost, an income) drawn exactly as today, including the one-pixel gap between the button and its tooltip, so that nothing visible changes in pass 1.
7. As a player, I want the planet panel's buttons, drawn in world space at fractional positions while the panel slides, placed exactly as today, so that sliding panels look the same.
8. As a player, I want a button drawn under a fill pattern or a remapped palette (if the game ever does) to look exactly as today, so that a rare state never shows a wrong button.
9. As a player, I want every button pixel identical on PC and PSX, so that the two platforms stay the same game.
10. As a player, I want the game itself unchanged, so that my game plays out exactly as before.
11. As the plan pilot, I want N7 checked against "covered" and "under 2%" on its remaining cost, measured, before any capture or code work, so that a skip costs nothing.
12. As the plan pilot, I want the residual measured with a throwaway probe that draws every button twice, so that the verdict rests on today's code (after N2's inlining).
13. As the plan pilot, I want the "under 2%" verdict judged by step 5's two rules with the thresholds N5 used, so that it uses the same rules as the ranking.
14. As the plan pilot, I want an in-game `rectfill` capture added and accepted on today's code, so that the rectfill family stays proven when the title's button fills move into images.
15. As the plan pilot, I want an `all` capture of the tech panel added to the message-box list and accepted on today's code, so that screen-space text buttons with side tooltips are proven on the GPU.
16. As the plan pilot, I want the list of captures that draw buttons written down, so that the coverage claim can be checked.
17. As the plan pilot, I want every capture of both lists at `differ 0` after N7, and the changed references accepted only under the agent rule, so that pixel identity is observed, not argued.
18. As the plan pilot, I want a fresh probe baseline after the capture commit, so that the measured delta compares against the code N7 starts from.
19. As the plan pilot, I want N7 measured on `tech-panel-5895` rather than `map-move-1057`, so that its expected gain is above the run's noise floor.
20. As the plan pilot, I want the ratio pooled over the three representative runs against their fixed window estimates, so that one noisy run does not decide it.
21. As the plan pilot, I want each situation's gain set to its unscaled estimate × N7's ratio, so that the projection uses the same sizing as the estimate.
22. As the plan pilot, I want `game-state same` on every probed run, so that the change is proven not to touch the game.
23. As the plan pilot, I want N7 reverted when it is not a gain, so that the plan's revert rule holds.
24. As the plan pilot, I want the `scale calls` line re-derived as the median of D3's, N3's and N7's ratios, so that the table records the evidence even though no `calls` lever is left to scale.
25. As the plan pilot, I want the projection recomputed and the remaining levers re-ranked if the order changes, so that N6 is confirmed next.
26. As the plan pilot, I want no `psx-full-game-check` for N7, so that the phase's full-game check stays at step 8.
27. As the plan pilot, I want the open warn-review warnings in N7's files listed and decided, so that the amended step 7 is followed.
28. As the plan pilot, I want the new slots' VRAM room, the software store's bytes, `ram-room` and `game-ram-room` reported, so that the memory cost of N7 shows.
29. As the plan pilot, I want the build buffer's peak (`zone gpu_build`) checked under `make state`, and resized by its own rule if the button builds push it over its margin, so that no PSX frame can overflow it.
30. As the plan pilot, I want the slot sizes of rows and buttons still to fit (`largest-slots`, `make state`), so that the row's new field costs nothing hidden.
31. As the plan pilot, I want the text image cache's pressure after N7 reported, so that D3's concern about the 32 text slots is answered.
32. As a developer, I want the button image cache as its own small module with a narrow interface, so that its slot and eviction rules are tested without a game.
33. As a developer, I want the cache to never rebuild a slot that was copied earlier in the same frame, so that the GPU never copies a slot that was overwritten later in the frame.
34. As a developer, I want a test that draws every kind of button (icon, text, sprite and text, side tooltip, no on-click, hovered, pad tab) through the cache and compares it pixel for pixel with the direct draw, at screen edges, under cameras and at fractional positions, so that a wrong box or offset is caught on the host.
35. As a developer, I want a test that a refreshed row (new contents, same address) rebuilds, so that a stale image is never copied.
36. As a developer, I want a test that a fill pattern, a remapped palette, a too-wide box and a full pool all draw directly, so that every fallback is pinned.
37. As a developer, I want a test that a build touches neither random stream, so that the image cache cannot change the game.
38. As a developer, I want the VRAM layout test to pin where the new slots go, so that no slot overlaps another or crosses its page.
39. As a developer, I want `make test` and `make e2e` no slower beyond noise, so that the fast loop stays fast.
40. As a reviewer, I want the step note to say what N7 measured, against what, the scale it sets, the captures and references touched, and why it was kept or reverted, so that the decision can be traced.

## Implementation Decisions

### Checks before any work (plan rules)

- **Covered?** No lever taken touches the button's own calls:
  - D3 routed the message box's text through the text image cache (button
    labels already went through it, by plan 21's D2);
  - N1 changed slipway lines, N3 the PSX circle, G4 the CLUT and pattern
    uploads;
  - N2 inlined the fixed-point and vector helpers, which lowered
    `button_draw`'s own code cost but left its calls.

  So N7 is not covered. The residual measures what N2 left.
- **Under 2%?** Measured with a throwaway probe, as N5 did, **before** any
  capture or code work:
  - the patch makes `button_draw_method` call `button_draw` twice (same
    pixels: every call is opaque and redraws the same box; the text copies
    hit the same slots);
  - it runs with `make psx-probe` on all 12 short runs, against the
    baseline valid at `HEAD` (`3d3daca`; N5 changed no code, so the one
    re-taken at N3's commit still matches);
  - the residual is the window draw delta a window frame. It counts the
    whole button, the part N7 keeps as one copy included, so it is an
    **upper bound**, stated as such.
- **The two rules**, with the figures N5 used:
  - **Rule (a):** the residual per situation, from map-move-1057,
    menu-5954, tech-panel-5895, map-rest-5581 and build-5752 (the last two
    best tries, weaker), and slipway-aim and score-panel from the pooled
    representative residual. These are weighted by steady frames ÷ 4,276,
    against T = 4.4 h.
  - **Rule (b):** each situation against its threshold: map-rest 6.2,
    map-move 7.0, slipway-aim 6.6, build 6.2, menu 6.2, tech-panel 7.3,
    score-panel 7.2.
  - Either rule holds → go on. Neither → N7 is **not taken**, nothing is
    committed but the bookkeeping, recorded as N5's was. The lever's
    `lever` and `gain` lines are removed and its entry is kept as a short
    record out of the numbered ranking.
  - Step 5 measured 25.6 h a frame on tech-panel (rule (b) threshold 7.3),
    so a skip is not expected.

### What N7 changes (chunk 2)

**Button image cache, a new game module `game/button_image`.** It is a
generic, small, content-keyed slot cache, modelled on `text_image`, that
knows nothing about buttons beyond its key:

- **Key:** four 32-bit words (the decisions below put the main row's
  serial, the side-tooltip row's serial, the body colour and the top-line
  colour in them). Two equal keys must mean the same pixels.
- **Interface:**
  - one call draws the image of a key at a world position under the
    camera. It takes the image's width and height and a draw function with
    its context (the `p8_image_draw_t` shape):
    - when a slot of the right pool holds the key, it copies it;
    - else it builds the key into the least recently used slot of that pool
      not copied this frame, then copies it.

    It returns 1 when it drew, and 0, drawing nothing, when the size is over
    the largest slot or every slot of the pool was copied this frame. The
    caller then draws directly.
  - one call starts a draw frame (as `text_image_frame_begin`).
- **Pools:** an image at most 16 wide goes to the **icon pool**, at most
  112 wide to the **wide pool**; both are 11 high. The HUD's four icon
  buttons never evict a text button's image, nor the reverse.
- **Rules carried from `text_image`:**
  - a slot copied earlier in the frame is never rebuilt in it (the GPU runs
    the frame's copies after the CPU has queued every build);
  - the build draws through `p8_image_build`, so the camera is restored and
    nested builds or copies are refused by the runtime;
  - the build reads no game state and draws no random number.
- **Header contract:** what the call returns, the pool sizes, and that the
  caller owns the key's meaning. No other comment (comment rules).

**Slots.** 18 new p8 image slots, appended after the planet ring slots, so
no existing slot number, size or VRAM place moves:

- `image_slot.h` gains `IMAGE_SLOT_BUTTON_ICON_FIRST`, `_COUNT = 8` and
  `IMAGE_SLOT_BUTTON_WIDE_FIRST`, `_COUNT = 10`.
- `P8_IMAGE_SLOTS` appends 8 × `SLOT(16, 11)` and 10 × `SLOT(112, 11)`.
  Both widths are multiples of 16 and every side is at most 256, as the
  runtime requires. The existing compile-time check ties the two counts.
- **Why these sizes:** a button's box is 11 rows (border at `top` and
  `bottom + 1`). The widest box in every e2e replay and the full game is
  100 px (a button plus its side tooltip). Icon buttons are 11 wide.
- **Why these counts:** at most 5 icon buttons and 7 text buttons share a
  frame. Eight icon slots hold the four HUD buttons lit and unlit. Ten
  wide slots hold a full tech panel plus three hover looks.
- **PSX VRAM place** (`psx_vram_layout.c`), all inside the images region's
  free room, each on 16-unit bounds and inside its own 15-bit page:
  - icon slot k at (784 + 16k, 440), in the free 176×16 strip right of the
    square slots;
  - wide slot k at (704 + 112·(k mod 2), 456 + 11·(k div 2)): five bands of
    two in the free 256×56 block, the last band ending at row 510.
  - `docs/psx-game.md`'s VRAM table row for `images` gets the two new
    groups and the free room that is left.
- **Software store:** 18 slots add 1,408 + 12,320 = 13,728 bytes to the
  static image store linked into every ELF. `ram-room` and `game-ram-room`
  are reported before and after (145,868 / 302,036 at N3's end).

**Layout rows get a serial.** `layout_row_t` gains a 32-bit `serial`,
written from a process-wide counter (starting at 1) every time a row's
items are filled. This covers both `row_create` and the in-place
`layout_row_init`. 0 means "no row".

- **Why:** a row's contents never change after it is filled. A refresh
  (for example the HUD tech button's) builds a new row, possibly at the
  freed row's address. A serial tells a new row from an old one in one
  compare. Content keys (texts, items) would cost tens of compares per
  button per frame, and the gain is only a few hblanks. A pointer key would
  copy a stale image when a freed row's address is reused.
- The serial is not game state: it is never saved, never in `make state`'s
  output, and never read by the game's logic.
- **Size:** `layout_row_t` grows by 4 bytes (or 0 with padding). The `ui
  row` and `ui button` slot sizes are guarded by `make state` and
  `psx-check`'s `largest-slots`. If either fails, the size grows by its
  existing rule, and the commit says so.

**`button_draw` routes through the cache.** Its signature and contract
don't change: the same pixels, the same draw palette and pen colour after.

- **Cacheable when all of these hold** (otherwise the direct path, today's
  code, unchanged):
  - no fill pattern in force (`fillp_pattern == 0`): a pattern is indexed
    by world coordinates and an image would bake one phase;
  - the draw palette in force is palette block 0. A new hud helper answers
    this with one compare of the 16 bytes against block 0. The direct draw
    leaves block 0 after a sprite item and touches nothing otherwise, so
    both paths leave the same palette;
  - the button's width, the side tooltip's width and every item's x offset
    are whole numbers, and every item's y offset is 0. Then every pixel the
    direct draw writes is at the floored draw position plus a constant;
  - the image box fits a pool (at most 112×11).

  Every button in the e2e replays and the full game meets all four
  (traced on `3d3daca`: 46,205 button draws, every one whole, unpatterned,
  under block 0, at most 100 wide).
- **Image box:** the union of every call's box, relative to
  `(flr(x) − 1, flr(y))`:
  - the border;
  - the body and top line;
  - the side tooltip's fill;
  - each text item's outlined box (one pixel left of and above its print
    position, 4 per character plus 1 wide, 7 high);
  - each sprite's 8×8.

  It is computed from the rows once per draw (a handful of integer adds),
  never stored.
- **Key:** (main row serial, side row serial, body colour, top-line colour).
  - The body colour (5, 12 or 13) carries the hover and pad-tab state.
  - The top colour (6 or 5) carries whether the button is clickable.
  - Width, items, texts, side fill colour and palette blocks all come from
    the two rows, which the serials name.
- **Build:** the draw function draws today's calls at the draw position
  `(1, 0)` (image origin `(0, 0)` = the border's top-left). Its text items
  go through `layout_row_draw_at`, not the text image path: a copy inside a
  build is a fatal error, and `text_image`'s contract guarantees the two
  give the same pixels. So today's body becomes one static function with a
  "text through the text cache / direct" choice, used by the direct path
  and by the build.
- **Copy:** at `(flr(draw_pos.x) − 1, flr(draw_pos.y))` under the current
  camera. Buttons drawn in world space (the planet panel, camera ≠ 0, x and
  y often fractional while sliding) work the same, since flooring a whole
  offset commutes.
- **Draw frame:** `cart_draw` calls the cache's frame start next to
  `text_image_frame_begin`.
- `button_draw_overlay` (the hovered tooltip) is not changed.

**Text image slots (D3's pressure):** a cached button no longer copies its
labels from the text cache: they are inside its image. So N7 *frees* text
slots on the tech panel and the menus (up to two labels a text button) and
never takes one. The button and text pools are disjoint, so the two caches
never compete. When a button falls back to the direct path (pool full), its
labels use the text cache as today. G4 skips unchanged CLUT and pattern
uploads. A button build uses the CLUTs its sprites always used, on the
build frame only, and a copy uses none. So N7 can only lower the CLUT
traffic G4 handles.

**Build buffer.** A frame that opens a panel builds up to seven buttons,
labels and sprites included, into the image build buffer
(`GPU_BUILD_BUFFER_BYTES`, 13,312; `make state` peak 11,256 on `attract`).
`make state` measures the new `zone gpu_build` peak.

- If it eats the 1/8 margin, `GPU_BUILD_BUFFER_BYTES` is resized by its
  own rule (peak + 1/8, rounded up to 1 KiB), and `ram-room` must stay
  green.
- Capping builds per frame was considered and rejected: it adds a rule
  whose only effect is to move work to the next frame, and the existing
  sizing rule already covers it.

**PC is unchanged in picture:** the software backend's copy is exact, so
every e2e `.sha256` and `.end-state` must stay as they are.

### Captures (chunk 1, before the change)

**What already draws buttons in `psx-check`.** Traced on `3d3daca`
(throwaway prints in a scratch worktree; replay frame = draw frame + 2
after the two loading frames):

- `pad_slipway`:
  - **every in-game frame** draws the four HUD icon buttons (11×11, sprite
    icon, body 13 unless lit), at (0, 0), (63, 0), (150, 0) and (0, 109).
    So `all 102`, `all 190`, `all 235` and `all 267` cover them;
  - **f234–239** draw the planet panel's three buttons in world space
    (camera 218, 276; fractional x and y): one with no on-click (body 5,
    text only), one hovered (body 12) with a side tooltip, two texts and
    four sprites, one unhovered (13) with a side tooltip. `all 235` covers
    them, and `spr 236` draws their sprites;
  - **f1–11** (title) draw the four difficulty buttons (63 wide, text and
    sprite, one lit). Only `rectfill 11` lands there, and **every rectfill
    of f11 is a button's**;
  - `rect 302` draws the HUD borders and four other rects; `spr 236` the
    HUD icons, the panel's sprites and 21 other sprites; `print 277` no
    button text (the HUD buttons have none).
- `pad_message_box`:
  - `print 142`, `all 143` and `all 164` draw the four HUD buttons only;
  - the tech panel (R1, f154–156: 4 HUD + 5 panel buttons, side tooltips,
    a body-5 header, screen space) and the pause menu (f105–119) are not
    captured.

**After N7, the draw filter drops a whole button in a family capture.** A
copy is drawn only when the mask holds every family its build used: `rect`,
`rectfill`, `spr`, plus `print` for a labelled button. This happens on PC
and PSX alike, so `differ 0` still holds, but:

- `rectfill 11` would draw nothing but `cls`. Its family call count stays
  (the filter counts a copy's families), so `gpu-capture-list` still
  passes, but the capture would prove nothing about `rectfill`;
- `rect 302` and `spr 236` lose the button parts and keep their other calls.

**Decisions:**

- **Add `rectfill 300 cls,rectfill`** to `psx/gpu_captures.txt`, in frame
  order between `line 278` and `rect 302`.
  - f300 draws 25 rectfills outside buttons (map panel fills and bars).
  - It is not a 32nd frame or the last, not next to `display 14`, and its
    family's call is on screen.
  - `rectfill 11` stays: it is accepted, cheap, and still proves a filtered
    copy is dropped the same way on both sides.
- **Add `all 155 all`** to `psx/gpu_captures_message_box.txt`, between
  `all 143` and `all 164`.
  - f155 shows the tech panel's five buttons over the year box, in screen
    space, with side tooltips. These are N7's main target (tech-panel) and
    its kind of button that `pad_slipway` only shows in world space.
  - It is not a 32nd frame (160 is) or the last.
- **Accept both on today's code**, before N7, as N3 did (agent rule,
  `docs/psx-game.md` § Acceptance):
  1. run `make psx-check`;
  2. `make psx-gpu-capture-ceilings LABEL=rectfill`, then `LABEL=all
     CAPTURES=message_box`;
  3. run `make psx-check` again, green;
  4. `make psx-gpu-capture-accept` for the same two.

  Each new capture must be at `differ 0`. If one is not, the mismatch
  predates N7: N7 stops, and it goes to `fix-bug` as its own bug.
- If a GPU replay passes its vsync or timeout limit (`PSX_GPU_CEILING`,
  `PSX_GPU_TIMEOUT`, `PSX_MESSAGE_BOX_GPU_CEILING`,
  `PSX_MESSAGE_BOX_GPU_TIMEOUT`), the limit is raised by the capture's
  measured cost and the commit says so.
- `docs/psx-game.md`'s "Capture list" bullet gains one sentence for the
  `rectfill` pair. The "Second capture replay" bullet gains `all 155`, the
  tech panel's buttons.
- Chunk 1 is committed on its own and **stays even if N7 is reverted**:
  both captures check today's code too. It is skipped only if the residual
  probe says "under 2%".

**References after N7.** No new ceiling is written after chunk 1.

- These labels may change their raw capture:
  - on `pad_slipway`: `all` (the HUD copies carry the mask bit), `rectfill`
    (f11 loses the title's fills), `rect` and `spr` (they lose the button
    parts);
  - on `pad_message_box`: `all` (HUD, and the tech panel at f155).
- A label whose captures no longer match their reference is accepted
  again with `make psx-gpu-capture-accept LABEL=<label>` (`CAPTURES=message_box`
  for the second list), **only when every capture of that label reports
  `differ 0`** (the agent rule).
- A capture above `differ 0` is a pixel-identity failure: N7 is reverted,
  never accepted.
- The note lists every label re-accepted and why.

### Measurement and bookkeeping

- **Order of operations:**
  1. Residual probe on the current baseline → verdict. If under 2%, stop
     at the bookkeeping (see "Checks before any work").
  2. Chunk 1: commit the captures.
  3. `make psx-probe-baseline` (all runs valid). Chunk 1 changes `psx/`,
     which is in the fingerprint.
  4. Tests first (red on today's code where they pin the cache), then the
     change.
  5. Measure N7 **before** committing it: its diff against `HEAD` is the
     `psx-probe` patch.
  6. Commit only if kept.
- **Measuring run: `tech-panel-5895`, not `map-move-1057`.** The lever table
  names map-move-1057, but there N7 is estimated at 2.3 h a frame (1.4 h
  scaled), under that run's noise floor (`max(3 h, 1% of the window)`,
  3.3–3.5 h in the earlier notes). On tech-panel-5895, where ten buttons a
  frame are drawn, it is 11.1 h (6.7 h scaled), clearly above its floor.
  Menu-5954 (4.0 h, 2.4 h scaled) is in between. N7's `lever` line gets
  `measure tech-panel-5895`, and the note says why.
- **Probe:** `make psx-probe PATCH=<N7 diff>` on all 12 runs, every one
  valid, `game-state same`.
- **Ratio** (fixed here, before measuring), pooled as N1's was:
  - N7's ratio = Σ (− window draw delta a frame) on map-move-1057,
    menu-5954 and tech-panel-5895 ÷ (2.3 + 4.0 + 11.1 = 17.4 h);
  - each run's own ratio is reported too.
  - The estimates are the unscaled window figures of the lever entry, the
    basis D3's and N3's ratios used.
- **Measured gains:** each N7 `gain` line becomes its unscaled estimate ×
  N7's ratio: tech-panel 3.4, menu 1.4, map-move 1.0, map-rest, build and
  score-panel 0.8 (% of draw). Slipway-aim keeps none, as estimated.
- **Weaker evidence, reported only:** map-rest-5581, build-5752 and the
  one-frame runs' deltas, against 2.1 h (0.8%) where they apply.
- **Keep rule:** N7 is kept when all of these hold:
  - tech-panel-5895's window draw delta and total delta are negative;
  - no run's total delta is positive beyond its noise floor;
  - every run is `game-state same`;
  - every capture is at `differ 0`.

  Otherwise N7 is reverted (never committed), and every run's deltas go in
  the note. Build frames (a panel opening, a hover change) are inside the
  windows, so their extra cost is in the deltas.
- **`scale calls`:** the median of D3's 0.42, N3's 0.77 and N7's ratio
  (floored at 0; a reverted N7's ratio still counts, since it was
  measured). It scales no remaining lever. The note gives the three ratios.
- **Lever table** (`docs/psx-levers.md`):
  - kept: N7's line becomes `measured`, `measure tech-panel-5895`, with its
    six gains;
  - reverted: its `lever` and `gain` lines are removed and its entry kept
    as a short record out of the ranking, as N5's;
  - N7's entry gets the residual, the per-run probe figures, the ratio, the
    slot geometry, the captures and the references re-accepted;
  - the "What each function gets" row for `button_draw_method` says N7
    taken (or reverted) with its figure.
- **Projection:**
  - run `make lever-estimate` after the table edit and paste its block in
    place of the current one, with `order ok` or the re-ordered and
    renumbered list;
  - update the closing paragraph's figures and `docs/psx-game.md`'s "Still
    to do" figure (379 / 530 at `3d3daca`);
  - the note gives the steady frames over after N7, nominal and
    pessimistic, against N5's projection.
- **No `psx-full-game-check`**, no rebase, no `in-base` mark: N7 is measured
  on short runs, and step 8 is the phase's check.
- **`psx-check`:** green, all 39 conditions, with these requirements:
  - every capture of both lists at `differ 0` and matching its (possibly
    re-accepted) reference;
  - `largest-slots` green;
  - `gpu-cost` and `game-gpu-cost` within their meters;
  - `make raylib` still links.
- **`make state`:** green: every zone and cap within its 1/8 margin,
  `gpu_build` included (resized as above if needed).
- **Fast loop:** `make test` and `make e2e` wall times before and after, in
  the note. Neither may be slower beyond run-to-run noise.
- **`docs/psx-game.md`:**
  - the image-cache paragraph lists slots 70–77 (16×11, button icons) and
    78–87 (112×11, text buttons), built by `button_image.c`, keyed by row
    serials and colours;
  - the VRAM table row (new places, free room left);
  - the two capture sentences;
  - the "Still to do" figure.
- **Step note** `### Step 3.7 — N7` in `docs/plan-22.md`, after N5's. It
  gives the residual and verdict, the captures and their acceptance, the
  baseline, the probe deltas per run (draw, update, submit, total,
  `game-state`, `text`), the ratio and the `scale calls` value, kept or
  reverted and why, the re-accepted labels, the slot and RAM figures, the
  `gpu_build` peak, the text-slot pressure, the fast-loop timings, the
  warn-review decisions below, and any deviation.

### Open warn-review warnings in N7's files

N7 changes these files:

- code: `src/game/ui_button.c`, `src/game/button_image.{c,h}` (new),
  `src/game/image_slot.h`, `src/game/layout.{c,h}`, `src/game/hud.{c,h}`,
  `src/game/cart.c` (one call in `cart_draw`), `src/p8/image.h`,
  `src/platform/psx_vram_layout.c`, possibly `src/p8/gpu_sizes.h` and the
  ui zone sizes;
- tests: `tests/test_button.c`, `tests/test_button_image.c` (new),
  `tests/test_psx_vram_layout.c`, `tests/test_image.c`, and the link lines of
  tests that link `ui_button.c`;
- the Makefile (a test target, a source list entry, link lines; possibly a
  GPU replay limit);
- `psx/gpu_captures.txt`, `psx/gpu_captures_message_box.txt` and their
  ceiling and reference files;
- `docs/psx-levers.md`, `docs/plan-22.md` and `docs/psx-game.md`.

The open warnings are the end-of-phase notes of `docs/plan-22.md` (phase 1,
13; phase 2, 13), plus plan 21's, which plan 22 still governs.

Decisions:

- **`cart.c` (phase 2):** it holds a whole `button_t` as the difficulty
  carrier, copies its fields one by one, and duplicates `cart_update60`'s
  prologue and epilogue. **Left open.** N7 adds one call in `cart_draw`, next
  to `text_image_frame_begin`, and touches neither the carrier nor
  `cart_update60`. That carrier is never drawn, so it never reaches the
  button cache.
- **`loading_screen.c` in `ENGINE_SRCS` (phase 2):** **left open.**
  `button_image.c` goes into `ENGINE_SRCS` next to `text_image.c`, where
  every game source of the replay already sits. This extends the pattern
  the warning names, so the note says so, for step 9's list. Renaming or
  splitting the list is a Makefile-wide change outside a lever.
- **The other Makefile warnings** (phase 1's duplicated situation lists,
  inline 263 / 31 / 15 / 3, two variables for one table, steady-profile's
  untested recipe lines; plan 21's 3,000-line Makefile): **left open.** N7
  adds a test target, a list entry and link lines, none of them in those
  recipes.
- **E2e frame numbers inline everywhere (phase 2):** **left open.** The two
  new capture lines use the capture list's own `<label> <frame> <mask>`
  format, which is the only way that file names a frame.
- **Hand-copied condition counts (plan 21):** **left open.** The captures
  change `gpu-captures`' printed counts, which nothing copies, and not
  `psx-check`'s 39 conditions.
- **"One frame late" vs F−2 wording in `docs/psx-game.md` (phase 2):**
  **left open.** N7 edits the image-cache paragraph, the VRAM table row,
  two capture sentences and the "Still to do" figure, not the "Display
  capture" bullet or the loading-screen text.
- **Every other warning: left open**, because it is not in a file N7
  changes: `situation.c`, `pad_author.c`, `psx_gpu_replay.c`,
  `psx_gpu_peaks`, the Lua and awk scripts (the capture lists name masks by
  family, not by the `0x8000` sentinel), the O3 default, and
  `asserts_enabled()`.
- `ui_button.c`, `layout.{c,h}`, `hud.{c,h}`, `image_slot.h`, `image.h`,
  `psx_vram_layout.c` and the tests above carry no open warning.

## Implementation Chunks

1. **Residual, then captures.** Run the residual probe and record the
   verdict; if "under 2%", go straight to chunk 3's skip bookkeeping. Else
   add `rectfill 300 cls,rectfill` and `all 155 all` (message-box list),
   accept both at `differ 0` on today's code (ceilings, green `psx-check`,
   references), and add the two `docs/psx-game.md` sentences. Commit. This
   chunk stays if N7 is later reverted.
2. **N7 itself.** Re-take the probe baseline. Then build, tests first:
   - the row serial in `layout`;
   - the `button_image` module and its 18 slots (runtime slot list, game
     slot enum, PSX VRAM places, layout and slot-count tests);
   - the hud palette check;
   - `button_draw`'s cacheable path and the shared body;
   - the frame start in `cart_draw`.

   Run `make test-all` and `make state`. Measure with `psx-probe PATCH=`,
   then run `psx-check`, re-accepting changed references under the agent
   rule. Keep and commit, or revert.
3. **Bookkeeping.**
   - `docs/psx-levers.md`: N7's entry and line, `scale calls`, the
     projection and any re-rank;
   - `docs/psx-game.md`: the slots, the VRAM row, the "Still to do" figure;
   - the step note.

   This chunk runs whatever the outcome: skip, revert or keep.

## Testing Decisions

- **What a good test does:** it draws through the public interface (`button_draw`,
  the cache's draw and frame-start calls) on the software backend and
  compares the screen with a direct draw pixel for pixel. It never
  depends on which slot was chosen or how the key is packed, only on what
  is drawn and on how many builds, copies and direct draws a counting
  backend sees (the counting-backend pattern of `test_text_image`).
- **`test_button_image` (new), the cache alone** (fake draw functions, no
  game):
  - a key is built once and then copied, frame after frame;
  - equal keys share a slot, and a changed word rebuilds;
  - the icon and wide pools don't evict each other;
  - more keys than a pool's slots in one frame: the extra ones return 0 and
    draw nothing. No slot copied this frame is rebuilt in it, and the next
    frame evicts what it needs;
  - a size over 112×11 returns 0;
  - a build touches neither random stream;
  - each case runs in a fresh child process, as `test_text_image` does.
- **`test_button`, the pixel identity** (extending `test_draw`):
  - every kind of button seen in the game, drawn through `button_draw` and
    compared with today's direct draw on a second screen:
    - an 11×11 icon button, lit and unlit;
    - a text button with a sprite;
    - a text-only button with no on-click (body 5);
    - a button with a side tooltip of two texts and sprites;
    - the hovered and pad-tab bodies;
  - each at whole and fractional positions, at every screen edge (partly
    off on each side) and under three cameras, the direct path's palette
    and pen colour after compared too;
  - a refreshed row (new contents, possibly the same address) rebuilds on
    the frame it changes, and an unchanged button copies;
  - a fill pattern, a remapped palette, a fractional width or offset, a
    box over 112, and a full wide pool each draw directly and identically;
  - the first two tests are red on today's code (no build counted) and pin
    the lever.
- **`test_psx_vram_layout`:** the slot count becomes `1 + 32 + 32 + 5 + 8 +
  10`. A new test pins the icon slots in a row from (784, 440) and the wide
  slots two a band from (704, 456). The existing no-overlap,
  inside-the-region, 16-unit-bound and own-page tests cover them.
- **`test_image`:** its slot-count assertion follows the new list.
- **Layout serial:** a `test_layout_row` case that two rows filled one
  after the other, and a row re-filled in place, never share a serial.
- **Prior art:** `test_text_image` (built once then copied, edges and
  cameras, a slot copied this frame never rebuilt, random streams),
  `test_row_image`, and `test_button`'s `test_draw`.
- **Bounded:** the edge and camera loops stay the size of
  `test_text_image`'s. The note gives `make test`'s wall time.
- **Not unit-tested:** the GPU copy of the new slots. It is PSX-only, and
  `psx-check`'s captures prove it (the `all` captures on both lists, `all
  235` and the new `all 155` above all), together with `gpu-cost` /
  `game-gpu-cost` and the probe's `game-state same`.
- **The real proofs:** every e2e `.sha256` and `.end-state` unchanged; every
  capture of both lists at `differ 0`; `game-state same` on every probed
  run; `make state` and `largest-slots` green.

## Out of Scope

- Every other lever (N6 has its own PRD).
- `button_draw_overlay` and the hovered tooltip (`tooltip_draw_at`); the
  lever doc found no pixel-identical lever there.
- Caching other UI pieces (panel backgrounds, message boxes) as images.
- Changing `text_image`, `row_image`, the slot sizes of existing slots, or
  their VRAM places.
- Batching several buttons into one image, or redrawing a button only when
  its look changes on a persistent screen (that would be a different,
  stateful lever).
- Tightening the draw meter's costs.
- A `psx-full-game-check` (that is step 8's).
- Acting on any warn-review warning (each is decided above, all left
  open).
- Any picture change, e2e re-record, or change to the situation rule or
  tables.

## Further Notes

- **Pixel identity holds by construction.** All of these hold:
  - the build replays today's calls at a whole offset;
  - every coordinate the direct draw uses is the floored draw position plus
    a whole constant;
  - the image box covers every written pixel;
  - unwritten pixels (the gap between a button and its side tooltip, the
    tooltip's top and bottom rows) stay transparent;
  - text goes through the direct print, which `text_image` guarantees to
    match its copy.
- **Where the gain comes from:** a cached button is one copy (a page switch,
  one `SPRT`, a page switch back) instead of a `rect` (up to four tiles),
  two or three `rectfill`s, one to five sprites with their palette-block
  switches, and one or two text copies, plus the game code that floors and
  adds every position. On map-move the four HUD buttons are the only ones
  drawn, so the gain there is small; on the tech panel it is largest.
- **Why the ratio may land under 1:** the estimate counts a copy at the
  `spr` cost, but an image copy also switches the texture page twice, and
  frames that build pay the build on top of the direct draw. The ratio
  measures both.
- **Trace used for the decisions:** throwaway prints in `button_draw` and in
  the gfx entry points, in a scratch worktree at `3d3daca`, over
  `full_game.json` and every e2e. The prints gave:
  - buttons per frame (at most 11: 7 text, 5 icon);
  - box widths (11 to 100);
  - wholeness, fill pattern, palette and camera;
  - the calls outside buttons on each capture frame.

  The implementer re-reads `counts.txt` from `psx-check` after chunk 1.
