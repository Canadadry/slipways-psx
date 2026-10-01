#ifndef PICO8_PLANET_DISPLAY_H
#define PICO8_PLANET_DISPLAY_H

#include "pico8_research.h"
#include "pico8_struct.h"
#include "p8/gfx.h"
#include "p8/mem.h"
#include "pico8_planet.h"
#include "pico8_vec.h"

/* Throughout this module a NULL instance argument is a safe no-op (Lua nil
 * for the value-returning functions), and an absent or wrong-kind field
 * reads as absent rather than crashing. */

/* Rebinds `self.lq` to a fresh row in a place of its own, and gives the
 * previous row's place back on the spot: nothing reads it any more. */
void pico8_jo_hb(pico8_jo_t *self);

/* Builds its rows, their texts and tuples in the frame zone and hands them
 * to `pz()`, whose buttons copy the rows and their texts. */
void pico8_jo_gh(const pico8_jo_t *self);

/* Answering "no" for the drag's own origin planet also opens that
 * planet's info panel. */
pico8_down_t pico8_jo_gv(const pico8_jo_t *self, const pico8_struct_header_t *s);

/* Every row it builds carries the captured values of its `my` callback --
 * see `pico8_jo_fe_my_dispatch` in the .c. */
void pico8_jo_fe(pico8_jo_t *self);

/* Draws, and mutates `self`: `ma()` decays the halo, and the `fo` flash is
 * cleared once drawn. */
void pico8_jo_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_jo_t *self, pico8_vec_t p);

/* In the `hp.cw` branch `self` is marked for removal and the tail of the
 * work lands on a fresh planet the caller never sees -- or on nothing,
 * when `hp.cw` is neither "pm" nor "ga". */
void pico8_jo_hu(pico8_jo_t *self, pico8_hp_t *hp_instance);

#endif
