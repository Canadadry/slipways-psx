#include "platform/psx_gpu.h"

#include "p8/draw_cost.h"
#include "p8/fatal.h"
#include "p8/gpu_sizes.h"
#include "p8/palette.h"
#include "p8/sprite_sheet.h"
#include "platform/psx_clut_cache.h"
#include "platform/psx_gpu_textures.h"
#include "platform/psx_pattern.h"
#include "platform/psx_pattern_cache.h"
#include "platform/psx_sprite.h"
#include "platform/psx_text.h"
#include "platform/psx_tiles.h"
#include "platform/psx_vram_layout.h"

#include <assert.h>
#include <psxgpu.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE_CHECK(name, condition) typedef char name[(condition) ? 1 : -1]
SIZE_CHECK(fill_size_check, sizeof(FILL) == P8_GPU_FILL_BYTES);
SIZE_CHECK(tile_1_size_check, sizeof(TILE_1) == P8_GPU_TILE_1_BYTES);
SIZE_CHECK(tile_size_check, sizeof(TILE) == P8_GPU_TILE_BYTES);
SIZE_CHECK(line_f2_size_check, sizeof(LINE_F2) == P8_GPU_LINE_F2_BYTES);
SIZE_CHECK(sprt_size_check, sizeof(SPRT) == P8_GPU_SPRT_BYTES);
SIZE_CHECK(dr_tpage_size_check, sizeof(DR_TPAGE) == P8_GPU_DR_TPAGE_BYTES);
SIZE_CHECK(dr_twin_size_check, sizeof(DR_TWIN) == P8_GPU_DR_TWIN_BYTES);
SIZE_CHECK(dr_env_size_check, sizeof(DR_ENV) == P8_GPU_DR_ENV_BYTES);
SIZE_CHECK(poly_ft4_size_check, sizeof(POLY_FT4) == P8_GPU_POLY_FT4_BYTES);
/* What frame_begin and frame_end add must fit what p8_draw_cost_frame_begin meters. */
SIZE_CHECK(frame_overhead_size_check,
           2 * (sizeof(DR_AREA) + sizeof(DR_OFFSET) + sizeof(DR_TWIN)) + sizeof(DR_TPAGE) + sizeof(FILL) + sizeof(POLY_FT4)
               <= 2 * P8_GPU_DR_ENV_BYTES + P8_GPU_DR_TPAGE_BYTES + P8_GPU_DR_TWIN_BYTES + P8_GPU_POLY_FT4_BYTES);
SIZE_CHECK(frame_buffer_word_check, GPU_FRAME_BUFFER_BYTES % 4 == 0);

enum { DISPLAY_WIDTH = 320, DISPLAY_HEIGHT = 240, TEXTURE_4_BIT = 0, TEXTURE_15_BIT = 2, SHEET_UPLOAD_LINES = 16 };

static uint32_t buffers[2][GPU_FRAME_BUFFER_BYTES / 4];
static int current;
static size_t used;
static uint32_t *previous;
static int frame_number;
static DISPENV displays[2];
/* One of each per primitive buffer: frame N fills its caches while frame N-1's LoadImage may still read the others. */
static psx_clut_cache_t clut_caches[2];
static psx_pattern_cache_t pattern_caches[2];
/* The pattern slot the frame's last DR_TWIN repeats, NO_WINDOW for none. */
static int window_slot;
static psx_gpu_peaks_t peaks;

enum { NO_WINDOW = -1 };

static void *packet(size_t bytes) {
    if (used + bytes > GPU_FRAME_BUFFER_BYTES) {
        static char message[128];
        snprintf(message, sizeof message, "psx_gpu: frame %d asked %u bytes with %u of %u used", frame_number,
                 (unsigned)bytes, (unsigned)used, (unsigned)GPU_FRAME_BUFFER_BYTES);
        p8_fatal(message);
    }
    uint32_t *words = &buffers[current][used / 4];
    used += bytes;
    termPrim(words);
    if (previous != NULL) {
        catPrim(previous, words);
    }
    previous = words;
    return words;
}

