#include "pico8_research.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_data_init.h"
#include "pico8_research_display.h"
#include "pico8_tech_gate.h"
#include "pico8_type_tag.h"

static int g_hp_up = 0;
static pico8_ob_table_t *g_gb = NULL;
/* Borrowed from `fd`'s backing storage, never copied or freed here. */
static const char *g_rr = NULL;

static const char *const g_hp_named_keys[] = {"rr", "ii", "cw", "go", "rn"};

static int value_is_nil(pico8_value_t v) {
    return v.kind == PICO8_VALUE_REF && v.as.ref == NULL;
}

/* Lua truthiness, widened so this port's "absent" filler REF(NULL) is
 * falsy too. */
static int pico8_research_truthy(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) {
        return 0;
    }
    if (v.kind == PICO8_VALUE_REF && v.as.ref == NULL) {
        return 0;
    }
    return 1;
}

/* Returns a new, heap-backed list the caller owns and must release with
 * pico8_list_free(). */
static pico8_list_t pico8_research_positional_values(const pico8_ob_table_t *table) {
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

static void pico8_research_free_gb_instances(void) {
    int i;

    if (g_gb == NULL) {
        return;
    }
    for (i = 0; i < g_gb->count; i++) {
        if (g_gb->entries[i].key.kind == PICO8_OB_KEY_INDEX && g_gb->entries[i].value.kind == PICO8_VALUE_REF &&
            g_gb->entries[i].value.as.ref != NULL) {
            free(g_gb->entries[i].value.as.ref);
        }
    }
}

static pico8_value_t pico8_hp_lq_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_value_ref(pico8_hp_lq((pico8_hp_t *)self));
}

/* A nil `lq` member falls back to the class's `lq` method, as the cart's
 * `mv(b,"lq")` finds it: the member written by the constructor shadows the
 * method for a project with `ii`. An `lq` entry in the descriptor's methods
 * would be looked up first and invert that. */
