#ifndef GAME_PAD_NAV_H
#define GAME_PAD_NAV_H

#include "engine/entity_class.h"
#include "engine/entity_id_type.h"
#include "p8/input.h"

typedef struct {
    entity_id_t focus;
    int previous_pad;
    int cursor_x;
    int cursor_y;
} pad_nav_t;

void pad_nav_init(pad_nav_t *nav);

/* Writes only the played frame's mouse_x, mouse_y and mouse_buttons. */
void pad_nav_step(pad_nav_t *nav, int pad_buttons, p8_input_frame_t *played);

/* NULL when no panel is open, the panel has no item, or the focus is no longer one of its items. */
entity_t *pad_nav_focus(const pad_nav_t *nav);

int pad_nav_panel_open(void);

/* NULL when no panel is open or the panel has no item. */
entity_t *pad_nav_first_item(void);

/* NULL when no item lies in that direction. */
entity_t *pad_nav_neighbor(const entity_t *item, int direction);

int pad_nav_is_item(const entity_t *e);

#endif
