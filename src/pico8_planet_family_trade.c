#include "pico8_planet_family_trade.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_struct.h"
#include "pico8_collect.h"
#include "pico8_entity.h"
#include "pico8_list.h"
#include "pico8_ob.h"
#include "pico8_particle.h"
#include "pico8_pi.h"
#include "pico8_planet.h"
#include "pico8_planet_family.h"
#include "pico8_planet_trade.h"
#include "pico8_probe.h"
#include "pico8_slipway.h"
#include "pico8_trade.h"
#include "pico8_type_tag.h"
#include "pico8_ref_audit.h"

/* What `mx:kf` leaves in `ht` once it accepts a resource: `o()`. */
static const pico8_int_seq_t g_mx_ht_spent = {0, NULL};

static int g_family_trade_up = 0;

static int pico8_pft_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

pico8_value_t pico8_nr_gv(const pico8_nr_t *self, const pico8_struct_header_t *s) {
    pico8_value_t qu_val = pico8_value_ref(NULL);
    int qu_not_self;
    int oe_count = 0;

    if (self == NULL) {
        return pico8_value_ref(NULL);
    }

    if (s != NULL) {
        pico8_entity_get_field(s, "qu", &qu_val);
    }
    qu_not_self = !(qu_val.kind == PICO8_VALUE_REF && qu_val.as.ref == (const void *)self);

    oe_count = self->qh.jo.oe != NULL ? self->qh.jo.oe->count : 0;

    return pico8_value_bool(qu_not_self && oe_count < 3);
}

static pico8_value_t pico8_nr_gv_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    const pico8_struct_header_t *s = NULL;
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        s = (const pico8_struct_header_t *)arg0.as.ref;
    }
    return pico8_nr_gv((const pico8_nr_t *)self, s);
}

static int pico8_nr_ldown_empty_field(const void *self, const char *name, pico8_value_t *out_value) {
    (void)self;
    (void)name;
    (void)out_value;
    return 0;
}

static const pico8_struct_class_t g_nr_ldown_empty_struct_class = {
    {0},
    pico8_nr_ldown_empty_field,
    NULL,
};

typedef struct {
    pico8_struct_header_t header;
} pico8_nr_ldown_empty_t;

/* The `{}` `nr:ldown` hands to `nr:gv`: never allocated, never freed; no
 * field, and no writer, so a write reaching it stops the run. */
static const pico8_nr_ldown_empty_t g_nr_ldown_empty = {{PICO8_TYPE_TAG_STRUCT, &g_nr_ldown_empty_struct_class}};

pico8_value_t pico8_nr_ldown(pico8_nr_t *self) {
    pico8_value_t gv_result;
    pico8_jc_keys_t keys = {0};
    pico8_jc_t *made;

    if (self == NULL) {
        return pico8_value_ref(NULL);
    }

    gv_result = pico8_nr_gv(self, (const pico8_struct_header_t *)&g_nr_ldown_empty);

    if (!pico8_pft_truthy(gv_result)) {
        return gv_result;
    }

    assert(pico8_jc_ready() && "pico8_jc_init() must run before nr:ldown builds a jc");
    keys.qu = (pico8_struct_header_t *)self;
    made = pico8_jc_new(&keys);
    return pico8_value_ref(made);
}

static pico8_value_t pico8_nr_ldown_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
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

static pico8_value_t pico8_nr_hb_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_nr_hb((pico8_nr_t *)self);
    return pico8_value_ref(NULL);
}

