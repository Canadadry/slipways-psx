#ifndef PICO8_ECONOMY_H
#define PICO8_ECONOMY_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_ob.h"
#include "pico8_value.h"

/* Throughout this module a NULL `self` is a safe no-op, and an absent or
 * wrong-kind field reads as its zero/false default rather than crashing. */

/* Builds the `hy` class only. The singleton `fm=hy()` is built later, by
 * `eu()` (pico8_cart.c), which hands it to pico8_probe_set_fm(). Calls
 * pico8_economy_shutdown() first. */
void pico8_economy_init(void);
void pico8_economy_shutdown(void);

/* NULL before pico8_economy_init() and after pico8_economy_shutdown(). */
pico8_class_t *pico8_hy_class(void);

/* The caller owns the instance; pico8_probe_set_fm() frees the previous
 * `fm` when it takes the new one. */
pico8_instance_t *pico8_hy_new(const pico8_ob_table_t *overrides);

void pico8_hy_nx(pico8_instance_t *self, pico8_fixed_t hx);

void pico8_hy_hk(pico8_instance_t *self);

/* Returns 1 once `jt` is paid, 0 when credits were short. A NULL `self`
 * has 0 credits, so any positive `jt` fails. */
int pico8_hy_lu(pico8_instance_t *self, pico8_fixed_t jt);

/* Frees the `po`/`fu` tables its own previous call installed, so no caller
 * may hold either pointer across a call. `hl` is BOOL(0), not a string,
 * when neither end-of-game condition holds. */
void pico8_hy_lj(pico8_instance_t *self);

/* `t` is only read. The `px` value appended to `invented` is borrowed from
 * it, never copied: `t` must outlive the instance. */
void pico8_hy_kt(pico8_instance_t *self, const pico8_ob_table_t *t);

#endif
