#ifndef PICO8_ENTITY_H
#define PICO8_ENTITY_H

#include "pico8_list.h"
#include "pico8_ob.h"
#include "pico8_struct.h"
#include "pico8_value.h"

/* (Re)builds hv, cv, cg and ci to a fresh, empty state, freeing the
 * previous build first via pico8_entity_shutdown(), and puts back the
 * placeholder mo. Must run before any of hv, cv, cg, lt, he, dc, mv, ci or
 * mo below is used. pico8_jl() (pico8_bootstrap.c) calls it, then replaces
 * the placeholder mo through pico8_entity_set_mo(). */
void pico8_entity_init(void);

/* Releases this module's own bookkeeping (hv/cv/cg's backing arrays, ci's
 * table); mo is never freed: the placeholder is static, a `qb` struct in
 * its place is dropped (TROU-7). Nor are the entities registered in
 * hv/cv/cg, which stay owned by whoever instantiated them. A second
 * call, or a call before the first pico8_entity_init(), is a no-op. */
void pico8_entity_shutdown(void);

/* Every entity lt()'d and not yet he()'d, in registration order, as
 * PICO8_VALUE_REF values whose .as.ref is a ke-descended struct
 * (pico8_ke.h). */
const pico8_list_t *pico8_hv(void);

/* The entities registered under bm field name `name` ("kj", "ev", "hd",
 * "hk" or "hb"), or NULL if `name` is not one of those five. The five
 * buckets are fixed, so a returned pointer stays valid for the module's
 * whole lifetime. */
const pico8_list_t *pico8_cv(const char *name);

/* The entities registered under `tag`, or NULL if no entity carrying
 * that tag has ever been lt()'d. Unlike cv, cg's storage grows as new
 * tags are first seen: do not hold a returned pointer across a later
 * lt() call, call pico8_cg() again instead. */
const pico8_list_t *pico8_cg(const char *tag);

pico8_struct_header_t *pico8_lt(pico8_struct_header_t *e);
void pico8_he(pico8_struct_header_t *e);

/* `e[name]`: returns 1 and writes *out_value when the struct `e` has a
 * method by that name or a non-nil field; returns 0, leaving *out_value
 * untouched, otherwise. Asserts that `e` is an entity struct: an ob table
 * or a list REF passed as an entity stops the run. */
int pico8_entity_get_field(const void *e, const char *name, pico8_value_t *out_value);

/* `e[name]=value` into the struct's member `name`. Asserts, as
 * pico8_entity_get_field() does, that `e` is an entity struct, and that it
 * has such a member: a struct never grows a field. */
void pico8_entity_set_field(void *e, const char *name, pico8_value_t value);

/* Returns 1 and writes *out_value when `e` is non-NULL and has a field
 * named `method_name` (pico8_entity_get_field()) -- calling it with `e`,
 * `argc`, `arg0` and `arg1` when the field is PICO8_VALUE_FUNCTION-kind,
 * using the field's own value as-is otherwise. Returns 0, leaving
 * *out_value untouched, when `e` is NULL or has no such field. `argc`
 * says how many of arg0/arg1 are real; a caller passes
 * pico8_value_ref(NULL) for the slots it does not cover. */
int pico8_mv(pico8_struct_header_t *e, const char *method_name, int argc, pico8_value_t arg0, pico8_value_t arg1,
             pico8_value_t *out_value);

void pico8_dc(const char *nt);

/* The per-layer blend factors dc()'s `kj` branch indexes into, 1-indexed
 * as in the cart. Owned by this module (built and freed by
 * pico8_entity_init()/_shutdown()); callers must not free it. */
const pico8_ob_table_t *pico8_ci(void);

/* Until pico8_jl() runs, the placeholder struct, with no field and no
 * method, so that dc()'s `mo:md(ci[o])` has an object to dispatch through;
 * afterwards the
 * real `qb` struct (pico8_scroll.h). Either is read and written through
 * pico8_entity_get_field()/pico8_entity_set_field(). Callers must not free
 * it. */
pico8_struct_header_t *pico8_mo(void);

const pico8_struct_class_t *pico8_mo_placeholder_struct_class(void);

/* Makes `instance` what pico8_mo() returns. Does NOT free the outgoing
 * one: pico8_jl() can run from inside a dispatch chain that still holds
 * the old mo, and freeing it there is a use-after-free. NULL is a valid
 * "no mo"; passing back the current pico8_mo() is a no-op. */
void pico8_entity_set_mo(pico8_struct_header_t *instance);

#endif
