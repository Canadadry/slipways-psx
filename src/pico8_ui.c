#include "pico8_ui.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_ke.h"
#include "pico8_list.h"
#include "pico8_pi.h"
#include "pico8_vec.h"
#include "pico8_ref_audit.h"

static pico8_class_t *g_ui_class = NULL;
static pico8_instance_t *g_je = NULL;
static pico8_instance_t *g_ok = NULL;

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
static pico8_vec_t pico8_ui_field_vec(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_entity_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static pico8_value_t pico8_ui_field_raw(const pico8_instance_t *inst, const char *name) {
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
           "je[bt]: plan-3 section 5 enumerates bt as l or r only; another key is an unmeasured field -- stop");
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
    pico8_ui_ev(pico8_dispatch_gfx(), pico8_dispatch_mem(), (const pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

void pico8_ui_init(void) {
    pico8_ob_table_t *raw_le = NULL;
    pico8_value_t le_val;
    pico8_list_t *heap_le;

    pico8_ui_shutdown();
    pico8_pi_init();

    g_ui_class = pico8_class_declare(pico8_ke_class(), "\nql=58,fa=15,le=o(),hw=b(10,10,118,118),\n");
    PICO8_CLASS_COVERAGE_NAME(g_ui_class, "ui");

    /* `gu()`/`nv()` need a list, not the table `ob()` parses for `le=o()`.
     * The raw table is freed here: nothing else ever sees it. */
    if (pico8_ob_table_get_name(g_ui_class->defaults, "le", &le_val) && le_val.kind == PICO8_VALUE_REF) {
        raw_le = (pico8_ob_table_t *)le_val.as.ref;
    }
    heap_le = malloc(sizeof *heap_le);
    pico8_list_init(heap_le);
    pico8_ob_table_set_name(g_ui_class->defaults, "le", pico8_value_ref(heap_le));
    pico8_ob_table_free(raw_le);

    pico8_ob_table_set_name(g_ui_class->defaults, "ev", pico8_value_function(pico8_ui_ev_dispatch));

    g_je = (pico8_instance_t *)&g_je_placeholder;
}

void pico8_ui_shutdown(void) {
    if (g_ui_class != NULL) {
        pico8_class_free(g_ui_class);
        g_ui_class = NULL;
    }
    g_je = NULL;
    if (g_ok != NULL) {
        pico8_instance_free(g_ok);
        g_ok = NULL;
    }
}

const pico8_struct_class_t *pico8_je_placeholder_struct_class(void) {
    return &g_je_placeholder_struct_class;
}

pico8_class_t *pico8_ui_class(void) {
    return g_ui_class;
}

pico8_instance_t *pico8_ui_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_ui_class, overrides);
}

pico8_instance_t *pico8_je(void) {
    return g_je;
}

/* Deliberately does not free the outgoing instance. `pico8_jl()` rebuilds
 * je/ok mid-frame, reachable from a click dispatched inside pico8_ui_lj,
 * whose `self` is still the old `ok` after the callback returns -- freeing
 * here is a use-after-free. */
void pico8_ui_set_je(pico8_instance_t *instance) {
    if (instance == g_je) {
        return;
    }
    g_je = instance;
}

pico8_instance_t *pico8_ok(void) {
    return g_ok;
}

/* Same non-freeing rule as pico8_ui_set_je(), and the same reason. */
void pico8_ui_set_ok(pico8_instance_t *instance) {
    if (instance == g_ok) {
        return;
    }
    g_ok = instance;
}

/* `le` is read before gw()/bh() run: the `self.le == le` test further down
 * is a pointer identity check against the list this frame started with. */
void pico8_ui_lj(pico8_mem_t *mem, pico8_instance_t *self) {
    pico8_value_t le_val;
    void *le_ptr_before = NULL;
    int le_count;
    pico8_vec_t qa;
    pico8_vec_t *heap_qa;
    int hit;

    if (self == NULL) {
        return;
    }

    if (pico8_instance_get_field(self, "le", &le_val) && le_val.kind == PICO8_VALUE_REF && le_val.as.ref != NULL) {
        le_ptr_before = le_val.as.ref;
        le_count = PICO8_REF_AS_LIST_CONST(le_val.as.ref)->count;
    } else {
        le_count = 0;
    }

    qa = pico8_vec_add(pico8_ui_field_vec(g_je, "ra"), pico8_ui_field_vec(pico8_mo(), "ra"));
    /* TROU-7 */
    heap_qa = malloc(sizeof *heap_qa);
    *heap_qa = qa;
    pico8_ob_table_set_name(self->fields, "qa", pico8_value_ref(heap_qa));

    pico8_ui_gw(mem, self);

    hit = (le_count == 0) || pico8_gu(PICO8_REF_AS_LIST_CONST(le_ptr_before), pico8_ui_field_raw(self, "o_"));
    if (hit) {
        pico8_ui_bh(self, "l");
    }

    pico8_ui_bh(self, "r");

    if (pico8_ui_je_button_field_bool("l", "jn")) {
        pico8_value_t le_val2;
        int same = pico8_instance_get_field(self, "le", &le_val2) && le_val2.kind == PICO8_VALUE_REF &&
                    le_val2.as.ref == le_ptr_before;
        if (same) {
            pico8_ui_cj(self);
        }
    }

    pico8_ob_table_set_name(self->fields, "gj", pico8_ui_field_raw(self, "o_"));
}

/* `(rheld and 9) or (o_ and o_.ql) or self.ql`: a hovered entity whose own
 * `ql` is absent falls through to self's own cursor sprite. */
void pico8_ui_ev(const pico8_gfx_t *gfx, pico8_mem_t *mem, const pico8_instance_t *self) {
    pico8_vec_t je_ra;
    int spr_id = 0;
    pico8_value_t o_val;

    if (self == NULL || gfx == NULL || mem == NULL) {
        return;
    }

    if (pico8_ui_truthy(pico8_ui_field_raw(self, "rheld"))) {
        spr_id = 9;
    } else {
        o_val = pico8_ui_field_raw(self, "o_");
        if (o_val.kind == PICO8_VALUE_REF && o_val.as.ref != NULL) {
            pico8_value_t ql_val = pico8_ui_field_raw((const pico8_instance_t *)o_val.as.ref, "ql");
            if (ql_val.kind != PICO8_VALUE_FIXED) {
                ql_val = pico8_ui_field_raw(self, "ql");
            }
            spr_id = (ql_val.kind == PICO8_VALUE_FIXED) ? pico8_ui_to_int(ql_val.as.fixed) : 0;
        } else {
            pico8_value_t ql_val = pico8_ui_field_raw(self, "ql");
            spr_id = (ql_val.kind == PICO8_VALUE_FIXED) ? pico8_ui_to_int(ql_val.as.fixed) : 0;
        }
    }

    je_ra = pico8_ui_field_vec(g_je, "ra");
    pico8_spr_default(gfx, mem, spr_id, pico8_ui_to_int(je_ra.x), pico8_ui_to_int(je_ra.y));
}

/* Lua's `(self.gj or {}).m_=false` writes onto a throwaway table when the
 * field is absent, with no observable effect, so the write is simply
 * skipped here rather than allocating a table nothing reads. */
void pico8_ui_gw(pico8_mem_t *mem, pico8_instance_t *self) {
    pico8_ob_table_t *ic;
    pico8_value_t qa_val;
    pico8_value_t je_ra_val;
    const pico8_list_t *hd_bucket;
    pico8_value_t gj_val;
    pico8_value_t o_val;
    int i;
    int poke_val;

    if (self == NULL || mem == NULL) {
        return;
    }

    /* TROU-7 */
    ic = pico8_ob_table_new();
    pico8_ob_table_set_name(self->fields, "ic", pico8_value_ref(ic));
    pico8_ob_table_set_name(self->fields, "o_", pico8_value_ref(NULL));

    qa_val = pico8_ui_field_raw(self, "qa");
    je_ra_val = pico8_ui_field_raw(g_je, "ra");

    hd_bucket = pico8_cv("hd");
    if (hd_bucket != NULL) {
        for (i = 0; i < hd_bucket->count; i++) {
            pico8_instance_t *e = hd_bucket->items[i].as.ref;
            pico8_value_t hit_out;

            /* TROU-11 */
            if (pico8_mv(e, "hd", 2, qa_val, je_ra_val, &hit_out) && pico8_ui_truthy(hit_out)) {
                pico8_value_t fa_val;
                if (pico8_entity_get_field(e, "fa", &fa_val) && fa_val.kind == PICO8_VALUE_FIXED) {
                    pico8_ft_index(ic, pico8_ui_to_int(fa_val.as.fixed), pico8_value_ref(e));
                }
                pico8_ob_table_set_name(self->fields, "o_", pico8_value_ref(e));
            }
        }
    }

    gj_val = pico8_ui_field_raw(self, "gj");
    if (gj_val.kind == PICO8_VALUE_REF && gj_val.as.ref != NULL) {
        pico8_entity_set_field(gj_val.as.ref, "m_", pico8_value_bool(0));
    }

    o_val = pico8_ui_field_raw(self, "o_");
    if (o_val.kind == PICO8_VALUE_REF && o_val.as.ref != NULL) {
        pico8_entity_set_field(o_val.as.ref, "m_", pico8_value_bool(1));
    }

    if (o_val.kind == PICO8_VALUE_REF && o_val.as.ref != NULL) {
        pico8_value_t fg_val = pico8_ui_field_raw((const pico8_instance_t *)o_val.as.ref, "fg");
        poke_val = (fg_val.kind == PICO8_VALUE_FIXED) ? pico8_ui_to_int(fg_val.as.fixed) : 0;
    } else {
        int prev = pico8_peek(mem, 0x5f80);
        poke_val = (prev - 1 > 0) ? prev - 1 : 0;
    }
    pico8_poke(mem, 0x5f80, (unsigned char)poke_val);
}

void pico8_ui_bh(pico8_instance_t *self, const char *bt) {
    char ni[16];
    pico8_value_t held_val;
    pico8_instance_t *held_inst = NULL;
    pico8_value_t qa_val;
    pico8_value_t je_ra_val;
    pico8_value_t discard;

    if (self == NULL) {
        return;
    }

    snprintf(ni, sizeof ni, "%sheld", bt);
    qa_val = pico8_ui_field_raw(self, "qa");
    je_ra_val = pico8_ui_field_raw(g_je, "ra");

    if (pico8_ui_je_button_field_bool(bt, "jn")) {
        char down_name[16];
        pico8_instance_t *kv = NULL;
        pico8_value_t kn;

        snprintf(down_name, sizeof down_name, "%sdown", bt);

        if (pico8_ui_jz(self, down_name, 2, qa_val, je_ra_val, &kv, &kn) && kn.kind == PICO8_VALUE_REF) {
            pico8_ob_table_set_name(self->fields, ni, kn);
        }
    }

    held_val = pico8_ui_field_raw(self, ni);
    if (held_val.kind == PICO8_VALUE_REF && held_val.as.ref != NULL) {
        /* TROU-6 */
        held_inst = (pico8_instance_t *)held_val.as.ref;
    }

    pico8_mv(held_inst, "hq", 2, qa_val, je_ra_val, &discard);

    if (pico8_ui_je_button_field_bool(bt, "hm") && held_inst != NULL) {
        pico8_instance_t *nw_kv = NULL;
        pico8_value_t nw_kn;
        pico8_value_t ey_arg = pico8_value_ref(NULL);

        /* `ey`'s argument is the entity `jz` found, not a field read on
         * `held`: no object in this cart carries a "kv" field. */
        if (pico8_ui_jz(self, "gv", 1, held_val, pico8_value_ref(NULL), &nw_kv, &nw_kn) && nw_kv != NULL) {
            ey_arg = pico8_value_ref(nw_kv);
        }

        pico8_mv(held_inst, "ey", 1, ey_arg, pico8_value_ref(NULL), &discard);
        pico8_ob_table_set_name(self->fields, ni, pico8_value_ref(NULL));
    }
}

/* Layers descend from 15 to 0 and the first truthy answer wins: that order
 * is the dispatch priority between overlapping entities, not an arbitrary
 * walk. */
int pico8_ui_jz(pico8_instance_t *self, const char *rm, int argc, pico8_value_t arg0, pico8_value_t arg1,
                pico8_instance_t **out_kv, pico8_value_t *out_kn) {
    int mf;
    pico8_value_t ic_val;
    const pico8_ob_table_t *ic;

    if (self == NULL) {
        return 0;
    }

    if (!pico8_instance_get_field(self, "ic", &ic_val) || ic_val.kind != PICO8_VALUE_REF || ic_val.as.ref == NULL) {
        return 0;
    }
    ic = (const pico8_ob_table_t *)ic_val.as.ref;

    for (mf = 15; mf >= 0; mf--) {
        pico8_value_t bucket_val;
        const pico8_list_t *bucket;
        int i;

        if (!pico8_ob_table_get_index(ic, mf, &bucket_val) || bucket_val.kind != PICO8_VALUE_REF ||
            bucket_val.as.ref == NULL) {
            continue;
        }
        bucket = PICO8_REF_AS_LIST_CONST(bucket_val.as.ref);

        for (i = 0; i < bucket->count; i++) {
            pico8_instance_t *h = bucket->items[i].as.ref;
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
        pico8_he((pico8_instance_t *)e.as.ref);
    }
    return 0;
}

void pico8_ui_cj(pico8_instance_t *self) {
    pico8_value_t le_val;
    pico8_list_t *heap_le;

    if (self == NULL) {
        return;
    }

    if (pico8_instance_get_field(self, "le", &le_val) && le_val.kind == PICO8_VALUE_REF && le_val.as.ref != NULL) {
        pico8_list_t discarded = pico8_nv(PICO8_REF_AS_LIST_CONST(le_val.as.ref), pico8_ui_cj_he_mapper, NULL);
        pico8_list_free(&discarded);
    }

    /* TROU-7 */
    heap_le = malloc(sizeof *heap_le);
    pico8_list_init(heap_le);
    pico8_ob_table_set_name(self->fields, "le", pico8_value_ref(heap_le));
}
