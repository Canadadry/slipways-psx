/* pico8/lua.lua's two closure-returning generator functions -- jf, gn --
 * see pico8_gen.h's own top comment for the shared "closure factory ->
 * heap state struct + a _next() function taking that state" convention
 * this file uses in place of a native C closure. */

#include "pico8_gen.h"

#include <stdlib.h>

#include "pico8_ob.h"

/* function jf(n_,b)
 * return function()local v=rnd(1-abs(b))+max(b,0)
 * b+=(0.5-v)*n_
 * return v
 * end
 * end
 * pico8_jf builds the closure's own captured state (n_, and b -- which the
 * closure body itself mutates on every call, an upvalue in the original);
 * pico8_jf_next is the closure body itself (below). */
pico8_jf_state_t *pico8_jf(pico8_fixed_t n_, pico8_fixed_t b) {
    pico8_jf_state_t *state = malloc(sizeof *state);
    state->n_ = n_;
    state->b = b;
    return state;
}

/* local v=rnd(1-abs(b))+max(b,0)
 * b+=(0.5-v)*n_
 * return v
 * See pico8_gen.h's own comment on this declaration. */
pico8_fixed_t pico8_jf_next(pico8_jf_state_t *state) {
    pico8_fixed_t one_minus_abs_b = pico8_fixed_sub(pico8_fixed_from_int(1), pico8_fixed_abs(state->b));
    pico8_fixed_t b_pos = (state->b > 0) ? state->b : 0;
    pico8_fixed_t v = pico8_fixed_add(pico8_fixed_rnd(one_minus_abs_b), b_pos);
    pico8_fixed_t half = pico8_fixed_from_double(0.5);

    state->b = pico8_fixed_add(state->b, pico8_fixed_mul(pico8_fixed_sub(half, v), state->n_));
    return v;
}

void pico8_jf_free(pico8_jf_state_t *state) {
    free(state);
}

/* `nm`'s own real Lua shape is a table written ONLY by direct index
 * assignment (`nm[3-#oc%3]=e`, never add()), so it is never actually
 * built as a pico8_list_t -- gu(nm,e)'s own real `all(nm)` walk is
 * PICO-8's documented "sequence" iteration: walk keys 1,2,3 in order,
 * STOPPING at the first one that was never assigned (a genuine hole
 * breaks the sequence, exactly like `all()`/`ipairs()` on a table with a
 * gap) -- ported here as a 3-slot array plus a parallel "was this slot
 * ever set" flag array, scanned with that same stop-at-first-gap rule,
 * rather than routed through pico8_gu/pico8_gu_content (which both
 * assume an add()-built, gap-free pico8_list_t). */
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

/* function gn(hr,og,r)local oc,nm={},{}
 * for e in all(hr)do
 * for n=1,e.r*og do
 * add(oc,e)
 * end
 * end
 * return function()local e
 * repeat
 * e=oc[flr(r()*#oc+1)]
 * until not gu(nm,e)del(oc,e)nm[3-#oc%3]=e
 * return e
 * end
 * end
 * pico8_gn builds the ticket pool `oc` (each entity `e` from `hr` appears
 * `flr(e.r*og)` times -- Lua's `for n=1,X do` iterates exactly floor(X)
 * times for X>=1, 0 times otherwise, matching pico8_fixed_flr here);
 * pico8_gn_next is the returned closure's own body (below). `r` (a
 * zero-argument Lua closure the real gn() call itself receives, e.g.
 * `rnd`) becomes a plain `pico8_fixed_t(*)(void*)` function pointer plus a
 * ctx, this codebase's own established convention for a captured Lua
 * closure with no native C equivalent.
 *
 * *** CHUNK 20 FIX -- `hr`-ENTRY SHAPE CORRECTED, `e.r` READ VIA
 * `pico8_ob_table_get_name`, NOT `pico8_instance_get_field` *** (Phase 3,
 * Chunk 1's own flagged, deferred gap -- see docs/plan.md's own "Notes de
 * portage" for the full trace): `gn(` has exactly ONE real call site in
 * the whole cart (confirmed by grepping `pico8/lua.lua` directly: line 732
 * is `gn`'s own definition, line 751 is its only call), `gn(df,2,jf(0.5,
 * dk.en))` inside `bw()` (Chunk 20, pico8_galaxy.c). `df` (Chunk 11's
 * `pico8_df()`, pico8_data_init.h) is a plain, POSITIONALLY-`ob()`-parsed
 * table whose own entries are themselves plain `pico8_ob_table_t*` records
 * (e.g. `e.s=160,e.r=4.5,e.d="earth-like"`, confirmed by Chunk 11's own
 * verification) -- `df` is never `pico8_class_instantiate`'d anywhere in
 * this port, so `hr`'s own list elements are never `pico8_instance_t*`
 * despite this function's own original (Chunk 1) assumption that they
 * were. Reading `e.r` via `pico8_instance_get_field` against a genuine
 * `pico8_ob_table_t*` reinterprets that struct's `{entries,count,
 * capacity}` layout as `pico8_instance_t`'s own `{fields,cls}` layout --
 * undefined behavior, not a simple "field absent" miss -- so every real
 * ticket in `oc` would previously have been silently dropped (`weight_val`
 * never resolving to a genuine `PICO8_VALUE_FIXED`) the moment this
 * function was ever exercised against its one real caller. Fixed by
 * reading `e.r` via `pico8_ob_table_get_name(entity, "r", &weight_val)`
 * against `e.as.ref` reinterpreted as `const pico8_ob_table_t*` instead --
 * the correct, and only evidenced, shape for `hr`'s own real elements. No
 * other line of this function changed. */
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
        count = (int)pico8_fixed_to_double(count_fixed);
        for (n = 1; n <= count; n++) {
            pico8_list_add(&state->oc, e);
        }
    }

    return state;
}

/* return function()local e
 * repeat
 * e=oc[flr(r()*#oc+1)]
 * until not gu(nm,e)del(oc,e)nm[3-#oc%3]=e
 * return e
 * end
 * See pico8_gen.h's own comment on this declaration and this file's own
 * pico8_gn_nm_contains comment for nm's own sequence-scan semantics. */
pico8_value_t pico8_gn_next(pico8_gn_state_t *state) {
    pico8_value_t e;

    for (;;) {
        pico8_fixed_t r_val = state->r(state->r_ctx);
        pico8_fixed_t len_fixed = pico8_fixed_from_int(state->oc.count);
        pico8_fixed_t idx_fixed =
            pico8_fixed_flr(pico8_fixed_add(pico8_fixed_mul(r_val, len_fixed), pico8_fixed_from_int(1)));
        int idx0 = (int)pico8_fixed_to_double(idx_fixed) - 1;

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
