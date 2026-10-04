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

/* Places a list from ui_ephemerals_new() takes in its slot at creation. */
#define UI_EPHEMERALS_RESERVE 8

/* A handle that gives and takes back a whole slot of the kind: an
 * allocation no larger than the slot size, never a grow. */
const allocator_t *ui_zone(slot_kind_t slot_kind);

/* A slot of the kind, as an arena named `slot <kind>`, given back by
 * arena_release. A full zone calls p8_fatal with `zone ui <kind> full`. */
const allocator_t *ui_zone_slot(slot_kind_t slot_kind);

/* Slots taken and not yet given back. */
int ui_zone_live(slot_kind_t slot_kind);

/* Maxima since the process started: live slots of the kind; the largest
 * slot of the kind, header included, the slots still live measured when
 * read; the reserved bytes of the live slots of every kind, as
 * SLOT_ZONE_BYTES counts them. */
int ui_zone_max_live(slot_kind_t slot_kind);
size_t ui_zone_max_bytes(slot_kind_t slot_kind);
size_t ui_zone_used(void);
size_t ui_zone_max_used(void);

/* Ephemerals only: the largest element count ui_zone_record_elements has
 * seen for a list, since the process started. */
void ui_zone_record_elements(slot_kind_t slot_kind, int count);
int ui_zone_max_elements(slot_kind_t slot_kind);

/* Limits: slots of the kind, bytes of one, bytes reserved for the zone. */
int ui_zone_count(slot_kind_t slot_kind);
size_t ui_zone_slot_size(slot_kind_t slot_kind);
size_t ui_zone_reserved(void);

#endif
