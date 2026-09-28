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

/* qh's own defaults, read once through pico8_qh_default() at init: the very
 * values its spec holds, tables included. Each lands in the member of that
 * name, jo's when it has one. */
static struct {
    const char *name;
    size_t offset;
    pico8_value_t value;
} g_qh_defaults[] = {
    {"hp", offsetof(pico8_qh_t, hp), {0}},
    {"px", offsetof(pico8_qh_t, px), {0}},
    {"dq", offsetof(pico8_qh_t, dq), {0}},
    {"qh", offsetof(pico8_qh_t, qh), {0}},    {"od", offsetof(pico8_qh_t, od), {0}},
    {"fp", offsetof(pico8_qh_t, fp), {0}},    {"dw", offsetof(pico8_qh_t, dw), {0}},
    {"s", offsetof(pico8_qh_t, s), {0}},
    {"ba", offsetof(pico8_qh_t, ba), {0}},
};

static const char *g_qh_me = NULL;
static pico8_ob_table_t *g_qh_nu = NULL;
static pico8_fixed_t g_qh_hf = 0;
static int g_qh_hf_is_bool = 0;
static pico8_fixed_t g_qh_mi = 0;

static void pico8_qh_defaults_reset(void) {
    size_t i;
    g_qh_me = NULL;
    g_qh_nu = NULL;
    g_qh_hf = 0;
    g_qh_hf_is_bool = 0;
    g_qh_mi = 0;
    for (i = 0; i < sizeof g_qh_defaults / sizeof g_qh_defaults[0]; i++) {
        g_qh_defaults[i].value = pico8_value_ref(NULL);
    }
}

static const struct {
    const char *name;
    size_t offset;
} g_qh_members[] = {
    {"hp", offsetof(pico8_qh_t, hp)}, {"px", offsetof(pico8_qh_t, px)}, {"dq", offsetof(pico8_qh_t, dq)},
    {"qh", offsetof(pico8_qh_t, qh)}, {"od", offsetof(pico8_qh_t, od)}, {"fp", offsetof(pico8_qh_t, fp)},
    {"dw", offsetof(pico8_qh_t, dw)}, {"s", offsetof(pico8_qh_t, s)},   {"ba", offsetof(pico8_qh_t, ba)},
    {"et", offsetof(pico8_qh_t, et)},
};

/* qh's own member named `name`, or NULL: jo's members are jo's to find. */
static pico8_value_t *pico8_qh_member(const pico8_qh_t *qh, const char *name) {
    size_t i;
    for (i = 0; i < sizeof g_qh_members / sizeof g_qh_members[0]; i++) {
        if (strcmp(name, g_qh_members[i].name) == 0) {
            return (pico8_value_t *)((const char *)qh + g_qh_members[i].offset);
        }
    }
    return NULL;
}

static int pico8_qh_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_value_t *member = pico8_qh_member(self, name);

    if (member == NULL) {
        return pico8_jo_struct_class()->field(self, name, out_value);
    }
    if (member->kind == PICO8_VALUE_REF && member->as.ref == NULL) {
        return 0;
    }
    *out_value = *member;
    return 1;
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

/* `et`, `ra`, `p`, which qh's own `hq` (`bv`) writes, then jo's 17 names;
 * 0 for any other. */
