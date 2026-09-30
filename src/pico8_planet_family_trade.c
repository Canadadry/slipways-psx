#include "pico8_planet_family_trade.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_struct.h"
#include "pico8_entity.h"
#include "pico8_particle.h"
#include "pico8_pi.h"
#include "pico8_planet.h"
#include "pico8_planet_family.h"
#include "pico8_planet_trade.h"
#include "pico8_probe.h"
#include "pico8_slipway.h"
#include "pico8_trade.h"

/* What `mx:kf` leaves in `ht` once it accepts a resource: `o()`. */
static const pico8_int_seq_t g_mx_ht_spent = {0, NULL};

static int g_family_trade_up = 0;

pico8_down_t pico8_nr_gv(const pico8_nr_t *self, const pico8_struct_header_t *s) {
    int qu_not_self;
    int oe_count = 0;
    pico8_down_t out = {0, NULL};

    if (self == NULL) {
        return out;
    }

    /* Only a held `jc` carries `qu`: a held `bg`, `mz` or planet reads it absent. */
    qu_not_self = !(s != NULL && s->cls == pico8_jc_struct_class() &&
                    ((const pico8_jc_t *)s)->qu == &self->qh.jo.header);

    oe_count = self->qh.jo.oe != NULL ? self->qh.jo.oe->count : 0;

    out.stop = qu_not_self && oe_count < 3;
    return out;
}

static pico8_down_t pico8_nr_gv_dispatch(void *self, const pico8_struct_header_t *s) {
    return pico8_nr_gv((const pico8_nr_t *)self, s);
}

static const pico8_struct_class_t g_nr_ldown_empty_struct_class = {0};

typedef struct {
    pico8_struct_header_t header;
} pico8_nr_ldown_empty_t;

/* The `{}` `nr:ldown` hands to `nr:gv`: never allocated, never freed,
 * never written. */
static const pico8_nr_ldown_empty_t g_nr_ldown_empty = {{.cls = &g_nr_ldown_empty_struct_class}};

pico8_down_t pico8_nr_ldown(pico8_nr_t *self) {
    pico8_down_t gv_result = {0, NULL};
    pico8_jc_keys_t keys = {0};

    if (self == NULL) {
        return gv_result;
    }

    gv_result = pico8_nr_gv(self, (const pico8_struct_header_t *)&g_nr_ldown_empty);

    if (!gv_result.stop) {
        return gv_result;
    }

    assert(pico8_jc_ready() && "pico8_jc_init() must run before nr:ldown builds a jc");
    keys.qu = (pico8_struct_header_t *)self;
    gv_result.held = (pico8_struct_header_t *)pico8_jc_new(&keys);
    gv_result.stop = gv_result.held != NULL;
    return gv_result;
}

static pico8_down_t pico8_nr_ldown_dispatch(void *self, pico8_vec_t *qa, pico8_vec_t *ra) {
    (void)qa;
    (void)ra;
    return pico8_nr_ldown((pico8_nr_t *)self);
}

void pico8_nr_hb(pico8_nr_t *self) {
    int oe_count = 0;
    pico8_fixed_t s_val = pico8_fixed_from_int(0);

    if (self == NULL) {
        return;
    }

    oe_count = self->qh.jo.oe != NULL ? self->qh.jo.oe->count : 0;

    if (self->hz != NULL) {
        int entry;
        if (pico8_int_seq_at(self->hz->items, self->hz->count, oe_count + 1, &entry)) {
            s_val = pico8_fixed_from_int(entry);
        }
    }

    self->qh.s = s_val;
}

static void pico8_nr_hb_dispatch(void *self) {
    pico8_nr_hb((pico8_nr_t *)self);
}

void pico8_nr_cn(pico8_nr_t *self, pico8_jo_trade_proposal_t *lw) {
    pico8_trade_t *t;
    const pico8_ents_t *oe = NULL;
    pico8_ents_iter_t it;
    pico8_struct_header_t *w;

    if (self == NULL || lw == NULL) {
        return;
    }
    t = lw->t;

    oe = self->qh.jo.oe;

    it = pico8_ents_all(oe);
    while (pico8_ents_iter_next(&it, &w)) {
        const pico8_ents_t *t_rl = NULL;

        if (w == NULL) {
            continue;
        }

        if (lw->mu <= pico8_fixed_from_int(0)) {
            continue;
        }

        if (t != NULL && t->rl != NULL) {
            t_rl = t->rl;
        }
        if (pico8_ents_has(t_rl, w)) {
            continue;
        }

        {
            pico8_jo_trade_proposal_t gf = *lw;
            pico8_trade_t *extended = pico8_trade_kg(t, &self->qh.jo.header, w);
            pico8_struct_header_t *other;

            gf.t = extended;
            assert(w->cls == pico8_jc_struct_class() && "nr.oe holds slipways only: jo:kf adds to it");
            other = pico8_jc_eb((const pico8_jc_t *)w, (const pico8_struct_header_t *)self);
            /* `&gf` is a stack address passed to a dispatched `cn`: safe
             * only while no `cn` implementation stores its proposal. */
            if (other != NULL && other->cls->cn != NULL) {
                other->cls->cn(other, &gf);
            }
            lw->mu = gf.mu;
        }
    }
}

