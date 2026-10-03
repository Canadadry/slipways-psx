/* The screencast viewer: a raylib window that plays the pixel file made
 * by `make screencast`, so that a human can watch a session.
 *
 * It opens nothing for writing, anywhere: a round trip through the viewer
 * that changed a byte would make the hash of `make e2e` diverge. That is
 * why it calls neither raylib_backend_init() nor raylib_backend_present():
 * the first opens a session recording, the second reads the memory of a
 * running game. From the backend it takes only RAYLIB_BACKEND_SCALE and
 * the two functions that receive no backend, and their signatures are the
 * guarantee.
 *
 * Every decision lives elsewhere, with its tests: the format and the
 * indexed read in devtools/screencast, the playback cursor and the refusal
 * sentence in devtools/screencast_player, the speed and the status line in
 * devtools/replay_control, the sixteen colors in p8/palette. What is left
 * here needs a window, so `make unit` does not exercise it. */

#include "devtools/screencast.h"
#include "devtools/screencast_player.h"
#include "p8/palette.h"
#include "devtools/replay_control.h"
#include "platform/raylib_backend.h"
#include "p8/gfx.h"

#include "raylib.h"

#include <stdio.h>

/* The pixels of the current frame (one byte per pixel) and their colors.
 * Static because 16 KiB plus 64 KiB have no business in a stack frame. */
static unsigned char g_frame_pixels[SCREENCAST_FRAME_PIXELS];
static Color g_frame_colors[SCREENCAST_FRAME_PIXELS];

int main(int argc, char **argv) {
    /* No default path, on purpose: it would open a window on a capture
     * that is not the one the human believes they are watching. */
    if (argc != 2) {
        fprintf(stderr,
                "screencast-viewer: usage: %s <pixel file>\n"
                "screencast-viewer: to make one: make screencast EVENTS=<events.json> (writes tmp/screencast.bin)\n",
                argv[0]);
        return 1;
    }
    const char *path = argv[1];

    /* The refusal comes before any raylib call: a window that opens on
     * black cannot be told apart from a session that starts on a black
     * screen. */
    screencast_reader_t reader;
    screencast_status_t status = screencast_reader_open(&reader, path);
    if (status != SCREENCAST_OK) {
        char refusal[SCREENCAST_PLAYER_REFUSAL_MAX];
        screencast_player_format_refusal(status, path, refusal, sizeof refusal);
        fprintf(stderr, "%s\n", refusal);
        screencast_reader_close(&reader);
        return 1;
    }

    int frame_count = screencast_frame_count(&reader);

    screencast_player_t player;
    screencast_player_init(&player, frame_count);

    replay_control_t control;
    replay_control_init(&control);

    fprintf(stderr,
            "screencast-viewer: '%s', %d frames, played once, then held on the last frame.\n"
            "screencast-viewer: PAGE UP faster, PAGE DOWN slower, END uncapped; close the window to quit.\n",
            path, frame_count);

    InitWindow(P8_SCREEN_WIDTH * RAYLIB_BACKEND_SCALE, P8_SCREEN_HEIGHT * RAYLIB_BACKEND_SCALE,
               "Slipways screencast viewer");

    Image blank = GenImageColor(P8_SCREEN_WIDTH, P8_SCREEN_HEIGHT, BLACK);
    Texture2D screen_texture = LoadTextureFromImage(blank);
    UnloadImage(blank);
    /* Nearest neighbour: smoothing would blur exactly what a human comes
     * to check, the pixel. */
    SetTextureFilter(screen_texture, TEXTURE_FILTER_POINT);

    int exit_code = 0;
    while (!WindowShouldClose()) {
        raylib_backend_poll_replay_controls(&control);
        /* The replay_active argument is always 1, even once playback is
         * held on the last frame: there is no live game here to hand the
         * game's 60 FPS back to, and 0 would silently cancel the speed
         * scale. */
        raylib_backend_set_frame_rate_cap(replay_control_target_fps(&control, 1));

        int index = screencast_player_current_frame(&player);
        if (!screencast_read_frame(&reader, index, g_frame_pixels)) {
            /* The index comes from the cursor and never leaves the
             * file: getting here means the read itself failed. Exit in
             * error rather than keep showing the previous frame, which
             * would pass a failure off as a held image. */
            fprintf(stderr, "screencast-viewer: cannot read frame %d of '%s' -- stopping.\n", index, path);
            exit_code = 1;
            break;
        }

        for (int i = 0; i < SCREENCAST_FRAME_PIXELS; i++) {
            p8_rgb_t rgb = P8_PALETTE_RGB[g_frame_pixels[i] & 0x0f];
            g_frame_colors[i] = (Color){ rgb.red, rgb.green, rgb.blue, 0xFF };
        }
        UpdateTexture(screen_texture, g_frame_colors);

        /* replay_control_format_status counts frames from 1, the cursor
         * index from 0: hence the + 1. */
        char status_line[REPLAY_CONTROL_STATUS_MAX];
        replay_control_format_status(&control, index + 1, frame_count, status_line, sizeof status_line);

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTextureEx(screen_texture, (Vector2){ 0, 0 }, 0.0f, (float)RAYLIB_BACKEND_SCALE, WHITE);
        /* Translucent band: the line must stay readable whatever image
         * moves underneath. */
        DrawRectangle(0, 0, P8_SCREEN_WIDTH * RAYLIB_BACKEND_SCALE, 26, (Color){ 0, 0, 0, 190 });
        DrawText(status_line, 8, 5, 18, RAYWHITE);
        EndDrawing();

        /* Advances AFTER the display: the first frame of the file is the
         * first one shown. No end condition here -- the cursor holds on the
         * last frame by itself. */
        screencast_player_advance(&player);
    }

    UnloadTexture(screen_texture);
    CloseWindow();
    screencast_reader_close(&reader);

    return exit_code;
}
