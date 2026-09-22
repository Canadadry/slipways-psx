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

/* Field readers: an absent field, a wrong kind, or a NULL instance all read
 * as the zero/false default. */
static int pico8_mz_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

static pico8_fixed_t pico8_mz_field_fixed(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v) && v.kind == PICO8_VALUE_FIXED) {
        return v.as.fixed;
    }
    return pico8_fixed_from_int(0);
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
    if (!pico8_instance_get_field(p, "fx", &fx_val)) {
        return 0;
    }
    return pico8_mz_truthy(fx_val);
}

void pico8_mz_pq(pico8_instance_t *self) {
    pico8_list_t *heap_ox;

    if (self == NULL) {
        return;
    }

    pico8_rn(pico8_value_fixed(pico8_fixed_from_int(53)));

    /* TROU-7 */
    heap_ox = malloc(sizeof *heap_ox);
    *heap_ox = pico8_jp(pico8_hv(), pico8_mz_pq_ox_predicate, NULL);
    pico8_ob_table_set_name(self->fields, "ox", pico8_value_ref(heap_ox));
}

void pico8_mz_hq(pico8_instance_t *self, pico8_value_t mp, pico8_value_t rp) {
    pico8_vec_t p_vec;
    pico8_value_t ox_val;
    const pico8_list_t *ox_list = NULL;
    pico8_value_t nearest = pico8_value_ref(NULL);
    pico8_fixed_t nq;
    pico8_value_t threshold_val;
    pico8_fixed_t threshold;

    pico8_bv(self, mp, rp);
    if (self == NULL) {
        return;
    }

    p_vec = pico8_mz_field_vec(self, "p");
    if (pico8_instance_get_field(self, "ox", &ox_val) && ox_val.kind == PICO8_VALUE_REF && ox_val.as.ref != NULL) {
        ox_list = PICO8_REF_AS_LIST_CONST(ox_val.as.ref);
    }

    nq = pico8_ij(p_vec, ox_list, &nearest);
    pico8_ob_table_set_name(self->fields, "ij", nearest);

    threshold_val = pico8_ov(pico8_value_string("space folding"), pico8_value_fixed(pico8_fixed_from_double(0.19)),
                              pico8_value_fixed(pico8_fixed_from_double(0.27)));
    threshold = (threshold_val.kind == PICO8_VALUE_FIXED) ? threshold_val.as.fixed : pico8_fixed_from_int(0);

    if (nq > threshold) {
        pico8_ob_table_set_name(self->fields, "et", pico8_value_ref(pico8_mz_jj_wrapper()));
    }
}

static pico8_value_t pico8_mz_hq_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    pico8_mz_hq((pico8_instance_t *)self, arg0, arg1);
    return pico8_value_ref(NULL);
}

void pico8_mz_kz(pico8_instance_t *self, pico8_fixed_t t) {
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
        pico8_fixed_mul(pico8_fixed_sin(pico8_fixed_div(t, pico8_fixed_from_int(40))), pico8_fixed_from_double(0.1));

    result = pico8_fixed_mul(threshold, pico8_fixed_add(min_val, sin_term));
    pico8_ob_table_set_name(self->fields, "oi", pico8_value_fixed(result));
}

static pico8_value_t pico8_mz_kz_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_fixed_t t = pico8_fixed_from_int(0);
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_FIXED) {
        t = arg0.as.fixed;
    }
    pico8_mz_kz((pico8_instance_t *)self, t);
    return pico8_value_ref(NULL);
}

void pico8_mz_ey(pico8_instance_t *self) {
    pico8_value_t t_val;
    pico8_fixed_t t = pico8_fixed_from_int(0);
    int result;

    if (self == NULL) {
        return;
    }

    if (pico8_instance_get_field(self, "t", &t_val) && t_val.kind == PICO8_VALUE_FIXED) {
        t = t_val.as.fixed;
    }

    /* The `or` short-circuits: `d_`'s side effects must not run before tick
     * 15, so this call cannot be hoisted out of the else branch. */
    if (t < pico8_fixed_from_int(15)) {
        result = 1;
    } else {
        result = !pico8_mz_truthy(pico8_d_(self));
    }

    pico8_ob_table_set_name(self->fields, "p_", pico8_value_bool(result));
}

