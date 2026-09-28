#include "pico8_help.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_data_init.h"
#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_ke.h"
#include "pico8_msgbox.h"
#include "pico8_particle.h"
#include "pico8_pi.h"
#include "pico8_probe.h"
#include "pico8_struct.h"
#include "pico8_type_tag.h"

static pico8_ob_table_t *g_fv_spec = NULL;
static pico8_ob_table_t *g_bo = NULL;

/* Returns a new, heap-backed list the caller owns and must release with
 * pico8_list_free(). */
static pico8_list_t pico8_help_positional_values(const pico8_ob_table_t *table) {
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

static pico8_value_t pico8_fv_eu_placeholder(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_value_ref(NULL);
}

static pico8_value_function_t g_eu_dispatch = pico8_fv_eu_placeholder;

pico8_value_t pico8_eu_field_value(void) {
    return pico8_value_function(g_eu_dispatch);
}

void pico8_help_set_eu(pico8_value_function_t fn) {
    g_eu_dispatch = (fn != NULL) ? fn : pico8_fv_eu_placeholder;
}

static pico8_value_t pico8_fv_hd_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    return pico8_gk((const pico8_struct_header_t *)self, arg0, arg1);
}

void pico8_fv_pq(pico8_fv_t *self) {
    int i;

    if (g_bo != NULL) {
        for (i = 0; i < g_bo->count; i++) {
            if (g_bo->entries[i].key.kind == PICO8_OB_KEY_INDEX && g_bo->entries[i].value.kind == PICO8_VALUE_REF &&
                g_bo->entries[i].value.as.ref != NULL) {
                pico8_ob_table_t *d = (pico8_ob_table_t *)g_bo->entries[i].value.as.ref;
                assert(pico8_type_tag_of(d) == PICO8_TYPE_TAG_OB_TABLE && "bo's rows must be plain spec tables");
                pico8_ob_table_set_name(d, "my", pico8_eu_field_value());
            }
        }
    }

    if (self != NULL) {
        pico8_list_t items = pico8_help_positional_values(g_bo);
        pico8_pz((const pico8_struct_header_t *)self, &items);
        pico8_list_free(&items);
    }
}

void pico8_fv_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_fv_t *self, pico8_vec_t p) {
    (void)p;

    pico8_ei(gfx, mem, (pico8_struct_header_t *)self);

    if (gfx != NULL && mem != NULL) {
        pico8_ob_table_t *rb_view = self != NULL && self->rb != NULL ? pico8_jb_row_view(self->rb) : NULL;
        pico8_list_t items = pico8_help_positional_values(rb_view);

        pico8_bl_default(gfx, mem, &items);
        pico8_list_free(&items);
        pico8_ob_table_free(rb_view);
    }
}

pico8_value_t pico8_fv_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_vec_t p = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        p = *(const pico8_vec_t *)arg0.as.ref;
    }
    pico8_fv_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_fv_t *)self, p);
    return pico8_value_ref(NULL);
}

static pico8_fixed_t g_fv_fa = 0;
static pico8_vec_t *g_fv_ra = NULL;
static pico8_fixed_t g_fv_fc = 0;
static pico8_fixed_t g_fv_qo = 0;
static pico8_fixed_t g_fv_ro = 0;
static pico8_fixed_t g_fv_qg = 0;
static pico8_fixed_t g_fv_q_ = 0;
static pico8_fixed_t g_fv_pn = 0;
static pico8_fixed_t g_fv_du = 0;
static pico8_fixed_t g_fv_py = 0;
static pico8_fixed_t g_fv_pk = 0;
static pico8_fixed_t g_fv_ki = 0;
static pico8_vec_t *g_fv_hi = NULL;
static pico8_fixed_t g_fv_ca = 0;
static pico8_vec_t *g_fv_gq = NULL;
static pico8_fixed_t g_fv_eo = 0;
static pico8_fixed_t g_fv_ff = 0;
static pico8_qz_box_t *g_fv_kq = NULL;
static pico8_fixed_t g_fv_ql = 0;
static pico8_fixed_t g_fv_fg = 0;

static void pico8_fv_defaults_reset(void) {
    g_fv_fa = 0;
    g_fv_ra = NULL;
    g_fv_fc = 0;
    g_fv_qo = 0;
    g_fv_ro = 0;
    g_fv_qg = 0;
    g_fv_q_ = 0;
    g_fv_pn = 0;
    g_fv_du = 0;
    g_fv_py = 0;
    g_fv_pk = 0;
    g_fv_ki = 0;
    g_fv_hi = NULL;
    g_fv_ca = 0;
    g_fv_gq = NULL;
    g_fv_eo = 0;
    g_fv_ff = 0;
    g_fv_kq = NULL;
    g_fv_ql = 0;
    g_fv_fg = 0;
}

