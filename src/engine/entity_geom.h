#ifndef PICO8_ENTITY_GEOM_H
#define PICO8_ENTITY_GEOM_H

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
int pico8_gk(const pico8_struct_header_t *e, const pico8_vec_t *mp, const pico8_vec_t *rp);

int pico8_gd(const pico8_struct_header_t *e);

/* `e` is NULL or carries `ma` (asserted): a planet or the camera. Decays
 * `e.ma` in place. */
pico8_vec_t pico8_ma(pico8_vec_t p, pico8_struct_header_t *e);

/* Reads every element of `j_` as an entity instance. `*out_nearest` is
 * NULL -- Lua's nil -- when `j_` is NULL, empty, or holds
 * nothing away from `p`; the returned distance is then 32767. */
pico8_fixed_t pico8_ij_ents(pico8_vec_t p, const pico8_ents_t *j_, pico8_struct_header_t **out_nearest);

#endif
