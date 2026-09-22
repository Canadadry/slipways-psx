#include "pico8_particle.h"

#include <stddef.h>
#include <stdlib.h>

#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_fixed.h"
#include "pico8_vec.h"
#include "pico8_ref_audit.h"

/* See pico8_particle.h's own top comment for the full source of `f_`/`ez`/
 * `ei` and every design decision below (dispatch-shape split for `ei`,
 * particle-table representation, the `mm` forward-reference status). */

/* Per-module defensive field readers (fixed/vec/list/nested-table),
 * matching the same "read a field expected to hold kind K, absent/wrong-
 * kind/NULL-instance all read as a documented default" convention already
 * established by pico8_entity_geom.c/pico8_hud.c/pico8_ui_button.c/
 * pico8_bg.c/pico8_msgbox.c -- a private per-module copy, not shared/
 * exported, same reasoning those files already give. `pico8_particle_
 * field_list`/`_table` extend the same idiom to a REF-valued field this
 * chunk is the first to read defensively (a growable list, a nested named
 * table) rather than a single scalar/vector. */
static pico8_fixed_t pico8_particle_field_fixed(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v) && v.kind == PICO8_VALUE_FIXED) {
        return v.as.fixed;
    }
    return pico8_fixed_from_int(0);
}

static pico8_vec_t pico8_particle_field_vec(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

/* REVERSE-CAST AUDIT (pico8_ref_audit.h): SAFE BY CONSTRUCTION, for the
 * one name this module ever passes. Both call sites are
 * `pico8_particle_field_list(self, "qy")`, and "qy" has a single writer
 * in the whole port: pico8_f_() a few lines below (`self.qy={}` ported as
 * a freshly pico8_list_init()ed heap list), which is the `pq` every
 * particle-system class routes through (pico8_ga_pq(),
 * src/pico8_planet_family.c, calls it before anything can read the
 * field). No class spec string anywhere declares a `qy` default. */
static pico8_list_t *pico8_particle_field_list(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return PICO8_REF_AS_LIST(v.as.ref);
    }
    return NULL;
}

static const pico8_ob_table_t *pico8_particle_field_table(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return (const pico8_ob_table_t *)v.as.ref;
    }
    return NULL;
}

/* Lua truthiness (only nil/false are falsy) -- same idiom already
 * established across this port (pico8_layout_truthy, pico8_ui_field_
 * truthy, pico8_msgbox_truthy), a private per-module copy here too, same
 * reasoning those files already give. Used by `ei`'s own `if (p.a)`
 * check.
 *
 * Deliberately WITHOUT the REF(NULL) clause that 19 of this port's
 * truthiness helpers carry, and deliberately without an assert guarding
 * its absence: at this helper's single call site the very next conjunct
 * is already `a_value.kind == PICO8_VALUE_REF && a_value.as.ref != NULL`,
 * so a nil `p.a` is rejected there whatever this helper answers. The
 * missing clause is unreachable by construction rather than merely
 * unobserved, and an assert here would abort on a value the call site
 * handles correctly today. Contrast pico8_dg_truthy
 * (src/pico8_bootstrap.c), whose call site has no such second guard and
 * therefore does carry one. */
static int pico8_particle_truthy(pico8_value_t v) {
    return !(v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0);
}

/* fixed-to-int conversion for pixel coordinates/table indices, matching
 * the same "convert via double, then cast" convention already established
 * by pico8_bg_to_int/pico8_ui_to_int (private per-module copy, same
 * reasoning). */
static int pico8_particle_to_int(pico8_fixed_t x) {
    return (int)pico8_fixed_to_double(x);
}

/* function f_(self)self.qy={}
 * end
 * (pico8/lua.lua:685-686.) pico8_class_pq_fn-shaped already, no adapter --
 * see pico8_particle.h's own top comment. */
void pico8_f_(pico8_instance_t *self) {
    pico8_list_t *qy;

    if (self == NULL || self->fields == NULL) {
        return;
    }

    qy = malloc(sizeof *qy);
    pico8_list_init(qy);
    pico8_ob_table_set_name(self->fields, "qy", pico8_value_ref(qy));
}

/* function ez(self,n)while rnd()<n do
 * add(self.qy,self:mm())
 * n-=1
 * end
 * end
 * (pico8/lua.lua:687-691.) Never itself a dispatch-field value -- plain
 * signature. `self:mm()` is a genuinely polymorphic Lua method call,
 * routed through pico8_mv() (argc=0, both argument slots the established
 * "no real Lua value here" filler) rather than a direct call, since `ez`
 * is shared verbatim across classes that each define their own, different
 * `mm` -- see pico8_particle.h's own top comment for the full trace and
 * the "mm forward-reference status, confirmed inert today" note (no class
 * in scope yet defines `mm`, so this loop currently just burns iterations
 * without ever finding anything to add -- documented, not a bug). */
