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

static int g_family_up = 0;
/* Every burst's `by`, which `ib` extends in place; each init empties it. */
static pico8_list_t g_ga_by_shared;

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

static int family_ref_field(const void *ref, pico8_value_t *out_value) {
    if (ref == NULL) {
        return 0;
    }
    *out_value = pico8_value_ref((void *)ref);
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
           "jo.id: a write neither nil nor a string does not fit the member's type -- stop, do not widen the member");
    *out = value.as.string;
}

static void family_take_vec(pico8_value_t value, pico8_vec_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && value.as.ref != NULL &&
           pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_UNKNOWN &&
           "jo.ra/p: a write neither nil nor a vector ref does not fit the member's type "
           "-- stop, do not widen the member");
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
           "qh.et: a write neither nil, false, nor what bv aims at (a planet jo, qh, nl, os, hj, pm, nr, fr, mx, a jc, "
           "or bv's throwaway target) does not fit the member's type -- stop, do not widen the member");
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
    self->hp = pico8_qh_start.hp;
    self->px = pico8_qh_start.px;
    self->dq = pico8_qh_start.dq;
    self->qh = pico8_qh_start.qh;
    self->qh_is_bool = pico8_qh_start.qh_is_bool;
    self->od = pico8_qh_start.od;
    self->fp = pico8_qh_start.fp;
    self->dw = pico8_qh_start.dw;
    self->s = pico8_qh_start.s;
    self->ba = pico8_qh_start.ba;
    self->et = NULL;
    self->has_et = 0;
    self->jo.mi = pico8_qh_start.mi;
    self->jo.has_mi = 1;
    self->jo.nu = pico8_qh_start.nu;
    self->jo.hf = pico8_qh_start.hf;
    self->jo.hf_is_bool = pico8_qh_start.hf_is_bool;
    self->jo.ke.me = pico8_qh_start.me;
}

static void *pico8_qh_struct_new(const pico8_ob_table_t *overrides) {
    pico8_qh_t *self = malloc(sizeof *self);
    int k;

    assert(g_family_up && "pico8_planet_family_init() must run first");
    pico8_qh_struct_init(self, &g_qh_struct_class);
    for (k = 0; overrides != NULL && k < overrides->count; k++) {
        pico8_ob_key_t key = overrides->entries[k].key;
        int measured = key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "id") == 0;

        assert(measured && "qh: the only construction key is id (ie:ldown); another key has no member "
                           "-- stop, do not add one");
        (void)measured;
        family_take_string(overrides->entries[k].value, &self->jo.id);
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("qh");
#endif
    pico8_jo_pq((pico8_jo_t *)self);
    pico8_ke_struct_pq(&self->jo.ke);
    return self;
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
           "nl.qw, ga.qo/ro/qg/q_/iu/py/ms: a write that is not a fixed does not fit the member's type "
           "-- stop, do not widen the member");
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

    assert(g_family_up && "pico8_planet_family_init() must run first");
    pico8_qh_struct_init(&self->qh, &g_nl_struct_class);
    self->rr = pico8_nl_start.rr;
    self->nl = pico8_nl_start.nl;
    self->qw = pico8_nl_start.qw;
    self->qh.qh = pico8_nl_start.qh;
    self->qh.qh_is_bool = pico8_nl_start.qh_is_bool;
    self->qh.hp = pico8_nl_start.hp;
    self->qh.s = pico8_nl_start.s;
    self->qh.px = pico8_nl_start.px;
    self->qh.od = pico8_nl_start.od;
    self->qh.ba = pico8_nl_start.ba;
    self->qh.jo.nu = pico8_nl_start.nu;
    for (k = 0; overrides != NULL && k < overrides->count; k++) {
        pico8_ob_key_t key = overrides->entries[k].key;
        int measured = key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "id") == 0;

        assert(measured && "nl: the only construction key is id (ie:ldown); another key has no member "
                           "-- stop, do not add one");
        (void)measured;
        family_take_string(overrides->entries[k].value, &self->qh.jo.id);
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("nl");
#endif
    pico8_jo_pq((pico8_jo_t *)self);
    pico8_ke_struct_pq(&self->qh.jo.ke);
    return self;
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

    assert(g_family_up && "pico8_planet_family_init() must run first");
    pico8_qh_struct_init(&self->qh, &g_os_struct_class);
    self->qh.hp = pico8_os_start.hp;
    self->qh.px = pico8_os_start.px;
    self->qh.qh = pico8_os_start.qh;
    self->qh.qh_is_bool = pico8_os_start.qh_is_bool;
    self->qh.s = pico8_os_start.s;
    self->qh.od = pico8_os_start.od;
    self->qh.jo.nu = pico8_os_start.nu;
    self->qh.jo.hs = pico8_os_start.hs;
    for (k = 0; overrides != NULL && k < overrides->count; k++) {
        pico8_ob_key_t key = overrides->entries[k].key;
        int measured = key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "id") == 0;

        assert(measured && "os: the only construction key is id (ie:ldown); another key has no member "
                           "-- stop, do not add one");
        (void)measured;
        family_take_string(overrides->entries[k].value, &self->qh.jo.id);
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("os");
#endif
    pico8_jo_pq((pico8_jo_t *)self);
    pico8_ke_struct_pq(&self->qh.jo.ke);
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
           "hj/ga.qy: a write neither nil nor a list does not fit the member's type -- stop, do not widen the member");
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
    self->rr = pico8_hj_start.rr;
    self->py = pico8_hj_start.py;
    self->ki = pico8_hj_start.ki;
    self->pn = pico8_hj_start.pn;
    self->pk = pico8_hj_start.pk;
    self->pk_is_bool = pico8_hj_start.pk_is_bool;
    self->fc = pico8_hj_start.fc;
    self->k_ = pico8_hj_start.k_;
    self->du = pico8_hj_start.du;
    self->qo = pico8_hj_start.qo;
    self->ro = pico8_hj_start.ro;
    self->qg = pico8_hj_start.qg;
    self->q_ = pico8_hj_start.q_;
    self->qy = NULL;
    self->qh.hp = pico8_hj_start.hp;
    self->qh.qh = pico8_hj_start.qh;
    self->qh.qh_is_bool = pico8_hj_start.qh_is_bool;
    self->qh.px = pico8_hj_start.px;
    self->qh.od = pico8_hj_start.od;
    self->qh.s = pico8_hj_start.s;
    self->qh.jo.nu = pico8_hj_start.nu;
}