void pico8_nr_cn(pico8_nr_t *self, pico8_jo_trade_proposal_t *lw) {
    pico8_trade_t *t;
    const pico8_list_t *oe = NULL;
    pico8_list_iter_t it;
    pico8_value_t w_val;

    if (self == NULL || lw == NULL) {
        return;
    }
    t = lw->t;

    oe = self->qh.jo.oe;

    it = pico8_list_all(oe);
    while (pico8_list_iter_next(&it, &w_val)) {
        pico8_struct_header_t *w;
        const pico8_list_t *t_rl = NULL;

        if (w_val.kind != PICO8_VALUE_REF || w_val.as.ref == NULL) {
            continue;
        }
        w = (pico8_struct_header_t *)w_val.as.ref;

        if (lw->mu <= pico8_fixed_from_int(0)) {
            continue;
        }

        if (t != NULL && t->rl != NULL) {
            t_rl = t->rl;
        }
        if (pico8_gu(t_rl, pico8_value_ref(w))) {
            continue;
        }

        {
            pico8_jo_trade_proposal_t gf = *lw;
            pico8_trade_t *extended = pico8_trade_kg(t, pico8_value_ref(self), pico8_value_ref(w));
            pico8_value_t eb_result;

            gf.t = extended;
            if (pico8_mv(w, "eb", 1, pico8_value_ref(self), pico8_value_ref(NULL), &eb_result) &&
                eb_result.kind == PICO8_VALUE_REF && eb_result.as.ref != NULL) {
                pico8_value_t discard;
                /* `&gf` is a stack address passed to a dispatched `cn`:
                 * safe only while no `cn` implementation stores its
                 * proposal. */
                pico8_mv((pico8_struct_header_t *)eb_result.as.ref, "cn", 1, pico8_value_ref(&gf),
                         pico8_value_ref(NULL), &discard);
            }
            lw->mu = gf.mu;
        }
    }
}

static pico8_value_t pico8_nr_cn_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_jo_trade_proposal_t *lw = NULL;
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        lw = (pico8_jo_trade_proposal_t *)arg0.as.ref;
    }
    pico8_nr_cn((pico8_nr_t *)self, lw);
    return pico8_value_ref(NULL);
}

