#include "pico8_scroll.h"

#include <stddef.h>
#include <stdlib.h>

#include "pico8_dispatch_context.h"
#include "pico8_entity_geom.h"
#include "pico8_ke.h"
#include "pico8_pi.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* See pico8_scroll.h's own top comment for the full source/trace. */

static pico8_class_t *g_qb_class = NULL;

/* Small per-module defensive vector-field reader, mirroring
 * pico8_entity_geom.c's own private pico8_entity_geom_field_vec (not
 * exported there, so re-declared here rather than widening that module's
 * header surface for a 4-line helper -- same reasoning that file's own
 * comment already gives for not sharing pico8_ob.c's/pico8_value.c's own
 * private strdup-like copies either). Absent/wrong-kind/NULL-instance all
 * read as the zero vector. */
static pico8_vec_t pico8_qb_field_vec(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

/* Bridges pico8_value_function_t's (self,argc,arg0,arg1) shape (Phase 3
 * Chunk 5's own arity extension, pico8_value.h) to pico8_qb_ph's own real
 * signature; dg()'s own per-tick dispatch (`qc[qc.me](qc,qc.t)`) always
 * passes argc=1 with `qc.t` as arg0, unused here, matching the real Lua
 * body's own arity. See pico8_scroll.h's own comment on pico8_qb_ph. */
static pico8_value_t pico8_qb_ph_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_qb_ph(pico8_dispatch_input(), (pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

/* Bridges pico8_value_function_t's (self,argc,arg0,arg1) shape to
 * pico8_qb_md's own real signature, resolving `fy or 1` here (arg0 not
 * PICO8_VALUE_FIXED -- the never-evidenced omitted-fy case -- defaults to
 * 1, matching the real Lua fallback exactly). This chunk's own real call
 * site (dc()'s "kj" branch, `mo:md(ci[o])`, pico8_entity.c) always passes
 * argc=1 with a genuine arg0, so `arg1` is unused. See pico8_scroll.h's
 * own comment on pico8_qb_md for why this is also the "kj" dead-branch
 * connection. */
static pico8_value_t pico8_qb_md_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_fixed_t fy = (arg0.kind == PICO8_VALUE_FIXED) ? arg0.as.fixed : pico8_fixed_from_int(1);
    (void)argc;
    (void)arg1;
    pico8_qb_md(pico8_dispatch_gfx(), (pico8_instance_t *)self, fy);
    return pico8_value_ref(NULL);
}

/* qb=ke:jr([[
 * v=v(0,0),ma=0,p=v(192,192),ra=v(192,192),
 * pc=o(v(-1,0),v(1,0),v(0,-1),v(0,1)),
 * ]]) -- see pico8_scroll.h's own top comment. Calls pico8_pi_init()
 * defensively before declaring the class (same real ordering hazard, and
 * same fix, as pico8_mouse_reader.c's own pico8_nf_init() -- see that
 * function's own comment for the full rationale). */
void pico8_qb_init(void) {
    pico8_qb_shutdown();
    pico8_pi_init();
    g_qb_class = pico8_class_declare(pico8_ke_class(),
                                      "\nv=v(0,0),ma=0,p=v(192,192),ra=v(192,192),"
                                      "pc=o(v(-1,0),v(1,0),v(0,-1),v(0,1)),\n");
    pico8_ob_table_set_name(g_qb_class->defaults, "ph", pico8_value_function(pico8_qb_ph_dispatch));
    pico8_ob_table_set_name(g_qb_class->defaults, "md", pico8_value_function(pico8_qb_md_dispatch));
}

void pico8_qb_shutdown(void) {
    if (g_qb_class != NULL) {
        pico8_class_free(g_qb_class);
        g_qb_class = NULL;
    }
}

pico8_class_t *pico8_qb_class(void) {
    return g_qb_class;
}

pico8_instance_t *pico8_qb_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_qb_class, overrides);
}