static void *pico8_hj_struct_new(const pico8_ob_table_t *overrides) {
    pico8_hj_t *self = malloc(sizeof *self);
    int k;

    assert(g_family_up && "pico8_planet_family_init() must run first");
    pico8_hj_struct_init(self, &g_hj_struct_class);
    for (k = 0; overrides != NULL && k < overrides->count; k++) {
        pico8_ob_key_t key = overrides->entries[k].key;
        int measured = key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "id") == 0;

        assert(measured && "hj: the only construction key is id (ie:ldown); another key has no member "
                           "-- stop, do not add one");
        (void)measured;
        family_take_string(overrides->entries[k].value, &self->qh.jo.id);
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("hj");
#endif
    pico8_f_((pico8_struct_header_t *)self);
    pico8_jo_pq((pico8_jo_t *)self);
    pico8_ke_struct_pq(&self->qh.jo.ke);
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

    spawn_ra = pico8_vec_add(pico8_pf_value_vec(pico8_value_ref(self->hj.qh.jo.ra)),
                             pico8_pf_value_vec(pico8_value_ref((void *)self->ng)));
    spawn_ra_heap = malloc(sizeof *spawn_ra_heap);
    *spawn_ra_heap = spawn_ra;

    defaults = pico8_ob_table_new();
    pico8_ob_table_set_name(defaults, "ra", pico8_value_ref(spawn_ra_heap));

    overrides = pico8_ob("\n"
                          "qo=0.97,ro=-0.1255,\n"
                          "qg=0.1255,q_=0.97,\n"
                          "iu=-0.009,py=9,ms=1.5,\n",
                          defaults);
    pico8_ob_table_set_name(overrides, "k_", pico8_value_ref((void *)&pico8_pm_ga_k__seq));
    pico8_ob_table_free(defaults);

    spawned = pico8_ga_new(overrides);
    (void)spawned;
    pico8_ob_table_free(overrides);
}

