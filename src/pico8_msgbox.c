#include "pico8_msgbox.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

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

static pico8_ob_table_t *g_mh_spec = NULL;
static pico8_ob_table_t *g_mh_nu = NULL;
static pico8_ob_table_t *g_iv_spec = NULL;
static pico8_mh_t *g_kc = NULL;

pico8_mh_t *pico8_kc(void) {
    return g_kc;
}

void pico8_msgbox_set_kc(pico8_mh_t *mh) {
    g_kc = mh;
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
static pico8_vec_t pico8_msgbox_field_vec(const pico8_struct_header_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_entity_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

void pico8_mh_pq(pico8_mh_t *self) {
    pico8_mh_t *previous;

    if (self == NULL) {
        return;
    }

    previous = pico8_kc();
    if (previous != NULL) {
        previous->p_ = pico8_value_bool(1);
    }
    pico8_msgbox_set_kc(self);
}

static pico8_value_t pico8_mh_ev_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_mh_ev(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_mh_t *)self);
    return pico8_value_ref(NULL);
}

static pico8_fixed_t pico8_mh_value_fixed(pico8_value_t v) {
    return v.kind == PICO8_VALUE_FIXED ? v.as.fixed : pico8_fixed_from_int(0);
}

void pico8_mh_ev(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_mh_t *self) {
    pico8_fixed_t self_t, rh, t, nc, t3;
    pico8_value_t lh_result;
    int lh_found;

    if (self == NULL) {
        return;
    }

    self_t = pico8_mh_value_fixed(self->ke.t);
    rh = pico8_mh_value_fixed(self->rh);
    t = pico8_fixed_sub(pico8_fixed_div(self_t, rh), pico8_fixed_from_int(10));

    if (t >= 0) {
        lh_found =
            pico8_mv((pico8_struct_header_t *)self, "lh", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &lh_result);
        if (lh_found && pico8_msgbox_truthy(lh_result)) {
            self->ke.t = pico8_value_fixed(pico8_fixed_mul(rh, pico8_fixed_from_int(10)));
            t = pico8_fixed_from_int(0);
        }
    }

    /* `-t^3/9` is `(-(t^3))/9`: in Lua `^` binds tighter than unary `-`,
     * which in turn binds tighter than `/`. */
    t3 = pico8_fixed_mul(pico8_fixed_mul(t, t), t);
    nc = pico8_fixed_div(pico8_fixed_neg(t3), pico8_fixed_from_int(9));

    if (gfx != NULL && mem != NULL) {
        pico8_bl(gfx, mem, &self->items, nc);
        pico8_camera_reset(gfx);
    }

    if (t > PICO8_FIXED_LIT(0x000a8000, 10.5)) {
        self->p_ = pico8_value_bool(1);
        pico8_msgbox_set_kc(NULL);
        assert(pico8_kc() == NULL && "`self.p_,kc=true` is one value for two targets: kc gets nil, not true");
    }
}

static int pico8_mh_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_mh_t *mh = self;
    const pico8_value_t *member;

    if (strcmp(name, "nu") == 0) {
        member = &mh->nu;
    } else if (strcmp(name, "fa") == 0) {
        member = &mh->fa;
    } else if (strcmp(name, "rh") == 0) {
        member = &mh->rh;
    } else if (strcmp(name, "lh") == 0) {
        member = &mh->lh;
    } else if (strcmp(name, "p_") == 0) {
        member = &mh->p_;
    } else {
        return pico8_ke_field(self, name, out_value);
    }
    if (member->kind == PICO8_VALUE_REF && member->as.ref == NULL) {
        return 0;
    }
    *out_value = *member;
    return 1;
}

static const pico8_struct_class_t g_mh_struct_class = {
    {[PICO8_METHOD_EV] = pico8_mh_ev_dispatch},
    pico8_mh_field,
    NULL,
};

