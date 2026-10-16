---
title: "Spike levers that keep the game state: U4, U6, U2, U1, U3, then the judge split into spike, image-build and other frames"
description: "The judge's update spikes (saves, slipway drops, wormhole launches, project and year recomputes, galaxy generation) repeat work whose result is already known; the step removes that work lever by lever, measures each one class by class on the judge's spike frames against the run just before it, keeps only exact gains, and closes with a judge run whose frames still over are split into the three groups steps 19 and 20 will spread."
status: needs-triage
---

## Problem Statement

Plan 21 step 18, verbatim:

> 18. Spike levers that keep the game state: U4 (table-driven CRC-32,
>     `src/game/save_state.c:601`), U6, U2, U1, U3. Each keeps every e2e hash
>     or is reverted and noted. Most spike classes appear in no short run,
>     so these levers are measured class by class on the judge's spike frames
>     (`psx/perf/spike-frames.txt`), each against the full run just before
>     it; the short runs serve only where they hold the class. Then
>     `psx-full-game-check`, with its summary, the frames still over split
>     into three groups with count and overrun each: (a) those in
>     `spike-frames.txt`, (b) the image-build frames from steps 8, 11, 12, 15
>     and 16's notes, (c) every other frame;
>     steps 19–20 spread those
>     frames, not the report's estimates.

