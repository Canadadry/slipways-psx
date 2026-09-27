#include "pico8_research.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_collect.h"
#include "pico8_data_init.h"
#include "pico8_research_display.h"
#include "pico8_stash.h"
#include "pico8_tech_gate.h"
#include "pico8_type_tag.h"

static pico8_ob_table_t *g_hp_spec = NULL;
static pico8_ob_table_t *g_gb = NULL;
/* Borrowed from `fd`'s backing storage, never copied or freed here. */
static const char *g_rr = NULL;

/* `hp.mr`, `hp.gp`, `hp.fk`, read once from hp's spec at init: the spec's
 * REFs, never re-parsed. Nil outside init/shutdown. */
static pico8_value_t g_mr = {PICO8_VALUE_REF, {0}};
static pico8_value_t g_gp = {PICO8_VALUE_REF, {0}};
static pico8_value_t g_fk = {PICO8_VALUE_REF, {0}};

static const char *const g_hp_mr_names[] = {"px", "lr", "fj", "ja", "i", "o", "il", "dm"};
static const char *const g_hp_named_keys[] = {"rr", "ii", "cw", "go", "rn"};

static const struct {
    const char *name;
    size_t offset;
} g_hp_members[] = {
    {"mr", offsetof(pico8_hp_t, mr)}, {"gp", offsetof(pico8_hp_t, gp)}, {"fk", offsetof(pico8_hp_t, fk)},
    {"px", offsetof(pico8_hp_t, px)}, {"lr", offsetof(pico8_hp_t, lr)}, {"fj", offsetof(pico8_hp_t, fj)},
    {"ja", offsetof(pico8_hp_t, ja)}, {"i", offsetof(pico8_hp_t, i)},   {"o", offsetof(pico8_hp_t, o)},
    {"il", offsetof(pico8_hp_t, il)}, {"dm", offsetof(pico8_hp_t, dm)}, {"rr", offsetof(pico8_hp_t, rr)},
    {"ii", offsetof(pico8_hp_t, ii)}, {"cw", offsetof(pico8_hp_t, cw)}, {"go", offsetof(pico8_hp_t, go)},
    {"rn", offsetof(pico8_hp_t, rn)}, {"lq", offsetof(pico8_hp_t, lq)},
};

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

static pico8_value_t *hp_member(pico8_hp_t *self, const char *name) {
    size_t i;
    for (i = 0; i < sizeof g_hp_members / sizeof g_hp_members[0]; i++) {
        if (strcmp(name, g_hp_members[i].name) == 0) {
            return (pico8_value_t *)((char *)self + g_hp_members[i].offset);
        }
    }
    return NULL;
}

/* A nil `lq` member falls back to the class's `lq` method, as the cart's
 * `mv(b,"lq")` finds it: the member written by the constructor shadows the
 * method for a project with `ii`. An `lq` entry in the descriptor's methods
 * would be looked up first and invert that. */
static int pico8_hp_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_value_t *member = hp_member((pico8_hp_t *)self, name);

    if (member == NULL) {
        return 0;
    }
    if (value_is_nil(*member)) {
        if (strcmp(name, "lq") == 0) {
            *out_value = pico8_value_function(pico8_hp_lq_dispatch);
            return 1;
        }
        return 0;
    }
    *out_value = *member;
    return 1;
}

static const pico8_struct_class_t g_hp_struct_class = {
    {NULL},
    pico8_hp_field,
    NULL,
};

