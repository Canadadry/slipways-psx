#include "pico8_ui.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_ke.h"
#include "pico8_list.h"
#include "pico8_pi.h"
#include "pico8_planet_family.h"
#include "pico8_probe.h"
#include "pico8_slipway.h"
#include "pico8_type_tag.h"
#include "pico8_vec.h"
#include "pico8_wormhole.h"
#include "pico8_ref_audit.h"

static int g_ui_up = 0;
static pico8_struct_header_t *g_je = NULL;
static pico8_ui_t *g_ok = NULL;
static pico8_list_t g_shared_le;

const pico8_qz_box_t *pico8_ui_hw(void) {
    return g_ui_up ? &pico8_ui_start.hw : NULL;
}

pico8_list_t *pico8_ui_le_shared(void) {
    return &g_shared_le;
}

static int pico8_je_placeholder_field(const void *self, const char *name, pico8_value_t *out_value) {
    (void)self;
    (void)name;
    (void)out_value;
    return 0;
}

static const pico8_struct_class_t g_je_placeholder_struct_class = {
    {0},
    pico8_je_placeholder_field,
    NULL,
};

typedef struct {
    pico8_struct_header_t header;
} pico8_je_placeholder_t;

/* The only one: never allocated, never freed. */
static pico8_je_placeholder_t g_je_placeholder = {{PICO8_TYPE_TAG_STRUCT, &g_je_placeholder_struct_class}};

/* Field readers: an absent field, a wrong kind, or a NULL instance all read
 * as the default -- a zero vector here, "no value" for the raw one. */
static pico8_vec_t pico8_ui_field_vec(const pico8_struct_header_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_entity_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static pico8_value_t pico8_ui_field_raw(const pico8_struct_header_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_entity_get_field(inst, name, &v)) {
        return v;
    }
    return pico8_value_ref(NULL);
}

/* Lua truthiness, used on every dispatch result below: false is falsy, and
 * so is this port's stand-in for nil, a REF holding NULL. A check that only
 * caught false swallowed every click near an unrevealed planet. */
static int pico8_ui_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

static int pico8_ui_to_int(pico8_fixed_t x) {
    return pico8_fixed_flr_int(x);
}

/* je[bt] (bt "l" or "r"): NULL when `je` has no such button-state table,
 * which is the case for the field-less placeholder. */
static const pico8_ob_table_t *pico8_ui_je_button_table(const char *bt) {
    pico8_value_t b_val;
    assert((strcmp(bt, "l") == 0 || strcmp(bt, "r") == 0) &&
           "je[bt]: bt is l or r only; another button has no state table -- stop");
    if (pico8_entity_get_field(g_je, bt, &b_val) && b_val.kind == PICO8_VALUE_REF && b_val.as.ref != NULL) {
        assert(pico8_type_tag_of(b_val.as.ref) == PICO8_TYPE_TAG_OB_TABLE &&
               "je's per-button state must be a plain table");
        return (const pico8_ob_table_t *)b_val.as.ref;
    }
    return NULL;
}

static int pico8_ui_je_button_field_bool(const char *bt, const char *field) {
    const pico8_ob_table_t *b = pico8_ui_je_button_table(bt);
    pico8_value_t v;
    if (b != NULL && pico8_ob_table_get_name(b, field, &v) && v.kind == PICO8_VALUE_BOOL) {
        return v.as.boolean;
    }
    return 0;
}

static pico8_value_t pico8_ui_ev_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_ui_ev(pico8_dispatch_gfx(), pico8_dispatch_mem(), (const pico8_ui_t *)self);
    return pico8_value_ref(NULL);
}

static int ui_ref_field(void *ref, pico8_value_t *out_value) {
    if (ref == NULL) {
        return 0;
    }
    *out_value = pico8_value_ref(ref);
    return 1;
}

static int pico8_ui_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_ui_t *ui = self;

    if (strcmp(name, "ql") == 0) {
        *out_value = pico8_value_fixed(ui->ql);
        return 1;
    }
    if (strcmp(name, "fa") == 0) {
        *out_value = pico8_value_fixed(ui->fa);
        return 1;
    }
    if (strcmp(name, "le") == 0) {
        return ui_ref_field(ui->le, out_value);
    }
    if (strcmp(name, "hw") == 0) {
        return ui_ref_field((void *)ui->hw, out_value);
    }
    if (strcmp(name, "qa") == 0) {
        return ui_ref_field(ui->qa, out_value);
    }
    if (strcmp(name, "ic") == 0) {
        return ui_ref_field(ui->ic, out_value);
    }
    if (strcmp(name, "o_") == 0) {
        return ui_ref_field(ui->o_, out_value);
    }
    if (strcmp(name, "gj") == 0) {
        return ui_ref_field(ui->gj, out_value);
    }
    if (strcmp(name, "lheld") == 0) {
        return ui_ref_field(ui->lheld, out_value);
    }
    if (strcmp(name, "rheld") == 0) {
        return ui_ref_field(ui->rheld, out_value);
    }
    return pico8_ke_field(self, name, out_value);
}

