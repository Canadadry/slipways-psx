#include <assert.h>
#include <stdio.h>

#include "engine/entity.h"
#include "engine/entity_registry.h"
#include "engine/exit_queue.h"
#include "engine/game_zone.h"

static const entity_class_t tagged_cls = {.tags = TAG(TAG_PROBE)};
static const entity_class_t untagged_cls = {0};

static entity_t g_entities[3];

static void registry_world(void) {
    entity_registry_shutdown();
    game_zone_reset();
    entity_registry_init(NULL);
}

static entity_t *fresh_entity(int i, const entity_class_t *cls) {
    entity_t none = {0};
    g_entities[i] = none;
    g_entities[i].class = cls;
    return &g_entities[i];
}

static void a_tag_never_registered_counts_zero(void) {
    registry_world();
    entity_register(fresh_entity(0, &untagged_cls));
    assert(entities_by_tag(TAG_PROBE) == NULL);
    assert(entity_registry_count_tagged(TAG_PROBE) == 0);
}

static void the_count_follows_registered_entities_of_the_tag(void) {
    registry_world();
    entity_t *first = fresh_entity(0, &tagged_cls);
    entity_register(first);
    entity_register(fresh_entity(1, &tagged_cls));
    entity_register(fresh_entity(2, &untagged_cls));
    assert(entity_registry_count_tagged(TAG_PROBE) == 2);
    entity_unregister(first);
    exit_queue_flush();
    assert(entity_registry_count_tagged(TAG_PROBE) == 1);
}

int main(void) {
    a_tag_never_registered_counts_zero();
    the_count_follows_registered_entities_of_the_tag();
    printf("test_entity_registry: all tests passed\n");
    return 0;
}