void pico8_ez(pico8_instance_t *self, pico8_fixed_t n) {
    pico8_list_t *qy;

    if (self == NULL) {
        return;
    }

    qy = pico8_particle_field_list(self, "qy");

    while (pico8_fixed_rnd_default() < n) {
        pico8_value_t result;
        if (qy != NULL && pico8_mv(self, "mm", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &result)) {
            pico8_list_add(qy, result);
        }
        n = pico8_fixed_sub(n, pico8_fixed_from_int(1));
    }
}

/* function ei(self)
 * if (self.fa<10 and not gd(self))return
 * local cx,cy,qo,qg,ro,q_,cs=
 * self.ra.x,self.ra.y,self.qo,self.qg,self.ro,self.q_,self.k_
 * for _,p in pairs(self.qy)do
 * local pv=p.v
 * p.p+=pv
 * p.v.x=qo*pv.x+ro*pv.y
 * p.v.y=qg*pv.x+q_*pv.y
 * if (p.a)p.v+=p.a
 * p.l-=0.14
 * if p.l<=1 then
 * del(self.qy,p)else
 * pset(cx+p.p.x,cy+p.p.y,cs[flr(p.l)])end
 * end
 * ez(self,self.fc)end
 * (pico8/lua.lua:692-707.) See pico8_particle.h's own top comment for the
 * dispatch-shape split (this is the plain business-logic half,
 * pico8_ei_dispatch below the field-shape bridge) and the particle-table
 * representation this chunk establishes.
 *
 * Two subtleties preserved literally, not "fixed":
 *   - `local pv=p.v` aliases the SAME pico8_vec_t Lua would (a plain
 *     table reference, not a copy) for the READS in `p.v.x=qo*pv.x+
 *     ro*pv.y`, but by the time the SECOND assignment
 *     (`p.v.y=qg*pv.x+q_*pv.y`) runs, `pv.x` in real Lua would already
 *     reflect the FIRST assignment's write (same object). This port keeps
 *     a plain `pv` snapshot struct (not an alias) but reproduces the same
 *     observable effect by explicitly reading the just-written `v_ptr->x`
 *     (not the snapshot's `pv.x`) for the second assignment's x term,
 *     while still using the snapshot's `pv.y` (unwritten so far, so
 *     identical to `v_ptr->y` at that point either way).
 *   - The real loop is `pairs()`, not `all()`: like `dg()`'s own `pairs
 *     (hv)` loop (pico8_entity.c), a `del()` mid-iteration has NO recovery
 *     -- the element that slides into the just-vacated slot is skipped
 *     that same tick. This port reproduces that by iterating `qy`'s raw
 *     backing array by plain index with no compensation, the exact same
 *     divergence already documented for `pico8_dg`, not a new one. */
