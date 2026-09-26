#ifndef PICO8_PLANET_FAMILY_H
#define PICO8_PLANET_FAMILY_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_planet.h"
#include "pico8_struct.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* Throughout this module -- this file and its `_trade` sibling -- a NULL
 * instance, gfx or mem argument is a safe no-op, and an absent or
 * wrong-kind field reads as its zero/false default rather than crashing. */

/* TROU-8 */
/* Calls `pico8_pi_init()` first: the class specs contain `v(1,0)` and
 * `b(0,0,-1,-1)`, so the 'v'/'b' constructors must already be registered
 * when the spec strings are parsed. */
void pico8_planet_family_init(void);

/* Frees the six classes, never any instance. Idempotent, and safe before
 * the first `pico8_planet_family_init()`. */
void pico8_planet_family_shutdown(void);

/* A laboratory: a bare planet, then qh's own defaults and `et`, which its
 * own `hq` (`bv`) writes. A default of qh whose name is a member of `jo` or
 * `ke` (`me`, `nu`, `hf`, `mi`) lives in that member, never in a second
 * copy here. `nu`, `px` and `ba` hold the very tables of qh's class,
 * borrowed. Nothing frees the struct: TROU-7. */
typedef struct {
    pico8_jo_t jo;
    pico8_value_t hp;
    pico8_value_t px;
    pico8_value_t dq;
    pico8_value_t qh;
    pico8_value_t od;
    pico8_value_t fp;
    pico8_value_t dw;
    pico8_value_t s;
    pico8_value_t ba;
    pico8_value_t et;
} pico8_qh_t;

/* Every accessor below is NULL before `pico8_planet_family_init()` and
 * after `pico8_planet_family_shutdown()`. `qh`'s class instantiation builds
 * a pico8_qh_t, while its descendants still inherit the class's defaults
 * and methods. */
pico8_class_t *pico8_qh_class(void);
const pico8_struct_class_t *pico8_qh_struct_class(void);

/* `overrides` (NULL means none) is read, not kept: `id` only. The new
 * laboratory registers itself into `hv`/`cv`/`cg` through `lt`, and nothing
 * ever frees it. Requires `pico8_planet_family_init()`. */
pico8_qh_t *pico8_qh_new(const pico8_ob_table_t *overrides);

/* Each `_new` below requires `pico8_planet_family_init()`. `overrides` is
 * copied, not kept; the new instance registers itself into `hv`/`cv`/`cg`
 * through `lt`, and nothing ever frees it. */

pico8_class_t *pico8_nl_class(void);
pico8_instance_t *pico8_nl_new(const pico8_ob_table_t *overrides);

pico8_class_t *pico8_os_class(void);
pico8_instance_t *pico8_os_new(const pico8_ob_table_t *overrides);

pico8_class_t *pico8_hj_class(void);
pico8_instance_t *pico8_hj_new(const pico8_ob_table_t *overrides);

/* `pm` and `ga` are the only classes here whose spec sets `fz`, so they are
 * also registered under that name for `pico8_class_lookup_named()` -- the
 * two targets `jo:hu`'s dynamic reclass can resolve. */
pico8_class_t *pico8_pm_class(void);
pico8_instance_t *pico8_pm_new(const pico8_ob_table_t *overrides);

pico8_class_t *pico8_ga_class(void);
pico8_instance_t *pico8_ga_new(const pico8_ob_table_t *overrides);

void pico8_nl_ly(pico8_instance_t *self, pico8_fixed_t t);

void pico8_hj_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self, pico8_vec_t p);

/* TROU-7 */
void pico8_pm_pq(pico8_instance_t *self);

void pico8_ga_pq(pico8_instance_t *self);

/* TROU-7 */
/* Returns a fresh particle record: a REF to a heap table holding heap
 * vectors, which `ez` stores into `self.qy`. */
pico8_value_t pico8_ga_mm(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1);

void pico8_ga_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self, pico8_vec_t p);

#endif
