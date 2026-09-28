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
#include "pico8_ob.h"
#include "pico8_particle.h"
#include "pico8_pi.h"
#include "pico8_planet.h"
#include "pico8_planet_display.h"
#include "pico8_probe.h"
#include "pico8_ref_audit.h"
#include "pico8_slipway.h"
#include "pico8_type_tag.h"

static pico8_ob_table_t *g_qh_spec = NULL;
static pico8_ob_table_t *g_nl_spec = NULL;
static pico8_ob_table_t *g_os_spec = NULL;
static pico8_ob_table_t *g_hj_spec = NULL;
static pico8_ob_table_t *g_pm_spec = NULL;
static pico8_ob_table_t *g_ga_spec = NULL;

static pico8_vec_t pico8_pf_value_vec(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

pico8_value_t pico8_qh_ly_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_fixed_t t = pico8_fixed_from_int(0);
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_FIXED) {
        t = arg0.as.fixed;
    }
    pico8_na((pico8_struct_header_t *)self, t);
    return pico8_value_ref(NULL);
}

static const char *g_qh_me = NULL;
static pico8_ob_table_t *g_qh_nu = NULL;
static pico8_fixed_t g_qh_hf = 0;
static int g_qh_hf_is_bool = 0;
static pico8_fixed_t g_qh_mi = 0;
static pico8_fixed_t g_qh_hp = 0;
static pico8_ob_table_t *g_qh_px = NULL;
static const char *g_qh_dq = NULL;
static pico8_fixed_t g_qh_qh = 0;
static int g_qh_qh_is_bool = 0;
static pico8_fixed_t g_qh_od = 0;
static pico8_fixed_t g_qh_fp = 0;
static pico8_fixed_t g_qh_dw = 0;
static pico8_fixed_t g_qh_s = 0;
static pico8_ob_table_t *g_qh_ba = NULL;

static void pico8_qh_defaults_reset(void) {
    g_qh_me = NULL;
    g_qh_nu = NULL;
    g_qh_hf = 0;
    g_qh_hf_is_bool = 0;
    g_qh_mi = 0;
    g_qh_hp = 0;
    g_qh_px = NULL;
    g_qh_dq = NULL;
    g_qh_qh = 0;
    g_qh_qh_is_bool = 0;
    g_qh_od = 0;
    g_qh_fp = 0;
    g_qh_dw = 0;
    g_qh_s = 0;
    g_qh_ba = NULL;
}

pico8_value_t pico8_qh_qh(const pico8_qh_t *self) {
    if (self == NULL) {
        return pico8_value_ref(NULL);
    }
    if (self->qh_is_bool) {
        return pico8_value_bool(self->qh != 0);
    }
    return pico8_value_fixed(self->qh);
}

pico8_value_t pico8_qh_et(const pico8_qh_t *self) {
    if (self == NULL || !self->has_et) {
        return pico8_value_ref(NULL);
    }
    if (self->et == NULL) {
        return pico8_value_bool(0);
    }
    return pico8_value_ref(self->et);
}

static int family_ref_field(void *ref, pico8_value_t *out_value) {
    if (ref == NULL) {
        return 0;
    }
    *out_value = pico8_value_ref(ref);
    return 1;
}

static int pico8_qh_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_qh_t *qh = self;

    if (strcmp(name, "hp") == 0) {
        *out_value = pico8_value_fixed(qh->hp);
        return 1;
    }
    if (strcmp(name, "px") == 0) {
        return family_ref_field(qh->px, out_value);
    }
    if (strcmp(name, "dq") == 0) {
        if (qh->dq == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(qh->dq);
        return 1;
    }
    if (strcmp(name, "qh") == 0) {
        *out_value = pico8_qh_qh(qh);
        return 1;
    }
    if (strcmp(name, "od") == 0) {
        *out_value = pico8_value_fixed(qh->od);
        return 1;
    }
    if (strcmp(name, "fp") == 0) {
        *out_value = pico8_value_fixed(qh->fp);
        return 1;
    }
    if (strcmp(name, "dw") == 0) {
        *out_value = pico8_value_fixed(qh->dw);
        return 1;
    }
    if (strcmp(name, "s") == 0) {
        *out_value = pico8_value_fixed(qh->s);
        return 1;
    }
    if (strcmp(name, "ba") == 0) {
        return family_ref_field(qh->ba, out_value);
    }
    if (strcmp(name, "et") == 0) {
        pico8_value_t et = pico8_qh_et(qh);
        if (et.kind == PICO8_VALUE_REF && et.as.ref == NULL) {
            return 0;
        }
        *out_value = et;
        return 1;
    }
    return pico8_jo_struct_class()->field(self, name, out_value);
}

static void family_take_string(pico8_value_t value, const char **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_STRING && value.as.string != NULL &&
           "jo.id: a write neither nil nor a string is off its census row (chaine) -- a field under no rule of "
           "plan-3 section 5: stop, do not widen the member");
    *out = value.as.string;
}

static void family_take_vec(pico8_value_t value, pico8_vec_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && value.as.ref != NULL &&
           pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_UNKNOWN &&
           "jo.ra/p: a write neither nil nor a vector ref is off its census row (ref:autre) -- a field under no rule "
           "of plan-3 section 5: stop, do not widen the member");
    *out = value.as.ref;
}

/* What `bv` aims at: `cg.np` (jo, and qh's tree: exactly the descriptors
 * holding `ly`), a `jc`, or the throwaway target. */
static int family_is_target_class(const pico8_struct_class_t *cls) {
    return cls->methods[PICO8_METHOD_LY] != NULL || cls == pico8_jo_struct_class() ||
           cls == pico8_jc_struct_class() || cls == pico8_bv_et_struct_class();
}

static void family_take_target(pico8_value_t value, pico8_struct_header_t **out, int *has) {
    *out = NULL;
    *has = 0;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(((value.kind == PICO8_VALUE_BOOL && value.as.boolean == 0) ||
            (value.kind == PICO8_VALUE_REF && pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_STRUCT &&
             family_is_target_class(((const pico8_struct_header_t *)value.as.ref)->cls))) &&
           "qh.et: a write neither nil, false, nor what bv aims at (a planet jo, qh, nl, os, hj, pm, nr, fr, mx, a "
           "jc, or bv's throwaway target) is off its census row (bool,ref: to those targets) -- a field under no "
           "rule of plan-3 section 5: stop, do not widen the member");
    *out = value.kind == PICO8_VALUE_REF ? value.as.ref : NULL;
    *has = 1;
}

/* `et`, `ra`, `p`, which qh's own `hq` (`bv`) writes, then jo's 17 names;
 * 0 for any other. */
static int pico8_qh_set_field(void *self, const char *name, pico8_value_t value) {
    pico8_qh_t *qh = self;

    if (strcmp(name, "et") == 0) {
        family_take_target(value, &qh->et, &qh->has_et);
        return 1;
    }
    if (strcmp(name, "ra") == 0) {
        family_take_vec(value, &qh->jo.ra);
        return 1;
    }
    if (strcmp(name, "p") == 0) {
        family_take_vec(value, &qh->jo.p);
        return 1;
    }
    return pico8_jo_struct_class()->set_field(self, name, value);
}

static const pico8_struct_class_t g_qh_struct_class = {
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
        [PICO8_METHOD_HQ] = pico8_bv_dispatch,
        [PICO8_METHOD_EV] = pico8_dl_dispatch,
        [PICO8_METHOD_EY] = pico8_d__dispatch,
        [PICO8_METHOD_LY] = pico8_qh_ly_dispatch,
    },
    pico8_qh_field,
    pico8_qh_set_field,
};

void pico8_qh_struct_init(pico8_qh_t *self, const pico8_struct_class_t *cls) {
    pico8_jo_struct_init(&self->jo, cls);
    self->hp = g_qh_hp;
    self->px = g_qh_px;
    self->dq = g_qh_dq;
    self->qh = g_qh_qh;
    self->qh_is_bool = g_qh_qh_is_bool;
    self->od = g_qh_od;
    self->fp = g_qh_fp;
    self->dw = g_qh_dw;
    self->s = g_qh_s;
    self->ba = g_qh_ba;
    self->et = NULL;
    self->has_et = 0;
    self->jo.mi = g_qh_mi;
    self->jo.has_mi = 1;
    self->jo.nu = g_qh_nu;
    self->jo.hf = g_qh_hf;
    self->jo.hf_is_bool = g_qh_hf_is_bool;
    self->jo.ke.me = g_qh_me;
}

