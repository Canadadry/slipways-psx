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
static vec_t g_nf_ra;

/* dg() dispatches the per-tick state method as `qc[qc.me](qc,qc.t)`, so
 * `t` is the frame counter, which nf:ph does not read. */
static void pico8_nf_ph_dispatch(entity_t *self, p8_fixed_t t) {
    (void)t;
    pico8_nf_ph(dispatch_context_input(), (pico8_nf_t *)self);
}

static const entity_class_t g_nf_struct_class = {
    .states = {[ENTITY_STATE_NORMAL] = pico8_nf_ph_dispatch},
};

static void pico8_nf_pq(p8_memory_t *mem) {
    if (mem == NULL) {
        return;
    }
    p8_poke(mem, 0x5f2d, 1);
}

static pico8_nf_t *pico8_nf_struct_new(void) {
    pico8_nf_t *self = (pico8_nf_t *)allocator_alloc(game_zone(), sizeof *self);

    entity_init_instance(&self->header, &g_nf_struct_class, pico8_nf_def.me, pico8_nf_def.t);
    self->def = &pico8_nf_def;
    /* Read-only: this module writes through its member. */
    self->header.pos = (const vec_t *const *)&self->ra;
    self->ra = &g_nf_ra;
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("mouse");
#endif
    pico8_nf_pq(dispatch_context_mem());
    entity_construct(&self->header);
    return self;
}

void pico8_nf_init(void) {
    pico8_nf_shutdown();
    pico8_pi_init();
    assert(entity_ready() && "nf descends from ke");
    g_nf_up = 1;
    g_nf_ra = vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
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

const entity_class_t *pico8_nf_struct_class(void) {
    return &g_nf_struct_class;
}

pico8_nf_t *pico8_nf_new(void) {
    return pico8_nf_struct_new();
}

void pico8_nf_ph(const p8_input_t *input, pico8_nf_t *self) {
    if (self == NULL || input == NULL) {
        return;
    }

    self->ra_own = vec_make(p8_fixed_from_int(p8_stat(input, 32)), p8_fixed_from_int(p8_stat(input, 33)));
    self->ra = &self->ra_own;

    if (g_nf_up) {
        pico8_bu(input, &g_nf_l, 1);
        pico8_bu(input, &g_nf_r, 6);
    }
}
