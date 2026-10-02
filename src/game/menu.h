#ifndef GAME_MENU_H
#define GAME_MENU_H

#include "engine/entity_class.h"
#include "game/ui_button.h"

/* Throughout this module a missing singleton (`fm`, `ju`, `pj`'s class) or an
 * absent table is a safe no-op rather than a crash. */

/* Rebuilds the tech-tree rows and hands them to `pz`. Their texts, and
 * `pj.cr.ep`, are in the frame zone: the buttons copy them. */
void menu_show_tech_tree_screen(void);

/* The button of `ix`'s row `i` (1..PICO8_IX_COUNT), from `pico8_ix_table`,
 * its `my` (and `hb`) the row's callbacks; asserts before the init. */
button_t *galaxy_menu_button(int i);

/* The `my` callback of `ix`'s row `i`. `ha`'s rows get theirs from `ix[4]`'s,
 * on every run, as in the cart. */
button_fn_t galaxy_menu_on_click(int i);

/* The struct `ix[4]`'s callback hands `pz` for `ha`, with `ha`'s own
 * `hi`/`gq`/`ca`; NULL before the init and after the shutdown. */
const entity_t *pause_menu_open_box(void);
const entity_class_t *pause_menu_open_box_class(void);

/* An init resets `ix`'s `ra` and `ha`'s `hi`/`gq` to their generated
 * values. */
void menu_init(void);
void menu_shutdown(void);

#endif
