---
title: "Lever D3: message-box text through the text image cache"
description: "An open message box draws its outlined text glyph by glyph, about 13 prints a frame at 1.7-1.8 hblanks each on PSX. It should copy each string from the text image cache, with the picture and every e2e hash unchanged."
status: needs-triage
---

## Plan step (verbatim)

Plan 22, phase 3, step 7 (`docs/plan-22.md`, section "3. Phases and steps"):

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
>    small levers that touch the same code.) A lever measured on
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

This PRD covers **only the first lever of the ranked list: D3** (rank 1 in
`docs/psx-levers.md`). N1 and every later lever get their own PRDs.

## Problem Statement

When a message box is open, every one of its outlined strings is drawn
glyph by glyph on PSX. That covers the year banner, the countdown ("5 years
remain") and the score panel's rows. Each `print` costs about 1.7–1.8
hblanks, against about 0.4 for one image copy. `message_box_draw_overlay_method`
weighs 42.6 h a frame on `map-rest-5581`. It is also in the steady
situations that are furthest over budget: map-rest, build, slipway-aim,
score-panel and tech-panel. `docs/psx-levers.md` ranks D3 first. Alone it
is estimated to remove 75 of the 1,133 steady frames over 263 (nominal; the
pessimistic projection gives 1,068).

Plan 21 built the text image cache (D2: 32 slots of 128×8, keyed by string
and colour), but skipped D3. The reason was that no `psx-check` capture
showed message-box text, so pixel identity on PSX could not be proven.
That gap still exists. `psx/gpu_captures.txt` is bound to
`tests/e2e/pad_slipway.json`, which never opens a box, and that recording
cannot change because every e2e hash must stay put.

## Solution

Two things, in this order.

1. **A capture of message-box text in `psx-check`, before any game change.**
   `psx-check` gains a second capture replay, `tests/e2e/pad_message_box.json`.
   That recording shows both kinds of box: the year box (scalar
   `print_outlined` articles and the shared banner row) and the score box
   (layout-row articles). The second replay has its own capture list, its
   own ceilings file and its own references file, all in the existing
   formats, so `pad_slipway`'s lines stay byte-for-byte as they are. Its
   captures are accepted at `differ 0` on **today's** code, under the agent
   rule. The references then pin the current picture.
2. **D3 itself.** The box's outlined strings are drawn through
   `text_image_print_outlined`, and its layout rows through
   `layout_row_draw_at_text_image`. Positions, alignment and the per-article
   camera shake stay exactly as they are. Bars, help frames and sprites in
   the box are drawn as before.

The lever is then measured on `tech-panel-5895` with `make psx-probe`,
against a probe baseline freshly taken at the commit before the change.
The measured gain replaces D3's estimate in `docs/psx-levers.md`, and the
projection is recomputed. If the change does not save hblanks, it is
reverted and noted.

The player sees nothing new. The same pixels arrive sooner.

## User Stories

1. As a player with a year box open, I want the frame drawn faster, so that the map does not stutter while the banner shows.
2. As a player who opened the score panel, I want its rows drawn faster, so that the panel stays at 60 fps.
3. As a player in a tech panel with a box open behind it, I want that frame to fit 263 hblanks more often, so that browsing techs is smooth.
4. As a player, I want the box to look exactly as before, so that nothing visible changes in pass 1.
5. As a player, I want the box's slide-in and slide-out shake unchanged, so that its animation is the same.
6. As a player, I want the countdown text ("5 years remain") drawn in its colour with its outline, so that it reads as before.
7. As a player, I want the score rows' tier icons unchanged, so that only text goes through the cache.
8. As a player, I want a long string, too wide for a text slot, still drawn, so that no text disappears when the cache cannot hold it.
9. As a player, I want text drawn correctly when all 32 slots are already used this frame, so that a busy frame falls back to glyph drawing and stays correct.
10. As the plan pilot, I want a `psx-check` capture showing year-box text, so that pixel identity of D3's scalar path is proven on PSX.
11. As the plan pilot, I want a `psx-check` capture showing the score box's layout rows, so that pixel identity of D3's layout-row path is proven on PSX.
12. As the plan pilot, I want those captures accepted at `differ 0` before the game changes, so that the references pin today's picture and not D3's.
13. As the plan pilot, I want the same captures to match their references byte for byte after D3, so that "pixel-identical" is observed, not argued.
14. As the plan pilot, I want `pad_slipway`'s capture list, ceilings and references unchanged, so that the existing proofs are not disturbed.
15. As the plan pilot, I want the second list in its own files in the existing formats, so that no label or frame key can collide with `pad_slipway`'s.
16. As the plan pilot, I want the second replay's GPU boot run twice, so that `gpu-capture-deterministic` holds for it as for the first.
17. As the plan pilot, I want every existing `psx-check` capture condition applied to the second list, so that it is held to the same rules (`clean`, `report`, `ceilings`, `references`, `capture-list`).
18. As the plan pilot, I want the ceilings and accept targets able to address the second list, so that an agent can accept it under the agent rule.
19. As the plan pilot, I want the capture frames off the `DIGEST=` sample frames (every 32nd and the last), so that the capture list rule holds.
20. As the plan pilot, I want every e2e `.sha256` unchanged, so that the PC picture and the game are proven unchanged too.
21. As the plan pilot, I want every `.end-state` unchanged, so that the game state is untouched (D3 is draw only).
22. As the plan pilot, I want a fresh probe baseline before the change, so that the measured delta compares with the commit just before D3.
23. As the plan pilot, I want D3's remaining cost measured before the change and judged against T = 4.4 h, so that the plan's "covered" and "under 2%" rules are applied to a measured figure.
24. As the plan pilot, I want D3 measured on `tech-panel-5895`, its "measured by" run, so that the gain rests on a representative run.
25. As the plan pilot, I want the probe's deltas on every short run in the note, so that the box-drawing best tries (`map-rest-5581`, `score-panel-5876`) are recorded as weaker evidence.
26. As the plan pilot, I want the change reverted if `tech-panel-5895`'s draw delta is not a gain, so that the plan's revert rule holds.
27. As the plan pilot, I want the measured gain against the 18.6 h estimate in the note, so that a lever under its estimate is noted and the plan goes on.
28. As the plan pilot, I want D3's line in the lever table switched to `measured` with measured gains, so that `make lever-estimate` uses the measurement.
29. As the plan pilot, I want the projection recomputed and the remaining levers re-ranked if the order changes, so that N1 and the later levers keep their right order.
30. As the plan pilot, I want no `scale` line written after D3, so that the per-basis ratio waits for a second `calls` lever, as the plan states.
31. As the plan pilot, I want D3's measured-to-estimated ratio recorded, so that the next `calls` lever (N3 or N7) can form the median.
32. As the plan pilot, I want no `psx-full-game-check` run for D3, so that the phase's full-game check stays at step 8 (D3 is measured on a short run).
33. As a developer, I want the text-image path for a box's scalar print to keep `hud_print_outlined`'s alignment arithmetic, so that centred and right-aligned texts land on the same pixel.
34. As a developer, I want a unit test drawing every box article kind both ways on the PC renderer and comparing framebuffers, so that a regression is caught in `make test`, not only in `psx-check`.
35. As a developer, I want `make test` and `make e2e` no slower, so that the fast loop stays fast.
36. As a developer, I want the PSX RAM figures (`ram-room`, `game-ram-room`) in the note, so that a size drift is visible.
37. As a reviewer, I want the step note to list the new captures and how they were accepted, so that the agent-rule acceptance is traceable.

## Implementation Decisions

### Checks before the change (plan rules)

- **Covered or under 2%?** No lever was taken since step 5's list, so D3
  cannot be covered by a lever of this plan. Its remaining cost is still
  measured. A throwaway `psx-probe` patch draws the open box's articles a
  second time (`message_box_draw_overlay`'s article loop run twice, timing
  only; never committed). The patch is run on `tech-panel-5895`, and
  half-informatively on `map-rest-5581` and `score-panel-5876`. The draw
  delta on `tech-panel-5895` is the box's remaining cost a window frame.
  Under T = 4.4 h (step 5's 2% of the steady mean), D3 is skipped as
  "under 2%", noted, and nothing else in this PRD is built. Above T, go on.
  The figure goes in the note in both cases.
- **Fresh probe baseline**: `make psx-probe-baseline` (all runs) at the
  commit that holds chunk 1, before the D3 change. Its runs must be valid.

### Chunk 1: second capture replay in `psx-check` (decided by `guideline`)

Decision (guideline): `psx-check` gains a second capture replay,
`pad_message_box`, with its own capture list, ceilings and references
files in the existing formats. The ceilings and accept targets take an
optional list selector and default to `pad_slipway`'s files. Why: plan
22's "every capture at differ 0" is checked inside `psx-check`, and a
lever is skipped only when covered or under 2%, so skipping D3 again is
out. Separate files are the smallest change that keeps `pad_slipway`'s
lines byte-for-byte and leaves `make test`/`make e2e` untouched.

- **Files**: a second capture list (`replay pad_message_box`), its ceilings
  file and its references file, next to the existing three in `psx/`.
  Their formats and parser are unchanged (`capture_list_parse` already
  refuses a list for another replay).
- **Captures** (frames given in today's numbering; see H-6-2 for the shift):
  - year box settled, no menu, no panel, aim mode: a `print` capture
    (`cls,print`) at f140 and an `all` capture at f141;
  - score box, rows slid in, before the O press: an `all` capture at f162.

  The label rule applies: the `print` label needs an on-screen `print` call
  at its frame on PC (`gpu-capture-list` decides). No frame may be a
  multiple of 32 or the recording's last. If a frame is refused, the
  nearest frame that shows the same box and is not refused is used. The
  note gives the frame.
- **Build and boots**: a second `psx-build` directory with
  `pad_message_box.json` embedded, building only the GPU replay. The PC
  references come from one `tmp/replay` run with `CAPTURE_LIST=` on that
  recording. The GPU replay is booted twice with that list's capture
  script. Only the seven GPU capture conditions run on it, through the
  existing `tools/psx_capture_check.sh` and `tools/psx_capture_report.sh`,
  each prefixed so their condition lines are distinguishable (for example
  `message-box gpu-captures`). The other `psx-check` conditions stay on
  `pad_slipway` alone. `psx-check` gets about 30–40 s slower; it is not
  part of `make test`, `make e2e` or `make test-all`.
- **Targets**: `psx-gpu-capture-ceilings` and `psx-gpu-capture-accept`
  accept a selector (for example `CAPTURES=message_box`). The default is
  today's `pad_slipway` files. `psx-gpu-capture-report` stays on
  `pad_slipway` only.
- **Acceptance**: after a green `psx-check` with every capture of the new
  list at `differ 0`, ceilings are written at 0 and the references are
  accepted (agent rule, `docs/psx-game.md` § Acceptance). If a new capture
  is not at `differ 0` on today's code, that is a pre-existing PSX
  divergence. In that case D3 stops: the step note records the report
  line, and D3 is noted as not proven and not taken. No agent accepts a
  non-zero capture.

### Chunk 2: the lever

- **Scalar articles** (`message_box_draw_article`): a
  `DRAW_FUNCTION_PRINT_OUTLINED` row, whether built from scalars or reached
  through the shared banner row, is drawn by a text-image variant of
  `hud_print_outlined`. It computes the same integer x (alignment `a × 4 ×
  strlen` subtracted after `to_int`) and the same y and colour (`& 0x0F`),
  then calls `text_image_print_outlined`.
- **Interfaces**:
  - `draw_functions` gains `draw_functions_draw_row_text_image(gfx, mem,
    row)`, with `draw_functions_draw_row`'s contract except that
    `PRINT_OUTLINED` goes through the text image. It takes a non-const gfx,
    as the text image requires.
  - `hud` gains `hud_print_outlined_text_image`, with
    `hud_print_outlined`'s contract (same pixels, same draw palette after;
    gfx's camera moves during a build and is restored).

  Only `message_box.c` calls the new functions. The HUD top bar and every
  other `draw_functions_draw_row` caller keep the old path (D4 was dropped,
  under T).
- **Layout-row articles**: `layout_row_draw_at` is replaced by
  `layout_row_draw_at_text_image` at the same position and alignment.
  Sprites and other non-text items keep their draw.
- **Camera shake**: `p8_camera` is still set per article before the draw.
  The text image copy honours the camera like any draw. A slot built
  during the box draw restores the camera (text image contract).
- **Fallbacks**: a string longer than 31 characters, or wider than a slot,
  and a frame with all 32 slots already copied, both fall back to
  `p8_print_outlined` inside `text_image_print_outlined`, so they are
  correct as they stand. No slot count or size changes.
- **No game state touched**: draw only. Any e2e hash or `.end-state` change
  means a bug in the change, never a re-record.

### Measurement and bookkeeping

- **Probe**: chunk 2 as a patch, through `make psx-probe` on every short
  run against chunk 1's fresh baseline. The measured gain is
  `tech-panel-5895`'s window draw delta (hblanks a window frame, and % of
  its draw), set against the 18.6 h (5.7%) estimate. `game-state same` is
  required on every run. The deltas of `map-rest-5581` and
  `score-panel-5876` are reported as weaker evidence (failed representative
  runs), and so is `build-5752`'s, whose window draws no box (its delta is
  expected to be about 0). The `delay`/text-size lines are reported.
- **Revert rule**: if `tech-panel-5895`'s draw delta is not negative beyond
  its run's noise floor (step 4's `max(3 h, 1% of the window)`), chunk 2 is
  reverted (or never committed) and noted. Chunk 1's captures stay: they
  prove existing pixels.
