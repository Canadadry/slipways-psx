#include "p8/draw_cost.h"

#include "p8/vram.h"

static const p8_draw_cost_t FREE = { 0, 0 };

static p8_draw_cost_t cost_of(unsigned long bytes_each, int primitives) {
    p8_draw_cost_t cost = { bytes_each * (unsigned long)primitives, primitives };
    return cost;
}

static int on_screen(const p8_graphics_t *gfx, int xmin, int ymin, int xmax, int ymax) {
    xmin -= gfx->camera_x;
    xmax -= gfx->camera_x;
    ymin -= gfx->camera_y;
    ymax -= gfx->camera_y;
    return xmax >= 0 && xmin < P8_SCREEN_WIDTH && ymax >= 0 && ymin < P8_SCREEN_HEIGHT;
}

static int min(int a, int b) {
    return a < b ? a : b;
}

static int max(int a, int b) {
    return a < b ? b : a;
}

static int box_on_screen(const p8_graphics_t *gfx, int x1, int y1, int x2, int y2) {
    return on_screen(gfx, min(x1, x2), min(y1, y2), max(x1, x2), max(y1, y2));
}

static p8_draw_cost_t cost_add(p8_draw_cost_t a, p8_draw_cost_t b) {
    a.bytes += b.bytes;
    a.primitives += b.primitives;
    return a;
}

static p8_draw_cost_t window_pair(void) {
    p8_draw_cost_t pair = { P8_GPU_DR_TPAGE_BYTES + P8_GPU_DR_TWIN_BYTES, 2 };
    return pair;
}

static p8_draw_cost_t window_switch(p8_draw_cost_window_t *window, p8_draw_cost_texture_t texture, unsigned long pattern_key) {
    if (window->texture == texture && window->pattern_key == pattern_key) {
        return FREE;
    }
    window->texture = texture;
    window->pattern_key = pattern_key;
    return window_pair();
}

p8_draw_cost_t p8_draw_cost_frame_begin(p8_draw_cost_window_t *window) {
    window->texture = P8_DRAW_COST_TEXTURE_SHEET;
    window->pattern_key = 0;
    return cost_add(cost_of(P8_GPU_DR_ENV_BYTES, 1), window_pair());
}

unsigned long p8_draw_cost_pattern_key(const p8_graphics_t *gfx, int col) {
    return (gfx->fillp_pattern & 0xFFFFul) | ((unsigned long)(col & 0xFF) << 16) | ((unsigned long)(gfx->fillp_transparent != 0) << 24);
}

static int patterned(const p8_graphics_t *gfx) {
    return (gfx->fillp_pattern & 0xFFFFu) != 0;
}

/* 1 when the patterned span from screen x xmin to xmax, once clipped, needs a second SPRT. */
static int span_splits(const p8_graphics_t *gfx, int xmin, int xmax) {
    xmin = max(xmin, 0);
    xmax = min(xmax, P8_SCREEN_WIDTH - 1);
    int phase = ((xmin + gfx->camera_x) % P8_GPU_PATTERN_PHASES + P8_GPU_PATTERN_PHASES) % P8_GPU_PATTERN_PHASES;
    return xmin <= xmax && (phase + xmax - xmin + 1) * P8_GPU_SCALE > P8_GPU_SPRT_U_LIMIT;
}

static p8_draw_cost_t fill_spans(p8_draw_cost_window_t *window, const p8_graphics_t *gfx, int col, int spans, int splits) {
    if (!patterned(gfx)) {
        return cost_of(P8_GPU_TILE_BYTES, spans);
    }
    p8_draw_cost_t pair = window_switch(window, P8_DRAW_COST_TEXTURE_PATTERN, p8_draw_cost_pattern_key(gfx, col));
    return cost_add(pair, cost_of(P8_GPU_SPRT_BYTES, spans + splits));
}

p8_draw_cost_t p8_draw_cost_cls(void) {
    return cost_of(P8_GPU_TILE_BYTES, 1);
}

p8_draw_cost_t p8_draw_cost_pset(const p8_graphics_t *gfx, int x, int y) {
    return on_screen(gfx, x, y, x, y) ? cost_of(P8_GPU_TILE_BYTES, 1) : FREE;
}

p8_draw_cost_t p8_draw_cost_line(const p8_graphics_t *gfx, int x1, int y1, int x2, int y2) {
    return box_on_screen(gfx, x1, y1, x2, y2) ? cost_of(P8_GPU_LINE_F2_BYTES, 1) : FREE;
}

p8_draw_cost_t p8_draw_cost_rect(const p8_graphics_t *gfx, int x1, int y1, int x2, int y2) {
    return box_on_screen(gfx, x1, y1, x2, y2) ? cost_of(P8_GPU_TILE_BYTES, 4) : FREE;
}

