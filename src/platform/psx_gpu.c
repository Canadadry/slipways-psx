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
#include <string.h>

#define SIZE_CHECK(name, condition) typedef char name[(condition) ? 1 : -1]
SIZE_CHECK(tile_size_check, sizeof(TILE) == P8_GPU_TILE_BYTES);
SIZE_CHECK(sprt_size_check, sizeof(SPRT) == P8_GPU_SPRT_BYTES);
SIZE_CHECK(dr_tpage_size_check, sizeof(DR_TPAGE) == P8_GPU_DR_TPAGE_BYTES);
SIZE_CHECK(dr_twin_size_check, sizeof(DR_TWIN) == P8_GPU_DR_TWIN_BYTES);
SIZE_CHECK(dr_env_size_check, sizeof(DR_ENV) == P8_GPU_DR_ENV_BYTES);
/* What frame_begin adds must fit what p8_draw_cost_frame_begin meters. */
SIZE_CHECK(frame_overhead_size_check, sizeof(DR_AREA) + sizeof(DR_OFFSET) + sizeof(DR_TPAGE) + sizeof(DR_TWIN)
                                          <= P8_GPU_DR_ENV_BYTES + P8_GPU_DR_TPAGE_BYTES + P8_GPU_DR_TWIN_BYTES);
SIZE_CHECK(frame_buffer_word_check, GPU_FRAME_BUFFER_BYTES % 4 == 0);

enum { DISPLAY_WIDTH = 320, DISPLAY_HEIGHT = 240, TEXTURE_4_BIT = 0, UPLOAD_LINES = 16, PAGE_TEXELS = 256 };
SIZE_CHECK(display_scale_check, DISPLAY_WIDTH == P8_SCREEN_WIDTH * PSX_TILES_SCALE
                                    && DISPLAY_HEIGHT == P8_SCREEN_HEIGHT * PSX_TILES_SCALE);

static uint32_t buffers[2][GPU_FRAME_BUFFER_BYTES / 4];
static int current;
static size_t used;
static uint32_t *previous;
static int frame_number;
static DISPENV displays[2];
/* One of each per primitive buffer: frame N fills its caches while frame N-1's LoadImage may still read the others. */
static psx_clut_cache_t clut_caches[2];
static psx_pattern_cache_t pattern_caches[2];
enum { NO_WINDOW = -1 };

/* The texture the frame's last DR_TPAGE selects the page of, and the pattern slot its last DR_TWIN
 * repeats (NO_WINDOW for none). */
typedef struct {
    psx_vram_region_t texture;
    int slot;
} texture_state_t;

static texture_state_t texture_state;

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

/* Sends the DR_TPAGE + DR_TWIN pair p8_draw_cost meters for every change of texture state. */
static void add_texture_state(psx_vram_region_t texture, int slot) {
    psx_vram_rect_t page = psx_vram_layout_texture_page(texture);
    DR_TPAGE *tpage = packet(sizeof *tpage);
    setDrawTPage(tpage, 0, 0, getTPage(TEXTURE_4_BIT, 0, page.x, page.y));
    RECT rect = { 0, 0, 0, 0 };
    if (slot != NO_WINDOW) {
        psx_pattern_window_t fields = psx_pattern_window(slot);
        rect = (RECT){ (int16_t)fields.offset_x, (int16_t)fields.offset_y, (int16_t)fields.mask_x, (int16_t)fields.mask_y };
    }
    DR_TWIN *window = packet(sizeof *window);
    setTexWindow(window, &rect);
    texture_state = (texture_state_t){ texture, slot };
}

static void use_texture(psx_vram_region_t texture, int slot) {
    if (texture != texture_state.texture || slot != texture_state.slot) {
        add_texture_state(texture, slot);
    }
}

static uint32_t upload_chunk[UPLOAD_LINES * PAGE_TEXELS / 8];

static void upload_upscaled(psx_vram_rect_t region, const unsigned char *source, int source_width) {
    for (int line = 0; line < region.h; line += UPLOAD_LINES) {
        psx_gpu_textures_upscale(source, source_width, line, UPLOAD_LINES, (unsigned char *)upload_chunk);
        RECT rect = { (int16_t)region.x, (int16_t)(region.y + line), (int16_t)region.w, UPLOAD_LINES };
        LoadImage(&rect, upload_chunk);
        DrawSync(0);
    }
}

static void upload_sprite_sheet(void) {
    psx_vram_rect_t sheet = psx_vram_layout_region(PSX_VRAM_SPRITE_SHEET);
    upload_upscaled(sheet, p8_sprite_sheet_packed(), P8_SPRITE_SHEET_WIDTH);
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
    if (font.w != 2 * PSX_GPU_TEXTURES_FONT_SIZE / 4 || font.h != 2 * PSX_GPU_TEXTURES_FONT_LINES
        || font.h % UPLOAD_LINES != 0) {
        p8_fatal("psx_gpu: the font region does not hold the x2 font texture");
    }
    psx_gpu_textures_font(texture);
    /* The upscaler reads bytes, low nibble first: the font's units are the same bytes only on a
     * little-endian CPU, which the PSX is. */
    upload_upscaled(font, (const unsigned char *)texture, PSX_GPU_TEXTURES_FONT_SIZE);
    for (int colour = 0; colour < PSX_VRAM_FONT_COLOURS; colour++) {
        psx_gpu_textures_font_clut(colour, clut);
        load_and_wait(psx_vram_layout_font_clut(colour), clut);
    }
}