- **Lever table** (`docs/psx-levers.md`): D3's line becomes `measured`. The
  tech-panel gain is the measured % of draw. Every other situation's gain
  is its estimate × (tech-panel measured ÷ 5.7). There is one measurement,
  and the lever's own ratio applies to its own other estimates. That ratio
  goes in the note and in D3's entry. This is not the per-basis `scale`
  line, which waits for a second `calls` lever.
- **Projection**: `make lever-estimate` on the committed tables, its block
  and `order ok` (or the new order, with the list re-ranked to match)
  pasted in place of the current projection. The note gives steady frames
  over after D3, nominal and pessimistic, against step 5's 1,058 / 1,068
  (or against what the committed table gives at the start, see H-6-4).
- **No `psx-full-game-check`** in this step. D3 is short-run measured, and
  step 8 is the phase's full-game check. Probe-first and grouping apply
  only to full-game-measured levers (N6).
- **Docs**: `docs/psx-levers.md` (D3 entry: measured gain, ratio, the new
  captures; lever table; projection; the "What each function gets" row).
  `docs/psx-game.md` § GPU family captures and § Checks: the second capture
  replay, its files, its selector and the time added to `psx-check`, in a
  few sentences. `tests/e2e/pad_message_box.txt` is not edited (its hash is
  unchanged, and it gains no capture readings).
