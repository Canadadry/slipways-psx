#ifndef PLATFORM_PSX_TEXT_H
#define PLATFORM_PSX_TEXT_H

#include "p8/gfx.h"
#include "platform/psx_sprite.h"

/* print's glyphs as GPU SPRTs, in string order, walked as the software backend walks them: one
 * per glyph with a lit texel that shows, after the camera, clipped to the screen, (u, v) its
 * texel in the font texture (psx_gpu_textures_font). */
void psx_text_place(const p8_graphics_t *gfx, const char *str, int x, int y,
                    void (*emit)(psx_sprite_t glyph, void *context), void *context);

/* p8_print_outlined's glyphs as GPU SPRTs, walked as psx_text_place walks print's: one per glyph with
 * a lit texel, its outlined cell (psx_gpu_textures_outlined_font_cell) placed one pixel up and left
 * of the glyph, after the camera, clipped to the screen. */
void psx_text_place_outlined(const p8_graphics_t *gfx, const char *str, int x, int y,
                             void (*emit)(psx_sprite_t glyph, void *context), void *context);

#endif
