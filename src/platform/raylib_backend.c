#include "platform/raylib_backend.h"

#include "platform/keymap_names.h"
#include "p8/gfx.h"
#include "p8/palette.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#define KEYMAP_CHECK_KEY(name, code) typedef char keymap_check_key_##name[(KEY_##name == (code)) ? 1 : -1];
#define KEYMAP_CHECK_GAMEPAD(name, code) typedef char keymap_check_gamepad_##name[(GAMEPAD_BUTTON_##name == (code)) ? 1 : -1];
KEYMAP_KEYBOARD_NAMES(KEYMAP_CHECK_KEY)
KEYMAP_GAMEPAD_NAMES(KEYMAP_CHECK_GAMEPAD)

static Color palette_color(unsigned char index) {
    p8_rgb_t rgb = P8_PALETTE_RGB[index & 0x0f];
    return (Color){ rgb.red, rgb.green, rgb.blue, 0xFF };
}

void raylib_backend_init(raylib_backend_t *backend, const keymap_t *keymap) {
    backend->keymap = *keymap;
    /* One clock read for both names: a human pairs the marker file with its recording by name. */
    time_t now = time(NULL);
    char record_path[SESSION_RECORDER_PATH_MAX];
    snprintf(record_path, sizeof(record_path), "tmp/events_recorded_%ld.json", (long)now);
    snprintf(backend->markers_path, sizeof(backend->markers_path), "tmp/events_recorded_%ld.markers.txt", (long)now);
    session_recorder_open(&backend->recorder, record_path);

    SetTraceLogLevel(LOG_WARNING);
    InitWindow(P8_SCREEN_WIDTH * RAYLIB_BACKEND_SCALE, P8_SCREEN_HEIGHT * RAYLIB_BACKEND_SCALE,
               "Slipways (raylib PC port)");
    SetTargetFPS(60);

    Image blank = GenImageColor(P8_SCREEN_WIDTH, P8_SCREEN_HEIGHT, BLACK);
    backend->screen_texture = LoadTextureFromImage(blank);
    UnloadImage(blank);
    SetTextureFilter(backend->screen_texture, TEXTURE_FILTER_POINT);

    HideCursor();
}

int raylib_backend_should_continue(const raylib_backend_t *backend) {
    (void)backend;
    return !WindowShouldClose();
}

static int binding_down(keymap_device_t device, int code, void *context) {
    (void)context;
    if (device == KEYMAP_KEYBOARD) {
        return IsKeyDown(code);
    }
    /* Checked every frame, so a gamepad plugged in mid-game starts working. */
    return IsGamepadAvailable(0) && IsGamepadButtonDown(0, code);
}

void raylib_backend_poll_input(raylib_backend_t *backend, p8_input_frame_t *out_frame) {
    memset(out_frame, 0, sizeof *out_frame);
    out_frame->pad_buttons = keymap_pad_buttons(&backend->keymap, binding_down, NULL);

    if (IsKeyReleased(KEY_SPACE)) {
        int marked_frame = session_recorder_frame_count(&backend->recorder);
        FILE *markers = fopen(backend->markers_path, "a");
        if (markers) {
            fprintf(markers, "%d\n", marked_frame);
            fclose(markers);
        }
        printf("marker: frame %d (%s)\n", marked_frame, backend->markers_path);
        fflush(stdout);
    }
}

void raylib_backend_poll_replay_controls(replay_control_t *control) {
    if (IsKeyPressed(KEY_PAGE_UP)) {
        replay_control_faster(control);
    }
    if (IsKeyPressed(KEY_PAGE_DOWN)) {
        replay_control_slower(control);
    }
    if (IsKeyPressed(KEY_END)) {
        replay_control_uncap(control);
    }
}

void raylib_backend_set_frame_rate_cap(int target_fps) {
    SetTargetFPS(target_fps);
}

void raylib_backend_present(raylib_backend_t *backend, const p8_vram_t *mem, const p8_input_frame_t *frame, const char *replay_status) {
    for (int y = 0; y < P8_SCREEN_HEIGHT; y++) {
        for (int x = 0; x < P8_SCREEN_WIDTH; x++) {
            unsigned char idx = mem->screen[y][x];
            backend->pixel_buffer[y * P8_SCREEN_WIDTH + x] = palette_color(idx);
        }
    }
    UpdateTexture(backend->screen_texture, backend->pixel_buffer);

    BeginDrawing();
    ClearBackground(BLACK);
    DrawTextureEx(backend->screen_texture, (Vector2){ 0, 0 }, 0.0f, (float)RAYLIB_BACKEND_SCALE, WHITE);
    if (replay_status) {
        DrawRectangle(0, 0, P8_SCREEN_WIDTH * RAYLIB_BACKEND_SCALE, 26, (Color){ 0, 0, 0, 190 });
        DrawText(replay_status, 8, 5, 18, RAYWHITE);
    }
    EndDrawing();

    (void)frame;
}

void raylib_backend_record_frame(raylib_backend_t *backend, const p8_input_frame_t *frame) {
    (void)session_recorder_record_frame(&backend->recorder, frame);
}

void raylib_backend_shutdown(raylib_backend_t *backend) {
    UnloadTexture(backend->screen_texture);
    ShowCursor();
    CloseWindow();
    session_recorder_close(&backend->recorder);
}
