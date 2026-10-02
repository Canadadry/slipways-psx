#include "engine/entity.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "engine/entity_registry.h"
#include "p8/fixed.h"

static int g_ke_up = 0;

void pico8_ke_init(void) {
    pico8_ke_shutdown();
    g_ke_up = 1;
}

void pico8_ke_shutdown(void) {
    g_ke_up = 0;
}

int pico8_ke_ready(void) {
    return g_ke_up;
}

void pico8_ke_ks(entity_t *self, const char *me) {
    if (self == NULL) {
        return;
    }
    self->state = me;
    self->state_time = p8_fixed_from_int(0);
}

void pico8_ke_struct_init(entity_t *self, const entity_class_t *cls, const char *me,
                          p8_fixed_t t) {
    entity_init_header(self, cls);
    self->state = me;
    self->state_time = t;
}

void pico8_ke_struct_pq(entity_t *self) {
    pico8_lt(self);
}
