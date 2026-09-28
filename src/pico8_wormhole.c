#include "pico8_wormhole.h"

#include <assert.h>
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
#include "pico8_slipway.h"
#include "pico8_tech_gate.h"
#include "pico8_wormhole_display.h"

static pico8_ob_table_t *g_mz_spec = NULL;
static pico8_ob_table_t *g_mz_nu = NULL;
static pico8_ob_table_t *g_mz_kk = NULL;

static int pico8_mz_jj_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_mz_jj_t *jj = self;

    if (strcmp(name, "qs") == 0) {
        if (jj->qs == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(jj->qs);
        return 1;
    }
    return 0;
}

static const pico8_struct_class_t g_mz_jj_struct_class = {
    {0},
    pico8_mz_jj_field,
    NULL,
};

/* The only one: never allocated, never freed; no writer, so a write
 * reaching it stops the run. */
static pico8_mz_jj_t g_mz_jj_wrapper = {
    {PICO8_TYPE_TAG_STRUCT, &g_mz_jj_struct_class},
    "-too far-",
};

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

static pico8_vec_t pico8_mz_field_vec(const pico8_struct_header_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_entity_get_field(inst, name, &v)) {
        return pico8_mz_value_vec(v);
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static int pico8_mz_pq_ox_predicate(pico8_value_t e, void *ctx) {
    pico8_struct_header_t *p;
    pico8_value_t fx_val;
    (void)ctx;
    if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
        return 0;
    }
    p = (pico8_struct_header_t *)e.as.ref;
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
    self->ox = heap_ox;
}

