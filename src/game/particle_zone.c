#include "game/particle_zone.h"

#include <assert.h>

#include "engine/slot_tally.h"
#include "engine/zone_sizes.h"

/* Size: arena header included (x86-64), a list holds just its reserved
 * capacity. Each count and capacity: its peak in make state plus 1/8, but
 * the help screen's list, a singleton (one slot, no margin). */
#define PARTICLES_SIZE (96u + 24u + PARTICLES_CAPACITY * 8u)
#define PARTICLES_HELP_SCREEN_SIZE (96u + 24u + PARTICLES_HELP_SCREEN_CAPACITY * 8u)
#define BURST_SIZE 640u
#define PARTICLES_COUNT 18
#define PARTICLES_HELP_SCREEN_COUNT 1
#define BURST_COUNT 4
/* Accepted risk: ~21 transit dots live per slipway served both ways, and
 * 437 slots hold about twenty such visible slipways. */
#define TRANSIT_DOTS_COUNT 437
/* Accepted risk: the particles grow with the game, the help screen's 580
 * alone on the title. */
#define PARTICLE_CAPACITY 887

typedef char particle_zone_holds_help_screen_t[PARTICLE_CAPACITY >= PARTICLES_HELP_SCREEN_CAPACITY ? 1 : -1];

typedef char particle_zone_exceeded_t[
    (SLOT_ZONE_BYTES(PARTICLE_SLOT, PARTICLE_CAPACITY) + SLOT_ZONE_BYTES(PARTICLES_SIZE, PARTICLES_COUNT) +
     SLOT_ZONE_BYTES(PARTICLES_HELP_SCREEN_SIZE, PARTICLES_HELP_SCREEN_COUNT) + SLOT_ZONE_BYTES(BURST_SIZE, BURST_COUNT) +
     SLOT_ZONE_BYTES(TRANSIT_DOTS_SLOT, TRANSIT_DOTS_COUNT)
     <= PARTICLES_ZONE_BYTES) ? 1 : -1];

SLOT_ZONE_STORAGE(particle_cells, particle_taken, PARTICLE_SLOT, PARTICLE_CAPACITY);
SLOT_ZONE_STORAGE(particles_cells, particles_taken, PARTICLES_SIZE, PARTICLES_COUNT);
SLOT_ZONE_STORAGE(particles_help_screen_cells, particles_help_screen_taken, PARTICLES_HELP_SCREEN_SIZE, PARTICLES_HELP_SCREEN_COUNT);
SLOT_ZONE_STORAGE(burst_cells, burst_taken, BURST_SIZE, BURST_COUNT);
SLOT_ZONE_STORAGE(transit_dots_cells, transit_dots_taken, TRANSIT_DOTS_SLOT, TRANSIT_DOTS_COUNT);

static const slot_kind_zone_t SLOT_KINDS[PARTICLE_SLOT_COUNT] = {
    [PARTICLE_SLOT_PARTICLE] = {"particles particle", NULL, particle_cells, particle_taken,
                                    PARTICLE_SLOT, PARTICLE_CAPACITY},
    [PARTICLE_SLOT_LIST] = {"particles particles", "slot particles", particles_cells, particles_taken, PARTICLES_SIZE, PARTICLES_COUNT},
    [PARTICLE_SLOT_HELP_LIST] = {"particles particles_help_screen", "slot particles_help_screen", particles_help_screen_cells, particles_help_screen_taken, PARTICLES_HELP_SCREEN_SIZE,
                                PARTICLES_HELP_SCREEN_COUNT},
    [PARTICLE_SLOT_BURST] = {"particles burst", "slot burst", burst_cells, burst_taken, BURST_SIZE, BURST_COUNT},
    [PARTICLE_SLOT_TRANSIT_DOT] = {"particles transit_dots", NULL, transit_dots_cells, transit_dots_taken, TRANSIT_DOTS_SLOT, TRANSIT_DOTS_COUNT},
};

static slot_zone_t g_zones[PARTICLE_SLOT_COUNT];
static allocator_t g_slots[PARTICLE_SLOT_COUNT];
static int g_ready;
static slot_tally_t g_tally;
static int g_max_elements[PARTICLE_SLOT_COUNT];

static particle_slot_kind_t slot_kind_of(const allocator_t *z);

/* Lazy: slot_zone_init, through allocator_init_slot_kinds, poisons the
 * storage under ASan, which a static initializer would not do. */
static void init_once(void) {
    if (g_ready) return;
    allocator_init_slot_kinds(SLOT_KINDS, PARTICLE_SLOT_COUNT, g_zones, g_slots);
    slot_tally_init(&g_tally, SLOT_KINDS, g_zones, PARTICLE_SLOT_COUNT);
    g_ready = 1;
}

static int is_arena(particle_slot_kind_t slot_kind) {
    return SLOT_KINDS[slot_kind].slot_name != NULL;
}

