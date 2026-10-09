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
