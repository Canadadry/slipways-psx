#include "pico8_planet.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_audio.h"
#include "pico8_collect.h"
#include "pico8_dispatch_context.h"
#include "pico8_economy.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_gen.h"
#include "pico8_ke.h"
#include "pico8_msgbox.h"
#include "pico8_pi.h"
#include "pico8_planet_display.h"
#include "pico8_planet_family.h"
#include "pico8_planet_trade.h"
#include "pico8_probe.h"
#include "pico8_research.h"
#include "pico8_slipway.h"
#include "pico8_tech_gate.h"
#include "pico8_type_tag.h"

static int g_jo_up = 0;

static pico8_gn_state_t *g_jo_oc = NULL;
static pico8_jf_state_t *g_jo_ir = NULL;

/* Shared by every bare planet: written in place by `bb` and `mx:kf` until
 * `lz` gives the planet its own. */
static pico8_kw_t g_jo_kw_shared;

static int pico8_jo_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

int pico8_jo_hd_dispatch(void *self, const pico8_vec_t *qa, const pico8_vec_t *ra) {
    return pico8_gk((const pico8_struct_header_t *)self, qa, ra);
}

void pico8_jo_hb_dispatch(void *self) {
    pico8_jo_hb((pico8_jo_t *)self);
}

pico8_down_t pico8_jo_gv_dispatch(void *self, const pico8_struct_header_t *s) {
    return pico8_jo_gv((const pico8_jo_t *)self, s);
}

void pico8_jo_hk_dispatch(void *self, pico8_fixed_t yr) {
    (void)yr;
    pico8_jo_hk((const pico8_jo_t *)self);
}

void pico8_jo_kj_dispatch(void *self, const pico8_vec_t *ra) {
    pico8_vec_t p = ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    pico8_jo_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_jo_t *)self, p);
}

pico8_down_t pico8_jo_ldown_dispatch(void *self, pico8_vec_t *qa, pico8_vec_t *ra) {
    (void)qa;
    (void)ra;
    return pico8_jo_ldown((pico8_jo_t *)self);
}

int pico8_jo_is_family(const pico8_struct_header_t *e) {
    return e->cls->states[PICO8_STATE_LL] == pico8_jo_ll_dispatch;
}

void pico8_jo_kf_dispatch(void *self, pico8_struct_header_t *sw) {
    pico8_jo_kf((pico8_jo_t *)self, sw);
}

void pico8_jo_cn_dispatch(void *self, struct pico8_jo_trade_proposal *lw) {
    pico8_jo_cn((pico8_jo_t *)self, lw);
}

void pico8_jo_ll_dispatch(void *self, pico8_fixed_t t) {
    pico8_jo_ll((pico8_jo_t *)self, t);
}

const pico8_str_seq_t *pico8_jo_get_nu(const void *self) {
    return ((const pico8_jo_t *)self)->nu;
}

int pico8_jo_get_fa(const void *self, pico8_fixed_t *out) {
    *out = ((const pico8_jo_t *)self)->fa;
    return 1;
}

const pico8_vec_t *pico8_jo_get_ra(const void *self) {
    return ((const pico8_jo_t *)self)->ra;
}

const pico8_vec_t *pico8_jo_get_p(const void *self) {
    return ((const pico8_jo_t *)self)->p;
}

const pico8_qz_box_t *pico8_jo_get_kq(const void *self) {
    return ((const pico8_jo_t *)self)->kq;
}

int pico8_jo_get_fx(const void *self) {
    const pico8_jo_t *jo = self;

    return jo->has_fx && jo->fx;
}

int pico8_jo_get_p_(const void *self) {
    const pico8_jo_t *jo = self;

    return jo->has_p_ && jo->p_;
}

void pico8_jo_set_m_(void *self, int m_) {
    pico8_jo_t *jo = self;

    jo->m_ = m_ != 0;
    jo->has_m_ = 1;
}

static const pico8_struct_class_t g_jo_struct_class = {
    .kj = pico8_jo_kj_dispatch,
    .hd = pico8_jo_hd_dispatch,
    .hk = pico8_jo_hk_dispatch,
    .hb = pico8_jo_hb_dispatch,
    .ldown = pico8_jo_ldown_dispatch,
    .gv = pico8_jo_gv_dispatch,
    .kf = pico8_jo_kf_dispatch,
    .cn = pico8_jo_cn_dispatch,
    .states = {[PICO8_STATE_LL] = pico8_jo_ll_dispatch},
    .get_nu = pico8_jo_get_nu,
    .get_fa = pico8_jo_get_fa,
    .get_ra = pico8_jo_get_ra,
    .get_p = pico8_jo_get_p,
    .get_kq = pico8_jo_get_kq,
    .get_fx = pico8_jo_get_fx,
    .get_p_ = pico8_jo_get_p_,
    .set_m_ = pico8_jo_set_m_,
};

