#ifndef GAME_ECONOMY_FORMULAS_H
#define GAME_ECONOMY_FORMULAS_H

#include "p8/fixed.h"
#include "game/map_tables.h"
#include "engine/vec.h"

/* Only resource_map_get() accepts a NULL map; resource_map_set() and
 * economy_formulas_fill_resource_map() dereference theirs. */

/* The 13 resource codes and at most 3 more. */
enum { RESOURCE_MAP_CAP = RESOURCE_CODE_COUNT + 3 };

/* A table keyed by resource (the fixed-point key floored to an integer),
 * entries in insertion order. */
typedef struct {
    int count;
    struct {
        int resource;
        p8_fixed_t value;
    } entries[RESOURCE_MAP_CAP];
} resource_map_t;

/* Returns 1 and writes *out when the resource is a key, 0 and leaves *out
 * untouched otherwise. */
int resource_map_get(const resource_map_t *map, p8_fixed_t qe, p8_fixed_t *out);
void resource_map_set(resource_map_t *map, p8_fixed_t qe, p8_fixed_t v);

/* Empties the map first: every resource VALUE of `resource_codes` (not its
 * code letters) mapped to the given value. */
void economy_formulas_fill_resource_map(resource_map_t *out, p8_fixed_t jm);

/* Built on the SQUARED distance between the two positions, not the
 * Euclidean one. */
p8_fixed_t economy_formulas_route_base_cost(vec_t from_pos, vec_t to_pos);

/* An absent slipway tag counts as zero slipways. */
p8_fixed_t economy_formulas_cost_scale(void);

#endif