static int pico8_pm_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_pm_t *pm = self;

    if (strcmp(name, "ng") == 0) {
        if (pm->ng == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref((void *)pm->ng);
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

    assert(g_family_up && "pico8_planet_family_init() must run first");
    pico8_hj_struct_init(&self->hj, &g_pm_struct_class);
    self->ng = &pico8_pm_start.ng;
    self->fz = pico8_pm_start.fz;
    self->hj.py = pico8_pm_start.py;
    self->hj.ki = pico8_pm_start.ki;
    self->hj.pn = pico8_pm_start.pn;
    self->hj.pk = pico8_pm_start.pk;
    self->hj.pk_is_bool = pico8_pm_start.pk_is_bool;
    self->hj.du = pico8_pm_start.du;
    self->hj.fc = pico8_pm_start.fc;
    self->hj.k_ = pico8_pm_start.k_;
    self->hj.qo = pico8_pm_start.qo;
    self->hj.ro = pico8_pm_start.ro;
    self->hj.qg = pico8_pm_start.qg;
    self->hj.q_ = pico8_pm_start.q_;
    self->hj.qh.hp = pico8_pm_start.hp;
    self->hj.qh.px = pico8_pm_start.px;
    self->hj.qh.s = pico8_pm_start.s;
    self->hj.qh.jo.nu = pico8_pm_start.nu;
    self->hj.qh.jo.ke.me = pico8_pm_start.me;
    for (k = 0; overrides != NULL && k < overrides->count; k++) {
        pico8_ob_key_t key = overrides->entries[k].key;
        int is_p = key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "p") == 0;
        int is_ra = key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "ra") == 0;
        int is_id = key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "id") == 0;

        assert((is_p || is_ra || is_id) &&
               "pm: the only construction keys are p, ra, id (jo:hu); another key has no member "
               "-- stop, do not add one");
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
    return self;
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
            int k_entry;

            if (pico8_ob_table_get_name(pt, "p", &p_field) && p_field.kind == PICO8_VALUE_REF &&
                p_field.as.ref != NULL) {
                pt_p = *(const pico8_vec_t *)p_field.as.ref;
            }
            if (pico8_ob_table_get_name(pt, "l", &l_field) && l_field.kind == PICO8_VALUE_FIXED) {
                l = l_field.as.fixed;
            }
            idx = pico8_fixed_flr_int(pico8_fixed_flr(pico8_fixed_div(l, pico8_fixed_from_int(2))));
            /* TROU-21 */
            if (pico8_int_seq_at(pico8_ga_k__seq.items, pico8_ga_k__seq.count, idx, &k_entry)) {
                color = k_entry;
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

static void family_take_seq(pico8_value_t value, const pico8_int_seq_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && value.as.ref != NULL &&
           pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_UNKNOWN &&
           "ga.k_: a write neither nil nor an integer sequence ref does not fit the member's type "
           "-- stop, do not widen the member");
    *out = value.as.ref;
}

static void *pico8_ga_struct_new(const pico8_ob_table_t *overrides) {
    pico8_ga_t *self = malloc(sizeof *self);
    int k;

    assert(g_family_up && "pico8_planet_family_init() must run first");
    pico8_jo_struct_init(&self->jo, &g_ga_struct_class);
    self->fz = pico8_ga_start.fz;
    self->fc = pico8_ga_start.fc;
    self->qo = pico8_ga_start.qo;
    self->ro = pico8_ga_start.ro;
    self->qg = pico8_ga_start.qg;
    self->q_ = pico8_ga_start.q_;
    self->k_ = pico8_ga_start.k_;
    self->iu = pico8_ga_start.iu;
    self->py = pico8_ga_start.py;
    self->ms = pico8_ga_start.ms;
    self->qy = NULL;
    self->jo.by = &g_ga_by_shared;
    /* `nu=o()` is empty and replaces jo's `nu` outright, so no burst is
     * ever registered under jo/np/prober in `cg`. This is not a merge. */
    self->jo.nu = pico8_ga_start.nu;
    self->jo.hf = pico8_ga_start.hf;
    self->jo.hf_is_bool = pico8_ga_start.hf_is_bool;
    self->jo.kq = &pico8_ga_start.kq;
    self->jo.fa = pico8_ga_start.fa;
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
            family_take_seq(value, &self->k_);
        } else if (strcmp(name, "iu") == 0) {
            family_take_fixed(value, &self->iu);
        } else if (strcmp(name, "py") == 0) {
            family_take_fixed(value, &self->py);
        } else if (strcmp(name, "ms") == 0) {
            family_take_fixed(value, &self->ms);
        } else {
            assert(0 &&
                   "ga: the only construction keys are p, ra, id (jo:hu) and ra, qo, ro, qg, q_, k_, iu, py, ms "
                   "(pm:pq); another key has no member -- stop, do not add one");
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("ga");
#endif
    pico8_ga_pq(self);
    pico8_jo_pq((pico8_jo_t *)self);
    pico8_ke_struct_pq(&self->jo.ke);
    return self;
}

