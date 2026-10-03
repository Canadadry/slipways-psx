#ifndef GAME_MENU_H
#define GAME_MENU_H

#include "engine/entity_class.h"
#include "game/ui_button.h"

/* Throughout this module a NULL main economy reads as era 0 with nothing
 * invented, and a NULL main score panel or score-panel layout block skips
 * what needs it; neither is a crash. */

/* Rebuilds the tech-tree rows and hands them to message_box_open_box().
 * Their texts, and the layout block's description, are in the frame zone:
 * the buttons copy them. */
void menu_show_tech_tree_screen(void);

/* The button of the galaxy menu's given row (1..GALAXY_MENU_COUNT), from
 * galaxy_menu_table, its on_click (and refresh) the row's callbacks;
 * asserts before the init. */
button_t *galaxy_menu_button(int i);

/* The on_click callback of the galaxy menu's given row. The pause menu's
 * rows get theirs from the fourth row's callback, on every run. */
button_fn_t galaxy_menu_on_click(int i);

/* The struct that carries the pause menu's own fixed_pos, row_step and back
 * color, which the fourth galaxy row's callback opens the pause menu with;
 * NULL before the init and after the shutdown. */
const entity_t *pause_menu_open_box(void);
const entity_class_t *pause_menu_open_box_class(void);

/* An init resets the galaxy menu's positions and the pause menu's fixed_pos
 * and row_step to their generated values. */
void menu_init(void);
void menu_shutdown(void);

#endif