static void *pico8_qh_struct_new(const pico8_ob_table_t *overrides) {
    pico8_qh_t *self = malloc(sizeof *self);
    int k;

    assert(g_qh_spec != NULL && "pico8_planet_family_init() must run first");
    pico8_qh_struct_init(self, &g_qh_struct_class);
    for (k = 0; overrides != NULL && k < overrides->count; k++) {
        pico8_ob_key_t key = overrides->entries[k].key;
        int measured = key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "id") == 0;

        assert(measured && "qh: the only measured construction key is id (ie:ldown); another would be a field "
                           "under no rule of plan-3 section 5 -- stop, do not add a member");
        (void)measured;
        family_take_string(overrides->entries[k].value, &self->jo.id);
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("qh");
#endif
    pico8_jo_pq((pico8_jo_t *)self);
    pico8_ke_struct_pq(&self->jo.ke);
    PICO8_MEMBER_CENSUS_SEE(self, pico8_qh_t);
    return self;
}

static const char *g_nl_rr = NULL;
static pico8_fixed_t g_nl_nl = 0;
static pico8_fixed_t g_nl_qw = 0;
static pico8_ob_table_t *g_nl_nu = NULL;
static pico8_fixed_t g_nl_qh = 0;
static int g_nl_qh_is_bool = 0;
static pico8_fixed_t g_nl_hp = 0;
static pico8_fixed_t g_nl_s = 0;
static pico8_ob_table_t *g_nl_px = NULL;
static pico8_fixed_t g_nl_od = 0;
static pico8_ob_table_t *g_nl_ba = NULL;

static void pico8_nl_defaults_reset(void) {
    g_nl_rr = NULL;
    g_nl_nl = 0;
    g_nl_qw = 0;
    g_nl_nu = NULL;
    g_nl_qh = 0;
    g_nl_qh_is_bool = 0;
    g_nl_hp = 0;
    g_nl_s = 0;
    g_nl_px = NULL;
    g_nl_od = 0;
    g_nl_ba = NULL;
}

static int pico8_nl_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_nl_t *nl = self;

    if (strcmp(name, "rr") == 0) {
        if (nl->rr == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(nl->rr);
        return 1;
    }
    if (strcmp(name, "nl") == 0) {
        *out_value = pico8_value_fixed(nl->nl);
        return 1;
    }
    if (strcmp(name, "qw") == 0) {
        *out_value = pico8_value_fixed(nl->qw);
        return 1;
    }
    return pico8_qh_struct_class()->field(self, name, out_value);
}

static void family_take_fixed(pico8_value_t value, pico8_fixed_t *out) {
    assert(value.kind == PICO8_VALUE_FIXED &&
           "nl.qw, ga.qo/ro/qg/q_/iu/py/ms: a write that is not a fixed is off its census row (fixe, never nil) -- a "
           "field under no rule of "
           "plan-3 section 5: stop, do not widen the member");
    *out = value.as.fixed;
}

/* `qw`, which nl's own `ly` writes, then qh's 20 names; 0 for any other. */
static int pico8_nl_set_field(void *self, const char *name, pico8_value_t value) {
    if (strcmp(name, "qw") == 0) {
        family_take_fixed(value, &((pico8_nl_t *)self)->qw);
        return 1;
    }
    return pico8_qh_struct_class()->set_field(self, name, value);
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

static pico8_value_t pico8_nl_ly_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_fixed_t t = pico8_fixed_from_int(0);
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_FIXED) {
        t = arg0.as.fixed;
    }
    pico8_nl_ly((pico8_nl_t *)self, t);
    return pico8_value_ref(NULL);
}

static const pico8_struct_class_t g_nl_struct_class = {
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
        [PICO8_METHOD_HQ] = pico8_bv_dispatch,
        [PICO8_METHOD_EV] = pico8_dl_dispatch,
        [PICO8_METHOD_EY] = pico8_d__dispatch,
        [PICO8_METHOD_LY] = pico8_nl_ly_dispatch,
    },
    pico8_nl_field,
    pico8_nl_set_field,
};

static void *pico8_nl_struct_new(const pico8_ob_table_t *overrides) {
    pico8_nl_t *self = malloc(sizeof *self);
    int k;

    assert(g_nl_spec != NULL && "pico8_planet_family_init() must run first");
    pico8_qh_struct_init(&self->qh, &g_nl_struct_class);
    self->rr = g_nl_rr;
    self->nl = g_nl_nl;
    self->qw = g_nl_qw;
    self->qh.qh = g_nl_qh;
    self->qh.qh_is_bool = g_nl_qh_is_bool;
    self->qh.hp = g_nl_hp;
    self->qh.s = g_nl_s;
    self->qh.px = g_nl_px;
    self->qh.od = g_nl_od;
    self->qh.ba = g_nl_ba;
    self->qh.jo.nu = g_nl_nu;
    for (k = 0; overrides != NULL && k < overrides->count; k++) {
        pico8_ob_key_t key = overrides->entries[k].key;
        int measured = key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "id") == 0;

        assert(measured && "nl: the only measured construction key is id (ie:ldown); another would be a field "
                           "under no rule of plan-3 section 5 -- stop, do not add a member");
        (void)measured;
        family_take_string(overrides->entries[k].value, &self->qh.jo.id);
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("nl");
#endif
    pico8_jo_pq((pico8_jo_t *)self);
    pico8_ke_struct_pq(&self->qh.jo.ke);
    PICO8_MEMBER_CENSUS_SEE(self, pico8_nl_t);
    return self;
}

static pico8_ob_table_t *g_os_nu = NULL;
static pico8_fixed_t g_os_hs = 0;
static pico8_fixed_t g_os_hp = 0;
static pico8_ob_table_t *g_os_px = NULL;
static pico8_fixed_t g_os_qh = 0;
static int g_os_qh_is_bool = 0;
static pico8_fixed_t g_os_s = 0;
static pico8_fixed_t g_os_od = 0;

static void pico8_os_defaults_reset(void) {
    g_os_nu = NULL;
    g_os_hs = 0;
    g_os_hp = 0;
    g_os_px = NULL;
    g_os_qh = 0;
    g_os_qh_is_bool = 0;
    g_os_s = 0;
    g_os_od = 0;
}

static const pico8_struct_class_t g_os_struct_class = {
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
        [PICO8_METHOD_HQ] = pico8_bv_dispatch,
        [PICO8_METHOD_EV] = pico8_dl_dispatch,
        [PICO8_METHOD_EY] = pico8_d__dispatch,
        [PICO8_METHOD_LY] = pico8_qh_ly_dispatch,
    },
    pico8_qh_field,
    pico8_qh_set_field,
};

static void *pico8_os_struct_new(const pico8_ob_table_t *overrides) {
    pico8_os_t *self = malloc(sizeof *self);
    int k;

    assert(g_os_spec != NULL && "pico8_planet_family_init() must run first");
    pico8_qh_struct_init(&self->qh, &g_os_struct_class);
    self->qh.hp = g_os_hp;
    self->qh.px = g_os_px;
    self->qh.qh = g_os_qh;
    self->qh.qh_is_bool = g_os_qh_is_bool;
    self->qh.s = g_os_s;
    self->qh.od = g_os_od;
    self->qh.jo.nu = g_os_nu;
    self->qh.jo.hs = g_os_hs;
    for (k = 0; overrides != NULL && k < overrides->count; k++) {
        pico8_ob_key_t key = overrides->entries[k].key;
        int measured = key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "id") == 0;

        assert(measured && "os: the only measured construction key is id (ie:ldown); another would be a field "
                           "under no rule of plan-3 section 5 -- stop, do not add a member");
        (void)measured;
        family_take_string(overrides->entries[k].value, &self->qh.jo.id);
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("os");
#endif
    pico8_jo_pq((pico8_jo_t *)self);
    pico8_ke_struct_pq(&self->qh.jo.ke);
    PICO8_MEMBER_CENSUS_SEE(self, pico8_os_t);
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

pico8_value_t pico8_hj_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_vec_t p = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        p = *(const pico8_vec_t *)arg0.as.ref;
    }
    pico8_hj_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_hj_t *)self, p);
    return pico8_value_ref(NULL);
}

