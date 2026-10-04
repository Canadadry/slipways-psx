---
title: "Header as values: the entity header owns its data"
description: "The entity header reaches its layer, position, hit box, flags, targeting and emitter through 12 pointers into the derived struct or the archetype, so every piece of header data has two names and two owners."
status: needs-triage
---

## Problem Statement

Plan 13 (`docs/plan-13.md`), phase 2 (Readability), step 4.

`entity_t` holds 12 data pointers besides `class`: `layer`, `pos`, `map_pos`, `hit_box`, `cursor_sprite`, `infraspace`, `shake`, `fold_anchor`, `is_dead`, `is_hovered`, `emitter`, `targeting`. Each constructor wires them by hand to a field of the derived struct (`header.is_dead = &self->is_dead`) or of the archetype (`header.layer = &self->archetype->layer`). The result is hard to read and easy to get wrong:

- Every piece of header data has two names: `planet->is_dead` and `*planet->header.is_dead`, `camera->pos_before_shake` and `*camera->header.map_pos`. A reader must find the constructor to learn which is which.
- `pos` and `map_pos` are pointers to pointers (`const vec_t *const *`). They point at a pointer field of the derived struct, which in turn points at a backing buffer (`pos_own`, `pos_box`, `track_pos`, `point_pos`, an archetype vector, the galaxy's box, another entity's vector). Reading a position takes two NULL checks and a double dereference.
- A derived position borrowed from another object follows that object's in-place writes (`planet_bob` writes `pos->y` through the pointer). Whether a copy or a share was meant is invisible at the assignment.
- `engine/` names `targeting` and `emitter`, two game components, only to give the game a place to hang them.

The step-1 sorting assigns 33 counted pointers to this step: the 12 header pointers, the 15 derived position fields the header points at (`planet_t.pos`, `camera_t.pos_before_shake`, `slipway_mouse_point_t.map_pos`…) and the 6 `targeting`/`emitter` pointers of `planet_t`, `probe_t`, `slipway_t` and `wormhole_t`.

## Solution

The header owns its data as values:

- The 10 data fields `layer`, `pos`, `map_pos`, `hit_box`, `cursor_sprite`, `infraspace`, `shake`, `fold_anchor`, `is_dead`, `is_hovered` become values in `entity_t`. Whether a class carries each one is a bit in one `uint16_t has` in the header.
- An archetype's values (layer, hit box, cursor sprite, infraspace, fold anchor) are copied into the header at construction.
- The copies in derived structs go away. `planet->pos`, `probe->is_dead`, `button->layer`, `camera->pos_before_shake`, `slipway->infraspace`… are gone: the header is the only owner, and the game reads `self->header.x`.
- `targeting_track` writes the dragged object's position into its header instead of re-pointing it at a buffer in the targeting component.
- `targeting` and `emitter` leave `entity_t`. Each struct that carries one keeps it as a plain value field (`probe_t.targeting`, `planet_t.emitter`), and `planet_t`, where it is optional, records its presence in a `has` bit. The game reaches them through `targeting_of(e)` / `emitter_of(e)`, which switch on the class and whose result nobody keeps. `engine/` no longer names either.

Every `*.sha256` in `tests/e2e/` stays identical. The pointer guard goes down by exactly 33 pointers (211 → 178), and the entity-list count stays 0. `make test` and `make e2e` are no slower. `make sanitize-net` and `make full-game` pass.

## User Stories