static int pico8_qh_set_field(void *self, const char *name, pico8_value_t value) {
    pico8_qh_t *qh = self;

    if (strcmp(name, "et") == 0) {
        qh->et = value;
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
    size_t i;

    pico8_jo_struct_init(&self->jo, cls);
    for (i = 0; i < sizeof g_qh_members / sizeof g_qh_members[0]; i++) {
        *(pico8_value_t *)((char *)self + g_qh_members[i].offset) = pico8_value_ref(NULL);
    }
    for (i = 0; i < sizeof g_qh_defaults / sizeof g_qh_defaults[0]; i++) {
        *(pico8_value_t *)((char *)self + g_qh_defaults[i].offset) = g_qh_defaults[i].value;
    }
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

/* nl's own defaults, read once through pico8_nl_default() at init: the very
 * values its spec holds, tables included. Each lands in the member of that
 * name, qh's or jo's when it has one. */
static struct {
    const char *name;
    size_t offset;
    pico8_value_t value;
} g_nl_defaults[] = {
    {"rr", offsetof(pico8_nl_t, rr), {0}},
    {"qh", offsetof(pico8_nl_t, qh.qh), {0}},    {"nl", offsetof(pico8_nl_t, nl), {0}},
    {"hp", offsetof(pico8_nl_t, qh.hp), {0}},    {"s", offsetof(pico8_nl_t, qh.s), {0}},
    {"px", offsetof(pico8_nl_t, qh.px), {0}},    {"od", offsetof(pico8_nl_t, qh.od), {0}},
    {"qw", offsetof(pico8_nl_t, qw), {0}},       {"ba", offsetof(pico8_nl_t, qh.ba), {0}},
};

static pico8_ob_table_t *g_nl_nu = NULL;

static void pico8_nl_defaults_reset(void) {
    size_t i;
    g_nl_nu = NULL;
    for (i = 0; i < sizeof g_nl_defaults / sizeof g_nl_defaults[0]; i++) {
        g_nl_defaults[i].value = pico8_value_ref(NULL);
    }
}

static const struct {
    const char *name;
    size_t offset;
} g_nl_members[] = {
    {"rr", offsetof(pico8_nl_t, rr)},
    {"nl", offsetof(pico8_nl_t, nl)},
    {"qw", offsetof(pico8_nl_t, qw)},
};

static int pico8_nl_field(const void *self, const char *name, pico8_value_t *out_value) {
    size_t i;

    for (i = 0; i < sizeof g_nl_members / sizeof g_nl_members[0]; i++) {
        if (strcmp(name, g_nl_members[i].name) == 0) {
            const pico8_value_t *member = (const pico8_value_t *)((const char *)self + g_nl_members[i].offset);
            if (member->kind == PICO8_VALUE_REF && member->as.ref == NULL) {
                return 0;
            }
            *out_value = *member;
            return 1;
        }
    }
    return pico8_qh_struct_class()->field(self, name, out_value);
}

/* `qw`, which nl's own `ly` writes, then qh's 20 names; 0 for any other. */
static int pico8_nl_set_field(void *self, const char *name, pico8_value_t value) {
    if (strcmp(name, "qw") == 0) {
        ((pico8_nl_t *)self)->qw = value;
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
        self->qw = pico8_value_fixed(pico8_fixed_add(
            pico8_fixed_from_int(8),
            pico8_fixed_div(pico8_fixed_mod(t, pico8_fixed_from_int(42)), pico8_fixed_from_int(6))));
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
    size_t i;
    int k;

    assert(g_nl_spec != NULL && "pico8_planet_family_init() must run first");
    pico8_qh_struct_init(&self->qh, &g_nl_struct_class);
    for (i = 0; i < sizeof g_nl_members / sizeof g_nl_members[0]; i++) {
        *(pico8_value_t *)((char *)self + g_nl_members[i].offset) = pico8_value_ref(NULL);
    }
    for (i = 0; i < sizeof g_nl_defaults / sizeof g_nl_defaults[0]; i++) {
        *(pico8_value_t *)((char *)self + g_nl_defaults[i].offset) = g_nl_defaults[i].value;
    }
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

/* os's own defaults, read once through pico8_os_default() at init: the very
 * values its spec holds, tables included. Each lands in the member of that
 * name, qh's or jo's: os has none of its own. */
static struct {
    const char *name;
    size_t offset;
    pico8_value_t value;
} g_os_defaults[] = {
    {"hp", offsetof(pico8_os_t, qh.hp), {0}},
    {"px", offsetof(pico8_os_t, qh.px), {0}},    {"qh", offsetof(pico8_os_t, qh.qh), {0}},
    {"s", offsetof(pico8_os_t, qh.s), {0}},      {"od", offsetof(pico8_os_t, qh.od), {0}},
};

static pico8_ob_table_t *g_os_nu = NULL;
static pico8_fixed_t g_os_hs = 0;

static void pico8_os_defaults_reset(void) {
    size_t i;
    g_os_nu = NULL;
    g_os_hs = 0;
    for (i = 0; i < sizeof g_os_defaults / sizeof g_os_defaults[0]; i++) {
        g_os_defaults[i].value = pico8_value_ref(NULL);
    }
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
    size_t i;
    int k;

    assert(g_os_spec != NULL && "pico8_planet_family_init() must run first");
    pico8_qh_struct_init(&self->qh, &g_os_struct_class);
    for (i = 0; i < sizeof g_os_defaults / sizeof g_os_defaults[0]; i++) {
        *(pico8_value_t *)((char *)self + g_os_defaults[i].offset) = g_os_defaults[i].value;
    }
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

/* hj's own defaults, read once through pico8_hj_default() at init: the very
 * values its spec holds, tables included. Each lands in the member of that
 * name, qh's or jo's when it has one. */
static struct {
    const char *name;
    size_t offset;
    pico8_value_t value;
} g_hj_defaults[] = {
    {"rr", offsetof(pico8_hj_t, rr), {0}},
    {"hp", offsetof(pico8_hj_t, qh.hp), {0}},    {"qh", offsetof(pico8_hj_t, qh.qh), {0}},
    {"px", offsetof(pico8_hj_t, qh.px), {0}},    {"od", offsetof(pico8_hj_t, qh.od), {0}},
    {"s", offsetof(pico8_hj_t, qh.s), {0}},      {"py", offsetof(pico8_hj_t, py), {0}},
    {"ki", offsetof(pico8_hj_t, ki), {0}},       {"pn", offsetof(pico8_hj_t, pn), {0}},
    {"pk", offsetof(pico8_hj_t, pk), {0}},       {"fc", offsetof(pico8_hj_t, fc), {0}},
    {"k_", offsetof(pico8_hj_t, k_), {0}},       {"du", offsetof(pico8_hj_t, du), {0}},
    {"qo", offsetof(pico8_hj_t, qo), {0}},       {"ro", offsetof(pico8_hj_t, ro), {0}},
    {"qg", offsetof(pico8_hj_t, qg), {0}},       {"q_", offsetof(pico8_hj_t, q_), {0}},
};

static pico8_ob_table_t *g_hj_nu = NULL;

static void pico8_hj_defaults_reset(void) {
    size_t i;
    g_hj_nu = NULL;
    for (i = 0; i < sizeof g_hj_defaults / sizeof g_hj_defaults[0]; i++) {
        g_hj_defaults[i].value = pico8_value_ref(NULL);
    }
}

static const struct {
    const char *name;
    size_t offset;
} g_hj_members[] = {
    {"rr", offsetof(pico8_hj_t, rr)}, {"py", offsetof(pico8_hj_t, py)}, {"ki", offsetof(pico8_hj_t, ki)},
    {"pn", offsetof(pico8_hj_t, pn)}, {"pk", offsetof(pico8_hj_t, pk)}, {"fc", offsetof(pico8_hj_t, fc)},
    {"k_", offsetof(pico8_hj_t, k_)}, {"du", offsetof(pico8_hj_t, du)}, {"qo", offsetof(pico8_hj_t, qo)},
    {"ro", offsetof(pico8_hj_t, ro)}, {"qg", offsetof(pico8_hj_t, qg)}, {"q_", offsetof(pico8_hj_t, q_)},
    {"qy", offsetof(pico8_hj_t, qy)},
};

static int pico8_hj_field(const void *self, const char *name, pico8_value_t *out_value) {
    size_t i;

    for (i = 0; i < sizeof g_hj_members / sizeof g_hj_members[0]; i++) {
        if (strcmp(name, g_hj_members[i].name) == 0) {
            const pico8_value_t *member = (const pico8_value_t *)((const char *)self + g_hj_members[i].offset);
            if (member->kind == PICO8_VALUE_REF && member->as.ref == NULL) {
                return 0;
            }
            *out_value = *member;
            return 1;
        }
    }
    return pico8_qh_struct_class()->field(self, name, out_value);
}

/* `qy`, which hj's own `pq` (`f_`) writes, then qh's 20 names; 0 for any
 * other. */
static int pico8_hj_set_field(void *self, const char *name, pico8_value_t value) {
    if (strcmp(name, "qy") == 0) {
        ((pico8_hj_t *)self)->qy = value;
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
    size_t i;

    pico8_qh_struct_init(&self->qh, cls);
    for (i = 0; i < sizeof g_hj_members / sizeof g_hj_members[0]; i++) {
        *(pico8_value_t *)((char *)self + g_hj_members[i].offset) = pico8_value_ref(NULL);
    }
    for (i = 0; i < sizeof g_hj_defaults / sizeof g_hj_defaults[0]; i++) {
        *(pico8_value_t *)((char *)self + g_hj_defaults[i].offset) = g_hj_defaults[i].value;
    }
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

    spawn_ra = pico8_vec_add(pico8_pf_value_vec(pico8_value_ref(self->hj.qh.jo.ra)), pico8_pf_value_vec(self->ng));
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

/* pm's own defaults, read once through pico8_pm_default() at init: the very
 * values its spec holds, tables included. Each lands in the member of that
 * name, hj's, qh's or jo's when it has one. */
static struct {
    const char *name;
    size_t offset;
    pico8_value_t value;
} g_pm_defaults[] = {
    {"fz", offsetof(pico8_pm_t, fz), {0}},
    {"hp", offsetof(pico8_pm_t, hj.qh.hp), {0}}, {"px", offsetof(pico8_pm_t, hj.qh.px), {0}},
    {"s", offsetof(pico8_pm_t, hj.qh.s), {0}},
    {"py", offsetof(pico8_pm_t, hj.py), {0}}, {"ki", offsetof(pico8_pm_t, hj.ki), {0}},
    {"pn", offsetof(pico8_pm_t, hj.pn), {0}}, {"pk", offsetof(pico8_pm_t, hj.pk), {0}},
    {"ng", offsetof(pico8_pm_t, ng), {0}},    {"du", offsetof(pico8_pm_t, hj.du), {0}},
    {"fc", offsetof(pico8_pm_t, hj.fc), {0}}, {"k_", offsetof(pico8_pm_t, hj.k_), {0}},
    {"qo", offsetof(pico8_pm_t, hj.qo), {0}}, {"ro", offsetof(pico8_pm_t, hj.ro), {0}},
    {"qg", offsetof(pico8_pm_t, hj.qg), {0}}, {"q_", offsetof(pico8_pm_t, hj.q_), {0}},
};

static const char *g_pm_me = NULL;
static pico8_ob_table_t *g_pm_nu = NULL;

static void pico8_pm_defaults_reset(void) {
    size_t i;
    g_pm_me = NULL;
    g_pm_nu = NULL;
    for (i = 0; i < sizeof g_pm_defaults / sizeof g_pm_defaults[0]; i++) {
        g_pm_defaults[i].value = pico8_value_ref(NULL);
    }
}

static const struct {
    const char *name;
    size_t offset;
} g_pm_members[] = {
    {"ng", offsetof(pico8_pm_t, ng)},
    {"fz", offsetof(pico8_pm_t, fz)},
};

static int pico8_pm_field(const void *self, const char *name, pico8_value_t *out_value) {
    size_t i;

    for (i = 0; i < sizeof g_pm_members / sizeof g_pm_members[0]; i++) {
        if (strcmp(name, g_pm_members[i].name) == 0) {
            const pico8_value_t *member = (const pico8_value_t *)((const char *)self + g_pm_members[i].offset);
            if (member->kind == PICO8_VALUE_REF && member->as.ref == NULL) {
                return 0;
            }
            *out_value = *member;
            return 1;
        }
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
    size_t i;
    int k;

    assert(g_pm_spec != NULL && "pico8_planet_family_init() must run first");
    pico8_hj_struct_init(&self->hj, &g_pm_struct_class);
    for (i = 0; i < sizeof g_pm_members / sizeof g_pm_members[0]; i++) {
        *(pico8_value_t *)((char *)self + g_pm_members[i].offset) = pico8_value_ref(NULL);
    }
    for (i = 0; i < sizeof g_pm_defaults / sizeof g_pm_defaults[0]; i++) {
        *(pico8_value_t *)((char *)self + g_pm_defaults[i].offset) = g_pm_defaults[i].value;
    }
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

/* ga's own defaults, read once through pico8_ga_default() at init, after the
 * swap of `by`: the very values its spec holds, tables included. Each lands
 * in the member of that name, jo's when it has one. */
static struct {
    const char *name;
    size_t offset;
    pico8_value_t value;
} g_ga_defaults[] = {
    {"fz", offsetof(pico8_ga_t, fz), {0}},
    {"fc", offsetof(pico8_ga_t, fc), {0}},    {"qo", offsetof(pico8_ga_t, qo), {0}},
    {"ro", offsetof(pico8_ga_t, ro), {0}},    {"qg", offsetof(pico8_ga_t, qg), {0}},
    {"q_", offsetof(pico8_ga_t, q_), {0}},    {"k_", offsetof(pico8_ga_t, k_), {0}},
    {"iu", offsetof(pico8_ga_t, iu), {0}},    {"py", offsetof(pico8_ga_t, py), {0}},
    {"ms", offsetof(pico8_ga_t, ms), {0}},
};

static pico8_ob_table_t *g_ga_nu = NULL;
static pico8_fixed_t g_ga_hf = 0;
static int g_ga_hf_is_bool = 0;
static pico8_qz_box_t *g_ga_kq = NULL;
static pico8_fixed_t g_ga_fa = 0;
static pico8_list_t *g_ga_by = NULL;

static void pico8_ga_defaults_reset(void) {
    size_t i;
    g_ga_nu = NULL;
    g_ga_hf = 0;
    g_ga_hf_is_bool = 0;
    g_ga_kq = NULL;
    g_ga_fa = 0;
    g_ga_by = NULL;
    for (i = 0; i < sizeof g_ga_defaults / sizeof g_ga_defaults[0]; i++) {
        g_ga_defaults[i].value = pico8_value_ref(NULL);
    }
}

/* The default `name` of ga's spec as read at init, nil outside
 * init..shutdown. */
static pico8_value_t pico8_ga_struct_default(const char *name) {
    size_t i;
    for (i = 0; i < sizeof g_ga_defaults / sizeof g_ga_defaults[0]; i++) {
        if (strcmp(g_ga_defaults[i].name, name) == 0) {
            return g_ga_defaults[i].value;
        }
    }
    return pico8_value_ref(NULL);
}

static const struct {
    const char *name;
    size_t offset;
} g_ga_members[] = {
    {"fz", offsetof(pico8_ga_t, fz)}, {"fc", offsetof(pico8_ga_t, fc)}, {"qo", offsetof(pico8_ga_t, qo)},
    {"ro", offsetof(pico8_ga_t, ro)}, {"qg", offsetof(pico8_ga_t, qg)}, {"q_", offsetof(pico8_ga_t, q_)},
    {"k_", offsetof(pico8_ga_t, k_)}, {"iu", offsetof(pico8_ga_t, iu)}, {"py", offsetof(pico8_ga_t, py)},
    {"ms", offsetof(pico8_ga_t, ms)}, {"qy", offsetof(pico8_ga_t, qy)},
};

static int pico8_ga_field(const void *self, const char *name, pico8_value_t *out_value) {
    size_t i;

    for (i = 0; i < sizeof g_ga_members / sizeof g_ga_members[0]; i++) {
        if (strcmp(name, g_ga_members[i].name) == 0) {
            const pico8_value_t *member = (const pico8_value_t *)((const char *)self + g_ga_members[i].offset);
            if (member->kind == PICO8_VALUE_REF && member->as.ref == NULL) {
                return 0;
            }
            *out_value = *member;
            return 1;
        }
    }
    return pico8_jo_struct_class()->field(self, name, out_value);
}

/* `qy`, which ga's own `pq` (`f_`) writes, then jo's 17 names; 0 for any
 * other. */
static int pico8_ga_set_field(void *self, const char *name, pico8_value_t value) {
    if (strcmp(name, "qy") == 0) {
        ((pico8_ga_t *)self)->qy = value;
        return 1;
    }
    return pico8_jo_struct_class()->set_field(self, name, value);
}

static pico8_fixed_t pico8_ga_fixed(pico8_value_t v) {
    return v.kind == PICO8_VALUE_FIXED ? v.as.fixed : pico8_fixed_from_int(0);
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

    ms = pico8_ga_fixed(self->ms);
    iu = pico8_ga_fixed(self->iu);
    py = pico8_ga_fixed(self->py);

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

    if (self->qy.kind == PICO8_VALUE_REF && self->qy.as.ref != NULL) {
        qy = PICO8_REF_AS_LIST_CONST(self->qy.as.ref);
    }

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
            pico8_value_t k_val;
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
            k_val = pico8_ga_struct_default("k_");
            if (k_val.kind == PICO8_VALUE_REF && k_val.as.ref != NULL &&
                pico8_ob_table_get_index((const pico8_ob_table_t *)k_val.as.ref, idx, &k_entry) &&
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

static void *pico8_ga_struct_new(const pico8_ob_table_t *overrides) {
    static const struct {
        const char *name;
        size_t offset;
    } keys[] = {
        {"qo", offsetof(pico8_ga_t, qo)}, {"ro", offsetof(pico8_ga_t, ro)}, {"qg", offsetof(pico8_ga_t, qg)},
        {"q_", offsetof(pico8_ga_t, q_)}, {"k_", offsetof(pico8_ga_t, k_)}, {"iu", offsetof(pico8_ga_t, iu)},
        {"py", offsetof(pico8_ga_t, py)}, {"ms", offsetof(pico8_ga_t, ms)},
    };
    pico8_ga_t *self = malloc(sizeof *self);
    size_t i;
    int k;

    assert(g_ga_spec != NULL && "pico8_planet_family_init() must run first");
    pico8_jo_struct_init(&self->jo, &g_ga_struct_class);
    for (i = 0; i < sizeof g_ga_members / sizeof g_ga_members[0]; i++) {
        *(pico8_value_t *)((char *)self + g_ga_members[i].offset) = pico8_value_ref(NULL);
    }
    for (i = 0; i < sizeof g_ga_defaults / sizeof g_ga_defaults[0]; i++) {
        *(pico8_value_t *)((char *)self + g_ga_defaults[i].offset) = g_ga_defaults[i].value;
    }
    self->jo.by = g_ga_by;
    self->jo.nu = g_ga_nu;
    self->jo.hf = g_ga_hf;
    self->jo.hf_is_bool = g_ga_hf_is_bool;
    self->jo.kq = g_ga_kq;
    self->jo.fa = g_ga_fa;
    for (k = 0; overrides != NULL && k < overrides->count; k++) {
        pico8_ob_key_t key = overrides->entries[k].key;
        size_t found = sizeof keys / sizeof keys[0];

        if (key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "p") == 0) {
            family_take_vec(overrides->entries[k].value, &self->jo.p);
            continue;
        }
        if (key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "ra") == 0) {
            family_take_vec(overrides->entries[k].value, &self->jo.ra);
            continue;
        }
        if (key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "id") == 0) {
            family_take_string(overrides->entries[k].value, &self->jo.id);
            continue;
        }
        for (i = 0; key.kind == PICO8_OB_KEY_NAME && i < sizeof keys / sizeof keys[0]; i++) {
            if (strcmp(key.name, keys[i].name) == 0) {
                found = i;
            }
        }
        assert(found < sizeof keys / sizeof keys[0] &&
               "ga: the only measured construction keys are p, ra, id (jo:hu) and ra, qo, ro, qg, q_, k_, iu, "
               "py, ms (pm:pq); another would be a field under no rule of plan-3 section 5 -- stop, do not add a member");
        *(pico8_value_t *)((char *)self + keys[found].offset) = overrides->entries[k].value;
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
           "qh/nl/os/hj/pm/ga.nu: a default of the struct is a key of its spec and a table (ref:table)");
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

static pico8_fixed_t family_default_fixed(int found, pico8_value_t v) {
    assert(found && v.kind == PICO8_VALUE_FIXED &&
           "os.hs, ga.fa, qh.mi: a default of the struct is a key of its spec and a fixed (fixe)");
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
           "qh/ga.hf: a default of the struct is a key of its spec and a fixed or a boolean (bool,fixe)");
    (void)found;
    *is_bool = v.kind == PICO8_VALUE_BOOL;
    return v.kind == PICO8_VALUE_BOOL ? (pico8_fixed_t)(v.as.boolean != 0) : v.as.fixed;
}

void pico8_planet_family_init(void) {
    size_t i;
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
    for (i = 0; i < sizeof g_qh_defaults / sizeof g_qh_defaults[0]; i++) {
        int found = pico8_qh_default(g_qh_defaults[i].name, &g_qh_defaults[i].value);
        assert(found && "qh: every default of the struct is a key of qh's spec");
        (void)found;
    }
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
    for (i = 0; i < sizeof g_nl_defaults / sizeof g_nl_defaults[0]; i++) {
        int found = pico8_nl_default(g_nl_defaults[i].name, &g_nl_defaults[i].value);
        assert(found && "nl: every default of the struct is a key of nl's spec");
        (void)found;
    }
    found = pico8_nl_default("nu", &v);
    g_nl_nu = family_default_table(found, v);

    g_os_spec = pico8_ob("\n"
                         "nu=o(\"jo\",\"np\",\"prober\"),\n"
                         "hp=4,px=o(\"processor\",c=13),\n"
                         "qh=f,s=100,\n"
                         "od=15,\n"
                         "hs=0.5,\n",
                         NULL);
    for (i = 0; i < sizeof g_os_defaults / sizeof g_os_defaults[0]; i++) {
        int found = pico8_os_default(g_os_defaults[i].name, &g_os_defaults[i].value);
        assert(found && "os: every default of the struct is a key of os's spec");
        (void)found;
    }
    found = pico8_os_default("nu", &v);
    g_os_nu = family_default_table(found, v);
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
    for (i = 0; i < sizeof g_hj_defaults / sizeof g_hj_defaults[0]; i++) {
        int found = pico8_hj_default(g_hj_defaults[i].name, &g_hj_defaults[i].value);
        assert(found && "hj: every default of the struct is a key of hj's spec");
        (void)found;
    }
    found = pico8_hj_default("nu", &v);
    g_hj_nu = family_default_table(found, v);

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
    for (i = 0; i < sizeof g_pm_defaults / sizeof g_pm_defaults[0]; i++) {
        int found = pico8_pm_default(g_pm_defaults[i].name, &g_pm_defaults[i].value);
        assert(found && "pm: every default of the struct is a key of pm's spec");
        (void)found;
    }
    found = pico8_pm_default("nu", &v);
    g_pm_nu = family_default_table(found, v);
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

    for (i = 0; i < sizeof g_ga_defaults / sizeof g_ga_defaults[0]; i++) {
        int found = pico8_ga_default(g_ga_defaults[i].name, &g_ga_defaults[i].value);
        assert(found && "ga: every default of the struct is a key of ga's spec");
        (void)found;
    }
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
