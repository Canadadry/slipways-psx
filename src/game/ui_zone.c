#include "game/ui_zone.h"

#include <stddef.h>

#include "engine/slot_group.h"
#include "engine/zone_sizes.h"

#define BUTTON_SIZE 6056u
#define BUTTON_COUNT 14u
#define MODAL_BACKDROP_SIZE 280u
#define MODAL_BACKDROP_COUNT 2u
#define MESSAGE_BOX_SIZE 5128u
#define MESSAGE_BOX_COUNT 4u
#define TOAST_SIZE 344u
#define TOAST_COUNT 48u
#define ROW_SIZE 1184u
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

static slot_group_t g_group = SLOT_GROUP(SLOT_KINDS, SLOT_KIND_COUNT);

const allocator_t *ui_zone(slot_kind_t slot_kind) {
    return slot_group_handle(&g_group, slot_kind);
}

const allocator_t *ui_zone_slot(slot_kind_t slot_kind) {
    return slot_group_slot(&g_group, slot_kind);
}

int ui_zone_live(slot_kind_t slot_kind) {
    return slot_group_live(&g_group, slot_kind);
}

int ui_zone_max_live(slot_kind_t slot_kind) {
    return slot_group_max_live(&g_group, slot_kind);
}

size_t ui_zone_max_bytes(slot_kind_t slot_kind) {
    return slot_group_max_bytes(&g_group, slot_kind);
}

size_t ui_zone_used(void) {
    return slot_group_used(&g_group);
}

size_t ui_zone_max_used(void) {
    return slot_group_max_used(&g_group);
}

void ui_zone_record_ephemerals(int count) {
    slot_group_record_elements(&g_group, SLOT_KIND_EPHEMERALS, count);
}

int ui_zone_max_ephemerals(void) {
    return slot_group_max_elements(&g_group, SLOT_KIND_EPHEMERALS);
}

int ui_zone_count(slot_kind_t slot_kind) {
    return slot_group_count(&g_group, slot_kind);
}

size_t ui_zone_slot_size(slot_kind_t slot_kind) {
    return slot_group_slot_size(&g_group, slot_kind);
}

size_t ui_zone_reserved(void) {
    return slot_group_reserved(&g_group);
}
