#include "pico8_ui_button.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_audio.h"
#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_geom.h"
#include "pico8_ke.h"
#include "pico8_layout.h"
#include "pico8_ref_audit.h"

static pico8_ob_table_t *g_js_spec = NULL;
static pico8_ob_table_t *g_fq_spec = NULL;

static int pico8_js_is_lq(const void *ref) {
    return pico8_type_tag_of(ref) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)ref)->cls == pico8_lq_struct_class();
}

/* The overrides table `lq(x)` copies from: NULL when the field is absent or
 * holds no REF, else the REF itself (borrowed) for a table, a freshly
 * allocated one for a list, or the row's table view for a row, which the
 * caller frees. All three shapes reach this field. */
static const pico8_ob_table_t *pico8_js_lq_arg_table(pico8_value_t v) {
    if (v.kind != PICO8_VALUE_REF || v.as.ref == NULL) {
        return NULL;
    }
    if (pico8_js_is_lq(v.as.ref)) {
        return pico8_lq_table_view((const pico8_lq_t *)v.as.ref);
    }

    switch (pico8_type_tag_of(v.as.ref)) {
    case PICO8_TYPE_TAG_OB_TABLE:
        return (const pico8_ob_table_t *)v.as.ref;
    case PICO8_TYPE_TAG_LIST:
        /* TROU-7 */
        return pico8_ob_table_from_list(PICO8_REF_AS_LIST_CONST(v.as.ref));
    default:
        assert(0 && "untagged or unmeasured REF reached js:pq's lq()/h_() argument");
        return NULL;
    }
}

static int pico8_ui_to_int(pico8_fixed_t x) {
    return pico8_fixed_flr_int(x);
}

static int pico8_js_truthy(pico8_value_t v) {
    return !(v.kind == PICO8_VALUE_REF && v.as.ref == NULL) && !(v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0);
}

static pico8_value_t pico8_js_ph_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_js_ph(self);
    return pico8_value_ref(NULL);
}

static pico8_value_t pico8_js_ev_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_js_ev(pico8_dispatch_gfx(), pico8_dispatch_mem(), self);
    return pico8_value_ref(NULL);
}

static pico8_value_t pico8_js_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_vec_t p = (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) ? *(const pico8_vec_t *)arg0.as.ref
                                                                           : pico8_vec_v(pico8_fixed_from_int(0),
                                                                                         pico8_fixed_from_int(0));
    (void)argc;
    (void)arg1;
    pico8_js_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), self, p);
    return pico8_value_ref(NULL);
}

/* `js.hd=gk`: a field assignment in the cart, not a method definition. */
static pico8_value_t pico8_js_hd_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_value_t mp = arg0;
    pico8_value_t rp = (argc >= 2) ? arg1 : arg0;
    return pico8_gk((const pico8_struct_header_t *)self, mp, rp);
}

static pico8_value_t pico8_js_ldown_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_js_ldown(self);
}

pico8_value_t pico8_js_h_(const pico8_js_t *self) {
    if (self == NULL || !self->has_h_) {
        return pico8_value_ref(NULL);
    }
    if (self->h_ == NULL) {
        return pico8_value_bool(0);
    }
    return pico8_value_ref(self->h_);
}

pico8_value_t pico8_js_my(const pico8_js_t *self) {
    if (self == NULL || !self->has_my) {
        return pico8_value_ref(NULL);
    }
    if (self->my == NULL) {
        return pico8_value_bool(0);
    }
    return pico8_value_function(self->my);
}

static int js_ref_field(void *ref, pico8_value_t *out_value) {
    if (ref == NULL) {
        return 0;
    }
    *out_value = pico8_value_ref(ref);
    return 1;
}

static int js_fixed_field(pico8_fixed_t fixed, int has, pico8_value_t *out_value) {
    if (!has) {
        return 0;
    }
    *out_value = pico8_value_fixed(fixed);
    return 1;
}

static int js_value_field(pico8_value_t v, pico8_value_t *out_value) {
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    *out_value = v;
    return 1;
}

