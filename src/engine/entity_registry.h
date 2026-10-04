#ifndef ENGINE_ENTITY_REGISTRY_H
#define ENGINE_ENTITY_REGISTRY_H

#include "engine/entity_list.h"
#include "engine/entity_class.h"

typedef enum {
    ENTITY_MESSAGE_DRAW,
    ENTITY_MESSAGE_DRAW_OVERLAY,
    ENTITY_MESSAGE_HIT_TEST,
    ENTITY_MESSAGE_NEW_YEAR,
    ENTITY_MESSAGE_REFRESH,
    ENTITY_MESSAGE_COUNT
} entity_message_t;

typedef struct {
    int live;
    int by_message[ENTITY_MESSAGE_COUNT];
    int by_tag[TAG_COUNT];
} entity_registry_reserve_t;

void entity_registry_init(const entity_registry_reserve_t *reserve);

void entity_registry_shutdown(void);

void entity_registry_abandon_live_entities(void);

void entity_registry_forget_registries(void);

const entity_list_t *live_entities(void);

const entity_list_t *entities_by_message(entity_message_t message);

const entity_list_t *entities_by_tag(tag_t tag);

entity_t *entity_register(entity_t *e);
void entity_unregister(entity_t *e);

void draw_layers(entity_message_t message);

int entity_registry_hit_test_always(entity_t *self, const vec_t *mouse_world, const vec_t *mouse_screen);

entity_t *main_camera(void);

const entity_class_t *main_camera_placeholder_class(void);

void entity_registry_set_main_camera(entity_t *instance);

#endif
