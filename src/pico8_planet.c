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

pico8_value_t pico8_jo_hd_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    return pico8_gk((const pico8_struct_header_t *)self, arg0, arg1);
}

pico8_value_t pico8_jo_gg_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_jo_gg((const pico8_jo_t *)self);
}

pico8_value_t pico8_jo_hb_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_jo_hb((pico8_jo_t *)self);
    return pico8_value_ref(NULL);
}

pico8_value_t pico8_jo_gv_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    const pico8_struct_header_t *s = NULL;
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        s = (const pico8_struct_header_t *)arg0.as.ref;
    }
    return pico8_jo_gv((const pico8_jo_t *)self, s);
}

pico8_value_t pico8_jo_hk_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_jo_hk((const pico8_jo_t *)self);
    return pico8_value_ref(NULL);
}

pico8_value_t pico8_jo_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_vec_t p = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        p = *(const pico8_vec_t *)arg0.as.ref;
    }
    pico8_jo_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_jo_t *)self, p);
    return pico8_value_ref(NULL);
}

pico8_value_t pico8_jo_ldown_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_jo_ldown((pico8_jo_t *)self);
}

pico8_value_t pico8_jo_kf_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_struct_header_t *sw = NULL;
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        sw = (pico8_struct_header_t *)arg0.as.ref;
    }
    pico8_jo_kf((pico8_jo_t *)self, sw);
    return pico8_value_ref(NULL);
}

pico8_value_t pico8_jo_cn_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_jo_trade_proposal_t *lw = NULL;
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        lw = (pico8_jo_trade_proposal_t *)arg0.as.ref;
    }
    pico8_jo_cn((pico8_jo_t *)self, lw);
    return pico8_value_ref(NULL);
}

pico8_value_t pico8_jo_ib_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_value_ref(pico8_jo_ib((pico8_jo_t *)self));
}

pico8_value_t pico8_jo_ll_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_fixed_t t = pico8_fixed_from_int(0);
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_FIXED) {
        t = arg0.as.fixed;
    }
    pico8_jo_ll((pico8_jo_t *)self, t);
    return pico8_value_ref(NULL);
}

static void jo_take_kw(pico8_value_t value, pico8_kw_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && value.as.ref != NULL &&
           pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_UNKNOWN &&
           "jo.kw: a write neither nil nor a kw ref does not fit the member's type -- stop, do not widen the member");
    *out = value.as.ref;
}

static void jo_take_nk(pico8_value_t value, const pico8_df_entry_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && value.as.ref != NULL &&
           pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_UNKNOWN &&
           "jo.nk: a write neither nil nor a df entry ref does not fit the member's type "
           "-- stop, do not widen the member");
    *out = value.as.ref;
}

static void jo_take_hp(pico8_value_t value, pico8_hp_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && value.as.ref != NULL &&
           pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)value.as.ref)->cls == pico8_hp_struct_class() &&
           "jo.hn: a write neither nil nor a project hp does not fit the member's type "
           "-- stop, do not widen the member");
    *out = value.as.ref;
}

static void jo_take_string(pico8_value_t value, const char **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_STRING && value.as.string != NULL &&
           "jo.id/ep: a write neither nil nor a string does not fit the member's type "
           "-- stop, do not widen the member");
    *out = value.as.string;
}

static void jo_take_list(pico8_value_t value, pico8_list_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && value.as.ref != NULL &&
           pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_LIST &&
           "jo.oe/jy/cc/by: a write neither nil nor a list does not fit the member's type "
           "-- stop, do not widen the member");
    *out = value.as.ref;
}

static void jo_take_bool(pico8_value_t value, int *out, int *has) {
    *out = 0;
    *has = 0;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_BOOL &&
           "jo.fx/p_/m_: a write neither nil nor a boolean does not fit the member's type "
           "-- stop, do not widen the member");
    *out = value.as.boolean != 0;
    *has = 1;
}

static void jo_take_fixed(pico8_value_t value, pico8_fixed_t *out, int *has) {
    *out = 0;
    *has = 0;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_FIXED &&
           "jo.jh/kr/mi: a write neither nil nor a number does not fit the member's type "
           "-- stop, do not widen the member");
    *out = value.as.fixed;
    *has = 1;
}

