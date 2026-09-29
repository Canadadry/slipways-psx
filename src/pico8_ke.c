#include "pico8_ke.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "pico8_entity.h"
#include "pico8_fixed.h"
#include "pico8_game_data.h"
#include "pico8_type_tag.h"

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

static void ke_take_string(pico8_value_t value, const char **out) {
    *out = NULL;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_STRING && value.as.string != NULL &&
           "ke.me: a write neither nil nor a string does not fit the member's type -- stop, do not widen the member");
    *out = value.as.string;
}

void pico8_ke_ks(pico8_struct_header_t *self, pico8_value_t me) {
    if (self == NULL) {
        return;
    }
    assert(pico8_type_tag_of(self) == PICO8_TYPE_TAG_STRUCT &&
           "ke:ks: self is a struct; no class instance descends from ke any more");
    ke_take_string(me, &((pico8_ke_t *)self)->me);
    ((pico8_ke_t *)self)->t = pico8_fixed_from_int(0);
}

void pico8_ke_struct_init(pico8_ke_t *self, const pico8_struct_class_t *cls) {
    self->header.tag = PICO8_TYPE_TAG_STRUCT;
    self->header.cls = cls;
    self->me = pico8_ke_start.me;
    self->t = pico8_ke_start.t;
}

int pico8_ke_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_ke_t *ke = self;

    if (strcmp(name, "me") == 0) {
        if (ke->me == NULL) {
            return 0;
        }
        *out_value = pico8_value_string(ke->me);
        return 1;
    }
    if (strcmp(name, "t") == 0) {
        *out_value = pico8_value_fixed(ke->t);
        return 1;
    }
    return 0;
}

void pico8_ke_struct_pq(pico8_ke_t *self) {
    pico8_lt(self);
}