1. As a port developer, I want an entity's position stored once, in its header, so that I never wonder whether `planet->pos` or `*planet->header.pos` is the real one.
2. As a port developer, I want `entity_t.pos` and `entity_t.map_pos` to be `vec_t` values, so that reading a position is one field access, not a double dereference with two NULL checks.
3. As a port developer, I want a presence bit for each header value, so that "this class has no position" and "this class has a position" stay distinct without a NULL pointer.
4. As a port developer, I want the header's presence bits in one `uint16_t has`, so that the header follows the plan's "optional value" rule like every other struct.
5. As a port developer, I want an archetype's layer, hit box, cursor sprite, infraspace and fold anchor copied into the header at construction, so that the header never points into static data it does not need to share.
6. As a port developer, I want `button_t.layer`, `slipway_t.layer` and `score_panel_t.layer` gone, so that a layer that changes at runtime is written in one place.
7. As a port developer, I want `slipway_t.infraspace` and `.has_infraspace` gone, the header holding the value and its presence bit, so that "an infraspace of 0 still counts" is expressed by the bit, not by a pointer being non-NULL.
8. As a port developer, I want `planet_t.fold_anchor` gone, so that the wormhole and trade code read the fold anchor from the header like the engine does.
9. As a port developer, I want `is_dead` to be a header value, so that `update_entities` reads one flag and the derived structs lose their copy.
10. As a port developer, I want `is_hovered` to be a header value with a presence bit, so that the UI's "the hovered entity has no is_hovered member" `assert` still holds.
11. As a port developer, I want `shake` to be a header value with a presence bit, so that `entity_geom_shake` decrements it in place and keeps its `assert` that only a planet or the camera shakes.
12. As a port developer, I want `hit_box` to be a header `box_t` value, so that a button's hit box, recomputed from its width, is written into the header rather than allocated in its slot.
13. As a port developer, I want `cursor_sprite` to be a header value with a presence bit, so that the UI falls back to its own sprite exactly when the hovered class has none.
14. As a port developer, I want the camera's position before shake held in the header's `map_pos`, as the header already maps it today, so that the camera loses `pos_before_shake`, `pos_before_shake_own`, `pos` and `pos_own`.
15. As a port developer, I want the mouse to write its position into its header each frame, so that `mouse_t.pos`, `pos_own` and the static zero vector go away.
16. As a port developer, I want a layout row, a toast, a structure icon and a button to hold their position in the header, so that `layout_row_t.pos`, `toast_t.pos` / `pos_box`, `structure_icon_t.pos` and `button_t.pos` / `pos_box` go away.
17. As a port developer, I want a planet, a probe and a wormhole to hold position and map position in the header, so that `planet_t.pos/map_pos`, `probe_t.pos/map_pos` and `wormhole_t.pos/map_pos` go away.
18. As a port developer, I want the slipway's mouse point to hold its position in its own header, so that `slipway_mouse_point_t.pos/map_pos` and `slipway_t.point_pos/point_map_pos` go away.
19. As a port developer, I want `targeting_track` to write the host's position and map position into its header (and clear the position bit when there is no mouse position), so that the host no longer points into its targeting component.
20. As a port developer, I want `targeting_t.track_pos` and `.track_map_pos` removed, so that the targeting component holds only what targeting needs.
21. As a port developer, I want `planet_bob` to write the bob into the header's position, so that the in-place write the cart does on its shared table lands on the only owner.
22. As a port developer, I want a planet built from keys to copy the keys' position, never keep the keys' address, so that a planet's position cannot change behind its back.
23. As a port developer, I want a toast built from a planet's position to follow that planet until the toast's first update, so that the toast reproduces the cart's shared table during the one window where the cart shares it.
24. As a port developer, I want `targeting` and `emitter` out of `entity_t`, so that `engine/` names no game component.
25. As a port developer, I want `targeting_of(e)` to return the targeting component of a probe, a help screen, a planet that carries one, a slipway or a wormhole, and NULL otherwise, so that code that held `e->targeting` reads the same.
26. As a port developer, I want `emitter_of(e)` to return the emitter of a probe, a help screen or a planet that carries one, and NULL otherwise, so that the particle code reads the same.
27. As a port developer, I want `planet_t.targeting_own` / `emitter_own` renamed to plain `targeting` / `emitter` values, with presence bits in a `uint16_t has` of `planet_t`, so that the plan's optional-value rule applies.
28. As a port developer, I want small inline header helpers (presence test, position read with zero fallback, position set and clear), so that call sites stay as short as they were.
29. As a port developer, I want the header helpers to take and return `vec_t` by value, so that the step adds no small value passed by pointer.
30. As a port developer, I want the draw dispatch to pass the address of the header's position (or NULL when absent) only for the call, so that `draw` keeps its signature until step 6 and no callee keeps that address.
31. As a port developer, I want `make state` text unchanged (infragate, positions), so that the dump reads the header the same way it read the pointers.
32. As a reviewer, I want `tools/pointer_guard_exceptions.txt` lowered by exactly the 33 removed pointers, chunk by chunk, so that the ratchet proves what each commit did.
33. As a reviewer, I want no new pointer field or file-scope pointer in `src/game/` or `entity_class.h`, so that the guard only goes down.
34. As a reviewer, I want `targeting_of` and `emitter_of` results never stored in a struct field or a file-scope variable, so that the verifier finds no kept address.
35. As a reviewer, I want every `*.sha256` in `tests/e2e/` unchanged after every chunk, so that each chunk is proven not to change the game.
36. As a reviewer, I want `make sanitize-net` and `make full-game` to pass after the step, so that the smaller header and the moved components leave no use-after-free and no full zone.
37. As a port developer, I want `make test` and `make e2e` no slower, so that the fast loop stays fast.
38. As a port developer, I want `test_entity_header.c` to check presence bits and copied values per class instead of pointer identity, so that the header's contract is pinned in its new form.
39. As a future step-5 implementer, I want the step-4 derived fields already gone, so that step 5's list of `_own` cases is exactly the 15 the sorting left it.
40. As a future step-6 implementer, I want the call-scoped key bundles (`planet_keys_t.pos`, `toast_keys_t.pos`, `button_keys_t.pos`, `layout_row_keys_t.pos`) left as they are, so that step 6 converts them with its own counts.

