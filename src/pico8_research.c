#include "pico8_research.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "pico8_collect.h"
#include "pico8_data_init.h"
#include "pico8_research_display.h"
#include "pico8_stash.h"
#include "pico8_tech_gate.h"

static pico8_class_t *g_hp_class = NULL;
static pico8_ob_table_t *g_gb = NULL;
/* Borrowed from `fd`'s backing storage, never copied or freed here. */
static const char *g_rr = NULL;

static void pico8_hp_pq(pico8_instance_t *self);

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
            pico8_instance_free(g_gb->entries[i].value.as.ref);
        }
    }
}

static pico8_value_t pico8_hp_lq_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_value_ref(pico8_hp_lq((pico8_instance_t *)self));
}

/* A root class: no parent, so it never self-registers into hv/cv/cg. */
void pico8_research_init(void) {
    pico8_research_shutdown();
    g_hp_class =
        pico8_class_declare(NULL, "\nmr=o(\"px\",\"lr\",\"fj\",\"ja\",\"i\",\"o\",\"il\",\"dm\"),\n"
                                   "gp=o(\"o\",\"dm\"),\n"
                                   "fk=o(f,2),\n");
    PICO8_CLASS_COVERAGE_NAME(g_hp_class, "hp");
    pico8_class_set_pq(g_hp_class, pico8_hp_pq);
    pico8_ob_table_set_name(g_hp_class->defaults, "lq", pico8_value_function(pico8_hp_lq_dispatch));
}

void pico8_research_shutdown(void) {
    pico8_research_free_gb_instances();
    pico8_ob_table_free(g_gb);
    g_gb = NULL;
    g_rr = NULL;
    if (g_hp_class != NULL) {
        pico8_class_free(g_hp_class);
        g_hp_class = NULL;
    }
}

pico8_class_t *pico8_hp_class(void) {
    return g_hp_class;
}

pico8_instance_t *pico8_hp_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_hp_class, overrides);
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
/* pico8_ho_apply also indexes the record into `rc`, so it gets a throwaway
 * one here: the real `gb[px]=self` must store the instance, not its bare
 * fields table. */
static void pico8_hp_pq(pico8_instance_t *self) {
    pico8_value_t mr_val;
    pico8_value_t gp_val;
    const pico8_ob_table_t *mr = NULL;
    pico8_list_t gp_list;
    pico8_value_t ii_val;
    pico8_value_t i_val;
    pico8_list_t new_i;
    pico8_list_t *new_i_heap;
    const char *i_src = "";

    if (self == NULL || g_hp_class == NULL) {
        return;
    }

    if (pico8_ob_table_get_name(g_hp_class->defaults, "mr", &mr_val) && mr_val.kind == PICO8_VALUE_REF) {
        mr = mr_val.as.ref;
    }
    pico8_list_init(&gp_list);
    if (pico8_ob_table_get_name(g_hp_class->defaults, "gp", &gp_val) && gp_val.kind == PICO8_VALUE_REF) {
        gp_list = pico8_research_positional_values(gp_val.as.ref);
    }
    {
        pico8_ob_table_t *scratch_rc = pico8_ob_table_new();
        pico8_value_t px_val;

        pico8_ho_apply(scratch_rc, mr, &gp_list, self->fields);
        pico8_ob_table_free(scratch_rc);

        if (pico8_ob_table_get_name(self->fields, "px", &px_val) && px_val.kind == PICO8_VALUE_STRING) {
            assert(g_gb != NULL && "hp:pq registers itself in gb: build gb before instantiating an hp");
            pico8_ob_table_set_name(g_gb, px_val.as.string, pico8_value_ref(self));
        }
    }
    pico8_list_free(&gp_list);

    /* This "lq" is a DATA field, and it shadows the class-level "lq" method
     * slot: for an `ii`-bearing project, `mv(b,"lq")` in `jo:fe` returns
     * this table instead of calling pico8_hp_lq. The collision is the
     * cart's, not a porting slip. */
    if (pico8_instance_get_field(self, "ii", &ii_val) && pico8_research_truthy(ii_val)) {
        pico8_value_t px_val;
        pico8_ob_table_t *inner = pico8_ob_table_new();
        pico8_ob_table_t *outer = pico8_ob_table_new();

        pico8_ob_table_set_index(inner, 1, ii_val);
        pico8_ob_table_set_index(inner, 2, pico8_value_fixed(pico8_fixed_from_int(8)));
        if (pico8_instance_get_field(self, "px", &px_val)) {
            pico8_ob_table_set_index(outer, 1, px_val);
        }
        pico8_ob_table_set_index(outer, 2, pico8_value_fixed(pico8_fixed_from_int(32)));
        pico8_ob_table_set_index(outer, 3, pico8_value_ref(inner));
        pico8_ob_table_set_name(self->fields, "lq", pico8_value_ref(outer));
    }

    if (pico8_instance_get_field(self, "i", &i_val) && i_val.kind == PICO8_VALUE_STRING && i_val.as.string != NULL) {
        i_src = i_val.as.string;
    }
    new_i = pico8_fi(i_src, pico8_hp_pq_i_step, NULL);
    new_i_heap = malloc(sizeof *new_i_heap);
    *new_i_heap = new_i;
    pico8_ob_table_set_name(self->fields, "i", pico8_value_ref(new_i_heap));
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
            pico8_instance_t *inst;

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

const pico8_ob_table_t *pico8_hp_po(const pico8_instance_t *self) {
    pico8_value_t ja_val;
    pico8_value_t qp_val;
    const pico8_ob_table_t *qp = NULL;
    pico8_value_t il_val;

    if (self == NULL) {
        return NULL;
    }
    if (pico8_instance_get_field(self, "ja", &ja_val) && pico8_research_lookup(pico8_hc(), ja_val, &qp_val) &&
        qp_val.kind == PICO8_VALUE_REF) {
        qp = qp_val.as.ref;
    }

    if (pico8_instance_get_field(self, "il", &il_val) && pico8_research_truthy(il_val) && pico8_oj(il_val) &&
        qp != NULL) {
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

pico8_fixed_t pico8_hp_od(const pico8_instance_t *self) {
    pico8_value_t fj_val;
    pico8_value_t il_val;
    pico8_value_t ii_val;
    pico8_fixed_t od = pico8_fixed_from_int(0);
    int has_ii;

    if (self != NULL && pico8_instance_get_field(self, "fj", &fj_val) && fj_val.kind == PICO8_VALUE_FIXED) {
        od = fj_val.as.fixed;
    }
    if (self != NULL && pico8_instance_get_field(self, "il", &il_val) && pico8_research_truthy(il_val) &&
        pico8_oj(il_val)) {
        od = pico8_fixed_mul(od, PICO8_FIXED_LIT(0x00018000, 1.5));
    }
    has_ii = self != NULL && pico8_instance_get_field(self, "ii", &ii_val) && pico8_research_truthy(ii_val);
    if (!has_ii && pico8_oj(pico8_value_string("nanomaterials"))) {
        od = pico8_fixed_mul(od, PICO8_FIXED_LIT(0x0000d999, 0.85));
    }
    return pico8_fixed_flr(od);
}

