#include "pico8_ui_button.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "pico8_audio.h"
#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_geom.h"
#include "pico8_ke.h"
#include "pico8_layout.h"
#include "pico8_ref_audit.h"

static pico8_class_t *g_js_class = NULL;
static pico8_class_t *g_fq_class = NULL;

/* The overrides table `lq(x)` copies from: NULL when the field is absent or
 * holds no REF, else the REF itself (borrowed) for an instance or a table, or
 * a freshly allocated one for a list. All three shapes reach this field. */
static const pico8_ob_table_t *pico8_js_lq_arg_table(pico8_value_t v) {
    if (v.kind != PICO8_VALUE_REF || v.as.ref == NULL) {
        return NULL;
    }

    switch (pico8_type_tag_of(v.as.ref)) {
    case PICO8_TYPE_TAG_INSTANCE:
        return ((const pico8_instance_t *)v.as.ref)->fields;
    case PICO8_TYPE_TAG_OB_TABLE:
        return (const pico8_ob_table_t *)v.as.ref;
    case PICO8_TYPE_TAG_LIST:
        /* TROU-7 */
        return pico8_ob_table_from_list(PICO8_REF_AS_LIST_CONST(v.as.ref));
    default:
        assert(0 && "untagged REF reached js:pq's lq()/h_() argument");
        return NULL;
    }
}

/* Field readers: an absent field, a wrong kind, or a NULL instance all read as
 * the zero/false default. */
static int pico8_ui_to_int(pico8_fixed_t x) {
    return pico8_fixed_flr_int(x);
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

static void pico8_js_pq_dispatch(pico8_instance_t *instance) {
    pico8_js_pq(instance);
}

static pico8_value_t pico8_js_ph_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_js_ph((pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

static pico8_value_t pico8_js_ev_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_js_ev(pico8_dispatch_gfx(), pico8_dispatch_mem(), (const pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

static pico8_value_t pico8_js_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_vec_t p = (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) ? *(const pico8_vec_t *)arg0.as.ref
                                                                           : pico8_vec_v(pico8_fixed_from_int(0),
                                                                                         pico8_fixed_from_int(0));
    (void)argc;
    (void)arg1;
    pico8_js_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (const pico8_instance_t *)self, p);
    return pico8_value_ref(NULL);
}

static pico8_value_t pico8_js_hd_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_value_t mp = arg0;
    pico8_value_t rp = (argc >= 2) ? arg1 : arg0;
    return pico8_gk((const pico8_instance_t *)self, mp, rp);
}

static pico8_value_t pico8_js_ldown_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_js_ldown((pico8_instance_t *)self);
}

void pico8_js_init(void) {
    pico8_js_shutdown();
    g_js_class = pico8_class_declare(pico8_ke_class(), "\nfa=11,\n");
    PICO8_CLASS_COVERAGE_NAME(g_js_class, "js");
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

void pico8_js_pq(pico8_instance_t *self) {
    /* pico8_mv() leaves *out_value untouched when the field is absent, so this
     * init is what makes the no-"lq"-override path well-defined. */
    pico8_value_t lq_overrides_val = pico8_value_ref(NULL);
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

    pico8_mv(self, "lq", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &lq_overrides_val);
    lq_overrides = pico8_js_lq_arg_table(lq_overrides_val);
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
        const pico8_ob_table_t *h_overrides = pico8_js_lq_arg_table(h_val);
        pico8_instance_t *new_h = pico8_lq_new(h_overrides);
        pico8_ob_table_set_name(self->fields, "h_", pico8_value_ref(new_h));
    }

    if (pico8_instance_get_field(self, "jb", &jb_val) &&
        !(jb_val.kind == PICO8_VALUE_BOOL && jb_val.as.boolean == 0)) {
        const pico8_ob_table_t *jb_overrides =
            (jb_val.kind == PICO8_VALUE_REF) ? (const pico8_ob_table_t *)jb_val.as.ref : NULL;
        pico8_jb_t *new_jb;

        assert((jb_overrides == NULL || pico8_type_tag_of(jb_overrides) == PICO8_TYPE_TAG_OB_TABLE) &&
               "js:pq's jb field must hold a plain spec table");
        new_jb = pico8_jb_new(jb_overrides);
        pico8_ob_table_set_name(self->fields, "jb", pico8_value_ref(new_jb));
    }
}

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
        pico8_vec_t delta = (len_sq <= pico8_fixed_from_int(1)) ? d : pico8_vec_mul(d, PICO8_FIXED_LIT(0x00006666, 0.4));
        pico8_vec_t new_ra = pico8_vec_add(ra, delta);
        /* TROU-7 */
        pico8_vec_t *heap_ra = malloc(sizeof *heap_ra);
        *heap_ra = new_ra;
        pico8_ob_table_set_name(self->fields, "ra", pico8_value_ref(heap_ra));
    }
}

void pico8_js_ev(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self) {
    pico8_value_t jb_val;

    if (self == NULL) {
        return;
    }

    if (pico8_instance_get_field(self, "jb", &jb_val) && jb_val.kind == PICO8_VALUE_REF && jb_val.as.ref != NULL &&
        pico8_ui_field_truthy(self, "m_")) {
        assert(pico8_type_tag_of(jb_val.as.ref) == PICO8_TYPE_TAG_STRUCT &&
               "js:ev: the jb field must hold the jb struct js:pq built, not a spec table");
        pico8_jb_ol(gfx, mem, (const pico8_jb_t *)jb_val.as.ref, pico8_fixed_from_int(127), pico8_fixed_from_int(1));
    }
}

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

pico8_value_t pico8_js_ldown(pico8_instance_t *self) {
    pico8_value_t discard;
    int has_my = pico8_ui_field_truthy(self, "my");

    pico8_rn(has_my ? pico8_value_fixed(pico8_fixed_from_int(60)) : pico8_value_bool(0));
    pico8_mv(self, "my", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &discard);

    return pico8_value_bool(1);
}

static pico8_value_t pico8_fq_ldown_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_fq_ldown((pico8_instance_t *)self);
}

void pico8_fq_init(void) {
    pico8_fq_shutdown();
    g_fq_class = pico8_class_declare(pico8_ke_class(), "\nfa=14,hd=1,\n");
    PICO8_CLASS_COVERAGE_NAME(g_fq_class, "fq");
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

pico8_value_t pico8_fq_ldown(pico8_instance_t *self) {
    pico8_value_t my_result;

    if (self == NULL) {
        return pico8_value_bool(1);
    }
    /* TROU-9 */
    if (pico8_mv(self, "my", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &my_result)) {
        pico8_ob_table_set_name(self->fields, "p_", my_result);
    }
    return pico8_value_bool(1);
}
