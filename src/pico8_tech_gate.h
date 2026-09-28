#ifndef PICO8_TECH_GATE_H
#define PICO8_TECH_GATE_H

#include "pico8_fixed.h"
#include "pico8_list.h"
#include "pico8_map_tables.h"
#include "pico8_ob.h"
#include "pico8_value.h"

/* `px` may be this port's absent-value filler, which reads falsy and so
 * answers true: no prerequisite is an unlocked prerequisite. */
int pico8_oj(pico8_value_t px);

/* Lua's `A and B or C`, three-way: `mc` comes back only when it is itself
 * truthy, so a falsy `mc` still yields `lk` on an unlocked `px`. Not a
 * ternary on oj(px) alone. */
pico8_value_t pico8_ov(pico8_value_t px, pico8_value_t lk, pico8_value_t mc);

/* `t` is a plain nb record, never a class instance. A NULL `t`, or an
 * absent `od`/`nk`, reads as 0. */
pico8_fixed_t pico8_gi(const pico8_ob_table_t *t);

/* The same cost off an entry of `pico8_nb_table`; a NULL `t` costs 0. */
pico8_fixed_t pico8_gi_nb(const pico8_nb_entry_t *t);

/* Writes `e_` onto every nb record it visits, kept or not, before deciding
 * whether to keep it -- a real side effect, not a by-product. Returns a new
 * list the caller owns and must release with pico8_list_free(); the records
 * inside it stay owned by nb. */
pico8_list_t pico8_cb(pico8_value_t bi);

#endif
