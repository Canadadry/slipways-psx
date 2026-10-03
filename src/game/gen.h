#ifndef GAME_GEN_H
#define GAME_GEN_H

#include "p8/fixed.h"
#include "game/game_data.h"
#include "engine/allocator.h"

/* Two generators, each a state struct taken from the caller's zone and a
 * `_next()` that draws one value per call. */

/* random_walk_next() rewrites `bias` on every draw. */
typedef struct {
    p8_fixed_t step;
    p8_fixed_t bias;
} random_walk_t;

/* The state is taken from `zone` and lives as long as it: nothing to
 * release. */
random_walk_t *random_walk_new(const allocator_t *zone, p8_fixed_t n_, p8_fixed_t b);
p8_fixed_t random_walk_next(random_walk_t *state);

/* The random source of weighted_picker_new(): a function and the context it
 * is called with, the convention of filter_pointer_list_predicate_fn_t
 * (engine/collect.h). */
typedef p8_fixed_t (*weighted_picker_random_fn_t)(void *ctx);

/* The one game call, planet_types_table at a multiplier of 2, makes
 * flr(weight * 2) tickets per entry: 66. */
enum { WEIGHTED_PICKER_TICKETS_CAP = 66 };

/* `tickets` holds one ticket per weighted copy of an entry;
 * `recent`/`recent_set` are the 3-slot anti-repeat window. */
typedef struct {
    const planet_types_entry_t *tickets[WEIGHTED_PICKER_TICKETS_CAP];
    int tickets_count;
    const planet_types_entry_t *recent[3];
    int recent_set[3];
    weighted_picker_random_fn_t random;
    void *random_ctx;
} weighted_picker_t;

/* The table holds `entries_count` entries weighted by their `weight`; the
 * tickets point into it. The state is taken from `zone` and lives as long as
 * it, nothing to release; the table and the random context must live at
 * least as long as the state. weighted_picker_next() hands back one of the
 * table's entries. */
weighted_picker_t *weighted_picker_new(const allocator_t *zone, const planet_types_entry_t *hr, int entries_count,
                           p8_fixed_t og, weighted_picker_random_fn_t r, void *r_ctx);
const planet_types_entry_t *weighted_picker_next(weighted_picker_t *state);

#endif
