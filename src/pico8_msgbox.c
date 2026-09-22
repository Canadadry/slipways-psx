#include "pico8_msgbox.h"

#include <stddef.h>
#include <stdlib.h>

#include "pico8_audio.h"
#include "pico8_collect.h"
#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_fixed.h"
#include "pico8_hud.h"
#include "pico8_ke.h"
#include "pico8_pi.h"
#include "pico8_ui.h"
#include "pico8_ui_button.h"

/* See pico8_msgbox.h's own top comment for the full source/trace of `mh`,
 * `iv`, `pz()` and the `kc` global below. */

static pico8_class_t *g_mh_class = NULL;
static pico8_class_t *g_iv_class = NULL;
static pico8_instance_t *g_kc = NULL;

/* See pico8_msgbox.h's own comment on pico8_kc/pico8_msgbox_set_kc for the
 * full rationale, including why (unlike pico8_entity_set_mo/pico8_ui_set_je/
 * pico8_ui_set_ok) this setter deliberately never frees the previous
 * value: `kc` is a borrowed bookmark into a still-hv-registered `mh`
 * instance owned by whoever constructed it, not a singleton this module
 * owns. */
pico8_instance_t *pico8_kc(void) {
    return g_kc;
}

void pico8_msgbox_set_kc(pico8_instance_t *instance) {
    g_kc = instance;
}

/* Lua truthiness (only nil/false are falsy) -- same idiom already
 * established across this port (e.g. pico8_layout.c's own
 * pico8_layout_truthy, pico8_ui_button.c's own pico8_ui_field_truthy), a
 * private per-module copy here too, same reasoning those files already
 * give for not sharing a few-line helper across a module boundary.
 *
 * The REF-NULL clause is not optional: this port models Lua nil as
 * pico8_value_ref(NULL) (src/pico8_value.h's own convention), so a helper
 * that only rejects BOOL false reports nil as TRUE and the comment above
 * it becomes a lie. Measured consequence, the reason this clause is here:
 * pz()'s own `if p.ep then` (pico8/lua.lua:280) prepended a description
 * row to a BUILT planet's panel, whose `ep` jo:lz just nilled
 * (`self.hn,self.ep=hp`, pico8/lua.lua:461) -- a row real Lua never
 * builds, which lq:mn then drew as spr(0), an 8x8 block of raw sprite-
 * sheet ramp data on top of the planet. See tests/test_pico8_built_
 * planet_panel_nil_row.c. */
static int pico8_msgbox_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

/* Per-module defensive field readers (fixed/vec), matching the same
 * "read a field expected to hold kind K, absent/wrong-kind/NULL-instance
 * all read as a documented default" convention already established by
 * pico8_entity_geom.c/pico8_hud.c/pico8_ui_button.c/pico8_bg.c -- a
 * private per-module copy, not shared/exported, same established
 * reasoning. */
static pico8_fixed_t pico8_msgbox_field_fixed(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v) && v.kind == PICO8_VALUE_FIXED) {
        return v.as.fixed;
    }
    return pico8_fixed_from_int(0);
}

