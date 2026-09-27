#include "pico8_entity.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_struct.h"

#define PICO8_BM_COUNT 5
static const char *const g_bm_names[PICO8_BM_COUNT] = {"kj", "ev", "hd", "hk", "hb"};

typedef struct {
    const char *name; /* borrowed from g_bm_names -- not owned here */
    pico8_list_t list;
} pico8_cv_entry_t;

static pico8_cv_entry_t g_cv[PICO8_BM_COUNT];

/* Each tag is an owned copy: it arrives borrowed from an entity's own
 * `.nu` table, which this module does not keep alive past the lt() call
 * that reads it. */
typedef struct {
    char *tag;
    pico8_list_t list;
} pico8_cg_entry_t;

static pico8_cg_entry_t *g_cg = NULL;
static int g_cg_count = 0;
static int g_cg_capacity = 0;

static pico8_list_t g_hv;
static pico8_ob_table_t *g_ci = NULL;
static pico8_struct_header_t *g_mo = NULL;
static int g_initialized = 0;

static int pico8_mo_placeholder_field(const void *self, const char *name, pico8_value_t *out_value) {
    (void)self;
    (void)name;
    (void)out_value;
    return 0;
}

static const pico8_struct_class_t g_mo_placeholder_struct_class = {
    {0},
    pico8_mo_placeholder_field,
    NULL,
};

typedef struct {
    pico8_struct_header_t header;
} pico8_mo_placeholder_t;

/* The only one: never allocated, never freed; no field, and no writer, so
 * a write reaching it stops the run. */
static pico8_mo_placeholder_t g_mo_placeholder = {{PICO8_TYPE_TAG_STRUCT, &g_mo_placeholder_struct_class}};

/* strdup() is POSIX; this build is -std=c99. */
static char *pico8_entity_strdup(const char *s) {
    size_t n = strlen(s);
    char *out = malloc(n + 1);
    memcpy(out, s, n + 1);
    return out;
}

static pico8_cg_entry_t *pico8_cg_find(const char *tag) {
    int i;
    for (i = 0; i < g_cg_count; i++) {
        if (strcmp(g_cg[i].tag, tag) == 0) {
            return &g_cg[i];
        }
    }
    return NULL;
}

/* Callers must not hold the returned pointer across a later call that
 * grows g_cg; pico8_cg() re-derives its own fresh on every call. */
static pico8_cg_entry_t *pico8_cg_find_or_create(const char *tag) {
    pico8_cg_entry_t *found = pico8_cg_find(tag);

    if (found != NULL) {
        return found;
    }

    if (g_cg_count == g_cg_capacity) {
        int new_capacity = g_cg_capacity == 0 ? 4 : g_cg_capacity * 2;
        pico8_cg_entry_t *new_entries = realloc(g_cg, (size_t)new_capacity * sizeof *new_entries);
        if (new_entries == NULL) {
            abort();
        }
        g_cg = new_entries;
        g_cg_capacity = new_capacity;
    }

    g_cg[g_cg_count].tag = pico8_entity_strdup(tag);
    pico8_list_init(&g_cg[g_cg_count].list);
    g_cg_count++;
    return &g_cg[g_cg_count - 1];
}

/* See pico8_entity.h's own comment on this declaration for what is and
 * is not freed here. */
void pico8_entity_shutdown(void) {
    int i;

    if (!g_initialized) {
        return;
    }

    pico8_list_free(&g_hv);

    for (i = 0; i < PICO8_BM_COUNT; i++) {
        pico8_list_free(&g_cv[i].list);
        g_cv[i].name = NULL;
    }

    for (i = 0; i < g_cg_count; i++) {
        pico8_list_free(&g_cg[i].list);
        free(g_cg[i].tag);
    }
    free(g_cg);
    g_cg = NULL;
    g_cg_count = 0;
    g_cg_capacity = 0;

    pico8_ob_table_free(g_ci);
    g_ci = NULL;

    /* TROU-7 */
    g_mo = NULL;

    g_initialized = 0;
}

