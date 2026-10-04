---
title: "Handle table: checked entity ids in engine/"
description: "Entities have no checked id, so a link to an entity that died can only be a raw pointer that silently reads freed or reused memory."
status: needs-triage
---

## Problem Statement

Plan 13 (`docs/plan-13.md`), phase 1 (Safety), step 2.

Every link from one game object to an entity is an `entity_t *` today. When the entity dies, its exit-queue entry releases its memory at `exit_queue_flush`. From then on, any pointer still aimed at it dangles. Worse, the slot allocators hand the same address to the next entity, so a stale pointer reads a live, unrelated object, and ASan does not see that. The only defence is the hand-written `forget` callback in the cart, which clears the stale pointers someone remembered to list.

To move those links to ids in step 3, the engine first needs an id that can be checked. Given an id, it must answer "is this still the entity it was?" and return NULL when it is not. That id and its table don't exist yet.

## Solution

`engine/` gets a handle table and the id type that indexes it:

- `entity_id_t` is a value made of `uint16_t index` and `uint16_t generation`. `{0,0}` means "no link".
- The table has a fixed 512 slots. Each slot holds the entity it designates and a generation. Taking a slot when all 512 are in use stops the run with `p8_fatal`.
- `entity_construct()` takes a slot for every entity it builds.
- A slot is freed, and its generation incremented, when the entity's exit-queue entry is flushed (`exit_queue_flush`). This holds for **every** class, not only the classes that have a `release`. `entity_unregister` therefore queues every entity it removes from the live list, and the flush calls `release` only when the class has one.
- Between the death (`entity_unregister`) and the flush, `entity_get(id)` still returns the dead entity, as the cart still reads it during that frame. After the flush, `entity_get(id)` returns NULL.
- `reset_world()` clears the table: no id from the previous world resolves afterwards.
- The game does not use the table yet. It is proven by unit tests, and every `*.sha256` in `tests/e2e/` stays identical.

## User Stories

1. As a port developer, I want an `entity_id_t` value type in `engine/`, so that step 3 can replace entity pointers with ids.
2. As a port developer, I want `entity_id_t` to be two `uint16_t` fields (index, generation), so that it is 4 bytes, copied by value and compared field by field.
3. As a port developer, I want `{0,0}` to mean "no link", so that a zero-initialised struct holds no link without any setup.
4. As a port developer, I want a named constant or helper for the "no link" id and a test for it, so that call sites don't spell `{0,0}` by hand.
5. As a port developer, I want `entity_get(id)` on the "no link" id to return NULL, so that an absent link and a dead link read the same way.
6. As a port developer, I want no live slot ever to carry generation 0, so that `{0,0}` can never designate a real entity, slot 0 included.
7. As a port developer, I want `entity_construct()` to take a slot for the entity it builds, so that every constructed entity has an id with no extra call.
8. As a port developer, I want `entity_id_of(e)` to return the id of a constructed, not yet flushed entity, so that tests (and step 3) can get an entity's id.
9. As a port developer, I want `entity_id_of(e)` to return the "no link" id for an entity that holds no slot, so that the function has a defined result in every case.
10. As a port developer, I want `entity_get(entity_id_of(e)) == e` right after construction, so that the round trip is the basic contract.
11. As a port developer, I want `entity_get` to keep returning the entity after `entity_unregister` and before the flush, so that the frame in which an entity dies keeps the cart's behaviour (the cart still reads it until the end of the frame).
12. As a port developer, I want `entity_get` to return NULL for that id once the exit queue has been flushed, so that a link to a dead entity fails loudly instead of reading freed memory.
13. As a port developer, I want the flushed slot's generation incremented, so that an id taken before the death never resolves to the next entity that reuses the slot.
14. As a port developer, I want the flush to free the slot of an entity whose class has no `release`, so that every class follows the same id lifecycle.
15. As a port developer, I want `entity_unregister` to queue every entity it removes from the live list, with or without a `release`, so that the flush is the single place where slots are freed.
16. As a port developer, I want the existing GAP-5 behaviour kept (an entity is queued only when it was still in the live list), so that an entity unregistered twice is not queued twice.
17. As a port developer, I want the flush to call `release` only when the class defines one, so that classes without a `release` don't crash.
18. As a port developer, I want the flush to free the slot before calling `release`, so that the table never points at memory that has already been returned to its zone.
19. As a port developer, I want the `forget` callback to keep running exactly as today, so that the game doesn't change during this step (step 3 removes it).
20. As a port developer, I want an arena entry (`exit_queue_put_arena`) to be flushed exactly as today, so that the grab arena's lifecycle is untouched.
21. As a port developer, I want taking a 513th slot to stop the run with `p8_fatal` and a message that names the limit, so that a full table is never silent (modelled on "exit queue full, limit %d").
22. As a port developer, I want a freed slot to be reusable by a later construction, so that 512 slots cover a whole game, not just its first 512 entities.
23. As a port developer, I want `reset_world()` to clear the table, so that a new world starts with every slot free.
24. As a port developer, I want an id taken before `reset_world()` to resolve to NULL afterwards, even when its index is taken again in the new world, so that nothing from the old world aliases a new entity.
25. As a port developer, I want the entities that `reset_world()` abandons (and the exit queue flushes later in the frame) not to free any slot of the new world, so that a new entity built in the same frame keeps its id.
26. As a port developer, I want `cart_shutdown()` and the game-zone teardown (`quit_game_zone`) to leave no dangling table, so that a fresh `cart_load()` starts clean.
27. As a port developer, I want the table's storage to count against the zone budget, so that the 768 KiB budget stays a true upper bound.
28. As a port developer, I want a live count and a peak count of taken slots, so that the step note can report the slot-table peak and `make state` can watch the margin.
29. As a port developer, I want `make state` to print the table's current, peak and limit on a `cap` line, so that the peak over the worst-case games is measured like every other cap.
30. As a port developer, I want the measured peak (329 live entities today) to stay well under 512, so that `make state`'s 1/8 margin rule holds.
31. As a port developer, I want every `*.sha256` in `tests/e2e/` to stay identical, so that the step is proven not to change the game.
32. As a port developer, I want `make sanitize-net` to pass, so that the new flush order (slot freed, then `release`) has no use-after-free.
33. As a port developer, I want `make full-game` to pass, so that the worst-case games neither fill the table, nor the exit queue, nor any zone.
34. As a port developer, I want `make test` and `make e2e` to be no slower, so that the fast loop stays fast.
35. As a port developer, I want the table's operations to be O(1) for construct and `entity_get`, so that no per-frame path gets slower.
36. As a port developer, I want no game code to call the new API in this step, so that the step stays a pure engine addition proven by unit tests.
37. As a reviewer, I want the pointer-guard counts unchanged by this step, so that the plan's ratchet ("steps 2 and 6 leave them as they are") holds.
38. As a reviewer, I want the table's internal entity pointers to stay inside `engine/`, so that they fall under the section-2 exception for the engine registry's internal lists.
39. As a future step-3 implementer, I want `entity_id_t` declared in a small engine header that game headers can include cheaply, so that game structs can hold ids without pulling in the registry.
40. As a future step-3 implementer, I want the flush to be the only point where an id goes dead, so that removing dead ids from game lists "where the cart calls `del()`" has one clear deadline.

