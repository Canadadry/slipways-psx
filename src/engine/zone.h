#ifndef ENGINE_ZONE_H
#define ENGINE_ZONE_H

#include <stddef.h>

/* Le stockage est fourni par le proprietaire de la zone, declare par
 * PICO8_BUMP_ZONE_STORAGE ou PICO8_SLOT_ZONE_STORAGE avec la meme capacite
 * que celle passee a l'init. Le nom de la zone doit vivre aussi longtemps
 * qu'elle : il compose le message de pico8_fatal quand elle est pleine. */

/* Chaque allocation commence sur une cellule, alignee pour tout objet. */
typedef union {
    unsigned long long align_integer;
    char *align_pointer;
} zone_cell_t;

#define ZONE_CELLS(bytes) \
    (((bytes) + sizeof(zone_cell_t) - 1) / sizeof(zone_cell_t))

#if defined(__SANITIZE_ADDRESS__)
#define ZONE_ASAN 1
#elif defined(__has_feature)
#if __has_feature(address_sanitizer)
#define ZONE_ASAN 1
#endif
#endif

#ifdef ZONE_ASAN
#define BUMP_ZONE_HALVES 2
#else
#define BUMP_ZONE_HALVES 1
#endif

#define BUMP_ZONE_STORAGE(cells, capacity) \
    static zone_cell_t cells[BUMP_ZONE_HALVES * ZONE_CELLS(capacity)]

#define FLIP_ZONE_STORAGE(cells, capacity) \
    static zone_cell_t cells[ZONE_CELLS(capacity)]

#define SLOT_ZONE_STORAGE(cells, taken, slot_size, count) \
    static zone_cell_t cells[ZONE_CELLS(slot_size) * (count)]; \
    static unsigned char taken[count]

/* Octets que PICO8_SLOT_ZONE_STORAGE reserve : places et octets pris. */
#define SLOT_ZONE_BYTES(size, count) \
    (ZONE_CELLS(size) * sizeof(zone_cell_t) * (count) + (count))

typedef struct {
    const char *name;
    zone_cell_t *storage;
    size_t capacity;
    size_t used;
    size_t half;
    int never_reset; /* zone d'une arene : pico8_bump_zone_reset s'y arrete */
} bump_zone_t;

typedef struct {
    const char *name;
    zone_cell_t *storage;
    unsigned char *taken;
    size_t slot_size;
    size_t count;
    size_t cursor;
} slot_zone_t;

/* Zone a deux bouts : une generation alloue depuis un bout, la suivante
 * depuis l'autre. */
typedef struct {
    const char *name;
    zone_cell_t *storage;
    size_t capacity;
    size_t bottom;
    size_t top;
    int current_top;
    int pending; /* l'ancienne generation attend pico8_bascule_zone_rendre */
    size_t max;
} flip_zone_t;

void bump_zone_init(bump_zone_t *zone, const char *name,
                          zone_cell_t *storage, size_t capacity);
/* Une seule moitie, quel que soit le build, et jamais de reset : la zone
 * d'une arene (pico8_zone_choisie.h), qui n'a que son bloc. */
void bump_zone_init_single(bump_zone_t *zone, const char *name,
                                 zone_cell_t *storage, size_t capacity);
/* Memoire non initialisee, valable jusqu'au prochain reset. */
unsigned char *bump_zone_alloc(bump_zone_t *zone, size_t size);
void bump_zone_reset(bump_zone_t *zone);

void slot_zone_init(slot_zone_t *zone, const char *name,
                          zone_cell_t *storage, unsigned char *taken,
                          size_t slot_size, size_t count);
/* Une place de slot_size octets non initialises, valable jusqu'a son
 * release. */
unsigned char *slot_zone_alloc(slot_zone_t *zone);
void slot_zone_release(slot_zone_t *zone, unsigned char *slot);
/* Places prises et pas encore rendues. */
size_t slot_zone_taken(const slot_zone_t *zone);

void flip_zone_init(flip_zone_t *zone, const char *name,
                             zone_cell_t *storage, size_t capacity);
/* Memoire non initialisee, prise au bout courant, valable jusqu'au rendu de
 * sa generation ; pico8_fatal quand les deux bouts se rejoignent. */
unsigned char *flip_zone_alloc(flip_zone_t *zone, size_t size);
/* Change de bout ; l'ancienne generation reste valide jusqu'a
 * pico8_bascule_zone_rendre. */
void flip_zone_flip(flip_zone_t *zone);
/* Vrai entre pico8_bascule_zone_basculer et le rendu de l'ancienne
 * generation. */
int flip_zone_pending(const flip_zone_t *zone);
/* Rend et empoisonne l'ancienne generation ; sans generation en attente, ne
 * fait rien. */
void flip_zone_release(flip_zone_t *zone);
/* Rend et empoisonne les deux bouts ; le maximum reste. */
void flip_zone_reset(flip_zone_t *zone);
/* Octets pris par les deux bouts, et leur maximum depuis l'init. */
size_t flip_zone_used(const flip_zone_t *zone);
size_t flip_zone_max(const flip_zone_t *zone);

#endif
