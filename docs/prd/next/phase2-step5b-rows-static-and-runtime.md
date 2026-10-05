---
title: "Rows: static and runtime apart"
description: "The row types (`data_row_t`, `data_item_t`, `layout_row_cell_t`, `message_box_t.items[]`) point sometimes at static tables and sometimes at runtime rows, and several stored rows keep pointers into the frame zone after it is rewound."
status: needs-triage
---

## Problem Statement

Plan 13 (`docs/plan-13.md`), phase 2 (Readability), step 5b.

The step-1 sorting put seven pointers under "Rows used both ways" (section 7), and the user moved them to this step ("Step 1 sorting — user decisions"):

- `src/game/layout.h` (2): `layout_row_cell_t.as.text`, `.as.row`
- `src/game/map_tables.h` (4): `data_item_t.text`, `.row`, `.ref`, `data_row_t.items`
- `src/game/message_box.h` (1): `message_box_t.items[]`

The same types describe two different things. Some instances are `static const` tables generated from the cart (`game_data.c`, `map_tables.c`). Others are built at runtime, in the frame zone, on the stack, in a ui-zone slot or inside their owner. Because one type covers both, nothing tells a reader whether a row can be kept. Several rows are in fact kept past the memory they point into:

- **Planet row.** `planet_refresh` builds `planet_t.layout_row` (a ui-zone row slot) from cells whose `as.row` point at frame-zone tuples, then rewinds the frame zone. The stored row's `items[]` dangle. The only thing that keeps it safe is a comment ("read only `placed_items[]` from here").
- **Button rows.** Every `button_t.layout_row` / `side_tooltip` / tooltip line keeps its cells. A cell's `as.row` points at a frame tuple (the planet production tuple, the planet resource tuples), at a game-zone `need_t`, or into the research project table (`layout_row_inner`).
- **Score rows.** `score_panel_update` writes frame-zone strings (`score_panel_final_score_text`, the "score 0" label) into the file-scope `g_score_rows[].items[]`. Those stay there after the frame. Each update rewrites them before reading them, so they are safe only by coincidence.
- **Research project row.** `research_project_t` builds a row whose items point into the project itself (`layout_row_own_items`, `layout_row_inner`). It is safe, but it is a runtime `data_row_t` living in the project table.
- **Message box articles.** `message_box_t.items[]` points at `static const` rows, at the score panel's file-scope runtime rows, or at copies in the box's own slot (`message_box_draw_row_t`). Those copies reach a layout row and a position through `data_item_t.ref`, a `const void *` whose meaning depends on the draw function.
- **`data_item_t.ref`.** No static table uses it. At runtime it carries a layout row or a `vec_t` for `DRAW_FUNCTION_DRAW_ROW`, or it is NULL as a "nil" marker (`research_display`, `score_panel_refresh`).

Plan rule (step 5b): "Static rows keep `const` pointers to static data. A stored runtime row never points into the frame zone: it holds an index into the static table, or is rebuilt when drawn; it owns a copy of its text and items only when neither fits, and the zone holding a copy has its budget raised by the copy's measured size."

## Solution

`data_row_t` / `data_item_t` describe static data. No runtime row of those types is ever stored. Runtime rows use value types that state what they hold:

