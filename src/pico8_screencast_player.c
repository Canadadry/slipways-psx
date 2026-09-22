/* Voir pico8_screencast_player.h pour ce qu'est ce module, pourquoi il
 * vit hors du fichier raylib, et pourquoi la lecture reboucle. */

#include "pico8_screencast_player.h"

#include <assert.h>
#include <stdio.h>

void pico8_screencast_player_init(pico8_screencast_player_t *player, int frame_count) {
    assert(player);
    /* Zero frame n'est pas un cas a gerer ici : pico8_screencast_reader_
     * open refuse deja un fichier vide (PICO8_SCREENCAST_EMPTY) et le
     * visualiseur s'arrete avant d'ouvrir sa fenetre, donc arriver ici
     * avec un compte nul serait un appelant casse, pas un fichier
     * douteux -- et un curseur sur zero frame demanderait la frame 0
     * d'un fichier qui n'en a aucune. */
    assert(frame_count > 0);

    player->frame_count = frame_count;
    player->current = 0;
}

int pico8_screencast_player_current_frame(const pico8_screencast_player_t *player) {
    assert(player);

    return player->current;
}

void pico8_screencast_player_advance(pico8_screencast_player_t *player) {
    assert(player);

    player->current++;
    /* Le rebouclage, et le seul endroit du depot ou il est ecrit. Fait
     * ici plutot que par un modulo au moment de l'affichage : l'index
     * tenu reste toujours un index valide du fichier, donc aucun
     * appelant ne peut demander au chunk 1 une frame qui n'existe pas,
     * et la ligne d'etat ne peut pas annoncer une frame au-dela du
     * total. */
    if (player->current >= player->frame_count) {
        player->current = 0;
    }
}

void pico8_screencast_player_format_refusal(pico8_screencast_status_t status, const char *path, char *out, size_t out_size) {
    assert(path);
    assert(out);
    assert(out_size > 0);

    switch (status) {
    case PICO8_SCREENCAST_UNREADABLE:
        snprintf(out, out_size,
                 "screencast-viewer: '%s' est introuvable ou illisible -- aucune fenetre n'a ete ouverte. "
                 "Fabriquer les pixels : make screencast", path);
        break;
    case PICO8_SCREENCAST_EMPTY:
        snprintf(out, out_size,
                 "screencast-viewer: '%s' est vide -- aucune fenetre n'a ete ouverte. "
                 "Une capture interrompue avant sa premiere frame donne ce fichier ; la refaire : make screencast", path);
        break;
    case PICO8_SCREENCAST_TRUNCATED:
        snprintf(out, out_size,
                 "screencast-viewer: '%s' est tronque : sa taille n'est pas un multiple de %d octets, "
                 "donc il se termine au milieu d'une frame -- aucune fenetre n'a ete ouverte. "
                 "La refaire : make screencast", path, PICO8_SCREENCAST_FRAME_BYTES);
        break;
    case PICO8_SCREENCAST_OK:
    default:
        assert(0 && "pico8_screencast_player_format_refusal: rien a refuser");
        snprintf(out, out_size, "screencast-viewer: '%s' : etat inattendu", path);
        break;
    }
}
