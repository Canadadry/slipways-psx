# Plan 13 — Fewer pointers

Draft: run it through `plan-converge` before executing anything. Comes out of a grill-me session.

## 1. Goal

Game state holds no pointer that can dangle or that needs a paragraph to explain. Entities link to each other through checked ids, a component an entity owns is a value, and a small value travels by value.

Two goals, and only these two, justify every rule below:

- **Safety.** A link to something that died fails loudly (NULL, an `assert`), never reads freed memory silently, and doesn't depend on someone remembering to clear it.
- **Readability.** One name per piece of data, and a field whose meaning its type states.

A rule that works against one of these goals in a case nobody anticipated may be broken on purpose. The commit message then says why. `warn-review` reports it as `breaks rule N on purpose` and doesn't count it as a violation.

## 2. Rules

- **Pointers that stay:** `self` and large structs as parameters (valid only during the call), `const` pointers to static data that is never freed (archetypes, tables, strings), function pointers, runtime handles (`allocator_t`, `p8_graphics_t`, `p8_vram_t`), and the engine registry's internal lists (live list, per tag, per message). Every other pointer goes.
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

1. **Pointer guard.** `tools/pointer_guard.sh` counts every data pointer, `const` included, in the struct fields and the file-scope variables of `src/game/` (headers and `.c` files) and of `src/engine/entity_class.h`, excluding function pointers and the runtime handles from section 2. `tools/pointer_guard_exceptions.txt` lists one `<file> <count>` line per file, matching today's counts; the `const` pointers to static data that stay end up as its last lines. It also counts, per file, the game fields and file-scope variables that hold a list of entity pointers (`entity_list_t`, or a `pointer_list_t` of entities), by pointer or by value: the list type hides those pointers from the first count. Strict equality, run in `make test`, modelled on `tools/alloc_guard.sh`. Step 1 also sorts every counted pointer: allowed by section 2, or removed by step 3 (a link to an entity or to another object that can die), step 4 (the header) or step 5 (an owned component, a `const` alias into memory that can be freed, a list). The sorting is written in section 7. A pointer that fits none of these stops the run before step 2, for the user to decide. A unit test proves it fails on a raised count, a new file and each kind of pointer it counts, like `tests/test_alloc_guard.c`. From here on, no step raises a count; steps 3 to 5 lower them, and steps 2 and 6 leave them as they are.
2. **Handle table.** `engine/` gets `entity_id_t` = `uint16_t index` + `uint16_t generation`, with `{0,0}` meaning no link.
   - A fixed table of 512 slots. The measured peak is 329 live entities (`cart_game_registry_reserve`), and the table counts against the zone budget. `p8_fatal` when it is full.
   - `entity_construct()` takes a slot. The slot is freed, and its generation incremented, at `exit_queue_flush`, for **every** class: `entity_unregister` therefore queues every entity, with or without a `release`. During the frame, `entity_get(id)` still returns the dead entity; after the flush, it returns NULL.
   - `reset_world()` clears the table.
   - Nobody uses the table yet. It is covered by unit tests.
3. **Links → ids.** About 15 fields (`slipway.start/end/targeted_by`, `planet.targeted_by`, `targeting.target`, `wormhole.anchor`, `ui.hovered/previous_hovered/left_held`, `project_planet`…) and about 10 game lists (`planet.slipways/incoming_routes/outgoing_routes`, `trade.planets/slipways`, `wormhole.anchors`, `accepted_routes`, `ui.ephemerals`…) become ids.
   - A game list of ids holds no dead id: it is removed at the entity's death, where the cart calls `del()`. A walk that meets a dead id `assert`s.
   - `down_t.held` becomes an id.
   - The `forget` callback of `exit_queue_flush` goes away.

### Phase 2 — Readability

4. **Header as values.** The 12 pointers of `entity_t`, besides `class`, (`layer`, `pos`, `map_pos`, `hit_box`, `cursor_sprite`, `infraspace`, `shake`, `fold_anchor`, `is_dead`, `is_hovered`, `emitter`, `targeting`) become values, with their presence in one `uint16_t has`.
   - An archetype's values are copied at construction.
   - The copies in derived structs (`planet->pos`, `probe->pos`, `planet->shake`, `planet->is_dead`…) go away: the header is the only owner.
   - `targeting_track` writes the position instead of re-pointing it.
   - `targeting` and `emitter` leave `entity_t`. Each struct that has one keeps it as a plain field (`probe_t.targeting`), with presence in its `has` bit. The game reaches it through `targeting_of(e)` / `emitter_of(e)`, which switch on the class and whose result nobody keeps. `engine/` no longer names them.
5. **`_own` and lists by value in `game/`.** The 25 `x = &self->x_own` cases (`production`, `demand`, `velocity`, `pos_before_shake`, `served_planets`, `layout_row`, `mouse_world`, `hovered_by_layer`, `pos`…) become a value + a presence bit. The 15 lists held by pointer become lists held by value. A list the cart distinguishes as "absent" from "empty" gets a presence bit.
6. **Parameters.** `vec_t`, `box_t` and `p8_fixed_t` are passed by value: about 170 `vec_t *` today. Where the pointer meant "not given", use a presence flag or a separate function, whichever reads better at the call site. The `entity_class_t` contract that lets `left_press`, `right_press` and `drag` keep their vectors goes away.

## 4. Review

`warn-review`, run by `plan-pilot` at the end of each phase, checks the rules of section 2 (its "Project ground rules: pointers" section). At the end of phase 1, a warning about a pointer the step-1 sorting assigns to a later step is passed on as "planned for step N", not as a finding: only links held as ids, no dead id in a list and no `forget` count then. The guard from step 1 checks the struct fields at every commit. From step 2 on, every step's PRD lists `make sanitize-net` and `make full-game` passing among its acceptance criteria, checked before the step graduates: nothing else in the `plan-pilot` chain runs them.

## 5. Done when

`tools/pointer_guard_exceptions.txt` holds only the `const` pointers to static data allowed by section 2, plus exceptions the user has validated. No game field or file-scope variable holds a list of entity pointers. `exit_queue_flush` has no `forget` left. `entity_class.h` names neither `targeting` nor `emitter`. The last `warn-review` (end of phase 2) reports no violation of its pointer rules, only `breaks rule N on purpose` lines the user has validated. All of this with the same `*.sha256`, and with `make sanitize-net` and `make full-game` passing.

## 6. Open questions

- Should the step-4 verifier of `prd-autopilot` check the rules the guard can't see (parameters, kept addresses, dead ids in lists) at every step? Without it, they are only checked at the end of a phase.

## 7. Step notes

An exception or a `breaks rule N on purpose` is written here as "pending user validation" and only counts as validated once the user has reviewed it. When a step graduates, it adds one line here about what it found that differs from what the plan assumed: final counts, slot-table peak, GAPs added, rules broken on purpose. The next step's PRD checks its hypotheses against these lines.
