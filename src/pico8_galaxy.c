/* See pico8_galaxy.h's own top comment for the full source/trace of `bw()`
 * ported here, including the rejection-loop/self-registration, discard-
 * without-freeing, `gn`-callsite-shape, `dk.en`-fallback and `r_ctx`-
 * ownership resolutions this file implements. */

#include "pico8_galaxy.h"

#include <stdlib.h>
#include <string.h>

#include "pico8_data_init.h"
#include "pico8_economy_score.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_gen.h"
#include "pico8_planet.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* See pico8_galaxy.h's own top comment ("`jo.oc,jo.ir=...`: CALLS THE
 * ALREADY-EXISTING SETTERS") for the full ownership trace this global
 * resolves: the `pico8_jf_state_t*` serving as `gn`'s own inner `r()`
 * random source, paired 1:1 with whichever `pico8_gn_state_t*` currently
 * lives in `jo.oc` (`pico8_jo_set_oc`, pico8_planet.h) -- rebuilt and the
 * previous one freed together, every time `pico8_bw()` runs. */
static pico8_jf_state_t *g_bw_oc_r_ctx = NULL;

/* Not a port of any named Lua function: this module's own private
 * two-piece string concat helper for `"p"..id` -- the same per-module-
 * duplicate shape already established by `pj_concat2`
 * (pico8_score_panel.c/pico8_score_panel_display.c) and `ie_concat2`
 * (pico8_research_icon.c, for `ie:ldown`'s own identically-shaped
 * `"s"..ie.id`). Returns a newly heap-allocated string the caller owns and
 * must free(). */
static char *pico8_bw_concat2(const char *a, const char *b) {
    size_t la = strlen(a);
    size_t lb = strlen(b);
    char *out = malloc(la + lb + 1);
    memcpy(out, a, la);
    memcpy(out + la, b, lb);
    out[la + lb] = '\0';
    return out;
}

/* Not a port of any named Lua function: private per-module bridge from
 * "positional entries of a pico8_ob_table_t" to a pico8_list_t (needed
 * here for `df`, `gn(df,...)`'s own first argument) -- the same
 * established pattern already independently duplicated well over a dozen
 * times in this port (e.g. `pico8_jo_positional_values`,
 * pico8_planet_display.c; `pico8_data_init_positional_values`,
 * pico8_data_init.c) -- this is this chunk's own Nth copy, documented as
 * such rather than shared, matching every prior copy's own stated
 * convention. */
static pico8_list_t pico8_bw_positional_values(const pico8_ob_table_t *table) {
    pico8_list_t list;
    int i;

    pico8_list_init(&list);
    if (table == NULL) {
        return list;
    }
    for (i = 0; i < table->count; i++) {
        if (table->entries[i].key.kind == PICO8_OB_KEY_INDEX) {
            pico8_list_add(&list, table->entries[i].value);
        }
    }
    return list;
}

/* Bridges `pico8_jf_next`'s own real signature (`pico8_fixed_t(*)
 * (pico8_jf_state_t*)`) to `pico8_gn_random_fn`'s generic ctx-based shape
 * (`pico8_fixed_t(*)(void*)`, pico8_gen.h) -- `jf(0.5,dk.en)`'s own return
 * value IS `gn`'s third argument `r` in the real source, a genuine
 * zero-argument Lua closure, not a direct `rnd`-style call (pico8_gen.h's
 * own doc comment on `pico8_gn_random_fn` uses "e.g. rnd" only as a
 * generic illustrative example, not a claim that this call site uses
 * `rnd` directly). No existing bridge in this port already adapts
 * `pico8_jf_next`'s own concrete pointer type this way (grepped for a
 * `static pico8_fixed_t X(void *ctx)` precedent -- none found; every
 * existing `_dispatch`-style bridge in this port instead adapts a
 * `pico8_value_function_t` call shape, a different need), so this is a new,
 * minimal, one-line adapter of the same general shape and spirit -- a
 * pointer cast on `ctx` back to its own real, known-at-this-call-site
 * type, never an unsafe direct function-pointer cast between
 * `pico8_gn_random_fn` and `pico8_jf_next`'s own incompatible C type. */
static pico8_fixed_t pico8_bw_gn_r(void *ctx) {
    return pico8_jf_next((pico8_jf_state_t *)ctx);
}

