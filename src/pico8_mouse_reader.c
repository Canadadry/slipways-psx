#include "pico8_mouse_reader.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_dispatch_context.h"
#include "pico8_game_data.h"
#include "pico8_mouse.h"
#include "pico8_pi.h"
#include "pico8_vec.h"

static int g_nf_up = 0;
static pico8_ob_table_t *g_nf_l = NULL;
static pico8_ob_table_t *g_nf_r = NULL;
/* Not `&pico8_nf_start.ra`: two TUs write `je.ra` in place. */
static pico8_vec_t g_nf_ra;

/* dg() dispatches the per-tick state method as `qc[qc.me](qc,qc.t)`, so
 * arg0 is the frame counter, which nf:ph does not read. */
static pico8_value_t pico8_nf_ph_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_nf_ph(pico8_dispatch_input(), (pico8_nf_t *)self);
    return pico8_value_ref(NULL);
}

static int pico8_nf_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_nf_t *nf = self;
    pico8_value_t value;

    if (strcmp(name, "ra") == 0) {
        if (nf->ra == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref(nf->ra);
        return 1;
    } else if (strcmp(name, "l") == 0) {
        value = pico8_value_ref(g_nf_l);
    } else if (strcmp(name, "r") == 0) {
        value = pico8_value_ref(g_nf_r);
    } else {
        return pico8_ke_field(self, name, out_value);
    }
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return 0;
    }
    *out_value = value;
    return 1;
}

static const pico8_struct_class_t g_nf_struct_class = {
    {[PICO8_METHOD_PH] = pico8_nf_ph_dispatch},
    pico8_nf_field,
    NULL,
};

static void pico8_nf_pq(pico8_mem_t *mem) {
    if (mem == NULL) {
        return;
    }
    pico8_poke(mem, 0x5f2d, 1);
}

static void *pico8_nf_struct_new(void) {
    pico8_nf_t *self = malloc(sizeof *self);

    pico8_ke_struct_init(&self->ke, &g_nf_struct_class);
    self->ra = &g_nf_ra;
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("nf");
#endif
    pico8_nf_pq(pico8_dispatch_mem());
    pico8_ke_struct_pq(&self->ke);
    return self;
}

void pico8_nf_init(void) {
    pico8_nf_shutdown();
    pico8_pi_init();
    assert(pico8_ke_ready() && "nf descends from ke");
    g_nf_up = 1;
    g_nf_ra = pico8_nf_start.ra;
    g_nf_l = pico8_ob_table_new();
    g_nf_r = pico8_ob_table_new();
}

void pico8_nf_shutdown(void) {
    g_nf_up = 0;
    pico8_ob_table_free(g_nf_l);
    pico8_ob_table_free(g_nf_r);
    g_nf_l = NULL;
    g_nf_r = NULL;
}

int pico8_nf_ready(void) {
    return g_nf_up;
}

const pico8_struct_class_t *pico8_nf_struct_class(void) {
    return &g_nf_struct_class;
}

pico8_nf_t *pico8_nf_new(void) {
    return pico8_nf_struct_new();
}

void pico8_nf_ph(const pico8_input_t *input, pico8_nf_t *self) {
    pico8_vec_t *ra;

    if (self == NULL || input == NULL) {
        return;
    }

    /* TROU-7 */
    ra = malloc(sizeof *ra);
    *ra = pico8_vec_v(pico8_fixed_from_int(pico8_stat(input, 32)), pico8_fixed_from_int(pico8_stat(input, 33)));
    self->ra = ra;

    if (g_nf_l != NULL) {
        pico8_bu(input, g_nf_l, 1);
    }
    if (g_nf_r != NULL) {
        pico8_bu(input, g_nf_r, 6);
    }
}
