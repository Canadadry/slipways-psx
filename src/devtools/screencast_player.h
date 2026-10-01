/* Les deux decisions du visualiseur de screencast qui ne demandent pas
 * de fenetre : quelle frame afficher maintenant, et ce qu'on dit d'un
 * fichier qu'on refuse de jouer. Rien ici n'inclut raylib.h, et rien ici
 * ne doit l'inclure : c'est ce qui rend ce module exercable par
 * `make unit`, que src/cmd/screencast_viewer.c n'est pas. */

#ifndef PICO8_SCREENCAST_PLAYER_H
#define PICO8_SCREENCAST_PLAYER_H

#include "devtools/screencast.h"

#include <stddef.h>

/* Commodite pour l'appelant, pas un contrat qu'il pourrait violer : la
 * fonction tronque plutot que de deborder ce qu'on lui donne. Le plus
 * long des trois messages de refus fait 188 caracteres hors chemin, donc
 * 256 tient un chemin jusqu'a 67. */
#define PICO8_SCREENCAST_PLAYER_REFUSAL_MAX 256

/* Ou en est la lecture. Un seul chiffre qui bouge, l'index de la frame
 * affichee ; `frame_count` est fixe a l'ouverture et ne change plus. */
typedef struct {
    int frame_count;
    int current;
} pico8_screencast_player_t;

/* Positionne la lecture sur la frame 0. `frame_count` est celui que
 * pico8_screencast_frame_count vient de rendre, donc au moins 1. */
void pico8_screencast_player_init(pico8_screencast_player_t *player, int frame_count);

/* L'index de la frame a afficher maintenant, entre 0 et frame_count-1,
 * a passer tel quel a pico8_screencast_read_frame. Pas tel quel a
 * pico8_replay_control_format_status, qui compte a partir de 1 :
 * l'appelant y ajoute 1. */
int pico8_screencast_player_current_frame(const pico8_screencast_player_t *player);

/* Passe a la frame suivante. Sur la derniere frame l'appel ne fait
 * rien, indefiniment : la lecture se fige sur la fin du fichier, elle ne
 * reboucle pas. Rien ici ne touche au fichier. */
void pico8_screencast_player_advance(pico8_screencast_player_t *player);

/* Ecrit dans `out` (toujours termine par un NUL, tronque plutot que
 * deborde) la phrase que le visualiseur imprime sur stderr avant de
 * s'arreter, pour un `status` que pico8_screencast_reader_open a rendu
 * et qui n'est pas PICO8_SCREENCAST_OK. */
void pico8_screencast_player_format_refusal(pico8_screencast_status_t status, const char *path, char *out, size_t out_size);

#endif
