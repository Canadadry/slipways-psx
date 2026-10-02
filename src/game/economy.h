#ifndef GAME_ECONOMY_H
#define GAME_ECONOMY_H

#include "game/economy_formulas.h"
#include "p8/fixed.h"
#include "game/main_economy.h"
#include "game/map_tables.h"
#include "engine/entity_class.h"

/* Throughout this module a NULL `self` is a safe no-op. */

extern const economy_archetype_t economy_archetype;

/* Marks the module ready only. The singleton `fm=hy()` is built later, by
 * `eu()` (pico8_cart.c), which hands it to pico8_probe_set_fm(). Calls
 * pico8_economy_shutdown() first. */
void economy_init(void);
void economy_shutdown(void);

/* 1 from pico8_economy_init() to pico8_economy_shutdown(), 0 otherwise. */
int economy_ready(void);
const entity_class_t *economy_class(void);

/* Takes no key. Built in the game's generation (pico8_zone_partie()),
 * never freed. */
economy_t *economy_new(void);

void economy_advance_clock(economy_t *self, p8_fixed_t hx);

void economy_new_year(economy_t *self);

/* Returns 1 once `jt` is paid, 0 when credits were short. A NULL `self`
 * has 0 credits, so any positive `jt` fails. */
int economy_spend(economy_t *self, p8_fixed_t jt);

/* Refills `po`/`fu` in place, so what a caller read through either before
 * the call no longer holds after it. `hl` is BOOL(0), not a string, when
 * neither end-of-game condition holds. */
void economy_recompute(economy_t *self);

/* `t` is an entry of `pico8_nb_table`, or NULL (no `px`, a cost of 0);
 * its `px` string is appended to `invented`, borrowed. */
void economy_invent(economy_t *self, const technologies_entry_t *t);

/* Each empties `out` first. `bx`: `cm(0)` plus each producing planet's
 * `kw.mu`; `ck`: `cm(1)` less 0.15 per `qh` for the resource of its last
 * `jy`. */
void economy_total_production(resource_map_t *out);
void economy_crowding_penalty(resource_map_t *out);

#endif
