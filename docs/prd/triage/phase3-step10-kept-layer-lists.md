---
title: "Layer lists kept sorted at registration (L1)"
description: "Each frame, both draw passes rebuild fifteen per-layer lists from scratch and re-bucket every drawable entity, though an entity's layer almost never changes; the registry should keep the draw order instead."
status: needs-triage
---

## Problem Statement

`draw_layers` runs twice a frame: once for the draw pass, once for the
overlay pass. Each call does the same thing from scratch:

- it takes fifteen empty lists from the frame zone;
- it walks the whole message bucket (103 / 132 / 130–132 entities for the
  draw pass on `start` / `draw-5564` / `draw-6000`; 5 / 62 / 59–61 for the
  overlay pass);
- for each entity it tests the layer bit, floors the layer and appends the
  entity to that layer's list, growing the list block by block;
- it dispatches the lists in layer order and throws them away.

The result changes only when an entity is registered or unregistered, or
when its layer changes. Layers are set at construction and change in
three places in the whole game. So nearly every frame rebuilds the same
order.

The report (`docs/psx-perf-levers.md`, L1) bounds the gain per window
frame:

| Run | `#1` (draw) | `#2` (overlay) | Total |
|---|---:|---:|---:|
| `start` | ≤ 23 | — | ≤ 23 |
| `draw-5564` | ≤ 137 | ≤ 16 | ≤ 153 |
| `draw-6000` | ≤ 95 | ≤ 2 | ≤ 95 |

These are upper bounds. They also hold the dispatch loop, which stays,
and some unmeasured selves. They were taken at O2 with the meter and the
filter compiled in.

L2 ("the overlay pass reuses the draw pass's sort") is the report's
alternative inside L1: `#2` alone, ≤ 16 / ≤ 2. The plan's rule "L1
against L2" applies.

This is an engine change, not a GPU one. It must not change a single
pixel or any game state: the draw order must stay exactly what it is
today.

## Solution

1. **The registry keeps the draw order.** For each of the two draw
   messages it keeps the layered order: the drawable members grouped by
   layer 1..15, each group in registration order. That is exactly what
   today's bucket pass builds. The order is updated in three cases:
   - on `entity_register`: append to the end of its layer's group;
   - on `entity_unregister`: remove, with the order of the rest kept;
   - on a layer change: move into the new group, at its registration rank.
2. **Layers change through one engine call.** Every write of an entity's
   layer goes through `entity_registry_set_layer`. This includes the
   constructors, the slipway's infraspace layer and the score panel's
   end-of-game layer 13. The call re-sorts only when the floored layer
   really changes on a constructed entity.
3. **`draw_layers` walks the kept order.** It no longer buckets
   anything. Per non-empty layer it applies the camera on the draw pass,
   then dispatches each entity as today.
4. **Measured as the plan requires:**
   - a throwaway gross-target probe opens the step;
   - the "L1 against L2" pair is decided on it;
   - the lever's delta is measured against a fresh baseline, on update as
     well as draw, because the work moves to registration time.

Draw order, pixels, e2e hashes, end states and `psx-check` captures stay
the same on both platforms. The draw passes get cheaper on every frame.

## User Stories