static int pico8_nr_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_nr_t *nr = self;

    if (strcmp(name, "hz") == 0) {
        if (nr->hz == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref((void *)nr->hz);
        return 1;
    }
    if (strcmp(name, "rr") == 0) {
        if (nr->rr == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(nr->rr);
        return 1;
    }
    return pico8_qh_struct_class()->field(self, name, out_value);
}

static void family_trade_take_fixed(pico8_value_t value, pico8_fixed_t *out) {
    assert(value.kind == PICO8_VALUE_FIXED &&
           "qh.s: a write that is not a fixed does not fit the member's type -- stop, do not widen the member");
    *out = value.as.fixed;
}

/* `s`, which nr's own `hb` writes, then qh's 20 names; 0 for any other. */
static int pico8_nr_set_field(void *self, const char *name, pico8_value_t value) {
    if (strcmp(name, "s") == 0) {
        family_trade_take_fixed(value, &((pico8_nr_t *)self)->qh.s);
        return 1;
    }
    return pico8_qh_struct_class()->set_field(self, name, value);
}

static const pico8_struct_class_t g_nr_struct_class = {
    {
        [PICO8_METHOD_HD] = pico8_jo_hd_dispatch,
        [PICO8_METHOD_KJ] = pico8_jo_kj_dispatch,
        [PICO8_METHOD_LDOWN] = pico8_nr_ldown_dispatch,
        [PICO8_METHOD_HB] = pico8_nr_hb_dispatch,
        [PICO8_METHOD_GG] = pico8_jo_gg_dispatch,
        [PICO8_METHOD_GV] = pico8_nr_gv_dispatch,
        [PICO8_METHOD_HK] = pico8_jo_hk_dispatch,
        [PICO8_METHOD_KF] = pico8_jo_kf_dispatch,
        [PICO8_METHOD_CN] = pico8_nr_cn_dispatch,
        [PICO8_METHOD_IB] = pico8_jo_ib_dispatch,
        [PICO8_METHOD_LL] = pico8_jo_ll_dispatch,
        [PICO8_METHOD_HQ] = pico8_bv_dispatch,
        [PICO8_METHOD_EV] = pico8_dl_dispatch,
        [PICO8_METHOD_EY] = pico8_d__dispatch,
        [PICO8_METHOD_LY] = pico8_qh_ly_dispatch,
    },
    pico8_nr_field,
    pico8_nr_set_field,
};

/* Initializes an allocated pico8_nr_t under `cls`: qh's part, nr's members
 * nil, nr's defaults. Reads no key, runs neither `pq` nor `lt`, records no
 * coverage. Requires `pico8_planet_family_trade_init()`. */
static void pico8_nr_struct_init(pico8_nr_t *self, const pico8_struct_class_t *cls) {
    pico8_qh_struct_init(&self->qh, cls);
    self->hz = pico8_nr_start.hz;
    self->rr = pico8_nr_start.rr;
    self->qh.od = pico8_nr_start.od;
    self->qh.fp = pico8_nr_start.fp;
    self->qh.s = pico8_nr_start.s;
    self->qh.dw = pico8_nr_start.dw;
    self->qh.jo.nu = pico8_nr_start.nu;
    self->qh.jo.kq = &pico8_nr_start.kq;
}

static void *pico8_nr_struct_new(const pico8_jo_keys_t *keys) {
    pico8_nr_t *self = malloc(sizeof *self);

    assert(g_family_trade_up && "pico8_planet_family_trade_init() must run first");
    pico8_nr_struct_init(self, &g_nr_struct_class);
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
    pico8_ke_struct_pq(&self->qh.jo.ke);
    return self;
}

static int pico8_fr_field(const void *self, const char *name, pico8_value_t *out_value) {
    if (strcmp(name, "mt") == 0) {
        *out_value = pico8_value_fixed(((const pico8_fr_t *)self)->mt);
        return 1;
    }
    return pico8_nr_field(self, name, out_value);
}

static const pico8_struct_class_t g_fr_struct_class = {
    {
        [PICO8_METHOD_HD] = pico8_jo_hd_dispatch,
        [PICO8_METHOD_KJ] = pico8_jo_kj_dispatch,
        [PICO8_METHOD_LDOWN] = pico8_nr_ldown_dispatch,
        [PICO8_METHOD_HB] = pico8_nr_hb_dispatch,
        [PICO8_METHOD_GG] = pico8_jo_gg_dispatch,
        [PICO8_METHOD_GV] = pico8_nr_gv_dispatch,
        [PICO8_METHOD_HK] = pico8_jo_hk_dispatch,
        [PICO8_METHOD_KF] = pico8_jo_kf_dispatch,
        [PICO8_METHOD_CN] = pico8_nr_cn_dispatch,
        [PICO8_METHOD_IB] = pico8_jo_ib_dispatch,
        [PICO8_METHOD_LL] = pico8_jo_ll_dispatch,
        [PICO8_METHOD_HQ] = pico8_bv_dispatch,
        [PICO8_METHOD_EV] = pico8_dl_dispatch,
        [PICO8_METHOD_EY] = pico8_d__dispatch,
        [PICO8_METHOD_LY] = pico8_qh_ly_dispatch,
    },
    pico8_fr_field,
    pico8_nr_set_field,
};

static void *pico8_fr_struct_new(const pico8_jo_keys_t *keys) {
    pico8_fr_t *self = malloc(sizeof *self);

    assert(g_family_trade_up && "pico8_planet_family_trade_init() must run first");
    pico8_nr_struct_init(&self->nr, &g_fr_struct_class);
    self->mt = pico8_fr_start.mt;
    self->nr.hz = pico8_fr_start.hz;
    self->nr.rr = pico8_fr_start.rr;
    self->nr.qh.od = pico8_fr_start.od;
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
    pico8_ke_struct_pq(&self->nr.qh.jo.ke);
    return self;
}

void pico8_mx_kf(pico8_mx_t *self, pico8_struct_header_t *sw) {
    pico8_value_t eb_result;
    pico8_struct_header_t *cp = NULL;

    if (self != NULL && sw != NULL &&
        pico8_mv(sw, "eb", 1, pico8_value_ref(self), pico8_value_ref(NULL), &eb_result) &&
        eb_result.kind == PICO8_VALUE_REF && eb_result.as.ref != NULL) {
        cp = (pico8_struct_header_t *)eb_result.as.ref;
    }

    if (cp != NULL) {
        pico8_value_t jy_val;
        const pico8_list_t *jy = NULL;
        pico8_list_iter_t it;
        pico8_value_t i_val;

        if (pico8_entity_get_field(cp, "jy", &jy_val) && jy_val.kind == PICO8_VALUE_REF && jy_val.as.ref != NULL) {
            jy = PICO8_REF_AS_LIST_CONST(jy_val.as.ref);
        }

        it = pico8_list_all(jy);
        while (pico8_list_iter_next(&it, &i_val)) {
            const pico8_ob_table_t *i_table;
            pico8_value_t gc_val;
            pico8_fixed_t gc = pico8_fixed_from_int(0);
            pico8_value_t qe_val;

            if (i_val.kind != PICO8_VALUE_REF || i_val.as.ref == NULL) {
                continue;
            }
            i_table = (const pico8_ob_table_t *)i_val.as.ref;

            if (pico8_ob_table_get_name(i_table, "gc", &gc_val) && gc_val.kind == PICO8_VALUE_FIXED) {
                gc = gc_val.as.fixed;
            }
            if (gc != pico8_fixed_from_int(0)) {
                continue;
            }

            if (pico8_ob_table_get_name(i_table, "qe", &qe_val) && self->ht != NULL &&
                qe_val.kind == PICO8_VALUE_FIXED && pico8_int_seq_has_fixed(self->ht, qe_val.as.fixed)) {
                pico8_kw_t *kw = self->hj.qh.jo.kw;
                if (kw != NULL) {
                    kw->has_qe = 1;
                    kw->qe = qe_val.as.fixed;
                }
                self->ht = &g_mx_ht_spent;
            }
        }
    }

    pico8_jo_kf((pico8_jo_t *)self, sw);
}

static pico8_value_t pico8_mx_kf_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_struct_header_t *sw = NULL;
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        sw = (pico8_struct_header_t *)arg0.as.ref;
    }
    pico8_mx_kf((pico8_mx_t *)self, sw);
    return pico8_value_ref(NULL);
}

