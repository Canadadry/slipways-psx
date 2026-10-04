#include "game/ui_zone.h"

#include <assert.h>
#include <stddef.h>

#include "engine/slot_tally.h"
#include "engine/zone_sizes.h"

/* Sizes in bytes on x86-64, arena header included. Each count, and each
 * size but two: its peak in make state plus 1/8. The button and row sizes
 * are the largest the game's rules can build, which the tests
 * test_largest_buttons_fit_their_slots and
 * test_planet_resource_row_largest_row_fits_a_slot build: no margin. An
 * ephemerals slot holds its arena header, its list and the list's
 * reserve. */
#define BUTTON_SIZE 5880u
#define BUTTON_COUNT 14u
#define MODAL_BACKDROP_SIZE 272u
#define MODAL_BACKDROP_COUNT 2u
#define MESSAGE_BOX_SIZE 5056u
#define MESSAGE_BOX_COUNT 4u
#define TOAST_SIZE 336u
#define TOAST_COUNT 48u
#define ROW_SIZE 1176u
#define ROW_COUNT 71u
#define EPHEMERALS_SIZE (96u + 24u + UI_EPHEMERALS_RESERVE * 8u)
#define EPHEMERALS_COUNT 6u

typedef char ui_zone_exceeded_t[
    (SLOT_ZONE_BYTES(BUTTON_SIZE, BUTTON_COUNT) + SLOT_ZONE_BYTES(MODAL_BACKDROP_SIZE, MODAL_BACKDROP_COUNT) +
     SLOT_ZONE_BYTES(MESSAGE_BOX_SIZE, MESSAGE_BOX_COUNT) + SLOT_ZONE_BYTES(TOAST_SIZE, TOAST_COUNT) +
     SLOT_ZONE_BYTES(ROW_SIZE, ROW_COUNT) + SLOT_ZONE_BYTES(EPHEMERALS_SIZE, EPHEMERALS_COUNT)
     <= UI_ZONE_BYTES) ? 1 : -1];

SLOT_ZONE_STORAGE(button_cells, button_taken, BUTTON_SIZE, BUTTON_COUNT);
SLOT_ZONE_STORAGE(modal_backdrop_cells, modal_backdrop_taken, MODAL_BACKDROP_SIZE, MODAL_BACKDROP_COUNT);
SLOT_ZONE_STORAGE(message_box_cells, message_box_taken, MESSAGE_BOX_SIZE, MESSAGE_BOX_COUNT);
SLOT_ZONE_STORAGE(toast_cells, toast_taken, TOAST_SIZE, TOAST_COUNT);
SLOT_ZONE_STORAGE(row_cells, row_taken, ROW_SIZE, ROW_COUNT);
SLOT_ZONE_STORAGE(ephemerals_cells, ephemerals_taken, EPHEMERALS_SIZE, EPHEMERALS_COUNT);

static const slot_kind_zone_t SLOT_KINDS[SLOT_KIND_COUNT] = {
    [SLOT_KIND_BUTTON] = {"ui button", "slot button", button_cells, button_taken, BUTTON_SIZE, BUTTON_COUNT},
    [SLOT_KIND_MODAL_BACKDROP] = {"ui modal_backdrop", "slot modal_backdrop", modal_backdrop_cells, modal_backdrop_taken, MODAL_BACKDROP_SIZE, MODAL_BACKDROP_COUNT},
    [SLOT_KIND_MESSAGE_BOX] = {"ui message_box", "slot message_box", message_box_cells, message_box_taken, MESSAGE_BOX_SIZE, MESSAGE_BOX_COUNT},
    [SLOT_KIND_TOAST] = {"ui toast", "slot toast", toast_cells, toast_taken, TOAST_SIZE, TOAST_COUNT},
    [SLOT_KIND_ROW] = {"ui row", "slot row", row_cells, row_taken, ROW_SIZE,
                            ROW_COUNT},
    [SLOT_KIND_EPHEMERALS] = {"ui ephemerals", "slot ephemerals", ephemerals_cells, ephemerals_taken, EPHEMERALS_SIZE, EPHEMERALS_COUNT},
};

static slot_zone_t g_zones[SLOT_KIND_COUNT];
static allocator_t g_slots[SLOT_KIND_COUNT];
static int g_ready;
static slot_tally_t g_tally;
static int g_max_ephemerals_elements;

static slot_kind_t slot_kind_of(const allocator_t *z);

/* Lazy: allocator_init_slot_kinds poisons the storage under ASan, which a
 * static initializer would not do. */