static const pico8_struct_class_t g_ui_struct_class = {
    {
        [PICO8_METHOD_EV] = pico8_ui_ev_dispatch,
    },
    pico8_ui_field,
    NULL,
};

static void *pico8_ui_struct_new(void) {
    pico8_ui_t *self;
    int i;

    assert(g_ui_up && "pico8_ui_init() must run first");
    self = malloc(sizeof *self);
    pico8_ke_struct_init(&self->ke, &g_ui_struct_class);
    self->ql = pico8_ui_start.ql;
    self->fa = pico8_ui_start.fa;
    self->le = &g_shared_le;
    self->hw = &pico8_ui_start.hw;
    self->qa = NULL;
    self->ic = NULL;
    self->ic_own.layer_count = PICO8_UI_IC_LAYERS;
    for (i = 0; i < PICO8_UI_IC_LAYERS; i++) {
        pico8_list_init(&self->ic_own.layers[i]);
    }
    self->o_ = NULL;
    self->gj = NULL;
    self->lheld = NULL;
    self->rheld = NULL;
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("ui");
#endif
    pico8_ke_struct_pq(&self->ke);
    return self;
}

void pico8_ui_init(void) {
    pico8_ui_shutdown();
    pico8_pi_init();
    pico8_list_init(&g_shared_le);
    g_ui_up = 1;

    g_je = (pico8_struct_header_t *)&g_je_placeholder;
}

void pico8_ui_shutdown(void) {
    g_ui_up = 0;
    g_je = NULL;
    if (g_ok != NULL) {
        free(g_ok);
        g_ok = NULL;
    }
}

const pico8_struct_class_t *pico8_je_placeholder_struct_class(void) {
    return &g_je_placeholder_struct_class;
}

int pico8_ui_ready(void) {
    return g_ui_up;
}

const pico8_struct_class_t *pico8_ui_struct_class(void) {
    return &g_ui_struct_class;
}

pico8_ui_t *pico8_ui_new(void) {
    return pico8_ui_struct_new();
}

pico8_struct_header_t *pico8_je(void) {
    return g_je;
}

/* Deliberately does not free the outgoing instance. `pico8_jl()` rebuilds
 * je/ok mid-frame, reachable from a click dispatched inside pico8_ui_lj,
 * whose `self` is still the old `ok` after the callback returns -- freeing
 * here is a use-after-free. */
void pico8_ui_set_je(pico8_struct_header_t *instance) {
    if (instance == g_je) {
        return;
    }
    g_je = instance;
}

pico8_ui_t *pico8_ok(void) {
    return g_ok;
}

/* Same non-freeing rule as pico8_ui_set_je(), and the same reason. */
void pico8_ui_set_ok(pico8_ui_t *ui) {
    if (ui == g_ok) {
        return;
    }
    g_ok = ui;
}

/* `le` is read before gw()/bh() run: the `self.le == le` test further down
 * is a pointer identity check against the list this frame started with. */
void pico8_ui_lj(pico8_mem_t *mem, pico8_ui_t *self) {
    pico8_list_t *le_ptr_before;
    int le_count;
    pico8_vec_t qa;
    pico8_vec_t *heap_qa;
    int hit;

    if (self == NULL) {
        return;
    }

    le_ptr_before = self->le;
    le_count = self->le != NULL ? self->le->count : 0;

    qa = pico8_vec_add(pico8_ui_field_vec(g_je, "ra"), pico8_ui_field_vec(pico8_mo(), "ra"));
    /* TROU-7 */
    heap_qa = malloc(sizeof *heap_qa);
    *heap_qa = qa;
    self->qa = heap_qa;

    pico8_ui_gw(mem, self);

    hit = (le_count == 0) || pico8_gu(le_ptr_before, pico8_value_ref(self->o_));
    if (hit) {
        pico8_ui_bh(self, "l");
    }

    pico8_ui_bh(self, "r");

    if (pico8_ui_je_button_field_bool("l", "jn") && self->le == le_ptr_before) {
        pico8_ui_cj(self);
    }

    self->gj = self->o_;
}

