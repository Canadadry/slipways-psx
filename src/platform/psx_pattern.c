#include "platform/psx_pattern.h"

#include "platform/psx_gpu_textures.h"
#include "platform/psx_vram_layout.h"

#include <assert.h>

enum { WINDOW_UNIT = 8, WINDOW_MASK = 0x1F };

typedef char a_window_unit_is_one_tile[(int)PSX_GPU_TEXTURES_PATTERN_SIZE == (int)WINDOW_UNIT ? 1 : -1];

psx_pattern_window_t psx_pattern_window(int slot) {
    psx_vram_rect_t page = psx_vram_layout_texture_page();
    psx_vram_rect_t texture = psx_vram_layout_pattern_slot(slot);
    int u = (texture.x - page.x) * 4;
    int v = texture.y - page.y;
    assert(u % WINDOW_UNIT == 0 && v % WINDOW_UNIT == 0 && u < 256 && v < 256 &&
           "the texture window only reaches 8-texel bounds of the page");
    psx_pattern_window_t window = { WINDOW_MASK, WINDOW_MASK, u / WINDOW_UNIT, v / WINDOW_UNIT };
    return window;
}

static int phase(int world) {
    return ((world % WINDOW_UNIT) + WINDOW_UNIT) % WINDOW_UNIT;
}

psx_sprite_t psx_pattern_sprite(const p8_graphics_t *gfx, psx_tile_t tile) {
    psx_sprite_t sprite = { tile.x, tile.y, phase(tile.x + gfx->camera_x), phase(tile.y + gfx->camera_y), tile.w, tile.h };
    return sprite;
}
