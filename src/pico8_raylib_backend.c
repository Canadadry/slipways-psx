#include "pico8_raylib_backend.h"

#include "pico8_raylib_backend_coords.h"
#include "pico8_gfx.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* The standard, published PICO-8 default palette (16 RGB colors), in
 * index order. Presentation-layer plumbing, not a ported function --
 * these are well-known, universally documented values (see e.g. the
 * PICO-8 wiki's "Palette" page), not something traced from pico8/
 * lua.lua. Alpha is always opaque; PICO-8's own transparency (palt())
 * is already handled inside the mimicry layer before a pixel ever
 * reaches the framebuffer this module reads. */
static const Color PICO8_PALETTE[16] = {
    { 0x00, 0x00, 0x00, 0xFF },
    { 0x1D, 0x2B, 0x53, 0xFF },
    { 0x7E, 0x25, 0x53, 0xFF },
    { 0x00, 0x87, 0x51, 0xFF },
    { 0xAB, 0x52, 0x36, 0xFF },
    { 0x5F, 0x57, 0x4F, 0xFF },
    { 0xC2, 0xC3, 0xC7, 0xFF },
    { 0xFF, 0xF1, 0xE8, 0xFF },
    { 0xFF, 0x00, 0x4D, 0xFF },
    { 0xFF, 0xA3, 0x00, 0xFF },
    { 0xFF, 0xEC, 0x27, 0xFF },
    { 0x00, 0xE4, 0x36, 0xFF },
    { 0x29, 0xAD, 0xFF, 0xFF },
    { 0x83, 0x76, 0x9C, 0xFF },
    { 0xFF, 0x77, 0xA8, 0xFF },
    { 0xFF, 0xCC, 0xAA, 0xFF },
};

/* Appends `frame` to the session's growing in-memory frame list,
 * doubling capacity as needed (same growth pattern as pico8_input.c's
 * own frame-array loading code). A failed realloc silently drops the
 * append rather than crashing a live playtest session -- the next
 * present() call's pico8_input_save will simply rewrite the same
 * (unchanged) frame count again; this is presentation/recording-layer
 * robustness, not a ported function's error-handling contract. */
static void append_session_frame(pico8_raylib_backend_t *backend, const pico8_input_frame_t *frame) {
    if (backend->session_frame_count == backend->session_frame_capacity) {
        int new_capacity = (backend->session_frame_capacity == 0) ? 64 : backend->session_frame_capacity * 2;
        pico8_input_frame_t *bigger = realloc(backend->session_frames, sizeof(pico8_input_frame_t) * (size_t)new_capacity);
        if (!bigger) {
            return;
        }
        backend->session_frames = bigger;
        backend->session_frame_capacity = new_capacity;
    }
    backend->session_frames[backend->session_frame_count++] = *frame;
}

void pico8_raylib_backend_init(pico8_raylib_backend_t *backend) {
    backend->session_frames = NULL;
    backend->session_frame_count = 0;
    backend->session_frame_capacity = 0;

    time_t now = time(NULL);
    snprintf(backend->record_path, sizeof(backend->record_path), "tmp/events_recorded_%ld.json", (long)now);
    snprintf(backend->markers_path, sizeof(backend->markers_path), "tmp/events_recorded_%ld.markers.txt", (long)now);

    InitWindow(PICO8_SCREEN_WIDTH * PICO8_RAYLIB_BACKEND_SCALE, PICO8_SCREEN_HEIGHT * PICO8_RAYLIB_BACKEND_SCALE,
               "Slipways (raylib PC port)");
    SetTargetFPS(60);

    Image blank = GenImageColor(PICO8_SCREEN_WIDTH, PICO8_SCREEN_HEIGHT, BLACK);
    backend->screen_texture = LoadTextureFromImage(blank);
    UnloadImage(blank);
    SetTextureFilter(backend->screen_texture, TEXTURE_FILTER_POINT);

    /* This cart's own nf:pq() (pico8_mouse_reader.c) unconditionally does
     * poke(0x5f2d,1) -- real PICO-8's "enable mouse" flag -- once during
     * boot, and never turns it back off. On real hardware that flag also
     * hides the host cursor, since the cart is expected to draw its own
     * (ui:ev(), pico8_ui.c, already ported and already dispatched every
     * frame via _draw()'s dc("ev")) -- without this, a human sees BOTH
     * cursors overlapping (a real second bug found alongside the title-
     * screen transparency fix: the same fix also cleaned up this cart-
     * drawn cursor sprite's own art, but never addressed the host cursor
     * still being shown underneath it). This cart's own poke(0x5f2d,1)
     * happens unconditionally on every boot (pico8_cart_load() always
     * builds nf via pico8_jl()), so hiding the host cursor once here,
     * rather than polling mem every frame for a flag this cart never
     * clears, is the correct, minimal match for this specific game. */
    HideCursor();
}

int pico8_raylib_backend_should_continue(const pico8_raylib_backend_t *backend) {
    (void)backend;
    return !WindowShouldClose();
}

void pico8_raylib_backend_poll_input(pico8_raylib_backend_t *backend, pico8_input_frame_t *out_frame) {
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

    out_frame->dpad[0] = IsKeyDown(KEY_LEFT) ? 1 : 0;
    out_frame->dpad[1] = IsKeyDown(KEY_RIGHT) ? 1 : 0;
    out_frame->dpad[2] = IsKeyDown(KEY_UP) ? 1 : 0;
    out_frame->dpad[3] = IsKeyDown(KEY_DOWN) ? 1 : 0;

    /* Debug/dev tooling only, no Lua-source equivalent and no effect on
     * *out_frame or game input: lets a human playtester bookmark the
     * frame they want to talk about by tapping space, without needing to
     * describe "it happened somewhere around..." Appended (never
     * rewritten) so a marker already on disk survives a crash later in
     * the session; session_frame_count is the index this frame will get
     * once present() appends it below, i.e. exactly the 0-based index
     * into the paired recording's own "frames" array. */
    if (IsKeyReleased(KEY_SPACE)) {
        FILE *markers = fopen(backend->markers_path, "a");
        if (markers) {
            fprintf(markers, "%d\n", backend->session_frame_count);
            fclose(markers);
        }
        printf("marker: frame %d (%s)\n", backend->session_frame_count, backend->markers_path);
        fflush(stdout);
    }
}

void pico8_raylib_backend_present(pico8_raylib_backend_t *backend, const pico8_mem_t *mem, const pico8_input_frame_t *frame) {
    for (int y = 0; y < PICO8_SCREEN_HEIGHT; y++) {
        for (int x = 0; x < PICO8_SCREEN_WIDTH; x++) {
            unsigned char idx = pico8_peek(mem, PICO8_ADDR_SCREEN + y * PICO8_SCREEN_WIDTH + x);
            backend->pixel_buffer[y * PICO8_SCREEN_WIDTH + x] = PICO8_PALETTE[idx & 0x0f];
        }
    }
    UpdateTexture(backend->screen_texture, backend->pixel_buffer);

    BeginDrawing();
    ClearBackground(BLACK);
    DrawTextureEx(backend->screen_texture, (Vector2){ 0, 0 }, 0.0f, (float)PICO8_RAYLIB_BACKEND_SCALE, WHITE);
    EndDrawing();

    append_session_frame(backend, frame);
    pico8_input_save(backend->session_frames, backend->session_frame_count, backend->record_path);
}

void pico8_raylib_backend_shutdown(pico8_raylib_backend_t *backend) {
    UnloadTexture(backend->screen_texture);
    ShowCursor();
    CloseWindow();
    free(backend->session_frames);
    backend->session_frames = NULL;
    backend->session_frame_count = 0;
    backend->session_frame_capacity = 0;
}
