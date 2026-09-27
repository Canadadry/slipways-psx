#include "pico8_probe.h"

#include <assert.h>
#include <stddef.h>
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
#include "pico8_ui.h"

static pico8_class_t *g_probe_class = NULL;
static pico8_instance_t *g_fm = NULL;

static int pico8_fm_placeholder_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_fm_placeholder_t *fm = self;
    const pico8_value_t *member = NULL;

    if (strcmp(name, "jk") == 0) {
        member = &fm->jk;
    } else if (strcmp(name, "nz") == 0) {
        member = &fm->nz;
    }
    if (member == NULL || (member->kind == PICO8_VALUE_REF && member->as.ref == NULL)) {
        return 0;
    }
    *out_value = *member;
    return 1;
}

static const pico8_struct_class_t g_fm_placeholder_struct_class = {
    {NULL},
    pico8_fm_placeholder_field,
    NULL,
};

static pico8_fm_placeholder_t g_fm_placeholder = {
    {PICO8_TYPE_TAG_STRUCT, &g_fm_placeholder_struct_class}, {PICO8_VALUE_REF, {0}}, {PICO8_VALUE_REF, {0}}};

typedef struct {
    pico8_struct_header_t header;
    pico8_value_t fo;
} pico8_bv_et_t;

static int pico8_bv_et_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_bv_et_t *et = self;

    if (strcmp(name, "fo") != 0 || (et->fo.kind == PICO8_VALUE_REF && et->fo.as.ref == NULL)) {
        return 0;
    }
    *out_value = et->fo;
    return 1;
}

static int pico8_bv_et_set_field(void *self, const char *name, pico8_value_t value) {
    pico8_bv_et_t *et = self;

    if (strcmp(name, "fo") != 0) {
        return 0;
    }
    et->fo = value;
    return 1;
}

static const pico8_struct_class_t g_bv_et_struct_class = {
    {NULL},
    pico8_bv_et_field,
    pico8_bv_et_set_field,
};

/* The `{}` `bv` puts in `et` over the void, off the panel: never allocated,
 * never freed; only `fo` is written, and the next `bv` overwrites it. */
static pico8_bv_et_t g_bv_et = {{PICO8_TYPE_TAG_STRUCT, &g_bv_et_struct_class}, {PICO8_VALUE_REF, {0}}};

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

static pico8_fixed_t pico8_probe_field_fixed(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_entity_get_field(inst, name, &v) && v.kind == PICO8_VALUE_FIXED) {
        return v.as.fixed;
    }
    return pico8_fixed_from_int(0);
}

static int pico8_probe_field_truthy(const pico8_instance_t *inst, const char *name) {
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

static pico8_vec_t pico8_probe_field_vec(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_entity_get_field(inst, name, &v)) {
        return pico8_probe_value_vec(v);
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static pico8_instance_t *pico8_probe_field_instance(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_entity_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return (pico8_instance_t *)v.as.ref;
    }
    return NULL;
}

typedef struct {
    pico8_vec_t self_p;
} pico8_bv_ctx_t;

static int pico8_bv_jc_predicate(pico8_value_t e, void *ctx_ptr) {
    const pico8_bv_ctx_t *ctx = (const pico8_bv_ctx_t *)ctx_ptr;
    pico8_instance_t *sw;
    pico8_instance_t *qu;
    pico8_instance_t *re;
    pico8_vec_t qu_p;
    pico8_vec_t re_p;

    if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
        return 0;
    }
    sw = (pico8_instance_t *)e.as.ref;

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

void pico8_bv(pico8_instance_t *self, pico8_value_t mp, pico8_value_t rp) {
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
        assert((pico8_type_tag_of(et.as.ref) == PICO8_TYPE_TAG_INSTANCE ||
                pico8_type_tag_of(et.as.ref) == PICO8_TYPE_TAG_STRUCT) &&
               "bv: et is an entity (a planet, a jc, the static throwaway struct), never a bare table");
        pico8_entity_set_field(et.as.ref, "fo", pico8_value_ref(self));
    }
}

pico8_value_t pico8_bv_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    pico8_bv((pico8_instance_t *)self, arg0, arg1);
    return pico8_value_ref(NULL);
}

