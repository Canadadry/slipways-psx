# Plan 13 — Fewer pointers

## 1. Goal

Game state holds no pointer that can dangle or that needs a paragraph to explain. Entities link to each other through checked ids, a component an entity owns is a value, and a small value travels by value.

Two goals, and only these two, justify every rule below:

- **Safety.** A link to something that died fails loudly (NULL, an `assert`), never reads freed memory silently, and doesn't depend on someone remembering to clear it.
- **Readability.** One name per piece of data, and a field whose meaning its type states.

A rule that works against one of these goals in a case nobody anticipated may be broken on purpose. The commit message then says why. `warn-review` reports it as `breaks rule N on purpose` and doesn't count it as a violation.

## 2. Rules

- **Pointers that stay:** `self` and large structs as parameters (valid only during the call), `const` pointers to static data that is never freed (archetypes, tables, strings), function pointers, runtime handles (`allocator_t`, `p8_graphics_t`, `p8_vram_t`), the engine registry's internal lists (live list, per tag, per message), a variable-size buffer allocated in its owner's zone and freed with its owner (`layout_row_t.items`, `tooltip_t.rows`, `planet_t.layout_row`…), and generator state handed over by `galaxy_populate` (`g_type_picker`, `g_planet_vanish_roll`, `weighted_picker_t.random_ctx`). Every other pointer goes.
- **Optional value:** `T *x` + `T x_own` becomes `T x` + a presence bit in the struct's `uint16_t has`.
- **Link to an entity:** an `entity_id_t`, alone or in a list, never an `entity_t *` or a concrete type pointer.
- **List:** held by value, never by pointer.
- **Small values** (`vec_t`, `box_t`, `p8_fixed_t`) are passed by value.
- **No callee keeps an address it received.** It copies the value or stores an id.
- **The game doesn't change.** Every `*.sha256` in `tests/e2e/` stays identical, and `make test-all` passes on every commit. A changed sha means the step is wrong: it is never regenerated, and an `assert` is never relaxed to get back to green. Investigate with `help-me-debug`. If the cart really reads an object after the port frees it, free it where the cart forgets it, and record the divergence in `docs/porting-gaps.md` if there is one.
- `make test` and `make e2e` don't get slower.
- **Memory checks.** `make sanitize-net` (the e2e under ASan and UBSan) and `make full-game` (worst-case games, failing on `p8_fatal` or a full zone) pass after every step from 2 on. They stay outside `make test-all`.
- **Out of scope:** moving `entity_t` and `entity_class.h` from `engine/` into `game/` is a separate effort. This plan only removes `targeting` and `emitter` from `engine/`.

## 3. Phases and steps

### Phase 1 — Safety

1. **Pointer guard.** `tools/pointer_guard.sh` counts every data pointer, `const` included, in the struct fields and the file-scope variables of `src/game/` (headers and `.c` files) and of `src/engine/entity_class.h`, excluding function pointers and the runtime handles from section 2. `tools/pointer_guard_exceptions.txt` lists one `<file> <count>` line per file, matching today's counts; the `const` pointers to static data that stay end up as its last lines. It also counts, per file, the game fields and file-scope variables that hold a list of entity pointers (`entity_list_t`, or a `pointer_list_t` of entities), by pointer or by value: the list type hides those pointers from the first count. Strict equality, run in `make test`, modelled on `tools/alloc_guard.sh`. Step 1 also sorts every counted pointer: allowed by section 2, or removed by step 3 (a link to an entity or to another object that can die), step 4 (the header) or step 5 (an owned component, a `const` alias into memory that can be freed, a list). The sorting is written in section 7. A pointer that fits none of these goes under its own `Unsorted` heading in section 7. A non-empty `Unsorted` list stops the run before step 2: `plan-pilot` reports it to the user after writing step 1's note. It is not a `QUESTION:` for `guideline`. A unit test proves it fails on a raised count, a new file and each kind of pointer it counts, like `tests/test_alloc_guard.c`. From here on, no step raises a count; steps 3 to 6 lower them, and step 2 leaves them as they are.
2. **Handle table.** `engine/` gets `entity_id_t` = `uint16_t index` + `uint16_t generation`, with `{0,0}` meaning no link.
   - A fixed table of 512 slots. The measured peak is 329 live entities (`cart_game_registry_reserve`), and the table counts against the zone budget: the game zone budget is raised by the table's size. `p8_fatal` when it is full.
   - `entity_construct()` takes a slot. The slot is freed, and its generation incremented, at `exit_queue_flush`, for **every** class: `entity_unregister` therefore queues every entity, with or without a `release`. During the frame, `entity_get(id)` still returns the dead entity; after the flush, it returns NULL.
   - `reset_world()` clears the table.
   - Nobody uses the table yet. It is covered by unit tests.
