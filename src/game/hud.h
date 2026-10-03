#ifndef GAME_HUD_H
#define GAME_HUD_H

#include "p8/fixed.h"
#include "p8/gfx.h"
#include "p8/mem.h"

/* Fixed-point stays fixed-point across this module's API and is truncated
 * to int only at the p8/gfx draw call. An omitted argument gets its own
 * entry point (_default, _y1) rather than a sentinel value. */

/* Activates palette block flr(no), 0..15, by writing its 16 bytes into the
 * draw palette -- overwriting whatever p8_palt() had set there.
 * Every block makes color 3 transparent. Needs no initialization. The
 * _default variant activates block 0. */
void hud_select_palette_block(p8_memory_t *mem, p8_fixed_t no);
void hud_select_palette_block_default(p8_memory_t *mem);

/* Full-width horizontal bar. An explicit `col` also becomes the current
 * pen color (pen_color of the graphics state), which is what
 * hud_draw_bar_default then draws with -- its one caller is the bottom
 * curtain bar of score_panel_draw(). That write is why the graphics state is
 * not const here. */
void hud_draw_bar(p8_graphics_t *gfx, p8_memory_t *mem, p8_fixed_t y1, p8_fixed_t y2, int col);
void hud_draw_bar_y1(p8_graphics_t *gfx, p8_memory_t *mem, p8_fixed_t y1, int col);
void hud_draw_bar_default(p8_graphics_t *gfx, p8_memory_t *mem, p8_fixed_t y1, p8_fixed_t y2);

void hud_draw_help_frame(const p8_graphics_t *gfx, p8_memory_t *mem, p8_fixed_t x1, p8_fixed_t y1, p8_fixed_t x2);

/* Outlined text: the same string printed at the 8 neighbouring pixels in
 * black, then once at the center in the given color. 0 & color is 0 and
 * 15 & color is the color, which is how one loop over the outline offsets
 * draws both. The last argument is the optional centering factor, NULL
 * meaning omitted: then no shift is computed at all, as in
 * hud_print_outlined_default. */
void hud_print_outlined(const p8_graphics_t *gfx, p8_memory_t *mem, const char *t, p8_fixed_t x, p8_fixed_t y, int c,
              const p8_fixed_t *a);
void hud_print_outlined_default(const p8_graphics_t *gfx, p8_memory_t *mem, const char *t, p8_fixed_t x, p8_fixed_t y,
                       int c);

#endif