## Implementation Decisions

- **New engine module: entity ids.** A new module in `engine/` owns `entity_id_t`, the "no link" id, and the table. Interface (names follow `module_verb[_object]`, with `entity_get` kept as the plan writes it):
  - `entity_id_t` = `{ uint16_t index; uint16_t generation; }`, plus a "no link" value and an equality/"is no link" helper.
  - `entity_get(entity_id_t id)` returns `entity_t *` or NULL (NULL for "no link", for an index out of range, for a free slot, or for a generation mismatch).
  - `entity_id_of(const entity_t *e)` returns the entity's id, or "no link" when it holds no slot. It is a scan, kept for tests, and step 3 decides whether the header carries the id (see the guideline decision below).
  - Internal to the engine: take a slot for an entity (called by `entity_construct`), free the slot of an entity (called by the exit-queue flush), reset the table (called by the registry init), plus `live` and `max` counts for `make state`.
  - Capacity: a named constant of 512.
- **Storage (guideline DECISION).** The 512-slot table is allocated from `game_zone()` in `entity_registry_init()`, next to the registries, which `reset_world()` already calls. The budget constants (`ZONE_BUDGET_*`, `zone_sizes.h`) stay unchanged. Being inside an existing zone is how the table counts against the zone budget. WHY: only 1,980 bytes of budget are left and a 512-slot table is ≥ 5 KiB on the 64-bit host; the registries already live there.
- **entity_t unchanged (guideline DECISION).** Step 2 does not add an id field to `entity_t`: every entity would grow and the fixed UI and particle slot arenas would get tighter. The flush finds an entity's slot by scanning the table for its pointer, and `entity_id_of` scans too. Whether the header carries its id is left to step 3 or step 4. WHY: the plan says nobody uses the table yet, so the smaller change wins.
- **Table layout.** Two parallel arrays of 512, the entity pointers and the `uint16_t` generations, plus a way to find a free slot in O(1) (a free-index stack of `uint16_t`, or an equivalent). Construct and `entity_get` are O(1). Only the flush scans, once per queued entity (at most a few hundred entries, once per world change), which does not measurably slow `make e2e`.
- **Generations.** A slot's generation is never 0 while it holds an entity. Incrementing skips 0 on wraparound (65535 → 1). Allocating a fresh table at each `entity_registry_init()` would restart every generation, so the table seeds all its slots from a file-scope world epoch that `reset_world` advances (also skipping 0). An id from the previous world then fails the generation check even when its index is reused. The epoch is a plain integer, not a pointer.
- **Freeing at flush, safely.** For each entity entry, `exit_queue_flush` runs, in order: the existing `forget(e)`, then frees the slot that holds `e` (only if the table holds exactly this pointer: an entity abandoned by `reset_world()` is not in the new table and frees nothing), then `release(e)` only if the class has one. Freeing before `release` ensures no slot ever points at returned memory. A pointer comparison is sound here: `e` is still allocated at that point, so no new entity can share its address.
- **`entity_unregister`.** It queues every entity it removes from the live list. The GAP-5 condition (queued only if `entity_list_del` found it) stays, and the `release != NULL` condition goes. `exit_queue_put` drops its assert "the class has nothing to release".
- **`reset_world()` / `entity_registry_abandon_live_entities()`.** Abandoning still queues only the entities with a `release`, as today. The cleared table already makes every old id resolve to NULL, so the release-less entities need no entry. This keeps the reset burst at 286 entries, under `EXIT_QUEUE_CAP` (315).
- **Exit-queue capacity.** `EXIT_QUEUE_CAP` stays at 315. A frame's ordinary deaths add only a few release-less entries, far from the reset burst that sets the cap. If `make full-game` reports the queue full, the fix is a measured raise of the cap that stays within `make state`'s margin, never a filter on which entities are queued.
- **Teardown.** `entity_registry_shutdown()` releases the table along with the lists. `entity_registry_forget_registries()` (after `game_zone_reset()`) forgets it like the lists. When there is no table, `entity_get` returns NULL. Taking a slot without a table is a broken contract and `assert`s with a message ("entity_registry_init() must run before entity_construct").
- **Entities never constructed.** Tests that call `entity_register` directly hold no slot. The flush finds nothing to free for them, and `entity_id_of` returns "no link". This is a defined outcome, not an error.
- **Full table.** `p8_fatal` with a message naming the limit, in the same style as the exit queue's "exit queue full, limit %d".
- **`make state`.** The devtools state writer adds a `cap` line for the table (current, peak, limit) next to `cap exit_queue`. The peak survives a reset, like the exit queue's max. The step note reports the peak measured over the worst-case games.
- **Comments.** Each comment follows CLAUDE.md § Comments, and the three traps get an `assert` with a message: the flush order, the generation that is never 0, and freeing only on an exact pointer match.
- **No game change.** No file in `game/` calls the new API. `cart.c`'s `forget` is untouched. Every `*.sha256` stays identical. The `make test` and `make e2e` running times do not grow.

