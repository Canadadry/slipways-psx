#ifndef GAME_DRAW_FUNCTIONS_H
#define GAME_DRAW_FUNCTIONS_H

#include "p8/fixed.h"
#include "p8/gfx.h"
#include "game/map_tables.h"
#include "p8/mem.h"

/* Draws article `e` of `bl()` from its items, `e[1]..e[5]` in the cart. */
typedef void (*draw_functions_fn_t)(const data_row_t *e);

/* Fixed capacity: every registration happens once, from pico8_pi_init().
 * A name of 8 characters or more would be truncated on the way in and
 * never found again by pico8_pi_dispatch_lookup(). */
#define DRAW_FUNCTIONS_DISPATCH_CAPACITY 8

void draw_functions_dispatch_register(const char *name, draw_functions_fn_t fn);

/* NULL when `name` was never registered. */
draw_functions_fn_t draw_functions_dispatch_lookup(const char *name);

/* Test-support only (no cart equivalent): resets the table to empty. */
void draw_functions_dispatch_clear(void);

/* Draws the `n` articles of `rb` in order, each by the function its `fn`
 * names. An article with no `fn`, or one naming an unregistered function,
 * draws nothing.
 *
 * draw_functions_draw_rows_default is the `nc == nil` case, where the camera-shake branch
 * never runs at all. `gfx`/`mem` are borrowed, and also stashed in this
 * module's dispatch context for the drawing functions to read. */
void draw_functions_draw_rows(p8_graphics_t *gfx, p8_memory_t *mem, const data_row_t *const *rb, int n, p8_fixed_t nc);
void draw_functions_draw_rows_default(p8_graphics_t *gfx, p8_memory_t *mem, const data_row_t *const *rb, int n);

/* Fills the registry; idempotent.
 *
 * "ld" dispatches to lq:ol, so pico8_layout_init() must have run before
 * that entry is exercised. */
void draw_functions_init(void);

#endif
