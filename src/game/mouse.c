#include "game/mouse.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "engine/dispatch_context.h"
#include "game/game_data.h"
#include "game/mouse_button.h"
#include "game/draw_functions.h"
#include "engine/vec.h"
#include "engine/game_zone.h"

static int g_nf_up = 0;
static pico8_nf_button_t g_nf_l;
static pico8_nf_button_t g_nf_r;
/* Not in the archetype: two TUs write `je.ra` in place. */
static pico8_vec_t g_nf_ra;

/* dg() dispatches the per-tick state method as `qc[qc.me](qc,qc.t)`, so
 * `t` is the frame counter, which nf:ph does not read. */
static void pico8_nf_ph_dispatch(pico8_struct_header_t *self, pico8_fixed_t t) {
    (void)t;
    pico8_nf_ph(pico8_dispatch_input(), (pico8_nf_t *)self);
}

static const pico8_struct_class_t g_nf_struct_class = {
    .states = {[PICO8_STATE_PH] = pico8_nf_ph_dispatch},
};

static void pico8_nf_pq(pico8_mem_t *mem) {
    if (mem == NULL) {
        return;
    }
    pico8_poke(mem, 0x5f2d, 1);
}

static pico8_nf_t *pico8_nf_struct_new(void) {
    pico8_nf_t *self = (pico8_nf_t *)pico8_zone_alloc(pico8_zone_partie(), sizeof *self);

    pico8_ke_struct_init(&self->header, &g_nf_struct_class, pico8_nf_def.me, pico8_nf_def.t);
    self->def = &pico8_nf_def;
    /* Read-only: this module writes through its member. */
    self->header.ra = (const pico8_vec_t *const *)&self->ra;
    self->ra = &g_nf_ra;
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("nf");
#endif
    pico8_nf_pq(pico8_dispatch_mem());
    pico8_ke_struct_pq(&self->header);
    return self;
}

void pico8_nf_init(void) {
    pico8_nf_shutdown();
    pico8_pi_init();
    assert(pico8_ke_ready() && "nf descends from ke");
    g_nf_up = 1;
    g_nf_ra = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    memset(&g_nf_l, 0, sizeof g_nf_l);
    memset(&g_nf_r, 0, sizeof g_nf_r);
}

void pico8_nf_shutdown(void) {
    g_nf_up = 0;
}

const pico8_nf_button_t *pico8_nf_l(void) {
    return g_nf_up ? &g_nf_l : NULL;
}

const pico8_nf_button_t *pico8_nf_r(void) {
    return g_nf_up ? &g_nf_r : NULL;
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
    if (self == NULL || input == NULL) {
        return;
    }

    self->ra_own = pico8_vec_v(pico8_fixed_from_int(pico8_stat(input, 32)), pico8_fixed_from_int(pico8_stat(input, 33)));
    self->ra = &self->ra_own;

    if (g_nf_up) {
        pico8_bu(input, &g_nf_l, 1);
        pico8_bu(input, &g_nf_r, 6);
    }
}
