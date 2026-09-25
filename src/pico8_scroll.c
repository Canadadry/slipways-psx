#include "pico8_scroll.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "pico8_dispatch_context.h"
#include "pico8_entity_geom.h"
#include "pico8_ke.h"
#include "pico8_pi.h"
#include "pico8_value.h"
#include "pico8_vec.h"

static pico8_class_t *g_qb_class = NULL;

/* Absent, wrong-kind and NULL-instance all read as the zero vector. */
static pico8_vec_t pico8_qb_field_vec(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

/* dg() dispatches the per-tick state method as `qc[qc.me](qc,qc.t)`, so
 * arg0 is the frame counter, which qb:ph does not read. */
static pico8_value_t pico8_qb_ph_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_qb_ph(pico8_dispatch_input(), (pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

/* Resolves Lua's `fy or 1` here, so pico8_qb_md itself always gets a real
 * factor; dc()'s "kj" branch passes `ci[o]` as arg0. */
static pico8_value_t pico8_qb_md_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_fixed_t fy = (arg0.kind == PICO8_VALUE_FIXED) ? arg0.as.fixed : pico8_fixed_from_int(1);
    (void)argc;
    (void)arg1;
    pico8_qb_md(pico8_dispatch_gfx(), (pico8_instance_t *)self, fy);
    return pico8_value_ref(NULL);
}

void pico8_qb_init(void) {
    pico8_qb_shutdown();
    pico8_pi_init();
    assert(pico8_ob_lookup_constructor('v') != NULL && "qb's spec parses v(...) as vectors");
    assert(pico8_ke_class() != NULL && "qb descends from ke");
    g_qb_class = pico8_class_declare(pico8_ke_class(),
                                      "\nv=v(0,0),ma=0,p=v(192,192),ra=v(192,192),"
                                      "pc=o(v(-1,0),v(1,0),v(0,-1),v(0,1)),\n");
    PICO8_CLASS_COVERAGE_NAME(g_qb_class, "qb");
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
        /* TROU-2 */
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
    v = pico8_vec_add(v, pico8_vec_mul(pico8_vec_sub(ip, v), PICO8_FIXED_LIT(0x00003333, 0.2)));

    p.x = pico8_fixed_mid(pico8_fixed_from_int(-64), pico8_fixed_from_int(320), p.x);
    p.y = pico8_fixed_mid(pico8_fixed_from_int(-64), pico8_fixed_from_int(320), p.y);

    /* TROU-7 */
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

void pico8_qb_md(pico8_gfx_t *gfx, pico8_instance_t *self, pico8_fixed_t fy) {
    pico8_vec_t ra;
    pico8_vec_t cp;

    if (self == NULL || gfx == NULL) {
        return;
    }

    ra = pico8_qb_field_vec(self, "ra");
    cp = pico8_vec_mul(ra, fy);
    pico8_camera(gfx, pico8_fixed_flr_int(cp.x), pico8_fixed_flr_int(cp.y));
}
