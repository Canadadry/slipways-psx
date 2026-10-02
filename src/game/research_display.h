#ifndef GAME_RESEARCH_DISPLAY_H
#define GAME_RESEARCH_DISPLAY_H

#include "engine/entity_class.h"
#include "p8/fixed.h"
#include "game/layout.h"
#include "engine/pointer_list.h"
#include "game/research.h"

/* Returns a row in the frame zone, valid until its next reset, or NULL
 * when research_display_planet_resource_row() does. The two items appended
 * after construction miss the `placed_items`/`width` the row's construction
 * computed, so its draw_at skips them -- but a button's copy of the row,
 * built from all its items, draws them. */
layout_row_t *research_project_layout_row(research_project_t *self);

/* Returns a row of one line, its texts included, valid until the next
 * reset of the frame zone, or NULL when the project has an `icon_sprite`. */
const data_row_t *research_project_tooltip(const research_project_t *self);

/* Returns a row in `zone`, or NULL when the output is NULL, having taken
 * nothing from `zone` -- which a planet's refresh does reach, for a planet
 * whose production carries no resource. The output is read during the call
 * only. Its tuples are in the frame zone: a layout_row_copy() of the row is
 * only valid in the frame that built it. */
layout_row_t *research_display_planet_resource_row(const allocator_t *zone, const pointer_list_t *inputs, const p8_fixed_t *output,
                        p8_fixed_t amount, p8_fixed_t used, p8_fixed_t spacing);

/* The projects research_display_available_projects() keeps, in rank order. */
typedef struct {
    research_project_t *items[PROJECT_RECORDS_COUNT];
    int count;
} available_projects_kept_t;

/* The projects offered on that planet. A NULL planet, or one without a
 * planet type name, matches nothing. */
available_projects_kept_t research_display_available_projects(const entity_t *planet_instance);

#endif