static int pico8_fv_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_fv_t *fv = self;

    if (strcmp(name, "hi") == 0) {
        if (fv->hi == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref(fv->hi);
        return 1;
    }
    if (strcmp(name, "ca") == 0) {
        *out_value = pico8_value_fixed(fv->ca);
        return 1;
    }
    if (strcmp(name, "gq") == 0) {
        if (fv->gq == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref(fv->gq);
        return 1;
    }
    if (strcmp(name, "eo") == 0) {
        *out_value = pico8_value_fixed(fv->eo);
        return 1;
    }
    if (strcmp(name, "ff") == 0) {
        *out_value = pico8_value_fixed(fv->ff);
        return 1;
    }
    if (strcmp(name, "kq") == 0) {
        if (fv->kq == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref(fv->kq);
        return 1;
    }
    if (strcmp(name, "ql") == 0) {
        *out_value = pico8_value_fixed(fv->ql);
        return 1;
    }
    if (strcmp(name, "fg") == 0) {
        *out_value = pico8_value_fixed(fv->fg);
        return 1;
    }
    if (strcmp(name, "rb") == 0) {
        if (fv->rb == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref((void *)fv->rb);
        return 1;
    }
    if (strcmp(name, "m_") == 0) {
        if (!fv->has_m_) {
            return 0;
        }
        *out_value = pico8_value_bool(fv->m_);
        return 1;
    }
    return pico8_g__struct_class()->field(self, name, out_value);
}

static void help_take_bool(pico8_value_t value, int *out, int *has) {
    *out = 0;
    *has = 0;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_BOOL &&
           "fv.m_: a write neither nil nor a boolean does not fit the member's type -- stop, do not widen the member");
    *out = value.as.boolean;
    *has = 1;
}

/* `m_` (`gw`, hovering the help), then g_'s 5; 0 for any other. */
static int pico8_fv_set_field(void *self, const char *name, pico8_value_t value) {
    if (strcmp(name, "m_") == 0) {
        pico8_fv_t *fv = self;
        help_take_bool(value, &fv->m_, &fv->has_m_);
        return 1;
    }
    return pico8_g__struct_class()->set_field(self, name, value);
}

static const pico8_struct_class_t g_fv_struct_class = {
    {
        [PICO8_METHOD_HQ] = pico8_bv_dispatch,
        [PICO8_METHOD_EY] = pico8_d__dispatch,
        [PICO8_METHOD_EV] = pico8_dl_dispatch,
        [PICO8_METHOD_MM] = pico8_g__mm,
        [PICO8_METHOD_KJ] = pico8_fv_kj_dispatch,
        [PICO8_METHOD_HD] = pico8_fv_hd_dispatch,
    },
    pico8_fv_field,
    pico8_fv_set_field,
};

static void *pico8_fv_struct_new(const pico8_ob_table_t *overrides) {
    pico8_fv_t *self;

    assert(g_fv_spec != NULL && "pico8_help_init() must run first");
    self = malloc(sizeof *self);
    pico8_g__struct_init(&self->g_, &g_fv_struct_class);
    self->m_ = 0;
    self->has_m_ = 0;
    self->g_.fa = g_fv_fa;
    self->g_.ra = g_fv_ra;
    self->g_.fc = g_fv_fc;
    self->g_.k_ = &pico8_fv_k__seq;
    self->g_.qo = g_fv_qo;
    self->g_.ro = g_fv_ro;
    self->g_.qg = g_fv_qg;
    self->g_.q_ = g_fv_q_;
    self->g_.pn = g_fv_pn;
    self->g_.du = g_fv_du;
    self->g_.py = g_fv_py;
    self->g_.pk = g_fv_pk;
    self->g_.ki = g_fv_ki;
    self->hi = g_fv_hi;
    self->ca = g_fv_ca;
    self->gq = g_fv_gq;
    self->eo = g_fv_eo;
    self->ff = g_fv_ff;
    self->kq = g_fv_kq;
    self->ql = g_fv_ql;
    self->fg = g_fv_fg;
    self->rb = &pico8_fv_rb_row;
    assert((overrides == NULL || overrides->count == 0) &&
           "fv: the only road is pico8_fv_new(NULL) (cart.c); a construction key has no member "
           "-- stop, do not add one");
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("fv");
#endif
    pico8_fv_pq(self);
    pico8_f_((pico8_struct_header_t *)self);
    pico8_ke_struct_pq(&self->g_.ke);
    return self;
}

static pico8_fixed_t help_default_fixed(int found, pico8_value_t v) {
    assert(found && v.kind == PICO8_VALUE_FIXED &&
           "fv.fa/fc/qo/ro/qg/q_/pn/du/py/pk/ki/ca/eo/ff/ql/fg: a default of the struct is a key of fv's spec "
           "and a fixed (fixe)");
    (void)found;
    return v.as.fixed;
}

static pico8_vec_t *help_default_vec(int found, pico8_value_t v) {
    assert(found && v.kind == PICO8_VALUE_REF && v.as.ref != NULL &&
           pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_UNKNOWN &&
           "fv.ra/hi/gq: a default of the struct is a key of fv's spec and a vector v() (ref:autre)");
    (void)found;
    return v.as.ref;
}

static pico8_qz_box_t *help_default_box(int found, pico8_value_t v) {
    assert(found && v.kind == PICO8_VALUE_REF && v.as.ref != NULL &&
           pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_UNKNOWN &&
           "fv.kq: a default of the struct is a key of fv's spec and a box b() (ref:autre)");
    (void)found;
    return v.as.ref;
}

/* This module, unlike most, does not call pico8_pi_init() itself, and `fv`
 * extends `g_`, not `ke`: both preconditions are asserted, not repaired. */
void pico8_help_init(void) {
    pico8_value_t v;
    int found;

    assert(pico8_ob_lookup_constructor('v') != NULL && "pico8_help_init() runs after pico8_pi_init()");
    assert(pico8_g__ready() && "pico8_help_init() runs after pico8_probe_init()");

    pico8_help_shutdown();

    g_bo = pico8_bo_view();

    g_fv_spec = pico8_ob("\nfa=10,ra=v(64,28),\nhi=v(34,56),\nca=11,\ngq=v(0,11),\neo=-1,\nff=1,\nfc=5,"
                         "\nk_=o(0,1,1,2,4,9,10,9,4,2,1,1,1),\nqo=1.02,ro=-0.007,\nqg=0.007,q_=1.02,"
                         "\npn=5,du=15,\npy=0.0067,pk=1,\nki=17,\nkq=b(-40,78,40,98),\nql=11,fg=5,"
                         "\nrb=o(\no(192,0,11,16,4,fn=\"s\"),\no(172,20,107,2,2,fn=\"s\"),"
                         "\no(\"need help?\",40,110,13,fn=\"qr\"),\no(\"slipways.net/help\",40,116,5,fn=\"qr\"),\n),\n",
                         NULL);
    found = pico8_fv_default("hi", &v);
    g_fv_hi = help_default_vec(found, v);
    found = pico8_fv_default("ca", &v);
    g_fv_ca = help_default_fixed(found, v);
    found = pico8_fv_default("gq", &v);
    g_fv_gq = help_default_vec(found, v);
    found = pico8_fv_default("eo", &v);
    g_fv_eo = help_default_fixed(found, v);
    found = pico8_fv_default("ff", &v);
    g_fv_ff = help_default_fixed(found, v);
    found = pico8_fv_default("kq", &v);
    g_fv_kq = help_default_box(found, v);
    found = pico8_fv_default("ql", &v);
    g_fv_ql = help_default_fixed(found, v);
    found = pico8_fv_default("fg", &v);
    g_fv_fg = help_default_fixed(found, v);
    found = pico8_fv_default("fa", &v);
    g_fv_fa = help_default_fixed(found, v);
    found = pico8_fv_default("ra", &v);
    g_fv_ra = help_default_vec(found, v);
    found = pico8_fv_default("fc", &v);
    g_fv_fc = help_default_fixed(found, v);
    found = pico8_fv_default("qo", &v);
    g_fv_qo = help_default_fixed(found, v);
    found = pico8_fv_default("ro", &v);
    g_fv_ro = help_default_fixed(found, v);
    found = pico8_fv_default("qg", &v);
    g_fv_qg = help_default_fixed(found, v);
    found = pico8_fv_default("q_", &v);
    g_fv_q_ = help_default_fixed(found, v);
    found = pico8_fv_default("pn", &v);
    g_fv_pn = help_default_fixed(found, v);
    found = pico8_fv_default("du", &v);
    g_fv_du = help_default_fixed(found, v);
    found = pico8_fv_default("py", &v);
    g_fv_py = help_default_fixed(found, v);
    found = pico8_fv_default("pk", &v);
    g_fv_pk = help_default_fixed(found, v);
    found = pico8_fv_default("ki", &v);
    g_fv_ki = help_default_fixed(found, v);

    pico8_help_set_eu(NULL);
}

void pico8_help_shutdown(void) {
    pico8_fv_defaults_reset();
    pico8_ob_table_free(g_fv_spec);
    g_fv_spec = NULL;
    if (g_bo != NULL) {
        pico8_ob_table_free(g_bo);
        g_bo = NULL;
    }
    pico8_help_set_eu(NULL);
}

int pico8_fv_ready(void) {
    return g_fv_spec != NULL;
}

int pico8_fv_default(const char *name, pico8_value_t *out_value) {
    return g_fv_spec != NULL &&
           (pico8_ob_table_get_name(g_fv_spec, name, out_value) || pico8_g__default(name, out_value));
}

const pico8_struct_class_t *pico8_fv_struct_class(void) {
    return &g_fv_struct_class;
}

pico8_fv_t *pico8_fv_new(const pico8_ob_table_t *overrides) {
    return pico8_fv_struct_new(overrides);
}
