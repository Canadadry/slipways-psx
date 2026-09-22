#include "pico8_raylib_backend.h"

#include "pico8_raylib_backend_coords.h"
#include "pico8_gfx.h"
#include "pico8_palette.h"

#include <stdio.h>
#include <time.h>

/* The standard, published PICO-8 default palette, which used to be a
 * `static const Color` table right here. It moved to src/pico8_palette.
 * {c,h} in Chunk 4 of docs/prd/triage/screencast-capture-and-viewer.md,
 * for a sharing reason rather than a design one: that chunk adds a
 * SECOND raylib program (src/main_screencast_viewer.c) which presents
 * the very same 128x128 PICO-8 screen memory, and its PRD forbids it a
 * palette of its own -- "le visualiseur affiche exactement la meme
 * chose et ne doit pas redefinir sa propre palette". A copied table
 * would agree with this one right up until the day somebody corrects
 * only one of them.
 *
 * Nothing about what this file draws changed: the same sixteen RGB
 * values, in the same index order, with the same always-opaque alpha --
 * which is now added here, at the point of use, because the shared
 * table is deliberately raylib-free (three bytes per color, not a
 * `Color`) so that `make unit` can assert its values without a window.
 * Still presentation-layer plumbing, not a ported function; PICO-8's
 * own transparency (palt()) is already handled inside the mimicry layer
 * before a pixel ever reaches the framebuffer this module reads. */
static Color palette_color(unsigned char index) {
    pico8_rgb_t rgb = PICO8_PALETTE_RGB[index & 0x0f];
    return (Color){ rgb.r, rgb.g, rgb.b, 0xFF };
}

void pico8_raylib_backend_init(pico8_raylib_backend_t *backend) {
    /* One time(NULL) for both names, because the whole point of the
     * marker file is that a human can pair it with its recording by
     * name; two clock reads could straddle a second and silently break
     * that pairing. It is also why the recording's path is minted here
     * and handed to the recorder rather than chosen by it -- see
     * pico8_session_recorder.h. */
    time_t now = time(NULL);
    char record_path[PICO8_SESSION_RECORDER_PATH_MAX];
    snprintf(record_path, sizeof(record_path), "tmp/events_recorded_%ld.json", (long)now);
    snprintf(backend->markers_path, sizeof(backend->markers_path), "tmp/events_recorded_%ld.markers.txt", (long)now);
    pico8_session_recorder_open(&backend->recorder, record_path);

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
     * the session; the recorder's frame count is the index this frame gets
     * once pico8_raylib_backend_record_frame() appends it (the caller's
     * very next call, immediately after this one), i.e. exactly the
     * 0-based index into the paired recording's own "frames" array. */
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
    /* Three keys, three calls, and nothing else in this function: no
     * frame is touched, no marker is written, and the backend is not
     * even in scope. That narrowness is the design, not an accident of
     * it -- see this function's declaration in the header for why the
     * replay cannot simply borrow poll_input. IsKeyPressed rather than
     * IsKeyDown so that one tap is exactly one level, which is what
     * makes a speed the player can count to rather than guess at. */
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
    /* Passed through untranslated, 0 included: raylib's own SetTargetFPS
     * treats anything below 1 as "do not wait between frames at all",
     * which is precisely the uncapped level, so there is no mapping
     * here to get wrong. The entire abbreviation mechanism is this one
     * line -- shortening the wait between frames, never skipping one
     * (see the declaration in the header, and the PRD's own constraint
     * quoted there). */
    SetTargetFPS(target_fps);
}

void pico8_raylib_backend_present(pico8_raylib_backend_t *backend, const pico8_mem_t *mem, const pico8_input_frame_t *frame, const char *replay_status) {
    for (int y = 0; y < PICO8_SCREEN_HEIGHT; y++) {
        for (int x = 0; x < PICO8_SCREEN_WIDTH; x++) {
            unsigned char idx = pico8_peek(mem, PICO8_ADDR_SCREEN + y * PICO8_SCREEN_WIDTH + x);
            backend->pixel_buffer[y * PICO8_SCREEN_WIDTH + x] = palette_color(idx);
        }
    }
    UpdateTexture(backend->screen_texture, backend->pixel_buffer);

    BeginDrawing();
    ClearBackground(BLACK);
    DrawTextureEx(backend->screen_texture, (Vector2){ 0, 0 }, 0.0f, (float)PICO8_RAYLIB_BACKEND_SCALE, WHITE);
    /* The replay's progress line, and the only raylib-drawn thing in
     * this port -- see this function's declaration for why user story 6
     * earns the exception and how it is kept to one. Drawn here, after
     * the framebuffer has already been uploaded and blitted, so it is
     * physically incapable of altering a single PICO-8 pixel or a single
     * recorded frame: it lands on the window, never on the game. A
     * translucent band behind it because the line has to be readable
     * over whatever the cart happens to be drawing, and the cart owns
     * every pixel underneath. NULL -- live play, and every frame after
     * the recording runs out -- draws nothing, which is how the overlay
     * ceases to exist once the replay is over. */
    if (replay_status) {
        DrawRectangle(0, 0, PICO8_SCREEN_WIDTH * PICO8_RAYLIB_BACKEND_SCALE, 26, (Color){ 0, 0, 0, 190 });
        DrawText(replay_status, 8, 5, 18, RAYWHITE);
    }
    EndDrawing();

    (void)frame;
}

void pico8_raylib_backend_record_frame(pico8_raylib_backend_t *backend, const pico8_input_frame_t *frame) {
    /* The return value is deliberately ignored: a failed append drops
     * the frame rather than crashing a live playtest session, and the
     * recorder already leaves its own count alone in that case, so the
     * next frame takes the slot this one never reached and poll_input's
     * marker file keeps naming the right frame (pico8_session_recorder.
     * h). There is nothing this backend could do about a drop that
     * would not be worse than the drop. Presentation/recording-layer
     * robustness, not a ported function's error-handling contract. */
    (void)pico8_session_recorder_record_frame(&backend->recorder, frame);
}

void pico8_raylib_backend_shutdown(pico8_raylib_backend_t *backend) {
    UnloadTexture(backend->screen_texture);
    ShowCursor();
    CloseWindow();
    pico8_session_recorder_close(&backend->recorder);
}
