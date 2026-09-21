#include "pico8_ui.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "pico8_collect.h"
#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_ke.h"
#include "pico8_list.h"
#include "pico8_pi.h"
#include "pico8_vec.h"

/* See pico8_ui.h's own top comment for the full source/trace, the `le`
 * representation decision, and the `je` placeholder's own rationale. */

static pico8_class_t *g_ui_class = NULL;
static pico8_instance_t *g_je = NULL;
static pico8_instance_t *g_ok = NULL;

/* Per-module defensive field readers, matching the same "read a field
 * expected to hold kind K, absent/wrong-kind/NULL-instance all read as a
 * documented default" convention already established by
 * pico8_entity_geom.c/pico8_hud.c/pico8_scroll.c/pico8_ui_button.c -- a
 * private per-module copy each time, not shared/exported, same reasoning
 * those files already give. */
static pico8_vec_t pico8_ui_field_vec(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static pico8_value_t pico8_ui_field_raw(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v)) {
        return v;
    }
    return pico8_value_ref(NULL);
}

/* Lua truthiness (nil/false are falsy, everything else -- including 0 --
 * is truthy).
 *
 * PHASE 4 BUG FIX (found by this phase's own first real full-game
 * playability exercise, tests/test_pico8_playability.c -- see
 * docs/portage-progress.md): this function's own PREVIOUS body only
 * caught `PICO8_VALUE_BOOL(false)` as falsy, leaving `pico8_value_ref
 * (NULL)` -- this port's own established, pervasive stand-in for real Lua
 * `nil` (pico8_value.h's own top comment; already correctly handled by
 * e.g. pico8_probe_truthy, pico8_probe.c) -- reading as TRUTHY here. That
 * was flat wrong for this file's own real, evidenced call sites: contrary
 * to this comment's OLD claim that "every real call site... always checks
 * REF-and-non-NULL separately... before falling back to this generic
 * check", none of the three real call sites below (`rheld`, `hd`'s own
 * `hit_out`, `jz`'s own `kn`) do any such separate check first. This
 * concretely broke `pico8_ui_jz`'s own click-dispatch precedence: a real
 * `jo:ldown()` (`pico8_jo_ldown`, pico8_planet.c) returns
 * `pico8_value_ref(NULL)` (Lua nil) for an "unknown" (unrevealed) planet
 * -- clicking one should fall through `jz`'s own layer walk to a LOWER-
 * priority entity (e.g. `bg:ldown`, the background/probe-launch handler),
 * exactly like real Lua's `if kn then return {...} end` skipping a nil
 * `kn` -- but the old, incomplete truthy check here treated that nil as
 * truthy, so `jz` incorrectly stopped at the planet and returned a REF
 * wrapping a NULL pointer, silently swallowing every click landing near
 * an unrevealed planet and permanently blocking `bg:ldown` (and therefore
 * ever creating a `mz` wormhole once a `g_` probe already exists) for
 * that click. Fixed to match `pico8_probe_truthy`'s own already-correct
 * "REF with a NULL pointer is falsy too" rule -- the direct C
 * consequence of choosing REF(NULL) as this port's own nil
 * representation in the first place, not a new convention. */
static int pico8_ui_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

static int pico8_ui_to_int(pico8_fixed_t x) {
    return (int)pico8_fixed_to_double(x);
}

/* je[bt] (bt "l" or "r"): reads the nested button-state table bu()
 * (pico8_mouse.h) would normally populate -- absent on this chunk's own
 * bare `je` placeholder, so every read through this helper defensively
 * returns NULL/false, matching this file's own top-comment rationale. */
static const pico8_ob_table_t *pico8_ui_je_button_table(const char *bt) {
    pico8_value_t b_val;
    if (pico8_instance_get_field(g_je, bt, &b_val) && b_val.kind == PICO8_VALUE_REF && b_val.as.ref != NULL) {
        return (const pico8_ob_table_t *)b_val.as.ref;
    }
    return NULL;
}

static int pico8_ui_je_button_field_bool(const char *bt, const char *field) {
    const pico8_ob_table_t *b = pico8_ui_je_button_table(bt);
    pico8_value_t v;
    if (b != NULL && pico8_ob_table_get_name(b, field, &v) && v.kind == PICO8_VALUE_BOOL) {
        return v.as.boolean;
    }
    return 0;
}