static void jo_take_lq(pico8_value_t value, pico8_lq_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && value.as.ref != NULL &&
           pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)value.as.ref)->cls == pico8_lq_struct_class() &&
           "jo.lq: a write neither nil nor a layout row lq does not fit the member's type "
           "-- stop, do not widen the member");
    *out = value.as.ref;
}

static void jo_take_ma(pico8_value_t value, pico8_fixed_t *out) {
    assert(value.kind == PICO8_VALUE_FIXED &&
           "jo.ma: a write that is not a fixed does not fit the member's type -- stop, do not widen the member");
    *out = value.as.fixed;
}

static int jo_is_fo_holder_class(const pico8_struct_class_t *cls) {
    return cls->methods[PICO8_METHOD_HQ] == pico8_bv_dispatch || cls == pico8_jc_struct_class();
}

static void jo_take_holder(pico8_value_t value, pico8_struct_header_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_STRUCT &&
           jo_is_fo_holder_class(((const pico8_struct_header_t *)value.as.ref)->cls) &&
           "jo.fo: a write neither nil nor an object whose hq runs bv (g_, fv, qh, nl, os, hj, pm, nr, fr, mx) or a jc "
           "does not fit the member's type -- stop, do not widen the member");
    *out = value.as.ref;
}

static int jo_field_string(const char *s, pico8_value_t *out_value) {
    if (s == NULL) {
        return 0;
    }
    *out_value = pico8_value_string(s);
    return 1;
}

static int jo_field_list(pico8_list_t *l, pico8_value_t *out_value) {
    if (l == NULL) {
        return 0;
    }
    *out_value = pico8_value_ref(l);
    return 1;
}

static int jo_field_vec(const pico8_vec_t *v, pico8_value_t *out_value) {
    if (v == NULL) {
        return 0;
    }
    *out_value = pico8_value_ref((void *)v);
    return 1;
}

static int jo_field_data(const void *data, pico8_value_t *out_value) {
    if (data == NULL) {
        return 0;
    }
    *out_value = pico8_value_ref((void *)data);
    return 1;
}

