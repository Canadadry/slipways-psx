#ifndef PICO8_TECH_GATE_H
#define PICO8_TECH_GATE_H

#include "pico8_fixed.h"
#include "pico8_list.h"
#include "pico8_map_tables.h"
#include "pico8_value.h"

/* `px` may be this port's absent-value filler, which reads falsy and so
 * answers true: no prerequisite is an unlocked prerequisite. */
int pico8_oj(pico8_value_t px);

/* Lua's `A and B or C`, three-way: `mc` comes back only when it is itself
 * truthy, so a falsy `mc` still yields `lk` on an unlocked `px`. Not a
 * ternary on oj(px) alone. */
pico8_value_t pico8_ov(pico8_value_t px, pico8_value_t lk, pico8_value_t mc);

/* The cost of the tech `t`, an entry of `pico8_nb_table`; a NULL `t` costs
 * 0. */
pico8_fixed_t pico8_gi_nb(const pico8_nb_entry_t *t);

/* Writes `e_` for every nb entry it visits, kept or not, before deciding
 * whether to keep it -- a real side effect, not a by-product. Returns a new
 * list the caller owns and must release with pico8_list_free(), of REFs to
 * entries of `pico8_nb_table`. */
pico8_list_t pico8_cb(pico8_value_t bi);

/* `t.e_` as `cb` last wrote it: 1 for true, 0 for false or never written. */
int pico8_nb_e_(const pico8_nb_entry_t *t);

/* Every `e_` back to never written, as the cart's `nb` starts. */
void pico8_nb_e_reset(void);

#endif