/* `(rheld and 9) or (o_ and o_.ql) or self.ql`: a hovered entity whose own
 * `ql` is absent falls through to self's own cursor sprite. */
void pico8_ui_ev(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_ui_t *self) {
    pico8_vec_t je_ra;
    int spr_id = 0;

    if (self == NULL || gfx == NULL || mem == NULL) {
        return;
    }

    if (self->rheld != NULL) {
        spr_id = 9;
    } else if (self->o_ != NULL) {
        pico8_value_t ql_val = pico8_ui_field_raw(self->o_, "ql");
        spr_id = (ql_val.kind == PICO8_VALUE_FIXED) ? pico8_ui_to_int(ql_val.as.fixed) : pico8_ui_to_int(self->ql);
    } else {
        spr_id = pico8_ui_to_int(self->ql);
    }

    je_ra = pico8_ui_field_vec(g_je, "ra");
    pico8_spr_default(gfx, mem, spr_id, pico8_ui_to_int(je_ra.x), pico8_ui_to_int(je_ra.y));
}

/* Lua's `(self.gj or {}).m_=false` writes onto a throwaway table when the
 * field is absent, with no observable effect, so the write is simply
 * skipped here rather than allocating a table nothing reads. */
void pico8_ui_gw(pico8_mem_t *mem, pico8_ui_t *self) {
    pico8_value_t qa_val;
    pico8_value_t je_ra_val;
    const pico8_list_t *hd_bucket;
    int i;
    int poke_val;

    if (self == NULL || mem == NULL) {
        return;
    }

    for (i = 0; i < PICO8_UI_IC_LAYERS; i++) {
        pico8_list_free(&self->ic_own.layers[i]);
        pico8_list_init(&self->ic_own.layers[i]);
    }
    self->ic = &self->ic_own;
    self->o_ = NULL;

    qa_val = pico8_value_ref(self->qa);
    je_ra_val = pico8_ui_field_raw(g_je, "ra");

    hd_bucket = pico8_cv("hd");
    if (hd_bucket != NULL) {
        for (i = 0; i < hd_bucket->count; i++) {
            pico8_struct_header_t *e = hd_bucket->items[i].as.ref;
            pico8_value_t hit_out;

            /* TROU-11 */
            if (pico8_mv(e, "hd", 2, qa_val, je_ra_val, &hit_out) && pico8_ui_truthy(hit_out)) {
                pico8_value_t fa_val;
                if (pico8_entity_get_field(e, "fa", &fa_val) && fa_val.kind == PICO8_VALUE_FIXED) {
                    int fa = pico8_ui_to_int(fa_val.as.fixed);

                    assert(fa >= 0 && fa < PICO8_UI_IC_LAYERS &&
                           "ui.ic: a hovered entity's fa outside 0..15, the layers jz walks -- stop, do not widen");
                    pico8_list_add(&self->ic->layers[fa], pico8_value_ref(e));
                }
                self->o_ = e;
            }
        }
    }

    if (self->gj != NULL) {
        pico8_entity_set_field(self->gj, "m_", pico8_value_bool(0));
    }

    if (self->o_ != NULL) {
        pico8_entity_set_field(self->o_, "m_", pico8_value_bool(1));
    }

    if (self->o_ != NULL) {
        pico8_value_t fg_val = pico8_ui_field_raw(self->o_, "fg");
        poke_val = (fg_val.kind == PICO8_VALUE_FIXED) ? pico8_ui_to_int(fg_val.as.fixed) : 0;
    } else {
        int prev = pico8_peek(mem, 0x5f80);
        poke_val = (prev - 1 > 0) ? prev - 1 : 0;
    }
    pico8_poke(mem, 0x5f80, (unsigned char)poke_val);
}

static int ui_is_held_class(const pico8_struct_class_t *cls) {
    return cls == pico8_jc_struct_class() || cls == pico8_g__struct_class() || cls == pico8_mz_struct_class() ||
           (cls->methods[PICO8_METHOD_LY] != NULL && cls != pico8_pm_struct_class());
}

static void ui_take_held(pico8_value_t value, pico8_struct_header_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_STRUCT &&
           ui_is_held_class(((const pico8_struct_header_t *)value.as.ref)->cls) &&
           "ui.lheld: a value neither nil nor what an ldown hands over (a jc; a g_ or an mz from bg; a qh, os, nr, fr, "
           "nl, mx or hj from an ie) does not fit the member's type -- stop, do not widen the member");
    *out = value.as.ref;
}