static const char *g_hj_rr = NULL;
static pico8_fixed_t g_hj_py = 0;
static pico8_fixed_t g_hj_ki = 0;
static pico8_fixed_t g_hj_pn = 0;
static pico8_fixed_t g_hj_pk = 0;
static int g_hj_pk_is_bool = 0;
static pico8_fixed_t g_hj_fc = 0;
static pico8_ob_table_t *g_hj_k_ = NULL;
static pico8_fixed_t g_hj_du = 0;
static pico8_fixed_t g_hj_qo = 0;
static pico8_fixed_t g_hj_ro = 0;
static pico8_fixed_t g_hj_qg = 0;
static pico8_fixed_t g_hj_q_ = 0;
static pico8_ob_table_t *g_hj_nu = NULL;
static pico8_fixed_t g_hj_hp = 0;
static pico8_fixed_t g_hj_qh = 0;
static int g_hj_qh_is_bool = 0;
static pico8_ob_table_t *g_hj_px = NULL;
static pico8_fixed_t g_hj_od = 0;
static pico8_fixed_t g_hj_s = 0;

static void pico8_hj_defaults_reset(void) {
    g_hj_rr = NULL;
    g_hj_py = 0;
    g_hj_ki = 0;
    g_hj_pn = 0;
    g_hj_pk = 0;
    g_hj_pk_is_bool = 0;
    g_hj_fc = 0;
    g_hj_k_ = NULL;
    g_hj_du = 0;
    g_hj_qo = 0;
    g_hj_ro = 0;
    g_hj_qg = 0;
    g_hj_q_ = 0;
    g_hj_nu = NULL;
    g_hj_hp = 0;
    g_hj_qh = 0;
    g_hj_qh_is_bool = 0;
    g_hj_px = NULL;
    g_hj_od = 0;
    g_hj_s = 0;
}

pico8_value_t pico8_hj_pk(const pico8_hj_t *self) {
    if (self == NULL) {
        return pico8_value_ref(NULL);
    }
    if (self->pk_is_bool) {
        return pico8_value_bool(self->pk != 0);
    }
    return pico8_value_fixed(self->pk);
}

static int pico8_hj_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_hj_t *hj = self;

    if (strcmp(name, "rr") == 0) {
        if (hj->rr == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(hj->rr);
        return 1;
    }
    if (strcmp(name, "py") == 0) {
        *out_value = pico8_value_fixed(hj->py);
        return 1;
    }
    if (strcmp(name, "ki") == 0) {
        *out_value = pico8_value_fixed(hj->ki);
        return 1;
    }
    if (strcmp(name, "pn") == 0) {
        *out_value = pico8_value_fixed(hj->pn);
        return 1;
    }
    if (strcmp(name, "pk") == 0) {
        *out_value = pico8_hj_pk(hj);
        return 1;
    }
    if (strcmp(name, "fc") == 0) {
        *out_value = pico8_value_fixed(hj->fc);
        return 1;
    }
    if (strcmp(name, "k_") == 0) {
        return family_ref_field(hj->k_, out_value);
    }
    if (strcmp(name, "du") == 0) {
        *out_value = pico8_value_fixed(hj->du);
        return 1;
    }
    if (strcmp(name, "qo") == 0) {
        *out_value = pico8_value_fixed(hj->qo);
        return 1;
    }
    if (strcmp(name, "ro") == 0) {
        *out_value = pico8_value_fixed(hj->ro);
        return 1;
    }
    if (strcmp(name, "qg") == 0) {
        *out_value = pico8_value_fixed(hj->qg);
        return 1;
    }
    if (strcmp(name, "q_") == 0) {
        *out_value = pico8_value_fixed(hj->q_);
        return 1;
    }
    if (strcmp(name, "qy") == 0) {
        return family_ref_field(hj->qy, out_value);
    }
    return pico8_qh_struct_class()->field(self, name, out_value);
}

static void family_take_list(pico8_value_t value, pico8_list_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && value.as.ref != NULL &&
           pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_LIST &&
           "hj/ga.qy: a write neither nil nor a list is off its census row (ref:liste) -- a field under no rule of "
           "plan-3 section 5: stop, do not widen the member");
    *out = value.as.ref;
}

/* `qy`, which hj's own `pq` (`f_`) writes, then qh's 20 names; 0 for any
 * other. */
static int pico8_hj_set_field(void *self, const char *name, pico8_value_t value) {
    if (strcmp(name, "qy") == 0) {
        family_take_list(value, &((pico8_hj_t *)self)->qy);
        return 1;
    }
    return pico8_qh_struct_class()->set_field(self, name, value);
}

static const pico8_struct_class_t g_hj_struct_class = {
    {
        [PICO8_METHOD_HD] = pico8_jo_hd_dispatch,
        [PICO8_METHOD_KJ] = pico8_hj_kj_dispatch,
        [PICO8_METHOD_LDOWN] = pico8_jo_ldown_dispatch,
        [PICO8_METHOD_HB] = pico8_jo_hb_dispatch,
        [PICO8_METHOD_GG] = pico8_jo_gg_dispatch,
        [PICO8_METHOD_GV] = pico8_jo_gv_dispatch,
        [PICO8_METHOD_HK] = pico8_jo_hk_dispatch,
        [PICO8_METHOD_KF] = pico8_jo_kf_dispatch,
        [PICO8_METHOD_CN] = pico8_jo_cn_dispatch,
        [PICO8_METHOD_IB] = pico8_jo_ib_dispatch,
        [PICO8_METHOD_LL] = pico8_jo_ll_dispatch,
        [PICO8_METHOD_HQ] = pico8_bv_dispatch,
        [PICO8_METHOD_EV] = pico8_dl_dispatch,
        [PICO8_METHOD_EY] = pico8_d__dispatch,
        [PICO8_METHOD_LY] = pico8_qh_ly_dispatch,
        [PICO8_METHOD_MM] = pico8_g__mm,
    },
    pico8_hj_field,
    pico8_hj_set_field,
};

void pico8_hj_struct_init(pico8_hj_t *self, const pico8_struct_class_t *cls) {
    pico8_qh_struct_init(&self->qh, cls);
    self->rr = g_hj_rr;
    self->py = g_hj_py;
    self->ki = g_hj_ki;
    self->pn = g_hj_pn;
    self->pk = g_hj_pk;
    self->pk_is_bool = g_hj_pk_is_bool;
    self->fc = g_hj_fc;
    self->k_ = g_hj_k_;
    self->du = g_hj_du;
    self->qo = g_hj_qo;
    self->ro = g_hj_ro;
    self->qg = g_hj_qg;
    self->q_ = g_hj_q_;
    self->qy = NULL;
    self->qh.hp = g_hj_hp;
    self->qh.qh = g_hj_qh;
    self->qh.qh_is_bool = g_hj_qh_is_bool;
    self->qh.px = g_hj_px;
    self->qh.od = g_hj_od;
    self->qh.s = g_hj_s;
    self->qh.jo.nu = g_hj_nu;
}

static void *pico8_hj_struct_new(const pico8_ob_table_t *overrides) {
    pico8_hj_t *self = malloc(sizeof *self);
    int k;

    assert(g_hj_spec != NULL && "pico8_planet_family_init() must run first");
    pico8_hj_struct_init(self, &g_hj_struct_class);
    for (k = 0; overrides != NULL && k < overrides->count; k++) {
        pico8_ob_key_t key = overrides->entries[k].key;
        int measured = key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "id") == 0;

        assert(measured && "hj: the only measured construction key is id (ie:ldown); another would be a field "
                           "under no rule of plan-3 section 5 -- stop, do not add a member");
        (void)measured;
        family_take_string(overrides->entries[k].value, &self->qh.jo.id);
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("hj");
#endif
    pico8_f_((pico8_struct_header_t *)self);
    pico8_jo_pq((pico8_jo_t *)self);
    pico8_ke_struct_pq(&self->qh.jo.ke);
    PICO8_MEMBER_CENSUS_SEE(self, pico8_hj_t);
    return self;
}

