#include "engine/entity.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "engine/entity_registry.h"
#include "p8/fixed.h"

static int g_entity_up = 0;

void entity_init(void) {
    entity_shutdown();
    g_entity_up = 1;
}

void entity_shutdown(void) {
    g_entity_up = 0;
}

int entity_ready(void) {
    return g_entity_up;
}

void entity_set_state(entity_t *self, const char *me) {
    if (self == NULL) {
        return;
    }
    self->state = me;
    self->state_time = p8_fixed_from_int(0);
}

void entity_init_instance(entity_t *self, const entity_class_t *cls, const char *me,
                          p8_fixed_t t) {
    entity_init_header(self, cls);
    self->state = me;
    self->state_time = t;
}

void entity_construct(entity_t *self) {
    entity_register(self);
}
