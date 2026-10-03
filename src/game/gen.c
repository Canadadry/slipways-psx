#include "game/gen.h"

#include <assert.h>
#include <stddef.h>

#include "game/game_data.h"
#include "engine/allocator.h"

random_walk_t *random_walk_new(const allocator_t *zone, p8_fixed_t n_, p8_fixed_t b) {
    random_walk_t *state = (random_walk_t *)allocator_alloc(zone, sizeof *state);
    state->step = n_;
    state->bias = b;
    return state;
}

p8_fixed_t random_walk_next(random_walk_t *state) {
    p8_fixed_t one_minus_abs_b = p8_fixed_sub(p8_fixed_from_int(1), p8_fixed_abs(state->bias));
    p8_fixed_t b_pos = (state->bias > 0) ? state->bias : 0;
    p8_fixed_t v = p8_fixed_add(p8_fixed_rnd(one_minus_abs_b), b_pos);
    p8_fixed_t half = P8_FIXED_LIT(0x00008000, 0.5);

    state->bias = p8_fixed_add(state->bias, p8_fixed_mul(p8_fixed_sub(half, v), state->step));
    return v;
}

/* The scan stops at the first slot never written. Slots fill in the order
 * the shrinking ticket count picks: a pool whose size is not a multiple of
 * three fills a later slot first, which the early return keeps out of the
 * window until the slots before it are written. */
static int weighted_picker_recent_contains(const weighted_picker_t *state, const planet_types_entry_t *e) {
    int i;
    for (i = 0; i < 3; i++) {
        if (!state->recent_set[i]) {
            return 0;
        }
        if (state->recent[i] == e) {
            return 1;
        }
    }
    return 0;
}

/* Each entry gets flr(weight * multiplier) tickets, none when the product
 * is below 1. */
weighted_picker_t *weighted_picker_new(const allocator_t *zone, const planet_types_entry_t *hr, int entries_count,
                           p8_fixed_t og, weighted_picker_random_fn_t r, void *r_ctx) {
    weighted_picker_t *state = (weighted_picker_t *)allocator_alloc(zone, sizeof *state);
    int i;

    state->tickets_count = 0;
    for (i = 0; i < 3; i++) {
        state->recent[i] = NULL;
        state->recent_set[i] = 0;
    }
    state->random = r;
    state->random_ctx = r_ctx;

    for (i = 0; i < entries_count; i++) {
        const planet_types_entry_t *entity = &hr[i];
        p8_fixed_t count_fixed;
        int count, n;

        count_fixed = p8_fixed_flr(p8_fixed_mul(entity->weight, og));
        count = p8_fixed_flr_int(count_fixed);
        for (n = 1; n <= count; n++) {
            assert(state->tickets_count < WEIGHTED_PICKER_TICKETS_CAP && "weighted_picker_new: the ticket pool is sized for its one game call, "
                   "planet_types_table at a multiplier of 2; a larger table or multiplier needs a larger WEIGHTED_PICKER_TICKETS_CAP");
            state->tickets[state->tickets_count++] = entity;
        }
    }

    return state;
}

/* The first ticket equal to the picked entry goes and the later ones shift
 * down; the picked entry is always in the pool. */
static void weighted_picker_tickets_del(weighted_picker_t *state, const planet_types_entry_t *e) {
    int i = 0;

    while (state->tickets[i] != e) {
        i++;
    }
    state->tickets_count--;
    for (; i < state->tickets_count; i++) {
        state->tickets[i] = state->tickets[i + 1];
    }
}

/* Each draw removes its ticket for good: 66 tickets, while a 90-planet
 * galaxy can draw more, and an emptied pool would index out of range. */
/* GAP-16 */
const planet_types_entry_t *weighted_picker_next(weighted_picker_t *state) {
    const planet_types_entry_t *e;

    for (;;) {
        p8_fixed_t r_val = state->random(state->random_ctx);
        p8_fixed_t len_fixed = p8_fixed_from_int(state->tickets_count);
        p8_fixed_t idx_fixed =
            p8_fixed_flr(p8_fixed_add(p8_fixed_mul(r_val, len_fixed), p8_fixed_from_int(1)));
        int idx0 = p8_fixed_flr_int(idx_fixed) - 1;

        e = state->tickets[idx0];
        if (!weighted_picker_recent_contains(state, e)) {
            break;
        }
    }

    weighted_picker_tickets_del(state, e);
    {
        int slot = 3 - (state->tickets_count % 3);
        state->recent[slot - 1] = e;
        state->recent_set[slot - 1] = 1;
    }
    return e;
}
