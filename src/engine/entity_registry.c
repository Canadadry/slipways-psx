#include "engine/entity_registry.h"

#include <assert.h>
#include <string.h>

#include "engine/exit_queue.h"
#include "game/game_data.h"
#include "engine/entity_class.h"
#include "engine/frame_zone.h"
#include "engine/game_zone.h"

#define PICO8_BM_COUNT 5
static const char *const g_bm_names[PICO8_BM_COUNT] = {"draw", "draw_overlay", "hit_test", "new_year", "refresh"};

typedef struct {
    const char *name; /* borrowed from g_bm_names -- not owned here */
    pico8_ents_t list;
} pico8_cv_entry_t;

static pico8_cv_entry_t g_cv[PICO8_BM_COUNT];

/* `seen`: an entity carrying the tag has been lt()'d since
 * pico8_entity_init(); until then pico8_cg() returns NULL for it. */
typedef struct {
    int seen;
    pico8_ents_t list;
} pico8_cg_entry_t;

static pico8_cg_entry_t g_cg[TAG_COUNT];

static pico8_ents_t g_hv;
static entity_t *g_mo = NULL;
static int g_initialized = 0;

static const entity_class_t g_mo_placeholder_struct_class = {0};

typedef struct {
    entity_t header;
} pico8_mo_placeholder_t;

/* The only one: never allocated, never freed; no field, and no writer, so
 * a write reaching it stops the run. */
static pico8_mo_placeholder_t g_mo_placeholder = {{.class = &g_mo_placeholder_struct_class}};

/* See pico8_entity.h's own comment on this declaration for what is and
 * is not freed here. */
void pico8_entity_shutdown(void) {
    int i;

    if (!g_initialized) {
        return;
    }

    pico8_ents_free(&g_hv);

    for (i = 0; i < PICO8_BM_COUNT; i++) {
        pico8_ents_free(&g_cv[i].list);
        g_cv[i].name = NULL;
    }

    for (i = 0; i < TAG_COUNT; i++) {
        pico8_ents_free(&g_cg[i].list);
        g_cg[i].seen = 0;
    }

    g_mo = NULL;

    g_initialized = 0;
}

void pico8_entity_init(void) {
    int i;

    pico8_entity_shutdown();

    pico8_ents_init(&g_hv, pico8_zone_partie());

    for (i = 0; i < PICO8_BM_COUNT; i++) {
        g_cv[i].name = g_bm_names[i];
        pico8_ents_init(&g_cv[i].list, pico8_zone_partie());
    }

    for (i = 0; i < TAG_COUNT; i++) {
        pico8_ents_init(&g_cg[i].list, pico8_zone_partie());
    }

    g_mo = (entity_t *)&g_mo_placeholder;

    g_initialized = 1;
}

void pico8_entity_abandonner_hv(void) {
    int i;

    for (i = 0; i < g_hv.count; i++) {
        if (g_hv.items[i]->class->release != NULL) {
            pico8_file_sortie_mettre(g_hv.items[i]);
        }
    }
}

static void oublier_liste(pico8_ents_t *liste) {
    liste->items = NULL;
    liste->count = 0;
    liste->capacity = 0;
}

void pico8_entity_oublier_registres(void) {
    int i;

    oublier_liste(&g_hv);
    for (i = 0; i < PICO8_BM_COUNT; i++) {
        oublier_liste(&g_cv[i].list);
    }
    for (i = 0; i < TAG_COUNT; i++) {
        oublier_liste(&g_cg[i].list);
        g_cg[i].seen = 0;
    }
}

const pico8_ents_t *pico8_hv(void) {
    return &g_hv;
}