1. As the plan pilot, I want L1's gross target measured on the six short runs before any code, so that coverage is decided on today's cost as the plan's rules require.
2. As the plan pilot, I want the overlay pass's share measured on its own in the same opening probe, so that the "L1 against L2" pair is decided on a figure and the note can cite it.
3. As the plan pilot, I want a written rule for "covered" that matches steps 8 and 9 (under 10.6 hblanks a window frame on both `draw-5564` and `draw-6000`), so that the decision is mechanical.
4. As the plan pilot, I want the gross target cited next to the report's bound and its O3-scaled value, with the reason the per-family unit-cost table has no entry for it, so that the note follows the "cite both figures" rule.
5. As the plan pilot, I want the probe to leave the picture and the game state untouched, so that its run is valid and nothing it measures depends on what it changed.
6. As a game developer, I want one call that sets an entity's layer, so that no code can move an entity between layers without the draw order following.
7. As a game developer, I want that call's contract on its declaration (it also sets the layer bit, re-sorts only a constructed entity whose floored layer changed, is cheap when nothing changes), so that I know what it costs and when it acts.
8. As a game developer, I want every constructor to set its layer through that call, so that there is one way to write a layer.
9. As a game developer, I want the slipway's layer switch (2 in infraspace, 4 otherwise) to go through it, so that a slipway that connects to an infraspace planet moves to its new layer in the right place.
10. As a game developer, I want the score panel's switch to layer 13 to go through it, so that the end-of-game panel draws where it does today.
11. As a game developer, I want a button built with a layer key to take it through the same call, so that message box rows and the end-game buttons keep their layers.
12. As a game developer, I want a slipway dragged frame after frame (its layer rewritten with the same value) to cost no re-sort, so that dragging does not add update time.
13. As a game developer, I want an entity registered while it has no layer, or a layer outside 1..15, to stay out of the draw order and still be removable, so that today's "dropped, not fatal" rule holds.
14. As a game developer, I want an entity whose layer leaves 1..15 to leave the draw order, and one whose layer enters it to join at its registration rank, so that the order always equals the one today's bucket pass would build.
15. As a game developer, I want the draw order within a layer to stay registration order through any mix of registrations, removals and layer changes, so that overlapping sprites cover each other exactly as before.
16. As a game developer, I want the camera applied once per non-empty layer on the draw pass, before that layer's entities, as today, so that parallax is unchanged.
17. As a game developer, I want the dispatch to read the entity's class's draw or overlay method at draw time, as today, so that a test that swaps a class after registration still sees its spy called.
18. As a game developer, I want an assert, in builds with asserts, that fires when a draw or overlay method registers, unregisters or re-layers an entity during a pass, so that a mutation the kept order cannot follow is caught at once with a message.
19. As a game developer, I want an assert, in builds with asserts, that each dispatched entity's floored layer still matches its group, so that a layer written behind the registry's back is caught in the first test or replay that draws it.
20. As a maintainer of the registry, I want the message buckets and their order untouched, so that hit-testing, refresh, new-year dispatch and every test that ranks an entity in a bucket keep working.
21. As a maintainer of the registry, I want the kept order reset by `entity_registry_init`, `entity_registry_shutdown` and `entity_registry_forget_registries`, so that a return to the title, a load and a new game start from an empty order like the buckets do.
22. As a maintainer of the registry, I want the kept order bounded by the entity table's capacity, with a fatal error rather than an overrun if that is ever exceeded, so that the PSX build, which runs without asserts, cannot write past the array.
23. As a maintainer of the zones, I want the kept order outside the game zone, so that the game zone's byte accounting, its reserves and the tests that check them stay as they are.
24. As a maintainer of the zones, I want the frame zone's new peak from `make state` recorded, so that the frame zone's lower use is known for later steps.
25. As the PSX RAM budget's owner, I want the static cost recorded (about 4 KB on PSX) with `ram-room`, `game-ram-room` and `gpu-ram-room` holding, so that the cost is visible.
26. As the maintainer of the e2e replays, I want every `*.sha256` unchanged and `make end-state` unchanged, so that the change is proven not to touch the game.
27. As the maintainer of `psx-check`, I want every capture unchanged at 0 tolerance, so that PSX pixels are proven identical too.
28. As the plan pilot, I want the lever's delta per part (update, draw, submit) on all six short runs, with asserts mode and opt level, so that the work moved into registration is seen as well as the work removed from the draw passes.
29. As the plan pilot, I want L1 kept only if the two draw runs show a pooled draw gain and a pooled whole-frame gain, so that a lever that shifts more time into update than it saves in draw is reverted.
30. As the plan pilot, I want the update delta on the three one-frame update runs recorded, so that phase 4's spike work knows what registration now costs.
31. As the plan pilot, I want galaxy generation's extra registration cost estimated and named as judge frame 12, so that phase 4's spike list has it.
32. As the plan pilot, I want every assumption about step 9's outcome written as a hypothesis, so that a different outcome changes a known decision and nothing silently.
33. As a developer running `make test` and `make e2e`, I want them no slower, so that the fast loop stays fast.
34. As a maintainer, I want the order invariant tested by a randomized sequence against a reference built the old way, so that every edge (same-value rewrite, out-of-range layer, layer-less entity, removal from the middle) is covered without guessing them one by one.

## Hypotheses on step 9

Step 9 (`docs/prd/triage/phase3-step9-outlined-font.md`) is being
implemented while this PRD is written. Each hypothesis below is checked
when this step starts, against step 9's note in `docs/plan-21.md` and the
code at HEAD. When one does not hold, the decision it names changes as
stated.

