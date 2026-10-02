#include "game/ui_zone.h"

#include <assert.h>
#include <stddef.h>

#include "engine/zone_sizes.h"

/* Taille : la plus grande place que le jeu construit, en-tete d'arene
 * compris (x86-64). Nombre : ce qui vit a la fois au pire de la frame. */
#define BUTTON_SIZE 6240u
#define BUTTON_COUNT 24u
#define MODAL_BACKDROP_SIZE 256u
#define MODAL_BACKDROP_COUNT 4u
#define MESSAGE_BOX_SIZE 4624u
#define MESSAGE_BOX_COUNT 6u
#define TOAST_SIZE 320u
#define TOAST_COUNT 128u
#define ROW_SIZE 1192u
#define ROW_COUNT 100u
#define EPHEMERALS_SIZE 216u
#define EPHEMERALS_COUNT 16u

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
static allocator_t g_handles[SLOT_KIND_COUNT];
static int g_ready;

/* Paresseuse : pico8_slot_zone_init empoisonne le stockage sous ASan, ce
 * qu'un initialiseur statique ne ferait pas. */
static void init_once(void) {
    if (g_ready) return;
    allocator_init_slot_kinds(SLOT_KINDS, SLOT_KIND_COUNT, g_zones, g_handles);
    g_ready = 1;
}

const allocator_t *ui_zone(slot_kind_t slot_kind) {
    assert(slot_kind >= 0 && slot_kind < SLOT_KIND_COUNT);
    init_once();
    return &g_handles[slot_kind];
}

const allocator_t *ui_zone_slot(slot_kind_t slot_kind) {
    return arena_take(ui_zone(slot_kind), SLOT_KINDS[slot_kind].slot_name, SLOT_KINDS[slot_kind].size);
}

int ui_zone_live(slot_kind_t slot_kind) {
    assert(slot_kind >= 0 && slot_kind < SLOT_KIND_COUNT);
    init_once();
    return (int)slot_zone_taken(&g_zones[slot_kind]);
}
