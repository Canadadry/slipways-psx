#ifndef GAME_DRAW_FUNCTIONS_H
#define GAME_DRAW_FUNCTIONS_H

#include "p8/fixed.h"
#include "p8/gfx.h"
#include "game/map_tables.h"
#include "p8/mem.h"

/* Draws one row from its items, the drawing function's arguments in order. */
typedef void (*draw_functions_fn_t)(const data_row_t *e);

/* Fixed capacity: draw_functions_init() registers five names, and
 * registering a name again replaces its function. A name of 16 characters
 * or more fails an assert instead of being truncated. */
#define DRAW_FUNCTIONS_DISPATCH_CAPACITY 8

void draw_functions_dispatch_register(const char *name, draw_functions_fn_t fn);

/* NULL when `name` was never registered. */
draw_functions_fn_t draw_functions_dispatch_lookup(const char *name);

/* Empties the table. */
void draw_functions_dispatch_clear(void);

/* Draws the rows of the array in order, each by the function its
 * `draw_function` names. A row with no draw_function, or one naming an
 * unregistered function, draws nothing.
 *
 * draw_functions_draw_rows() sets the camera x to the shake offset before
 * each row and negates the offset after it; the _default variant leaves the
 * camera alone. The graphics state and PICO-8 memory are borrowed, and also
 * kept in this module's dispatch context for the drawing functions to read. */
void draw_functions_draw_rows(p8_graphics_t *gfx, p8_memory_t *mem, const data_row_t *const *rb, int n, p8_fixed_t nc);
void draw_functions_draw_rows_default(p8_graphics_t *gfx, p8_memory_t *mem, const data_row_t *const *rb, int n);

/* Fills the table; calling it again changes nothing. */
void draw_functions_init(void);

#endif
