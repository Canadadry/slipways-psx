#include "pico8_help.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_ke.h"
#include "pico8_msgbox.h"
#include "pico8_particle.h"
#include "pico8_pi.h"
#include "pico8_probe.h"
#include "pico8_struct.h"
#include "pico8_type_tag.h"

static pico8_ob_table_t *g_fv_spec = NULL;
static pico8_ob_table_t *g_bo = NULL;

/* Returns a new, heap-backed list the caller owns and must release with
 * pico8_list_free(). */
static pico8_list_t pico8_help_positional_values(const pico8_ob_table_t *table) {
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

static pico8_value_t pico8_fv_eu_placeholder(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_value_ref(NULL);
}

static pico8_value_function_t g_eu_dispatch = pico8_fv_eu_placeholder;

pico8_value_t pico8_eu_field_value(void) {
    return pico8_value_function(g_eu_dispatch);
}

void pico8_help_set_eu(pico8_value_function_t fn) {
    g_eu_dispatch = (fn != NULL) ? fn : pico8_fv_eu_placeholder;
}

static pico8_value_t pico8_fv_hd_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    return pico8_gk((const pico8_struct_header_t *)self, arg0, arg1);
}

void pico8_fv_pq(pico8_fv_t *self) {
    int i;

    if (g_bo != NULL) {
        for (i = 0; i < g_bo->count; i++) {
            if (g_bo->entries[i].key.kind == PICO8_OB_KEY_INDEX && g_bo->entries[i].value.kind == PICO8_VALUE_REF &&
                g_bo->entries[i].value.as.ref != NULL) {
                pico8_ob_table_t *d = (pico8_ob_table_t *)g_bo->entries[i].value.as.ref;
                assert(pico8_type_tag_of(d) == PICO8_TYPE_TAG_OB_TABLE && "bo's rows must be plain spec tables");
                pico8_ob_table_set_name(d, "my", pico8_eu_field_value());
            }
        }
    }

    if (self != NULL) {
        pico8_list_t items = pico8_help_positional_values(g_bo);
        pico8_pz((const pico8_struct_header_t *)self, &items);
        pico8_list_free(&items);
    }
}

void pico8_fv_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_fv_t *self, pico8_vec_t p) {
    const pico8_ob_table_t *rb_table = NULL;

    (void)p;

    pico8_ei(gfx, mem, (pico8_struct_header_t *)self);

    if (self != NULL && self->rb.kind == PICO8_VALUE_REF && self->rb.as.ref != NULL) {
        assert(pico8_type_tag_of(self->rb.as.ref) == PICO8_TYPE_TAG_OB_TABLE &&
               "fv's rb field must hold a plain spec table");
        rb_table = (const pico8_ob_table_t *)self->rb.as.ref;
    }

    if (gfx != NULL && mem != NULL) {
        pico8_list_t items = pico8_help_positional_values(rb_table);
        pico8_bl_default(gfx, mem, &items);
        pico8_list_free(&items);
    }
}

pico8_value_t pico8_fv_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_vec_t p = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        p = *(const pico8_vec_t *)arg0.as.ref;
    }
    pico8_fv_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_fv_t *)self, p);
    return pico8_value_ref(NULL);
}

/* fv's 22 spec defaults, read once through pico8_fv_default() at init: the
 * 13 that override g_'s land in g_'s members. */
