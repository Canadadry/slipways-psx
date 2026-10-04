---
title: "Links to ids: game state links entities and research projects through checked ids"
description: "Game objects link to entities, research projects, trade routes and singletons through raw pointers that silently dangle once the target is released, and a hand-written forget callback is the only defence."
status: needs-triage
---

## Problem Statement

Plan 13 (`docs/plan-13.md`), phase 1 (Safety), step 3.

Every link from one game object to another is a raw pointer today: a slipway's two ends, the object a planet or a slipway is targeted by, the target of a targeting component, the wormhole's anchor, the UI's hovered and held objects, the singletons (main mouse, UI, economy, score panel, shown message box, chosen difficulty), the research project a planet or a button carries, and the 13 game lists of entity pointers (a planet's slipways and routes, a route's planets and slipways, the wormhole's anchors, the UI's ephemerals and hover layers…).

When the target dies, the exit queue releases its memory at the flush, and the slot allocators hand the same address to the next object. A pointer someone forgot to clear then reads a live, unrelated object, and ASan does not see it. The only defence is `forget`, the callback that `exit_queue_flush` runs per released entity, which clears the seven links somebody remembered to list. A new link added without a matching line in `forget` is a silent use-after-free waiting to happen.

Step 2 gives the engine a handle table with checked `entity_id_t` values. Nothing in the game uses it yet.

## Solution

Every link that step 1 sorted under step 3 (28 pointers and 13 entity lists), plus the two pointers the user moved there (`g_projects[]`, `g_projects_by_name[].project`), stops being a pointer:

- A link to an entity is an `entity_id_t`. Reading it goes through `entity_get`, which returns the entity until the flush that follows its death, and NULL after. That is exactly what `forget` did by hand, so the game does not change, and `forget` goes away together with the callback parameter of `exit_queue_flush`.
- A game list of entities is a list of ids. It never holds a dead id: walking one resolves every id, and a NULL `assert`s.
- A research project is not an entity. It lives in a fixed project table, and a link to it is a project id (an index into that table plus an epoch). The id resolves to NULL once the projects are forgotten or the world is reset.
- A trade route is not an entity either (never constructed, never registered). It becomes a plain value: route lists hold routes by value, and an offer carries its route by value.
- A link that may point at a static sentinel (the targeting's "too far" or "throwaway" targets, the slipway's own mouse point) carries a kind next to its id, and an accessor turns it back into the pointer the cart's logic expects.

Every `*.sha256` in `tests/e2e/` stays identical. The pointer guard goes down by exactly 30 pointers (241 → 211) and 13 entity lists (13 → 0). `make sanitize-net` and `make full-game` pass.

## User Stories

