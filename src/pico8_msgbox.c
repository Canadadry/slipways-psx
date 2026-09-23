#include "pico8_msgbox.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "pico8_audio.h"
#include "pico8_collect.h"
#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_fixed.h"
#include "pico8_hud.h"
#include "pico8_ke.h"
#include "pico8_pi.h"
#include "pico8_ui.h"
#include "pico8_ui_button.h"

static pico8_class_t *g_mh_class = NULL;
static pico8_class_t *g_iv_class = NULL;
static pico8_instance_t *g_kc = NULL;

pico8_instance_t *pico8_kc(void) {
    return g_kc;
}

void pico8_msgbox_set_kc(pico8_instance_t *instance) {
    g_kc = instance;
}

/* Lua truthiness : seuls nil et false sont faux. La clause REF-NULL n'est pas
 * optionnelle — ce port modélise le nil de Lua par ref(NULL), donc un test qui
 * ne rejette que BOOL false rend nil vrai. C'est ce qui faisait dessiner une
 * rangée fantôme en sprite 0 sur une planète construite. */
static int pico8_msgbox_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

/* Field readers: an absent field, a wrong kind, or a NULL instance all read
 * as the zero default. */
static pico8_fixed_t pico8_msgbox_field_fixed(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v) && v.kind == PICO8_VALUE_FIXED) {
        return v.as.fixed;
    }
    return pico8_fixed_from_int(0);
}

static pico8_vec_t pico8_msgbox_field_vec(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_instance_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

/* `all(self)` over an instance: Lua walks the array part, so `bl(self,nc)`
 * sees the positional entries only. Returns a fresh list the caller must
 * release with pico8_list_free(). */
static pico8_list_t pico8_msgbox_positional_values(const pico8_ob_table_t *table) {
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

void pico8_mh_pq(pico8_instance_t *self) {
    pico8_instance_t *previous;

    if (self == NULL) {
        return;
    }

    previous = pico8_kc();
    if (previous != NULL) {
        pico8_ob_table_set_name(previous->fields, "p_", pico8_value_bool(1));
    }
    pico8_msgbox_set_kc(self);
}

static pico8_value_t pico8_mh_ev_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_mh_ev(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

void pico8_mh_ev(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self) {
    pico8_fixed_t self_t, rh, t, nc, t3;
    pico8_value_t lh_result;
    int lh_found;

    if (self == NULL) {
        return;
    }

    self_t = pico8_msgbox_field_fixed(self, "t");
    rh = pico8_msgbox_field_fixed(self, "rh");
    t = pico8_fixed_sub(pico8_fixed_div(self_t, rh), pico8_fixed_from_int(10));

    if (t >= 0) {
        lh_found = pico8_mv(self, "lh", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &lh_result);
        if (lh_found && pico8_msgbox_truthy(lh_result)) {
            pico8_fixed_t new_t = pico8_fixed_mul(rh, pico8_fixed_from_int(10));
            pico8_ob_table_set_name(self->fields, "t", pico8_value_fixed(new_t));
            t = pico8_fixed_from_int(0);
        }
    }

    /* `-t^3/9` is `(-(t^3))/9`: in Lua `^` binds tighter than unary `-`,
     * which in turn binds tighter than `/`. */
    t3 = pico8_fixed_mul(pico8_fixed_mul(t, t), t);
    nc = pico8_fixed_div(pico8_fixed_neg(t3), pico8_fixed_from_int(9));

    if (gfx != NULL && mem != NULL) {
        pico8_list_t items = pico8_msgbox_positional_values(self->fields);
        pico8_bl(gfx, mem, &items, nc);
        pico8_list_free(&items);
        pico8_camera_reset(gfx);
    }

    if (t > pico8_fixed_from_double(10.5)) {
        pico8_ob_table_set_name(self->fields, "p_", pico8_value_bool(1));
        pico8_msgbox_set_kc(NULL);
        assert(pico8_kc() == NULL && "`self.p_,kc=true` is one value for two targets: kc gets nil, not true");
    }
}

void pico8_mh_init(void) {
    pico8_mh_shutdown();
    g_mh_class = pico8_class_declare(pico8_ke_class(), "\nnu=o(\"mh\"),\nfa=14,\nrh=5,\n");
    pico8_class_set_pq(g_mh_class, pico8_mh_pq);
    pico8_ob_table_set_name(g_mh_class->defaults, "ev", pico8_value_function(pico8_mh_ev_dispatch));
}

void pico8_mh_shutdown(void) {
    if (g_mh_class != NULL) {
        pico8_class_free(g_mh_class);
        g_mh_class = NULL;
    }
    pico8_msgbox_set_kc(NULL);
}

pico8_class_t *pico8_mh_class(void) {
    return g_mh_class;
}

pico8_instance_t *pico8_mh_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_mh_class, overrides);
}

void pico8_iv_pq(pico8_instance_t *self) {
    pico8_value_t ra_val;
    int ra_present;

    if (self == NULL) {
        return;
    }

    ra_present = pico8_instance_get_field(self, "ra", &ra_val);
    if (!ra_present || !pico8_msgbox_truthy(ra_val)) {
        pico8_vec_t je_ra = pico8_msgbox_field_vec(pico8_je(), "ra");
        pico8_vec_t mo_ra = pico8_msgbox_field_vec(pico8_mo(), "ra");
        pico8_vec_t offset = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(5));
        pico8_vec_t new_ra = pico8_vec_sub(pico8_vec_add(je_ra, mo_ra), offset);
        /* TROU-7 */
        pico8_vec_t *heap_ra = malloc(sizeof *heap_ra);
        *heap_ra = new_ra;
        pico8_ob_table_set_name(self->fields, "ra", pico8_value_ref(heap_ra));
    }
}

