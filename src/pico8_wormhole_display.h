#ifndef PICO8_WORMHOLE_DISPLAY_H
#define PICO8_WORMHOLE_DISPLAY_H

#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_vec.h"
#include "pico8_wormhole.h"

/* TROU-3 */
/* `p` is the struct's `ra`, as `dc("kj")` passes it. A NULL `self` draws
 * nothing; a NULL `gfx` or `mem` skips the drawing calls only. */
void pico8_mz_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_mz_t *self, pico8_vec_t p);

#endif
