#include "engine/entity_registry.h"

#include <assert.h>
#include <string.h>

#include "engine/exit_queue.h"
#include "game/game_data.h"
#include "engine/entity_class.h"
#include "engine/frame_zone.h"
#include "engine/game_zone.h"

#define MESSAGE_COUNT 5
static const char *const g_message_names[MESSAGE_COUNT] = {"draw", "draw_overlay", "hit_test", "new_year", "refresh"};

typedef struct {
    const char *name; /* borrowed from g_bm_names -- not owned here */
    entity_list_t list;
} entities_by_message_entry_t;

static entities_by_message_entry_t g_entities_by_message[MESSAGE_COUNT];

/* `seen`: an entity carrying the tag has been lt()'d since
 * pico8_entity_init(); until then pico8_cg() returns NULL for it. */
typedef struct {
    int seen;
    entity_list_t list;
} entities_by_tag_entry_t;

static entities_by_tag_entry_t g_entities_by_tag[TAG_COUNT];

static entity_list_t g_live_entities;
static entity_t *g_main_camera = NULL;
static int g_initialized = 0;

static const entity_class_t g_main_camera_placeholder_class = {0};

typedef struct {
    entity_t header;
} main_camera_placeholder_t;

/* The only one: never allocated, never freed; no field, and no writer, so
 * a write reaching it stops the run. */
static main_camera_placeholder_t g_main_camera_placeholder = {{.class = &g_main_camera_placeholder_class}};

/* See pico8_entity.h's own comment on this declaration for what is and
 * is not freed here. */
void entity_registry_shutdown(void) {
    int i;

    if (!g_initialized) {
        return;
    }

    entity_list_free(&g_live_entities);

    for (i = 0; i < MESSAGE_COUNT; i++) {
        entity_list_free(&g_entities_by_message[i].list);
        g_entities_by_message[i].name = NULL;
    }

    for (i = 0; i < TAG_COUNT; i++) {
        entity_list_free(&g_entities_by_tag[i].list);
        g_entities_by_tag[i].seen = 0;
    }

    g_main_camera = NULL;

    g_initialized = 0;
}

void entity_registry_init(void) {
    int i;

    entity_registry_shutdown();

    entity_list_init(&g_live_entities, game_zone());

    for (i = 0; i < MESSAGE_COUNT; i++) {
        g_entities_by_message[i].name = g_message_names[i];
        entity_list_init(&g_entities_by_message[i].list, game_zone());
    }

    for (i = 0; i < TAG_COUNT; i++) {
        entity_list_init(&g_entities_by_tag[i].list, game_zone());
    }

    g_main_camera = (entity_t *)&g_main_camera_placeholder;

    g_initialized = 1;
}

void entity_registry_abandon_live_entities(void) {
    int i;

    for (i = 0; i < g_live_entities.count; i++) {
        if (g_live_entities.items[i]->class->release != NULL) {
            exit_queue_put(g_live_entities.items[i]);
        }
    }
}

