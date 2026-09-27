#include "pico8_class.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* An instance's tag, which pico8_type_tag_of() reads as UNKNOWN: no
 * entity accessor takes an instance any more. */
#define PICO8_CLASS_INSTANCE_TAG 0x9E5C1451u

typedef struct {
    char *name;
    pico8_class_t *cls;
} pico8_class_registry_entry_t;

static pico8_class_registry_entry_t *g_class_registry = NULL;
static int g_class_registry_count = 0;
static int g_class_registry_capacity = 0;

/* strdup() is POSIX; this build is -std=c99. */
static char *pico8_class_strdup(const char *s) {
    size_t n = strlen(s);
    char *out = malloc(n + 1);
    memcpy(out, s, n + 1);
    return out;
}

/* Overwrites an existing same-named entry in place -- which is how every
 * class without an `fz` clobbers the shared "" slot, as in the cart --
 * else appends a new entry holding its own owned copy of `name`. */
static void pico8_class_register(const char *name, pico8_class_t *cls) {
    int i;

    for (i = 0; i < g_class_registry_count; i++) {
        if (strcmp(g_class_registry[i].name, name) == 0) {
            g_class_registry[i].cls = cls;
            return;
        }
    }

    if (g_class_registry_count == g_class_registry_capacity) {
        int new_capacity = g_class_registry_capacity == 0 ? 4 : g_class_registry_capacity * 2;
        pico8_class_registry_entry_t *new_entries =
            realloc(g_class_registry, (size_t)new_capacity * sizeof *new_entries);
        if (new_entries == NULL) {
            abort();
        }
        g_class_registry = new_entries;
        g_class_registry_capacity = new_capacity;
    }

    g_class_registry[g_class_registry_count].name = pico8_class_strdup(name);
    g_class_registry[g_class_registry_count].cls = cls;
    g_class_registry_count++;
}

pico8_class_t *pico8_class_lookup(const char *name) {
    int i;

    for (i = 0; i < g_class_registry_count; i++) {
        if (strcmp(g_class_registry[i].name, name) == 0) {
            return g_class_registry[i].cls;
        }
    }
    return NULL;
}

void pico8_class_registry_clear(void) {
    int i;

    for (i = 0; i < g_class_registry_count; i++) {
        free(g_class_registry[i].name);
    }
    free(g_class_registry);
    g_class_registry = NULL;
    g_class_registry_count = 0;
    g_class_registry_capacity = 0;
}

typedef struct {
    char name[8];
    pico8_class_t *cls;
} pico8_class_named_entry_t;

static pico8_class_named_entry_t g_class_named_registry[PICO8_CLASS_NAMED_REGISTRY_CAPACITY];
static int g_class_named_registry_count = 0;

void pico8_class_register_named(const char *name, pico8_class_t *cls) {
    int i;

    assert(strlen(name) < sizeof g_class_named_registry[0].name &&
           "named registry: a truncated name would silently alias another class");

    for (i = 0; i < g_class_named_registry_count; i++) {
        if (strcmp(g_class_named_registry[i].name, name) == 0) {
            g_class_named_registry[i].cls = cls;
            return;
        }
    }

    if (g_class_named_registry_count >= PICO8_CLASS_NAMED_REGISTRY_CAPACITY) {
        abort();
    }
    strncpy(g_class_named_registry[g_class_named_registry_count].name, name,
            sizeof g_class_named_registry[g_class_named_registry_count].name - 1);
    g_class_named_registry[g_class_named_registry_count]
        .name[sizeof g_class_named_registry[g_class_named_registry_count].name - 1] = '\0';
    g_class_named_registry[g_class_named_registry_count].cls = cls;
    g_class_named_registry_count++;
}

pico8_class_t *pico8_class_lookup_named(const char *name) {
    int i;

    for (i = 0; i < g_class_named_registry_count; i++) {
        if (strcmp(g_class_named_registry[i].name, name) == 0) {
            return g_class_named_registry[i].cls;
        }
    }
    return NULL;
}

void pico8_class_registry_named_clear(void) {
    g_class_named_registry_count = 0;
}

pico8_class_t *pico8_class_declare(pico8_class_t *parent, const char *spec) {
    pico8_class_t *cls = malloc(sizeof *cls);
    pico8_value_t name_val;
    const char *name = "";

    cls->defaults = pico8_ob(spec != NULL ? spec : "", NULL);
    cls->parent = parent;
    cls->pq = NULL;
    cls->struct_new = NULL;
#ifdef PICO8_CLASS_COVERAGE
    cls->coverage_name = "";
#endif

    if (pico8_ob_table_get_name(cls->defaults, "fz", &name_val) && name_val.kind == PICO8_VALUE_STRING) {
        name = name_val.as.string;
    }
    cls->name = pico8_class_strdup(name);

    pico8_class_register(cls->name, cls);

    return cls;
}

void pico8_class_free(pico8_class_t *cls) {
    if (cls == NULL) {
        return;
    }
    free(cls->name);
    pico8_ob_table_free(cls->defaults);
    free(cls);
}

void pico8_class_set_pq(pico8_class_t *cls, pico8_class_pq_fn fn) {
    cls->pq = fn;
}

void pico8_class_set_struct_new(pico8_class_t *cls, pico8_class_struct_new_fn fn) {
    cls->struct_new = fn;
}

/* The nearest `pq` at or above `cls`. In the cart this is an ordinary
 * field read on the class table, which its metatable chain makes fall
 * back to the nearest ancestor's; C struct access has no such fallback,
 * so it is spelled out here and re-evaluated at each step of the walk
 * below rather than cached. */
static pico8_class_pq_fn pico8_class_effective_pq(const pico8_class_t *cls) {
    while (cls != NULL) {
        if (cls->pq != NULL) {
            return cls->pq;
        }
        cls = cls->parent;
    }
    return NULL;
}

