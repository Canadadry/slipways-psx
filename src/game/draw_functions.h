#ifndef GAME_DRAW_FUNCTIONS_H
#define GAME_DRAW_FUNCTIONS_H

#include "p8/fixed.h"
#include "p8/gfx.h"
#include "game/map_tables.h"
#include "p8/vram.h"

typedef void (*draw_functions_fn_t)(const data_row_t *e);

void draw_functions_draw_row(p8_graphics_t *gfx, p8_vram_t *mem, const data_row_t *row);
void draw_functions_draw_rows_default(p8_graphics_t *gfx, p8_vram_t *mem, const data_row_t *const *rb, int n);

#endif
