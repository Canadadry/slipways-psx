#include "pico8_ui.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_ke.h"
#include "pico8_list.h"
#include "pico8_pi.h"
#include "pico8_type_tag.h"
#include "pico8_vec.h"
#include "pico8_ref_audit.h"

static pico8_ob_table_t *g_ui_spec = NULL;
static pico8_instance_t *g_je = NULL;
static pico8_ui_t *g_ok = NULL;

enum { UI_QL, UI_FA, UI_LE, UI_HW };

/* ui's 4 spec defaults, read once from its spec at init, after the swap of
 * `le`: the spec's REFs, never re-parsed. */
static struct {
    const char *name;
    size_t offset;
    pico8_value_t value;
} g_ui_defaults[] = {
    [UI_QL] = {"ql", offsetof(pico8_ui_t, ql), {0}},
    [UI_FA] = {"fa", offsetof(pico8_ui_t, fa), {0}},
    [UI_LE] = {"le", offsetof(pico8_ui_t, le), {0}},
    [UI_HW] = {"hw", offsetof(pico8_ui_t, hw), {0}},
};

static void pico8_ui_defaults_reset(void) {
    size_t i;
    for (i = 0; i < sizeof g_ui_defaults / sizeof g_ui_defaults[0]; i++) {
        g_ui_defaults[i].value = pico8_value_ref(NULL);
    }
}

const pico8_qz_box_t *pico8_ui_hw(void) {
    return (const pico8_qz_box_t *)g_ui_defaults[UI_HW].value.as.ref;
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
    pico8_ui_ev(pico8_dispatch_gfx(), pico8_dispatch_mem(), (const pico8_ui_t *)self);
    return pico8_value_ref(NULL);
}

static int pico8_ui_field(const void *self, const char *name, pico8_value_t *out_value) {
    static const struct {
        const char *name;
        size_t offset;
    } members[] = {
        {"ql", offsetof(pico8_ui_t, ql)}, {"fa", offsetof(pico8_ui_t, fa)},
        {"le", offsetof(pico8_ui_t, le)}, {"hw", offsetof(pico8_ui_t, hw)},
        {"qa", offsetof(pico8_ui_t, qa)}, {"ic", offsetof(pico8_ui_t, ic)},
        {"o_", offsetof(pico8_ui_t, o_)}, {"gj", offsetof(pico8_ui_t, gj)},
        {"lheld", offsetof(pico8_ui_t, lheld)}, {"rheld", offsetof(pico8_ui_t, rheld)},
    };
    const pico8_value_t *member;
    size_t i;

    for (i = 0; i < sizeof members / sizeof members[0]; i++) {
        if (strcmp(name, members[i].name) == 0) {
            break;
        }
    }
    if (i == sizeof members / sizeof members[0]) {
        return pico8_ke_field(self, name, out_value);
    }
    member = (const pico8_value_t *)((const char *)self + members[i].offset);
    if (member->kind == PICO8_VALUE_REF && member->as.ref == NULL) {
        return 0;
    }
    *out_value = *member;
    return 1;
}

static const pico8_struct_class_t g_ui_struct_class = {
    {
        [PICO8_METHOD_EV] = pico8_ui_ev_dispatch,
    },
    pico8_ui_field,
    NULL,
};

static void *pico8_ui_struct_new(const pico8_ob_table_t *overrides) {
    pico8_ui_t *self;
    size_t i;

    assert(g_ui_spec != NULL && "pico8_ui_init() must run first");
    self = malloc(sizeof *self);
    pico8_ke_struct_init(&self->ke, &g_ui_struct_class);
    for (i = 0; i < sizeof g_ui_defaults / sizeof g_ui_defaults[0]; i++) {
        *(pico8_value_t *)((char *)self + g_ui_defaults[i].offset) = g_ui_defaults[i].value;
    }
    self->qa = pico8_value_ref(NULL);
    self->ic = pico8_value_ref(NULL);
    self->o_ = pico8_value_ref(NULL);
    self->gj = pico8_value_ref(NULL);
    self->lheld = pico8_value_ref(NULL);
    self->rheld = pico8_value_ref(NULL);
    assert((overrides == NULL || overrides->count == 0) &&
           "ui: the only measured road is pico8_ui_new(NULL) (jl); a construction key would be a field under "
           "no rule of plan-3 section 5 -- stop, do not add a member");
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("ui");
#endif
    pico8_ke_struct_pq(&self->ke);
    return self;
}