void pico8_entity_init(void) {
    int i;

    pico8_entity_shutdown();

    pico8_list_init(&g_hv);

    for (i = 0; i < PICO8_BM_COUNT; i++) {
        g_cv[i].name = g_bm_names[i];
        pico8_list_init(&g_cv[i].list);
    }

    g_ci = pico8_ob("0.5,1,1,1,1,1,1,1,1,0,0,0,0,0,0,", NULL);

    g_mo = (pico8_struct_header_t *)&g_mo_placeholder;
    PICO8_MEMBER_CENSUS_SEE(&g_mo_placeholder, pico8_mo_placeholder_t);

    g_initialized = 1;
}

const pico8_list_t *pico8_hv(void) {
    return &g_hv;
}

const pico8_list_t *pico8_cv(const char *name) {
    int i;
    assert(g_initialized && "pico8_entity_init() must run before cv is read");
    for (i = 0; i < PICO8_BM_COUNT; i++) {
        if (strcmp(g_cv[i].name, name) == 0) {
            return &g_cv[i].list;
        }
    }
    return NULL;
}

const pico8_list_t *pico8_cg(const char *tag) {
    pico8_cg_entry_t *entry = pico8_cg_find(tag);
    return entry != NULL ? &entry->list : NULL;
}

int pico8_entity_get_field(const void *e, const char *name, pico8_value_t *out_value) {
    const pico8_struct_header_t *header;
    int method;

    assert(pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           "entity: e is an entity struct; an ob table or list REF passed as an entity would be read as one");
    header = e;
    method = pico8_method_of(name);
    if (method >= 0 && header->cls->methods[method] != NULL) {
        *out_value = pico8_value_function(header->cls->methods[method]);
        return 1;
    }
    assert(header->cls->field != NULL && "entity: a struct a generic consumer reaches needs a field reader");
    return PICO8_MEMBER_CENSUS_READ(e, name, header->cls->field(e, name, out_value), out_value);
}

void pico8_entity_set_field(void *e, const char *name, pico8_value_t value) {
    const pico8_struct_header_t *header;
    int written;

    assert(pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           "entity: e is an entity struct; an ob table or list REF passed as an entity would be written as one");
    header = e;
    assert(header->cls->set_field != NULL && "entity: a struct a generic writer reaches needs a field writer");
    written = header->cls->set_field(e, name, value);
    PICO8_MEMBER_CENSUS_WRITE(e, name, value);
    assert(written && "entity: a generic write names a field this struct has no member for, a field under no "
                      "rule of plan-3 section 5 -- stop, do not add a member");
    (void)written;
}

pico8_struct_header_t *pico8_lt(pico8_struct_header_t *e) {
    int i;
    pico8_value_t discard;
    pico8_value_t nu_val;

    pico8_list_add(&g_hv, pico8_value_ref(e));

    /* `if(e[p])` is ported as a presence test, not as Lua truthiness: a
     * bm field explicitly set to `false` still registers `e` in cv[p]. */
    for (i = 0; i < PICO8_BM_COUNT; i++) {
        if (pico8_entity_get_field(e, g_cv[i].name, &discard)) {
            pico8_list_add(&g_cv[i].list, pico8_value_ref(e));
        }
    }

    /* `.nu` is a table of positional tag strings; an absent one, or a
     * non-string entry, registers nothing, as `all(nil)` would. */
    if (pico8_entity_get_field(e, "nu", &nu_val) && nu_val.kind == PICO8_VALUE_REF && nu_val.as.ref != NULL) {
        const pico8_ob_table_t *nu_table = nu_val.as.ref;
        for (i = 0; i < nu_table->count; i++) {
            if (nu_table->entries[i].value.kind == PICO8_VALUE_STRING) {
                pico8_cg_entry_t *bucket = pico8_cg_find_or_create(nu_table->entries[i].value.as.string);
                pico8_list_add(&bucket->list, pico8_value_ref(e));
            }
        }
    }

    return e;
}

/* `e` must be unregistered under exactly the fields and tags it was
 * registered under: a bm field or a `.nu` entry changed since lt()
 * leaves `e` in a list this never removes it from. */
