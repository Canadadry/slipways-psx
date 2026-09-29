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

static int g_js_up = 0;
static int g_fq_up = 0;

static int pico8_js_is_lq(const void *ref) {
    return pico8_type_tag_of(ref) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)ref)->cls == pico8_lq_struct_class();
}

static int pico8_ui_to_int(pico8_fixed_t x) {
    return pico8_fixed_flr_int(x);
}

static int pico8_js_truthy(pico8_value_t v) {
    return !(v.kind == PICO8_VALUE_REF && v.as.ref == NULL) && !(v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0);
}

static void pico8_js_ph_dispatch(void *self, pico8_fixed_t t) {
    (void)t;
    pico8_js_ph(self);
}

static void pico8_js_ev_dispatch(void *self, const pico8_vec_t *ra) {
    (void)ra;
    pico8_js_ev(pico8_dispatch_gfx(), pico8_dispatch_mem(), self);
}

static void pico8_js_kj_dispatch(void *self, const pico8_vec_t *ra) {
    pico8_vec_t p = ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    pico8_js_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), self, p);
}

/* `js.hd=gk`: a field assignment in the cart, not a method definition. */
static int pico8_js_hd_dispatch(void *self, const pico8_vec_t *qa, const pico8_vec_t *ra) {
    return pico8_gk((const pico8_struct_header_t *)self, qa, ra);
}

static pico8_down_t pico8_js_ldown_dispatch(void *self, pico8_vec_t *qa, pico8_vec_t *ra) {
    (void)qa;
    (void)ra;
    return pico8_js_ldown(self);
}

/* `js.hb` is the row's own callback, set by only one row of `ix`: a button
 * without one has no `hb` and stays out of `cv.hb`. */
static int pico8_js_has_hb(const void *self) {
    return ((const pico8_js_t *)self)->hb != NULL;
}

static void pico8_js_hb_dispatch(void *self) {
    pico8_js_t *js = self;

    if (js->hb != NULL) {
        (void)js->hb(self, 0, pico8_value_ref(NULL), pico8_value_ref(NULL));
    }
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
    .kj = pico8_js_kj_dispatch,
    .ev = pico8_js_ev_dispatch,
    .hd = pico8_js_hd_dispatch,
    .hb = pico8_js_hb_dispatch,
    .has_hb = pico8_js_has_hb,
    .ldown = pico8_js_ldown_dispatch,
    .states = {[PICO8_STATE_PH] = pico8_js_ph_dispatch},
    .field = pico8_js_field,
    .set_field = pico8_js_set_field,
};

