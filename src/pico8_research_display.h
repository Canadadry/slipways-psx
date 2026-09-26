#ifndef PICO8_RESEARCH_DISPLAY_H
#define PICO8_RESEARCH_DISPLAY_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_list.h"
#include "pico8_ob.h"
#include "pico8_value.h"

/* TROU-7 */
/* Returns a fresh `lq` instance, or NULL when pico8_jo_ex() does. The two
 * entries appended after construction miss the `fs`/`w` `lq:pq` computed,
 * so `lq:ol` skips them -- but `js:pq` rebuilds an `lq` from this whole
 * fields table, where they do appear. */
pico8_instance_t *pico8_hp_lq(pico8_instance_t *self);

/* TROU-7 */
/* Returns a fresh table the caller never frees, or NULL when `self.ii` is
 * truthy. Its entry 1 holds a heap string nothing ever frees either. */
pico8_ob_table_t *pico8_hp_jb(const pico8_instance_t *self);

/* TROU-7 */
/* A static function of `jo`, not a method: `jo.ex(...)`, never `jo:ex(...)`.
 * Returns a fresh `lq` instance, or NULL when `o` is falsy -- which `jo:hb`
 * does reach, for a planet whose `kw` carries no `qe`. Reads `jo.em` through
 * pico8_jo_em(), so `jo`'s own init must have run. */
pico8_instance_t *pico8_jo_ex(const pico8_list_t *is, pico8_value_t o, pico8_fixed_t jq, pico8_fixed_t mj,
                               pico8_fixed_t d);

/* The Lua parameter is literally named `jo`: a local shadowing the class,
 * naming the planet the projects are offered on. Returns a new list the
 * caller owns and must release with pico8_list_free(). A `jo_instance`
 * without `.nk.px` matches nothing. */
pico8_list_t pico8_bj(const pico8_instance_t *jo_instance);

#endif
