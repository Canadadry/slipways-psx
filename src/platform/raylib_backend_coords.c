#include "platform/raylib_backend_coords.h"

static int clamp_0_127(int v) {
    if (v < 0) {
        return 0;
    }
    if (v > 127) {
        return 127;
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
    *out_x = clamp_0_127(px);
    *out_y = clamp_0_127(py);
}
