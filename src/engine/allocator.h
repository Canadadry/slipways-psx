#ifndef ENGINE_ALLOCATOR_H
#define ENGINE_ALLOCATOR_H

#include <stddef.h>

#include "engine/zone.h"

/* The handle an owner or a caller passes to the shared functions that
 * allocate on its behalf. A block is grown or released only through the
 * handle that allocated it. */
typedef struct allocator {
    unsigned char *(*alloc)(const struct allocator *z, size_t n);
    unsigned char *(*grow)(const struct allocator *z, unsigned char *p,
                           size_t old_n, size_t new_n);
    void (*release)(const struct allocator *z, unsigned char *p);
    bump_zone_t *bump; /* NULL unless a bump zone */
    slot_zone_t *slot; /* NULL unless a slot zone */
} allocator_t;

/* Handle on an already initialised zone, which must outlive it. Release
 * does nothing: the memory comes back at the zone's reset. */
allocator_t allocator_from_bump_zone(bump_zone_t *bump);

/* Handle on an already initialised slot zone, which must outlive it: each
 * block is a whole slot. alloc asks for at most the slot size, grow is
 * refused, release gives the slot back. */
allocator_t allocator_from_slot_zone(slot_zone_t *slot);

/* The grow of a slot zone handle: asserts, returns NULL. */
unsigned char *allocator_grow_refused(const allocator_t *z, unsigned char *p,
                                       size_t old_n, size_t new_n);

/* One kind of slot; slot_name is NULL when the slot is not an arena. */
typedef struct {
    const char *zone_name;
    const char *slot_name;
    zone_cell_t *cells;
    unsigned char *taken;
    size_t size;
    size_t count;
} slot_kind_zone_t;

/* For each kind, initialises the zone at the same index and stores its
 * handle at that index of handles. */
void allocator_init_slot_kinds(const slot_kind_zone_t *s, int n, slot_zone_t *zones,
                            allocator_t *handles);

/* Handle on the current generation of a two-ended zone, kept in separate
 * storage: one more field in allocator_t would grow the header of every
 * arena, and slots are sized to fit exactly. */
typedef struct {
    allocator_t handle; /* first: its address is the whole struct's */
    flip_zone_t *zone;
} flip_zone_allocator_t;

/* Stores in storage a handle on the already initialised zone and returns
 * it; storage and zone must outlive the handle. Release does nothing and
 * grow copies: the memory comes back when its generation is released. */
const allocator_t *allocator_from_flip_zone(flip_zone_allocator_t *storage,
                                                       flip_zone_t *zone);

/* Uninitialised memory; p8_fatal when the zone is full. */
unsigned char *allocator_alloc(const allocator_t *z, size_t n);
/* Returns a block of new_n bytes that starts with the old_n bytes of the
 * old block; the old block, NULL only when old_n is 0, is not used again. */
unsigned char *allocator_grow(const allocator_t *z, unsigned char *p,
                               size_t old_n, size_t new_n);
/* The block may be NULL. */
void allocator_release(const allocator_t *z, unsigned char *p);

/* Arena: a block of size bytes taken from the from handle, whose header
 * holds a private bump zone and the returned handle, which lives as long as
 * the block. What is allocated there lives until arena_release; releasing a
 * block there does nothing, growing copies and leaves the old block dead.
 * The name must live as long as the arena: it makes up "zone <name> full". */
const allocator_t *arena_take(const allocator_t *from,
                                                     const char *name, size_t size);
/* The size to pass to arena_take for an arena that holds exactly n bytes of
 * blocks, rounded up to whole cells. */
size_t arena_size_for(size_t n);
/* The size the arena was taken with, header included. */
size_t arena_size(const allocator_t *arena);
/* Bytes taken in the block, header included. */
size_t arena_used(const allocator_t *arena);
/* From now on, a block or a growth that no longer fits in the arena is
 * taken from the handle the arena came from instead of a p8_fatal: it lives
 * as long as that handle's memory, not as long as the arena, and
 * arena_release does not give it back. arena_flush ends it. */
void arena_spill(const allocator_t *arena);
/* Brings the arena back to its header alone, out of arena_spill, and
 * poisons the rest of its block; the block stays taken from its zone. ASan
 * blind spot: the next allocations reuse the same addresses, so a pointer
 * kept from before the flush reads the new block there without crashing. */
void arena_flush(const allocator_t *arena);
/* Releases the whole block to the handle it was taken from. */
void arena_release(const allocator_t *arena);

#endif
