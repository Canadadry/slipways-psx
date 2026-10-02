#include "platform/raylib_backend.h"

#include "platform/raylib_backend_coords.h"
#include "p8/gfx.h"
#include "p8/palette.h"

#include <stdio.h>
#include <time.h>

/* La table partagée est sans raylib (trois octets par couleur) pour que
 * `make unit` l'asserte sans fenêtre ; l'alpha, toujours opaque, s'ajoute
 * ici. Le second binaire raylib la lit là aussi et n'en redéfinit pas. */
static Color palette_color(unsigned char index) {
    p8_rgb_t rgb = P8_PALETTE_RGB[index & 0x0f];
    return (Color){ rgb.red, rgb.green, rgb.blue, 0xFF };
}

void pico8_raylib_backend_init(pico8_raylib_backend_t *backend) {
    /* One time(NULL) for both names: the marker file is only usable if a
     * human can pair it with its recording by name, and two clock reads
     * could straddle a second. */
    time_t now = time(NULL);
    char record_path[PICO8_SESSION_RECORDER_PATH_MAX];
    snprintf(record_path, sizeof(record_path), "tmp/events_recorded_%ld.json", (long)now);
    snprintf(backend->markers_path, sizeof(backend->markers_path), "tmp/events_recorded_%ld.markers.txt", (long)now);
    pico8_session_recorder_open(&backend->recorder, record_path);

    InitWindow(P8_SCREEN_WIDTH * PICO8_RAYLIB_BACKEND_SCALE, P8_SCREEN_HEIGHT * PICO8_RAYLIB_BACKEND_SCALE,
               "Slipways (raylib PC port)");
    SetTargetFPS(60);

    Image blank = GenImageColor(P8_SCREEN_WIDTH, P8_SCREEN_HEIGHT, BLACK);
    backend->screen_texture = LoadTextureFromImage(blank);
    UnloadImage(blank);
    SetTextureFilter(backend->screen_texture, TEXTURE_FILTER_POINT);

    /* Hidden once, not polled per frame: nf:pq() pokes 0x5f2d on every
     * boot and never clears it. */
    HideCursor();
}

int pico8_raylib_backend_should_continue(const pico8_raylib_backend_t *backend) {
    (void)backend;
    return !WindowShouldClose();
}

void pico8_raylib_backend_poll_input(pico8_raylib_backend_t *backend, p8_input_frame_t *out_frame) {
    int px;
    int py;
    pico8_raylib_backend_window_to_pico8(GetMouseX(), GetMouseY(), PICO8_RAYLIB_BACKEND_SCALE, &px, &py);
    out_frame->mouse_x = px;
    out_frame->mouse_y = py;

    int buttons = 0;
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        buttons |= 1;
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        buttons |= 2;
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        buttons |= 4;
    }
    out_frame->mouse_buttons = buttons;

    /* TROU-2 */
    out_frame->dpad[0] = IsKeyDown(KEY_LEFT) ? 1 : 0;
    out_frame->dpad[1] = IsKeyDown(KEY_RIGHT) ? 1 : 0;
    out_frame->dpad[2] = IsKeyDown(KEY_UP) ? 1 : 0;
    out_frame->dpad[3] = IsKeyDown(KEY_DOWN) ? 1 : 0;
    out_frame->dpad[4] = IsKeyDown(KEY_Z) ? 1 : 0;
    out_frame->dpad[5] = IsKeyDown(KEY_X) ? 1 : 0;

    /* Appended rather than rewritten, so a marker already on disk
     * survives a later crash. The count read here is the index this frame
     * gets when the caller records it on its very next call. */
    if (IsKeyReleased(KEY_SPACE)) {
        int marked_frame = pico8_session_recorder_frame_count(&backend->recorder);
        FILE *markers = fopen(backend->markers_path, "a");
        if (markers) {
            fprintf(markers, "%d\n", marked_frame);
            fclose(markers);
        }
        printf("marker: frame %d (%s)\n", marked_frame, backend->markers_path);
        fflush(stdout);
    }
}

void pico8_raylib_backend_poll_replay_controls(pico8_replay_control_t *control) {
    if (IsKeyPressed(KEY_PAGE_UP)) {
        pico8_replay_control_faster(control);
    }
    if (IsKeyPressed(KEY_PAGE_DOWN)) {
        pico8_replay_control_slower(control);
    }
    if (IsKeyPressed(KEY_END)) {
        pico8_replay_control_uncap(control);
    }
}

void pico8_raylib_backend_set_frame_rate_cap(int target_fps) {
    SetTargetFPS(target_fps);
}

void pico8_raylib_backend_present(pico8_raylib_backend_t *backend, const p8_memory_t *mem, const p8_input_frame_t *frame, const char *replay_status) {
    for (int y = 0; y < P8_SCREEN_HEIGHT; y++) {
        for (int x = 0; x < P8_SCREEN_WIDTH; x++) {
            unsigned char idx = p8_peek(mem, P8_ADDR_SCREEN + y * P8_SCREEN_WIDTH + x);
            backend->pixel_buffer[y * P8_SCREEN_WIDTH + x] = palette_color(idx);
        }
    }
    UpdateTexture(backend->screen_texture, backend->pixel_buffer);

    BeginDrawing();
    ClearBackground(BLACK);
    DrawTextureEx(backend->screen_texture, (Vector2){ 0, 0 }, 0.0f, (float)PICO8_RAYLIB_BACKEND_SCALE, WHITE);
    /* The band behind the text is there because the cart owns every pixel
     * underneath. */
    if (replay_status) {
        DrawRectangle(0, 0, P8_SCREEN_WIDTH * PICO8_RAYLIB_BACKEND_SCALE, 26, (Color){ 0, 0, 0, 190 });
        DrawText(replay_status, 8, 5, 18, RAYWHITE);
    }
    EndDrawing();

    (void)frame;
}

void pico8_raylib_backend_record_frame(pico8_raylib_backend_t *backend, const p8_input_frame_t *frame) {
    /* The return value is ignored on purpose: the recorder leaves its own
     * count alone on failure, so the marker file keeps naming the right
     * frame. */
    (void)pico8_session_recorder_record_frame(&backend->recorder, frame);
}

void pico8_raylib_backend_shutdown(pico8_raylib_backend_t *backend) {
    UnloadTexture(backend->screen_texture);
    ShowCursor();
    CloseWindow();
    pico8_session_recorder_close(&backend->recorder);
}
