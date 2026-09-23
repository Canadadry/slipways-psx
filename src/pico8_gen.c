#include "pico8_gen.h"

#include <stdlib.h>

#include "pico8_ob.h"

pico8_jf_state_t *pico8_jf(pico8_fixed_t n_, pico8_fixed_t b) {
    pico8_jf_state_t *state = malloc(sizeof *state);
    state->n_ = n_;
    state->b = b;
    return state;
}

pico8_fixed_t pico8_jf_next(pico8_jf_state_t *state) {
    pico8_fixed_t one_minus_abs_b = pico8_fixed_sub(pico8_fixed_from_int(1), pico8_fixed_abs(state->b));
    pico8_fixed_t b_pos = (state->b > 0) ? state->b : 0;
    pico8_fixed_t v = pico8_fixed_add(pico8_fixed_rnd(one_minus_abs_b), b_pos);
    pico8_fixed_t half = PICO8_FIXED_LIT(0x00008000, 0.5);

    state->b = pico8_fixed_add(state->b, pico8_fixed_mul(pico8_fixed_sub(half, v), state->n_));
    return v;
}

void pico8_jf_free(pico8_jf_state_t *state) {
    free(state);
}

/* `nm` is written by bare index assignment, so `all(nm)` is PICO-8
 * sequence iteration: it stops at the first slot never assigned. Hence the
 * early return on an unset slot -- dropping it would widen the anti-repeat
 * window past what the cart does. */
static int pico8_gn_nm_contains(const pico8_gn_state_t *state, pico8_value_t e) {
    int i;
    for (i = 0; i < 3; i++) {
        if (!state->nm_set[i]) {
            return 0;
        }
        if (pico8_value_equal(state->nm[i], e)) {
            return 1;
        }
    }
    return 0;
}

/* `for n=1,e.r*og`: Lua runs the body floor(X) times for X>=1 and never
 * for X<1, which is what pico8_fixed_flr reproduces here. */
pico8_gn_state_t *pico8_gn(const pico8_list_t *hr, pico8_fixed_t og, pico8_gn_random_fn r, void *r_ctx) {
    pico8_gn_state_t *state = malloc(sizeof *state);
    pico8_list_iter_t it;
    pico8_value_t e;
    int i;

    pico8_list_init(&state->oc);
    for (i = 0; i < 3; i++) {
        state->nm[i] = pico8_value_ref(NULL);
        state->nm_set[i] = 0;
    }
    state->r = r;
    state->r_ctx = r_ctx;

    it = pico8_list_all(hr);
    while (pico8_list_iter_next(&it, &e)) {
        const pico8_ob_table_t *entity = (const pico8_ob_table_t *)e.as.ref;
        pico8_value_t weight_val;
        pico8_fixed_t count_fixed;
        int count, n;

        if (!pico8_ob_table_get_name(entity, "r", &weight_val) || weight_val.kind != PICO8_VALUE_FIXED) {
            continue;
        }
        count_fixed = pico8_fixed_flr(pico8_fixed_mul(weight_val.as.fixed, og));
        count = pico8_fixed_flr_int(count_fixed);
        for (n = 1; n <= count; n++) {
            pico8_list_add(&state->oc, e);
        }
    }

    return state;
}

/* `del` drops the picked ticket for good, so `oc` only ever shrinks. `df`
 * at `og=2` yields 66 tickets while `bw(90)` can reveal 90 planets; an
 * emptied pool would index out of range just below. */
pico8_value_t pico8_gn_next(pico8_gn_state_t *state) {
    pico8_value_t e;

    for (;;) {
        pico8_fixed_t r_val = state->r(state->r_ctx);
        pico8_fixed_t len_fixed = pico8_fixed_from_int(state->oc.count);
        pico8_fixed_t idx_fixed =
            pico8_fixed_flr(pico8_fixed_add(pico8_fixed_mul(r_val, len_fixed), pico8_fixed_from_int(1)));
        int idx0 = pico8_fixed_flr_int(idx_fixed) - 1;

        e = state->oc.items[idx0];
        if (!pico8_gn_nm_contains(state, e)) {
            break;
        }
    }

    pico8_list_del(&state->oc, e);
    {
        int slot = 3 - (state->oc.count % 3);
        state->nm[slot - 1] = e;
        state->nm_set[slot - 1] = 1;
    }
    return e;
}

void pico8_gn_free(pico8_gn_state_t *state) {
    if (state == NULL) {
        return;
    }
    pico8_list_free(&state->oc);
    free(state);
}
