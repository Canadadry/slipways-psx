#ifndef GAME_WORMHOLE_DISPLAY_H
#define GAME_WORMHOLE_DISPLAY_H

#include "p8/gfx.h"
#include "p8/mem.h"
#include "engine/vec.h"
#include "game/wormhole.h"

/* TROU-3 */
/* screen_pos is the struct's pos, as the draw dispatch passes it. A NULL
 * `self` draws nothing; NULL graphics or memory skips the drawing calls
 * only. */
void wormhole_draw(p8_graphics_t *graphics, p8_memory_t *memory, wormhole_t *self, vec_t screen_pos);

#endif
