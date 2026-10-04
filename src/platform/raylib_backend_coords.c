#include "platform/raylib_backend_coords.h"

static int clamp_to_axis(int v, int size) {
    if (v < 0) {
        return 0;
    }
    if (v > size - 1) {
        return size - 1;
    }
    return v;
}

void raylib_backend_window_to_screen(int window_x, int window_y, int scale, int *out_x, int *out_y) {
    int px = 0;
    int py = 0;
    if (scale > 0) {
        px = window_x / scale;
        py = window_y / scale;
    }
    *out_x = clamp_to_axis(px, P8_SCREEN_WIDTH);
    *out_y = clamp_to_axis(py, P8_SCREEN_HEIGHT);
}
