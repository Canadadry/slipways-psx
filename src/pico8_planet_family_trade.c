#include "pico8_planet_family_trade.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_class.h"
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
#include "pico8_trade.h"
#include "pico8_ref_audit.h"

static pico8_class_t *g_nr_class = NULL;
static pico8_class_t *g_fr_class = NULL;
static pico8_class_t *g_mx_class = NULL;

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

pico8_value_t pico8_nr_gv(const pico8_instance_t *self, const pico8_instance_t *s) {
    pico8_value_t qu_val = pico8_value_ref(NULL);
    int qu_not_self;
    pico8_value_t oe_val;
    int oe_count = 0;

    if (self == NULL) {
        return pico8_value_ref(NULL);
    }

    if (s != NULL) {
        pico8_entity_get_field(s, "qu", &qu_val);
    }
    qu_not_self = !(qu_val.kind == PICO8_VALUE_REF && qu_val.as.ref == (const void *)self);

    if (pico8_entity_get_field(self, "oe", &oe_val) && oe_val.kind == PICO8_VALUE_REF && oe_val.as.ref != NULL) {
        oe_count = PICO8_REF_AS_LIST_CONST(oe_val.as.ref)->count;
    }

    return pico8_value_bool(qu_not_self && oe_count < 3);
}

static pico8_value_t pico8_nr_gv_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    const pico8_instance_t *s = NULL;
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        s = (const pico8_instance_t *)arg0.as.ref;
    }
    return pico8_nr_gv((const pico8_instance_t *)self, s);
}

pico8_value_t pico8_nr_ldown(pico8_instance_t *self) {
    pico8_ob_table_t *empty_fields;
    pico8_instance_t empty;
    pico8_value_t gv_result;
    pico8_class_t *jc_class;
    pico8_ob_table_t *overrides;
    pico8_instance_t *made;

    if (self == NULL) {
        return pico8_value_ref(NULL);
    }

    empty_fields = pico8_ob_table_new();
    empty.tag = PICO8_TYPE_TAG_INSTANCE;
    empty.fields = empty_fields;
    empty.cls = NULL;
    gv_result = pico8_nr_gv(self, &empty);
    pico8_ob_table_free(empty_fields);

    if (!pico8_pft_truthy(gv_result)) {
        return gv_result;
    }

    jc_class = pico8_class_lookup_named("jc");
    if (jc_class == NULL) {
        return pico8_value_ref(NULL);
    }
    overrides = pico8_ob_table_new();
    pico8_ob_table_set_name(overrides, "qu", pico8_value_ref(self));
    made = pico8_class_instantiate(jc_class, overrides);
    pico8_ob_table_free(overrides);
    return pico8_value_ref(made);
}

static pico8_value_t pico8_nr_ldown_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_nr_ldown((pico8_instance_t *)self);
}

void pico8_nr_hb(pico8_instance_t *self) {
    pico8_value_t oe_val;
    int oe_count = 0;
    pico8_value_t hz_val;
    pico8_fixed_t s_val = pico8_fixed_from_int(0);

    if (self == NULL) {
        return;
    }

    if (pico8_entity_get_field(self, "oe", &oe_val) && oe_val.kind == PICO8_VALUE_REF && oe_val.as.ref != NULL) {
        oe_count = PICO8_REF_AS_LIST_CONST(oe_val.as.ref)->count;
    }

    if (pico8_entity_get_field(self, "hz", &hz_val) && hz_val.kind == PICO8_VALUE_REF && hz_val.as.ref != NULL) {
        pico8_value_t entry;
        if (pico8_ob_table_get_index((const pico8_ob_table_t *)hz_val.as.ref, oe_count + 1, &entry) &&
            entry.kind == PICO8_VALUE_FIXED) {
            s_val = entry.as.fixed;
        }
    }

    pico8_entity_set_field(self, "s", pico8_value_fixed(s_val));
}

