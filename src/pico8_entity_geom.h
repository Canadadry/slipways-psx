#ifndef PICO8_ENTITY_GEOM_H
#define PICO8_ENTITY_GEOM_H

#include "pico8_ents.h"
#include "pico8_struct.h"
#include "pico8_vec.h"

/* A vector-valued entity field -- `ra`, `p`, and `kq`'s box -- is a
 * pointer to a heap vector the field borrows and never owns.
 * Throughout this module a NULL instance, an absent field or a wrong-kind
 * field reads as the zero vector, as 0, or as a false hit test. */
/* TROU-15 */

/* `gk`'s `e` and `na`'s `self` are NULL or a struct (asserted); `gk`'s
 * NULL `mp`/`rp` reads as the zero vector. */
int pico8_gk(const pico8_struct_header_t *e, const pico8_vec_t *mp, const pico8_vec_t *rp);

int pico8_gd(const pico8_struct_header_t *e);

/* Mutates the pico8_vec_t `self.ra` points at, in place: every other holder
 * of that vector -- the `iv` toast `jo:hk` builds from `self.ra` -- sees the
 * bob. Rebinding the field to a fresh vector, as `qb:kj` does, would not. */
void pico8_na(pico8_struct_header_t *self, pico8_fixed_t t);

/* `e` is NULL or carries `ma` (asserted): a planet or the camera. Decays
 * `e.ma` in place. */
pico8_vec_t pico8_ma(pico8_vec_t p, pico8_struct_header_t *e);

/* Reads every element of `j_` as an entity instance. `*out_nearest` is
 * NULL -- Lua's nil -- when `j_` is NULL, empty, or holds
 * nothing away from `p`; the returned distance is then 32767. */
pico8_fixed_t pico8_ij_ents(pico8_vec_t p, const pico8_ents_t *j_, pico8_struct_header_t **out_nearest);

#endif