void pico8_mz_hq(pico8_mz_t *self, pico8_value_t mp, pico8_value_t rp) {
    pico8_vec_t p_vec;
    pico8_value_t nearest = pico8_value_ref(NULL);
    pico8_fixed_t nq;
    pico8_value_t threshold_val;
    pico8_fixed_t threshold;

    pico8_bv((pico8_struct_header_t *)self, mp, rp);
    if (self == NULL) {
        return;
    }

    p_vec = pico8_mz_value_vec(pico8_value_ref(self->p));
    nq = pico8_ij(p_vec, self->ox, &nearest);
    self->ij = nearest.as.ref;

    threshold_val = pico8_ov(pico8_value_string("space folding"), pico8_value_fixed(PICO8_FIXED_LIT(0x000030a3, 0.19)),
                              pico8_value_fixed(PICO8_FIXED_LIT(0x0000451e, 0.27)));
    threshold = (threshold_val.kind == PICO8_VALUE_FIXED) ? threshold_val.as.fixed : pico8_fixed_from_int(0);

    if (nq > threshold) {
        self->et = (pico8_struct_header_t *)pico8_mz_jj_wrapper();
        self->has_et = 1;
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
    self->oi = result;
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
    pico8_fixed_t t;
    int result;

    if (self == NULL) {
        return;
    }

    t = self->ke.t;

    /* The `or` short-circuits: `d_`'s side effects must not run before tick
     * 15, so this call cannot be hoisted out of the else branch. */
    if (t < pico8_fixed_from_int(15)) {
        result = 1;
    } else {
        result = !pico8_mz_truthy(pico8_d_((pico8_struct_header_t *)self));
    }

    self->p_ = result;
    self->has_p_ = 1;
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

    self_p = pico8_mz_value_vec(pico8_value_ref(self->p));
    jo_list = pico8_cg("jo");
    if (jo_list != NULL) {
        for (i = 0; i < jo_list->count; i++) {
            pico8_value_t pt_val = jo_list->items[i];
            pico8_struct_header_t *pt;
            pico8_value_t me_val;

            if (pt_val.kind != PICO8_VALUE_REF || pt_val.as.ref == NULL) {
                continue;
            }
            pt = (pico8_struct_header_t *)pt_val.as.ref;

            if (pico8_entity_get_field(pt, "me", &me_val) && me_val.kind == PICO8_VALUE_STRING &&
                me_val.as.string != NULL && strcmp(me_val.as.string, "unknown") == 0) {
                pico8_vec_t pt_p = pico8_mz_field_vec(pt, "p");
                pico8_vec_t diff = pico8_vec_sub(pt_p, self_p);
                if (pico8_vec_qx(diff) < sr) {
                    pico8_jo_hh((pico8_jo_t *)pt);
                }
            }
        }
    }

    self->p_ = t > pico8_fixed_from_int(25);
    self->has_p_ = 1;
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

    if (self != NULL) {
        oi = self->oi;
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

pico8_value_t pico8_mz_et(const pico8_mz_t *self) {
    if (self == NULL || !self->has_et) {
        return pico8_value_ref(NULL);
    }
    if (self->et == NULL) {
        return pico8_value_bool(0);
    }
    return pico8_value_ref(self->et);
}

static int mz_ref_field(void *ref, pico8_value_t *out_value) {
    if (ref == NULL) {
        return 0;
    }
    *out_value = pico8_value_ref(ref);
    return 1;
}

static int mz_fixed_field(pico8_fixed_t x, pico8_value_t *out_value) {
    *out_value = pico8_value_fixed(x);
    return 1;
}

static int pico8_mz_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_mz_t *mz = self;

    if (strcmp(name, "nu") == 0) {
        return mz_ref_field(mz->nu, out_value);
    }
    if (strcmp(name, "dw") == 0) {
        return mz_fixed_field(mz->dw, out_value);
    }
    if (strcmp(name, "dq") == 0) {
        if (mz->dq == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(mz->dq);
        return 1;
    }
    if (strcmp(name, "spawn_snd") == 0) {
        return mz_fixed_field(mz->spawn_snd, out_value);
    }
    if (strcmp(name, "ik") == 0) {
        return mz_fixed_field(mz->ik, out_value);
    }
    if (strcmp(name, "oi") == 0) {
        return mz_fixed_field(mz->oi, out_value);
    }
    if (strcmp(name, "od") == 0) {
        return mz_fixed_field(mz->od, out_value);
    }
    if (strcmp(name, "fp") == 0) {
        return mz_fixed_field(mz->fp, out_value);
    }
    if (strcmp(name, "fa") == 0) {
        return mz_fixed_field(mz->fa, out_value);
    }
    if (strcmp(name, "kk") == 0) {
        return mz_ref_field(mz->kk, out_value);
    }
    if (strcmp(name, "jj") == 0) {
        return mz_ref_field(mz->jj, out_value);
    }
    if (strcmp(name, "ox") == 0) {
        return mz_ref_field(mz->ox, out_value);
    }
    if (strcmp(name, "ra") == 0) {
        return mz_ref_field(mz->ra, out_value);
    }
    if (strcmp(name, "p") == 0) {
        return mz_ref_field(mz->p, out_value);
    }
    if (strcmp(name, "et") == 0) {
        pico8_value_t et = pico8_mz_et(mz);
        if (et.kind == PICO8_VALUE_REF && et.as.ref == NULL) {
            return 0;
        }
        *out_value = et;
        return 1;
    }
    if (strcmp(name, "ij") == 0) {
        return mz_ref_field(mz->ij, out_value);
    }
    if (strcmp(name, "p_") == 0) {
        if (!mz->has_p_) {
            return 0;
        }
        *out_value = pico8_value_bool(mz->p_);
        return 1;
    }
    return pico8_ke_field(self, name, out_value);
}

static void mz_take_vec(pico8_value_t value, pico8_vec_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && value.as.ref != NULL &&
           pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_UNKNOWN &&
           "mz.ra/p: a write neither nil nor a vector ref does not fit the member's type "
           "-- stop, do not widen the member");
    *out = value.as.ref;
}

/* What `bv` aims at from an `mz` (never a planet: `dw` is 0, so `nq<dw`
 * never holds), plus the `jj` wrapper `mz:hq` puts there. */
static int mz_is_et_class(const pico8_struct_class_t *cls) {
    return cls == pico8_jc_struct_class() || cls == pico8_bv_et_struct_class() || cls == pico8_mz_jj_struct_class();
}

static void mz_take_et(pico8_value_t value, pico8_struct_header_t **out, int *has) {
    *out = NULL;
    *has = 0;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(((value.kind == PICO8_VALUE_BOOL && value.as.boolean == 0) ||
            (value.kind == PICO8_VALUE_REF && pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_STRUCT &&
             mz_is_et_class(((const pico8_struct_header_t *)value.as.ref)->cls))) &&
           "mz.et: a write neither nil, false, nor what bv or mz:hq aims at (a jc, bv's throwaway target, the jj "
           "wrapper) does not fit the member's type -- stop, do not widen the member");
    *out = value.kind == PICO8_VALUE_REF ? value.as.ref : NULL;
    *has = 1;
}

static void mz_take_bool(pico8_value_t value, int *out, int *has) {
    *out = 0;
    *has = 0;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_BOOL &&
           "mz.p_: a write neither nil nor a boolean does not fit the member's type -- stop, do not widen the member");
    *out = value.as.boolean != 0;
    *has = 1;
}

/* `ra`, `p`, `et` (bv) and `p_` (d_): the only names a generic writer
 * ever writes on an `mz`. */
static int pico8_mz_set_field(void *self, const char *name, pico8_value_t value) {
    pico8_mz_t *mz = self;

    if (strcmp(name, "ra") == 0) {
        mz_take_vec(value, &mz->ra);
        return 1;
    }
    if (strcmp(name, "p") == 0) {
        mz_take_vec(value, &mz->p);
        return 1;
    }
    if (strcmp(name, "et") == 0) {
        mz_take_et(value, &mz->et, &mz->has_et);
        return 1;
    }
    if (strcmp(name, "p_") == 0) {
        mz_take_bool(value, &mz->p_, &mz->has_p_);
        return 1;
    }
    return 0;
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
    self->ke.me = "kz";
    self->nu = g_mz_nu;
    self->dw = pico8_fixed_from_int(0);
    self->dq = "pr";
    self->spawn_snd = pico8_fixed_from_int(52);
    self->ik = pico8_fixed_from_int(49);
    self->oi = pico8_fixed_from_int(0);
    self->od = pico8_fixed_from_int(3);
    self->fp = pico8_fixed_from_int(1);
    self->fa = pico8_fixed_from_int(5);
    self->kk = g_mz_kk;
    self->jj = &g_mz_jj_wrapper;
    self->ox = NULL;
    self->ra = NULL;
    self->p = NULL;
    self->et = NULL;
    self->has_et = 0;
    self->ij = NULL;
    self->p_ = 0;
    self->has_p_ = 0;
    assert((overrides == NULL || overrides->count == 0) &&
           "mz: takes no construction table; a key has no member -- stop, do not add one");
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
           "mz's nu and kk spec entries must parse to tables (the spec's trailing comma is load-bearing)");
    (void)found;
    return (pico8_ob_table_t *)v.as.ref;
}