static struct {
    const char *name;
    size_t offset;
    pico8_value_t value;
} g_fv_defaults[] = {
    {"fa", offsetof(pico8_fv_t, g_.fa), {0}}, {"ra", offsetof(pico8_fv_t, g_.ra), {0}},
    {"hi", offsetof(pico8_fv_t, hi), {0}},    {"ca", offsetof(pico8_fv_t, ca), {0}},
    {"gq", offsetof(pico8_fv_t, gq), {0}},    {"eo", offsetof(pico8_fv_t, eo), {0}},
    {"ff", offsetof(pico8_fv_t, ff), {0}},    {"fc", offsetof(pico8_fv_t, g_.fc), {0}},
    {"k_", offsetof(pico8_fv_t, g_.k_), {0}}, {"qo", offsetof(pico8_fv_t, g_.qo), {0}},
    {"ro", offsetof(pico8_fv_t, g_.ro), {0}}, {"qg", offsetof(pico8_fv_t, g_.qg), {0}},
    {"q_", offsetof(pico8_fv_t, g_.q_), {0}}, {"pn", offsetof(pico8_fv_t, g_.pn), {0}},
    {"du", offsetof(pico8_fv_t, g_.du), {0}}, {"py", offsetof(pico8_fv_t, g_.py), {0}},
    {"pk", offsetof(pico8_fv_t, g_.pk), {0}}, {"ki", offsetof(pico8_fv_t, g_.ki), {0}},
    {"kq", offsetof(pico8_fv_t, kq), {0}},    {"ql", offsetof(pico8_fv_t, ql), {0}},
    {"fg", offsetof(pico8_fv_t, fg), {0}},    {"rb", offsetof(pico8_fv_t, rb), {0}},
};

static void pico8_fv_defaults_reset(void) {
    size_t i;
    for (i = 0; i < sizeof g_fv_defaults / sizeof g_fv_defaults[0]; i++) {
        g_fv_defaults[i].value = pico8_value_ref(NULL);
    }
}

static const struct {
    const char *name;
    size_t offset;
} g_fv_members[] = {
    {"hi", offsetof(pico8_fv_t, hi)}, {"ca", offsetof(pico8_fv_t, ca)}, {"gq", offsetof(pico8_fv_t, gq)},
    {"eo", offsetof(pico8_fv_t, eo)}, {"ff", offsetof(pico8_fv_t, ff)}, {"kq", offsetof(pico8_fv_t, kq)},
    {"ql", offsetof(pico8_fv_t, ql)}, {"fg", offsetof(pico8_fv_t, fg)}, {"rb", offsetof(pico8_fv_t, rb)},
    {"m_", offsetof(pico8_fv_t, m_)},
};

static int pico8_fv_field(const void *self, const char *name, pico8_value_t *out_value) {
    size_t i;

    for (i = 0; i < sizeof g_fv_members / sizeof g_fv_members[0]; i++) {
        if (strcmp(name, g_fv_members[i].name) == 0) {
            const pico8_value_t *member = (const pico8_value_t *)((const char *)self + g_fv_members[i].offset);
            if (member->kind == PICO8_VALUE_REF && member->as.ref == NULL) {
                return 0;
            }
            *out_value = *member;
            return 1;
        }
    }
    return pico8_g__struct_class()->field(self, name, out_value);
}

/* `m_` (`gw`, hovering the help), then g_'s 5; 0 for any other. */
static int pico8_fv_set_field(void *self, const char *name, pico8_value_t value) {
    if (strcmp(name, "m_") == 0) {
        ((pico8_fv_t *)self)->m_ = value;
        return 1;
    }
    return pico8_g__struct_class()->set_field(self, name, value);
}

static const pico8_struct_class_t g_fv_struct_class = {
    {
        [PICO8_METHOD_HQ] = pico8_bv_dispatch,
        [PICO8_METHOD_EY] = pico8_d__dispatch,
        [PICO8_METHOD_EV] = pico8_dl_dispatch,
        [PICO8_METHOD_MM] = pico8_g__mm,
        [PICO8_METHOD_KJ] = pico8_fv_kj_dispatch,
        [PICO8_METHOD_HD] = pico8_fv_hd_dispatch,
    },
    pico8_fv_field,
    pico8_fv_set_field,
};

