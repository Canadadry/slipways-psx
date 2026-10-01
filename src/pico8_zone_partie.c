#include "pico8_zone_partie.h"

#include "pico8_zone_sizes.h"

PICO8_BASCULE_ZONE_STORAGE(partie_cells, PICO8_ZONE_GAME_BYTES);

static pico8_bascule_zone_t partie;
/* Statique : une liste ents/ptrs garde un pointeur vers sa zone. */
static pico8_zone_choisie_bascule_t partie_handle;
static const pico8_zone_choisie_t *partie_choisie;

/* Paresseuse : pico8_bascule_zone_init empoisonne le stockage sous ASan, ce
 * qu'un initialiseur statique ne ferait pas. */
static void partie_init_once(void) {
    if (partie_choisie != NULL) return;
    pico8_bascule_zone_init(&partie, "partie", partie_cells, PICO8_ZONE_GAME_BYTES);
    partie_choisie = pico8_zone_choisie_bascule(&partie_handle, &partie);
}

const pico8_zone_choisie_t *pico8_zone_partie(void) {
    partie_init_once();
    return partie_choisie;
}

void pico8_zone_partie_basculer(void) {
    partie_init_once();
    pico8_bascule_zone_basculer(&partie);
}

int pico8_zone_partie_en_attente(void) {
    partie_init_once();
    return partie.en_attente;
}

void pico8_zone_partie_rendre(void) {
    partie_init_once();
    pico8_bascule_zone_rendre(&partie);
}

void pico8_zone_partie_reset(void) {
    partie_init_once();
    pico8_bascule_zone_reset(&partie);
}

size_t pico8_zone_partie_prise(void) {
    partie_init_once();
    return pico8_bascule_zone_prise(&partie);
}

size_t pico8_zone_partie_max(void) {
    partie_init_once();
    return pico8_bascule_zone_max(&partie);
}
