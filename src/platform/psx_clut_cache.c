#include "platform/psx_clut_cache.h"

#include <assert.h>
#include <string.h>

void psx_clut_cache_reset(psx_clut_cache_t *cache) {
    cache->count = 0;
}

int psx_clut_cache_slot(psx_clut_cache_t *cache, const unsigned char draw_palette[PSX_GPU_TEXTURES_CLUT_ENTRIES]) {
    for (int slot = 0; slot < cache->count; slot++) {
        if (memcmp(cache->keys[slot], draw_palette, sizeof cache->keys[slot]) == 0) {
            return slot;
        }
    }
    if (cache->count == GPU_CLUT_SLOTS) {
        return -1;
    }
    int slot = cache->count++;
    memcpy(cache->keys[slot], draw_palette, sizeof cache->keys[slot]);
    psx_gpu_textures_clut(draw_palette, cache->cluts[slot]);
    return slot;
}

int psx_clut_cache_count(const psx_clut_cache_t *cache) {
    return cache->count;
}

const uint16_t *psx_clut_cache_clut(const psx_clut_cache_t *cache, int slot) {
    assert(slot >= 0 && slot < cache->count);
    return cache->cluts[slot];
}
