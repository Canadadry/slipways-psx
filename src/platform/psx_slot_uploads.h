#ifndef PLATFORM_PSX_SLOT_UPLOADS_H
#define PLATFORM_PSX_SLOT_UPLOADS_H

#include "p8/gpu_sizes.h"
#include "platform/psx_clut_cache.h"
#include "platform/psx_gpu_textures.h"
#include "platform/psx_pattern_cache.h"
#include "platform/psx_vram_layout.h"

#include <stdint.h>

enum { PSX_SLOT_UPLOADS_MAX = GPU_CLUT_SLOTS + 2 * GPU_PATTERN_SLOTS };

typedef struct {
    uint16_t palette_cluts[GPU_CLUT_SLOTS][PSX_GPU_TEXTURES_CLUT_ENTRIES];
    uint16_t pattern_tiles[GPU_PATTERN_SLOTS][PSX_GPU_TEXTURES_PATTERN_UNITS];
    uint16_t pattern_cluts[GPU_PATTERN_SLOTS][PSX_GPU_TEXTURES_CLUT_ENTRIES];
    unsigned char palette_clut_known[GPU_CLUT_SLOTS];
    unsigned char pattern_tile_known[GPU_PATTERN_SLOTS];
    unsigned char pattern_clut_known[GPU_PATTERN_SLOTS];
} psx_slot_uploads_t;

typedef struct {
    psx_vram_rect_t rect;
    const uint16_t *words;
} psx_slot_upload_t;

void psx_slot_uploads_forget(psx_slot_uploads_t *record);

/* Fills out with the uploads whose words differ from what record says their VRAM slot holds, and
 * returns how many (at most PSX_SLOT_UPLOADS_MAX). words points into cluts or patterns: valid until
 * that cache is reset or refilled. */
int psx_slot_uploads_collect(psx_slot_uploads_t *record, const psx_clut_cache_t *cluts,
                             const psx_pattern_cache_t *patterns, psx_slot_upload_t out[PSX_SLOT_UPLOADS_MAX]);

#endif
