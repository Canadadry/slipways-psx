---
title: "Ordinary update levers: ask \"is a panel open?\" once per pad step (O2, or O1), box-reject the hover hit tests (O3), initialise only the written hover layers (O5)"
description: "On every ordinary frame the pad step asks the same panel question about nine times, each time scanning the 89-entity hit-test bucket, and the hover pass runs the full hit-test call on about 78 planets for one cursor point and initialises 16 mostly empty lists; the step removes that repeated work without changing a single e2e hash."
status: needs-triage
---

## Problem Statement

Plan 21 step 17 opens phase 4 (update). Phase 3 cut draw by half (step
3.16: mean frame 318.4, 3,942 of 6,059 judge frames still over 263), so
update now weighs more in every ordinary frame. Three levers of the report
(`docs/psx-perf-levers.md`, "Ordinary update (step 10)" and the do-less
table) sit in the two update functions that run on every frame:

| Lever | Where | What is repeated | Report gain a frame (ordinary / `draw-5564` / `draw-6000`) | Cost |
|---|---|---|---|---|
| O1 | `pad_panel_open` → `pad_panel_entity_under` | 7.4 scans a frame of the 89-entity hit-test bucket to learn whether a modal backdrop exists | 28.2 / 2.9 / 7.3 | M |
| O2 | the twelve `pad_nav_panel_open()` sites of `pad_nav.c` | 8.8 asks a frame of the same question inside one `pad_nav_step` | 25.0 / 2.5 / 6.6 | S |
| O3 | `ui_update_hover`'s hit-test loop → `planet_hit_test_method` | 77.7 planet hit tests a frame, 0.2 hblank each, for one cursor point; planets do not move | 14.4 / 12.1 / 13.3 | M |
| O5 | `ui_update_hover`'s layer loop | 16 `entity_id_list_init` + 16 `frame_zone()` calls a frame for lists that mostly stay empty | 1.7 / 1.3 / 1.3 | S |

The report priced them at O2 with asserts, meter and filter in; update has
since lost 13.8% to G2 (`-O3`, step 2.5) and nothing else touched these
functions. O1 and O2 are alternatives (not additive, the report's estimator
counts O1). The plan names O2 and "O1 if cheaper", then O3 and O5, and
requires each to keep every e2e hash: a stale panel answer or a wrong
hovered entity is a game-state change, and the e2e replays decide.

Two facts make the short runs a weak mirror for O1 and O2: the panel
question is costly on ordinary frames with no panel open (the scan runs the
whole bucket), and the two draw windows hold few of those (report: 2.5 and
6.6 against 25.0 on the ordinary population). The step must still be judged
on the short runs (plan rule), so its note also gives host counts that
relate the windows to the whole game.

## Solution

One chunk of throwaway probes and host-only counts decides what is built
and in which form; then each lever lands in its own commit, measured with
`make psx-probe` on its own diff against the HEAD just before it, and kept
only when its pooled update delta is a gain and every e2e hash,
`make end-state` and every capture stay unchanged.

- **O2** (or O1, whichever probe shows the larger gross target): the pad
  step answers "is a panel open?" once and reuses the answer for as long as
  nothing it depends on has changed; outside a pad step the question is
  asked fresh, as today.
- **O3:** the hover pass walks the hit-test bucket in the same order, but a
  planet whose box cannot hold the cursor is rejected by an inline box test
  instead of the full indirect hit-test call. Same hits, same order, same
  hovered entity, by construction.
- **O5:** the hover pass initialises a layer's list only when it adds to it;
  every reader treats a layer not written in this pass as empty.

Nothing visible changes for the player on PC or PSX.

## User Stories