void pico8_pm_pq(pico8_pm_t *self) {
    pico8_vec_t spawn_ra;
    pico8_vec_t *spawn_ra_heap;
    pico8_ob_table_t *defaults;
    pico8_ob_table_t *overrides;
    pico8_ga_t *spawned;

    if (self == NULL) {
        return;
    }

    spawn_ra = pico8_vec_add(pico8_pf_value_vec(pico8_value_ref(self->hj.qh.jo.ra)), pico8_pf_value_vec(pico8_value_ref(self->ng)));
    spawn_ra_heap = malloc(sizeof *spawn_ra_heap);
    *spawn_ra_heap = spawn_ra;

    defaults = pico8_ob_table_new();
    pico8_ob_table_set_name(defaults, "ra", pico8_value_ref(spawn_ra_heap));

    overrides = pico8_ob("\n"
                          "qo=0.97,ro=-0.1255,\n"
                          "qg=0.1255,q_=0.97,\n"
                          "k_=o(1,2,8,14,14,14,14,8,8,2,2,1,1),\n"
                          "iu=-0.009,py=9,ms=1.5,\n",
                          defaults);
    pico8_ob_table_free(defaults);

    spawned = pico8_ga_new(overrides);
    (void)spawned;
    pico8_ob_table_free(overrides);
}

static pico8_vec_t *g_pm_ng = NULL;
static const char *g_pm_fz = NULL;
static const char *g_pm_me = NULL;
static pico8_ob_table_t *g_pm_nu = NULL;
static pico8_fixed_t g_pm_hp = 0;
static pico8_ob_table_t *g_pm_px = NULL;
static pico8_fixed_t g_pm_s = 0;
static pico8_fixed_t g_pm_py = 0;
static pico8_fixed_t g_pm_ki = 0;
static pico8_fixed_t g_pm_pn = 0;
static pico8_fixed_t g_pm_pk = 0;
static int g_pm_pk_is_bool = 0;
static pico8_fixed_t g_pm_du = 0;
static pico8_fixed_t g_pm_fc = 0;
static pico8_ob_table_t *g_pm_k_ = NULL;
static pico8_fixed_t g_pm_qo = 0;
static pico8_fixed_t g_pm_ro = 0;
static pico8_fixed_t g_pm_qg = 0;
static pico8_fixed_t g_pm_q_ = 0;

static void pico8_pm_defaults_reset(void) {
    g_pm_ng = NULL;
    g_pm_fz = NULL;
    g_pm_me = NULL;
    g_pm_nu = NULL;
    g_pm_hp = 0;
    g_pm_px = NULL;
    g_pm_s = 0;
    g_pm_py = 0;
    g_pm_ki = 0;
    g_pm_pn = 0;
    g_pm_pk = 0;
    g_pm_pk_is_bool = 0;
    g_pm_du = 0;
    g_pm_fc = 0;
    g_pm_k_ = NULL;
    g_pm_qo = 0;
    g_pm_ro = 0;
    g_pm_qg = 0;
    g_pm_q_ = 0;
}

static int pico8_pm_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_pm_t *pm = self;

    if (strcmp(name, "ng") == 0) {
        if (pm->ng == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref(pm->ng);
        return 1;
    }
    if (strcmp(name, "fz") == 0) {
        if (pm->fz == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(pm->fz);
        return 1;
    }
    return pico8_hj_field(self, name, out_value);
}

/* No site writes `ng` or `fz`: the writer is hj's, its 21 names. */
static const pico8_struct_class_t g_pm_struct_class = {
    {
        [PICO8_METHOD_HD] = pico8_jo_hd_dispatch,
        [PICO8_METHOD_KJ] = pico8_hj_kj_dispatch,
        [PICO8_METHOD_LDOWN] = pico8_jo_ldown_dispatch,
        [PICO8_METHOD_HB] = pico8_jo_hb_dispatch,
        [PICO8_METHOD_GG] = pico8_jo_gg_dispatch,
        [PICO8_METHOD_GV] = pico8_jo_gv_dispatch,
        [PICO8_METHOD_HK] = pico8_jo_hk_dispatch,
        [PICO8_METHOD_KF] = pico8_jo_kf_dispatch,
        [PICO8_METHOD_CN] = pico8_jo_cn_dispatch,
        [PICO8_METHOD_IB] = pico8_jo_ib_dispatch,
        [PICO8_METHOD_LL] = pico8_jo_ll_dispatch,
        [PICO8_METHOD_HQ] = pico8_bv_dispatch,
        [PICO8_METHOD_EV] = pico8_dl_dispatch,
        [PICO8_METHOD_EY] = pico8_d__dispatch,
        [PICO8_METHOD_LY] = pico8_qh_ly_dispatch,
        [PICO8_METHOD_MM] = pico8_g__mm,
    },
    pico8_pm_field,
    pico8_hj_set_field,
};

static void *pico8_pm_struct_new(const pico8_ob_table_t *overrides) {
    pico8_pm_t *self = malloc(sizeof *self);
    int k;

    assert(g_pm_spec != NULL && "pico8_planet_family_init() must run first");
    pico8_hj_struct_init(&self->hj, &g_pm_struct_class);
    self->ng = g_pm_ng;
    self->fz = g_pm_fz;
    self->hj.py = g_pm_py;
    self->hj.ki = g_pm_ki;
    self->hj.pn = g_pm_pn;
    self->hj.pk = g_pm_pk;
    self->hj.pk_is_bool = g_pm_pk_is_bool;
    self->hj.du = g_pm_du;
    self->hj.fc = g_pm_fc;
    self->hj.k_ = g_pm_k_;
    self->hj.qo = g_pm_qo;
    self->hj.ro = g_pm_ro;
    self->hj.qg = g_pm_qg;
    self->hj.q_ = g_pm_q_;
    self->hj.qh.hp = g_pm_hp;
    self->hj.qh.px = g_pm_px;
    self->hj.qh.s = g_pm_s;
    self->hj.qh.jo.nu = g_pm_nu;
    self->hj.qh.jo.ke.me = g_pm_me;
    for (k = 0; overrides != NULL && k < overrides->count; k++) {
        pico8_ob_key_t key = overrides->entries[k].key;
        int is_p = key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "p") == 0;
        int is_ra = key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "ra") == 0;
        int is_id = key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "id") == 0;

        assert((is_p || is_ra || is_id) &&
               "pm: the only measured construction keys are p, ra, id (jo:hu); another would be a field "
               "under no rule of plan-3 section 5 -- stop, do not add a member");
        if (is_p) {
            family_take_vec(overrides->entries[k].value, &self->hj.qh.jo.p);
        } else if (is_ra) {
            family_take_vec(overrides->entries[k].value, &self->hj.qh.jo.ra);
        } else {
            family_take_string(overrides->entries[k].value, &self->hj.qh.jo.id);
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("pm");
#endif
    pico8_pm_pq(self);
    pico8_f_((pico8_struct_header_t *)self);
    pico8_jo_pq((pico8_jo_t *)self);
    pico8_ke_struct_pq(&self->hj.qh.jo.ke);
    PICO8_MEMBER_CENSUS_SEE(self, pico8_pm_t);
    return self;
}

static const char *g_ga_fz = NULL;
static pico8_fixed_t g_ga_fc = 0;
static pico8_fixed_t g_ga_qo = 0;
static pico8_fixed_t g_ga_ro = 0;
static pico8_fixed_t g_ga_qg = 0;
static pico8_fixed_t g_ga_q_ = 0;
static pico8_ob_table_t *g_ga_k_ = NULL;
static pico8_fixed_t g_ga_iu = 0;
static pico8_fixed_t g_ga_py = 0;
static pico8_fixed_t g_ga_ms = 0;
static pico8_ob_table_t *g_ga_nu = NULL;
static pico8_fixed_t g_ga_hf = 0;
static int g_ga_hf_is_bool = 0;
static pico8_qz_box_t *g_ga_kq = NULL;
static pico8_fixed_t g_ga_fa = 0;
static pico8_list_t *g_ga_by = NULL;