3. **Links → ids.** About 15 fields (`slipway.start/end/targeted_by`, `planet.targeted_by`, `targeting.target`, `wormhole.anchor`, `ui.hovered/previous_hovered/left_held`, `project_planet`…) and about 10 game lists (`planet.slipways/incoming_routes/outgoing_routes`, `trade.planets/slipways`, `wormhole.anchors`, `accepted_routes`, `ui.ephemerals`…) become ids.
   - A game list of ids holds no dead id: it is removed at the entity's death, where the cart calls `del()`. A walk that meets a dead id `assert`s.
   - `down_t.held` becomes an id.
   - The `forget` callback of `exit_queue_flush` goes away.
   - Research projects are not entities: the links to them (`planet_t.project`, `button_t.project`, `button_keys_t.project`, `available_projects_kept_t.items[]`) and the game-side registry `g_projects[]` / `g_projects_by_name[].project` become project ids (an index into the project table, checked), not entity handles. Projects are freed by `reset_world()`: an id from before the reset resolves to NULL, like an entity id.

### Phase 2 — Readability

4. **Header as values.** The 12 pointers of `entity_t`, besides `class`, (`layer`, `pos`, `map_pos`, `hit_box`, `cursor_sprite`, `infraspace`, `shake`, `fold_anchor`, `is_dead`, `is_hovered`, `emitter`, `targeting`) become values, with their presence in one `uint16_t has`.
   - An archetype's values are copied at construction.
   - The copies in derived structs (`planet->pos`, `probe->pos`, `planet->shake`, `planet->is_dead`…) go away: the header is the only owner.
   - `targeting_track` writes the position instead of re-pointing it.
   - `targeting` and `emitter` leave `entity_t`. Each struct that has one keeps it as a plain field (`probe_t.targeting`), with presence in its `has` bit. The game reaches it through `targeting_of(e)` / `emitter_of(e)`, which switch on the class and whose result nobody keeps. `engine/` no longer names them.
5. **`_own` and lists by value in `game/`.** The fields the step-1 sorting assigns to step 4 (the derived fields the header points at, such as `pos` and `pos_before_shake`) are done by step 4, not here. The rest of the 25 `x = &self->x_own` cases (`production`, `demand`, `velocity`, `pos_before_shake`, `served_planets`, `layout_row`, `mouse_world`, `hovered_by_layer`, `pos`…) become a value + a presence bit. The 15 lists held by pointer become lists held by value. A list the cart distinguishes as "absent" from "empty" gets a presence bit. `wormhole_t.too_far_target` (a pointer to a static sentinel) becomes a flag.
5b. **Rows: static and runtime apart.** `data_item_t.text/row/ref`, `data_row_t.items`, `layout_row_cell_t.as.text/as.row` and `message_box_t.items[]` point sometimes at static tables, sometimes at runtime rows, and a stored planet row points into frame-zone memory that is rewound. Static rows keep `const` pointers to static data. A stored runtime row never points into the frame zone: it holds an index into the static table, or is rebuilt when drawn; it owns a copy of its text and items only when neither fits, and the zone holding a copy has its budget raised by the copy's measured size (`make full-game` proves it).
6. **Parameters.** `vec_t`, `box_t` and `p8_fixed_t` are passed by value: about 170 `vec_t *` today. Where the pointer meant "not given", use a presence flag or a separate function, whichever reads better at the call site. The `entity_class_t` contract that lets `left_press`, `right_press` and `drag` keep their vectors goes away. The call-scoped bundles (`planet_keys_t`, `layout_row_keys_t`, `message_box_keys_t`, `toast_keys_t`, `open_box_frame_t`, `open_box_ctx_t`, `burst_keys_t`, `button_keys_t`, `final_score_t.happiness`, `score_panel_update_ctx_t.final_score`) hold values, not addresses: the callee copies, never keeps a pointer (e.g. `planet_t.pos = keys->pos`).
7. **Memory balance.** Measurement only, no code change. Run `make state` on the commit before this plan (`807cf47`) and on the last commit, and compare, per zone and cap: the peak used and the budget constant (`ZONE_BUDGET_*`). The step note records the table and the net gain or loss per zone, with the main causes (handle table, ids vs pointers, values vs `_own`, step 5b copies).

## 4. Review

`warn-review`, run by `plan-pilot` at the end of each phase, checks the rules of section 2 (its "Project ground rules: pointers" section). At the end of phase 1, a warning about a pointer the step-1 sorting assigns to a later step is passed on as "planned for step N", not as a finding. So are the warnings outside that sorting that phase 2 resolves: small values passed by pointer and kept addresses ("planned for step 6"), `targeting` or `emitter` named in `engine/` ("planned for step 4"), mixed static/runtime rows ("planned for step 5b"). Only links held as ids, no dead id in a list and no `forget` count then. Any other pointer-rule warning, at either phase end, that is neither "planned for step N" nor a `breaks rule N on purpose` line is a violation: it goes into section 7 as "pending user validation", and the run stops so `plan-pilot` can report it to the user. It is not a `QUESTION:` for `guideline`, and it does not go to a `plan-review` consultation. The guard from step 1 checks the struct fields at every commit. From step 2 on, `plan-pilot` runs `make sanitize-net` and `make full-game` after `prd-autopilot` returns and before writing the step note; nothing else in the chain runs them. A failure goes to `fix-bug`, like any bug, or stops the run if `fix-bug` fails. The step note records that both passed.

