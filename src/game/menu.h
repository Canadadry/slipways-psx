#ifndef PICO8_MENU_H
#define PICO8_MENU_H

#include "engine/entity_class.h"
#include "pico8_ui_button.h"

/* Throughout this module a missing singleton (`fm`, `ju`, `pj`'s class) or an
 * absent table is a safe no-op rather than a crash. */

/* Rebuilds the tech-tree rows and hands them to `pz`. Their texts, and
 * `pj.cr.ep`, are in the frame zone: the buttons copy them. */
void pico8_cr(void);

/* The button of `ix`'s row `i` (1..PICO8_IX_COUNT), from `pico8_ix_table`,
 * its `my` (and `hb`) the row's callbacks; asserts before the init. */
pico8_js_t *pico8_ix_button(int i);

/* The `my` callback of `ix`'s row `i`. `ha`'s rows get theirs from `ix[4]`'s,
 * on every run, as in the cart. */
pico8_js_fn pico8_ix_my(int i);

/* The struct `ix[4]`'s callback hands `pz` for `ha`, with `ha`'s own
 * `hi`/`gq`/`ca`; NULL before the init and after the shutdown. */
const pico8_struct_header_t *pico8_ha_pz(void);
const pico8_struct_class_t *pico8_ha_pz_struct_class(void);

/* An init resets `ix`'s `ra` and `ha`'s `hi`/`gq` to their generated
 * values. */
void pico8_menu_init(void);
void pico8_menu_shutdown(void);

#endif
