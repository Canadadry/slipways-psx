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