/* Bridges pico8_value_function_t's (self,argc,arg0,arg1) shape to
 * pico8_ui_ev's own signature; dc()'s own "ev" dispatch passes argc=1
 * with `qc.ra` as arg0 (ui:ev's real body never references it, so unused
 * here too, exactly like js:ev), reading pico8_dispatch_context.h's
 * shared gfx/mem for spr()'s own real drawing need. */
static pico8_value_t pico8_ui_ev_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_ui_ev(pico8_dispatch_gfx(), pico8_dispatch_mem(), (const pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

/* ui=ke:jr([[
 * ql=58,fa=15,le=o(),hw=b(10,10,118,118),
 * ]]) -- see pico8_ui.h's own top comment for the full trace. Calls
 * pico8_pi_init() defensively (idempotent) before declaring the class:
 * the spec string's own `b(...)` needs Chunk 1/2's 'b' char-keyed pi
 * registration to already exist, same real ordering hazard as
 * pico8_nf_init()/pico8_qb_init()'s own `v(...)` fields. */
void pico8_ui_init(void) {
    pico8_ob_table_t *raw_le = NULL;
    pico8_value_t le_val;
    pico8_list_t *heap_le;

    pico8_ui_shutdown();
    pico8_pi_init();

    g_ui_class = pico8_class_declare(pico8_ke_class(), "\nql=58,fa=15,le=o(),hw=b(10,10,118,118),\n");

    /* le=o(): ob()'s own generic "identifier(...)" fallback parses an
     * empty, untyped table here (matching every other unregistered-
     * identifier default in this port), but this class's own real
     * consumers (gu()/nv()) need a pico8_list_t*, not a raw
     * pico8_ob_table_t* -- see this file's own top comment (via
     * pico8_ui.h) for the full rationale. Overridden here, right after
     * declaring, with a properly-typed empty heap pico8_list_t* REF; the
     * raw table ob() actually built for "le" is freed immediately since
     * nothing else ever sees it. */
    if (pico8_ob_table_get_name(g_ui_class->defaults, "le", &le_val) && le_val.kind == PICO8_VALUE_REF) {
        raw_le = (pico8_ob_table_t *)le_val.as.ref;
    }
    heap_le = malloc(sizeof *heap_le);
    pico8_list_init(heap_le);
    pico8_ob_table_set_name(g_ui_class->defaults, "le", pico8_value_ref(heap_le));
    pico8_ob_table_free(raw_le);

    /* `ev` IS one of bm's 5 dispatch names -- wired as a real
     * PICO8_VALUE_FUNCTION class-default field, exactly like js:ev/js:kj
     * (pico8_ui_button.c). `lj`/`gw`/`bh`/`jz`/`cj` are NOT bm names and
     * are never routed through mv()/dc() -- see pico8_ui.h's own top
     * comment -- so they need no such wiring. */
    pico8_ob_table_set_name(g_ui_class->defaults, "ev", pico8_value_function(pico8_ui_ev_dispatch));

    /* je -- Chunk 5's own placeholder for the real `je=nf()` mouse-reader
     * singleton, mirroring pico8_mo()'s exact shape (pico8_entity.c): a
     * single, bare, classless instance with no fields set. See pico8_ui.h's
     * own top comment for the full rationale and the note for Chunk 6. */
    g_je = malloc(sizeof *g_je);
    g_je->fields = pico8_ob_table_new();
    g_je->cls = NULL;
}

void pico8_ui_shutdown(void) {
    if (g_ui_class != NULL) {
        pico8_class_free(g_ui_class);
        g_ui_class = NULL;
    }
    if (g_je != NULL) {
        pico8_instance_free(g_je);
        g_je = NULL;
    }
    if (g_ok != NULL) {
        pico8_instance_free(g_ok);
        g_ok = NULL;
    }
}

pico8_class_t *pico8_ui_class(void) {
    return g_ui_class;
}

pico8_instance_t *pico8_ui_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_ui_class, overrides);
}

pico8_instance_t *pico8_je(void) {
    return g_je;
}

