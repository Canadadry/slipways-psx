#include "pico8_wormhole.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_audio.h"
#include "pico8_collect.h"
#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_ke.h"
#include "pico8_planet.h"
#include "pico8_probe.h"
#include "pico8_tech_gate.h"
#include "pico8_wormhole_display.h"
#include "pico8_ref_audit.h"

static pico8_class_t *g_mz_class = NULL;
static pico8_instance_t *g_mz_jj_wrapper = NULL;
static pico8_ob_table_t *g_mz_nu = NULL;
static pico8_ob_table_t *g_mz_kk = NULL;
static pico8_ob_table_t *g_mz_jj = NULL;

static int pico8_mz_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

static pico8_vec_t pico8_mz_value_vec(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static pico8_vec_t pico8_mz_field_vec(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v)) {
        return pico8_mz_value_vec(v);
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static int pico8_mz_pq_ox_predicate(pico8_value_t e, void *ctx) {
    pico8_instance_t *p;
    pico8_value_t fx_val;
    (void)ctx;
    if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
        return 0;
    }
    p = (pico8_instance_t *)e.as.ref;
    if (!pico8_entity_get_field(p, "fx", &fx_val)) {
        return 0;
    }
    return pico8_mz_truthy(fx_val);
}

void pico8_mz_pq(pico8_mz_t *self) {
    pico8_list_t *heap_ox;

    if (self == NULL) {
        return;
    }

    pico8_rn(pico8_value_fixed(pico8_fixed_from_int(53)));

    /* TROU-7 */
    heap_ox = malloc(sizeof *heap_ox);
    *heap_ox = pico8_jp(pico8_hv(), pico8_mz_pq_ox_predicate, NULL);
    self->ox = pico8_value_ref(heap_ox);
}

void pico8_mz_hq(pico8_mz_t *self, pico8_value_t mp, pico8_value_t rp) {
    pico8_vec_t p_vec;
    const pico8_list_t *ox_list = NULL;
    pico8_value_t nearest = pico8_value_ref(NULL);
    pico8_fixed_t nq;
    pico8_value_t threshold_val;
    pico8_fixed_t threshold;

    pico8_bv((pico8_instance_t *)self, mp, rp);
    if (self == NULL) {
        return;
    }

    p_vec = pico8_mz_value_vec(self->p);
    if (self->ox.kind == PICO8_VALUE_REF && self->ox.as.ref != NULL) {
        ox_list = PICO8_REF_AS_LIST_CONST(self->ox.as.ref);
    }

    nq = pico8_ij(p_vec, ox_list, &nearest);
    self->ij = nearest;

    threshold_val = pico8_ov(pico8_value_string("space folding"), pico8_value_fixed(PICO8_FIXED_LIT(0x000030a3, 0.19)),
                              pico8_value_fixed(PICO8_FIXED_LIT(0x0000451e, 0.27)));
    threshold = (threshold_val.kind == PICO8_VALUE_FIXED) ? threshold_val.as.fixed : pico8_fixed_from_int(0);

    if (nq > threshold) {
        self->et = pico8_value_ref(pico8_mz_jj_wrapper());
    }
}

static pico8_value_t pico8_mz_hq_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    pico8_mz_hq((pico8_mz_t *)self, arg0, arg1);
    return pico8_value_ref(NULL);
}

void pico8_mz_kz(pico8_mz_t *self, pico8_fixed_t t) {
    pico8_value_t threshold_val;
    pico8_fixed_t threshold;
    pico8_fixed_t one;
    pico8_fixed_t td15;
    pico8_fixed_t min_val;
    pico8_fixed_t sin_term;
    pico8_fixed_t result;

    if (self == NULL) {
        return;
    }

    threshold_val = pico8_ov(pico8_value_string("space folding"), pico8_value_fixed(pico8_fixed_from_int(29)),
                              pico8_value_fixed(pico8_fixed_from_int(35)));
    threshold = (threshold_val.kind == PICO8_VALUE_FIXED) ? threshold_val.as.fixed : pico8_fixed_from_int(0);

    one = pico8_fixed_from_int(1);
    td15 = pico8_fixed_div(t, pico8_fixed_from_int(15));
    min_val = (td15 < one) ? td15 : one;
    sin_term =
        pico8_fixed_mul(pico8_fixed_sin(pico8_fixed_div(t, pico8_fixed_from_int(40))), PICO8_FIXED_LIT(0x00001999, 0.1));

    result = pico8_fixed_mul(threshold, pico8_fixed_add(min_val, sin_term));
    self->oi = pico8_value_fixed(result);
}

