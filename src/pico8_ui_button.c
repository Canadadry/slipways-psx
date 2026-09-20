#include "pico8_ui_button.h"

#include <stddef.h>
#include <stdlib.h>

#include "pico8_audio.h"
#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_geom.h"
#include "pico8_ke.h"
#include "pico8_layout.h"

/* Phase 3, Chunk 4 (docs/prd/triage/phase3-etape1-function-by-function-c-
 * port.md): `js` and `fq`, both real `ke`-descended classes declared
 * back-to-back in the real source (`js=ke:jr(...)`, pico8/lua.lua:258;
 * `fq=ke:jr(...)`, pico8/lua.lua:324). Reading their own real methods,
 * `js` turns out to be the cart's general-purpose clickable UI icon/text
 * button (used for dialog options, the difficulty menu, tooltips via its
 * own `.h_`/`.jb` fields) and `fq` its small modal-backdrop sibling (a
 * full-screen, always-hit click-catcher used to dismiss the topmost modal
 * -- confirmed by its real construction site, `fq()js(ob([[...]],
 * {my=_init}))`, pico8/lua.lua:871, and its own `hd=1` making it always
 * "hit" in `ui:gw()`'s future hit-test loop regardless of position).
 * Grouped in one module: both are small, thematically identical clickable
 * UI widgets, and this PRD's own Chunk 4 groups them together. See each
 * function's own comment below for its exact real Lua source. */

static pico8_class_t *g_js_class = NULL;
static pico8_class_t *g_fq_class = NULL;

/* Per-module defensive field readers (fixed/bool/vec), matching the same
 * "read a field expected to hold kind K, absent/wrong-kind/NULL-instance
 * all read as a documented default" convention already established by
 * pico8_entity_geom.c/pico8_hud.c/pico8_scroll.c -- a private per-module
 * copy each time (not shared/exported), same reasoning those files'
 * own comments already give for not widening another module's header
 * surface for a few-line helper. */
static int pico8_ui_to_int(pico8_fixed_t x) {
    return (int)pico8_fixed_to_double(x);
}

static pico8_fixed_t pico8_ui_field_fixed(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v) && v.kind == PICO8_VALUE_FIXED) {
        return v.as.fixed;
    }
    return pico8_fixed_from_int(0);
}

static int pico8_ui_field_truthy(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst == NULL || !pico8_instance_get_field(inst, name, &v)) {
        return 0;
    }
    return !(v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0);
}