static int pico8_js_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_js_t *js = self;

    if (strcmp(name, "fa") == 0) {
        *out_value = pico8_value_fixed(js->fa);
        return 1;
    }
    if (strcmp(name, "lq") == 0) {
        return js_ref_field(js->lq, out_value);
    }
    if (strcmp(name, "mb") == 0) {
        *out_value = pico8_value_fixed(js->mb);
        return 1;
    }
    if (strcmp(name, "kq") == 0) {
        return js_ref_field(js->kq, out_value);
    }
    if (strcmp(name, "h_") == 0) {
        return js_value_field(pico8_js_h_(js), out_value);
    }
    if (strcmp(name, "jb") == 0) {
        return js_ref_field(js->jb, out_value);
    }
    if (strcmp(name, "ra") == 0) {
        return js_ref_field(js->ra, out_value);
    }
    if (strcmp(name, "qf") == 0) {
        return js_ref_field(js->qf, out_value);
    }
    if (strcmp(name, "my") == 0) {
        return js_value_field(pico8_js_my(js), out_value);
    }
    if (strcmp(name, "m_") == 0) {
        if (!js->has_m_) {
            return 0;
        }
        *out_value = pico8_value_bool(js->m_);
        return 1;
    }
    if (strcmp(name, "hb") == 0) {
        if (js->hb == NULL) {
            return 0;
        }
        *out_value = pico8_value_function(js->hb);
        return 1;
    }
    if (strcmp(name, "fg") == 0) {
        return js_fixed_field(js->fg, js->has_fg, out_value);
    }
    if (strcmp(name, "px") == 0) {
        if (js->px == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(js->px);
        return 1;
    }
    if (strcmp(name, "trade") == 0) {
        return js_fixed_field(js->trade, js->has_trade, out_value);
    }
    if (strcmp(name, "en") == 0) {
        return js_fixed_field(js->en, js->has_en, out_value);
    }
    if (strcmp(name, "_cr_t") == 0) {
        return js_ref_field((void *)js->_cr_t, out_value);
    }
    if (strcmp(name, "_jo_fe_pt") == 0) {
        return js_ref_field(js->_jo_fe_pt, out_value);
    }
    if (strcmp(name, "_jo_fe_b") == 0) {
        return js_ref_field(js->_jo_fe_b, out_value);
    }
    if (strcmp(name, "_jo_fe_od") == 0) {
        return js_fixed_field(js->_jo_fe_od, js->has__jo_fe_od, out_value);
    }
    return pico8_ke_field(self, name, out_value);
}

static void button_take_bool(pico8_value_t value, int *out, int *has) {
    *out = 0;
    *has = 0;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_BOOL &&
           "fq.m_/p_, js.m_: a write neither nil nor a boolean does not fit the member's type "
           "-- stop, do not widen the member");
    *out = value.as.boolean;
    *has = 1;
}

static void button_take_function(pico8_value_t value, pico8_value_function_t *out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_FUNCTION && value.as.function != NULL &&
           "fq.my, js.hb: a key neither nil nor a non-NULL function does not fit the member's type "
           "-- stop, do not widen the member");
    *out = value.as.function;
}

static void button_take_my(pico8_value_t value, pico8_value_function_t *out, int *has) {
    *out = NULL;
    *has = 0;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(((value.kind == PICO8_VALUE_BOOL && value.as.boolean == 0) ||
            (value.kind == PICO8_VALUE_FUNCTION && value.as.function != NULL)) &&
           "js.my: a key neither nil, false, nor a non-NULL function does not fit the member's type "
           "-- stop, do not widen the member");
    if (value.kind == PICO8_VALUE_FUNCTION) {
        *out = value.as.function;
    }
    *has = 1;
}

static void button_take_fixed(pico8_value_t value, pico8_fixed_t *out, int *has) {
    *out = 0;
    *has = 0;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_FIXED &&
           "js.fg/trade/en/_jo_fe_od: a key neither nil nor a fixed does not fit the member's type "
           "-- stop, do not widen the member");
    *out = value.as.fixed;
    *has = 1;
}

static void button_take_fa(pico8_value_t value, pico8_fixed_t *out) {
    assert(value.kind == PICO8_VALUE_FIXED && "js.fa: a key that is not a fixed does not fit the member's type "
                                              "-- stop, do not widen the member");
    *out = value.as.fixed;
}

