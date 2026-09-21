#include "pico8_ke.h"

#include <stddef.h>

#include "pico8_entity.h"
#include "pico8_fixed.h"
#include "pico8_ob.h"

static pico8_class_t *g_ke_class = NULL;

/* Bridges pico8_class_pq_fn (`void(*)(pico8_instance_t*)`) to pico8_lt's
 * own real signature (`pico8_instance_t*(*)(pico8_instance_t*)`) -- see
 * pico8_ke.h's own top comment on `ke.pq=lt`. lt()'s returned pointer
 * (always the same instance it was given) is simply discarded. */
static void pico8_ke_pq_lt(pico8_instance_t *instance) {
    pico8_lt(instance);
}

void pico8_ke_init(void) {
    pico8_ke_shutdown();
    g_ke_class = pico8_class_declare(NULL, "\nme=\"ph\",t=0,\n");
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

/* function ke:ks(me)self.me,self.t=me,0
 * end
 * See pico8_ke.h's own comment on this declaration. */
void pico8_ke_ks(pico8_instance_t *self, pico8_value_t me) {
    if (self == NULL) {
        return;
    }
    pico8_ob_table_set_name(self->fields, "me", me);
    pico8_ob_table_set_name(self->fields, "t", pico8_value_fixed(pico8_fixed_from_int(0)));
}
