#ifndef PICO8_ECONOMY_H
#define PICO8_ECONOMY_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_ob.h"
#include "pico8_struct.h"
#include "pico8_value.h"

/* Throughout this module a NULL `self` is a safe no-op, and an absent or
 * wrong-kind field reads as its zero/false default rather than crashing. */

/* The economy, one per game. Nothing here frees it: pico8_probe_set_fm()
 * and pico8_probe_shutdown() free the struct alone, never `invented`, `po`
 * nor `fu` (TROU-7). `hl` is absent before the first pico8_hy_lj(). */
typedef struct {
    pico8_struct_header_t header;
    pico8_value_t nz;
    pico8_value_t ge;
    pico8_value_t jk;
    pico8_value_t ew;
    pico8_value_t invented;
    pico8_value_t lo;
    pico8_value_t yr;
    pico8_value_t po;
    pico8_value_t fu;
    pico8_value_t happiness;
    pico8_value_t hl;
} pico8_hy_t;

/* Parses the `hy` spec only. The singleton `fm=hy()` is built later, by
 * `eu()` (pico8_cart.c), which hands it to pico8_probe_set_fm(). Calls
 * pico8_economy_shutdown() first. */
void pico8_economy_init(void);
void pico8_economy_shutdown(void);

/* 1 from pico8_economy_init() to pico8_economy_shutdown(), 0 otherwise. */
int pico8_hy_ready(void);

/* `hy`'s spec default `name`; 0 when absent, or when not ready. A REF is
 * borrowed from the spec, valid until pico8_economy_shutdown(). */
int pico8_hy_default(const char *name, pico8_value_t *out_value);
const pico8_struct_class_t *pico8_hy_struct_class(void);

/* Takes no key. The caller owns the struct until pico8_probe_set_fm()
 * takes it. */
pico8_hy_t *pico8_hy_new(const pico8_ob_table_t *overrides);

void pico8_hy_nx(pico8_hy_t *self, pico8_fixed_t hx);

void pico8_hy_hk(pico8_hy_t *self);

/* Returns 1 once `jt` is paid, 0 when credits were short. A NULL `self`
 * has 0 credits, so any positive `jt` fails. */
int pico8_hy_lu(pico8_hy_t *self, pico8_fixed_t jt);

/* Frees the `po`/`fu` tables its own previous call installed, so no caller
 * may hold either pointer across a call. `hl` is BOOL(0), not a string,
 * when neither end-of-game condition holds. */
void pico8_hy_lj(pico8_hy_t *self);

/* `t` is only read. The `px` value appended to `invented` is borrowed from
 * it, never copied: `t` must outlive the struct. */
void pico8_hy_kt(pico8_hy_t *self, const pico8_ob_table_t *t);

#endif
