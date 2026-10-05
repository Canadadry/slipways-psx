---
title: "Owned values and lists by value in game/"
description: "Game structs still reach data they own, or a list they hold, through a pointer to a sibling `_own` field, a separately allocated list or a frame-zone alias, so a field's type does not say whether it is present, owned or shared."
status: needs-triage
---

## Problem Statement

Plan 13 (`docs/plan-13.md`), phase 2 (Readability), step 5.

After step 4, the header owns its data as values, but the derived game structs still hold 30 pointers that the step-1 sorting assigned to this step ("Step 5: owned values, aliases, lists by pointer (30)"), plus `wormhole_t.too_far_target`, which the user moved to this step (section 7, "Step 1 sorting — user decisions"):

- **`x = &self->x_own` pairs.** `economy_t.production` / `.demand`, `planet_t.production`, `camera_t.velocity`, `ui_t.mouse_world`, `ui_t.hovered_by_layer`, `slipway_t.served_planets`, `research_project_t.layout_row`, `button_t.slide_target` (backed by `slide_target_box`), `toast_t.text` (backed by `text_buf`). Each piece of data has two names, and "present" is spelled "the pointer is non-NULL".
- **Lists held by pointer (15).** `economy_t.invented`, `emitter_t.particles`, `planet_t.slipways` / `.needs` / `.incoming_routes` / `.outgoing_routes`, `trade_offer_t.accepted_routes`, `research_project_t.inputs`, `slipway_t.transit_dots`, `trade_route_t.planets` / `.slipways`, `trade_keys_t.planets` / `.slipways`, `ui_t.ephemerals`, `wormhole_t.anchors`. Each list struct is allocated apart from its owner, and the code cannot tell "no list" from "an empty list" without reading every site.
- **Aliases into memory that can go away.** `slipway_served_planets_t.ids[]` keeps the addresses of game-zone planet-id strings. `score_panel_layout_block_t.description` keeps a frame-zone string in a file-scope variable, where it dangles once the frame zone is rewound. `structure_icon.c`'s `g_cursor` points at a fresh game-zone `vec_t` on every reveal. `pause_menu_open_box_t.fixed_pos` / `.row_step` point at two file-scope vectors kept only so they can be pointed at.
- **A pointer that is really a flag.** `wormhole_t.too_far_target` always holds the address of the same static sentinel.

The pointer guard counts every one of them. The plan's Goal (readability: "one name per piece of data, and a field whose meaning its type states") and its rules ("optional value: `T x` + a presence bit in the struct's `uint16_t has`"; "list: held by value, never by pointer") call for removing them.

## Solution

Every owned component is a value, and every list a struct holds is a value:

- Each `x` / `x_own` pair becomes one value field `x`. When the cart can see that the value is absent, its presence is a bit in the struct's `uint16_t has`. Otherwise the field is just the value.
- Each list held by pointer becomes a list held by value (`pointer_list_t`, `entity_id_list_t`, `trade_route_list_t`). The list's `zone` member still names the slot or arena its items live in. A list the cart tells apart as "absent" from "empty" gets a presence bit, and every other list is empty until it is built.
- Aliases become copies. A slipway's served planets hold copies of the planet-id strings. The structure-icon cursor is a file-scope `vec_t` plus a presence flag. The pause menu's frame positions are values. The score panel stops storing a frame-zone string it never reads back.
- `wormhole_t.too_far_target` becomes a presence bit.

Every `*.sha256` in `tests/e2e/` stays identical. The pointer guard goes down by exactly 31 (30 step-5 pointers plus `too_far_target`): from 178 to 147 if step 4 lands as planned (H-5-1). The entity-list count stays 0. `make test` and `make e2e` are no slower. `make sanitize-net` and `make full-game` pass.

## User Stories

