#ifndef PICO8_MENU_H
#define PICO8_MENU_H

#include "pico8_ob.h"
#include "pico8_struct.h"

/* Throughout this module a missing singleton (`fm`, `ju`, `pj`'s class) or an
 * absent table is a safe no-op rather than a crash. */

/* TROU-7 */
/* Rebuilds the tech-tree rows and hands them to `pz`; the rows and the
 * `pj.cr.ep` string a previous call left behind are dropped, not freed. */
void pico8_cr(void);

/* Owned by this module and only borrowed by callers: built by
 * `pico8_menu_init()`, freed by `pico8_menu_shutdown()`, NULL in between.
 * `pico8_menu_init()` is what installs the rows' "my"/"hb" callbacks. */
const pico8_ob_table_t *pico8_ix(void);

/* Same ownership. `ha`'s rows carry no "my" until `ix[4]`'s own callback runs,
 * and it re-installs them on every run, as in the cart. */
const pico8_ob_table_t *pico8_ha(void);

/* The struct `ix[4]`'s callback hands `pz` for `ha`, with `ha`'s own
 * `hi`/`gq`/`ca`; NULL whenever `pico8_ha()` is. */
const void *pico8_ha_pz(void);
const pico8_struct_class_t *pico8_ha_pz_struct_class(void);

/* An init frees the previous `ix`/`ha` and reparses both, so any row pointer
 * taken before it dangles after it. */
void pico8_menu_init(void);
void pico8_menu_shutdown(void);

#endif
