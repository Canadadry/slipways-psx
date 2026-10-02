#ifndef GAME_TECH_GATE_H
#define GAME_TECH_GATE_H

#include "p8/fixed.h"
#include "game/map_tables.h"

/* A NULL technology name answers 1: no prerequisite is an unlocked one. */
int tech_gate_is_unlocked(const char *px);

p8_fixed_t tech_gate_if_unlocked(const char *px, p8_fixed_t lk, p8_fixed_t mc);

/* The cost of a technology, an entry of `technologies_table`; NULL costs
 * 0. */
p8_fixed_t tech_gate_research_cost(const technologies_entry_t *t);

/* The entries of `technologies_table` that
 * tech_gate_available_technologies() keeps, in table order. */
typedef struct {
    const technologies_entry_t *items[TECHNOLOGIES_COUNT];
    int count;
} available_technologies_kept_t;

/* Writes the affordability flag of every entry it visits, kept or not,
 * before deciding whether to keep it -- a real side effect, not a
 * by-product. */
available_technologies_kept_t tech_gate_available_technologies(int bi);

/* The entry's flag as tech_gate_available_technologies() last wrote it: 1
 * when affordable, 0 when not or never written. */
int tech_gate_affordable_flags(const technologies_entry_t *t);

/* Every flag back to never written, as data_init() leaves them. */
void tech_gate_reset_affordable_flags(void);

#endif