void pico8_planet_family_init(void) {
    assert(pico8_jo_ready() && "pico8_jo_init() must run first");
    pico8_planet_family_shutdown();
    pico8_pi_init();
    g_family_up = 1;
    pico8_list_init(&g_ga_by_shared);
}

void pico8_planet_family_shutdown(void) {
    g_family_up = 0;
}

int pico8_qh_ready(void) {
    return g_family_up;
}

static int pico8_qh_kind_field(const char *name, pico8_value_t *out_value) {
    if (!g_family_up) {
        return 0;
    }
    if (strcmp(name, "s") == 0) {
        *out_value = pico8_value_fixed(pico8_qh_start.s);
        return 1;
    }
    if (strcmp(name, "me") == 0) {
        *out_value = pico8_value_string(pico8_qh_start.me);
        return 1;
    }
    if (strcmp(name, "od") == 0) {
        *out_value = pico8_value_fixed(pico8_qh_start.od);
        return 1;
    }
    return pico8_jo_kind_field(name, out_value);
}

static const pico8_entity_kind_t g_qh_kind = {pico8_qh_struct_new, pico8_qh_kind_field};

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
    return g_family_up;
}

static int pico8_nl_kind_field(const char *name, pico8_value_t *out_value) {
    if (!g_family_up) {
        return 0;
    }
    if (strcmp(name, "rr") == 0) {
        *out_value = pico8_value_string(pico8_nl_start.rr);
        return 1;
    }
    if (strcmp(name, "s") == 0) {
        *out_value = pico8_value_fixed(pico8_nl_start.s);
        return 1;
    }
    if (strcmp(name, "od") == 0) {
        *out_value = pico8_value_fixed(pico8_nl_start.od);
        return 1;
    }
    return pico8_qh_kind_field(name, out_value);
}

static const pico8_entity_kind_t g_nl_kind = {pico8_nl_struct_new, pico8_nl_kind_field};

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
    return g_family_up;
}

static int pico8_os_kind_field(const char *name, pico8_value_t *out_value) {
    if (!g_family_up) {
        return 0;
    }
    if (strcmp(name, "s") == 0) {
        *out_value = pico8_value_fixed(pico8_os_start.s);
        return 1;
    }
    if (strcmp(name, "od") == 0) {
        *out_value = pico8_value_fixed(pico8_os_start.od);
        return 1;
    }
    return pico8_qh_kind_field(name, out_value);
}

static const pico8_entity_kind_t g_os_kind = {pico8_os_struct_new, pico8_os_kind_field};

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
    return g_family_up;
}

static int pico8_hj_kind_field(const char *name, pico8_value_t *out_value) {
    if (!g_family_up) {
        return 0;
    }
    if (strcmp(name, "rr") == 0) {
        *out_value = pico8_value_string(pico8_hj_start.rr);
        return 1;
    }
    if (strcmp(name, "s") == 0) {
        *out_value = pico8_value_fixed(pico8_hj_start.s);
        return 1;
    }
    if (strcmp(name, "od") == 0) {
        *out_value = pico8_value_fixed(pico8_hj_start.od);
        return 1;
    }
    return pico8_qh_kind_field(name, out_value);
}

static const pico8_entity_kind_t g_hj_kind = {pico8_hj_struct_new, pico8_hj_kind_field};

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
    return g_family_up;
}

static int pico8_pm_kind_field(const char *name, pico8_value_t *out_value) {
    if (!g_family_up) {
        return 0;
    }
    if (strcmp(name, "s") == 0) {
        *out_value = pico8_value_fixed(pico8_pm_start.s);
        return 1;
    }
    if (strcmp(name, "me") == 0) {
        *out_value = pico8_value_string(pico8_pm_start.me);
        return 1;
    }
    return pico8_hj_kind_field(name, out_value);
}

static const pico8_entity_kind_t g_pm_kind = {pico8_pm_struct_new, pico8_pm_kind_field};

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
    return g_family_up;
}

static int pico8_ga_kind_field(const char *name, pico8_value_t *out_value) {
    if (!g_family_up) {
        return 0;
    }
    return pico8_jo_kind_field(name, out_value);
}

static const pico8_entity_kind_t g_ga_kind = {pico8_ga_struct_new, pico8_ga_kind_field};

const pico8_entity_kind_t *pico8_ga_kind(void) {
    return &g_ga_kind;
}

const pico8_struct_class_t *pico8_ga_struct_class(void) {
    return &g_ga_struct_class;
}

pico8_ga_t *pico8_ga_new(const pico8_ob_table_t *overrides) {
    return pico8_ga_struct_new(overrides);
}
