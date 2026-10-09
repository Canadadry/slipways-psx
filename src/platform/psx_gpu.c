#include "platform/psx_gpu.h"

#include "p8/draw_cost.h"
#include "p8/fatal.h"
#include "p8/gpu_sizes.h"
#include "p8/sprite_sheet.h"
#include "platform/psx_vram_layout.h"

#include <psxgpu.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
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
/* The frame overhead the cost model meters as one DR_ENV each side of the frame. */
SIZE_CHECK(frame_setup_size_check, sizeof(DR_AREA) + sizeof(DR_OFFSET) + sizeof(DR_TWIN) <= P8_GPU_DR_ENV_BYTES);
SIZE_CHECK(frame_buffer_word_check, GPU_FRAME_BUFFER_BYTES % 4 == 0);

enum { DISPLAY_WIDTH = 320, DISPLAY_HEIGHT = 240, TEXTURE_4_BIT = 0, TEXTURE_15_BIT = 2, SHEET_UPLOAD_LINES = 16 };

static uint32_t buffers[2][GPU_FRAME_BUFFER_BYTES / 4];
static int current;
static size_t used;
static uint32_t *previous;
static int frame_number;
static DISPENV displays[2];

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
    current = 0;
    PutDispEnv(&displays[1]);
    SetDispMask(1);
}

static void gpu_cls(p8_vram_t *mem) {
    (void)mem;
    psx_vram_rect_t area = psx_vram_layout_region(PSX_VRAM_DRAW_AREA);
    FILL *fill = packet(sizeof *fill);
    setFill(fill);
    setRGB0(fill, 0, 0, 0);
    setXY0(fill, area.x, area.y);
    setWH(fill, area.w, area.h);
}

/* TODO step 4: TILE_1. */
static void gpu_pset(const p8_graphics_t *gfx, p8_vram_t *mem, int x, int y, int col) {
    (void)gfx, (void)mem, (void)x, (void)y, (void)col;
}

/* TODO step 4: LINE_F2. */
static void gpu_line(const p8_graphics_t *gfx, p8_vram_t *mem, int x1, int y1, int x2, int y2, int col) {
    (void)gfx, (void)mem, (void)x1, (void)y1, (void)x2, (void)y2, (void)col;
}

/* TODO step 4: one TILE per edge. */
static void gpu_rect(const p8_graphics_t *gfx, p8_vram_t *mem, int x1, int y1, int x2, int y2, int col) {
    (void)gfx, (void)mem, (void)x1, (void)y1, (void)x2, (void)y2, (void)col;
}

/* TODO step 4: TILE, or a pattern-slot SPRT under fillp. */
static void gpu_rectfill(const p8_graphics_t *gfx, p8_vram_t *mem, int x1, int y1, int x2, int y2, int col) {
    (void)gfx, (void)mem, (void)x1, (void)y1, (void)x2, (void)y2, (void)col;
}

/* TODO step 4: TILE_1 per midpoint pixel. */
static void gpu_circ(const p8_graphics_t *gfx, p8_vram_t *mem, int cx, int cy, int r, int col) {
    (void)gfx, (void)mem, (void)cx, (void)cy, (void)r, (void)col;
}

/* TODO step 4: TILE spans, or pattern-slot SPRT spans under fillp. */
static void gpu_circfill(const p8_graphics_t *gfx, p8_vram_t *mem, int cx, int cy, int r, int col) {
    (void)gfx, (void)mem, (void)cx, (void)cy, (void)r, (void)col;
}

/* TODO step 4: SPRT through the draw-palette CLUT cache. */
static void gpu_spr(const p8_graphics_t *gfx, p8_vram_t *mem, int n, int x, int y, int w, int h) {
    (void)gfx, (void)mem, (void)n, (void)x, (void)y, (void)w, (void)h;
}

/* TODO step 4: one SPRT per lit glyph from the font texture. */
static void gpu_print(const p8_graphics_t *gfx, p8_vram_t *mem, const char *str, int x, int y, int col) {
    (void)gfx, (void)mem, (void)str, (void)x, (void)y, (void)col;
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

void psx_gpu_frame_end(unsigned long metered_bytes) {
    add_draw_target(psx_vram_layout_region(current == 0 ? PSX_VRAM_DISPLAY_0 : PSX_VRAM_DISPLAY_1));
    add_no_texture_window();
    add_upscale();
    if (used > metered_bytes) {
        static char message[128];
        snprintf(message, sizeof message, "psx_gpu: frame %d sent %u bytes, metered %lu", frame_number, (unsigned)used,
                 metered_bytes);
        p8_fatal(message);
    }
    DrawSync(0);
    PutDispEnv(&displays[current ^ 1]);
    DrawOTag(buffers[current]);
    current ^= 1;
}

void psx_gpu_frame_show_last(void) {
    DrawSync(0);
    PutDispEnv(&displays[current ^ 1]);
}
