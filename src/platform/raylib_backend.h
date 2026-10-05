#ifndef PLATFORM_RAYLIB_BACKEND_H
#define PLATFORM_RAYLIB_BACKEND_H

#include "p8/vram.h"
#include "p8/input.h"
#include "devtools/session_recorder.h"
#include "devtools/replay_control.h"
#include "platform/keymap.h"

#include "raylib.h"

#define RAYLIB_BACKEND_SCALE 6

typedef struct {
    Texture2D screen_texture;
    Color pixel_buffer[P8_SCREEN_WIDTH * P8_SCREEN_HEIGHT];
    session_recorder_t recorder;
    char markers_path[64];
    p8_input_mode_t mode;
    keymap_t keymap;
} raylib_backend_t;

void raylib_backend_init(raylib_backend_t *backend, int keyboard_cursor, p8_input_mode_t mode, const keymap_t *keymap);

int raylib_backend_should_continue(const raylib_backend_t *backend);

void raylib_backend_poll_input(raylib_backend_t *backend, p8_input_frame_t *out_frame);

void raylib_backend_present(raylib_backend_t *backend, const p8_vram_t *mem, const p8_input_frame_t *frame, const char *replay_status);

void raylib_backend_poll_replay_controls(replay_control_t *control);

void raylib_backend_set_frame_rate_cap(int target_fps);

void raylib_backend_record_frame(raylib_backend_t *backend, const p8_input_frame_t *frame);

void raylib_backend_shutdown(raylib_backend_t *backend);

#endif
