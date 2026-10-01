#include "engine/zone.h"

#include <assert.h>
#include <stdio.h>

#include "p8/fatal.h"
#include "engine/zone_sizes.h"

#ifdef PICO8_ZONE_ASAN
#include <sanitizer/asan_interface.h>
#define POISON(addr, size) ASAN_POISON_MEMORY_REGION((addr), (size))
#define UNPOISON(addr, size) ASAN_UNPOISON_MEMORY_REGION((addr), (size))
#else
#define POISON(addr, size) ((void)(addr), (void)(size))
#define UNPOISON(addr, size) ((void)(addr), (void)(size))
#endif

static void zone_full(const char *name, size_t limit, const char *unit) {
    char message[160];
    snprintf(message, sizeof message, "zone %s pleine, limite %lu %s",
             name, (unsigned long)limit, unit);
    pico8_fatal(message);
}

static unsigned char *bump_half(const pico8_bump_zone_t *zone) {
    return (unsigned char *)(zone->storage + zone->half * PICO8_ZONE_CELLS(zone->capacity));
}

void pico8_bump_zone_init(pico8_bump_zone_t *zone, const char *name,
                          pico8_zone_cell_t *storage, size_t capacity) {
    zone->name = name;
    zone->storage = storage;
    zone->capacity = capacity;
    zone->used = 0;
    zone->half = 0;
    zone->jamais_remise = 0;
    POISON(storage, PICO8_BUMP_ZONE_HALVES * PICO8_ZONE_CELLS(capacity) * sizeof *storage);
}

void pico8_bump_zone_init_single(pico8_bump_zone_t *zone, const char *name,
                                 pico8_zone_cell_t *storage, size_t capacity) {
    zone->name = name;
    zone->storage = storage;
    zone->capacity = capacity;
    zone->used = 0;
    zone->half = 0;
    zone->jamais_remise = 1;
    POISON(storage, PICO8_ZONE_CELLS(capacity) * sizeof *storage);
}

unsigned char *pico8_bump_zone_alloc(pico8_bump_zone_t *zone, size_t size) {
    size_t rounded = PICO8_ZONE_CELLS(size) * sizeof(pico8_zone_cell_t);
    if (rounded > zone->capacity - zone->used)
        zone_full(zone->name, zone->capacity, "octets");
    unsigned char *p = bump_half(zone) + zone->used;
    zone->used += rounded;
    UNPOISON(p, size);
    return p;
}

/* Sous ASan, chaque reset passe a l'autre moitie et empoisonne celle qu'il
 * quitte : revenir au debut rendrait ses adresses aux allocations suivantes,
 * et un pointeur perime y lirait sans planter. */
void pico8_bump_zone_reset(pico8_bump_zone_t *zone) {
    assert(!zone->jamais_remise &&
           "reset d'une zone a une moitie : une arene ne se remet jamais a zero, on la rend");
    POISON(bump_half(zone), PICO8_ZONE_CELLS(zone->capacity) * sizeof(pico8_zone_cell_t));
    zone->half = (zone->half + 1) % PICO8_BUMP_ZONE_HALVES;
    zone->used = 0;
}

void pico8_bascule_zone_init(pico8_bascule_zone_t *zone, const char *name,
                             pico8_zone_cell_t *storage, size_t capacity) {
    zone->name = name;
    zone->storage = storage;
    zone->capacity = capacity;
    zone->bas = 0;
    zone->haut = 0;
    zone->courant_haut = 0;
    zone->en_attente = 0;
    zone->max = 0;
    POISON(storage, PICO8_ZONE_CELLS(capacity) * sizeof *storage);
}

static unsigned char *bascule_fin(const pico8_bascule_zone_t *zone) {
    return (unsigned char *)(zone->storage + PICO8_ZONE_CELLS(zone->capacity));
}

unsigned char *pico8_bascule_zone_alloc(pico8_bascule_zone_t *zone, size_t size) {
    size_t rounded = PICO8_ZONE_CELLS(size) * sizeof(pico8_zone_cell_t);
    if (rounded > zone->capacity - zone->bas - zone->haut)
        zone_full(zone->name, zone->capacity, "octets");
    unsigned char *p;
    if (zone->courant_haut) {
        zone->haut += rounded;
        p = bascule_fin(zone) - zone->haut;
    } else {
        p = (unsigned char *)zone->storage + zone->bas;
        zone->bas += rounded;
    }
    if (zone->bas + zone->haut > zone->max) zone->max = zone->bas + zone->haut;
    UNPOISON(p, size);
    return p;
}