static int hp_accepts_key(pico8_ob_key_t key) {
    size_t i;

    if (key.kind == PICO8_OB_KEY_INDEX) {
        return key.index >= 1 && key.index <= (int)(sizeof g_hp_mr_names / sizeof g_hp_mr_names[0]);
    }
    for (i = 0; i < sizeof g_hp_named_keys / sizeof g_hp_named_keys[0]; i++) {
        if (strcmp(key.name, g_hp_named_keys[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Contributes a `{qe=r[c]}` wrapper for every character, never skipping
 * one: the Lua table constructor is non-nil even when `r[c]` is absent. */
static int pico8_hp_pq_i_step(char c, int i, void *ctx, pico8_value_t *out_value) {
    pico8_ob_table_t *wrapper = pico8_ob_table_new();
    pico8_value_t qe;
    char key[2];

    (void)i;
    (void)ctx;
    key[0] = c;
    key[1] = '\0';
    if (!pico8_ob_table_get_name(pico8_r(), key, &qe)) {
        qe = pico8_value_ref(NULL);
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
    pico8_list_t gp_list;
    pico8_list_t new_i;
    pico8_list_t *new_i_heap;
    const char *i_src = "";
    size_t n;
    int k;

    self->header.tag = PICO8_TYPE_TAG_STRUCT;
    self->header.cls = &g_hp_struct_class;
    for (n = 0; n < sizeof g_hp_members / sizeof g_hp_members[0]; n++) {
        *hp_member(self, g_hp_members[n].name) = pico8_value_ref(NULL);
    }
    self->mr = g_mr;
    self->gp = g_gp;
    self->fk = g_fk;

    for (k = 0; overrides != NULL && k < overrides->count; k++) {
        pico8_ob_key_t key = overrides->entries[k].key;
        assert(hp_accepts_key(key) && "hp: every measured record of fd passes only positionals 1-8 and rr, ii, cw, "
                                      "go, rn; another key would be a field under no rule of plan-3 section 5 -- "
                                      "stop, do not add a member");
        if (key.kind == PICO8_OB_KEY_INDEX) {
            pico8_ob_table_set_index(ri, key.index, overrides->entries[k].value);
        } else {
            pico8_ob_table_set_name(ri, key.name, overrides->entries[k].value);
        }
    }

    gp_list = pico8_research_positional_values(g_gp.as.ref);
    pico8_ho_apply(scratch_rc, g_mr.as.ref, &gp_list, ri);
    pico8_ob_table_free(scratch_rc);
    pico8_list_free(&gp_list);
    for (n = 0; n < sizeof g_hp_mr_names / sizeof g_hp_mr_names[0]; n++) {
        (void)pico8_ob_table_get_name(ri, g_hp_mr_names[n], hp_member(self, g_hp_mr_names[n]));
    }
    for (n = 0; n < sizeof g_hp_named_keys / sizeof g_hp_named_keys[0]; n++) {
        (void)pico8_ob_table_get_name(ri, g_hp_named_keys[n], hp_member(self, g_hp_named_keys[n]));
    }
    pico8_ob_table_free(ri);

    if (self->px.kind == PICO8_VALUE_STRING) {
        assert(g_gb != NULL && "hp:pq registers itself in gb: build gb before instantiating an hp");
        pico8_ob_table_set_name(g_gb, self->px.as.string, pico8_value_ref(self));
    }

    if (pico8_research_truthy(self->ii)) {
        pico8_ob_table_t *inner = pico8_ob_table_new();
        pico8_ob_table_t *outer = pico8_ob_table_new();

        pico8_ob_table_set_index(inner, 1, self->ii);
        pico8_ob_table_set_index(inner, 2, pico8_value_fixed(pico8_fixed_from_int(8)));
        if (!value_is_nil(self->px)) {
            pico8_ob_table_set_index(outer, 1, self->px);
        }
        pico8_ob_table_set_index(outer, 2, pico8_value_fixed(pico8_fixed_from_int(32)));
        pico8_ob_table_set_index(outer, 3, pico8_value_ref(inner));
        self->lq = pico8_value_ref(outer);
    }

    if (self->i.kind == PICO8_VALUE_STRING && self->i.as.string != NULL) {
        i_src = self->i.as.string;
    }
    new_i = pico8_fi(i_src, pico8_hp_pq_i_step, NULL);
    new_i_heap = malloc(sizeof *new_i_heap);
    *new_i_heap = new_i;
    self->i = pico8_value_ref(new_i_heap);

#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("hp");
#endif
    return self;
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
    const struct {
        const char *name;
        pico8_value_t *value;
    } defaults[] = {{"mr", &g_mr}, {"gp", &g_gp}, {"fk", &g_fk}};
    const pico8_ob_table_t *mr;
    size_t n;

    pico8_research_shutdown();
    g_hp_spec = pico8_ob("\nmr=o(\"px\",\"lr\",\"fj\",\"ja\",\"i\",\"o\",\"il\",\"dm\"),\n"
                         "gp=o(\"o\",\"dm\"),\n"
                         "fk=o(f,2),\n",
                         NULL);

    for (n = 0; n < sizeof defaults / sizeof defaults[0]; n++) {
        int found = pico8_hp_default(defaults[n].name, defaults[n].value);
        assert(found && defaults[n].value->kind == PICO8_VALUE_REF && defaults[n].value->as.ref != NULL &&
               pico8_type_tag_of(defaults[n].value->as.ref) == PICO8_TYPE_TAG_OB_TABLE &&
               "hp: mr, gp, fk are tables of hp's spec, read by every construction and by lq");
        (void)found;
    }
    mr = g_mr.as.ref;
    (void)mr;
    assert(mr->count == (int)(sizeof g_hp_mr_names / sizeof g_hp_mr_names[0]) &&
           "hp: the members ho fills are exactly hp.mr's names, in its order; the constructor copies them by name");
    for (n = 0; n < sizeof g_hp_mr_names / sizeof g_hp_mr_names[0]; n++) {
        assert(mr->entries[n].key.kind == PICO8_OB_KEY_INDEX && mr->entries[n].key.index == (int)n + 1 &&
               mr->entries[n].value.kind == PICO8_VALUE_STRING &&
               strcmp(mr->entries[n].value.as.string, g_hp_mr_names[n]) == 0 &&
               "hp: the members ho fills are exactly hp.mr's names, in its order; the constructor copies them by "
               "name");
    }
}

void pico8_research_shutdown(void) {
    pico8_research_free_gb_instances();
    pico8_ob_table_free(g_gb);
    g_gb = NULL;
    g_rr = NULL;
    g_mr = pico8_value_ref(NULL);
    g_gp = pico8_value_ref(NULL);
    g_fk = pico8_value_ref(NULL);
    pico8_ob_table_free(g_hp_spec);
    g_hp_spec = NULL;
}

int pico8_hp_ready(void) {
    return g_hp_spec != NULL;
}

int pico8_hp_default(const char *name, pico8_value_t *out_value) {
    return g_hp_spec != NULL && pico8_ob_table_get_name(g_hp_spec, name, out_value);
}

const pico8_struct_class_t *pico8_hp_struct_class(void) {
    return &g_hp_struct_class;
}

pico8_value_t pico8_hp_fk(void) {
    return g_fk;
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

/* `hc[key]`, where the key's Lua type is only known at runtime: a FIXED
 * key indexes, a STRING key names. */
static int pico8_research_lookup(const pico8_ob_table_t *table, pico8_value_t key, pico8_value_t *out) {
    if (table == NULL) {
        return 0;
    }
    if (key.kind == PICO8_VALUE_FIXED) {
        return pico8_ob_table_get_index(table, pico8_fixed_flr_int(key.as.fixed), out);
    }
    if (key.kind == PICO8_VALUE_STRING && key.as.string != NULL) {
        return pico8_ob_table_get_name(table, key.as.string, out);
    }
    return 0;
}

const pico8_ob_table_t *pico8_hp_po(const pico8_hp_t *self) {
    pico8_value_t qp_val;
    const pico8_ob_table_t *qp = NULL;

    pico8_hp_assert(self);
    if (self == NULL) {
        return NULL;
    }
    if (pico8_research_lookup(pico8_hc(), self->ja, &qp_val) && qp_val.kind == PICO8_VALUE_REF) {
        qp = qp_val.as.ref;
    }

    if (pico8_research_truthy(self->il) && pico8_oj(self->il) && qp != NULL) {
        pico8_value_t mc_val;

        if (pico8_ob_table_get_name(qp, "mc", &mc_val)) {
            pico8_value_t hc_mc;

            if (pico8_research_lookup(pico8_hc(), mc_val, &hc_mc) && hc_mc.kind == PICO8_VALUE_REF) {
                return hc_mc.as.ref;
            }
        }
    }
    return qp;
}

pico8_fixed_t pico8_hp_od(const pico8_hp_t *self) {
    pico8_fixed_t od = pico8_fixed_from_int(0);
    int has_ii;

    pico8_hp_assert(self);
    if (self != NULL && self->fj.kind == PICO8_VALUE_FIXED) {
        od = self->fj.as.fixed;
    }
    if (self != NULL && pico8_research_truthy(self->il) && pico8_oj(self->il)) {
        od = pico8_fixed_mul(od, PICO8_FIXED_LIT(0x00018000, 1.5));
    }
    has_ii = self != NULL && pico8_research_truthy(self->ii);
    if (!has_ii && pico8_oj(pico8_value_string("nanomaterials"))) {
        od = pico8_fixed_mul(od, PICO8_FIXED_LIT(0x0000d999, 0.85));
    }
    return pico8_fixed_flr(od);
}
