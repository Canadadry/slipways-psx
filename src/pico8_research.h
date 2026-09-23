#ifndef PICO8_RESEARCH_H
#define PICO8_RESEARCH_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_ob.h"
#include "pico8_value.h"

/* Throughout this module -- this file and its `_display` sibling -- a NULL
 * instance argument is a safe no-op, and an absent or wrong-kind field
 * reads as its zero/false default rather than crashing. */

/* Also wires "lq" as a PICO8_VALUE_FUNCTION on `hp`'s defaults: `jo:fe`
 * reaches it through `mv(b,"lq")`, so a plain C function is not enough.
 * `jb`/`po`/`od` have no such field and are direct calls only. Calls
 * pico8_research_shutdown() first. */
void pico8_research_init(void);

/* Frees every `hp` instance `gb` still holds, then `gb` and the class.
 * Walks `gb`'s positional entries only: its named `gb[px]` entries alias
 * the same instances, so freeing both would free them twice. Idempotent, and
 * safe before the first pico8_research_init(). */
void pico8_research_shutdown(void);

/* NULL before pico8_research_init() and after pico8_research_shutdown(). */
pico8_class_t *pico8_hp_class(void);

/* `hp`'s own `pq` registers the new instance into `gb` under its `px`, so
 * pico8_research_build_gb() must have built `gb` first. Nothing ever frees
 * the instance except pico8_research_shutdown(). */
pico8_instance_t *pico8_hp_new(const pico8_ob_table_t *overrides);

/* Rebuilds `gb` from `fd`, freeing whatever a previous call left. A bare
 * string entry of `fd` sets the running `rr` category every later record is
 * tagged with. `fd`'s strings are borrowed, never copied, so `fd` must
 * outlive `gb` -- pico8_data_shutdown() frees them in that order. A NULL
 * `fd` leaves `gb` empty. */
void pico8_research_build_gb(const pico8_ob_table_t *fd);

/* Owned by this module; callers must not free it. */
const pico8_ob_table_t *pico8_gb(void);

/* Returns an `hc` entry borrowed from `hc`'s own storage, never a copy, or
 * NULL when `self.ja` does not resolve to one. */
const pico8_ob_table_t *pico8_hp_po(const pico8_instance_t *self);

pico8_fixed_t pico8_hp_od(const pico8_instance_t *self);

#endif
