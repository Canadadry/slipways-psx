#include "pico8_planet_family_trade.h"

#include <assert.h>
#include <stddef.h>
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

static pico8_ob_table_t *g_nr_spec = NULL;
static pico8_ob_table_t *g_fr_spec = NULL;
static pico8_ob_table_t *g_mx_spec = NULL;

static int pico8_pft_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

static pico8_list_t positional_values(const pico8_ob_table_t *table) {
    pico8_list_t list;
    int i;

    pico8_list_init(&list);
    if (table == NULL) {
        return list;
    }
    for (i = 0; i < table->count; i++) {
        if (table->entries[i].key.kind == PICO8_OB_KEY_INDEX) {
            pico8_list_add(&list, table->entries[i].value);
        }
    }
    return list;
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
    pico8_ob_table_t *overrides;
    pico8_jc_t *made;

    if (self == NULL) {
        return pico8_value_ref(NULL);
    }

    gv_result = pico8_nr_gv(self, (const pico8_struct_header_t *)&g_nr_ldown_empty);

    if (!pico8_pft_truthy(gv_result)) {
        return gv_result;
    }

    assert(pico8_jc_ready() && "pico8_jc_init() must run before nr:ldown builds a jc");
    overrides = pico8_ob_table_new();
    pico8_ob_table_set_name(overrides, "qu", pico8_value_ref(self));
    made = pico8_jc_new(overrides);
    pico8_ob_table_free(overrides);
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
        pico8_value_t entry;
        if (pico8_ob_table_get_index(self->hz, oe_count + 1, &entry) &&
            entry.kind == PICO8_VALUE_FIXED) {
            s_val = entry.as.fixed;
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

static pico8_ob_table_t *g_nr_nu = NULL;
static pico8_qz_box_t *g_nr_kq = NULL;
static pico8_fixed_t g_nr_od = 0;
static pico8_fixed_t g_nr_fp = 0;
static pico8_fixed_t g_nr_s = 0;
static pico8_fixed_t g_nr_dw = 0;
static pico8_ob_table_t *g_nr_hz = NULL;
static const char *g_nr_rr = NULL;

static void pico8_nr_defaults_reset(void) {
    g_nr_nu = NULL;
    g_nr_kq = NULL;
    g_nr_od = 0;
    g_nr_fp = 0;
    g_nr_s = 0;
    g_nr_dw = 0;
    g_nr_hz = NULL;
    g_nr_rr = NULL;
}

static int pico8_nr_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_nr_t *nr = self;

    if (strcmp(name, "hz") == 0) {
        if (nr->hz == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref(nr->hz);
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
           "qh.s: a write that is not a fixed is off its census row (fixe, never nil) -- a field under no rule of "
           "plan-3 section 5: stop, do not widen the member");
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
    self->hz = g_nr_hz;
    self->rr = g_nr_rr;
    self->qh.od = g_nr_od;
    self->qh.fp = g_nr_fp;
    self->qh.s = g_nr_s;
    self->qh.dw = g_nr_dw;
    self->qh.jo.nu = g_nr_nu;
    self->qh.jo.kq = g_nr_kq;
}

static void family_trade_take_string(pico8_value_t value, const char **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_STRING && value.as.string != NULL &&
           "jo.id: a write neither nil nor a string is off its census row (chaine) -- a field under no rule of "
           "plan-3 section 5: stop, do not widen the member");
    *out = value.as.string;
}

static void family_trade_take_vec(pico8_value_t value, pico8_vec_t **out) {
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

static void *pico8_nr_struct_new(const pico8_ob_table_t *overrides) {
    pico8_nr_t *self = malloc(sizeof *self);
    int k;

    assert(g_nr_spec != NULL && "pico8_planet_family_trade_init() must run first");
    pico8_nr_struct_init(self, &g_nr_struct_class);
    for (k = 0; overrides != NULL && k < overrides->count; k++) {
        pico8_ob_key_t key = overrides->entries[k].key;
        pico8_value_t value = overrides->entries[k].value;

        if (key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "id") == 0) {
            family_trade_take_string(value, &self->qh.jo.id);
        } else if (key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "p") == 0) {
            family_trade_take_vec(value, &self->qh.jo.p);
        } else if (key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "ra") == 0) {
            family_trade_take_vec(value, &self->qh.jo.ra);
        } else {
            assert(0 && "nr: the only measured construction keys are id (ie:ldown), p and ra (test_pico8_mz, "
                        "test_pico8_bg); another would be a field under no rule of plan-3 section 5 -- stop, do "
                        "not add a member");
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("nr");
#endif
    pico8_jo_pq((pico8_jo_t *)self);
    pico8_ke_struct_pq(&self->qh.jo.ke);
    PICO8_MEMBER_CENSUS_SEE(self, pico8_nr_t);
    return self;
}

/* fr's own defaults, read once through pico8_fr_default() at init: the very
 * values its spec holds. */
static struct {
    const char *name;
    size_t offset;
    pico8_value_t value;
} g_fr_defaults[] = {
    {"mt", offsetof(pico8_fr_t, mt), {0}},
};

static pico8_fixed_t g_fr_od = 0;
static pico8_fixed_t g_fr_s = 0;
static pico8_ob_table_t *g_fr_hz = NULL;
static const char *g_fr_rr = NULL;

static void pico8_fr_defaults_reset(void) {
    size_t i;
    g_fr_od = 0;
    g_fr_s = 0;
    g_fr_hz = NULL;
    g_fr_rr = NULL;
    for (i = 0; i < sizeof g_fr_defaults / sizeof g_fr_defaults[0]; i++) {
        g_fr_defaults[i].value = pico8_value_ref(NULL);
    }
}

static int pico8_fr_field(const void *self, const char *name, pico8_value_t *out_value) {
    if (strcmp(name, "mt") == 0) {
        const pico8_value_t *mt = &((const pico8_fr_t *)self)->mt;
        if (mt->kind == PICO8_VALUE_REF && mt->as.ref == NULL) {
            return 0;
        }
        *out_value = *mt;
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

static void *pico8_fr_struct_new(const pico8_ob_table_t *overrides) {
    pico8_fr_t *self = malloc(sizeof *self);
    size_t i;
    int k;

    assert(g_fr_spec != NULL && "pico8_planet_family_trade_init() must run first");
    pico8_nr_struct_init(&self->nr, &g_fr_struct_class);
    self->mt = pico8_value_ref(NULL);
    for (i = 0; i < sizeof g_fr_defaults / sizeof g_fr_defaults[0]; i++) {
        *(pico8_value_t *)((char *)self + g_fr_defaults[i].offset) = g_fr_defaults[i].value;
    }
    self->nr.hz = g_fr_hz;
    self->nr.rr = g_fr_rr;
    self->nr.qh.od = g_fr_od;
    self->nr.qh.s = g_fr_s;
    for (k = 0; overrides != NULL && k < overrides->count; k++) {
        pico8_ob_key_t key = overrides->entries[k].key;
        int measured = key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "id") == 0;

        assert(measured && "fr: the only measured construction key is id (ie:ldown); another would be a field "
                           "under no rule of plan-3 section 5 -- stop, do not add a member");
        (void)measured;
        family_trade_take_string(overrides->entries[k].value, &self->nr.qh.jo.id);
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("fr");
#endif
    pico8_jo_pq((pico8_jo_t *)self);
    pico8_ke_struct_pq(&self->nr.qh.jo.ke);
    PICO8_MEMBER_CENSUS_SEE(self, pico8_fr_t);
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

            if (pico8_ob_table_get_name(i_table, "qe", &qe_val)) {
                const pico8_ob_table_t *ht_table = NULL;

                ht_table = self->ht;
                if (ht_table != NULL) {
                    pico8_list_t ht_list = positional_values(ht_table);
                    int accepted = pico8_gu(&ht_list, qe_val);
                    pico8_list_free(&ht_list);
                    if (accepted) {
                        pico8_ob_table_t *kw = self->hj.qh.jo.kw;
                        if (kw != NULL) {
                            pico8_ob_table_set_name(kw, "qe", qe_val);
                        }
                        /* The previous `ht` is mx's spec default, shared
                         * by every other synthesizer: never freed. */
                        self->ht = pico8_ob_table_new();
                    }
                }
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

static pico8_ob_table_t *g_mx_nu = NULL;
static pico8_fixed_t g_mx_hp = 0;
static pico8_fixed_t g_mx_s = 0;
static pico8_ob_table_t *g_mx_px = NULL;
static pico8_fixed_t g_mx_od = 0;
static const char *g_mx_rr = NULL;
static pico8_fixed_t g_mx_py = 0;
static pico8_fixed_t g_mx_ki = 0;
static pico8_fixed_t g_mx_pn = 0;
static pico8_fixed_t g_mx_pk = 0;
static int g_mx_pk_is_bool = 0;
static pico8_fixed_t g_mx_fc = 0;
static pico8_ob_table_t *g_mx_k_ = NULL;
static pico8_fixed_t g_mx_du = 0;
static pico8_fixed_t g_mx_qo = 0;
static pico8_fixed_t g_mx_ro = 0;
static pico8_fixed_t g_mx_qg = 0;
static pico8_fixed_t g_mx_q_ = 0;
static pico8_ob_table_t *g_mx_ht = NULL;

static void pico8_mx_defaults_reset(void) {
    g_mx_nu = NULL;
    g_mx_hp = 0;
    g_mx_s = 0;
    g_mx_px = NULL;
    g_mx_od = 0;
    g_mx_rr = NULL;
    g_mx_py = 0;
    g_mx_ki = 0;
    g_mx_pn = 0;
    g_mx_pk = 0;
    g_mx_pk_is_bool = 0;
    g_mx_fc = 0;
    g_mx_k_ = NULL;
    g_mx_du = 0;
    g_mx_qo = 0;
    g_mx_ro = 0;
    g_mx_qg = 0;
    g_mx_q_ = 0;
    g_mx_ht = NULL;
}

static int pico8_mx_field(const void *self, const char *name, pico8_value_t *out_value) {
    if (strcmp(name, "ht") == 0) {
        pico8_ob_table_t *ht = ((const pico8_mx_t *)self)->ht;
        if (ht == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref(ht);
        return 1;
    }
    return pico8_hj_struct_class()->field(self, name, out_value);
}

static void family_trade_take_table(pico8_value_t value, pico8_ob_table_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && value.as.ref != NULL &&
           pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_OB_TABLE &&
           "mx.ht: a write neither nil nor a table is off its census row (ref:table) -- a field under no rule of "
           "plan-3 section 5: stop, do not widen the member");
    *out = value.as.ref;
}

/* `ht`, which mx's own `kf` writes, then hj's 21 names; 0 for any other. */
static int pico8_mx_set_field(void *self, const char *name, pico8_value_t value) {
    if (strcmp(name, "ht") == 0) {
        family_trade_take_table(value, &((pico8_mx_t *)self)->ht);
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

static void *pico8_mx_struct_new(const pico8_ob_table_t *overrides) {
    pico8_mx_t *self = malloc(sizeof *self);
    int k;

    assert(g_mx_spec != NULL && "pico8_planet_family_trade_init() must run first");
    pico8_hj_struct_init(&self->hj, &g_mx_struct_class);
    self->ht = g_mx_ht;
    self->hj.rr = g_mx_rr;
    self->hj.py = g_mx_py;
    self->hj.ki = g_mx_ki;
    self->hj.pn = g_mx_pn;
    self->hj.pk = g_mx_pk;
    self->hj.pk_is_bool = g_mx_pk_is_bool;
    self->hj.fc = g_mx_fc;
    self->hj.k_ = g_mx_k_;
    self->hj.du = g_mx_du;
    self->hj.qo = g_mx_qo;
    self->hj.ro = g_mx_ro;
    self->hj.qg = g_mx_qg;
    self->hj.q_ = g_mx_q_;
    self->hj.qh.hp = g_mx_hp;
    self->hj.qh.s = g_mx_s;
    self->hj.qh.px = g_mx_px;
    self->hj.qh.od = g_mx_od;
    self->hj.qh.jo.nu = g_mx_nu;
    for (k = 0; overrides != NULL && k < overrides->count; k++) {
        pico8_ob_key_t key = overrides->entries[k].key;
        int measured = key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "id") == 0;

        assert(measured && "mx: the only measured construction key is id (ie:ldown); another would be a field "
                           "under no rule of plan-3 section 5 -- stop, do not add a member");
        (void)measured;
        family_trade_take_string(overrides->entries[k].value, &self->hj.qh.jo.id);
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("mx");
#endif
    pico8_f_((pico8_struct_header_t *)self);
    pico8_jo_pq((pico8_jo_t *)self);
    pico8_ke_struct_pq(&self->hj.qh.jo.ke);
    PICO8_MEMBER_CENSUS_SEE(self, pico8_mx_t);
    return self;
}

static pico8_ob_table_t *family_trade_default_table(int found, pico8_value_t v) {
    assert(found && v.kind == PICO8_VALUE_REF && v.as.ref != NULL &&
           pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_OB_TABLE &&
           "nr/mx.nu, nr/fr.hz, mx.px, mx.ht, mx.k_: a default of the struct is a key of its spec and a table "
           "(ref:table)");
    (void)found;
    return v.as.ref;
}

static pico8_qz_box_t *family_trade_default_box(int found, pico8_value_t v) {
    assert(found && v.kind == PICO8_VALUE_REF && v.as.ref != NULL &&
           pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_UNKNOWN &&
           "nr.kq: a default of the struct is a key of nr's spec and a box b() (ref:autre)");
    (void)found;
    return v.as.ref;
}

static pico8_fixed_t family_trade_default_fixed(int found, pico8_value_t v) {
    assert(found && v.kind == PICO8_VALUE_FIXED &&
           "nr/fr/mx.od, nr.fp/dw, nr/fr/mx.s, mx.hp, mx.py/ki/pn/fc/du/qo/ro/qg/q_: a default of the struct is a "
           "key of its spec and a fixed (fixe)");
    (void)found;
    return v.as.fixed;
}

static const char *family_trade_default_string(int found, pico8_value_t v) {
    assert(found && v.kind == PICO8_VALUE_STRING && v.as.string != NULL &&
           "nr/fr/mx.rr: a default of the struct is a key of its spec and a string (chaine)");
    (void)found;
    return v.as.string;
}

static pico8_fixed_t family_trade_default_hf(int found, pico8_value_t v, int *is_bool) {
    assert(found && (v.kind == PICO8_VALUE_FIXED || v.kind == PICO8_VALUE_BOOL) &&
           "mx.pk: a default of the struct is a key of mx's spec and a fixed or a boolean (bool,fixe)");
    (void)found;
    *is_bool = v.kind == PICO8_VALUE_BOOL;
    return v.kind == PICO8_VALUE_BOOL ? (pico8_fixed_t)(v.as.boolean != 0) : v.as.fixed;
}

void pico8_planet_family_trade_init(void) {
    size_t i;
    pico8_value_t v;
    int found;

    assert(pico8_qh_ready() && pico8_hj_ready() &&
           "pico8_planet_family_init() must run first");
    pico8_planet_family_trade_shutdown();
    pico8_pi_init();

    g_nr_spec = pico8_ob("\n"
                         "nu=o(\"nr\",\"np\",\"prober\"),\n"
                         "od=10,fp=1,\n"
                         "kq=b(-4,-4,5,5),\n"
                         "s=170,\n"
                         "hz=o(170,170,170,168),\n"
                         "dw=0.01,\n"
                         "rr=\"slipgates\",\n",
                         NULL);
    found = pico8_nr_default("nu", &v);
    g_nr_nu = family_trade_default_table(found, v);
    found = pico8_nr_default("kq", &v);
    g_nr_kq = family_trade_default_box(found, v);
    found = pico8_nr_default("od", &v);
    g_nr_od = family_trade_default_fixed(found, v);
    found = pico8_nr_default("fp", &v);
    g_nr_fp = family_trade_default_fixed(found, v);
    found = pico8_nr_default("s", &v);
    g_nr_s = family_trade_default_fixed(found, v);
    found = pico8_nr_default("dw", &v);
    g_nr_dw = family_trade_default_fixed(found, v);
    found = pico8_nr_default("hz", &v);
    g_nr_hz = family_trade_default_table(found, v);
    found = pico8_nr_default("rr", &v);
    g_nr_rr = family_trade_default_string(found, v);

    g_fr_spec = pico8_ob("\n"
                         "mt=1,\n"
                         "rr=\"infraspace\",\n"
                         "od=25,\n"
                         "s=78,hz=o(78,78,78,76),\n",
                         NULL);
    for (i = 0; i < sizeof g_fr_defaults / sizeof g_fr_defaults[0]; i++) {
        int found = pico8_fr_default(g_fr_defaults[i].name, &g_fr_defaults[i].value);
        assert(found && "fr: every default of the struct is a key of fr's spec");
        (void)found;
    }
    found = pico8_fr_default("od", &v);
    g_fr_od = family_trade_default_fixed(found, v);
    found = pico8_fr_default("s", &v);
    g_fr_s = family_trade_default_fixed(found, v);
    found = pico8_fr_default("hz", &v);
    g_fr_hz = family_trade_default_table(found, v);
    found = pico8_fr_default("rr", &v);
    g_fr_rr = family_trade_default_string(found, v);

    g_mx_spec = pico8_ob("\n"
                         "nu=o(\"jo\",\"np\",\"prober\"),\n"
                         "rr=\"void synthesis\",\n"
                         "hp=5,s=66,px=o(\"synthesizer\",c=13),\n"
                         "od=40,\n"
                         "ht=o(28,20,21,22),\n"
                         "py=0.12,ki=9,\n"
                         "pn=5,pk=1,\n"
                         "fc=0.3,\n"
                         "k_=o(1,1,5,5,3,3,3,5,5,1),\n"
                         "du=5,\n"
                         "qo=0.98,ro=-0.07,\n"
                         "qg=0.07,q_=0.98,\n",
                         NULL);
    found = pico8_mx_default("nu", &v);
    g_mx_nu = family_trade_default_table(found, v);
    found = pico8_mx_default("hp", &v);
    g_mx_hp = family_trade_default_fixed(found, v);
    found = pico8_mx_default("s", &v);
    g_mx_s = family_trade_default_fixed(found, v);
    found = pico8_mx_default("px", &v);
    g_mx_px = family_trade_default_table(found, v);
    found = pico8_mx_default("od", &v);
    g_mx_od = family_trade_default_fixed(found, v);
    found = pico8_mx_default("rr", &v);
    g_mx_rr = family_trade_default_string(found, v);
    found = pico8_mx_default("py", &v);
    g_mx_py = family_trade_default_fixed(found, v);
    found = pico8_mx_default("ki", &v);
    g_mx_ki = family_trade_default_fixed(found, v);
    found = pico8_mx_default("pn", &v);
    g_mx_pn = family_trade_default_fixed(found, v);
    found = pico8_mx_default("pk", &v);
    g_mx_pk = family_trade_default_hf(found, v, &g_mx_pk_is_bool);
    found = pico8_mx_default("fc", &v);
    g_mx_fc = family_trade_default_fixed(found, v);
    found = pico8_mx_default("k_", &v);
    g_mx_k_ = family_trade_default_table(found, v);
    found = pico8_mx_default("du", &v);
    g_mx_du = family_trade_default_fixed(found, v);
    found = pico8_mx_default("qo", &v);
    g_mx_qo = family_trade_default_fixed(found, v);
    found = pico8_mx_default("ro", &v);
    g_mx_ro = family_trade_default_fixed(found, v);
    found = pico8_mx_default("qg", &v);
    g_mx_qg = family_trade_default_fixed(found, v);
    found = pico8_mx_default("q_", &v);
    g_mx_q_ = family_trade_default_fixed(found, v);
    found = pico8_mx_default("ht", &v);
    g_mx_ht = family_trade_default_table(found, v);
    PICO8_MEMBER_CENSUS_SEE(&g_nr_ldown_empty, pico8_nr_ldown_empty_t);
}

void pico8_planet_family_trade_shutdown(void) {
    pico8_mx_defaults_reset();
    pico8_fr_defaults_reset();
    pico8_nr_defaults_reset();
    pico8_ob_table_free(g_mx_spec);
    g_mx_spec = NULL;
    pico8_ob_table_free(g_fr_spec);
    g_fr_spec = NULL;
    pico8_ob_table_free(g_nr_spec);
    g_nr_spec = NULL;
}

int pico8_nr_ready(void) {
    return g_nr_spec != NULL;
}

int pico8_nr_default(const char *name, pico8_value_t *out_value) {
    return g_nr_spec != NULL &&
           (pico8_ob_table_get_name(g_nr_spec, name, out_value) || pico8_qh_default(name, out_value));
}

static const pico8_entity_kind_t g_nr_kind = {pico8_nr_struct_new, pico8_nr_default};

const pico8_entity_kind_t *pico8_nr_kind(void) {
    return &g_nr_kind;
}

const pico8_struct_class_t *pico8_nr_struct_class(void) {
    return &g_nr_struct_class;
}

pico8_nr_t *pico8_nr_new(const pico8_ob_table_t *overrides) {
    return pico8_nr_struct_new(overrides);
}

int pico8_fr_ready(void) {
    return g_fr_spec != NULL;
}

int pico8_fr_default(const char *name, pico8_value_t *out_value) {
    return g_fr_spec != NULL &&
           (pico8_ob_table_get_name(g_fr_spec, name, out_value) || pico8_nr_default(name, out_value));
}

static const pico8_entity_kind_t g_fr_kind = {pico8_fr_struct_new, pico8_fr_default};

const pico8_entity_kind_t *pico8_fr_kind(void) {
    return &g_fr_kind;
}

const pico8_struct_class_t *pico8_fr_struct_class(void) {
    return &g_fr_struct_class;
}

pico8_fr_t *pico8_fr_new(const pico8_ob_table_t *overrides) {
    return pico8_fr_struct_new(overrides);
}

int pico8_mx_ready(void) {
    return g_mx_spec != NULL;
}

int pico8_mx_default(const char *name, pico8_value_t *out_value) {
    return g_mx_spec != NULL &&
           (pico8_ob_table_get_name(g_mx_spec, name, out_value) || pico8_hj_default(name, out_value));
}

static const pico8_entity_kind_t g_mx_kind = {pico8_mx_struct_new, pico8_mx_default};

const pico8_entity_kind_t *pico8_mx_kind(void) {
    return &g_mx_kind;
}

const pico8_struct_class_t *pico8_mx_struct_class(void) {
    return &g_mx_struct_class;
}

pico8_mx_t *pico8_mx_new(const pico8_ob_table_t *overrides) {
    return pico8_mx_struct_new(overrides);
}