static void pico8_nr_cn_dispatch(void *self, struct pico8_jo_trade_proposal *lw) {
    pico8_nr_cn((pico8_nr_t *)self, lw);
}

static const pico8_struct_class_t g_nr_struct_class = {
    .kj = pico8_jo_kj_dispatch,
    .ev = pico8_dl_dispatch,
    .hd = pico8_jo_hd_dispatch,
    .hk = pico8_jo_hk_dispatch,
    .hb = pico8_nr_hb_dispatch,
    .ldown = pico8_nr_ldown_dispatch,
    .gv = pico8_nr_gv_dispatch,
    .hq = pico8_bv_dispatch,
    .ey = pico8_d__dispatch,
    .kf = pico8_jo_kf_dispatch,
    .cn = pico8_nr_cn_dispatch,
    .states =
        {
            [PICO8_STATE_LL] = pico8_jo_ll_dispatch,
            [PICO8_STATE_LY] = pico8_qh_ly_dispatch,
        },
    .parent = pico8_qh_struct_class,
    .probe_of = pico8_qh_probe_of,
    .tags = PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
    .get_fa = pico8_jo_get_fa,
    .get_ra = pico8_jo_get_ra,
    .get_p = pico8_jo_get_p,
    .get_kq = pico8_jo_get_kq,
    .get_fx = pico8_jo_get_fx,
    .get_p_ = pico8_jo_get_p_,
    .set_m_ = pico8_jo_set_m_,
};

/* Initializes an allocated pico8_nr_t under `cls`: qh's part, nr's members
 * nil, nr's defaults. Reads no key, runs neither `pq` nor `lt`, records no
 * coverage. Requires `pico8_planet_family_trade_init()`. */
static void pico8_nr_struct_init(pico8_nr_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
    pico8_qh_struct_init(&self->qh, cls, def);
    self->hz = pico8_nr_start.hz;
    self->rr = pico8_nr_start.rr;
    self->qh.probe->od = pico8_nr_start.od;
    self->qh.probe->fp = pico8_nr_start.fp;
    self->qh.s = pico8_nr_start.s;
    self->qh.probe->dw = pico8_nr_start.dw;
    self->qh.jo.kq = &pico8_nr_start.kq;
}

