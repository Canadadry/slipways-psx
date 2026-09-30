#include "pico8_ui_button.h"

#include <stdlib.h>
#include <string.h>

#include "pico8_audio.h"
#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_geom.h"
#include "pico8_ke.h"
#include "pico8_layout.h"

static int g_js_up = 0;
static int g_fq_up = 0;

static int pico8_ui_to_int(pico8_fixed_t x) {
    return pico8_fixed_flr_int(x);
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
        js->hb(js);
    }
}

static int js_get_fa(const void *self, pico8_fixed_t *out) {
    *out = ((const pico8_js_t *)self)->fa;
    return 1;
}

static const pico8_vec_t *js_get_ra(const void *self) {
    return ((const pico8_js_t *)self)->ra;
}

static const pico8_qz_box_t *js_get_kq(const void *self) {
    return ((const pico8_js_t *)self)->kq;
}

static int js_get_fg(const void *self, pico8_fixed_t *out) {
    const pico8_js_t *js = self;

    if (!js->has_fg) {
        return 0;
    }
    *out = js->fg;
    return 1;
}

static void js_set_m_(void *self, int m_) {
    pico8_js_t *js = self;

    js->m_ = m_ != 0;
    js->has_m_ = 1;
}

static const pico8_struct_class_t g_js_struct_class = {
    .kj = pico8_js_kj_dispatch,
    .ev = pico8_js_ev_dispatch,
    .hd = pico8_js_hd_dispatch,
    .hb = pico8_js_hb_dispatch,
    .has_hb = pico8_js_has_hb,
    .ldown = pico8_js_ldown_dispatch,
    .states = {[PICO8_STATE_PH] = pico8_js_ph_dispatch},
    .get_fa = js_get_fa,
    .get_ra = js_get_ra,
    .get_kq = js_get_kq,
    .get_fg = js_get_fg,
    .set_m_ = js_set_m_,
};

static pico8_js_t *pico8_js_struct_new(const pico8_js_keys_t *keys) {
    pico8_js_t *self = malloc(sizeof *self);

    pico8_ke_struct_init(&self->header, &g_js_struct_class, pico8_js_def.me, pico8_js_def.t);
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
    self->def = &pico8_js_def;
    self->fa = self->def->fa;
    if (keys != NULL) {
        if (keys->has_fa) {
            self->fa = keys->fa;
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
    pico8_js_pq(self, keys);
    pico8_ke_struct_pq(&self->header);
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

/* `lq(x)` of the cart for `js.lq`: built from a row's keys, else a copy of
 * an `lq`, else an empty row. */
static pico8_lq_t *pico8_js_lq(const pico8_lq_t *copy, const pico8_lq_keys_t *row) {
    if (row != NULL) {
        return pico8_lq_new(row);
    }
    if (copy == NULL) {
        return pico8_lq_new(NULL);
    }
    return pico8_lq_copy(copy);
}

void pico8_js_pq(pico8_js_t *self, const pico8_js_keys_t *keys) {
    static const pico8_js_keys_t none;

    if (self == NULL) {
        return;
    }
    if (keys == NULL) {
        keys = &none;
    }

    self->lq = pico8_js_lq(keys->lq_copy, keys->lq_row);
    self->mb = pico8_fixed_add(self->lq->w, pico8_fixed_from_int(2));
    self->kq = pico8_qz(pico8_fixed_from_int(0), pico8_fixed_from_int(-1), self->mb, pico8_fixed_from_int(11));

    /* TROU-23 */
    if (keys->h_row != NULL || (keys->has_h_ && !keys->h_is_false)) {
        self->h_ = pico8_lq_new(keys->h_row);
        self->has_h_ = 1;
    } else if (keys->has_h_) {
        self->h_ = NULL;
        self->has_h_ = 1;
    }
    if (keys->jb_row != NULL || keys->has_jb) {
        self->jb = pico8_jb_new(keys->jb_row);
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
    int has_my = self != NULL && self->my != NULL;
    pico8_down_t out = {1, NULL};

    if (has_my) {
        pico8_rn(pico8_fixed_from_int(60));
        self->my(self);
    }

    return out;
}

static pico8_down_t pico8_fq_ldown_dispatch(void *self, pico8_vec_t *qa, pico8_vec_t *ra) {
    (void)qa;
    (void)ra;
    return pico8_fq_ldown((pico8_fq_t *)self);
}

static int fq_get_fa(const void *self, pico8_fixed_t *out) {
    *out = ((const pico8_fq_t *)self)->def->fa;
    return 1;
}

static int fq_get_p_(const void *self) {
    const pico8_fq_t *fq = self;

    return fq->has_p_ && fq->p_;
}

static void fq_set_m_(void *self, int m_) {
    pico8_fq_t *fq = self;

    fq->m_ = m_ != 0;
    fq->has_m_ = 1;
}

static const pico8_struct_class_t g_fq_struct_class = {
    .hd = pico8_hd_true,
    .ldown = pico8_fq_ldown_dispatch,
    .get_fa = fq_get_fa,
    .get_p_ = fq_get_p_,
    .set_m_ = fq_set_m_,
};

static pico8_fq_t *pico8_fq_struct_new(const pico8_fq_keys_t *keys) {
    pico8_fq_t *self = malloc(sizeof *self);

    pico8_ke_struct_init(&self->header, &g_fq_struct_class, pico8_fq_def.me, pico8_fq_def.t);
    self->def = &pico8_fq_def;
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
    pico8_ke_struct_pq(&self->header);
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
    pico8_down_t out = {1, NULL};

    if (self == NULL) {
        return out;
    }
    self->p_ = 0;
    self->has_p_ = 0;
    if (self->my != NULL) {
        self->p_ = self->my(self) != 0;
        self->has_p_ = 1;
    }
    return out;
}