static void add_draw_target(psx_vram_rect_t target) {
    DR_AREA *area = packet(sizeof *area);
    setDrawArea(area, &target);
    DR_OFFSET *offset = packet(sizeof *offset);
    setDrawOffset(offset, target.x, target.y);
}

static void add_no_texture_window(void) {
    static const RECT none = { 0, 0, 0, 0 };
    DR_TWIN *window = packet(sizeof *window);
    setTexWindow(window, &none);
    window_slot = NO_WINDOW;
}

/* Only a DR_TWIN, where the cost model meters a DR_TPAGE too: every textured call reads the one
 * page frame_begin sets. */
static void use_texture_window(int slot) {
    if (slot == window_slot) {
        return;
    }
    if (slot == NO_WINDOW) {
        add_no_texture_window();
        return;
    }
    psx_pattern_window_t fields = psx_pattern_window(slot);
    RECT rect = { (int16_t)fields.offset_x, (int16_t)fields.offset_y, (int16_t)fields.mask_x, (int16_t)fields.mask_y };
    DR_TWIN *window = packet(sizeof *window);
    setTexWindow(window, &rect);
    window_slot = slot;
}

static void upload_sprite_sheet(void) {
    static uint32_t lines[SHEET_UPLOAD_LINES * P8_SPRITE_SHEET_WIDTH / 8];
    psx_vram_rect_t sheet = psx_vram_layout_region(PSX_VRAM_SPRITE_SHEET);
    const unsigned char *packed = p8_sprite_sheet_packed();
    for (int y = 0; y < sheet.h; y += SHEET_UPLOAD_LINES) {
        memcpy(lines, packed + y * sheet.w * 2, sizeof lines);
        RECT rect = { (int16_t)sheet.x, (int16_t)(sheet.y + y), (int16_t)sheet.w, SHEET_UPLOAD_LINES };
        LoadImage(&rect, lines);
        DrawSync(0);
    }
}

static void queue_load(psx_vram_rect_t region, const uint16_t *pixels) {
    RECT rect = { (int16_t)region.x, (int16_t)region.y, (int16_t)region.w, (int16_t)region.h };
    assert(((uintptr_t)pixels & 3) == 0 && "LoadImage's DMA reads whole words");
    LoadImage(&rect, (const uint32_t *)pixels);
}

static void load_and_wait(psx_vram_rect_t region, const uint16_t *pixels) {
    queue_load(region, pixels);
    DrawSync(0);
}

static void upload_font(void) {
    static uint16_t texture[PSX_GPU_TEXTURES_FONT_UNITS];
    static uint16_t clut[PSX_GPU_TEXTURES_CLUT_ENTRIES];
    psx_vram_rect_t font = psx_vram_layout_region(PSX_VRAM_FONT);
    if (font.w * 4 != PSX_GPU_TEXTURES_FONT_SIZE || font.h != PSX_GPU_TEXTURES_FONT_SIZE) {
        p8_fatal("psx_gpu: the font region does not hold the font texture");
    }
    psx_gpu_textures_font(texture);
    load_and_wait(font, texture);
    for (int colour = 0; colour < PSX_VRAM_FONT_COLOURS; colour++) {
        psx_gpu_textures_font_clut(colour, clut);
        load_and_wait(psx_vram_layout_font_clut(colour), clut);
    }
}

