#include "pico8_planet_family.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_struct.h"
#include "pico8_collect.h"
#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_ke.h"
#include "pico8_particle.h"
#include "pico8_pi.h"
#include "pico8_planet.h"
#include "pico8_planet_display.h"
#include "pico8_probe.h"
#include "pico8_slipway.h"

static int g_family_up = 0;
/* Every burst's `by`, which `ib` extends in place; each init empties it. */
static pico8_ents_t g_ga_by_shared;

void pico8_qh_ly_dispatch(void *self, pico8_fixed_t t) {
    pico8_na((pico8_struct_header_t *)self, t);
}

pico8_probe_t *pico8_qh_probe_of(void *self) {
    return ((pico8_qh_t *)self)->probe;
}

static const pico8_struct_class_t g_qh_struct_class = {
    .kj = pico8_jo_kj_dispatch,
    .ev = pico8_dl_dispatch,
    .hd = pico8_jo_hd_dispatch,
    .hk = pico8_jo_hk_dispatch,
    .hb = pico8_jo_hb_dispatch,
    .ldown = pico8_jo_ldown_dispatch,
    .gv = pico8_jo_gv_dispatch,
    .hq = pico8_bv_dispatch,
    .ey = pico8_d__dispatch,
    .kf = pico8_jo_kf_dispatch,
    .cn = pico8_jo_cn_dispatch,
    .states =
        {
            [PICO8_STATE_LL] = pico8_jo_ll_dispatch,
            [PICO8_STATE_LY] = pico8_qh_ly_dispatch,
        },
    .parent = pico8_jo_struct_class,
    .probe_of = pico8_qh_probe_of,
    .tags = PICO8_TAG(PICO8_TAG_QH) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
    .get_fa = pico8_jo_get_fa,
    .get_ra = pico8_jo_get_ra,
    .get_p = pico8_jo_get_p,
    .get_kq = pico8_jo_get_kq,
    .get_fx = pico8_jo_get_fx,
    .get_p_ = pico8_jo_get_p_,
    .set_m_ = pico8_jo_set_m_,
};

void pico8_qh_struct_init(pico8_qh_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
    pico8_jo_struct_init(&self->jo, cls, def);
    self->hp = pico8_qh_start.hp;
    self->px = pico8_qh_start.px;
    self->probe = &self->probe_own;
    self->probe->def = NULL;
    self->probe->dq = pico8_qh_start.dq;
    self->qh = pico8_qh_start.qh;
    self->probe->od = pico8_qh_start.od;
    self->probe->has_od = 1;
    self->probe->fp = pico8_qh_start.fp;
    self->probe->ik = 0;
    self->probe->has_ik = 0;
    self->probe->dw = pico8_qh_start.dw;
    self->s = pico8_qh_start.s;
    self->ba = pico8_qh_start.ba;
    self->probe->et = NULL;
    self->jo.mi = pico8_qh_start.mi;
    self->jo.has_mi = 1;
    self->jo.hf = pico8_qh_start.hf;
    self->jo.header.me = pico8_qh_start.me;
}

