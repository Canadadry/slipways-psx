#ifndef GAME_PAD_NAV_H
#define GAME_PAD_NAV_H

#include "engine/entity_class.h"
#include "engine/entity_id_type.h"
#include "p8/input.h"

typedef enum { PAD_NAV_MAP_FREE, PAD_NAV_MAP_NETWORK } pad_nav_map_mode_t;

typedef struct {
    entity_id_t focus;
    pad_nav_map_mode_t map_mode;
    entity_id_t selection;
    int release_pending;
    int previous_pad;
    int cursor_x;
    int cursor_y;
    int held_frames[4];
    int cross_engaged;
} pad_nav_t;

void pad_nav_init(pad_nav_t *nav);

/* Writes only the played frame's mouse_x, mouse_y and mouse_buttons. */
void pad_nav_step(pad_nav_t *nav, int pad_buttons, p8_input_frame_t *played);

/* NULL when no panel is open, the panel has no item, or the focus is no longer one of its items. */
entity_t *pad_nav_focus(const pad_nav_t *nav);

int pad_nav_panel_open(void);

/* A game is running, no panel is open and the map mode is free: the cursor is the screen centre and the D-pad pans the camera. */
int pad_nav_free_mode(const pad_nav_t *nav);

/* The selected planet in network mode, panel open or not; NULL in free mode or once that planet is gone or dead. */
entity_t *pad_nav_selection(const pad_nav_t *nav);

/* NULL when no panel is open or the panel has no item. */
entity_t *pad_nav_first_item(void);

/* NULL when no item lies in that direction. */
entity_t *pad_nav_neighbor(const entity_t *item, int direction);

int pad_nav_is_item(const entity_t *e);

#endif