static int pico8_mx_field(const void *self, const char *name, pico8_value_t *out_value) {
    if (strcmp(name, "ht") == 0) {
        const pico8_int_seq_t *ht = ((const pico8_mx_t *)self)->ht;
        if (ht == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref((void *)ht);
        return 1;
    }
    return pico8_hj_struct_class()->field(self, name, out_value);
}

static void family_trade_take_seq(pico8_value_t value, const pico8_int_seq_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && value.as.ref != NULL &&
           pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_UNKNOWN &&
           "mx.ht: a write neither nil nor an integer sequence ref does not fit the member's type "
           "-- stop, do not widen the member");
    *out = value.as.ref;
}

/* `ht`, which mx's own `kf` writes, then hj's 21 names; 0 for any other. */
static int pico8_mx_set_field(void *self, const char *name, pico8_value_t value) {
    if (strcmp(name, "ht") == 0) {
        family_trade_take_seq(value, &((pico8_mx_t *)self)->ht);
        return 1;
    }
    return pico8_hj_struct_class()->set_field(self, name, value);
}

static const pico8_struct_class_t g_mx_struct_class = {
    {
        [PICO8_METHOD_HD] = pico8_jo_hd_dispatch,
        [PICO8_METHOD_KJ] = pico8_hj_kj_dispatch,
        [PICO8_METHOD_LDOWN] = pico8_jo_ldown_dispatch,
        [PICO8_METHOD_HB] = pico8_jo_hb_dispatch,
        [PICO8_METHOD_GG] = pico8_jo_gg_dispatch,
        [PICO8_METHOD_GV] = pico8_jo_gv_dispatch,
        [PICO8_METHOD_HK] = pico8_jo_hk_dispatch,
        [PICO8_METHOD_KF] = pico8_mx_kf_dispatch,
        [PICO8_METHOD_CN] = pico8_jo_cn_dispatch,
        [PICO8_METHOD_IB] = pico8_jo_ib_dispatch,
        [PICO8_METHOD_LL] = pico8_jo_ll_dispatch,
        [PICO8_METHOD_HQ] = pico8_bv_dispatch,
        [PICO8_METHOD_EV] = pico8_dl_dispatch,
        [PICO8_METHOD_EY] = pico8_d__dispatch,
        [PICO8_METHOD_LY] = pico8_qh_ly_dispatch,
        [PICO8_METHOD_MM] = pico8_g__mm,
    },
    pico8_mx_field,
    pico8_mx_set_field,
};

