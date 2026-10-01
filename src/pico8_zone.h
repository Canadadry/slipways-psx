#ifndef PICO8_ZONE_H
#define PICO8_ZONE_H

#include <stddef.h>

/* Le stockage est fourni par le proprietaire de la zone, declare par
 * PICO8_BUMP_ZONE_STORAGE ou PICO8_SLOT_ZONE_STORAGE avec la meme capacite
 * que celle passee a l'init. Le nom de la zone doit vivre aussi longtemps
 * qu'elle : il compose le message de pico8_fatal quand elle est pleine. */

/* Chaque allocation commence sur une cellule, alignee pour tout objet. */
typedef union {
    unsigned long long u;
    char *p;
} pico8_zone_cell_t;

#define PICO8_ZONE_CELLS(bytes) \
    (((bytes) + sizeof(pico8_zone_cell_t) - 1) / sizeof(pico8_zone_cell_t))

#if defined(__SANITIZE_ADDRESS__)
#define PICO8_ZONE_ASAN 1
#elif defined(__has_feature)
#if __has_feature(address_sanitizer)
#define PICO8_ZONE_ASAN 1
#endif
#endif

#ifdef PICO8_ZONE_ASAN
#define PICO8_BUMP_ZONE_HALVES 2
#else
#define PICO8_BUMP_ZONE_HALVES 1
#endif

#define PICO8_BUMP_ZONE_STORAGE(cells, capacity) \
    static pico8_zone_cell_t cells[PICO8_BUMP_ZONE_HALVES * PICO8_ZONE_CELLS(capacity)]

#define PICO8_SLOT_ZONE_STORAGE(cells, taken, slot_size, count) \
    static pico8_zone_cell_t cells[PICO8_ZONE_CELLS(slot_size) * (count)]; \
    static unsigned char taken[count]

typedef struct {
    const char *name;
    pico8_zone_cell_t *storage;
    size_t capacity;
    size_t used;
    size_t half;
    int jamais_remise; /* zone d'une arene : pico8_bump_zone_reset s'y arrete */
} pico8_bump_zone_t;

typedef struct {
    const char *name;
    pico8_zone_cell_t *storage;
    unsigned char *taken;
    size_t slot_size;
    size_t count;
    size_t cursor;
} pico8_slot_zone_t;

void pico8_bump_zone_init(pico8_bump_zone_t *zone, const char *name,
                          pico8_zone_cell_t *storage, size_t capacity);
/* Une seule moitie, quel que soit le build, et jamais de reset : la zone
 * d'une arene (pico8_zone_choisie.h), qui n'a que son bloc. */
void pico8_bump_zone_init_single(pico8_bump_zone_t *zone, const char *name,
                                 pico8_zone_cell_t *storage, size_t capacity);
/* Memoire non initialisee, valable jusqu'au prochain reset. */
unsigned char *pico8_bump_zone_alloc(pico8_bump_zone_t *zone, size_t size);
void pico8_bump_zone_reset(pico8_bump_zone_t *zone);

void pico8_slot_zone_init(pico8_slot_zone_t *zone, const char *name,
                          pico8_zone_cell_t *storage, unsigned char *taken,
                          size_t slot_size, size_t count);
/* Une place de slot_size octets non initialises, valable jusqu'a son
 * release. */
unsigned char *pico8_slot_zone_alloc(pico8_slot_zone_t *zone);
void pico8_slot_zone_release(pico8_slot_zone_t *zone, unsigned char *slot);

#endif
