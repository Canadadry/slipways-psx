#include "pico8_help.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_data_init.h"
#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_ke.h"
#include "pico8_msgbox.h"
#include "pico8_particle.h"
#include "pico8_pi.h"
#include "pico8_probe.h"
#include "pico8_struct.h"

static int g_fv_up = 0;

static void pico8_fv_eu_placeholder(pico8_js_t *self) {
    (void)self;
}

static pico8_js_fn g_eu_dispatch = pico8_fv_eu_placeholder;

pico8_js_fn pico8_eu_field_value(void) {
    return g_eu_dispatch;
}

void pico8_help_set_eu(pico8_js_fn fn) {
    g_eu_dispatch = (fn != NULL) ? fn : pico8_fv_eu_placeholder;
}

static int pico8_fv_hd_dispatch(void *self, const pico8_vec_t *qa, const pico8_vec_t *ra) {
    return pico8_gk((const pico8_struct_header_t *)self, qa, ra);
}

/* The most items a row of `bo` holds. */
#define PICO8_BO_ROW_CAP 4

/* `pz(self, bo)`, each row's `my` the `eu` of this game. */
void pico8_fv_pq(pico8_fv_t *self) {
    pico8_js_keys_t bs[PICO8_BO_COUNT];
    pico8_lq_keys_t lq[PICO8_BO_COUNT];
    pico8_lq_cell_t items[PICO8_BO_COUNT][PICO8_BO_ROW_CAP];
    pico8_pz_frame_t frame = {0};
    int i;

    if (self == NULL) {
        return;
    }
    for (i = 0; i < PICO8_BO_COUNT; i++) {
        const pico8_bo_entry_t *e = &pico8_bo_table[i];

        memset(&bs[i], 0, sizeof bs[i]);
        lq[i] = pico8_lq_keys_of_row(e->lq, items[i], PICO8_BO_ROW_CAP);
        bs[i].lq_row = &lq[i];
        bs[i].px = e->px;
        bs[i].has_trade = 1;
        bs[i].trade = pico8_fixed_from_int(e->trade);
        bs[i].has_en = 1;
        bs[i].en = e->en;
        bs[i].has_my = 1;
        bs[i].my = pico8_eu_field_value();
    }
    frame.has_eo = 1;
    frame.eo = self->g_.def->eo;
    frame.hi = &self->g_.def->hi;
    frame.ff = 1;
    frame.has_ca = 1;
    frame.ca = self->g_.def->ca;
    frame.gq = &self->g_.def->gq;
    frame.ra = self->g_.ra;
    pico8_pz(&frame, bs, PICO8_BO_COUNT);
}

/* `fv.rb` holds four articles in the cart. */
enum { PICO8_FV_RB_CAP = 4 };

void pico8_fv_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_fv_t *self, pico8_vec_t p) {
    (void)p;

    pico8_ei(gfx, mem, (pico8_struct_header_t *)self);

    if (gfx != NULL && mem != NULL) {
        const pico8_jb_row_t *items[PICO8_FV_RB_CAP];
        int n = 0;
        int i;

        for (i = 0; self != NULL && self->g_.def->rb != NULL && i < self->g_.def->rb->count; i++) {
            assert(n < PICO8_FV_RB_CAP && "fv.rb has four articles in the cart");
            items[n++] = self->g_.def->rb->items[i].row;
        }
        pico8_bl_default(gfx, mem, items, n);
    }
}

void pico8_fv_kj_dispatch(void *self, const pico8_vec_t *ra) {
    pico8_vec_t p = ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    pico8_fv_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_fv_t *)self, p);
}

static void fv_set_m_(void *self, int m_) {
    pico8_fv_t *fv = self;

    fv->m_ = m_ != 0;
    fv->has_m_ = 1;
}

static const pico8_struct_class_t g_fv_struct_class = {
    .kj = pico8_fv_kj_dispatch,
    .ev = pico8_dl_dispatch,
    .hd = pico8_fv_hd_dispatch,
    .hq = pico8_bv_dispatch,
    .ey = pico8_d__dispatch,
    .mm = pico8_g__mm,
    .parent = pico8_g__struct_class,
    .emitter_of = pico8_g__emitter_of,
    .probe_of = pico8_g__probe_of,
    .tags = PICO8_TAG(PICO8_TAG_G_) | PICO8_TAG(PICO8_TAG_PROBER),
    .get_fx = pico8_g__get_fx,
    .get_p_ = pico8_g__get_p_,
    .set_m_ = fv_set_m_,
};

static pico8_fv_t *pico8_fv_struct_new(void) {
    pico8_fv_t *self;

    assert(g_fv_up && "pico8_help_init() must run first");
    self = malloc(sizeof *self);
    pico8_g__struct_init(&self->g_, &g_fv_struct_class, &pico8_fv_def);
    self->g_.header.kq = &self->g_.def->kq;
    self->g_.header.ql = &self->g_.def->ql;
    self->g_.header.fg = &self->g_.def->fg;
    self->m_ = 0;
    self->has_m_ = 0;
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("fv");
#endif
    pico8_fv_pq(self);
    pico8_f_((pico8_struct_header_t *)self);
    pico8_ke_struct_pq(&self->g_.header);
    return self;
}

/* This module, unlike most, does not call pico8_pi_init() itself, and `fv`
 * extends `g_`, not `ke`: both preconditions are asserted, not repaired. */
void pico8_help_init(void) {
    assert(pico8_pi_dispatch_lookup("s") != NULL && "pico8_help_init() runs after pico8_pi_init()");
    assert(pico8_g__ready() && "pico8_help_init() runs after pico8_probe_init()");

    pico8_help_shutdown();

    g_fv_up = 1;

    pico8_help_set_eu(NULL);
}

void pico8_help_shutdown(void) {
    g_fv_up = 0;
    pico8_help_set_eu(NULL);
}

int pico8_fv_ready(void) {
    return g_fv_up;
}

const pico8_struct_class_t *pico8_fv_struct_class(void) {
    return &g_fv_struct_class;
}

pico8_fv_t *pico8_fv_new(void) {
    return pico8_fv_struct_new();
}
