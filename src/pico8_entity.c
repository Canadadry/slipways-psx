#include "pico8_entity.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_dispatch_context.h"
#include "pico8_game_data.h"
#include "pico8_scroll.h"
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
static pico8_struct_header_t *g_mo = NULL;
static int g_initialized = 0;

static int pico8_mo_placeholder_field(const void *self, const char *name, pico8_value_t *out_value) {
    (void)self;
    (void)name;
    (void)out_value;
    return 0;
}

static const pico8_struct_class_t g_mo_placeholder_struct_class = {
    .field = pico8_mo_placeholder_field,
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

    g_mo = (pico8_struct_header_t *)&g_mo_placeholder;

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

/* `e[bm[i]]` exists: the member of `e`'s class for the i-th name of
 * g_bm_names. */
static int pico8_bm_has(const pico8_struct_header_t *e, int i) {
    const pico8_struct_class_t *cls;

    assert(pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           "entity: e is an entity struct; a list REF passed as an entity would be read as one");
    cls = e->cls;

    switch (i) {
    case 0:
        return cls->kj != NULL;
    case 1:
        return cls->ev != NULL;
    case 2:
        return cls->hd != NULL;
    case 3:
        return cls->hk != NULL;
    default:
        return cls->hb != NULL && (cls->has_hb == NULL || cls->has_hb(e));
    }
}

int pico8_hd_true(void *self, const pico8_vec_t *qa, const pico8_vec_t *ra) {
    (void)self;
    (void)qa;
    (void)ra;
    return 1;
}

const pico8_list_t *pico8_cg(const char *tag) {
    pico8_cg_entry_t *entry = pico8_cg_find(tag);
    return entry != NULL ? &entry->list : NULL;
}

int pico8_entity_get_field(const void *e, const char *name, pico8_value_t *out_value) {
    const pico8_struct_header_t *header;

    assert(pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           "entity: e is an entity struct; a list REF passed as an entity would be read as one");
    header = e;
    assert(header->cls->field != NULL && "entity: a struct a generic consumer reaches needs a field reader");
    return header->cls->field(e, name, out_value);
}

void pico8_entity_set_field(void *e, const char *name, pico8_value_t value) {
    const pico8_struct_header_t *header;
    int written;

    assert(pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT &&
           "entity: e is an entity struct; a list REF passed as an entity would be written as one");
    header = e;
    assert(header->cls->set_field != NULL && "entity: a struct a generic writer reaches needs a field writer");
    written = header->cls->set_field(e, name, value);
    assert(written && "entity: a generic write names a field this struct has no member for "
                      "-- stop, do not add a member");
    (void)written;
}

pico8_struct_header_t *pico8_lt(pico8_ke_t *ke) {
    pico8_struct_header_t *e = &ke->header;
    int i;
    pico8_value_t nu_val;

    pico8_list_add(&g_hv, pico8_value_ref(e));

    for (i = 0; i < PICO8_BM_COUNT; i++) {
        if (pico8_bm_has(e, i)) {
            pico8_list_add(&g_cv[i].list, pico8_value_ref(e));
        }
    }

    /* `.nu` is a pico8_str_seq_t; an absent one registers nothing, as
     * `all(nil)` would. */
    if (pico8_entity_get_field(e, "nu", &nu_val) && nu_val.kind == PICO8_VALUE_REF && nu_val.as.ref != NULL) {
        const pico8_str_seq_t *tags = nu_val.as.ref;
        for (i = 0; i < tags->count; i++) {
            pico8_cg_entry_t *bucket = pico8_cg_find_or_create(tags->items[i]);
            pico8_list_add(&bucket->list, pico8_value_ref(e));
        }
    }

    return e;
}

/* `e` must be unregistered under exactly the fields and tags it was
 * registered under: a bm field or a `.nu` entry changed since lt()
 * leaves `e` in a list this never removes it from. */
void pico8_he(pico8_struct_header_t *e) {
    int i;
    pico8_value_t nu_val;

    /* TROU-5 */
    pico8_list_del(&g_hv, pico8_value_ref(e));

    for (i = 0; i < PICO8_BM_COUNT; i++) {
        if (pico8_bm_has(e, i)) {
            pico8_list_del(&g_cv[i].list, pico8_value_ref(e));
        }
    }

    /* A tag never lt()'d has no bucket; Lua would error on indexing it,
     * this port does nothing. */
    if (pico8_entity_get_field(e, "nu", &nu_val) && nu_val.kind == PICO8_VALUE_REF && nu_val.as.ref != NULL) {
        const pico8_str_seq_t *tags = nu_val.as.ref;
        for (i = 0; i < tags->count; i++) {
            pico8_cg_entry_t *bucket = pico8_cg_find(tags->items[i]);
            if (bucket != NULL) {
                pico8_list_del(&bucket->list, pico8_value_ref(e));
            }
        }
    }
}

void pico8_dc(const char *nt) {
    const pico8_list_t *bucket_src;
    pico8_list_t layers[15];
    int kj = strcmp(nt, "kj") == 0;
    int i, o;

    assert((kj || strcmp(nt, "ev") == 0) && "dc: only kj and ev are drawn through dc");

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

        if (kj && bucket->count > 0) {
            pico8_fixed_t ci;
            /* The placeholder mo, before jl(), has no `md`. */
            if (pico8_fixed_seq_at(pico8_ci_table, PICO8_CI_COUNT, o, &ci) && g_mo != NULL &&
                g_mo->cls == pico8_qb_struct_class()) {
                pico8_qb_md(pico8_dispatch_gfx(), (pico8_qb_t *)g_mo, ci);
            }
        }

        for (i = 0; i < bucket->count; i++) {
            pico8_struct_header_t *qc = bucket->items[i].as.ref;
            void (*draw)(void *self, const pico8_vec_t *ra) = kj ? qc->cls->kj : qc->cls->ev;
            pico8_value_t ra_val;
            const pico8_vec_t *ra = NULL;

            if (pico8_entity_get_field(qc, "ra", &ra_val) && ra_val.kind == PICO8_VALUE_REF) {
                ra = ra_val.as.ref;
            }
            if (draw != NULL) {
                draw(qc, ra);
            }
        }
    }

    for (i = 0; i < 15; i++) {
        pico8_list_free(&layers[i]);
    }
}

const pico8_struct_class_t *pico8_mo_placeholder_struct_class(void) {
    return &g_mo_placeholder_struct_class;
}

pico8_struct_header_t *pico8_mo(void) {
    return g_mo;
}

pico8_qb_t *pico8_mo_qb(void) {
    if (g_mo == NULL || g_mo == &g_mo_placeholder.header) {
        return NULL;
    }
    assert(g_mo->cls == pico8_qb_struct_class() && "mo: only the placeholder or a qb is ever stored");
    return (pico8_qb_t *)g_mo;
}

/* See pico8_entity.h's own comment on this declaration: the outgoing mo
 * is deliberately leaked rather than freed. */
void pico8_entity_set_mo(pico8_struct_header_t *instance) {
    if (instance == g_mo) {
        return;
    }
    g_mo = instance;
}