static void *pico8_mx_struct_new(const pico8_jo_keys_t *keys) {
    pico8_mx_t *self = malloc(sizeof *self);

    assert(g_family_trade_up && "pico8_planet_family_trade_init() must run first");
    pico8_hj_struct_init(&self->hj, &g_mx_struct_class);
    self->ht = pico8_mx_start.ht;
    self->hj.rr = pico8_mx_start.rr;
    self->hj.py = pico8_mx_start.py;
    self->hj.ki = pico8_mx_start.ki;
    self->hj.pn = pico8_mx_start.pn;
    self->hj.pk = pico8_mx_start.pk;
    self->hj.pk_is_bool = pico8_mx_start.pk_is_bool;
    self->hj.fc = pico8_mx_start.fc;
    self->hj.k_ = pico8_mx_start.k_;
    self->hj.du = pico8_mx_start.du;
    self->hj.qo = pico8_mx_start.qo;
    self->hj.ro = pico8_mx_start.ro;
    self->hj.qg = pico8_mx_start.qg;
    self->hj.q_ = pico8_mx_start.q_;
    self->hj.qh.hp = pico8_mx_start.hp;
    self->hj.qh.s = pico8_mx_start.s;
    self->hj.qh.px = pico8_mx_start.px;
    self->hj.qh.od = pico8_mx_start.od;
    self->hj.qh.jo.nu = pico8_mx_start.nu;
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
    pico8_ke_struct_pq(&self->hj.qh.jo.ke);
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

static int pico8_nr_kind_field(const char *name, pico8_value_t *out_value) {
    if (!g_family_trade_up) {
        return 0;
    }
    if (strcmp(name, "rr") == 0) {
        *out_value = pico8_value_string(pico8_nr_start.rr);
        return 1;
    }
    if (strcmp(name, "s") == 0) {
        *out_value = pico8_value_fixed(pico8_nr_start.s);
        return 1;
    }
    if (strcmp(name, "od") == 0) {
        *out_value = pico8_value_fixed(pico8_nr_start.od);
        return 1;
    }
    return pico8_qh_kind()->field(name, out_value);
}

static const pico8_entity_kind_t g_nr_kind = {pico8_nr_struct_new, pico8_nr_kind_field};

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

static int pico8_fr_kind_field(const char *name, pico8_value_t *out_value) {
    if (!g_family_trade_up) {
        return 0;
    }
    if (strcmp(name, "rr") == 0) {
        *out_value = pico8_value_string(pico8_fr_start.rr);
        return 1;
    }
    if (strcmp(name, "s") == 0) {
        *out_value = pico8_value_fixed(pico8_fr_start.s);
        return 1;
    }
    if (strcmp(name, "od") == 0) {
        *out_value = pico8_value_fixed(pico8_fr_start.od);
        return 1;
    }
    return pico8_nr_kind_field(name, out_value);
}

static const pico8_entity_kind_t g_fr_kind = {pico8_fr_struct_new, pico8_fr_kind_field};

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

static int pico8_mx_kind_field(const char *name, pico8_value_t *out_value) {
    if (!g_family_trade_up) {
        return 0;
    }
    if (strcmp(name, "rr") == 0) {
        *out_value = pico8_value_string(pico8_mx_start.rr);
        return 1;
    }
    if (strcmp(name, "s") == 0) {
        *out_value = pico8_value_fixed(pico8_mx_start.s);
        return 1;
    }
    if (strcmp(name, "od") == 0) {
        *out_value = pico8_value_fixed(pico8_mx_start.od);
        return 1;
    }
    return pico8_hj_kind()->field(name, out_value);
}

static const pico8_entity_kind_t g_mx_kind = {pico8_mx_struct_new, pico8_mx_kind_field};

const pico8_entity_kind_t *pico8_mx_kind(void) {
    return &g_mx_kind;
}

const pico8_struct_class_t *pico8_mx_struct_class(void) {
    return &g_mx_struct_class;
}

pico8_mx_t *pico8_mx_new(const pico8_jo_keys_t *keys) {
    return pico8_mx_struct_new(keys);
}
