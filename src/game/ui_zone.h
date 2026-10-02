#ifndef GAME_UI_ZONE_H
#define GAME_UI_ZONE_H

#include "engine/allocator.h"

/* Une sorte prend une place a elle par objet ; `rangee` est un `lq`
 * reaffecte dans un objet qui lui survit, `le` une liste `ok.le`. */
typedef enum {
    SLOT_KIND_BUTTON,
    SLOT_KIND_MODAL_BACKDROP,
    SLOT_KIND_MESSAGE_BOX,
    SLOT_KIND_TOAST,
    SLOT_KIND_ROW,
    SLOT_KIND_EPHEMERALS,
    SLOT_KIND_COUNT
} slot_kind_t;

/* Poignee qui donne et reprend une place entiere de la sorte : alloc(n)
 * exige n <= taille de place, grow est refuse. */
const allocator_t *ui_zone(slot_kind_t slot_kind);

/* Une place de la sorte, en arene nommee « place <sorte> » ; on la rend
 * par pico8_zone_arene_rendre. Zone pleine : pico8_fatal « zone interface
 * <sorte> pleine ». */
const allocator_t *ui_zone_slot(slot_kind_t slot_kind);

/* Places prises et pas encore rendues. */
int ui_zone_live(slot_kind_t slot_kind);

#endif