## 5. Done when

`tools/pointer_guard_exceptions.txt` holds only the `const` pointers to static data allowed by section 2, plus exceptions the user has validated. No game field or file-scope variable holds a list of entity pointers. `exit_queue_flush` has no `forget` left. `entity_class.h` names neither `targeting` nor `emitter`. The last `warn-review` (end of phase 2) reports no violation of its pointer rules, only `breaks rule N on purpose` lines the user has validated. All of this with the same `*.sha256`, and with `make sanitize-net` and `make full-game` passing.

## 6. Open questions

- Settled: yes. `plan-pilot` tells each step's `prd-autopilot` verifier to check the diff against the section-2 rules the guard can't see (small values by pointer, kept addresses, dead ids in lists); a new violation fails the verification.

## 7. Step notes

An exception or a `breaks rule N on purpose` is written here as "pending user validation" and only counts as validated once the user has reviewed it. When a step graduates, it adds one line here about what it found that differs from what the plan assumed: final counts, slot-table peak, GAPs added, rules broken on purpose. The next step's PRD checks its hypotheses against these lines.

### Step 1 sorting

241 pointers in 39 files and 13 `entity_list_t` in 7 files, as `tools/pointer_guard.sh` counts them. Every per-file total below was checked against `tools/pointer_guard_exceptions.txt`: each file's numbers across the five categories add up to its line. Types are the typedef names; `[]` marks an array declarator (counted once).

- **No typedef hides a pointer:** no `typedef` in `src/game/` or `entity_class.h` names a data pointer type (only function-pointer typedefs).
- **No game `pointer_list_t` holds entities:** every `pointer_list_add` in `src/game/` was read. `economy_t.invented` holds technology names; `planet_t.needs`, `research_project_t.inputs` and `planet_inspection_panel_ctx_t.received_needs` (by value, `planet_display.c`) hold `need_t`; `emitter_t.particles` and `slipway_t.transit_dots` hold `particle_t`.
- **Header:** `entity_t` holds 13 pointers: `class` plus the 12 that step 4 names. Step 4's count is right. Two of them, `pos` and `map_pos`, are pointers to pointers (`const vec_t *const *`) that point at a pointer field of the derived struct, so step 4 also removes those derived fields (listed under step 4). Step 5's examples `pos` and `pos_before_shake` are such fields: they are sorted under step 4 here.
- **Research projects are not entities:** they have a header but are never `entity_construct`ed. They live in the load zone, which `reset_world` frees. Links to them are sorted under step 3 as "another object that can die"; step 3 needs an id for them that is not a handle (their index, say).

#### Allowed by section 2 (104)

Each one was checked by reading every assignment: it only ever points to a `static const` archetype, record, table or row, a string literal or a `static const` compound literal.

- `src/engine/entity_class.h` (2): entity_t.class, entity_kind_t.record
- `src/game/background.h` (1): background_t.archetype
- `src/game/economy_score.c` (1): sum_by_development_tier_ctx_t.seq
- `src/game/game_data.c` (1): period_names_table[]
- `src/game/game_data.h` (31): int_seq_t.items, str_seq_t.items (no instance), vec_seq_t.items, kind_record_t.required_tech, kind_record_t.archetype, planet_types_entry_t.name, planet_types_entry_t.description, galaxy_menu_entry_t.layout_row, pause_menu_entry_t.layout_row, difficulty_options_entry_t.layout_row, difficulty_options_entry_t.name, background_archetype_t.tile_choices, camera_archetype_t.dpad_directions, research_project_archetype_t.row_tail, score_panel_archetype_t.countdown_rows, score_panel_archetype_t.frame_rows, wormhole_archetype_t.color_ramp, slipway_archetype_t.trail_colors, emitter_archetype_t.trail_colors, probe_archetype_t.rows, planet_archetype_t.ring_colors, .mood_rows, .made_resources, .spacer, .name, .accepted_resources, .required_tech, .class_name, .link_sprites, .synthesizable_resources, burst_construct_keys_t.trail_colors
- `src/game/gen.h` (2): weighted_picker_t.tickets[], weighted_picker_t.recent[] (into `planet_types_table`)
- `src/game/main_economy.h` (2): economy_t.archetype, economy_t.end_status
- `src/game/map_tables.h` (18): resource_code_t.key, resource_names_entry_t.key, .name, production_tables_entry_t.key, .upgraded_entry, project_records_entry_t.header, .name, .planet_codes, .production_key, .inputs, .output, .upgrade_tech, .byproduct, .next_type, .class_name, technologies_entry_t.name, .icon_key, .tooltip_rows
- `src/game/message_box.h` (2): message_box_t.archetype, toast_t.archetype
- `src/game/mouse.h` (1): mouse_t.archetype
- `src/game/particle.h` (2): emitter_t.archetype, emitter_t.trail_colors
- `src/game/planet.h` (4): planet_t.archetype, .description, .planet_type, .synthesizable_resources
- `src/game/planet_family.c` (1): planet_kinds[].class_name
- `src/game/planet_family.h` (1): burst_keys_t.construct
- `src/game/probe.h` (1): probe_t.archetype
- `src/game/research.c` (2): g_projects_by_name[].name, g_required_tech
- `src/game/research.h` (10): research_project_t.archetype, .name, .planet_codes, .production_key, .upgrade_tech, .required_tech, .class_name, .next_type, research_project_keys_t.record, research_project_keys_t.required_tech
- `src/game/score_panel.h` (4): score_panel_t.archetype, score_panel_layout_block_t.fixed_pos, .row_step, score_row_t.entry
- `src/game/scroll.h` (1): camera_t.archetype
- `src/game/slipway.h` (2): slipway_t.archetype, slipway_too_far_t.status_text
- `src/game/structure_icon.h` (4): structure_icon_t.archetype, structure_icon_t.kind, structure_icon_kind_t.required_tech, structure_icon_keys_t.kind
- `src/game/targeting.h` (1): targeting_t.archetype
- `src/game/tech_gate.h` (1): available_technologies_kept_t.items[]
- `src/game/ui.h` (1): ui_t.archetype
- `src/game/ui_button.h` (6): button_t.archetype, .name, .technology, button_keys_t.name, .technology, modal_backdrop_t.archetype
- `src/game/wormhole.h` (2): wormhole_too_far_target_t.status_text, wormhole_t.archetype