1. As the plan pilot, I want O1's and O2's gross targets measured by throwaway probes on today's build, so that "O1 if cheaper" is decided on today's cost, not on the report's O2-with-asserts figure.
2. As the plan pilot, I want "cheaper" read as "leaves the update cheaper" (the larger gross target pooled over the two draw windows), so that the choice follows step 14's precedent of building the side with the larger measured ceiling.
3. As the plan pilot, I want the alternative not taken re-probed on top of the one taken and judged covered when under 6.4 hblanks on both draw runs, so that the plan's covered rule uses step 3.16's threshold.
4. As the plan pilot, I want O3's and O5's gross targets probed too, so that every measured gain in the note sits next to an upper bound and the report's figure.
5. As the plan pilot, I want host counts of panel asks per pad step and hover hits per pass over the full game, so that the note can say how the short-run windows relate to the ordinary judge frames.
6. As the plan pilot, I want each lever kept or reverted on its own pooled update delta, so that a lever is credited only with what it removes.
7. As the plan pilot, I want any changed e2e hash to revert the lever whatever its gain, so that the plan's "e2e decides" rule for O1, O2 and O3 holds.
8. As the plan pilot, I want the step's figures (probe, report, measured, asserts mode, opt level) written in the step note, so that step 18 starts from the right baseline.
9. As a player on PC, I want the pad to open, close and move through menus, the tech panel, the score panel, the pause menu and message boxes exactly as before, so that the change is invisible.
10. As a player on PSX, I want the same game as on PC, frame for frame, so that the platforms stay identical.
11. As a player, I want the planet under the cursor highlighted exactly as before, including where two boxes overlap and on a box's edge pixel, so that hover never jumps to another planet.
12. As a player, I want buttons and panels drawn above planets to keep catching the cursor first, so that the hover order is unchanged.
13. As a player, I want a click on a planet, a button or a backdrop to reach the same entity as before, so that the left press order through the hover layers is unchanged.
14. As a player, I want a Back press that closes a box to be followed, in the same frame, by the same decisions as today, so that a reused panel answer is never stale.
15. As a player, I want a slipway cancelled inside a pad step that spawns or kills an entity to be seen by the rest of the step, so that the memo is re-asked after any registration.
16. As a game developer, I want the panel memo to validate itself on a key of everything the answer reads (ui, economy, the box's ephemerals count, the registry's registration generation), so that correctness does not rest on a hand-kept list of in-step changes.
17. As a game developer, I want the memo active only between the pad step's begin and end, so that tests, devtools and any caller outside a pad step get a fresh answer.
18. As a game developer, I want the registry to expose a registration generation that changes on every register and unregister, so that any game-side cache can tell that the registered set moved, without the engine knowing planets or panels.
19. As a game developer, I want the hover loop's box reject to run the very body of `entity_geom_contains_cursor`, so that the inline test and the method can never disagree.
20. As a game developer, I want the inline path chosen by the class's hit-test method being the planet's one, so that no new class field and no planet name in the engine are needed.
21. As a game developer, I want one reader for a hover layer's list, so that every reader treats a layer not written in this pass as empty.
22. As a game developer, I want `ui_forget_world` to empty every hover layer through the same mechanism, so that a forgotten world never shows a stale hover list.
23. As a maintainer of `make state`, I want the frame zone's peak unchanged, so that O5 is proven to spend no extra memory.
24. As a maintainer of the e2e replays, I want every `*.sha256` and `make end-state` unchanged after each lever, so that the game is proven untouched.
25. As the human who alone accepts capture references, I want every capture and reference unchanged, so that I accept nothing new.
26. As a developer running `make test` and `make e2e`, I want them no slower, with no debug cross-check added to the hot loops, so that the fast loop stays fast.
27. As a maintainer, I want unit tests that compare the memoised panel answer with a fresh scan after every kind of change, so that O2 is proven exact on PC.
28. As a maintainer, I want unit tests that compare the hover pass with a reference walk calling every entity's hit test, over a grid of cursor points, so that O3 is proven exact on PC.
29. As a maintainer, I want unit tests that read unwritten and forgotten hover layers as empty, so that O5 is proven exact on PC.
30. As a maintainer of `pad_panel`, I want the module to get its own test file, so that its contract is tested where it lives.
31. As the PSX RAM budget's owner, I want `ram-room` and `game-ram-room` recorded after the step, so that any growth is visible.
32. As the plan pilot, I want every assumption about the starting state checked and recorded before the probes, so that no decision rests on a state that does not hold.

## Starting state (checked at the start of chunk 0)

Chunk 0 checks each item against `docs/plan-21.md` and HEAD and records the
outcome. If one fails, the decision it names is adapted as said; nothing
else changes.

