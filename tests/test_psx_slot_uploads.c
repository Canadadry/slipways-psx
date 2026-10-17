#include "platform/psx_slot_uploads.h"

#include "p8/gfx.h"
#include "p8/gpu_sizes.h"
#include "platform/psx_clut_cache.h"
#include "platform/psx_gpu_textures.h"
#include "platform/psx_pattern_cache.h"
#include "platform/psx_vram_layout.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

typedef struct {
    psx_clut_cache_t cluts;
    psx_pattern_cache_t patterns;
} frame_caches_t;

static void palette_numbered(unsigned char palette[PSX_GPU_TEXTURES_CLUT_ENTRIES], int number) {
    for (int i = 0; i < PSX_GPU_TEXTURES_CLUT_ENTRIES; i++) {
        palette[i] = (unsigned char)i;
    }
    palette[1] = (unsigned char)(number & 0x0F);
    palette[2] = (unsigned char)((number >> 4) & 0x0F);
}

/* Fills caches with one palette per number in palettes, then one fill pattern per (pattern, col)
 * pair, in order. */
static void frame_fill(frame_caches_t *caches, const int *palettes, int palette_count, const unsigned int *patterns,
                       const int *cols, int pattern_count) {
    psx_clut_cache_reset(&caches->cluts);
    psx_pattern_cache_reset(&caches->patterns);
    for (int i = 0; i < palette_count; i++) {
        unsigned char palette[PSX_GPU_TEXTURES_CLUT_ENTRIES];
        palette_numbered(palette, palettes[i]);
        assert(psx_clut_cache_slot(&caches->cluts, palette) == i);
    }
    p8_graphics_t gfx;
    p8_gfx_init(&gfx);
    for (int i = 0; i < pattern_count; i++) {
        p8_fillp(&gfx, patterns[i]);
        assert(psx_pattern_cache_slot(&caches->patterns, &gfx, cols[i]) == i);
    }
}

static int collect(psx_slot_uploads_t *record, const frame_caches_t *caches, psx_slot_upload_t out[PSX_SLOT_UPLOADS_MAX]) {
    return psx_slot_uploads_collect(record, &caches->cluts, &caches->patterns, out);
}

static int same_rect(psx_vram_rect_t a, psx_vram_rect_t b) {
    return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h;
}

static void assert_palette_upload(const psx_slot_upload_t *upload, const frame_caches_t *caches, int slot) {
    assert(same_rect(upload->rect, psx_vram_layout_palette_clut(slot)));
    assert(upload->words == psx_clut_cache_clut(&caches->cluts, slot));
}

static void assert_tile_upload(const psx_slot_upload_t *upload, const frame_caches_t *caches, int slot) {
    assert(same_rect(upload->rect, psx_vram_layout_pattern_slot(slot)));
    assert(upload->words == psx_pattern_cache_tile(&caches->patterns, slot));
}

static void assert_pattern_clut_upload(const psx_slot_upload_t *upload, const frame_caches_t *caches, int slot) {
    assert(same_rect(upload->rect, psx_vram_layout_pattern_clut(slot)));
    assert(upload->words == psx_pattern_cache_clut(&caches->patterns, slot));
}

static const int two_palettes[] = { 1, 2 };
static const unsigned int two_patterns[] = { 0xA5A5, 0x0F0F };
static const int two_cols[] = { 0x12, 0x03 };

static void test_the_first_frame_after_forget_uploads_every_slot_in_use_in_slot_order(void) {
    psx_slot_uploads_t record;
    psx_slot_uploads_forget(&record);
    frame_caches_t frame;
    frame_fill(&frame, two_palettes, 2, two_patterns, two_cols, 2);
    psx_slot_upload_t out[PSX_SLOT_UPLOADS_MAX];
    assert(collect(&record, &frame, out) == 6);
    assert_palette_upload(&out[0], &frame, 0);
    assert_palette_upload(&out[1], &frame, 1);
    assert_tile_upload(&out[2], &frame, 0);
    assert_pattern_clut_upload(&out[3], &frame, 0);
    assert_tile_upload(&out[4], &frame, 1);
    assert_pattern_clut_upload(&out[5], &frame, 1);
}

static void test_an_identical_frame_from_the_other_buffer_uploads_nothing(void) {
    psx_slot_uploads_t record;
    psx_slot_uploads_forget(&record);
    frame_caches_t frames[2];
    psx_slot_upload_t out[PSX_SLOT_UPLOADS_MAX];
    frame_fill(&frames[0], two_palettes, 2, two_patterns, two_cols, 2);
    assert(collect(&record, &frames[0], out) == 6);
    frame_fill(&frames[1], two_palettes, 2, two_patterns, two_cols, 2);
    assert(collect(&record, &frames[1], out) == 0);
    frame_fill(&frames[0], two_palettes, 2, two_patterns, two_cols, 2);
    assert(collect(&record, &frames[0], out) == 0);
}

static void test_a_palette_that_changes_uploads_its_slot_only(void) {
    psx_slot_uploads_t record;
    psx_slot_uploads_forget(&record);
    frame_caches_t frames[2];
    psx_slot_upload_t out[PSX_SLOT_UPLOADS_MAX];
    frame_fill(&frames[0], two_palettes, 2, two_patterns, two_cols, 2);
    collect(&record, &frames[0], out);
    const int changed[] = { 1, 7 };
    frame_fill(&frames[1], changed, 2, two_patterns, two_cols, 2);
    assert(collect(&record, &frames[1], out) == 1);
    assert_palette_upload(&out[0], &frames[1], 1);
}