void pico8_jo_struct_init(pico8_jo_t *self, const pico8_struct_class_t *cls) {
    assert(g_jo_up && "pico8_jo_init() must run first");
    pico8_ke_struct_init(&self->ke, cls);
    self->nu = pico8_jo_start.nu;
    self->hf = pico8_jo_start.hf;
    self->hf_is_bool = pico8_jo_start.hf_is_bool;
    self->hs = pico8_jo_start.hs;
    self->fa = pico8_jo_start.fa;
    self->kq = &pico8_jo_start.kq;
    self->ma = pico8_jo_start.ma;
    self->ka = pico8_jo_start.ka;
    self->dz = pico8_jo_start.dz;
    self->cq = pico8_jo_start.cq;
    self->kw = &g_jo_kw_shared;
    self->gq = &pico8_jo_start.gq;
    self->ca = pico8_jo_start.ca;
    self->em = pico8_jo_start.em;
    self->id = NULL;
    self->p = NULL;
    self->ra = NULL;
    self->oe = NULL;
    self->hn = NULL;
    self->ep = NULL;
    self->jy = NULL;
    self->nk = NULL;
    self->fx = 0;
    self->has_fx = 0;
    self->lq = NULL;
    self->fo = NULL;
    self->jh = 0;
    self->has_jh = 0;
    self->p_ = 0;
    self->has_p_ = 0;
    self->kr = 0;
    self->has_kr = 0;
    self->cc = NULL;
    self->by = NULL;
    self->mi = 0;
    self->has_mi = 0;
    self->m_ = 0;
    self->has_m_ = 0;
    self->kw_own = *pico8_jo_start.kw;
    self->ke.me = pico8_jo_start.me;
}

