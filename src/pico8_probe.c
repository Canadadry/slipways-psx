#include "pico8_probe.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_dispatch_context.h"
#include "pico8_economy.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_fixed.h"
#include "pico8_geom.h"
#include "pico8_gfx.h"
#include "pico8_hud.h"
#include "pico8_audio.h"
#include "pico8_collect.h"
#include "pico8_ke.h"
#include "pico8_particle.h"
#include "pico8_planet.h"
#include "pico8_slipway.h"
#include "pico8_ui.h"
#include "pico8_wormhole.h"

static int g_g__up = 0;
static pico8_struct_header_t *g_fm = NULL;

static int pico8_fm_placeholder_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_fm_placeholder_t *fm = self;

    if (strcmp(name, "jk") == 0) {
        if (fm->has_jk == 0) {
            return 0;
        }
        *out_value = pico8_value_fixed(fm->jk);
        return 1;
    }
    if (strcmp(name, "nz") == 0) {
        if (fm->has_nz == 0) {
            return 0;
        }
        *out_value = pico8_value_fixed(fm->nz);
        return 1;
    }
    return 0;
}

static const pico8_struct_class_t g_fm_placeholder_struct_class = {
    {NULL},
    pico8_fm_placeholder_field,
    NULL,
};

static pico8_fm_placeholder_t g_fm_placeholder = {
    {PICO8_TYPE_TAG_STRUCT, &g_fm_placeholder_struct_class}, 0, 0, 0, 0};

typedef struct {
    pico8_struct_header_t header;
    pico8_struct_header_t *fo;
} pico8_bv_et_t;

static int pico8_bv_et_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_bv_et_t *et = self;

    if (strcmp(name, "fo") == 0) {
        if (et->fo == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref(et->fo);
        return 1;
    }
    return 0;
}

static int bv_et_is_holder_class(const pico8_struct_class_t *cls) {
    return cls->methods[PICO8_METHOD_HQ] == pico8_bv_dispatch || cls == pico8_mz_struct_class();
}

static void bv_et_take_holder(pico8_value_t value, pico8_struct_header_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_STRUCT &&
           bv_et_is_holder_class(((const pico8_struct_header_t *)value.as.ref)->cls) &&
           "bv_et.fo: a write neither nil nor an object whose hq runs bv (g_, fv, mz, qh, nl, os, hj, pm, nr, fr, mx) "
           "does not fit the member's type -- stop, do not widen the member");
    *out = value.as.ref;
}

static int pico8_bv_et_set_field(void *self, const char *name, pico8_value_t value) {
    pico8_bv_et_t *et = self;

    if (strcmp(name, "fo") == 0) {
        bv_et_take_holder(value, &et->fo);
        return 1;
    }
    return 0;
}

static const pico8_struct_class_t g_bv_et_struct_class = {
    {NULL},
    pico8_bv_et_field,
    pico8_bv_et_set_field,
};

/* The `{}` `bv` puts in `et` over the void, off the panel: never allocated,
 * never freed; only `fo` is written, and the next `bv` overwrites it. */
static pico8_bv_et_t g_bv_et = {{PICO8_TYPE_TAG_STRUCT, &g_bv_et_struct_class}, NULL};

static int pico8_probe_to_int(pico8_fixed_t x) {
    return pico8_fixed_flr_int(x);
}

static int pico8_probe_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

static pico8_fixed_t pico8_probe_field_fixed(const pico8_struct_header_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_entity_get_field(inst, name, &v) && v.kind == PICO8_VALUE_FIXED) {
        return v.as.fixed;
    }
    return pico8_fixed_from_int(0);
}

static int pico8_probe_field_truthy(const pico8_struct_header_t *inst, const char *name) {
    pico8_value_t v;
    if (inst == NULL || !pico8_entity_get_field(inst, name, &v)) {
        return 0;
    }
    return pico8_probe_truthy(v);
}

