#include "platform/psx_slot_uploads.h"

#include <stddef.h>
#include <string.h>

void psx_slot_uploads_forget(psx_slot_uploads_t *record) {
    memset(record, 0, sizeof *record);
}

static int add_if_changed(psx_slot_upload_t *out, int count, uint16_t *held, unsigned char *known, size_t bytes,
                          psx_vram_rect_t rect, const uint16_t *words) {
    if (*known && memcmp(held, words, bytes) == 0) return count;
    memcpy(held, words, bytes);
    *known = 1;
    out[count].rect = rect;
    out[count].words = words;
    return count + 1;
}

int psx_slot_uploads_collect(psx_slot_uploads_t *record, const psx_clut_cache_t *cluts,
                             const psx_pattern_cache_t *patterns, psx_slot_upload_t out[PSX_SLOT_UPLOADS_MAX]) {
    int count = 0;
    for (int slot = 0; slot < psx_clut_cache_count(cluts); slot++) {
        count = add_if_changed(out, count, record->palette_cluts[slot], &record->palette_clut_known[slot],
                               sizeof record->palette_cluts[slot], psx_vram_layout_palette_clut(slot),
                               psx_clut_cache_clut(cluts, slot));
    }
    for (int slot = 0; slot < psx_pattern_cache_count(patterns); slot++) {
        count = add_if_changed(out, count, record->pattern_tiles[slot], &record->pattern_tile_known[slot],
                               sizeof record->pattern_tiles[slot], psx_vram_layout_pattern_slot(slot),
                               psx_pattern_cache_tile(patterns, slot));
        count = add_if_changed(out, count, record->pattern_cluts[slot], &record->pattern_clut_known[slot],
                               sizeof record->pattern_cluts[slot], psx_vram_layout_pattern_clut(slot),
                               psx_pattern_cache_clut(patterns, slot));
    }
    return count;
}