- **Step note** (`### Step 7.1` in the plan): the residual cost and the
  covered/under-2% verdict; the captures added and their acceptance;
  measured vs estimated; the ratio; the probe deltas per run; steady frames
  over before and after (nominal / pessimistic); any re-rank; `ram-room` /
  `game-ram-room`; any guideline decision or deviation.

## Implementation Chunks

1. **Second capture replay**: the files, the second build and boots in
   `psx-check`, the prefixed conditions, the target selectors, the docs
   lines; `psx-check` green; the new captures accepted at `differ 0`.
   Committed on its own. Then the throwaway residual probe, and
   `psx-probe-baseline` at this commit.
2. **D3**: the two new functions, and `message_box_draw_article` routed
   through them. Unit tests first (TDD). Then `make test-all`, `psx-check`
   green with every reference matched, `psx-probe`, the lever table,
   projection and docs. Reverted per the rule above if the probe shows no
   gain.

## Testing Decisions

- A good test drives the public draw functions on the PC software renderer
  and compares framebuffers, plus the draw palette after the call. It never
  inspects slot internals.
- **`test_hud`**: `hud_print_outlined_text_image` draws like
  `hud_print_outlined` for alignments 0, 0.5 and 1, several colours
  (including a colour above 15), a non-integer x, a string too long for a
  slot, and with every slot already used this frame.
