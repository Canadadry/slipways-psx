#ifndef GAME_PARTICLE_ZONE_H
#define GAME_PARTICLE_ZONE_H

#include <stddef.h>

#include "engine/allocator.h"

/* PARTICLE_SLOT_PARTICLE holds an element of a particle list, _LIST an
 * emitter's particle list, _HELP_LIST the help screen's, _BURST a burst with
 * its slipways list and its pos box, _TRANSIT_DOT a transit dot of a
 * slipway. */
typedef enum {
    PARTICLE_SLOT_PARTICLE,
    PARTICLE_SLOT_LIST,
    PARTICLE_SLOT_HELP_LIST,
    PARTICLE_SLOT_BURST,
    PARTICLE_SLOT_TRANSIT_DOT,
    PARTICLE_SLOT_COUNT
} particle_slot_kind_t;

#define PARTICLE_SLOT 32u
#define TRANSIT_DOTS_SLOT 24u
/* Elements reserved for a list of the kind, which its slot holds just. */
#define PARTICLES_CAPACITY 160
#define PARTICLES_HELP_SCREEN_CAPACITY 768

/* The handle that gives and takes back a whole slot of the kind: alloc
 * asks for the slot size at most, grow is refused. A slot of a particle
 * list, a help screen list or a burst is an arena, taken by
 * particle_zone_slot. More live slots than the limit: p8_fatal "zone
 * particles <kind> full, limit <count> slots". */
const allocator_t *particle_zone(particle_slot_kind_t slot_kind);

/* A slot of a particle list, a help screen list or a burst, as an arena
 * named "slot <kind>"; given back by arena_release. */
const allocator_t *particle_zone_slot(particle_slot_kind_t slot_kind);

/* Slots taken and not yet given back. */
int particle_zone_live(particle_slot_kind_t slot_kind);

/* Reserved bytes of the live slots of every kind, as SLOT_ZONE_BYTES
 * counts them, and their maximum since the process started. */
size_t particle_zone_used(void);
size_t particle_zone_max_used(void);

/* Session maxima. Bytes: those of an arena slot, header included, the
 * slots still live measured when read; 0 for a particle. Elements: the
 * largest count particle_zone_record_elements has seen for a particle list,
 * a help screen list, or a slipway's transit dots list. */
int particle_zone_max_live(particle_slot_kind_t slot_kind);
size_t particle_zone_max_bytes(particle_slot_kind_t slot_kind);
void particle_zone_record_elements(particle_slot_kind_t slot_kind, int count);
int particle_zone_max_elements(particle_slot_kind_t slot_kind);

#endif
