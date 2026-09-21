#ifndef PICO8_GEN_H
#define PICO8_GEN_H

#include "pico8_class.h"
#include "pico8_fixed.h"
#include "pico8_list.h"
#include "pico8_value.h"

/* Phase 3, Chunk 1, second sub-lot: jf, gn -- pico8/lua.lua's own two
 * "returns a function" (closure-factory) helpers used as procedural
 * generators (a bounded random walk, and a weighted-with-anti-repeat
 * picker). Since C has no closures, each becomes a heap-allocated state
 * struct (the captured upvalues) plus a `_next()` function that IS the
 * returned closure's own body, called once per real Lua `f()` invocation
 * -- their own shared, nameable concern ("procedural generators"),
 * distinct from every other Chunk 1 helper's own module. */

/* function jf(n_,b)
 * return function()local v=rnd(1-abs(b))+max(b,0)
 * b+=(0.5-v)*n_
 * return v
 * end
 * end
 * A bounded random walk generator: pico8_jf builds it, pico8_jf_next
 * advances it by one step (see pico8_gen.c for the trace), pico8_jf_free
 * releases it. */
typedef struct {
    pico8_fixed_t n_;
    pico8_fixed_t b;
} pico8_jf_state_t;

pico8_jf_state_t *pico8_jf(pico8_fixed_t n_, pico8_fixed_t b);
pico8_fixed_t pico8_jf_next(pico8_jf_state_t *state);
void pico8_jf_free(pico8_jf_state_t *state);

/* `r()` (gn's own third parameter, a zero-argument Lua closure, e.g.
 * `rnd`): this codebase's established ctx-pointer convention for a
 * captured Lua closure with no native C equivalent (see
 * pico8_jp_predicate_fn, pico8_list.h). */
typedef pico8_fixed_t (*pico8_gn_random_fn)(void *ctx);

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
 * A weighted random picker with a 3-slot anti-repeat window: `oc` is a
 * ticket pool (each entity in `hr` appears `flr(e.r*og)` times, so pick
 * probability is proportional to its own `.r` weight field); `nm` tracks
 * the last 3 picks (see pico8_gen.c's own comment on its real "sequence,
 * stop at first gap" semantics) so a pick isn't immediately repeated.
 * pico8_gn builds the pool, pico8_gn_next is the returned closure's own
 * body (one pick per call, removing the picked ticket from the pool),
 * pico8_gn_free releases it. The returned entity is a PICO8_VALUE_REF
 * (whatever kind of value `hr`'s own list held) -- not decoded further
 * here.
 *
 * *** CHUNK 20 CORRECTION (was: "e.g. a pico8_instance_t* for gn(df,...),
 * not decoded further here") *** -- `gn(` has exactly one real call site
 * in the whole cart, `gn(df,2,jf(0.5,dk.en))` inside `bw()` (Chunk 20,
 * pico8_galaxy.c), and `hr`'s own real elements there (`df`'s own
 * positional entries, Chunk 11's `pico8_df()`) are plain
 * `pico8_ob_table_t*` records, NEVER `pico8_instance_t*` (`df` is never
 * `pico8_class_instantiate`'d anywhere in this port) -- the previous note
 * above was an unverified assumption made when this function was first
 * built (Chunk 1, before `gn`'s one real caller existed), corrected here
 * once Chunk 20 confirmed the real shape and fixed `pico8_gn`'s own `e.r`
 * read in pico8_gen.c accordingly (see that file's own comment on
 * `pico8_gn` for the full trace; logged in docs/plan.md's "Notes de
 * portage"). */
typedef struct {
    pico8_list_t oc;
    pico8_value_t nm[3];
    int nm_set[3];
    pico8_gn_random_fn r;
    void *r_ctx;
} pico8_gn_state_t;

pico8_gn_state_t *pico8_gn(const pico8_list_t *hr, pico8_fixed_t og, pico8_gn_random_fn r, void *r_ctx);
pico8_value_t pico8_gn_next(pico8_gn_state_t *state);
void pico8_gn_free(pico8_gn_state_t *state);

#endif