/* See pico8_ui.h's own comment on this declaration.
 *
 * REAL BUG FOUND AND FIXED (Phase 5, raylib interactive backend): this
 * used to unconditionally pico8_instance_free() the outgoing g_je before
 * replacing it, matching the shape of every other _set_ singleton setter
 * in this port. But je is one of the three singletons (mo/je/ok) pico8_jl
 * rebuilds UNCONDITIONALLY on every call (see pico8_bootstrap.c), and
 * pico8_jl() itself runs a second time from inside pico8_eu() -- which is
 * reachable not just from main.c's own direct boot-time call, but from a
 * REAL mouse click on a difficulty button, dispatched through pico8_ui_lj
 * (self=the pre-rebuild ok) -> ... -> mv(button,"my") -> pico8_eu(). At
 * that point pico8_ui_lj's own call frame is still using its `self`
 * (the OLD ok) after the callback returns (pico8_ui_lj's own final
 * self->fields write) -- exactly the moment the OLD je/ok would already
 * have been freed here, a confirmed ASan heap-use-after-free reproduced
 * via a real simulated click (never hit by any existing test/replay,
 * since main.c/test_pico8_playability all reach eu() via a direct call
 * BEFORE the frame loop starts, never through a live click mid-dispatch).
 * Real Lua has no equivalent hazard: reassigning the global `je`/`ok`
 * never invalidates whatever a live call's own `self` local still
 * references -- the old table simply becomes garbage, reclaimed by the
 * GC only once nothing points to it anymore, never while a frame like
 * pico8_ui_lj's is still running on it. The C-faithful fix is therefore
 * to stop freeing the outgoing instance here and let it leak instead,
 * matching real Lua's own deferred-reclaim semantics -- and matching
 * this project's own already-accepted "no full teardown of long-lived
 * singletons" convention (see e.g. the fv/particle instances pico8_jl()
 * itself never frees either). Zero Lua-observable behavior change: only
 * C memory-reclamation timing differs. */
void pico8_ui_set_je(pico8_instance_t *instance) {
    if (instance == g_je) {
        return;
    }
    g_je = instance;
}

pico8_instance_t *pico8_ok(void) {
    return g_ok;
}

/* See pico8_ui.h's own comment on this declaration, and pico8_ui_set_je's
 * own comment just above for the full real-bug trace this shares (ok is
 * the specific singleton the confirmed use-after-free was reproduced
 * against). */
void pico8_ui_set_ok(pico8_instance_t *instance) {
    if (instance == g_ok) {
        return;
    }
    g_ok = instance;
}

/* function ui:lj()local le=self.le
 * self.qa=je.ra+mo.ra
 * self:gw()if #le==0 or gu(le,self.o_)then
 * self:bh("l")end
 * self:bh("r")if je.l.jn and self.le==le then
 * self:cj()end
 * self.gj=self.o_
 * end
 * -- see pico8_ui.h's own comment on this declaration for the full trace.
 * `local le=self.le` is captured BEFORE gw()/bh() run so `self.le==le`
 * (an identity/pointer comparison, not a value comparison) below still
 * refers to the list this call started with. `self.qa` is a field REBIND
 * (fresh heap vector), matching every other "field write always lands on
 * the instance" case already established. */
void pico8_ui_lj(pico8_mem_t *mem, pico8_instance_t *self) {
    pico8_value_t le_val;
    void *le_ptr_before = NULL;
    int le_count;
    pico8_vec_t qa;
    pico8_vec_t *heap_qa;
    int hit;

    if (self == NULL) {
        return;
    }

    if (pico8_instance_get_field(self, "le", &le_val) && le_val.kind == PICO8_VALUE_REF && le_val.as.ref != NULL) {
        le_ptr_before = le_val.as.ref;
        le_count = ((const pico8_list_t *)le_val.as.ref)->count;
    } else {
        le_count = 0;
    }

    qa = pico8_vec_add(pico8_ui_field_vec(g_je, "ra"), pico8_ui_field_vec(pico8_mo(), "ra"));
    heap_qa = malloc(sizeof *heap_qa);
    *heap_qa = qa;
    pico8_ob_table_set_name(self->fields, "qa", pico8_value_ref(heap_qa));

    pico8_ui_gw(mem, self);

    hit = (le_count == 0) ||
          pico8_gu((const pico8_list_t *)le_ptr_before, pico8_ui_field_raw(self, "o_"));
    if (hit) {
        pico8_ui_bh(self, "l");
    }

    pico8_ui_bh(self, "r");

    if (pico8_ui_je_button_field_bool("l", "jn")) {
        pico8_value_t le_val2;
        int same = pico8_instance_get_field(self, "le", &le_val2) && le_val2.kind == PICO8_VALUE_REF &&
                    le_val2.as.ref == le_ptr_before;
        if (same) {
            pico8_ui_cj(self);
        }
    }

    pico8_ob_table_set_name(self->fields, "gj", pico8_ui_field_raw(self, "o_"));
}

