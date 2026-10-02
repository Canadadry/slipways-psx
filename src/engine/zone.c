#include "engine/zone.h"

#include <assert.h>
#include <stdio.h>

#include "p8/fatal.h"
#include "engine/zone_sizes.h"

#ifdef ZONE_ASAN
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

static unsigned char *bump_half(const bump_zone_t *zone) {
    return (unsigned char *)(zone->storage + zone->half * ZONE_CELLS(zone->capacity));
}

void bump_zone_init(bump_zone_t *zone, const char *name,
                          zone_cell_t *storage, size_t capacity) {
    zone->name = name;
    zone->storage = storage;
    zone->capacity = capacity;
    zone->used = 0;
    zone->half = 0;
    zone->never_reset = 0;
    POISON(storage, BUMP_ZONE_HALVES * ZONE_CELLS(capacity) * sizeof *storage);
}

void bump_zone_init_single(bump_zone_t *zone, const char *name,
                                 zone_cell_t *storage, size_t capacity) {
    zone->name = name;
    zone->storage = storage;
    zone->capacity = capacity;
    zone->used = 0;
    zone->half = 0;
    zone->never_reset = 1;
    POISON(storage, ZONE_CELLS(capacity) * sizeof *storage);
}

unsigned char *bump_zone_alloc(bump_zone_t *zone, size_t size) {
    size_t rounded = ZONE_CELLS(size) * sizeof(zone_cell_t);
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
void bump_zone_reset(bump_zone_t *zone) {
    assert(!zone->never_reset &&
           "reset d'une zone a une moitie : une arene ne se remet jamais a zero, on la rend");
    POISON(bump_half(zone), ZONE_CELLS(zone->capacity) * sizeof(zone_cell_t));
    zone->half = (zone->half + 1) % BUMP_ZONE_HALVES;
    zone->used = 0;
}

void flip_zone_init(flip_zone_t *zone, const char *name,
                             zone_cell_t *storage, size_t capacity) {
    zone->name = name;
    zone->storage = storage;
    zone->capacity = capacity;
    zone->bottom = 0;
    zone->top = 0;
    zone->current_top = 0;
    zone->pending = 0;
    zone->max = 0;
    POISON(storage, ZONE_CELLS(capacity) * sizeof *storage);
}

static unsigned char *flip_zone_end(const flip_zone_t *zone) {
    return (unsigned char *)(zone->storage + ZONE_CELLS(zone->capacity));
}

unsigned char *flip_zone_alloc(flip_zone_t *zone, size_t size) {
    size_t rounded = ZONE_CELLS(size) * sizeof(zone_cell_t);
    if (rounded > zone->capacity - zone->bottom - zone->top)
        zone_full(zone->name, zone->capacity, "octets");
    unsigned char *p;
    if (zone->current_top) {
        zone->top += rounded;
        p = flip_zone_end(zone) - zone->top;
    } else {
        p = (unsigned char *)zone->storage + zone->bottom;
        zone->bottom += rounded;
    }
    if (zone->bottom + zone->top > zone->max) zone->max = zone->bottom + zone->top;
    UNPOISON(p, size);
    return p;
}

void flip_zone_flip(flip_zone_t *zone) {
    assert(!zone->pending &&
           "bascule alors que l'ancienne generation attend son rendu : vider la file de sortie entre deux eu");
    zone->current_top = !zone->current_top;
    assert((zone->current_top ? zone->top : zone->bottom) == 0);
    zone->pending = 1;
}

int flip_zone_pending(const flip_zone_t *zone) {
    return zone->pending;
}

/* Pas de seconde moitie sous ASan : les generations alternent deja entre les
 * bouts. Angle mort : un pointeur garde a travers deux bascules retombe sur
 * le bout reutilise et ne plante plus. */
void flip_zone_release(flip_zone_t *zone) {
    if (!zone->pending) return;
    if (zone->current_top) {
        POISON(zone->storage, zone->bottom);
        zone->bottom = 0;
    } else {
        POISON(flip_zone_end(zone) - zone->top, zone->top);
        zone->top = 0;
    }
    zone->pending = 0;
}

void flip_zone_reset(flip_zone_t *zone) {
    POISON(zone->storage, ZONE_CELLS(zone->capacity) * sizeof(zone_cell_t));
    zone->bottom = 0;
    zone->top = 0;
    zone->current_top = 0;
    zone->pending = 0;
}

size_t flip_zone_used(const flip_zone_t *zone) {
    return zone->bottom + zone->top;
}

size_t flip_zone_max(const flip_zone_t *zone) {
    return zone->max;
}

static unsigned char *slot_at(const slot_zone_t *zone, size_t index) {
    return (unsigned char *)(zone->storage + index * ZONE_CELLS(zone->slot_size));
}

void slot_zone_init(slot_zone_t *zone, const char *name,
                          zone_cell_t *storage, unsigned char *taken,
                          size_t slot_size, size_t count) {
    assert(slot_size > 0 && count > 0);
    zone->name = name;
    zone->storage = storage;
    zone->taken = taken;
    zone->slot_size = slot_size;
    zone->count = count;
    zone->cursor = 0;
    for (size_t i = 0; i < count; i++) taken[i] = 0;
    POISON(storage, count * ZONE_CELLS(slot_size) * sizeof *storage);
}

/* Curseur tournant, pas de liste libre LIFO : une place rendue n'est reprise
 * qu'apres un tour complet, sinon un pointeur vers elle retomberait tout de
 * suite sur une place redevenue valide. */
unsigned char *slot_zone_alloc(slot_zone_t *zone) {
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

void slot_zone_release(slot_zone_t *zone, unsigned char *slot) {
    size_t stride = ZONE_CELLS(zone->slot_size) * sizeof(zone_cell_t);
    unsigned char *base = slot_at(zone, 0);
    assert(slot >= base && slot < base + zone->count * stride &&
           "release d'un pointeur hors de la zone");
    size_t i = (size_t)(slot - base) / stride;
    assert(slot == slot_at(zone, i) && "release d'un pointeur qui n'est pas un debut de place");
    assert(zone->taken[i] && "release d'une place deja rendue");
    zone->taken[i] = 0;
    POISON(slot, zone->slot_size);
}

size_t slot_zone_taken(const slot_zone_t *zone) {
    size_t n = 0;
    for (size_t i = 0; i < zone->count; i++) n += zone->taken[i];
    return n;
}