After phase 3 (step 3.16) the judge has 3,942 of 6,059 frames over 263, a
mean frame of 318.4 and its worst frame at 12 (7,962, update 7,659). Draw
has been halved; update was only touched by G1 and G2. The update spikes
(166 frames in `psx/perf/spike-frames.txt`, 19 classes C1–C19, labels in
`psx/perf/gains.txt`) each carry work that computes a figure the game
already holds or that a later call recomputes anyway. The report
(`docs/psx-perf-levers.md`, "Update spikes (step 9)" and "Spikes and
spreading") names five such levers:

| Lever | Where (function) | Repeated work | Classes | Report gain (judge hblanks, allocated / bound) | Cost | Risk |
|---|---|---|---|---|---|---|
| U4 | `save_state_crc32` | bitwise CRC-32, 8 shift steps a byte, on every save | C5, C7, and the save of C1–C3, C8–C18 | 10,375 (bound 11,851) | S | timing model; RAM 1 KB |
| U6 | `entity_geom_nearest` in `galaxy_populate` | each candidate point scans for the true nearest planet while the loop only asks "is one within 0.05" | C6 (frame 12) | 3,980 (ceiling 7,963) | S | none (same draws, same planets) |
| U2 | `planet_refresh` from `economy_recompute`'s refresh pass | every recompute rebuilds the layout row of every planet | C1–C4, C8–C18 | 12,289 / 24,577 | M | timing only if exact; e2e decides |
| U1 | `slipway_drop` → `targeting_confirm` → `economy_advance_clock(0)` | a full recompute before the slipway is attached, then a second one after it | C1, C11 | 6,739 / 13,478 | M | game state unless the second recompute reaches every effect of the first; e2e decides |
| U3 | `wormhole_drop` → `targeting_confirm` → `economy_advance_clock(1)` → `economy_recompute` | a launch changes credits and one clock tick, not routes or production, yet recomputes everything | C4 | 5,446 / 10,892 (alternative to U2 on C4) | M | game state if a recomputed figure changes; e2e decides |

The report priced them at O2 with asserts, meter and filter in; since then
G2 took 13.8% off update, and step 17 (O2, O3, O5 kept; `### Step 4.17`)
cut the ordinary update work present on every frame, spike frames
included, without a judge run: the latest judge table is still step
3.16's (`a5c8d88`).

Two facts shape the method:

- **The short runs barely hold these classes.** Only `start` (frame 12,
  C6) and `update-5500` (frame 5500, C1, with its save) hold a class frame;
  `update-4737` and `update-5719` lose their spike on resume; the draw
  windows hold none. So the plan requires each lever to be measured on the
  judge's spike frames, against the judge run just before it: one
  `psx-full-game-check` (about 12–20 min) per lever built.
- **Steps 19 and 20 spread what is left, frame by frame.** They need the
  frames still over after these levers, split into (a) spike frames,
  (b) frames that build cached images (map, rows, text), (c) the rest, with
  a count and an overrun per group, measured, not estimated.

## Solution

One chunk of starting-state checks, a baseline judge run when step 17
changed the code, host-side gross targets and exactness checks decides
which levers are built. Then each lever, in the plan's order U4, U6, U2,
U1, U3, lands in its own commit, gated by `make test-all`, `make
end-state`, `make state` and `psx-check`, then measured by a judge run
against the run just before it, class by class on its spike frames. It is
kept when its target frames gain and every e2e hash, end-state, capture and
reference is unchanged; otherwise it is reverted and noted. Each kept
lever's run replaces `psx/perf/full-run-frames.txt`.

What each lever does, from the player's point of view nothing:

- **U4:** the save's CRC-32 is computed from a 256-entry table, one lookup
  a byte. Same bytes, same CRC, same save.
- **U6:** galaxy generation rejects a candidate point at the first planet
  within 0.05 instead of finding the nearest one. Same random draws, same
  planets.
- **U2:** a planet's layout row is rebuilt only when what the row shows
  (needs and their satisfied flag, output, amount, route count) changed
  since it was built.
- **U1:** a slipway drop charges and changes state as today, but advances
  the clock (and recomputes the economy) once, after the slipway is
  attached, instead of also recomputing before.
- **U3:** a wormhole launch advances the clock and refreshes the
  credit-dependent end status without a full economy recompute; a new year
  still recomputes in full.

Finally the closing judge run's summary, and the frames still over in three
groups with count and overrun each, go in the step note; the frame list of
group (b) is committed for step 20.

## User Stories

1. As the plan pilot, I want every assumption about step 17's outcome written as a hypothesis and checked before anything is measured, so that no decision rests on an outcome that did not happen.
2. As the plan pilot, I want a baseline judge run at the starting HEAD when step 17 changed `src/`, so that U4's delta is not credited with step 17's gains.
3. As the plan pilot, I want each lever measured against the judge run just before it, so that each lever is credited only with what it removes.
4. As the plan pilot, I want each lever's delta read on its own classes' spike frames (the `list` of each class in `spike-frames.txt`), so that a gain is measured where the report expects it.
5. As the plan pilot, I want the whole-run update total read too, so that a lever that gains on its classes but costs elsewhere is not kept.
6. As the plan pilot, I want the timer jitter bound (3 hblanks a frame, step 1.2) used to tell a gain from noise, so that a kept lever is a real gain.
7. As the plan pilot, I want each lever's gross target estimated on the host before it is built, from the PC profile of its target function on its class frames scaled to the judge, so that the note puts every measured gain next to an upper bound.
8. As the plan pilot, I want the short runs used where they hold the class (`start` for U6, `update-5500` for U4, U2 and U1), so that the plan's "short runs serve only where they hold the class" is applied and recorded.
9. As the plan pilot, I want U2 and U3 treated as the report's alternatives on C4, U3 re-estimated after U2 and skipped as covered when its residual is under 6.4 hblanks a class frame, so that work U2 already removed is not built twice.
10. As the plan pilot, I want U1 re-estimated after U2 too, so that its note says what U2 left of it.
11. As the plan pilot, I want host-side exactness checks run before U2, U1 and U3 are built, so that a lever that would change the game is skipped without a build.
12. As the plan pilot, I want a lever that changes any e2e hash, `make end-state` line, capture or capture reference reverted whatever its gain, so that the plan's "keeps every e2e hash or is reverted" holds.
13. As the plan pilot, I want `psx/perf/full-run-frames.txt` replaced only by the run of a kept lever, so that the next lever is measured against the code that stays.
14. As the plan pilot, I want the closing judge run's summary (frames over, worst, mean) and every other condition's status in the note, so that phase 4 continues from a measured state.
15. As the plan pilot, I want the frames still over split into (a) spike frames, (b) image-build frames, (c) the rest, with count and overrun each, so that steps 19 and 20 spread measured frames, not the report's estimates.
16. As the plan pilot, I want the image-build frames listed by a host count at the closing HEAD, by kind (map, rows, text), so that group (b) is a frame list, not the counts the notes give.
17. As the plan pilot, I want a frame that is both a spike frame and an image-build frame counted in (a) only, so that the groups partition the frames over.
18. As the author of step 20, I want group (b)'s frame list committed with how it was made, so that I spread exactly those frames.
19. As a player on PC, I want saving, loading, galaxy generation, slipway drops, wormhole launches, projects and new years to behave exactly as before, so that the change is invisible.
20. As a player on PSX, I want the same game as on PC, frame for frame, so that the platforms stay identical.
21. As a player, I want a save written by the new build to load in the old one and the reverse, so that the CRC change is invisible on disk.
22. As a player, I want the same galaxy from the same seed, planet for planet, so that U6 changes nothing I see.
23. As a player, I want a planet's row of needs and output to show its new state on the frame it changes, so that U2 never shows a stale row.
24. As a player, I want a slipway drop to give the same credits, income, tiers, year and rows as before, so that U1 is invisible.
25. As a player, I want a wormhole launch to give the same credits, end status (including bankruptcy) and new year as before, so that U3 is invisible.
26. As a game developer, I want the CRC table derived and checked by a test against the bitwise definition, so that the table can never drift from the polynomial.
27. As a game developer, I want the "any planet within" scan in the engine, planet-free, with the same same-position rule as `entity_geom_nearest`, so that the early exit answers exactly what the old comparison answered.
28. As a game developer, I want the row signature to hold every input `planet_resource_row_keys` reads from the planet, so that an equal signature means an equal row by construction.
29. As a game developer, I want a released row to invalidate its signature, so that a planet whose row was freed always rebuilds.
30. As a game developer, I want the slipway drop's charge split from its clock advance through one shared body, so that every other caller of `targeting_confirm` keeps its exact sequence.
31. As a game developer, I want the wormhole's clock-only path selected by its targeting archetype, so that no class name appears in the economy or the targeting code.
32. As a game developer, I want the end-status test shared between the full recompute and the clock-only path, so that the two can never disagree.
33. As a maintainer of the e2e replays, I want every `*.sha256` and `make end-state` unchanged after each kept lever, so that the game is proven untouched.
34. As the human who alone accepts capture references, I want every capture and reference unchanged, so that I accept nothing new.
35. As a developer running `make test` and `make e2e`, I want them no slower, with no debug cross-check added, so that the fast loop stays fast.
36. As the PSX RAM budget's owner, I want `ram-room`, `game-ram-room` and `gpu-ram-room` recorded after the step, with U4's table and U2's signatures accounted for, so that any growth is visible.
37. As a maintainer of `make state`, I want slot peaks recorded before and after U2, so that the change in row-slot churn is visible.
38. As the plan pilot, I want every judge run's asserts mode and opt level recorded as the run reported them, so that no figure taken at another setting is used.

## Starting state: hypotheses on step 17, and their outcomes

This PRD was first written before step 17 (O2 or O1, O3, O5) was done, so
each assumption about its outcome was a hypothesis. They were then checked
against `docs/plan-21.md`'s `### Step 4.17` note and commits `7b4b50f`
(O2), `e9c2395` (O3, O1 covered) and `9ccfd1d` (O5), at HEAD `74c0b4f`.
The outcome follows each one. Chunk 0 re-checks only the two left open
(H-17-8's frame profile, H-17-12) and confirms that HEAD has not moved
since `74c0b4f`. If it has, every hypothesis is checked again against the
new commits.

- **H-17-1.** Step 17 is done and its note is in `docs/plan-21.md`. It ran
  no `psx-full-game-check`, so `psx/perf/full-run-frames.txt` is still step
  3.16's table (judge on `a5c8d88`).
  **Outcome: holds.** The note is committed (`74c0b4f`), with no judge run
  since `a5c8d88`.
- **H-17-2.** Step 17 kept at least one lever, so `src/` differs from
  `a5c8d88`'s and update moved on every frame, spike frames included. A
  baseline judge run at the starting HEAD is needed before U4.
  **Outcome: holds.** O2 kept (pooled −1.4 a window frame; −16 on the
  one-frame update-spike runs), O3 kept (−9.9), O5 kept (−0.4); O1 skipped,
  covered by O2 (residual 0.00 / 0.60). **Chunk 0 runs the baseline
  `psx-full-game-check` at the starting HEAD; it is not optional.**
- **H-17-3.** Step 17 changed only `pad_panel`, `pad_nav`, the ui's hover
  pass, the entity registry and `entity_geom.h`. It changed none of the
  functions this step touches.
  **Outcome: fails in part.** Step 17 also changed:
  - `geom.h` and `geom.c`: `box_contains` became a `static inline` min/max
    test in the header, and its out-of-line definition was removed;
  - `entity_geom.c`: `entity_geom_contains_cursor` now wraps the header's
    inline body;
  - `ui.h`;
  - the Makefile: the `test_pad_panel` build rule and its `UNIT_TESTS`
    entry.

  None of `save_state_crc32`, `entity_geom_nearest`, `nearest_step`,
  `map_pos_of`, `galaxy_populate`, `planet_refresh`, `economy_recompute`,
  `economy_advance_clock`, `economy_new_year`, `targeting_confirm`,
  `slipway_drop` or `wormhole_drop` changed. No U lever uses `box_contains`
  or the contains-cursor body. U6 works on squared map distances through
  `nearest_step`'s same-position rule, which `e9c2395` did not touch.
  **Adaptation:** U6's new function goes in `entity_geom.c` next to
  `entity_geom_nearest`. It uses neither `box_contains` nor the inline
  contains-cursor body, and leaves both as step 17 left them. The new
  `test_entity_geom.c` must not rebuild `box_contains` out of line (it is
  header-only now). No other decision changes.
- **H-17-4.** Every e2e hash, `make end-state` line, capture and capture
  reference at the starting HEAD is the one step 17 kept, so this step's
  baselines are valid.
  **Outcome: holds.** The note says every e2e hash, end state, `make state`
  peak and capture reference is unchanged. `all 233`'s reference is still
  pending for the human.
- **H-17-5.** The phase 4 threshold for "covered" is still **6.4
  hblanks** (2% of step 3.16's 318.4).
  **Outcome: holds.** Step 17 used 6.4 and ran no judge run. The chunk 0
  baseline run does not move the threshold: it is the phase's, fixed by
  step 3.16's note.
- **H-17-6.** `psx/perf/spike-frames.txt`, `psx/perf/gains.txt`,
  `psx/short_runs.txt` and `worst-case/full_game.json` are unchanged, so
  the class lists, labels and frame numbers are step 9's.
  **Outcome: holds.** `psx/` and `worst-case/` are unchanged from
  `d16a6a3` to `74c0b4f`.
- **H-17-7.** No `warn-review` ran since phase 3's, and no flagged code is
  extended here, so there is no prerequisite chunk.
  **Outcome: holds, with one precision.** No warn-review ran in step 17. As
  step 17 did for `test_pad_panel`, this step adds one unit-test build rule
  and its `UNIT_TESTS` entry (`test_entity_geom`) to the Makefile. That is
  routine registration of a test, not an extension of the flagged
  Makefile logic. The step adds no other Makefile target and no check
  condition.
- **H-17-8.** The probe tooling and the PC frame profile work as before;
  `representative` still fails on every short run.
  **Outcome: probe tooling holds** (step 17 ran `make psx-probe` on all six
  runs; `representative` failed on every run, as expected). **The frame
  profile is unconfirmed:** step 17 did not use it. Chunk 0 checks that
  `make frame-profile` still reports same-game yes on `full_game.json`
  before it uses the profile for gross targets.
- **H-17-9.** No committed tool counts image builds per frame, so this step
  writes its own throwaway count for group (b).
  **Outcome: holds.** Step 17's counts were throwaway, and none of its
  commits adds a devtool.
- **H-17-10.** Step 17's note gives the starting RAM figures.
  **Outcome: holds.** **`ram-room` 154,940, `game-ram-room` 307,772** (step
  17's close, at `9ccfd1d`). This step's RAM figures are given against
  them. `gpu-ram-room` is still step 3.16's 154,480, since step 17 made no
  judge run; the baseline run re-measures it.
- **H-17-11.** If O2 was kept, its memo key holds nothing a recompute
  writes, so U1 and U3 leave it exact.
  **Outcome: holds, with a correction.** O2 was kept. Its key holds the ui
  and economy **entity ids** (not pointers), the box's ephemerals count and
  `entity_registry_generation()`, which every register, unregister and
  registry reset moves. U1 and U3 remove recomputes, which write none of
  these. U2 acquires and releases fewer row slots, and a row slot is not a
  registered entity, so the generation is untouched. No extra pad-step
  test is needed.
- **H-17-12.** The PC replay's frame index equals the judge table's frame
  index, so host counts and judge frames can be joined by frame number.
  **Outcome: not tested by step 17.** It holds by step 3.16's use. Chunk 0
  confirms it on frame 12: the galaxy build is a C6 frame in
  `spike-frames.txt` and in the host count, and the map's image build
  falls on the same frame.

## Implementation Decisions

### The rules applied in this step

- **Part read:** update. Draw and submit deltas are recorded, not used.
- **Target frames of a lever:** the frames listed by `spike-frames.txt` for
  the classes the report gives it, labels from `psx/perf/gains.txt`:
  U4 C1–C3, C5, C7–C18; U6 C6; U2 C1–C4, C8–C18; U1 C1, C11; U3 C4.
- **Measured gain:** the judge run with the lever against the judge run
  just before it (`psx/perf/full-run-frames.txt` at that moment): per
  class, frames, summed update delta and mean per frame; pooled over the
  lever's target frames; and the whole-run update total.
- **Keep:** kept when (1) the mean update delta over its target frames is
  below −3 hblanks a frame (step 1.2's timer jitter), (2) the whole-run
  update total does not rise by more than 3 hblanks × the number of
  non-target frames that changed, and (3) every e2e hash, `make end-state`,
  capture and reference is unchanged. Otherwise reverted (a revert commit,
  as step 3.11) and noted. The run of a reverted lever never replaces the
  table.
- **Short runs:** where a run holds a class, `make psx-probe` with the
  lever's diff as the patch, on that run only (`PSX_SHORT_RUNS=start` for
  U6, `update-5500` for U4, U2, U1), against a baseline at the HEAD before
  the lever. Recorded with its asserts mode and opt level; not used for the
  keep decision (the plan makes the judge decide).
- **Gross target (host):** for each lever, before it is built, the PC
  profile of `full_game.json` with the target function as update root
  (the mechanism `make frame-profile-spikes` uses for each symbol of its
  walk, run throwaway into `tmp/`), its share of each target frame's PC
  update, times that frame's update in the latest judge table, summed over
  the target frames. This is the step's "throwaway probe of its gross
  target cost" for classes no short run holds. Where a short run holds the
  class, a "twice" probe adds the PSX figure: CRC computed a second time
  into a `volatile` sink (U4), the candidate scan run a second time (U6),
  the refresh pass run a second time (U2), the first recompute run a
  second time (U1). A twice probe whose game state differs is invalid and
  noted; the host figure stands.
- **Covered:** the plan's covered rule, on the judge since no short run
  holds C4: before U1 and U3, the gross target is re-estimated on the HEAD
  just before them. U3 is skipped as covered (by U2) when its residual is
  under **6.4 hblanks** a C4 frame on average; U1 likewise against what U2
  left of C1 and C11. The note records the residual and the figure used.
- **E2e decides:** any changed `tests/e2e/*.sha256`, any `make end-state`
  difference, any capture or reference change reverts the lever. No hash is
  re-recorded in this step (step 20 does).
- **Judge setting:** every judge run is `make psx-full-game-check` at its
  defaults (asserts off, O3, no draw filter, no draw meter); the note
  records the asserts mode and opt level it printed. A run at any other
  setting is invalid.
- **Order:** U4, U6, U2, U1, U3, each built only if it stands after chunk 0
  and its covered check.

### Chunk 0: host counts and exactness checks (PC, nothing tracked written)

From throwaway instrumented PC builds replaying `worst-case/full_game.json`
and every e2e replay, output in `tmp/`:

- **Saves:** bytes per save, saves per frame, which spike frames save
  (turns U4's gross target into a per-byte figure).
- **U6:** candidate points tried, scans, planets visited per scan, and how
  many steps an early exit would visit (exact count of what U6 removes).
- **U2 signature check:** at every `planet_refresh`, whether the inputs
  signature (below) equals the one of the planet's current row, and if so
  whether the keys built now equal the keys the current row was built from,
  field by field. **One equal signature with different keys fails U2's
  design:** the missing input is added and the check repeated; if no
  planet-held input explains it, U2 is skipped on exactness. Also the
  largest needs count of any planet (sets the signature's capacity), and
  per recompute how many rows an equal signature would skip.
- **U1 check:** at every slipway drop, whether the first (0-tick)
  `economy_advance_clock` starts a new year (expected: never), whether its
  recompute changes any economy figure (tiers, production, demand, income,
  happiness, end status) or any planet row, and whether the second recompute
  ends on the same figures with and without the first (two throwaway runs
  compared drop by drop). Any drop where they differ fails U1 on
  exactness: U1 is skipped without a build.
- **U3 check:** at every wormhole launch, the economy figures and every
  planet row before the launch's recompute against after it. Expected:
  equal except the end status. Any other difference fails U3 on exactness:
  U3 is skipped without a build.

### U4: table-driven CRC-32

- `save_state_crc32` keeps its signature and result; its body becomes the
  byte-wise table loop (`crc = table[(crc ^ byte) & 0xFF] ^ (crc >> 8)`).
- The table is a `static const uint32_t[256]` literal in the source
  (1 KB of read-only data, no first-call build, so no one-off cost on any
  frame). Decision: literal over a table built at init, which would add a
  one-off cost to some frame and 1 KB of writable RAM.
- Timing-model caveat (report label): the gain rests on pcsx-redux charging
  no data wait for the table read. Accepted (plan rule).

### U6: early-exit scan in galaxy generation

- The engine gains `entity_geom_any_within(p, list, threshold)`: 1 at the
  first entity of the list whose squared map distance to `p` is at most
  `threshold`, skipping entities at exactly `p` (the same-position rule of
  `nearest_step`), 0 otherwise or for a NULL list. It answers exactly
  `entity_geom_nearest(p, list, &n) <= threshold` for every threshold under
  `entity_geom_nearest`'s starting bound; its contract line says so.
- `galaxy_populate`'s loop calls it; the random draws (two `rnd` a
  candidate) and the order of candidates are unchanged.
  `entity_geom_nearest` is unchanged for its other callers.

### U2: a planet row rebuilt only when its inputs changed

- `planet_t` gains a row signature: valid flag, has-output, output, amount,
  route count, needs count, and per need its resource number (or nil) and
  its "satisfied and zero" flag, the only values `planet_resource_row_keys`
  reads from the planet (other inputs are constants: the `w` resource code,
  the spacer row, spacing 0).
- `planet_refresh` computes the signature first; when it is valid and
  equal, it returns without building keys or a row (same row, same slot).
  Otherwise it builds as today and stores the signature.
- Capacity: the largest needs count from chunk 0, rounded up to 4. A planet
  with more needs always rebuilds (exact by construction) and is counted.
- `planet_release_row` invalidates the signature; a new planet starts
  invalid. Any other path that frees or replaces a planet's row (chunk 0
  greps `layout_row_slot` writes) invalidates it too.
- Same pixels: the row is a pure function of its keys, and the keys of the
  signature (checked by chunk 0); `row_image` is content-keyed, so a reused
  row draws from the same slot as a rebuilt equal one.
- Side effect, recorded: fewer row-slot acquire/release pairs, so `make
  state`'s row-slot peaks may fall. Zones are dropped from `make end-state`,
  and `psx-full-game-check`'s zone and cap conditions accept lower peaks.

### U1: one clock advance per slipway drop

- `targeting_confirm`'s body splits into a shared charge (spend, state,
  `is_dead` on refusal) and the clock advance. A new
  `targeting_confirm_without_clock` runs the charge only; `targeting_confirm`
  is charge then clock, unchanged for every other caller.
- `slipway_drop` calls the new function; its existing
  `economy_advance_clock(1)` after both attaches stays and becomes the
  drop's only clock advance. The slipway archetype's `clock_ticks` is 0,
  so only a 0-tick advance (save pending, a full recompute, a new-year test
  that chunk 0 shows never fires) is removed; the save is still pending
  through the second call.
- `slipway_drop_method` (C11) reaches it through `slipway_drop`.

### U3: a wormhole launch without a full recompute

- The targeting archetype gains a flag "the clock advance needs no
  recompute", set on the wormhole archetype only. `targeting_confirm` then
  calls a new `economy_tick_clock` instead of `economy_advance_clock`.
- `economy_tick_clock` does what `economy_advance_clock` does (save pending,
  `time compression` gate, ticks added, new year past the gate with its
  full recompute) except the recompute, replaced by the end-status test.
  The end-status test (final score, bankrupt, none) becomes one function
  shared with `economy_recompute`'s tail.
- If U2 was kept, U3 is judged on its residual first (covered rule).

### Closing judge run and the three groups

- The closing `psx-full-game-check` runs at the step's final HEAD. If the
  last judge run of a kept lever was taken on the same `src/` and `psx/`
  tree ids (no later change, only reverts back to it), that run is the
  closing one and the note says so; otherwise it runs again.
- Its summary in the note: frames over, worst frame (with update, draw,
  submit), mean frame, mean overrun, every other condition's status
  (14 today, read from its output, not hand-copied), zones and caps,
  `gpu-ram-room`.
- **Groups**, each frame over 263 in exactly one: (a) its number is a
  `frame` line of `spike-frames.txt`; else (b) it builds at least one
  cached image (map, row or text) in the host count; else (c). Per group:
  count, summed overrun (total − 263), mean and largest overrun, worst
  frame. Group (b) also split by kind. Step 11's D8 builds were reverted
  (`### Step 3.11`), so no planet image kind exists; the note says so.
- **Image-build count:** a throwaway instrumented PC build replaying
  `full_game.json` at the final HEAD prints, per frame, the number of
  `p8_image_build` calls by kind (slot range). Committed as
  `psx/perf/image-build-frames.txt` (one line a frame with builds: frame,
  map, rows, text), with a header naming the commit it was counted on and
  how; step 20 reads it. Decision: committed data, not a new Makefile
  target (H-17-7: the Makefile is flagged code; the only Makefile change
  in this step is `test_entity_geom`'s build rule and `UNIT_TESTS` entry).
- The split itself is a throwaway join in `tmp/`; its result goes in the
  note, not in a tool.

### What is not changed

- `entity_geom_nearest` and its other callers, the save format and its
  bytes, `economy_recompute`'s figures, every other caller of
  `targeting_confirm` (probes, help screen, structures, wormhole before
  tick 15), the planet row's layout and drawing, `row_image`.
- U5 (save one frame later), U7 (spread galaxy generation), U8, S: steps
  19–20.
- The report's line numbers are the report's; the functions named here are
  what counts.

## Implementation Chunks

0. **Starting state, baseline, gross targets, exactness** (no committed
   source change).
   - Confirm HEAD is still `74c0b4f` (or re-check every H-17-k against
     the newer commits); check the two left open (H-17-8's frame profile,
     H-17-12); record the outcomes already settled above.
   - Baseline `make psx-full-game-check` at the starting HEAD (required:
     H-17-2 held, no judge run since `a5c8d88`); its table replaces
     `psx/perf/full-run-frames.txt` in a
     `chore:` commit with its summary in the message.
   - Host gross targets for the five levers on their target frames;
     `make psx-probe-baseline PSX_SHORT_RUNS="start update-5500"` and the
     twice probes on the runs that hold a class.
   - The host counts and the U2, U1, U3 exactness checks.
   - Per lever: standing, or skipped on exactness, with figures. Figures
     and outcomes in a progress commit's message, as steps 8 to 17 did.
1. **U4.** The literal table and the byte-wise loop; tests; `make
   test-all`, `make end-state`, `make state`, `psx-check`; commit;
   short-run probe on `update-5500`; judge run; keep (table replaced) or
   revert.
2. **U6.** `entity_geom_any_within` in `entity_geom.c` (step 17's inline
   contains-cursor body and inline `box_contains` left as they are; the new
   `test_entity_geom` registered in the Makefile as step 17 registered
   `test_pad_panel`), `galaxy_populate` switched; same
   gates; probe on `start`; judge run; keep or revert.
3. **U2** (if standing). The signature, its invalidation, `planet_refresh`'s
   early return; same gates plus `make state` slot peaks before and after;
   probe on `update-5500`; judge run; keep or revert.
4. **U1** (if standing and not covered after chunk 3). Residual estimate;
   the charge split, `slipway_drop` switched; same gates; probe on
   `update-5500`; judge run; keep or revert.
5. **U3** (if standing and not covered). Residual estimate; the archetype
   flag, `economy_tick_clock`, the shared end-status test; same gates;
   judge run; keep or revert.
6. **Close.** Closing judge run (or reuse, per the rule above), table
   replaced; the image-build count committed as
   `psx/perf/image-build-frames.txt`; the three groups; `ram-room`,
   `game-ram-room`, `gpu-ram-room`, each against its starting figure
   (154,940, 307,772, and the baseline run's `gpu-ram-room`). The step note in `docs/plan-21.md`
   under `### Step 4.18`: hypotheses' outcomes; per lever host gross
   target, short-run figure where held, judge figure per class and pooled,
   whole-run delta, report figure, decision; the closing summary and the
   three groups with count and overrun each.

## Testing Decisions

- **What makes a good test here:** the outputs are bytes (a CRC), a yes/no
  (a point too close), a row, and economy figures. Tests drive public calls
  and compare with a reference computed by the test (the bitwise CRC, the
  nearest-distance comparison, a freshly built row, the old call sequence
  reproduced in the test). No test reads a memo or a signature.
- **U4 (`tests/test_save_state.c`):** the CRC equals a bitwise reference in
  the test on the standard check value, the empty buffer and buffers of
  every length up to a few hundred bytes of pseudo-random content; every
  table entry equals its bitwise derivation (computed by the test); a
  save written and read back round-trips as before.
- **U6 (`tests/test_entity_geom.c`, new):** `entity_geom_any_within`
  equals `entity_geom_nearest(...) <= threshold` over random point sets,
  with points at exactly `p`, a NULL list, an empty list, thresholds of 0,
  0.05 and large.
- **U2 (`tests/test_planet.c`):** after a refresh with unchanged inputs the
  planet's row lays out the same items as a row freshly built from the same
  keys; after each change (a need added or removed, a need's satisfied
  flipping to and from zero, output changed, amount crossing 5, route count
  changed) the row equals a fresh build; after release and refresh, a
  fresh build; a planet over the signature capacity always equals a fresh
  build.
- **U1 (`tests/test_slipway.c`):** a drop gives the same credits, income,
  year, year ticks, tiers, end status, slipway state and planet rows as the
  pre-lever sequence (charge, 0-tick advance, attaches, 1-tick advance)
  reproduced in the test; a refused drop is unchanged.
  **(`tests/test_probe.c`):** `targeting_confirm` unchanged for a probe and
  a structure.
- **U3 (`tests/test_wormhole.c`, `tests/test_economy.c`):** a launch gives
  the same credits, year ticks, year, end status and figures as the
  full-recompute path reproduced in the test, including a launch that
  leaves the player bankrupt and one that crosses into a new year; a launch
  before tick 15 is unchanged; `economy_recompute`'s end status unchanged.
- **Whole-game proof:** every `tests/e2e/*.sha256` and `make end-state`
  unchanged after each lever; `make state`'s zone peaks recorded; `psx-check`
  green with every capture and reference unchanged.
- No debug cross-check (signature against keys, early exit against nearest)
  is added to `make test` or `make e2e`: it would slow the fast loop. The
  chunk 0 host checks, the unit tests and the e2e hashes carry the proof.
- No test for a lever chunk 0 skips. A unit test that froze old behaviour
  and breaks while every e2e hash holds is updated (CLAUDE.md).

## Out of Scope

- Spreading: U5, U7, U8, S, the spreading mechanism and applying it (steps
  19 and 20); re-recording e2e hashes.
- O1–O5 (step 17), any draw lever, any lever not in the step.
- Speeding up `entity_geom_nearest` for its other callers, or the economy
  recompute beyond U2 and U3.
- A new Makefile target or check condition for the group split.
- Accepting capture references; merging `claude/plan-21-d8-pending`.
- Measuring on the Miyoo or PCSX-ReARMed; audio.

## Further Notes

- **Expected outcome, not a decision:** U6 is the largest single-frame gain
  (frame 12, a few thousand hblanks before G2) but leaves frame 12 far over
  263: step 20 spreads it (U7). U4 and U2 spread small gains over most of
  the 166 spike frames. U1 and U3 may fail exactness or be largely covered
  by U2. A step that keeps only U4 and U6 is a valid outcome; frames over
  will fall little, since group (c) (ordinary frames over 263, mostly draw)
  is untouched by these levers.
- **Run time:** up to seven judge runs (baseline, five levers, closing),
  each about 12–20 minutes, outside the fast loop.
- **Why the judge and not the short runs:** `update-5500` holds one C1
  frame and `start` the C6 frame; C4, C5, C7 and most others appear in no
  short run.
- **Timing-model caveat:** U4 rests on no data-wait charge for its table;
  U6, U2, U1, U3 remove instructions only.
- **Starting figures** (step 17's close): `ram-room` 154,940,
  `game-ram-room` 307,772; `gpu-ram-room` 154,480 (step 3.16, re-measured
  by the baseline run); judge table step 3.16's (3,942 over, mean 318.4,
  worst frame 12 at 7,962).
- **Code read at HEAD `d16a6a3`** when this PRD was first written, then
  re-checked at `74c0b4f` (step 17 changed none of it; see H-17-3):
  `save_state_crc32` is bitwise (8 steps a byte), called on encode and on
  decode; `galaxy_populate` draws two `rnd` per candidate and loops while
  `entity_geom_nearest(p, planets) <= 0.05`, `nearest_step` skipping a
  point at exactly `p`; `economy_recompute` iterates tiers to a fixed
  point, sums production, income, happiness, sets the end status, then
  calls every refresh-message entity's `refresh` (planets:
  `planet_refresh`, which builds keys from needs, output, amount and route
  count and a new row in a fresh slot, releasing the old one);
  `targeting_confirm` spends, sets the confirmed state and calls
  `economy_advance_clock(clock_ticks)` (slipway 0, wormhole 1, structures 1
  or 2); `slipway_drop` calls it, attaches both ends, then
  `economy_advance_clock(1)`; `economy_advance_clock` sets save pending,
  adds ticks, recomputes, and starts a new year past 15 ticks (12 with
  time compression), which recomputes again.
