#ifndef PICO8_SLIPWAY_H
#define PICO8_SLIPWAY_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_value.h"

/* Throughout this module a NULL instance argument is a safe no-op (Lua nil
 * for the value-returning functions), and an absent or wrong-kind field
 * reads as absent rather than crashing. */

void pico8_jc_init(void);
void pico8_jc_shutdown(void);

/* NULL before pico8_jc_init(), and again after pico8_jc_shutdown(). */
pico8_class_t *pico8_jc_class(void);

pico8_instance_t *pico8_jc_new(const pico8_ob_table_t *overrides);

/* TROU-7 */
/* Allocates the `ts` and `hg` containers every other method here assumes
 * are already present. */
void pico8_jc_pq(pico8_instance_t *self);

/* TROU-7 */
/* Rebinds `self.re` every frame: the hovered entity when it answers to
 * `gv`, otherwise a fresh classless `{ra,p}` table -- so `self.re` is not
 * always a class instance. Also rebinds `od`, `mt`, `et` and `fa`, and
 * points the `fo` of whatever `ce()` found back at this slipway. */
void pico8_jc_hq(pico8_instance_t *self, pico8_value_t mp);

/* A NULL `re`, or a `d_()` that refuses, only raises `self.p_`; both
 * endpoints are registered on success. */
void pico8_jc_ey(pico8_instance_t *self, pico8_instance_t *re);

/* TROU-7 */
/* Returns the existing slipway to reuse, the first planet or slipway in
 * the way, a fresh `{qs="-too far-"}` table, or nothing. */
pico8_value_t pico8_jc_ce(pico8_instance_t *self);

pico8_value_t pico8_jc_eb(const pico8_instance_t *self, const pico8_instance_t *jo_instance);

void pico8_jc_ev(pico8_instance_t *self);

#endif