void pico8_ui_init(void) {
    pico8_ob_table_t *raw_le = NULL;
    pico8_value_t le_val;
    pico8_list_t *heap_le;
    size_t i;

    pico8_ui_shutdown();
    pico8_pi_init();

    g_ui_spec = pico8_ob("\nql=58,fa=15,le=o(),hw=b(10,10,118,118),\n", NULL);

    /* `gu()`/`nv()` need a list, not the table `ob()` parses for `le=o()`.
     * The raw table is freed here: nothing else ever sees it. */
    if (pico8_ob_table_get_name(g_ui_spec, "le", &le_val) && le_val.kind == PICO8_VALUE_REF) {
        raw_le = (pico8_ob_table_t *)le_val.as.ref;
    }
    heap_le = malloc(sizeof *heap_le);
    pico8_list_init(heap_le);
    pico8_ob_table_set_name(g_ui_spec, "le", pico8_value_ref(heap_le));
    pico8_ob_table_free(raw_le);
    for (i = 0; i < sizeof g_ui_defaults / sizeof g_ui_defaults[0]; i++) {
        int found = pico8_ui_default(g_ui_defaults[i].name, &g_ui_defaults[i].value);
        assert(found && "ui: every default of the struct is a key of ui's spec");
        (void)found;
    }
    assert(g_ui_defaults[UI_LE].value.kind == PICO8_VALUE_REF &&
           pico8_type_tag_of(g_ui_defaults[UI_LE].value.as.ref) == PICO8_TYPE_TAG_LIST &&
           g_ui_defaults[UI_HW].value.kind == PICO8_VALUE_REF && g_ui_defaults[UI_HW].value.as.ref != NULL &&
           "ui: lj/cj read le as a list, shared by every new ui until its cj or pz; pico8_ui_hw() casts hw to a box");

    g_je = (pico8_instance_t *)&g_je_placeholder;
}

void pico8_ui_shutdown(void) {
    pico8_ui_defaults_reset();
    pico8_ob_table_free(g_ui_spec);
    g_ui_spec = NULL;
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
    return g_ui_spec != NULL;
}

int pico8_ui_default(const char *name, pico8_value_t *out_value) {
    return g_ui_spec != NULL &&
           (pico8_ob_table_get_name(g_ui_spec, name, out_value) || pico8_ke_default(name, out_value));
}

const pico8_struct_class_t *pico8_ui_struct_class(void) {
    return &g_ui_struct_class;
}

