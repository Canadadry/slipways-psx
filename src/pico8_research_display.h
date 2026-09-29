#ifndef PICO8_RESEARCH_DISPLAY_H
#define PICO8_RESEARCH_DISPLAY_H

#include "pico8_struct.h"
#include "pico8_fixed.h"
#include "pico8_layout.h"
#include "pico8_ptrs.h"
#include "pico8_research.h"
#include "pico8_value.h"

/* TROU-7 */
/* Returns a fresh row, or NULL when pico8_jo_ex() does. The two items
 * appended to `items` after construction miss the `fs`/`w` `lq:pq`
 * computed, so `lq:ol` skips them -- but `js:pq`'s copy of the row, built
 * from all its items, draws them. */
pico8_lq_t *pico8_hp_lq(pico8_hp_t *self);

/* TROU-7 */
/* Returns a fresh row of one line, never freed, or NULL when `self.ii` is
 * truthy. The line's first item is a heap string nothing frees either. */
const pico8_jb_row_t *pico8_hp_jb(const pico8_hp_t *self);

/* TROU-7 */
/* A static function of `jo`, not a method: `jo.ex(...)`, never `jo:ex(...)`.
 * Returns a fresh row, or NULL when `o` is NULL (nil) -- which `jo:hb`
 * does reach, for a planet whose `kw` carries no `qe`. `o` is read during the call only. */
pico8_lq_t *pico8_jo_ex(const pico8_ptrs_t *is, const pico8_fixed_t *o, pico8_fixed_t jq, pico8_fixed_t mj,
                         pico8_fixed_t d);

/* The projects of `gb` that `bj` keeps, in `gb` order. */
typedef struct {
    pico8_hp_t *items[PICO8_FD_COUNT];
    int count;
} pico8_bj_kept_t;

/* The Lua parameter is literally named `jo`: a local shadowing the class,
 * naming the planet the projects are offered on. A `jo_instance` without
 * `.nk.px` matches nothing. */
pico8_bj_kept_t pico8_bj(const pico8_struct_header_t *jo_instance);

#endif
