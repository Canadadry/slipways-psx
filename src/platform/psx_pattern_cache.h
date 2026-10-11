#ifndef PLATFORM_PSX_PATTERN_CACHE_H
#define PLATFORM_PSX_PATTERN_CACHE_H

#include "p8/gfx.h"
#include "p8/gpu_sizes.h"
#include "platform/psx_gpu_textures.h"

#include <stdint.h>

/* One frame's patterned-fill tiles and CLUTs, keyed by p8_draw_cost_pattern_key, as p8_draw_meter
 * counts them. */
typedef struct {
    uint16_t tiles[GPU_PATTERN_SLOTS][PSX_GPU_TEXTURES_PATTERN_UNITS];
    uint16_t cluts[GPU_PATTERN_SLOTS][PSX_GPU_TEXTURES_CLUT_ENTRIES];
    unsigned long keys[GPU_PATTERN_SLOTS];
    int count;
} psx_pattern_cache_t;

void psx_pattern_cache_reset(psx_pattern_cache_t *cache);

/* The slot of gfx's fill pattern drawn in col, built at its first use since the reset; -1 (cache
 * unchanged) when the GPU_PATTERN_SLOTS slots all hold other patterns. */
int psx_pattern_cache_slot(psx_pattern_cache_t *cache, const p8_graphics_t *gfx, int col);

/* Slots 0..count-1 are in use. */
int psx_pattern_cache_count(const psx_pattern_cache_t *cache);

/* Owned by the cache, word-aligned when the cache is. slot < psx_pattern_cache_count. */
const uint16_t *psx_pattern_cache_tile(const psx_pattern_cache_t *cache, int slot);
const uint16_t *psx_pattern_cache_clut(const psx_pattern_cache_t *cache, int slot);

#endif