1. As a port developer, I want a slipway's `start` and `end` held as entity ids, so that a slipway never reads a planet released under it.
2. As a port developer, I want a slipway whose end is its own mouse point to say so with a flag, not with a pointer into itself, so that `end` is an id or "my mouse point", never an address.
3. As a port developer, I want `slipway_keys_t.start` to be an id, so that the keys that build a slipway carry no pointer.
4. As a port developer, I want `planet_t.targeted_by` and `slipway_t.targeted_by` to be ids, so that a released held object is never read again through them.
5. As a port developer, I want the `targeted_by` of the two static sentinels (`slipway_too_far_t`, `targeting_track_target_t`) to be ids, so that a sentinel that outlives every world holds no address of a world object.
6. As a port developer, I want `targeting_t.target` to be a kind plus an id, so that a target that is a planet, a slipway or one of the three static sentinels is stored without a pointer.
7. As a port developer, I want one accessor that turns a targeting's target back into an `entity_t *` (entity, sentinel or NULL), so that the targeting logic reads exactly as before.
8. As a port developer, I want `wormhole_t.anchor` to be an id and `wormhole_t.anchors` a list of ids, so that the wormhole never reads a released planet.
9. As a port developer, I want `planet_t.slipways` to be a list of ids, so that a planet's slipway list cannot dangle.
10. As a port developer, I want `trade_route_t.planets` and `.slipways` (and the same fields of `trade_keys_t`) to be lists of ids, so that a route stores no entity address.
11. As a port developer, I want `planet_t.incoming_routes`, `planet_t.outgoing_routes`, `trade_offer_t.accepted_routes` and the shared empty burst list to hold trade routes by value, so that no list holds an address of a route that a zone flip or reset can free.
12. As a port developer, I want `trade_offer_t.route` held by value, so that an offer built on the stack never points into a rewound frame zone.
13. As a port developer, I want `trade_route_t` to lose its entity header, so that nothing pretends a route is an entity just to fit in an entity list.
14. As a port developer, I want `ui_t.hovered`, `.previous_hovered`, `.left_held` and `.right_held` to be ids, so that the UI never reads an object released since it was hovered or picked up.
15. As a port developer, I want `ui_t.ephemerals`, the shared empty ephemerals list and the per-layer hover lists to be lists of ids, so that the UI's lists hold no entity address.
16. As a port developer, I want `down_t.held` to be an id, so that a press result hands over an id and the receiver stores an id.
17. As a port developer, I want the main mouse, the main UI, the main score panel and the shown message box held as ids, so that the singletons resolve to NULL once their entity is released.
18. As a port developer, I want the main mouse placeholder and the main economy placeholder expressed as a state, not as an address, so that "placeholder", "real object" and "none" stay three distinct answers without a stored pointer.
19. As a port developer, I want the main economy held as an id, so that the economy of an earlier world never resolves after the world is reset.
20. As a port developer, I want `score_panel_t.last_box` to be an id, so that the score panel never reads a message box released since it opened.
21. As a port developer, I want the chosen difficulty kept as a copy of the button's fields taken when it is chosen, so that the difficulty is still readable for the whole game after the menu button is released, with no `forget` hook.
22. As a port developer, I want `planet_t.project`, `button_t.project`, `button_keys_t.project` and `available_projects_kept_t.items[]` to be project ids, so that no game object holds the address of a research project.
23. As a port developer, I want research projects stored by value in a fixed project table, so that a project id is an index that can be checked.
24. As a port developer, I want the project registry (`g_projects[]`, `g_projects_by_name[].project`) to hold table indexes, so that the registry itself holds no pointer.
25. As a port developer, I want a project id taken before `reset_world()`, or before the projects are rebuilt, to resolve to NULL, so that a project link behaves like an entity link.
26. As a port developer, I want filling the project table beyond its capacity to stop the run with `p8_fatal`, so that a full table is never silent.
27. As a port developer, I want `button_t.project_planet` and `button_keys_t.project_planet` to be ids, so that a project button never applies a project to a released planet.
28. As a port developer, I want the `forget` callback, `planet_forget_targeted_by` and `economy_score_difficulty_quits` removed, so that clearing stale links is no longer a list someone has to maintain.
29. As a port developer, I want `exit_queue_flush` to take no callback, so that the engine has no game hook at release time.
30. As a port developer, I want walking a game id list to `assert` with a message when it meets an id that resolves to NULL, so that a dead id in a list is caught at the first walk.
31. As a port developer, I want an id removed from a game list where the entity dies (where the cart calls `del()`), so that lists never hold dead ids in the first place.
32. As a port developer, I want link presence tested by resolving the id, never by testing the stored id against "no link", so that a dead link reads as absent exactly as the cleared pointer did.
33. As a port developer, I want two links compared through what they resolve to, so that two dead links compare equal (both NULL) as two cleared pointers did.
34. As a port developer, I want `entity_id_of(e)` to be O(1), so that writing a link every frame (hover, targeting) does not slow `make e2e`.
35. As a port developer, I want an engine id-list type with add, delete, has, clear, concat, filter and a resolving iterator, so that game lists of ids read as simply as the entity lists did.
36. As a port developer, I want `entity_geom_nearest` to accept a list of ids too, so that the wormhole's anchor search works on its id list while the registry's tag lists keep their entity list.
37. As a port developer, I want an engine list that holds fixed-size values, so that the game can hold trade routes by value without declaring a pointer field of its own.
38. As a reviewer, I want `tools/pointer_guard_exceptions.txt` lowered by exactly the 30 removed pointers and every `entity-lists` line removed, so that the ratchet proves the step did what it says.
39. As a reviewer, I want each chunk to lower the guard by exactly the pointers and lists it converts, so that every commit is green and the ratchet never jumps.
40. As a reviewer, I want no new pointer field in `src/game/` or `entity_class.h`, so that the guard can only go down.
41. As a reviewer, I want no small value newly passed by pointer, no callee that keeps an address it received, and no dead id in a list, so that the step's verifier finds nothing in the diff against section 2.
42. As a reviewer, I want every `*.sha256` in `tests/e2e/` unchanged after every chunk, so that each chunk is proven not to change the game.
43. As a reviewer, I want `make sanitize-net` and `make full-game` to pass at the end of the step, so that the new tables, the larger header and the route values fit their zones and leave no use-after-free.
44. As a port developer, I want `make test` and `make e2e` no slower, so that the fast loop stays fast.
45. As a port developer, I want GAP-26 rewritten to describe the id that resolves to NULL instead of `forget`, so that `docs/porting-gaps.md` says what is true.
46. As a future step-4 implementer, I want the header's new `id` field to be a value, so that step 4's "header as values" starts from a header whose only additions are values.
47. As a future step-5 implementer, I want the lists held by pointer to stay held by pointer (now pointers to id or route lists), so that step 5 converts them to values as planned and its counts stay right.
48. As a port developer, I want a unit test per converted family proving that a link resolves to its target before the flush and to NULL after, so that the `forget` behaviour is pinned without `forget`.

