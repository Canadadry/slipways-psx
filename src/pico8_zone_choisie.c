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
    pico8_zone_choisie_t z = {bump_alloc, bump_grow, bump_release, bump, NULL};
    return z;
}

static unsigned char *slot_alloc(const pico8_zone_choisie_t *z, size_t n) {
    assert(n <= z->slot->slot_size && "zone a places : bloc plus grand qu'une place");
    return pico8_slot_zone_alloc(z->slot);
}

static unsigned char *slot_grow(const pico8_zone_choisie_t *z, unsigned char *p,
                                size_t old_n, size_t new_n) {
    (void)z;
    (void)p;
    (void)old_n;
    (void)new_n;
    assert(0 && "zone a places : une place ne grandit pas");
    return NULL;
}

static void slot_release(const pico8_zone_choisie_t *z, unsigned char *p) {
    if (p != NULL) pico8_slot_zone_release(z->slot, p);
}

pico8_zone_choisie_t pico8_zone_choisie_slot(pico8_slot_zone_t *slot) {
    assert(slot != NULL && "poignee sur une zone absente");
    pico8_zone_choisie_t z = {slot_alloc, slot_grow, slot_release, NULL, slot};
    return z;
}

static pico8_bascule_zone_t *bascule_of(const pico8_zone_choisie_t *z) {
    return ((const pico8_zone_choisie_bascule_t *)z)->zone;
}

static unsigned char *bascule_alloc(const pico8_zone_choisie_t *z, size_t n) {
    return pico8_bascule_zone_alloc(bascule_of(z), n);
}

static unsigned char *bascule_grow(const pico8_zone_choisie_t *z, unsigned char *p,
                                   size_t old_n, size_t new_n) {
    unsigned char *q = pico8_bascule_zone_alloc(bascule_of(z), new_n);
    if (old_n > 0) memcpy(q, p, old_n);
    return q;
}

const pico8_zone_choisie_t *pico8_zone_choisie_bascule(pico8_zone_choisie_bascule_t *storage,
                                                       pico8_bascule_zone_t *zone) {
    assert(storage != NULL && zone != NULL && "poignee sur une zone absente");
    pico8_zone_choisie_t z = {bascule_alloc, bascule_grow, bump_release, NULL, NULL};
    storage->handle = z;
    storage->zone = zone;
    return &storage->handle;
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

size_t pico8_zone_arene_prise(const pico8_zone_choisie_t *arene) {
    const arene_t *a = (const arene_t *)arene;
    assert(arene != NULL && arene->bump == &a->bump && "mesure d'une poignee qui n'est pas une arene");
    return ARENE_HEAD_CELLS * sizeof(pico8_zone_cell_t) + a->bump.used;
}

void pico8_zone_arene_vider(const pico8_zone_choisie_t *arene) {
    arene_t *a = (arene_t *)arene;
    assert(arene != NULL && arene->bump == &a->bump && "vidage d'une poignee qui n'est pas une arene");
    pico8_bump_zone_init_single(&a->bump, a->bump.name, a->bump.storage, a->bump.capacity);
}

void pico8_zone_arene_rendre(const pico8_zone_choisie_t *arene) {
    const arene_t *a = (const arene_t *)arene;
    assert(arene != NULL && arene->bump == &a->bump && "rendu d'une poignee qui n'est pas une arene");
    pico8_zone_release(a->from, (unsigned char *)a);
}