static pico8_vec_t pico8_probe_value_vec(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static pico8_vec_t pico8_probe_field_vec(const pico8_struct_header_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_entity_get_field(inst, name, &v)) {
        return pico8_probe_value_vec(v);
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static pico8_struct_header_t *pico8_probe_field_instance(const pico8_struct_header_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_entity_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return (pico8_struct_header_t *)v.as.ref;
    }
    return NULL;
}

typedef struct {
    pico8_vec_t self_p;
} pico8_bv_ctx_t;

static int pico8_bv_jc_predicate(pico8_value_t e, void *ctx_ptr) {
    const pico8_bv_ctx_t *ctx = (const pico8_bv_ctx_t *)ctx_ptr;
    pico8_struct_header_t *sw;
    pico8_struct_header_t *qu;
    pico8_struct_header_t *re;
    pico8_vec_t qu_p;
    pico8_vec_t re_p;

    if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
        return 0;
    }
    sw = (pico8_struct_header_t *)e.as.ref;

    qu = pico8_probe_field_instance(sw, "qu");
    re = pico8_probe_field_instance(sw, "re");
    qu_p = pico8_probe_field_vec(qu, "p");
    re_p = pico8_probe_field_vec(re, "p");

    return pico8_geom_be(qu_p, re_p, ctx->self_p, PICO8_FIXED_LIT(0x000000d1, 0.0032));
}

/* `hw` is read off the `ui` class, not off `ok`; no class reads as "the
 * point is not inside the panel". */
static int pico8_probe_ui_hw_call(pico8_vec_t rp) {
    const pico8_qz_box_t *hw = pico8_ui_hw();

    if (hw == NULL) {
        return 0;
    }
    return pico8_qz_call(hw, rp);
}

void pico8_bv(pico8_struct_header_t *self, pico8_value_t mp, pico8_value_t rp) {
    pico8_vec_t mp_vec;
    pico8_vec_t p_vec;
    pico8_vec_t *p_heap;
    pico8_fixed_t nq;
    pico8_value_t c;
    pico8_fixed_t dw;
    pico8_value_t et;

    if (self == NULL) {
        return;
    }
    assert(pico8_type_tag_of(self) == PICO8_TYPE_TAG_STRUCT &&
           "bv: self is a struct; no class instance reaches g_'s functions any more");

    pico8_entity_set_field(self, "ra", mp);
    mp_vec = pico8_probe_value_vec(mp);
    p_vec = pico8_vec_div(mp_vec, pico8_fixed_from_int(128));
    /* TROU-7 */
    p_heap = malloc(sizeof *p_heap);
    *p_heap = p_vec;
    pico8_entity_set_field(self, "p", pico8_value_ref(p_heap));

    c = pico8_value_ref(NULL);
    nq = pico8_ij(p_vec, pico8_cg("np"), &c);
    dw = pico8_probe_field_fixed(self, "dw");

    et = pico8_value_ref(NULL);

    if (nq < dw && pico8_probe_truthy(c)) {
        et = c;
    } else {
        pico8_bv_ctx_t ctx;
        pico8_list_t filtered;
        int have_first = 0;
        pico8_value_t first = pico8_value_ref(NULL);

        ctx.self_p = p_vec;
        filtered = pico8_jp(pico8_cg("jc"), pico8_bv_jc_predicate, &ctx);
        if (filtered.count > 0) {
            first = filtered.items[0];
            have_first = 1;
        }
        pico8_list_free(&filtered);

        if (have_first && pico8_probe_truthy(first)) {
            et = first;
        } else {
            pico8_vec_t rp_vec = pico8_probe_value_vec(rp);
            if (!pico8_probe_ui_hw_call(rp_vec)) {
                et = pico8_value_ref(&g_bv_et);
            } else {
                et = pico8_value_bool(0);
            }
        }
    }

    pico8_entity_set_field(self, "et", et);

    if (pico8_probe_truthy(et) && et.kind == PICO8_VALUE_REF && et.as.ref != NULL) {
        assert(pico8_type_tag_of(et.as.ref) == PICO8_TYPE_TAG_STRUCT &&
               "bv: et is an entity (a planet, a jc, the static throwaway struct), never a bare table");
        pico8_entity_set_field(et.as.ref, "fo", pico8_value_ref(self));
    }
}