static int pico8_jo_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_jo_t *jo = self;

    if (strcmp(name, "nu") == 0) {
        return jo_field_data(jo->nu, out_value);
    }
    if (strcmp(name, "hf") == 0) {
        *out_value = pico8_jo_hf(jo);
        return 1;
    }
    if (strcmp(name, "hs") == 0) {
        *out_value = pico8_value_fixed(jo->hs);
        return 1;
    }
    if (strcmp(name, "fa") == 0) {
        *out_value = pico8_value_fixed(jo->fa);
        return 1;
    }
    if (strcmp(name, "kq") == 0) {
        if (jo->kq == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref((void *)jo->kq);
        return 1;
    }
    if (strcmp(name, "ma") == 0) {
        *out_value = pico8_value_fixed(jo->ma);
        return 1;
    }
    if (strcmp(name, "ka") == 0) {
        return jo_field_data(jo->ka, out_value);
    }
    if (strcmp(name, "dz") == 0) {
        return jo_field_data(jo->dz, out_value);
    }
    if (strcmp(name, "cq") == 0) {
        return jo_field_data(jo->cq, out_value);
    }
    if (strcmp(name, "kw") == 0) {
        return jo_field_data(jo->kw, out_value);
    }
    if (strcmp(name, "gq") == 0) {
        return jo_field_vec(jo->gq, out_value);
    }
    if (strcmp(name, "ca") == 0) {
        *out_value = pico8_value_fixed(jo->ca);
        return 1;
    }
    if (strcmp(name, "em") == 0) {
        return jo_field_data(jo->em, out_value);
    }
    if (strcmp(name, "id") == 0) {
        return jo_field_string(jo->id, out_value);
    }
    if (strcmp(name, "p") == 0) {
        return jo_field_vec(jo->p, out_value);
    }
    if (strcmp(name, "ra") == 0) {
        return jo_field_vec(jo->ra, out_value);
    }
    if (strcmp(name, "oe") == 0) {
        return jo_field_list(jo->oe, out_value);
    }
    if (strcmp(name, "hn") == 0) {
        if (jo->hn == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref(jo->hn);
        return 1;
    }
    if (strcmp(name, "ep") == 0) {
        return jo_field_string(jo->ep, out_value);
    }
    if (strcmp(name, "jy") == 0) {
        return jo_field_list(jo->jy, out_value);
    }
    if (strcmp(name, "nk") == 0) {
        return jo_field_data(jo->nk, out_value);
    }
    if (strcmp(name, "fx") == 0) {
        if (!jo->has_fx) {
            return 0;
        }
        *out_value = pico8_value_bool(jo->fx);
        return 1;
    }
    if (strcmp(name, "lq") == 0) {
        if (jo->lq == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref(jo->lq);
        return 1;
    }
    if (strcmp(name, "fo") == 0) {
        if (jo->fo == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref(jo->fo);
        return 1;
    }
    if (strcmp(name, "jh") == 0) {
        if (!jo->has_jh) {
            return 0;
        }
        *out_value = pico8_value_fixed(jo->jh);
        return 1;
    }
    if (strcmp(name, "p_") == 0) {
        if (!jo->has_p_) {
            return 0;
        }
        *out_value = pico8_value_bool(jo->p_);
        return 1;
    }
    if (strcmp(name, "kr") == 0) {
        if (!jo->has_kr) {
            return 0;
        }
        *out_value = pico8_value_fixed(jo->kr);
        return 1;
    }
    if (strcmp(name, "cc") == 0) {
        return jo_field_list(jo->cc, out_value);
    }
    if (strcmp(name, "by") == 0) {
        return jo_field_list(jo->by, out_value);
    }
    if (strcmp(name, "mi") == 0) {
        if (!jo->has_mi) {
            return 0;
        }
        *out_value = pico8_value_fixed(jo->mi);
        return 1;
    }
    if (strcmp(name, "m_") == 0) {
        if (!jo->has_m_) {
            return 0;
        }
        *out_value = pico8_value_bool(jo->m_);
        return 1;
    }
    return pico8_ke_field(self, name, out_value);
}

/* The 17 names jo's own functions and the generic writers of `cg.jo`
 * (`hy:lj`, `bd`, `bb`, the hover loop, `ma()`, `bv`, `jc:hq`) write on a
 * bare planet; 0 for any other. */
static int pico8_jo_set_field(void *self, const char *name, pico8_value_t value) {
    pico8_jo_t *jo = self;

    if (strcmp(name, "ma") == 0) {
        jo_take_ma(value, &jo->ma);
        return 1;
    }
    if (strcmp(name, "kw") == 0) {
        jo_take_kw(value, &jo->kw);
        return 1;
    }
    if (strcmp(name, "oe") == 0) {
        jo_take_list(value, &jo->oe);
        return 1;
    }
    if (strcmp(name, "hn") == 0) {
        jo_take_hp(value, &jo->hn);
        return 1;
    }
    if (strcmp(name, "ep") == 0) {
        jo_take_string(value, &jo->ep);
        return 1;
    }
    if (strcmp(name, "jy") == 0) {
        jo_take_list(value, &jo->jy);
        return 1;
    }
    if (strcmp(name, "nk") == 0) {
        jo_take_nk(value, &jo->nk);
        return 1;
    }
    if (strcmp(name, "fx") == 0) {
        jo_take_bool(value, &jo->fx, &jo->has_fx);
        return 1;
    }
    if (strcmp(name, "lq") == 0) {
        jo_take_lq(value, &jo->lq);
        return 1;
    }
    if (strcmp(name, "fo") == 0) {
        jo_take_holder(value, &jo->fo);
        return 1;
    }
    if (strcmp(name, "jh") == 0) {
        jo_take_fixed(value, &jo->jh, &jo->has_jh);
        return 1;
    }
    if (strcmp(name, "p_") == 0) {
        jo_take_bool(value, &jo->p_, &jo->has_p_);
        return 1;
    }
    if (strcmp(name, "kr") == 0) {
        jo_take_fixed(value, &jo->kr, &jo->has_kr);
        return 1;
    }
    if (strcmp(name, "cc") == 0) {
        jo_take_list(value, &jo->cc);
        return 1;
    }
    if (strcmp(name, "by") == 0) {
        jo_take_list(value, &jo->by);
        return 1;
    }
    if (strcmp(name, "mi") == 0) {
        jo_take_fixed(value, &jo->mi, &jo->has_mi);
        return 1;
    }
    if (strcmp(name, "m_") == 0) {
        jo_take_bool(value, &jo->m_, &jo->has_m_);
        return 1;
    }
    return 0;
}

static const pico8_struct_class_t g_jo_struct_class = {
    {
        [PICO8_METHOD_HD] = pico8_jo_hd_dispatch,
        [PICO8_METHOD_KJ] = pico8_jo_kj_dispatch,
        [PICO8_METHOD_LDOWN] = pico8_jo_ldown_dispatch,
        [PICO8_METHOD_HB] = pico8_jo_hb_dispatch,
        [PICO8_METHOD_GG] = pico8_jo_gg_dispatch,
        [PICO8_METHOD_GV] = pico8_jo_gv_dispatch,
        [PICO8_METHOD_HK] = pico8_jo_hk_dispatch,
        [PICO8_METHOD_KF] = pico8_jo_kf_dispatch,
        [PICO8_METHOD_CN] = pico8_jo_cn_dispatch,
        [PICO8_METHOD_IB] = pico8_jo_ib_dispatch,
        [PICO8_METHOD_LL] = pico8_jo_ll_dispatch,
    },
    pico8_jo_field,
    pico8_jo_set_field,
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

int pico8_jo_kind_field(const char *name, pico8_value_t *out_value) {
    if (!g_jo_up) {
        return 0;
    }
    if (strcmp(name, "me") == 0) {
        *out_value = pico8_value_string(pico8_jo_start.me);
        return 1;
    }
    return 0;
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
    pico8_value_t hp_val;
    pico8_list_t *oe;

    if (self == NULL) {
        return;
    }

    oe = malloc(sizeof *oe);
    pico8_list_init(oe);
    self->oe = oe;

    if (pico8_entity_get_field(self, "hp", &hp_val) && pico8_jo_truthy(hp_val)) {
        pico8_hp_t *target = NULL;

        if (hp_val.kind == PICO8_VALUE_FIXED) {
            target = pico8_gb_at(pico8_fixed_flr_int(hp_val.as.fixed));
        } else if (hp_val.kind == PICO8_VALUE_STRING) {
            target = pico8_gb_find(hp_val.as.string);
        }
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
        pico8_ke_ks((pico8_struct_header_t *)self, pico8_value_string("ly"));
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
    pico8_value_t ks_arg;

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
            pico8_value_t e = pico8_gn_next(g_jo_oc);

            nk = e.kind == PICO8_VALUE_REF ? e.as.ref : NULL;
        }
    }
    self->nk = nk;
    self->ep = nk != NULL ? nk->d : NULL;

    if (nk != NULL && nk->me != NULL) {
        ks_arg = pico8_value_string(nk->me);
    } else {
        ks_arg = pico8_value_string("known");
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

pico8_value_t pico8_jo_ldown(pico8_jo_t *self) {
    const char *me_val;

    if (self == NULL || self->ke.me == NULL) {
        return pico8_value_ref(NULL);
    }
    me_val = self->ke.me;
    if (strcmp(me_val, "known") == 0) {
        pico8_jo_fe(self);
        return pico8_value_bool(1);
    }
    if (strcmp(me_val, "ly") == 0) {
        pico8_jc_t *made;
        pico8_jc_keys_t keys = {0};

        assert(pico8_jc_ready() && "pico8_jc_init() must run before jo:ldown builds a jc");
        keys.qu = (pico8_struct_header_t *)self;
        made = pico8_jc_new(&keys);
        return pico8_value_ref(made);
    }
    return pico8_value_ref(NULL);
}
