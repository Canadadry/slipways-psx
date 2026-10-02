#include "game/gen.h"

#include <assert.h>
#include <stddef.h>

#include "game/game_data.h"
#include "engine/allocator.h"

pico8_jf_state_t *pico8_jf(const allocator_t *zone, p8_fixed_t n_, p8_fixed_t b) {
    pico8_jf_state_t *state = (pico8_jf_state_t *)allocator_alloc(zone, sizeof *state);
    state->n_ = n_;
    state->b = b;
    return state;
}

p8_fixed_t pico8_jf_next(pico8_jf_state_t *state) {
    p8_fixed_t one_minus_abs_b = p8_fixed_sub(p8_fixed_from_int(1), p8_fixed_abs(state->b));
    p8_fixed_t b_pos = (state->b > 0) ? state->b : 0;
    p8_fixed_t v = p8_fixed_add(p8_fixed_rnd(one_minus_abs_b), b_pos);
    p8_fixed_t half = P8_FIXED_LIT(0x00008000, 0.5);

    state->b = p8_fixed_add(state->b, p8_fixed_mul(p8_fixed_sub(half, v), state->n_));
    return v;
}

/* `nm` is written by bare index assignment, so `all(nm)` is PICO-8
 * sequence iteration: it stops at the first slot never assigned. Hence the
 * early return on an unset slot -- dropping it would widen the anti-repeat
 * window past what the cart does. */
static int pico8_gn_nm_contains(const pico8_gn_state_t *state, const pico8_df_entry_t *e) {
    int i;
    for (i = 0; i < 3; i++) {
        if (!state->nm_set[i]) {
            return 0;
        }
        if (state->nm[i] == e) {
            return 1;
        }
    }
    return 0;
}

/* `for n=1,e.r*og`: Lua runs the body floor(X) times for X>=1 and never
 * for X<1, which is what pico8_fixed_flr reproduces here. */
pico8_gn_state_t *pico8_gn(const allocator_t *zone, const pico8_df_entry_t *hr, int hr_count,
                           p8_fixed_t og, pico8_gn_random_fn r, void *r_ctx) {
    pico8_gn_state_t *state = (pico8_gn_state_t *)allocator_alloc(zone, sizeof *state);
    int i;

    state->oc_count = 0;
    for (i = 0; i < 3; i++) {
        state->nm[i] = NULL;
        state->nm_set[i] = 0;
    }
    state->r = r;
    state->r_ctx = r_ctx;

    for (i = 0; i < hr_count; i++) {
        const pico8_df_entry_t *entity = &hr[i];
        p8_fixed_t count_fixed;
        int count, n;

        count_fixed = p8_fixed_flr(p8_fixed_mul(entity->r, og));
        count = p8_fixed_flr_int(count_fixed);
        for (n = 1; n <= count; n++) {
            assert(state->oc_count < PICO8_GN_OC_CAP &&
                   "gn: the pool is bounded by the cart's only gn(df,2): a larger df or og needs a larger array");
            state->oc[state->oc_count++] = entity;
        }
    }

    return state;
}

/* del(oc,e): the first entry identical to `e` goes, the later ones shift
 * down. `e` is always in `oc`. */
static void pico8_gn_oc_del(pico8_gn_state_t *state, const pico8_df_entry_t *e) {
    int i = 0;

    while (state->oc[i] != e) {
        i++;
    }
    state->oc_count--;
    for (; i < state->oc_count; i++) {
        state->oc[i] = state->oc[i + 1];
    }
}

/* `del` drops the picked ticket for good, so `oc` only ever shrinks. `df`
 * at `og=2` yields 66 tickets while `bw(90)` can reveal 90 planets; an
 * emptied pool would index out of range just below. */
const pico8_df_entry_t *pico8_gn_next(pico8_gn_state_t *state) {
    const pico8_df_entry_t *e;

    for (;;) {
        p8_fixed_t r_val = state->r(state->r_ctx);
        p8_fixed_t len_fixed = p8_fixed_from_int(state->oc_count);
        p8_fixed_t idx_fixed =
            p8_fixed_flr(p8_fixed_add(p8_fixed_mul(r_val, len_fixed), p8_fixed_from_int(1)));
        int idx0 = p8_fixed_flr_int(idx_fixed) - 1;

        e = state->oc[idx0];
        if (!pico8_gn_nm_contains(state, e)) {
            break;
        }
    }

    pico8_gn_oc_del(state, e);
    {
        int slot = 3 - (state->oc_count % 3);
        state->nm[slot - 1] = e;
        state->nm_set[slot - 1] = 1;
    }
    return e;
}