pico8_value_t pico8_bv_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    pico8_bv((pico8_struct_header_t *)self, arg0, arg1);
    return pico8_value_ref(NULL);
}

void pico8_dl(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_struct_header_t *self) {
    pico8_value_t me_val;
    pico8_vec_t mp;
    const char *display = "";
    char *heap_display = NULL;
    pico8_value_t et_val;
    int has_et;

    if (self == NULL) {
        return;
    }
    assert(pico8_type_tag_of(self) == PICO8_TYPE_TAG_STRUCT &&
           "dl: self is a struct; no class instance reaches g_'s functions any more");

    if (!pico8_entity_get_field(self, "me", &me_val) || me_val.kind != PICO8_VALUE_STRING ||
        me_val.as.string == NULL || strcmp(me_val.as.string, "kz") != 0) {
        return;
    }

    mp = pico8_probe_field_vec(pico8_je(), "ra");

    has_et = pico8_entity_get_field(self, "et", &et_val) && pico8_probe_truthy(et_val);

    if (has_et) {
        const void *et_obj = (et_val.kind == PICO8_VALUE_REF) ? et_val.as.ref : NULL;
        pico8_value_t qs_val;
        if (et_obj != NULL && pico8_entity_get_field(et_obj, "qs", &qs_val) &&
            qs_val.kind == PICO8_VALUE_STRING && pico8_probe_truthy(qs_val)) {
            display = qs_val.as.string;
        } else {
            if (gfx != NULL && mem != NULL) {
                pico8_spr_default(gfx, mem, 31, pico8_probe_to_int(pico8_fixed_sub(mp.x, pico8_fixed_from_int(3))),
                                   pico8_probe_to_int(pico8_fixed_sub(mp.y, pico8_fixed_from_int(8))));
            }
        }
    } else {
        pico8_value_t od_val;
        if (pico8_entity_get_field(self, "od", &od_val) && pico8_probe_truthy(od_val)) {
            char *od_str = pico8_value_tostr(od_val);
            size_t len = strlen(od_str);
            heap_display = malloc(len + 2);
            memcpy(heap_display, od_str, len);
            heap_display[len] = '$';
            heap_display[len + 1] = '\0';
            free(od_str);
            display = heap_display;
        }
    }

    if (gfx != NULL && mem != NULL) {
        pico8_fixed_t a = PICO8_FIXED_LIT(0x00008000, 0.5);
        pico8_io(gfx, mem, display, mp.x, pico8_fixed_sub(mp.y, pico8_fixed_from_int(8)), 8, &a);
    }

    free(heap_display);
}

pico8_value_t pico8_dl_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_dl(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_struct_header_t *)self);
    return pico8_value_ref(NULL);
}

