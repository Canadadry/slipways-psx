#ifndef ENGINE_ENTITY_GEOM_H
#define ENGINE_ENTITY_GEOM_H

#include "engine/entity_id_list.h"
#include "engine/entity_list.h"
#include "engine/entity_class.h"
#include "engine/vec.h"

int entity_geom_contains_cursor(const entity_t *e, mouse_pos_t mouse);

int entity_geom_is_on_screen(const entity_t *e);

vec_t entity_geom_shake(vec_t p, entity_t *e);

p8_fixed_t entity_geom_nearest(vec_t p, const entity_list_t *j_, entity_t **out_nearest);
/* Skips only `excluded`, by identity: another entity on the very point counts. */
p8_fixed_t entity_geom_nearest_except(vec_t p, const entity_list_t *j_, const entity_t *excluded, entity_t **out_nearest);
p8_fixed_t entity_geom_nearest_in_ids(vec_t p, const entity_id_list_t *ids, entity_t **out_nearest);

#endif