static void forget_list(entity_list_t *list) {
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

void entity_registry_forget_registries(void) {
    int i;

    forget_list(&g_live_entities);
    for (i = 0; i < MESSAGE_COUNT; i++) {
        forget_list(&g_entities_by_message[i].list);
    }
    for (i = 0; i < TAG_COUNT; i++) {
        forget_list(&g_entities_by_tag[i].list);
        g_entities_by_tag[i].seen = 0;
    }
}

const entity_list_t *live_entities(void) {
    return &g_live_entities;
}

const entity_list_t *entities_by_message(const char *name) {
    int i;
    assert(g_initialized && "pico8_entity_init() must run before cv is read");
    for (i = 0; i < MESSAGE_COUNT; i++) {
        if (strcmp(g_entities_by_message[i].name, name) == 0) {
            return &g_entities_by_message[i].list;
        }
    }
    return NULL;
}

/* `e[bm[i]]` exists: the member of `e`'s class for the i-th name of
 * g_bm_names. */
static int is_message_name(const entity_t *e, int i) {
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

int entity_registry_hit_test_always(entity_t *self, const vec_t *qa, const vec_t *ra) {
    (void)self;
    (void)qa;
    (void)ra;
    return 1;
}

const entity_list_t *entities_by_tag(const char *tag) {
    int t = entity_tag_named(tag);
    return t >= 0 && g_entities_by_tag[t].seen ? &g_entities_by_tag[t].list : NULL;
}

entity_t *entity_register(entity_t *e) {
    int i;

    entity_list_add(&g_live_entities, e);

    for (i = 0; i < MESSAGE_COUNT; i++) {
        if (is_message_name(e, i)) {
            entity_list_add(&g_entities_by_message[i].list, e);
        }
    }

    for (i = 0; i < TAG_COUNT; i++) {
        if (e->class->tags & TAG(i)) {
            g_entities_by_tag[i].seen = 1;
            entity_list_add(&g_entities_by_tag[i].list, e);
        }
    }

    return e;
}

/* `e` must be unregistered under exactly the fields it was registered
 * under: a bm field changed since lt() leaves `e` in a list this never
 * removes it from. */
void entity_unregister(entity_t *e) {
    int i;

    /* TROU-5 */
    if (entity_list_del(&g_live_entities, e) && e->class->release != NULL) {
        exit_queue_put(e);
    }

    for (i = 0; i < MESSAGE_COUNT; i++) {
        if (is_message_name(e, i)) {
            entity_list_del(&g_entities_by_message[i].list, e);
        }
    }

    /* A tag never lt()'d: Lua would error on indexing its bucket, this
     * port deletes from an empty list. */
    for (i = 0; i < TAG_COUNT; i++) {
        if (e->class->tags & TAG(i)) {
            entity_list_del(&g_entities_by_tag[i].list, e);
        }
    }
}

void draw_layers(const char *message) {
    const entity_list_t *bucket_src;
    entity_list_t layers[15];
    int is_draw = strcmp(message, "draw") == 0;
    int index, layer_index;

    assert((is_draw || strcmp(message, "draw_overlay") == 0) && "dc: only kj and ev are drawn through dc");

    for (index = 0; index < 15; index++) {
        entity_list_init(&layers[index], frame_zone());
    }

    bucket_src = entities_by_message(message);
    if (bucket_src != NULL) {
        for (index = 0; index < bucket_src->count; index++) {
            entity_t *qc = bucket_src->items[index];
            int layer;

            /* An entity whose `.fa` is absent or outside 1..15 is dropped
             * from every layer, where Lua would error on the nil key. */
            if (qc->layer == NULL) {
                continue;
            }
            layer = p8_fixed_flr_int(*qc->layer);
            if (layer < 1 || layer > 15) {
                continue;
            }
            entity_list_add(&layers[layer - 1], qc);
        }
    }

    for (layer_index = 1; layer_index <= 15; layer_index++) {
        entity_list_t *bucket = &layers[layer_index - 1];

        if (is_draw && bucket->count > 0) {
            p8_fixed_t ci;
            /* The placeholder mo, before jl(), has no `md`. */
            if (pico8_fixed_seq_at(pico8_ci_table, PICO8_CI_COUNT, layer_index, &ci) && g_main_camera != NULL &&
                g_main_camera->class->apply_camera != NULL) {
                g_main_camera->class->apply_camera(g_main_camera, ci);
            }
        }

        for (index = 0; index < bucket->count; index++) {
            entity_t *qc = bucket->items[index];
            void (*draw)(entity_t *self, const vec_t *ra) = is_draw ? qc->class->draw : qc->class->draw_overlay;
            const vec_t *ra = qc->pos != NULL ? *qc->pos : NULL;

            if (draw != NULL) {
                draw(qc, ra);
            }
        }
    }
}

const entity_class_t *main_camera_placeholder_class(void) {
    return &g_main_camera_placeholder_class;
}

entity_t *main_camera(void) {
    return g_main_camera;
}

/* See pico8_entity.h's own comment on this declaration: the outgoing mo
 * lives on in its generation, given back when that generation is reset. */
void entity_registry_set_main_camera(entity_t *instance) {
    if (instance == g_main_camera) {
        return;
    }
    g_main_camera = instance;
}
