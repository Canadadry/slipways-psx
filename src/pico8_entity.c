#include "pico8_entity.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_dispatch_context.h"
#include "pico8_game_data.h"
#include "pico8_ptrs.h"
#include "pico8_scroll.h"
#include "pico8_struct.h"

#define PICO8_BM_COUNT 5
static const char *const g_bm_names[PICO8_BM_COUNT] = {"kj", "ev", "hd", "hk", "hb"};

typedef struct {
    const char *name; /* borrowed from g_bm_names -- not owned here */
    pico8_ptrs_t list;
} pico8_cv_entry_t;

static pico8_cv_entry_t g_cv[PICO8_BM_COUNT];

/* `seen`: an entity carrying the tag has been lt()'d since
 * pico8_entity_init(); until then pico8_cg() returns NULL for it. */
typedef struct {
    int seen;
    pico8_ptrs_t list;
} pico8_cg_entry_t;

static pico8_cg_entry_t g_cg[PICO8_TAG_COUNT];

static pico8_ptrs_t g_hv;
static pico8_struct_header_t *g_mo = NULL;
static int g_initialized = 0;

static const pico8_struct_class_t g_mo_placeholder_struct_class = {0};

typedef struct {
    pico8_struct_header_t header;
} pico8_mo_placeholder_t;

/* The only one: never allocated, never freed; no field, and no writer, so
 * a write reaching it stops the run. */
static pico8_mo_placeholder_t g_mo_placeholder = {{.cls = &g_mo_placeholder_struct_class}};

/* See pico8_entity.h's own comment on this declaration for what is and
 * is not freed here. */
void pico8_entity_shutdown(void) {
    int i;

    if (!g_initialized) {
        return;
    }

    pico8_ptrs_free(&g_hv);

    for (i = 0; i < PICO8_BM_COUNT; i++) {
        pico8_ptrs_free(&g_cv[i].list);
        g_cv[i].name = NULL;
    }

    for (i = 0; i < PICO8_TAG_COUNT; i++) {
        pico8_ptrs_free(&g_cg[i].list);
        g_cg[i].seen = 0;
    }

    /* TROU-7 */
    g_mo = NULL;

    g_initialized = 0;
}

void pico8_entity_init(void) {
    int i;

    pico8_entity_shutdown();

    pico8_ptrs_init(&g_hv);

    for (i = 0; i < PICO8_BM_COUNT; i++) {
        g_cv[i].name = g_bm_names[i];
        pico8_ptrs_init(&g_cv[i].list);
    }

    g_mo = (pico8_struct_header_t *)&g_mo_placeholder;

    g_initialized = 1;
}

const pico8_ptrs_t *pico8_hv(void) {
    return &g_hv;
}

const pico8_ptrs_t *pico8_cv(const char *name) {
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

const pico8_ptrs_t *pico8_cg(const char *tag) {
    int t = pico8_tag_named(tag);
    return t >= 0 && g_cg[t].seen ? &g_cg[t].list : NULL;
}

pico8_struct_header_t *pico8_lt(pico8_struct_header_t *e) {
    int i;

    pico8_ptrs_add(&g_hv, e);

    for (i = 0; i < PICO8_BM_COUNT; i++) {
        if (pico8_bm_has(e, i)) {
            pico8_ptrs_add(&g_cv[i].list, e);
        }
    }

    for (i = 0; i < PICO8_TAG_COUNT; i++) {
        if (e->cls->tags & PICO8_TAG(i)) {
            g_cg[i].seen = 1;
            pico8_ptrs_add(&g_cg[i].list, e);
        }
    }

    return e;
}

/* `e` must be unregistered under exactly the fields it was registered
 * under: a bm field changed since lt() leaves `e` in a list this never
 * removes it from. */
void pico8_he(pico8_struct_header_t *e) {
    int i;

    /* TROU-5 */
    pico8_ptrs_del(&g_hv, e);

    for (i = 0; i < PICO8_BM_COUNT; i++) {
        if (pico8_bm_has(e, i)) {
            pico8_ptrs_del(&g_cv[i].list, e);
        }
    }

    /* A tag never lt()'d: Lua would error on indexing its bucket, this
     * port deletes from an empty list. */
    for (i = 0; i < PICO8_TAG_COUNT; i++) {
        if (e->cls->tags & PICO8_TAG(i)) {
            pico8_ptrs_del(&g_cg[i].list, e);
        }
    }
}

void pico8_dc(const char *nt) {
    const pico8_ptrs_t *bucket_src;
    pico8_ptrs_t layers[15];
    int kj = strcmp(nt, "kj") == 0;
    int i, o;

    assert((kj || strcmp(nt, "ev") == 0) && "dc: only kj and ev are drawn through dc");

    for (i = 0; i < 15; i++) {
        pico8_ptrs_init(&layers[i]);
    }

    bucket_src = pico8_cv(nt);
    if (bucket_src != NULL) {
        for (i = 0; i < bucket_src->count; i++) {
            pico8_struct_header_t *qc = bucket_src->items[i];
            pico8_fixed_t fa;
            int layer;

            /* An entity whose `.fa` is absent or outside 1..15 is dropped
             * from every layer, where Lua would error on the nil key. */
            if (qc->cls->get_fa == NULL || !qc->cls->get_fa(qc, &fa)) {
                continue;
            }
            layer = pico8_fixed_flr_int(fa);
            if (layer < 1 || layer > 15) {
                continue;
            }
            pico8_ptrs_add(&layers[layer - 1], qc);
        }
    }

    for (o = 1; o <= 15; o++) {
        pico8_ptrs_t *bucket = &layers[o - 1];

        if (kj && bucket->count > 0) {
            pico8_fixed_t ci;
            /* The placeholder mo, before jl(), has no `md`. */
            if (pico8_fixed_seq_at(pico8_ci_table, PICO8_CI_COUNT, o, &ci) && g_mo != NULL &&
                g_mo->cls == pico8_qb_struct_class()) {
                pico8_qb_md(pico8_dispatch_gfx(), (pico8_qb_t *)g_mo, ci);
            }
        }

        for (i = 0; i < bucket->count; i++) {
            pico8_struct_header_t *qc = bucket->items[i];
            void (*draw)(void *self, const pico8_vec_t *ra) = kj ? qc->cls->kj : qc->cls->ev;
            const pico8_vec_t *ra = qc->cls->get_ra != NULL ? qc->cls->get_ra(qc) : NULL;

            if (draw != NULL) {
                draw(qc, ra);
            }
        }
    }

    for (i = 0; i < 15; i++) {
        pico8_ptrs_free(&layers[i]);
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