void pico8_ei(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self) {
    pico8_fixed_t fa;
    pico8_vec_t ra;
    pico8_fixed_t cx;
    pico8_fixed_t cy;
    pico8_fixed_t qo;
    pico8_fixed_t qg;
    pico8_fixed_t ro;
    pico8_fixed_t q_;
    const pico8_ob_table_t *cs;
    pico8_list_t *qy;
    int i;

    if (self == NULL) {
        return;
    }

    fa = pico8_particle_field_fixed(self, "fa");
    if (fa < pico8_fixed_from_int(10) && !pico8_gd(self)) {
        return;
    }

    ra = pico8_particle_field_vec(self, "ra");
    cx = ra.x;
    cy = ra.y;
    qo = pico8_particle_field_fixed(self, "qo");
    qg = pico8_particle_field_fixed(self, "qg");
    ro = pico8_particle_field_fixed(self, "ro");
    q_ = pico8_particle_field_fixed(self, "q_");
    cs = pico8_particle_field_table(self, "k_");

    qy = pico8_particle_field_list(self, "qy");
    if (qy != NULL) {
        for (i = 0; i < qy->count; i++) {
            pico8_value_t p_value = qy->items[i];
            pico8_ob_table_t *p;
            pico8_value_t v_value;
            pico8_vec_t *v_ptr;
            pico8_vec_t pv;
            pico8_value_t p_pos_value;
            pico8_vec_t p_pos;
            pico8_vec_t *new_pos;
            pico8_fixed_t new_x;
            pico8_fixed_t new_y;
            pico8_value_t a_value;
            pico8_value_t l_value;
            pico8_fixed_t old_l;
            pico8_fixed_t new_l;

            if (p_value.kind != PICO8_VALUE_REF || p_value.as.ref == NULL) {
                continue;
            }
            p = (pico8_ob_table_t *)p_value.as.ref;

            if (!pico8_ob_table_get_name(p, "v", &v_value) || v_value.kind != PICO8_VALUE_REF || v_value.as.ref == NULL) {
                continue;
            }
            v_ptr = (pico8_vec_t *)v_value.as.ref;
            pv = *v_ptr;

            if (pico8_ob_table_get_name(p, "p", &p_pos_value) && p_pos_value.kind == PICO8_VALUE_REF &&
                p_pos_value.as.ref != NULL) {
                p_pos = *(const pico8_vec_t *)p_pos_value.as.ref;
            } else {
                p_pos = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
            }
            new_pos = malloc(sizeof *new_pos);
            *new_pos = pico8_vec_add(p_pos, pv);
            pico8_ob_table_set_name(p, "p", pico8_value_ref(new_pos));

            /* p.v.x=qo*pv.x+ro*pv.y -- pv.x/pv.y still both original here. */
            new_x = pico8_fixed_add(pico8_fixed_mul(qo, pv.x), pico8_fixed_mul(ro, pv.y));
            v_ptr->x = new_x;
            /* p.v.y=qg*pv.x+q_*pv.y -- real Lua's `pv.x` is now the JUST-
             * WRITTEN new_x (pv/p.v alias the same table); v_ptr->x
             * (not pv.x) reproduces that aliasing. pv.y is still original
             * (identical to v_ptr->y at this point either way). */
            new_y = pico8_fixed_add(pico8_fixed_mul(qg, v_ptr->x), pico8_fixed_mul(q_, pv.y));
            v_ptr->y = new_y;

            if (pico8_ob_table_get_name(p, "a", &a_value) && pico8_particle_truthy(a_value) &&
                a_value.kind == PICO8_VALUE_REF && a_value.as.ref != NULL) {
                pico8_vec_t *new_v = malloc(sizeof *new_v);
                *new_v = pico8_vec_add(*v_ptr, *(const pico8_vec_t *)a_value.as.ref);
                pico8_ob_table_set_name(p, "v", pico8_value_ref(new_v));
            }

            old_l = pico8_fixed_from_int(0);
            if (pico8_ob_table_get_name(p, "l", &l_value) && l_value.kind == PICO8_VALUE_FIXED) {
                old_l = l_value.as.fixed;
            }
            new_l = pico8_fixed_sub(old_l, pico8_fixed_from_double(0.14));
            pico8_ob_table_set_name(p, "l", pico8_value_fixed(new_l));

            if (new_l <= pico8_fixed_from_int(1)) {
                /* del(self.qy,p) -- pairs()+del() divergence, see this
                 * function's own top comment: no index compensation, same
                 * as pico8_dg's own established treatment. */
                pico8_list_del(qy, p_value);
            } else {
                int px = pico8_particle_to_int(pico8_fixed_add(cx, new_pos->x));
                int py = pico8_particle_to_int(pico8_fixed_add(cy, new_pos->y));
                int col = 0;
                pico8_value_t col_value;
                int idx = pico8_particle_to_int(pico8_fixed_flr(new_l));

                if (cs != NULL && pico8_ob_table_get_index(cs, idx, &col_value) && col_value.kind == PICO8_VALUE_FIXED) {
                    col = pico8_particle_to_int(col_value.as.fixed);
                }

                pico8_pset(gfx, mem, px, py, col);
            }
        }
    }

    pico8_ez(self, pico8_particle_field_fixed(self, "fc"));
}

/* Bridges pico8_value_function_t's (self,argc,arg0,arg1) shape to
 * pico8_ei's own signature, reading gfx/mem from the shared per-frame
 * dispatch context -- the same split as pico8_bg_kj/pico8_bg_kj_dispatch
 * (pico8_bg.c). This is what Chunk 10 wires verbatim as `g_.kj=ei`; see
 * pico8_particle.h's own top comment for the full rationale. dc()'s own
 * `entity.ra` second argument (`argc`/`arg0`/`arg1`) is never read here,
 * matching real Lua silently dropping an excess argument to a function
 * declared with fewer parameters (`ei` only declares `self`). */
pico8_value_t pico8_ei_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_ei(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}
