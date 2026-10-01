#include "pico8_zone_partie.h"

#include "pico8_zone_sizes.h"

PICO8_BASCULE_ZONE_STORAGE(partie_cells, PICO8_ZONE_GENERATION_BYTES);
PICO8_BUMP_ZONE_STORAGE(chargement_cells, PICO8_ZONE_CHARGEMENT_BYTES);

static pico8_bascule_zone_t partie;
/* Statique : une liste ents/ptrs garde un pointeur vers sa zone. */
static pico8_zone_choisie_bascule_t partie_handle;
static const pico8_zone_choisie_t *partie_choisie;
static pico8_bump_zone_t chargement;
static pico8_zone_choisie_t chargement_handle;
static size_t chargement_max;

/* Paresseuse : pico8_bascule_zone_init empoisonne le stockage sous ASan, ce
 * qu'un initialiseur statique ne ferait pas. */
static void partie_init_once(void) {
    if (partie_choisie != NULL) return;
    pico8_bascule_zone_init(&partie, "partie", partie_cells, PICO8_ZONE_GENERATION_BYTES);
    partie_choisie = pico8_zone_choisie_bascule(&partie_handle, &partie);
    pico8_bump_zone_init(&chargement, "chargement", chargement_cells, PICO8_ZONE_CHARGEMENT_BYTES);
    chargement_handle = pico8_zone_choisie_bump(&chargement);
}

/* La zone montante ne garde pas son pic : `used` ne fait que monter entre
 * deux resets, donc le relire avant chaque reset et a chaque lecture du
 * maximum suffit. */
static void chargement_noter_max(void) {
    if (chargement.used > chargement_max) chargement_max = chargement.used;
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
    chargement_noter_max();
    pico8_bump_zone_reset(&chargement);
}

size_t pico8_zone_partie_prise(void) {
    partie_init_once();
    return pico8_bascule_zone_prise(&partie);
}

size_t pico8_zone_partie_max(void) {
    partie_init_once();
    return pico8_bascule_zone_max(&partie);
}

const pico8_zone_choisie_t *pico8_zone_chargement(void) {
    partie_init_once();
    return &chargement_handle;
}

size_t pico8_zone_chargement_prise(void) {
    partie_init_once();
    return chargement.used;
}

size_t pico8_zone_chargement_max(void) {
    partie_init_once();
    chargement_noter_max();
    return chargement_max;
}
