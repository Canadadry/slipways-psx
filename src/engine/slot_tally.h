#ifndef ENGINE_SLOT_TALLY_H
#define ENGINE_SLOT_TALLY_H

#include <stddef.h>

#include "engine/allocator.h"
#include "engine/zone.h"

#define SLOT_TALLY_KINDS_MAX 8

/* Counts of a group of slot kinds, set up by allocator_init_slot_kinds: the
 * zone's owner reports each slot it takes and gives back. Peaks hold since
 * slot_tally_init. */
typedef struct {
    const slot_kind_zone_t *kinds;
    const slot_zone_t *zones;
    int n;
    int live[SLOT_TALLY_KINDS_MAX];
    int max_live[SLOT_TALLY_KINDS_MAX];
    size_t max_bytes[SLOT_TALLY_KINDS_MAX];
    size_t zone_bytes;
    size_t zone_max_bytes;
} slot_tally_t;

/* kinds and zones must outlive the tally. */
void slot_tally_init(slot_tally_t *t, const slot_kind_zone_t *kinds, const slot_zone_t *zones, int n);
void slot_tally_take(slot_tally_t *t, int kind);
/* Before the slot goes back to its zone: an arena is measured there. */
void slot_tally_give_back(slot_tally_t *t, int kind, const unsigned char *slot);

int slot_tally_live(const slot_tally_t *t, int kind);
int slot_tally_max_live(const slot_tally_t *t, int kind);
/* The largest arena of the kind, header included, the arenas still live
 * measured when read; 0 for a kind that is not an arena. */
size_t slot_tally_max_bytes(slot_tally_t *t, int kind);
/* The reserved bytes of the live slots of every kind, as SLOT_ZONE_BYTES
 * counts them. */
size_t slot_tally_zone_bytes(const slot_tally_t *t);
size_t slot_tally_zone_max_bytes(const slot_tally_t *t);
/* What SLOT_ZONE_STORAGE reserves for every slot of every kind. */
size_t slot_tally_zone_reserved(const slot_tally_t *t);

#endif