## Implementation Chunks

1. **The table alone.** The id type, "no link", the 512-slot table with its generations, take, free-by-pointer, `entity_get`, `entity_id_of`, reset with an epoch, `live`/`max`, `p8_fatal` when full. Unit-tested in isolation, with no other engine module calling it yet.
2. **Wiring into the lifecycle.** `entity_registry_init`, shutdown and forget own the table. `entity_construct` takes a slot. `entity_unregister` queues every class. `exit_queue_put` loses its release-only assert. `exit_queue_flush` frees the slot between `forget` and an optional `release`. The `cap` line goes into `make state`. Tests cover the lifecycle through `entity_construct` / `entity_unregister` / flush / `reset_world`. `make test-all`, `make sanitize-net` and `make full-game` are green, with every sha unchanged.

## Testing Decisions

- A good test drives the public interface only (`entity_construct`, `entity_unregister`, `exit_queue_flush` / `cart_flush_exit_queue`, `reset_world`, `entity_get`, `entity_id_of`) and checks what a caller observes: which entity an id resolves to, and when it turns NULL. It never inspects the table's arrays.
- New file `tests/test_entity_id.c` (one test file per module), added to `UNIT_TESTS`. It links only the engine objects it needs, so `make test` stays fast. Cases:
  - "no link" resolves to NULL, and a zeroed `entity_id_t` equals "no link";
  - construct → `entity_get(entity_id_of(e)) == e`;
  - an id stays valid between unregister and flush, and turns NULL after the flush;
  - a class without `release` gets its slot freed at the flush (and `release` is not called);
  - a slot freed and taken again gives a different id, and the old id stays NULL;
  - generation wraparound skips 0;
  - 512 constructs succeed, and the 513th stops with `p8_fatal` (child-process or fatal-hook pattern, as in `tests/test_exit_queue.c`'s `expect_fatal`);
  - `reset_world()` → every pre-reset id is NULL, including one whose index is reused by a post-reset entity;
  - an entity abandoned by `reset_world()` and flushed later in the same frame does not free the slot of an entity built after the reset;
  - an entity registered without `entity_construct` has "no link" as its id, and its flush frees nothing;
  - the live count goes up and down, and the peak survives the flush and the reset.
- `tests/test_exit_queue.c` currently has a case that relies on `exit_queue_put` asserting on a class without `release`, if it has one, and its comments describe release-only queuing. Update those cases to the new contract (every unregistered entity is queued). Per CLAUDE.md, the e2e replays are the source of truth, and a unit test that froze the old behaviour is updated. Its reset-burst case (286 entries) must still pass unchanged, because abandonment stays release-only.
- Prior art: `tests/test_exit_queue.c` (fatal on overflow, child processes under ASan, flush behaviour), `tests/test_entity_list.c` (small engine container), `tests/test_camera.c` (`reset_world` cases).
- Outside the unit tests: every `*.sha256` unchanged (`make e2e`), `make sanitize-net`, `make full-game`. `plan-pilot` runs the last two after `prd-autopilot`, and the step note records them.

## Hypotheses on step 1

Step 1 (pointer guard) is being implemented in parallel. This PRD is written against the committed tree, before step 1's commits. Each of the following hypotheses must be checked against plan section 7 and the tree as it is when step 2 starts. A false one is re-decided before coding.

- **H-2-1.** `tools/pointer_guard.sh` and `tools/pointer_guard_exceptions.txt` exist and run in `make test` with strict equality, so any count step 2 changed would turn `make test` red.
- **H-2-2.** The guard's scope is `src/game/` (headers and `.c` files) plus `src/engine/entity_class.h`, as the plan states. The new engine module (its header and `.c` file) is outside that scope, so the table's internal `entity_t *` array is not counted.
- **H-2-3.** Step 2 changes no count. `entity_t` and `entity_class.h` are untouched (guideline decision), `entity_id_t` holds no pointer, and no field or file-scope variable is added in `src/game/`. The only file-scope state added (world epoch, table handle) lives in `engine/` outside `entity_class.h`. If the guard turns out to scan all of `engine/`, the table's pointer is an "engine registry internal list" allowed by section 2 and gets a line in the exceptions file, sorted as allowed.
- **H-2-4.** The second count (game fields and file-scope variables holding a list of entity pointers) is unchanged: the table is not an `entity_list_t` or `pointer_list_t`, and it is not in `game/`.
- **H-2-5.** The section-7 sorting has an empty `Unsorted` heading (otherwise the run would have stopped before step 2), and step 1's note reports no surprise that touches the exit queue, the registry, `entity_construct` or `reset_world`.
- **H-2-6.** Step 1 does not change `exit_queue_flush`, its `forget` callback, `entity_unregister`, `entity_registry_init`, `reset_world` or the zone constants. Its only `Makefile` edits are the new guard target in `make test` and its unit test, so step 2's `UNIT_TESTS` addition merges trivially.
- **H-2-7.** Step 1 leaves `make test` and `make e2e` at today's speed, so step 2's own budget is measured against the same baseline.
- **H-2-8.** Counts are frozen by step 1 at today's values. From step 2 on, "no step raises a count" is checked by the guard and not by this PRD.

## Out of Scope

- Using ids anywhere in `game/`: links, lists, `down_t.held`, and removing `forget` are step 3.
- Adding an id field to `entity_t`, or any change to the header: step 3 or 4.
- Changing `entity_registry_abandon_live_entities` to queue release-less entities.
- Moving `entity_t` / `entity_class.h` out of `engine/`.
- Lowering pointer-guard counts.
- Resizing zones or changing `ZONE_BUDGET_BYTES`.
- Audio, the raylib binary, PSX.

## Further Notes

- Peak: `cart_game_registry_reserve` measures 329 live entities. With 512 slots, `make state`'s margin rule (9 × peak ≤ 8 × limit) holds with room to spare. The title screen and the frame of a world change are also covered, because a reset clears the table before the new world's entities take slots.
- Risk: the game zone's generation now carries about 5 to 6 KiB more per world. `make full-game` fails on a full zone. If it fails, the fix is a measured raise of the generation zone within the 1,980 bytes of budget left, or a smaller table layout. The 512 capacity and the "counts against the budget" rule are never relaxed.
- Step 2 graduates with one line in plan section 7: the slot-table peak measured by `make state`, the exit-queue peak, and confirmation that the guard counts did not move.
