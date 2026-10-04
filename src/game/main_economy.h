#ifndef GAME_MAIN_ECONOMY_H
#define GAME_MAIN_ECONOMY_H

#include "game/economy_formulas.h"
#include "p8/fixed.h"
#include "engine/pointer_list.h"
#include "engine/entity_class.h"

typedef struct {
    p8_fixed_t credits;
    p8_fixed_t era;
    p8_fixed_t year_ticks;
    p8_fixed_t year;
} economy_archetype_t;

typedef struct {
    entity_t header;
    const economy_archetype_t *archetype;
    p8_fixed_t credits;
    p8_fixed_t income;
    p8_fixed_t research_points;
    p8_fixed_t era;
    pointer_list_t *invented;
    p8_fixed_t year_ticks;
    p8_fixed_t year;
    resource_map_t *production;
    resource_map_t *demand;
    resource_map_t production_own;
    resource_map_t demand_own;
    p8_fixed_t happiness;
    int has_happiness;
    const char *end_status;
    int has_end_status;
} economy_t;

typedef struct {
    entity_t header;
    p8_fixed_t research_points;
    int has_research_points;
    p8_fixed_t credits;
    int has_credits;
} main_economy_placeholder_t;

const entity_class_t *main_economy_placeholder_class(void);

entity_t *main_economy(void);
void probe_set_main_economy(entity_t *instance);

economy_t *main_economy_typed(void);

const main_economy_placeholder_t *main_economy_placeholder(void);

#endif