#### Step 3: links (28 pointers, 13 entity lists)

- `src/engine/entity_class.h` (1): down_t.held
- `src/game/economy_score.c` (1): g_difficulty (a difficulty button; `economy_score_difficulty_quits` swaps it for a static copy)
- `src/game/message_box.c` (1): g_shown_message_box
- `src/game/planet.h` (2): planet_t.project (research project), planet_t.targeted_by
- `src/game/planet_trade.h` (1): trade_offer_t.route (a trade route, not registered)
- `src/game/probe.c` (1): g_main_economy
- `src/game/research_display.h` (1): available_projects_kept_t.items[] (research projects)
- `src/game/score_panel.c` (1): g_main_score_panel
- `src/game/score_panel.h` (1): score_panel_t.last_box
- `src/game/slipway.h` (5): slipway_t.start, .end, .targeted_by, slipway_too_far_t.targeted_by, slipway_keys_t.start
- `src/game/targeting.h` (2): targeting_t.target, targeting_track_target_t.targeted_by
- `src/game/ui.c` (2): g_main_mouse, g_main_ui
- `src/game/ui.h` (4): ui_t.hovered, .previous_hovered, .left_held, .right_held
- `src/game/ui_button.h` (4): button_t.project_planet, button_t.project (research project), button_keys_t.project_planet, button_keys_t.project (research project)
- `src/game/wormhole.h` (1): wormhole_t.anchor
- `entity-lists src/game/planet.h` (3): planet_t.slipways, .incoming_routes, .outgoing_routes
- `entity-lists src/game/planet_family.c` (1): g_burst_outgoing_routes
- `entity-lists src/game/planet_trade.h` (1): trade_offer_t.accepted_routes
- `entity-lists src/game/trade.h` (4): trade_route_t.planets, .slipways, trade_keys_t.planets, .slipways
- `entity-lists src/game/ui.c` (1): g_shared_ephemerals
- `entity-lists src/game/ui.h` (2): ui_hovered_by_layer_t.layers[], ui_t.ephemerals
- `entity-lists src/game/wormhole.h` (1): wormhole_t.anchors

#### Step 4: header (33)

The 12 header pointers, then the derived fields the header points at (`header.pos = &self->pos`) and the `targeting`/`emitter` pointers that step 4 turns into plain fields.

- `src/engine/entity_class.h` (12): entity_t.layer, .pos, .map_pos, .hit_box, .cursor_sprite, .infraspace, .shake, .fold_anchor, .is_dead, .is_hovered, .emitter, .targeting
- `src/game/layout.h` (1): layout_row_t.pos
- `src/game/message_box.h` (1): toast_t.pos
- `src/game/mouse.h` (1): mouse_t.pos
- `src/game/planet.h` (4): planet_t.map_pos, .pos, .targeting, .emitter
- `src/game/probe.h` (4): probe_t.emitter, .targeting, .pos, .map_pos
- `src/game/scroll.h` (2): camera_t.pos_before_shake, camera_t.pos
- `src/game/slipway.h` (3): slipway_mouse_point_t.pos, .map_pos, slipway_t.targeting
- `src/game/structure_icon.h` (1): structure_icon_t.pos
- `src/game/ui_button.h` (1): button_t.pos
- `src/game/wormhole.h` (3): wormhole_t.pos, .map_pos, .targeting

