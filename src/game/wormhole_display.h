#ifndef PICO8_WORMHOLE_DISPLAY_H
#define PICO8_WORMHOLE_DISPLAY_H

#include "p8/gfx.h"
#include "p8/mem.h"
#include "engine/vec.h"
#include "game/wormhole.h"

/* TROU-3 */
/* `p` is the struct's `ra`, as `dc("kj")` passes it. A NULL `self` draws
 * nothing; a NULL `gfx` or `mem` skips the drawing calls only. */
void pico8_mz_kj(p8_graphics_t *gfx, p8_memory_t *mem, pico8_mz_t *self, vec_t p);

#endif
