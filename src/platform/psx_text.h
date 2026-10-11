#ifndef PLATFORM_PSX_TEXT_H
#define PLATFORM_PSX_TEXT_H

#include "p8/gfx.h"
#include "platform/psx_sprite.h"

/* print's glyphs as GPU SPRTs, in string order, walked as the software backend walks them: one
 * per glyph with a lit texel that shows, after the camera, clipped to the screen, (u, v) its
 * texel in the font texture (psx_gpu_textures_font). */
void psx_text_place(const p8_graphics_t *gfx, const char *str, int x, int y,
                    void (*emit)(psx_sprite_t glyph, void *context), void *context);

#endif
