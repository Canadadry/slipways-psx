#ifndef PICO8_PI_H
#define PICO8_PI_H

#include "pico8_fixed.h"
#include "pico8_gfx.h"
#include "pico8_list.h"
#include "pico8_map_tables.h"
#include "pico8_mem.h"

/* `pi` has two consumers needing two lookups: ob() indexes it by a single
 * character (pico8_ob_register_constructor, pico8_ob.h), bl() by the whole
 * string an entity holds in `.fn`. This module owns the string-keyed one. */

/* Draws article `e` of `bl()` from its items, `e[1]..e[5]` in the cart. */
typedef void (*pico8_pi_fn)(const pico8_jb_row_t *e);

/* Fixed capacity: every registration happens once, from pico8_pi_init().
 * A name of 8 characters or more would be truncated on the way in and
 * never found again by pico8_pi_dispatch_lookup(). */
#define PICO8_PI_DISPATCH_CAPACITY 8

void pico8_pi_dispatch_register(const char *name, pico8_pi_fn fn);

/* NULL when `name` was never registered. */
pico8_pi_fn pico8_pi_dispatch_lookup(const char *name);

/* Test-support only (no cart equivalent): resets the table to empty. */
void pico8_pi_dispatch_clear(void);

/* Every element of `rb` must be a PICO8_VALUE_REF to an article, a
 * `const pico8_jb_row_t *` whose `fn` names its drawing function. An
 * article with no `fn`, or one naming an unregistered function, draws
 * nothing.
 *
 * pico8_bl_default is the `nc == nil` case, where the camera-shake branch
 * never runs at all. `gfx`/`mem` are borrowed, and also stashed in this
 * module's dispatch context for the constructors to read. */
void pico8_bl(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_list_t *rb, pico8_fixed_t nc);
void pico8_bl_default(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_list_t *rb);

/* Fills both registries; idempotent. Must run before ob() parses a spec
 * holding `v(...)` or `b(...)`: an unregistered identifier falls back to a
 * plain positional table, so the field silently becomes something other
 * than a vector or a box. Modules whose specs need that call this
 * themselves.
 *
 * "ld" dispatches to lq:ol, so pico8_layout_init() must have run before
 * that entry is exercised. */
void pico8_pi_init(void);

#endif