#### Step 5: owned values, aliases, lists by pointer (30)

15 lists held by pointer (as the plan says) and 15 other pointers, not 25: the other `_own` cases are header-backed and sorted under step 4.

- `src/game/main_economy.h` (3): economy_t.invented (list), .production, .demand
- `src/game/menu.c` (2): pause_menu_open_box_t.fixed_pos, .row_step (point at file-scope `vec_t`s)
- `src/game/message_box.h` (1): toast_t.text (points at its own `text_buf`)
- `src/game/particle.h` (1): emitter_t.particles (list)
- `src/game/planet.h` (5): planet_t.production, .slipways (list), .needs (list), .incoming_routes (list), .outgoing_routes (list)
- `src/game/planet_trade.h` (1): trade_offer_t.accepted_routes (list)
- `src/game/research.h` (2): research_project_t.inputs (list), .layout_row
- `src/game/score_panel.h` (1): score_panel_layout_block_t.description (a frame-zone string kept in a file-scope variable)
- `src/game/scroll.h` (1): camera_t.velocity
- `src/game/slipway.h` (3): slipway_served_planets_t.ids[] (aliases of `planet_t.planet_id`), slipway_t.transit_dots (list), slipway_t.served_planets
- `src/game/structure_icon.c` (1): g_cursor (game-zone `vec_t`)
- `src/game/trade.h` (4): trade_route_t.planets, .slipways, trade_keys_t.planets, .slipways (lists)
- `src/game/ui.h` (3): ui_t.ephemerals (list), .mouse_world, .hovered_by_layer
- `src/game/ui_button.h` (1): button_t.slide_target
- `src/game/wormhole.h` (1): wormhole_t.anchors (list)

#### Unsorted (46)

**Call-scoped bundles (22).** Keys, contexts and returned structs that live on the stack for one call. Their pointers are not static: `vec_t`s, rows, zone strings. Section 2 allows parameters, but these are struct fields, and several callees keep the address (`planet_t.pos = keys->pos`, `layout_row_t.pos = keys->pos`, `toast_t.pos = keys->pos`), which is step 6's rule, and step 6 lowers no count.
- `src/engine/entity_class.h` (3): planet_keys_t.planet_id, .map_pos, .pos
- `src/game/economy_score.h` (1): final_score_t.happiness (frame-zone string)
- `src/game/layout.h` (2): layout_row_keys_t.items, .pos
- `src/game/message_box.c` (1): open_box_ctx_t.frame
- `src/game/message_box.h` (7): message_box_keys_t.items, toast_keys_t.pos, .text, open_box_frame_t.description, .fixed_pos, .row_step, .pos
- `src/game/planet_family.h` (1): burst_keys_t.pos_copy
- `src/game/score_panel_display.c` (1): score_panel_update_ctx_t.final_score
- `src/game/ui_button.h` (6): button_keys_t.layout_row_copy, .layout_row_keys, .side_tooltip_row, .tooltip_row, .pos, .slide_target

**Rows used both ways (7).** The same type points at static tables and at runtime rows in the frame zone, on the stack or inside their owner (`research_project_t.layout_row_own`, the `message_box_draw_row_t` copy). Stored rows can outlive what they point at: `planet_display.c` notes that a planet row's `items[]` point into rewound frame-zone tuples.
- `src/game/layout.h` (2): layout_row_cell_t.as.text, .as.row
- `src/game/map_tables.h` (4): data_item_t.text, .row, .ref, data_row_t.items
- `src/game/message_box.h` (1): message_box_t.items[]

**Owned variable-size objects and buffers (11).** Allocated in the owner's zone or ui_zone slot and freed with it. They are not `x = &self->x_own` and cannot become a plain value.
- `src/game/layout.h` (4): layout_row_item_t.text, layout_row_t.items, layout_row_t.placed_items, tooltip_t.rows
- `src/game/planet.h` (2): planet_t.planet_id (zone string), planet_t.layout_row
- `src/game/structure_icon.h` (2): structure_icon_t.tooltip, structure_icon_keys_t.tooltip
- `src/game/ui_button.h` (3): button_t.layout_row, .side_tooltip, .tooltip

**Other (6).**
- `src/game/gen.h` (1): weighted_picker_t.random_ctx (`void *` context of a function pointer, a game-zone `random_walk_t`)
- `src/game/planet.c` (2): g_type_picker, g_planet_vanish_roll (game-zone generators that `galaxy_populate` hands to the planet module)
- `src/game/research.c` (2): g_projects[], g_projects_by_name[].project (a game-side registry that owns the load-zone research projects; section 2 allows only the engine registry)
- `src/game/wormhole.h` (1): wormhole_t.too_far_target (non-`const` pointer to a file-scope sentinel, always the same address)

### Step 1 sorting — user decisions

