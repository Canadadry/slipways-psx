/* See pico8_bootstrap.h's own top comment for the full jl()/dg() source
 * and trace -- both are ported here, not split, since they're this
 * chunk's own two functions and neither is large enough on its own to
 * warrant a separate module. */

#include "pico8_bootstrap.h"

#include <assert.h>

#include "pico8_entity.h"
#include "pico8_fixed.h"
#include "pico8_ke.h"
#include "pico8_list.h"
#include "pico8_mouse_reader.h"
#include "pico8_ob.h"
#include "pico8_scroll.h"
#include "pico8_ui.h"
#include "pico8_value.h"

/* Lua truthiness (nil/false falsy, everything else -- including 0 --
 * truthy): a private per-module copy, matching the same convention
 * already established independently by pico8_ui.c's own pico8_ui_truthy
 * (not shared/exported -- see that file's own reasoning, reused here
 * verbatim). Used by dg()'s own `if qc.p_ then he(qc) end`.
 *
 * This helper is deliberately NOT given the REF(NULL) clause that 19 of
 * this port's truthiness helpers carry: no caller was ever shown to reach
 * it with a Lua nil, and adding the clause speculatively would assert
 * fidelity nobody measured. The assert states that assumption instead of
 * trusting it. Should it ever fire, the two-clause form below would have
 * reported nil as TRUE -- exactly the defect fixed in pz()/lq:mn (see
 * docs/plan.md's journal entry on the built-planet sprite-0 block), so
 * aborting loudly is cheaper than drawing garbage: the raylib backend
 * records every session to events.json including the crashing frame, so a
 * fired assert hands back a replayable case.
 *
 * It is a live guard, not a dormant one: dg() runs it 2249 times over
 * events.json's 9729 frames (and 141-469 times on each scripted fixture),
 * every one of them non-nil. Its call site below -- `if
 * (pico8_instance_get_field(qc, "p_", &p_val) && pico8_dg_truthy(p_val))`
 * -- has no second guard rejecting REF(NULL), which is why this helper
 * needs the assert where pico8_particle_truthy does not (see that
 * helper's own comment in src/pico8_particle.c). */
static int pico8_dg_truthy(pico8_value_t v) {
    assert(!(v.kind == PICO8_VALUE_REF && v.as.ref == NULL));
    return !(v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0);
}

/* jl() (pico8/lua.lua:153-155) -- see pico8_bootstrap.h's own top comment
 * for the full trace and the exact reconciliation this performs. */
void pico8_jl(void) {
    pico8_instance_t *real_mo;
    pico8_instance_t *real_je;
    pico8_instance_t *real_ok;

    /* hv,cv,cg={},{},{} -- unchanged Phase 2 slice (also rebuilds ci and
     * the classless mo placeholder pico8_entity_set_mo() below replaces).
     * Unconditional on every call, matching real Lua's own jl() body. */
    pico8_entity_init();

    /* CHUNK 22 FIX (see this file's own header comment on pico8_jl() for
     * the full trace of the real dangling-.parent-pointer bug this
     * resolves): ke/qb/nf/ui are built ONCE (by pico8_cart_load(),
     * src/pico8_cart.c, in the normal case) -- only build them here if
     * they don't already exist, never unconditionally rebuild them.
     * ke must exist before qb/nf/ui's own pico8_class_declare(pico8_ke_
     * class(),...) calls, hence this order even in the defensive/no-prior-
     * pico8_cart_load() case (every existing caller that invokes
     * pico8_jl() directly, e.g. tests/test_pico8_planet_kj_dispatch.c,
     * still gets a freshly-built ke/qb/nf/ui on ITS OWN first call here,
     * exactly as before this fix). */
    if (pico8_ke_class() == NULL) {
        pico8_ke_init();
    }
    if (pico8_qb_class() == NULL) {
        pico8_qb_init();
    }
    if (pico8_nf_class() == NULL) {
        pico8_nf_init();
    }
    if (pico8_ui_class() == NULL) {
        pico8_ui_init();
    }

    /* mo,je,ok=qb(),nf(),ui() -- each pico8_*_new(NULL) also runs that
     * class's own pq chain (ke's inherited pq=lt, plus nf's own distinct
     * pq), so mo/je/ok self-register into hv/cv/cg the moment they're
     * built here, exactly like any other real ke-descended instance. */
    real_mo = pico8_qb_new(NULL);
    real_je = pico8_nf_new(NULL);
    real_ok = pico8_ui_new(NULL);

    /* Reconciling Phase 2's/Chunk 5's own placeholder accessors: same
     * public accessor (pico8_mo()/pico8_je()/pico8_ok()), real backing
     * instance now -- see each setter's own comment. */
    pico8_entity_set_mo(real_mo);
    pico8_ui_set_je(real_je);
    pico8_ui_set_ok(real_ok);
}