static void button_take_vec(pico8_value_t value, pico8_vec_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && value.as.ref != NULL &&
           pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_UNKNOWN &&
           "js.ra/qf: a key neither nil nor a vector ref does not fit the member's type "
           "-- stop, do not widen the member");
    *out = value.as.ref;
}

static void button_take_string(pico8_value_t value, const char **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_STRING && value.as.string != NULL &&
           "js.px: a key neither nil nor a string does not fit the member's type -- stop, do not widen the member");
    *out = value.as.string;
}

static void button_take_nb(pico8_value_t value, const pico8_nb_entry_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && value.as.ref != NULL &&
           pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_UNKNOWN &&
           "js._cr_t: a key neither nil nor an nb entry does not fit the member's type "
           "-- stop, do not widen the member");
    *out = value.as.ref;
}

static void button_take_jo(pico8_value_t value, pico8_jo_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)value.as.ref)->cls == pico8_jo_struct_class() &&
           "js._jo_fe_pt: a key neither nil nor a jo does not fit the member's type -- stop, do not widen the member");
    *out = value.as.ref;
}

static void button_take_hp(pico8_value_t value, pico8_hp_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)value.as.ref)->cls == pico8_hp_struct_class() &&
           "js._jo_fe_b: a key neither nil nor an hp does not fit the member's type -- stop, do not widen the member");
    *out = value.as.ref;
}

static void button_take_lq(pico8_value_t value, pico8_lq_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && value.as.ref != NULL && pico8_js_is_lq(value.as.ref) &&
           "js.lq: a write neither nil nor an lq row does not fit the member's type -- stop, do not widen the member");
    *out = value.as.ref;
}

/* `m_` (the hover loop) and `lq` (`ix[2].hb`): the only names a generic
 * writer ever writes on a `js`. */
static int pico8_js_set_field(void *self, const char *name, pico8_value_t value) {
    pico8_js_t *js = self;

    if (strcmp(name, "lq") == 0) {
        button_take_lq(value, &js->lq);
        return 1;
    }
    if (strcmp(name, "m_") == 0) {
        button_take_bool(value, &js->m_, &js->has_m_);
        return 1;
    }
    return 0;
}

static const pico8_struct_class_t g_js_struct_class = {
    {
        [PICO8_METHOD_HD] = pico8_js_hd_dispatch,
        [PICO8_METHOD_PH] = pico8_js_ph_dispatch,
        [PICO8_METHOD_EV] = pico8_js_ev_dispatch,
        [PICO8_METHOD_KJ] = pico8_js_kj_dispatch,
        [PICO8_METHOD_LDOWN] = pico8_js_ldown_dispatch,
    },
    pico8_js_field,
    pico8_js_set_field,
};