- **`test_message_box`**: a year box (scalar print rows, the shared banner
  row, a bar row) and a score box (layout rows with text and tier icons),
  drawn through `message_box_draw_overlay` at several `state_time`s
  (shake on). The framebuffer must equal the one from the old article path
  (the `p8_print_outlined`/`layout_row_draw_at` reference, run in the same
  test), so the test stays meaningful after the switch.
- **Prior art**: `test_layout_row`'s
  `test_the_text_image_sibling_draws_like_draw_at` and `test_text_image`
  (`text_image_frame_begin` between frames, run in a fresh child).
- **Capture list**: `test_capture_list` needs no new case (the format is
  unchanged). If the selector logic lives in a shell script,
  `psx-tools-test` covers it the way it covers `psx_capture_record.sh`.
- **The real proofs**: every e2e `.sha256` and `.end-state` unchanged
  (`make e2e`), and the new captures matching their references in
  `psx-check`.

## Out of Scope

- Every other lever: N1, N2, N3, N5, G4, N7, N6, and plan 21's dropped D4
  (score-panel readouts in the HUD bar). No other `draw_functions_draw_row`
  caller changes.
- Text image slot count or size, and the image cache itself.
- `psx-gpu-capture-report` for the second list.
- A `psx-full-game-check` (step 8's).
- Any picture change, any e2e re-record, any change to the situation rule
  or tables.
- Step 6's work (loading screen, renumbering).

## Hypotheses on step 6's outcome

Step 6 (`docs/prd/triage/phase2-step6-loading-screen.md`) is not done when
this PRD is written. Each assumption below must be checked against step 6's
committed result before chunk 1. A false one is resolved in favour of what
step 6 committed, and the difference goes in the step note.

- **H-6-1**: `full_game.json` has 6,061 frames (+2 from frame 12). The
  short runs keep their names: `tech-panel-5895`'s window is now
  5897–5905, and so on. `docs/psx-levers.md` names runs by those unchanged
  names, so D3's "measured by `tech-panel-5895`" still holds.
- **H-6-2**: `tests/e2e/pad_message_box.json` was re-recorded with two
  loading frames after its difficulty press at f12, so every frame quoted
  above from f12 on moves by +2. The planned captures become f142 (print),
  f143 (all) and f164 (all). None is a multiple of 32, and none is the
  last frame (224 if the recording is 225 frames). Its `.txt` is
  renumbered by step 6, and the frames are re-read from it before use.
- **H-6-3**: step 6 re-took the probe baseline on the new numbering. This
  PRD still takes a fresh one at chunk 1's commit, because the fingerprint
  (tree of `psx/` and `src/`) changes with chunk 1's `psx/` files.
- **H-6-4**: step 6's `psx-full-game-check` replaced
  `psx/perf/full-run-frames.txt` (6,061 frames) and the situation table.
  Under step 7's rebase rule, no lever is in the measured code yet, so
  `in-base` is none. The starting projection is what `make lever-estimate`
  prints on the committed tables. It may differ slightly from step 5's
  1,133 → 1,058, and the note quotes both.
- **H-6-5**: step 6 added the `display` capture kind to the capture list
  parser and to the capture conditions (exempt from the family rule). The
  second list neither needs nor uses it, and the parser's other rules are
  unchanged.
- **H-6-6**: step 6 changed none of `message_box.c`, `draw_functions.c`,
  `hud.c`, `layout.c` or `text_image.c`. The loading screen lives in the
  cart's start path and does not hold text image slots across frames in
  play.
- **H-6-7**: `psx-check` is green at step 6's end, with every
  `pad_slipway` capture at `differ 0` and the references re-accepted by
  step 6. `pad_slipway`'s files are the baseline that chunk 1 must leave
  byte-for-byte unchanged.
- **H-6-8**: the e2e `.sha256` set after step 6's re-record is the set that
  "unchanged" refers to in this PRD.

## Further Notes

- Plan 21 history: in step 3.10, D3's residual was 36.1 / 0 h a frame on
  `draw-5564` / `draw-6000`, "not covered". In step 3.15 it was skipped by
  `guideline` because "no `pad_slipway` frame shows message-box text". The
  same step's spike showed that outlined glyph texels survive an image
  build and copy bit for bit. That is the basis for risk `none`.
- One-off cost: each distinct box string is built once into a slot when the
  box opens or its year text changes. Plan 21 measured D2's builds at up to
  12 a frame. These land on `new-year` / `screen-change` frames (one-off),
  whose vsync sum is far under 10,800.
- The score box's first row, "current score (reasonable)" (26 characters),
  fits a slot. A longer `end_status` string falls back to glyph drawing.