void pico8_iv_ph(pico8_instance_t *self) {
    pico8_vec_t ra, v, new_ra;
    pico8_fixed_t l, new_l;
    pico8_vec_t *heap_ra;

    if (self == NULL) {
        return;
    }

    ra = pico8_msgbox_field_vec(self, "ra");
    v = pico8_msgbox_field_vec(self, "v");
    new_ra = pico8_vec_add(ra, v);
    /* TROU-7 */
    heap_ra = malloc(sizeof *heap_ra);
    *heap_ra = new_ra;
    pico8_ob_table_set_name(self->fields, "ra", pico8_value_ref(heap_ra));

    l = pico8_msgbox_field_fixed(self, "l");
    new_l = pico8_fixed_add(l, pico8_fixed_from_double(0.1));
    pico8_ob_table_set_name(self->fields, "l", pico8_value_fixed(new_l));

    pico8_ob_table_set_name(self->fields, "p_", pico8_value_bool(new_l >= pico8_fixed_from_int(5)));
}

static pico8_value_t pico8_iv_ph_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_iv_ph((pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

void pico8_iv_kj(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self, pico8_vec_t p) {
    pico8_fixed_t l;
    pico8_value_t text_val;
    pico8_value_t color_val;
    const char *text = "";
    int color = 0;
    pico8_fixed_t a_center = pico8_fixed_from_double(0.5);

    if (self == NULL || gfx == NULL || mem == NULL) {
        return;
    }

    l = pico8_msgbox_field_fixed(self, "l");
    pico8_dv(mem, l);

    if (pico8_ob_table_get_index(self->fields, 1, &text_val) && text_val.kind == PICO8_VALUE_STRING) {
        text = text_val.as.string;
    }
    if (pico8_ob_table_get_index(self->fields, 2, &color_val) && color_val.kind == PICO8_VALUE_FIXED) {
        color = (int)pico8_fixed_to_double(color_val.as.fixed);
    }

    pico8_io(gfx, mem, text, p.x, p.y, color, &a_center);

    pico8_dv_default(mem);
}

static pico8_value_t pico8_iv_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_vec_t p = (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL)
                        ? *(const pico8_vec_t *)arg0.as.ref
                        : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    (void)argc;
    (void)arg1;
    pico8_iv_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (const pico8_instance_t *)self, p);
    return pico8_value_ref(NULL);
}

void pico8_iv_init(void) {
    pico8_iv_shutdown();
    pico8_pi_init();
    g_iv_class = pico8_class_declare(pico8_ke_class(), "\nfa=9,\nl=0,v=v(0,-0.34),\n");
    pico8_class_set_pq(g_iv_class, pico8_iv_pq);
    pico8_ob_table_set_name(g_iv_class->defaults, "ph", pico8_value_function(pico8_iv_ph_dispatch));
    pico8_ob_table_set_name(g_iv_class->defaults, "kj", pico8_value_function(pico8_iv_kj_dispatch));
}

void pico8_iv_shutdown(void) {
    if (g_iv_class != NULL) {
        pico8_class_free(g_iv_class);
        g_iv_class = NULL;
    }
}

pico8_class_t *pico8_iv_class(void) {
    return g_iv_class;
}

pico8_instance_t *pico8_iv_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_iv_class, overrides);
}

typedef struct {
    const pico8_instance_t *p;
    pico8_vec_t bp;
    pico8_vec_t *ra_heap;
} pico8_pz_ctx_t;