static void init_once(void) {
    if (g_ready) return;
    allocator_init_slot_kinds(SLOT_KINDS, SLOT_KIND_COUNT, g_zones, g_slots);
    slot_tally_init(&g_tally, SLOT_KINDS, g_zones, SLOT_KIND_COUNT);
    g_ready = 1;
}

static unsigned char *slot_alloc(const allocator_t *z, size_t n) {
    slot_kind_t slot_kind = slot_kind_of(z);
    init_once();
    unsigned char *p = allocator_alloc(&g_slots[slot_kind], n);
    slot_tally_take(&g_tally, slot_kind);
    return p;
}

static void slot_release(const allocator_t *z, unsigned char *p) {
    slot_kind_t slot_kind = slot_kind_of(z);
    if (p == NULL) return;
    slot_tally_give_back(&g_tally, slot_kind, p);
    allocator_release(&g_slots[slot_kind], p);
}

static const allocator_t slot_allocators[SLOT_KIND_COUNT] = {
    [SLOT_KIND_BUTTON] = {slot_alloc, allocator_grow_refused, slot_release, NULL, NULL},
    [SLOT_KIND_MODAL_BACKDROP] = {slot_alloc, allocator_grow_refused, slot_release, NULL, NULL},
    [SLOT_KIND_MESSAGE_BOX] = {slot_alloc, allocator_grow_refused, slot_release, NULL, NULL},
    [SLOT_KIND_TOAST] = {slot_alloc, allocator_grow_refused, slot_release, NULL, NULL},
    [SLOT_KIND_ROW] = {slot_alloc, allocator_grow_refused, slot_release, NULL, NULL},
    [SLOT_KIND_EPHEMERALS] = {slot_alloc, allocator_grow_refused, slot_release, NULL, NULL},
};

static slot_kind_t slot_kind_of(const allocator_t *z) {
    assert(z >= slot_allocators && z < slot_allocators + SLOT_KIND_COUNT && "slot_kind_of: handle outside the ui zone");
    return (slot_kind_t)(z - slot_allocators);
}

const allocator_t *ui_zone(slot_kind_t slot_kind) {
    assert(slot_kind >= 0 && slot_kind < SLOT_KIND_COUNT);
    return &slot_allocators[slot_kind];
}

const allocator_t *ui_zone_slot(slot_kind_t slot_kind) {
    return arena_take(ui_zone(slot_kind), SLOT_KINDS[slot_kind].slot_name, SLOT_KINDS[slot_kind].size);
}

int ui_zone_live(slot_kind_t slot_kind) {
    assert(slot_kind >= 0 && slot_kind < SLOT_KIND_COUNT);
    init_once();
    assert((size_t)slot_tally_live(&g_tally, slot_kind) == slot_zone_taken(&g_zones[slot_kind]) &&
           "ui_zone_live: a slot taken or given back without its kind's handle");
    return slot_tally_live(&g_tally, slot_kind);
}

int ui_zone_max_live(slot_kind_t slot_kind) {
    assert(slot_kind >= 0 && slot_kind < SLOT_KIND_COUNT);
    init_once();
    return slot_tally_max_live(&g_tally, slot_kind);
}

size_t ui_zone_max_bytes(slot_kind_t slot_kind) {
    assert(slot_kind >= 0 && slot_kind < SLOT_KIND_COUNT);
    init_once();
    return slot_tally_max_bytes(&g_tally, slot_kind);
}

size_t ui_zone_used(void) {
    init_once();
    return slot_tally_zone_bytes(&g_tally);
}

size_t ui_zone_max_used(void) {
    init_once();
    return slot_tally_zone_max_bytes(&g_tally);
}

void ui_zone_record_elements(slot_kind_t slot_kind, int count) {
    assert(slot_kind == SLOT_KIND_EPHEMERALS && "ui_zone_record_elements: only the ephemerals slot holds a list");
    if (count > g_max_ephemerals_elements) g_max_ephemerals_elements = count;
}

int ui_zone_max_elements(slot_kind_t slot_kind) {
    assert(slot_kind == SLOT_KIND_EPHEMERALS && "ui_zone_max_elements: only the ephemerals slot holds a list");
    return g_max_ephemerals_elements;
}

int ui_zone_count(slot_kind_t slot_kind) {
    assert(slot_kind >= 0 && slot_kind < SLOT_KIND_COUNT);
    return (int)SLOT_KINDS[slot_kind].count;
}

size_t ui_zone_slot_size(slot_kind_t slot_kind) {
    assert(slot_kind >= 0 && slot_kind < SLOT_KIND_COUNT);
    return SLOT_KINDS[slot_kind].size;
}

size_t ui_zone_reserved(void) {
    init_once();
    return slot_tally_zone_reserved(&g_tally);
}
