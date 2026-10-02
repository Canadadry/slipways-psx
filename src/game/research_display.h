#ifndef PICO8_RESEARCH_DISPLAY_H
#define PICO8_RESEARCH_DISPLAY_H

#include "engine/entity_class.h"
#include "p8/fixed.h"
#include "game/layout.h"
#include "engine/pointer_list.h"
#include "game/research.h"

/* Returns a row in the frame zone, valid until its next reset, or NULL
 * when pico8_jo_ex() does. The two items appended to `items` after
 * construction miss the `fs`/`w` `lq:pq` computed, so `lq:ol` skips them --
 * but `js:pq`'s copy of the row, built from all its items, draws them. */
pico8_lq_t *pico8_hp_lq(pico8_hp_t *self);

/* Returns a row of one line, its texts included, valid until the next
 * reset of the frame zone, or NULL when `self.ii` is truthy. */
const pico8_jb_row_t *pico8_hp_jb(const pico8_hp_t *self);

/* A static function of `jo`, not a method: `jo.ex(...)`, never `jo:ex(...)`.
 * Returns a row in `zone`, or NULL when `o` is NULL (nil), having taken
 * nothing from `zone` -- which `jo:hb` does reach, for a planet whose `kw`
 * carries no `qe`. `o` is read during the call only. Its tuples are in the
 * frame zone: a pico8_lq_copy of the row is only valid in the frame that
 * built it. */
pico8_lq_t *pico8_jo_ex(const pico8_zone_choisie_t *zone, const pico8_ptrs_t *is, const pico8_fixed_t *o,
                        pico8_fixed_t jq, pico8_fixed_t mj, pico8_fixed_t d);

/* The projects of `gb` that `bj` keeps, in `gb` order. */
typedef struct {
    pico8_hp_t *items[PICO8_FD_COUNT];
    int count;
} pico8_bj_kept_t;

/* The Lua parameter is literally named `jo`: a local shadowing the class,
 * naming the planet the projects are offered on. A `jo_instance` without
 * `.nk.px` matches nothing. */
pico8_bj_kept_t pico8_bj(const entity_t *jo_instance);

#endif