The user validated (2026-10-04) the resolution of the 46 Unsorted pointers: call-scoped bundles (22) → step 6; mixed static/runtime rows (7) → new step 5b; owned variable-size buffers (11) and generator state (`g_type_picker`, `g_planet_vanish_roll`, `random_ctx`) → allowed by section 2; `g_projects[]`, `g_projects_by_name[].project` → step 3 (project ids); `wormhole_t.too_far_target` → step 5 (flag). Step 2 raises the game zone budget by the table's size. Unsorted is now empty.

### Step 1.1

- Done: `tools/pointer_guard.sh` (one awk pass over source text, ~50 ms), `tests/test_pointer_guard.c` (~0.1 s), `tools/pointer_guard_exceptions.txt`, `pointer-guard` in `make test`. `make test` wall time unchanged (5.72 s vs 5.75 s, 8 alternating pairs). `make e2e` green, no `*.sha256` changed, nothing in `src/` touched.
- Final counts: 241 pointers in 39 files, 13 `entity_list_t` in 7 files. Sorting above: 104 allowed, step 3: 28 + 13 lists, step 4: 33, step 5: 30, Unsorted: 46.
- Differs from the plan: `entity_t` has exactly `class` + the 12 step-4 pointers, but `pos` and `map_pos` are pointers to pointer fields of the derived struct, so 21 derived fields (`pos`, `pos_before_shake`…) sort under step 4, not step 5; step 5 has 15 owned values/aliases, not 25, and 15 lists by pointer as planned. Research projects are not entities (load zone, never registered): the step-3 links to them (`planet_t.project`, `button_t.project`…) need an id that is not an entity handle.
- The guard counts `entity_list_t` only: no game `pointer_list_t` holds entities (read from the code, not a `guideline` decision).
- Deviation: sub-agents had no tool to spawn `guideline`; no `guideline` decision was needed for step 1.
- Unsorted was not empty (46): the run stopped before step 2; resolved by the user decisions above.

### Step 1.2

- Done: `engine/entity_id.{h,c}` (`entity_id_t` = 16-bit index + 16-bit generation, `ENTITY_ID_NONE` = `{0,0}`; `entity_get`, `entity_id_of` (table scan), `entity_id_take`, `entity_id_release`, `entity_id_table_init/forget`, `entity_id_live/max`). The 512-slot table (6144 bytes, 64-bit) comes from `game_zone()` in `entity_registry_init()`; `GENERATION_ZONE_BYTES` and `ZONE_BUDGET_BYTES` rise by that amount, 1980 bytes left under the budget as before.
- `entity_construct` takes a slot; `entity_unregister` queues every class (GAP-5 condition kept); `exit_queue_flush` runs `forget` (classes with a `release` only, as before), frees the slot, then `release` if any. `reset_world` still queues only classes with a `release`; an abandoned entity flushed later frees no slot of the new world.
- Deviation from the PRD: a new table starts one above the highest generation ever handed out, not at epoch+1 — the epoch scheme let an id of the previous world resolve to an entity of the new one (test `ids_of_a_previous_world_never_resolve`). No separate shutdown function: `entity_id_table_forget()` serves shutdown and forget.
- Table peak: 160 of 512 (`game_end_bankrupt`, `make state` line `cap entity_table`), below the plan's 329: only constructed entities take a slot. Exit-queue peak 102 of 315.
- Guard unchanged (241 pointers, 13 entity lists). `make e2e` same shas; `make sanitize-net` (30 clean) and `make full-game` passed. `make test` not slower.
- Guideline decisions (PRD writer applied the `guideline` skill itself): the table lives in the game zone; `entity_t` unchanged in this step.

### Step 1.3

- Done in six commits: engine groundwork (`entity_id_type.h`, header `id` → O(1) `entity_id_of`/`release`, `entity_id_list_t` asserting on a dead id, `value_list_t`, `entity_geom_nearest_in_ids`, `entity_id_link`); research project ids; trade routes as values; map links; UI links; singletons. `exit_queue_flush` takes no callback; `forget`, `planet_forget_targeted_by` and `economy_score_difficulty_quits` are gone.
- Guard: 241 → 211 pointers, 13 → 0 entity lists. `make e2e` same shas; `make sanitize-net` (30 clean) and `make full-game` passed after the last chunk.
- Differs from the plan: trade routes are not entities — `trade_route_t` lost its header, route lists are value lists, route planets/slipways are id lists. Research projects die at `game_zone_reset` (rebuild) as well as `reset_world`: projects live by value in a static 42-row table (23520 bytes outside the zones) with an epoch id; the load-zone peak drops by 19040 bytes. The economy takes an id slot without being registered.
- Memory: `entity_t` +8 bytes (header id): ui/particles/load slots re-measured, `ZONE_BUDGET_BYTES` +4056 (`HEADER_ID_BYTES`), slack still 1980. Game-zone peak 117424 (was 117048 before step 3).
- Dead ids: the cart never drops a kept route and only unlinked planets die, so both planet death sites assert the planet has no slipway; attached slipways never die before `reset_world`; `reset_world` empties the outgoing ui's lists (`ui_forget_world`) since a restart click runs mid-`ui_update`.
- GAPs: GAP-26 rewritten (the halo link reads NULL after the flush); GAP-28 added (a right drag held across a restart no longer moves the new camera; no e2e reaches it).
- Deviation: chunk 3's first verification failed (dead planet ids in kept routes); the implementer proved the case unreachable in the cart and added the asserts above instead of pruning routes. Guideline decisions (applied by the PRD writer): D1 routes as values, D2 economy slot, D3 header id, D4 targeting kind + id and slipway `end_is_point`, D5 project table with epoch, D6 difficulty copied, D7 placeholders as a state, D8 links compared only by resolving.

