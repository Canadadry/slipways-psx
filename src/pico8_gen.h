#ifndef PICO8_GEN_H
#define PICO8_GEN_H

#include "pico8_struct.h"
#include "pico8_fixed.h"
#include "pico8_list.h"
#include "pico8_value.h"

/* C has no closures, so each of these two closure factories becomes a heap
 * state struct (the captured upvalues) plus a `_next()` holding the body
 * the returned closure would have run, one call per Lua invocation. */

/* `b` is an upvalue the body itself rewrites on every step. */
typedef struct {
    pico8_fixed_t n_;
    pico8_fixed_t b;
} pico8_jf_state_t;

/* The state is the caller's, to release with pico8_jf_free. */
pico8_jf_state_t *pico8_jf(pico8_fixed_t n_, pico8_fixed_t b);
pico8_fixed_t pico8_jf_next(pico8_jf_state_t *state);
void pico8_jf_free(pico8_jf_state_t *state);

/* `gn`'s third argument is a zero-argument Lua closure: a function pointer
 * plus a ctx here, this port's convention for a captured closure with no
 * native C equivalent (see pico8_jp_predicate_fn, pico8_list.h). */
typedef pico8_fixed_t (*pico8_gn_random_fn)(void *ctx);

/* `oc` holds one ticket per weighted copy of an `hr` entry; `nm`/`nm_set`
 * are the 3-slot anti-repeat window. */
typedef struct {
    pico8_list_t oc;
    pico8_value_t nm[3];
    int nm_set[3];
    pico8_gn_random_fn r;
    void *r_ctx;
} pico8_gn_state_t;

/* `hr`'s entries must be PICO8_VALUE_REF-wrapped pico8_df_entry_t
 * (pico8_game_data.h), whose `r` is the weight. `hr` itself is copied out,
 * not retained. The state is the caller's, to
 * release with pico8_gn_free -- which frees the ticket pool but never
 * `r_ctx`, left to whoever built it. pico8_gn_next hands back `hr`'s own
 * value, undecoded. */
pico8_gn_state_t *pico8_gn(const pico8_list_t *hr, pico8_fixed_t og, pico8_gn_random_fn r, void *r_ctx);
pico8_value_t pico8_gn_next(pico8_gn_state_t *state);
void pico8_gn_free(pico8_gn_state_t *state);

#endif