- **H-9-1.** Step 9 is finished (committed, its note written) before this
  step starts, whether D1/B4 was kept or reverted. L1 shares no code with
  step 9 (the hud, the print primitive, the PSX font). So nothing here
  waits on its outcome except the baseline: every probe of this step
  takes its baseline at the HEAD it runs on.
- **H-9-2.** Step 9 left every e2e hash, every end state and every
  `psx-check` capture unchanged. "Unchanged" in this step means unchanged
  against the HEAD it starts from. If step 9 re-recorded anything (the
  plan forbids it before step 20), stop at chunk 0 and go to `fix-bug`.
- **H-9-3.** Step 9 did not touch `draw_layers`, the registry, the
  draw-pass dispatch or any layer write. Its new print primitive
  registers no entity during a draw. If it did touch one of them, chunk 0
  re-reads that code before the probe. The probe patch and the list of
  layer writes are rebuilt from HEAD.
- **H-9-4.** The probe tooling (`make psx-probe-baseline`,
  `make psx-probe PATCH=… PSX_PROBE_LABEL=…`, `PSX_SHORT_RUNS`) and
  `make frame-profile-unit-cost` work as in steps 8 and 9: asserts off,
  O3, no meter, no filter.
- **H-9-5.** Step 9 re-took the unit costs after its switch. Its table
  replaces `be39841`'s wherever this step cites one. L1 removes no draw
  call, so this changes only which table is cited (see "Measuring").
- **H-9-6.** The 2% threshold stays 10.6 hblanks (step 2.7's mean of
  530.2). No phase ends between steps 9 and 10, so no judge run moves it.
- **H-9-7.** Step 9 lowered draw on the draw runs, so the short runs'
  `representative` condition still fails. As in earlier notes, the
  failure is recorded and does not void a figure.
- **H-9-8.** The latest `warn-review` is still step 2.7's (none runs
  before step 16). This step extends `entity_registry.c`, which it did
  not flag, so there is no prerequisite chunk (see "Flagged code").

## Implementation Decisions

### Coverage, both ways

- **L1 against the levers already taken:** F1, G1, B1, G2, B3, D5, and
  D1/B4 under H-9-1. None of them touches the registry or the bucket
  pass. G2 made it cheaper but removed none of it. So L1 is expected not
  to be covered, and the measurement decides.
- **Gross target probe** (throwaway, uncommitted), `layers-twice`:
  - In `draw_layers`, before the real bucket pass, it runs the same pass
    a second time into a separate set of fifteen frame-zone lists. A
    volatile sink keeps the counts so the work cannot be optimized away.
  - Picture, dispatch and game state are unchanged. The delta is the
    cost of the work L1 removes: fifteen list inits, the walk, the layer
    test and floor, and the appends with their growth.
  - It is run with `make psx-probe` on all six short runs, against
    `make psx-probe-baseline` at the HEAD the step starts from.
  - It doubles the frame zone's per-frame list use. Before the run,
    check `make state`'s `zone frame` peak against its limit for room
    for a second copy. If there is none, the probe writes its copy into
    static scratch arrays instead, and the note says so.
- **Overlay share** (throwaway), `overlay-layers-twice`: the same
  doubling on the overlay pass only, run on
  `PSX_SHORT_RUNS="draw-5564 draw-6000"`. This is L2's gross target.
- **Rule** (steps 8 and 9's, recorded here as a decision): a lever is
  covered when its gross target is under 10.6 hblanks a window frame on
  both `draw-5564` and `draw-6000`.
- **L1 against L2.** L1's structure is per message, so it covers L2
  entirely:
  - L1's gross ≥ 10.6 on either draw run: L1 is built. L2 is skipped as
    covered by L1, and its gross figure goes in the note.
  - L1's gross under 10.6 on both: L1 is skipped and nothing is built.
    L2 is a part of L1, so it is skipped with it. The note gives both
    figures and the step ends.
- **Figures cited next to the gross target:**
  - The report's bound: 153 / 95 / 23 (`draw-5564` / `draw-6000` /
    `start`), at O2 with meter and filter.
  - Scaled by G2's measured update ratio (0.862, step 2.5: −13.8%),
    which is the right scale for plain engine C. The meter and the filter
    wrap draw calls only, so their removal does not scale this work. The
    scaled bound is about 132 / 82 / 20.
  - The re-measured per-family unit-cost table (step 3.8's, or step 9's
    under H-9-5) has no row for this work: L1 removes no `p8_*` draw
    call. The note says so and cites the table's date, which is how
    this step meets the "cite both figures" rule.

