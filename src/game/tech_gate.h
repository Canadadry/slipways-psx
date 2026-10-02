#ifndef PICO8_TECH_GATE_H
#define PICO8_TECH_GATE_H

#include "p8/fixed.h"
#include "game/map_tables.h"

/* A NULL `px` (the cart's nil or false) answers 1: no prerequisite is an
 * unlocked one. */
int pico8_oj(const char *px);

p8_fixed_t pico8_ov(const char *px, p8_fixed_t lk, p8_fixed_t mc);

/* The cost of the tech `t`, an entry of `pico8_nb_table`; a NULL `t` costs
 * 0. */
p8_fixed_t pico8_gi_nb(const pico8_nb_entry_t *t);

/* The entries of `pico8_nb_table` that `cb` keeps, in table order. */
typedef struct {
    const pico8_nb_entry_t *items[PICO8_NB_COUNT];
    int count;
} pico8_cb_kept_t;

/* Writes `e_` for every nb entry it visits, kept or not, before deciding
 * whether to keep it -- a real side effect, not a by-product. */
pico8_cb_kept_t pico8_cb(int bi);

/* `t.e_` as `cb` last wrote it: 1 for true, 0 for false or never written. */
int pico8_nb_e_(const pico8_nb_entry_t *t);

/* Every `e_` back to never written, as the cart's `nb` starts. */
void pico8_nb_e_reset(void);

#endif
