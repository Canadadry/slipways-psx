#include "screen_map.h"

#include <stdint.h>

#include "fixed.h"
#include "screen.h"

/* Arbitrary rendering-scaffolding constant, not a traced game-design
 * value: keeps mapped planets off the literal screen edge. Revisitable
 * once a real camera/pan/zoom system exists. */
#define SCREEN_MAP_MARGIN 8

/* Q16.16 fixed-point only, no float/double at runtime: normalized in
 * [0, FX_ONE) locates coord within [PLACEMENT_MIN, PLACEMENT_MIN+SPAN),
 * then pixel stretches that independently onto [MARGIN, axis_res-MARGIN].
 * Intermediate products use int64_t: diff (< SPAN, ~180224) * FX_ONE
 * (65536) can reach ~1.18e10, which overflows int32_t but not int64_t. */
static int screen_map_axis(fixed_t coord, int axis_res) {
    int64_t diff = (int64_t)fixed_sub(coord, PLANETGEN_PLACEMENT_MIN);
    int64_t normalized = (diff * (int64_t)FX_ONE) / (int64_t)PLANETGEN_PLACEMENT_SPAN;
    int64_t usable = (int64_t)axis_res - 2 * SCREEN_MAP_MARGIN;
    int64_t pixel = SCREEN_MAP_MARGIN + (normalized * usable) / (int64_t)FX_ONE;
    return (int)pixel;
}

void screen_map_planet(planet_pos_t p, int *out_x, int *out_y) {
    *out_x = screen_map_axis(p.x, SCREEN_XRES);
    *out_y = screen_map_axis(p.y, SCREEN_YRES);
}

/* Algebraic inverse of screen_map_axis: solving
 *   pixel = MARGIN + (normalized * usable) / FX_ONE
 *   normalized = (coord - PLACEMENT_MIN) * FX_ONE / PLACEMENT_SPAN
 * for coord given pixel gives
 *   normalized = (pixel - MARGIN) * FX_ONE / usable
 *   coord = PLACEMENT_MIN + normalized * PLACEMENT_SPAN / FX_ONE
 * Same int64_t-intermediate, overflow-safe style as screen_map_axis: p-
 * MARGIN is at most axis_res (<=320) in magnitude, so
 * (p-MARGIN)*FX_ONE reaches at most ~2.1e7 (fits int64_t easily); the
 * resulting normalized is bounded by roughly FX_ONE, so
 * normalized*PLACEMENT_SPAN reaches at most ~65536*180224 =~ 1.18e10,
 * which overflows int32_t but not int64_t -- identical bound to
 * screen_map_axis's own documented reasoning. */
static fixed_t screen_map_unproject_axis(int pixel, int axis_res) {
    int64_t usable = (int64_t)axis_res - 2 * SCREEN_MAP_MARGIN;
    int64_t p = (int64_t)pixel - SCREEN_MAP_MARGIN;
    int64_t normalized = (p * (int64_t)FX_ONE) / usable;
    int64_t diff = (normalized * (int64_t)PLANETGEN_PLACEMENT_SPAN) / (int64_t)FX_ONE;
    return (fixed_t)(diff + (int64_t)PLANETGEN_PLACEMENT_MIN);
}

planet_pos_t screen_map_unproject(int x, int y) {
    planet_pos_t p;
    p.x = screen_map_unproject_axis(x, SCREEN_XRES);
    p.y = screen_map_unproject_axis(y, SCREEN_YRES);
    return p;
}