### Registry: the kept layer order (engine)

- **Invariant.** For each draw message (draw, overlay), the kept order
  equals what today's bucket pass builds from that message's bucket:
  - the members that have the layer bit and a floored layer in 1..15;
  - grouped by floored layer, ascending;
  - within a group, in bucket order.

  Bucket order is registration order: `entity_list_add` appends and
  `entity_list_del` keeps order. The message buckets themselves are not
  changed.
- **Storage.** Per draw message:
  - a static array of entity pointers of capacity `ENTITY_TABLE_CAP`
    (512), which is at least the peak bucket (322 draw, 265 overlay
    reserve);
  - sixteen group start offsets;
  - a count.

  This is about 4.2 KB of BSS on the PSX, outside every zone, so the
  game zone's accounting and the reserve tests do not move.
  - The game zone was rejected: its byte accounting, the reserves and
    the tests that check them would all have to change, for a
    structure whose bound is already fixed by the entity table.
  - Exceeding the capacity is a `p8_fatal` with the limit in its message,
    like a full entity table. It is never reached through
    `entity_construct`.
- **Register:** after the buckets are updated, for each draw message the
  entity belongs to, and only if its key (its floored layer, or 0 when it
  has no layer bit or the layer is outside 1..15) is in 1..15:
  - insert it at the end of its group (the next group's start);
  - shift the tail by one;
  - add one to every later start.

  It is the newest registration, so the end of its group is its
  registration rank. No scan is needed.
- **Unregister:** for each draw message, find the entity by pointer in
  the kept order. If it is there, remove it (the tail shifts down) and
  subtract one from every later start. That is the same O(n) as the
  bucket's own ordered delete.
- **Layer change, `entity_registry_set_layer(entity, layer)`.**
  - It writes the layer and sets the layer bit (no code clears that
    bit).
  - It compares the old key with the new key. If they are equal, it does
    nothing more. That is the common case: a slipway rewrites its layer
    with the same value while it is dragged.
  - If the key changed and the entity holds an entity-table slot (it went
    through `entity_construct`), then for each draw message whose bucket
    the entity's class puts it in:
    - remove it from the kept order if it is there;
    - if the new key is in 1..15, insert it into the new group at its
      registration rank. The rank is the number of the bucket's members
      before it, in bucket order, whose key equals the new key. One walk
      of the bucket finds it.
  - If the entity holds no slot, the layer is written and nothing else
    happens. That is a constructor before `entity_construct`: the key
    goes from 0 to the archetype's layer at almost every construction.
    The slot test keeps that free, so galaxy generation does not pay one
    bucket scan per entity.
  - **Contract on the declaration:** sets the layer and its bit. It
    re-sorts the draw order only for a constructed entity whose floored
    layer changed. An entity registered bare (`entity_register` alone, as
    some tests do) must not change its layer afterwards.
- **`draw_layers(message)`:**
  - for each layer 1..15 with a non-empty group: on the draw pass, the
    camera lookup and `apply_camera` exactly as today;
  - then each entity of the group in order: the class's draw or overlay
    method read at dispatch time, called with `entity_pos` when not
    NULL, as today;
  - no frame-zone list.
  - The function's signature and its assert on the message stay.
- **Traps, as asserts** (on in the PC build and the tests, off on the
  PSX):
  - A mutation counter in the registry goes up on every change to a kept
    order. `draw_layers` asserts it is unchanged after each dispatch.
    The message says a draw method must not register, unregister or
    re-layer an entity.
  - Before each dispatch, `draw_layers` asserts that the entity's
    current key equals its group. The message names
    `entity_registry_set_layer`. This catches a layer written directly.
  - Both are one compare per dispatched entity, far less than the bucket
    pass they replace, so `make e2e` does not get slower.
- **If the first assert fires in a test or replay** (a draw method does
  mutate the registry), the decision is already made: `draw_layers`
  copies the group being walked into the frame zone before dispatching
  it, and iterates the copy. That keeps today's snapshot behaviour at
  the cost of one block copy. The assert then guards only the
  key-match. The note records which method did it.
- **Read accessor for tests:** `entity_registry_layer(message, layer)`
  returns a read-only view (items, count) of one group. `draw_layers`
  uses the same view internally.
- **Lifecycle:** `entity_registry_init`, `entity_registry_shutdown` and
  `entity_registry_forget_registries` set every count and start to 0.

### Layer writes (game)

- Every write of an entity's layer in `src/` goes through
  `entity_registry_set_layer`:
  - the archetype layer in each constructor (background, message box
    ×2, planet, probe, score panel, slipway, structure icon, ui, button
    ×2, wormhole);
  - the button's `keys->layer` override;
  - `slipway_inherit_infraspace`, both before registration (from the
    constructor path) and after it (drag update, `slipway_connect`);
  - the score panel's layer 13.
- `button_keys_t.layer`, the message box's `b.layer` and the end-game
  keys' `layer` are fields of a key struct, not of an entity. They stay
  as they are.
- **No new static guard.** A grep guard for direct layer writes was
  considered and rejected: it would add a job and a test binary to
  `make test`, which must not get slower. The key-match assert covers
  the writes that matter (those that move an entity between groups) in
  every test and replay that draws them. The setter's contract covers
  the rest.

### Flagged code

- The latest `warn-review` (step 2.7) flagged no code this step extends.
  It changes nothing in the Makefile, the PSX harness, the replays or
  `psx_gpu_peaks`. So, by the plan's rule, there is no prerequisite test
  or derived figure (H-9-8).

### Measuring and verdict

- **Before (chunk 0):** the two gross probes, the coverage verdict, the
  frame zone check and the H-9 check.
- **The lever's delta (chunk 2):**
  - Measured with `make psx-probe`, using chunk 2's diff as the patch,
    against `make psx-probe-baseline` at chunk 1's HEAD, on all six short
    runs. Measuring before the commit avoids step 1's reversed
    measurement.
  - Chunk 1 changes no timing worth measuring: the setter is the same
    write plus a compare. Its delta is not separated.
- **Keep or revert:** kept when both hold, on the pooled `draw-5564` +
  `draw-6000` window:
  - the draw delta is a gain (the plan's rule on the target part);
  - the whole-frame delta (update + draw + submit) is a gain. This is
    added because L1 moves work into registration, which runs in update.

  Otherwise chunk 2 is reverted and noted. Chunk 1's setter stays as a
  facility, since one write path for layers is worth having anyway.
- **Resume frame:** a short run resumes from a save, which re-registers
  every entity. If the window's first frame carries that insertion cost,
  the plan's resume-frame rule applies. The lever is judged without that
  frame, and the one-off cost is recorded separately. Registration is
  append-to-group with a shift of the later groups, so the cost is
  expected to be small.
- **Spike input for phase 4:**
  - The update deltas of `update-4737`, `update-5500` and `update-5719`
    go in the note.
  - So does an estimate of galaxy generation's extra cost (judge frame
    12, the only frame that registers the whole galaxy). The estimate is
    the registrations of that frame × the measured per-registration
    cost, read from the resume frame or the update runs.
  - The judge itself is not run in this step: step 16 ends the phase.
- **Note contents:**
  - the gross targets (L1, L2 share) per run against 10.6, the report
    bound and its scaled value;
  - the measured delta per part per run, with asserts mode and opt
    level as the runs reported;
  - `representative` failures;
  - `make state`'s `zone frame` peak before and after;
  - `ram-room`, `game-ram-room` and `gpu-ram-room`;
  - e2e, `make end-state` and `psx-check` status.

## Implementation Chunks

0. **Gross target** (no source change).
   - Check H-9-1 to H-9-8 and record which hold.
   - Check the frame zone peak (`make state`).
   - `make psx-probe-baseline`, then `layers-twice` on the six short runs
     and `overlay-layers-twice` on the two draw runs.
   - Record the figures against 10.6, the report bound and the scaled
     bound. If covered, the step ends here with its note.
   - Figures go in the progress commit's message, as steps 8 and 9 did.
1. **One write path for layers** (no behaviour change).
   - `entity_registry_set_layer` writes the layer and its bit; nothing
     else yet.
   - Every layer write listed above moves to it.
   - Unit test: it sets both the field and the bit.
   - `make test-all` green. The e2e hashes are unchanged by
     construction.
2. **The kept layer order and the walk.**
   - The static order per draw message, kept on register, unregister
     and layer change; the read accessor; `draw_layers` walking it; the
     two asserts; lifecycle resets.
   - Unit tests below. `make test-all`, `make end-state` and `psx-check`
     green, with every hash and capture unchanged.
   - The `psx-probe` measurement of this diff, then keep or revert.
3. **Close.**
   - `make psx-short-runs` at the harness defaults, the `make state`
     frame zone peak, the RAM figures.
   - The step note in `docs/plan-21.md`.

## Testing Decisions

- **What makes a good test here:** only the registry's public interface
  is exercised (`entity_construct` / `entity_register`,
  `entity_unregister`, `entity_registry_set_layer`, `draw_layers`,
  `entity_registry_layer`). Expected orders are computed by a reference
  that rebuilds the grouping from `entities_by_message` the old way.
  Nothing is read from the registry's private arrays.
- **Registry tests** (the registry's existing test module):
  - **Randomized invariant.** A fixed-seed sequence of a few thousand
    operations on stub entities whose class has a draw method, an overlay
    method, or both:
    - construct with a layer in 0..16, or with no layer;
    - unregister a random live one (then flush the exit queue);
    - set the layer to a random value, the same value, or out of range.

    After every operation, each of the 15 groups of both messages equals
    the reference.
  - **Edges, one test each:**
    - a layer change into a group lands at registration rank, between
      older and newer members, not at the end;
    - a same-value rewrite does not move the entity;
    - a layer-less entity can be unregistered and is never dispatched;
    - a layer change before `entity_construct` costs no re-sort and
      lands at the end of its group at registration;
    - `forget_registries` and `init` empty both orders;
    - exceeding the capacity is fatal. Use the existing fatal-capture
      test pattern if one exists. Otherwise this is covered by
      reading, not by a test.
  - **Dispatch:** `draw_layers` calls the draw methods in kept order and
    applies the camera once per non-empty layer before that layer's
    entities. Prior art: `test_draw_layers_camera.c`, which must pass
    unchanged.
  - **Trap:** a stub draw method that constructs an entity trips the
    mutation assert. Run it in a forked child, as other assert tests do,
    or leave it out if the suite has no such pattern. The assert's
    message is the documentation either way.
- **Setter test:** sets the field and the bit (chunk 1).
- **Unchanged tests stay unchanged:** every test that ranks an entity in
  a message bucket, since the buckets do not change. Any unit test that
  froze the frame-zone use of `draw_layers` is updated, since the e2e
  replays are the truth.
- **Whole-game proof:**
  - every `tests/e2e/*.sha256` unchanged;
  - `make end-state` unchanged on all its replays;
  - `psx-check` green with every capture at 0 tolerance.

  The captures show many layers at once, so a wrong order would show as
  pixels.
- **Prior art:** `tests/test_entity_registry.c` (stub classes,
  `registry_world`), `tests/test_entity_list.c`,
  `tests/test_draw_layers_camera.c`.

## Out of Scope

- L2 as a separate structure (covered by L1, see "Coverage").
- L3 (the parallax camera computed per layer): step 15 decides it.
- L4 and D9 (skipping off-screen entities at dispatch).
- Changing the order or the content of any message bucket, or of the
  hit-test order.
- Dropping the "no layer / out of range is skipped" rule, or making it
  fatal.
- A static guard against direct layer writes (rejected above).
- Running the judge (`psx-full-game-check`): step 16 does it.
- Any measurement on the Miyoo or PCSX-ReARMed; audio.

## Further Notes

- **Timing-model caveat** (the report's label on L1): the gain trades a
  per-frame pass for reads of a standing structure. pcsx-redux charges
  no data wait, so the gain is real on the judge, and the plan accepts
  that.
- **Draw-time mutation:** the opening reading found no `entity_unregister`
  outside update paths (bootstrap's update walk, the ui, save-load,
  devtools). Construction during a draw method was not ruled out by
  reading. That is why the mutation assert exists and the snapshot
  fallback is decided in advance.
- **Layer writes at HEAD when this PRD was written:**
  - twelve constructor sites (message box and button have two each);
  - the button's key override;
  - `slipway_inherit_infraspace`, called from the slipway's construction
    path, its drag update and `slipway_connect`;
  - the score panel's update (layer 13).

  Chunk 1 re-greps HEAD in case step 9 added one (H-9-3).
