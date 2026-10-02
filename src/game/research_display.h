#ifndef GAME_RESEARCH_DISPLAY_H
#define GAME_RESEARCH_DISPLAY_H

#include "engine/entity_class.h"
#include "p8/fixed.h"
#include "game/layout.h"
#include "engine/pointer_list.h"
#include "game/research.h"

/* Returns a row in the frame zone, valid until its next reset, or NULL
 * when pico8_jo_ex() does. The two items appended to `items` after
 * construction miss the `fs`/`w` `lq:pq` computed, so `lq:ol` skips them --
 * but `js:pq`'s copy of the row, built from all its items, draws them. */
layout_row_t *research_project_layout_row(research_project_t *self);

/* Returns a row of one line, its texts included, valid until the next
 * reset of the frame zone, or NULL when `self.ii` is truthy. */
const data_row_t *research_project_tooltip(const research_project_t *self);

/* A static function of `jo`, not a method: `jo.ex(...)`, never `jo:ex(...)`.
 * Returns a row in `zone`, or NULL when `o` is NULL (nil), having taken
 * nothing from `zone` -- which `jo:hb` does reach, for a planet whose `kw`
 * carries no `qe`. `o` is read during the call only. Its tuples are in the
 * frame zone: a pico8_lq_copy of the row is only valid in the frame that
 * built it. */
layout_row_t *research_display_planet_resource_row(const allocator_t *zone, const pointer_list_t *inputs, const p8_fixed_t *output,
                        p8_fixed_t amount, p8_fixed_t used, p8_fixed_t spacing);

/* The projects of `gb` that `bj` keeps, in `gb` order. */
typedef struct {
    research_project_t *items[PROJECT_RECORDS_COUNT];
    int count;
} available_projects_kept_t;

/* The Lua parameter is literally named `jo`: a local shadowing the class,
 * naming the planet the projects are offered on. A `jo_instance` without
 * `.nk.px` matches nothing. */
available_projects_kept_t research_display_available_projects(const entity_t *planet_instance);

#endif