1. As a port developer, I want `economy_t.production` and `.demand` to be `resource_map_t` values with presence bits, so that "the economy has not counted a year yet" is a bit, not a NULL pointer next to an `_own` copy.
2. As a port developer, I want `economy_t.invented` held by value, so that the list of invented technologies is part of the economy and not a separate game-zone block.
3. As a port developer, I want a planet's own production stored as a value with a presence bit, so that "this planet has its own production" is stated by the bit.
4. As a port developer, I want a planet with no production of its own to keep reading and writing the shared production every such planet shares, so that the cart's class-level table behaves exactly as before.
5. As a port developer, I want one accessor that returns a planet's production (its own, or the shared one), so that no call site repeats the fallback and nobody keeps the result.
6. As a port developer, I want `camera_t.velocity` to be a `vec_t` copied from the archetype at construction, so that the camera owns its velocity from the first frame.
7. As a port developer, I want `ui_t.mouse_world` to be a `vec_t` with a presence bit, so that "no ui update has run yet" stays distinct from a zero position.
8. As a port developer, I want `ui_t.hovered_by_layer` held by value with a presence bit, so that `ui_send_to_hovered` still does nothing before the first hover pass.
9. As a port developer, I want `ui_t.ephemerals` to be a list held by value, so that the ui owns the list header and the slot holds only the items.
10. As a port developer, I want "the ui owns its ephemerals slot" to be a bit, so that the shared empty list and the address comparison against it go away.
11. As a port developer, I want `ui_update` to detect that the ephemerals were replaced during the frame through a counter, so that the test no longer compares list addresses.
12. As a port developer, I want `ui_ephemerals_new` to return a list by value and `ui_set_ephemerals` to take one, so that ownership of the slot moves with the value.
13. As a port developer, I want `emitter_t.particles` held by value with a presence bit, so that an emitter with no particle list (the burst) is still told apart from one with an empty list.
14. As a port developer, I want `slipway_t.transit_dots` and `slipway_t.served_planets` held by value, so that a slipway's lists live in the slipway.
15. As a port developer, I want a slipway's served planets to hold copies of the planet-id strings, so that the set never points into memory a reset frees.
16. As a port developer, I want an `assert` with a message when a planet id does not fit the copy, so that nobody widens the buffer silently.
17. As a port developer, I want `wormhole_t.anchors` and `planet_t.slipways` held by value, so that the slot they recover from the list's zone is still the owner's slot.
18. As a port developer, I want `planet_t.needs` held by value with a presence bit, so that "an uncolonised planet has no needs" (no production counted, crowding skipped) is stated by the bit.
19. As a port developer, I want `research_project_t.inputs` held by value, so that every project owns its inputs list in its table row.
20. As a port developer, I want `research_project_t.layout_row` to be a `data_row_t` value with a presence bit, so that `layout_row_own` goes and "the project has no icon row" is a bit.
21. As a port developer, I want `planet_t.incoming_routes` and `.outgoing_routes` held by value with presence bits, so that a planet the rebuild never reached still reads as having no routes.
22. As a port developer, I want a burst to carry an empty outgoing-routes list of its own, so that the file-scope `g_burst_outgoing_routes` goes.
23. As a port developer, I want `trade_route_t.planets` / `.slipways` and `trade_keys_t.planets` / `.slipways` held by value, so that a route is one value with no pointer to a list header.
24. As a port developer, I want a route's id lists treated as immutable once built, so that two copies of a route can share the same items safely.
25. As a port developer, I want `trade_offer_t.accepted_routes` held by value, with the planet taking the list back after each offer, so that the stack offer holds no address of the planet's list.
26. As a port developer, I want `planet_rebuild_trades` to update the planet's own outgoing routes in place, so that the caller no longer reassigns a returned pointer.
27. As a port developer, I want `button_t.slide_target` to be a `vec_t` with a presence bit, so that `slide_target_box` goes.
28. As a port developer, I want `toast_t.text` to be the character buffer itself, so that `text_buf` and the pointer into it go.
29. As a port developer, I want the pause menu's fixed position and row step stored as values in its open-box record, so that the two file-scope vectors behind them go.
30. As a port developer, I want the structure-icon cursor to be a file-scope `vec_t` with a presence flag, so that each reveal stops allocating a vector in the game zone.
31. As a port developer, I want the score panel's layout block to stop storing the tech-tree description, so that no file-scope variable keeps a frame-zone string.
32. As a port developer, I want `wormhole_t.too_far_target` to become a presence bit, so that a pointer that always held one address says what it means.
33. As a port developer, I want new presence bits to live in the struct's `uint16_t has` (created where the struct has none), so that every struct follows the plan's optional-value rule.
34. As a port developer, I want the construction-byte formulas (planet, slipway, wormhole) to stop adding a separate list block, so that the grab arena is sized for what is really allocated.
35. As a reviewer, I want `tools/pointer_guard_exceptions.txt` lowered in each chunk by exactly that chunk's removals, with a file's line removed when it reaches 0, so that the ratchet proves what each commit did.
36. As a reviewer, I want no new pointer field or file-scope pointer in `src/game/` or `entity_class.h`, so that the guard only goes down.
37. As a reviewer, I want no new function that takes a `vec_t`, `box_t` or `p8_fixed_t` by pointer, and no callee that keeps an address it received, so that the verifier finds no new violation of the rules the guard cannot see.
38. As a reviewer, I want the accessors this step adds (planet production, address-or-NULL of an optional value) used only within a call and never stored, so that no address is kept.
39. As a reviewer, I want every `*.sha256` in `tests/e2e/` unchanged after every chunk, so that each chunk is proven not to change the game.
40. As a reviewer, I want `make sanitize-net` and `make full-game` to pass after the step, so that the embedded lists leave no use-after-free and no full zone or slot.
41. As a port developer, I want `make test` and `make e2e` no slower, so that the fast loop stays fast.
42. As a port developer, I want no game concept added to `src/engine/`, so that the engine stays free of planets and slipways.
43. As a future step-5b implementer, I want the research project's row held by value but its items untouched, so that step 5b handles the mixed static/runtime rows on its own.
44. As a future step-6 implementer, I want the call-scoped bundles (`button_keys_t.slide_target`, `toast_keys_t.text`, `open_box_frame_t.fixed_pos` / `.row_step` / `.description`) left as they are, so that step 6 converts them with its own counts.