static pico8_vec_t pico8_ui_field_vec(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

/* Bridges pico8_class_pq_fn's shape to pico8_js_pq's own (identical)
 * signature -- js:pq needs no external context, so this is a plain
 * passthrough, unlike pico8_mouse_reader.c's own pico8_nf_pq_dispatch. */
static void pico8_js_pq_dispatch(pico8_instance_t *instance) {
    pico8_js_pq(instance);
}

/* Bridges pico8_value_function_t's fixed (self,arg) shape to pico8_js_ph's
 * own signature; `arg` (dg()'s `qc.t`) unused, matching the real Lua
 * body's own arity -- js:ph needs no external context either. */
static pico8_value_t pico8_js_ph_dispatch(void *self, pico8_value_t arg) {
    (void)arg;
    pico8_js_ph((pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

/* Bridges to pico8_js_ev's own signature; dc()'s own "ev" dispatch passes
 * `qc.ra` as `arg` (js:ev's real body never references it, so unused
 * here too), reading pico8_dispatch_context.h's shared gfx/mem for
 * jb:ol's own real drawing need. */
static pico8_value_t pico8_js_ev_dispatch(void *self, pico8_value_t arg) {
    (void)arg;
    pico8_js_ev(pico8_dispatch_gfx(), pico8_dispatch_mem(), (const pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

/* Bridges to pico8_js_kj's own signature; dc()'s own "kj" dispatch passes
 * `qc.ra` as `arg` -- js:kj's real body DOES use it (`p`, its own drawing
 * position), unpacked here as a REF-to-vec exactly like every other
 * vector-carrying pico8_value_t in this port. */
static pico8_value_t pico8_js_kj_dispatch(void *self, pico8_value_t arg) {
    pico8_vec_t p = (arg.kind == PICO8_VALUE_REF && arg.as.ref != NULL) ? *(const pico8_vec_t *)arg.as.ref
                                                                         : pico8_vec_v(pico8_fixed_from_int(0),
                                                                                       pico8_fixed_from_int(0));
    pico8_js_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (const pico8_instance_t *)self, p);
    return pico8_value_ref(NULL);
}

/* `js.hd=gk` -- see pico8_ui_button.h's own top comment for why this is a
 * KNOWINGLY INCOMPLETE adapter (passes the same single `arg` for both
 * pico8_gk's own `mp`/`rp` parameters, since pico8_value_function_t's
 * current shape has only one extra-argument slot) that Chunk 5's own
 * mv()/pico8_value_function_t arity extension must revisit once `ui:gw()`
 * (the real 2-argument `mv(e,"hd",self.qa,je.ra)` caller) is ported. */
static pico8_value_t pico8_js_hd_dispatch(void *self, pico8_value_t arg) {
    return pico8_gk((const pico8_instance_t *)self, arg, arg);
}

/* Bridges to pico8_js_ldown's own signature; `ui:jz(rm,...)`'s own
 * `mv(h,rm,...)` dispatch (Chunk 5) would pass `self.qa`/`je.ra` as extra
 * arguments here, but js:ldown's real body never references any --
 * unused, matching that arity exactly. */
static pico8_value_t pico8_js_ldown_dispatch(void *self, pico8_value_t arg) {
    (void)arg;
    return pico8_js_ldown((pico8_instance_t *)self);
}

/* js=ke:jr([[
 * fa=11,
 * ]])js.hd=gk -- see pico8_ui_button.h's own top comment for the full
 * trace. No pico8_pi_init() call needed here (unlike pico8_mouse_reader.c/
 * pico8_scroll.c's own nf/qb): js's spec string has no `v(...)`/`o(...)`
 * nested constructor calls to worry about. */
void pico8_js_init(void) {
    pico8_js_shutdown();
    g_js_class = pico8_class_declare(pico8_ke_class(), "\nfa=11,\n");
    pico8_class_set_pq(g_js_class, pico8_js_pq_dispatch);
    pico8_ob_table_set_name(g_js_class->defaults, "hd", pico8_value_function(pico8_js_hd_dispatch));
    pico8_ob_table_set_name(g_js_class->defaults, "ph", pico8_value_function(pico8_js_ph_dispatch));
    pico8_ob_table_set_name(g_js_class->defaults, "ev", pico8_value_function(pico8_js_ev_dispatch));
    pico8_ob_table_set_name(g_js_class->defaults, "kj", pico8_value_function(pico8_js_kj_dispatch));
    pico8_ob_table_set_name(g_js_class->defaults, "ldown", pico8_value_function(pico8_js_ldown_dispatch));
}

void pico8_js_shutdown(void) {
    if (g_js_class != NULL) {
        pico8_class_free(g_js_class);
        g_js_class = NULL;
    }
}

pico8_class_t *pico8_js_class(void) {
    return g_js_class;
}

pico8_instance_t *pico8_js_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_js_class, overrides);
}

/* function js:pq()self.lq=lq(mv(self,"lq"))self.mb=self.lq.w+2
 * self.kq=qz(0,-1,self.mb,11)self.h_=self.h_ and lq(self.h_)self.jb=self.jb and jb(self.jb)end
 * -- see pico8_ui_button.h's own comment on this declaration for the full
 * trace. `mv(self,"lq",...)` is this port's own established zero-extra-
 * argument mv() convention (pico8_value_ref(NULL) standing in for "no
 * real Lua argument"), returning self's own raw "lq" override field
 * as-is (never a function at any real call site) for pico8_lq_new() to
 * consume as overrides. */
void pico8_js_pq(pico8_instance_t *self) {
    pico8_value_t lq_overrides_val;
    const pico8_ob_table_t *lq_overrides;
    pico8_instance_t *new_lq;
    pico8_value_t w_val;
    pico8_fixed_t mb;
    pico8_qz_box_t *kq;
    pico8_value_t h_val;
    pico8_value_t jb_val;

    if (self == NULL) {
        return;
    }

    pico8_mv(self, "lq", pico8_value_ref(NULL), &lq_overrides_val);
    lq_overrides =
        (lq_overrides_val.kind == PICO8_VALUE_REF) ? (const pico8_ob_table_t *)lq_overrides_val.as.ref : NULL;
    new_lq = pico8_lq_new(lq_overrides);
    pico8_ob_table_set_name(self->fields, "lq", pico8_value_ref(new_lq));

    mb = pico8_fixed_from_int(2);
    if (pico8_instance_get_field(new_lq, "w", &w_val) && w_val.kind == PICO8_VALUE_FIXED) {
        mb = pico8_fixed_add(w_val.as.fixed, mb);
    }
    pico8_ob_table_set_name(self->fields, "mb", pico8_value_fixed(mb));

    kq = pico8_qz(pico8_fixed_from_int(0), pico8_fixed_from_int(-1), mb, pico8_fixed_from_int(11));
    pico8_ob_table_set_name(self->fields, "kq", pico8_value_ref(kq));

    if (pico8_instance_get_field(self, "h_", &h_val) && !(h_val.kind == PICO8_VALUE_BOOL && h_val.as.boolean == 0)) {
        const pico8_ob_table_t *h_overrides =
            (h_val.kind == PICO8_VALUE_REF) ? (const pico8_ob_table_t *)h_val.as.ref : NULL;
        pico8_instance_t *new_h = pico8_lq_new(h_overrides);
        pico8_ob_table_set_name(self->fields, "h_", pico8_value_ref(new_h));
    }

    if (pico8_instance_get_field(self, "jb", &jb_val) &&
        !(jb_val.kind == PICO8_VALUE_BOOL && jb_val.as.boolean == 0)) {
        const pico8_ob_table_t *jb_overrides =
            (jb_val.kind == PICO8_VALUE_REF) ? (const pico8_ob_table_t *)jb_val.as.ref : NULL;
        pico8_instance_t *new_jb = pico8_jb_new(jb_overrides);
        pico8_ob_table_set_name(self->fields, "jb", pico8_value_ref(new_jb));
    }
}

/* function js:ph()if self.qf then
 * local d=self.qf-self.ra
 * self.ra+=(#d<=1 and d or d*0.4)
 * end
 * end
 * -- see pico8_ui_button.h's own comment on this declaration. `self.ra+=
 * ...` is a field REBIND (fresh heap vector), not an in-place mutation --
 * same reasoning as pico8_scroll.c's own pico8_qb_ph. */
void pico8_js_ph(pico8_instance_t *self) {
    pico8_value_t qf_val;

    if (self == NULL) {
        return;
    }

    if (pico8_instance_get_field(self, "qf", &qf_val) && qf_val.kind == PICO8_VALUE_REF && qf_val.as.ref != NULL) {
        pico8_vec_t qf = *(const pico8_vec_t *)qf_val.as.ref;
        pico8_vec_t ra = pico8_ui_field_vec(self, "ra");
        pico8_vec_t d = pico8_vec_sub(qf, ra);
        pico8_fixed_t len_sq = pico8_vec_len_sq(d);
        pico8_vec_t delta = (len_sq <= pico8_fixed_from_int(1)) ? d : pico8_vec_mul(d, pico8_fixed_from_double(0.4));
        pico8_vec_t new_ra = pico8_vec_add(ra, delta);
        pico8_vec_t *heap_ra = malloc(sizeof *heap_ra);
        *heap_ra = new_ra;
        pico8_ob_table_set_name(self->fields, "ra", pico8_value_ref(heap_ra));
    }
}

/* function js:ev()if self.jb and self.m_ then
 * self.jb:ol(127,1)end
 * end
 * -- see pico8_ui_button.h's own comment on this declaration. */
void pico8_js_ev(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self) {
    pico8_value_t jb_val;

    if (self == NULL) {
        return;
    }

    if (pico8_instance_get_field(self, "jb", &jb_val) && jb_val.kind == PICO8_VALUE_REF && jb_val.as.ref != NULL &&
        pico8_ui_field_truthy(self, "m_")) {
        pico8_jb_ol(gfx, mem, (const pico8_instance_t *)jb_val.as.ref, pico8_fixed_from_int(127),
                    pico8_fixed_from_int(1));
    }
}

/* function js:kj(p)local qt,dn,c=
 * p.x+self.mb-1,p.y+9,self.my
 * rect(p.x-1,p.y,qt+1,dn+1,1)rectfill(p.x,p.y,qt,dn,c and (self.m_ and 12 or 13)or 5)rectfill(p.x,p.y,qt,p.y,c and 6 or 5)self.lq:ol(p+v(1,2))local c=self.h_
 * if c then
 * rectfill(qt+3,p.y+1,qt+c.w+4,dn,c.bg or 2)c:ol(p+v(self.mb+3,2))end
 * end
 * -- see pico8_ui_button.h's own comment on this declaration. The real
 * body's LOCAL `c` is reused for two different purposes (first `self.my`,
 * a truthy/falsy check only; then reassigned to `self.h_`) -- kept as two
 * separate C reads (`has_my` and the later `h_val`/`c` lookup) rather than
 * one shared variable, for clarity, without changing behavior. */
void pico8_js_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self, pico8_vec_t p) {
    pico8_fixed_t mb = pico8_ui_field_fixed(self, "mb");
    pico8_fixed_t qt = pico8_fixed_sub(pico8_fixed_add(p.x, mb), pico8_fixed_from_int(1));
    pico8_fixed_t dn = pico8_fixed_add(p.y, pico8_fixed_from_int(9));
    int has_my;
    int m_flag;
    pico8_value_t lq_val;
    pico8_value_t h_val;
    int ix_p;
    int iy_p;
    int ix_qt;
    int iy_dn;

    if (self == NULL) {
        return;
    }

    has_my = pico8_ui_field_truthy(self, "my");
    m_flag = pico8_ui_field_truthy(self, "m_");

    ix_p = pico8_ui_to_int(p.x);
    iy_p = pico8_ui_to_int(p.y);
    ix_qt = pico8_ui_to_int(qt);
    iy_dn = pico8_ui_to_int(dn);

    pico8_rect(gfx, mem, ix_p - 1, iy_p, ix_qt + 1, iy_dn + 1, 1);
    pico8_rectfill(gfx, mem, ix_p, iy_p, ix_qt, iy_dn, has_my ? (m_flag ? 12 : 13) : 5);
    pico8_rectfill(gfx, mem, ix_p, iy_p, ix_qt, iy_p, has_my ? 6 : 5);

    if (pico8_instance_get_field(self, "lq", &lq_val) && lq_val.kind == PICO8_VALUE_REF && lq_val.as.ref != NULL) {
        pico8_vec_t off = pico8_vec_add(p, pico8_vec_v(pico8_fixed_from_int(1), pico8_fixed_from_int(2)));
        pico8_lq_ol_default(gfx, mem, (const pico8_instance_t *)lq_val.as.ref, off);
    }

    if (pico8_instance_get_field(self, "h_", &h_val) && h_val.kind == PICO8_VALUE_REF && h_val.as.ref != NULL) {
        const pico8_instance_t *c = (const pico8_instance_t *)h_val.as.ref;
        pico8_fixed_t c_w = pico8_ui_field_fixed(c, "w");
        pico8_value_t bg_val;
        int bg = 2;
        pico8_fixed_t x2 = pico8_fixed_add(qt, c_w);

        if (pico8_instance_get_field(c, "bg", &bg_val) && bg_val.kind == PICO8_VALUE_FIXED) {
            bg = pico8_ui_to_int(bg_val.as.fixed);
        }
        pico8_rectfill(gfx, mem, pico8_ui_to_int(qt) + 3, iy_p + 1, pico8_ui_to_int(x2) + 4, iy_dn, bg);

        {
            pico8_vec_t off2 =
                pico8_vec_add(p, pico8_vec_v(pico8_fixed_add(mb, pico8_fixed_from_int(3)), pico8_fixed_from_int(2)));
            pico8_lq_ol_default(gfx, mem, c, off2);
        }
    }
}

/* function js:ldown()rn(self.my and 60)mv(self,"my")
 * return true
 * end
 * -- see pico8_ui_button.h's own comment on this declaration. */
pico8_value_t pico8_js_ldown(pico8_instance_t *self) {
    pico8_value_t discard;
    int has_my = pico8_ui_field_truthy(self, "my");

    pico8_rn(has_my ? pico8_value_fixed(pico8_fixed_from_int(60)) : pico8_value_bool(0));
    pico8_mv(self, "my", pico8_value_ref(NULL), &discard);

    return pico8_value_bool(1);
}

/* Bridges to pico8_fq_ldown's own signature; unused `arg`, same reasoning
 * as pico8_js_ldown_dispatch. */
static pico8_value_t pico8_fq_ldown_dispatch(void *self, pico8_value_t arg) {
    (void)arg;
    return pico8_fq_ldown((pico8_instance_t *)self);
}

/* fq=ke:jr([[
 * fa=14,
 * hd=1,
 * ]]) -- see pico8_ui_button.h's own top comment. `hd=1` is parsed
 * straight from the spec string as plain data (PICO8_VALUE_FIXED), no
 * dispatch adapter needed -- only `ldown` (a real function) is wired. No
 * pico8_pi_init() call needed either (no `v(...)`/`o(...)` in this spec). */
void pico8_fq_init(void) {
    pico8_fq_shutdown();
    g_fq_class = pico8_class_declare(pico8_ke_class(), "\nfa=14,hd=1,\n");
    pico8_ob_table_set_name(g_fq_class->defaults, "ldown", pico8_value_function(pico8_fq_ldown_dispatch));
}

void pico8_fq_shutdown(void) {
    if (g_fq_class != NULL) {
        pico8_class_free(g_fq_class);
        g_fq_class = NULL;
    }
}

pico8_class_t *pico8_fq_class(void) {
    return g_fq_class;
}

pico8_instance_t *pico8_fq_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_fq_class, overrides);
}

/* function fq:ldown()self.p_=mv(self,"my")
 * return true
 * end
 * -- see pico8_ui_button.h's own comment on this declaration. */
pico8_value_t pico8_fq_ldown(pico8_instance_t *self) {
    pico8_value_t my_result;

    if (self == NULL) {
        return pico8_value_bool(1);
    }
    if (pico8_mv(self, "my", pico8_value_ref(NULL), &my_result)) {
        pico8_ob_table_set_name(self->fields, "p_", my_result);
    }
    return pico8_value_bool(1);
}