static void pico8_ga_defaults_reset(void) {
    g_ga_fz = NULL;
    g_ga_fc = 0;
    g_ga_qo = 0;
    g_ga_ro = 0;
    g_ga_qg = 0;
    g_ga_q_ = 0;
    g_ga_k_ = NULL;
    g_ga_iu = 0;
    g_ga_py = 0;
    g_ga_ms = 0;
    g_ga_nu = NULL;
    g_ga_hf = 0;
    g_ga_hf_is_bool = 0;
    g_ga_kq = NULL;
    g_ga_fa = 0;
    g_ga_by = NULL;
}

static int pico8_ga_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_ga_t *ga = self;

    if (strcmp(name, "fz") == 0) {
        if (ga->fz == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(ga->fz);
        return 1;
    }
    if (strcmp(name, "fc") == 0) {
        *out_value = pico8_value_fixed(ga->fc);
        return 1;
    }
    if (strcmp(name, "qo") == 0) {
        *out_value = pico8_value_fixed(ga->qo);
        return 1;
    }
    if (strcmp(name, "ro") == 0) {
        *out_value = pico8_value_fixed(ga->ro);
        return 1;
    }
    if (strcmp(name, "qg") == 0) {
        *out_value = pico8_value_fixed(ga->qg);
        return 1;
    }
    if (strcmp(name, "q_") == 0) {
        *out_value = pico8_value_fixed(ga->q_);
        return 1;
    }
    if (strcmp(name, "k_") == 0) {
        return family_ref_field(ga->k_, out_value);
    }
    if (strcmp(name, "iu") == 0) {
        *out_value = pico8_value_fixed(ga->iu);
        return 1;
    }
    if (strcmp(name, "py") == 0) {
        *out_value = pico8_value_fixed(ga->py);
        return 1;
    }
    if (strcmp(name, "ms") == 0) {
        *out_value = pico8_value_fixed(ga->ms);
        return 1;
    }
    if (strcmp(name, "qy") == 0) {
        return family_ref_field(ga->qy, out_value);
    }
    return pico8_jo_struct_class()->field(self, name, out_value);
}

/* `qy`, which ga's own `pq` (`f_`) writes, then jo's 17 names; 0 for any
 * other. */
static int pico8_ga_set_field(void *self, const char *name, pico8_value_t value) {
    if (strcmp(name, "qy") == 0) {
        family_take_list(value, &((pico8_ga_t *)self)->qy);
        return 1;
    }
    return pico8_jo_struct_class()->set_field(self, name, value);
}

void pico8_ga_pq(pico8_ga_t *self) {
    pico8_struct_header_t *mo;

    if (self == NULL) {
        return;
    }
    pico8_f_((pico8_struct_header_t *)self);
    pico8_ez((pico8_struct_header_t *)self, pico8_fixed_from_int(140));

    mo = pico8_mo();
    if (mo != NULL) {
        pico8_entity_set_field(mo, "ma", pico8_value_fixed(pico8_fixed_from_int(3)));
    }
}

pico8_value_t pico8_ga_mm(void *self_ptr, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    const pico8_ga_t *self = (const pico8_ga_t *)self_ptr;
    pico8_fixed_t qd;
    pico8_fixed_t ms;
    pico8_fixed_t iu;
    pico8_fixed_t py;
    pico8_vec_t p;
    pico8_vec_t v;
    pico8_vec_t a;
    pico8_ob_table_t *particle;
    pico8_vec_t *p_heap;
    pico8_vec_t *v_heap;
    pico8_vec_t *a_heap;

    (void)argc;
    (void)arg0;
    (void)arg1;

    qd = pico8_fixed_add(pico8_fixed_rnd(PICO8_FIXED_LIT(0x00018000, 1.5)), PICO8_FIXED_LIT(0x00008000, 0.5));
    if (pico8_fixed_rnd_default() < PICO8_FIXED_LIT(0x00009999, 0.6)) {
        qd = pico8_fixed_mul(pico8_fixed_flr(pico8_fixed_div(qd, PICO8_FIXED_LIT(0x00008000, 0.5))),
                              PICO8_FIXED_LIT(0x00008000, 0.5));
    }
    p = pico8_vec_qm(qd, pico8_fixed_rnd_default());

    ms = self->ms;
    iu = self->iu;
    py = self->py;

    v = pico8_vec_mul(p, ms);
    a = pico8_vec_mul(p, iu);

    particle = pico8_ob_table_new();
    p_heap = malloc(sizeof *p_heap);
    *p_heap = p;
    v_heap = malloc(sizeof *v_heap);
    *v_heap = v;
    a_heap = malloc(sizeof *a_heap);
    *a_heap = a;
    pico8_ob_table_set_name(particle, "p", pico8_value_ref(p_heap));
    pico8_ob_table_set_name(particle, "v", pico8_value_ref(v_heap));
    pico8_ob_table_set_name(particle, "a", pico8_value_ref(a_heap));
    pico8_ob_table_set_name(particle, "l", pico8_value_fixed(pico8_fixed_add(pico8_fixed_rnd(py), py)));

    return pico8_value_ref(particle);
}

void pico8_ga_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_ga_t *self, pico8_vec_t p) {
    const pico8_list_t *qy = NULL;
    pico8_ob_table_t *last_pt = NULL;
    int i;
    int px;
    int py;

    if (self == NULL) {
        return;
    }

    qy = self->qy;

    px = pico8_fixed_flr_int(p.x);
    py = pico8_fixed_flr_int(p.y);

    for (i = 1; i <= 3; i++) {
        pico8_ob_table_t *pt = NULL;

        if (qy != NULL && i <= qy->count) {
            pico8_value_t entry = qy->items[i - 1];
            if (entry.kind == PICO8_VALUE_REF && entry.as.ref != NULL) {
                pt = (pico8_ob_table_t *)entry.as.ref;
            }
        }

        if (pt != NULL) {
            pico8_value_t p_field;
            pico8_vec_t pt_p = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
            pico8_value_t l_field;
            pico8_fixed_t l = pico8_fixed_from_int(0);
            int idx;
            int color = 0;
            pico8_value_t k_entry;

            if (pico8_ob_table_get_name(pt, "p", &p_field) && p_field.kind == PICO8_VALUE_REF &&
                p_field.as.ref != NULL) {
                pt_p = *(const pico8_vec_t *)p_field.as.ref;
            }
            if (pico8_ob_table_get_name(pt, "l", &l_field) && l_field.kind == PICO8_VALUE_FIXED) {
                l = l_field.as.fixed;
            }
            idx = pico8_fixed_flr_int(pico8_fixed_flr(pico8_fixed_div(l, pico8_fixed_from_int(2))));
            /* TROU-21 */
            if (g_ga_k_ != NULL && pico8_ob_table_get_index(g_ga_k_, idx, &k_entry) &&
                k_entry.kind == PICO8_VALUE_FIXED) {
                color = pico8_fixed_flr_int(k_entry.as.fixed);
            }
            pico8_circ(gfx, mem, px, py, pico8_fixed_flr_int(pico8_vec_qx(pt_p)), color);
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

static pico8_value_t pico8_ga_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_vec_t p = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        p = *(const pico8_vec_t *)arg0.as.ref;
    }
    pico8_ga_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_ga_t *)self, p);
    return pico8_value_ref(NULL);
}

static const pico8_struct_class_t g_ga_struct_class = {
    {
        [PICO8_METHOD_HD] = pico8_jo_hd_dispatch,
        [PICO8_METHOD_KJ] = pico8_ga_kj_dispatch,
        [PICO8_METHOD_LDOWN] = pico8_jo_ldown_dispatch,
        [PICO8_METHOD_HB] = pico8_jo_hb_dispatch,
        [PICO8_METHOD_GG] = pico8_jo_gg_dispatch,
        [PICO8_METHOD_GV] = pico8_jo_gv_dispatch,
        [PICO8_METHOD_HK] = pico8_jo_hk_dispatch,
        [PICO8_METHOD_KF] = pico8_jo_kf_dispatch,
        [PICO8_METHOD_CN] = pico8_jo_cn_dispatch,
        [PICO8_METHOD_IB] = pico8_jo_ib_dispatch,
        [PICO8_METHOD_LL] = pico8_jo_ll_dispatch,
        [PICO8_METHOD_MM] = pico8_ga_mm,
    },
    pico8_ga_field,
    pico8_ga_set_field,
};