/* function qb:ph()local ip=v(0,0)for b=0,3 do
 * if btn(b)or btn(b,1)then
 * ip+=self.pc[b+1]*3
 * end
 * end
 * self.p+=self.v
 * self.v+=(ip-self.v)*0.2
 * self.p.x=mid(-64,320,self.p.x)self.p.y=mid(-64,320,self.p.y)self.ra=ma(self.p,self)end
 * -- see pico8_scroll.h's own comment on this declaration for the full
 * trace, including the field-rebind-vs-in-place-mutation reasoning. */
void pico8_qb_ph(const pico8_input_t *input, pico8_instance_t *self) {
    pico8_vec_t ip = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    pico8_value_t pc_val;
    const pico8_ob_table_t *pc_table = NULL;
    pico8_vec_t p;
    pico8_vec_t v;
    pico8_vec_t *p_new;
    pico8_vec_t *v_new;
    pico8_vec_t *ra_new;
    int b;

    if (self == NULL || input == NULL) {
        return;
    }

    if (pico8_instance_get_field(self, "pc", &pc_val) && pc_val.kind == PICO8_VALUE_REF && pc_val.as.ref != NULL) {
        pc_table = (const pico8_ob_table_t *)pc_val.as.ref;
    }

    for (b = 0; b <= 3; b++) {
        int btn_p1 = pico8_btn(input, b);
        /* btn(b,1): second-controller check. ALREADY-DOCUMENTED project
         * decision (PRD Implementation Decisions / User Story 9) -- no
         * second-player representation exists in pico8_input_t/
         * events.json, and none is being added, so this is hardcoded
         * false rather than extending pico8_btn's own signature. */
        int btn_p2 = 0;

        if (btn_p1 || btn_p2) {
            pico8_vec_t dir = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
            pico8_value_t dir_val;
            if (pc_table != NULL && pico8_ob_table_get_index(pc_table, b + 1, &dir_val) &&
                dir_val.kind == PICO8_VALUE_REF && dir_val.as.ref != NULL) {
                dir = *(const pico8_vec_t *)dir_val.as.ref;
            }
            ip = pico8_vec_add(ip, pico8_vec_mul(dir, pico8_fixed_from_int(3)));
        }
    }

    p = pico8_qb_field_vec(self, "p");
    v = pico8_qb_field_vec(self, "v");

    p = pico8_vec_add(p, v);
    v = pico8_vec_add(v, pico8_vec_mul(pico8_vec_sub(ip, v), pico8_fixed_from_double(0.2)));

    p.x = pico8_fixed_mid(pico8_fixed_from_int(-64), pico8_fixed_from_int(320), p.x);
    p.y = pico8_fixed_mid(pico8_fixed_from_int(-64), pico8_fixed_from_int(320), p.y);

    p_new = malloc(sizeof *p_new);
    *p_new = p;
    v_new = malloc(sizeof *v_new);
    *v_new = v;
    pico8_ob_table_set_name(self->fields, "p", pico8_value_ref(p_new));
    pico8_ob_table_set_name(self->fields, "v", pico8_value_ref(v_new));

    ra_new = malloc(sizeof *ra_new);
    *ra_new = pico8_ma(p, self);
    pico8_ob_table_set_name(self->fields, "ra", pico8_value_ref(ra_new));
}

/* function qb:md(fy)local cp=self.ra*(fy or 1)camera(cp.x,cp.y)end
 * -- see pico8_scroll.h's own comment on this declaration for the full
 * trace, including the dc("kj")-dead-branch connection. */
void pico8_qb_md(pico8_gfx_t *gfx, pico8_instance_t *self, pico8_fixed_t fy) {
    pico8_vec_t ra;
    pico8_vec_t cp;

    if (self == NULL || gfx == NULL) {
        return;
    }

    ra = pico8_qb_field_vec(self, "ra");
    cp = pico8_vec_mul(ra, fy);
    pico8_camera(gfx, (int)pico8_fixed_to_double(cp.x), (int)pico8_fixed_to_double(cp.y));
}