void psx_gpu_init(void) {
    psx_vram_rect_t sheet = psx_vram_layout_region(PSX_VRAM_SPRITE_SHEET);
    if (sheet.w * sheet.h * 2 != 4 * P8_SPRITE_SHEET_PACKED_SIZE || sheet.w * 4 != 2 * P8_SPRITE_SHEET_WIDTH
        || sheet.h % UPLOAD_LINES != 0) {
        p8_fatal("psx_gpu: the sprite sheet region does not hold the x2 packed sheet");
    }
    SetVideoMode(MODE_NTSC);
    for (int i = 0; i < 2; i++) {
        psx_vram_rect_t display = psx_vram_layout_region(i == 0 ? PSX_VRAM_DISPLAY_0 : PSX_VRAM_DISPLAY_1);
        SetDefDispEnv(&displays[i], display.x, display.y, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    }
    upload_sprite_sheet();
    upload_font();
    current = 0;
    PutDispEnv(&displays[1]);
    SetDispMask(1);
}

static void add_tile(psx_tile_t tile, int col) {
    p8_rgb_t rgb = P8_PALETTE_RGB[col & 0x0F];
    psx_tile_t doubled = psx_tiles_double(tile);
    TILE *primitive = packet(sizeof *primitive);
    setTile(primitive);
    setRGB0(primitive, rgb.red, rgb.green, rgb.blue);
    setXY0(primitive, doubled.x, doubled.y);
    setWH(primitive, doubled.w, doubled.h);
}

static void gpu_cls(p8_vram_t *mem) {
    (void)mem;
    psx_tile_t screen = { 0, 0, P8_SCREEN_WIDTH, P8_SCREEN_HEIGHT };
    add_tile(screen, 0);
}

static void add_pixel(psx_tile_t pixel, void *context) {
    const int *col = context;
    add_tile(pixel, *col);
}

static void gpu_line(const p8_graphics_t *gfx, p8_vram_t *mem, int x1, int y1, int x2, int y2, int col) {
    (void)mem;
    psx_tiles_line(gfx, x1, y1, x2, y2, add_pixel, &col);
}

static void gpu_pset(const p8_graphics_t *gfx, p8_vram_t *mem, int x, int y, int col) {
    (void)mem;
    psx_tile_t pixel;
    if (psx_tiles_pset(gfx, x, y, &pixel)) {
        add_tile(pixel, col);
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
    use_texture(PSX_VRAM_PATTERNS, slot);
    psx_vram_rect_t clut = psx_vram_layout_pattern_clut(slot);
    for (int s = 0; s < count; s++) {
        psx_sprite_t pieces[PSX_PATTERN_PIECES_MAX];
        int sent = psx_pattern_sprites_x2(gfx, spans[s], pieces);
        for (int p = 0; p < sent; p++) {
            psx_sprite_t piece = pieces[p];
            assert(piece.u < PAGE_TEXELS && piece.u + piece.w <= PAGE_TEXELS && piece.v < PSX_GPU_TEXTURES_PATTERN_X2_SIZE
                   && piece.w >= PSX_TILES_SCALE && piece.h >= PSX_TILES_SCALE && "a patterned SPRT reads its x2 tile with no UV wrap");
            add_sprt(piece.x, piece.y, piece.u, piece.v, piece.w, piece.h, clut);
        }
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
    psx_tiles_circ(gfx, cx, cy, r, add_pixel, &col);
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

static void add_textured_sprite(psx_sprite_t sprite, psx_vram_region_t texture, psx_vram_rect_t clut) {
    use_texture(texture, NO_WINDOW);
    psx_vram_rect_t region = psx_vram_layout_region(texture);
    psx_vram_rect_t page = psx_vram_layout_texture_page(texture);
    psx_sprite_t doubled = psx_sprite_double(sprite);
    int u = (region.x - page.x) * 4 + doubled.u;
    int v = region.y - page.y + doubled.v;
    assert(u + doubled.w <= PAGE_TEXELS && v + doubled.h <= PAGE_TEXELS && "a SPRT's 8-bit UV wraps inside its page");
    add_sprt(doubled.x, doubled.y, u, v, doubled.w, doubled.h, clut);
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
    add_textured_sprite(sprite, PSX_VRAM_SPRITE_SHEET, clut);
}

static void add_glyph(psx_sprite_t glyph, void *context) {
    const psx_vram_rect_t *clut = context;
    add_textured_sprite(glyph, PSX_VRAM_FONT, *clut);
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
    add_draw_target(psx_vram_layout_region(current == 0 ? PSX_VRAM_DISPLAY_0 : PSX_VRAM_DISPLAY_1));
    add_texture_state(PSX_VRAM_SPRITE_SHEET, NO_WINDOW);
}

/* Queued behind the previous frame's DrawSync and ahead of this frame's DrawOTag, so no slot
 * changes while a frame reads it. */
static void upload_slots(const psx_clut_cache_t *cluts, const psx_pattern_cache_t *patterns) {
    for (int slot = 0; slot < psx_clut_cache_count(cluts); slot++) {
        queue_load(psx_vram_layout_palette_clut(slot), psx_clut_cache_clut(cluts, slot));
    }
    for (int slot = 0; slot < psx_pattern_cache_count(patterns); slot++) {
        psx_vram_rect_t texture = psx_vram_layout_pattern_slot(slot);
        assert(texture.w * texture.h == PSX_GPU_TEXTURES_PATTERN_X2_UNITS && "a slot holds one x2 tile");
        queue_load(texture, psx_pattern_cache_tile(patterns, slot));
        queue_load(psx_vram_layout_pattern_clut(slot), psx_pattern_cache_clut(patterns, slot));
    }
}

void psx_gpu_frame_end(unsigned long metered_bytes) {
    if (used > metered_bytes) {
        static char message[128];
        snprintf(message, sizeof message, "psx_gpu: frame %d sent %u bytes, metered %lu", frame_number, (unsigned)used,
                 metered_bytes);
        p8_fatal(message);
    }
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
