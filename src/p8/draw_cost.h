#ifndef P8_DRAW_COST_H
#define P8_DRAW_COST_H

#include "p8/gfx.h"

/* PSn00bSDK packet sizes on MIPS, tag included. */
enum {
    P8_GPU_TILE_BYTES = 16,
    P8_GPU_LINE_F2_BYTES = 16,
    P8_GPU_SPRT_BYTES = 20,
    P8_GPU_DR_TPAGE_BYTES = 8,
    P8_GPU_DR_TWIN_BYTES = 8,
    P8_GPU_DR_ENV_BYTES = 36,
};

typedef struct {
    unsigned long bytes;
    int primitives;
} p8_draw_cost_t;

/* The texture window of the frame's last textured call: none for spr and print, the pattern key
 * for a patterned fill. */
typedef struct {
    int patterned;
    unsigned long pattern_key;
} p8_draw_cost_window_t;

/* An upper bound of what the GPU backend may emit for each call, 0 when the call's screen bounding
 * box misses 0..159 x 0..119. Coordinates are the API's, before gfx's camera. */

/* Resets *window to the frame's initial state and returns the fixed per-frame overhead
 * (draw environment, texture state). */
p8_draw_cost_t p8_draw_cost_frame_begin(p8_draw_cost_window_t *window);

/* What tells two pattern slots apart: the pattern, the colour byte and the transparent flag. */
unsigned long p8_draw_cost_pattern_key(const p8_graphics_t *gfx, int col);

p8_draw_cost_t p8_draw_cost_cls(void);
p8_draw_cost_t p8_draw_cost_pset(const p8_graphics_t *gfx, int x, int y);
p8_draw_cost_t p8_draw_cost_line(const p8_graphics_t *gfx, int x1, int y1, int x2, int y2);
p8_draw_cost_t p8_draw_cost_rect(const p8_graphics_t *gfx, int x1, int y1, int x2, int y2);
p8_draw_cost_t p8_draw_cost_rectfill(p8_draw_cost_window_t *window, const p8_graphics_t *gfx, int x1, int y1, int x2, int y2,
                                     int col);
p8_draw_cost_t p8_draw_cost_circ(const p8_graphics_t *gfx, int cx, int cy, int r);
p8_draw_cost_t p8_draw_cost_circfill(p8_draw_cost_window_t *window, const p8_graphics_t *gfx, int cx, int cy, int r, int col);
p8_draw_cost_t p8_draw_cost_spr(p8_draw_cost_window_t *window, const p8_graphics_t *gfx, int n, int x, int y, int w, int h);
p8_draw_cost_t p8_draw_cost_print(p8_draw_cost_window_t *window, const p8_graphics_t *gfx, const char *str, int x, int y);

#endif