### Phase 1 review

`warn-review` at the end of phase 1 (55 `src/` files):
- **Violation:** `src/engine/bootstrap.c` names research projects (`#include "game/research.h"`, `research_expire_project_ids()` in `reset_world()`), a game concept in `engine/`. It extends a reverse dependency that predates the plan (`bootstrap.c` already includes `game/mouse.h`, `game/scroll.h`, `game/ui.h`).
- **Breaks rule 15 on purpose, validated by the user:** `planet.c` and `planet_display.c` never remove a dying planet's id from id lists; they assert the dying planet has no slipway (the cart never drops kept routes, and only unlinked planets die).
- Every other pointer warning is planned for steps 4 to 6.
- Smells, for a later `plan-review`: `engine/` includes `game/` headers (`bootstrap.c`, `entity_registry.c`); `entity_class.h` declares `slipway_class`/`probe_class`; targeting is implemented in `probe.c`; hand-measured zone sizes inline (`HEADER_ID_BYTES`, `BUTTON_SIZE`…); the economy's id slot is taken outside `entity_construct` and never freed by the exit queue; `slipway_create` silently drops a start key that resolves to NULL.
- User decisions (2026-10-05): the violation is fixed (`cart_init` and `cart_start_game` expire project ids after `reset_world()`; `engine/bootstrap.c` no longer names research); the rule-15 break is validated. Smells left for later.

### Step 2.4

- Done in five commits: archetype values (layer, hit_box, cursor_sprite, infraspace, fold_anchor) copied into the header with presence bits in `entity_t.has`; shake, is_dead, is_hovered as header values; position readers through `entity_pos`/`entity_map_pos`; `pos`/`map_pos` as `vec_t` values (`ENTITY_HAS_POS`/`MAP_POS`, `entity_set_pos`…); `targeting`/`emitter` moved to plain fields of their carriers, reached through `targeting_of`/`emitter_of`. `src/engine/` names neither.
- Guard: 211 → 178 pointers, 0 entity lists. `make e2e` same shas; `make sanitize-net` (30 clean) and `make full-game` passed after the last chunk.
- Differs from the plan: a toast keeps its source planet's id and re-reads its position at each draw until its first update — a plain copy moved one toast 1 px in `game_end_bankrupt` (in the cart a toast can be drawn several frames before its first update while the planet bobs). The particle host assert keeps its old meaning ("the class carries a position", `emitter_host_carries_pos`: probe or planet lineage), not the position bit, since a probe dragged without a mouse position has no position but still hosts an emitter. Only planets get presence bits for targeting/emitter (probe, slipway, wormhole always carry them). `ui.c` still passes `&mouse->header.pos` to press/drag/hover for the call only (step 6).
- Memory: game-zone peak 117384 → 104896; button, toast, message box, burst, row slots all smaller. No budget changed.
- Guideline decisions (PRD writer): toast follows its source by id; every header value gets a presence bit; only `planet_t` gets bits for targeting/emitter.

### Step 2.5

- Done in six commits: small owned values and the too-far flag; economy, planet production (`planet_production()` falls back to the shared one) and the project's layout row; ui ephemerals/mouse_world/hovered_by_layer (a serial replaces the address comparison, `g_shared_ephemerals` gone); lists in slots and arenas (particles, transit dots, served planets, anchors, planet slipways); planet needs (`PLANET_HAS_NEEDS` keeps GAP-15's absent-vs-empty) and project inputs; trade lists (offer copy-in and take-back, slipgate copy-back, shared burst list gone).
- Guard: 178 → 147 pointers, 0 entity lists. `make e2e` same shas; `make sanitize-net` (30 clean) and `make full-game` passed after the last chunk.
- Differs from the PRD: a slipway's served set copies ids into 8-byte entries, not 4 — structure ids are "s" plus an unbounded counter; an assert stops a longer id. GAP-15 lost its no-served-set bullet. Route id lists share their items between value copies (nothing appends after construction, trap comment on `trade_route_t`).
- Memory: no zone constant or budget changed. Game-zone peak 104672 (104896 after step 2.4), load 2352. `planet_t` grew (392 → 440) with embedded lists but the burst slot still fits (536 of 648). The static project table grew by 672 bytes outside the zones.
- Guideline decisions (PRD writer): shared production through an accessor; served ids copied; ephemerals replaced detected by a serial; offer copy-in/take-back; score-panel description deleted (unread); no budget raised.

