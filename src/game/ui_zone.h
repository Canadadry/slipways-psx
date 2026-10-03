#ifndef GAME_UI_ZONE_H
#define GAME_UI_ZONE_H

#include <stddef.h>

#include "engine/allocator.h"

/* Each kind takes a slot of its own per object; a row slot holds a
 * `layout_row_t` reassigned inside an object that outlives it, an
 * ephemerals slot the `ephemerals` list of a `ui_t`. */
typedef enum {
    SLOT_KIND_BUTTON,
    SLOT_KIND_MODAL_BACKDROP,
    SLOT_KIND_MESSAGE_BOX,
    SLOT_KIND_TOAST,
    SLOT_KIND_ROW,
    SLOT_KIND_EPHEMERALS,
    SLOT_KIND_COUNT
} slot_kind_t;

/* A handle that gives and takes back a whole slot of the kind: an
 * allocation no larger than the slot size, never a grow. */
const allocator_t *ui_zone(slot_kind_t slot_kind);

/* A slot of the kind, as an arena named `slot <kind>`, given back by
 * arena_release. A full zone calls p8_fatal with `zone ui <kind> full`. */
const allocator_t *ui_zone_slot(slot_kind_t slot_kind);

/* Slots taken and not yet given back. */
int ui_zone_live(slot_kind_t slot_kind);
/* Session maximum of ui_zone_live. */
int ui_zone_max_live(slot_kind_t slot_kind);
/* Slots of the kind, and bytes of one slot. */
int ui_zone_capacity(slot_kind_t slot_kind);
size_t ui_zone_slot_size(slot_kind_t slot_kind);

#endif