static pico8_value_t pico8_mz_ey_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_mz_ey((pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

void pico8_mz_pr(pico8_instance_t *self, pico8_fixed_t t) {
    pico8_fixed_t sr;
    pico8_vec_t self_p;
    const pico8_list_t *jo_list;
    int i;

    if (self == NULL) {
        return;
    }

    sr = pico8_fixed_add(pico8_fixed_div(pico8_mz_ej(self, t), pico8_fixed_from_int(128)),
                          pico8_fixed_from_double(0.031));

    self_p = pico8_mz_field_vec(self, "p");
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

    pico8_ob_table_set_name(self->fields, "p_", pico8_value_bool(t > pico8_fixed_from_int(25)));
}

static pico8_value_t pico8_mz_pr_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_fixed_t t = pico8_fixed_from_int(0);
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_FIXED) {
        t = arg0.as.fixed;
    }
    pico8_mz_pr((pico8_instance_t *)self, t);
    return pico8_value_ref(NULL);
}

pico8_fixed_t pico8_mz_ej(pico8_instance_t *self, pico8_fixed_t t) {
    pico8_fixed_t sq = pico8_fixed_sqrt(pico8_fixed_div(t, pico8_fixed_from_int(15)));
    pico8_fixed_t one = pico8_fixed_from_int(1);
    pico8_fixed_t min_val = (sq < one) ? sq : one;
    pico8_fixed_t oi = pico8_mz_field_fixed(self, "oi");
    return pico8_fixed_mul(min_val, oi);
}

static pico8_value_t pico8_mz_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_vec_t p = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        p = *(const pico8_vec_t *)arg0.as.ref;
    }
    pico8_mz_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_instance_t *)self, p);
    return pico8_value_ref(NULL);
}

void pico8_wormhole_init(void) {
    pico8_ob_table_t *jj_table = NULL;
    pico8_value_t jj_val;

    pico8_wormhole_shutdown();

    g_mz_class = pico8_class_declare(pico8_ke_class(),
                                      "\nnu=o(\"mz\"),\nme=\"kz\",dw=0,\ndq=\"pr\",\nspawn_snd=52,ik=49,\noi=0,"
                                      "\nod=3,fp=1,\nfa=5,\nkk=o(7,7,6,13,5,1,1,1,0),\njj=o(qs=\"-too far-\"),\n");
    pico8_class_set_pq(g_mz_class, pico8_mz_pq);
    /* Lua's `mz.hq=bv` is rebound three statements later by `function
     * mz:hq`, so only that second definition is ever reachable. Registering
     * pico8_bv_dispatch here instead would look faithful and would drop the
     * nearest-anchor search. */
    pico8_ob_table_set_name(g_mz_class->defaults, "hq", pico8_value_function(pico8_mz_hq_dispatch));
    pico8_ob_table_set_name(g_mz_class->defaults, "ev", pico8_value_function(pico8_dl_dispatch));
    pico8_ob_table_set_name(g_mz_class->defaults, "kz", pico8_value_function(pico8_mz_kz_dispatch));
    pico8_ob_table_set_name(g_mz_class->defaults, "ey", pico8_value_function(pico8_mz_ey_dispatch));
    pico8_ob_table_set_name(g_mz_class->defaults, "pr", pico8_value_function(pico8_mz_pr_dispatch));
    pico8_ob_table_set_name(g_mz_class->defaults, "kj", pico8_value_function(pico8_mz_kj_dispatch));
    pico8_class_register_named("mz", g_mz_class);

    if (pico8_ob_table_get_name(g_mz_class->defaults, "jj", &jj_val) && jj_val.kind == PICO8_VALUE_REF &&
        jj_val.as.ref != NULL) {
        jj_table = (pico8_ob_table_t *)jj_val.as.ref;
    }
    assert(jj_table != NULL && pico8_type_tag_of(jj_table) == PICO8_TYPE_TAG_OB_TABLE &&
           "mz's jj spec entry must parse to a table (the spec's trailing comma is load-bearing)");
    g_mz_jj_wrapper = malloc(sizeof *g_mz_jj_wrapper);
    g_mz_jj_wrapper->tag = PICO8_TYPE_TAG_INSTANCE;
    g_mz_jj_wrapper->fields = jj_table;
    g_mz_jj_wrapper->cls = NULL;
}

/* Plain `free()` on the wrapper, never `pico8_instance_free()`: its
 * `->fields` is `mz.jj`'s table, owned by the class defaults. */
void pico8_wormhole_shutdown(void) {
    if (g_mz_class != NULL) {
        pico8_class_free(g_mz_class);
        g_mz_class = NULL;
    }
    if (g_mz_jj_wrapper != NULL) {
        free(g_mz_jj_wrapper);
        g_mz_jj_wrapper = NULL;
    }
}

pico8_class_t *pico8_mz_class(void) {
    return g_mz_class;
}

pico8_instance_t *pico8_mz_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_mz_class, overrides);
}

pico8_instance_t *pico8_mz_jj_wrapper(void) {
    return g_mz_jj_wrapper;
}
