#ifndef PLATFORM_RAYLIB_BACKEND_COORDS_H
#define PLATFORM_RAYLIB_BACKEND_COORDS_H

#include "p8/vram.h"

void raylib_backend_window_to_screen(int window_x, int window_y, int scale, int *out_x, int *out_y);

#endif