static pico8_value_t pico8_mz_kz_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_fixed_t t = pico8_fixed_from_int(0);
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_FIXED) {
        t = arg0.as.fixed;
    }
    pico8_mz_kz((pico8_mz_t *)self, t);
    return pico8_value_ref(NULL);
}

void pico8_mz_ey(pico8_mz_t *self) {
    pico8_fixed_t t = pico8_fixed_from_int(0);
    int result;

    if (self == NULL) {
        return;
    }

    if (self->ke.t.kind == PICO8_VALUE_FIXED) {
        t = self->ke.t.as.fixed;
    }

    /* The `or` short-circuits: `d_`'s side effects must not run before tick
     * 15, so this call cannot be hoisted out of the else branch. */
    if (t < pico8_fixed_from_int(15)) {
        result = 1;
    } else {
        result = !pico8_mz_truthy(pico8_d_((pico8_instance_t *)self));
    }

    self->p_ = pico8_value_bool(result);
}

static pico8_value_t pico8_mz_ey_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_mz_ey((pico8_mz_t *)self);
    return pico8_value_ref(NULL);
}

void pico8_mz_pr(pico8_mz_t *self, pico8_fixed_t t) {
    pico8_fixed_t sr;
    pico8_vec_t self_p;
    const pico8_list_t *jo_list;
    int i;

    if (self == NULL) {
        return;
    }

    sr = pico8_fixed_add(pico8_fixed_div(pico8_mz_ej(self, t), pico8_fixed_from_int(128)),
                          PICO8_FIXED_LIT(0x000007ef, 0.031));

    self_p = pico8_mz_value_vec(self->p);
    jo_list = pico8_cg("jo");
    if (jo_list != NULL) {
        for (i = 0; i < jo_list->count; i++) {
            pico8_value_t pt_val = jo_list->items[i];
            pico8_instance_t *pt;
            pico8_value_t me_val;

            if (pt_val.kind != PICO8_VALUE_REF || pt_val.as.ref == NULL) {
                continue;
            }
            pt = (pico8_instance_t *)pt_val.as.ref;

            if (pico8_instance_get_field(pt, "me", &me_val) && me_val.kind == PICO8_VALUE_STRING &&
                me_val.as.string != NULL && strcmp(me_val.as.string, "unknown") == 0) {
                pico8_vec_t pt_p = pico8_mz_field_vec(pt, "p");
                pico8_vec_t diff = pico8_vec_sub(pt_p, self_p);
                if (pico8_vec_qx(diff) < sr) {
                    pico8_jo_hh(pt);
                }
            }
        }
    }

    self->p_ = pico8_value_bool(t > pico8_fixed_from_int(25));
}

static pico8_value_t pico8_mz_pr_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_fixed_t t = pico8_fixed_from_int(0);
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_FIXED) {
        t = arg0.as.fixed;
    }
    pico8_mz_pr((pico8_mz_t *)self, t);
    return pico8_value_ref(NULL);
}

pico8_fixed_t pico8_mz_ej(pico8_mz_t *self, pico8_fixed_t t) {
    pico8_fixed_t sq = pico8_fixed_sqrt(pico8_fixed_div(t, pico8_fixed_from_int(15)));
    pico8_fixed_t one = pico8_fixed_from_int(1);
    pico8_fixed_t min_val = (sq < one) ? sq : one;
    pico8_fixed_t oi = pico8_fixed_from_int(0);

    if (self != NULL && self->oi.kind == PICO8_VALUE_FIXED) {
        oi = self->oi.as.fixed;
    }
    return pico8_fixed_mul(min_val, oi);
}