/* function ui:ev()spr(self.rheld and 9 or self.o_ and self.o_.ql or self.ql,je.ra.x,je.ra.y)end
 * -- see pico8_ui.h's own comment on this declaration for the full trace.
 * `self.rheld and 9 or self.o_ and self.o_.ql or self.ql`: Lua's
 * and/or idiom, `(self.rheld and 9) or (self.o_ and self.o_.ql) or
 * self.ql` -- rheld truthy picks 9; else, if self.o_ is present AND its
 * own "ql" is itself truthy, that; else self's own "ql" (58, per the
 * class spec). */
void pico8_ui_ev(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self) {
    pico8_vec_t je_ra;
    int spr_id = 0;
    pico8_value_t o_val;

    if (self == NULL || gfx == NULL || mem == NULL) {
        return;
    }

    if (pico8_ui_truthy(pico8_ui_field_raw(self, "rheld"))) {
        spr_id = 9;
    } else {
        o_val = pico8_ui_field_raw(self, "o_");
        if (o_val.kind == PICO8_VALUE_REF && o_val.as.ref != NULL) {
            pico8_value_t ql_val = pico8_ui_field_raw((const pico8_instance_t *)o_val.as.ref, "ql");
            if (ql_val.kind != PICO8_VALUE_FIXED) {
                ql_val = pico8_ui_field_raw(self, "ql");
            }
            spr_id = (ql_val.kind == PICO8_VALUE_FIXED) ? pico8_ui_to_int(ql_val.as.fixed) : 0;
        } else {
            pico8_value_t ql_val = pico8_ui_field_raw(self, "ql");
            spr_id = (ql_val.kind == PICO8_VALUE_FIXED) ? pico8_ui_to_int(ql_val.as.fixed) : 0;
        }
    }

    je_ra = pico8_ui_field_vec(g_je, "ra");
    pico8_spr_default(gfx, mem, spr_id, pico8_ui_to_int(je_ra.x), pico8_ui_to_int(je_ra.y));
}

/* function ui:gw()self.ic,self.o_={}
 * for e in all(cv.hd)do
 * if mv(e,"hd",self.qa,je.ra)then
 * ft(self.ic,e.fa,e)self.o_=e
 * end
 * end
 * (self.gj or {}).m_=false
 * (self.o_ or {}).m_=true
 * poke(0x5f80,self.o_
 * and self.o_.fg
 * or max(peek(0x5f80)-1,0))end
 * -- see pico8_ui.h's own comment on this declaration for the full trace
 * and the arity-extension rationale. `self.ic,self.o_={}`: Lua multiple
 * assignment with one RHS value -- `self.ic` gets the fresh table,
 * `self.o_` gets nil (ported as pico8_value_ref(NULL), this port's own
 * "no real Lua value" filler). `(self.gj or {}).m_=false`/`(self.o_ or
 * {}).m_=true`: writing onto a throwaway `{}` when the LHS is absent has
 * no observable effect in real Lua, so this port simply skips the write
 * when absent rather than allocating a table nothing will ever read. */
void pico8_ui_gw(pico8_mem_t *mem, pico8_instance_t *self) {
    pico8_ob_table_t *ic;
    pico8_value_t qa_val;
    pico8_value_t je_ra_val;
    const pico8_list_t *hd_bucket;
    pico8_value_t gj_val;
    pico8_value_t o_val;
    int i;
    int poke_val;

    if (self == NULL || mem == NULL) {
        return;
    }

    ic = pico8_ob_table_new();
    pico8_ob_table_set_name(self->fields, "ic", pico8_value_ref(ic));
    pico8_ob_table_set_name(self->fields, "o_", pico8_value_ref(NULL));

    qa_val = pico8_ui_field_raw(self, "qa");
    je_ra_val = pico8_ui_field_raw(g_je, "ra");

    hd_bucket = pico8_cv("hd");
    if (hd_bucket != NULL) {
        for (i = 0; i < hd_bucket->count; i++) {
            pico8_instance_t *e = hd_bucket->items[i].as.ref;
            pico8_value_t hit_out;

            if (pico8_mv(e, "hd", 2, qa_val, je_ra_val, &hit_out) && pico8_ui_truthy(hit_out)) {
                pico8_value_t fa_val;
                if (pico8_instance_get_field(e, "fa", &fa_val) && fa_val.kind == PICO8_VALUE_FIXED) {
                    pico8_ft_index(ic, pico8_ui_to_int(fa_val.as.fixed), pico8_value_ref(e));
                }
                pico8_ob_table_set_name(self->fields, "o_", pico8_value_ref(e));
            }
        }
    }

    gj_val = pico8_ui_field_raw(self, "gj");
    if (gj_val.kind == PICO8_VALUE_REF && gj_val.as.ref != NULL) {
        pico8_ob_table_set_name(((pico8_instance_t *)gj_val.as.ref)->fields, "m_", pico8_value_bool(0));
    }

    o_val = pico8_ui_field_raw(self, "o_");
    if (o_val.kind == PICO8_VALUE_REF && o_val.as.ref != NULL) {
        pico8_ob_table_set_name(((pico8_instance_t *)o_val.as.ref)->fields, "m_", pico8_value_bool(1));
    }

    if (o_val.kind == PICO8_VALUE_REF && o_val.as.ref != NULL) {
        pico8_value_t fg_val = pico8_ui_field_raw((const pico8_instance_t *)o_val.as.ref, "fg");
        poke_val = (fg_val.kind == PICO8_VALUE_FIXED) ? pico8_ui_to_int(fg_val.as.fixed) : 0;
    } else {
        int prev = pico8_peek(mem, 0x5f80);
        poke_val = (prev - 1 > 0) ? prev - 1 : 0;
    }
    pico8_poke(mem, 0x5f80, (unsigned char)poke_val);
}

