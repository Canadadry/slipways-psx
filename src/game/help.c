#include "game/help.h"

#include <assert.h>
#include <string.h>

#include "engine/dispatch_context.h"
#include "engine/entity_geom.h"
#include "engine/entity.h"
#include "game/message_box.h"
#include "game/particle.h"
#include "game/draw_functions.h"
#include "game/probe.h"
#include "engine/entity_class.h"
#include "engine/game_zone.h"

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

static int pico8_fv_hd_dispatch(entity_t *self, const pico8_vec_t *qa, const pico8_vec_t *ra) {
    return pico8_gk(self, qa, ra);
}

/* The most items a row of `bo` holds. */
#define PICO8_BO_ROW_CAP 4

/* `pz(self, bo)`, each row's `my` the `eu` of this game. */
void pico8_fv_pq(pico8_g__t *self) {
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
        bs[i].trade = p8_fixed_from_int(e->trade);
        bs[i].has_en = 1;
        bs[i].en = e->en;
        bs[i].has_my = 1;
        bs[i].my = pico8_eu_field_value();
    }
    frame.has_eo = 1;
    frame.eo = self->def->eo;
    frame.hi = &self->def->hi;
    frame.ff = 1;
    frame.has_ca = 1;
    frame.ca = self->def->ca;
    frame.gq = &self->def->gq;
    frame.ra = self->ra;
    pico8_pz(&frame, bs, PICO8_BO_COUNT);
}

/* `fv.rb` holds four articles in the cart. */
enum { PICO8_FV_RB_CAP = 4 };

void pico8_fv_kj(p8_graphics_t *gfx, p8_memory_t *mem, pico8_g__t *self, pico8_vec_t p) {
    (void)p;

    pico8_ei(gfx, mem, (entity_t *)self);

    if (gfx != NULL && mem != NULL) {
        const pico8_jb_row_t *items[PICO8_FV_RB_CAP];
        int n = 0;
        int i;

        for (i = 0; self != NULL && self->def->rb != NULL && i < self->def->rb->count; i++) {
            assert(n < PICO8_FV_RB_CAP && "fv.rb has four articles in the cart");
            items[n++] = self->def->rb->items[i].row;
        }
        pico8_bl_default(gfx, mem, items, n);
    }
}

void pico8_fv_kj_dispatch(entity_t *self, const pico8_vec_t *ra) {
    pico8_vec_t p = ra != NULL ? *ra : pico8_vec_v(p8_fixed_from_int(0), p8_fixed_from_int(0));
    pico8_fv_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_g__t *)self, p);
}

static const entity_class_t g_fv_struct_class = {
    .draw = pico8_fv_kj_dispatch,
    .draw_overlay = pico8_dl_dispatch,
    .hit_test = pico8_fv_hd_dispatch,
    .drag = pico8_bv_dispatch,
    .drop = pico8_d__dispatch,
    .spawn_particle = pico8_g__mm,
    .release = pico8_g__rendre_dispatch,
    .tags = TAG(TAG_PROBE) | TAG(TAG_TARGETING),
    .lineage = LINEAGE(LINEAGE_PROBE),
};

static pico8_g__t *pico8_fv_struct_new(void) {
    pico8_g__t *self;

    assert(g_fv_up && "pico8_help_init() must run first");
    self = (pico8_g__t *)pico8_zone_alloc(pico8_zone_partie(), sizeof *self);
    pico8_g__struct_init(self, &g_fv_struct_class, &pico8_fv_def);
    self->header.hit_box = &self->def->kq;
    self->header.cursor_sprite = &self->def->ql;
    self->header.gpio_signal = &self->def->fg;
    self->header.is_hovered = &self->m_;
    self->m_ = 0;
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("help_screen");
#endif
    pico8_fv_pq(self);
    pico8_f__dans((entity_t *)self, PICO8_PARTICULES_QY_FV);
    pico8_ke_struct_pq(&self->header);
    return self;
}

/* This module, unlike most, does not call pico8_pi_init() itself, and `fv`
 * is built by `g_`'s init: both preconditions are asserted, not repaired. */
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

const entity_class_t *pico8_fv_struct_class(void) {
    return &g_fv_struct_class;
}

pico8_g__t *pico8_fv_new(void) {
    return pico8_fv_struct_new();
}