void pico8_he(pico8_struct_header_t *e) {
    int i;
    pico8_value_t discard;
    pico8_value_t nu_val;

    /* TROU-5 */
    pico8_list_del(&g_hv, pico8_value_ref(e));

    for (i = 0; i < PICO8_BM_COUNT; i++) {
        if (pico8_entity_get_field(e, g_cv[i].name, &discard)) {
            pico8_list_del(&g_cv[i].list, pico8_value_ref(e));
        }
    }

    if (pico8_entity_get_field(e, "nu", &nu_val) && nu_val.kind == PICO8_VALUE_REF && nu_val.as.ref != NULL) {
        const pico8_ob_table_t *nu_table = nu_val.as.ref;
        for (i = 0; i < nu_table->count; i++) {
            if (nu_table->entries[i].value.kind == PICO8_VALUE_STRING) {
                /* A tag never lt()'d has no bucket; Lua would error on
                 * indexing it, this port does nothing. */
                pico8_cg_entry_t *bucket = pico8_cg_find(nu_table->entries[i].value.as.string);
                if (bucket != NULL) {
                    pico8_list_del(&bucket->list, pico8_value_ref(e));
                }
            }
        }
    }
}

int pico8_mv(pico8_struct_header_t *e, const char *method_name, int argc, pico8_value_t arg0, pico8_value_t arg1,
             pico8_value_t *out_value) {
    pico8_value_t field_val;

    /* `e and e[rm]`: a falsy `e` short-circuits before the field read. */
    if (e == NULL) {
        return 0;
    }
    if (!pico8_entity_get_field(e, method_name, &field_val)) {
        return 0;
    }

    if (field_val.kind == PICO8_VALUE_FUNCTION) {
        *out_value = field_val.as.function(e, argc, arg0, arg1);
    } else {
        *out_value = field_val;
    }
    return 1;
}

void pico8_dc(const char *nt) {
    const pico8_list_t *bucket_src;
    pico8_list_t layers[15];
    int i, o;

    for (i = 0; i < 15; i++) {
        pico8_list_init(&layers[i]);
    }

    bucket_src = pico8_cv(nt);
    if (bucket_src != NULL) {
        for (i = 0; i < bucket_src->count; i++) {
            pico8_struct_header_t *qc = bucket_src->items[i].as.ref;
            pico8_value_t fa_val;
            int layer;

            assert(pico8_type_tag_of(qc) == PICO8_TYPE_TAG_STRUCT && "cv holds entities only");

            /* An entity whose `.fa` is absent or outside 1..15 is dropped
             * from every layer, where Lua would error on the nil key. */
            if (!pico8_entity_get_field(qc, "fa", &fa_val) || fa_val.kind != PICO8_VALUE_FIXED) {
                continue;
            }
            layer = pico8_fixed_flr_int(fa_val.as.fixed);
            if (layer < 1 || layer > 15) {
                continue;
            }
            pico8_list_add(&layers[layer - 1], pico8_value_ref(qc));
        }
    }

    for (o = 1; o <= 15; o++) {
        pico8_list_t *bucket = &layers[o - 1];

        if (strcmp(nt, "kj") == 0 && bucket->count > 0) {
            pico8_value_t ci_val;
            if (pico8_ob_table_get_index(g_ci, o, &ci_val)) {
                pico8_value_t discard;
                pico8_mv(g_mo, "md", 1, ci_val, pico8_value_ref(NULL), &discard);
            }
        }

        for (i = 0; i < bucket->count; i++) {
            pico8_struct_header_t *qc = bucket->items[i].as.ref;
            pico8_value_t ra_val;
            pico8_value_t discard;

            /* An absent `.ra` is passed as pico8_value_ref(NULL), this
             * port's stand-in for Lua nil as a call argument. */
            if (!pico8_entity_get_field(qc, "ra", &ra_val)) {
                ra_val = pico8_value_ref(NULL);
            }
            /* TROU-11 */
            pico8_mv(qc, nt, 1, ra_val, pico8_value_ref(NULL), &discard);
        }
    }

    for (i = 0; i < 15; i++) {
        pico8_list_free(&layers[i]);
    }
}

const pico8_ob_table_t *pico8_ci(void) {
    return g_ci;
}

const pico8_struct_class_t *pico8_mo_placeholder_struct_class(void) {
    return &g_mo_placeholder_struct_class;
}

pico8_struct_header_t *pico8_mo(void) {
    return g_mo;
}

/* See pico8_entity.h's own comment on this declaration: the outgoing mo
 * is deliberately leaked rather than freed. */
void pico8_entity_set_mo(pico8_struct_header_t *instance) {
    if (instance == g_mo) {
        return;
    }
    g_mo = instance;
}
