#ifndef GAME_RESEARCH_DISPLAY_H
#define GAME_RESEARCH_DISPLAY_H

#include "engine/entity_class.h"
#include "p8/fixed.h"
#include "game/layout.h"
#include "engine/pointer_list.h"
#include "game/research.h"

layout_row_t *research_project_layout_row(research_project_t *self);

const data_row_t *research_project_tooltip(const research_project_t *self);

layout_row_t *research_display_planet_resource_row(const allocator_t *zone, const pointer_list_t *inputs, const p8_fixed_t *output,
                        p8_fixed_t amount, p8_fixed_t used, p8_fixed_t spacing);

typedef struct {
    research_project_id_t items[PROJECT_RECORDS_COUNT];
    int count;
} available_projects_kept_t;

available_projects_kept_t research_display_available_projects(const entity_t *planet_instance);

#endif