pico8_value_t pico8_d_(pico8_struct_header_t *self) {
    pico8_value_t et_val;
    int has_et;
    pico8_value_t od_val = pico8_value_ref(NULL);
    pico8_hy_t *fm;
    int lu_ok = 0;

    if (self == NULL) {
        return pico8_value_ref(NULL);
    }
    assert(pico8_type_tag_of(self) == PICO8_TYPE_TAG_STRUCT &&
           "d_: self is a struct; no class instance reaches g_'s functions any more");

    has_et = pico8_entity_get_field(self, "et", &et_val) && pico8_probe_truthy(et_val);

    fm = pico8_fm_hy();
    if (!has_et) {
        pico8_fixed_t jt = pico8_fixed_from_int(0);
        (void)pico8_entity_get_field(self, "od", &od_val);
        if (od_val.kind == PICO8_VALUE_FIXED) {
            jt = od_val.as.fixed;
        }
        lu_ok = pico8_hy_lu(fm, jt);
    }

    assert((!has_et || !lu_ok) && "d_: fm:lu must stay short-circuited when et is set");

    if (!has_et && lu_ok) {
        pico8_value_t dq_val;
        pico8_value_t fp_val;
        pico8_value_t ik_val;
        pico8_fixed_t hx = pico8_fixed_from_int(0);

        if (!pico8_entity_get_field(self, "dq", &dq_val) || !pico8_probe_truthy(dq_val)) {
            dq_val = pico8_value_string("ph");
        }
        pico8_ke_ks(self, dq_val);

        if (!pico8_entity_get_field(self, "fp", &fp_val) || !pico8_probe_truthy(fp_val)) {
            fp_val = pico8_value_fixed(pico8_fixed_from_int(0));
        }
        if (fp_val.kind == PICO8_VALUE_FIXED) {
            hx = fp_val.as.fixed;
        }
        pico8_hy_nx(pico8_fm_hy(), hx);

        if (!pico8_entity_get_field(self, "ik", &ik_val) || !pico8_probe_truthy(ik_val)) {
            ik_val = pico8_value_fixed(pico8_fixed_from_int(54));
        }
        pico8_rn(ik_val);

        return pico8_value_bool(1);
    }

    pico8_rn(pico8_value_fixed(pico8_fixed_from_int(57)));
    pico8_entity_set_field(self, "p_", pico8_value_bool(1));
    return pico8_value_ref(NULL);
}

pico8_value_t pico8_d__dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_d_((pico8_struct_header_t *)self);
}

pico8_value_t pico8_g__mm(void *self_ptr, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_struct_header_t *self = (pico8_struct_header_t *)self_ptr;
    pico8_fixed_t pn, t, du, py, ki;
    pico8_fixed_t a, rnd_pn, rnd_half;
    pico8_vec_t p, v;
    pico8_ob_table_t *particle;
    pico8_vec_t *p_heap;
    pico8_vec_t *v_heap;

    (void)argc;
    (void)arg0;
    (void)arg1;
    assert((self == NULL || pico8_type_tag_of(self) == PICO8_TYPE_TAG_STRUCT) &&
           "g_.mm: self is a struct; no class instance reaches g_'s functions any more");

    pn = pico8_probe_field_fixed(self, "pn");
    t = pico8_probe_field_fixed(self, "t");
    du = pico8_probe_field_fixed(self, "du");
    py = pico8_probe_field_fixed(self, "py");
    ki = pico8_probe_field_fixed(self, "ki");

    /* Two statements, not two arguments: C leaves argument order unspecified
     * (gcc drew rnd(0.5) first), Lua draws rnd(self.pn) first. */
    rnd_pn = pico8_fixed_rnd(pn);
    rnd_half = pico8_fixed_rnd(PICO8_FIXED_LIT(0x00008000, 0.5));
    a = pico8_fixed_add(
        pico8_fixed_div(pico8_fixed_add(pico8_fixed_flr(rnd_pn), rnd_half), pn),
        pico8_fixed_mul(t, PICO8_FIXED_LIT(0x00000041, 0.001)));

    p = pico8_vec_qm(pico8_fixed_add(du, pico8_fixed_rnd_default()), a);
    v = pico8_vec_mul(p, py);

    if (pico8_probe_field_truthy(self, "pk")) {
        v = pico8_vec_pa(v);
    }

    /* TROU-7 */
    particle = pico8_ob_table_new();
    p_heap = malloc(sizeof *p_heap);
    *p_heap = p;
    v_heap = malloc(sizeof *v_heap);
    *v_heap = v;
    pico8_ob_table_set_name(particle, "p", pico8_value_ref(p_heap));
    pico8_ob_table_set_name(particle, "v", pico8_value_ref(v_heap));
    pico8_ob_table_set_name(particle, "l", pico8_value_fixed(ki));

    return pico8_value_ref(particle);
}

