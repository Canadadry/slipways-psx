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
    if (inst == NULL || !pico8_instance_get_field(inst, name, &v)) {
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

/* `hw` is read off the `ui` class defaults, not off `ok`; an absent class or
 * an absent/wrong-kind box reads as "the point is not inside the panel". */
static int pico8_probe_ui_hw_call(pico8_vec_t rp) {
    pico8_class_t *ui_class = pico8_ui_class();
    pico8_value_t hw_val;

    if (ui_class == NULL || ui_class->defaults == NULL) {
        return 0;
    }
    if (!pico8_ob_table_get_name(ui_class->defaults, "hw", &hw_val) || hw_val.kind != PICO8_VALUE_REF ||
        hw_val.as.ref == NULL) {
        return 0;
    }
    return pico8_qz_call((const pico8_qz_box_t *)hw_val.as.ref, rp);
}

void pico8_bv(pico8_instance_t *self, pico8_value_t mp, pico8_value_t rp) {
    pico8_vec_t mp_vec;
    pico8_vec_t p_vec;
    pico8_vec_t *p_heap;
    pico8_fixed_t nq;
    pico8_value_t c;
    pico8_fixed_t dw;
    pico8_value_t et;

    if (self == NULL || (pico8_type_tag_of(self) == PICO8_TYPE_TAG_INSTANCE && self->fields == NULL)) {
        return;
    }

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
                /* TROU-7 */
                pico8_instance_t *empty = malloc(sizeof *empty);
                empty->tag = PICO8_TYPE_TAG_INSTANCE;
                empty->fields = pico8_ob_table_new();
                empty->cls = NULL;
                et = pico8_value_ref(empty);
            } else {
                et = pico8_value_bool(0);
            }
        }
    }

    pico8_entity_set_field(self, "et", et);

    if (pico8_probe_truthy(et) && et.kind == PICO8_VALUE_REF && et.as.ref != NULL) {
        assert((pico8_type_tag_of(et.as.ref) == PICO8_TYPE_TAG_INSTANCE ||
                pico8_type_tag_of(et.as.ref) == PICO8_TYPE_TAG_STRUCT) &&
               "bv: et is an entity (a planet or class-less throwaway instance, a jc struct), never a bare table");
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
    pico8_instance_t *fm;
    int lu_ok = 0;

    if (self == NULL) {
        return pico8_value_ref(NULL);
    }

    has_et = pico8_entity_get_field(self, "et", &et_val) && pico8_probe_truthy(et_val);

    fm = pico8_fm();
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
        pico8_hy_nx(pico8_fm(), hx);

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

void pico8_probe_init(void) {
    pico8_probe_shutdown();

    g_probe_class = pico8_class_declare(
        pico8_ke_class(),
        "\nnu=o(\"g_\",\"prober\"),\nfx=1,\ndq=\"ly\",\nod=0,fp=f,\nfa=2,\ndw=0.025,\npy=0.06,ki=9,\npn=3,pk=1,\nfc=1,"
        "\nk_=o(1,1,5,13,6),\nqo=1.02,ro=-0.0525,\nqg=0.0525,q_=1.02,\ndu=2,\n");
    PICO8_CLASS_COVERAGE_NAME(g_probe_class, "g_");
    pico8_class_set_pq(g_probe_class, pico8_f_);
    pico8_ob_table_set_name(g_probe_class->defaults, "hq", pico8_value_function(pico8_bv_dispatch));
    pico8_ob_table_set_name(g_probe_class->defaults, "ey", pico8_value_function(pico8_d__dispatch));
    pico8_ob_table_set_name(g_probe_class->defaults, "ev", pico8_value_function(pico8_dl_dispatch));
    pico8_ob_table_set_name(g_probe_class->defaults, "kj", pico8_value_function(pico8_ei_dispatch));
    pico8_ob_table_set_name(g_probe_class->defaults, "mm", pico8_value_function(pico8_g__mm));
    pico8_class_register_named("g_", g_probe_class);

    g_fm = malloc(sizeof *g_fm);
    g_fm->tag = PICO8_TYPE_TAG_INSTANCE;
    g_fm->fields = pico8_ob_table_new();
    g_fm->cls = NULL;
}

void pico8_probe_shutdown(void) {
    if (g_probe_class != NULL) {
        pico8_class_free(g_probe_class);
        g_probe_class = NULL;
    }
    if (g_fm != NULL) {
        pico8_instance_free(g_fm);
        g_fm = NULL;
    }
}

pico8_class_t *pico8_g__class(void) {
    return g_probe_class;
}

pico8_instance_t *pico8_g__new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_probe_class, overrides);
}

pico8_instance_t *pico8_fm(void) {
    return g_fm;
}

void pico8_probe_set_fm(pico8_instance_t *instance) {
    if (g_fm == instance) {
        return;
    }
    pico8_instance_free(g_fm);
    g_fm = instance;
}