static pico8_value_t pico8_nr_hb_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_nr_hb((pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

void pico8_nr_cn(pico8_instance_t *self, pico8_jo_trade_proposal_t *lw) {
    pico8_instance_t *t;
    pico8_value_t oe_val;
    const pico8_list_t *oe = NULL;
    pico8_list_iter_t it;
    pico8_value_t w_val;

    if (self == NULL || lw == NULL) {
        return;
    }
    t = lw->t;

    if (pico8_entity_get_field(self, "oe", &oe_val) && oe_val.kind == PICO8_VALUE_REF && oe_val.as.ref != NULL) {
        oe = PICO8_REF_AS_LIST_CONST(oe_val.as.ref);
    }

    it = pico8_list_all(oe);
    while (pico8_list_iter_next(&it, &w_val)) {
        pico8_instance_t *w;
        pico8_value_t t_rl_val;
        const pico8_list_t *t_rl = NULL;

        if (w_val.kind != PICO8_VALUE_REF || w_val.as.ref == NULL) {
            continue;
        }
        w = (pico8_instance_t *)w_val.as.ref;

        if (lw->mu <= pico8_fixed_from_int(0)) {
            continue;
        }

        if (t != NULL && pico8_instance_get_field(t, "rl", &t_rl_val) && t_rl_val.kind == PICO8_VALUE_REF &&
            t_rl_val.as.ref != NULL) {
            t_rl = PICO8_REF_AS_LIST_CONST(t_rl_val.as.ref);
        }
        if (pico8_gu(t_rl, pico8_value_ref(w))) {
            continue;
        }

        {
            pico8_jo_trade_proposal_t gf = *lw;
            pico8_instance_t *extended = pico8_trade_kg(t, pico8_value_ref(self), pico8_value_ref(w));
            pico8_value_t eb_result;

            gf.t = extended;
            if (pico8_mv(w, "eb", 1, pico8_value_ref(self), pico8_value_ref(NULL), &eb_result) &&
                eb_result.kind == PICO8_VALUE_REF && eb_result.as.ref != NULL) {
                pico8_value_t discard;
                /* `&gf` is a stack address passed to a dispatched `cn`:
                 * safe only while no `cn` implementation stores its
                 * proposal. */
                pico8_mv((pico8_instance_t *)eb_result.as.ref, "cn", 1, pico8_value_ref(&gf), pico8_value_ref(NULL),
                         &discard);
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
    pico8_nr_cn((pico8_instance_t *)self, lw);
    return pico8_value_ref(NULL);
}

void pico8_mx_kf(pico8_mx_t *self, pico8_instance_t *sw) {
    pico8_value_t eb_result;
    pico8_instance_t *cp = NULL;

    if (self != NULL && sw != NULL &&
        pico8_mv(sw, "eb", 1, pico8_value_ref(self), pico8_value_ref(NULL), &eb_result) &&
        eb_result.kind == PICO8_VALUE_REF && eb_result.as.ref != NULL) {
        cp = (pico8_instance_t *)eb_result.as.ref;
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

                if (self->ht.kind == PICO8_VALUE_REF && self->ht.as.ref != NULL) {
                    ht_table = (const pico8_ob_table_t *)self->ht.as.ref;
                }
                if (ht_table != NULL) {
                    pico8_list_t ht_list = positional_values(ht_table);
                    int accepted = pico8_gu(&ht_list, qe_val);
                    pico8_list_free(&ht_list);
                    if (accepted) {
                        pico8_value_t kw_val = self->hj.qh.jo.kw;
                        if (kw_val.kind == PICO8_VALUE_REF && kw_val.as.ref != NULL) {
                            pico8_ob_table_set_name((pico8_ob_table_t *)kw_val.as.ref, "qe", qe_val);
                        }
                        /* The previous `ht` is mx's class default, shared
                         * by every other synthesizer: never freed. */
                        self->ht = pico8_value_ref(pico8_ob_table_new());
                    }
                }
            }
        }
    }

    pico8_jo_kf((pico8_instance_t *)self, sw);
}

static pico8_value_t pico8_mx_kf_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_instance_t *sw = NULL;
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        sw = (pico8_instance_t *)arg0.as.ref;
    }
    pico8_mx_kf((pico8_mx_t *)self, sw);
    return pico8_value_ref(NULL);
}

/* mx's own defaults, read once from its class at init: the very values its
 * `defaults` holds, tables included. Each lands in the member of that name,
 * hj's, qh's or jo's when it has one. */
static struct {
    const char *name;
    size_t offset;
    pico8_value_t value;
} g_mx_defaults[] = {
    {"nu", offsetof(pico8_mx_t, hj.qh.jo.nu), {0}}, {"rr", offsetof(pico8_mx_t, hj.rr), {0}},
    {"hp", offsetof(pico8_mx_t, hj.qh.hp), {0}},    {"s", offsetof(pico8_mx_t, hj.qh.s), {0}},
    {"px", offsetof(pico8_mx_t, hj.qh.px), {0}},    {"od", offsetof(pico8_mx_t, hj.qh.od), {0}},
    {"ht", offsetof(pico8_mx_t, ht), {0}},          {"py", offsetof(pico8_mx_t, hj.py), {0}},
    {"ki", offsetof(pico8_mx_t, hj.ki), {0}},       {"pn", offsetof(pico8_mx_t, hj.pn), {0}},
    {"pk", offsetof(pico8_mx_t, hj.pk), {0}},       {"fc", offsetof(pico8_mx_t, hj.fc), {0}},
    {"k_", offsetof(pico8_mx_t, hj.k_), {0}},       {"du", offsetof(pico8_mx_t, hj.du), {0}},
    {"qo", offsetof(pico8_mx_t, hj.qo), {0}},       {"ro", offsetof(pico8_mx_t, hj.ro), {0}},
    {"qg", offsetof(pico8_mx_t, hj.qg), {0}},       {"q_", offsetof(pico8_mx_t, hj.q_), {0}},
};

static void pico8_mx_defaults_reset(void) {
    size_t i;
    for (i = 0; i < sizeof g_mx_defaults / sizeof g_mx_defaults[0]; i++) {
        g_mx_defaults[i].value = pico8_value_ref(NULL);
    }
}

static int pico8_mx_field(const void *self, const char *name, pico8_value_t *out_value) {
    if (strcmp(name, "ht") == 0) {
        const pico8_value_t *ht = &((const pico8_mx_t *)self)->ht;
        if (ht->kind == PICO8_VALUE_REF && ht->as.ref == NULL) {
            return 0;
        }
        *out_value = *ht;
        return 1;
    }
    return pico8_hj_struct_class()->field(self, name, out_value);
}

/* `ht`, which mx's own `kf` writes, then hj's 21 names; 0 for any other. */
static int pico8_mx_set_field(void *self, const char *name, pico8_value_t value) {
    if (strcmp(name, "ht") == 0) {
        ((pico8_mx_t *)self)->ht = value;
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
    size_t i;
    int k;

    assert(g_mx_class != NULL && "pico8_planet_family_trade_init() must run first");
    pico8_hj_struct_init(&self->hj, &g_mx_struct_class);
    self->ht = pico8_value_ref(NULL);
    for (i = 0; i < sizeof g_mx_defaults / sizeof g_mx_defaults[0]; i++) {
        *(pico8_value_t *)((char *)self + g_mx_defaults[i].offset) = g_mx_defaults[i].value;
    }
    for (k = 0; overrides != NULL && k < overrides->count; k++) {
        pico8_ob_key_t key = overrides->entries[k].key;
        int measured = key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "id") == 0;

        assert(measured && "mx: the only measured construction key is id (ie:ldown); another would be a field "
                           "under no rule of plan-3 section 5 -- stop, do not add a member");
        (void)measured;
        self->hj.qh.jo.id = overrides->entries[k].value;
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("mx");
#endif
    pico8_f_((pico8_instance_t *)self);
    pico8_jo_pq((pico8_instance_t *)self);
    pico8_ke_struct_pq(&self->hj.qh.jo.ke);
    return self;
}

void pico8_planet_family_trade_init(void) {
    size_t i;

    assert(pico8_qh_class() != NULL && pico8_hj_class() != NULL &&
           "pico8_planet_family_init() must run first");
    pico8_planet_family_trade_shutdown();
    pico8_pi_init();

    g_nr_class = pico8_class_declare(pico8_qh_class(),
                                      "\n"
                                      "nu=o(\"nr\",\"np\",\"prober\"),\n"
                                      "od=10,fp=1,\n"
                                      "kq=b(-4,-4,5,5),\n"
                                      "s=170,\n"
                                      "hz=o(170,170,170,168),\n"
                                      "dw=0.01,\n"
                                      "rr=\"slipgates\",\n");
    PICO8_CLASS_COVERAGE_NAME(g_nr_class, "nr");
    pico8_ob_table_set_name(g_nr_class->defaults, "gv", pico8_value_function(pico8_nr_gv_dispatch));
    pico8_ob_table_set_name(g_nr_class->defaults, "ldown", pico8_value_function(pico8_nr_ldown_dispatch));
    pico8_ob_table_set_name(g_nr_class->defaults, "hb", pico8_value_function(pico8_nr_hb_dispatch));
    pico8_ob_table_set_name(g_nr_class->defaults, "cn", pico8_value_function(pico8_nr_cn_dispatch));

    g_fr_class = pico8_class_declare(g_nr_class,
                                      "\n"
                                      "mt=1,\n"
                                      "rr=\"infraspace\",\n"
                                      "od=25,\n"
                                      "s=78,hz=o(78,78,78,76),\n");
    PICO8_CLASS_COVERAGE_NAME(g_fr_class, "fr");

    g_mx_class = pico8_class_declare(pico8_hj_class(),
                                      "\n"
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
                                      "qg=0.07,q_=0.98,\n");
    PICO8_CLASS_COVERAGE_NAME(g_mx_class, "mx");
    pico8_class_set_struct_new(g_mx_class, pico8_mx_struct_new);
    for (i = 0; i < sizeof g_mx_defaults / sizeof g_mx_defaults[0]; i++) {
        int found = pico8_class_get_field(g_mx_class, g_mx_defaults[i].name, &g_mx_defaults[i].value);
        assert(found && "mx: every default of the struct is a key of mx's spec");
        (void)found;
    }
}

void pico8_planet_family_trade_shutdown(void) {
    pico8_mx_defaults_reset();
    if (g_mx_class != NULL) {
        pico8_class_free(g_mx_class);
        g_mx_class = NULL;
    }
    if (g_fr_class != NULL) {
        pico8_class_free(g_fr_class);
        g_fr_class = NULL;
    }
    if (g_nr_class != NULL) {
        pico8_class_free(g_nr_class);
        g_nr_class = NULL;
    }
}

pico8_class_t *pico8_nr_class(void) {
    return g_nr_class;
}

pico8_instance_t *pico8_nr_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_nr_class, overrides);
}

pico8_class_t *pico8_fr_class(void) {
    return g_fr_class;
}

pico8_instance_t *pico8_fr_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_fr_class, overrides);
}

pico8_class_t *pico8_mx_class(void) {
    return g_mx_class;
}

const pico8_struct_class_t *pico8_mx_struct_class(void) {
    return &g_mx_struct_class;
}

pico8_mx_t *pico8_mx_new(const pico8_ob_table_t *overrides) {
    return pico8_mx_struct_new(overrides);
}
