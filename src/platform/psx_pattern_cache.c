#include "platform/psx_pattern_cache.h"

#include "p8/draw_cost.h"

#include <assert.h>

void psx_pattern_cache_reset(psx_pattern_cache_t *cache) {
    cache->count = 0;
}

int psx_pattern_cache_slot(psx_pattern_cache_t *cache, const p8_graphics_t *gfx, int col) {
    unsigned long key = p8_draw_cost_pattern_key(gfx, col);
    for (int slot = 0; slot < cache->count; slot++) {
        if (cache->keys[slot] == key) {
            return slot;
        }
    }
    if (cache->count == GPU_PATTERN_SLOTS) {
        return -1;
    }
    int slot = cache->count++;
    cache->keys[slot] = key;
    psx_gpu_textures_pattern(gfx->fillp_pattern, cache->tiles[slot]);
    psx_gpu_textures_pattern_clut(col, gfx->fillp_transparent != 0, cache->cluts[slot]);
    return slot;
}

int psx_pattern_cache_count(const psx_pattern_cache_t *cache) {
    return cache->count;
}

const uint16_t *psx_pattern_cache_tile(const psx_pattern_cache_t *cache, int slot) {
    assert(slot >= 0 && slot < cache->count);
    return cache->tiles[slot];
}

const uint16_t *psx_pattern_cache_clut(const psx_pattern_cache_t *cache, int slot) {
    assert(slot >= 0 && slot < cache->count);
    return cache->cluts[slot];
}
