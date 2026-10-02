#ifndef GAME_ECONOMY_FORMULAS_H
#define GAME_ECONOMY_FORMULAS_H

#include "p8/fixed.h"
#include "game/map_tables.h"
#include "engine/vec.h"

/* Throughout this module an absent or wrong-kind field reads as its
 * zero/false default rather than crashing, and a NULL instance is a safe
 * no-op. */

/* `r`'s 13 resources and at most 3 more. */
enum { RESOURCE_MAP_CAP = RESOURCE_CODE_COUNT + 3 };

/* A table keyed by resource (`qe`, floored to an integer as Lua indexes
 * it), entries in insertion order. */
typedef struct {
    int count;
    struct {
        int resource;
        p8_fixed_t value;
    } entries[RESOURCE_MAP_CAP];
} resource_map_t;

/* Returns 1 and writes *out when `qe` is a key, 0 and leaves *out untouched
 * otherwise. */
int resource_map_get(const resource_map_t *map, p8_fixed_t qe, p8_fixed_t *out);
void resource_map_set(resource_map_t *map, p8_fixed_t qe, p8_fixed_t v);

/* Empties `out` first: every `qe` VALUE of `r` (not `r`'s keys) mapped to
 * `jm`. */
void economy_formulas_fill_resource_map(resource_map_t *out, p8_fixed_t jm);

/* `#(to.p-pb.p)` is the SQUARED distance, not the Euclidean one. */
p8_fixed_t economy_formulas_route_base_cost(vec_t from_pos, vec_t to_pos);

/* An absent `cg.jc` counts as zero slipways. */
p8_fixed_t economy_formulas_cost_scale(void);

#endif
