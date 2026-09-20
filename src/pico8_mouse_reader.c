#include "pico8_mouse_reader.h"

#include <stddef.h>
#include <stdlib.h>

#include "pico8_dispatch_context.h"
#include "pico8_ke.h"
#include "pico8_mouse.h"
#include "pico8_pi.h"
#include "pico8_value.h"
#include "pico8_vec.h"

/* See pico8_mouse_reader.h's own top comment for the full source/trace. */

static pico8_class_t *g_nf_class = NULL;

/* Bridges pico8_class_pq_fn's fixed shape to pico8_nf_pq's own real
 * signature -- see pico8_mouse_reader.h's own comment on pico8_nf_pq. */
static void pico8_nf_pq_dispatch(pico8_instance_t *instance) {
    pico8_nf_pq(pico8_dispatch_mem(), instance);
}

/* Bridges pico8_value_function_t's fixed (self,arg) shape to pico8_nf_ph's
 * own real signature -- `arg` (dg()'s own `qc.t`) is unused, matching the
 * real Lua body's own arity. See pico8_mouse_reader.h's own comment on
 * pico8_nf_ph. */
static pico8_value_t pico8_nf_ph_dispatch(void *self, pico8_value_t arg) {
    (void)arg;
    pico8_nf_ph(pico8_dispatch_input(), (pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}

/* nf=ke:jr([[
 * l=o(),r=o(),ra=v(0,0),
 * ]]) -- see pico8_mouse_reader.h's own top comment. Calls pico8_pi_init()
 * defensively (idempotent, per pico8_pi.h's own comment) before declaring
 * this class: the spec string's own `v(0,0)` needs Chunk 2's 'v' char-
 * keyed pi registration to already exist, or ob() would silently produce
 * a raw positional table instead of a real pico8_vec_t-backed REF for
 * `ra` -- a real, evidenced ordering hazard this chunk is the first to hit
 * (Chunk 3's own ke/lq/jb/trade specs never used `v(...)`/`o(...)`), fixed
 * here defensively rather than left to caller discipline given how
 * silently wrong the alternative would be. */
void pico8_nf_init(void) {
    pico8_nf_shutdown();
    pico8_pi_init();
    g_nf_class = pico8_class_declare(pico8_ke_class(), "\nl=o(),r=o(),ra=v(0,0),\n");
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

/* function nf:pq()poke(0x5f2d,1)end -- see pico8_mouse_reader.h. */
void pico8_nf_pq(pico8_mem_t *mem, pico8_instance_t *self) {
    (void)self;
    if (mem == NULL) {
        return;
    }
    pico8_poke(mem, 0x5f2d, 1);
}

/* function nf:ph()self.ra=v(stat(32),stat(33))bu(self.l,1)bu(self.r,6)end
 * -- see pico8_mouse_reader.h. */
void pico8_nf_ph(const pico8_input_t *input, pico8_instance_t *self) {
    pico8_vec_t *ra;
    pico8_value_t l_val;
    pico8_value_t r_val;

    if (self == NULL || input == NULL) {
        return;
    }

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