- **S-1.** Step 16's note (`### Step 3.16`) and the phase 3 plan-review
  (`d16a6a3`) are at the HEAD this step starts from. Every probe takes its
  baseline there (`make psx-probe-baseline`; the fingerprint refuses a
  stale one).
- **S-2.** The threshold for "covered" is **6.4 hblanks a window frame**
  (2% of step 3.16's mean frame, 318.4). No `psx-full-game-check` runs in
  this step (step 18 runs the next one).
- **S-3.** The only per-call unit-cost table is step 3.9's, and it holds
  draw families only; no update unit cost exists. "Both figures" in the
  note are therefore the report's figure and the probe's figure (plus the
  host counts that turn the probe into a per-call cost).
- **S-4.** `pad_panel_open`, `pad_panel_entity_under`, the twelve
  `pad_nav_panel_open()` sites, `ui_update_hover`, `ui_send_to_hovered`,
  `ui_forget_world` and `planet_hit_test_method` are as described under
  Further Notes. If one changed, chunk 0 re-reads it before writing a probe.
- **S-5.** Phase 3's warn-review touched no phase 4 step, and step 2.7's
  flagged code (`asserts_enabled()` duplicates, the O3 default, condition
  counts, GPU replay stubs, the Makefile) is not extended here: no
  prerequisite chunk.
- **S-6.** The probe tooling (`make psx-probe`, `make psx-probe-baseline`,
  `psx/probes/`) works as in steps 8 to 15; `representative` still fails on
  every short run, recorded, not a blocker.

## Implementation Decisions

### The rules applied in this step

- **Part read:** update, a window frame. The keep and covered decisions read
  `draw-5564` and `draw-6000` (15 ordinary frames each); the one-frame runs
  (`start`, `update-4737`, `update-5500`, `update-5719`) are recorded, not
  used.
- **"Cheaper" (O1 against O2):** the lever whose gross-target probe is
  larger, pooled over the two draw windows, is built first: it is the one
  that leaves the update cheaper. Its build cost (S for O2, M for O1) is not
  weighed. A tie within 0.5 a frame goes to O2 (cost S, no new tag).
- **Covered:** after the first of O1/O2 is kept, the other is re-probed on
  top of it. Under 6.4 on both draw runs → skipped, covered, with its
  residual. Otherwise it is built in its own chunk and kept or reverted on
  its own delta. If the first is reverted, the other is built as the
  lever of record.
- **No size threshold for O3 and O5.** The step names both; the plan's
  "under 2%" skip belongs to step 15. Each is built and kept on a measured
  gain. The note gives O5's figure against 6.4 for information.
- **Keep:** a built lever is measured with `make psx-probe` on its own diff
  against a baseline at the HEAD just before it, all six runs, asserts off,
  O3. Kept when the pooled `draw-5564` + `draw-6000` update delta a frame
  is a gain; otherwise reverted and noted. No lever here builds images, so
  no resume-frame exclusion applies.
- **E2e decides:** any changed `tests/e2e/*.sha256`, any `make end-state`
  difference, or any capture or reference change reverts the lever,
  whatever its gain. No hash is re-recorded in this step.
- **Order:** O2 or O1 (per chunk 0), the other if not covered, O3, O5.

### Chunk 0: probes (throwaway, uncommitted)

Each probe is a patch run with `make psx-probe` on the six short runs
against one `make psx-probe-baseline` at the starting HEAD. None changes
the game state, the picture or `visual_random`: each repeats a pure query
into a `volatile` sink. Prior art: `psx/probes/call-twice-cart-draw.patch`
and the "twice" probes of steps 3.10 to 3.15.

- **`o2-panel-open-twice`:** `pad_nav_panel_open` evaluates
  `pad_panel_open` a second time into the sink. The delta is the cost of
  every ask at the twelve sites: O2's gross target (O2 keeps about one ask
  a step).
- **`o1-backdrop-scan-twice`:** inside `pad_panel_open`, the backdrop scan
  (`pad_panel_entity_under(0, 0, PAD_PANEL_ANY_LAYER, is_modal_backdrop)`)
  runs a second time into the sink whenever it runs once. The delta is the
  cost of every backdrop scan from every caller: O1's gross target.
