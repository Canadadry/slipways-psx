#ifndef GAME_GEN_H
#define GAME_GEN_H

#include "p8/fixed.h"
#include "game/game_data.h"
#include "engine/allocator.h"

/* C has no closures, so each of these two closure factories becomes a state
 * struct (the captured upvalues) taken from the caller's zone, plus a
 * `_next()` holding the body the returned closure would have run, one call
 * per Lua invocation. */

/* `b` is an upvalue the body itself rewrites on every step. */
typedef struct {
    p8_fixed_t step;
    p8_fixed_t bias;
} random_walk_t;

/* The state is taken from `zone` and lives as long as it: nothing to
 * release. */
random_walk_t *random_walk_new(const allocator_t *zone, p8_fixed_t n_, p8_fixed_t b);
p8_fixed_t random_walk_next(random_walk_t *state);

/* `gn`'s third argument is a zero-argument Lua closure: a function pointer
 * plus a ctx here, this port's convention for a captured closure with no
 * native C equivalent (see pico8_jp_ptrs_predicate_fn, pico8_collect.h). */
typedef p8_fixed_t (*weighted_picker_random_fn_t)(void *ctx);

/* The cart's only `gn(df,2)` makes flr(r*2) tickets per `df` entry: 66. */
enum { WEIGHTED_PICKER_TICKETS_CAP = 66 };

/* `oc` holds one ticket per weighted copy of an `hr` entry; `nm`/`nm_set`
 * are the 3-slot anti-repeat window. */
typedef struct {
    const planet_types_entry_t *tickets[WEIGHTED_PICKER_TICKETS_CAP];
    int tickets_count;
    const planet_types_entry_t *recent[3];
    int recent_set[3];
    weighted_picker_random_fn_t random;
    void *random_ctx;
} weighted_picker_t;

/* `hr` is `hr_count` entries whose `r` is the weight; the tickets point into
 * `hr`. The state is taken from `zone` and lives as long as it, nothing to
 * release; `hr` and `r_ctx` must live at least as long as the state.
 * pico8_gn_next hands back one of `hr`'s entries. */
weighted_picker_t *weighted_picker_new(const allocator_t *zone, const planet_types_entry_t *hr, int entries_count,
                           p8_fixed_t og, weighted_picker_random_fn_t r, void *r_ctx);
const planet_types_entry_t *weighted_picker_next(weighted_picker_t *state);

#endif
