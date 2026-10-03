#ifndef ENGINE_ZONE_H
#define ENGINE_ZONE_H

#include <stddef.h>

/* The storage is provided by the zone's owner, declared by
 * BUMP_ZONE_STORAGE, FLIP_ZONE_STORAGE or SLOT_ZONE_STORAGE with the same
 * capacity as the one passed to init. The zone's name must live as long as
 * the zone: it makes up the p8_fatal message when the zone is full. */

/* Every allocation starts on a cell, aligned for any object. */
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

/* Bytes that SLOT_ZONE_STORAGE reserves: the slots and their taken bytes. */
#define SLOT_ZONE_BYTES(size, count) \
    (ZONE_CELLS(size) * sizeof(zone_cell_t) * (count) + (count))

typedef struct {
    const char *name;
    zone_cell_t *storage;
    size_t capacity;
    size_t used;
    size_t half;
    int never_reset; /* an arena's zone: bump_zone_reset stops on it */
} bump_zone_t;

typedef struct {
    const char *name;
    zone_cell_t *storage;
    unsigned char *taken;
    size_t slot_size;
    size_t count;
    size_t cursor;
} slot_zone_t;

/* Two-ended zone: one generation allocates from one end, the next one from
 * the other. */
typedef struct {
    const char *name;
    zone_cell_t *storage;
    size_t capacity;
    size_t bottom;
    size_t top;
    int current_top;
    int pending; /* the old generation awaits flip_zone_release */
    size_t max;
} flip_zone_t;

void bump_zone_init(bump_zone_t *zone, const char *name,
                          zone_cell_t *storage, size_t capacity);
/* A single half, whatever the build, and never a reset: the zone of an
 * arena (engine/allocator.h), which has only its block. */
void bump_zone_init_single(bump_zone_t *zone, const char *name,
                                 zone_cell_t *storage, size_t capacity);
/* Uninitialised memory, valid until the next reset. */
unsigned char *bump_zone_alloc(bump_zone_t *zone, size_t size);
void bump_zone_reset(bump_zone_t *zone);
/* Brings `used` back to a value it held since the last reset and poisons
 * the room given back: the blocks taken since then are dead. */
void bump_zone_rewind(bump_zone_t *zone, size_t used);

void slot_zone_init(slot_zone_t *zone, const char *name,
                          zone_cell_t *storage, unsigned char *taken,
                          size_t slot_size, size_t count);
/* A slot of slot_size uninitialised bytes, valid until it is
 * released. */
unsigned char *slot_zone_alloc(slot_zone_t *zone);
void slot_zone_release(slot_zone_t *zone, unsigned char *slot);
/* Slots taken and not yet released. */
size_t slot_zone_taken(const slot_zone_t *zone);

void flip_zone_init(flip_zone_t *zone, const char *name,
                             zone_cell_t *storage, size_t capacity);
/* Uninitialised memory, taken at the current end, valid until its
 * generation is released; p8_fatal when the two ends meet. */
unsigned char *flip_zone_alloc(flip_zone_t *zone, size_t size);
/* Switches ends; the old generation stays valid until
 * flip_zone_release. */
void flip_zone_flip(flip_zone_t *zone);
/* Nonzero between flip_zone_flip and the release of the old
 * generation. */
int flip_zone_pending(const flip_zone_t *zone);
/* Releases and poisons the old generation; does nothing when no generation
 * is pending. */
void flip_zone_release(flip_zone_t *zone);
/* Releases and poisons both ends; the maximum stays. */
void flip_zone_reset(flip_zone_t *zone);
/* Bytes taken by both ends, and their maximum since init. */
size_t flip_zone_used(const flip_zone_t *zone);
size_t flip_zone_max(const flip_zone_t *zone);

#endif
