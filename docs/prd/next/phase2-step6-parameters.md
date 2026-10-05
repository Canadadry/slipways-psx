---
title: "Parameters: small values by value, bundles hold values"
description: "Small values (`vec_t`, `box_t`, `p8_fixed_t`) still travel by pointer, with NULL meaning \"not given\", and the call-scoped key bundles hold addresses that callees could keep."
status: needs-triage
---

## Problem Statement

Plan 13 (`docs/plan-13.md`), phase 2 (Readability), step 6.

About 150 parameters and locals in `src/` still pass a `vec_t`, `box_t` or `p8_fixed_t` by pointer: about 120 `vec_t *`, 17 `p8_fixed_t *`, a few `box_t *`. The plan counted about 170 `vec_t *` before steps 4 and 5 removed some. A reader cannot tell from the signature whether the pointer means "maybe absent", "write here" or "I keep this":

- **Entity methods.** `entity_class_t.draw` / `draw_overlay` get `const vec_t *screen_pos`, NULL when the entity has no position. `hit_test` gets two `const vec_t *`. `left_press`, `right_press` and `drag` get two non-`const` `vec_t *`, so the class contract allows a callee to keep or write the vectors. Today every callee copies them, but only by convention. `ui.c` passes `&ui->mouse_world` and `&mouse->header.pos`, with the comment "Valid only for the call it is passed to". NULL means "no world mouse" or "no main mouse". `targeting_track`, `slipway_drag`, `background_right_press` / `background_drag` read that NULL as a difference: they clear the position or store `has_drag_mouse = 0`.
- **Engine helpers.** `entity_geom_contains_cursor(e, const vec_t *mp, const vec_t *rp)` and `box_contains(const box_t *, vec_t)`.
- **Scalars.** `hud_print_outlined(..., const p8_fixed_t *a)`, where NULL means "no alignment", and `need_new(zone, const p8_fixed_t *qe)`, where NULL means "no resource".
- **Call-scoped bundles.** Keys and contexts that live on the stack for one call hold addresses. The step-1 sorting (plan section 7, "Unsorted → Call-scoped bundles", moved to this step by the user) lists 22 pointers. Step 5b removes one of them (`button_keys_t.layout_row_copy`, H-6-2), which leaves 21:
  - `planet_keys_t.planet_id`, `.map_pos`, `.pos` (`entity_class.h`). `galaxy_populate` allocates two `vec_t`s per planet in the game zone only so the planet can point at them. The structure icon opens its grab arena before it builds the id string, because "the planet keeps that pointer".
  - `final_score_t.happiness`, a frame-zone string (`economy_score.h`).
  - `layout_row_keys_t.items`, `.pos` (`layout.h`).
  - `open_box_ctx_t.frame` (`message_box.c`).
  - `message_box_keys_t.items`, `toast_keys_t.pos`, `.text`, `open_box_frame_t.description`, `.fixed_pos`, `.row_step`, `.pos` (`message_box.h`).
  - `burst_keys_t.pos_copy` (`planet_family.h`).
  - `score_panel_update_ctx_t.final_score` (`score_panel_display.c`).
  - `button_keys_t.layout_row_keys`, `.side_tooltip_row`, `.tooltip_row`, `.pos`, `.slide_target` (`ui_button.h`).

Plan rules: "Small values (`vec_t`, `box_t`, `p8_fixed_t`) are passed by value." "No callee keeps an address it received." Step 6: "Where the pointer meant 'not given', use a presence flag or a separate function, whichever reads better at the call site. The `entity_class_t` contract that lets `left_press`, `right_press` and `drag` keep their vectors goes away. The call-scoped bundles … hold values, not addresses: the callee copies, never keeps a pointer."

## Solution

- **Entity methods take values.** `draw` and `draw_overlay` take `vec_t screen_pos`, which is zero when the entity has no position. `hit_test`, `left_press`, `right_press` and `drag` take one engine value, `mouse_pos_t`: the world point, the screen point, and a presence bit for each. No method receives an address it could keep. `ui.c` builds one `mouse_pos_t` per dispatch.
- **Helpers take values.** `entity_geom_contains_cursor(e, mouse_pos_t)`, `box_contains(box_t, vec_t)`, `hud_print_outlined(..., p8_fixed_t a)` and `need_new(zone, resource_t)`.
- **Bundles hold values.** Every keys or context struct that the plan names holds its positions as `vec_t` plus a presence flag, its short texts as bounded `char` arrays, its cells as an embedded array, its nested keys and rows by value, and its frame-zone string as the number it was printed from. The pointers that only existed so a callee could find caller memory go: the planet's game-zone position boxes, `burst_keys_t.pos_copy`, `message_box_keys_t.items` and `open_box_ctx_t.frame`.

