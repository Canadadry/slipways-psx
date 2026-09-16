#ifndef SLIPWAYS_SCREEN_MAP_H
#define SLIPWAYS_SCREEN_MAP_H

#include "planetgen.h"

void screen_map_planet(planet_pos_t p, int *out_x, int *out_y);

/* Algebraic inverse of screen_map_planet/screen_map_axis: given screen
 * pixel coordinates, returns the corresponding world-space position. Used
 * by the live slipway-validity check for its raw-cursor-position case
 * (no planet currently hovered), mirroring jc:hq's {ra=mp,p=mp/128}
 * fallback -- see docs/prd/triage/psx-liaison-validity.md, "New
 * screen->world inverse". Pixel coordinates are integers while world
 * positions are Q16.16, so screen_map_planet -> screen_map_unproject is
 * not an exact round trip (pixel quantization); it round-trips only
 * within a small, axis-dependent tolerance -- see screen_map_test.c. */
planet_pos_t screen_map_unproject(int x, int y);

#endif