static void *pico8_js_struct_new(const pico8_ob_table_t *overrides) {
    pico8_js_t *self = malloc(sizeof *self);
    pico8_value_t lq_arg = pico8_value_ref(NULL);
    pico8_value_t h__arg = pico8_value_ref(NULL);
    pico8_value_t jb_arg = pico8_value_ref(NULL);
    int k;

    pico8_ke_struct_init(&self->ke, &g_js_struct_class);
    self->lq = NULL;
    self->mb = 0;
    self->kq = NULL;
    self->h_ = NULL;
    self->has_h_ = 0;
    self->jb = NULL;
    self->ra = NULL;
    self->qf = NULL;
    self->my = NULL;
    self->has_my = 0;
    self->m_ = 0;
    self->has_m_ = 0;
    self->hb = NULL;
    self->fg = 0;
    self->has_fg = 0;
    self->px = NULL;
    self->trade = 0;
    self->has_trade = 0;
    self->en = 0;
    self->has_en = 0;
    self->_cr_t = NULL;
    self->_jo_fe_pt = NULL;
    self->_jo_fe_b = NULL;
    self->_jo_fe_od = 0;
    self->has__jo_fe_od = 0;
    self->fa = pico8_fixed_from_int(11);
    for (k = 0; overrides != NULL && k < overrides->count; k++) {
        pico8_ob_key_t key = overrides->entries[k].key;
        pico8_value_t value = overrides->entries[k].value;
        const char *name = key.kind == PICO8_OB_KEY_NAME ? key.name : "";

        if ((strcmp(name, "h_") == 0 || strcmp(name, "jb") == 0) && value.kind == PICO8_VALUE_REF &&
            value.as.ref == NULL) {
            /* TROU-23 */
            value = pico8_value_bool(1);
        }
        if (strcmp(name, "fa") == 0) {
            button_take_fa(value, &self->fa);
        } else if (strcmp(name, "lq") == 0) {
            lq_arg = value;
        } else if (strcmp(name, "h_") == 0) {
            h__arg = value;
        } else if (strcmp(name, "jb") == 0) {
            jb_arg = value;
        } else if (strcmp(name, "ra") == 0) {
            button_take_vec(value, &self->ra);
        } else if (strcmp(name, "qf") == 0) {
            button_take_vec(value, &self->qf);
        } else if (strcmp(name, "my") == 0) {
            button_take_my(value, &self->my, &self->has_my);
        } else if (strcmp(name, "hb") == 0) {
            button_take_function(value, &self->hb);
        } else if (strcmp(name, "fg") == 0) {
            button_take_fixed(value, &self->fg, &self->has_fg);
        } else if (strcmp(name, "px") == 0) {
            button_take_string(value, &self->px);
        } else if (strcmp(name, "trade") == 0) {
            button_take_fixed(value, &self->trade, &self->has_trade);
        } else if (strcmp(name, "en") == 0) {
            button_take_fixed(value, &self->en, &self->has_en);
        } else if (strcmp(name, "_cr_t") == 0) {
            button_take_nb(value, &self->_cr_t);
        } else if (strcmp(name, "_jo_fe_pt") == 0) {
            button_take_jo(value, &self->_jo_fe_pt);
        } else if (strcmp(name, "_jo_fe_b") == 0) {
            button_take_hp(value, &self->_jo_fe_b);
        } else if (strcmp(name, "_jo_fe_od") == 0) {
            button_take_fixed(value, &self->_jo_fe_od, &self->has__jo_fe_od);
        } else {
            assert(0 && "js: the only construction keys are fa, lq, h_, jb, ra, qf, my, hb, fg, px, trade, en, _cr_t "
                        "and _jo_fe_pt/_b/_od; another key has no member -- stop, do not add one");
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("js");
#endif
    pico8_js_pq(self, lq_arg, h__arg, jb_arg);
    pico8_ke_struct_pq(&self->ke);
    return self;
}

void pico8_js_init(void) {
    pico8_js_shutdown();
    g_js_spec = pico8_ob("\nfa=11,\n", NULL);
}

void pico8_js_shutdown(void) {
    pico8_ob_table_free(g_js_spec);
    g_js_spec = NULL;
}

int pico8_js_ready(void) {
    return g_js_spec != NULL;
}

int pico8_js_default(const char *name, pico8_value_t *out_value) {
    return g_js_spec != NULL &&
           (pico8_ob_table_get_name(g_js_spec, name, out_value) || pico8_ke_default(name, out_value));
}

const pico8_struct_class_t *pico8_js_struct_class(void) {
    return &g_js_struct_class;
}

pico8_js_t *pico8_js_new(const pico8_ob_table_t *overrides) {
    return pico8_js_struct_new(overrides);
}

/* `lq(x)` of the cart: a row passed in is copied through its table view,
 * freed once the copy is built. */
static pico8_lq_t *pico8_js_lq(pico8_value_t v) {
    const pico8_ob_table_t *overrides = pico8_js_lq_arg_table(v);
    pico8_lq_t *built = pico8_lq_new(overrides);

    if (v.kind == PICO8_VALUE_REF && pico8_js_is_lq(v.as.ref)) {
        pico8_ob_table_free((pico8_ob_table_t *)overrides);
    }
    return built;
}

void pico8_js_pq(pico8_js_t *self, pico8_value_t lq, pico8_value_t h_, pico8_value_t jb) {
    if (self == NULL) {
        return;
    }

    self->lq = pico8_js_lq(lq);
    self->mb = pico8_fixed_add(self->lq->w, pico8_fixed_from_int(2));
    self->kq = pico8_qz(pico8_fixed_from_int(0), pico8_fixed_from_int(-1), self->mb, pico8_fixed_from_int(11));

    if (pico8_js_truthy(h_)) {
        self->h_ = pico8_js_lq(h_);
        self->has_h_ = 1;
    } else if (h_.kind == PICO8_VALUE_BOOL) {
        self->h_ = NULL;
        self->has_h_ = 1;
    }

    if (pico8_js_truthy(jb)) {
        const pico8_ob_table_t *jb_overrides =
            (jb.kind == PICO8_VALUE_REF) ? (const pico8_ob_table_t *)jb.as.ref : NULL;

        assert((jb_overrides == NULL || pico8_type_tag_of(jb_overrides) == PICO8_TYPE_TAG_OB_TABLE) &&
               "js:pq's jb field must hold a plain spec table");
        self->jb = pico8_jb_new(jb_overrides);
    } else if (jb.kind == PICO8_VALUE_BOOL) {
        assert(0 && "js.jb: a key false does not fit the member's type -- stop, do not widen the member");
    }
}

void pico8_js_ph(pico8_js_t *self) {
    if (self == NULL) {
        return;
    }

    if (self->qf != NULL) {
        pico8_vec_t qf = *self->qf;
        pico8_vec_t ra = self->ra != NULL ? *self->ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
        pico8_vec_t d = pico8_vec_sub(qf, ra);
        pico8_fixed_t len_sq = pico8_vec_len_sq(d);
        pico8_vec_t delta = (len_sq <= pico8_fixed_from_int(1)) ? d : pico8_vec_mul(d, PICO8_FIXED_LIT(0x00006666, 0.4));
        /* TROU-7 */
        pico8_vec_t *heap_ra = malloc(sizeof *heap_ra);
        *heap_ra = pico8_vec_add(ra, delta);
        self->ra = heap_ra;
    }
}

void pico8_js_ev(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_js_t *self) {
    if (self == NULL) {
        return;
    }

    if (self->jb != NULL && self->has_m_ && self->m_) {
        pico8_jb_ol(gfx, mem, self->jb, pico8_fixed_from_int(127), pico8_fixed_from_int(1));
    }
}

void pico8_js_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_js_t *self, pico8_vec_t p) {
    pico8_fixed_t mb;
    pico8_fixed_t qt;
    pico8_fixed_t dn;
    int has_my;
    int m_flag;
    int ix_p;
    int iy_p;
    int ix_qt;
    int iy_dn;

    if (self == NULL) {
        return;
    }

    mb = self->mb;
    qt = pico8_fixed_sub(pico8_fixed_add(p.x, mb), pico8_fixed_from_int(1));
    dn = pico8_fixed_add(p.y, pico8_fixed_from_int(9));
    has_my = self->my != NULL;
    m_flag = self->has_m_ && self->m_;

    ix_p = pico8_ui_to_int(p.x);
    iy_p = pico8_ui_to_int(p.y);
    ix_qt = pico8_ui_to_int(qt);
    iy_dn = pico8_ui_to_int(dn);

    pico8_rect(gfx, mem, ix_p - 1, iy_p, ix_qt + 1, iy_dn + 1, 1);
    pico8_rectfill(gfx, mem, ix_p, iy_p, ix_qt, iy_dn, has_my ? (m_flag ? 12 : 13) : 5);
    pico8_rectfill(gfx, mem, ix_p, iy_p, ix_qt, iy_p, has_my ? 6 : 5);

    if (self->lq != NULL) {
        pico8_vec_t off = pico8_vec_add(p, pico8_vec_v(pico8_fixed_from_int(1), pico8_fixed_from_int(2)));
        pico8_lq_ol_default(gfx, mem, self->lq, off);
    }

    if (self->h_ != NULL) {
        const pico8_lq_t *c = self->h_;
        pico8_fixed_t c_w = c->w;
        int bg = 2;
        pico8_fixed_t x2 = pico8_fixed_add(qt, c_w);

        if (c->has_bg && !c->bg_is_bool) {
            bg = pico8_ui_to_int(c->bg);
        }
        pico8_rectfill(gfx, mem, pico8_ui_to_int(qt) + 3, iy_p + 1, pico8_ui_to_int(x2) + 4, iy_dn, bg);

        {
            pico8_vec_t off2 =
                pico8_vec_add(p, pico8_vec_v(pico8_fixed_add(mb, pico8_fixed_from_int(3)), pico8_fixed_from_int(2)));
            pico8_lq_ol_default(gfx, mem, c, off2);
        }
    }
}

pico8_value_t pico8_js_ldown(pico8_js_t *self) {
    pico8_value_t discard;
    int has_my = self != NULL && self->my != NULL;

    pico8_rn(has_my ? pico8_value_fixed(pico8_fixed_from_int(60)) : pico8_value_bool(0));
    pico8_mv((pico8_struct_header_t *)self, "my", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &discard);

    return pico8_value_bool(1);
}

static pico8_value_t pico8_fq_ldown_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_fq_ldown((pico8_fq_t *)self);
}

