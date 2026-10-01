#include "pico8_zone_interface.h"

#include <assert.h>
#include <stddef.h>

/* Au doigt mouille : la struct de la sorte et ce qu'elle possede. */
static const struct {
    const char *place_name;
    size_t size;
} SORTES[PICO8_SORTE_COUNT] = {
    [PICO8_SORTE_JS] = {"place js", 5u * 1024u},
    [PICO8_SORTE_FQ] = {"place fq", 256u},
    [PICO8_SORTE_MH] = {"place mh", 12u * 1024u},
    [PICO8_SORTE_IV] = {"place iv", 256u},
    [PICO8_SORTE_RANGEE] = {"place rangee", 768u},
    [PICO8_SORTE_LE] = {"place le", 512u},
};

static const pico8_zone_choisie_t *g_adossement;
static int g_vivantes[PICO8_SORTE_COUNT];

static pico8_sorte_t sorte_of(const pico8_zone_choisie_t *z);

static unsigned char *place_alloc(const pico8_zone_choisie_t *z, size_t n) {
    pico8_sorte_t sorte = sorte_of(z);
    assert(g_adossement != NULL && "zone interface sans adossement");
    assert(n <= SORTES[sorte].size && "zone interface : on n'y prend qu'une place entiere, pas plus");
    g_vivantes[sorte]++;
    return pico8_zone_alloc(g_adossement, SORTES[sorte].size);
}

static unsigned char *place_grow(const pico8_zone_choisie_t *z, unsigned char *p,
                                 size_t old_n, size_t new_n) {
    (void)z;
    (void)p;
    (void)old_n;
    (void)new_n;
    assert(0 && "zone interface : une place ne grandit pas");
    return NULL;
}

static void place_release(const pico8_zone_choisie_t *z, unsigned char *p) {
    if (p == NULL) return;
    g_vivantes[sorte_of(z)]--;
    pico8_zone_release(g_adossement, p);
}

static const pico8_zone_choisie_t g_handles[PICO8_SORTE_COUNT] = {
    [PICO8_SORTE_JS] = {place_alloc, place_grow, place_release, NULL},
    [PICO8_SORTE_FQ] = {place_alloc, place_grow, place_release, NULL},
    [PICO8_SORTE_MH] = {place_alloc, place_grow, place_release, NULL},
    [PICO8_SORTE_IV] = {place_alloc, place_grow, place_release, NULL},
    [PICO8_SORTE_RANGEE] = {place_alloc, place_grow, place_release, NULL},
    [PICO8_SORTE_LE] = {place_alloc, place_grow, place_release, NULL},
};

static pico8_sorte_t sorte_of(const pico8_zone_choisie_t *z) {
    assert(z >= g_handles && z < g_handles + PICO8_SORTE_COUNT && "poignee hors de la zone interface");
    return (pico8_sorte_t)(z - g_handles);
}

void pico8_zone_interface_adosser(const pico8_zone_choisie_t *adossement) {
    g_adossement = adossement;
}

const pico8_zone_choisie_t *pico8_zone_interface(pico8_sorte_t sorte) {
    assert(sorte >= 0 && sorte < PICO8_SORTE_COUNT);
    return &g_handles[sorte];
}

const pico8_zone_choisie_t *pico8_zone_interface_place(pico8_sorte_t sorte) {
    return pico8_zone_arene_prendre(pico8_zone_interface(sorte), SORTES[sorte].place_name, SORTES[sorte].size);
}

int pico8_zone_interface_vivantes(pico8_sorte_t sorte) {
    assert(sorte >= 0 && sorte < PICO8_SORTE_COUNT);
    return g_vivantes[sorte];
}
