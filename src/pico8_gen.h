#ifndef PICO8_GEN_H
#define PICO8_GEN_H

#include "pico8_fixed.h"
#include "pico8_game_data.h"
#include "pico8_zone_choisie.h"

/* C has no closures, so each of these two closure factories becomes a state
 * struct (the captured upvalues) taken from the caller's zone, plus a
 * `_next()` holding the body the returned closure would have run, one call
 * per Lua invocation. */

/* `b` is an upvalue the body itself rewrites on every step. */
typedef struct {
    pico8_fixed_t n_;
    pico8_fixed_t b;
} pico8_jf_state_t;

/* The state is taken from `zone` and lives as long as it: nothing to
 * release. */
pico8_jf_state_t *pico8_jf(const pico8_zone_choisie_t *zone, pico8_fixed_t n_, pico8_fixed_t b);
pico8_fixed_t pico8_jf_next(pico8_jf_state_t *state);

/* `gn`'s third argument is a zero-argument Lua closure: a function pointer
 * plus a ctx here, this port's convention for a captured closure with no
 * native C equivalent (see pico8_jp_ptrs_predicate_fn, pico8_collect.h). */
typedef pico8_fixed_t (*pico8_gn_random_fn)(void *ctx);

/* The cart's only `gn(df,2)` makes flr(r*2) tickets per `df` entry: 66. */
enum { PICO8_GN_OC_CAP = 66 };

/* `oc` holds one ticket per weighted copy of an `hr` entry; `nm`/`nm_set`
 * are the 3-slot anti-repeat window. */
typedef struct {
    const pico8_df_entry_t *oc[PICO8_GN_OC_CAP];
    int oc_count;
    const pico8_df_entry_t *nm[3];
    int nm_set[3];
    pico8_gn_random_fn r;
    void *r_ctx;
} pico8_gn_state_t;

/* `hr` is `hr_count` entries whose `r` is the weight; the tickets point into
 * `hr`. The state is taken from `zone` and lives as long as it, nothing to
 * release; `hr` and `r_ctx` must live at least as long as the state.
 * pico8_gn_next hands back one of `hr`'s entries. */
pico8_gn_state_t *pico8_gn(const pico8_zone_choisie_t *zone, const pico8_df_entry_t *hr, int hr_count,
                           pico8_fixed_t og, pico8_gn_random_fn r, void *r_ctx);
const pico8_df_entry_t *pico8_gn_next(pico8_gn_state_t *state);

#endif