void pico8_wormhole_init(void) {
    pico8_ob_table_t *spec;

    pico8_wormhole_shutdown();

    g_mz_spec = pico8_ob("\nnu=o(\"mz\"),\nme=\"kz\",dw=0,\ndq=\"pr\",\nspawn_snd=52,ik=49,\noi=0,"
                         "\nod=3,fp=1,\nfa=5,\nkk=o(7,7,6,13,5,1,1,1,0),\njj=o(qs=\"-too far-\"),\n",
                         NULL);

    spec = pico8_ob("\nnu=o(\"mz\"),\nkk=o(7,7,6,13,5,1,1,1,0),\n", NULL);
    g_mz_nu = pico8_mz_spec_table(spec, "nu");
    g_mz_kk = pico8_mz_spec_table(spec, "kk");
    pico8_ob_table_free(spec);
}

void pico8_wormhole_shutdown(void) {
    pico8_ob_table_free(g_mz_spec);
    g_mz_spec = NULL;
    pico8_ob_table_free(g_mz_nu);
    g_mz_nu = NULL;
    pico8_ob_table_free(g_mz_kk);
    g_mz_kk = NULL;
}

int pico8_mz_ready(void) {
    return g_mz_spec != NULL;
}

int pico8_mz_default(const char *name, pico8_value_t *out_value) {
    return g_mz_spec != NULL &&
           (pico8_ob_table_get_name(g_mz_spec, name, out_value) || pico8_ke_default(name, out_value));
}

const pico8_struct_class_t *pico8_mz_struct_class(void) {
    return &g_mz_struct_class;
}

pico8_mz_t *pico8_mz_new(const pico8_ob_table_t *overrides) {
    return pico8_mz_struct_new(overrides);
}

pico8_mz_jj_t *pico8_mz_jj_wrapper(void) {
    return &g_mz_jj_wrapper;
}

const pico8_struct_class_t *pico8_mz_jj_struct_class(void) {
    return &g_mz_jj_struct_class;
}
