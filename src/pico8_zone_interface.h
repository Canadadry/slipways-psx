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

/* Poignee qui donne et reprend une place entiere de la sorte : alloc(n)
 * exige n <= taille de place, grow est refuse. */
const pico8_zone_choisie_t *pico8_zone_interface(pico8_sorte_t sorte);

/* Une place de la sorte, en arene nommee « place <sorte> » ; on la rend
 * par pico8_zone_arene_rendre. Zone pleine : pico8_fatal « zone interface
 * <sorte> pleine ». */
const pico8_zone_choisie_t *pico8_zone_interface_place(pico8_sorte_t sorte);

/* Places prises et pas encore rendues, et leur maximum sur la session. */
int pico8_zone_interface_vivantes(pico8_sorte_t sorte);
int pico8_zone_interface_max_vivantes(pico8_sorte_t sorte);
/* Places de la sorte, et octets d'une place. */
int pico8_zone_interface_capacite(pico8_sorte_t sorte);
size_t pico8_zone_interface_taille(pico8_sorte_t sorte);

#endif
