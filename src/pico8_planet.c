#include "pico8_planet.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_audio.h"
#include "pico8_collect.h"
#include "pico8_data_init.h"
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

static pico8_ob_table_t *g_jo_spec = NULL;

static pico8_gn_state_t *g_jo_oc = NULL;
static pico8_jf_state_t *g_jo_ir = NULL;

/* jo's own defaults, read once through pico8_jo_default() at init: the very
 * values its spec holds, tables included (the same REF: `kw` is written in place
 * by `bb` and `mx:kf`). Never a table outside init..shutdown. */
static struct {
    const char *name;
    pico8_value_t value;
} g_jo_defaults[] = {
    {"nu", {0}}, {"me", {0}}, {"hf", {0}}, {"hs", {0}}, {"fa", {0}}, {"kq", {0}}, {"ma", {0}},
    {"ka", {0}}, {"dz", {0}}, {"cq", {0}}, {"kw", {0}}, {"gq", {0}}, {"ca", {0}}, {"em", {0}},
};

static void pico8_jo_defaults_reset(void) {
    size_t i;
    for (i = 0; i < sizeof g_jo_defaults / sizeof g_jo_defaults[0]; i++) {
        g_jo_defaults[i].value = pico8_value_ref(NULL);
    }
}

static const pico8_ob_table_t *pico8_jo_default_table(const char *name) {
    size_t i;
    for (i = 0; i < sizeof g_jo_defaults / sizeof g_jo_defaults[0]; i++) {
        if (strcmp(g_jo_defaults[i].name, name) == 0) {
            return g_jo_defaults[i].value.kind == PICO8_VALUE_REF ? g_jo_defaults[i].value.as.ref : NULL;
        }
    }
    return NULL;
}

static int pico8_jo_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

/* A nil member reads as absent. */
static int pico8_jo_present(pico8_value_t v) {
    return !(v.kind == PICO8_VALUE_REF && v.as.ref == NULL);
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

static const struct {
    const char *name;
    size_t offset;
} g_jo_members[] = {
    {"nu", offsetof(pico8_jo_t, nu)}, {"hf", offsetof(pico8_jo_t, hf)}, {"hs", offsetof(pico8_jo_t, hs)},
    {"fa", offsetof(pico8_jo_t, fa)}, {"kq", offsetof(pico8_jo_t, kq)}, {"ma", offsetof(pico8_jo_t, ma)},
    {"ka", offsetof(pico8_jo_t, ka)}, {"dz", offsetof(pico8_jo_t, dz)}, {"cq", offsetof(pico8_jo_t, cq)},
    {"kw", offsetof(pico8_jo_t, kw)}, {"gq", offsetof(pico8_jo_t, gq)}, {"ca", offsetof(pico8_jo_t, ca)},
    {"em", offsetof(pico8_jo_t, em)}, {"id", offsetof(pico8_jo_t, id)}, {"p", offsetof(pico8_jo_t, p)},
    {"ra", offsetof(pico8_jo_t, ra)}, {"oe", offsetof(pico8_jo_t, oe)}, {"hn", offsetof(pico8_jo_t, hn)},
    {"ep", offsetof(pico8_jo_t, ep)}, {"jy", offsetof(pico8_jo_t, jy)}, {"nk", offsetof(pico8_jo_t, nk)},
    {"fx", offsetof(pico8_jo_t, fx)}, {"lq", offsetof(pico8_jo_t, lq)}, {"fo", offsetof(pico8_jo_t, fo)},
    {"jh", offsetof(pico8_jo_t, jh)}, {"p_", offsetof(pico8_jo_t, p_)}, {"kr", offsetof(pico8_jo_t, kr)},
    {"cc", offsetof(pico8_jo_t, cc)}, {"by", offsetof(pico8_jo_t, by)}, {"mi", offsetof(pico8_jo_t, mi)},
    {"m_", offsetof(pico8_jo_t, m_)},
};

/* The member named `name`, or NULL when a `jo` has none. */
static pico8_value_t *pico8_jo_member(const pico8_jo_t *jo, const char *name) {
    size_t i;
    for (i = 0; i < sizeof g_jo_members / sizeof g_jo_members[0]; i++) {
        if (strcmp(name, g_jo_members[i].name) == 0) {
            return (pico8_value_t *)((const char *)jo + g_jo_members[i].offset);
        }
    }
    return NULL;
}

static int pico8_jo_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_value_t *member = pico8_jo_member(self, name);

    if (member == NULL) {
        return pico8_ke_field(self, name, out_value);
    }
    if (member->kind == PICO8_VALUE_REF && member->as.ref == NULL) {
        return 0;
    }
    *out_value = *member;
    return 1;
}

