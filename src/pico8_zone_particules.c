#include "pico8_zone_particules.h"

#include <assert.h>

#include "pico8_zone_sizes.h"

/* Taille : en-tete d'arene compris (x86-64), une liste tient juste sa
 * capacite reservee. Nombre : ce qui vit a la fois au pire de la frame. */
#define QY_SIZE (96u + 24u + PICO8_QY_CAPACITE * 8u)
#define QY_FV_SIZE (96u + 24u + PICO8_QY_FV_CAPACITE * 8u)
#define GA_SIZE 640u
#define QY_COUNT 24
#define QY_FV_COUNT 1
#define GA_COUNT 8
#define ARENES_MAX QY_COUNT
/* Risque accepte : ~21 trainees vivent par slipway desservi dans les deux
 * sens, 680 places tiennent une trentaine de tels slipways visibles. */
#define TS_COUNT 680

/* Particules : chaque liste `qy` a son pire, plus les 5 de la nouvelle
 * aide, nees au `_init` avant le vidage qui rend l'ancienne partie. Un
 * eclat en fait 140 (`ez(self,140)`) ; une sonde ou un `pm` (`fc=1`,
 * `ki=9`) ~58, 60 avec les sauts de TROU-14. */
#define GA_ECLAT 140
#define EMETTEUR_PIRE 60
#define FV_PAR_FRAME 5
#define PARTICULE_COUNT (GA_COUNT * GA_ECLAT + (QY_COUNT - GA_COUNT) * EMETTEUR_PIRE + FV_PAR_FRAME)

typedef char pico8_zone_particules_arenes_max[(QY_FV_COUNT <= ARENES_MAX && GA_COUNT <= ARENES_MAX) ? 1 : -1];
typedef char pico8_zone_particules_tient_l_aide[PARTICULE_COUNT >= PICO8_QY_FV_CAPACITE ? 1 : -1];

typedef char pico8_zone_particules_exceeded[
    (PICO8_SLOT_ZONE_BYTES(PICO8_PARTICULE_PLACE, PARTICULE_COUNT) + PICO8_SLOT_ZONE_BYTES(QY_SIZE, QY_COUNT) +
     PICO8_SLOT_ZONE_BYTES(QY_FV_SIZE, QY_FV_COUNT) + PICO8_SLOT_ZONE_BYTES(GA_SIZE, GA_COUNT) +
     PICO8_SLOT_ZONE_BYTES(PICO8_TS_PLACE, TS_COUNT)
     <= PICO8_ZONE_PARTICLES_BYTES) ? 1 : -1];

PICO8_SLOT_ZONE_STORAGE(particule_cells, particule_taken, PICO8_PARTICULE_PLACE, PARTICULE_COUNT);
PICO8_SLOT_ZONE_STORAGE(qy_cells, qy_taken, QY_SIZE, QY_COUNT);
PICO8_SLOT_ZONE_STORAGE(qy_fv_cells, qy_fv_taken, QY_FV_SIZE, QY_FV_COUNT);
PICO8_SLOT_ZONE_STORAGE(ga_cells, ga_taken, GA_SIZE, GA_COUNT);
PICO8_SLOT_ZONE_STORAGE(ts_cells, ts_taken, PICO8_TS_PLACE, TS_COUNT);

static const pico8_zone_sorte_t SORTES[PICO8_PARTICULES_COUNT] = {
    [PICO8_PARTICULES_PARTICULE] = {"particules particule", NULL, particule_cells, particule_taken,
                                    PICO8_PARTICULE_PLACE, PARTICULE_COUNT},
    [PICO8_PARTICULES_QY] = {"particules qy", "place qy", qy_cells, qy_taken, QY_SIZE, QY_COUNT},
    [PICO8_PARTICULES_QY_FV] = {"particules qy_fv", "place qy_fv", qy_fv_cells, qy_fv_taken, QY_FV_SIZE,
                                QY_FV_COUNT},
    [PICO8_PARTICULES_GA] = {"particules ga", "place ga", ga_cells, ga_taken, GA_SIZE, GA_COUNT},
    [PICO8_PARTICULES_TS] = {"particules ts", NULL, ts_cells, ts_taken, PICO8_TS_PLACE, TS_COUNT},
};

static pico8_slot_zone_t g_zones[PICO8_PARTICULES_COUNT];
static pico8_zone_choisie_t g_slots[PICO8_PARTICULES_COUNT];
static int g_ready;
static int g_vivantes[PICO8_PARTICULES_COUNT];
static int g_max_vivantes[PICO8_PARTICULES_COUNT];
static size_t g_max_octets[PICO8_PARTICULES_COUNT];
static int g_max_elements[PICO8_PARTICULES_COUNT];
/* Places-arenes vivantes, mesurees a la lecture du maximum d'octets. */
static unsigned char *g_arenes[PICO8_PARTICULES_COUNT][ARENES_MAX];

static pico8_particules_sorte_t sorte_of(const pico8_zone_choisie_t *z);

/* Paresseuse : pico8_slot_zone_init empoisonne le stockage sous ASan, ce
 * qu'un initialiseur statique ne ferait pas. */
static void init_once(void) {
    if (g_ready) return;
    pico8_zone_sortes_init(SORTES, PICO8_PARTICULES_COUNT, g_zones, g_slots);
    g_ready = 1;
}