static void *pico8_mh_struct_new(const pico8_ob_table_t *overrides) {
    pico8_mh_t *self = malloc(sizeof *self);
    int i;

    pico8_ke_struct_init(&self->ke, &g_mh_struct_class);
    self->nu = pico8_value_ref(g_mh_nu);
    self->fa = pico8_value_fixed(pico8_fixed_from_int(14));
    self->rh = pico8_value_fixed(pico8_fixed_from_int(5));
    self->lh = pico8_value_ref(NULL);
    self->p_ = pico8_value_ref(NULL);
    pico8_list_init(&self->items);
    for (i = 0; overrides != NULL && i < overrides->count; i++) {
        pico8_ob_key_t key = overrides->entries[i].key;
        pico8_value_t value = overrides->entries[i].value;

        if (key.kind != PICO8_OB_KEY_NAME) {
            pico8_list_add(&self->items, value);
        } else if (strcmp(key.name, "rh") == 0) {
            self->rh = value;
        } else if (strcmp(key.name, "lh") == 0) {
            self->lh = value;
        } else {
            assert(0 && "mh: every measured key is positional, rh or lh; another would be a field under no rule "
                        "of plan-3 section 5 -- stop, do not add a member");
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("mh");
#endif
    pico8_mh_pq(self);
    pico8_ke_struct_pq(&self->ke);
    PICO8_MEMBER_CENSUS_SEE(self, pico8_mh_t);
    return self;
}

void pico8_mh_init(void) {
    pico8_ob_table_t *spec;
    pico8_value_t nu = pico8_value_ref(NULL);
    int found;

    pico8_mh_shutdown();
    g_mh_spec = pico8_ob("\nnu=o(\"mh\"),\nfa=14,\nrh=5,\n", NULL);
    spec = pico8_ob("\nnu=o(\"mh\"),\n", NULL);
    found = pico8_ob_table_get_name(spec, "nu", &nu);
    assert(found && nu.kind == PICO8_VALUE_REF && nu.as.ref != NULL);
    (void)found;
    g_mh_nu = nu.as.ref;
    pico8_ob_table_free(spec);
}

void pico8_mh_shutdown(void) {
    pico8_ob_table_free(g_mh_spec);
    g_mh_spec = NULL;
    pico8_ob_table_free(g_mh_nu);
    g_mh_nu = NULL;
    pico8_msgbox_set_kc(NULL);
}

int pico8_mh_ready(void) {
    return g_mh_spec != NULL;
}

int pico8_mh_default(const char *name, pico8_value_t *out_value) {
    return g_mh_spec != NULL &&
           (pico8_ob_table_get_name(g_mh_spec, name, out_value) || pico8_ke_default(name, out_value));
}

pico8_mh_t *pico8_mh_new(const pico8_ob_table_t *overrides) {
    return pico8_mh_struct_new(overrides);
}

static pico8_vec_t g_iv_v = {0, -PICO8_FIXED_LIT(0x0000570a, 0.34)};

static pico8_fixed_t pico8_iv_value_fixed(pico8_value_t v) {
    return v.kind == PICO8_VALUE_FIXED ? v.as.fixed : pico8_fixed_from_int(0);
}

static pico8_vec_t pico8_iv_value_vec(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

void pico8_iv_pq(pico8_iv_t *self) {
    if (self == NULL) {
        return;
    }

    if (!pico8_msgbox_truthy(self->ra)) {
        pico8_vec_t je_ra = pico8_msgbox_field_vec(pico8_je(), "ra");
        pico8_vec_t mo_ra = pico8_msgbox_field_vec(pico8_mo(), "ra");
        pico8_vec_t offset = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(5));
        pico8_vec_t new_ra = pico8_vec_sub(pico8_vec_add(je_ra, mo_ra), offset);
        /* TROU-7 */
        pico8_vec_t *heap_ra = malloc(sizeof *heap_ra);
        *heap_ra = new_ra;
        self->ra = pico8_value_ref(heap_ra);
    }
}

void pico8_iv_ph(pico8_iv_t *self) {
    pico8_vec_t new_ra;
    pico8_fixed_t new_l;
    pico8_vec_t *heap_ra;

    if (self == NULL) {
        return;
    }

    new_ra = pico8_vec_add(pico8_iv_value_vec(self->ra), pico8_iv_value_vec(self->v));
    /* TROU-7 */
    heap_ra = malloc(sizeof *heap_ra);
    *heap_ra = new_ra;
    self->ra = pico8_value_ref(heap_ra);

    new_l = pico8_fixed_add(pico8_iv_value_fixed(self->l), PICO8_FIXED_LIT(0x00001999, 0.1));
    self->l = pico8_value_fixed(new_l);

    self->p_ = pico8_value_bool(new_l >= pico8_fixed_from_int(5));
}

static pico8_value_t pico8_iv_ph_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_iv_ph((pico8_iv_t *)self);
    return pico8_value_ref(NULL);
}

void pico8_iv_kj(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_iv_t *self, pico8_vec_t p) {
    pico8_value_t text_val;
    pico8_value_t color_val;
    const char *text = "";
    int color = 0;
    pico8_fixed_t a_center = PICO8_FIXED_LIT(0x00008000, 0.5);

    if (self == NULL || gfx == NULL || mem == NULL) {
        return;
    }

    pico8_dv(mem, pico8_iv_value_fixed(self->l));

    text_val = self->at1;
    color_val = self->at2;
    if (text_val.kind == PICO8_VALUE_STRING) {
        text = text_val.as.string;
    }
    if (color_val.kind == PICO8_VALUE_FIXED) {
        color = pico8_fixed_flr_int(color_val.as.fixed);
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
    pico8_iv_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (const pico8_iv_t *)self, p);
    return pico8_value_ref(NULL);
}

static int pico8_iv_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_iv_t *iv = self;
    const pico8_value_t *member;

    if (strcmp(name, "fa") == 0) {
        member = &iv->fa;
    } else if (strcmp(name, "l") == 0) {
        member = &iv->l;
    } else if (strcmp(name, "v") == 0) {
        member = &iv->v;
    } else if (strcmp(name, "ra") == 0) {
        member = &iv->ra;
    } else if (strcmp(name, "p_") == 0) {
        member = &iv->p_;
    } else {
        return pico8_ke_field(self, name, out_value);
    }
    if (member->kind == PICO8_VALUE_REF && member->as.ref == NULL) {
        return 0;
    }
    *out_value = *member;
    return 1;
}