## Hypotheses on step 4

Step 4 (`docs/prd/triage/phase2-step4-header-as-values.md`) is being implemented while this PRD is written. Nothing below was checked against its code. Before chunk 1, the implementer checks each hypothesis against the committed code and the step-4 note in plan section 7. When one fails, the chunk adapts to the real code and keeps the intent of the decisions below. The guard deltas are then taken from the real exception file: each chunk lowers each file by exactly the pointers it removes.

- **H-5-1 — guard after step 4.** 178 pointers, 0 entity lists. Per-file values for the files this step touches: `main_economy.h` 5, `menu.c` 2, `message_box.h` 11, `particle.h` 3, `planet.h` 11, `planet_trade.h` 1, `research.h` 12, `score_panel.h` 5, `scroll.h` 2, `slipway.h` 5, `structure_icon.c` 1, `trade.h` 4, `ui.h` 4, `ui_button.h` 16, `wormhole.h` 4. All 31 pointers of this step are still declared.
- **H-5-2 — header values and `has`.** `entity_t` holds `uint16_t has` and value fields (`pos`, `map_pos`, `layer`, `is_dead`…) with `ENTITY_HAS_*` bits and the inline helpers (`entity_has`, position readers and setters taking and returning `vec_t` by value). An archetype's values are copied into the header at construction.
- **H-5-3 — derived position fields gone.** `camera_t.pos` / `pos_before_shake` (and their `_own`), `mouse_t.pos`, `toast_t.pos` / `pos_box`, `button_t.pos` / `pos_box`, `structure_icon_t.pos`, `planet_t.pos` / `map_pos`, `wormhole_t.pos` / `map_pos` and the slipway point fields are gone. Code reads the header. `camera_t.velocity` / `velocity_own` are untouched.
- **H-5-4 — `targeting_of` / `emitter_of`.** `targeting` and `emitter` are no longer in `entity_t`. `planet_t` holds `targeting` and `emitter` as value fields, with `PLANET_HAS_TARGETING` / `PLANET_HAS_EMITTER` in a new `uint16_t has` of `planet_t`. `probe_t` holds both as plain values. The game reaches an emitter through `emitter_of(e)`, whose result nobody keeps. `emitter_t.particles` is still a `pointer_list_t *`.
- **H-5-5 — no other `uint16_t has`.** Apart from `entity_t` and `planet_t`, no struct this step touches (`economy_t`, `ui_t`, `emitter_t`, `button_t`, `wormhole_t`, `research_project_t`, the pause menu's open-box record) has a `uint16_t has`. Their existing `int has_*` flags are unchanged.
- **H-5-6 — structure-icon cursor.** `structure_icon_refresh` copies the shared cursor into the icon's header position (bit set when the cursor is present) instead of re-pointing `self->pos`. `g_cursor` is still a `const vec_t *` that a reveal re-points at a new game-zone vector, and `structure_icon_cursor()` / `structure_icon_set_cursor()` keep their signatures.
- **H-5-7 — the ui's mouse position.** `ui_update` computes the world mouse position from the mouse's and the camera's header positions (zero fallback) and still writes it through `mouse_world_own` / `mouse_world`. `hit_test` still takes `const vec_t *mouse_world`. Draw and press signatures keep their vector pointers until step 6.
- **H-5-8 — toast.** `toast_t.text` / `text_buf` and `toast_keys_t.text` are untouched. The toast may hold a source entity id for its first update (step 4's guideline decision). That does not affect this step.
- **H-5-9 — lists untouched.** Every list this step converts (H-5-1) is still held by pointer, allocated where it is today (planet and slipway lists in the grab-arena construction zone, particle lists in particle-zone slots, ephemerals in a ui-zone slot, `invented` and the route lists in the game zone, `inputs` in the load zone). `g_burst_outgoing_routes` and `g_shared_ephemerals` still exist.
- **H-5-10 — memory constants.** Step 4 lowered no zone constant and no hand-measured slot size (`BURST_SIZE`, `PARTICLES_SIZE`, `EPHEMERALS_SIZE`…), and `planet_t` / `probe_t` got smaller. Byte-count tests (`test_state_peaks`, `test_slot_tally`, `test_ui_zone`, `test_game_zone`, `test_live_slots`) were re-measured to step 4's layout.
- **H-5-11 — green baseline.** After step 4: shas unchanged, `make sanitize-net` and `make full-game` passed, and `make test-all` is green.

## Implementation Decisions

### General rules for this step

- A value with an `_own` twin becomes one field under the plain name. The `_own` / `_box` / `_buf` name goes.
- **Presence bits.** A bit is added only where some reader acts differently on "absent" than on "empty" or "zero". New bits go in a `uint16_t has` of the struct, created where the struct has none (H-5-5), with `<STRUCT>_HAS_<FIELD>` names. Existing `int has_*` flags are not converted (out of scope).
  - DECISION (guideline): mixing `int has_*` flags and a `uint16_t has` in one struct is accepted for this step. WHY: plan section 2 places new presence bits in `uint16_t has`, and converting the older flags is not what step 5 needs.
- **Lists by value.** A list that is built at construction and read only after it gets no bit. A zero-initialised list (count 0) reads as empty, and every engine iterator and count already handles count 0. A list is never added to before it is initialised with a zone.
- **Passing optional values on.** Where a converted value is handed to a function that still takes a pointer (hit tests, row builders, step-6 bundles), the caller passes the field's address when the bit is set and NULL otherwise. That address is valid only for the call, the same pattern step 4 uses for `draw`. No function this step adds or changes takes a `vec_t`, `box_t` or `p8_fixed_t` by pointer.
- **Zone and slot sizes.** Embedding a list header in its owner moves bytes from a separate block into the owner. No zone constant and no `ZONE_BUDGET_BYTES` is raised. The construction-byte formulas (`planet_kind_construction_bytes`, `planet_project_bytes`, `SLIPWAY_CONSTRUCTION_BYTES`, the wormhole's grab-arena size) drop their separate list term. Hand-measured slot sizes (`BURST_SIZE`, `PARTICLES_SIZE`, `EPHEMERALS_SIZE`) are left as they are unless a slot overflows.
  - DECISION (guideline): if a measured struct no longer fits its hand-measured slot (the burst's `planet_t` in `BURST_SIZE`, say), that slot size rises by exactly the measured growth, and its zone's static assert must still hold without raising `ZONE_BUDGET_BYTES`. The change is recorded in the step note. WHY: plan section 2 forbids weakening a check to get green; a slot sized by measurement follows the measurement, as step 3 did for `HEADER_ID_BYTES`.
- **No engine change expected.** If an engine list function misbehaves on a zero-initialised list, it is fixed generically in `engine/` without naming any game concept (plan section 2, "no game concept in `src/engine/`").

### Economy and planet production

- `economy_t`: `invented` becomes a `pointer_list_t` value, initialised in the game zone at creation and always present (no bit). `production` and `demand` become `resource_map_t` values. A new `uint16_t has` holds `ECONOMY_HAS_PRODUCTION` and `ECONOMY_HAS_DEMAND`, set where the year's totals are written today. Readers (`score_panel`, `economy_score`, `tech_gate`, the new-year research total) test the bit where they tested NULL. `devtools/state.c` reads `invented` by value, and its dump text is unchanged.
- `planet_t`: `production_own` becomes `production`, with `PLANET_HAS_PRODUCTION` in `planet_t.has` (H-5-4), set by `planet_install_project`. The pointer goes.
  - DECISION (guideline): `planet_production(planet)` returns the planet's own production when the bit is set, and otherwise the file-scope shared production that `planet_shared_production()` already exposes. Every reader and writer (economy tiers, synthesizer resource pick, trade rebuild, display, total production, upkeep) goes through it, and its result is never stored. WHY: today every planet without its own production points at one shared struct, and `economy_tiers` writes through it (`test_planet` pins that a write through one such planet shows on another): the cart's class-level table. An accessor reproduces the sharing exactly, while a per-planet copy would split the shared writes and could change a sha.
  - It gets one short comment: writing through the result for a planet without its own production writes the shared production. That is the trap someone would "fix".

### Owned small values

- `camera_t.velocity` becomes a `vec_t`, copied from the archetype at creation and always present (no bit). `camera_set_velocity` writes it. The test that forces a NULL velocity is rewritten to set the value to zero.
- `button_t.slide_target` becomes a `vec_t` with `BUTTON_HAS_SLIDE_TARGET` in a new `button_t.has`. `slide_target_box` goes. `button_keys_t.slide_target` stays a pointer (step 6), and the constructor copies from it.
- `toast_t.text` becomes the `char` buffer itself (`TOAST_TEXT_CAP + 1`), and `text_buf` goes. No bit: the only reader draws an absent text as `""`, so an empty string and an absent one draw the same. The length `assert` is kept.
- Pause menu: the open-box record holds `vec_t fixed_pos` and `vec_t row_step` with presence bits in a new `has` (set by `menu_init`, cleared by `menu_shutdown`). The file-scope `g_pause_menu_fixed_pos` / `g_pause_menu_row_step` go. The menu fills `open_box_frame_t` (a step-6 bundle) with the address of the value when its bit is set, valid only for the `message_box_open_box` call.
- Structure-icon cursor: the file-scope `g_cursor` becomes a `vec_t` plus a `g_has_cursor` flag. A reveal writes the new cursor value instead of allocating a game-zone vector. The API becomes `structure_icon_has_cursor()`, `structure_icon_cursor()` returning a `vec_t` (zero when absent), `structure_icon_set_cursor(vec_t)` and a clear on init and shutdown. `cart.c` sets the rest cursor by value.
- Score panel: `score_panel_layout_block_t.description` goes.
  - DECISION (guideline): remove the field instead of copying the string into it. WHY: the menu writes it and reads it back in the same call to fill the open-box frame, and no game code reads it afterwards. Storing a frame-zone string in a file-scope variable is the dangling alias the step removes, and removing it is the smaller change. The `test_score_panel` check that hashes it reads the description through the box the menu opened.
- `wormhole_t.too_far_target` becomes `WORMHOLE_HAS_TOO_FAR_TARGET` in a new `wormhole_t.has`, set at construction. The sentinel `g_wormhole_too_far_target_wrapper`, `wormhole_too_far_target_wrapper()` and `wormhole_too_far_target_t` stay, since they are allowed static data the targeting code uses directly.

### UI

- `ui_t.mouse_world` becomes a `vec_t` with `UI_HAS_MOUSE_WORLD`. `ui_update` sets it, and `ui_update_hover` passes its address to `hit_test` when the bit is set, NULL otherwise (H-5-7).
- `ui_t.hovered_by_layer` becomes the value (`hovered_by_layer_own` goes), with `UI_HAS_HOVERED_BY_LAYER` set by `ui_update_hover`. `ui_send_to_hovered` returns 0 while the bit is clear.
- `ui_t.ephemerals` becomes an `entity_id_list_t` value. `UI_HAS_OWN_EPHEMERALS` says its items live in a ui-zone slot the ui owns. `g_shared_ephemerals` and `ui_ephemerals_shared()` go: a new ui starts with an empty list it does not own, which is what the shared list was, since nothing ever adds to it.
- `ui_ephemerals_new()` returns a list by value whose zone is a fresh slot. `ui_set_ephemerals(ui, list)` takes the list by value, records the element count, sets the ownership bit, and sends a previously owned slot to the exit queue as today. Release frees an owned slot. `ui_forget_world` empties the list when owned.
  - DECISION (guideline): `ui_t` gains a `uint16_t ephemerals_serial` that `ui_set_ephemerals` increments. `ui_update` compares it before and after handling input to decide whether the ephemerals were replaced during the frame. WHY: the cart compares table identity, and a counter states that directly. Comparing slot addresses would also work, but only because a replaced slot is freed at the flush: a trap the plan's readability goal rules out.
- `ui_update` keeps a value copy of the list from the start of the frame for the "was the hovered entity one of the ephemerals" test. Its items stay readable until the exit-queue flush, as the old pointer's did.
- `message_box_open_box` builds its ephemerals in a local value list. When there is no ui it builds nothing, as it did with NULL.

### Lists held in a slot or an arena

- `emitter_t.particles` becomes a `pointer_list_t` value with `EMITTER_HAS_PARTICLES` in a new `emitter_t.has`. `emitter_init_in` sets it, and `emitter_release_particles` frees the slot (`particles.zone`) and clears it. Spawning and drawing skip an emitter whose bit is clear (the burst), as they skipped NULL.
- `slipway_t.transit_dots` becomes a value, initialised in the construction zone at `slipway_construct` (no bit: built before registration, read only after). Release reads `transit_dots.zone` as today.
- `slipway_t.served_planets` becomes a value (`served_planets_own` goes), always present after construction (no bit).
  - DECISION (guideline): `slipway_served_planets_t` holds a copy of each planet id in a fixed `char` array sized by a named constant ("p" plus up to two digits plus the terminator), with an `assert` on the length ("stop, do not widen"). The string comparison is unchanged. WHY: plan section 2, "no callee keeps an address it received: it copies the value". A copy keeps the string semantics that a structure built on a planet's id relies on (it reuses the id), where an entity id would not. The set also gets smaller (4 bytes per entry instead of 8).
  - The `economy_tiers` "slipway with no served set" branch goes, and with it the matching bullet of GAP-15 in `docs/porting-gaps.md`: the case can no longer be represented.
- `wormhole_t.anchors` and `planet_t.slipways` become `entity_id_list_t` values, initialised in the construction zone at construct (no bit). The release paths read the slot from `anchors.zone` / `slipways.zone` as today. The dead-planet `assert`s read the value's count.

### Needs and inputs

- `planet_t.needs` becomes a `pointer_list_t` value with `PLANET_HAS_NEEDS`, set by `planet_install_project`. `planet_offer_trade`'s merge assigns the merged list by value instead of allocating a list header. Total production, crowding (GAP-15's "absent or empty" bullet stays true) and display test the bit where they tested NULL.
- `research_project_t.inputs` becomes a `pointer_list_t` value, initialised in the load zone at creation and always present (no bit).
- `research_project_t.layout_row` becomes a `data_row_t` value with `RESEARCH_PROJECT_HAS_LAYOUT_ROW` in a new `research_project_t.has`. `layout_row_own` goes. The row's items still point at the project's own item arrays (step 5b). `planet_display` passes the row's address when the bit is set.

### Trade lists

- `trade_route_t.planets` / `.slipways` and `trade_keys_t.planets` / `.slipways` become `entity_id_list_t` values. A route built from no keys has two empty lists.
  - DECISION (guideline): no presence bit on a route's lists. WHY: every reader (`trade_equal`, `trade_last_planet`, `trade_extend`, `economy_tiers`, `planet_family_trade`) treats a missing list exactly like an empty one, and the game always builds routes with both.
  - A route's id lists are immutable once built: `trade_copy` and `trade_extend` build new lists, and nothing adds to a route's list afterwards. Value copies of a route therefore share items safely, as the pointer copies shared the list. `trade_route_list_add`'s "id lists are shared, not copied" contract stays.
- `planet_t.incoming_routes` and `.outgoing_routes` become `trade_route_list_t` values with `PLANET_HAS_INCOMING_ROUTES` / `PLANET_HAS_OUTGOING_ROUTES`. The incoming reset initialises the list in the game zone and sets the bit the first time, and clears it afterwards. A destination whose bit is clear receives nothing, as before.
- A burst initialises an empty outgoing list with no zone and sets the bit, so it reads as "has routes, none". `g_burst_outgoing_routes` and its init and free go. Nothing ever adds to a burst's outgoing list.
- `planet_rebuild_trades` works on the planet's own `outgoing_routes`: it initialises them in the game zone when the bit is clear and keeps the existing routes otherwise (the cart never drops a kept route). It returns a pointer to that field (NULL for a NULL planet), valid only for the call. `economy_tiers` no longer assigns the result to the planet.
- `trade_offer_t.accepted_routes` becomes a `trade_route_list_t` value. Before each offer, `planet_rebuild_trades` copies its working list into the offer. After `offer_trade` returns, it takes the list back from the offer and recomputes the next offer's amount from it. The accept closure adds to the offer's own value. The game-zone `assert` on the kept route's list is kept.
  - DECISION (guideline): copy in and take back, rather than giving the offer the planet's id. WHY: the offer lives on the stack for one call, and copying the value in and out is the "callee copies, never keeps a pointer" pattern of plan section 2. It needs no change to the offer protocol or to `offer_trade`'s signature.

### Guard

- `tools/pointer_guard.sh` is unchanged. Each chunk lowers `tools/pointer_guard_exceptions.txt` by exactly the pointers it removes. A file whose count reaches 0 loses its line (`menu.c`, `planet_trade.h`, `structure_icon.c`, `trade.h`).
- Final deltas: `main_economy.h` −3, `menu.c` −2, `message_box.h` −1, `particle.h` −1, `planet.h` −5, `planet_trade.h` −1, `research.h` −2, `score_panel.h` −1, `scroll.h` −1, `slipway.h` −3, `structure_icon.c` −1, `trade.h` −4, `ui.h` −3, `ui_button.h` −1, `wormhole.h` −2. That is −31.
- Final values if H-5-1 holds: `main_economy.h` 2, `message_box.h` 10, `particle.h` 2, `planet.h` 6, `research.h` 10, `score_panel.h` 4, `scroll.h` 1, `slipway.h` 2, `ui.h` 1, `ui_button.h` 15, `wormhole.h` 2. Total 147 pointers, 0 entity lists.
- What remains in these files is allowed static data (archetypes, `end_status`, names, `status_text`), owned variable-size buffers (`planet_t.planet_id`, `planet_t.layout_row`), the step-5b rows and the step-6 bundles.

### Comments

Per CLAUDE.md § Comments: one short comment on the planet production accessor (the shared write, above), and `assert` messages for the served-planet id length and for adding to a list that was never initialised. Comments that explained the removed indirections go: "Built in place: a project's layout_row points into the project itself" now belongs to step 5b's items only and is reworded or removed, and `rebind the cursor` goes if step 4 left it. Everything else goes in the commit messages.

## Implementation Chunks

Each chunk is green and committable on its own. `make test-all` passes, every sha is unchanged, and the exception file goes down by exactly the chunk's removals. The order goes from the most isolated changes to the most connected.

1. **Owned small values and the flag.** `camera_t.velocity`, `button_t.slide_target`, `toast_t.text`, the pause menu's frame values, the structure-icon cursor, the score panel's description and `wormhole_t.too_far_target`. Guard: `scroll.h` −1, `ui_button.h` −1, `message_box.h` −1, `menu.c` −2 (line removed), `structure_icon.c` −1 (line removed), `score_panel.h` −1, `wormhole.h` −1 (−8).
2. **Economy and production.** `economy_t.invented` / `production` / `demand` with `economy_t.has`, the planet's own production with `PLANET_HAS_PRODUCTION` and `planet_production()`, and `research_project_t.layout_row` with its bit. Guard: `main_economy.h` −3, `planet.h` −1, `research.h` −1 (−5).
3. **UI.** `mouse_world`, `hovered_by_layer`, `ephemerals` by value with the ownership bit and the serial. `g_shared_ephemerals` goes, and `message_box_open_box` builds a local list. Guard: `ui.h` −3.
4. **Lists in slots and arenas.** `emitter_t.particles` (with its bit), `slipway_t.transit_dots`, `slipway_t.served_planets` (with string copies, and the GAP-15 bullet removed), `wormhole_t.anchors`, `planet_t.slipways`, and the construction-byte formulas for slipway, wormhole and planet slipways. Guard: `particle.h` −1, `slipway.h` −3, `wormhole.h` −1, `planet.h` −1 (−6).
5. **Needs and inputs.** `planet_t.needs` with `PLANET_HAS_NEEDS`, `research_project_t.inputs`, and the needs term of `planet_project_bytes`. Guard: `planet.h` −1, `research.h` −1 (−2).
6. **Trade lists.** Route and key lists by value, `planet_t.incoming_routes` / `outgoing_routes` with bits, the burst's empty list, `planet_rebuild_trades` in place, and the offer's copy-in and take-back. Guard: `trade.h` −4 (line removed), `planet.h` −2, `planet_trade.h` −1 (line removed) (−7). After this chunk, `plan-pilot` runs `make sanitize-net` and `make full-game`.

## Testing Decisions

- A good test builds objects through their public constructors and drives them through the real update and message paths, then checks what a caller sees: whether a presence bit is set, the value read, which entities a list yields, what is drawn or dumped. It never depends on struct layout beyond the public fields, and never compares the addresses of internal buffers.
- Tests that froze pointer identity are updated to the new contract (CLAUDE.md: the e2e replays are the source of truth). For example, `test_ui`'s "ephemerals == shared list" becomes "an empty list the ui does not own", `test_camera`'s NULL velocity becomes a zero velocity, `test_spec_defaults`' `too_far_target == wrapper` becomes the bit, and `test_live_slots`' address snapshot of the camera, mouse and ui vectors goes.
- New or rewritten checks:
  - `tests/test_planet.c`: a planet without its own production reads and writes the shared production through `planet_production` (a write via one shows on another), and a planet with a project reads its own. The existing shared-production assertions are kept in their new form.
  - `tests/test_economy*.c` (or the module test that drives the year update): the production and demand bits are clear before the first year and set after it, and `invented` grows on each research.
  - `tests/test_ui.c`: `ui_update` treats an ephemerals replacement during the frame as a replacement (the serial), a new ui owns no slot, and the slot is sent to the exit queue on replacement and freed on release.
  - `tests/test_slipway.c`: a served planet id added from a buffer that is then overwritten is still found (the set holds a copy), and an id too long for the copy trips the `assert` (in the existing death-test style, if the file has one).
  - `tests/test_structure_icon.c`: two reveals move the cursor by the step, and the game-zone use does not grow per reveal.
  - `tests/test_burst.c`: a burst has outgoing routes present and empty, and no particle list.
  - `tests/test_trade.c`: a route copied into a route list shares its ids, `trade_new(NULL)` gives empty lists, and `trade_equal` / `trade_last_planet` behave as before on empty lists.
  - `tests/test_ascension.c` / `test_burst.c`: `planet_rebuild_trades` updates the planet's outgoing routes in place and keeps earlier routes. The existing callers read the returned pointer, or the planet's field.
  - `tests/test_wormhole.c`: the too-far bit is set after construction, and the anchors list yields the anchors.
- Byte-count tests (`test_state_peaks`, `test_slot_tally`, `test_ui_zone`, `test_game_zone`, `test_live_slots`) are re-measured to the new layout, never loosened past the measurement.
- `tests/test_pointer_guard.c` is unchanged. The guard itself proves the counts.
- Outside unit tests: every sha unchanged (`make e2e`) after every chunk. `make sanitize-net` and `make full-game` after the step. `make state` is run once to confirm no peak eats its 1/8 margin. `make test` and `make e2e` wall time are compared before and after in alternating pairs, as in Step 1.1.

## Out of Scope

- Step 5b: `data_item_t.text/row/ref`, `data_row_t.items`, `layout_row_cell_t.as.*`, `message_box_t.items[]`. The research project's row items keep pointing at the project's own arrays.
- Step 6: call-scoped bundles and small values passed by pointer (`button_keys_t.slide_target`, `toast_keys_t.text`, `open_box_frame_t.*`, `planet_keys_t.*`, `layout_row_keys_t.*`), and existing function signatures taking `vec_t *` (`hit_test`, `draw`, `left_press`, `slipway_drag`, `wormhole_drag`, `ui_send_to_hovered`).
- Converting existing `int has_*` flags into `uint16_t has` bits.
- Pointers allowed by section 2: archetypes, `economy_t.end_status`, `planet_t.planet_id` and `.layout_row` (owned buffers), `score_panel_layout_block_t.fixed_pos` / `.row_step` (static compound literals), `wormhole_too_far_target_t.status_text`, the generator state.
- Lowering zone constants or hand-measured slot sizes to bank the bytes saved. Step 7 measures the balance.
- Moving `entity_t` / `entity_class.h` into `game/`. Audio, the raylib binary, PSX.

## Further Notes

- Count check: 15 lists (`invented`, `particles`, `planet.slipways` / `needs` / `incoming_routes` / `outgoing_routes`, `accepted_routes`, `inputs`, `transit_dots`, `trade_route_t.planets` / `slipways`, `trade_keys_t.planets` / `slipways`, `ui.ephemerals`, `wormhole.anchors`) + 15 other pointers (`economy.production` / `demand`, `menu` `fixed_pos` / `row_step`, `toast.text`, `planet.production`, `research.layout_row`, `score_panel` `description`, `camera.velocity`, `served_planets.ids[]`, `slipway.served_planets`, `g_cursor`, `ui.mouse_world` / `hovered_by_layer`, `button.slide_target`) = 30, matching "Step 5: owned values, aliases, lists by pointer (30)", plus `wormhole_t.too_far_target` = 31.
- Risk, sharing: the planet production (handled by the accessor) and the route id lists (immutable, shared by value copies) are the two places where the pointer form shared data. If a sha changes, find the shared write with `help-me-debug`, and reproduce the sharing without keeping a pointer in a struct. Never relax an `assert`, never regenerate a sha.
- Risk, slot fit: `planet_t` grows by the embedded list headers (slipways, needs, two route lists) and `emitter_t` by the particle list header, while step 4 shrank `planet_t`. If the burst slot overflows, apply the slot-size decision above and record it.
- Risk, ephemerals: the old list's items must stay readable until the flush, for the "hovered was an ephemeral" test after a replacement during the frame. `make sanitize-net` covers it.
- The step graduates with one line in plan section 7: final counts (147 / 0 if H-5-1 held), whether any slot size changed, the GAP-15 bullet removed, and which step-4 hypotheses failed, if any.