static void ui_take_bg(pico8_value_t value, pico8_bg_t **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_REF && pico8_type_tag_of(value.as.ref) == PICO8_TYPE_TAG_STRUCT &&
           ((const pico8_struct_header_t *)value.as.ref)->cls == pico8_bg_struct_class() &&
           "ui.rheld: a value neither nil nor the bg that rdown hands over does not fit the member's type "
           "-- stop, do not widen the member");
    *out = value.as.ref;
}

void pico8_ui_bh(pico8_ui_t *self, const char *bt) {
    int l;
    const char *down;
    pico8_struct_header_t *held_inst;
    pico8_value_t qa_val;
    pico8_value_t je_ra_val;
    pico8_value_t discard;

    if (self == NULL) {
        return;
    }

    assert((strcmp(bt, "l") == 0 || strcmp(bt, "r") == 0) &&
           "ui:bh: bt is l or r only; another button has no state table -- stop");
    l = bt[0] == 'l';
    down = l ? "ldown" : "rdown";
    qa_val = pico8_value_ref(self->qa);
    je_ra_val = pico8_ui_field_raw(g_je, "ra");

    if (pico8_ui_je_button_field_bool(bt, "jn")) {
        pico8_struct_header_t *kv = NULL;
        pico8_value_t kn;

        if (pico8_ui_jz(self, down, 2, qa_val, je_ra_val, &kv, &kn) && kn.kind == PICO8_VALUE_REF) {
            /* TROU-6 */
            if (l) {
                ui_take_held(kn, &self->lheld);
            } else {
                ui_take_bg(kn, &self->rheld);
            }
        }
    }

    held_inst = l ? self->lheld : (self->rheld != NULL ? &self->rheld->ke.header : NULL);

    pico8_mv(held_inst, "hq", 2, qa_val, je_ra_val, &discard);

    if (pico8_ui_je_button_field_bool(bt, "hm") && held_inst != NULL) {
        pico8_struct_header_t *nw_kv = NULL;
        pico8_value_t nw_kn;
        pico8_value_t ey_arg = pico8_value_ref(NULL);

        /* `ey`'s argument is the entity `jz` found, not a field read on
         * `held`: no object in this cart carries a "kv" field. */
        if (pico8_ui_jz(self, "gv", 1, pico8_value_ref(held_inst), pico8_value_ref(NULL), &nw_kv, &nw_kn) &&
            nw_kv != NULL) {
            ey_arg = pico8_value_ref(nw_kv);
        }

        pico8_mv(held_inst, "ey", 1, ey_arg, pico8_value_ref(NULL), &discard);
        if (l) {
            self->lheld = NULL;
        } else {
            self->rheld = NULL;
        }
    }
}

/* Layers descend from 15 to 0 and the first truthy answer wins: that order
 * is the dispatch priority between overlapping entities, not an arbitrary
 * walk. */
int pico8_ui_jz(pico8_ui_t *self, const char *rm, int argc, pico8_value_t arg0, pico8_value_t arg1,
                pico8_struct_header_t **out_kv, pico8_value_t *out_kn) {
    int mf;

    if (self == NULL) {
        return 0;
    }

    if (self->ic == NULL) {
        return 0;
    }

    for (mf = PICO8_UI_IC_LAYERS - 1; mf >= 0; mf--) {
        const pico8_list_t *bucket = &self->ic->layers[mf];
        int i;

        for (i = 0; i < bucket->count; i++) {
            pico8_struct_header_t *h = bucket->items[i].as.ref;
            pico8_value_t kn;

            if (pico8_mv(h, rm, argc, arg0, arg1, &kn) && pico8_ui_truthy(kn)) {
                *out_kv = h;
                *out_kn = kn;
                return 1;
            }
        }
    }

    return 0;
}

/* `he()` returns nothing, so this never produces a mapped value (always 0)
 * and the mapped list stays empty; the point is `pico8_he()`'s side effect
 * of unregistering each entity. */
static int pico8_ui_cj_he_mapper(pico8_value_t e, void *ctx, pico8_value_t *out_value) {
    (void)ctx;
    (void)out_value;
    if (e.kind == PICO8_VALUE_REF && e.as.ref != NULL) {
        pico8_he((pico8_struct_header_t *)e.as.ref);
    }
    return 0;
}

void pico8_ui_cj(pico8_ui_t *self) {
    pico8_list_t *heap_le;

    if (self == NULL) {
        return;
    }

    if (self->le != NULL) {
        pico8_list_t discarded = pico8_nv(self->le, pico8_ui_cj_he_mapper, NULL);
        pico8_list_free(&discarded);
    }

    /* TROU-7 */
    heap_le = malloc(sizeof *heap_le);
    pico8_list_init(heap_le);
    self->le = heap_le;
}
