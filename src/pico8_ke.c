#include "pico8_ke.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "pico8_entity.h"
#include "pico8_fixed.h"
#include "pico8_game_data.h"

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

void pico8_ke_ks(pico8_struct_header_t *self, const char *me) {
    if (self == NULL) {
        return;
    }
    ((pico8_ke_t *)self)->me = me;
    ((pico8_ke_t *)self)->t = pico8_fixed_from_int(0);
}

void pico8_ke_struct_init(pico8_ke_t *self, const pico8_struct_class_t *cls) {
    self->header.cls = cls;
    self->me = pico8_ke_start.me;
    self->t = pico8_ke_start.t;
}

void pico8_ke_struct_pq(pico8_ke_t *self) {
    pico8_lt(self);
}