static pico8_value_t pico8_mz_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_vec_t p = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        p = *(const pico8_vec_t *)arg0.as.ref;
    }
    pico8_mz_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_mz_t *)self, p);
    return pico8_value_ref(NULL);
}

static int pico8_mz_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_mz_t *mz = self;
    static const struct {
        const char *name;
        size_t offset;
    } members[] = {
        {"nu", offsetof(pico8_mz_t, nu)},     {"dw", offsetof(pico8_mz_t, dw)},
        {"dq", offsetof(pico8_mz_t, dq)},     {"spawn_snd", offsetof(pico8_mz_t, spawn_snd)},
        {"ik", offsetof(pico8_mz_t, ik)},     {"oi", offsetof(pico8_mz_t, oi)},
        {"od", offsetof(pico8_mz_t, od)},     {"fp", offsetof(pico8_mz_t, fp)},
        {"fa", offsetof(pico8_mz_t, fa)},     {"kk", offsetof(pico8_mz_t, kk)},
        {"jj", offsetof(pico8_mz_t, jj)},     {"ox", offsetof(pico8_mz_t, ox)},
        {"ra", offsetof(pico8_mz_t, ra)},     {"p", offsetof(pico8_mz_t, p)},
        {"et", offsetof(pico8_mz_t, et)},     {"ij", offsetof(pico8_mz_t, ij)},
        {"p_", offsetof(pico8_mz_t, p_)},
    };
    size_t i;

    for (i = 0; i < sizeof members / sizeof members[0]; i++) {
        if (strcmp(name, members[i].name) == 0) {
            pico8_value_t value = *(const pico8_value_t *)((const char *)mz + members[i].offset);
            if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
                return 0;
            }
            *out_value = value;
            return 1;
        }
    }
    return pico8_ke_field(self, name, out_value);
}

/* `ra`, `p`, `et` (bv) and `p_` (d_): the only names a generic writer
 * ever writes on an `mz`. */
static int pico8_mz_set_field(void *self, const char *name, pico8_value_t value) {
    pico8_mz_t *mz = self;

    if (strcmp(name, "ra") == 0) {
        mz->ra = value;
    } else if (strcmp(name, "p") == 0) {
        mz->p = value;
    } else if (strcmp(name, "et") == 0) {
        mz->et = value;
    } else if (strcmp(name, "p_") == 0) {
        mz->p_ = value;
    } else {
        return 0;
    }
    return 1;
}

/* Lua's `mz.hq=bv` is rebound three statements later by `function mz:hq`,
 * so only that second definition is ever reachable. Putting
 * pico8_bv_dispatch in the HQ entry instead would look faithful and would
 * drop the nearest-anchor search. */
static const pico8_struct_class_t g_mz_struct_class = {
    {
        [PICO8_METHOD_HQ] = pico8_mz_hq_dispatch,
        [PICO8_METHOD_EV] = pico8_dl_dispatch,
        [PICO8_METHOD_KJ] = pico8_mz_kj_dispatch,
        [PICO8_METHOD_KZ] = pico8_mz_kz_dispatch,
        [PICO8_METHOD_EY] = pico8_mz_ey_dispatch,
        [PICO8_METHOD_PR] = pico8_mz_pr_dispatch,
    },
    pico8_mz_field,
    pico8_mz_set_field,
};

static void *pico8_mz_struct_new(const pico8_ob_table_t *overrides) {
    pico8_mz_t *self = malloc(sizeof *self);

    pico8_ke_struct_init(&self->ke, &g_mz_struct_class);
    self->ke.me = pico8_value_string("kz");
    self->nu = pico8_value_ref(g_mz_nu);
    self->dw = pico8_value_fixed(pico8_fixed_from_int(0));
    self->dq = pico8_value_string("pr");
    self->spawn_snd = pico8_value_fixed(pico8_fixed_from_int(52));
    self->ik = pico8_value_fixed(pico8_fixed_from_int(49));
    self->oi = pico8_value_fixed(pico8_fixed_from_int(0));
    self->od = pico8_value_fixed(pico8_fixed_from_int(3));
    self->fp = pico8_value_fixed(pico8_fixed_from_int(1));
    self->fa = pico8_value_fixed(pico8_fixed_from_int(5));
    self->kk = pico8_value_ref(g_mz_kk);
    self->jj = pico8_value_ref(g_mz_jj);
    self->ox = pico8_value_ref(NULL);
    self->ra = pico8_value_ref(NULL);
    self->p = pico8_value_ref(NULL);
    self->et = pico8_value_ref(NULL);
    self->ij = pico8_value_ref(NULL);
    self->p_ = pico8_value_ref(NULL);
    assert((overrides == NULL || overrides->count == 0) &&
           "mz: no construction table was measured; a key would be a field under no rule of plan-3 section 5 "
           "-- stop, do not add a member");
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("mz");
#endif
    pico8_mz_pq(self);
    pico8_ke_struct_pq(&self->ke);
    return self;
}