static pico8_vec_t pico8_msgbox_field_vec(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

/* THIS CHUNK'S OWN RESOLUTION of the Chunk 2-flagged `bl(self,nc)` call-
 * site gap -- see pico8_msgbox.h's own comment on pico8_mh_ev for the full
 * trace. Walks `table`'s own entries in insertion order and collects every
 * PICO8_OB_KEY_INDEX one, exactly the established pattern pico8_layout.c's
 * own (private, static) pico8_layout_positional_values already uses for
 * `lq`/`jb`'s identical "the whole instance IS a list" shape (Chunk 3) --
 * reused here as its own private per-module copy, not exported/shared,
 * same reasoning that file already gives. Returns a new, heap-backed list
 * the caller owns and must release with pico8_list_free(). */
static pico8_list_t pico8_msgbox_positional_values(const pico8_ob_table_t *table) {
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

/* function mh:pq()
 * if (kc)kc.p_=true
 * kc=self
 * end
 * (pico8/lua.lua:584-587.) See pico8_msgbox.h's own comment on this
 * declaration for the full trace. Directly usable as this class's own
 * pico8_class_pq_fn (pico8_mh_init below) -- no separate dispatch adapter
 * needed, since this signature already matches `void(*)(pico8_instance_t
 * *)` exactly. */
void pico8_mh_pq(pico8_instance_t *self) {
    pico8_instance_t *previous;

    if (self == NULL) {
        return;
    }

    previous = pico8_kc();
    if (previous != NULL) {
        pico8_ob_table_set_name(previous->fields, "p_", pico8_value_bool(1));
    }
    pico8_msgbox_set_kc(self);
}

/* Bridges pico8_value_function_t's (self,argc,arg0,arg1) shape to
 * pico8_mh_ev's own signature; dc()'s own "ev" bm-name dispatch passes
 * argc=1 with `qc.ra` as arg0, unused (mh:ev's real body never references
 * it). */
static pico8_value_t pico8_mh_ev_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_mh_ev(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

/* function mh:ev()local t=self.t/self.rh-10
 * if t>=0 and mv(self,"lh")then
 * self.t,t=self.rh*10,0
 * end
 * local nc=-t^3/9
 * bl(self,nc)camera()if t>10.5 then
 * self.p_,kc=true
 * end
 * end
 * (pico8/lua.lua:588-596.) See pico8_msgbox.h's own comment on this
 * declaration for the full trace (the `t^3` precedence, the `bl(self,nc)`
 * instance-to-list gap resolution, and the `self.p_,kc=true` ->
 * `self.p_=true;kc=nil` multiple-assignment subtlety). */
void pico8_mh_ev(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self) {
    pico8_fixed_t self_t, rh, t, nc, t3;
    pico8_value_t lh_result;
    int lh_found;

    if (self == NULL) {
        return;
    }

    self_t = pico8_msgbox_field_fixed(self, "t");
    rh = pico8_msgbox_field_fixed(self, "rh");
    t = pico8_fixed_sub(pico8_fixed_div(self_t, rh), pico8_fixed_from_int(10));

    if (t >= 0) {
        lh_found = pico8_mv(self, "lh", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &lh_result);
        if (lh_found && pico8_msgbox_truthy(lh_result)) {
            pico8_fixed_t new_t = pico8_fixed_mul(rh, pico8_fixed_from_int(10));
            pico8_ob_table_set_name(self->fields, "t", pico8_value_fixed(new_t));
            t = pico8_fixed_from_int(0);
        }
    }

    /* local nc=-t^3/9 -- `^` binds tighter than unary `-`, which itself
     * binds tighter than `/` (Lua's own precedence table), so this is
     * (-(t^3))/9, not (-t)^3/9 or -(t^3/9). */
    t3 = pico8_fixed_mul(pico8_fixed_mul(t, t), t);
    nc = pico8_fixed_div(pico8_fixed_neg(t3), pico8_fixed_from_int(9));

    /* bl(self,nc)camera() -- the instance-to-list gap resolution (see
     * pico8_msgbox_positional_values above) plus the reset camera() call;
     * a NULL gfx/mem skips only this drawing/camera-reset portion. */
    if (gfx != NULL && mem != NULL) {
        pico8_list_t items = pico8_msgbox_positional_values(self->fields);
        pico8_bl(gfx, mem, &items, nc);
        pico8_list_free(&items);
        pico8_camera_reset(gfx);
    }

    /* if t>10.5 then self.p_,kc=true end -- Lua pads the extra assignment
     * target with nil when fewer values than targets are given: this is
     * self.p_=true; kc=nil, NOT self.p_=true; kc=true (see pico8_msgbox.h's
     * own comment on pico8_mh_ev for the full trace). */
    if (t > pico8_fixed_from_double(10.5)) {
        pico8_ob_table_set_name(self->fields, "p_", pico8_value_bool(1));
        pico8_msgbox_set_kc(NULL);
    }
}

/* mh=ke:jr([[
 * nu=o("mh"),
 * fa=14,
 * rh=5,
 * ]]) -- see pico8_msgbox.h's own top comment for the full trace. No
 * pico8_pi_init() call needed: `nu=o("mh")` hits ob()'s own unregistered-
 * identifier fallback (`o` is never a `pi` key), not the char-keyed
 * registry's own `v(...)`/`b(...)` ordering hazard. */
void pico8_mh_init(void) {
    pico8_mh_shutdown();
    g_mh_class = pico8_class_declare(pico8_ke_class(), "\nnu=o(\"mh\"),\nfa=14,\nrh=5,\n");
    pico8_class_set_pq(g_mh_class, pico8_mh_pq);
    pico8_ob_table_set_name(g_mh_class->defaults, "ev", pico8_value_function(pico8_mh_ev_dispatch));
}

/* Also resets `kc` to NULL -- not part of the Lua original (which has no
 * manual teardown), but needed here for the same test-isolation reason
 * pico8_entity_init()/pico8_ui_init() already reset their own singleton
 * globals: a stale `kc` pointing at a freed instance from a PREVIOUS
 * pico8_mh_init()/test run must never survive into the next one. */
void pico8_mh_shutdown(void) {
    if (g_mh_class != NULL) {
        pico8_class_free(g_mh_class);
        g_mh_class = NULL;
    }
    pico8_msgbox_set_kc(NULL);
}

pico8_class_t *pico8_mh_class(void) {
    return g_mh_class;
}

/* `mh(overrides)` -- see pico8_msgbox.h's own comment on this declaration. */
pico8_instance_t *pico8_mh_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_mh_class, overrides);
}

/* function iv:pq()self.ra=self.ra or
 * je.ra+mo.ra-v(0,5)end
 * (pico8/lua.lua:944-945.) See pico8_msgbox.h's own comment on this
 * declaration for the full trace. Directly usable as this class's own
 * pico8_class_pq_fn -- no separate dispatch adapter needed, same reasoning
 * as pico8_mh_pq above. */
void pico8_iv_pq(pico8_instance_t *self) {
    pico8_value_t ra_val;
    int ra_present;

    if (self == NULL) {
        return;
    }

    ra_present = pico8_instance_get_field(self, "ra", &ra_val);
    if (!ra_present || !pico8_msgbox_truthy(ra_val)) {
        pico8_vec_t je_ra = pico8_msgbox_field_vec(pico8_je(), "ra");
        pico8_vec_t mo_ra = pico8_msgbox_field_vec(pico8_mo(), "ra");
        pico8_vec_t offset = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(5));
        pico8_vec_t new_ra = pico8_vec_sub(pico8_vec_add(je_ra, mo_ra), offset);
        pico8_vec_t *heap_ra = malloc(sizeof *heap_ra);
        *heap_ra = new_ra;
        pico8_ob_table_set_name(self->fields, "ra", pico8_value_ref(heap_ra));
    }
}

