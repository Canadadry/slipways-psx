#ifndef GAME_ECONOMY_H
#define GAME_ECONOMY_H

#include "game/economy_formulas.h"
#include "p8/fixed.h"
#include "game/main_economy.h"
#include "game/map_tables.h"
#include "engine/entity_class.h"

/* Every function here that takes `self` treats a NULL `self` as a no-op,
 * except economy_spend(), below. */

extern const economy_archetype_t economy_archetype;

/* Marks the module ready only. The game's economy is built later, by
 * cart_start_game(), which hands it to probe_set_main_economy(). Calls
 * economy_shutdown() first. */
void economy_init(void);
void economy_shutdown(void);

/* 1 from economy_init() to economy_shutdown(), 0 otherwise. */
int economy_ready(void);
const entity_class_t *economy_class(void);

/* Takes no key. Built in the game zone (game_zone()), never freed. */
economy_t *economy_new(void);

void economy_advance_clock(economy_t *self, p8_fixed_t hx);

void economy_new_year(economy_t *self);

/* Returns 1 once the cost is paid, 0 when credits were short. A NULL `self`
 * has 0 credits, so any positive cost fails and shows "-no money-". */
int economy_spend(economy_t *self, p8_fixed_t jt);

/* Refills `production`/`demand` in place, so what a caller read through
 * either before the call no longer holds after it. `end_status` is NULL,
 * with `has_end_status` 1, when neither end-of-game condition holds. */
void economy_recompute(economy_t *self);

/* The technology is an entry of `technologies_table`, or NULL (no name, a
 * cost of 0); its name is appended to `invented`, borrowed. */
void economy_invent(economy_t *self, const technologies_entry_t *t);

/* Each empties the map first. economy_total_production(): every resource
 * at 0, plus each producing planet's production amount;
 * economy_crowding_penalty(): every resource at 1, less 0.15 per structure
 * for the resource of its last need. */
void economy_total_production(resource_map_t *out);
void economy_crowding_penalty(resource_map_t *out);

#endif