## Hypotheses on step 2

Step 2 (handle table, `docs/prd/triage/phase1-step2-handle-table.md`) is being implemented in parallel. This PRD relies on its outcome as described below. Each hypothesis is checked against step 2's note in plan section 7 before implementing; the fallback says what changes if it fails.

- **H-3-1 — id type.** `entity_id_t` is a 4-byte value `{ uint16_t index; uint16_t generation; }`, declared in a small engine header that a game header includes without pulling in the registry. A zero-initialised id means "no link", and step 2 provides a named "no link" value and an equality helper. *Fallback:* other names are used as is; if the header is heavy, step 3 splits the type into its own header first.
- **H-3-2 — `entity_get`.** `entity_get(id)` returns `entity_t *` or NULL (no link, out of range, free slot, generation mismatch), in O(1). *Fallback:* none needed for semantics; if it is slower than O(1), stop and report, since every link read goes through it.
- **H-3-3 — `entity_id_of`.** `entity_id_of(e)` returns the entity's id, or "no link" when it holds no slot, and step 2 implements it as a scan of the table, with no id field in `entity_t`. *Fallback:* if step 2 already put the id in the header, chunk 1 drops its header change and keeps only the O(1) proof.
- **H-3-4 — construct takes a slot.** `entity_construct()` takes a slot for every entity it builds, and in `src/game/` an entity is registered if and only if it was constructed (no game code calls `entity_register` directly). *Fallback:* an entity registered without construct gets a slot at its construction site in the chunk that first links to it.
- **H-3-5 — death and flush.** The slot is freed and its generation incremented at `exit_queue_flush`, for every class, before `release`. Between `entity_unregister` and the flush, `entity_get` still returns the dead entity. *Fallback:* if the slot were freed at unregister, links read during the death frame would change the game; stop and report before converting any link.
- **H-3-6 — `forget` kept.** `exit_queue_flush` still takes the `forget` callback and runs it before freeing the slot; step 2 does not touch `cart.c`. *Fallback:* if step 2 already removed it, chunk 5b only checks that nothing clears links by hand.
- **H-3-7 — reset.** `reset_world()` (through `entity_registry_init`) clears the table with a world epoch, so every id taken before the reset resolves to NULL, even when its index is reused, and an entity abandoned by the reset and flushed later frees no slot of the new world. *Fallback:* if old ids could alias new entities, stop and report: singletons and project buttons rely on it.
- **H-3-8 — capacity and budget.** 512 slots, `p8_fatal` when full, the game zone budget raised by the table's size, a `cap` line in `make state`, and a measured peak close to the 329 live entities. *Fallback:* if the peak plus the one extra slot of D2 breaks `make state`'s margin, report it; the capacity is never relaxed silently.
- **H-3-9 — slot without registering.** The engine routine that takes a slot (called by `entity_construct`) can be made public without changing its semantics, and a slot taken by an object that is never registered is freed only by the next `reset_world()` clear. *Fallback:* if the routine cannot be exposed, the economy decision (D2) falls back to the economy being a value owned by its module, constructed in place.
- **H-3-10 — slot-less entities.** An entity registered without `entity_construct` (unit tests only) has "no link" as its id and its flush frees nothing. *Fallback:* tests that link such entities are moved to `entity_construct`.
- **H-3-11 — counts.** Step 2 leaves the guard at 241 pointers in 39 files and 13 entity lists in 7 files, and its new engine files are outside the guard. *Fallback:* the exception file targets of this PRD are recomputed as "step-2 counts minus exactly the 30 + 13 removed here".
- **H-3-12 — exit queue.** `entity_unregister` queues every class, `EXIT_QUEUE_CAP` stays 315 (or a measured raise), and `entity_registry_abandon_live_entities` still queues only classes with a `release`. *Fallback:* none needed by step 3; recorded because removing `forget` relies on every death passing through the flush.