- **`o3-planet-hit-tests-twice`:** in `ui_update_hover`'s loop, every entity
  whose class's hit test is `planet_hit_test_method` is hit-tested a second
  time into the sink. The delta is O3's gross target (the box reject keeps
  a few instructions a planet, so the measured gain is below it).
- **`o5-layer-init-twice`:** the 16-list initialisation loop runs a second
  time (on scratch lists, not the ui's). O5's gross target.

### Chunk 0: host-only counts (PC, nothing tracked written)

From a throwaway instrumented PC build replaying `worst-case/full_game.json`
(whole game and the two draw windows) and every e2e replay, output in
`tmp/`:

- `pad_nav_panel_open` asks and `pad_panel_open` calls (all callers) per
  `pad_nav_step`: mean, max, and on how many steps the answer changes
  inside the step, at which site, and after which call.
- On every step: whether the memo key below (ui, economy, ephemerals count,
  registration generation) changes every time the answer changes. Expected:
  always. **A single change of answer with an unchanged key fails the O2
  design**: chunk 0 then adds the missing input to the key and repeats the
  count; if no O(1) input explains it, O2 is skipped on exactness and O1
  is built as the lever of record.
- Any modal backdrop whose `is_dead` changes inside a `pad_nav_step`
  (expected: none; the only write is the backdrop's left press, reached
  from the ui's button handling, not the pad step).
- Hover: bucket size, planet-method entities per pass, hits per pass, the
  largest number of layers written in one pass; any planet-method entity
  with a layer of 10 or more, or without a position or a hit box (expected:
  none; the inline path handles them exactly anyway).

### O2: one answer per pad step (if chosen)

- The memo lives in `pad_panel`, next to the question, so every in-step
  ask benefits: the twelve `pad_nav_panel_open()` sites and `pad_panel`'s
  own callers inside the step (`pad_panel_is_item`, `pad_panel_first_item`,
  hence `pad_nav_focus` and `own_panel_up`).
- Interface: `pad_panel_step_begin()` and `pad_panel_step_end()`.
  `pad_nav_step` calls begin on entry and end on both of its exits (the
  pause return and the end). Outside a begin/end pair, `pad_panel_open`
  asks fresh, as today; an unbalanced pair is an assert.
- Inside the pair, `pad_panel_open` returns the remembered answer when the
  key equals the one it was computed under, and recomputes otherwise. The
  key holds everything the answer reads that can change: `main_ui()`,
  `main_economy_typed()`, the box's `ephemerals.count`, and the registry's
  registration generation (a backdrop entering or leaving the bucket).
- The registry gains `entity_registry_generation()`: a counter incremented
  by every `entity_register` and `entity_unregister`, and by the
  forget/abandon paths that empty the registries. Engine-level, planet- and
  panel-free; it never wraps into an equal value within a step (32-bit).
- Not in the key: a backdrop's `is_dead` (chunk 0's count proves it never
  changes inside a pad step). The memo's contract line says so, as the C
  contract the name does not state.
- `pad_nav_panel_open()` stays the public question with its signature; no
  site changes its call.

### O1: a backdrop's existence read from a backdrop-only list (if chosen, or not covered)

- The engine's tag enum gains `TAG_MODAL_BACKDROP`, set on the modal
  backdrop class (as `TAG_MESSAGE_BOX` is for message boxes); its registry
  reserve is the backdrop slot count.
- `pad_panel_open` replaces the bucket scan by a walk of
  `entities_by_tag(TAG_MODAL_BACKDROP)` for an entry that is not dead.
  Exact by construction: a backdrop is in the tag list exactly while it is
  registered, and every registered backdrop is in the hit-test bucket (its
  class has a hit test that always holds; the existing assert stays).
- `pad_panel_entity_under` is unchanged for its other callers.

### O3: the hover pass's box reject

- **Decision: an inline box reject in the bucket walk, not the report's
  grid.** A grid of planet boxes would need filing at registration, at
  save load and at any position write, plus a merge to keep the bucket's
  order (the last hit is the hovered entity, and each layer's list is in
  bucket order). The walk itself costs a few instructions an entity; what
  O3 removes is the indirect call, the wrapper and the call chain inside
  each test. The inline reject removes those for every planet and keeps the
  bucket order by construction. The grid's extra saving over it is about
  one hblank a frame at the report's counts, not worth its maintenance and
  exactness risk.
- `entity_geom_contains_cursor`'s body becomes a `static inline` in its
  header that the out-of-line function wraps, so both paths run one body.
- In `ui_update_hover`, an entity whose class's hit test is
  `planet_hit_test_method` is tested with that inline body; every other
  entity goes through its class's hit test as today. The loop, the
  per-layer adds, `hovered` and `is_hovered` are otherwise unchanged.
- `pad_panel_entity_under`'s loop is not changed (O1 handles its backdrop
  use; its other callers are rare).