static pico8_ob_table_t *pico8_mz_spec_table(const pico8_ob_table_t *spec, const char *name) {
    pico8_value_t v = pico8_value_ref(NULL);
    int found = pico8_ob_table_get_name(spec, name, &v);

    assert(found && v.kind == PICO8_VALUE_REF && v.as.ref != NULL &&
           pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_OB_TABLE &&
           "mz's nu, kk and jj spec entries must parse to tables (the spec's trailing comma is load-bearing)");
    (void)found;
    return (pico8_ob_table_t *)v.as.ref;
}

void pico8_wormhole_init(void) {
    pico8_ob_table_t *spec;

    pico8_wormhole_shutdown();

    g_mz_class = pico8_class_declare(pico8_ke_class(),
                                      "\nnu=o(\"mz\"),\nme=\"kz\",dw=0,\ndq=\"pr\",\nspawn_snd=52,ik=49,\noi=0,"
                                      "\nod=3,fp=1,\nfa=5,\nkk=o(7,7,6,13,5,1,1,1,0),\njj=o(qs=\"-too far-\"),\n");
    PICO8_CLASS_COVERAGE_NAME(g_mz_class, "mz");
    pico8_class_set_struct_new(g_mz_class, pico8_mz_struct_new);
    pico8_class_register_named("mz", g_mz_class);

    spec = pico8_ob("\nnu=o(\"mz\"),\nkk=o(7,7,6,13,5,1,1,1,0),\njj=o(qs=\"-too far-\"),\n", NULL);
    g_mz_nu = pico8_mz_spec_table(spec, "nu");
    g_mz_kk = pico8_mz_spec_table(spec, "kk");
    g_mz_jj = pico8_mz_spec_table(spec, "jj");
    pico8_ob_table_free(spec);

    g_mz_jj_wrapper = malloc(sizeof *g_mz_jj_wrapper);
    g_mz_jj_wrapper->tag = PICO8_TYPE_TAG_INSTANCE;
    g_mz_jj_wrapper->fields = g_mz_jj;
    g_mz_jj_wrapper->cls = NULL;
}

/* Plain `free()` on the wrapper, never `pico8_instance_free()`: its
 * `->fields` is the module's `jj` table, freed after it. */
void pico8_wormhole_shutdown(void) {
    if (g_mz_class != NULL) {
        pico8_class_free(g_mz_class);
        g_mz_class = NULL;
    }
    if (g_mz_jj_wrapper != NULL) {
        free(g_mz_jj_wrapper);
        g_mz_jj_wrapper = NULL;
    }
    pico8_ob_table_free(g_mz_nu);
    g_mz_nu = NULL;
    pico8_ob_table_free(g_mz_kk);
    g_mz_kk = NULL;
    pico8_ob_table_free(g_mz_jj);
    g_mz_jj = NULL;
}

pico8_class_t *pico8_mz_class(void) {
    return g_mz_class;
}

const pico8_struct_class_t *pico8_mz_struct_class(void) {
    return &g_mz_struct_class;
}

pico8_mz_t *pico8_mz_new(const pico8_ob_table_t *overrides) {
    return pico8_mz_struct_new(overrides);
}

pico8_instance_t *pico8_mz_jj_wrapper(void) {
    return g_mz_jj_wrapper;
}
