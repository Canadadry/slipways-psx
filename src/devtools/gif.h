/* Une capture en GIF89a animé, pour qu'un humain relise un e2e sans raylib :
 * palette PICO-8 globale, échelle 3, 3 cs par frame, boucle infinie
 * (NETSCAPE2.0), une image complète par frame. */

#ifndef PICO8_GIF_H
#define PICO8_GIF_H

#include "p8/input.h"

#include <stdio.h>

#define PICO8_GIF_SCALE 3

typedef struct {
    FILE *file;
    int ok;
} pico8_gif_writer_t;

/* Écrit l'en-tête dans `file`, que l'appelant ouvre en binaire et referme
 * lui-même. Rend 1 si tout est écrit. */
int pico8_gif_open(pico8_gif_writer_t *writer, FILE *file);

/* Ajoute une frame de 128x128 index (un octet par pixel, 0 à 15, dans
 * l'ordre de pico8_screencast_read_frame). Avec `event` non NULL, la souris
 * de cet event est dessinée par-dessus : anneau de rayon 2 en couleur 7 sans
 * bouton, de rayon 3 en couleur 8 avec un bouton. `pixels` n'est pas modifié. */
int pico8_gif_write_frame(pico8_gif_writer_t *writer, const unsigned char *pixels, const p8_input_frame_t *event);

/* Écrit le trailer ; rend 1 si aucune écriture n'a échoué depuis l'ouverture.
 * Ne ferme pas le fichier. */
int pico8_gif_close(pico8_gif_writer_t *writer);

#endif