p8_draw_cost_t p8_draw_cost_rectfill(p8_draw_cost_window_t *window, const p8_graphics_t *gfx, int x1, int y1, int x2, int y2,
                                     int col) {
    if (!box_on_screen(gfx, x1, y1, x2, y2)) {
        return FREE;
    }
    int splits = patterned(gfx) && span_splits(gfx, min(x1, x2) - gfx->camera_x, max(x1, x2) - gfx->camera_x);
    return fill_spans(window, gfx, col, 1, splits);
}

/* Must step as gfx_software.c's midpoint loop does, or circles go undercounted. Iterate with
 * for (m = midpoint_start(r); m.x >= m.y; midpoint_advance(&m)). */
typedef struct {
    int x;
    int y;
    int err;
} midpoint_t;

static midpoint_t midpoint_start(int r) {
    midpoint_t m = { r, 0, 0 };
    return m;
}

static void midpoint_advance(midpoint_t *m) {
    m->y++;
    if (m->err <= 0) {
        m->err += 2 * m->y + 1;
    }
    if (m->err > 0) {
        m->x--;
        m->err -= 2 * m->x + 1;
    }
}

static int circle_iterations(int r) {
    int iterations = 0;
    for (midpoint_t m = midpoint_start(r); m.x >= m.y; midpoint_advance(&m)) {
        iterations++;
    }
    return iterations;
}

static void widen(int half_widths[P8_SCREEN_HEIGHT], int row, int half_width) {
    if (row >= 0 && row < P8_SCREEN_HEIGHT && half_widths[row] < half_width) {
        half_widths[row] = half_width;
    }
}

/* The disc's screen rows whose patterned span needs a second SPRT. */
static int circle_splits(const p8_graphics_t *gfx, int cx, int cy, int r) {
    if ((2 * r + 1 + P8_GPU_PATTERN_PHASES - 1) * P8_GPU_SCALE <= P8_GPU_SPRT_U_LIMIT) {
        return 0;
    }
    int sx = cx - gfx->camera_x;
    int sy = cy - gfx->camera_y;
    int half_widths[P8_SCREEN_HEIGHT];
    for (int row = 0; row < P8_SCREEN_HEIGHT; row++) {
        half_widths[row] = -1;
    }
    for (midpoint_t m = midpoint_start(r); m.x >= m.y; midpoint_advance(&m)) {
        widen(half_widths, sy + m.y, m.x);
        widen(half_widths, sy - m.y, m.x);
        widen(half_widths, sy + m.x, m.y);
        widen(half_widths, sy - m.x, m.y);
    }
    int splits = 0;
    for (int row = 0; row < P8_SCREEN_HEIGHT; row++) {
        splits += half_widths[row] >= 0 && span_splits(gfx, sx - half_widths[row], sx + half_widths[row]);
    }
    return splits;
}

static int circle_on_screen(const p8_graphics_t *gfx, int cx, int cy, int r) {
    return r >= 0 && on_screen(gfx, cx - r, cy - r, cx + r, cy + r);
}

p8_draw_cost_t p8_draw_cost_circ(const p8_graphics_t *gfx, int cx, int cy, int r) {
    return circle_on_screen(gfx, cx, cy, r) ? cost_of(P8_GPU_TILE_BYTES, 8 * circle_iterations(r)) : FREE;
}

p8_draw_cost_t p8_draw_cost_circfill(p8_draw_cost_window_t *window, const p8_graphics_t *gfx, int cx, int cy, int r, int col) {
    if (!circle_on_screen(gfx, cx, cy, r)) {
        return FREE;
    }
    int splits = patterned(gfx) ? circle_splits(gfx, cx, cy, r) : 0;
    return fill_spans(window, gfx, col, 4 * circle_iterations(r), splits);
}

p8_draw_cost_t p8_draw_cost_spr(p8_draw_cost_window_t *window, const p8_graphics_t *gfx, int n, int x, int y, int w, int h) {
    (void)n;
    if (w <= 0 || h <= 0 || !on_screen(gfx, x, y, x + w * P8_SPRITE_SIZE - 1, y + h * P8_SPRITE_SIZE - 1)) {
        return FREE;
    }
    return cost_add(window_switch(window, P8_DRAW_COST_TEXTURE_SHEET, 0), cost_of(P8_GPU_SPRT_BYTES, 1));
}

/* Only the space glyph has no lit row. */
p8_draw_cost_t p8_draw_cost_print(p8_draw_cost_window_t *window, const p8_graphics_t *gfx, const char *str, int x, int y) {
    int length = 0;
    int glyphs = 0;
    for (const char *p = str; *p != '\0'; p++) {
        length++;
        glyphs += *p != ' ';
    }
    if (glyphs == 0 || !on_screen(gfx, x, y, x + (length - 1) * P8_FONT_CELL_WIDTH + P8_FONT_GLYPH_WIDTH - 1, y + P8_FONT_GLYPH_HEIGHT - 1)) {
        return FREE;
    }
    return cost_add(window_switch(window, P8_DRAW_COST_TEXTURE_FONT, 0), cost_of(P8_GPU_SPRT_BYTES, glyphs));
}