Every `*.sha256` in `tests/e2e/` stays identical. The pointer guard goes down by exactly 22, from 141 to 119 if step 5b lands as planned (H-6-1). That is the 21 bundle pointers, plus `planet_t.planet_id`, which becomes a value because the keys now carry the id by value (decision D6). The entity-list count stays 0. `make test` and `make e2e` are no slower. `make sanitize-net` and `make full-game` pass. No game concept enters `src/engine/`, and one (`planet_keys_t`) leaves it.

## User Stories

1. As a port developer, I want every `draw` and `draw_overlay` method to receive the screen position as a `vec_t`, so that no draw method tests a pointer for NULL.
2. As a port developer, I want the draw dispatch to pass the zero vector for an entity without a position, so that the three draw methods that read it (planet, ascension, help screen) draw exactly as before.
3. As a port developer, I want `hit_test`, `left_press`, `right_press` and `drag` to take a `mouse_pos_t` by value, so that the class contract no longer lets a callee keep or write the caller's vectors.
4. As a port developer, I want `mouse_pos_t` to state, with presence bits, whether the world point and the screen point are given, so that "no world mouse" stays distinct from "the mouse is at zero".
5. As a port developer, I want `targeting_track` to keep clearing the host's position when the world point is absent, so that a probe dragged without a world mouse behaves as before.
6. As a port developer, I want `background_right_press` / `background_drag` to keep storing `has_drag_mouse = 0` when the screen point is absent, so that the camera velocity is unchanged.
7. As a port developer, I want `slipway_drag` to keep clearing the point's position when the world point is absent, so that the slipway end is unchanged.
8. As a port developer, I want `ui.c` to build its `mouse_pos_t` from the ui's world mouse and the main mouse's header position, so that the comment "Valid only for the call it is passed to" and `ui_mouse_world_or_null` go.
9. As a port developer, I want `ui_send_to_hovered` to take the `mouse_pos_t` by value, so that an accept-drop dispatch passes an empty one instead of two NULLs.
10. As a port developer, I want `entity_geom_contains_cursor` to take the `mouse_pos_t` and pick the screen or the world point by layer, as it does today, so that hovering is unchanged.
11. As a port developer, I want `box_contains` to take its box by value, so that the engine's geometry has no small value behind a pointer.
12. As a port developer, I want `hud_print_outlined` to take its alignment by value, with `hud_print_outlined_default` passing alignment 0, so that a NULL alignment no longer exists. An alignment of 0 shifts by 0, which is what NULL did.
13. As a port developer, I want `need_new` to take a `resource_t`, so that "no resource" is `RESOURCE_NIL`, the type the code already uses for it.
14. As a port developer, I want `planet_keys_t` to carry its map position and position as `vec_t` values with presence flags, so that `galaxy_populate` no longer allocates two `vec_t`s per planet in the game zone.
15. As a port developer, I want `planet_keys_t` to carry the planet id as a bounded `char` array, so that the structure icon no longer has to open its grab arena before building the id string.
16. As a port developer, I want `planet_t` to hold its id as a bounded `char` array with a presence bit, so that a planet's id is not a pointer into whichever zone its creator used.
17. As a port developer, I want the planet id size to be the same constant as the slipway served-set entry, so that one name states how long an id can be.
18. As a port developer, I want an `assert` with a message when an id does not fit, so that nobody widens it silently.
19. As a port developer, I want `planet_keys_t` declared in `game/`, with only a forward declaration left in `entity_class.h`, so that the engine names one game struct fewer.
20. As a port developer, I want `burst_keys_t.pos_copy` removed and the protostar to give its burst a position through the planet keys, so that one position key does one job.
21. As a port developer, I want `layout_row_keys_t` to embed its cells in a fixed array whose capacity is measured, so that keys can be copied, returned and nested without pointing at caller memory.
22. As a port developer, I want `layout_row_keys_t` to carry its position as a `vec_t` with a presence flag, so that `layout_row_new` copies a value.
23. As a port developer, I want an `assert` with a message when a row has more cells than the keys hold, so that the capacity is raised only after measuring.
24. As a port developer, I want `layout_row_keys_of_row` to return keys by value without a caller cell buffer, so that the menus, the help screen and the planet panels drop their side arrays of cells.
25. As a port developer, I want the row builders that step 5b leaves (planet resource row, project buy row, project icon row) to return keys by value, so that their cells no longer live in the frame zone.
26. As a port developer, I want `button_keys_t` to hold its row keys and side-tooltip keys by value, with a presence flag for the side tooltip, so that GAP-23's "absent / false / row" cases read as they do today.
27. As a port developer, I want `button_keys_t` to hold its tooltip row as a `data_row_t` value with a presence flag, so that the button keys carry no address of the caller's row.
28. As a port developer, I want `button_keys_t` to hold `pos` and `slide_target` as `vec_t` values with presence flags, so that `open_box_step` no longer points the keys at its own locals.
29. As a port developer, I want `toast_keys_t` to carry its text in a `char` array of `TOAST_TEXT_CAP + 1` and its position as a value with a presence flag, so that the toast copies values, not a caller's string.
30. As a port developer, I want `open_box_frame_t` to hold its description in a bounded `char` array and its fixed position, row step and position as values with presence flags, so that the menus, the help screen and the planet panels pass values.
31. As a port developer, I want `open_box_step` to receive the frame as a parameter instead of through `open_box_ctx_t.frame`, so that the context holds only the positions it advances.
32. As a port developer, I want `message_box_keys_t.items` removed, and each caller to add its rows with `message_box_add_row` after opening the box, so that the keys hold no array of addresses.
33. As a port developer, I want `final_score_t.happiness` to be the `p8_fixed_t` it was printed from, and the score row to print it with its `%` when it builds its text, so that a returned struct holds no frame-zone string.
34. As a port developer, I want `score_panel_update_ctx_t` to hold the `final_score_t` by value, so that the context points at no local.
35. As a reviewer, I want `tools/pointer_guard_exceptions.txt` lowered in each chunk by exactly that chunk's removals, so that the ratchet proves what each commit did.
36. As a reviewer, I want a file whose count reaches 0 to lose its line, and a file whose remaining pointers are all allowed by section 2 to move under the "all stay" heading, so that the file shows what is left for the user to validate.
37. As a reviewer, I want no new pointer field or file-scope pointer in `src/game/` or `entity_class.h`, so that the guard only goes down.
38. As a reviewer, I want an audit of every `vec_t *`, `box_t *` and `p8_fixed_t *` left in `src/`, each with its category (out-parameter, array, returned pointer to static data, allocation), so that `warn-review` can check that no input parameter is left.
39. As a reviewer, I want every `*.sha256` in `tests/e2e/` unchanged after every chunk, so that each chunk is proven not to change the game.
40. As a reviewer, I want `make sanitize-net` and `make full-game` to pass after the step, so that no value copy overflows a slot or a zone.
41. As a port developer, I want `make test` and `make e2e` no slower, so that the fast loop stays fast.
42. As a port developer, I want unit tests that called methods with `vec_ref(...)` or NULL to build a `mouse_pos_t` instead, so that the tests use the contract the game uses.
43. As a future step-7 implementer, I want the step note to record the game-zone, grab-arena and frame-zone peaks that moved, so that the memory balance can attribute them to step 6.
44. As a user validating exceptions, I want the step note to list `layout_row_cell_t.as.text` as "pending user validation", with its reason, so that I decide on it instead of finding it in a review.

