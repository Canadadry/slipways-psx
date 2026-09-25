#include "pico8_mouse_reader.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "pico8_dispatch_context.h"
#include "pico8_ke.h"
#include "pico8_mouse.h"
#include "pico8_pi.h"
#include "pico8_value.h"
#include "pico8_vec.h"

static pico8_class_t *g_nf_class = NULL;

static void pico8_nf_pq_dispatch(pico8_instance_t *instance) {
    pico8_nf_pq(pico8_dispatch_mem(), instance);
}

/* dg() dispatches the per-tick state method as `qc[qc.me](qc,qc.t)`, so
 * arg0 is the frame counter, which nf:ph does not read. */
static pico8_value_t pico8_nf_ph_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_nf_ph(pico8_dispatch_input(), (pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

void pico8_nf_init(void) {
    pico8_nf_shutdown();
    pico8_pi_init();
    assert(pico8_ob_lookup_constructor('v') != NULL && "nf's spec parses v(0,0) as a vector");
    assert(pico8_ke_class() != NULL && "nf descends from ke");
    g_nf_class = pico8_class_declare(pico8_ke_class(), "\nl=o(),r=o(),ra=v(0,0),\n");
    PICO8_CLASS_COVERAGE_NAME(g_nf_class, "nf");
    pico8_class_set_pq(g_nf_class, pico8_nf_pq_dispatch);
    pico8_ob_table_set_name(g_nf_class->defaults, "ph", pico8_value_function(pico8_nf_ph_dispatch));
}

void pico8_nf_shutdown(void) {
    if (g_nf_class != NULL) {
        pico8_class_free(g_nf_class);
        g_nf_class = NULL;
    }
}

pico8_class_t *pico8_nf_class(void) {
    return g_nf_class;
}

pico8_instance_t *pico8_nf_new(const pico8_ob_table_t *overrides) {
    return pico8_class_instantiate(g_nf_class, overrides);
}

void pico8_nf_pq(pico8_mem_t *mem, pico8_instance_t *self) {
    (void)self;
    if (mem == NULL) {
        return;
    }
    pico8_poke(mem, 0x5f2d, 1);
}

void pico8_nf_ph(const pico8_input_t *input, pico8_instance_t *self) {
    pico8_vec_t *ra;
    pico8_value_t l_val;
    pico8_value_t r_val;

    if (self == NULL || input == NULL) {
        return;
    }

    /* TROU-7 */
    ra = malloc(sizeof *ra);
    *ra = pico8_vec_v(pico8_fixed_from_int(pico8_stat(input, 32)), pico8_fixed_from_int(pico8_stat(input, 33)));
    pico8_ob_table_set_name(self->fields, "ra", pico8_value_ref(ra));

    if (pico8_instance_get_field(self, "l", &l_val) && l_val.kind == PICO8_VALUE_REF && l_val.as.ref != NULL) {
        pico8_bu(input, (pico8_ob_table_t *)l_val.as.ref, 1);
    }
    if (pico8_instance_get_field(self, "r", &r_val) && r_val.kind == PICO8_VALUE_REF && r_val.as.ref != NULL) {
        pico8_bu(input, (pico8_ob_table_t *)r_val.as.ref, 6);
    }
}