pico8_value_t pico8_g__et(const pico8_g__t *self) {
    if (self == NULL || !self->has_et) {
        return pico8_value_ref(NULL);
    }
    if (self->et == NULL) {
        return pico8_value_bool(0);
    }
    return pico8_value_ref(self->et);
}

static int g__ref_field(const void *ref, pico8_value_t *out_value) {
    if (ref == NULL) {
        return 0;
    }
    *out_value = pico8_value_ref((void *)ref);
    return 1;
}

static int g__fixed_field(pico8_fixed_t x, pico8_value_t *out_value) {
    *out_value = pico8_value_fixed(x);
    return 1;
}

static int pico8_g__field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_g__t *g_ = self;

    if (strcmp(name, "nu") == 0) {
        return g__ref_field(g_->nu, out_value);
    }
    if (strcmp(name, "fx") == 0) {
        return g__fixed_field(g_->fx, out_value);
    }
    if (strcmp(name, "dq") == 0) {
        if (g_->dq == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(g_->dq);
        return 1;
    }
    if (strcmp(name, "od") == 0) {
        return g__fixed_field(g_->od, out_value);
    }
    if (strcmp(name, "fp") == 0) {
        *out_value = pico8_value_bool(g_->fp);
        return 1;
    }
    if (strcmp(name, "fa") == 0) {
        return g__fixed_field(g_->fa, out_value);
    }
    if (strcmp(name, "dw") == 0) {
        return g__fixed_field(g_->dw, out_value);
    }
    if (strcmp(name, "py") == 0) {
        return g__fixed_field(g_->py, out_value);
    }
    if (strcmp(name, "ki") == 0) {
        return g__fixed_field(g_->ki, out_value);
    }
    if (strcmp(name, "pn") == 0) {
        return g__fixed_field(g_->pn, out_value);
    }
    if (strcmp(name, "pk") == 0) {
        return g__fixed_field(g_->pk, out_value);
    }
    if (strcmp(name, "fc") == 0) {
        return g__fixed_field(g_->fc, out_value);
    }
    if (strcmp(name, "k_") == 0) {
        return g__ref_field(g_->k_, out_value);
    }
    if (strcmp(name, "qo") == 0) {
        return g__fixed_field(g_->qo, out_value);
    }
    if (strcmp(name, "ro") == 0) {
        return g__fixed_field(g_->ro, out_value);
    }
    if (strcmp(name, "qg") == 0) {
        return g__fixed_field(g_->qg, out_value);
    }
    if (strcmp(name, "q_") == 0) {
        return g__fixed_field(g_->q_, out_value);
    }
    if (strcmp(name, "du") == 0) {
        return g__fixed_field(g_->du, out_value);
    }
    if (strcmp(name, "qy") == 0) {
        return g__ref_field(g_->qy, out_value);
    }
    if (strcmp(name, "ra") == 0) {
        return g__ref_field(g_->ra, out_value);
    }
    if (strcmp(name, "p") == 0) {
        return g__ref_field(g_->p, out_value);
    }
    if (strcmp(name, "et") == 0) {
        pico8_value_t et = pico8_g__et(g_);
        if (et.kind == PICO8_VALUE_REF && et.as.ref == NULL) {
            return 0;
        }
        *out_value = et;
        return 1;
    }
    if (strcmp(name, "p_") == 0) {
        if (!g_->has_p_) {
            return 0;
        }
        *out_value = pico8_value_bool(g_->p_);
        return 1;
    }
    return pico8_ke_field(self, name, out_value);
}

static void g__take_list(pico8_value_t value, pico8_list_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && value.as.ref != NULL &&
           pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_LIST &&
           "g_.qy: a write neither nil nor a list does not fit the member's type -- stop, do not widen the member");
    *out = value.as.ref;
}

