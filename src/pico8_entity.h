#ifndef PICO8_ENTITY_H
#define PICO8_ENTITY_H

#include "pico8_ke.h"
#include "pico8_ptrs.h"
#include "pico8_scroll.h"
#include "pico8_struct.h"

/* (Re)builds hv, cv and cg to a fresh, empty state, freeing the
 * previous build first via pico8_entity_shutdown(), and puts back the
 * placeholder mo. Must run before any of hv, cv, cg, lt, he, dc or
 * mo below is used. pico8_jl() (pico8_bootstrap.c) calls it, then replaces
 * the placeholder mo through pico8_entity_set_mo(). */
void pico8_entity_init(void);

/* Releases this module's own bookkeeping (hv/cv/cg's backing arrays); mo
 * is never freed: the placeholder is static, a `qb` struct in
 * its place is dropped (TROU-7). Nor are the entities registered in
 * hv/cv/cg, which stay owned by whoever instantiated them. A second
 * call, or a call before the first pico8_entity_init(), is a no-op. */
void pico8_entity_shutdown(void);

/* Every entity lt()'d and not yet he()'d, in registration order, as
 * pointers to a ke-descended struct (pico8_ke.h). */
const pico8_ptrs_t *pico8_hv(void);

/* The entities registered under bm field name `name` ("kj", "ev", "hd",
 * "hk" or "hb"), or NULL if `name` is not one of those five. The five
 * buckets are fixed, so a returned pointer stays valid for the module's
 * whole lifetime. */
const pico8_ptrs_t *pico8_cv(const char *name);

/* The entities registered under `tag`, or NULL if no entity carrying
 * that tag has ever been lt()'d. Unlike cv, cg's storage grows as new
 * tags are first seen: do not hold a returned pointer across a later
 * lt() call, call pico8_cg() again instead. */
const pico8_ptrs_t *pico8_cg(const char *tag);

/* Only a `ke` enters `hv`: pico8_dg() reads `me` and `t` on every entity of it. */
pico8_struct_header_t *pico8_lt(pico8_ke_t *ke);
void pico8_he(pico8_struct_header_t *e);

/* `nt` is "kj" or "ev" (asserted). */
void pico8_dc(const char *nt);

/* `hd=1` of `bg` and `fq`: the hover test that every point passes. */
int pico8_hd_true(void *self, const pico8_vec_t *qa, const pico8_vec_t *ra);

/* Until pico8_jl() runs, the placeholder struct, with no member and no
 * `md`: dc() calls `mo:md(ci[o])` on a `qb` only; afterwards the
 * real `qb` struct (pico8_scroll.h). Its members are read and written
 * through pico8_mo_qb(). Callers must not free it. */
pico8_struct_header_t *pico8_mo(void);

/* `mo` as its `qb`: NULL when there is no mo and on the placeholder. */
pico8_qb_t *pico8_mo_qb(void);

const pico8_struct_class_t *pico8_mo_placeholder_struct_class(void);

/* Makes `instance` what pico8_mo() returns. Does NOT free the outgoing
 * one: pico8_jl() can run from inside a dispatch chain that still holds
 * the old mo, and freeing it there is a use-after-free. NULL is a valid
 * "no mo"; passing back the current pico8_mo() is a no-op. */
void pico8_entity_set_mo(pico8_struct_header_t *instance);

#endif
