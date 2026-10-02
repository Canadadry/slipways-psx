#ifndef ENGINE_ENTITY_GEOM_H
#define ENGINE_ENTITY_GEOM_H

#include "engine/entity_list.h"
#include "engine/entity_class.h"
#include "engine/vec.h"

/* A vector-valued entity field -- `ra`, `p`, and `kq`'s box -- is a
 * pointer to a vector the field borrows and never owns.
 * Throughout this module a NULL instance, an absent field or a wrong-kind
 * field reads as the zero vector, as 0, or as a false hit test. */
/* TROU-15 */

/* `gk`'s `e` is NULL or a struct (asserted); its NULL `mp`/`rp` reads as
 * the zero vector. */
int entity_geom_contains_cursor(const entity_t *e, const vec_t *mp, const vec_t *rp);

int entity_geom_is_on_screen(const entity_t *e);

/* `e` is NULL or carries `ma` (asserted): a planet or the camera. Decays
 * `e.ma` in place. */
vec_t entity_geom_shake(vec_t p, entity_t *e);

/* Reads every element of `j_` as an entity instance. `*out_nearest` is
 * NULL -- Lua's nil -- when `j_` is NULL, empty, or holds
 * nothing away from `p`; the returned distance is then 32767. */
p8_fixed_t entity_geom_nearest(vec_t p, const entity_list_t *j_, entity_t **out_nearest);

#endif
