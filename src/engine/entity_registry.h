#ifndef PICO8_ENTITY_H
#define PICO8_ENTITY_H

#include "engine/entity_list.h"
#include "engine/entity_class.h"

/* (Re)builds hv, cv and cg empty, growing in the current game-zone
 * generation (pico8_zone_partie.h), after pico8_entity_shutdown() forgot
 * the previous build, and puts back the placeholder mo. Must run before
 * any of hv, cv, cg, lt, he, dc or mo below is used. pico8_jl()
 * (pico8_bootstrap.c) calls it, then replaces the placeholder mo through
 * pico8_entity_set_mo(). */
void pico8_entity_init(void);

/* Forgets hv/cv/cg's backing arrays, given back with their game-zone
 * generation; mo is forgotten unread: the placeholder is static, a `qb`
 * struct lives in the game zone. Nor are the entities registered in
 * hv/cv/cg, which stay owned by whoever instantiated them. A second
 * call, or a call before the first pico8_entity_init(), is a no-op. */
void pico8_entity_shutdown(void);

/* Every entity of `hv` whose class has `rendre` goes to the exit queue
 * (pico8_file_sortie.h); `hv` itself is left as is. */
void pico8_entity_abandonner_hv(void);

/* Empties hv/cv/cg without reading their arrays, which a game-zone reset
 * has just poisoned; mo and the initialised state are kept. */
void pico8_entity_oublier_registres(void);

/* Every entity lt()'d and not yet he()'d, in registration order, as
 * pointers to a ke-descended struct (pico8_ke.h). */
const pico8_ents_t *pico8_hv(void);

/* The entities registered under bm field name `name` ("kj", "ev", "hd",
 * "hk" or "hb"), or NULL if `name` is not one of those five. The five
 * buckets are fixed, so a returned pointer stays valid for the module's
 * whole lifetime. */
const pico8_ents_t *pico8_cv(const char *name);

/* The entities registered under `tag`, or NULL if no entity carrying
 * that tag has been lt()'d since pico8_entity_init() -- an unknown tag
 * included. A returned pointer stays valid until the next
 * pico8_entity_init() or pico8_entity_shutdown(). */
const pico8_ents_t *pico8_cg(const char *tag);

/* Only pico8_ke_struct_pq calls it outside tests: every entity of `hv` has
 * had `me` and `t` set by pico8_ke_struct_init, and pico8_dg() reads them. */
entity_t *pico8_lt(entity_t *e);
/* `e` taken out of `hv` here, with a class that has `rendre`, goes to the
 * exit queue: once only, even if he() runs again after jl(). */
void pico8_he(entity_t *e);

/* `nt` is "kj" or "ev" (asserted). */
void pico8_dc(const char *nt);

/* `hd=1` of `bg` and `fq`: the hover test that every point passes. */
int pico8_hd_true(entity_t *self, const pico8_vec_t *qa, const pico8_vec_t *ra);

/* Until pico8_jl() runs, the placeholder struct, with no member and no
 * `md`: dc() calls `mo:md(ci[o])` on a `qb` only; afterwards the
 * real `qb` struct (pico8_scroll.h). Its members are read and written
 * through pico8_mo_qb(). Callers must not free it. */
entity_t *pico8_mo(void);

const entity_class_t *pico8_mo_placeholder_struct_class(void);

/* Makes `instance` what pico8_mo() returns. The outgoing one lives on
 * with its generation: pico8_jl() can run from inside a dispatch chain
 * that still holds the old mo. NULL is a valid
 * "no mo"; passing back the current pico8_mo() is a no-op. */
void pico8_entity_set_mo(entity_t *instance);

#endif
