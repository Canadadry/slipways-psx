#ifndef PICO8_ECONOMY_TIERS_H
#define PICO8_ECONOMY_TIERS_H

#include "pico8_class.h"
#include "pico8_fixed.h"

/* Throughout this module an absent or wrong-kind field reads as its
 * zero/false default rather than crashing, and a NULL instance is a safe
 * no-op. */

/* Each planet's `cc` is a heap list this function owns: the one a previous
 * call installed is freed before the new one replaces it. `by` is not
 * copied, it is whatever `pt:ib()` returned. */
void pico8_bd(void);

/* Returns 1 when at least one planet's tier ROSE this pass, which is what
 * ends `hy:lj()`'s `repeat bd() until not bb()` loop. */
int pico8_bb(void);

/* Writes the match count back onto each want's `gc` field: the side effect
 * is what `dh`'s own later branches, and `jo`'s display, read. */
pico8_fixed_t pico8_dh(const pico8_instance_t *pt);

#endif