### O5: hover layers initialised on first write

- The hover lists gain a 16-bit mask of the layers written in the current
  pass. `ui_update_hover` clears the mask instead of initialising 16 lists;
  the first add to a layer in a pass initialises that list (with the frame
  zone fetched once a pass) and sets its bit.
- One reader, `ui_hovered_layer(ui, layer)`: the layer's list when its bit
  is set, otherwise a shared empty list. `ui_send_to_hovered` and every
  test that read `hovered_by_layer.layers[...]` directly go through it.
- `ui_forget_world` clears the mask (a forgotten world reads empty, as the
  per-list clears did); `ui` creation starts with the mask clear.
- Frame-zone use is unchanged (initialising a list allocates nothing; the
  first add grows it as today): `make state`'s `zone frame` peak must be
  equal, and the note says so.

### What is not changed

- `pad_panel_entity_under`, the pad navigation logic and its decisions, the
  bindings, the ui's press handling and its ephemerals.
- Planet hit boxes and positions; the hit-test bucket and its order.
- The report's line numbers are the report's; the functions named here are
  what counts.

## Implementation Chunks

0. **Starting state, probes, counts, decision** (no committed source
   change).
   - Check S-1 to S-6; record each outcome.
   - `make psx-probe-baseline` at the starting HEAD; the four probes.
   - The host counts, including the O2 key check.
   - Per lever: probe figures on the two draw runs and the one-frame runs,
     the report's figure; O1 against O2 decided by the rule above.
   - This PRD, `progress.md` and the figures go in the commit's message,
     as steps 8 to 15 did.
1. **The chosen one of O2 / O1.** For O2: the registry generation (engine,
   with its test), the step bracket and the keyed memo in `pad_panel`,
   `pad_nav_step` bracketing; for O1: the tag, its reserve, the tag walk.
   Tests; `make test-all`, `make end-state`, `make state`, `psx-check`;
   `make psx-probe` on the diff; keep or revert.
2. **The other one, if not covered.** Residual probe on top of chunk 1
   (`o1-backdrop-scan-twice` after O2, `o2-panel-open-twice` after O1).
   Under 6.4 on both draw runs → skipped, no commit but the figure in the
   next commit's message. Otherwise built, same gates and measurement.
3. **O3.** The inline contains-cursor body, the hover loop's planet path;
   tests; same gates; `make psx-probe`; keep or revert.
4. **O5.** The written-layer mask, `ui_hovered_layer`, readers and tests
   switched, `ui_forget_world`; same gates, `make state`'s frame-zone peak
   checked equal; `make psx-probe`; keep or revert.
5. **Close.** If a lever was kept: `make psx-short-runs` at the harness
   defaults, `ram-room`, `game-ram-room`. The step note in
   `docs/plan-21.md` under `### Step 4.17`: the starting-state outcomes,
   each lever's decision with probe, report and measured figures (asserts
   mode and opt level as reported), the O1/O2 choice and the covered
   residual, the host counts relating windows to the whole game.

## Testing Decisions

- **What makes a good test here:** the output is decisions (a panel answer,
  a hovered entity, per-layer lists in order). Tests drive public calls and
  compare against a fresh computation done by the test; no test reads a
  module's internals (the memo, the mask).
- **Registry generation (`tests/test_entity_registry.c`):** changes on
  register, on unregister and on forgetting the registries; unchanged by a
  layer change or a read.
- **`pad_panel` (`tests/test_pad_panel.c`, new; prior art the panel cases
  of `tests/test_pad_nav.c`):** inside a begin/end pair, after each of
  opening a box, clearing the ephemerals, spawning a backdrop, unregistering
  it, losing the economy and losing the ui, `pad_panel_open` equals the
  answer outside any pair for the same state; outside a pair it always
  asks fresh; an unbalanced pair asserts. For O1: a dead backdrop still
  registered reads closed; two backdrops, one dead, read open.
