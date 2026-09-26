#include "pico8_ke.h"

#include <stddef.h>
#include <string.h>

#include "pico8_entity.h"
#include "pico8_fixed.h"
#include "pico8_ob.h"

static pico8_class_t *g_ke_class = NULL;

/* Bridges pico8_class_pq_fn to pico8_lt, whose returned pointer is always
 * the instance it was given and is discarded here. */
static void pico8_ke_pq_lt(pico8_instance_t *instance) {
    pico8_lt(instance);
}

void pico8_ke_init(void) {
    pico8_ke_shutdown();
    g_ke_class = pico8_class_declare(NULL, "\nme=\"ph\",t=0,\n");
    PICO8_CLASS_COVERAGE_NAME(g_ke_class, "ke");
    pico8_class_set_pq(g_ke_class, pico8_ke_pq_lt);
}

void pico8_ke_shutdown(void) {
    if (g_ke_class != NULL) {
        pico8_class_free(g_ke_class);
        g_ke_class = NULL;
    }
}

pico8_class_t *pico8_ke_class(void) {
    return g_ke_class;
}

void pico8_ke_ks(pico8_instance_t *self, pico8_value_t me) {
    if (self == NULL) {
        return;
    }
    if (pico8_type_tag_of(self) == PICO8_TYPE_TAG_STRUCT) {
        ((pico8_ke_t *)self)->me = me;
        ((pico8_ke_t *)self)->t = pico8_value_fixed(pico8_fixed_from_int(0));
        return;
    }
    pico8_ob_table_set_name(pico8_instance_fields(self), "me", me);
    pico8_ob_table_set_name(pico8_instance_fields(self), "t", pico8_value_fixed(pico8_fixed_from_int(0)));
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
    pico8_lt((pico8_instance_t *)self);
}