static pico8_nr_t *pico8_nr_struct_new(const pico8_jo_keys_t *keys) {
    pico8_nr_t *self = malloc(sizeof *self);

    assert(g_family_trade_up && "pico8_planet_family_trade_init() must run first");
    pico8_nr_struct_init(self, &g_nr_struct_class, &pico8_nr_def);
    if (keys != NULL) {
        if (keys->id != NULL) {
            self->qh.jo.id = keys->id;
        }
        if (keys->p != NULL) {
            self->qh.jo.p = keys->p;
        }
        if (keys->ra != NULL) {
            self->qh.jo.ra = keys->ra;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("nr");
#endif
    pico8_jo_pq((pico8_jo_t *)self);
    pico8_ke_struct_pq(&self->qh.jo.header);
    return self;
}

static int pico8_fr_get_mt(const void *self, pico8_fixed_t *out) {
    *out = ((const pico8_fr_t *)self)->mt;
    return 1;
}

static const pico8_struct_class_t g_fr_struct_class = {
    .kj = pico8_jo_kj_dispatch,
    .ev = pico8_dl_dispatch,
    .hd = pico8_jo_hd_dispatch,
    .hk = pico8_jo_hk_dispatch,
    .hb = pico8_nr_hb_dispatch,
    .ldown = pico8_nr_ldown_dispatch,
    .gv = pico8_nr_gv_dispatch,
    .hq = pico8_bv_dispatch,
    .ey = pico8_d__dispatch,
    .kf = pico8_jo_kf_dispatch,
    .cn = pico8_nr_cn_dispatch,
    .states =
        {
            [PICO8_STATE_LL] = pico8_jo_ll_dispatch,
            [PICO8_STATE_LY] = pico8_qh_ly_dispatch,
        },
    .parent = pico8_nr_struct_class,
    .probe_of = pico8_qh_probe_of,
    .tags = PICO8_TAG(PICO8_TAG_NR) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
    .get_fa = pico8_jo_get_fa,
    .get_ra = pico8_jo_get_ra,
    .get_p = pico8_jo_get_p,
    .get_kq = pico8_jo_get_kq,
    .get_fx = pico8_jo_get_fx,
    .get_p_ = pico8_jo_get_p_,
    .get_mt = pico8_fr_get_mt,
    .set_m_ = pico8_jo_set_m_,
};

static pico8_fr_t *pico8_fr_struct_new(const pico8_jo_keys_t *keys) {
    pico8_fr_t *self = malloc(sizeof *self);

    assert(g_family_trade_up && "pico8_planet_family_trade_init() must run first");
    pico8_nr_struct_init(&self->nr, &g_fr_struct_class, &pico8_fr_def);
    self->mt = pico8_fr_start.mt;
    self->nr.hz = pico8_fr_start.hz;
    self->nr.rr = pico8_fr_start.rr;
    self->nr.qh.probe->od = pico8_fr_start.od;
    self->nr.qh.s = pico8_fr_start.s;
    if (keys != NULL) {
        assert(keys->p == NULL && keys->ra == NULL &&
               "fr: the only construction key is id (ie:ldown); another key has no member "
               "-- stop, do not add one");
        if (keys->id != NULL) {
            self->nr.qh.jo.id = keys->id;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("fr");
#endif
    pico8_jo_pq((pico8_jo_t *)self);
    pico8_ke_struct_pq(&self->nr.qh.jo.header);
    return self;
}

void pico8_mx_kf(pico8_mx_t *self, pico8_struct_header_t *sw) {
    pico8_struct_header_t *cp = NULL;

    if (self != NULL && sw != NULL) {
        assert(sw->cls == pico8_jc_struct_class() && "mx:kf: sw is the slipway jc:ey registers");
        cp = pico8_jc_eb((const pico8_jc_t *)sw, (const pico8_struct_header_t *)self);
    }

    if (cp != NULL) {
        const pico8_ptrs_t *jy;
        pico8_ptrs_iter_t it;
        void *i_val;

        assert(pico8_jo_is_family(cp) && "mx:kf: the other end of a registered slipway is a planet");
        jy = ((const pico8_jo_t *)cp)->jy;

        it = pico8_ptrs_all(jy);
        while (pico8_ptrs_iter_next(&it, &i_val)) {
            const pico8_jy_t *i;
            pico8_fixed_t gc;

            if (i_val == NULL) {
                continue;
            }
            i = i_val;

            gc = i->has_gc ? i->gc : pico8_fixed_from_int(0);
            if (gc != pico8_fixed_from_int(0)) {
                continue;
            }

            if (i->row.has_qe && self->ht != NULL && pico8_int_seq_has_fixed(self->ht, i->row.qe)) {
                pico8_kw_t *kw = self->hj.qh.jo.kw;
                if (kw != NULL) {
                    kw->has_qe = 1;
                    kw->qe = i->row.qe;
                }
                self->ht = &g_mx_ht_spent;
            }
        }
    }

    pico8_jo_kf((pico8_jo_t *)self, sw);
}

static void pico8_mx_kf_dispatch(void *self, pico8_struct_header_t *sw) {
    pico8_mx_kf((pico8_mx_t *)self, sw);
}

static const pico8_struct_class_t g_mx_struct_class = {
    .kj = pico8_hj_kj_dispatch,
    .ev = pico8_dl_dispatch,
    .hd = pico8_jo_hd_dispatch,
    .hk = pico8_jo_hk_dispatch,
    .hb = pico8_jo_hb_dispatch,
    .ldown = pico8_jo_ldown_dispatch,
    .gv = pico8_jo_gv_dispatch,
    .hq = pico8_bv_dispatch,
    .ey = pico8_d__dispatch,
    .kf = pico8_mx_kf_dispatch,
    .cn = pico8_jo_cn_dispatch,
    .mm = pico8_g__mm,
    .states =
        {
            [PICO8_STATE_LL] = pico8_jo_ll_dispatch,
            [PICO8_STATE_LY] = pico8_qh_ly_dispatch,
        },
    .parent = pico8_hj_struct_class,
    .emitter_of = pico8_hj_emitter_of,
    .probe_of = pico8_qh_probe_of,
    .tags = PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
    .get_fa = pico8_jo_get_fa,
    .get_ra = pico8_jo_get_ra,
    .get_p = pico8_jo_get_p,
    .get_kq = pico8_jo_get_kq,
    .get_fx = pico8_jo_get_fx,
    .get_p_ = pico8_jo_get_p_,
    .set_m_ = pico8_jo_set_m_,
};

static pico8_mx_t *pico8_mx_struct_new(const pico8_jo_keys_t *keys) {
    pico8_mx_t *self = malloc(sizeof *self);

    assert(g_family_trade_up && "pico8_planet_family_trade_init() must run first");
    pico8_hj_struct_init(&self->hj, &g_mx_struct_class, &pico8_mx_def);
    self->ht = pico8_mx_start.ht;
    self->hj.rr = pico8_mx_start.rr;
    self->hj.emitter->py = pico8_mx_start.py;
    self->hj.emitter->ki = pico8_mx_start.ki;
    self->hj.emitter->pn = pico8_mx_start.pn;
    self->hj.emitter->pk = pico8_mx_start.pk;
    self->hj.emitter->fc = pico8_mx_start.fc;
    self->hj.emitter->k_ = pico8_mx_start.k_;
    self->hj.emitter->du = pico8_mx_start.du;
    self->hj.emitter->qo = pico8_mx_start.qo;
    self->hj.emitter->ro = pico8_mx_start.ro;
    self->hj.emitter->qg = pico8_mx_start.qg;
    self->hj.emitter->q_ = pico8_mx_start.q_;
    self->hj.qh.hp = pico8_mx_start.hp;
    self->hj.qh.s = pico8_mx_start.s;
    self->hj.qh.px = pico8_mx_start.px;
    self->hj.qh.probe->od = pico8_mx_start.od;
    if (keys != NULL) {
        assert(keys->p == NULL && keys->ra == NULL &&
               "mx: the only construction key is id (ie:ldown); another key has no member "
               "-- stop, do not add one");
        if (keys->id != NULL) {
            self->hj.qh.jo.id = keys->id;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("mx");
#endif
    pico8_f_((pico8_struct_header_t *)self);
    pico8_jo_pq((pico8_jo_t *)self);
    pico8_ke_struct_pq(&self->hj.qh.jo.header);
    return self;
}

void pico8_planet_family_trade_init(void) {
    assert(pico8_qh_ready() && pico8_hj_ready() &&
           "pico8_planet_family_init() must run first");
    pico8_planet_family_trade_shutdown();
    pico8_pi_init();
    g_family_trade_up = 1;
}

void pico8_planet_family_trade_shutdown(void) {
    g_family_trade_up = 0;
}

int pico8_nr_ready(void) {
    return g_family_trade_up;
}

static pico8_struct_header_t *pico8_nr_kind_new(const pico8_jo_keys_t *keys) {
    return &pico8_nr_struct_new(keys)->qh.jo.header;
}

static const pico8_entity_kind_t g_nr_kind = {pico8_nr_kind_new, &pico8_nr_kind_rec};

const pico8_entity_kind_t *pico8_nr_kind(void) {
    return &g_nr_kind;
}

const pico8_struct_class_t *pico8_nr_struct_class(void) {
    return &g_nr_struct_class;
}

pico8_nr_t *pico8_nr_new(const pico8_jo_keys_t *keys) {
    return pico8_nr_struct_new(keys);
}

int pico8_fr_ready(void) {
    return g_family_trade_up;
}

static pico8_struct_header_t *pico8_fr_kind_new(const pico8_jo_keys_t *keys) {
    return &pico8_fr_struct_new(keys)->nr.qh.jo.header;
}

static const pico8_entity_kind_t g_fr_kind = {pico8_fr_kind_new, &pico8_fr_kind_rec};

const pico8_entity_kind_t *pico8_fr_kind(void) {
    return &g_fr_kind;
}

const pico8_struct_class_t *pico8_fr_struct_class(void) {
    return &g_fr_struct_class;
}

pico8_fr_t *pico8_fr_new(const pico8_jo_keys_t *keys) {
    return pico8_fr_struct_new(keys);
}

int pico8_mx_ready(void) {
    return g_family_trade_up;
}

static pico8_struct_header_t *pico8_mx_kind_new(const pico8_jo_keys_t *keys) {
    return &pico8_mx_struct_new(keys)->hj.qh.jo.header;
}

static const pico8_entity_kind_t g_mx_kind = {pico8_mx_kind_new, &pico8_mx_kind_rec};

const pico8_entity_kind_t *pico8_mx_kind(void) {
    return &g_mx_kind;
}

const pico8_struct_class_t *pico8_mx_struct_class(void) {
    return &g_mx_struct_class;
}

pico8_mx_t *pico8_mx_new(const pico8_jo_keys_t *keys) {
    return pico8_mx_struct_new(keys);
}
