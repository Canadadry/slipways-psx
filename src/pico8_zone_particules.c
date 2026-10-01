#include "pico8_zone_particules.h"

#include <assert.h>
#include <stdio.h>

#include "pico8_fatal.h"

/* Taille : en-tete d'arene compris (x86-64), une liste tient juste sa
 * capacite reservee. Nombre : ce qui vit a la fois au pire de la frame. */
#define QY_SIZE (96u + 24u + PICO8_QY_CAPACITE * 8u)
#define QY_FV_SIZE (96u + 24u + PICO8_QY_FV_CAPACITE * 8u)
#define GA_SIZE 640u
#define PARTICULE_COUNT 2048
#define QY_COUNT 24
#define QY_FV_COUNT 1
#define GA_COUNT 8
#define ARENES_MAX QY_COUNT

typedef char pico8_zone_particules_arenes_max[(QY_FV_COUNT <= ARENES_MAX && GA_COUNT <= ARENES_MAX) ? 1 : -1];

static const struct {
    const char *zone_name;
    const char *place_name;
    size_t size;
    int count;
} SORTES[PICO8_PARTICULES_COUNT] = {
    [PICO8_PARTICULES_PARTICULE] = {"particules particule", NULL, PICO8_PARTICULE_PLACE, PARTICULE_COUNT},
    [PICO8_PARTICULES_QY] = {"particules qy", "place qy", QY_SIZE, QY_COUNT},
    [PICO8_PARTICULES_QY_FV] = {"particules qy_fv", "place qy_fv", QY_FV_SIZE, QY_FV_COUNT},
    [PICO8_PARTICULES_GA] = {"particules ga", "place ga", GA_SIZE, GA_COUNT},
};

static const pico8_zone_choisie_t *g_adossement;
static int g_vivantes[PICO8_PARTICULES_COUNT];
static int g_max_vivantes[PICO8_PARTICULES_COUNT];
static size_t g_max_octets[PICO8_PARTICULES_COUNT];
static int g_max_elements[PICO8_PARTICULES_COUNT];
/* Places-arenes vivantes, mesurees a la lecture du maximum d'octets. */
static unsigned char *g_arenes[PICO8_PARTICULES_COUNT][ARENES_MAX];

static pico8_particules_sorte_t sorte_of(const pico8_zone_choisie_t *z);

static int est_arene(pico8_particules_sorte_t sorte) {
    return SORTES[sorte].place_name != NULL;
}

static void relever_octets(pico8_particules_sorte_t sorte, const unsigned char *arene) {
    size_t prise = pico8_zone_arene_prise((const pico8_zone_choisie_t *)arene);
    if (prise > g_max_octets[sorte]) g_max_octets[sorte] = prise;
}

static unsigned char *place_alloc(const pico8_zone_choisie_t *z, size_t n) {
    pico8_particules_sorte_t sorte = sorte_of(z);
    assert(g_adossement != NULL && "zone particules sans adossement");
    assert(n <= SORTES[sorte].size && "zone particules : on n'y prend qu'une place entiere, pas plus");
    if (g_vivantes[sorte] == SORTES[sorte].count) {
        char message[96];
        snprintf(message, sizeof message, "zone %s pleine, limite %d places", SORTES[sorte].zone_name,
                 SORTES[sorte].count);
        pico8_fatal(message);
    }
    unsigned char *p = pico8_zone_alloc(g_adossement, SORTES[sorte].size);
    if (est_arene(sorte)) {
        int i = 0;
        while (g_arenes[sorte][i] != NULL) i++;
        g_arenes[sorte][i] = p;
    }
    g_vivantes[sorte]++;
    if (g_vivantes[sorte] > g_max_vivantes[sorte]) g_max_vivantes[sorte] = g_vivantes[sorte];
    return p;
}

static unsigned char *place_grow(const pico8_zone_choisie_t *z, unsigned char *p,
                                 size_t old_n, size_t new_n) {
    (void)z;
    (void)p;
    (void)old_n;
    (void)new_n;
    assert(0 && "zone particules : une place ne grandit pas");
    return NULL;
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
    pico8_zone_release(g_adossement, p);
}

static const pico8_zone_choisie_t g_handles[PICO8_PARTICULES_COUNT] = {
    [PICO8_PARTICULES_PARTICULE] = {place_alloc, place_grow, place_release, NULL, NULL},
    [PICO8_PARTICULES_QY] = {place_alloc, place_grow, place_release, NULL, NULL},
    [PICO8_PARTICULES_QY_FV] = {place_alloc, place_grow, place_release, NULL, NULL},
    [PICO8_PARTICULES_GA] = {place_alloc, place_grow, place_release, NULL, NULL},
};

static pico8_particules_sorte_t sorte_of(const pico8_zone_choisie_t *z) {
    assert(z >= g_handles && z < g_handles + PICO8_PARTICULES_COUNT && "poignee hors de la zone particules");
    return (pico8_particules_sorte_t)(z - g_handles);
}

void pico8_zone_particules_adosser(const pico8_zone_choisie_t *adossement) {
    g_adossement = adossement;
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
    assert((sorte == PICO8_PARTICULES_QY || sorte == PICO8_PARTICULES_QY_FV) && "seule une liste a des elements");
    if (count > g_max_elements[sorte]) g_max_elements[sorte] = count;
}

int pico8_zone_particules_max_elements(pico8_particules_sorte_t sorte) {
    assert(sorte >= 0 && sorte < PICO8_PARTICULES_COUNT);
    return g_max_elements[sorte];
}