static void test_content_that_comes_back_two_frames_later_is_uploaded_every_frame(void) {
    psx_slot_uploads_t record;
    psx_slot_uploads_forget(&record);
    frame_caches_t frames[2];
    psx_slot_upload_t out[PSX_SLOT_UPLOADS_MAX];
    const int a[] = { 4 };
    const int b[] = { 5 };
    frame_fill(&frames[0], a, 1, NULL, NULL, 0);
    assert(collect(&record, &frames[0], out) == 1);
    frame_fill(&frames[1], b, 1, NULL, NULL, 0);
    assert(collect(&record, &frames[1], out) == 1);
    assert_palette_upload(&out[0], &frames[1], 0);
    frame_fill(&frames[0], a, 1, NULL, NULL, 0);
    assert(collect(&record, &frames[0], out) == 1);
    assert_palette_upload(&out[0], &frames[0], 0);
}

static void test_a_slot_left_unused_is_compared_with_what_it_last_received_when_reused(void) {
    psx_slot_uploads_t record;
    psx_slot_uploads_forget(&record);
    frame_caches_t frames[2];
    psx_slot_upload_t out[PSX_SLOT_UPLOADS_MAX];
    const int three[] = { 1, 2, 3 };
    const int one[] = { 1 };
    const int other_third[] = { 1, 2, 9 };
    frame_fill(&frames[0], three, 3, NULL, NULL, 0);
    assert(collect(&record, &frames[0], out) == 3);
    frame_fill(&frames[1], one, 1, NULL, NULL, 0);
    assert(collect(&record, &frames[1], out) == 0);
    frame_fill(&frames[0], one, 1, NULL, NULL, 0);
    assert(collect(&record, &frames[0], out) == 0);
    frame_fill(&frames[1], three, 3, NULL, NULL, 0);
    assert(collect(&record, &frames[1], out) == 0);
    frame_fill(&frames[0], one, 1, NULL, NULL, 0);
    assert(collect(&record, &frames[0], out) == 0);
    frame_fill(&frames[1], other_third, 3, NULL, NULL, 0);
    assert(collect(&record, &frames[1], out) == 1);
    assert_palette_upload(&out[0], &frames[1], 2);
}

static void test_a_pattern_tile_and_its_clut_are_uploaded_apart(void) {
    psx_slot_uploads_t record;
    psx_slot_uploads_forget(&record);
    frame_caches_t frames[2];
    psx_slot_upload_t out[PSX_SLOT_UPLOADS_MAX];
    frame_fill(&frames[0], NULL, 0, two_patterns, two_cols, 2);
    assert(collect(&record, &frames[0], out) == 4);
    const int other_cols[] = { 0x12, 0x07 };
    frame_fill(&frames[1], NULL, 0, two_patterns, other_cols, 2);
    assert(collect(&record, &frames[1], out) == 1);
    assert_pattern_clut_upload(&out[0], &frames[1], 1);
    const unsigned int other_patterns[] = { 0x5A5A, 0x0F0F };
    frame_fill(&frames[0], NULL, 0, other_patterns, other_cols, 2);
    assert(collect(&record, &frames[0], out) == 1);
    assert_tile_upload(&out[0], &frames[0], 0);
}

static void test_two_slots_that_swap_contents_are_both_uploaded(void) {
    psx_slot_uploads_t record;
    psx_slot_uploads_forget(&record);
    frame_caches_t frames[2];
    psx_slot_upload_t out[PSX_SLOT_UPLOADS_MAX];
    frame_fill(&frames[0], two_palettes, 2, NULL, NULL, 0);
    collect(&record, &frames[0], out);
    const int swapped[] = { 2, 1 };
    frame_fill(&frames[1], swapped, 2, NULL, NULL, 0);
    assert(collect(&record, &frames[1], out) == 2);
    assert_palette_upload(&out[0], &frames[1], 0);
    assert_palette_upload(&out[1], &frames[1], 1);
}

static void test_forget_makes_the_next_frame_upload_every_slot_again(void) {
    psx_slot_uploads_t record;
    psx_slot_uploads_forget(&record);
    frame_caches_t frames[2];
    psx_slot_upload_t out[PSX_SLOT_UPLOADS_MAX];
    frame_fill(&frames[0], two_palettes, 2, two_patterns, two_cols, 2);
    collect(&record, &frames[0], out);
    psx_slot_uploads_forget(&record);
    frame_fill(&frames[1], two_palettes, 2, two_patterns, two_cols, 2);
    assert(collect(&record, &frames[1], out) == 6);
}

int main(void) {
    test_the_first_frame_after_forget_uploads_every_slot_in_use_in_slot_order();
    test_an_identical_frame_from_the_other_buffer_uploads_nothing();
    test_a_palette_that_changes_uploads_its_slot_only();
    test_content_that_comes_back_two_frames_later_is_uploaded_every_frame();
    test_a_slot_left_unused_is_compared_with_what_it_last_received_when_reused();
    test_a_pattern_tile_and_its_clut_are_uploaded_apart();
    test_two_slots_that_swap_contents_are_both_uploaded();
    test_forget_makes_the_next_frame_upload_every_slot_again();
    printf("test_psx_slot_uploads: ok\n");
    return 0;
}