static const pico8_struct_class_t g_iv_struct_class = {
    {[PICO8_METHOD_PH] = pico8_iv_ph_dispatch, [PICO8_METHOD_KJ] = pico8_iv_kj_dispatch},
    pico8_iv_field,
    NULL,
};

static void *pico8_iv_struct_new(const pico8_ob_table_t *overrides) {
    pico8_iv_t *self = malloc(sizeof *self);
    int i;

    pico8_ke_struct_init(&self->ke, &g_iv_struct_class);
    self->fa = pico8_value_fixed(pico8_fixed_from_int(9));
    self->l = pico8_value_fixed(pico8_fixed_from_int(0));
    self->v = pico8_value_ref(&g_iv_v);
    self->ra = pico8_value_ref(NULL);
    self->p_ = pico8_value_ref(NULL);
    self->at1 = pico8_value_ref(NULL);
    self->at2 = pico8_value_ref(NULL);
    for (i = 0; overrides != NULL && i < overrides->count; i++) {
        pico8_ob_key_t key = overrides->entries[i].key;
        pico8_value_t value = overrides->entries[i].value;

        if (key.kind == PICO8_OB_KEY_NAME && strcmp(key.name, "ra") == 0) {
            self->ra = value;
        } else if (key.kind != PICO8_OB_KEY_NAME && key.index == 1) {
            self->at1 = value;
        } else if (key.kind != PICO8_OB_KEY_NAME && key.index == 2) {
            self->at2 = value;
        } else {
            assert(0 && "iv: every measured key is ra, [1] or [2]; another would be a field under no rule of "
                        "plan-3 section 5 -- stop, do not add a member");
        }
    }
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("iv");
#endif
    pico8_iv_pq(self);
    pico8_ke_struct_pq(&self->ke);
    PICO8_MEMBER_CENSUS_SEE(self, pico8_iv_t);
    return self;
}