static void family_take_table(pico8_value_t value, pico8_ob_table_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && value.as.ref != NULL &&
           pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_OB_TABLE &&
           "ga.k_: a write neither nil nor a table is off its census row (ref:table) -- a field under no rule of "
           "plan-3 section 5: stop, do not widen the member");
    *out = value.as.ref;
}

static void *pico8_ga_struct_new(const pico8_ob_table_t *overrides) {
    pico8_ga_t *self = malloc(sizeof *self);
    int k;

    assert(g_ga_spec != NULL && "pico8_planet_family_init() must run first");
    pico8_jo_struct_init(&self->jo, &g_ga_struct_class);
    self->fz = g_ga_fz;
    self->fc = g_ga_fc;
    self->qo = g_ga_qo;
    self->ro = g_ga_ro;
    self->qg = g_ga_qg;
    self->q_ = g_ga_q_;
    self->k_ = g_ga_k_;
    self->iu = g_ga_iu;
    self->py = g_ga_py;
    self->ms = g_ga_ms;
    self->qy = NULL;
    self->jo.by = g_ga_by;
    self->jo.nu = g_ga_nu;
    self->jo.hf = g_ga_hf;
    self->jo.hf_is_bool = g_ga_hf_is_bool;
    self->jo.kq = g_ga_kq;
    self->jo.fa = g_ga_fa;
    for (k = 0; overrides != NULL && k < overrides->count; k++) {
        pico8_ob_key_t key = overrides->entries[k].key;
        pico8_value_t value = overrides->entries[k].value;
        const char *name = key.kind == PICO8_OB_KEY_NAME ? key.name : "";

        if (strcmp(name, "p") == 0) {
            family_take_vec(value, &self->jo.p);
        } else if (strcmp(name, "ra") == 0) {
            family_take_vec(value, &self->jo.ra);
        } else if (strcmp(name, "id") == 0) {
            family_take_string(value, &self->jo.id);
        } else if (strcmp(name, "qo") == 0) {
            family_take_fixed(value, &self->qo);
        } else if (strcmp(name, "ro") == 0) {
            family_take_fixed(value, &self->ro);
        } else if (strcmp(name, "qg") == 0) {
            family_take_fixed(value, &self->qg);
        } else if (strcmp(name, "q_") == 0) {
            family_take_fixed(value, &self->q_);
        } else if (strcmp(name, "k_") == 0) {
            family_take_table(value, &self->k_);
        } else if (strcmp(name, "iu") == 0) {
            family_take_fixed(value, &self->iu);
        } else if (strcmp(name, "py") == 0) {
            family_take_fixed(value, &self->py);
        } else if (strcmp(name, "ms") == 0) {
            family_take_fixed(value, &self->ms);
        } else {
            assert(0 &&
                   "ga: the only measured construction keys are p, ra, id (jo:hu) and ra, qo, ro, qg, q_, k_, iu, "
                   "py, ms (pm:pq); another would be a field under no rule of plan-3 section 5 -- stop, do not add a member");
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("ga");
#endif
    pico8_ga_pq(self);
    pico8_jo_pq((pico8_jo_t *)self);
    pico8_ke_struct_pq(&self->jo.ke);
    PICO8_MEMBER_CENSUS_SEE(self, pico8_ga_t);
    return self;
}

static pico8_ob_table_t *family_default_table(int found, pico8_value_t v) {
    assert(found && v.kind == PICO8_VALUE_REF && v.as.ref != NULL &&
           pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_OB_TABLE &&
           "qh/nl/os/hj/pm/ga.nu, qh/nl/os/hj/pm.px, qh/nl.ba, hj/pm/ga.k_: a default of the struct is a key of its spec "
           "and a table (ref:table)");
    (void)found;
    return v.as.ref;
}

static pico8_qz_box_t *family_default_box(int found, pico8_value_t v) {
    assert(found && v.kind == PICO8_VALUE_REF && v.as.ref != NULL &&
           pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_UNKNOWN &&
           "ga.kq: a default of the struct is a key of ga's spec and a box b() (ref:autre)");
    (void)found;
    return v.as.ref;
}

static pico8_vec_t *family_default_vec(int found, pico8_value_t v) {
    assert(found && v.kind == PICO8_VALUE_REF && v.as.ref != NULL &&
           pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_UNKNOWN &&
           "pm.ng: a default of the struct is a key of pm's spec and a vector v() (ref:autre)");
    (void)found;
    return v.as.ref;
}

static pico8_fixed_t family_default_fixed(int found, pico8_value_t v) {
    assert(found && v.kind == PICO8_VALUE_FIXED &&
           "os.hs, ga.fa, qh.mi, nl.nl/qw, qh/nl/os/hj/pm.hp, qh/nl/os/hj.od, qh.fp/dw, qh/nl/os/hj/pm.s, "
           "hj/pm.py/ki/pn/fc/du/qo/ro/qg/q_, ga.fc/qo/ro/qg/q_/iu/py/ms: a default of the struct is a key of its "
           "spec and a fixed (fixe)");
    (void)found;
    return v.as.fixed;
}

static pico8_list_t *family_default_list(int found, pico8_value_t v) {
    assert(found && v.kind == PICO8_VALUE_REF && v.as.ref != NULL &&
           pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_LIST &&
           "ga.by: a default of the struct is a key of ga's spec and a list (ref:liste)");
    (void)found;
    return v.as.ref;
}

static pico8_fixed_t family_default_hf(int found, pico8_value_t v, int *is_bool) {
    assert(found && (v.kind == PICO8_VALUE_FIXED || v.kind == PICO8_VALUE_BOOL) &&
           "qh/ga.hf, qh/nl/os/hj.qh, hj/pm.pk: a default of the struct is a key of its spec and a fixed or a "
           "boolean (bool,fixe)");
    (void)found;
    *is_bool = v.kind == PICO8_VALUE_BOOL;
    return v.kind == PICO8_VALUE_BOOL ? (pico8_fixed_t)(v.as.boolean != 0) : v.as.fixed;
}

static const char *family_default_string(int found, pico8_value_t v) {
    assert(found && v.kind == PICO8_VALUE_STRING && v.as.string != NULL &&
           "qh.dq, nl/hj.rr, pm/ga.fz: a default of the struct is a key of its spec and a string (chaine)");
    (void)found;
    return v.as.string;
}

void pico8_planet_family_init(void) {
    pico8_value_t me;
    pico8_value_t v;
    int me_found;
    int found;

    assert(pico8_jo_ready() && "pico8_jo_init() must run first");
    pico8_planet_family_shutdown();
    pico8_pi_init();

    g_qh_spec = pico8_ob("\n"
                         "nu=o(\"qh\",\"jo\",\"np\",\"prober\"),\n"
                         "hp=1,px=o(\"laboratory\",c=13),\n"
                         "me=\"kz\",dq=\"ly\",\n"
                         "hf=f,qh=1,\n"
                         "od=15,fp=2,\n"
                         "dw=0.025,\n"
                         "mi=1,\n"
                         "s=174,\n"
                         "ba=o(28,18,20,21,22,48),\n",
                         NULL);
    found = pico8_qh_default("hp", &v);
    g_qh_hp = family_default_fixed(found, v);
    found = pico8_qh_default("px", &v);
    g_qh_px = family_default_table(found, v);
    found = pico8_qh_default("dq", &v);
    g_qh_dq = family_default_string(found, v);
    found = pico8_qh_default("qh", &v);
    g_qh_qh = family_default_hf(found, v, &g_qh_qh_is_bool);
    found = pico8_qh_default("od", &v);
    g_qh_od = family_default_fixed(found, v);
    found = pico8_qh_default("fp", &v);
    g_qh_fp = family_default_fixed(found, v);
    found = pico8_qh_default("dw", &v);
    g_qh_dw = family_default_fixed(found, v);
    found = pico8_qh_default("s", &v);
    g_qh_s = family_default_fixed(found, v);
    found = pico8_qh_default("ba", &v);
    g_qh_ba = family_default_table(found, v);
    found = pico8_qh_default("mi", &v);
    g_qh_mi = family_default_fixed(found, v);
    found = pico8_qh_default("nu", &v);
    g_qh_nu = family_default_table(found, v);
    found = pico8_qh_default("hf", &v);
    g_qh_hf = family_default_hf(found, v, &g_qh_hf_is_bool);
    me_found = pico8_qh_default("me", &me);
    assert(me_found && me.kind == PICO8_VALUE_STRING && me.as.string != NULL &&
           "qh: me, a default of the struct, is a key of qh's spec and a string (chaine)");
    (void)me_found;
    g_qh_me = me.as.string;

    g_nl_spec = pico8_ob("\n"
                         "nu=o(\"jo\",\"np\",\"prober\"),\n"
                         "rr=\"trade league\",qh=f,nl=1,\n"
                         "hp=6,s=68,px=o(\"trading hub\",c=13),\n"
                         "od=20,qw=15,\n"
                         "ba=o(28,48,18,20,21,22),\n",
                         NULL);
    found = pico8_nl_default("rr", &v);
    g_nl_rr = family_default_string(found, v);
    found = pico8_nl_default("nl", &v);
    g_nl_nl = family_default_fixed(found, v);
    found = pico8_nl_default("qw", &v);
    g_nl_qw = family_default_fixed(found, v);
    found = pico8_nl_default("nu", &v);
    g_nl_nu = family_default_table(found, v);
    found = pico8_nl_default("qh", &v);
    g_nl_qh = family_default_hf(found, v, &g_nl_qh_is_bool);
    found = pico8_nl_default("hp", &v);
    g_nl_hp = family_default_fixed(found, v);
    found = pico8_nl_default("s", &v);
    g_nl_s = family_default_fixed(found, v);
    found = pico8_nl_default("px", &v);
    g_nl_px = family_default_table(found, v);
    found = pico8_nl_default("od", &v);
    g_nl_od = family_default_fixed(found, v);
    found = pico8_nl_default("ba", &v);
    g_nl_ba = family_default_table(found, v);

    g_os_spec = pico8_ob("\n"
                         "nu=o(\"jo\",\"np\",\"prober\"),\n"
                         "hp=4,px=o(\"processor\",c=13),\n"
                         "qh=f,s=100,\n"
                         "od=15,\n"
                         "hs=0.5,\n",
                         NULL);
    found = pico8_os_default("nu", &v);
    g_os_nu = family_default_table(found, v);
    found = pico8_os_default("hp", &v);
    g_os_hp = family_default_fixed(found, v);
    found = pico8_os_default("px", &v);
    g_os_px = family_default_table(found, v);
    found = pico8_os_default("qh", &v);
    g_os_qh = family_default_hf(found, v, &g_os_qh_is_bool);
    found = pico8_os_default("s", &v);
    g_os_s = family_default_fixed(found, v);
    found = pico8_os_default("od", &v);
    g_os_od = family_default_fixed(found, v);
    found = pico8_os_default("hs", &v);
    g_os_hs = family_default_fixed(found, v);

    g_hj_spec = pico8_ob("\n"
                         "nu=o(\"hj\",\"jo\",\"np\",\"prober\"),\n"
                         "rr=\"ascension\",\n"
                         "hp=2,qh=f,\n"
                         "px=o(\"ascension nr\",c=13),\n"
                         "od=60,\n"
                         "s=166,\n"
                         "py=0.12,ki=9,\n"
                         "pn=5,pk=1,\n"
                         "fc=0.5,\n"
                         "k_=o(1,13,12,12,12,13,13,5,1,1,1),\n"
                         "du=7.5,\n"
                         "qo=0.98,ro=-0.07,\n"
                         "qg=0.07,q_=0.98,\n",
                         NULL);
    found = pico8_hj_default("rr", &v);
    g_hj_rr = family_default_string(found, v);
    found = pico8_hj_default("py", &v);
    g_hj_py = family_default_fixed(found, v);
    found = pico8_hj_default("ki", &v);
    g_hj_ki = family_default_fixed(found, v);
    found = pico8_hj_default("pn", &v);
    g_hj_pn = family_default_fixed(found, v);
    found = pico8_hj_default("pk", &v);
    g_hj_pk = family_default_hf(found, v, &g_hj_pk_is_bool);
    found = pico8_hj_default("fc", &v);
    g_hj_fc = family_default_fixed(found, v);
    found = pico8_hj_default("k_", &v);
    g_hj_k_ = family_default_table(found, v);
    found = pico8_hj_default("du", &v);
    g_hj_du = family_default_fixed(found, v);
    found = pico8_hj_default("qo", &v);
    g_hj_qo = family_default_fixed(found, v);
    found = pico8_hj_default("ro", &v);
    g_hj_ro = family_default_fixed(found, v);
    found = pico8_hj_default("qg", &v);
    g_hj_qg = family_default_fixed(found, v);
    found = pico8_hj_default("q_", &v);
    g_hj_q_ = family_default_fixed(found, v);
    found = pico8_hj_default("nu", &v);
    g_hj_nu = family_default_table(found, v);
    found = pico8_hj_default("hp", &v);
    g_hj_hp = family_default_fixed(found, v);
    found = pico8_hj_default("qh", &v);
    g_hj_qh = family_default_hf(found, v, &g_hj_qh_is_bool);
    found = pico8_hj_default("px", &v);
    g_hj_px = family_default_table(found, v);
    found = pico8_hj_default("od", &v);
    g_hj_od = family_default_fixed(found, v);
    found = pico8_hj_default("s", &v);
    g_hj_s = family_default_fixed(found, v);

    g_pm_spec = pico8_ob("\n"
                         "fz=\"pm\",\n"
                         "nu=o(\"jo\",\"np\",\"prober\"),\n"
                         "hp=3,px=o(\"protostar\",c=13),\n"
                         "me=\"ly\",s=98,\n"
                         "py=0.06,ki=9,\n"
                         "pn=8,pk=f,\n"
                         "ng=v(1,0),\n"
                         "du=2,\n"
                         "fc=1,\n"
                         "k_=o(1,2,4,9,10,7),\n"
                         "qo=1.02,ro=-0.0525,\n"
                         "qg=0.0525,q_=1.02,\n",
                         NULL);
    found = pico8_pm_default("ng", &v);
    g_pm_ng = family_default_vec(found, v);
    found = pico8_pm_default("fz", &v);
    g_pm_fz = family_default_string(found, v);
    found = pico8_pm_default("py", &v);
    g_pm_py = family_default_fixed(found, v);
    found = pico8_pm_default("ki", &v);
    g_pm_ki = family_default_fixed(found, v);
    found = pico8_pm_default("pn", &v);
    g_pm_pn = family_default_fixed(found, v);
    found = pico8_pm_default("pk", &v);
    g_pm_pk = family_default_hf(found, v, &g_pm_pk_is_bool);
    found = pico8_pm_default("du", &v);
    g_pm_du = family_default_fixed(found, v);
    found = pico8_pm_default("fc", &v);
    g_pm_fc = family_default_fixed(found, v);
    found = pico8_pm_default("k_", &v);
    g_pm_k_ = family_default_table(found, v);
    found = pico8_pm_default("qo", &v);
    g_pm_qo = family_default_fixed(found, v);
    found = pico8_pm_default("ro", &v);
    g_pm_ro = family_default_fixed(found, v);
    found = pico8_pm_default("qg", &v);
    g_pm_qg = family_default_fixed(found, v);
    found = pico8_pm_default("q_", &v);
    g_pm_q_ = family_default_fixed(found, v);
    found = pico8_pm_default("nu", &v);
    g_pm_nu = family_default_table(found, v);
    found = pico8_pm_default("hp", &v);
    g_pm_hp = family_default_fixed(found, v);
    found = pico8_pm_default("px", &v);
    g_pm_px = family_default_table(found, v);
    found = pico8_pm_default("s", &v);
    g_pm_s = family_default_fixed(found, v);
    me_found = pico8_pm_default("me", &me);
    assert(me_found && me.kind == PICO8_VALUE_STRING && me.as.string != NULL &&
           "pm: me, a default of the struct, is a key of pm's spec and a string (chaine)");
    (void)me_found;
    g_pm_me = me.as.string;

    /* `nu=o()` is empty and replaces jo's `nu` outright, so no burst is
     * ever registered under jo/np/prober in `cg`. This is not a merge. */
    g_ga_spec = pico8_ob("\n"
                         "fz=\"ga\",\n"
                         "nu=o(),\n"
                         "hf=f,\n"
                         "by=o(),\n"
                         "kq=b(0,0,-1,-1),\n"
                         "fa=3,\n"
                         "fc=0,\n"
                         "qo=0.9,ro=0,\n"
                         "qg=0,q_=0.9,\n"
                         "k_=o(1,2,4,9,9,10,10,7,7,7,7,7),\n"
                         "iu=-0.06,py=6,ms=3,\n",
                         NULL);

    /* `by=o()` parses to an ob_table, but every reader of `by` casts it
     * to a list; without this swap the tag check in PICO8_REF_AS_LIST
     * aborts on the first read. The raw table ob() built is freed here. */
    {
        pico8_ob_table_t *raw_by = NULL;
        pico8_value_t by_val;
        pico8_list_t *heap_by;

        if (pico8_ob_table_get_name(g_ga_spec, "by", &by_val) && by_val.kind == PICO8_VALUE_REF) {
            raw_by = (pico8_ob_table_t *)by_val.as.ref;
        }
        heap_by = malloc(sizeof *heap_by);
        pico8_list_init(heap_by);
        pico8_ob_table_set_name(g_ga_spec, "by", pico8_value_ref(heap_by));
        pico8_ob_table_free(raw_by);
    }

    found = pico8_ga_default("fz", &v);
    g_ga_fz = family_default_string(found, v);
    found = pico8_ga_default("fc", &v);
    g_ga_fc = family_default_fixed(found, v);
    found = pico8_ga_default("qo", &v);
    g_ga_qo = family_default_fixed(found, v);
    found = pico8_ga_default("ro", &v);
    g_ga_ro = family_default_fixed(found, v);
    found = pico8_ga_default("qg", &v);
    g_ga_qg = family_default_fixed(found, v);
    found = pico8_ga_default("q_", &v);
    g_ga_q_ = family_default_fixed(found, v);
    found = pico8_ga_default("k_", &v);
    g_ga_k_ = family_default_table(found, v);
    found = pico8_ga_default("iu", &v);
    g_ga_iu = family_default_fixed(found, v);
    found = pico8_ga_default("py", &v);
    g_ga_py = family_default_fixed(found, v);
    found = pico8_ga_default("ms", &v);
    g_ga_ms = family_default_fixed(found, v);
    found = pico8_ga_default("by", &v);
    g_ga_by = family_default_list(found, v);
    found = pico8_ga_default("nu", &v);
    g_ga_nu = family_default_table(found, v);
    found = pico8_ga_default("hf", &v);
    g_ga_hf = family_default_hf(found, v, &g_ga_hf_is_bool);
    found = pico8_ga_default("kq", &v);
    g_ga_kq = family_default_box(found, v);
    found = pico8_ga_default("fa", &v);
    g_ga_fa = family_default_fixed(found, v);
}

void pico8_planet_family_shutdown(void) {
    pico8_qh_defaults_reset();
    pico8_nl_defaults_reset();
    pico8_os_defaults_reset();
    pico8_hj_defaults_reset();
    pico8_pm_defaults_reset();
    pico8_ga_defaults_reset();
    pico8_ob_table_free(g_qh_spec);
    g_qh_spec = NULL;
    pico8_ob_table_free(g_nl_spec);
    g_nl_spec = NULL;
    pico8_ob_table_free(g_os_spec);
    g_os_spec = NULL;
    pico8_ob_table_free(g_hj_spec);
    g_hj_spec = NULL;
    pico8_ob_table_free(g_pm_spec);
    g_pm_spec = NULL;
    pico8_ob_table_free(g_ga_spec);
    g_ga_spec = NULL;
}

int pico8_qh_ready(void) {
    return g_qh_spec != NULL;
}

int pico8_qh_default(const char *name, pico8_value_t *out_value) {
    return g_qh_spec != NULL &&
           (pico8_ob_table_get_name(g_qh_spec, name, out_value) || pico8_jo_default(name, out_value));
}

static const pico8_entity_kind_t g_qh_kind = {pico8_qh_struct_new, pico8_qh_default};

const pico8_entity_kind_t *pico8_qh_kind(void) {
    return &g_qh_kind;
}

const pico8_struct_class_t *pico8_qh_struct_class(void) {
    return &g_qh_struct_class;
}

pico8_qh_t *pico8_qh_new(const pico8_ob_table_t *overrides) {
    return pico8_qh_struct_new(overrides);
}

int pico8_nl_ready(void) {
    return g_nl_spec != NULL;
}

int pico8_nl_default(const char *name, pico8_value_t *out_value) {
    return g_nl_spec != NULL &&
           (pico8_ob_table_get_name(g_nl_spec, name, out_value) || pico8_qh_default(name, out_value));
}

static const pico8_entity_kind_t g_nl_kind = {pico8_nl_struct_new, pico8_nl_default};

const pico8_entity_kind_t *pico8_nl_kind(void) {
    return &g_nl_kind;
}

const pico8_struct_class_t *pico8_nl_struct_class(void) {
    return &g_nl_struct_class;
}

pico8_nl_t *pico8_nl_new(const pico8_ob_table_t *overrides) {
    return pico8_nl_struct_new(overrides);
}

int pico8_os_ready(void) {
    return g_os_spec != NULL;
}

int pico8_os_default(const char *name, pico8_value_t *out_value) {
    return g_os_spec != NULL &&
           (pico8_ob_table_get_name(g_os_spec, name, out_value) || pico8_qh_default(name, out_value));
}

static const pico8_entity_kind_t g_os_kind = {pico8_os_struct_new, pico8_os_default};

const pico8_entity_kind_t *pico8_os_kind(void) {
    return &g_os_kind;
}

const pico8_struct_class_t *pico8_os_struct_class(void) {
    return &g_os_struct_class;
}

pico8_os_t *pico8_os_new(const pico8_ob_table_t *overrides) {
    return pico8_os_struct_new(overrides);
}

int pico8_hj_ready(void) {
    return g_hj_spec != NULL;
}

int pico8_hj_default(const char *name, pico8_value_t *out_value) {
    return g_hj_spec != NULL &&
           (pico8_ob_table_get_name(g_hj_spec, name, out_value) || pico8_qh_default(name, out_value));
}

static const pico8_entity_kind_t g_hj_kind = {pico8_hj_struct_new, pico8_hj_default};

const pico8_entity_kind_t *pico8_hj_kind(void) {
    return &g_hj_kind;
}

const pico8_struct_class_t *pico8_hj_struct_class(void) {
    return &g_hj_struct_class;
}

pico8_hj_t *pico8_hj_new(const pico8_ob_table_t *overrides) {
    return pico8_hj_struct_new(overrides);
}

int pico8_pm_ready(void) {
    return g_pm_spec != NULL;
}

int pico8_pm_default(const char *name, pico8_value_t *out_value) {
    return g_pm_spec != NULL &&
           (pico8_ob_table_get_name(g_pm_spec, name, out_value) || pico8_hj_default(name, out_value));
}

static const pico8_entity_kind_t g_pm_kind = {pico8_pm_struct_new, pico8_pm_default};

const pico8_entity_kind_t *pico8_pm_kind(void) {
    return &g_pm_kind;
}

const pico8_struct_class_t *pico8_pm_struct_class(void) {
    return &g_pm_struct_class;
}

pico8_pm_t *pico8_pm_new(const pico8_ob_table_t *overrides) {
    return pico8_pm_struct_new(overrides);
}

int pico8_ga_ready(void) {
    return g_ga_spec != NULL;
}

int pico8_ga_default(const char *name, pico8_value_t *out_value) {
    return g_ga_spec != NULL &&
           (pico8_ob_table_get_name(g_ga_spec, name, out_value) || pico8_jo_default(name, out_value));
}

static const pico8_entity_kind_t g_ga_kind = {pico8_ga_struct_new, pico8_ga_default};

const pico8_entity_kind_t *pico8_ga_kind(void) {
    return &g_ga_kind;
}

const pico8_struct_class_t *pico8_ga_struct_class(void) {
    return &g_ga_struct_class;
}

pico8_ga_t *pico8_ga_new(const pico8_ob_table_t *overrides) {
    return pico8_ga_struct_new(overrides);
}