/* function ui:bh(bt)local b,ni=
 * je[bt],bt.."held"if b.jn then
 * local pw=self:jz(bt.."down",self.qa,je.ra)if pw and type(pw.kn)=="table" then
 * self[ni]=pw.kn
 * end
 * end
 * local held=self[ni]
 * mv(held,"hq",self.qa,je.ra)if b.hm and held then
 * local nw=self:jz("gv",held)mv(held,"ey",nw and nw.kv)self[ni]=nil
 * end
 * end
 * -- see pico8_ui.h's own comment on this declaration for the full trace.
 * `bt.."held"`/`bt.."down"` (dynamic field-name/method-name string
 * concatenation): pico8_ob_table_set_name/pico8_mv already take an
 * arbitrary runtime `const char *`, so no special mechanism is needed --
 * built here with snprintf into small fixed buffers. `mv(held,"hq",...)`'s
 * result is discarded (side effect only), matching the real source's own
 * bare statement.
 *
 * `nw and nw.kv`: `nw` is the FRESH `{kv=h,kn=kn}` table ui:jz builds
 * (pico8/lua.lua:251), so `nw.kv` IS `h`, the entity jz found -- which
 * pico8_ui_jz already hands back directly as `*out_kv` (see pico8_ui.h).
 * It is NOT a field read on that entity: no object in this cart ever has a
 * field named "kv" (`grep -n "kv=" pico8/lua.lua` -> only the global class
 * registry and jz's own return table), so reading one would make this
 * always pass nil to jc:ey() and kill slipway construction outright. */
void pico8_ui_bh(pico8_instance_t *self, const char *bt) {
    char ni[16];
    pico8_value_t held_val;
    pico8_instance_t *held_inst = NULL;
    pico8_value_t qa_val;
    pico8_value_t je_ra_val;
    pico8_value_t discard;

    if (self == NULL) {
        return;
    }

    snprintf(ni, sizeof ni, "%sheld", bt);
    qa_val = pico8_ui_field_raw(self, "qa");
    je_ra_val = pico8_ui_field_raw(g_je, "ra");

    if (pico8_ui_je_button_field_bool(bt, "jn")) {
        char down_name[16];
        pico8_instance_t *kv = NULL;
        pico8_value_t kn;

        snprintf(down_name, sizeof down_name, "%sdown", bt);

        if (pico8_ui_jz(self, down_name, 2, qa_val, je_ra_val, &kv, &kn) && kn.kind == PICO8_VALUE_REF) {
            pico8_ob_table_set_name(self->fields, ni, kn);
        }
    }

    held_val = pico8_ui_field_raw(self, ni);
    if (held_val.kind == PICO8_VALUE_REF && held_val.as.ref != NULL) {
        held_inst = (pico8_instance_t *)held_val.as.ref;
    }

    pico8_mv(held_inst, "hq", 2, qa_val, je_ra_val, &discard);

    if (pico8_ui_je_button_field_bool(bt, "hm") && held_inst != NULL) {
        pico8_instance_t *nw_kv = NULL;
        pico8_value_t nw_kn;
        pico8_value_t ey_arg = pico8_value_ref(NULL);

        if (pico8_ui_jz(self, "gv", 1, held_val, pico8_value_ref(NULL), &nw_kv, &nw_kn) && nw_kv != NULL) {
            ey_arg = pico8_value_ref(nw_kv);
        }

        pico8_mv(held_inst, "ey", 1, ey_arg, pico8_value_ref(NULL), &discard);
        pico8_ob_table_set_name(self->fields, ni, pico8_value_ref(NULL));
    }
}

