#include "pico8_zone.h"

#include <assert.h>
#include <stdio.h>

#include "pico8_fatal.h"
#include "pico8_zone_sizes.h"

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
    zone->halves = PICO8_BUMP_ZONE_HALVES;
    POISON(storage, PICO8_BUMP_ZONE_HALVES * PICO8_ZONE_CELLS(capacity) * sizeof *storage);
}

void pico8_bump_zone_init_single(pico8_bump_zone_t *zone, const char *name,
                                 pico8_zone_cell_t *storage, size_t capacity) {
    zone->name = name;
    zone->storage = storage;
    zone->capacity = capacity;
    zone->used = 0;
    zone->half = 0;
    zone->halves = 1;
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
    assert(zone->halves == PICO8_BUMP_ZONE_HALVES &&
           "reset d'une zone a une moitie : une arene ne se remet jamais a zero, on la rend");
    POISON(bump_half(zone), PICO8_ZONE_CELLS(zone->capacity) * sizeof(pico8_zone_cell_t));
    zone->half = (zone->half + 1) % PICO8_BUMP_ZONE_HALVES;
    zone->used = 0;
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
