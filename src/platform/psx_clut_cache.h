#ifndef PLATFORM_PSX_CLUT_CACHE_H
#define PLATFORM_PSX_CLUT_CACHE_H

#include "p8/gpu_sizes.h"
#include "platform/psx_gpu_textures.h"

#include <stdint.h>

/* One frame's spr CLUTs, keyed by the 16 draw-palette bytes, as p8_draw_meter counts them. */
typedef struct {
    uint16_t cluts[GPU_CLUT_SLOTS][PSX_GPU_TEXTURES_CLUT_ENTRIES];
    unsigned char keys[GPU_CLUT_SLOTS][PSX_GPU_TEXTURES_CLUT_ENTRIES];
    int count;
} psx_clut_cache_t;

void psx_clut_cache_reset(psx_clut_cache_t *cache);

/* The slot of draw_palette's CLUT, built at its first use since the reset; -1 (cache unchanged)
 * when the GPU_CLUT_SLOTS slots all hold other palettes. */
int psx_clut_cache_slot(psx_clut_cache_t *cache, const unsigned char draw_palette[PSX_GPU_TEXTURES_CLUT_ENTRIES]);

/* Slots 0..count-1 are in use. */
int psx_clut_cache_count(const psx_clut_cache_t *cache);

/* PSX_GPU_TEXTURES_CLUT_ENTRIES entries, owned by the cache, word-aligned when the cache is. slot <
 * psx_clut_cache_count. */
const uint16_t *psx_clut_cache_clut(const psx_clut_cache_t *cache, int slot);

#endif
