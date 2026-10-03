#ifndef GAME_DRAW_FUNCTIONS_H
#define GAME_DRAW_FUNCTIONS_H

#include "p8/fixed.h"
#include "p8/gfx.h"
#include "game/map_tables.h"
#include "p8/mem.h"

/* Draws one row from its items, the drawing function's arguments in order. */
typedef void (*draw_functions_fn_t)(const data_row_t *e);

/* Draws the rows of the array in order, each by the function its
 * `draw_function` names. A row whose draw_function is DRAW_FUNCTION_NONE
 * draws nothing.
 *
 * draw_functions_draw_rows() sets the camera x to the shake offset before
 * each row and negates the offset after it; the _default variant leaves the
 * camera alone. The graphics state and PICO-8 memory are borrowed, and also
 * kept in this module's dispatch context for the drawing functions to read. */
void draw_functions_draw_rows(p8_graphics_t *gfx, p8_memory_t *mem, const data_row_t *const *rb, int n, p8_fixed_t nc);
void draw_functions_draw_rows_default(p8_graphics_t *gfx, p8_memory_t *mem, const data_row_t *const *rb, int n);

#endif
