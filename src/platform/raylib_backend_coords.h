#ifndef PLATFORM_RAYLIB_BACKEND_COORDS_H
#define PLATFORM_RAYLIB_BACKEND_COORDS_H

#include "p8/vram.h"

/* Divides by `scale` and clamps into the screen: raylib still reports a
 * cursor outside the window while a button is held. A `scale` of 0 or
 * less maps to (0, 0). */
void raylib_backend_window_to_screen(int window_x, int window_y, int scale, int *out_x, int *out_y);

#endif