static int pico8_hp_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_hp_t *hp = self;

    if (strcmp(name, "mr") == 0) {
        if (hp->mr == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref((void *)hp->mr);
        return 1;
    }
    if (strcmp(name, "gp") == 0) {
        if (hp->gp == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref((void *)hp->gp);
        return 1;
    }
    if (strcmp(name, "fk") == 0) {
        if (hp->fk == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref((void *)hp->fk);
        return 1;
    }
    if (strcmp(name, "px") == 0) {
        if (hp->px == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(hp->px);
        return 1;
    }
    if (strcmp(name, "lr") == 0) {
        if (hp->lr == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(hp->lr);
        return 1;
    }
    if (strcmp(name, "fj") == 0) {
        if (!hp->has_fj) {
            return 0;
        }
        *out_value = pico8_value_fixed(hp->fj);
        return 1;
    }
    if (strcmp(name, "ja") == 0) {
        if (hp->ja == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(hp->ja);
        return 1;
    }
    if (strcmp(name, "i") == 0) {
        if (hp->i == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref(hp->i);
        return 1;
    }
    if (strcmp(name, "o") == 0) {
        if (!hp->has_o) {
            return 0;
        }
        *out_value = pico8_value_fixed(hp->o);
        return 1;
    }
    if (strcmp(name, "il") == 0) {
        pico8_value_t v = pico8_hp_il(hp);
        if (value_is_nil(v)) {
            return 0;
        }
        *out_value = v;
        return 1;
    }
    if (strcmp(name, "dm") == 0) {
        if (!hp->has_dm) {
            return 0;
        }
        *out_value = pico8_value_fixed(hp->dm);
        return 1;
    }
    if (strcmp(name, "rr") == 0) {
        if (hp->rr == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(hp->rr);
        return 1;
    }
    if (strcmp(name, "ii") == 0) {
        if (!hp->has_ii) {
            return 0;
        }
        *out_value = pico8_value_fixed(hp->ii);
        return 1;
    }
    if (strcmp(name, "cw") == 0) {
        if (hp->cw == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(hp->cw);
        return 1;
    }
    if (strcmp(name, "go") == 0) {
        if (hp->go == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(hp->go);
        return 1;
    }
    if (strcmp(name, "rn") == 0) {
        if (!hp->has_rn) {
            return 0;
        }
        *out_value = pico8_value_fixed(hp->rn);
        return 1;
    }
    if (strcmp(name, "lq") == 0) {
        if (hp->lq == NULL) {
            *out_value = pico8_value_function(pico8_hp_lq_dispatch);
            return 1;
        }
        *out_value = pico8_value_ref((void *)hp->lq);
        return 1;
    }
    return 0;
}

static const pico8_struct_class_t g_hp_struct_class = {
    {NULL},
    pico8_hp_field,
    NULL,
};

static int hp_accepts_key(pico8_ob_key_t key) {
    size_t i;

    if (key.kind == PICO8_OB_KEY_INDEX) {
        return key.index >= 1 && key.index <= pico8_hp_mr_names.count;
    }
    for (i = 0; i < sizeof g_hp_named_keys / sizeof g_hp_named_keys[0]; i++) {
        if (strcmp(key.name, g_hp_named_keys[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static void hp_take_string(const pico8_ob_table_t *ri, const char *name, const char **out) {
    pico8_value_t v;

    *out = NULL;
    if (!pico8_ob_table_get_name(ri, name, &v) || value_is_nil(v)) {
        return;
    }
    assert(v.kind == PICO8_VALUE_STRING && v.as.string != NULL &&
           "hp.px/lr/ja/rr/cw/go: a key neither nil nor a string does not fit the member's type "
           "-- stop, do not widen the member");
    *out = v.as.string;
}

static void hp_take_fixed(const pico8_ob_table_t *ri, const char *name, pico8_fixed_t *out, int *has) {
    pico8_value_t v;

    *out = 0;
    *has = 0;
    if (!pico8_ob_table_get_name(ri, name, &v) || value_is_nil(v)) {
        return;
    }
    assert(v.kind == PICO8_VALUE_FIXED &&
           "hp.fj/o/dm/ii/rn: a key neither nil nor fixed does not fit the member's type "
           "-- stop, do not widen the member");
    *out = v.as.fixed;
    *has = 1;
}

static void hp_take_il(const pico8_ob_table_t *ri, const char **out, int *has) {
    pico8_value_t v;

    *out = NULL;
    *has = 0;
    if (!pico8_ob_table_get_name(ri, "il", &v) || value_is_nil(v)) {
        return;
    }
    assert(((v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0) ||
            (v.kind == PICO8_VALUE_STRING && v.as.string != NULL)) &&
           "hp.il: a key neither nil, false nor a string does not fit the member's type "
           "-- stop, do not widen the member");
    *out = v.kind == PICO8_VALUE_STRING ? v.as.string : NULL;
    *has = 1;
}

/* Contributes a `{qe=r[c]}` wrapper for every character, never skipping
 * one: the Lua table constructor is non-nil even when `r[c]` is absent. */
static int pico8_hp_pq_i_step(char c, int i, void *ctx, pico8_value_t *out_value) {
    pico8_ob_table_t *wrapper = pico8_ob_table_new();
    pico8_value_t qe = pico8_value_ref(NULL);
    int r_qe;
    char key[2];

    (void)i;
    (void)ctx;
    key[0] = c;
    key[1] = '\0';
    if (pico8_r_lookup(key, &r_qe)) {
        qe = pico8_value_fixed(pico8_fixed_from_int(r_qe));
    }
    pico8_ob_table_set_name(wrapper, "qe", qe);
    *out_value = pico8_value_ref(wrapper);
    return 1;
}

/* TROU-7 */
/* `i` must be rewritten after `ho`, which leaves the raw string in it. `ho`
 * runs on a working copy of the overrides and indexes it into a throwaway
 * `rc`: the real `gb[px]` must hold the struct. */
static void *pico8_hp_struct_new(const pico8_ob_table_t *overrides) {
    pico8_hp_t *self = malloc(sizeof *self);
    pico8_ob_table_t *ri = pico8_ob_table_new();
    pico8_ob_table_t *scratch_rc = pico8_ob_table_new();
    pico8_list_t new_i;
    pico8_list_t *new_i_heap;
    pico8_value_t raw_i;
    const char *i_src = "";
    int k;

    self->header.tag = PICO8_TYPE_TAG_STRUCT;
    self->header.cls = &g_hp_struct_class;
    self->mr = NULL;
    self->gp = NULL;
    self->fk = NULL;
    self->px = NULL;
    self->lr = NULL;
    self->fj = 0;
    self->has_fj = 0;
    self->ja = NULL;
    self->i = NULL;
    self->o = 0;
    self->has_o = 0;
    self->il = NULL;
    self->has_il = 0;
    self->dm = 0;
    self->has_dm = 0;
    self->rr = NULL;
    self->ii = 0;
    self->has_ii = 0;
    self->cw = NULL;
    self->go = NULL;
    self->rn = 0;
    self->has_rn = 0;
    self->lq = NULL;
    self->mr = pico8_hp_start.mr;
    self->gp = pico8_hp_start.gp;
    self->fk = pico8_hp_start.fk;

    for (k = 0; overrides != NULL && k < overrides->count; k++) {
        pico8_ob_key_t key = overrides->entries[k].key;
        assert(hp_accepts_key(key) && "hp: every record of fd passes only positionals 1-8 and rr, ii, cw, go, rn; "
                                      "another key has no member -- stop, do not add one");
        if (key.kind == PICO8_OB_KEY_INDEX) {
            pico8_ob_table_set_index(ri, key.index, overrides->entries[k].value);
        } else {
            pico8_ob_table_set_name(ri, key.name, overrides->entries[k].value);
        }
    }

    pico8_ho_apply(scratch_rc, self->mr, self->gp, ri);
    pico8_ob_table_free(scratch_rc);
    hp_take_string(ri, "px", &self->px);
    hp_take_string(ri, "lr", &self->lr);
    hp_take_fixed(ri, "fj", &self->fj, &self->has_fj);
    hp_take_string(ri, "ja", &self->ja);
    hp_take_fixed(ri, "o", &self->o, &self->has_o);
    hp_take_il(ri, &self->il, &self->has_il);
    hp_take_fixed(ri, "dm", &self->dm, &self->has_dm);
    hp_take_string(ri, "rr", &self->rr);
    hp_take_fixed(ri, "ii", &self->ii, &self->has_ii);
    hp_take_string(ri, "cw", &self->cw);
    hp_take_string(ri, "go", &self->go);
    hp_take_fixed(ri, "rn", &self->rn, &self->has_rn);
    if (!pico8_ob_table_get_name(ri, "i", &raw_i)) {
        raw_i = pico8_value_ref(NULL);
    }
    pico8_ob_table_free(ri);

    if (self->px != NULL) {
        assert(g_gb != NULL && "hp:pq registers itself in gb: build gb before instantiating an hp");
        pico8_ob_table_set_name(g_gb, self->px, pico8_value_ref(self));
    }

    if (self->has_ii) {
        assert(self->px != NULL && "hp.lq: {px, 32, {ii, 8}} has no hole; a project with ii and no px "
                                   "does not fit the row -- stop, do not widen the member");
        self->lq_inner_items[0] = (pico8_jb_item_t){.is_fixed = 1, .fixed = self->ii};
        self->lq_inner_items[1] = (pico8_jb_item_t){.n = 8};
        self->lq_inner = (pico8_jb_row_t){.count = 2, .items = self->lq_inner_items};
        self->lq_own_items[0] = (pico8_jb_item_t){.s = self->px};
        self->lq_own_items[1] = (pico8_jb_item_t){.n = 32};
        self->lq_own_items[2] = (pico8_jb_item_t){.row = &self->lq_inner};
        self->lq_own = (pico8_jb_row_t){.count = 3, .items = self->lq_own_items};
        self->lq = &self->lq_own;
    }

    if (raw_i.kind == PICO8_VALUE_STRING && raw_i.as.string != NULL) {
        i_src = raw_i.as.string;
    }
    new_i = pico8_fi(i_src, pico8_hp_pq_i_step, NULL);
    new_i_heap = malloc(sizeof *new_i_heap);
    *new_i_heap = new_i;
    self->i = new_i_heap;

#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("hp");
#endif
    return self;
}

pico8_value_t pico8_hp_il(const pico8_hp_t *self) {
    if (self == NULL || !self->has_il) {
        return pico8_value_ref(NULL);
    }
    if (self->il == NULL) {
        return pico8_value_bool(0);
    }
    return pico8_value_string(self->il);
}

void pico8_hp_assert(const pico8_hp_t *self) {
    assert((self == NULL ||
            (pico8_type_tag_of(self) == PICO8_TYPE_TAG_STRUCT && self->header.cls == &g_hp_struct_class)) &&
           "hp: every gb entry, every planet's hn and every row's project is an hp; anything else would be read "
           "through the wrong layout");
    (void)self;
}

/* A root class: no parent, so it never self-registers into hv/cv/cg. */
void pico8_research_init(void) {
    pico8_research_shutdown();
    g_hp_up = 1;
}

void pico8_research_shutdown(void) {
    pico8_research_free_gb_instances();
    pico8_ob_table_free(g_gb);
    g_gb = NULL;
    g_rr = NULL;
    g_hp_up = 0;
}

int pico8_hp_ready(void) {
    return g_hp_up;
}

const pico8_struct_class_t *pico8_hp_struct_class(void) {
    return &g_hp_struct_class;
}

const pico8_jb_row_t *pico8_hp_fk(void) {
    return pico8_hp_start.fk;
}

pico8_hp_t *pico8_hp_new(const pico8_ob_table_t *overrides) {
    return pico8_hp_struct_new(overrides);
}

/* `fd` itself is never mutated here, only its nested per-record tables,
 * through `ri.rr=rr`. */
void pico8_research_build_gb(const pico8_ob_table_t *fd) {
    pico8_list_t items;
    pico8_list_iter_t it;
    pico8_value_t entry;
    int next_index = 1;

    pico8_research_free_gb_instances();
    pico8_ob_table_free(g_gb);
    g_gb = pico8_ob_table_new();
    g_rr = NULL;

    items = pico8_research_positional_values(fd);
    it = pico8_list_all(&items);
    while (pico8_list_iter_next(&it, &entry)) {
        if (entry.kind == PICO8_VALUE_STRING) {
            g_rr = entry.as.string;
        } else if (entry.kind == PICO8_VALUE_REF && entry.as.ref != NULL) {
            pico8_ob_table_t *ri = entry.as.ref;
            pico8_hp_t *inst;

            pico8_ob_table_set_name(ri, "rr", g_rr != NULL ? pico8_value_string(g_rr) : pico8_value_ref(NULL));
            inst = pico8_hp_new(ri);
            pico8_ob_table_set_index(g_gb, next_index++, pico8_value_ref(inst));
        }
    }
    pico8_list_free(&items);
}

const pico8_ob_table_t *pico8_gb(void) {
    return g_gb;
}

const pico8_hc_entry_t *pico8_hp_po(const pico8_hp_t *self) {
    pico8_value_t il;
    const pico8_hc_entry_t *qp = NULL;

    pico8_hp_assert(self);
    if (self == NULL) {
        return NULL;
    }
    if (self->ja != NULL) {
        qp = pico8_hc_lookup(self->ja);
    }

    il = pico8_hp_il(self);
    if (pico8_research_truthy(il) && pico8_oj(il) && qp != NULL && qp->mc != NULL) {
        const pico8_hc_entry_t *hc_mc = pico8_hc_lookup(qp->mc);

        if (hc_mc != NULL) {
            return hc_mc;
        }
    }
    return qp;
}

pico8_fixed_t pico8_hp_od(const pico8_hp_t *self) {
    pico8_fixed_t od = pico8_fixed_from_int(0);
    pico8_value_t il;
    int has_ii;

    pico8_hp_assert(self);
    il = pico8_hp_il(self);
    if (self != NULL && self->has_fj) {
        od = self->fj;
    }
    if (self != NULL && pico8_research_truthy(il) && pico8_oj(il)) {
        od = pico8_fixed_mul(od, PICO8_FIXED_LIT(0x00018000, 1.5));
    }
    has_ii = self != NULL && self->has_ii;
    if (!has_ii && pico8_oj(pico8_value_string("nanomaterials"))) {
        od = pico8_fixed_mul(od, PICO8_FIXED_LIT(0x0000d999, 0.85));
    }
    return pico8_fixed_flr(od);
}