## Hypotheses on step 3

Step 3 is done (commits `f10d620`..`d645544`, plan section 7 "Step 1.3"). Each hypothesis was checked against the committed code.

- **H-4-1 — header id: held.** `entity_t` ends with `entity_id_t id`. The type is in `engine/entity_id_type.h` (only `<stdint.h>`). `entity_init_header` zeroes it, and `entity_id_take` writes it in `entity_construct`. The header grew 8 bytes, padding included, and `HEADER_ID_BYTES` went into `ZONE_BUDGET_BYTES`. Step 4's header writes must never overwrite `id` with a whole-struct assignment after construction.
- **H-4-2 — targeting target: held.** `targeting_t.target` is a `targeting_target_t` (kind + id), read through `targeting_target()` and written through `targeting_set_target()`. `targeting.h` counts 1 (`archetype`). `targeting_t` still holds `track_pos` / `track_map_pos`.
- **H-4-3 — no forget: held.** `exit_queue_flush` takes no callback. `forget`, `planet_forget_targeted_by` and `economy_score_difficulty_quits` are gone.
- **H-4-4 — guard counts: held.** 211 pointers, 0 entity lists. Step 4's files read `entity_class.h` 17, `layout.h` 9, `message_box.h` 12, `mouse.h` 2, `planet.h` 15, `probe.h` 5, `scroll.h` 4, `slipway.h` 8, `structure_icon.h` 7, `ui_button.h` 17, `wormhole.h` 7. All 33 step-4 pointers are still declared.
- **H-4-5 — slipway end: held.** `slipway_t.end` is an id plus `int end_is_point`, and `slipway_end(self)` returns the embedded `point.header` or the resolved entity. `slipway_mouse_point_t` keeps its derived `pos` / `map_pos`, and the slipway keeps `point_pos` / `point_map_pos`. `slipway_end_pos` / `_map_pos` still take an `entity_t *`.
- **H-4-6 — links resolve before the header is read: held.** `planet_display` reads `entity_get(self->targeted_by)->targeting`, and `ui_update_hover` writes `is_hovered` through `ui_previous_hovered()` and the resolved `hovered`.
- **H-4-7 — singletons: held, nuance.** The mouse is a state plus an id. `main_mouse_typed()` returns the real mouse or NULL, never the placeholder: only `main_mouse()` returns the placeholder's header. The static placeholders (`g_main_mouse_placeholder`, `g_main_economy_placeholder`) initialise their header with `{.class = …}`, which stays valid with the new layout (zeroed `has`).
- **H-4-8 — positions untouched: held.** The derived position fields, `planet_keys_t`, `toast_keys_t`, `probe_host_set_positions` and the net-income toast (`keys.pos = self->pos`) are unchanged. `trade_route_t` has no header. Research projects and both economy structs still embed a never-constructed header.
- **H-4-9 — green baseline: held, corrected.** Shas are unchanged, and `make sanitize-net` and `make full-game` passed. The header grew 8 bytes, not 4. Step 4 shrinks the header and does not lower `HEADER_ID_BYTES` or any budget: the plan only forbids raising them.
- **H-4-10 — id-list twin: held.** `entity_geom_nearest_in_ids` exists, and both variants read through the static `map_pos_of`, which step 4 rewrites once. `entity_id_link` exists for writing links.

