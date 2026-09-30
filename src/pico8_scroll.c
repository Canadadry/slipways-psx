#include "pico8_scroll.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_dispatch_context.h"
#include "pico8_entity_geom.h"
#include "pico8_pi.h"
#include "pico8_vec.h"

static int g_qb_up = 0;

/* NULL reads as the zero vector. */
static pico8_vec_t pico8_qb_vec(const pico8_vec_t *v) {
    if (v != NULL) {
        return *v;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

/* dg() dispatches the per-tick state method as `qc[qc.me](qc,qc.t)`, so
 * `t` is the frame counter, which qb:ph does not read. */
static void pico8_qb_ph_dispatch(void *self, pico8_fixed_t t) {
    (void)t;
    pico8_qb_ph(pico8_dispatch_input(), (pico8_qb_t *)self);
}

static const pico8_vec_t *qb_get_ra(const void *self) {
    return ((const pico8_qb_t *)self)->ra;
}

static const pico8_vec_t *qb_get_p(const void *self) {
    return ((const pico8_qb_t *)self)->p;
}

static const pico8_struct_class_t g_qb_struct_class = {
    .states = {[PICO8_STATE_PH] = pico8_qb_ph_dispatch},
    .get_ra = qb_get_ra,
    .get_p = qb_get_p,
};

static void *pico8_qb_struct_new(void) {
    pico8_qb_t *self = malloc(sizeof *self);

    pico8_ke_struct_init(&self->ke.header, &g_qb_struct_class);
    self->v = &pico8_qb_start.v;
    self->ma = pico8_qb_start.ma;
    self->p = &pico8_qb_start.p;
    self->ra = &pico8_qb_start.ra;
    self->pc = pico8_qb_start.pc;
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("qb");
#endif
    pico8_ke_struct_pq(&self->ke.header);
    return self;
}

void pico8_qb_init(void) {
    pico8_qb_shutdown();
    pico8_pi_init();
    assert(pico8_ke_ready() && "qb descends from ke");
    g_qb_up = 1;
}

void pico8_qb_shutdown(void) {
    g_qb_up = 0;
}

int pico8_qb_ready(void) {
    return g_qb_up;
}

const pico8_struct_class_t *pico8_qb_struct_class(void) {
    return &g_qb_struct_class;
}

pico8_qb_t *pico8_qb_new(void) {
    return pico8_qb_struct_new();
}

void pico8_qb_ph(const pico8_input_t *input, pico8_qb_t *self) {
    pico8_vec_t ip = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    const pico8_vec_seq_t *pc = NULL;
    pico8_vec_t p;
    pico8_vec_t v;
    pico8_vec_t *p_new;
    pico8_vec_t *v_new;
    pico8_vec_t *ra_new;
    int b;

    if (self == NULL || input == NULL) {
        return;
    }

    pc = self->pc;

    for (b = 0; b <= 3; b++) {
        int btn_p1 = pico8_btn(input, b);
        /* TROU-2 */
        int btn_p2 = 0;

        if (btn_p1 || btn_p2) {
            pico8_vec_t dir = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
            if (pc != NULL && b < pc->count) {
                dir = pc->items[b];
            }
            ip = pico8_vec_add(ip, pico8_vec_mul(dir, pico8_fixed_from_int(3)));
        }
    }

    p = pico8_qb_vec(self->p);
    v = pico8_qb_vec(self->v);

    p = pico8_vec_add(p, v);
    v = pico8_vec_add(v, pico8_vec_mul(pico8_vec_sub(ip, v), PICO8_FIXED_LIT(0x00003333, 0.2)));

    p.x = pico8_fixed_mid(pico8_fixed_from_int(-64), pico8_fixed_from_int(320), p.x);
    p.y = pico8_fixed_mid(pico8_fixed_from_int(-64), pico8_fixed_from_int(320), p.y);

    /* TROU-7 */
    p_new = malloc(sizeof *p_new);
    *p_new = p;
    v_new = malloc(sizeof *v_new);
    *v_new = v;
    self->p = p_new;
    self->v = v_new;

    ra_new = malloc(sizeof *ra_new);
    *ra_new = pico8_ma(p, (pico8_struct_header_t *)self);
    self->ra = ra_new;
}

void pico8_qb_md(pico8_gfx_t *gfx, pico8_qb_t *self, pico8_fixed_t fy) {
    pico8_vec_t cp;

    if (self == NULL || gfx == NULL) {
        return;
    }

    cp = pico8_vec_mul(pico8_qb_vec(self->ra), fy);
    pico8_camera(gfx, pico8_fixed_flr_int(cp.x), pico8_fixed_flr_int(cp.y));
}