- **Pad navigation (`tests/test_pad_nav.c`):** unchanged and green; a test
  that froze the old behaviour and breaks while every e2e hash holds is
  updated (CLAUDE.md). A step where Back closes a box and a step where a
  cancelled slipway re-registers entities give the same played frame as the
  pre-lever code reproduced in the test.
- **Hover (`tests/test_ui.c`):** over a grid of cursor points covering
  planets, overlapping boxes, box edges (inclusive bounds), a button and a
  backdrop above the planets, under a non-zero camera: `hovered`, every
  layer's list in order and `is_hovered` equal a reference walk in the test
  that calls every entity's class hit test.
- **Hover layers (`tests/test_ui.c`, and the readers in
  `tests/test_background.c`, `tests/test_help_screen.c`,
  `tests/test_modal_backdrop.c`):** a layer not written in this pass reads
  empty even after a previous pass wrote it; after `ui_forget_world` every
  layer reads empty; `ui_send_to_hovered` reaches the same entity as before.
- **Whole-game proof:** every `tests/e2e/*.sha256` and `make end-state`
  unchanged after each lever; `make state`'s zone peaks recorded (frame zone
  equal after O5); `psx-check` green with every capture and reference
  unchanged.
- No debug cross-check (fresh answer compared with the memo, full hit test
  compared with the reject) is added to `make test` or `make e2e` runs: it
  would slow the fast loop. The unit tests and the e2e hashes carry the
  proof.
- No test is added for a lever that chunk 0 skips.

## Out of Scope

- O4 (reusing last frame's hover): not in the step; the report enters it
  at 0.
- A grid of planet boxes (decided against above).
- Speeding up `pad_panel_entity_under` for its other callers, or any other
  hit-test walk.
- The spike levers (U1–U8, step 18), spreading (steps 19–20).
- Running `psx-full-game-check` (step 18 runs the next one).
- Accepting capture references; merging `claude/plan-21-d8-pending`.
- Measuring on the Miyoo or PCSX-ReARMed; audio.

## Further Notes

- **Expected outcome, not a decision:** the windows hold few ordinary
  no-panel frames, so O1 and O2 will likely measure a few hblanks a window
  frame (report 2.5 / 6.6 before G2), while the judge's ordinary frames
  would gain about ten times more. O3 is the largest on the short runs
  (report 12.1 / 13.3). O5 is about one hblank. A kept O1 or O2 with a
  small window figure is a valid outcome; the host counts give the judge
  estimate for step 18's note.
- **Why the key, not a list of re-ask sites:** inside one pad step the
  answer can change through `back_closes_the_box`, `message_box_close_shown`,
  `close_panels`, `ui_clear_ephemerals` in four places, and a slipway's
  `drop` that may spawn or kill entities. A hand-kept list would break the
  first time a new site appears; the key catches every change of an input
  the answer reads.
- **Timing-model caveat:** all four levers remove instructions only; the
  judge's flat cycle model charges them fully.
- **Code read at HEAD `d16a6a3`** when this PRD was written:
  `pad_nav_panel_open` forwards to `pad_panel_open`, which returns 0 with no
  ui and otherwise "no economy, or a box open (ephemerals count > 0), or a
  live modal backdrop in the hit-test bucket", the last found by
  `pad_panel_entity_under` walking the whole bucket with a function-pointer
  filter; `pad_panel_is_item` and `pad_panel_first_item` call
  `pad_panel_open` again; `pad_nav_step` has two exits (pause, end). The
  backdrop is created from a slot zone with a hit test that always holds,
  dies only from its own left press, and is unregistered by the update
  walk. `ui_update_hover` initialises 16 frame-zone lists, then walks the
  hit-test bucket calling each class's hit test and adding every hit to its
  layer's list, the last hit becoming `hovered`; `ui_send_to_hovered` walks
  the layers from 15 down; `ui_forget_world` clears all 16.
  `planet_hit_test_method` is `entity_geom_contains_cursor` (world point
  below layer 10, screen point above, box test on the hit box); planet
  positions are written at construction and at save load only.