## Implementation Decisions

### Header layout (engine)

- `entity_t` keeps `class`, `state`, `state_time` and step 3's `id` (H-4-1), and replaces the 12 pointers with: `uint16_t has`, `p8_fixed_t layer`, `vec_t pos`, `vec_t map_pos`, `box_t hit_box`, `p8_fixed_t cursor_sprite`, `p8_fixed_t infraspace`, `p8_fixed_t shake`, `p8_fixed_t fold_anchor`, `int is_dead`, `int is_hovered`.
- One presence bit per value: `ENTITY_HAS_LAYER`, `ENTITY_HAS_POS`, `ENTITY_HAS_MAP_POS`, `ENTITY_HAS_HIT_BOX`, `ENTITY_HAS_CURSOR_SPRITE`, `ENTITY_HAS_INFRASPACE`, `ENTITY_HAS_SHAKE`, `ENTITY_HAS_FOLD_ANCHOR`, `ENTITY_HAS_IS_DEAD`, `ENTITY_HAS_IS_HOVERED`. `entity_init_header` leaves `has` at 0.
  - DECISION (guideline): every one of the 10 values gets a bit, including `is_dead`, whose absence reads as 0 anyway. WHY: the plan says "with their presence in one `uint16_t has`", and the existing `assert`s on `is_hovered` and `shake` need presence. One uniform rule is the smaller design.
- Both levels of absence collapse into one bit. "The class has no position member" (header pointer NULL) and "the member is absent" (derived pointer NULL) are both "bit clear", which is how every reader treats them today (`pos_of` → zero vector, `draw` → NULL).
- Static inline helpers in `entity_class.h`: a presence test (`entity_has(e, bit)`), position and map-position readers that return a `vec_t` with the zero fallback, and setters and clearers for position and map position (value in, bit set or cleared). They take and return `vec_t` by value (section 2, small values). Other fields are read and written directly with their bit.
- `struct emitter;`, `struct targeting;` and the two fields leave `entity_class.h`. `struct particle` (the `spawn_particle` return type) stays.
- `draw_layers` passes `entity_has(qc, ENTITY_HAS_POS) ? &qc->pos : NULL` to `draw` / `draw_overlay`, an address valid only for the call. The draw signature changes in step 6, not here.
- `entity_geom` (contains-cursor, on-screen, shake, nearest and its id twin), `update_entities`, `draw_layers` and `devtools/state.c` read the header values and bits. Their behaviour, `assert`s and dump text stay as they are.

### Construction and ownership (game)

- Every constructor that wired a header pointer now writes the value and sets the bit. For an archetype field it copies the archetype's value. For a derived field the header becomes the field.
- Derived copies removed. The header is the only owner:
  - `planet_t`: `shake`, `pos`, `map_pos`, `fold_anchor`, `is_dead`, `is_hovered`.
  - `probe_t` (probe and help screen): `pos`, `map_pos`, `is_dead`, `is_hovered`.
  - `camera_t`: `shake`, `pos_before_shake`, `pos_before_shake_own`, `pos`, `pos_own`.
  - `mouse_t`: `pos`, `pos_own`, and the static zero vector behind it.
  - `layout_row_t.pos`.
  - `toast_t`: `pos`, `pos_box`, `is_dead`.
  - `message_box_t.is_dead`.
  - `slipway_t`: `layer`, `infraspace`, `has_infraspace`, `is_dead`, `point_pos`, `point_map_pos`.
  - `slipway_mouse_point_t`: `pos`, `map_pos`.
  - `structure_icon_t`: `pos`, `is_hovered`.
  - `button_t`: `layer`, `pos`, `pos_box`, `is_hovered`.
  - `modal_backdrop_t`: `is_dead`, `is_hovered`.
  - `wormhole_t`: `pos`, `map_pos`, `is_dead`.
  - `score_panel_t.layer`.
  - `background_t.is_hovered`.
  - `targeting_t`: `track_pos`, `track_map_pos`.

  If another derived field turns out to back a header pointer, it goes too, by the same rule.