static unsigned char *slot_alloc(const allocator_t *z, size_t n) {
    particle_slot_kind_t slot_kind = slot_kind_of(z);
    init_once();
    unsigned char *p = allocator_alloc(&g_slots[slot_kind], n);
    slot_tally_take(&g_tally, slot_kind);
    return p;
}

static void slot_release(const allocator_t *z, unsigned char *p) {
    particle_slot_kind_t slot_kind = slot_kind_of(z);
    if (p == NULL) return;
    slot_tally_give_back(&g_tally, slot_kind, p);
    allocator_release(&g_slots[slot_kind], p);
}

static const allocator_t slot_allocators[PARTICLE_SLOT_COUNT] = {
    [PARTICLE_SLOT_PARTICLE] = {slot_alloc, allocator_grow_refused, slot_release, NULL, NULL},
    [PARTICLE_SLOT_LIST] = {slot_alloc, allocator_grow_refused, slot_release, NULL, NULL},
    [PARTICLE_SLOT_HELP_LIST] = {slot_alloc, allocator_grow_refused, slot_release, NULL, NULL},
    [PARTICLE_SLOT_BURST] = {slot_alloc, allocator_grow_refused, slot_release, NULL, NULL},
    [PARTICLE_SLOT_TRANSIT_DOT] = {slot_alloc, allocator_grow_refused, slot_release, NULL, NULL},
};

static particle_slot_kind_t slot_kind_of(const allocator_t *z) {
    assert(z >= slot_allocators && z < slot_allocators + PARTICLE_SLOT_COUNT && "slot_kind_of: handle outside the particle zone");
    return (particle_slot_kind_t)(z - slot_allocators);
}

const allocator_t *particle_zone(particle_slot_kind_t slot_kind) {
    assert(slot_kind >= 0 && slot_kind < PARTICLE_SLOT_COUNT);
    return &slot_allocators[slot_kind];
}

const allocator_t *particle_zone_slot(particle_slot_kind_t slot_kind) {
    assert(slot_kind >= 0 && slot_kind < PARTICLE_SLOT_COUNT && is_arena(slot_kind) &&
           "particle_zone_slot: a particle is a bare slot, not an arena");
    return arena_take(&slot_allocators[slot_kind], SLOT_KINDS[slot_kind].slot_name, SLOT_KINDS[slot_kind].size);
}

int particle_zone_live(particle_slot_kind_t slot_kind) {
    assert(slot_kind >= 0 && slot_kind < PARTICLE_SLOT_COUNT);
    init_once();
    assert((size_t)slot_tally_live(&g_tally, slot_kind) == slot_zone_taken(&g_zones[slot_kind]) &&
           "particle_zone_live: a slot taken or given back without its kind's handle");
    return slot_tally_live(&g_tally, slot_kind);
}

int particle_zone_max_live(particle_slot_kind_t slot_kind) {
    assert(slot_kind >= 0 && slot_kind < PARTICLE_SLOT_COUNT);
    init_once();
    return slot_tally_max_live(&g_tally, slot_kind);
}

size_t particle_zone_max_bytes(particle_slot_kind_t slot_kind) {
    assert(slot_kind >= 0 && slot_kind < PARTICLE_SLOT_COUNT);
    init_once();
    return slot_tally_max_bytes(&g_tally, slot_kind);
}

void particle_zone_record_elements(particle_slot_kind_t slot_kind, int count) {
    assert((slot_kind == PARTICLE_SLOT_LIST || slot_kind == PARTICLE_SLOT_HELP_LIST || slot_kind == PARTICLE_SLOT_TRANSIT_DOT) &&
           "particle_zone_record_elements: only a list has elements");
    if (count > g_max_elements[slot_kind]) g_max_elements[slot_kind] = count;
}

int particle_zone_max_elements(particle_slot_kind_t slot_kind) {
    assert(slot_kind >= 0 && slot_kind < PARTICLE_SLOT_COUNT);
    return g_max_elements[slot_kind];
}

size_t particle_zone_used(void) {
    init_once();
    return slot_tally_zone_bytes(&g_tally);
}

size_t particle_zone_max_used(void) {
    init_once();
    return slot_tally_zone_max_bytes(&g_tally);
}

int particle_zone_count(particle_slot_kind_t slot_kind) {
    assert(slot_kind >= 0 && slot_kind < PARTICLE_SLOT_COUNT);
    return (int)SLOT_KINDS[slot_kind].count;
}

size_t particle_zone_slot_size(particle_slot_kind_t slot_kind) {
    assert(slot_kind >= 0 && slot_kind < PARTICLE_SLOT_COUNT);
    return SLOT_KINDS[slot_kind].size;
}

size_t particle_zone_reserved(void) {
    init_once();
    return slot_tally_zone_reserved(&g_tally);
}