/* Not part of the Lua original -- see pico8_bootstrap.h's own comment. */
void pico8_bootstrap_shutdown(void) {
    pico8_ui_shutdown();
    pico8_nf_shutdown();
    pico8_qb_shutdown();
    pico8_ke_shutdown();
    pico8_entity_shutdown();
}

/* dg() (pico8/lua.lua:173-181, preceded by bm=ob([[...]]) at 169-172,
 * reproduced only because it precedes dg()) -- see pico8_bootstrap.h's
 * own top comment
 * for the full trace, including the deliberate `pairs()`-vs-`all()`
 * skip-one-on-self-deletion divergence this loop reproduces on purpose:
 * a plain index-based walk over pico8_hv()'s own live count/items, never
 * compensating for a mid-loop pico8_he() shift. */
void pico8_dg(void) {
    const pico8_list_t *hv = pico8_hv();
    int i;

    /* for _,qc in pairs(hv) do ... end */
    for (i = 0; i < hv->count; i++) {
        pico8_instance_t *qc = hv->items[i].as.ref;
        pico8_value_t me_val;
        pico8_value_t p_val;
        pico8_value_t t_val;

        /* local fn=qc[qc.me] if fn then fn(qc,qc.t) end -- routed through
         * pico8_mv(), same "resolve field then call with self+1 arg"
         * mechanism dc()'s own qc[nt](qc,qc.ra) already uses; a non-
         * function `qc.me` field (never evidenced) is a defensive no-crash
         * return-as-is rather than reproducing Lua's own call-error. */
        if (pico8_instance_get_field(qc, "me", &me_val) && me_val.kind == PICO8_VALUE_STRING) {
            pico8_value_t t_arg;
            pico8_value_t discard;
            if (!pico8_instance_get_field(qc, "t", &t_arg)) {
                t_arg = pico8_value_ref(NULL);
            }
            pico8_mv(qc, me_val.as.string, 1, t_arg, pico8_value_ref(NULL), &discard);
        }

        /* if qc.p_ then he(qc) end -- Lua truthiness, not presence. */
        if (pico8_instance_get_field(qc, "p_", &p_val) && pico8_dg_truthy(p_val)) {
            pico8_he(qc);
        }

        /* qc.t+=1 -- a field rebind, landing on qc's own instance table
         * regardless of where t's current value resolved from on the
         * class chain, matching this port's "field write always lands on
         * the instance" convention (e.g. pico8_scroll.c's self.p+=self.v).
         * qc.t's absence (never evidenced -- ke's own default sets t=0) is
         * a defensive no-op divergence, same category as elsewhere. */
        if (pico8_instance_get_field(qc, "t", &t_val) && t_val.kind == PICO8_VALUE_FIXED) {
            pico8_ob_table_set_name(qc->fields, "t",
                                     pico8_value_fixed(pico8_fixed_add(t_val.as.fixed, pico8_fixed_from_int(1))));
        }
    }
}
