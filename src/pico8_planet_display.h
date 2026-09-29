#ifndef PICO8_PLANET_DISPLAY_H
#define PICO8_PLANET_DISPLAY_H

#include "pico8_struct.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_planet.h"
#include "pico8_vec.h"

/* Throughout this module a NULL instance argument is a safe no-op (Lua nil
 * for the value-returning functions), and an absent or wrong-kind field
 * reads as absent rather than crashing. */

/* TROU-7 */
/* Rebinds `self.lq` to a fresh row; the previous one is dropped. */
void pico8_jo_hb(pico8_jo_t *self);

/* TROU-7 */
/* Hands the rows it builds to `pz()`, which keeps them: only the scratch
 * lists that never reach a row are freed here. */
void pico8_jo_gh(const pico8_jo_t *self);

/* Answering "no" for the drag's own origin planet also opens that
 * planet's info panel. */
pico8_down_t pico8_jo_gv(const pico8_jo_t *self, const pico8_struct_header_t *s);

/* TROU-7 */
/* Every row it builds carries the captured values of its `my` callback --
 * see `pico8_jo_fe_my_dispatch` in the .c. */
void pico8_jo_fe(pico8_jo_t *self);

/* Draws, and mutates `self`: `ma()` decays the halo, and the `fo` flash is
 * cleared once drawn. */
void pico8_jo_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_jo_t *self, pico8_vec_t p);

#endif