const pico8_ents_t *pico8_cv(const char *name) {
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
static int pico8_bm_has(const entity_t *e, int i) {
    const entity_class_t *cls;

    cls = e->class;

    switch (i) {
    case 0:
        return cls->draw != NULL;
    case 1:
        return cls->draw_overlay != NULL;
    case 2:
        return cls->hit_test != NULL;
    case 3:
        return cls->new_year != NULL;
    default:
        return cls->refresh != NULL && (cls->has_refresh == NULL || cls->has_refresh(e));
    }
}

int pico8_hd_true(entity_t *self, const pico8_vec_t *qa, const pico8_vec_t *ra) {
    (void)self;
    (void)qa;
    (void)ra;
    return 1;
}

const pico8_ents_t *pico8_cg(const char *tag) {
    int t = entity_tag_named(tag);
    return t >= 0 && g_cg[t].seen ? &g_cg[t].list : NULL;
}

entity_t *pico8_lt(entity_t *e) {
    int i;

    pico8_ents_add(&g_hv, e);

    for (i = 0; i < PICO8_BM_COUNT; i++) {
        if (pico8_bm_has(e, i)) {
            pico8_ents_add(&g_cv[i].list, e);
        }
    }

    for (i = 0; i < TAG_COUNT; i++) {
        if (e->class->tags & TAG(i)) {
            g_cg[i].seen = 1;
            pico8_ents_add(&g_cg[i].list, e);
        }
    }

    return e;
}

/* `e` must be unregistered under exactly the fields it was registered
 * under: a bm field changed since lt() leaves `e` in a list this never
 * removes it from. */
void pico8_he(entity_t *e) {
    int i;

    /* TROU-5 */
    if (pico8_ents_del(&g_hv, e) && e->class->release != NULL) {
        pico8_file_sortie_mettre(e);
    }

    for (i = 0; i < PICO8_BM_COUNT; i++) {
        if (pico8_bm_has(e, i)) {
            pico8_ents_del(&g_cv[i].list, e);
        }
    }

    /* A tag never lt()'d: Lua would error on indexing its bucket, this
     * port deletes from an empty list. */
    for (i = 0; i < TAG_COUNT; i++) {
        if (e->class->tags & TAG(i)) {
            pico8_ents_del(&g_cg[i].list, e);
        }
    }
}

void pico8_dc(const char *nt) {
    const pico8_ents_t *bucket_src;
    pico8_ents_t layers[15];
    int kj = strcmp(nt, "draw") == 0;
    int i, o;

    assert((kj || strcmp(nt, "draw_overlay") == 0) && "dc: only kj and ev are drawn through dc");

    for (i = 0; i < 15; i++) {
        pico8_ents_init(&layers[i], pico8_zone_frame());
    }

    bucket_src = pico8_cv(nt);
    if (bucket_src != NULL) {
        for (i = 0; i < bucket_src->count; i++) {
            entity_t *qc = bucket_src->items[i];
            int layer;

            /* An entity whose `.fa` is absent or outside 1..15 is dropped
             * from every layer, where Lua would error on the nil key. */
            if (qc->layer == NULL) {
                continue;
            }
            layer = pico8_fixed_flr_int(*qc->layer);
            if (layer < 1 || layer > 15) {
                continue;
            }
            pico8_ents_add(&layers[layer - 1], qc);
        }
    }

    for (o = 1; o <= 15; o++) {
        pico8_ents_t *bucket = &layers[o - 1];

        if (kj && bucket->count > 0) {
            pico8_fixed_t ci;
            /* The placeholder mo, before jl(), has no `md`. */
            if (pico8_fixed_seq_at(pico8_ci_table, PICO8_CI_COUNT, o, &ci) && g_mo != NULL &&
                g_mo->class->apply_camera != NULL) {
                g_mo->class->apply_camera(g_mo, ci);
            }
        }

        for (i = 0; i < bucket->count; i++) {
            entity_t *qc = bucket->items[i];
            void (*draw)(entity_t *self, const pico8_vec_t *ra) = kj ? qc->class->draw : qc->class->draw_overlay;
            const pico8_vec_t *ra = qc->pos != NULL ? *qc->pos : NULL;

            if (draw != NULL) {
                draw(qc, ra);
            }
        }
    }
}

const entity_class_t *pico8_mo_placeholder_struct_class(void) {
    return &g_mo_placeholder_struct_class;
}

entity_t *pico8_mo(void) {
    return g_mo;
}

/* See pico8_entity.h's own comment on this declaration: the outgoing mo
 * lives on in its generation, given back when that generation is reset. */
void pico8_entity_set_mo(entity_t *instance) {
    if (instance == g_mo) {
        return;
    }
    g_mo = instance;
}
