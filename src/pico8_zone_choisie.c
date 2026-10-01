#include "pico8_zone_choisie.h"

#include <assert.h>
#include <string.h>

static unsigned char *bump_alloc(const pico8_zone_choisie_t *z, size_t n) {
    return pico8_bump_zone_alloc(z->bump, n);
}

/* Copie, pas de realloc : le bloc vit dans la zone, pas dans le tas. */
static unsigned char *bump_grow(const pico8_zone_choisie_t *z, unsigned char *p,
                                size_t old_n, size_t new_n) {
    unsigned char *q = pico8_bump_zone_alloc(z->bump, new_n);
    if (old_n > 0) memcpy(q, p, old_n);
    return q;
}

static void bump_release(const pico8_zone_choisie_t *z, unsigned char *p) {
    (void)z;
    (void)p;
}

pico8_zone_choisie_t pico8_zone_choisie_bump(pico8_bump_zone_t *bump) {
    assert(bump != NULL && "poignee sur une zone absente");
    pico8_zone_choisie_t z = {bump_alloc, bump_grow, bump_release, bump};
    return z;
}

unsigned char *pico8_zone_alloc(const pico8_zone_choisie_t *z, size_t n) {
    assert(z != NULL && "allocation sans zone designee");
    return z->alloc(z, n);
}

unsigned char *pico8_zone_grow(const pico8_zone_choisie_t *z, unsigned char *p,
                               size_t old_n, size_t new_n) {
    assert(z != NULL && "agrandissement sans zone designee");
    assert(old_n <= new_n && (p != NULL || old_n == 0));
    return z->grow(z, p, old_n, new_n);
}

void pico8_zone_release(const pico8_zone_choisie_t *z, unsigned char *p) {
    assert(z != NULL && "rendu sans zone designee");
    z->release(z, p);
}

/* `handle` en premier : l'adresse de la poignee est celle du bloc. */
typedef struct {
    pico8_zone_choisie_t handle;
    pico8_bump_zone_t bump;
    const pico8_zone_choisie_t *from;
} arene_t;

#define ARENE_HEAD_CELLS PICO8_ZONE_CELLS(sizeof(arene_t))

/* Piege : pas de pico8_bump_zone_init, qui empoisonne deux moities sous
 * ASan, donc au-dela du bloc ; et pas de reset, l'arene se rend entiere. */
const pico8_zone_choisie_t *pico8_zone_arene_prendre(const pico8_zone_choisie_t *from,
                                                     const char *name, size_t size) {
    size_t cells = size / sizeof(pico8_zone_cell_t);
    assert(cells > ARENE_HEAD_CELLS && "arene plus petite que sa tete");
    pico8_zone_cell_t *block = (pico8_zone_cell_t *)pico8_zone_alloc(from, size);
    arene_t *arene = (arene_t *)block;
    pico8_bump_zone_init_single(&arene->bump, name, block + ARENE_HEAD_CELLS,
                                (cells - ARENE_HEAD_CELLS) * sizeof(pico8_zone_cell_t));
    arene->handle = pico8_zone_choisie_bump(&arene->bump);
    arene->from = from;
    return &arene->handle;
}

void pico8_zone_arene_rendre(const pico8_zone_choisie_t *arene) {
    const arene_t *a = (const arene_t *)arene;
    assert(arene != NULL && arene->bump == &a->bump && "rendu d'une poignee qui n'est pas une arene");
    pico8_zone_release(a->from, (unsigned char *)a);
}
