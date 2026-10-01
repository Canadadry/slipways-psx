#ifndef PICO8_ZONE_INTERFACE_H
#define PICO8_ZONE_INTERFACE_H

#include "pico8_zone_choisie.h"

/* Une sorte prend une place a elle par objet ; `rangee` est un `lq`
 * reaffecte dans un objet qui lui survit, `le` une liste `ok.le`. */
typedef enum {
    PICO8_SORTE_JS,
    PICO8_SORTE_FQ,
    PICO8_SORTE_MH,
    PICO8_SORTE_IV,
    PICO8_SORTE_RANGEE,
    PICO8_SORTE_LE,
    PICO8_SORTE_COUNT
} pico8_sorte_t;

/* Les places sont prises a `adossement`, qui doit leur survivre. Avant le
 * premier appel, prendre une place s'arrete sur un assert. */
void pico8_zone_interface_adosser(const pico8_zone_choisie_t *adossement);

/* Poignee qui donne et reprend une place entiere de la sorte : alloc(n)
 * exige n <= taille de place, grow est refuse. */
const pico8_zone_choisie_t *pico8_zone_interface(pico8_sorte_t sorte);

/* Une place de la sorte, en arene nommee « place <sorte> » ; on la rend
 * par pico8_zone_arene_rendre. */
const pico8_zone_choisie_t *pico8_zone_interface_place(pico8_sorte_t sorte);

/* Places prises et pas encore rendues. */
int pico8_zone_interface_vivantes(pico8_sorte_t sorte);

#endif