static void g__take_vec(pico8_value_t value, pico8_vec_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && value.as.ref != NULL &&
           pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_UNKNOWN &&
           "g_.ra/p: a write neither nil nor a vector ref does not fit the member's type "
           "-- stop, do not widen the member");
    *out = value.as.ref;
}

/* What `bv` aims at: `cg.np` (jo, and qh's tree: exactly the descriptors
 * holding `ly`), a `jc`, or the throwaway target. */
static int g__is_target_class(const pico8_struct_class_t *cls) {
    return cls->methods[PICO8_METHOD_LY] != NULL || cls == pico8_jo_struct_class() ||
           cls == pico8_jc_struct_class() || cls == pico8_bv_et_struct_class();
}

static void g__take_target(pico8_value_t value, pico8_struct_header_t **out, int *has) {
    *out = NULL;
    *has = 0;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(((value.kind == PICO8_VALUE_BOOL && value.as.boolean == 0) ||
            (value.kind == PICO8_VALUE_REF && pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_STRUCT &&
             g__is_target_class(((const pico8_struct_header_t *)value.as.ref)->cls))) &&
           "g_.et: a write neither nil, false, nor what bv aims at (a planet jo, qh, nl, os, hj, pm, nr, fr, mx, a jc, "
           "or bv's throwaway target) does not fit the member's type -- stop, do not widen the member");
    *out = value.kind == PICO8_VALUE_REF ? value.as.ref : NULL;
    *has = 1;
}

static void g__take_bool(pico8_value_t value, int *out, int *has) {
    *out = 0;
    *has = 0;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_BOOL &&
           "g_.p_: a write neither nil nor a boolean does not fit the member's type -- stop, do not widen the member");
    *out = value.as.boolean != 0;
    *has = 1;
}

/* `qy` (`f_`), `ra`, `p`, `et` (`bv`), `p_` (`d_`); 0 for any other. */
static int pico8_g__set_field(void *self, const char *name, pico8_value_t value) {
    pico8_g__t *g_ = self;

    if (strcmp(name, "qy") == 0) {
        g__take_list(value, &g_->qy);
        return 1;
    }
    if (strcmp(name, "ra") == 0) {
        pico8_vec_t *ra;

        g__take_vec(value, &ra);
        g_->ra = ra;
        return 1;
    }
    if (strcmp(name, "p") == 0) {
        g__take_vec(value, &g_->p);
        return 1;
    }
    if (strcmp(name, "et") == 0) {
        g__take_target(value, &g_->et, &g_->has_et);
        return 1;
    }
    if (strcmp(name, "p_") == 0) {
        g__take_bool(value, &g_->p_, &g_->has_p_);
        return 1;
    }
    return 0;
}

static const pico8_struct_class_t g_g__struct_class = {
    {
        [PICO8_METHOD_HQ] = pico8_bv_dispatch,
        [PICO8_METHOD_EY] = pico8_d__dispatch,
        [PICO8_METHOD_EV] = pico8_dl_dispatch,
        [PICO8_METHOD_KJ] = pico8_ei_dispatch,
        [PICO8_METHOD_MM] = pico8_g__mm,
    },
    pico8_g__field,
    pico8_g__set_field,
};

void pico8_g__struct_init(pico8_g__t *self, const pico8_struct_class_t *cls) {
    pico8_ke_struct_init(&self->ke, cls);
    self->qy = NULL;
    self->ra = NULL;
    self->p = NULL;
    self->et = NULL;
    self->has_et = 0;
    self->p_ = 0;
    self->has_p_ = 0;
    self->nu = pico8_g__start.nu;
    self->fx = pico8_g__start.fx;
    self->dq = pico8_g__start.dq;
    self->od = pico8_g__start.od;
    self->fp = pico8_g__start.fp;
    self->fa = pico8_g__start.fa;
    self->dw = pico8_g__start.dw;
    self->py = pico8_g__start.py;
    self->ki = pico8_g__start.ki;
    self->pn = pico8_g__start.pn;
    self->pk = pico8_g__start.pk;
    self->fc = pico8_g__start.fc;
    self->k_ = pico8_g__start.k_;
    self->qo = pico8_g__start.qo;
    self->ro = pico8_g__start.ro;
    self->qg = pico8_g__start.qg;
    self->q_ = pico8_g__start.q_;
    self->du = pico8_g__start.du;
}