pico8_ui_t *pico8_ui_new(const pico8_ob_table_t *overrides) {
    return pico8_ui_struct_new(overrides);
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
    void *le_ptr_before = NULL;
    int le_count;
    pico8_vec_t qa;
    pico8_vec_t *heap_qa;
    int hit;

    if (self == NULL) {
        return;
    }

    if (self->le.kind == PICO8_VALUE_REF && self->le.as.ref != NULL) {
        le_ptr_before = self->le.as.ref;
        le_count = PICO8_REF_AS_LIST_CONST(self->le.as.ref)->count;
    } else {
        le_count = 0;
    }

    qa = pico8_vec_add(pico8_ui_field_vec(g_je, "ra"), pico8_ui_field_vec(pico8_mo(), "ra"));
    /* TROU-7 */
    heap_qa = malloc(sizeof *heap_qa);
    *heap_qa = qa;
    self->qa = pico8_value_ref(heap_qa);

    pico8_ui_gw(mem, self);

    hit = (le_count == 0) || pico8_gu(PICO8_REF_AS_LIST_CONST(le_ptr_before), self->o_);
    if (hit) {
        pico8_ui_bh(self, "l");
    }

    pico8_ui_bh(self, "r");

    if (pico8_ui_je_button_field_bool("l", "jn") && self->le.kind == PICO8_VALUE_REF &&
        self->le.as.ref == le_ptr_before) {
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

    if (pico8_ui_truthy(self->rheld)) {
        spr_id = 9;
    } else {
        if (self->o_.kind == PICO8_VALUE_REF && self->o_.as.ref != NULL) {
            pico8_value_t ql_val = pico8_ui_field_raw((const pico8_instance_t *)self->o_.as.ref, "ql");
            if (ql_val.kind != PICO8_VALUE_FIXED) {
                ql_val = self->ql;
            }
            spr_id = (ql_val.kind == PICO8_VALUE_FIXED) ? pico8_ui_to_int(ql_val.as.fixed) : 0;
        } else {
            pico8_value_t ql_val = self->ql;
            spr_id = (ql_val.kind == PICO8_VALUE_FIXED) ? pico8_ui_to_int(ql_val.as.fixed) : 0;
        }
    }

    je_ra = pico8_ui_field_vec(g_je, "ra");
    pico8_spr_default(gfx, mem, spr_id, pico8_ui_to_int(je_ra.x), pico8_ui_to_int(je_ra.y));
}

/* Lua's `(self.gj or {}).m_=false` writes onto a throwaway table when the
 * field is absent, with no observable effect, so the write is simply
 * skipped here rather than allocating a table nothing reads. */
void pico8_ui_gw(pico8_mem_t *mem, pico8_ui_t *self) {
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
    self->ic = pico8_value_ref(ic);
    self->o_ = pico8_value_ref(NULL);

    qa_val = self->qa;
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
                self->o_ = pico8_value_ref(e);
            }
        }
    }

    gj_val = self->gj;
    if (gj_val.kind == PICO8_VALUE_REF && gj_val.as.ref != NULL) {
        pico8_entity_set_field(gj_val.as.ref, "m_", pico8_value_bool(0));
    }

    o_val = self->o_;
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

void pico8_ui_bh(pico8_ui_t *self, const char *bt) {
    pico8_value_t *held;
    const char *down;
    pico8_value_t held_val;
    pico8_instance_t *held_inst = NULL;
    pico8_value_t qa_val;
    pico8_value_t je_ra_val;
    pico8_value_t discard;

    if (self == NULL) {
        return;
    }

    assert((strcmp(bt, "l") == 0 || strcmp(bt, "r") == 0) &&
           "ui:bh: plan-3 section 5 enumerates bt as l or r only; another key is an unmeasured field -- stop");
    if (bt[0] == 'l') {
        held = &self->lheld;
        down = "ldown";
    } else {
        held = &self->rheld;
        down = "rdown";
    }
    qa_val = self->qa;
    je_ra_val = pico8_ui_field_raw(g_je, "ra");

    if (pico8_ui_je_button_field_bool(bt, "jn")) {
        pico8_instance_t *kv = NULL;
        pico8_value_t kn;

        if (pico8_ui_jz(self, down, 2, qa_val, je_ra_val, &kv, &kn) && kn.kind == PICO8_VALUE_REF) {
            *held = kn;
        }
    }

    held_val = *held;
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
        *held = pico8_value_ref(NULL);
    }
}

/* Layers descend from 15 to 0 and the first truthy answer wins: that order
 * is the dispatch priority between overlapping entities, not an arbitrary
 * walk. */
int pico8_ui_jz(pico8_ui_t *self, const char *rm, int argc, pico8_value_t arg0, pico8_value_t arg1,
                pico8_instance_t **out_kv, pico8_value_t *out_kn) {
    int mf;
    const pico8_ob_table_t *ic;

    if (self == NULL) {
        return 0;
    }

    if (self->ic.kind != PICO8_VALUE_REF || self->ic.as.ref == NULL) {
        return 0;
    }
    ic = (const pico8_ob_table_t *)self->ic.as.ref;

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

void pico8_ui_cj(pico8_ui_t *self) {
    pico8_list_t *heap_le;

    if (self == NULL) {
        return;
    }

    if (self->le.kind == PICO8_VALUE_REF && self->le.as.ref != NULL) {
        pico8_list_t discarded = pico8_nv(PICO8_REF_AS_LIST_CONST(self->le.as.ref), pico8_ui_cj_he_mapper, NULL);
        pico8_list_free(&discarded);
    }

    /* TROU-7 */
    heap_le = malloc(sizeof *heap_le);
    pico8_list_init(heap_le);
    self->le = pico8_value_ref(heap_le);
}
