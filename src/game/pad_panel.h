#ifndef GAME_PAD_PANEL_H
#define GAME_PAD_PANEL_H

#include "engine/entity_class.h"

enum { PAD_PANEL_ANY_LAYER = -32768 };

/* The main ui holds at least one ephemeral; 0 without a main ui. */
int pad_panel_box_open(void);

int pad_panel_open(void);

/* Has a left press and a layer: what the ui may press. */
int pad_panel_is_candidate(const entity_t *e);

/* NULL when no hit-tested entity on a layer above `above` both passes accept and holds the screen point in its own hit
 * test; PAD_PANEL_ANY_LAYER lifts the layer bound. */
entity_t *pad_panel_entity_under(int x, int y, int above, int (*accept)(const entity_t *e));

int pad_panel_is_item(const entity_t *e);

/* The centre on screen of e's hit box, where the ui's hit test reads it. */
void pad_panel_focus_point(const entity_t *e, int *x, int *y);

/* NULL when no panel is open or the panel has no item. */
entity_t *pad_panel_first_item(void);

/* NULL when no item lies in that direction. */
entity_t *pad_panel_neighbor(const entity_t *item, int direction);

/* The focus after a move in direction (0 for none), from focus or, when focus is no longer an item, from the first
 * item. NULL when the panel has no item; otherwise also writes the new focus's centre on screen to x and y. Writes
 * P8_MOUSE_LEFT to mouse_buttons while × is held on a focus, 0 otherwise. */
entity_t *pad_panel_step(entity_t *focus, int pad_buttons, int direction, int *x, int *y, int *mouse_buttons);

#endif
