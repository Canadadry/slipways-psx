#include "engine/entity_registry.h"

#include <assert.h>

#include "engine/entity_id.h"
#include "engine/exit_queue.h"
#include "game/game_data.h"
#include "engine/entity_class.h"
#include "engine/frame_zone.h"
#include "engine/game_zone.h"

static entity_list_t g_entities_by_message[ENTITY_MESSAGE_COUNT];

typedef struct {
    int seen;
    entity_list_t list;
} entities_by_tag_entry_t;

static entities_by_tag_entry_t g_entities_by_tag[TAG_COUNT];

static entity_list_t g_live_entities;
static entity_t *g_main_camera = NULL;
static int g_initialized = 0;

static const entity_class_t main_camera_placeholder_class_def = {0};

typedef struct {
    entity_t header;
} main_camera_placeholder_t;

static main_camera_placeholder_t g_main_camera_placeholder = {{.class = &main_camera_placeholder_class_def}};

void entity_registry_shutdown(void) {
    int i;

    if (!g_initialized) {
        return;
    }

    entity_list_free(&g_live_entities);

    for (i = 0; i < ENTITY_MESSAGE_COUNT; i++) {
        entity_list_free(&g_entities_by_message[i]);
    }

    for (i = 0; i < TAG_COUNT; i++) {
        entity_list_free(&g_entities_by_tag[i].list);
        g_entities_by_tag[i].seen = 0;
    }

    entity_id_table_forget();

    g_main_camera = NULL;

    g_initialized = 0;
}

void entity_registry_init(const entity_registry_reserve_t *reserve) {
    static const entity_registry_reserve_t no_reserve = {0};
    int i;

    if (reserve == NULL) reserve = &no_reserve;

    entity_registry_shutdown();

    entity_id_table_init(game_zone());

    entity_list_init_reserve(&g_live_entities, game_zone(), reserve->live);

    for (i = 0; i < ENTITY_MESSAGE_COUNT; i++) {
        entity_list_init_reserve(&g_entities_by_message[i], game_zone(), reserve->by_message[i]);
    }

    for (i = 0; i < TAG_COUNT; i++) {
        entity_list_init_reserve(&g_entities_by_tag[i].list, game_zone(), reserve->by_tag[i]);
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

    entity_id_table_forget();
    forget_list(&g_live_entities);
    for (i = 0; i < ENTITY_MESSAGE_COUNT; i++) {
        forget_list(&g_entities_by_message[i]);
    }
    for (i = 0; i < TAG_COUNT; i++) {
        forget_list(&g_entities_by_tag[i].list);
        g_entities_by_tag[i].seen = 0;
    }
}

const entity_list_t *live_entities(void) {
    return &g_live_entities;
}

const entity_list_t *entities_by_message(entity_message_t message) {
    assert(g_initialized && "entity_registry_init() must run before entities_by_message is read");
    return &g_entities_by_message[message];
}

static int is_message_name(const entity_t *e, entity_message_t message) {
    const entity_class_t *cls;

    cls = e->class;

    switch (message) {
    case ENTITY_MESSAGE_DRAW:
        return cls->draw != NULL;
    case ENTITY_MESSAGE_DRAW_OVERLAY:
        return cls->draw_overlay != NULL;
    case ENTITY_MESSAGE_HIT_TEST:
        return cls->hit_test != NULL;
    case ENTITY_MESSAGE_NEW_YEAR:
        return cls->new_year != NULL;
    default:
        return cls->refresh != NULL && (cls->has_refresh == NULL || cls->has_refresh(e));
    }
}

int entity_registry_hit_test_always(entity_t *self, mouse_pos_t mouse) {
    (void)self;
    (void)mouse;
    return 1;
}

const entity_list_t *entities_by_tag(tag_t tag) {
    return g_entities_by_tag[tag].seen ? &g_entities_by_tag[tag].list : NULL;
}

int entity_registry_count_tagged(tag_t tag) {
    const entity_list_t *tagged = entities_by_tag(tag);
    return tagged == NULL ? 0 : tagged->count;
}

/* Missing or malformed data is skipped where Lua would crash: message members are tested by presence, unregistering an unknown entity does nothing, draw_layers drops an entity with no layer or one outside 1..15. Making one site fatal alone breaks consistency. */
entity_t *entity_register(entity_t *e) {
    int i;

    entity_list_add(&g_live_entities, e);

    for (i = 0; i < ENTITY_MESSAGE_COUNT; i++) {
        if (is_message_name(e, (entity_message_t)i)) {
            entity_list_add(&g_entities_by_message[i], e);
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

/* A message member changed since entity_register() leaves the entity in a list this never removes it from. */
void entity_unregister(entity_t *e) {
    int i;

    if (entity_list_del(&g_live_entities, e)) {
        exit_queue_put(e);
    }

    for (i = 0; i < ENTITY_MESSAGE_COUNT; i++) {
        if (is_message_name(e, (entity_message_t)i)) {
            entity_list_del(&g_entities_by_message[i], e);
        }
    }

    for (i = 0; i < TAG_COUNT; i++) {
        if (e->class->tags & TAG(i)) {
            entity_list_del(&g_entities_by_tag[i].list, e);
        }
    }
}

void draw_layers(entity_message_t message) {
    const entity_list_t *message_bucket;
    entity_list_t layers[15];
    int is_draw = message == ENTITY_MESSAGE_DRAW;
    int index, layer_index;

    assert((is_draw || message == ENTITY_MESSAGE_DRAW_OVERLAY) && "draw_layers: only draw and draw_overlay go through draw_layers");

    for (index = 0; index < 15; index++) {
        entity_list_init(&layers[index], frame_zone());
    }

    message_bucket = entities_by_message(message);
    for (index = 0; index < message_bucket->count; index++) {
        entity_t *qc = message_bucket->items[index];
        int layer;

        if (!entity_has(qc, ENTITY_HAS_LAYER)) {
            continue;
        }
        layer = p8_fixed_flr_int(qc->layer);
        if (layer < 1 || layer > 15) {
            continue;
        }
        entity_list_add(&layers[layer - 1], qc);
    }

    for (layer_index = 1; layer_index <= 15; layer_index++) {
        entity_list_t *bucket = &layers[layer_index - 1];

        if (is_draw && bucket->count > 0) {
            p8_fixed_t ci;
            if (game_data_fixed_seq_at(layer_fades_table, LAYER_FADES_COUNT, layer_index, &ci) && g_main_camera != NULL &&
                g_main_camera->class->apply_camera != NULL) {
                g_main_camera->class->apply_camera(g_main_camera, ci);
            }
        }

        for (index = 0; index < bucket->count; index++) {
            entity_t *qc = bucket->items[index];
            void (*draw)(entity_t *self, vec_t screen_pos) = is_draw ? qc->class->draw : qc->class->draw_overlay;

            if (draw != NULL) {
                draw(qc, entity_pos(qc));
            }
        }
    }
}

const entity_class_t *main_camera_placeholder_class(void) {
    return &main_camera_placeholder_class_def;
}

entity_t *main_camera(void) {
    return g_main_camera;
}

void entity_registry_set_main_camera(entity_t *instance) {
    if (instance == g_main_camera) {
        return;
    }
    g_main_camera = instance;
}