- A **layout row stores only what it draws**. `layout_row_t` is built once from call-scoped cells into its `placed_items` (text copied into the row's zone, as today) and keeps no cells. A planet row, a button row and a tooltip line can no longer point into the frame zone, a need or the project table.
- A **tuple is a value**. A row-kind cell holds its up to four scalars by value, copied when the cell is made (`layout_row_cell_t.as.row` goes). Runtime tuples (planet resource row, production tuple) are built directly as cells, with no frame-zone `data_row_t`.
- **Runtime row sources are rebuilt when drawn or built.** The research project's icon row is built from the project's fields when the projects panel opens (the project stores no row). The score rows are rebuilt from `score_rows_table` at each update (`score_row_t` and `g_score_rows` go).
- **The message box owns its articles by value** (`message_box_article_t`): a draw-function article copies its scalar arguments and its text, and a layout-row article holds its layout row, position and alignment. Copying is the fallback the plan allows, because no index or rebuild fits (see Implementation Decisions). `message_box_t.items[]` and `data_item_t.ref` go.

Every `*.sha256` in `tests/e2e/` stays identical. The pointer guard goes down by exactly 6, from 147 to 141 if step 5 lands as planned (H-5b-1): `layout.h` −2, `map_tables.h` −1, `message_box.h` −1, `score_panel.h` −1, `ui_button.h` −1. The entity-list count stays 0. `data_item_t.text` / `.row`, `data_row_t.items` and `layout_row_cell_t.as.text` stay counted (see Guard). `make test` and `make e2e` are no slower. `make sanitize-net` and `make full-game` pass.

## User Stories

1. As a port developer, I want `data_row_t` and `data_item_t` to describe only static data, so that a pointer of those types never needs a lifetime check.
2. As a port developer, I want a `layout_row_t` to keep only its placed items, so that a stored row cannot point at the cells it was built from.
3. As a port developer, I want a planet's stored layout row to hold nothing in the frame zone, so that the comment warning against reading its `items[]` after the rewind goes.
4. As a port developer, I want button rows, side tooltips and tooltip lines to hold nothing in the frame zone, in a need or in the project table, so that a button outliving any of them is safe.
5. As a port developer, I want a row-kind cell to carry its tuple by value, so that a cell built from a frame-zone tuple is still valid after the frame.
6. As a port developer, I want the planet resource row to be built straight into cells, so that it allocates no `data_row_t` / `data_item_t` tuples in the frame zone.
7. As a port developer, I want the inspection panel's production tuple built as a tuple cell, so that `planet_production_tuple` and its frame-zone `data_row_t` go.
8. As a port developer, I want the planet resource row builder to return call-scoped layout keys, so that its caller decides where the row lives and the row is built once.
9. As a port developer, I want the research project's buy row to be built as keys (resource row + tail + name) and laid out once by the button, so that `layout_row_add` and the copy that re-measured added cells go.
10. As a port developer, I want `button_keys_t.layout_row_copy` removed, so that a button gets its row from one key, `layout_row_keys`.
11. As a port developer, I want the planet resource row builder to take its output as a presence flag and a value, so that no changed function takes a `p8_fixed_t` by pointer.
12. As a port developer, I want the research project to store no layout row, so that the project table holds no runtime `data_row_t` pointing into itself.
13. As a port developer, I want the projects panel to build a project's icon row (`name`, 32, `{icon_sprite, 8}`) from the project's fields when it opens, so that the row is rebuilt when drawn.
14. As a port developer, I want the score rows rebuilt from `score_rows_table` at each update, so that no file-scope row keeps a frame-zone string.
15. As a port developer, I want the "score 0" row to keep its two elements (GAP-25) after the rebuild, so that the panel's centering is unchanged.
16. As a port developer, I want each score bar's draw function stated once, inside its static row, so that the score panel needs no runtime copy just to set it.
17. As a port developer, I want the message box to own its articles by value, so that it never points at the score panel's state or at static rows it does not need to name.
18. As a port developer, I want a message-box text article to copy its text into a bounded buffer, with an `assert` on the length, so that nobody widens it silently.
19. As a port developer, I want the message box to build a layout-row article in its own slot from call-scoped keys, a position and an alignment, so that the frame-zone layout row and its copy go.
20. As a port developer, I want the message box to draw its articles with the same camera alternation as `draw_functions_draw_rows`, so that every frame is drawn as before.
21. As a port developer, I want `data_item_t.ref` removed and the nil marker renamed `is_nil`, so that an item's fields say what they hold.
22. As a port developer, I want `draw_functions` to stop drawing `DRAW_FUNCTION_DRAW_ROW`, so that it only reads scalar rows.
23. As a port developer, I want an `assert` with a message when a tuple holds text or a nested row, so that the value tuple is not widened without measuring.
24. As a reviewer, I want `tools/pointer_guard_exceptions.txt` lowered in each chunk by exactly that chunk's removals, so that the ratchet proves what each commit did.
25. As a reviewer, I want no new pointer field or file-scope pointer in `src/game/` or `entity_class.h`, so that the guard only goes down.
26. As a reviewer, I want no new function that takes a `vec_t`, `box_t` or `p8_fixed_t` by pointer, and no callee that keeps an address it received, so that the verifier finds no new violation of the rules the guard cannot see.
27. As a reviewer, I want every `*.sha256` in `tests/e2e/` unchanged after every chunk, so that each chunk is proven not to change the game.
28. As a reviewer, I want `make sanitize-net` and `make full-game` to pass after the step, so that no row reads freed or rewound memory and no slot or zone fills up.
29. As a reviewer, I want every slot size and zone budget either unchanged or raised by exactly a measured copy size, so that the plan's copy rule is followed.
30. As a port developer, I want `make test` and `make e2e` no slower, so that the fast loop stays fast.
31. As a port developer, I want no game concept added to `src/engine/`, so that the engine stays free of planets and rows.
32. As a port developer, I want `docs/porting-gaps.md` GAP-25 reworded to the row's `placed_items`, so that it describes the code as it is.
33. As a future step-6 implementer, I want the call-scoped bundles (`layout_row_keys_t`, `message_box_keys_t`, `button_keys_t`, `open_box_frame_t`) left as bundles, so that step 6 converts them with its own counts.
34. As a future step-7 implementer, I want the step note to record every slot and zone peak that moved, so that the memory balance can attribute it to step 5b.

## Hypotheses on step 5

Step 5 (`docs/prd/triage/phase2-step5-own-and-lists-by-value.md`) is being implemented while this PRD is written. Everything below rests on its PRD, not on code. The implementer checks each hypothesis against the committed tree before starting and writes a verdict next to it in the step note. If one fails, the decisions keep their intent and take the real names and numbers. The guard deltas of this step are fixed (−6). Only the starting values move.

- **H-5b-1 — guard after step 5.** 147 pointers, 0 entity lists. Per-file values for the files this step touches: `layout.h` 8, `map_tables.h` 22, `message_box.h` 10 (step 5 removes `toast_t.text`), `score_panel.h` 4 (step 5 removes `score_panel_layout_block_t.description`), `ui_button.h` 15 (step 5 removes `button_t.slide_target`). All six pointers this step removes are still declared: `layout_row_cell_t.as.row`, `layout_row_t.items`, `data_item_t.ref`, `message_box_t.items[]`, `score_row_t.entry`, `button_keys_t.layout_row_copy`.
- **H-5b-2 — research project row.** `research_project_t.layout_row` is a `data_row_t` value with `RESEARCH_PROJECT_HAS_LAYOUT_ROW` in a new `research_project_t.has`. `layout_row_own` is gone. `layout_row_inner` and the item arrays (`layout_row_own_items` or its renamed equivalent, `layout_row_inner_items`) remain and are filled in `project_create`. `planet_projects_panel_keys` passes the row's address when the bit is set and calls `research_project_layout_row` otherwise.
- **H-5b-3 — row types untouched.** `data_row_t`, `data_item_t`, `layout_row_cell_t`, `layout_row_t`, `layout_row_keys_t`, `tooltip_t`, `message_box_t`, `message_box_keys_t`, `score_row_t`, `draw_functions` and `layout_row_copy` / `layout_row_add` are as they are at `HEAD` `534c9de` (step 5's Out of Scope).
- **H-5b-4 — planet fields read by the row.** `planet_t.needs` is a `pointer_list_t` value with `PLANET_HAS_NEEDS`. The production comes through `planet_production()` (the planet's own one with `PLANET_HAS_PRODUCTION`, or the shared one). `planet_t.outgoing_routes` is a value with `PLANET_HAS_OUTGOING_ROUTES`. `planet_t.layout_row` (`layout_row_t *`) and `layout_row_slot` are unchanged (allowed owned buffer). `planet_refresh` still builds the row in a ui-zone row slot under a frame-zone mark and rewinds afterwards.
- **H-5b-5 — research inputs.** `research_project_t.inputs` is a `pointer_list_t` value, always present. `research_project_layout_row` and the tooltip join read it by value.
- **H-5b-6 — economy and score panel.** `economy_t.invented` is a `pointer_list_t` value (`invented_lines` reads it by value or address). `economy_t.end_status` and `has_end_status` are unchanged. `g_score_rows`, `score_row_t`, `score_panel_rows()`, `g_score_panel_readouts` and `g_score_panel_year_banner` are unchanged.
- **H-5b-7 — memory constants.** Step 5 changed no zone constant and no hand-measured slot size, except possibly a slot raised by its "measured struct no longer fits" decision (recorded in its step note). `MESSAGE_BOX_SIZE` (5128), `ROW_SIZE` (1184), `BUTTON_SIZE` (6056) and `FRAME_ZONE_BYTES` (20256) are as at `HEAD`. If one moved, this step starts from the new value.
- **H-5b-8 — green baseline.** After step 5: same shas, `make sanitize-net` and `make full-game` pass, `make test-all` is green, and the plan's section 7 has a "Step 2.5" note.

## Implementation Decisions

### General rules for this step

- **What counts as stored.** A row is stored when it outlives the call that built it: a field of an entity or of a file-scope variable, or memory in a slot or a zone other than the frame zone. Call-scoped rows and cells (stack arrays, `layout_row_keys_t`, frame-zone keys consumed before the call returns) are parameters in the sense of plan section 2, and step 6 owns their bundles.
  - DECISION (guideline): transient frame-zone `data_row_t`s that are built and consumed within one call stay as they are: `research_project_tooltip` and `invented_lines` (both consumed by `tooltip_new` inside `button_construct`). WHY: the step's rule is about stored rows ("a stored runtime row never points into the frame zone"). A tooltip already copies its text into its own layout rows, and changing the tooltip key would add a pointer field to `button_keys_t`, which raises `ui_button.h`.
  - DECISION (guideline): the score panel's `g_score_panel_readouts` and `g_score_panel_year_banner` stay as file-scope rows. WHY: their items point only at static rows, at their own sub-rows or at their own `char` buffers, all in static storage that is never freed or rewound. The step's rule holds, and rewriting them is not what the step needs.
- **Preferred order, per the plan.** Rebuild when drawn (project icon row, score rows, planet resource row), then index into a static table (none fits here), then an owned copy (message-box articles only). Each case below says which one it uses.
- **Values, not addresses.** Functions this step adds or changes take `vec_t` and `p8_fixed_t` by value. Where a changed function used to take `const p8_fixed_t *output` to mean "maybe absent", it takes `int has_output, p8_fixed_t output`.
- **No engine change.** Every change is in `src/game/` (plus tests and the docs line). If an engine allocator call is missing, it is added generically, without naming rows.
- **Zone and slot sizes.** Dropping cells from stored rows and tuples from the frame zone lowers peaks. Hand-measured slot sizes and zone constants are not lowered (step 7 measures the balance). If one slot's measured use grows, as the message box could if its articles grow, that slot size rises by exactly the measured growth, its zone's static assert must still hold, and the change is recorded in the step note. This is the plan's "budget raised by the copy's measured size", and `make full-game` proves it.

### Layout rows keep no cells

- `layout_row_t` loses `items` and `items_count`. A row is laid out once, from the keys' cells, into `placed_items` (allocated in the row's zone, with each text copied into that zone as today), `placed_items_count` and `width`. Measuring is unchanged: the same `row_measure_item` logic, reading the keys' cell instead of a stored one, and the row's color.
- `layout_row_add` is removed. Only tests call it, and it never re-laid the row out: its cells were seen only by a later `layout_row_copy`.
- `layout_row_copy` copies the placed items (texts re-copied into the target zone), the width, the color, background and score flags, and the header position bit. Laying out the same cells with the same color gives the same placed items, so the result is identical to re-measuring.
  - DECISION (guideline): when no `src/` caller of `layout_row_copy` remains (after chunk 4), it is removed and its tests are folded into the tests of what replaced it. WHY: dead code is not kept, and the smaller interface is the deeper module.
- `research_display_planet_resource_row` becomes a keys builder: given the inputs list, a presence flag and value for the output, the amount, the used count and the spacing, it returns `layout_row_keys_t` whose cells live in the frame zone, and reports "no row" when there is no output. `planet_refresh` lays the row out once in its slot with `layout_row_new`, then rewinds. The comment about rewound tuples goes.
- `research_project_layout_row` becomes a keys builder too. It fills the resource-row cells, then appends the row-tail cell and the name cell into a frame-zone array sized for both, and reports "no row" as above. `planet_projects_panel_keys` passes those keys as `button_keys_t.layout_row_keys`. The button lays the row out once, which yields the same placed items as today's re-measured copy.
- `button_keys_t.layout_row_copy` goes, and `ui_button.c`'s copy branch goes with it.

### Tuples by value

- `layout_row_cell_t.as.row` becomes `as.tuple`, a `layout_row_tuple_t` value: a count (0 to 4) and four scalars, each a kind (nothing or fixed) and a `p8_fixed_t`. A row-kind cell is measured from the tuple exactly as it was from the row: first scalar as the drawn item, second the width, third the palette block, fourth the spacing ("nothing" keeps meaning "spacing 0, present").
- `layout_row_cell_row(const data_row_t *row)` stays as the way in from a `data_row_t` (static rows, a need's row). It copies the scalars into the tuple at once. A `has_resource` row becomes the tuple `{resource, 5}`, as the measure's `wrap` does today. A NULL row stays a nothing cell. `layout_row_cell_item` maps a nested `row` to `layout_row_cell_row` and an `is_nil` item to a nothing cell.
- A new `layout_row_cell_tuple(layout_row_tuple_t)` (plus small scalar helpers) builds runtime tuples. The planet resource row builds every tuple (inputs, separator pair, `35`/`32` pair, outputs) directly as tuple cells, and its frame-zone `planet_resource_row_tuple_t` goes. The inspection panel's production tuple becomes `{resource, 5}` when the production has a resource, and an empty tuple otherwise. `planet_production_tuple` goes.
- DECISION (guideline): a tuple holds no text, and making a tuple from a row whose scalar is text or a nested row trips an `assert` ("a tuple holds numbers only; a text one needs a pointer in the cell -- stop, measure first"). WHY: no tuple in the cart's data or in the runtime builders holds text (checked: galaxy menu, score row 10, gap/pair/back/tail/spacer, needs, planet resource row, project icon row). A text pointer in the tuple would put back the pointer this step removes. The case in `test_frame_zone.c` that builds a text tuple is rewritten.
- The cell grows from 16 to about 40 bytes. Cells are only call-scoped now (stack arrays and frame-zone keys), and the tuples they replace cost about 230 bytes each in the frame zone, so the frame peak is expected to drop. `make state` confirms it.

### Research project icon row

- The project stores no row: `layout_row` (value and presence bit, H-5b-2), `layout_row_inner` and their item arrays go from `research_project_t`, and `project_create` no longer fills them. Its `assert` that a project with an icon sprite has a name stays where the row is built.
- A new `research_project_icon_row_keys(project, cells, cap)` returns keys for `{name, 32, {icon_sprite, 8}}`: a string cell, a fixed cell 32 and a tuple cell, with no color, as `layout_row_keys_of_row` produced from the stored row. It is called only for a project with an icon sprite. `planet_projects_panel_keys` tests `has_icon_sprite` where it tested the row's presence.
- `research.h`'s count is unchanged: the removed fields are values.

### Score rows rebuilt at each update

- `score_row_t`, `g_score_rows` and `score_panel_rows()` go. `score_panel_update` walks `score_rows_table`.
  - A row without a score is a static row, passed to the message box as it is.
  - A row with a score builds its cells at each update from the entry's static items, with the same writes as today: index 3 = the score text, and for score 0 index 1 = the "end status (difficulty)" label; for score 6 indices 1..5 = the tier icons. Present indices are taken in ascending order (GAP-25, the "score 0" row keeps two elements). Then it adds a layout-row article at the entry's position, with alignment 0.5.
  - DECISION (guideline): rebuild rather than keep a mutable copy. WHY: every update writes the same indices before reading them (the copy's `has_value` only ever adds those indices), so a rebuild from the static entry gives the same cells. It removes the frame-zone strings left in file-scope storage. It is the plan's preferred "rebuilt when drawn".
- DECISION (guideline): each score bar's draw function moves into its static row in `game_data.c` (`.row = {…, .draw_function = DRAW_FUNCTION_DRAW_BAR}`), and `score_rows_entry_t.draw_function` goes. WHY: the copy existed only to set it. Stating it once in the static row lets the box take the row as is ("one name per piece of data"). The generated tables have no generator to keep in sync (only the `Makefile` compiles them).
- `score_panel.h` loses `score_row_t.entry` (−1).

### Message box articles by value

- `message_box_t.items[]` becomes `message_box_article_t articles[MESSAGE_BOX_ITEMS_CAP]` plus its count. An article is a value: its draw function, and either
  - scalar arguments: up to five, each a kind (number, false, nil, text) and a `p8_fixed_t`, with a text argument copied into a `char` buffer of `MESSAGE_BOX_TEXT_CAP + 1` (16 + 1, the longest being "2 years remain"); or
  - a layout-row article: a `layout_row_t` value laid out in place in the box's slot, a `vec_t` position with a presence flag, and an alignment.
  
  The two shapes may share storage. The struct declares no pointer field.
- DECISION (guideline): the box copies its articles. WHY: no index fits, since the articles come from three static tables (score rows, countdown rows, year-banner bar) plus the banner's runtime "year N" text. Rebuilding at draw does not fit either: the cart freezes the score values when the box opens, and recomputing `economy_score_final` every frame would cost time and could change a value. The box already owned copies of its draw-row articles, and the new article is smaller than that copy (a data row, three items and a layout-row copy with cells). This is the plan's fallback, under the copy budget rule above.
- Interface:
  - `message_box_new(keys)` keeps `message_box_keys_t.items` (call-scoped rows) and copies each row into a scalar article. A `DRAW_FUNCTION_DRAW_ROW` row there trips an `assert`.
  - A new `message_box_add_row(box, row)` copies one more static or call-scoped row.
  - A new `message_box_add_layout_row(box, layout_keys, pos, has_pos, align)` lays a row out in the box's slot as the next article.
  - Both assert the capacity with a message. The score panel opens its box with no rows and adds every score row in table order, so the draw order is unchanged. The year banner keeps passing its rows through the keys.
- Copying asserts: at most five arguments, text only in the first argument (the only one `print_outlined` reads as text), and a text no longer than the cap ("stop, do not widen").
- `message_box_draw_overlay` keeps the timing and closing logic. It walks the articles with the same per-article `p8_camera` alternation as `draw_functions_draw_rows`. A layout-row article is drawn with `layout_row_draw_at` at its position (zero when absent) and alignment. A scalar article is rebuilt into a stack `data_row_t` (text pointing at the article's buffer for the call) and drawn through a new single-row entry of `draw_functions`.
- `message_box_draw_row_t` and `message_box_article` go. `score_panel_update_draw_row_t` goes.

### `data_item_t.ref` and `draw_functions`

- `data_item_t.ref` goes, and `is_ref` becomes `is_nil`: "no value", which a layout row turns into a nothing cell and `draw_functions` treats as "not a number". Its writers are the planet resource row's missing resource, which now builds a nothing scalar directly, and the readouts' absent research points, year and month, which become `is_nil`.
- `draw_functions` loses `draw_functions_ref` and its `DRAW_FUNCTION_DRAW_ROW` entry. `DRAW_FUNCTION_DRAW_ROW` stays in `draw_function_t` as the message box's layout-row article kind, and `draw_functions` asserts it never receives one ("a layout row is drawn by its message box").

### Guard

- `tools/pointer_guard.sh` is unchanged. Each chunk lowers `tools/pointer_guard_exceptions.txt` by exactly what it removes.
- Final deltas: `layout.h` −2 (`layout_row_cell_t.as.row`, `layout_row_t.items`), `map_tables.h` −1 (`data_item_t.ref`), `message_box.h` −1 (`message_box_t.items[]`), `score_panel.h` −1 (`score_row_t.entry`), `ui_button.h` −1 (`button_keys_t.layout_row_copy`). That is −6.
- Final values if H-5b-1 holds: `layout.h` 6, `map_tables.h` 21, `message_box.h` 9, `score_panel.h` 3, `ui_button.h` 14. Total 141 pointers, 0 entity lists.
- What stays counted and why:
  - `data_item_t.text` / `.row` and `data_row_t.items` stay as `const` pointers. In every stored row they point at static data (or, for the readouts and banner, at their own static storage). Runtime instances are call-scoped only.
  - `layout_row_cell_t.as.text` stays. A cell is now call-scoped only, since no stored row holds a cell, so it belongs with the step-6 bundles that carry cells (`layout_row_keys_t.items`).
  - `layout_row_item_t.text`, `layout_row_t.placed_items` and `tooltip_t.rows` are owned buffers allowed by section 2.
- The step note lists these lines so `warn-review` can sort them. A reviewer who finds `as.text` or the transient tooltip rows a violation reports it as "planned for step 6" (call-scoped bundles), not as a step-5b leftover.

### Comments and docs

- Following CLAUDE.md § Comments: the comments "The row's items[] point into the rewound tuples…" and "A tuple is borrowed until this layout only…" go, along with anything else that explained the removed indirections. New `assert` messages cover the tuple without text, the article text cap and argument count, the article capacity, and `DRAW_ROW` never reaching `draw_functions`.
- GAP-25 in `docs/porting-gaps.md` is reworded: the row's `placed_items` keep both elements (no `items` member any more). `/* GAP-25 */` moves with the code that closes the hole. No new GAP is expected. If a sha moves, investigate with `help-me-debug`, never regenerate it.

## Implementation Chunks

Each chunk is green and committable on its own. `make test-all` passes, every sha is unchanged, and the exception file goes down by exactly the chunk's removals.

1. **Layout rows keep no cells.** Remove `layout_row_t.items` / `items_count` and `layout_row_add`. `layout_row_copy` copies placed items. The planet resource row and the research project buy row become keys builders (output by presence and value), `planet_refresh` lays out once, and `button_keys_t.layout_row_copy` goes. The message box still uses `layout_row_copy`. Guard: `layout.h` −1, `ui_button.h` −1 (−2).
2. **Tuples by value and the project icon row.** `layout_row_cell_t.as.tuple`, `layout_row_cell_tuple`, the copy in `layout_row_cell_row`, the text-in-tuple `assert`, the planet resource row and the production tuple built as tuple cells, and the research project icon row built on demand (the project's row fields go, H-5b-2). Guard: `layout.h` −1.
3. **Score rows rebuilt.** `score_row_t`, `g_score_rows` and `score_panel_rows()` go, the bars' draw function moves into their static rows, and `score_panel_update` builds each scored row's cells from the entry. The draw-row article it hands to today's message box keeps its position in a frame-zone `vec_t` the box copies within the same call. Guard: `score_panel.h` −1.
4. **Message box articles by value, `ref` gone.** `message_box_article_t`, `message_box_add_row`, `message_box_add_layout_row`, the draw loop, the single-row `draw_functions` entry, `is_ref` → `is_nil`, `data_item_t.ref` and the `DRAW_ROW` dispatch removed, and `layout_row_copy` removed if no `src/` caller remains. GAP-25 reworded. Byte-count tests re-measured. Guard: `message_box.h` −1, `map_tables.h` −1 (−2). After this chunk, `plan-pilot` runs `make sanitize-net` and `make full-game`.

## Testing Decisions

- A good test builds rows, buttons, planets and boxes through their public constructors, drives the real refresh, update and draw paths, and checks what a caller sees: the placed items, the width, the framebuffer hash, a box's article count and kinds, and the guard's counts. It never compares addresses of internal buffers, and never reads a struct member that this step removes.
- Tests that froze the old structure are updated to the new contract (CLAUDE.md: the e2e replays are the source of truth), never loosened. Hash folds over a row's `items[]` (`test_layout_row`, `test_research_project`, `test_score_panel`, `test_burst`, `test_planet_exports_row`, `test_burst_exports_row`) fold `placed_items` and the width instead. Their frozen constants are recomputed once, and the commit message says that what is drawn did not change: the framebuffer and the e2e are the proof. The `is_ref` folds become `is_nil`.
- New or rewritten checks:
  - `tests/test_frame_zone.c`: a planet row, a button row (projects panel), a tooltip line and a message-box layout-row article each draw the same pixels after `frame_zone_reset` plus re-allocation as before it. The score box still shows the happiness cell's text after the reset (read through the article's placed items). The text-tuple case becomes "a tuple from a row with text trips the assert", in the file's existing child-process style (`in_fresh_child`, if the file has it; otherwise in `test_layout_row.c`, which does).
  - `tests/test_layout_row.c`: a tuple cell made from a frame-zone row is unchanged after the row's memory is overwritten. The research project buy row laid out from keys draws what the old re-measured copy drew (same placed items and width as a reference built from the same cells). `drawn_after_adds` becomes "a row built from all eight cells draws the same in a bump zone and the heap".
  - `tests/test_research_project.c`: `research_project_icon_row_keys` yields string, 32, tuple `{icon_sprite, 8}`, and a project stores no row.
  - `tests/test_score_panel.c`: two updates build identical cells. The "score 0" row lays out two elements (GAP-25). The bars reach the box with `DRAW_FUNCTION_DRAW_BAR` from the static rows. No file-scope row keeps a frame string (draw after reset).
  - `tests/test_message_box.c`: an article copies its text (changing the year banner's buffer after opening does not change the drawn text). The capacity, argument-count and text-length asserts trip in child-process tests. A `DRAW_ROW` row passed as a plain row trips its assert.
  - `tests/test_button.c`: a button built from `layout_row_keys` for a project draws as before. The `layout_row_copy` key cases are removed.
- Byte-count tests (`test_state_peaks`, `test_slot_tally`, `test_ui_zone`, `test_live_slots`, `test_game_zone`) are re-measured to the new layout, never loosened past the measurement.
- `tests/test_pointer_guard.c` is unchanged. The guard itself proves the counts.
- Prior art: `test_frame_zone.c`'s "survives reset" cases, `test_layout_row.c`'s screen-hash helpers (`drawn_by_draw_at`, `screen_hash`), `test_score_panel.c`'s box inspection.
- Outside unit tests: every sha unchanged (`make e2e`) after every chunk, `make sanitize-net` and `make full-game` after the step, `make state` once to record the frame, row-slot, button-slot and message-box peaks, and `make test` / `make e2e` wall time compared before and after in alternating pairs, as in Step 1.1.

## Out of Scope

- Step 6: call-scoped bundles and their pointers (`layout_row_keys_t.items` / `.pos`, `message_box_keys_t.items`, `button_keys_t.*`, `open_box_frame_t.*`), and existing signatures taking `vec_t *` or `p8_fixed_t *` that this step does not otherwise change.
- Transient frame-zone `data_row_t`s consumed within one call (`research_project_tooltip`, `invented_lines`), and the tooltip key `button_keys_t.tooltip_row`.
- The score panel's file-scope readouts and year banner.
- `need_t`: its embedded `data_row_t` holds no pointer (`items` is NULL), and a cell copies it at once.
- The help screen and the score panel's own draw, which draw static rows and the readouts through `draw_functions` as today.
- Lowering slot sizes or zone constants to bank what this step saves (step 7 measures it).
- Moving `entity_t` / `entity_class.h` into `game/`. Audio, the raylib binary, PSX.

## Further Notes

- Count check: the seven "rows used both ways" pointers end as follows. `as.row`, `ref` and `message_box_t.items[]` are removed (3). `data_item_t.text`, `.row`, `data_row_t.items` and `layout_row_cell_t.as.text` stay counted (4). Three more pointers go because the fix makes them dead: `layout_row_t.items`, `score_row_t.entry` and `button_keys_t.layout_row_copy`. Total −6.
- Risk, measurement equivalence: laying out once from all cells must equal today's "lay out, add, copy and re-measure" for the project buy row. The color is the only row-level input, and the resource row sets it before the tail and name are added, so the result is the same. If a sha moves, compare placed items between the two paths with `help-me-debug`.
- Risk, order and camera: the score box's articles must keep table order (5 bars, then 7 rows) and the alternating `p8_camera` offset per article. The draw loop copies `draw_functions_draw_rows` exactly.
- Risk, message-box slot: the box's slot use is expected to drop (no data-row copies, no cells in its rows). If it grows, apply the copy budget rule above and record it.
- The step graduates with one line in plan section 7: final counts (141 / 0 if H-5b-1 held), the slot and zone peaks that moved, any slot size raised, GAP-25 reworded, and which step-5 hypotheses failed, if any.