- Positions are copies. A constructor that receives a position by pointer in its keys (`planet_keys_t`, `burst_keys_t`, `layout_row_keys_t`, `button_keys_t`, `toast_keys_t`) copies the value into the header and sets the bit if the pointer is non-NULL, and never keeps the address. `layout_row_copy` copies the source row's header position and bit.
- The camera's `map_pos` is its position before shake, as the header maps it today. Camera code reads it through a short named accessor in the camera module, so the name `pos_before_shake` survives where it helps the reader. `camera_t.velocity` stays a pointer: it belongs to step 5.
- A button's hit box is computed as a `box_t` value from its width and written into the header. No box is allocated in the button's slot any more. `box_make` itself is left alone (test_allocator still uses it).
- The slipway's infraspace: "an infraspace of 0 still counts as one" becomes the bit set with value 0. The slipway's layer (2 or 4 from the infraspace) is written into `header.layer`.
- `planet_bob` adds the bob to `header.pos.y` when the position bit is set.
- `targeting_track` (in `probe_host_set_positions`' place) sets the host's header map position to the mouse map position. It sets the header position to the mouse position when there is one, and clears the position bit when there is none, exactly the states the re-pointing produced. The function no longer needs to know the host's concrete type for positions, so the class switch in `probe_host_set_positions` goes away, and its `assert` (probe, planet family or wormhole) is kept where the host is checked.

### Toast following its source (guideline DECISION)

- DECISION: `toast_keys_t` gains an `entity_id_t` naming the entity whose position the toast starts from, set by `planet`'s net-income toast with `entity_id_link`. At construction the toast copies the source's header position. At its first update it re-reads the source's position, if the id still resolves and the position bit is set, before adding its velocity. It then clears the id. Every other toast (mouse-based, NULL position) is unchanged.
- WHY: today the toast's `pos` points at the planet's own vector, which `planet_bob` writes in place every settled update. That is the cart's shared Lua table, and `toast_update` stops sharing it by rebinding to a new vector. A plain copy at construction would lose any bob between the toast's creation and its first update, and a sub-pixel difference can flip a rounded pixel and change a sha (plan section 2, "the game doesn't change"). Following until the first update reproduces exactly the window the cart shares. The id is a value and resolves to NULL after the source's flush, so no address is kept (section 2, no callee keeps an address).
- Presence is tested by resolving the id, never by comparing it to "no link" (step 3 D8).

### `targeting` and `emitter` out of the header

- `planet_t`: `targeting_own` and `emitter_own` become plain value fields `targeting` and `emitter`, and the pointer fields `targeting` / `emitter` go. `planet_t` gains a `uint16_t has` with `PLANET_HAS_TARGETING` and `PLANET_HAS_EMITTER`, set by `structure_init_instance`, `ascension_init_instance` and `burst_create` where they wired the pointers.
- `probe_t` (probe and help screen), `slipway_t` and `wormhole_t`: the `_own` value becomes the plain field, and the pointer goes.
  - DECISION (guideline): no presence bit in structs whose every class carries the component (`probe_t` with both, `slipway_t` and `wormhole_t` with targeting). WHY: section 2's presence bit is for an optional value. Here the class guarantees presence, so `targeting_of` / `emitter_of` return the field unconditionally. The smaller change that satisfies the plan.
- `targeting_of(entity_t *e)` returns a `targeting_t *` into the host, or NULL. It checks the probe lineage (probe, help screen), then the planet family (field if `PLANET_HAS_TARGETING`), then `slipway_class()`, then `wormhole_class()`. It is declared in `game/targeting.h` and defined next to `targeting_track` (`probe.c`), which already knows every host type.
- `emitter_of(entity_t *e)` returns an `emitter_t *` into the host, or NULL. It checks the probe lineage, then the planet family (field if `PLANET_HAS_EMITTER`). It is declared in `game/particle.h` and defined in `particle.c`, which includes the host headers.
- Their result is call-scoped: it is never stored in a struct field or a file-scope variable. The existing `assert`s ("a class that carries no targeting is no receiver") test the accessor's result.
- Code that wrote `self->targeting->x` on its own concrete type writes `self->targeting.x`. The target still goes through `targeting_target()` / `targeting_set_target()` (H-4-2), now given `targeting_of(e)` or `&self->targeting`.

### Guard

- `tools/pointer_guard.sh` is unchanged. `tools/pointer_guard_exceptions.txt` goes down in each chunk by exactly what the chunk removes.
- Final deltas: `entity_class.h` −12, `planet.h` −4, `probe.h` −4, `slipway.h` −3, `wormhole.h` −3, `scroll.h` −2, `layout.h` −1, `message_box.h` −1, `mouse.h` −1, `structure_icon.h` −1, `ui_button.h` −1. That is −33 in total.
- Final values (H-4-4 held): `entity_class.h` 5, `layout.h` 8, `message_box.h` 11, `mouse.h` 1, `planet.h` 11, `probe.h` 1, `scroll.h` 2, `slipway.h` 5, `structure_icon.h` 6, `ui_button.h` 16, `wormhole.h` 4. Total 178 pointers, 0 entity lists.
- The pointers left in `entity_class.h` are `class`, `entity_kind_t.record` and `planet_keys_t`'s three fields (step 6).
- No field this step adds is a pointer: `has`, the values, the toast's source id.

### Comments

Per CLAUDE.md § Comments, the "rebind the cursor, never write through it" comment in `structure_icon.c` loses its reason once positions are values: it goes. The toast's follow-until-first-update rule gets one short comment, because it is a trap someone would "simplify" away. The `assert` messages that name a member ("has no is_hovered member") stay true with "member" read as "presence bit".

## Implementation Chunks

Each chunk is green and committable. `make test-all` passes, every sha is unchanged, and the exception file goes down by exactly the chunk's removals.

1. **`has` and the archetype scalars.** Add `uint16_t has` and the helpers. `layer`, `hit_box`, `cursor_sprite`, `infraspace` and `fold_anchor` become header values with bits, copied from the archetype at construction. Remove `button_t.layer`, `slipway_t.layer`, `score_panel_t.layer`, `slipway_t.infraspace` / `has_infraspace` and `planet_t.fold_anchor`. The button's hit box becomes a value. Update `draw_layers`, `ui_update_hover`, the UI cursor, `entity_geom_contains_cursor`, the slipway infraspace logic, `wormhole_anchor`, planet trade and `devtools/state.c`. Guard: `entity_class.h` −5.
2. **Flags.** `shake`, `is_dead` and `is_hovered` become header values with bits. Remove the derived `shake`, `is_dead` and `is_hovered` fields listed above. `update_entities`, `entity_geom_shake` and the hover code read and write the header. Guard: `entity_class.h` −3.
3. **Position readers.** Every reader of a header position (engine geom and its id twin, `draw_layers`, `devtools/state.c`, `slipway_end_pos` / `_map_pos`, particle host position, probe, wormhole) goes through the position helpers, still backed by today's pointers. No representation change, no count change. This chunk keeps chunk 4's diff to writers and fields only.
4. **Position values.** `pos` and `map_pos` become header values. Remove every derived position field and backing buffer listed above. `targeting_track` writes the header, `planet_bob` writes the header, constructors copy key positions, the camera, mouse, toast, button, layout row, structure icon and slipway point write their header, and the toast follows its source until its first update. Guard: `entity_class.h` −2, `planet.h` −2, `probe.h` −2, `scroll.h` −2, `slipway.h` −2, `wormhole.h` −2, `layout.h` −1, `message_box.h` −1, `mouse.h` −1, `structure_icon.h` −1, `ui_button.h` −1 (−17).
5. **`targeting` and `emitter` out of the header.** Plain value fields in `planet_t` (with `has` bits), `probe_t`, `slipway_t` and `wormhole_t`. `targeting_of` / `emitter_of` replace every `e->targeting` / `e->emitter`, and `entity_class.h` no longer names them. Guard: `entity_class.h` −2, `planet.h` −2, `probe.h` −2, `slipway.h` −1, `wormhole.h` −1 (−8). After this chunk, `plan-pilot` runs `make sanitize-net` and `make full-game`.

## Testing Decisions

- A good test builds entities through their public constructors and observes what a caller sees: which presence bits a class sets, the values copied from its archetype, where it draws, what `targeting_of` / `emitter_of` return, and the position after a drag, a bob or a toast update. It never relies on struct layout beyond the header's public fields.
- `tests/test_entity_header.c` (prior art: its malloc-filling re-exec) is rewritten. For every class it builds, it checks that the header's `has` is exactly the expected bit set (so an unset bit is 0 even under malloc fill), and that the copied values equal the archetype's. Pointer-identity checks go: per CLAUDE.md, a unit test that froze the old behaviour is updated.
- `tests/test_probe.c` (or the module test that drives `targeting_track`): a drag sets the host's header position and map position, a drag without a mouse position clears the position bit, and a second drag overwrites the first without re-pointing. Each host family is covered: probe, help screen, structure, wormhole.
- `tests/test_toast.c`: a toast built from a planet's position follows a bob applied before its first update, and is fixed after it. A toast whose source is flushed before its first update keeps its copied position. A mouse-based toast is unchanged.
- `tests/test_camera.c`: the camera's map position is its position before shake and its position is the shaken one, after an update.
- `tests/test_slipway.c`: the infraspace bit with a 0 value, the layer 2/4 rule, and the mouse point's header position when the end is the point.
- `tests/test_planet.c` / `test_structure.c` / `test_burst.c` / `test_probe.c`: `targeting_of` and `emitter_of` per class, including NULL for a planet without a component, a button and a layout row.
- `tests/test_button.c`: the hit box value follows the width.
- Existing tests that read `planet->pos`, `probe->is_dead`, `self->targeting->…` and the like are updated to the header or the accessor. About 40 test files touch these names. That is mechanical, not new behaviour.
- `tests/test_pointer_guard.c` is unchanged. The guard itself proves the counts.
- Outside unit tests: every sha unchanged (`make e2e`) after every chunk. `make sanitize-net` and `make full-game` after the step. `make test` and `make e2e` wall time compared before and after in alternating pairs, as in Step 1.1.

## Out of Scope

- The 15 `_own` values, aliases and lists held by pointer of step 5 (`camera_t.velocity`, `ui_t.mouse_world`, `planet_t.production`…), and `wormhole_t.too_far_target` as a flag.
- Mixed static/runtime rows (step 5b).
- Call-scoped bundles and small values passed by pointer (step 6): `planet_keys_t.pos/map_pos`, `toast_keys_t.pos`, `button_keys_t.pos`, `layout_row_keys_t.pos`, `open_box_frame_t.pos`, and the `const vec_t *screen_pos` parameter of `draw`.
- `emitter_t.particles` (a list held by pointer, step 5) and `targeting_t.archetype` (allowed).
- Removing `box_make`, or the galaxy's position boxes that `planet_keys_t` still points at.
- Moving `entity_t` / `entity_class.h` into `game/`. Audio, the raylib binary, PSX.

## Further Notes

- Count check: 12 header pointers + 15 derived position fields (`layout.h` 1, `message_box.h` 1, `mouse.h` 1, `planet.h` 2, `probe.h` 2, `scroll.h` 2, `slipway.h` 2, `structure_icon.h` 1, `ui_button.h` 1, `wormhole.h` 2) + 6 component pointers (`targeting` of planet, probe, slipway and wormhole; `emitter` of planet and probe) = 33, matching the step-1 sorting "Step 4: header (33)". The 21 non-header fields are the "21 derived fields" of the Step 1.1 note.
- Risk, aliasing: today some derived positions share a vector with another object. These include the planet replaced by a project (`planet_apply_project` passes its own position to the new planet), the toast above, and a host pointing at its targeting buffer, which `targeting_track` writes in place every drag frame. Only the toast case can be observed, as far as the code shows: the replaced planet is dead before the new one bobs, and nobody but the host borrows its targeting buffer. If a sha still changes, find the shared vector with `help-me-debug` and reproduce the cart's table sharing with an id, as the toast does. Never keep a pointer, never relax an `assert`.
- Risk, slot and zone use: the header shrinks (12 pointers of 8 bytes replaced by about 64 bytes of values, on 64-bit), and buttons no longer allocate a box. Peaks only fall, and no budget is lowered (`HEADER_ID_BYTES` stays, H-4-9). A unit test that froze a byte count (`test_state_peaks`, `test_slot_tally`, `test_ui_zone`) is updated to the new measurement, never loosened past it.
- The step graduates with one line in plan section 7: final counts (178 / 0), whether the toast follow was needed (did a plain copy change a sha?), and any GAP touched (none expected).