static void *pico8_jo_struct_new(const pico8_jo_keys_t *keys) {
    pico8_jo_t *self = malloc(sizeof *self);

    pico8_jo_struct_init(self, &g_jo_struct_class);
    if (keys != NULL) {
        if (keys->p != NULL) {
            self->p = keys->p;
        }
        if (keys->ra != NULL) {
            self->ra = keys->ra;
        }
        if (keys->id != NULL) {
            self->id = keys->id;
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("jo");
#endif
    pico8_jo_pq(self);
    pico8_ke_struct_pq(&self->ke);
    return self;
}

void pico8_jo_init(void) {
    pico8_jo_shutdown();
    pico8_pi_init();
    g_jo_up = 1;
    g_jo_kw_shared = *pico8_jo_start.kw;
}

void pico8_jo_shutdown(void) {
    g_jo_up = 0;
    if (g_jo_oc != NULL) {
        pico8_gn_free(g_jo_oc);
        g_jo_oc = NULL;
    }
    if (g_jo_ir != NULL) {
        pico8_jf_free(g_jo_ir);
        g_jo_ir = NULL;
    }
}

void pico8_jo_set_oc(pico8_gn_state_t *oc) {
    if (g_jo_oc == oc) {
        return;
    }
    pico8_gn_free(g_jo_oc);
    g_jo_oc = oc;
}

void pico8_jo_set_ir(pico8_jf_state_t *ir) {
    if (g_jo_ir == ir) {
        return;
    }
    pico8_jf_free(g_jo_ir);
    g_jo_ir = ir;
}

int pico8_jo_ready(void) {
    return g_jo_up;
}

const pico8_jb_row_t *pico8_jo_dz(void) {
    return pico8_jo_dz_rows;
}

const pico8_int_seq_t *pico8_jo_cq(void) {
    return &pico8_jo_cq_seq;
}

const pico8_int_seq_t *pico8_jo_ka(void) {
    return &pico8_jo_ka_seq;
}

const pico8_jb_row_t *pico8_jo_em(void) {
    return &pico8_jo_em_row;
}

pico8_kw_t *pico8_jo_shared_kw(void) {
    return &g_jo_kw_shared;
}

const pico8_struct_class_t *pico8_jo_struct_class(void) {
    return &g_jo_struct_class;
}

pico8_jo_t *pico8_jo_new(const pico8_jo_keys_t *keys) {
    return pico8_jo_struct_new(keys);
}

void pico8_jo_pq(pico8_jo_t *self) {
    const pico8_qh_t *qh;
    pico8_list_t *oe;

    if (self == NULL) {
        return;
    }

    oe = malloc(sizeof *oe);
    pico8_list_init(oe);
    self->oe = oe;

    qh = pico8_qh_of(&self->ke.header);
    if (qh != NULL) {
        pico8_hp_t *target = pico8_gb_at(pico8_fixed_flr_int(qh->hp));

        if (target != NULL) {
            pico8_jo_lz(self, target);
        }
    }
}

/* `mw(i)`: a copy of the want, never shared with `hp.i`. */
static int pico8_jo_lz_mw_step(pico8_value_t e, void *ctx, pico8_value_t *out_value) {
    (void)ctx;
    if (e.kind == PICO8_VALUE_REF && e.as.ref != NULL) {
        pico8_jy_t *copy = malloc(sizeof *copy);

        assert(copy != NULL);
        *copy = *(const pico8_jy_t *)e.as.ref;
        *out_value = pico8_value_ref(copy);
        return 1;
    }
    return 0;
}

void pico8_jo_lz(pico8_jo_t *self, pico8_hp_t *hp_instance) {
    pico8_list_t jy;
    pico8_list_t *jy_heap;

    pico8_hp_assert(hp_instance);
    if (self == NULL) {
        return;
    }

    self->hn = hp_instance;
    self->ep = NULL;

    pico8_list_init(&jy);
    if (hp_instance != NULL && hp_instance->i != NULL) {
        jy = pico8_nv(hp_instance->i, pico8_jo_lz_mw_step, NULL);
    }
    jy_heap = malloc(sizeof *jy_heap);
    *jy_heap = jy;
    self->jy = jy_heap;

    self->kw_own.has_qe = hp_instance != NULL && hp_instance->has_o;
    self->kw_own.qe = self->kw_own.has_qe ? hp_instance->o : 0;
    self->kw_own.mu = pico8_fixed_from_int(0);
    self->kw = &self->kw_own;
}

pico8_value_t pico8_jo_hf(const pico8_jo_t *self) {
    if (self == NULL) {
        return pico8_value_ref(NULL);
    }
    if (self->hf_is_bool) {
        return pico8_value_bool(self->hf != 0);
    }
    return pico8_value_fixed(self->hf);
}

pico8_value_t pico8_jo_gg(const pico8_jo_t *self) {
    pico8_value_t hf_val;

    if (self == NULL) {
        return pico8_value_ref(NULL);
    }
    hf_val = pico8_jo_hf(self);
    if (!pico8_jo_truthy(hf_val)) {
        return hf_val;
    }
    return pico8_value_bool(self->ke.me != NULL && strcmp(self->ke.me, "ly") == 0);
}

void pico8_jo_kd(pico8_jo_t *self) {
    if (self == NULL) {
        return;
    }
    self->ma = PICO8_FIXED_LIT(0x00014000, 1.25);
}

void pico8_jo_ll(pico8_jo_t *self, pico8_fixed_t t) {
    if (self == NULL) {
        return;
    }
    self->p_ = t > pico8_fixed_from_int(60);
    self->has_p_ = 1;
}

/* `kv`, indexed by `fz`: `pm` and `ga` are the only kinds whose spec sets
 * one. */
static const struct {
    const char *fz;
    const pico8_entity_kind_t *(*kind)(void);
} g_kv[] = {
    {"pm", pico8_pm_kind},
    {"ga", pico8_ga_kind},
};

/* NULL when no kind has `fz`. */
static const pico8_entity_kind_t *jo_kv(const char *fz) {
    size_t i;

    for (i = 0; i < sizeof g_kv / sizeof g_kv[0]; i++) {
        if (strcmp(g_kv[i].fz, fz) == 0) {
            return g_kv[i].kind();
        }
    }
    return NULL;
}

void pico8_jo_hu(pico8_jo_t *self, pico8_hp_t *hp_instance) {
    pico8_jo_t *working_self;
    pico8_fixed_t fp = pico8_fixed_from_int(2);

    pico8_hp_assert(hp_instance);
    if (self == NULL) {
        return;
    }
    working_self = self;

    if (hp_instance != NULL && hp_instance->cw != NULL) {
        const pico8_entity_kind_t *target_kind = NULL;
        pico8_jo_keys_t keys;

        self->p_ = 1;
        self->has_p_ = 1;

        target_kind = jo_kv(hp_instance->cw);

        keys.p = self->p;
        keys.ra = self->ra;
        keys.id = self->id;

        working_self = (target_kind != NULL) ? (pico8_jo_t *)target_kind->new_entity(&keys) : NULL;
    } else if (hp_instance != NULL && hp_instance->go != NULL) {
        self->nk = pico8_df_lookup(hp_instance->go);
    } else {
        pico8_value_t fp_val;
        pico8_ke_ks((pico8_struct_header_t *)self, "ly");
        pico8_jo_lz(self, hp_instance);
        self->fx = 1;
        self->has_fx = 1;
        fp_val = pico8_ov(pico8_value_string("autoassemblers"), pico8_value_fixed(pico8_fixed_from_int(3)),
                           pico8_value_fixed(pico8_fixed_from_int(2)));
        if (fp_val.kind == PICO8_VALUE_FIXED) {
            fp = fp_val.as.fixed;
        }
    }

    pico8_hy_nx(pico8_fm_hy(), fp);

    if (hp_instance != NULL && hp_instance->has_rn) {
        pico8_rn(pico8_value_fixed(hp_instance->rn));
    } else {
        pico8_rn(pico8_value_fixed(pico8_fixed_from_int(55)));
    }

    pico8_jo_kd(working_self);
}

void pico8_jo_hh(pico8_jo_t *self) {
    const pico8_df_entry_t *nk = NULL;
    const char *ks_arg;

    if (self == NULL) {
        return;
    }

    if (g_jo_ir != NULL) {
        pico8_fixed_t roll = pico8_jf_next(g_jo_ir);
        int use_ll = 0;

        /* `jo.ir()<0.45 and df.ll or jo.oc()`: a falsy `df.ll` falls through
         * to `jo.oc()` even on a winning roll -- both conditions, never the
         * roll alone. */
        if (roll < PICO8_FIXED_LIT(0x00007333, 0.45)) {
            use_ll = pico8_df_lookup("ll") != NULL;
        }
        if (use_ll) {
            nk = pico8_df_lookup("ll");
        } else if (g_jo_oc != NULL) {
            nk = pico8_gn_next(g_jo_oc);
        }
    }
    self->nk = nk;
    self->ep = nk != NULL ? nk->d : NULL;

    if (nk != NULL && nk->me != NULL) {
        ks_arg = nk->me;
    } else {
        ks_arg = "known";
    }
    pico8_ke_ks((pico8_struct_header_t *)self, ks_arg);
    pico8_jo_kd(self);
}

void pico8_jo_hk(const pico8_jo_t *self) {
    if (self == NULL) {
        return;
    }
    if (!self->has_kr || self->kr == pico8_fixed_from_int(0)) {
        return;
    }
    if (!pico8_gd((const pico8_struct_header_t *)self)) {
        return;
    }
    {
        pico8_iv_keys_t keys = {0};
        int color = 0;
        char *text;

        keys.ra = self->ra;
        text = pico8_ne(self->kr, 10, "$", &color);
        /* `ne()` is the last expression of the Lua constructor, so both of its
         * return values land as positional entries 1 and 2, not named fields. */
        keys.at1 = text;
        keys.at2 = pico8_fixed_from_int(color);
        pico8_iv_new(&keys);
        /* `text` is now owned by the new `iv` instance, which reads it every
         * frame it stays on screen: not freed here. */
    }
}

pico8_down_t pico8_jo_ldown(pico8_jo_t *self) {
    const char *me_val;
    pico8_down_t out = {0, NULL};

    if (self == NULL || self->ke.me == NULL) {
        return out;
    }
    me_val = self->ke.me;
    if (strcmp(me_val, "known") == 0) {
        pico8_jo_fe(self);
        out.stop = 1;
        return out;
    }
    if (strcmp(me_val, "ly") == 0) {
        pico8_jc_keys_t keys = {0};

        assert(pico8_jc_ready() && "pico8_jc_init() must run before jo:ldown builds a jc");
        keys.qu = (pico8_struct_header_t *)self;
        out.held = (pico8_struct_header_t *)pico8_jc_new(&keys);
        out.stop = out.held != NULL;
    }
    return out;
}