## Implementation Decisions

### Engine additions

- **Header carries its id (guideline DECISION D3).** `entity_t` gets an `entity_id_t id` value field. The slot-taking routine writes it, `entity_init_header` leaves it zeroed ("no link"), and `entity_id_of` returns it in O(1). It is a value, so the guard does not count it.
  - DECISION: add the id to the header in chunk 1. WHY: `make e2e` must not get slower (CLAUDE.md), and the hover loop and the targeting write a link per entity per frame, so a 512-slot scan per write is not affordable; step 2 left this choice to step 3.
- **Engine id list.** A new engine module holds `entity_id_list_t`, a zone-backed list of `entity_id_t` values with the same shape and operations as `entity_list_t` (init, init with reserve, free, clear, add, delete, has, concat, filter, iterate). Its iterator resolves each id and hands out the `entity_t *`. An id that resolves to NULL `assert`s with a message: "a game id list holds no dead id: remove it where the entity dies". `entity_list_t` stays for the registry's internal lists.
- **Engine value list.** A new engine module holds a zone-backed list of fixed-size values (element size given at init), with add, get-at, count, clear and copy. The game wraps it with typed accessors for trade routes. A pointer it hands out is call-scoped: nobody stores it.
- **`entity_geom_nearest`.** Gains an id-list twin; the registry-fed call keeps the entity list version.
- **`exit_queue_flush`.** Loses its callback parameter in the last chunk.
- **Public slot take (D2, H-3-9).** The engine exposes the routine that takes a slot for an object with a header, without registering it.

### Link representation

- **Entity links.** Each field listed in plan section 7 "Step 3: links" becomes `entity_id_t`. Writes store `entity_id_of(e)`. Reads go through a typed accessor where the type is fixed (e.g. "the slipway's start as a planet"), which resolves and checks the class with an `assert`, or through `entity_get` directly.
- **Presence and comparison (guideline DECISION D8).** DECISION: a stored link is tested for presence by resolving it, and two links are compared through what they resolve to; nothing tests a stored link against "no link" to decide behaviour. WHY: `forget` made a dead link read NULL; only resolution reproduces that, which keeps every sha identical (plan section 2, "the game doesn't change").
- **Targets and sentinels (guideline DECISION D4).** DECISION: `targeting_t.target` becomes a small value: a kind (none, entity, throwaway track target, slipway too far, wormhole too far) plus an `entity_id_t`; `slipway_t.end` becomes an id plus an "end is my mouse point" bit in a presence/state field. One accessor per field returns the `entity_t *` the cart's logic reads (resolved entity, address of the static sentinel or of the slipway's own point, or NULL). WHY: the sentinels are static and never constructed, so they have no slot; a kind is the smallest change that stores no address, and the address returned by an accessor is call-scoped, allowed by section 2.
- **Singleton placeholders (guideline DECISION D7).** DECISION: the main mouse and main economy keep their static placeholders; which one is current is a state (none, placeholder, real) next to an id, and the getters return the placeholder's address, the resolved object or NULL as today. WHY: the placeholders are static and slot-less; a state bit keeps the three answers distinct with no stored pointer.
- **Main economy (guideline DECISION D2).** DECISION: `economy_new` takes a slot for the economy through the public slot take (H-3-9) without registering it, and `g_main_economy` stores its id; the slot is freed by the next `reset_world()` clear. WHY: the economy is a game-zone object that is never constructed, and registering it would put it in the live list and change the update order; a slot is the smallest change that makes the link checked. Fallback if H-3-9 fails: the economy becomes a value owned by its module, constructed in place.
- **Difficulty (guideline DECISION D6).** DECISION: `economy_score_set_difficulty` copies the button's name, trade and type bias into a file-scope value with a presence flag, and `economy_score_difficulty` returns that copy or NULL; `economy_score_difficulty_quits` goes. WHY: the cart reads the difficulty for the whole game after the menu button dies; those fields are written only at the button's construction, so a copy taken at the choice is the copy `forget` took at the death, and a `button_t` value adds no counted pointer.
- **Down result.** `down_t.held` becomes an id; the receiver (`ui_handle_button`) stores it into `left_held` / `right_held` as is.

