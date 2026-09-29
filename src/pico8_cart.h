#ifndef PICO8_CART_H
#define PICO8_CART_H

#include "pico8_struct.h"
#include "pico8_ui_button.h"
#include "pico8_value.h"

/* Parses every module's spec and builds every data table the cart's
 * top-level statements build, which real Lua runs once before `_init()`.
 * Reads the cart memory through pico8_dispatch_mem(): the caller must have
 * set a `mem` with pico8_load_map_json() already run against it, otherwise
 * the cart data tables are simply not built and every spec still is. A
 * second call frees and reparses every spec, so it dangles every default
 * REF a struct built before it still holds.
 *
 * The asserts in its body cover the ordering constraints that were found;
 * an adjacency they do not cover is not thereby free. Re-derive before
 * inserting an `_init()`. */
void pico8_cart_load(void);

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