void pico8_bascule_zone_basculer(pico8_bascule_zone_t *zone) {
    assert(!zone->en_attente &&
           "bascule alors que l'ancienne generation attend son rendu : vider la file de sortie entre deux eu");
    zone->courant_haut = !zone->courant_haut;
    assert((zone->courant_haut ? zone->haut : zone->bas) == 0);
    zone->en_attente = 1;
}

int pico8_bascule_zone_en_attente(const pico8_bascule_zone_t *zone) {
    return zone->en_attente;
}

/* Pas de seconde moitie sous ASan : les generations alternent deja entre les
 * bouts. Angle mort : un pointeur garde a travers deux bascules retombe sur
 * le bout reutilise et ne plante plus. */
void pico8_bascule_zone_rendre(pico8_bascule_zone_t *zone) {
    if (!zone->en_attente) return;
    if (zone->courant_haut) {
        POISON(zone->storage, zone->bas);
        zone->bas = 0;
    } else {
        POISON(bascule_fin(zone) - zone->haut, zone->haut);
        zone->haut = 0;
    }
    zone->en_attente = 0;
}

void pico8_bascule_zone_reset(pico8_bascule_zone_t *zone) {
    POISON(zone->storage, PICO8_ZONE_CELLS(zone->capacity) * sizeof(pico8_zone_cell_t));
    zone->bas = 0;
    zone->haut = 0;
    zone->courant_haut = 0;
    zone->en_attente = 0;
}

size_t pico8_bascule_zone_prise(const pico8_bascule_zone_t *zone) {
    return zone->bas + zone->haut;
}

size_t pico8_bascule_zone_max(const pico8_bascule_zone_t *zone) {
    return zone->max;
}

static unsigned char *slot_at(const pico8_slot_zone_t *zone, size_t index) {
    return (unsigned char *)(zone->storage + index * PICO8_ZONE_CELLS(zone->slot_size));
}

void pico8_slot_zone_init(pico8_slot_zone_t *zone, const char *name,
                          pico8_zone_cell_t *storage, unsigned char *taken,
                          size_t slot_size, size_t count) {
    assert(slot_size > 0 && count > 0);
    zone->name = name;
    zone->storage = storage;
    zone->taken = taken;
    zone->slot_size = slot_size;
    zone->count = count;
    zone->cursor = 0;
    for (size_t i = 0; i < count; i++) taken[i] = 0;
    POISON(storage, count * PICO8_ZONE_CELLS(slot_size) * sizeof *storage);
}

/* Curseur tournant, pas de liste libre LIFO : une place rendue n'est reprise
 * qu'apres un tour complet, sinon un pointeur vers elle retomberait tout de
 * suite sur une place redevenue valide. */
unsigned char *pico8_slot_zone_alloc(pico8_slot_zone_t *zone) {
    for (size_t step = 0; step < zone->count; step++) {
        size_t i = (zone->cursor + step) % zone->count;
        if (!zone->taken[i]) {
            zone->taken[i] = 1;
            zone->cursor = (i + 1) % zone->count;
            unsigned char *p = slot_at(zone, i);
            UNPOISON(p, zone->slot_size);
            return p;
        }
    }
    zone_full(zone->name, zone->count, "places");
    return NULL;
}

void pico8_slot_zone_release(pico8_slot_zone_t *zone, unsigned char *slot) {
    size_t stride = PICO8_ZONE_CELLS(zone->slot_size) * sizeof(pico8_zone_cell_t);
    unsigned char *base = slot_at(zone, 0);
    assert(slot >= base && slot < base + zone->count * stride &&
           "release d'un pointeur hors de la zone");
    size_t i = (size_t)(slot - base) / stride;
    assert(slot == slot_at(zone, i) && "release d'un pointeur qui n'est pas un debut de place");
    assert(zone->taken[i] && "release d'une place deja rendue");
    zone->taken[i] = 0;
    POISON(slot, zone->slot_size);
}

size_t pico8_slot_zone_taken(const pico8_slot_zone_t *zone) {
    size_t n = 0;
    for (size_t i = 0; i < zone->count; i++) n += zone->taken[i];
    return n;
}
