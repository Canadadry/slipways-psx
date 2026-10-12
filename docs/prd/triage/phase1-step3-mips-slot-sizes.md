---
title: "Re-measure the UI and particle slot sizes on MIPS"
description: "The UI and particle zone slot sizes are PC figures (8-byte pointers) shared by the PSX build, so the MIPS build reserves RAM it never uses; each slot needs a size measured or proven on MIPS that still leaves the PC build green."
status: needs-triage
---

## Problem Statement

The UI zone (`button`, `modal_backdrop`, `message_box`, `toast`, `row`,
`ephemerals`) and the particle zone (`particle`, `particles`,
`particles_help_screen`, `burst`, `transit_dots`) reserve every slot statically,
at a byte size measured on the PC build. Both builds share those sizes today.
On MIPS, pointers are 4 bytes instead of 8, so every slot holding pointers
needs less room. The PSX build therefore reserves dead RAM in roughly 280 KB of
slot storage (`button` 15 × 6,056, `row` 66 × 1,184, `message_box` 4 × 5,128,
`toast` 48 × 344, `particles` 18 × 1,384, ...), and that RAM is what step 4
of plan 11 has to optimize within (`psx-check`'s `ram-room`).

Three sizes are not margin-sized but rest on a ceiling a largest-case
unit test proves: `bytes_button`, `bytes_row` and the `particles_help_screen`
count. Those tests run on the host only, so nobody has a MIPS ceiling for
them. A MIPS figure taken from a replay's `cap` line would just be the peak
of that replay, not a ceiling, and it would break the PC build if it replaced
the shared size, because the PC peak is larger.

## Solution

Each slot size gets one value per pointer width, selected at compile time.
The PC value stays what it is today, so the PC build and every e2e are
untouched. The MIPS value comes from MIPS figures, and the step states where
each figure comes from:

- **Lists reserved in full when they are created** (`particles`,
  `particles_help_screen`, `ephemerals`): the size is computed from the real
  struct sizes (arena head + list head + capacity × element size). It follows
  pointer width exactly on both builds, so nothing needs measuring. The
  element capacities are pointer-independent PC figures (`elements_*` peaks,
  the same game on both builds).
- **Bare slots** (`particle`, `transit_dots`): the size is the struct size
  rounded to a whole cell. A compile-time guard in each build pins it, in
  both directions.
- **Proven ceilings** (`button`, `row`): a new PS-EXE, `psx-largest-slots`,
  runs on MIPS the same largest-case scenarios the host tests run. The
  scenarios move into one shared module that the host tests and the PS-EXE
  both call. Its printed peaks are the MIPS ceilings, and the slots are sized
  on them with no margin. A new `make psx-check` condition, `largest-slots`,
  keeps them honest.
- **The `particles_help_screen` count ceiling** (1 slot): it is a count, so
  it is pointer-independent. The figure comes from the host tests and is
  unchanged.
- **Margin-sized slots** (`modal_backdrop`, `message_box`, `toast`, `burst`):
  the size is the MIPS peak + 1/8, rounded up to a whole cell. The peak is the
  highest MIPS `cap bytes_<slot>` line over a MIPS replay of each of the five
  `worst-case/` games.
- **Slot counts** (`*_COUNT`): pointer-independent and unchanged; `make
  state` keeps judging them on PC.

The step note in `docs/plan-11.md` records which figure came from where, each
slot's PC and MIPS size, and `ram-room` before and after.

## User Stories

1. As the plan pilot, I want every UI and particle slot size to have a MIPS value, so that the PSX build stops reserving RAM sized for 8-byte pointers.
2. As the plan pilot, I want the PC value of every slot size to stay exactly what it is today, so that `make state`, `make test-all` and every `*.sha256` of `tests/e2e/` stay as they are.
3. As a maintainer, I want one compile-time rule that picks a slot size by pointer width, so that "sized per build" is one visible mechanism and not scattered `#if` blocks.
4. As a maintainer, I want that rule to be an integer constant expression, so that the static slot storage arrays can still be dimensioned with it.
5. As a maintainer, I want the build to refuse a pointer width other than 4 or 8 bytes, so that a third target cannot silently pick the wrong column.
6. As a maintainer, I want each MIPS slot size to be a whole number of zone cells, so that the plan's "rounded up to a whole cell" rule is checked by the compiler, not by review.
7. As a maintainer, I want each MIPS slot size to be no larger than its PC size, so that a typo cannot make the PSX build reserve more than the PC.
8. As a maintainer, I want the `particles`, `particles_help_screen` and `ephemerals` slot sizes computed from the real arena head, list head and element sizes, so that they follow pointer width with no measurement and no constant to forget.
9. As a maintainer, I want the arena head size to be a compile-time constant that the allocator shares, so that the computed list slot sizes cannot drift from what the arena really takes.
10. As a maintainer, I want a test proving the shared arena head constant equals what the allocator really reserves, so that moving a field in the arena struct cannot silently undersize a list slot.
11. As a maintainer, I want the computed list slot sizes to evaluate on PC to today's constants (1,384, 5,344 and 152 bytes), so that the PC build is provably unchanged by the rewrite.
12. As a maintainer, I want the `particle` and `transit_dots` bare slots pinned to their struct size rounded to a cell, in both builds, so that a bare slot is never too small and never wastes a cell.
13. As the implementer, I want the bare-slot guard to fail the build with the struct's real size in view, so that the MIPS value is read from the compiler, not guessed.
14. As a maintainer, I want the largest-button and largest-row scenarios in one shared module, so that the host tests and the MIPS run exercise exactly the same cases.
15. As a maintainer, I want the host tests `test_button` and `test_layout_row` to call that module instead of keeping their own copy of the scenario, so that a scenario that grows on one side grows on both.
16. As a maintainer, I want the host tests to print the PC peak and size of `bytes_button` and `bytes_row` after their largest-case scenario, so that the PC ceilings sit in the test output next to the MIPS ones.
17. As a maintainer, I want the shared module to use only game API reachable from the PSX core (no fork, no JSON, no file), so that it builds into a PS-EXE.
18. As the plan pilot, I want a PS-EXE that runs the largest-case scenarios on MIPS and prints each slot's peak and size, so that the `button` and `row` ceilings are proven on the build that ships.
19. As the plan pilot, I want that PS-EXE to end with the replays' exit protocol (code 0, `p8_fatal` code 3, failed assert code 4), so that `psx_boot.sh --until-exit` and the shared conditions judge it like the other boots.
20. As the plan pilot, I want a `largest-slots` condition in `make psx-check`, so that a MIPS change that grows the largest button or row past its slot fails the PSX check target.
21. As a maintainer, I want `largest-slots` to fail on a missing line, a non-zero exit, a `stopped p8_fatal` or a peak above its size, naming the slot, so that a failure says what grew.
22. As a maintainer, I want the condition's parsing covered by a `psx-tools-test` test on fixture TTY files, so that it is proven without the emulator.
23. As a maintainer, I want the `largest-slots` peaks and sizes added to `measurements.txt` behind `largest `, so that the MIPS ceilings are kept with the other MIPS figures.
24. As the plan pilot, I want the MIPS peaks of the margin-sized slots taken over all five `worst-case/` games, not only `full_game.json`, so that a slot whose PC peak sits in another game is not undersized on MIPS.
25. As the plan pilot, I want those four extra MIPS replays run in the background through the existing replay check, with ceiling and timeout overridden for their length, so that no new target is needed and no agent command limit kills a run.
26. As the plan pilot, I want a MIPS replay counted as evidence only when it ended on its own, so that the plan's evidence rule holds.
27. As the plan pilot, I want the MIPS `cap` count lines (live and peak per slot kind) of each worst-case game to equal the PC's, so that I know no slot spilled or behaved differently on MIPS.
28. As the plan pilot, I want a slot whose PC peak sits in an e2e that cannot replay on MIPS (a mouse or `resume` recording) to keep its PC size on MIPS, so that an unmeasurable peak is never undersized.
29. As the plan pilot, I want each margin-sized slot's MIPS size to be its MIPS peak + 1/8 rounded up to a whole cell, so that MIPS follows the plan's sizing rule.
30. As the plan pilot, I want `button` and `row` sized on their MIPS ceiling with no margin, rounded up to a whole cell only, so that the plan's proven-ceiling rule holds on MIPS too.
31. As the plan pilot, I want step 2's `zone-sizes` and `cap-sizes` conditions to judge the MIPS figures against the MIPS limits without being edited, so that per-build sizes reach the full-game check through the existing state lines.
32. As the plan pilot, I want `make state` still green on PC with the 1/8 margin and the unchanged exempt list, so that the PC build stays green on its own larger peaks.
33. As the plan pilot, I want `ram-room` and `game-ram-room` printed before and after the change, so that step 4 starts from the RAM this step freed.
34. As the plan pilot reading the step note, I want a table naming, for each slot, its PC size, MIPS size and the source of the MIPS figure (computed, compiler, host test, `psx-largest-slots`, MIPS replay `cap` line), so that the step is auditable.
35. As the plan pilot reading the step note, I want the note to say that sizes are per build, picked by pointer width, so that the plan's question is settled on record.
36. As a maintainer, I want the zone totals (`UI_ZONE_BYTES`, `PARTICLES_ZONE_BYTES`, `ZONE_BUDGET_BYTES`) left as PC upper bounds, so that their compile-time checks keep holding on both builds without a second budget to maintain.
37. As a maintainer, I want `make test` and `make e2e` no slower, so that the fast loop stays fast; the scenarios move between existing test binaries and no new unit binary is added to the suite.
38. As a human reviewer, I want `docs/psx-game.md` to describe the new PS-EXE, the new condition and the per-build slot sizes, so that the docs say what is true.
39. As step 4's planner, I want the known reserves (`row` 1,176 → 1,120, `button` 14 → 12) left alone by this step, so that step 4 decides them on the measurements.

## Implementation Decisions

- **Per build, picked by pointer width (the plan's open choice).** The zone
  layer gains one macro that takes a PC value and a 32-bit value and yields
  the one matching `sizeof(void *)`, as an integer constant expression (so it
  can size static arrays). A compile-time guard refuses any pointer width
  other than 4 or 8. Rejected: `#if` on a target macro (two mechanisms, and it
  tests the target, not the property that matters), and expressing
  `button`/`row` as a linear function of pointer width (rounding to cells
  makes slot bytes non-linear in pointer size, so a formula would be a guess).
- **Where the rule applies.** Every slot size of the UI and particle zones is
  written either through that macro (PC value, MIPS value) or as a
  computation from real `sizeof`s, which follows width by construction. Each
  macro-written size gets compile-time checks, in the zone module that owns
  it: the MIPS value is a multiple of the zone cell, and the MIPS value is ≤
  the PC value.
- **Arena head is shared.** The arena struct the allocator puts at the front
  of every arena slot becomes visible to callers through a compile-time
  constant for its cell-rounded size. Its definition moves to the allocator's
  header for that, with no behaviour change. Computed list slot size =
  arena head + cell-rounded list head (`pointer_list_t` or
  `entity_id_list_t`) + cell-rounded `capacity × element size`. On PC this
  must reproduce today's 1,384 (`particles`), 5,344 (`particles_help_screen`)
  and 152 (`ephemerals`). A compile-time check on PC pins those three values
  for this step's commit and is removed in the same chunk once
  `make state` confirms them; the lasting proof is the `bytes_*` lines.
- **Bare slots.** `PARTICLE_SLOT` and `TRANSIT_DOTS_SLOT` become per-build
  pairs. The existing "struct fits its slot" checks are tightened in both
  directions: the slot equals the struct size rounded up to a whole cell. If
  today's PC value is looser than that, the PC value is tightened (bare slots
  have no `bytes_` line, and `make state` judges only the counts). `particle_t`
  holds no pointer, so its two values are expected to be equal.
- **Largest-case scenarios module (devtools).** A new module holds the two
  scenarios the proven ceilings rest on, moved from the host tests without
  changing what they build:
  - the largest buttons: every tech's tech-tree screen row, the "invented" row
    once every tech is invented, and every research project's row as the
    projects panel builds it;
  - the largest row: a planet producing luxury, with the most wants a project
    has and an amount of 5.

  Interface: one entry point that runs every scenario in order, in an already
  loaded cart, plus a function that writes one line per proven-ceiling slot,
  `bytes_<slot> <peak> <size>`, through a caller-given line sink (the same
  style as the state formatter's sink). Peaks are read from the zone modules'
  `*_max_bytes`. The scenarios keep their own invariant asserts (for example,
  "the most wants a project has is 2"). The module uses only game and engine
  API in the PSX core: no fork, no JSON, no file.
- **Host tests rewired.** `test_button`'s largest-buttons case and
  `test_layout_row`'s largest-row case call the module: each runs the entry
  point in its fresh child and prints the module's lines. The PC figures in the
  step note come from there. They also assert peak ≤ size, so a full slot still
  stops them as before. No new unit binary is added.
- **`psx-largest-slots` PS-EXE.** A new command, built by `make psx-build`
  next to the others and with the same core sources, state formatter and
  platform files as the software replay. It has no embedded replay, VRAM
  digest or GPU backend. It loads the sprite sheet and the cart like the
  replay, starts with the bindings default, and runs the module's entry point.
  It prints `psx-largest: asserts on|off`, one `psx-largest: bytes_<slot>
  <peak> <size>` line per slot, then `psx-largest: done`, and exits with 0.
  A `p8_fatal` prints `psx-state: stopped p8_fatal frame <n>` and the state,
  then exits with 3; a failed assert exits with 4. Its vsync ceiling and
  timeout follow the docs' rule: the first measured run plus half again.
- **`largest-slots` condition.** It is added to `make psx-check`: one more boot,
  a few seconds. Its function lives in the shared PSX conditions script.
  It passes when the boot exits with 0 and has `asserts on`, a `done` line, no
  `stopped p8_fatal`, a line for each of `bytes_button` and `bytes_row`, and
  each peak ≤ its size. Otherwise it fails naming the first offending slot or
  the missing line. Its lines go into `measurements.txt` behind `largest `.
  `psx-check`'s condition count and time in the docs are updated.
- **MIPS figures for margin-sized slots.** These are `modal_backdrop`,
  `message_box`, `toast` and `burst`. The MIPS peak of each is the highest
  `cap bytes_<slot>` over MIPS replays of the five `FULL_GAME_EVENTS` games:
  - `full_game.json` comes from `psx-full-game-check`'s `measurements.txt`;
  - the other four (`full_game_new_game`, `full_game_pause_restart`,
    `full_game_pause_idle`, `title_idle`) come from `make psx-replay-check
    PSX_EVENTS=<game>`. Each run overrides the vsync ceiling to frames × 12
    and the timeout to frames × 0.2 s (`full_game` measured 7.9 vsyncs and
    0.13 s a frame), runs in the background, and is waited on until it exits.

  The PC `make state` holder of each `bytes_<slot>` is read. If a holder is
  outside these five, it is added to the MIPS runs when it is a pad
  recording starting on the title. Otherwise that slot keeps its PC size on
  MIPS. MIPS size = peak + 1/8, rounded up to a whole cell (8 bytes).
- **Spill check.** For each worst-case game run on MIPS, the `cap <kind>` live
  and peak counts must equal the PC state's for that game. A difference means
  a slot spilled or behaved differently, and is treated first as a leak
  (plan rule).
- **Proven-ceiling sizes on MIPS.** `button` and `row` take the
  `psx-largest-slots` peak rounded up to a whole cell, with no margin. The
  `particles_help_screen` count stays 1 (host-proven, pointer-independent).
- **Unchanged.** These stay as they are: every PC slot size (except a loose
  bare slot, see above), every slot count, `PARTICLES_CAPACITY`,
  `PARTICLES_HELP_SCREEN_CAPACITY`, `UI_EPHEMERALS_RESERVE`, the zone totals
  and `ZONE_BUDGET_BYTES` (PC upper bounds, still satisfied on MIPS), and the
  exempt list of the state peaks script.
- **Figure sources (stated, per the plan):**

  | slot | PC figure | MIPS figure |
  |---|---|---|
  | `button`, `row` bytes | host test (`test_button`, `test_layout_row`, via the module) | `psx-largest-slots` (`largest-slots` line) |
  | `particles_help_screen` count | host tests (`test_help_screen`, `test_live_slots`) | same figure (count, pointer-independent) |
  | `particles`, `particles_help_screen`, `ephemerals` bytes | computed from `sizeof` + `elements_*` capacity (PC `make state`) | computed from MIPS `sizeof`, same capacity |
  | `particle`, `transit_dots` bytes | compiler guard | compiler guard (PSX build) |
  | `modal_backdrop`, `message_box`, `toast`, `burst` bytes | PC `make state` | MIPS `cap bytes_*` lines, five worst-case games |
  | all counts | PC `make state` | same (equal MIPS count lines checked) |

- **Done when:**
  - `make test-all` passes and every `*.sha256` in `tests/e2e/` is unchanged;
  - `make state` is green;
  - `make psx-check` is green with `largest-slots`;
  - `make psx-full-game-check` has `replay-done`, `gpu-replay-done` and step
    2's `zone-sizes` and `cap-sizes` green (its `frame-budget` may stay red,
    H-2-4);
  - the step note is written.

## Implementation Chunks

1. **Pointer-width rule and computed sizes.** Add the per-width macro and its
   guard, the shared arena head constant, and the computed `particles`,
   `particles_help_screen` and `ephemerals` sizes. Write every other slot size
   as a per-width pair, with the MIPS value equal to the PC value for now.
   Tighten the bare-slot guards in both directions. Host tests: the macro picks
   the column of the host's width; the arena head constant equals what
   `arena_size_for(0)` reports; the computed sizes equal today's PC constants.
   `make state` output is unchanged. The PSX build compiles; the bare-slot
   guard gives the MIPS value for `particle` and `transit_dots`.
2. **Shared largest-case module.** Move the two scenarios into the devtools
   module and rewire `test_button` and `test_layout_row` to call it and print
   the PC `bytes_button`/`bytes_row` lines. `make test` keeps its time.
3. **`psx-largest-slots` and `largest-slots`.** Add the PS-EXE, its build
   entry, the condition in the shared conditions script, its call from
   `psx-check`, its `measurements.txt` lines, and a `psx-tools-test` test of
   the condition on fixture TTYs (pass, missing line, peak over size, `stopped
   p8_fatal`, non-zero exit). `make psx-check` is green with the MIPS sizes
   still equal to PC.
4. **MIPS measurement and sizes.** Record `ram-room` and `game-ram-room`
   (baseline). Then:
   - read the `button`/`row` MIPS ceilings from `largest-slots`;
   - run the four extra worst-case games on MIPS in the background;
   - read `full_game`'s MIPS `cap bytes_*` lines;
   - check the count lines against the PC;
   - set every MIPS value;
   - rerun `make psx-check` and `make psx-full-game-check` (background), `make
     state` and `make test-all`;
   - record `ram-room` after.

   Update `docs/psx-game.md` and write the step 1.3 note: the figure source
   table, the PC and MIPS size of each slot, the per-build decision, and the
   RAM freed.

## Testing Decisions

- A good test checks external behaviour: sizes as the zone modules report
  them (`*_slot_size`, `*_max_bytes`), the state lines, and the TTY lines and
  exit code of a boot. It never checks how a size is spelled in a header.
- **Zone layer** (`test_zone`): the per-width macro returns the column of the
  host's pointer width (8 on the host). The 32-bit column cannot run on the
  host; it is proven by the PSX build's compile-time guards and by
  `largest-slots`.
- **Allocator** (`test_allocator`): the shared arena head constant equals the
  head size the allocator reserves.
- **UI and particle zones** (`test_ui_zone`, `test_particle_zone`): the
  reported slot sizes of `particles`, `particles_help_screen` and `ephemerals`
  equal arena head + list head + capacity × element size. The existing
  "holds its capacity / past capacity" cases stay and keep passing.
- **Largest-case module**: tested through `test_button` and `test_layout_row`,
  which run it and assert peak ≤ size (prior art: `test_burst`'s "largest
  bursts fit their slot", which prints its figure).
- **Condition script**: a `psx-tools-test` test on fixture TTY files, as
  `test_psx_full_game_check` and `test_psx_game_check` test their conditions.
  It never boots the emulator.
- **End to end**: the e2e replays (unchanged hashes), `make state` (PC
  margins), `make psx-check` (with `largest-slots`), and `make
  psx-full-game-check` (step 2's `zone-sizes`/`cap-sizes` on MIPS).

## Hypotheses on step 2

- **H-2-1**: step 2 adds conditions `zone-sizes` and `cap-sizes` to `make
  psx-full-game-check`. They read the GPU replay's `zone`/`cap` state lines
  and compare each peak with the limit printed in the same line. The limit is
  the build's own size (`*_slot_size`, `*_reserved`), so per-build sizes reach
  them with no edit. If step 2 instead compares against a PC-side or hard-coded
  limit, this step makes the condition read the MIPS line's own limit, and
  records it in the note.
- **H-2-2**: the GPU replay's `zone`/`cap` game-memory lines equal the software
  replay's for the same recording (the draw backends allocate from no game
  zone). This step takes the software replay's lines, which exist today in
  `measurements.txt`. If H-2-2 is false, the higher of the two peaks is used per
  slot, and the note says so.
- **H-2-3**: `cap-sizes` fails only when a peak exceeds its limit (`peak >
  limit`), with no 1/8 margin applied to proven-ceiling or reserved-in-full
  kinds. If step 2 applies the margin to every line, it must share the state
  peaks script's exempt list (`bytes_button bytes_row particles_help_screen
  bytes_particles bytes_particles_help_screen bytes_ephemerals`). This step
  makes it so as an application of the plan's sizing rule, not a threshold
  change, and records it.
- **H-2-4**: `make psx-full-game-check` is expected red after step 2 on
  `frame-budget` (and possibly `timer`) until step 4. This step's done
  condition therefore names only `replay-done`, `gpu-replay-done`,
  `zone-sizes` and `cap-sizes` of that target, not the whole target.
- **H-2-5**: step 2 changes no slot size, slot count, zone total or exempt
  list, and its note reports no UI or particle zone or cap over its size on
  MIPS. If it reports one, that slot is a leak to rule out first (plan rule),
  outside this step: the step stops and reports it.
- **H-2-6**: step 2's `gpu-ram-room` condition reads the GPU ELF's size as
  `ram-room` does. Smaller slots only add room, so this step cannot turn it red.
  The note records its before and after figures.

## Out of Scope

- Slot counts, element capacities, zone totals and `ZONE_BUDGET_BYTES` (step
  4 decides any of them on the measurements, including the known `row` and
  `button` reserves).
- The game, frame, load and GPU zones, the GPU primitive buffers, the CLUT
  and pattern slots.
- Any optimization, any change to step 2's thresholds, and the frame budget.
- A 32-bit host build of the unit tests: the MIPS ceilings come from the
  PS-EXE only.
- Re-deriving the PC sizes (except a loose bare slot); PC ceilings stay as
  they are.

## Further Notes

- Zone cells are 8 bytes on both builds (`zone_cell_t` holds an `unsigned
  long long`), so "whole cell" means a multiple of 8 on MIPS too.
- On MIPS the arena head is expected around 48 bytes (96 on PC) and the list
  heads around 16 (24 on PC). For example, `particles_help_screen` would go
  from 5,344 to roughly 2,680 bytes. These are estimates; the compiler gives
  the real figures.
- The PSX replays already exclude `zone`, `budget` and `cap` lines from
  `state-game-lines`, so differing limits between builds break no existing
  condition.
- Running the four extra worst-case games on MIPS takes about an hour of
  background emulator time. It is a one-time measurement for the note, not a
  new target.
