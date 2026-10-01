#include "pico8_zone_transitoire.h"

#include <stdlib.h>

static unsigned char *transient_alloc(const pico8_zone_choisie_t *z, size_t n) {
    (void)z;
    unsigned char *p = malloc(n);
    if (p == NULL) abort();
    return p;
}

static unsigned char *transient_grow(const pico8_zone_choisie_t *z, unsigned char *p,
                                     size_t old_n, size_t new_n) {
    (void)z;
    (void)old_n;
    unsigned char *q = realloc(p, new_n);
    if (q == NULL) abort();
    return q;
}

static void transient_release(const pico8_zone_choisie_t *z, unsigned char *p) {
    (void)z;
    free(p);
}

const pico8_zone_choisie_t pico8_zone_transitoire = {
    transient_alloc, transient_grow, transient_release, NULL};
