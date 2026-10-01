#include "zone_tas.h"

#include <stdlib.h>

static unsigned char *tas_alloc(const pico8_zone_choisie_t *z, size_t n) {
    (void)z;
    unsigned char *p = malloc(n);
    if (p == NULL) abort();
    return p;
}

static unsigned char *tas_grow(const pico8_zone_choisie_t *z, unsigned char *p, size_t old_n,
                               size_t new_n) {
    (void)z;
    (void)old_n;
    unsigned char *q = realloc(p, new_n);
    if (q == NULL) abort();
    return q;
}

static void tas_release(const pico8_zone_choisie_t *z, unsigned char *p) {
    (void)z;
    free(p);
}

const pico8_zone_choisie_t zone_tas = {tas_alloc, tas_grow, tas_release, NULL, NULL};