### Step 2.5b

- Done in four commits: a stored layout row keeps only its placed items (texts copied into its zone), built from call-scoped keys — the planet row no longer points into the rewound frame zone; tuple cells hold their scalars by value (`as.tuple`), the project icon row is built on demand (`research_project_t` loses its stored row); score rows are rebuilt from `score_rows_table` at each update (`score_row_t`, `g_score_rows` gone); the message box holds its articles by value, `data_item_t.ref` gone, `is_ref` → `is_nil`.
- Guard: 147 → 141 pointers, 0 entity lists. `make e2e` same shas; `make sanitize-net` (30 clean) and `make full-game` passed after the last chunk.
- Differs from the PRD: the year box shares the banner's static label row through a function read at each draw (`message_box_add_shared_row`) — copying it diverged `trade_rich_mine` and `tech_slipgates_relay`, because in the cart a year box still drawn when the next year opens shows the new year. Four pointers stay counted and point only at static data in stored rows (`data_item_t.text/row`, `data_row_t.items`); `layout_row_cell_t.as.text` lives only in call-scoped cells (step 6).
- Memory: no budget raised. Frame-zone peak 16552 → 9456, message box 4232 → 3360, row 752 → 600, button 3288 → 2984; static project table −13776 bytes.
- Bug fixed on the way (`fix:` 33df43a): a placed layout item's unused fields kept stale zone bytes; each item is now zeroed first.
- Guideline decisions (PRD writer): call-scoped frame-zone rows stay; score readouts and year banner stay file-scope rows; the message box copies its articles; a tuple holds no text (assert); bars' draw function in their static rows; `layout_row_copy` removed.

### Step 2.6

- Done in eight commits: draw positions and `box_contains` by value; `mouse_pos_t` (world and screen points with presence bits) for `hit_test`/`left_press`/`right_press`/`drag` — the class contract that let methods keep their vectors is gone; hud alignment and `need_new` by value; planet keys moved to `game/` and held by value, `planet_t.planet_id` an 8-byte array behind `PLANET_HAS_ID`; layout row keys embed up to 16 cells and their position; button keys by value; toast and open-box keys by value (description capped at 17 chars); final happiness a number. No input `vec_t *`/`box_t *`/`p8_fixed_t *` parameter is left (remaining: out-parameters, arrays, const static returns, `box_make`).
- Guard: 141 → 119 pointers, 0 entity lists. `make e2e` same shas; `make sanitize-net` (30 clean) and `make full-game` passed after the last chunk.
- Memory: no budget raised. Frame-zone peak 9456 → 13840 (keys embed their cells; peak 14464 after chunk 5), game zone 103856 → 101688.
- `tools/pointer_guard_exceptions.txt` regrouped: pending, static data plus user-validated owned buffers/generator state, static only.
- **Pending user validation (4 pointers):** `layout.h` `layout_row_cell_t.as.text` (call-scoped cells; `layout_row_new` copies the text); `map_tables.h` `data_item_t.text`, `data_item_t.row`, `data_row_t.items` (static in stored rows, but call-scoped rows aim them at frame-zone or stack memory).
- Guideline decisions (PRD writer): D3 out-parameters and const static returns stay; D5 `as.text` stays, pending; D11 budget only by measured growth (not needed); D12 screen point is the raw mouse header position.

### Step 2.7

Measurement only (`make state` and `size tmp/replay` on `807cf47`, the commit before the plan, and on `3aa64e4`; no PRD, no code change).

| Zone | Peak before | Peak after | Constant before | Constant after |
|---|---|---|---|---|
| game (generation) | 103344 | 101688 | 462520 | 468664 (+6144, handle table) |
| load | 21936 | 2352 | 24680 | 24984 (+304, header id) |
| frame | 16784 | 13840 | 20256 | 20256 |
| ui | 82090 | 84114 | 203961 | 207681 (+3720, header id) |
| particles | 24485 | 24485 | 73035 | 73067 (+32, header id) |
| **budget used** | | | 784452 / 786432 | 794652 / 796632 |

- Net: the zones' reserved size grew by 10200 bytes (6144 handle table, 4056 header id); slack under the budget unchanged (1980). Peaks fell in game (−1656), load (−19584, projects left the load zone) and frame (−2944), rose in ui (+2024, ephemerals and ui state).
- Slots: button 3768 → 2984, message box 4488 → 3360, toast 296 → 232, row 784 → 600, burst 568 → 536, modal backdrop 240 → 208.
- Outside the zones: the research projects now live in a static 42-row table (9072 bytes) instead of the load zone; `tmp/replay` bss 799408 → 813904 (+14496: the +10200 zone reservation and the project table, less removed statics), data 48992 → 47144, text 348005 → 370174 (+22169 of code).
- Verdict: the plan costs about 10 KB of reserved memory (the handle table and the header id) and gains about 23 KB of peak use across load and frame; every zone keeps more headroom than before except ui (+2 KB peak, 123 KB free).