/* function iv:ph()
 * self.ra+=self.v
 * self.l+=0.1
 * self.p_=self.l>=5
 * end
 * (pico8/lua.lua:946-949.) See pico8_msgbox.h's own comment on this
 * declaration for the full trace -- `self.p_=self.l>=5` reads `self.l`
 * AFTER the preceding `self.l+=0.1` has already updated it (evaluation
 * order preserved: `new_l` is computed, written, then compared). */
void pico8_iv_ph(pico8_instance_t *self) {
    pico8_vec_t ra, v, new_ra;
    pico8_fixed_t l, new_l;
    pico8_vec_t *heap_ra;

    if (self == NULL) {
        return;
    }

    ra = pico8_msgbox_field_vec(self, "ra");
    v = pico8_msgbox_field_vec(self, "v");
    new_ra = pico8_vec_add(ra, v);
    heap_ra = malloc(sizeof *heap_ra);
    *heap_ra = new_ra;
    pico8_ob_table_set_name(self->fields, "ra", pico8_value_ref(heap_ra));

    l = pico8_msgbox_field_fixed(self, "l");
    new_l = pico8_fixed_add(l, pico8_fixed_from_double(0.1));
    pico8_ob_table_set_name(self->fields, "l", pico8_value_fixed(new_l));

    pico8_ob_table_set_name(self->fields, "p_", pico8_value_bool(new_l >= pico8_fixed_from_int(5)));
}

/* Bridges pico8_value_function_t's shape to pico8_iv_ph's own signature;
 * dg()'s own per-tick dispatch always passes argc=1 with `qc.t` as arg0,
 * unused here (iv:ph's real body never references it). */
static pico8_value_t pico8_iv_ph_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_iv_ph((pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

/* function iv:kj(p)dv(self.l)io(self[1],p.x,p.y,self[2],0.5)dv()end
 * (pico8/lua.lua:951.) See pico8_msgbox.h's own comment on this
 * declaration for the full trace, including why `self[1]`/`self[2]` are
 * read directly off `self->fields` via pico8_ob_table_get_index rather
 * than pico8_instance_get_field. */
void pico8_iv_kj(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self, pico8_vec_t p) {
    pico8_fixed_t l;
    pico8_value_t text_val;
    pico8_value_t color_val;
    const char *text = "";
    int color = 0;
    pico8_fixed_t a_center = pico8_fixed_from_double(0.5);

    if (self == NULL || gfx == NULL || mem == NULL) {
        return;
    }

    l = pico8_msgbox_field_fixed(self, "l");
    pico8_dv(mem, l);

    if (pico8_ob_table_get_index(self->fields, 1, &text_val) && text_val.kind == PICO8_VALUE_STRING) {
        text = text_val.as.string;
    }
    if (pico8_ob_table_get_index(self->fields, 2, &color_val) && color_val.kind == PICO8_VALUE_FIXED) {
        color = (int)pico8_fixed_to_double(color_val.as.fixed);
    }

    pico8_io(gfx, mem, text, p.x, p.y, color, &a_center);

    pico8_dv_default(mem);
}

/* Bridges to pico8_iv_kj's own signature; dc()'s own "kj" bm-name dispatch
 * passes argc=1 with `qc.ra` as arg0 -- iv:kj's real body DOES use it
 * (`p`, its own drawing position), unpacked here as a REF-to-vec exactly
 * like js:kj/bg:kj's own precedent; arg1 is unused. */
static pico8_value_t pico8_iv_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_vec_t p = (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL)
                        ? *(const pico8_vec_t *)arg0.as.ref
                        : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    (void)argc;
    (void)arg1;
    pico8_iv_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (const pico8_instance_t *)self, p);
    return pico8_value_ref(NULL);
}