void pico8_iv_init(void) {
    pico8_iv_shutdown();
    pico8_pi_init();
    g_iv_spec = pico8_ob("\nfa=9,\nl=0,v=v(0,-0.34),\n", NULL);
}

void pico8_iv_shutdown(void) {
    pico8_ob_table_free(g_iv_spec);
    g_iv_spec = NULL;
}

int pico8_iv_ready(void) {
    return g_iv_spec != NULL;
}

int pico8_iv_default(const char *name, pico8_value_t *out_value) {
    return g_iv_spec != NULL &&
           (pico8_ob_table_get_name(g_iv_spec, name, out_value) || pico8_ke_default(name, out_value));
}

pico8_iv_t *pico8_iv_new(const pico8_ob_table_t *overrides) {
    return pico8_iv_struct_new(overrides);
}

typedef struct {
    const pico8_struct_header_t *p;
    pico8_vec_t bp;
    pico8_vec_t *ra_heap;
} pico8_pz_ctx_t;

static int pico8_pz_step(pico8_value_t e, void *ctx_v, pico8_value_t *out_value) {
    pico8_pz_ctx_t *ctx = ctx_v;
    pico8_ob_table_t *b;
    pico8_vec_t *qf_heap;
    pico8_value_t ca_val;
    pico8_vec_t gq;
    pico8_js_t *made;

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
    if (pico8_entity_get_field(ctx->p, "ca", &ca_val)) {
        pico8_ob_table_set_name(b, "fa", ca_val);
    }

    gq = pico8_msgbox_field_vec(ctx->p, "gq");
    ctx->bp = pico8_vec_add(ctx->bp, gq);

    made = pico8_js_new(b);
    *out_value = pico8_value_ref(made);
    return 1;
}

void pico8_pz(const pico8_struct_header_t *p, const pico8_list_t *bs) {
    pico8_value_t ep_val;
    const pico8_list_t *bs_effective = bs;
    pico8_list_t merged;
    int merged_built = 0;
    pico8_value_t hi_val;
    pico8_vec_t ra;
    pico8_pz_ctx_t ctx;
    pico8_list_t *le_heap;
    pico8_ui_t *ok;
    pico8_value_t eo_val;
    pico8_value_t rn_arg;
    pico8_value_t ff_val;

    if (p == NULL) {
        return;
    }
    assert(pico8_type_tag_of(p) == PICO8_TYPE_TAG_STRUCT &&
           "pz: p is a struct; no class instance reaches pz any more");

    if (pico8_entity_get_field(p, "ep", &ep_val) && pico8_msgbox_truthy(ep_val)) {
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

    if (pico8_entity_get_field(p, "eo", &eo_val) && pico8_msgbox_truthy(eo_val)) {
        rn_arg = eo_val;
    } else {
        rn_arg = pico8_value_fixed(pico8_fixed_from_int(62));
    }
    pico8_rn(rn_arg);

    if (pico8_entity_get_field(p, "hi", &hi_val) && pico8_msgbox_truthy(hi_val) &&
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
        ok->le = pico8_value_ref(le_heap);
    } else {
        pico8_list_free(le_heap);
        free(le_heap);
    }

    if (merged_built) {
        pico8_list_free(&merged);
    }

    /* TROU-7 */
    /* The `js` buttons built just above stay registered in `hv`/`cv`/`cg`
     * once `le` no longer holds them, exactly as in the cart. */
    if (pico8_entity_get_field(p, "ff", &ff_val) && pico8_msgbox_truthy(ff_val) && ok != NULL) {
        pico8_list_t *empty_heap = malloc(sizeof *empty_heap);
        pico8_list_init(empty_heap);
        ok->le = pico8_value_ref(empty_heap);
    }
}
