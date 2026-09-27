#include "pico8_ke.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "pico8_entity.h"
#include "pico8_fixed.h"
#include "pico8_ob.h"
#include "pico8_type_tag.h"

static pico8_ob_table_t *g_ke_spec = NULL;

void pico8_ke_init(void) {
    pico8_ke_shutdown();
    g_ke_spec = pico8_ob("\nme=\"ph\",t=0,\n", NULL);
}

void pico8_ke_shutdown(void) {
    pico8_ob_table_free(g_ke_spec);
    g_ke_spec = NULL;
}

int pico8_ke_ready(void) {
    return g_ke_spec != NULL;
}

int pico8_ke_default(const char *name, pico8_value_t *out_value) {
    return g_ke_spec != NULL && pico8_ob_table_get_name(g_ke_spec, name, out_value);
}

void pico8_ke_ks(pico8_struct_header_t *self, pico8_value_t me) {
    if (self == NULL) {
        return;
    }
    assert(pico8_type_tag_of(self) == PICO8_TYPE_TAG_STRUCT &&
           "ke:ks: self is a struct; no class instance descends from ke any more");
    ((pico8_ke_t *)self)->me = me;
    ((pico8_ke_t *)self)->t = pico8_value_fixed(pico8_fixed_from_int(0));
}

void pico8_ke_struct_init(pico8_ke_t *self, const pico8_struct_class_t *cls) {
    self->header.tag = PICO8_TYPE_TAG_STRUCT;
    self->header.cls = cls;
    self->me = pico8_value_string("ph");
    self->t = pico8_value_fixed(pico8_fixed_from_int(0));
}

int pico8_ke_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_ke_t *ke = self;
    const pico8_value_t *member;

    if (strcmp(name, "me") == 0) {
        member = &ke->me;
    } else if (strcmp(name, "t") == 0) {
        member = &ke->t;
    } else {
        return 0;
    }
    if (member->kind == PICO8_VALUE_REF && member->as.ref == NULL) {
        return 0;
    }
    *out_value = *member;
    return 1;
}

void pico8_ke_struct_pq(pico8_ke_t *self) {
    pico8_lt((pico8_struct_header_t *)self);
}
