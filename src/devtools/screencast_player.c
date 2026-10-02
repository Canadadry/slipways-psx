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
                 "screencast-viewer: '%s' est introuvable ou illisible -- aucune fenetre n'a ete ouverte. "
                 "Fabriquer les pixels : make screencast EVENTS=<events.json>", path);
        break;
    case SCREENCAST_EMPTY:
        snprintf(out, out_size,
                 "screencast-viewer: '%s' est vide -- aucune fenetre n'a ete ouverte. "
                 "Une capture interrompue avant sa premiere frame donne ce fichier ; la refaire : make screencast EVENTS=<events.json>", path);
        break;
    case SCREENCAST_TRUNCATED:
        snprintf(out, out_size,
                 "screencast-viewer: '%s' est tronque : sa taille n'est pas un multiple de %d octets, "
                 "donc il se termine au milieu d'une frame -- aucune fenetre n'a ete ouverte. "
                 "La refaire : make screencast EVENTS=<events.json>", path, SCREENCAST_FRAME_BYTES);
        break;
    case SCREENCAST_OK:
    default:
        assert(0 && "pico8_screencast_player_format_refusal: rien a refuser");
        snprintf(out, out_size, "screencast-viewer: '%s' : etat inattendu", path);
        break;
    }
}