/* function bw(n)for id=1,n do
 * local p
 * repeat
 * p=v(rnd(2.75)+0.125,rnd(2.75)+0.125
 * )until ij(p,cg.jo)>0.05
 * jo({id="p"..id,p=p,ra=p*128})end
 * jo.oc,jo.ir=
 * gn(df,2,jf(0.5,dk.en)),jf(0.8,0.5)end
 * (pico8/lua.lua:744-751.) See pico8_galaxy.h's own top comment on this
 * declaration for the full trace of every representation decision below
 * (the rejection loop against `cg.jo`, the discard-without-freeing
 * `jo(...)` construction, the `gn`/`hr`-shape fix this call site justifies,
 * `dk.en`'s NULL-guarded fallback, and `jo.oc`/`jo.ir`'s own setters). */
void pico8_bw(pico8_fixed_t n) {
    pico8_fixed_t one = pico8_fixed_from_int(1);
    pico8_fixed_t threshold = pico8_fixed_from_double(0.05);
    pico8_fixed_t two_seventy_five = pico8_fixed_from_double(2.75);
    pico8_fixed_t eighth = pico8_fixed_from_double(0.125);
    pico8_fixed_t id;
    const pico8_ob_table_t *df;
    const pico8_ob_table_t *dk;
    pico8_fixed_t dk_en = pico8_fixed_from_int(0);
    pico8_value_t en_val;
    pico8_list_t hr_list;
    pico8_jf_state_t *new_oc_r_ctx;
    pico8_gn_state_t *oc_state;
    pico8_jf_state_t *ir_state;

    /* for id=1,n do ... end -- `id`, like every other Lua scalar in this
     * port, is a `pico8_fixed_t`; direct `<=`/`pico8_fixed_add` on it is
     * this port's own established convention for comparing/incrementing a
     * fixed-point loop counter (e.g. `nq<dw`/`nq>threshold` elsewhere in
     * this port), safe here since real call sites only ever pass whole
     * numbers (`bw(90)`). */
    for (id = one; id <= n; id = pico8_fixed_add(id, one)) {
        pico8_vec_t p_vec;
        pico8_vec_t *p_heap;
        pico8_vec_t *ra_heap;
        pico8_value_t nearest;
        char *id_num_str;
        char *id_str;
        pico8_ob_table_t *overrides;
        pico8_instance_t *made;

        /* local p repeat p=v(rnd(2.75)+0.125,rnd(2.75)+0.125)until
         * ij(p,cg.jo)>0.05 -- `nearest` (Lua's own discarded second `ij()`
         * return, `nd`) is never read here, matching the real `until`
         * clause's own single-value context. `pico8_cg("jo")` starts empty
         * on the very first trial ever (see pico8_galaxy.h's own top
         * comment) but genuinely grows every accepted iteration below, via
         * `jo`'s own `ke.pq=lt` self-registration -- not a permanently
         * inert rejection loop. */
        do {
            pico8_fixed_t rx = pico8_fixed_add(pico8_fixed_rnd(two_seventy_five), eighth);
            pico8_fixed_t ry = pico8_fixed_add(pico8_fixed_rnd(two_seventy_five), eighth);
            p_vec = pico8_vec_v(rx, ry);
        } while (pico8_ij(p_vec, pico8_cg("jo"), &nearest) <= threshold);

        /* `p` is retained, as-is, as the new instance's own `p` field
         * below (a genuine alias of this same heap vector, matching real
         * Lua's own "a field holds the same live table object"
         * semantics); `p*128` is a SEPARATE fresh heap vector (a brand new
         * value, matching real Lua's own table-constructor semantics for
         * a freshly computed expression), via `pico8_vec_mul`
         * (pico8_vec.h's own name for Lua's `__mul` metamethod). Neither
         * heap vector is freed here -- both are retained by the new
         * instance's own fields once constructed below. */
        p_heap = malloc(sizeof *p_heap);
        *p_heap = p_vec;
        ra_heap = malloc(sizeof *ra_heap);
        *ra_heap = pico8_vec_mul(p_vec, pico8_fixed_from_int(128));

        /* "p"..id -- `id` stringified via `pico8_value_tostr` (yields
         * "1","2",... for a whole-number fixed value, matching
         * `pico8_ie_ldown`'s own identical counter-to-string precedent),
         * then concatenated onto the literal "p" via this file's own
         * `pico8_bw_concat2`. `id_num_str` (the intermediate number-only
         * string) is freed right after use; `id_str` itself is NOT freed
         * -- it is retained, as-is, inside the newly constructed
         * instance's own "id" field below, matching `pico8_ie_ldown`'s own
         * established "a freshly built string is stored directly, not
         * copied again" convention. */
        id_num_str = pico8_value_tostr(pico8_value_fixed(id));
        id_str = pico8_bw_concat2("p", id_num_str);
        free(id_num_str);

        /* jo({id="p"..id,p=p,ra=p*128}) -- a fresh three-field overrides
         * table, then a direct `pico8_jo_new` call (`pico8_class_
         * instantiate` under it). */
        overrides = pico8_ob_table_new();
        pico8_ob_table_set_name(overrides, "id", pico8_value_string(id_str));
        pico8_ob_table_set_name(overrides, "p", pico8_value_ref(p_heap));
        pico8_ob_table_set_name(overrides, "ra", pico8_value_ref(ra_heap));

        /* Constructed for its own side effect (self-registration into
         * hv/cv/cg via `jo`'s inherited `ke.pq=lt`, see pico8_galaxy.h's
         * own top comment) and then discarded, matching real Lua's own
         * bare `jo({...})` statement (no `local x=...`) and this port's
         * own established "construct for side effect, discard the
         * pointer, intentionally leak the placed entity for the rest of
         * the program" precedent (`pj:ph`'s own two discarded `js(...)`
         * calls, pico8_score_panel_display.c). `overrides` itself IS freed
         * right after -- `pico8_class_instantiate` shallow-copies each of
         * its entries into the new instance's own fields first, so the
         * temporary table is safe to release here (same precedent as
         * `pico8_ie_ldown`'s own `overrides`). */
        made = pico8_jo_new(overrides);
        (void)made;
        pico8_ob_table_free(overrides);
    }

    /* jo.oc,jo.ir=gn(df,2,jf(0.5,dk.en)),jf(0.8,0.5) */
    df = pico8_df();

    /* `dk.en`: NULL-guarded, defaulting to 0 on a NULL `dk` or an
     * absent/wrong-kind "en" field -- see pico8_galaxy.h's own top comment
     * on this exact fallback and why 0 (not a guessed `bo` entry) is the
     * evidenced choice, matching `pico8_db`'s own identical `dk.trade`
     * fallback (pico8_economy_score.c). */
    dk = pico8_dk();
    if (dk != NULL && pico8_ob_table_get_name(dk, "en", &en_val) && en_val.kind == PICO8_VALUE_FIXED) {
        dk_en = en_val.as.fixed;
    }

    /* `gn(df,2,jf(0.5,dk.en))`: `df`'s own positional entries bridged to a
     * `pico8_list_t` (this file's own `pico8_bw_positional_values`), freed
     * right after the call -- `pico8_gn` copies each ticket's own value
     * into its own internal pool, never retaining a pointer into `hr`
     * itself past the call (confirmed by reading its own body), matching
     * every other `positional_values` caller's own identical "build,
     * pass, free" precedent. `og=2` -> `pico8_fixed_from_int(2)`.
     * `jf(0.5,dk.en)` is built as `new_oc_r_ctx`, this call's own fresh
     * `r_ctx`, and passed through `pico8_bw_gn_r` (this file's own
     * adapter, above) as `gn`'s own `r`. */
    hr_list = pico8_bw_positional_values(df);
    new_oc_r_ctx = pico8_jf(pico8_fixed_from_double(0.5), dk_en);
    oc_state = pico8_gn(&hr_list, pico8_fixed_from_int(2), pico8_bw_gn_r, new_oc_r_ctx);
    pico8_list_free(&hr_list);

    /* `r_ctx` OWNERSHIP RESOLUTION (see pico8_galaxy.h's own top comment
     * for the full trace): `pico8_gn_free` never frees its own `r_ctx`
     * (confirmed by reading its body, pico8_gen.c), so THIS module frees
     * the PREVIOUS `g_bw_oc_r_ctx` here, in lockstep with `pico8_jo_
     * set_oc` freeing the previous `pico8_gn_state_t*` it was paired
     * with, before both globals are overwritten with this call's own
     * fresh pair -- no leak, no double free, no dependency added to
     * `pico8_jo_set_oc`'s own existing contract. */
    if (g_bw_oc_r_ctx != NULL) {
        pico8_jf_free(g_bw_oc_r_ctx);
    }
    g_bw_oc_r_ctx = new_oc_r_ctx;
    pico8_jo_set_oc(oc_state);

    /* `jf(0.8,0.5)` -- built directly and handed to `pico8_jo_set_ir`,
     * which owns it outright (unlike `jo.oc`'s own inner `r_ctx`, `jo.ir`
     * itself IS the whole state `jf_next` is ever called on directly, no
     * paired ctx of its own to track). */
    ir_state = pico8_jf(pico8_fixed_from_double(0.8), pico8_fixed_from_double(0.5));
    pico8_jo_set_ir(ir_state);
}

/* See pico8_galaxy.h's own top comment on this declaration. */
void pico8_galaxy_shutdown(void) {
    if (g_bw_oc_r_ctx != NULL) {
        pico8_jf_free(g_bw_oc_r_ctx);
        g_bw_oc_r_ctx = NULL;
    }
}
