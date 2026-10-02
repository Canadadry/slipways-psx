#include "devtools/screencast_player.h"

#include <assert.h>
#include <stdio.h>

void screencast_player_init(screencast_player_t *player, int frame_count) {
    assert(player);
    /* pico8_screencast_reader_open rend deja EMPTY sur un fichier de
     * taille nulle, donc un compte nul ici est un appelant casse, pas un
     * fichier douteux. */
    assert(frame_count > 0);

    player->frame_count = frame_count;
    player->current = 0;
}

int screencast_player_current_frame(const screencast_player_t *player) {
    assert(player);

    return player->current;
}

void screencast_player_advance(screencast_player_t *player) {
    assert(player);

    /* Increment garde, et non increment suivi d'un recadrage : `current`
     * ne prend jamais, meme le temps d'une instruction, une valeur hors
     * du fichier. */
    if (player->current + 1 < player->frame_count) {
        player->current++;
    }
}

void screencast_player_format_refusal(screencast_status_t status, const char *path, char *out, size_t out_size) {
    assert(path);
    assert(out);
    assert(out_size > 0);

    switch (status) {
    case SCREENCAST_UNREADABLE:
        snprintf(out, out_size,
                 "screencast-viewer: '%s' is missing or unreadable -- no window was opened. "
                 "Make the pixels: make screencast EVENTS=<events.json>", path);
        break;
    case SCREENCAST_EMPTY:
        snprintf(out, out_size,
                 "screencast-viewer: '%s' is empty -- no window was opened. "
                 "A capture interrupted before its first frame leaves this file; redo it: make screencast EVENTS=<events.json>", path);
        break;
    case SCREENCAST_TRUNCATED:
        snprintf(out, out_size,
                 "screencast-viewer: '%s' is truncated: its size is not a multiple of %d bytes, "
                 "so it ends in the middle of a frame -- no window was opened. "
                 "Redo it: make screencast EVENTS=<events.json>", path, SCREENCAST_FRAME_BYTES);
        break;
    case SCREENCAST_OK:
    default:
        assert(0 && "screencast_player_format_refusal: nothing to refuse");
        snprintf(out, out_size, "screencast-viewer: '%s': unexpected status", path);
        break;
    }
}
