#ifndef PICO8_WORMHOLE_DISPLAY_H
#define PICO8_WORMHOLE_DISPLAY_H

#include "pico8_class.h"
#include "pico8_gfx.h"
#include "pico8_mem.h"
#include "pico8_vec.h"

/* TROU-3 */
/* `p` is the entity's `ra`, as `dc("kj")` passes it. A NULL `self` draws
 * nothing; a NULL `gfx` or `mem` skips the drawing calls only. */
void pico8_mz_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self, pico8_vec_t p);

#endif
