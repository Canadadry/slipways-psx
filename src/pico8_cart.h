#ifndef PICO8_CART_H
#define PICO8_CART_H

#include "pico8_ui_button.h"

/* Runs every module's init, then builds the tables the cart's top-level
 * statements build (real Lua runs them once before `_init()`), but only
 * when pico8_dispatch_mem() is set: with a NULL mem they stay unbuilt. A
 * second call, as the tests make, frees what the first built before
 * building it again, so a pointer into it taken in between dangles.
 *
 * The asserts in its body cover the ordering constraints that were found;
 * an adjacency they do not cover is not thereby free. Re-derive before
 * inserting an `_init()`. */
void pico8_cart_load(void);

/* Vide la file de sortie, apres avoir remis a nil `o_` et `gj` de l'`ok`
 * courant, `kc` et `ju.ed` qui designaient une entite rendue.
 * pico8__update60(), pico8_cart_load() et pico8_cart_shutdown() l'appellent
 * en tete. */
void pico8_cart_vider_file_sortie(void);

/* Frees everything pico8_cart_load() built, in reverse order. Idempotent,
 * and safe before the first pico8_cart_load(). No Lua equivalent. */
void pico8_cart_shutdown(void);

/* Requires pico8_cart_load(). */
void pico8__init(void);

/* `_init` as a button's `my`: `self`, the button, is discarded: `_init()`
 * declares no parameter. */
void pico8_init_dispatch(pico8_js_t *self);

/* Starts a game from the clicked difficulty button `nj`, which becomes `dk`
 * (borrowed, not copied); a NULL `nj` gives a NULL `dk`.
 * Requires pico8_cart_load(). */
void pico8_eu(pico8_js_t *nj);

/* Requires pico8_cart_load() and at least one prior pico8__init()/pico8_eu();
 * a not-yet-built `ok` or `hv` is a no-op rather than an error. */
void pico8__update60(void);

/* Reads the cart memory through pico8_dispatch_mem(); a NULL mem makes the
 * drawing calls that need it no-ops. */
void pico8__draw(void);

#endif