static void *pico8_fv_struct_new(const pico8_ob_table_t *overrides) {
    pico8_fv_t *self;
    size_t i;

    assert(g_fv_spec != NULL && "pico8_help_init() must run first");
    self = malloc(sizeof *self);
    pico8_g__struct_init(&self->g_, &g_fv_struct_class);
    self->m_ = pico8_value_ref(NULL);
    for (i = 0; i < sizeof g_fv_defaults / sizeof g_fv_defaults[0]; i++) {
        *(pico8_value_t *)((char *)self + g_fv_defaults[i].offset) = g_fv_defaults[i].value;
    }
    assert((overrides == NULL || overrides->count == 0) &&
           "fv: the only measured road is pico8_fv_new(NULL) (cart.c); a construction key would be a field under "
           "no rule of plan-3 section 5 -- stop, do not add a member");
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("fv");
#endif
    pico8_fv_pq(self);
    pico8_f_((pico8_struct_header_t *)self);
    pico8_ke_struct_pq(&self->g_.ke);
    return self;
}

/* This module, unlike most, does not call pico8_pi_init() itself, and `fv`
 * extends `g_`, not `ke`: both preconditions are asserted, not repaired. */
void pico8_help_init(void) {
    size_t i;

    assert(pico8_ob_lookup_constructor('v') != NULL && "pico8_help_init() runs after pico8_pi_init()");
    assert(pico8_g__ready() && "pico8_help_init() runs after pico8_probe_init()");

    pico8_help_shutdown();

    g_bo = pico8_ob("\no(lq=o(49,\"forgiving   \"),px=\"forgiving\",\ntrade=7,en=-0.99),\n"
                     "o(lq=o(50,\"reasonable  \"),px=\"reasonable\",\ntrade=6,en=-0.6),\n"
                     "o(lq=o(51,\"challenging \"),px=\"challenging\",\ntrade=5,en=-0.4),\n"
                     "o(lq=o(52,\"tough       \"),px=\"tough\",\ntrade=4,en=-0.2),\n",
                     NULL);

    g_fv_spec = pico8_ob("\nfa=10,ra=v(64,28),\nhi=v(34,56),\nca=11,\ngq=v(0,11),\neo=-1,\nff=1,\nfc=5,"
                         "\nk_=o(0,1,1,2,4,9,10,9,4,2,1,1,1),\nqo=1.02,ro=-0.007,\nqg=0.007,q_=1.02,"
                         "\npn=5,du=15,\npy=0.0067,pk=1,\nki=17,\nkq=b(-40,78,40,98),\nql=11,fg=5,"
                         "\nrb=o(\no(192,0,11,16,4,fn=\"s\"),\no(172,20,107,2,2,fn=\"s\"),"
                         "\no(\"need help?\",40,110,13,fn=\"qr\"),\no(\"slipways.net/help\",40,116,5,fn=\"qr\"),\n),\n",
                         NULL);
    for (i = 0; i < sizeof g_fv_defaults / sizeof g_fv_defaults[0]; i++) {
        int found = pico8_fv_default(g_fv_defaults[i].name, &g_fv_defaults[i].value);
        assert(found && "fv: every default of the struct is a key of fv's spec");
        (void)found;
    }

    pico8_help_set_eu(NULL);
}

void pico8_help_shutdown(void) {
    pico8_fv_defaults_reset();
    pico8_ob_table_free(g_fv_spec);
    g_fv_spec = NULL;
    if (g_bo != NULL) {
        pico8_ob_table_free(g_bo);
        g_bo = NULL;
    }
    pico8_help_set_eu(NULL);
}

int pico8_fv_ready(void) {
    return g_fv_spec != NULL;
}

int pico8_fv_default(const char *name, pico8_value_t *out_value) {
    return g_fv_spec != NULL &&
           (pico8_ob_table_get_name(g_fv_spec, name, out_value) || pico8_g__default(name, out_value));
}

const pico8_struct_class_t *pico8_fv_struct_class(void) {
    return &g_fv_struct_class;
}

pico8_fv_t *pico8_fv_new(const pico8_ob_table_t *overrides) {
    return pico8_fv_struct_new(overrides);
}

const pico8_ob_table_t *pico8_bo(void) {
    return g_bo;
}