static void *pico8_js_struct_new(const pico8_js_keys_t *keys) {
    pico8_js_t *self = malloc(sizeof *self);
    pico8_value_t lq_arg = pico8_value_ref(NULL);
    pico8_value_t h__arg = pico8_value_ref(NULL);
    pico8_value_t jb_arg = pico8_value_ref(NULL);

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
    self->fa = pico8_js_start.fa;
    if (keys != NULL) {
        if (keys->has_fa) {
            self->fa = keys->fa;
        }
        lq_arg = keys->lq;
        if (keys->has_h_) {
            h__arg = keys->h_;
        }
        if (keys->has_jb) {
            jb_arg = keys->jb;
        }
        /* TROU-23 */
        if (keys->has_h_ && h__arg.kind == PICO8_VALUE_REF && h__arg.as.ref == NULL) {
            h__arg = pico8_value_bool(1);
        }
        if (keys->has_jb && jb_arg.kind == PICO8_VALUE_REF && jb_arg.as.ref == NULL) {
            jb_arg = pico8_value_bool(1);
        }
        self->ra = keys->ra;
        self->qf = keys->qf;
        self->my = keys->my;
        self->has_my = keys->has_my;
        self->hb = keys->hb;
        self->fg = keys->fg;
        self->has_fg = keys->has_fg;
        self->px = keys->px;
        self->trade = keys->trade;
        self->has_trade = keys->has_trade;
        self->en = keys->en;
        self->has_en = keys->has_en;
        self->_cr_t = keys->_cr_t;
        self->_jo_fe_pt = keys->_jo_fe_pt;
        self->_jo_fe_b = keys->_jo_fe_b;
        self->_jo_fe_od = keys->_jo_fe_od;
        self->has__jo_fe_od = keys->has__jo_fe_od;
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("js");
#endif
    pico8_js_pq(self, lq_arg, keys != NULL ? keys->lq_row : NULL, h__arg, keys != NULL ? keys->h_row : NULL, jb_arg,
                keys != NULL ? keys->jb_row : NULL);
    pico8_ke_struct_pq(&self->ke);
    return self;
}

void pico8_js_init(void) {
    pico8_js_shutdown();
    g_js_up = 1;
}

void pico8_js_shutdown(void) {
    g_js_up = 0;
}

int pico8_js_ready(void) {
    return g_js_up;
}

const pico8_struct_class_t *pico8_js_struct_class(void) {
    return &g_js_struct_class;
}

pico8_js_t *pico8_js_new(const pico8_js_keys_t *keys) {
    return pico8_js_struct_new(keys);
}

/* `lq(x)` of the cart: a row is built from its keys (`row`, when set), a
 * list gives its elements, nil included, as the positionals, an `lq` is
 * copied from its keys; anything else not a REF builds an empty row. */
static pico8_lq_t *pico8_js_lq(pico8_value_t v, const pico8_lq_keys_t *row) {
    pico8_lq_keys_t keys = {0};

    if (row != NULL) {
        return pico8_lq_new(row);
    }
    if (v.kind != PICO8_VALUE_REF || v.as.ref == NULL) {
        return pico8_lq_new(NULL);
    }
    if (pico8_js_is_lq(v.as.ref)) {
        keys = pico8_lq_keys_of((const pico8_lq_t *)v.as.ref);
        return pico8_lq_new(&keys);
    }
    assert(pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_LIST &&
           "js:pq's lq()/h_() argument: a REF is a list or an lq; a row comes as keys");
    {
        const pico8_list_t *list = PICO8_REF_AS_LIST_CONST(v.as.ref);

        keys.items = list->items;
        keys.count = list->count;
        return pico8_lq_new(&keys);
    }
}

void pico8_js_pq(pico8_js_t *self, pico8_value_t lq, const pico8_lq_keys_t *lq_row, pico8_value_t h_,
                 const pico8_lq_keys_t *h_row, pico8_value_t jb, const pico8_jb_row_t *jb_row) {
    if (self == NULL) {
        return;
    }

    self->lq = pico8_js_lq(lq, lq_row);
    self->mb = pico8_fixed_add(self->lq->w, pico8_fixed_from_int(2));
    self->kq = pico8_qz(pico8_fixed_from_int(0), pico8_fixed_from_int(-1), self->mb, pico8_fixed_from_int(11));

    if (h_row != NULL || pico8_js_truthy(h_)) {
        self->h_ = pico8_js_lq(h_, h_row);
        self->has_h_ = 1;
    } else if (h_.kind == PICO8_VALUE_BOOL) {
        self->h_ = NULL;
        self->has_h_ = 1;
    }

    if (jb_row != NULL) {
        self->jb = pico8_jb_new(jb_row);
    } else if (pico8_js_truthy(jb)) {
        assert(jb.kind != PICO8_VALUE_REF && "js:pq's jb: a row comes as jb_row");
        self->jb = pico8_jb_new(NULL);
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

pico8_down_t pico8_js_ldown(pico8_js_t *self) {
    pico8_value_t discard;
    int has_my = self != NULL && self->my != NULL;
    pico8_down_t out = {1, NULL};

    pico8_rn(has_my ? pico8_value_fixed(pico8_fixed_from_int(60)) : pico8_value_bool(0));
    pico8_mv((pico8_struct_header_t *)self, "my", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &discard);

    return out;
}

static pico8_down_t pico8_fq_ldown_dispatch(void *self, pico8_vec_t *qa, pico8_vec_t *ra) {
    (void)qa;
    (void)ra;
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
    .hd = pico8_hd_true,
    .ldown = pico8_fq_ldown_dispatch,
    .field = pico8_fq_field,
    .set_field = pico8_fq_set_field,
};

static void *pico8_fq_struct_new(const pico8_fq_keys_t *keys) {
    pico8_fq_t *self = malloc(sizeof *self);

    pico8_ke_struct_init(&self->ke, &g_fq_struct_class);
    self->fa = pico8_fq_start.fa;
    self->hd = pico8_fq_start.hd;
    self->my = NULL;
    self->p_ = 0;
    self->has_p_ = 0;
    self->m_ = 0;
    self->has_m_ = 0;
    if (keys != NULL) {
        self->my = keys->my;
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("fq");
#endif
    pico8_ke_struct_pq(&self->ke);
    return self;
}

void pico8_fq_init(void) {
    pico8_fq_shutdown();
    g_fq_up = 1;
}

void pico8_fq_shutdown(void) {
    g_fq_up = 0;
}

int pico8_fq_ready(void) {
    return g_fq_up;
}

const pico8_struct_class_t *pico8_fq_struct_class(void) {
    return &g_fq_struct_class;
}

pico8_fq_t *pico8_fq_new(const pico8_fq_keys_t *keys) {
    return pico8_fq_struct_new(keys);
}

pico8_down_t pico8_fq_ldown(pico8_fq_t *self) {
    pico8_value_t my_result;
    pico8_down_t out = {1, NULL};

    if (self == NULL) {
        return out;
    }
    if (!pico8_mv((pico8_struct_header_t *)self, "my", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &my_result)) {
        my_result = pico8_value_ref(NULL);
    }
    button_take_bool(my_result, &self->p_, &self->has_p_);
    return out;
}