#ifdef PICO8_CLASS_COVERAGE
typedef struct {
    const char *name;
    long count;
} pico8_class_coverage_entry_t;

static pico8_class_coverage_entry_t *g_coverage = NULL;
static int g_coverage_count = 0;
static int g_coverage_capacity = 0;

void pico8_class_coverage_record(const char *name) {
    int i;

    assert(name != NULL && "coverage: a NULL name cannot be reported as missing from KNOWN[]");
    for (i = 0; i < g_coverage_count; i++) {
        if (strcmp(g_coverage[i].name, name) == 0) {
            g_coverage[i].count++;
            return;
        }
    }
    if (g_coverage_count == g_coverage_capacity) {
        int new_capacity = g_coverage_capacity == 0 ? 32 : g_coverage_capacity * 2;
        pico8_class_coverage_entry_t *grown = realloc(g_coverage, (size_t)new_capacity * sizeof *grown);
        if (grown == NULL) {
            abort();
        }
        g_coverage = grown;
        g_coverage_capacity = new_capacity;
    }
    g_coverage[g_coverage_count].name = name;
    g_coverage[g_coverage_count].count = 1;
    g_coverage_count++;
}

long pico8_class_coverage_count(const char *name) {
    int i;

    for (i = 0; i < g_coverage_count; i++) {
        if (strcmp(g_coverage[i].name, name) == 0) {
            return g_coverage[i].count;
        }
    }
    return 0;
}

static int pico8_class_coverage_is_known(const char *name, const char *const *known, int known_count) {
    int i;

    for (i = 0; i < known_count; i++) {
        if (strcmp(known[i], name) == 0) {
            return 1;
        }
    }
    return 0;
}

const char *pico8_class_coverage_first_unknown(const char *const *known, int known_count) {
    int i;

    for (i = 0; i < g_coverage_count; i++) {
        if (!pico8_class_coverage_is_known(g_coverage[i].name, known, known_count)) {
            return g_coverage[i].name;
        }
    }
    for (i = 0; i < g_class_registry_count; i++) {
        if (g_class_registry[i].name[0] != '\0' &&
            !pico8_class_coverage_is_known(g_class_registry[i].name, known, known_count)) {
            return g_class_registry[i].name;
        }
    }
    for (i = 0; i < g_class_named_registry_count; i++) {
        if (!pico8_class_coverage_is_known(g_class_named_registry[i].name, known, known_count)) {
            return g_class_named_registry[i].name;
        }
    }
    return NULL;
}
#endif

pico8_instance_t *pico8_class_instantiate(pico8_class_t *cls, const pico8_ob_table_t *overrides) {
    pico8_instance_t *inst;
    const pico8_class_t *ko;
    pico8_class_pq_fn iy = NULL;
    int i;

    assert(cls != NULL && "instantiate: the class must be built first; a NULL one silently yields "
                          "an instance with no defaults and no pq");
    if (cls->struct_new != NULL) {
        return (pico8_instance_t *)cls->struct_new(overrides);
    }
    inst = malloc(sizeof *inst);
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record(cls->coverage_name);
#endif

    /* Only `overrides` is copied in, index-keyed and name-keyed entries
     * alike; class defaults stay on the class. */
    inst->tag = PICO8_CLASS_INSTANCE_TAG;
    inst->fields = pico8_ob_table_new();
    inst->cls = cls;

    if (overrides != NULL) {
        for (i = 0; i < overrides->count; i++) {
            pico8_ob_key_t key = overrides->entries[i].key;
            if (key.kind == PICO8_OB_KEY_NAME) {
                pico8_ob_table_set_name(inst->fields, key.name, overrides->entries[i].value);
            } else {
                pico8_ob_table_set_index(inst->fields, key.index, overrides->entries[i].value);
            }
        }
    }

    /* `iy` holds the last pq called, so a pq that several classes of the
     * chain resolve to by inheritance fires only at the first of them --
     * the cart's own dedup rule, reproduced, not simplified. */
    ko = cls;
    while (ko != NULL) {
        pico8_class_pq_fn pq = pico8_class_effective_pq(ko);
        if (pq != NULL && pq != iy) {
            iy = pq;
            iy(inst);
        }
        ko = ko->parent;
    }

    return inst;
}

void pico8_instance_free(pico8_instance_t *inst) {
    if (inst == NULL) {
        return;
    }
    pico8_ob_table_free(inst->fields);
    free(inst);
}

/* Reads only: a field ASSIGNMENT always lands on the instance's own
 * table, never on a class further up the chain. */
int pico8_instance_get_field(const pico8_instance_t *inst, const char *name, pico8_value_t *out_value) {
    assert(pico8_type_tag_of(inst) != PICO8_TYPE_TAG_STRUCT &&
           "instance_get_field: an entity struct has no fields table; a generic consumer of hv/cv/cg must read "
           "through pico8_entity_get_field()");
    if (pico8_ob_table_get_name(inst->fields, name, out_value)) {
        return 1;
    }
    return pico8_class_get_field(inst->cls, name, out_value);
}

pico8_ob_table_t *pico8_instance_fields(const pico8_instance_t *inst) {
    assert(pico8_type_tag_of(inst) != PICO8_TYPE_TAG_STRUCT &&
           "instance_fields: an entity struct has no fields table; read or write it through "
           "pico8_entity_get_field() / pico8_entity_set_field()");
    return inst->fields;
}

int pico8_class_get_field(const pico8_class_t *cls, const char *name, pico8_value_t *out_value) {
    for (; cls != NULL; cls = cls->parent) {
        if (pico8_ob_table_get_name(cls->defaults, name, out_value)) {
            return 1;
        }
    }
    return 0;
}
