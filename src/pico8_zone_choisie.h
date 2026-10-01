#ifndef PICO8_ZONE_CHOISIE_H
#define PICO8_ZONE_CHOISIE_H

#include <stddef.h>

#include "pico8_zone.h"

/* Zone designee : la poignee qu'un proprietaire ou un appelant passe aux
 * fonctions partagees qui allouent pour lui. Un bloc n'est agrandi ou rendu
 * que par la poignee qui l'a alloue. */
typedef struct pico8_zone_choisie {
    unsigned char *(*alloc)(const struct pico8_zone_choisie *z, size_t n);
    unsigned char *(*grow)(const struct pico8_zone_choisie *z, unsigned char *p,
                           size_t old_n, size_t new_n);
    void (*release)(const struct pico8_zone_choisie *z, unsigned char *p);
    pico8_bump_zone_t *bump; /* NULL hors zone a pointeur montant */
    pico8_slot_zone_t *slot; /* NULL hors zone a places */
} pico8_zone_choisie_t;

/* Poignee sur une zone deja initialisee, qui doit lui survivre. Rendre n'y
 * fait rien : la memoire repart au reset de la zone. */
pico8_zone_choisie_t pico8_zone_choisie_bump(pico8_bump_zone_t *bump);

/* Poignee sur une zone a places deja initialisee, qui doit lui survivre :
 * chaque bloc est une place entiere. alloc(n) exige n <= taille de place,
 * grow est refuse, rendre rend la place. */
pico8_zone_choisie_t pico8_zone_choisie_slot(pico8_slot_zone_t *slot);

/* Memoire non initialisee ; pico8_fatal si la zone est pleine. */
unsigned char *pico8_zone_alloc(const pico8_zone_choisie_t *z, size_t n);
/* Rend un bloc de new_n octets qui commence par les old_n octets de p ; p,
 * peut-etre NULL avec old_n nul, ne doit plus servir. */
unsigned char *pico8_zone_grow(const pico8_zone_choisie_t *z, unsigned char *p,
                               size_t old_n, size_t new_n);
/* p peut etre NULL. */
void pico8_zone_release(const pico8_zone_choisie_t *z, unsigned char *p);

/* Arene : un bloc de `size` octets pris a `from`, qui range dans sa tete
 * une zone montante privee et la poignee rendue, laquelle vit autant que le
 * bloc. Ce qu'on y alloue vit jusqu'a pico8_zone_arene_rendre ; y rendre un
 * bloc ne fait rien, y agrandir copie et laisse l'ancien bloc mort. `name`
 * doit vivre autant que l'arene : il compose « zone <name> pleine ». */
const pico8_zone_choisie_t *pico8_zone_arene_prendre(const pico8_zone_choisie_t *from,
                                                     const char *name, size_t size);
/* Octets pris dans le bloc, tete comprise. */
size_t pico8_zone_arene_prise(const pico8_zone_choisie_t *arene);
/* Rend le bloc entier a la poignee qui l'a donne. */
void pico8_zone_arene_rendre(const pico8_zone_choisie_t *arene);

#endif
