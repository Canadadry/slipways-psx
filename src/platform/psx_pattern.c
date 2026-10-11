#include "platform/psx_pattern.h"

#include "p8/draw_cost.h"
#include "platform/psx_gpu_textures.h"
#include "platform/psx_vram_layout.h"

#include <assert.h>

enum { WINDOW_UNIT = 8, WINDOW_MASK = 0x1E, TILE = PSX_GPU_TEXTURES_PATTERN_X2_SIZE };

typedef char a_tile_is_two_window_units[(int)TILE == 2 * (int)WINDOW_UNIT ? 1 : -1];
typedef char the_mask_repeats_one_tile[(~WINDOW_MASK & 0x1F) * WINDOW_UNIT + WINDOW_UNIT == (int)TILE ? 1 : -1];
typedef char the_meter_prices_these_pieces[(int)P8_GPU_SCALE == (int)PSX_TILES_SCALE
                                           && (int)P8_GPU_PATTERN_PHASES == (int)PSX_GPU_TEXTURES_PATTERN_SIZE
                                           && P8_GPU_SPRT_U_LIMIT % TILE == 0 ? 1 : -1];

psx_pattern_window_t psx_pattern_window(int slot) {
    psx_vram_rect_t page = psx_vram_layout_texture_page(PSX_VRAM_PATTERNS);
    psx_vram_rect_t texture = psx_vram_layout_pattern_slot(slot);
    int u = (texture.x - page.x) * 4;
    int v = texture.y - page.y;
    assert(u % TILE == 0 && v % TILE == 0 && u < 256 && v < 256 &&
           "the texture window only reaches 16-texel bounds of the page at x2");
    psx_pattern_window_t window = { WINDOW_MASK, WINDOW_MASK, u / WINDOW_UNIT, v / WINDOW_UNIT };
    return window;
}

static int phase(int world) {
    return ((world % P8_GPU_PATTERN_PHASES) + P8_GPU_PATTERN_PHASES) % P8_GPU_PATTERN_PHASES;
}

psx_sprite_t psx_pattern_sprite(const p8_graphics_t *gfx, psx_tile_t tile) {
    psx_sprite_t sprite = { tile.x, tile.y, phase(tile.x + gfx->camera_x), phase(tile.y + gfx->camera_y), tile.w, tile.h };
    return sprite;
}

int psx_pattern_sprites_x2(const p8_graphics_t *gfx, psx_tile_t tile, psx_sprite_t pieces[PSX_PATTERN_PIECES_MAX]) {
    psx_sprite_t whole = psx_sprite_double(psx_pattern_sprite(gfx, tile));
    if (whole.u + whole.w <= P8_GPU_SPRT_U_LIMIT) {
        pieces[0] = whole;
        return 1;
    }
    int left = P8_GPU_SPRT_U_LIMIT - whole.u;
    psx_sprite_t first = { whole.x, whole.y, whole.u, whole.v, left, whole.h };
    psx_sprite_t second = { whole.x + left, whole.y, 0, whole.v, whole.w - left, whole.h };
    assert(second.w <= P8_GPU_SPRT_U_LIMIT && "a 320-texel span needs two pieces at most");
    pieces[0] = first;
    pieces[1] = second;
    return 2;
}
