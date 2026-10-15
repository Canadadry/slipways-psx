#ifndef PLATFORM_PSX_IMAGE_H
#define PLATFORM_PSX_IMAGE_H

#include "p8/gfx.h"
#include "platform/psx_sprite.h"
#include "platform/psx_vram_layout.h"

/* The VRAM a w x h build of slot draws into during its window at image (x, y): the draw area, and
 * the draw offset at its top-left corner. */
psx_vram_rect_t psx_image_window_area(int slot, int w, int h, int x, int y);

/* The copy's SPRT for a w x h build of slot at world (x, y) after the camera, clipped to the
 * screen, (u, v) in the slot's page. Returns 0 (sprite untouched) when nothing of it shows, else 1. */
int psx_image_copy_place(const p8_graphics_t *gfx, int slot, int w, int h, int x, int y, psx_sprite_t *sprite);

#endif