static int pico8_fq_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_fq_t *fq = self;

    if (strcmp(name, "fa") == 0) {
        *out_value = pico8_value_fixed(fq->fa);
        return 1;
    }
    if (strcmp(name, "hd") == 0) {
        *out_value = pico8_value_fixed(fq->hd);
        return 1;
    }
    if (strcmp(name, "my") == 0) {
        if (fq->my == NULL) {
            return 0;
        }
        *out_value = pico8_value_function(fq->my);
        return 1;
    }
    if (strcmp(name, "p_") == 0) {
        if (!fq->has_p_) {
            return 0;
        }
        *out_value = pico8_value_bool(fq->p_);
        return 1;
    }
    if (strcmp(name, "m_") == 0) {
        if (!fq->has_m_) {
            return 0;
        }
        *out_value = pico8_value_bool(fq->m_);
        return 1;
    }
    return pico8_ke_field(self, name, out_value);
}

static int pico8_fq_set_field(void *self, const char *name, pico8_value_t value) {
    pico8_fq_t *fq = self;

    if (strcmp(name, "m_") == 0) {
        button_take_bool(value, &fq->m_, &fq->has_m_);
        return 1;
    }
    return 0;
}

static const pico8_struct_class_t g_fq_struct_class = {
    {[PICO8_METHOD_LDOWN] = pico8_fq_ldown_dispatch},
    pico8_fq_field,
    pico8_fq_set_field,
};

