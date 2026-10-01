#ifndef PICO8_ECONOMY_H
#define PICO8_ECONOMY_H

#include "game/economy_formulas.h"
#include "p8/fixed.h"
#include "game/main_economy.h"
#include "game/map_tables.h"
#include "engine/entity_class.h"

/* Throughout this module a NULL `self` is a safe no-op. */

extern const pico8_hy_def_t pico8_hy_def;

/* Marks the module ready only. The singleton `fm=hy()` is built later, by
 * `eu()` (pico8_cart.c), which hands it to pico8_probe_set_fm(). Calls
 * pico8_economy_shutdown() first. */
void pico8_economy_init(void);
void pico8_economy_shutdown(void);

/* 1 from pico8_economy_init() to pico8_economy_shutdown(), 0 otherwise. */
int pico8_hy_ready(void);
const pico8_struct_class_t *pico8_hy_struct_class(void);

/* Takes no key. Built in the game's generation (pico8_zone_partie()),
 * never freed. */
pico8_hy_t *pico8_hy_new(void);

void pico8_hy_nx(pico8_hy_t *self, pico8_fixed_t hx);

void pico8_hy_hk(pico8_hy_t *self);

/* Returns 1 once `jt` is paid, 0 when credits were short. A NULL `self`
 * has 0 credits, so any positive `jt` fails. */
int pico8_hy_lu(pico8_hy_t *self, pico8_fixed_t jt);

/* Refills `po`/`fu` in place, so what a caller read through either before
 * the call no longer holds after it. `hl` is BOOL(0), not a string, when
 * neither end-of-game condition holds. */
void pico8_hy_lj(pico8_hy_t *self);

/* `t` is an entry of `pico8_nb_table`, or NULL (no `px`, a cost of 0);
 * its `px` string is appended to `invented`, borrowed. */
void pico8_hy_kt(pico8_hy_t *self, const pico8_nb_entry_t *t);

/* Each empties `out` first. `bx`: `cm(0)` plus each producing planet's
 * `kw.mu`; `ck`: `cm(1)` less 0.15 per `qh` for the resource of its last
 * `jy`. */
void pico8_bx(pico8_qe_map_t *out);
void pico8_ck(pico8_qe_map_t *out);

#endif