static int pico8_pz_step(pico8_value_t e, void *ctx_v, pico8_value_t *out_value) {
    pico8_pz_ctx_t *ctx = ctx_v;
    pico8_ob_table_t *b;
    pico8_vec_t *qf_heap;
    pico8_value_t ca_val;
    pico8_vec_t gq;
    pico8_instance_t *made;

    if (e.kind != PICO8_VALUE_REF || e.as.ref == NULL) {
        return 0;
    }
    assert(pico8_type_tag_of(e.as.ref) == PICO8_TYPE_TAG_OB_TABLE && "pz's bs items must be plain spec tables");
    b = (pico8_ob_table_t *)e.as.ref;

    /* Every item's "ra" is the same vector object, as in Lua -- not a copy
     * per item. */
    pico8_ob_table_set_name(b, "ra", pico8_value_ref(ctx->ra_heap));

    qf_heap = malloc(sizeof *qf_heap);
    *qf_heap = ctx->bp;
    pico8_ob_table_set_name(b, "qf", pico8_value_ref(qf_heap));

    /* TROU-9 */
    if (pico8_instance_get_field(ctx->p, "ca", &ca_val)) {
        pico8_ob_table_set_name(b, "fa", ca_val);
    }

    gq = pico8_msgbox_field_vec(ctx->p, "gq");
    ctx->bp = pico8_vec_add(ctx->bp, gq);

    made = pico8_js_new(b);
    *out_value = pico8_value_ref(made);
    return 1;
}

void pico8_pz(const pico8_instance_t *p, const pico8_list_t *bs) {
    pico8_value_t ep_val;
    const pico8_list_t *bs_effective = bs;
    pico8_list_t merged;
    int merged_built = 0;
    pico8_value_t hi_val;
    pico8_vec_t ra;
    pico8_pz_ctx_t ctx;
    pico8_list_t *le_heap;
    pico8_instance_t *ok;
    pico8_value_t eo_val;
    pico8_value_t rn_arg;
    pico8_value_t ff_val;

    if (p == NULL) {
        return;
    }

    if (pico8_instance_get_field(p, "ep", &ep_val) && pico8_msgbox_truthy(ep_val)) {
        pico8_ob_table_t *inner = pico8_ob_table_new();
        pico8_ob_table_t *outer = pico8_ob_table_new();
        pico8_list_t prefix_item;

        pico8_ob_table_set_index(inner, 1, ep_val);
        pico8_ob_table_set_name(inner, "c", pico8_value_fixed(pico8_fixed_from_int(13)));
        pico8_ob_table_set_name(outer, "lq", pico8_value_ref(inner));

        pico8_list_init(&prefix_item);
        pico8_list_add(&prefix_item, pico8_value_ref(outer));

        merged = pico8_la(&prefix_item, bs);
        pico8_list_free(&prefix_item);
        merged_built = 1;
        bs_effective = &merged;
    }

    if (pico8_instance_get_field(p, "eo", &eo_val) && pico8_msgbox_truthy(eo_val)) {
        rn_arg = eo_val;
    } else {
        rn_arg = pico8_value_fixed(pico8_fixed_from_int(62));
    }
    pico8_rn(rn_arg);

    if (pico8_instance_get_field(p, "hi", &hi_val) && pico8_msgbox_truthy(hi_val) &&
        hi_val.kind == PICO8_VALUE_REF && hi_val.as.ref != NULL) {
        ra = *(const pico8_vec_t *)hi_val.as.ref;
    } else {
        ra = pico8_msgbox_field_vec(p, "ra");
    }

    ctx.p = p;
    ctx.bp = pico8_vec_add(ra, pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0)));
    ctx.ra_heap = malloc(sizeof *ctx.ra_heap);
    *ctx.ra_heap = ra;

    ok = pico8_ok();
    pico8_ui_cj(ok);

    /* TROU-7 */
    le_heap = malloc(sizeof *le_heap);
    *le_heap = pico8_nv(bs_effective, pico8_pz_step, &ctx);
    if (ok != NULL) {
        pico8_ob_table_set_name(ok->fields, "le", pico8_value_ref(le_heap));
    } else {
        pico8_list_free(le_heap);
        free(le_heap);
    }

    if (merged_built) {
        pico8_list_free(&merged);
    }

    /* TROU-7 */
    /* The `js` instances built just above stay registered in `hv`/`cv`/`cg`
     * once `le` no longer holds them, exactly as in the cart. */
    if (pico8_instance_get_field(p, "ff", &ff_val) && pico8_msgbox_truthy(ff_val) && ok != NULL) {
        pico8_list_t *empty_heap = malloc(sizeof *empty_heap);
        pico8_list_init(empty_heap);
        pico8_ob_table_set_name(ok->fields, "le", pico8_value_ref(empty_heap));
    }
}
