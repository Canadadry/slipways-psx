#include "platform/psx_tiles.h"

#include "p8/vram.h"

static int min(int a, int b) {
    return a < b ? a : b;
}

static int max(int a, int b) {
    return a < b ? b : a;
}

/* Bounds are inclusive. Returns the new count, unchanged when the clipped box is empty. */
static int add_clipped(psx_tile_t *tiles, int count, int xmin, int ymin, int xmax, int ymax) {
    xmin = max(xmin, 0);
    ymin = max(ymin, 0);
    xmax = min(xmax, P8_SCREEN_WIDTH - 1);
    ymax = min(ymax, P8_SCREEN_HEIGHT - 1);
    if (xmin > xmax || ymin > ymax) {
        return count;
    }
    psx_tile_t tile = { xmin, ymin, xmax - xmin + 1, ymax - ymin + 1 };
    tiles[count] = tile;
    return count + 1;
}

int psx_tiles_rect(const p8_graphics_t *gfx, int x1, int y1, int x2, int y2, psx_tile_t tiles[PSX_TILES_RECT_MAX]) {
    int xmin = min(x1, x2) - gfx->camera_x;
    int xmax = max(x1, x2) - gfx->camera_x;
    int ymin = min(y1, y2) - gfx->camera_y;
    int ymax = max(y1, y2) - gfx->camera_y;
    if (xmax - xmin < 2 || ymax - ymin < 2) {
        return add_clipped(tiles, 0, xmin, ymin, xmax, ymax);
    }
    int count = add_clipped(tiles, 0, xmin, ymin, xmax, ymin);
    count = add_clipped(tiles, count, xmin, ymax, xmax, ymax);
    count = add_clipped(tiles, count, xmin, ymin + 1, xmin, ymax - 1);
    return add_clipped(tiles, count, xmax, ymin + 1, xmax, ymax - 1);
}

int psx_tiles_rectfill(const p8_graphics_t *gfx, int x1, int y1, int x2, int y2, psx_tile_t *tile) {
    return add_clipped(tile, 0, min(x1, x2) - gfx->camera_x, min(y1, y2) - gfx->camera_y, max(x1, x2) - gfx->camera_x,
                       max(y1, y2) - gfx->camera_y);
}

int psx_tiles_pset(const p8_graphics_t *gfx, int x, int y, psx_tile_t *tile) {
    return add_clipped(tile, 0, x - gfx->camera_x, y - gfx->camera_y, x - gfx->camera_x, y - gfx->camera_y);
}

int psx_tiles_line(const p8_graphics_t *gfx, int x1, int y1, int x2, int y2, psx_line_t *line) {
    psx_tile_t box;
    if (!psx_tiles_rectfill(gfx, x1, y1, x2, y2, &box)) {
        return 0;
    }
    psx_line_t sent = { x1 - gfx->camera_x, y1 - gfx->camera_y, x2 - gfx->camera_x, y2 - gfx->camera_y };
    *line = sent;
    return 1;
}

/* The same loop as gfx_software.c's circles and draw_cost.c's circle_iterations: change all three
 * together. Iterate with for (m = midpoint_start(r); m.x >= m.y; midpoint_advance(&m)). */
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

static int emit_clipped(psx_tiles_emit_t emit, void *context, int xmin, int ymin, int xmax, int ymax) {
    psx_tile_t tile;
    if (!add_clipped(&tile, 0, xmin, ymin, xmax, ymax)) {
        return 0;
    }
    emit(tile, context);
    return 1;
}

/* The runs lo..hi and -hi..-lo along one axis, one run when lo is 0, at +across and -across on the
 * other axis; columns when along_rows is 0, else rows. Offsets are from (sx, sy). */
static int emit_mirrored_runs(int sx, int sy, int across, int lo, int hi, int along_rows, psx_tiles_emit_t emit,
                              void *context) {
    int count = 0;
    for (int side = across == 0 ? 1 : 0; side < 2; side++) {
        int at = side == 0 ? -across : across;
        int runs[2][2] = { { lo == 0 ? -hi : lo, hi }, { -hi, -lo } };
        for (int run = 0; run < (lo == 0 ? 1 : 2); run++) {
            if (along_rows) {
                count += emit_clipped(emit, context, sx + runs[run][0], sy + at, sx + runs[run][1], sy + at);
            } else {
                count += emit_clipped(emit, context, sx + at, sy + runs[run][0], sx + at, sy + runs[run][1]);
            }
        }
    }
    return count;
}

/* The steps sharing one x, y from y0 to y1: their pixels with |dx| >= |dy| as column runs, the others
 * (y < x) as row runs, so that no pixel is emitted twice. */
static int emit_step_group(int sx, int sy, int x, int y0, int y1, psx_tiles_emit_t emit, void *context) {
    int count = emit_mirrored_runs(sx, sy, x, y0, y1, 0, emit, context);
    int row_hi = min(y1, x - 1);
    if (y0 <= row_hi) {
        count += emit_mirrored_runs(sx, sy, x, y0, row_hi, 1, emit, context);
    }
    return count;
}

int psx_tiles_circ(const p8_graphics_t *gfx, int cx, int cy, int r, psx_tiles_emit_t emit, void *context) {
    psx_tile_t box;
    if (r < 0 || !psx_tiles_rectfill(gfx, cx - r, cy - r, cx + r, cy + r, &box)) {
        return 0;
    }
    int sx = cx - gfx->camera_x;
    int sy = cy - gfx->camera_y;
    int count = 0;
    int group_x = r;
    int group_y0 = 0;
    int group_y1 = 0;
    for (midpoint_t m = midpoint_start(r); m.x >= m.y; midpoint_advance(&m)) {
        if (m.x != group_x) {
            count += emit_step_group(sx, sy, group_x, group_y0, group_y1, emit, context);
            group_x = m.x;
            group_y0 = m.y;
        }
        group_y1 = m.y;
    }
    return count + emit_step_group(sx, sy, group_x, group_y0, group_y1, emit, context);
}

static void widen(int half_widths[P8_SCREEN_HEIGHT], int row, int half_width) {
    if (row >= 0 && row < P8_SCREEN_HEIGHT && half_widths[row] < half_width) {
        half_widths[row] = half_width;
    }
}

int psx_tiles_circfill(const p8_graphics_t *gfx, int cx, int cy, int r, psx_tile_t spans[PSX_TILES_CIRCFILL_MAX]) {
    psx_tile_t box;
    if (r < 0 || !psx_tiles_rectfill(gfx, cx - r, cy - r, cx + r, cy + r, &box)) {
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
    int count = 0;
    for (int row = 0; row < P8_SCREEN_HEIGHT; row++) {
        if (half_widths[row] >= 0) {
            count = add_clipped(spans, count, sx - half_widths[row], row, sx + half_widths[row], row);
        }
    }
    return count;
}