### Research projects

- **Project table (guideline DECISION D5).** DECISION: research projects are stored by value in a fixed table inside the research module (capacity `PROJECT_RECORDS_COUNT`, `p8_fatal` when full, constructed in place so the project's internal self-references stay valid). `research_project_new` takes the next row. A project id is `{ uint16_t index; uint16_t epoch; }`, with zero meaning "no project". The epoch advances when the projects are forgotten (`forget_projects`, i.e. every rebuild and shutdown) and at every `reset_world()` (`bootstrap.c` already calls game modules there). `g_projects[]` becomes an array of table indexes in build order, and `g_projects_by_name[].project` a table index; the public lookups mint an id with the current epoch and keep returning a resolved `research_project_t *` for callers that only read. WHY: the plan says project ids, checked, resolving to NULL after `reset_world()`; the code shows the projects actually die with the load zone at `game_zone_reset`, which precedes the rebuild, so advancing the epoch at both points satisfies the plan as written and the real lifetime. Storing indexes, not ids, in the registry keeps it valid across an epoch change.
- The projects' internal lists (`inputs`, needs) stay in the load zone; the load zone's budget is not resized (its peak only drops).
- Tests that create more projects than the table holds without rebuilding are updated to reset the research module between cases.

### Trade routes

- **Routes are values (guideline DECISION D1).** DECISION: `trade_route_t` loses its entity header and `trade_class()`; route lists (`incoming_routes`, `outgoing_routes`, `accepted_routes`, `g_burst_outgoing_routes`) hold routes by value through the engine value list, and `trade_offer_t.route` is a `trade_route_t` value. A route's `planets` and `slipways` become id lists (still held by pointer: step 5). WHY: routes are never constructed or registered, are created by the hundred in the frame zone, and would exhaust the 512 slots; they are small, never mutated after creation (`trade_extend` builds a new one), and compared by value already (`trade_equal`), so a value is the smallest change that leaves no route address behind. The plan's list naming these "become ids" assumed they were entities; step 1 showed they are not.
- Where `economy_tiers` adds the same route to an outgoing and an incoming list, the incoming list gets a copy; the copy shares the route's id lists, which are never mutated after creation.
- `planet_rebuild_trades` keeps returning the accepted routes list; its element type changes.

### Removing `forget`

- Each chunk deletes the `forget` lines its conversion made redundant. The last chunk deletes `forget`, `planet_forget_targeted_by`, `economy_score_difficulty_quits` and the callback parameter of `exit_queue_flush`. `cart_flush_exit_queue` keeps calling the flush and the game-zone release.
- GAP-26 stays a divergence (the port draws no halo for a released held object) and is rewritten: the planet's `targeted_by` id resolves to NULL after the flush, instead of `forget` clearing it. Its "Files" line follows.
- No list removal site is expected to be new: the cart's only `del()` on entity lists is the registry's (`he`), the ephemerals are replaced wholesale, and the hover layers are rebuilt every hover update. If the dead-id `assert` fires in a replay or in `make full-game`, the cause is investigated with `help-me-debug`; the fix removes the id where the cart forgets the entity, with a GAP if that differs from the cart. The `assert` is never relaxed.

### Guard

- `tools/pointer_guard.sh` is unchanged. `tools/pointer_guard_exceptions.txt` is lowered in each chunk by exactly what the chunk converts. Final values: `entity_class.h` 17, `economy_score.c` 1, `message_box.c` 1, `planet.h` 15, `planet_trade.h` 1, `research.c` 2, `score_panel.h` 5, `slipway.h` 8, `targeting.h` 1, `ui.h` 4, `ui_button.h` 17, `wormhole.h` 7; the lines of `probe.c`, `research_display.h`, `score_panel.c` and `ui.c` and every `entity-lists` line disappear. Total 211 pointers, 0 entity lists.
- The lists that step 5 sorts as "held by pointer" (`planet_t.slipways`, `.incoming_routes`, `.outgoing_routes`, `trade_route_t.planets`, `.slipways`, `trade_keys_t.*`, `trade_offer_t.accepted_routes`, `ui_t.ephemerals`, `wormhole_t.anchors`) stay pointers, now to id or route lists, so their pointer counts stay for step 5.
- No new struct field or file-scope variable in `src/game/` or `entity_class.h` is a pointer. The typed route accessors return call-scoped pointers only.

### Comments

- Per CLAUDE.md § Comments: the dead-id rule and the "resolve before testing presence" rule are `assert`s with messages; the "copy taken at the choice" rule of the difficulty gets one short comment (a trap someone would reintroduce); the comment above `forget` goes with it.

## Implementation Chunks

Each chunk is green and committable on its own: `make test-all` passes, every sha is unchanged, and the exception file goes down by exactly the chunk's conversions.

1. **Engine groundwork.** The header's `id` field and O(1) `entity_id_of`; the engine id list with its resolving, asserting iterator; the engine value list; the id-list twin of `entity_geom_nearest`; the public slot take. Unit tests only, no game change, no count change.
2. **Research project ids.** Project table by value, project id with epoch (rebuild and `reset_world()`), registry as indexes; `planet_t.project`, `button_t.project`, `button_keys_t.project`, `available_projects_kept_t.items[]`. Guard −6 (`research.c` −2, `planet.h` −1, `ui_button.h` −2, `research_display.h` −1).
3. **Trade routes as values.** Engine value list for route lists, `trade_route_t` without header, `trade_offer_t.route` by value; `trade_route_t` / `trade_keys_t` planets and slipways become id lists; incoming, outgoing, accepted and burst route lists hold values. Guard −1 (`planet_trade.h`), entity lists −8 (`planet.h` −2, `planet_family.c` −1, `planet_trade.h` −1, `trade.h` −4).
4. **Map links.** `slipway_t.start/end/targeted_by`, `slipway_too_far_t.targeted_by`, `slipway_keys_t.start`, `targeting_t.target` (kind + id), `targeting_track_target_t.targeted_by`, `planet_t.targeted_by`, `wormhole_t.anchor`, `wormhole_t.anchors`, `planet_t.slipways`, `button_t.project_planet`, `button_keys_t.project_planet`. Deletes the `left_held → end` and `planet_forget_targeted_by` lines of `forget`; GAP-26 rewritten. Guard −11 (`slipway.h` −5, `targeting.h` −2, `planet.h` −1, `wormhole.h` −1, `ui_button.h` −2), entity lists −2 (`planet.h`, `wormhole.h`).
5. **UI links.** (a) `ui_t.hovered/previous_hovered/left_held/right_held`, `down_t.held`, `ui_t.ephemerals`, `g_shared_ephemerals`, the hover layers; deletes the hovered lines of `forget`. Guard −5 (`entity_class.h` −1, `ui.h` −4), entity lists −3 (`ui.h` −2, `ui.c` −1). (b) Singletons and `forget`: `g_main_mouse`, `g_main_ui`, `g_main_score_panel`, `score_panel_t.last_box`, `g_shown_message_box`, `g_main_economy` (slot, D2), `g_difficulty` (copy, D6); deletes `forget`, `planet_forget_targeted_by`, `economy_score_difficulty_quits` and the flush callback. Guard −7 (`ui.c` −2, `score_panel.c` −1, `score_panel.h` −1, `message_box.c` −1, `probe.c` −1, `economy_score.c` −1). After 5b: `make sanitize-net` and `make full-game`.

## Testing Decisions

- A good test drives the public interface (constructors, the press and drop methods, `entity_unregister`, `cart_flush_exit_queue`, `reset_world`, the typed accessors) and checks what a caller observes: which object a link resolves to, and when it becomes NULL. It never inspects a stored id's fields or the table.
- New `tests/test_entity_id_list.c` for the engine id list (add, delete, has, concat, filter, resolving iteration, the dead-id `assert` in a child process as `tests/test_exit_queue.c`'s `expect_fatal` does) and the value list; it links only the engine objects it needs. The header id is covered there or in step 2's `tests/test_entity_id.c` (construct → `entity_id_of(e)` equals the id `entity_get` accepts).
- Per family, in the existing module test file (`test_slipway.c`, `test_wormhole.c`, `test_ui.c`, `test_trade.c`, `test_research_project.c`, `test_button.c`, `test_message_box.c`, `test_exit_queue.c`): a link resolves to its target before the flush and to NULL after it; for the singletons and project ids, after `reset_world()` too. These cases replace the `forget` cases (the cases that called `forget` through the flush keep their observable assertion).
- Project table: capacity `p8_fatal`, epoch at rebuild and at `reset_world()`, lookups by rank and by name across an epoch change.
- Difficulty: the copy survives the button's release and flush. Tests that asserted identity (`economy_score_difficulty() == button`) assert the copied fields instead: per CLAUDE.md, a unit test that froze the old behaviour is updated.
- Trade: route equality and extension by value, an accepted route list that copies a route, an incoming list that shares a route's id lists.
- Prior art: `tests/test_entity_list.c` (engine container), `tests/test_exit_queue.c` (fatal paths, flush), `tests/test_camera.c` (`reset_world` cases), `tests/test_pointer_guard.c` (the guard itself, unchanged).
- Outside unit tests: every sha unchanged (`make e2e`) after every chunk; `make sanitize-net` and `make full-game` after the step (`plan-pilot` runs them); `make test` and `make e2e` wall time compared before and after (alternating pairs, as in Step 1.1).

## Out of Scope

- Lists held by pointer becoming values, `_own` fields, `wormhole_t.too_far_target` as a flag: step 5.
- The header pointers (`pos`, `targeting`, `emitter`…) and moving `targeting`/`emitter` out of `entity_t`: step 4.
- Mixed static/runtime rows: step 5b. Call-scoped bundles and small values by pointer: step 6.
- `slipway_served_planets_t.ids[]` (step 5) and the allowed pointers of section 2.
- Changing `entity_registry_abandon_live_entities`, the exit-queue capacity, or any zone size beyond what step 2 set.
- Moving `entity_t` / `entity_class.h` into `game/`. Audio, the raylib binary, PSX.

## Further Notes

- Count check: 28 step-3 pointers from the sorting plus `g_projects[]` and `g_projects_by_name[].project` (user decision) = 30; 241 − 30 = 211. All 13 entity lists go.
- The step-1 note says research projects die at `reset_world`; the code shows they die at `game_zone_reset` (load zone, at `cart_load` and shutdown) and survive a `reset_world`. D5 covers both; the step note records the difference.
- Risk: a planet replaced by a project (`planet_apply_project` with a class name) dies while slipways, routes or anchors may still name it. If the dead-id `assert` or a NULL end fires there, it is the case the cart reads after the port frees it: handle it per plan section 2 (free or forget where the cart forgets, GAP if divergent), never by relaxing the `assert`.
- Risk: the header grows by 4 bytes; zones sized by `sizeof` follow, fixed arenas may tighten. `make full-game` proves it; a full zone is fixed by a measured budget raise recorded in the step note.
- The step graduates with one line in plan section 7: final counts (211 / 0), the slot-table peak with the economy slot, the project table capacity used, GAPs touched (26), and which hypotheses on step 2 held.