/* function ui:jz(rm,...)for mf=15,0,-1 do
 * for h in all(self.ic[mf])do
 * local kn=mv(h,rm,...)if kn then
 * return {kv=h,kn=kn}
 * end
 * end
 * end
 * end
 * -- see pico8_ui.h's own comment on this declaration for the full trace.
 * `argc`/`arg0`/`arg1` are threaded straight through to pico8_mv() for
 * every candidate `h`, unchanged -- this function's own second real
 * variable-arity mv()-forwarding site (alongside ui:bh()'s two direct
 * calls above), matching the real Lua's own `mv(h,rm,...)` vararg
 * forwarding. */
int pico8_ui_jz(pico8_instance_t *self, const char *rm, int argc, pico8_value_t arg0, pico8_value_t arg1,
                pico8_instance_t **out_kv, pico8_value_t *out_kn) {
    int mf;
    pico8_value_t ic_val;
    const pico8_ob_table_t *ic;

    if (self == NULL) {
        return 0;
    }

    if (!pico8_instance_get_field(self, "ic", &ic_val) || ic_val.kind != PICO8_VALUE_REF || ic_val.as.ref == NULL) {
        return 0;
    }
    ic = (const pico8_ob_table_t *)ic_val.as.ref;

    for (mf = 15; mf >= 0; mf--) {
        pico8_value_t bucket_val;
        const pico8_list_t *bucket;
        int i;

        if (!pico8_ob_table_get_index(ic, mf, &bucket_val) || bucket_val.kind != PICO8_VALUE_REF ||
            bucket_val.as.ref == NULL) {
            continue;
        }
        bucket = (const pico8_list_t *)bucket_val.as.ref;

        for (i = 0; i < bucket->count; i++) {
            pico8_instance_t *h = bucket->items[i].as.ref;
            pico8_value_t kn;

            if (pico8_mv(h, rm, argc, arg0, arg1, &kn) && pico8_ui_truthy(kn)) {
                *out_kv = h;
                *out_kn = kn;
                return 1;
            }
        }
    }

    return 0;
}

/* nv(self.le,he)'s own `kp` callback: he() (pico8_entity.h) has no return
 * statement in real Lua (always nil), so nv()'s own `kp(e) or nil`
 * contributes NOTHING to the mapped list for every element -- this always
 * returns 0 (pico8_nv's own "didn't produce a value" convention),
 * matching that exactly; only the per-element pico8_he() SIDE EFFECT
 * (removing each entity from hv/cv/cg) is what ui:cj() is actually for. */
static int pico8_ui_cj_he_mapper(pico8_value_t e, void *ctx, pico8_value_t *out_value) {
    (void)ctx;
    (void)out_value;
    if (e.kind == PICO8_VALUE_REF && e.as.ref != NULL) {
        pico8_he((pico8_instance_t *)e.as.ref);
    }
    return 0;
}

/* function ui:cj()nv(self.le,he)self.le={}
 * end
 * -- see pico8_ui.h's own comment on this declaration for the full trace.
 * The mapped list nv() produces is discarded immediately (see
 * pico8_ui_cj_he_mapper's own comment above); `self.le={}` rebinds
 * self.le to a fresh, empty heap pico8_list_t* REF, the same
 * representation pico8_ui_init() already establishes for `le`'s own
 * class-spec default. */
void pico8_ui_cj(pico8_instance_t *self) {
    pico8_value_t le_val;
    pico8_list_t *heap_le;

    if (self == NULL) {
        return;
    }

    if (pico8_instance_get_field(self, "le", &le_val) && le_val.kind == PICO8_VALUE_REF && le_val.as.ref != NULL) {
        pico8_list_t discarded = pico8_nv((const pico8_list_t *)le_val.as.ref, pico8_ui_cj_he_mapper, NULL);
        pico8_list_free(&discarded);
    }

    heap_le = malloc(sizeof *heap_le);
    pico8_list_init(heap_le);
    pico8_ob_table_set_name(self->fields, "le", pico8_value_ref(heap_le));
}
