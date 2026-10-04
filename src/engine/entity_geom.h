#ifndef ENGINE_ENTITY_GEOM_H
#define ENGINE_ENTITY_GEOM_H

#include "engine/entity_list.h"
#include "engine/entity_class.h"
#include "engine/vec.h"

/* A vector-valued entity field -- the position, the map position, and the
 * hit box -- is a pointer the entity borrows and never owns. Throughout
 * this module a NULL instance or an absent field reads as the zero vector,
 * as 0, or as a false hit test. */
/* GAP-15 */

/* The entity may be NULL. An entity on layer 10 or above tests the second
 * cursor position, any other the first; a NULL one reads as the zero vector. */
int entity_geom_contains_cursor(const entity_t *e, const vec_t *mp, const vec_t *rp);

/* Under the main camera, within the screen plus a 12-pixel margin on each
 * edge. */
int entity_geom_is_on_screen(const entity_t *e);

/* The entity is NULL or has a shake (asserted): a planet or the camera.
 * Decays that shake in place. */
vec_t entity_geom_shake(vec_t p, entity_t *e);

/* `*out_nearest` is NULL when the list is NULL, empty, or holds nothing
 * away from the point; the returned SQUARED distance is then 32767. */
p8_fixed_t entity_geom_nearest(vec_t p, const entity_list_t *j_, entity_t **out_nearest);

#endif
