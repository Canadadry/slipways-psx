#ifndef GAME_TECH_GATE_H
#define GAME_TECH_GATE_H

#include "p8/fixed.h"
#include "game/map_tables.h"

/* A NULL `px` (the cart's nil or false) answers 1: no prerequisite is an
 * unlocked one. */
int tech_gate_is_unlocked(const char *px);

p8_fixed_t tech_gate_if_unlocked(const char *px, p8_fixed_t lk, p8_fixed_t mc);

/* The cost of the tech `t`, an entry of `pico8_nb_table`; a NULL `t` costs
 * 0. */
p8_fixed_t tech_gate_research_cost(const technologies_entry_t *t);

/* The entries of `pico8_nb_table` that `cb` keeps, in table order. */
typedef struct {
    const technologies_entry_t *items[TECHNOLOGIES_COUNT];
    int count;
} available_technologies_kept_t;

/* Writes `e_` for every nb entry it visits, kept or not, before deciding
 * whether to keep it -- a real side effect, not a by-product. */
available_technologies_kept_t tech_gate_available_technologies(int bi);

/* `t.e_` as `cb` last wrote it: 1 for true, 0 for false or never written. */
int tech_gate_affordable_flags(const technologies_entry_t *t);

/* Every `e_` back to never written, as the cart's `nb` starts. */
void tech_gate_reset_affordable_flags(void);

#endif
