#ifndef ENGINE_ENTITY_GEOM_H
#define ENGINE_ENTITY_GEOM_H

#include "engine/entity_list.h"
#include "engine/entity_class.h"
#include "engine/vec.h"

/* GAP-15 */

int entity_geom_contains_cursor(const entity_t *e, const vec_t *mp, const vec_t *rp);

int entity_geom_is_on_screen(const entity_t *e);

vec_t entity_geom_shake(vec_t p, entity_t *e);

p8_fixed_t entity_geom_nearest(vec_t p, const entity_list_t *j_, entity_t **out_nearest);

#endif