/* iv=ke:jr([[
 * fa=9,
 * l=0,v=v(0,-0.34),
 * ]]) -- see pico8_msgbox.h's own top comment for the full trace. Calls
 * pico8_pi_init() defensively first (idempotent, matching
 * pico8_mouse_reader.c's/pico8_scroll.c's own established convention):
 * `v(0,-0.34)` needs Chunk 2's char-keyed `'v'` pi registration to already
 * exist, or ob() would silently produce a raw positional table instead of
 * a real pico8_vec_t-backed REF. */
void pico8_iv_init(void) {
    pico8_iv_shutdown();
    pico8_pi_init();
    g_iv_class = pico8_class_declare(pico8_ke_class(), "\nfa=9,\nl=0,v=v(0,-0.34),\n");
    pico8_class_set_pq(g_iv_class, pico8_iv_pq);
    pico8_ob_table_set_name(g_iv_class->defaults, "ph", pico8_value_function(pico8_iv_ph_dispatch));
    pico8_ob_table_set_name(g_iv_class->defaults, "kj", pico8_value_function(pico8_iv_kj_dispatch));
}

void pico8_iv_shutdown(void) {
    if (g_iv_class != NULL) {
        pico8_class_free(g_iv_class);
        g_iv_class = NULL;
    }
}

pico8_class_t *pico8_iv_class(void) {
    return g_iv_class;
}

/* `iv(overrides)` -- see pico8_msgbox.h's own comment on this declaration. */
pico8_instance_t *pico8_iv_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_iv_class, overrides);
}

/* Per-call context for pico8_pz_step below -- C's stand-in for the real
 * closure's own captured upvalues (`ra`, `bp`, and `p` itself), matching
 * this port's established closure-factory convention (e.g. Chunk 1's
 * pico8_jf/pico8_gn). */
typedef struct {
    const pico8_instance_t *p;
    pico8_vec_t bp;
    pico8_vec_t *ra_heap;
} pico8_pz_ctx_t;

/* function(b)b.ra,b.qf,b.fa=
 * ra,bp,p.ca
 * bp+=p.gq
 * return js(b)end
 * (pico8/lua.lua:283-286, pz()'s own inline closure -- see pico8_msgbox.h's
 * own comment on pico8_pz for the full trace.) `b.ra` always the SAME
 * shared `ctx->ra_heap` pointer for every item (matching Lua's own by-
 * reference table-assignment semantics), `b.fa` written only when `p.ca`
 * is actually present (Lua's own `b.fa=nil` when absent is a real field
 * removal). Always contributes a value (`return js(b)` is always truthy),
 * so this always returns 1 for a well-formed element. */
static int pico8_pz_step(pico8_value_t e, void *ctx_v, pico8_value_t *out_value) {
    pico8_pz_ctx_t *ctx = ctx_v;
    pico8_ob_table_t *b;
    pico8_vec_t *qf_heap;
    pico8_value_t ca_val;
    pico8_vec_t gq;
    pico8_instance_t *made;

    if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
        return 0;
    }
    b = (pico8_ob_table_t *)e.as.ref;

    pico8_ob_table_set_name(b, "ra", pico8_value_ref(ctx->ra_heap));

    qf_heap = malloc(sizeof *qf_heap);
    *qf_heap = ctx->bp;
    pico8_ob_table_set_name(b, "qf", pico8_value_ref(qf_heap));

    if (pico8_instance_get_field(ctx->p, "ca", &ca_val)) {
        pico8_ob_table_set_name(b, "fa", ca_val);
    }

    gq = pico8_msgbox_field_vec(ctx->p, "gq");
    ctx->bp = pico8_vec_add(ctx->bp, gq);

    made = pico8_js_new(b);
    *out_value = pico8_value_ref(made);
    return 1;
}