static int est_arene(pico8_particules_sorte_t sorte) {
    return SORTES[sorte].place_name != NULL;
}

static void relever_octets(pico8_particules_sorte_t sorte, const unsigned char *arene) {
    size_t prise = pico8_zone_arene_prise((const pico8_zone_choisie_t *)arene);
    if (prise > g_max_octets[sorte]) g_max_octets[sorte] = prise;
}

static unsigned char *place_alloc(const pico8_zone_choisie_t *z, size_t n) {
    pico8_particules_sorte_t sorte = sorte_of(z);
    init_once();
    unsigned char *p = pico8_zone_alloc(&g_slots[sorte], n);
    if (est_arene(sorte)) {
        int i = 0;
        while (g_arenes[sorte][i] != NULL) i++;
        g_arenes[sorte][i] = p;
    }
    g_vivantes[sorte]++;
    if (g_vivantes[sorte] > g_max_vivantes[sorte]) g_max_vivantes[sorte] = g_vivantes[sorte];
    return p;
}

static void place_release(const pico8_zone_choisie_t *z, unsigned char *p) {
    pico8_particules_sorte_t sorte = sorte_of(z);
    if (p == NULL) return;
    if (est_arene(sorte)) {
        int i = 0;
        while (g_arenes[sorte][i] != p) i++;
        relever_octets(sorte, p);
        g_arenes[sorte][i] = NULL;
    }
    g_vivantes[sorte]--;
    pico8_zone_release(&g_slots[sorte], p);
}

static const pico8_zone_choisie_t g_handles[PICO8_PARTICULES_COUNT] = {
    [PICO8_PARTICULES_PARTICULE] = {place_alloc, pico8_zone_grow_refusee, place_release, NULL, NULL},
    [PICO8_PARTICULES_QY] = {place_alloc, pico8_zone_grow_refusee, place_release, NULL, NULL},
    [PICO8_PARTICULES_QY_FV] = {place_alloc, pico8_zone_grow_refusee, place_release, NULL, NULL},
    [PICO8_PARTICULES_GA] = {place_alloc, pico8_zone_grow_refusee, place_release, NULL, NULL},
    [PICO8_PARTICULES_TS] = {place_alloc, pico8_zone_grow_refusee, place_release, NULL, NULL},
};

static pico8_particules_sorte_t sorte_of(const pico8_zone_choisie_t *z) {
    assert(z >= g_handles && z < g_handles + PICO8_PARTICULES_COUNT && "poignee hors de la zone particules");
    return (pico8_particules_sorte_t)(z - g_handles);
}

const pico8_zone_choisie_t *pico8_zone_particules(pico8_particules_sorte_t sorte) {
    assert(sorte >= 0 && sorte < PICO8_PARTICULES_COUNT);
    return &g_handles[sorte];
}

const pico8_zone_choisie_t *pico8_zone_particules_place(pico8_particules_sorte_t sorte) {
    assert(sorte >= 0 && sorte < PICO8_PARTICULES_COUNT && est_arene(sorte) &&
           "une particule est une place nue, pas une arene");
    return pico8_zone_arene_prendre(&g_handles[sorte], SORTES[sorte].place_name, SORTES[sorte].size);
}

int pico8_zone_particules_vivantes(pico8_particules_sorte_t sorte) {
    assert(sorte >= 0 && sorte < PICO8_PARTICULES_COUNT);
    init_once();
    assert((size_t)g_vivantes[sorte] == pico8_slot_zone_taken(&g_zones[sorte]) &&
           "places vivantes : une place prise ou rendue sans passer par la poignee de la sorte");
    return g_vivantes[sorte];
}

int pico8_zone_particules_max_vivantes(pico8_particules_sorte_t sorte) {
    assert(sorte >= 0 && sorte < PICO8_PARTICULES_COUNT);
    return g_max_vivantes[sorte];
}

size_t pico8_zone_particules_max_octets(pico8_particules_sorte_t sorte) {
    assert(sorte >= 0 && sorte < PICO8_PARTICULES_COUNT);
    if (est_arene(sorte)) {
        for (int i = 0; i < ARENES_MAX; i++)
            if (g_arenes[sorte][i] != NULL) relever_octets(sorte, g_arenes[sorte][i]);
    }
    return g_max_octets[sorte];
}

void pico8_zone_particules_noter_elements(pico8_particules_sorte_t sorte, int count) {
    assert((sorte == PICO8_PARTICULES_QY || sorte == PICO8_PARTICULES_QY_FV || sorte == PICO8_PARTICULES_TS) &&
           "seule une liste a des elements");
    if (count > g_max_elements[sorte]) g_max_elements[sorte] = count;
}

int pico8_zone_particules_max_elements(pico8_particules_sorte_t sorte) {
    assert(sorte >= 0 && sorte < PICO8_PARTICULES_COUNT);
    return g_max_elements[sorte];
}

int pico8_zone_particules_capacity(pico8_particules_sorte_t sorte) {
    assert(sorte >= 0 && sorte < PICO8_PARTICULES_COUNT);
    return (int)SORTES[sorte].count;
}

size_t pico8_zone_particules_slot_size(pico8_particules_sorte_t sorte) {
    assert(sorte >= 0 && sorte < PICO8_PARTICULES_COUNT);
    return SORTES[sorte].size;
}