void pico8_dl(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self) {
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
    pico8_dl(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

pico8_value_t pico8_d_(pico8_instance_t *self) {
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
    return pico8_d_((pico8_instance_t *)self);
}

pico8_value_t pico8_g__mm(void *self_ptr, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_instance_t *self = (pico8_instance_t *)self_ptr;
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

/* g_'s 18 spec defaults, read once from its class at init: the very values
 * its `defaults` holds, `nu` and `k_` included. */
static struct {
    const char *name;
    size_t offset;
    pico8_value_t value;
} g_g__defaults[] = {
    {"nu", offsetof(pico8_g__t, nu), {0}}, {"fx", offsetof(pico8_g__t, fx), {0}}, {"dq", offsetof(pico8_g__t, dq), {0}},
    {"od", offsetof(pico8_g__t, od), {0}}, {"fp", offsetof(pico8_g__t, fp), {0}}, {"fa", offsetof(pico8_g__t, fa), {0}},
    {"dw", offsetof(pico8_g__t, dw), {0}}, {"py", offsetof(pico8_g__t, py), {0}}, {"ki", offsetof(pico8_g__t, ki), {0}},
    {"pn", offsetof(pico8_g__t, pn), {0}}, {"pk", offsetof(pico8_g__t, pk), {0}}, {"fc", offsetof(pico8_g__t, fc), {0}},
    {"k_", offsetof(pico8_g__t, k_), {0}}, {"qo", offsetof(pico8_g__t, qo), {0}}, {"ro", offsetof(pico8_g__t, ro), {0}},
    {"qg", offsetof(pico8_g__t, qg), {0}}, {"q_", offsetof(pico8_g__t, q_), {0}}, {"du", offsetof(pico8_g__t, du), {0}},
};

static void pico8_g__defaults_reset(void) {
    size_t i;
    for (i = 0; i < sizeof g_g__defaults / sizeof g_g__defaults[0]; i++) {
        g_g__defaults[i].value = pico8_value_ref(NULL);
    }
}

static const struct {
    const char *name;
    size_t offset;
} g_g__members[] = {
    {"nu", offsetof(pico8_g__t, nu)}, {"fx", offsetof(pico8_g__t, fx)}, {"dq", offsetof(pico8_g__t, dq)},
    {"od", offsetof(pico8_g__t, od)}, {"fp", offsetof(pico8_g__t, fp)}, {"fa", offsetof(pico8_g__t, fa)},
    {"dw", offsetof(pico8_g__t, dw)}, {"py", offsetof(pico8_g__t, py)}, {"ki", offsetof(pico8_g__t, ki)},
    {"pn", offsetof(pico8_g__t, pn)}, {"pk", offsetof(pico8_g__t, pk)}, {"fc", offsetof(pico8_g__t, fc)},
    {"k_", offsetof(pico8_g__t, k_)}, {"qo", offsetof(pico8_g__t, qo)}, {"ro", offsetof(pico8_g__t, ro)},
    {"qg", offsetof(pico8_g__t, qg)}, {"q_", offsetof(pico8_g__t, q_)}, {"du", offsetof(pico8_g__t, du)},
    {"qy", offsetof(pico8_g__t, qy)}, {"ra", offsetof(pico8_g__t, ra)}, {"p", offsetof(pico8_g__t, p)},
    {"et", offsetof(pico8_g__t, et)}, {"p_", offsetof(pico8_g__t, p_)},
};

static int pico8_g__field(const void *self, const char *name, pico8_value_t *out_value) {
    size_t i;

    for (i = 0; i < sizeof g_g__members / sizeof g_g__members[0]; i++) {
        if (strcmp(name, g_g__members[i].name) == 0) {
            const pico8_value_t *member = (const pico8_value_t *)((const char *)self + g_g__members[i].offset);
            if (member->kind == PICO8_VALUE_REF && member->as.ref == NULL) {
                return 0;
            }
            *out_value = *member;
            return 1;
        }
    }
    return pico8_ke_field(self, name, out_value);
}

/* `qy` (`f_`), `ra`, `p`, `et` (`bv`), `p_` (`d_`); 0 for any other. */
static int pico8_g__set_field(void *self, const char *name, pico8_value_t value) {
    pico8_g__t *g_ = self;

    if (strcmp(name, "qy") == 0) {
        g_->qy = value;
    } else if (strcmp(name, "ra") == 0) {
        g_->ra = value;
    } else if (strcmp(name, "p") == 0) {
        g_->p = value;
    } else if (strcmp(name, "et") == 0) {
        g_->et = value;
    } else if (strcmp(name, "p_") == 0) {
        g_->p_ = value;
    } else {
        return 0;
    }
    return 1;
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
    size_t i;

    pico8_ke_struct_init(&self->ke, cls);
    self->qy = pico8_value_ref(NULL);
    self->ra = pico8_value_ref(NULL);
    self->p = pico8_value_ref(NULL);
    self->et = pico8_value_ref(NULL);
    self->p_ = pico8_value_ref(NULL);
    for (i = 0; i < sizeof g_g__defaults / sizeof g_g__defaults[0]; i++) {
        *(pico8_value_t *)((char *)self + g_g__defaults[i].offset) = g_g__defaults[i].value;
    }
}

static void *pico8_g__struct_new(const pico8_ob_table_t *overrides) {
    static const struct {
        const char *name;
        size_t offset;
    } keys[] = {
        {"pn", offsetof(pico8_g__t, pn)},
        {"t", offsetof(pico8_g__t, ke.t)},
        {"du", offsetof(pico8_g__t, du)},
        {"py", offsetof(pico8_g__t, py)},
    };
    pico8_g__t *self;
    size_t i;
    int k;

    assert(g_probe_class != NULL && "pico8_probe_init() must run first");
    self = malloc(sizeof *self);
    pico8_g__struct_init(self, &g_g__struct_class);
    for (k = 0; overrides != NULL && k < overrides->count; k++) {
        pico8_ob_key_t key = overrides->entries[k].key;
        size_t found = sizeof keys / sizeof keys[0];

        for (i = 0; key.kind == PICO8_OB_KEY_NAME && i < sizeof keys / sizeof keys[0]; i++) {
            if (strcmp(key.name, keys[i].name) == 0) {
                found = i;
            }
        }
        assert(found < sizeof keys / sizeof keys[0] &&
               "g_: the only measured construction keys are pn, t, du, py (probe_mm_rnd_order); another would be a "
               "field under no rule of plan-3 section 5 -- stop, do not add a member");
        *(pico8_value_t *)((char *)self + keys[found].offset) = overrides->entries[k].value;
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("g_");
#endif
    pico8_f_((pico8_instance_t *)self);
    pico8_ke_struct_pq(&self->ke);
    return self;
}

void pico8_probe_init(void) {
    size_t i;

    pico8_probe_shutdown();

    g_probe_class = pico8_class_declare(
        pico8_ke_class(),
        "\nnu=o(\"g_\",\"prober\"),\nfx=1,\ndq=\"ly\",\nod=0,fp=f,\nfa=2,\ndw=0.025,\npy=0.06,ki=9,\npn=3,pk=1,\nfc=1,"
        "\nk_=o(1,1,5,13,6),\nqo=1.02,ro=-0.0525,\nqg=0.0525,q_=1.02,\ndu=2,\n");
    PICO8_CLASS_COVERAGE_NAME(g_probe_class, "g_");
    pico8_class_set_struct_new(g_probe_class, pico8_g__struct_new);
    for (i = 0; i < sizeof g_g__defaults / sizeof g_g__defaults[0]; i++) {
        int found = pico8_class_get_field(g_probe_class, g_g__defaults[i].name, &g_g__defaults[i].value);
        assert(found && "g_: every default of the struct is a key of g_'s spec");
        (void)found;
    }

    g_fm_placeholder.jk = pico8_value_ref(NULL);
    g_fm_placeholder.nz = pico8_value_ref(NULL);
    g_fm = (pico8_instance_t *)&g_fm_placeholder;
}

static void fm_free(pico8_instance_t *fm) {
    if (fm == NULL || fm == (pico8_instance_t *)&g_fm_placeholder) {
        return;
    }
    assert(((const pico8_struct_header_t *)fm)->cls == pico8_hy_struct_class() &&
           "fm: pico8_probe_set_fm only ever stores a hy besides the provisional");
    free(fm);
}

void pico8_probe_shutdown(void) {
    pico8_g__defaults_reset();
    if (g_probe_class != NULL) {
        pico8_class_free(g_probe_class);
        g_probe_class = NULL;
    }
    fm_free(g_fm);
    g_fm = NULL;
}

pico8_class_t *pico8_g__class(void) {
    return g_probe_class;
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

pico8_instance_t *pico8_fm(void) {
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

void pico8_probe_set_fm(pico8_instance_t *instance) {
    if (g_fm == instance) {
        return;
    }
    fm_free(g_fm);
    g_fm = instance;
}

const pico8_struct_class_t *pico8_bv_et_struct_class(void) {
    return &g_bv_et_struct_class;
}