static void *pico8_fq_struct_new(const pico8_ob_table_t *overrides) {
    pico8_fq_t *self = malloc(sizeof *self);
    int i;

    pico8_ke_struct_init(&self->ke, &g_fq_struct_class);
    self->fa = pico8_fixed_from_int(14);
    self->hd = pico8_fixed_from_int(1);
    self->my = NULL;
    self->p_ = 0;
    self->has_p_ = 0;
    self->m_ = 0;
    self->has_m_ = 0;
    for (i = 0; overrides != NULL && i < overrides->count; i++) {
        pico8_ob_key_t key = overrides->entries[i].key;

        if (key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "my") == 0) {
            button_take_function(overrides->entries[i].value, &self->my);
        } else {
            assert(0 && "fq: the only key is my; another key has no member -- stop, do not add one");
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("fq");
#endif
    pico8_ke_struct_pq(&self->ke);
    return self;
}

void pico8_fq_init(void) {
    pico8_fq_shutdown();
    g_fq_spec = pico8_ob("\nfa=14,hd=1,\n", NULL);
}

void pico8_fq_shutdown(void) {
    pico8_ob_table_free(g_fq_spec);
    g_fq_spec = NULL;
}

int pico8_fq_ready(void) {
    return g_fq_spec != NULL;
}

int pico8_fq_default(const char *name, pico8_value_t *out_value) {
    return g_fq_spec != NULL &&
           (pico8_ob_table_get_name(g_fq_spec, name, out_value) || pico8_ke_default(name, out_value));
}

const pico8_struct_class_t *pico8_fq_struct_class(void) {
    return &g_fq_struct_class;
}

pico8_fq_t *pico8_fq_new(const pico8_ob_table_t *overrides) {
    return pico8_fq_struct_new(overrides);
}

pico8_value_t pico8_fq_ldown(pico8_fq_t *self) {
    pico8_value_t my_result;

    if (self == NULL) {
        return pico8_value_bool(1);
    }
    if (!pico8_mv((pico8_struct_header_t *)self, "my", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &my_result)) {
        my_result = pico8_value_ref(NULL);
    }
    button_take_bool(my_result, &self->p_, &self->has_p_);
    return pico8_value_bool(1);
}