static pico8_qh_t *pico8_qh_struct_new(const pico8_jo_keys_t *keys) {
    pico8_qh_t *self = malloc(sizeof *self);

    assert(g_family_up && "pico8_planet_family_init() must run first");
    pico8_qh_struct_init(self, &g_qh_struct_class, &pico8_qh_def);
    if (keys != NULL) {
        assert(keys->p == NULL && keys->ra == NULL &&
               "qh: the only construction key is id (ie:ldown); another key has no member "
               "-- stop, do not add one");
        if (keys->id != NULL) {
            self->jo.id = keys->id;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("qh");
#endif
    pico8_jo_pq((pico8_jo_t *)self);
    pico8_ke_struct_pq(&self->jo.header);
    return self;
}

void pico8_nl_ly(pico8_nl_t *self, pico8_fixed_t t) {
    if (self == NULL) {
        return;
    }
    pico8_na((pico8_struct_header_t *)self, t);

    assert(self->qh.jo.has_mi && "nl:ly reads self.mi unguarded, unlike hj:kj");
    if (self->qh.jo.has_mi && self->qh.jo.mi >= pico8_fixed_from_int(2)) {
        self->qw = pico8_fixed_add(pico8_fixed_from_int(8),
                                   pico8_fixed_div(pico8_fixed_mod(t, pico8_fixed_from_int(42)), pico8_fixed_from_int(6)));
    }
}

static void pico8_nl_ly_dispatch(void *self, pico8_fixed_t t) {
    pico8_nl_ly((pico8_nl_t *)self, t);
}

static const pico8_struct_class_t g_nl_struct_class = {
    .kj = pico8_jo_kj_dispatch,
    .ev = pico8_dl_dispatch,
    .hd = pico8_jo_hd_dispatch,
    .hk = pico8_jo_hk_dispatch,
    .hb = pico8_jo_hb_dispatch,
    .ldown = pico8_jo_ldown_dispatch,
    .gv = pico8_jo_gv_dispatch,
    .hq = pico8_bv_dispatch,
    .ey = pico8_d__dispatch,
    .kf = pico8_jo_kf_dispatch,
    .cn = pico8_jo_cn_dispatch,
    .states =
        {
            [PICO8_STATE_LL] = pico8_jo_ll_dispatch,
            [PICO8_STATE_LY] = pico8_nl_ly_dispatch,
        },
    .parent = pico8_qh_struct_class,
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

static pico8_nl_t *pico8_nl_struct_new(const pico8_jo_keys_t *keys) {
    pico8_nl_t *self = malloc(sizeof *self);

    assert(g_family_up && "pico8_planet_family_init() must run first");
    pico8_qh_struct_init(&self->qh, &g_nl_struct_class, &pico8_nl_def);
    self->rr = pico8_nl_start.rr;
    self->nl = pico8_nl_start.nl;
    self->qw = pico8_nl_start.qw;
    self->qh.qh = pico8_nl_start.qh;
    self->qh.hp = pico8_nl_start.hp;
    self->qh.s = pico8_nl_start.s;
    self->qh.px = pico8_nl_start.px;
    self->qh.probe->od = pico8_nl_start.od;
    self->qh.ba = pico8_nl_start.ba;
    if (keys != NULL) {
        assert(keys->p == NULL && keys->ra == NULL &&
               "nl: the only construction key is id (ie:ldown); another key has no member "
               "-- stop, do not add one");
        if (keys->id != NULL) {
            self->qh.jo.id = keys->id;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("nl");
#endif
    pico8_jo_pq((pico8_jo_t *)self);
    pico8_ke_struct_pq(&self->qh.jo.header);
    return self;
}

static const pico8_struct_class_t g_os_struct_class = {
    .kj = pico8_jo_kj_dispatch,
    .ev = pico8_dl_dispatch,
    .hd = pico8_jo_hd_dispatch,
    .hk = pico8_jo_hk_dispatch,
    .hb = pico8_jo_hb_dispatch,
    .ldown = pico8_jo_ldown_dispatch,
    .gv = pico8_jo_gv_dispatch,
    .hq = pico8_bv_dispatch,
    .ey = pico8_d__dispatch,
    .kf = pico8_jo_kf_dispatch,
    .cn = pico8_jo_cn_dispatch,
    .states =
        {
            [PICO8_STATE_LL] = pico8_jo_ll_dispatch,
            [PICO8_STATE_LY] = pico8_qh_ly_dispatch,
        },
    .parent = pico8_qh_struct_class,
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

static pico8_os_t *pico8_os_struct_new(const pico8_jo_keys_t *keys) {
    pico8_os_t *self = malloc(sizeof *self);

    assert(g_family_up && "pico8_planet_family_init() must run first");
    pico8_qh_struct_init(&self->qh, &g_os_struct_class, &pico8_os_def);
    self->qh.hp = pico8_os_start.hp;
    self->qh.px = pico8_os_start.px;
    self->qh.qh = pico8_os_start.qh;
    self->qh.s = pico8_os_start.s;
    self->qh.probe->od = pico8_os_start.od;
    self->qh.jo.hs = pico8_os_start.hs;
    if (keys != NULL) {
        assert(keys->p == NULL && keys->ra == NULL &&
               "os: the only construction key is id (ie:ldown); another key has no member "
               "-- stop, do not add one");
        if (keys->id != NULL) {
            self->qh.jo.id = keys->id;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("os");
#endif
    pico8_jo_pq((pico8_jo_t *)self);
    pico8_ke_struct_pq(&self->qh.jo.header);
    return self;
}

void pico8_hj_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_hj_t *self, pico8_vec_t p) {
    if (self == NULL) {
        return;
    }
    if (self->qh.jo.has_mi && self->qh.jo.mi >= pico8_fixed_from_int(2)) {
        pico8_ei(gfx, mem, (pico8_struct_header_t *)self);
    }
    pico8_jo_kj(gfx, mem, (pico8_jo_t *)self, p);
}

void pico8_hj_kj_dispatch(void *self, const pico8_vec_t *ra) {
    pico8_vec_t p = ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    pico8_hj_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_hj_t *)self, p);
}

pico8_emitter_t *pico8_hj_emitter_of(void *self) {
    return ((pico8_hj_t *)self)->emitter;
}

static const pico8_struct_class_t g_hj_struct_class = {
    .kj = pico8_hj_kj_dispatch,
    .ev = pico8_dl_dispatch,
    .hd = pico8_jo_hd_dispatch,
    .hk = pico8_jo_hk_dispatch,
    .hb = pico8_jo_hb_dispatch,
    .ldown = pico8_jo_ldown_dispatch,
    .gv = pico8_jo_gv_dispatch,
    .hq = pico8_bv_dispatch,
    .ey = pico8_d__dispatch,
    .kf = pico8_jo_kf_dispatch,
    .cn = pico8_jo_cn_dispatch,
    .mm = pico8_g__mm,
    .states =
        {
            [PICO8_STATE_LL] = pico8_jo_ll_dispatch,
            [PICO8_STATE_LY] = pico8_qh_ly_dispatch,
        },
    .parent = pico8_qh_struct_class,
    .emitter_of = pico8_hj_emitter_of,
    .probe_of = pico8_qh_probe_of,
    .tags = PICO8_TAG(PICO8_TAG_HJ) | PICO8_TAG(PICO8_TAG_JO) | PICO8_TAG(PICO8_TAG_NP) | PICO8_TAG(PICO8_TAG_PROBER),
    .get_fa = pico8_jo_get_fa,
    .get_ra = pico8_jo_get_ra,
    .get_p = pico8_jo_get_p,
    .get_kq = pico8_jo_get_kq,
    .get_fx = pico8_jo_get_fx,
    .get_p_ = pico8_jo_get_p_,
    .set_m_ = pico8_jo_set_m_,
};

void pico8_hj_struct_init(pico8_hj_t *self, const pico8_struct_class_t *cls, const pico8_jo_def_t *def) {
    pico8_qh_struct_init(&self->qh, cls, def);
    self->rr = pico8_hj_start.rr;
    self->emitter = &self->emitter_own;
    self->emitter->def = NULL;
    self->emitter->py = pico8_hj_start.py;
    self->emitter->ki = pico8_hj_start.ki;
    self->emitter->pn = pico8_hj_start.pn;
    self->emitter->pk = pico8_hj_start.pk;
    self->emitter->fc = pico8_hj_start.fc;
    self->emitter->k_ = pico8_hj_start.k_;
    self->emitter->du = pico8_hj_start.du;
    self->emitter->qo = pico8_hj_start.qo;
    self->emitter->ro = pico8_hj_start.ro;
    self->emitter->qg = pico8_hj_start.qg;
    self->emitter->q_ = pico8_hj_start.q_;
    self->emitter->qy = NULL;
    self->qh.hp = pico8_hj_start.hp;
    self->qh.qh = pico8_hj_start.qh;
    self->qh.px = pico8_hj_start.px;
    self->qh.probe->od = pico8_hj_start.od;
    self->qh.s = pico8_hj_start.s;
}

static pico8_hj_t *pico8_hj_struct_new(const pico8_jo_keys_t *keys) {
    pico8_hj_t *self = malloc(sizeof *self);

    assert(g_family_up && "pico8_planet_family_init() must run first");
    pico8_hj_struct_init(self, &g_hj_struct_class, &pico8_hj_def);
    if (keys != NULL) {
        assert(keys->p == NULL && keys->ra == NULL &&
               "hj: the only construction key is id (ie:ldown); another key has no member "
               "-- stop, do not add one");
        if (keys->id != NULL) {
            self->qh.jo.id = keys->id;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("hj");
#endif
    pico8_f_((pico8_struct_header_t *)self);
    pico8_jo_pq((pico8_jo_t *)self);
    pico8_ke_struct_pq(&self->qh.jo.header);
    return self;
}

void pico8_pm_pq(pico8_pm_t *self) {
    pico8_vec_t spawn_ra;
    pico8_vec_t *spawn_ra_heap;
    pico8_ga_keys_t keys;
    pico8_ga_t *spawned;
    const pico8_vec_t *ra;
    const pico8_vec_t *ng;

    if (self == NULL) {
        return;
    }

    ra = self->hj.qh.jo.ra;
    ng = self->ng;
    spawn_ra = pico8_vec_add(ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0)),
                             ng != NULL ? *ng : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0)));
    spawn_ra_heap = malloc(sizeof *spawn_ra_heap);
    *spawn_ra_heap = spawn_ra;

    keys.jo.id = NULL;
    keys.jo.p = NULL;
    keys.jo.ra = spawn_ra_heap;
    keys.pq = &pico8_pm_ga_pq_keys;

    spawned = pico8_ga_new(&keys);
    (void)spawned;
}

static const pico8_struct_class_t g_pm_struct_class = {
    .kj = pico8_hj_kj_dispatch,
    .ev = pico8_dl_dispatch,
    .hd = pico8_jo_hd_dispatch,
    .hk = pico8_jo_hk_dispatch,
    .hb = pico8_jo_hb_dispatch,
    .ldown = pico8_jo_ldown_dispatch,
    .gv = pico8_jo_gv_dispatch,
    .hq = pico8_bv_dispatch,
    .ey = pico8_d__dispatch,
    .kf = pico8_jo_kf_dispatch,
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

static pico8_pm_t *pico8_pm_struct_new(const pico8_jo_keys_t *keys) {
    pico8_pm_t *self = malloc(sizeof *self);

    assert(g_family_up && "pico8_planet_family_init() must run first");
    pico8_hj_struct_init(&self->hj, &g_pm_struct_class, &pico8_pm_def);
    self->ng = &pico8_pm_start.ng;
    self->fz = pico8_pm_start.fz;
    self->hj.emitter->py = pico8_pm_start.py;
    self->hj.emitter->ki = pico8_pm_start.ki;
    self->hj.emitter->pn = pico8_pm_start.pn;
    self->hj.emitter->pk = pico8_pm_start.pk;
    self->hj.emitter->du = pico8_pm_start.du;
    self->hj.emitter->fc = pico8_pm_start.fc;
    self->hj.emitter->k_ = pico8_pm_start.k_;
    self->hj.emitter->qo = pico8_pm_start.qo;
    self->hj.emitter->ro = pico8_pm_start.ro;
    self->hj.emitter->qg = pico8_pm_start.qg;
    self->hj.emitter->q_ = pico8_pm_start.q_;
    self->hj.qh.hp = pico8_pm_start.hp;
    self->hj.qh.px = pico8_pm_start.px;
    self->hj.qh.s = pico8_pm_start.s;
    self->hj.qh.jo.header.me = pico8_pm_start.me;
    if (keys != NULL) {
        if (keys->p != NULL) {
            self->hj.qh.jo.p = keys->p;
        }
        if (keys->ra != NULL) {
            self->hj.qh.jo.ra = keys->ra;
        }
        if (keys->id != NULL) {
            self->hj.qh.jo.id = keys->id;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("pm");
#endif
    pico8_pm_pq(self);
    pico8_f_((pico8_struct_header_t *)self);
    pico8_jo_pq((pico8_jo_t *)self);
    pico8_ke_struct_pq(&self->hj.qh.jo.header);
    return self;
}

void pico8_ga_pq(pico8_ga_t *self) {
    pico8_qb_t *mo;

    if (self == NULL) {
        return;
    }
    pico8_f_((pico8_struct_header_t *)self);
    pico8_ez((pico8_struct_header_t *)self, pico8_fixed_from_int(140));

    if (pico8_mo() != NULL) {
        mo = pico8_mo_qb();
        assert(mo != NULL && "mo: the placeholder has no writer; a write before jl() stops the run");
        mo->ma = pico8_fixed_from_int(3);
    }
}

pico8_qy_t *pico8_ga_mm(void *self_ptr) {
    const pico8_ga_t *self = (const pico8_ga_t *)self_ptr;
    pico8_fixed_t qd;
    pico8_fixed_t ms;
    pico8_fixed_t iu;
    pico8_fixed_t py;
    pico8_vec_t p;
    pico8_vec_t v;
    pico8_vec_t a;
    pico8_qy_t *particle;

    qd = pico8_fixed_add(pico8_fixed_rnd(PICO8_FIXED_LIT(0x00018000, 1.5)), PICO8_FIXED_LIT(0x00008000, 0.5));
    if (pico8_fixed_rnd_default() < PICO8_FIXED_LIT(0x00009999, 0.6)) {
        qd = pico8_fixed_mul(pico8_fixed_flr(pico8_fixed_div(qd, PICO8_FIXED_LIT(0x00008000, 0.5))),
                              PICO8_FIXED_LIT(0x00008000, 0.5));
    }
    p = pico8_vec_qm(qd, pico8_fixed_rnd_default());

    ms = self->ms;
    iu = self->iu;
    py = self->emitter->py;

    v = pico8_vec_mul(p, ms);
    a = pico8_vec_mul(p, iu);

    particle = malloc(sizeof *particle);
    particle->p = p;
    particle->v = v;
    particle->has_a = 1;
    particle->a = a;
    particle->l = pico8_fixed_add(pico8_fixed_rnd(py), py);

    return particle;
}

void pico8_ga_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_ga_t *self, pico8_vec_t p) {
    const pico8_ptrs_t *qy = NULL;
    const pico8_qy_t *last_pt = NULL;
    int i;
    int px;
    int py;

    if (self == NULL) {
        return;
    }

    qy = self->emitter->qy;

    px = pico8_fixed_flr_int(p.x);
    py = pico8_fixed_flr_int(p.y);

    for (i = 1; i <= 3; i++) {
        const pico8_qy_t *pt = NULL;

        if (qy != NULL && i <= qy->count) {
            pt = qy->items[i - 1];
        }

        if (pt != NULL) {
            int idx;
            int color = 0;
            int k_entry;

            idx = pico8_fixed_flr_int(pico8_fixed_flr(pico8_fixed_div(pt->l, pico8_fixed_from_int(2))));
            /* TROU-21 */
            if (pico8_int_seq_at(pico8_ga_k__seq.items, pico8_ga_k__seq.count, idx, &k_entry)) {
                color = k_entry;
            }
            pico8_circ(gfx, mem, px, py, pico8_fixed_flr_int(pico8_vec_qx(pt->p)), color);
        }

        /* Lua's `pt` outlives the loop, so `p_` follows the THIRD slot
         * alone: the burst dies when fewer than 3 particles remain, not
         * when none do. */
        last_pt = pt;
    }

    pico8_ei(gfx, mem, (pico8_struct_header_t *)self);
    self->jo.p_ = last_pt == NULL;
    self->jo.has_p_ = 1;
}

static void pico8_ga_kj_dispatch(void *self, const pico8_vec_t *ra) {
    pico8_vec_t p = ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    pico8_ga_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_ga_t *)self, p);
}

static pico8_emitter_t *ga_emitter_of(void *self) {
    return ((pico8_ga_t *)self)->emitter;
}

static const pico8_struct_class_t g_ga_struct_class = {
    .kj = pico8_ga_kj_dispatch,
    .hd = pico8_jo_hd_dispatch,
    .hk = pico8_jo_hk_dispatch,
    .hb = pico8_jo_hb_dispatch,
    .ldown = pico8_jo_ldown_dispatch,
    .gv = pico8_jo_gv_dispatch,
    .kf = pico8_jo_kf_dispatch,
    .cn = pico8_jo_cn_dispatch,
    .mm = pico8_ga_mm,
    .states = {[PICO8_STATE_LL] = pico8_jo_ll_dispatch},
    .parent = pico8_jo_struct_class,
    .emitter_of = ga_emitter_of,
    /* ga's cart list of tags is empty and replaces jo's: never jo's. */
    .tags = 0,
    .get_fa = pico8_jo_get_fa,
    .get_ra = pico8_jo_get_ra,
    .get_p = pico8_jo_get_p,
    .get_kq = pico8_jo_get_kq,
    .get_fx = pico8_jo_get_fx,
    .get_p_ = pico8_jo_get_p_,
    .set_m_ = pico8_jo_set_m_,
};

static pico8_ga_t *pico8_ga_struct_new(const pico8_ga_keys_t *keys) {
    pico8_ga_t *self = malloc(sizeof *self);

    assert(g_family_up && "pico8_planet_family_init() must run first");
    pico8_jo_struct_init(&self->jo, &g_ga_struct_class, &pico8_ga_def);
    self->fz = pico8_ga_start.fz;
    self->emitter = &self->emitter_own;
    self->emitter->def = NULL;
    self->emitter->pn = 0;
    self->emitter->du = 0;
    self->emitter->ki = 0;
    self->emitter->pk = 0;
    self->emitter->fc = pico8_ga_start.fc;
    self->emitter->qo = pico8_ga_start.qo;
    self->emitter->ro = pico8_ga_start.ro;
    self->emitter->qg = pico8_ga_start.qg;
    self->emitter->q_ = pico8_ga_start.q_;
    self->emitter->k_ = pico8_ga_start.k_;
    self->iu = pico8_ga_start.iu;
    self->emitter->py = pico8_ga_start.py;
    self->ms = pico8_ga_start.ms;
    self->emitter->qy = NULL;
    self->jo.by = &g_ga_by_shared;
    self->jo.hf = pico8_ga_start.hf;
    self->jo.kq = &pico8_ga_start.kq;
    self->jo.fa = pico8_ga_start.fa;
    if (keys != NULL) {
        if (keys->jo.p != NULL) {
            self->jo.p = keys->jo.p;
        }
        if (keys->jo.ra != NULL) {
            self->jo.ra = keys->jo.ra;
        }
        if (keys->jo.id != NULL) {
            self->jo.id = keys->jo.id;
        }
        if (keys->pq != NULL) {
            self->emitter->qo = keys->pq->qo;
            self->emitter->ro = keys->pq->ro;
            self->emitter->qg = keys->pq->qg;
            self->emitter->q_ = keys->pq->q_;
            self->emitter->k_ = keys->pq->k_;
            self->iu = keys->pq->iu;
            self->emitter->py = keys->pq->py;
            self->ms = keys->pq->ms;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("ga");
#endif
    pico8_ga_pq(self);
    pico8_jo_pq((pico8_jo_t *)self);
    pico8_ke_struct_pq(&self->jo.header);
    return self;
}

void pico8_planet_family_init(void) {
    assert(pico8_jo_ready() && "pico8_jo_init() must run first");
    pico8_planet_family_shutdown();
    pico8_pi_init();
    g_family_up = 1;
    pico8_ents_init(&g_ga_by_shared);
}

void pico8_planet_family_shutdown(void) {
    g_family_up = 0;
}

int pico8_qh_ready(void) {
    return g_family_up;
}

static pico8_struct_header_t *pico8_qh_kind_new(const pico8_jo_keys_t *keys) {
    return &pico8_qh_struct_new(keys)->jo.header;
}

static const pico8_entity_kind_t g_qh_kind = {pico8_qh_kind_new, &pico8_qh_kind_rec};

const pico8_entity_kind_t *pico8_qh_kind(void) {
    return &g_qh_kind;
}

const pico8_struct_class_t *pico8_qh_struct_class(void) {
    return &g_qh_struct_class;
}

pico8_qh_t *pico8_qh_new(const pico8_jo_keys_t *keys) {
    return pico8_qh_struct_new(keys);
}

int pico8_nl_ready(void) {
    return g_family_up;
}

static pico8_struct_header_t *pico8_nl_kind_new(const pico8_jo_keys_t *keys) {
    return &pico8_nl_struct_new(keys)->qh.jo.header;
}

static const pico8_entity_kind_t g_nl_kind = {pico8_nl_kind_new, &pico8_nl_kind_rec};

const pico8_entity_kind_t *pico8_nl_kind(void) {
    return &g_nl_kind;
}

const pico8_struct_class_t *pico8_nl_struct_class(void) {
    return &g_nl_struct_class;
}

pico8_nl_t *pico8_nl_new(const pico8_jo_keys_t *keys) {
    return pico8_nl_struct_new(keys);
}

const pico8_qh_t *pico8_qh_of(const pico8_struct_header_t *e) {
    const pico8_struct_class_t *cls = e != NULL ? e->cls : NULL;

    while (cls != NULL && cls != &g_qh_struct_class) {
        cls = cls->parent != NULL ? cls->parent() : NULL;
    }
    return cls != NULL ? (const pico8_qh_t *)e : NULL;
}

const pico8_nl_t *pico8_nl_of(const pico8_struct_header_t *e) {
    const pico8_struct_class_t *cls = e != NULL ? e->cls : NULL;

    while (cls != NULL && cls != &g_nl_struct_class) {
        cls = cls->parent != NULL ? cls->parent() : NULL;
    }
    return cls != NULL ? (const pico8_nl_t *)e : NULL;
}

int pico8_os_ready(void) {
    return g_family_up;
}

static pico8_struct_header_t *pico8_os_kind_new(const pico8_jo_keys_t *keys) {
    return &pico8_os_struct_new(keys)->qh.jo.header;
}

static const pico8_entity_kind_t g_os_kind = {pico8_os_kind_new, &pico8_os_kind_rec};

const pico8_entity_kind_t *pico8_os_kind(void) {
    return &g_os_kind;
}

const pico8_struct_class_t *pico8_os_struct_class(void) {
    return &g_os_struct_class;
}

pico8_os_t *pico8_os_new(const pico8_jo_keys_t *keys) {
    return pico8_os_struct_new(keys);
}

int pico8_hj_ready(void) {
    return g_family_up;
}

static pico8_struct_header_t *pico8_hj_kind_new(const pico8_jo_keys_t *keys) {
    return &pico8_hj_struct_new(keys)->qh.jo.header;
}

static const pico8_entity_kind_t g_hj_kind = {pico8_hj_kind_new, &pico8_hj_kind_rec};

const pico8_entity_kind_t *pico8_hj_kind(void) {
    return &g_hj_kind;
}

const pico8_struct_class_t *pico8_hj_struct_class(void) {
    return &g_hj_struct_class;
}

pico8_hj_t *pico8_hj_new(const pico8_jo_keys_t *keys) {
    return pico8_hj_struct_new(keys);
}

int pico8_pm_ready(void) {
    return g_family_up;
}

static pico8_struct_header_t *pico8_pm_kind_new(const pico8_jo_keys_t *keys) {
    return &pico8_pm_struct_new(keys)->hj.qh.jo.header;
}

static const pico8_entity_kind_t g_pm_kind = {pico8_pm_kind_new, &pico8_pm_kind_rec};

const pico8_entity_kind_t *pico8_pm_kind(void) {
    return &g_pm_kind;
}

const pico8_struct_class_t *pico8_pm_struct_class(void) {
    return &g_pm_struct_class;
}

pico8_pm_t *pico8_pm_new(const pico8_jo_keys_t *keys) {
    return pico8_pm_struct_new(keys);
}

int pico8_ga_ready(void) {
    return g_family_up;
}

/* `kv["ga"]` (jo:hu): jo's keys, no `pq`. */
static pico8_struct_header_t *pico8_ga_kind_new(const pico8_jo_keys_t *keys) {
    pico8_ga_keys_t ga_keys;

    if (keys == NULL) {
        return &pico8_ga_struct_new(NULL)->jo.header;
    }
    ga_keys.jo = *keys;
    ga_keys.pq = NULL;
    return &pico8_ga_struct_new(&ga_keys)->jo.header;
}

static const pico8_entity_kind_t g_ga_kind = {pico8_ga_kind_new, &pico8_ga_kind_rec};

const pico8_entity_kind_t *pico8_ga_kind(void) {
    return &g_ga_kind;
}

const pico8_struct_class_t *pico8_ga_struct_class(void) {
    return &g_ga_struct_class;
}

pico8_ga_t *pico8_ga_new(const pico8_ga_keys_t *keys) {
    return pico8_ga_struct_new(keys);
}
