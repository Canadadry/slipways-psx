#ifndef ENGINE_SLOT_GROUP_H
#define ENGINE_SLOT_GROUP_H

#include <stddef.h>

#include "engine/allocator.h"
#include "engine/slot_tally.h"
#include "engine/zone.h"

struct slot_group;

/* The counting handle of a kind: alloc asks for the slot size at most and
 * counts the slot, grow is refused, release counts it back. */
typedef struct {
    allocator_t handle; /* first: its address is the whole struct's */
    struct slot_group *group;
    int kind;
} slot_group_handle_t;

/* The slot zones of a table of kinds behind counting handles, set up at
 * first use: allocator_init_slot_kinds poisons the storage under ASan,
 * which a static initializer would not do. Peaks hold since the process
 * started. Declare one with SLOT_GROUP. */
typedef struct slot_group {
    const slot_kind_zone_t *kinds;
    int n;
    int ready;
    slot_zone_t zones[SLOT_TALLY_KINDS_MAX];
    allocator_t slots[SLOT_TALLY_KINDS_MAX];
    slot_group_handle_t handles[SLOT_TALLY_KINDS_MAX];
    slot_tally_t tally;
    int max_elements[SLOT_TALLY_KINDS_MAX];
} slot_group_t;

/* kinds must outlive the group. */
#define SLOT_GROUP(kind_table, kind_count) {.kinds = (kind_table), .n = (kind_count)}

const allocator_t *slot_group_handle(slot_group_t *g, int kind);
/* A slot of an arena kind, as an arena named after the kind's slot_name,
 * given back by arena_release. */
const allocator_t *slot_group_slot(slot_group_t *g, int kind);

int slot_group_live(slot_group_t *g, int kind);
int slot_group_max_live(slot_group_t *g, int kind);
/* See slot_tally_max_bytes. */
size_t slot_group_max_bytes(slot_group_t *g, int kind);
size_t slot_group_used(slot_group_t *g);
size_t slot_group_max_used(slot_group_t *g);
size_t slot_group_reserved(slot_group_t *g);

int slot_group_count(const slot_group_t *g, int kind);
size_t slot_group_slot_size(const slot_group_t *g, int kind);
int slot_group_is_arena(const slot_group_t *g, int kind);

/* The largest element count recorded for a list held in a slot of the
 * kind. */
void slot_group_record_elements(slot_group_t *g, int kind, int count);
int slot_group_max_elements(const slot_group_t *g, int kind);

#endif