static void g__take_fixed(pico8_value_t value, pico8_fixed_t *out) {
    assert(value.kind == PICO8_VALUE_FIXED &&
           "g_.ke.t/pn/du/py: a construction key that is not a fixed does not fit the member's type "
           "-- stop, do not widen the member");
    *out = value.as.fixed;
}

static void *pico8_g__struct_new(const pico8_ob_table_t *overrides) {
    pico8_g__t *self;
    int k;

    assert(g_g__up && "pico8_probe_init() must run first");
    self = malloc(sizeof *self);
    pico8_g__struct_init(self, &g_g__struct_class);
    for (k = 0; overrides != NULL && k < overrides->count; k++) {
        pico8_ob_key_t key = overrides->entries[k].key;
        pico8_value_t value = overrides->entries[k].value;

        if (key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "t") == 0) {
            g__take_fixed(value, &self->ke.t);
        } else if (key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "pn") == 0) {
            g__take_fixed(value, &self->pn);
        } else if (key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "du") == 0) {
            g__take_fixed(value, &self->du);
        } else if (key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "py") == 0) {
            g__take_fixed(value, &self->py);
        } else {
            assert(0 && "g_: the only construction keys are pn, t, du, py (probe_mm_rnd_order); another key has no "
                        "member -- stop, do not add one");
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("g_");
#endif
    pico8_f_((pico8_struct_header_t *)self);
    pico8_ke_struct_pq(&self->ke);
    return self;
}

void pico8_probe_init(void) {
    pico8_probe_shutdown();

    g_g__up = 1;

    g_fm_placeholder.jk = 0;
    g_fm_placeholder.has_jk = 0;
    g_fm_placeholder.nz = 0;
    g_fm_placeholder.has_nz = 0;
    g_fm = (pico8_struct_header_t *)&g_fm_placeholder;
}

static void fm_free(pico8_struct_header_t *fm) {
    if (fm == NULL || fm == (pico8_struct_header_t *)&g_fm_placeholder) {
        return;
    }
    assert(((const pico8_struct_header_t *)fm)->cls == pico8_hy_struct_class() &&
           "fm: pico8_probe_set_fm only ever stores a hy besides the provisional");
    free(fm);
}

void pico8_probe_shutdown(void) {
    g_g__up = 0;
    fm_free(g_fm);
    g_fm = NULL;
}

int pico8_g__ready(void) {
    return g_g__up;
}

const pico8_struct_class_t *pico8_g__struct_class(void) {
    return &g_g__struct_class;
}

pico8_g__t *pico8_g__new(const pico8_ob_table_t *overrides) {
    return pico8_g__struct_new(overrides);
}

const pico8_struct_class_t *pico8_fm_placeholder_struct_class(void) {
    return &g_fm_placeholder_struct_class;
}

pico8_struct_header_t *pico8_fm(void) {
    return g_fm;
}

pico8_hy_t *pico8_fm_hy(void) {
    if (g_fm == NULL) {
        return NULL;
    }
    assert(pico8_type_tag_of(g_fm) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)g_fm)->cls == pico8_hy_struct_class() &&
           "a hy function on the provisional fm calls through a nil fm in the cart");
    return (pico8_hy_t *)g_fm;
}

void pico8_probe_set_fm(pico8_struct_header_t *instance) {
    if (g_fm == instance) {
        return;
    }
    fm_free(g_fm);
    g_fm = instance;
}

const pico8_struct_class_t *pico8_bv_et_struct_class(void) {
    return &g_bv_et_struct_class;
}