## Hypotheses on step 5b

Step 5b (`docs/prd/next/phase2-step5b-rows-static-and-runtime.md`, committed at `HEAD` `274946d`) is not implemented yet and lands before this step. Everything below rests on that PRD, not on code. Before starting, the implementer checks each hypothesis against the committed tree and writes a verdict next to it in the step note. If one fails, the decisions keep their intent and take the real names and numbers. The guard deltas of this step are fixed (−22). Only the starting values move.

- **H-6-1 — guard after step 5b.** 141 pointers, 0 entity lists. Per-file values for the files this step touches: `entity_class.h` 5, `economy_score.h` 1, `layout.h` 6, `message_box.c` 1, `message_box.h` 9, `planet.h` 6, `planet_family.h` 2, `score_panel_display.c` 1, `ui_button.h` 14. All 22 pointers this step removes are still declared.
- **H-6-2 — layout rows keep no cells.** `button_keys_t.layout_row_copy`, `layout_row_t.items` / `items_count` and `layout_row_add` are gone. `layout_row_copy` is gone, or kept only if a caller remains. The planet resource row, the project buy row and the project icon row are keys builders that return `layout_row_keys_t` whose cells live in the frame zone. The resource-row builder takes its output as `int has_output, p8_fixed_t output`. `planet_projects_panel_keys` passes the buy row through `button_keys_t.layout_row_keys`.
- **H-6-3 — cells.** `layout_row_cell_t.as.row` is now `as.tuple`, a `layout_row_tuple_t` value: a count and four scalar kind/`p8_fixed_t` pairs. `as.text` remains a `const char *`. A cell is about 40 to 48 bytes. `layout_row_keys_t` keeps `items`, `count`, color, background, score and `vec_t *pos`, unchanged by 5b.
- **H-6-4 — message box articles.** `message_box_t` holds `message_box_article_t articles[]` by value. `message_box_keys_t.items` (call-scoped `const data_row_t *const *`) remains, and `message_box_new` copies each row into an article. `message_box_add_row(box, row)` and `message_box_add_layout_row(box, keys, pos, has_pos, align)` exist, with `pos` by value. The score panel opens its box with no rows and adds them. The year banner still passes its rows through `keys.items`.
- **H-6-5 — score panel.** `score_row_t`, `g_score_rows` and `score_panel_rows()` are gone, and `score_panel_update` rebuilds the scored rows from `score_rows_table`. `score_panel_update_ctx_t` (or whatever carries the final score into the row builder) still holds `const final_score_t *final_score`, which 5b's guard deltas imply, since they do not touch `score_panel_display.c`. `final_score_t.happiness` is still a frame-zone `char *`.
- **H-6-6 — `ref` gone.** `data_item_t.ref` and the `DRAW_FUNCTION_DRAW_ROW` dispatch in `draw_functions` are gone, and with them `draw_functions.c`'s `const vec_t *at`. `is_ref` is `is_nil`.
- **H-6-7 — bundles untouched by 5b.** `planet_keys_t`, `burst_keys_t`, `toast_keys_t`, `open_box_frame_t`, `open_box_ctx_t`, `final_score_t`, `button_keys_t.pos` / `.slide_target` / `.side_tooltip_row` / `.tooltip_row`, and every entity method signature are as at `HEAD` (5b's Out of Scope). The transient tooltip rows (`research_project_tooltip`, `invented_lines`) still reach the button as `const data_row_t *`.
- **H-6-8 — memory constants.** `FRAME_ZONE_BYTES` (20256), `ROW_SIZE` (1184) and `BUTTON_SIZE` (6056) are as at `HEAD`. `MESSAGE_BOX_SIZE` may have been raised by 5b's copy rule. This step starts from whatever values 5b's step note records.
- **H-6-9 — green baseline.** After 5b: same shas, `make sanitize-net` and `make full-game` pass, `make test-all` is green, and plan section 7 has a "Step 2.5b" note.

## Implementation Decisions

### General rules for this step

- **What changes.** Every *input* parameter of type `vec_t *`, `box_t *` or `p8_fixed_t *` in `src/` becomes a value. Every pointer field of the ten bundles the plan names becomes a value or goes.
- **D3 (guideline): what stays.** Out-parameters (`resource_map_get`'s `out`, `structure_icon_id_counter`, `game_data_fixed_seq_at`'s `out`, the layout measure's `out_w` / `out_spacing`, `emitter_host_layer_pos`'s `ra`), arrays (`game_data_fixed_seq_at`'s `seq`, `vec_seq_t.items`), returned `const` pointers to static data (`ui_panel_box`, `structure_icon_rest_cursor`), the static `score_panel_layout_block_t.fixed_pos` / `.row_step`, and `box_make`'s returned allocation all stay. WHY: the plan's rule is about values *passed in*. An out-parameter is a return channel the callee writes and never keeps, and section 2 allows `const` pointers to static data. The smaller change works.
- **Presence.** Where NULL meant "not given", the value gets a presence flag in the style of the struct that holds it. Bundles that already use `int has_x` (`button_keys_t`, `open_box_frame_t`, `layout_row_keys_t`) add `int has_x`. The engine's new `mouse_pos_t` uses a `uint16_t has` with named bits, as `entity_t` does. Where a NULL and a zero value behave the same for every reader (the draw position, the alignment, an empty row's keys), no flag is added. Each such case is listed below with the reason.
- **No callee keeps an address.** A function that receives keys copies what it needs, and a test proves it for each bundle (see Testing Decisions).
- **No engine game concept.** The engine gains `mouse_pos_t`, a generic input type next to the class methods, which already name `mouse_world` / `mouse_screen`. It loses `planet_keys_t`'s definition. Nothing else in `src/engine/` names a game type.
- **Zone and slot sizes.** Positions and ids leave the game zone and the grab arena, so their peaks drop. Constants are not lowered (step 7 measures). Embedded cells make keys larger, so the frame-zone peak can rise where keys are allocated in the frame zone (the projects panel's button array). The implementer runs `make state` after chunks 5 and 6. If `make full-game` fails on a full frame zone, `FRAME_ZONE_BYTES` rises by exactly the measured growth (its static assert must still hold), and the step note records it. D11 (guideline): raise only by measured growth. WHY: `make full-game` is the plan's proof for a zone, and a budget is never raised by a guess.

### Entity methods (engine)

- **D2 (guideline): draw takes a value.** `draw` and `draw_overlay` take `vec_t screen_pos`. The dispatch passes `entity_pos(qc)`, which is zero when the entity has no position. WHY: every draw method either ignores the parameter or reads NULL as zero (help screen, planet, ascension; checked at `HEAD`). A flag would add nothing.
- **D1 (guideline): one `mouse_pos_t` for the mouse methods.** `typedef struct { vec_t world; vec_t screen; uint16_t has; } mouse_pos_t;` with `MOUSE_POS_HAS_WORLD` / `MOUSE_POS_HAS_SCREEN`, plus inline readers that return zero when a point is absent and testers for each bit. `hit_test`, `left_press`, `right_press` and `drag` take it by value. `entity_registry_hit_test_always` follows. WHY: the methods pass the pair around together, and three callees distinguish absent from zero (`targeting_track`, `slipway_drag`, the background). One value with two bits reads better at each call site than four parameters or doubled functions.
- **D12 (guideline): the screen point is "given" exactly when the main mouse exists,** and its value is the mouse header's `pos` field, as `ui.c` passes today. It is not `entity_pos()`, because that function zeroes the value when the position bit is absent. WHY: same shas. A trap comment (at most 40 words) says this matches the old `&mouse->header.pos`.
- `ui_send_to_hovered(self, message, mouse_pos_t, held, …)`. Accept-drop passes an empty `mouse_pos_t`.
- `ui_mouse_world_or_null` and its comment go. `ui_update_hover` / `ui_handle_button` build the value once.
- Game callees take the value or the one point they use, with its flag: `targeting_track(self, mouse_pos_t)`, `wormhole_drag(self, mouse_pos_t)`, `slipway_drag(self, mouse_pos_t)`, `background_right_press(self, mouse_pos_t)`, `background_drag(self, mouse_pos_t)`. Each reads the presence bit where it tested NULL.
- `entity_geom_contains_cursor(e, mouse_pos_t)` picks the screen point for layer ≥ 10 and the world point otherwise, reading an absent point as zero, as today.
- `box_contains(box_t, vec_t)`. Its callers pass the header's `hit_box` value.

### Scalar parameters

- `hud_print_outlined(..., p8_fixed_t a)`. `hud_print_outlined_default` passes 0. WHY no flag: the shift is `a * 4 * len`, so 0 shifts by 0, which is exactly what NULL did.
- `need_new(zone, resource_t)`. Callers pass `{RESOURCE_FIXED, x}` or `{RESOURCE_NIL}`.
- If step 5b leaves the local `const p8_fixed_t *qe` alias in `planet_refresh`, it becomes a value plus a flag.

### Planet keys

- **D6 (guideline): the id travels by value.** `planet_keys_t` becomes `{ char planet_id[PLANET_ID_SIZE]; int has_planet_id; vec_t map_pos; int has_map_pos; vec_t pos; int has_pos; }`. The planet copies the id into `planet_t.planet_id[PLANET_ID_SIZE]` and sets a new `PLANET_HAS_ID` bit. `economy_tiers` tests the bit where it tested NULL. `PLANET_ID_SIZE` (8) replaces `SLIPWAY_PLANET_ID_SIZE`, so the served set uses the same name. Copying asserts that the id fits ("a planet id is \"p\"/\"s\" plus a counter: stop, measure before widening"). WHY: for the keys to hold a value, the planet must own its copy. A value in `planet_t` is the smallest way: no allocation, the same 8 bytes as the pointer, and the burst slot does not grow.
- **D6b (guideline): `planet_keys_t` moves to a `game/` header** (the planet header). `entity_class.h` keeps `typedef struct planet_keys planet_keys_t;` for `entity_kind_t.new_entity`. WHY: the id size is a game constant the engine cannot include, and the moved struct declares no pointer, so no game file's count rises.
- `galaxy_populate` fills the keys with values. Its two game-zone `vec_t` allocations go, and the id string is built in the frame zone and copied.
- `structure_icon_left_press` builds the id in the frame zone. The grab-arena reservation drops the id string's bytes, and the comment "The grab opens before `id_str` is made…" goes.
- `burst_keys_t.pos_copy` goes. The protostar sets `keys.planet.pos` / `has_pos`. WHY: both keys ended in the same `entity_set_pos`, and the assert existed only to keep them exclusive.
- All family constructors (planet, structure, trading hub, processor, ascension, protostar, burst, slipgate, infraspace gate, synthesizer) read the flags.

### Layout row keys

- **D4 (guideline): cells by value.** `layout_row_keys_t.items` becomes `layout_row_cell_t items[LAYOUT_ROW_KEYS_CAP]`, keeping `count`. `pos` becomes `vec_t pos` + `int has_pos`. The capacity is the largest cell count measured over `make e2e` and `make full-game` (planet inspection rows: 1 + needs + 1; resource row plus the buy row's tail and name). An `assert` with a message guards every append ("a row's cells are measured: stop, raise LAYOUT_ROW_KEYS_CAP after measuring"). WHY: the plan says the bundles hold values. Only a bounded array removes the pointer, and the measured cap is the rule the code already applies elsewhere (toast text, served ids, message-box items).
- A small helper appends a cell under that assert. Builders use it instead of writing into caller arrays.
- `layout_row_keys_of_row(row)` returns keys by value. Its `cells` / `cap` parameters go, along with the callers' side arrays (`MENU_ROW_CAP` arrays, `icon_items`, `planet_projects_panel_row_t.layout_row_items`, the help screen's arrays).
- The 5b keys builders (H-6-2) return keys by value, and their frame-zone cell arrays go. The planet inspection panel's per-row frame-zone cell arrays go too, along with `row_cap` / `planet_inspection_panel_row_items`. Its "every row keeps its last cell for the gap" assert becomes the capacity assert.
- `layout_row_new(zone, const layout_row_keys_t *keys)` keeps its signature, since keys are a large struct passed for the call. NULL keys stay legal and mean the same as zero keys (an empty row).
- **D5 (guideline): `layout_row_cell_t.as.text` stays,** and the step note lists it as "pending user validation". WHY: it is not one of the ten bundles the plan names. Its texts are static strings or frame-zone strings consumed within the call, and `layout_row_new` copies them into the row's zone. A `char` buffer in every cell would multiply the size of every embedded keys array. Step 6 needs only the bundles. The plan's section 7 rule sends exceptions to the user.

### Button keys

- `button_keys_t.layout_row_keys` becomes a `layout_row_keys_t` value. Zero keys mean "no row", as NULL did: `button_layout_row` already treats NULL keys as `layout_row_new(slot, NULL)`, which builds the same empty row as zero keys.
- `side_tooltip_row` becomes a `layout_row_keys_t` value + `int has_side_tooltip_row`. The GAP-23 test reads `has_side_tooltip_row || (has_side_tooltip && !side_tooltip_is_false)`, the same three cases as before. `/* GAP-23 */` stays.
- `tooltip_row` becomes a `data_row_t` value + `int has_tooltip_row`. `tooltip_new` receives the value's address for the call. The row's own `items` pointer is counted in `map_tables.h` and unchanged (5b).
- `pos` / `slide_target` become `vec_t` + `int has_pos` / `int has_slide_target`. `open_box_step` writes the values.
- Every caller fills values: menus, help screen, score-panel buttons, planet inspection and projects panels, message-box description row. `planet_projects_panel_row_t` goes, because its keys and cells are now inside the button keys.

### Message box, toast, open box

- **D7 (guideline): `message_box_keys_t.items` and `count` go.** The year banner opens its box and then calls `message_box_add_row` for each row, as the score panel does after 5b (H-6-4). WHY: the separate function already exists and keeps the draw order. A value array of `data_row_t` in the keys would duplicate it.
- `toast_keys_t.text` becomes `char text[TOAST_TEXT_CAP + 1]` + `int has_text`. Its length assert moves to where the caller fills it (a small setter). `pos` becomes `vec_t` + `int has_pos`. The economy and planet callers fill the values.
- **D10 (guideline): `open_box_frame_t.description` becomes `char description[OPEN_BOX_DESCRIPTION_CAP + 1]` + `int has_description`.** The cap is measured: the longest of the planet type descriptions and the galaxy menu's "level N (a/b)". An assert with a message checks the copy. `fixed_pos`, `row_step` and `pos` become `vec_t` + `int has_fixed_pos` / `has_row_step` / `has_pos`. WHY: a description is either a static name or a frame-zone string. A bounded copy is the value form, and the strings are short.
- **D8 (guideline): `open_box_ctx_t.frame` goes.** `open_box_step(ctx, frame, row)` takes the frame as a `const open_box_frame_t *` parameter, valid for the call. WHY: this is the plan's "separate function" form, and it is the smaller change. The context keeps only `button_pos` and `pos`.
- The pause menu reads its `PAUSE_MENU_OPEN_BOX_HAS_*` bits into the frame's flags. The galaxy menu reads `score_panel_layout_block_t`'s static pointers: when one is non-NULL, it sets the flag and copies the value.

### Final score

- **D9 (guideline): `final_score_t.happiness` becomes `p8_fixed_t`.** The score row's text for index 4 is built where the row is built, through one `economy_score` function that prints a fixed with a trailing `%` into a zone (today's `concat_percent(p8_tostr_fixed(...))`). WHY: the string is derived data. Printing it at use yields the same bytes, and the struct then holds only numbers.
- `score_panel_update_ctx_t` holds `final_score_t final_score` by value (H-6-5).

### Guard

- `tools/pointer_guard.sh` is unchanged. Each chunk lowers `tools/pointer_guard_exceptions.txt` by exactly what it removes. A line that reaches 0 is deleted.
- Final deltas: `entity_class.h` −3, `planet.h` −1, `planet_family.h` −1, `layout.h` −2, `ui_button.h` −5, `message_box.h` −7, `message_box.c` −1, `economy_score.h` −1, `score_panel_display.c` −1. That is −22.
- Final values if H-6-1 holds: `entity_class.h` 2, `planet.h` 5, `planet_family.h` 1, `layout.h` 4, `ui_button.h` 9, `message_box.h` 2. The lines for `message_box.c`, `economy_score.h` and `score_panel_display.c` are deleted. Total 119 pointers, 0 entity lists.
- The last chunk moves every file whose remaining pointers are all allowed by section 2 (static data, owned buffers, generator state) under the file's "all stay" heading. Counts do not change. `layout.h` stays above it, because of `as.text` (D5).

### Comments and docs

- Following CLAUDE.md § Comments, these go: "Valid only for the call it is passed to" (`ui.c`), the structure icon's grab-arena comment, `burst_create`'s both-keys assert, and any comment that explained a NULL position. New `assert` messages cover the id size, the cell capacity, the description cap and the toast text. One trap comment covers D12.
- No GAP is expected. GAP-23 keeps its marker. If a sha moves, investigate with `help-me-debug`. Never regenerate it.

## Implementation Chunks

Each chunk is green and committable. `make test-all` passes, every sha is unchanged, and the exception file goes down by exactly the chunk's removals. Engine signatures come first, then the bundles module by module.

1. **Draw by value (engine).** `draw` / `draw_overlay` take `vec_t screen_pos`, the registry passes `entity_pos`, and every draw method across `game/` is updated (help, planet, ascension, burst, slipway, wormhole, targeting tooltip, emitter, button, score panel, message box, toast, ui overlay, background, structure icon). `box_contains` takes a `box_t`. Tests follow. Guard: 0.
2. **Mouse methods by value (engine + ui).** `mouse_pos_t`, the `hit_test` / `left_press` / `right_press` / `drag` signatures, `entity_registry_hit_test_always`, `entity_geom_contains_cursor`, `ui_send_to_hovered`, the `ui.c` dispatch (D12), and every game callee (background, slipway, wormhole, targeting, planet, structure icon, slipgate, button, modal backdrop, help). The tests' `vec_ref` / NULL calls build a `mouse_pos_t`. Guard: 0.
3. **Scalar parameters.** `hud_print_outlined` by value, `need_new(zone, resource_t)`, and any other input `p8_fixed_t *` / `vec_t *` left outside bundles (the `planet_refresh` alias if 5b left it). Guard: 0.
4. **Planet keys.** `planet_keys_t` by value, moved to `game/`. `planet_t.planet_id` by value with `PLANET_HAS_ID`, and `PLANET_ID_SIZE` shared with the served set. Galaxy and structure-icon callers, family constructors, `burst_keys_t.pos_copy` gone. Grab-arena and game-zone byte tests re-measured. Guard: `entity_class.h` −3, `planet.h` −1, `planet_family.h` −1 (−5).
5. **Layout row keys.** Embedded cells with the measured `LAYOUT_ROW_KEYS_CAP`, `pos` by value, `layout_row_keys_of_row` by value, the 5b builders and the inspection panel return or fill keys by value, side cell arrays gone. `make state` once to see the frame peak. Guard: `layout.h` −2.
6. **Button keys.** Row and side-tooltip keys by value, tooltip row as `data_row_t`, `pos` / `slide_target` by value, `planet_projects_panel_row_t` gone, every caller updated. `make state` again. If the frame zone overflows, apply D11. Guard: `ui_button.h` −5.
7. **Message box, toast, open box.** `message_box_keys_t.items` gone (the year banner adds its rows), toast keys by value, `open_box_frame_t` by value, `open_box_ctx_t.frame` turned into a parameter. Guard: `message_box.h` −7, `message_box.c` −1 (−8).
8. **Final score and closing audit.** `final_score_t.happiness` as `p8_fixed_t`, `score_panel_update_ctx_t` by value, the exception file regrouped, and the audit of remaining `vec_t *` / `box_t *` / `p8_fixed_t *` written to the step note. Guard: `economy_score.h` −1, `score_panel_display.c` −1 (−2). After this chunk, `plan-pilot` runs `make sanitize-net` and `make full-game`.

## Testing Decisions

- A good test drives the public constructors and methods with values and checks what a caller sees: positions, hover and press results, drawn pixels, the placed items of a row, the guard's counts. It never compares addresses and never reads a field this step removes.
- Tests that froze the old contract (`vec_ref(...)` arguments, NULL mouse vectors, `layout_row_keys_of_row` with a caller buffer, `keys.items` arrays, `s.happiness` compared as a string) are updated to the new one, never loosened (CLAUDE.md: the e2e replays are the source of truth). Each test file's local `vec_ref` / `mouse_at` helper becomes a local `mouse_pos_t` builder.
- New or rewritten checks, one per behavior:
  - `tests/test_ui.c` / `tests/test_background.c` / `tests/test_probe.c` / `tests/test_slipway.c`: a press or drag with an absent world or screen point behaves as with the old NULL (position cleared, `has_drag_mouse` 0, slipway point cleared), and a point given as zero is not treated as absent.
  - `tests/test_entity_kind.c` or `tests/test_geom.c`: `entity_geom_contains_cursor` picks the screen point for layer ≥ 10 and the world point below.
  - **Keys are copied, never kept**, one test per bundle: build keys, construct, overwrite the keys, and check that the entity is unchanged. That covers a planet's position, map position and id, a burst's position, a toast's text and position, a button's position and slide target, a layout row's position, and an open box's description and positions.
  - Child-process asserts (prior art: `in_fresh_child` in `test_layout_row.c`): an id longer than `PLANET_ID_SIZE − 1`, a row past `LAYOUT_ROW_KEYS_CAP`, and a description past its cap each trip their assert.
  - `tests/test_score_panel.c`: `economy_score_final().happiness` is the fixed value, and the score row still prints "1%" (the old string case, moved to the drawn text).
  - `tests/test_hud.c`: alignment 0 draws the same pixels as the default.
  - `tests/test_research_project.c` / `tests/test_economy.c`: `need_new` with `RESOURCE_NIL` and with a fixed resource.
- Byte-count tests (`test_state_peaks`, `test_game_zone`, `test_slot_tally`, `test_ui_zone`, `test_live_slots`, and any grab-arena tally) are re-measured to the new layout, never loosened past the measurement.
- `tests/test_pointer_guard.c` is unchanged. The guard itself proves the counts.
- Prior art: `test_probe.c`'s drag cases, `test_planet.c`'s hit-test folds, `test_toast.c`, `test_button.c`, `test_layout_row.c`'s screen-hash helpers.
- Outside unit tests: every sha unchanged (`make e2e`) after every chunk, `make sanitize-net` and `make full-game` after the step, `make state` after chunks 5 and 6 and at the end, and `make test` / `make e2e` wall time compared before and after in alternating pairs, as in Step 1.1.

## Out of Scope

- Out-parameters, arrays, returned pointers to static data and `box_make` (D3).
- `layout_row_cell_t.as.text`, `data_item_t.text` / `.row` and `data_row_t.items` (D5, and step 5b's guard notes). They stay counted and go to the user as exceptions.
- Owned buffers allowed by section 2 (`planet_t.layout_row`, `button_t.layout_row` / `side_tooltip` / `tooltip`, `tooltip_t.rows`, `layout_row_t.placed_items`, `layout_row_item_t.text`, structure-icon tooltips) and the generator state.
- `planet_production()` returning a pointer (not a small value).
- Lowering slot sizes or zone constants to bank the savings (step 7).
- Moving `entity_t` / `entity_class.h` into `game/` (plan section 2), beyond `planet_keys_t`'s definition (D6b). Audio, the raylib binary, PSX.

## Further Notes

- Count check: 22 bundle pointers in the step-1 sorting, minus `layout_row_copy` (step 5b), gives 21. Add `planet_t.planet_id` (D6) for a total of −22.
- Risk, embedded cells: keys grow from about 80 bytes to about 1 KB (cap × cell). Stack copies are harmless on PC. The projects panel allocates one button keys per available project in the frame zone. That is why `make state` runs after chunks 5 and 6, under D11. A cap set too low trips the assert in `make full-game`, never silently.
- Risk, D12: if the screen point were taken from `entity_pos()` instead of the raw header field, a mouse without a position bit would read zero instead of its stored value. Keep the raw field, and if a sha moves, look there first.
- Risk, speed: values replace pointers in per-frame dispatch (draw, hit test). `mouse_pos_t` is 20 bytes. Measure `make e2e` wall time in alternating pairs before and after chunks 1 and 2.
- Guideline decisions (the PRD writer applied the `guideline` skill itself): D1 `mouse_pos_t`; D2 draw position as a value with zero for absent; D3 out-parameters and static returns stay; D4 embedded measured cells; D5 `as.text` stays, pending user validation; D6 / D6b id by value and `planet_keys_t` in `game/`; D7 rows added by function; D8 frame as a parameter; D9 happiness as a number; D10 bounded description; D11 frame budget raised only by measured growth; D12 raw mouse header position.
- The step graduates with one line in plan section 7: final counts (119 / 0 if H-6-1 held), the peaks that moved, any budget raised (D11), the "pending user validation" line for `layout_row_cell_t.as.text`, the remaining-pointer audit, and which step-5b hypotheses failed, if any.
