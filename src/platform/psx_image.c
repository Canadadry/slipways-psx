#include "platform/psx_image.h"

#include <assert.h>

static int min(int a, int b) {
    return a < b ? a : b;
}

psx_vram_rect_t psx_image_window_area(int slot, int w, int h, int x, int y) {
    psx_vram_rect_t image = psx_vram_layout_image(slot);
    assert(x >= 0 && x < w && y >= 0 && y < h && w <= image.w && h <= image.h);
    psx_vram_rect_t area = { image.x + x, image.y + y, min(P8_SCREEN_WIDTH, w - x), min(P8_SCREEN_HEIGHT, h - y) };
    return area;
}

int psx_image_copy_place(const p8_graphics_t *gfx, int slot, int w, int h, int x, int y, psx_sprite_t *sprite) {
    psx_vram_rect_t image = psx_vram_layout_image(slot);
    psx_vram_rect_t page = psx_vram_layout_image_page(slot);
    assert(w <= image.w && h <= image.h);
    if (w <= 0 || h <= 0) {
        return 0;
    }
    psx_sprite_t placed = { x - gfx->camera_x, y - gfx->camera_y, image.x - page.x, image.y - page.y, w, h };
    if (!psx_sprite_clip(&placed)) {
        return 0;
    }
    *sprite = placed;
    return 1;
}