void psx_gpu_init(void) {
    psx_vram_rect_t sheet = psx_vram_layout_region(PSX_VRAM_SPRITE_SHEET);
    if (sheet.w * sheet.h * 2 != P8_SPRITE_SHEET_PACKED_SIZE || sheet.h % SHEET_UPLOAD_LINES != 0) {
        p8_fatal("psx_gpu: the sprite sheet region does not hold the packed sheet");
    }
    SetVideoMode(MODE_NTSC);
    for (int i = 0; i < 2; i++) {
        psx_vram_rect_t display = psx_vram_layout_region(i == 0 ? PSX_VRAM_DISPLAY_0 : PSX_VRAM_DISPLAY_1);
        SetDefDispEnv(&displays[i], display.x, display.y, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    }
    upload_sprite_sheet();
    upload_font();
    current = 0;
    peaks = (psx_gpu_peaks_t){ 0, 0, 0 };
    PutDispEnv(&displays[1]);
    SetDispMask(1);
}

static void add_black_fill(psx_vram_rect_t region) {
    FILL *fill = packet(sizeof *fill);
    setFill(fill);
    setRGB0(fill, 0, 0, 0);
    setXY0(fill, region.x, region.y);
    setWH(fill, region.w, region.h);
}

static void gpu_cls(p8_vram_t *mem) {
    (void)mem;
    add_black_fill(psx_vram_layout_region(PSX_VRAM_DRAW_AREA));
}

static void gpu_line(const p8_graphics_t *gfx, p8_vram_t *mem, int x1, int y1, int x2, int y2, int col) {
    (void)mem;
    psx_line_t line;
    if (!psx_tiles_line(gfx, x1, y1, x2, y2, &line)) {
        return;
    }
    assert(abs(line.x2 - line.x1) < 1024 && abs(line.y2 - line.y1) < 512 && abs(line.x1) < 1024 &&
           abs(line.y1) < 1024 && abs(line.x2) < 1024 && abs(line.y2) < 1024 &&
           "the GPU silently drops a line this long or this far: the replays reach 297 at most");
    p8_rgb_t rgb = P8_PALETTE_RGB[col & 0x0F];
    LINE_F2 *primitive = packet(sizeof *primitive);
    setLineF2(primitive);
    setRGB0(primitive, rgb.red, rgb.green, rgb.blue);
    setXY2(primitive, line.x1, line.y1, line.x2, line.y2);
}

static void add_tile(psx_tile_t tile, int col) {
    p8_rgb_t rgb = P8_PALETTE_RGB[col & 0x0F];
    TILE *primitive = packet(sizeof *primitive);
    setTile(primitive);
    setRGB0(primitive, rgb.red, rgb.green, rgb.blue);
    setXY0(primitive, tile.x, tile.y);
    setWH(primitive, tile.w, tile.h);
}

static void add_pixel(psx_tile_t pixel, void *context) {
    const p8_rgb_t *rgb = context;
    TILE_1 *primitive = packet(sizeof *primitive);
    setTile1(primitive);
    setRGB0(primitive, rgb->red, rgb->green, rgb->blue);
    setXY0(primitive, pixel.x, pixel.y);
}

static void gpu_pset(const p8_graphics_t *gfx, p8_vram_t *mem, int x, int y, int col) {
    (void)mem;
    psx_tile_t pixel;
    if (psx_tiles_pset(gfx, x, y, &pixel)) {
        p8_rgb_t rgb = P8_PALETTE_RGB[col & 0x0F];
        add_pixel(pixel, &rgb);
    }
}

static void gpu_rect(const p8_graphics_t *gfx, p8_vram_t *mem, int x1, int y1, int x2, int y2, int col) {
    (void)mem;
    psx_tile_t tiles[PSX_TILES_RECT_MAX];
    int count = psx_tiles_rect(gfx, x1, y1, x2, y2, tiles);
    for (int t = 0; t < count; t++) {
        add_tile(tiles[t], col);
    }
}

static int patterned(const p8_graphics_t *gfx) {
    return (gfx->fillp_pattern & 0xFFFFu) != 0;
}

static void add_sprt(int x, int y, int u, int v, int w, int h, psx_vram_rect_t clut) {
    SPRT *primitive = packet(sizeof *primitive);
    setSprt(primitive);
    setShadeTex(primitive, 1);
    setRGB0(primitive, 128, 128, 128);
    setXY0(primitive, x, y);
    setUV0(primitive, u, v);
    setWH(primitive, w, h);
    primitive->clut = getClut(clut.x, clut.y);
}

static int pattern_slot(const p8_graphics_t *gfx, int col) {
    int slot = psx_pattern_cache_slot(&pattern_caches[current], gfx, col);
    if (slot < 0) {
        static char message[128];
        snprintf(message, sizeof message, "psx_gpu: frame %d needs more than %d fill patterns", frame_number,
                 GPU_PATTERN_SLOTS);
        p8_fatal(message);
    }
    return slot;
}

/* Called only with spans to draw, so that the window and the slots change only where the cost
 * model counts a call on screen. */
static void add_patterned_spans(const p8_graphics_t *gfx, const psx_tile_t *spans, int count, int col) {
    int slot = pattern_slot(gfx, col);
    use_texture_window(slot);
    psx_vram_rect_t clut = psx_vram_layout_pattern_clut(slot);
    for (int s = 0; s < count; s++) {
        psx_sprite_t sprite = psx_pattern_sprite(gfx, spans[s]);
        add_sprt(sprite.x, sprite.y, sprite.u, sprite.v, sprite.w, sprite.h, clut);
    }
}

static void gpu_rectfill(const p8_graphics_t *gfx, p8_vram_t *mem, int x1, int y1, int x2, int y2, int col) {
    (void)mem;
    psx_tile_t tile;
    if (!psx_tiles_rectfill(gfx, x1, y1, x2, y2, &tile)) {
        return;
    }
    if (patterned(gfx)) {
        add_patterned_spans(gfx, &tile, 1, col);
    } else {
        add_tile(tile, col);
    }
}

static void gpu_circ(const p8_graphics_t *gfx, p8_vram_t *mem, int cx, int cy, int r, int col) {
    (void)mem;
    p8_rgb_t rgb = P8_PALETTE_RGB[col & 0x0F];
    psx_tiles_circ(gfx, cx, cy, r, add_pixel, &rgb);
}

static void gpu_circfill(const p8_graphics_t *gfx, p8_vram_t *mem, int cx, int cy, int r, int col) {
    (void)mem;
    psx_tile_t spans[PSX_TILES_CIRCFILL_MAX];
    int count = psx_tiles_circfill(gfx, cx, cy, r, spans);
    if (count > 0 && patterned(gfx)) {
        add_patterned_spans(gfx, spans, count, col);
        return;
    }
    for (int s = 0; s < count; s++) {
        add_tile(spans[s], col);
    }
}

static void add_textured_sprite(psx_sprite_t sprite, psx_vram_rect_t texture, psx_vram_rect_t clut) {
    psx_vram_rect_t page = psx_vram_layout_texture_page();
    use_texture_window(NO_WINDOW);
    add_sprt(sprite.x, sprite.y, (texture.x - page.x) * 4 + sprite.u, texture.y - page.y + sprite.v, sprite.w, sprite.h,
             clut);
}

static int clut_slot(const unsigned char draw_palette[PSX_GPU_TEXTURES_CLUT_ENTRIES]) {
    int slot = psx_clut_cache_slot(&clut_caches[current], draw_palette);
    if (slot < 0) {
        static char message[128];
        snprintf(message, sizeof message, "psx_gpu: frame %d needs more than %d spr CLUTs", frame_number,
                 GPU_CLUT_SLOTS);
        p8_fatal(message);
    }
    return slot;
}

static void gpu_spr(const p8_graphics_t *gfx, p8_vram_t *mem, int n, int x, int y, int w, int h) {
    psx_sprite_t sprite;
    if (!psx_sprite_place(gfx, n, x, y, w, h, &sprite)) {
        return;
    }
    psx_vram_rect_t clut = psx_vram_layout_palette_clut(clut_slot(mem->draw_palette));
    add_textured_sprite(sprite, psx_vram_layout_region(PSX_VRAM_SPRITE_SHEET), clut);
}

static void add_glyph(psx_sprite_t glyph, void *context) {
    const psx_vram_rect_t *clut = context;
    add_textured_sprite(glyph, psx_vram_layout_region(PSX_VRAM_FONT), *clut);
}

static void gpu_print(const p8_graphics_t *gfx, p8_vram_t *mem, const char *str, int x, int y, int col) {
    (void)mem;
    psx_vram_rect_t clut = psx_vram_layout_font_clut(col & 0x0F);
    psx_text_place(gfx, str, x, y, add_glyph, &clut);
}

static const p8_draw_backend_t GPU_BACKEND = {
    gpu_cls, gpu_pset, gpu_line, gpu_rect, gpu_rectfill, gpu_circ, gpu_circfill, gpu_spr, gpu_print,
};

const p8_draw_backend_t *psx_gpu_backend(void) {
    return &GPU_BACKEND;
}

void psx_gpu_frame_begin(int frame) {
    frame_number = frame;
    used = 0;
    previous = NULL;
    psx_clut_cache_reset(&clut_caches[current]);
    psx_pattern_cache_reset(&pattern_caches[current]);
    add_draw_target(psx_vram_layout_region(PSX_VRAM_DRAW_AREA));
    psx_vram_rect_t page = psx_vram_layout_texture_page();
    DR_TPAGE *tpage = packet(sizeof *tpage);
    setDrawTPage(tpage, 0, 0, getTPage(TEXTURE_4_BIT, 0, page.x, page.y));
    add_no_texture_window();
}

static void add_upscale(void) {
    psx_vram_rect_t page = psx_vram_layout_upscale_page();
    psx_vram_rect_t area = psx_vram_layout_region(PSX_VRAM_DRAW_AREA);
    POLY_FT4 *quad = packet(sizeof *quad);
    setPolyFT4(quad);
    setShadeTex(quad, 1);
    setRGB0(quad, 128, 128, 128);
    setXY4(quad, 0, 0, DISPLAY_WIDTH, 0, 0, DISPLAY_HEIGHT, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    setUV4(quad, 0, 0, area.w, 0, 0, area.h, area.w, area.h);
    quad->tpage = getTPage(TEXTURE_15_BIT, 0, page.x, page.y);
    quad->clut = 0;
}

/* Queued behind the previous frame's DrawSync and ahead of this frame's DrawOTag, so no slot
 * changes while a frame reads it. */
static void upload_slots(const psx_clut_cache_t *cluts, const psx_pattern_cache_t *patterns) {
    for (int slot = 0; slot < psx_clut_cache_count(cluts); slot++) {
        queue_load(psx_vram_layout_palette_clut(slot), psx_clut_cache_clut(cluts, slot));
    }
    for (int slot = 0; slot < psx_pattern_cache_count(patterns); slot++) {
        queue_load(psx_vram_layout_pattern_slot(slot), psx_pattern_cache_tile(patterns, slot));
        queue_load(psx_vram_layout_pattern_clut(slot), psx_pattern_cache_clut(patterns, slot));
    }
}

static void record_peaks(void) {
    if (used > peaks.bytes) peaks.bytes = used;
    int cluts = psx_clut_cache_count(&clut_caches[current]);
    if (cluts > peaks.cluts) peaks.cluts = cluts;
    int patterns = psx_pattern_cache_count(&pattern_caches[current]);
    if (patterns > peaks.patterns) peaks.patterns = patterns;
}

psx_gpu_peaks_t psx_gpu_peaks(void) {
    return peaks;
}

void psx_gpu_frame_end(unsigned long limit_bytes) {
    psx_vram_rect_t display = psx_vram_layout_region(current == 0 ? PSX_VRAM_DISPLAY_0 : PSX_VRAM_DISPLAY_1);
    add_draw_target(display);
    add_no_texture_window();
    /* The upscale skips 0x0000 texels as transparent, and a black FILL or TILE writes 0x0000: without
     * this clear, black pixels would show the display buffer's previous frame. */
    add_black_fill(display);
    add_upscale();
    if (used > limit_bytes) {
        static char message[128];
        snprintf(message, sizeof message, "psx_gpu: frame %d sent %u bytes, limit %lu", frame_number, (unsigned)used,
                 limit_bytes);
        p8_fatal(message);
    }
    record_peaks();
    DrawSync(0);
    upload_slots(&clut_caches[current], &pattern_caches[current]);
    PutDispEnv(&displays[current ^ 1]);
    DrawOTag(buffers[current]);
    current ^= 1;
}

void psx_gpu_frame_show_last(void) {
    DrawSync(0);
    PutDispEnv(&displays[current ^ 1]);
}