/* function pz(p,bs)if p.ep then
 * bs=la({{lq={p.ep,c=13}}},bs)end
 * rn(p.eo or 62)local ra=p.hi or p.ra
 * local bp=ra+v(0,0)ok:cj()ok.le=nv(bs,function(b)b.ra,b.qf,b.fa=
 * ra,bp,p.ca
 * bp+=p.gq
 * return js(b)end)
 * if (p.ff)ok.le={}
 * end
 * (pico8/lua.lua:280-288.) See pico8_msgbox.h's own comment on this
 * declaration for the full trace. */
void pico8_pz(const pico8_instance_t *p, const pico8_list_t *bs) {
    pico8_value_t ep_val;
    const pico8_list_t *bs_effective = bs;
    pico8_list_t merged;
    int merged_built = 0;
    pico8_value_t hi_val;
    pico8_vec_t ra;
    pico8_pz_ctx_t ctx;
    pico8_list_t *le_heap;
    pico8_instance_t *ok;
    pico8_value_t eo_val;
    pico8_value_t rn_arg;
    pico8_value_t ff_val;

    if (p == NULL) {
        return;
    }

    /* if p.ep then bs=la({{lq={p.ep,c=13}}},bs) end -- rebinds the LOCAL
     * `bs`, never mutating whatever list the caller passed in. */
    if (pico8_instance_get_field(p, "ep", &ep_val) && pico8_msgbox_truthy(ep_val)) {
        pico8_ob_table_t *inner = pico8_ob_table_new();
        pico8_ob_table_t *outer = pico8_ob_table_new();
        pico8_list_t prefix_item;

        pico8_ob_table_set_index(inner, 1, ep_val);
        pico8_ob_table_set_name(inner, "c", pico8_value_fixed(pico8_fixed_from_int(13)));
        pico8_ob_table_set_name(outer, "lq", pico8_value_ref(inner));

        pico8_list_init(&prefix_item);
        pico8_list_add(&prefix_item, pico8_value_ref(outer));

        merged = pico8_la(&prefix_item, bs);
        pico8_list_free(&prefix_item);
        merged_built = 1;
        bs_effective = &merged;
    }

    if (pico8_instance_get_field(p, "eo", &eo_val) && pico8_msgbox_truthy(eo_val)) {
        rn_arg = eo_val;
    } else {
        rn_arg = pico8_value_fixed(pico8_fixed_from_int(62));
    }
    pico8_rn(rn_arg);

    if (pico8_instance_get_field(p, "hi", &hi_val) && pico8_msgbox_truthy(hi_val) &&
        hi_val.kind == PICO8_VALUE_REF && hi_val.as.ref != NULL) {
        ra = *(const pico8_vec_t *)hi_val.as.ref;
    } else {
        ra = pico8_msgbox_field_vec(p, "ra");
    }

    ctx.p = p;
    ctx.bp = pico8_vec_add(ra, pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0)));
    ctx.ra_heap = malloc(sizeof *ctx.ra_heap);
    *ctx.ra_heap = ra;

    /* ok:cj() -- closes/clears whatever option row was previously showing
     * before building the new one. */
    ok = pico8_ok();
    pico8_ui_cj(ok);

    /* ok.le=nv(bs,function(b)...end) -- overwrites the fresh empty list
     * pico8_ui_cj() just built for `ok.le` without freeing it first, the
     * SAME accepted "field-rebind leaks the old value" convention
     * pico8_ui_cj itself already establishes for its own `self.le={}`
     * rebind (pico8_ui.c). */
    le_heap = malloc(sizeof *le_heap);
    *le_heap = pico8_nv(bs_effective, pico8_pz_step, &ctx);
    if (ok != NULL) {
        pico8_ob_table_set_name(ok->fields, "le", pico8_value_ref(le_heap));
    } else {
        pico8_list_free(le_heap);
        free(le_heap);
    }

    if (merged_built) {
        pico8_list_free(&merged);
    }

    /* if (p.ff) ok.le={} -- discards the row just built (rebound to a
     * fresh empty list); the individual js instances already constructed
     * above are NOT un-lt()'d by this, matching the real Lua exactly. */
    if (pico8_instance_get_field(p, "ff", &ff_val) && pico8_msgbox_truthy(ff_val) && ok != NULL) {
        pico8_list_t *empty_heap = malloc(sizeof *empty_heap);
        pico8_list_init(empty_heap);
        pico8_ob_table_set_name(ok->fields, "le", pico8_value_ref(empty_heap));
    }
}