/* The 17 names jo's own functions and the generic writers of `cg.jo`
 * (`hy:lj`, `bd`, `bb`, the hover loop, `ma()`, `bv`, `jc:hq`) write on a
 * bare planet; 0 for any other. */
static int pico8_jo_set_field(void *self, const char *name, pico8_value_t value) {
    static const char *const written[] = {"by", "cc", "ep", "fo", "fx", "hn", "jh", "jy", "kr",
                                          "kw", "lq", "m_", "ma", "mi", "nk", "oe", "p_"};
    size_t i;

    for (i = 0; i < sizeof written / sizeof written[0]; i++) {
        if (strcmp(name, written[i]) == 0) {
            *pico8_jo_member(self, name) = value;
            return 1;
        }
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
    size_t i;

    assert(g_jo_spec != NULL && "pico8_jo_init() must run first");
    pico8_ke_struct_init(&self->ke, cls);
    for (i = 0; i < sizeof g_jo_members / sizeof g_jo_members[0]; i++) {
        *(pico8_value_t *)((char *)self + g_jo_members[i].offset) = pico8_value_ref(NULL);
    }
    for (i = 0; i < sizeof g_jo_defaults / sizeof g_jo_defaults[0]; i++) {
        if (strcmp(g_jo_defaults[i].name, "me") == 0) {
            self->ke.me = g_jo_defaults[i].value;
        } else {
            *pico8_jo_member(self, g_jo_defaults[i].name) = g_jo_defaults[i].value;
        }
    }
}

static void *pico8_jo_struct_new(const pico8_ob_table_t *overrides) {
    pico8_jo_t *self = malloc(sizeof *self);
    int k;

    pico8_jo_struct_init(self, &g_jo_struct_class);
    for (k = 0; overrides != NULL && k < overrides->count; k++) {
        pico8_ob_key_t key = overrides->entries[k].key;
        int measured = key.kind == PICO8_OB_KEY_NAME &&
                       (strcmp(key.name, "id") == 0 || strcmp(key.name, "p") == 0 || strcmp(key.name, "ra") == 0);

        assert(measured && "jo: the only measured construction keys are id, p and ra (bw); another would be a "
                           "field under no rule of plan-3 section 5 -- stop, do not add a member");
        (void)measured;
        *pico8_jo_member(self, key.name) = overrides->entries[k].value;
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("jo");
#endif
    pico8_jo_pq(self);
    pico8_ke_struct_pq(&self->ke);
    PICO8_MEMBER_CENSUS_SEE(self, pico8_jo_t);
    return self;
}

void pico8_jo_init(void) {
    size_t i;

    pico8_jo_shutdown();
    pico8_pi_init();
    g_jo_spec = pico8_ob("\n"
                         "nu=o(\"jo\",\"np\",\"prober\"),\n"
                         "me=\"unknown\",\n"
                         "hf=1,hs=1,\n"
                         "fa=6,\n"
                         "kq=b(-7,-7,7,7),\n"
                         "ma=0,\n"
                         "ka=o(2,3,9,12),\n"
                         "dz=o(\n"
                         "o(\"unhappy\",c=8),\n"
                         "o(\"content\",c=3),\n"
                         "o(\"prosperous\",c=9),\n"
                         "o(\"rich\",c=12),\n"
                         "),\n"
                         "cq=o(12,36,24,25),\n"
                         "kw=o(mu=0),\n"
                         "gq=v(0,11),ca=9,\n"
                         "em=o(f,3,0,-1),\n",
                         NULL);
    for (i = 0; i < sizeof g_jo_defaults / sizeof g_jo_defaults[0]; i++) {
        int found = pico8_jo_default(g_jo_defaults[i].name, &g_jo_defaults[i].value);
        assert(found && "jo: every default of the struct is a key of jo's spec");
        (void)found;
    }
}

void pico8_jo_shutdown(void) {
    pico8_jo_defaults_reset();
    pico8_ob_table_free(g_jo_spec);
    g_jo_spec = NULL;
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
    return g_jo_spec != NULL;
}

int pico8_jo_default(const char *name, pico8_value_t *out_value) {
    return g_jo_spec != NULL &&
           (pico8_ob_table_get_name(g_jo_spec, name, out_value) || pico8_ke_default(name, out_value));
}

const pico8_ob_table_t *pico8_jo_dz(void) {
    return pico8_jo_default_table("dz");
}

const pico8_ob_table_t *pico8_jo_cq(void) {
    return pico8_jo_default_table("cq");
}

const pico8_ob_table_t *pico8_jo_ka(void) {
    return pico8_jo_default_table("ka");
}

const pico8_ob_table_t *pico8_jo_em(void) {
    return pico8_jo_default_table("em");
}

const pico8_struct_class_t *pico8_jo_struct_class(void) {
    return &g_jo_struct_class;
}

pico8_jo_t *pico8_jo_new(const pico8_ob_table_t *overrides) {
    return pico8_jo_struct_new(overrides);
}

/* `pico8_gb()` is NULL until `pico8_data_init()` has run, and the
 * `pico8_ob_table_get_*` primitives do not guard their table argument. */
void pico8_jo_pq(pico8_jo_t *self) {
    pico8_value_t hp_val;
    pico8_list_t *oe;

    if (self == NULL) {
        return;
    }

    oe = malloc(sizeof *oe);
    pico8_list_init(oe);
    self->oe = pico8_value_ref(oe);

    if (pico8_entity_get_field(self, "hp", &hp_val) && pico8_jo_truthy(hp_val)) {
        const pico8_ob_table_t *gb = pico8_gb();
        pico8_value_t target;
        int found = 0;

        if (gb != NULL) {
            if (hp_val.kind == PICO8_VALUE_FIXED) {
                found = pico8_ob_table_get_index(gb, pico8_fixed_flr_int(hp_val.as.fixed), &target);
            } else if (hp_val.kind == PICO8_VALUE_STRING) {
                found = pico8_ob_table_get_name(gb, hp_val.as.string, &target);
            }
        }
        if (found && target.kind == PICO8_VALUE_REF && target.as.ref != NULL) {
            pico8_jo_lz(self, target.as.ref);
        }
    }
}

static int pico8_jo_lz_mw_step(pico8_value_t e, void *ctx, pico8_value_t *out_value) {
    (void)ctx;
    if (e.kind == PICO8_VALUE_REF && e.as.ref != NULL) {
        *out_value = pico8_value_ref(pico8_mw((const pico8_ob_table_t *)e.as.ref));
        return 1;
    }
    return 0;
}

void pico8_jo_lz(pico8_jo_t *self, pico8_hp_t *hp_instance) {
    pico8_list_t jy;
    pico8_list_t *jy_heap;
    pico8_ob_table_t *kw;

    pico8_hp_assert(hp_instance);
    if (self == NULL) {
        return;
    }

    self->hn = pico8_value_ref(hp_instance);
    self->ep = pico8_value_ref(NULL);

    pico8_list_init(&jy);
    if (hp_instance != NULL && hp_instance->i != NULL) {
        jy = pico8_nv(hp_instance->i, pico8_jo_lz_mw_step, NULL);
    }
    jy_heap = malloc(sizeof *jy_heap);
    *jy_heap = jy;
    self->jy = pico8_value_ref(jy_heap);

    kw = pico8_ob_table_new();
    if (hp_instance != NULL && hp_instance->has_o) {
        pico8_ob_table_set_name(kw, "qe", pico8_value_fixed(hp_instance->o));
    }
    pico8_ob_table_set_name(kw, "mu", pico8_value_fixed(pico8_fixed_from_int(0)));
    self->kw = pico8_value_ref(kw);
}

pico8_value_t pico8_jo_gg(const pico8_jo_t *self) {
    pico8_value_t hf_val;
    pico8_value_t me_val;

    if (self == NULL || !pico8_jo_present(self->hf)) {
        return pico8_value_ref(NULL);
    }
    hf_val = self->hf;
    if (!pico8_jo_truthy(hf_val)) {
        return hf_val;
    }
    me_val = self->ke.me;
    if (me_val.kind == PICO8_VALUE_STRING) {
        return pico8_value_bool(strcmp(me_val.as.string, "ly") == 0);
    }
    return pico8_value_bool(0);
}

void pico8_jo_kd(pico8_jo_t *self) {
    if (self == NULL) {
        return;
    }
    self->ma = pico8_value_fixed(PICO8_FIXED_LIT(0x00014000, 1.25));
}

void pico8_jo_ll(pico8_jo_t *self, pico8_fixed_t t) {
    if (self == NULL) {
        return;
    }
    self->p_ = pico8_value_bool(t > pico8_fixed_from_int(60));
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
        pico8_ob_table_t *overrides;

        self->p_ = pico8_value_bool(1);

        target_kind = jo_kv(hp_instance->cw);

        overrides = pico8_ob_table_new();
        if (pico8_jo_present(self->p)) {
            pico8_ob_table_set_name(overrides, "p", self->p);
        }
        if (pico8_jo_present(self->ra)) {
            pico8_ob_table_set_name(overrides, "ra", self->ra);
        }
        if (pico8_jo_present(self->id)) {
            pico8_ob_table_set_name(overrides, "id", self->id);
        }

        working_self = (target_kind != NULL) ? (pico8_jo_t *)target_kind->new_entity(overrides) : NULL;
        pico8_ob_table_free(overrides);
    } else if (hp_instance != NULL && hp_instance->go != NULL) {
        pico8_value_t nk_val = pico8_value_ref(NULL);

        if (!pico8_ob_table_get_name(pico8_df(), hp_instance->go, &nk_val)) {
            nk_val = pico8_value_ref(NULL);
        }
        self->nk = nk_val;
    } else {
        pico8_value_t fp_val;
        pico8_ke_ks((pico8_struct_header_t *)self, pico8_value_string("ly"));
        pico8_jo_lz(self, hp_instance);
        self->fx = pico8_value_bool(1);
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
    pico8_value_t nk_val = pico8_value_ref(NULL);
    pico8_value_t d_val = pico8_value_ref(NULL);
    pico8_value_t me_val;
    pico8_value_t ks_arg;

    if (self == NULL) {
        return;
    }

    if (g_jo_ir != NULL) {
        pico8_fixed_t roll = pico8_jf_next(g_jo_ir);
        int use_ll = 0;
        pico8_value_t ll_val = pico8_value_ref(NULL);

        /* `jo.ir()<0.45 and df.ll or jo.oc()`: a falsy `df.ll` falls through
         * to `jo.oc()` even on a winning roll -- both conditions, never the
         * roll alone. */
        if (roll < PICO8_FIXED_LIT(0x00007333, 0.45)) {
            use_ll = pico8_ob_table_get_name(pico8_df(), "ll", &ll_val) && pico8_jo_truthy(ll_val);
        }
        if (use_ll) {
            nk_val = ll_val;
        } else if (g_jo_oc != NULL) {
            nk_val = pico8_gn_next(g_jo_oc);
        }
    }
    self->nk = nk_val;

    if (nk_val.kind == PICO8_VALUE_REF && nk_val.as.ref != NULL) {
        pico8_ob_table_get_name((const pico8_ob_table_t *)nk_val.as.ref, "d", &d_val);
    }
    self->ep = d_val;

    if (nk_val.kind == PICO8_VALUE_REF && nk_val.as.ref != NULL &&
        pico8_ob_table_get_name((const pico8_ob_table_t *)nk_val.as.ref, "me", &me_val) && pico8_jo_truthy(me_val)) {
        ks_arg = me_val;
    } else {
        ks_arg = pico8_value_string("known");
    }
    pico8_ke_ks((pico8_struct_header_t *)self, ks_arg);
    pico8_jo_kd(self);
}

void pico8_jo_hk(const pico8_jo_t *self) {
    pico8_value_t kr_val;

    if (self == NULL) {
        return;
    }
    kr_val = self->kr;
    if (!pico8_jo_present(kr_val) || !pico8_jo_truthy(kr_val)) {
        return;
    }
    if (kr_val.kind == PICO8_VALUE_FIXED && kr_val.as.fixed == pico8_fixed_from_int(0)) {
        return;
    }
    if (!pico8_gd((const pico8_struct_header_t *)self)) {
        return;
    }
    {
        pico8_ob_table_t *overrides = pico8_ob_table_new();
        int color = 0;
        char *text;

        if (pico8_jo_present(self->ra)) {
            pico8_ob_table_set_name(overrides, "ra", self->ra);
        }
        text = pico8_ne(kr_val.kind == PICO8_VALUE_FIXED ? kr_val.as.fixed : pico8_fixed_from_int(0), 10, "$",
                        &color);
        /* `ne()` is the last expression of the Lua constructor, so both of its
         * return values land as positional entries 1 and 2, not named fields. */
        pico8_ob_table_set_index(overrides, 1, pico8_value_string(text));
        pico8_ob_table_set_index(overrides, 2, pico8_value_fixed(pico8_fixed_from_int(color)));
        pico8_iv_new(overrides);
        pico8_ob_table_free(overrides);
        /* `text` is now owned by the new `iv` instance, which reads it every
         * frame it stays on screen: not freed here. */
    }
}

pico8_value_t pico8_jo_ldown(pico8_jo_t *self) {
    pico8_value_t me_val;

    if (self == NULL || self->ke.me.kind != PICO8_VALUE_STRING) {
        return pico8_value_ref(NULL);
    }
    me_val = self->ke.me;
    if (strcmp(me_val.as.string, "known") == 0) {
        pico8_jo_fe(self);
        return pico8_value_bool(1);
    }
    if (strcmp(me_val.as.string, "ly") == 0) {
        pico8_jc_t *made;
        pico8_ob_table_t *overrides;

        assert(pico8_jc_ready() && "pico8_jc_init() must run before jo:ldown builds a jc");
        overrides = pico8_ob_table_new();
        pico8_ob_table_set_name(overrides, "qu", pico8_value_ref(self));
        made = pico8_jc_new(overrides);
        pico8_ob_table_free(overrides);
        return pico8_value_ref(made);
    }
    return pico8_value_ref(NULL);
}
