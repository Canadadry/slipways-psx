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
#include "pico8_type_tag.h"

static int g_fv_up = 0;

static pico8_value_t pico8_fv_eu_placeholder(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)self;
    (void)argc;
    (void)arg0;
    (void)arg1;
    return pico8_value_ref(NULL);
}

static pico8_value_function_t g_eu_dispatch = pico8_fv_eu_placeholder;

pico8_value_t pico8_eu_field_value(void) {
    return pico8_value_function(g_eu_dispatch);
}

void pico8_help_set_eu(pico8_value_function_t fn) {
    g_eu_dispatch = (fn != NULL) ? fn : pico8_fv_eu_placeholder;
}

static pico8_value_t pico8_fv_hd_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    return pico8_gk((const pico8_struct_header_t *)self, arg0, arg1);
}

/* The most items a row of `bo` holds. */
#define PICO8_BO_ROW_CAP 4

/* `pz(self, bo)`, each row's `my` the `eu` of this game. */
void pico8_fv_pq(pico8_fv_t *self) {
    pico8_js_keys_t bs[PICO8_BO_COUNT];
    pico8_lq_keys_t lq[PICO8_BO_COUNT];
    pico8_value_t items[PICO8_BO_COUNT][PICO8_BO_ROW_CAP];
    int i;

    if (self == NULL) {
        return;
    }
    for (i = 0; i < PICO8_BO_COUNT; i++) {
        const pico8_bo_entry_t *e = &pico8_bo_table[i];

        memset(&bs[i], 0, sizeof bs[i]);
        lq[i] = pico8_lq_keys_of_row(e->lq, items[i], PICO8_BO_ROW_CAP);
        bs[i].lq = pico8_value_ref(NULL);
        bs[i].lq_row = &lq[i];
        bs[i].px = e->px;
        bs[i].has_trade = 1;
        bs[i].trade = pico8_fixed_from_int(e->trade);
        bs[i].has_en = 1;
        bs[i].en = e->en;
        bs[i].has_my = 1;
        bs[i].my = pico8_eu_field_value().as.function;
    }
    pico8_pz((const pico8_struct_header_t *)self, bs, PICO8_BO_COUNT);
}

void pico8_fv_kj(pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_fv_t *self, pico8_vec_t p) {
    (void)p;

    pico8_ei(gfx, mem, (pico8_struct_header_t *)self);

    if (gfx != NULL && mem != NULL) {
        pico8_list_t items;
        int i;

        pico8_list_init(&items);
        for (i = 0; self != NULL && self->rb != NULL && i < self->rb->count; i++) {
            pico8_list_add(&items, pico8_value_ref((void *)self->rb->items[i].row));
        }
        pico8_bl_default(gfx, mem, &items);
        pico8_list_free(&items);
    }
}

pico8_value_t pico8_fv_kj_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    pico8_vec_t p = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    (void)argc;
    (void)arg1;
    if (arg0.kind == PICO8_VALUE_REF && arg0.as.ref != NULL) {
        p = *(const pico8_vec_t *)arg0.as.ref;
    }
    pico8_fv_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_fv_t *)self, p);
    return pico8_value_ref(NULL);
}

static int pico8_fv_field(const void *self, const char *name, pico8_value_t *out_value) {
    const pico8_fv_t *fv = self;

    if (strcmp(name, "hi") == 0) {
        if (fv->hi == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref((void *)fv->hi);
        return 1;
    }
    if (strcmp(name, "ca") == 0) {
        *out_value = pico8_value_fixed(fv->ca);
        return 1;
    }
    if (strcmp(name, "gq") == 0) {
        if (fv->gq == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref((void *)fv->gq);
        return 1;
    }
    if (strcmp(name, "eo") == 0) {
        *out_value = pico8_value_fixed(fv->eo);
        return 1;
    }
    if (strcmp(name, "ff") == 0) {
        *out_value = pico8_value_fixed(fv->ff);
        return 1;
    }
    if (strcmp(name, "kq") == 0) {
        if (fv->kq == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref((void *)fv->kq);
        return 1;
    }
    if (strcmp(name, "ql") == 0) {
        *out_value = pico8_value_fixed(fv->ql);
        return 1;
    }
    if (strcmp(name, "fg") == 0) {
        *out_value = pico8_value_fixed(fv->fg);
        return 1;
    }
    if (strcmp(name, "rb") == 0) {
        if (fv->rb == NULL) {
            return 0;
        }
        *out_value = pico8_value_ref((void *)fv->rb);
        return 1;
    }
    if (strcmp(name, "m_") == 0) {
        if (!fv->has_m_) {
            return 0;
        }
        *out_value = pico8_value_bool(fv->m_);
        return 1;
    }
    return pico8_g__struct_class()->field(self, name, out_value);
}

static void help_take_bool(pico8_value_t value, int *out, int *has) {
    *out = 0;
    *has = 0;
    if (value.kind == PICO8_VALUE_REF && value.as.ref == NULL) {
        return;
    }
    assert(value.kind == PICO8_VALUE_BOOL &&
           "fv.m_: a write neither nil nor a boolean does not fit the member's type -- stop, do not widen the member");
    *out = value.as.boolean;
    *has = 1;
}

/* `m_` (`gw`, hovering the help), then g_'s 5; 0 for any other. */
static int pico8_fv_set_field(void *self, const char *name, pico8_value_t value) {
    if (strcmp(name, "m_") == 0) {
        pico8_fv_t *fv = self;
        help_take_bool(value, &fv->m_, &fv->has_m_);
        return 1;
    }
    return pico8_g__struct_class()->set_field(self, name, value);
}

static const pico8_struct_class_t g_fv_struct_class = {
    {
        [PICO8_METHOD_HQ] = pico8_bv_dispatch,
        [PICO8_METHOD_EY] = pico8_d__dispatch,
        [PICO8_METHOD_EV] = pico8_dl_dispatch,
        [PICO8_METHOD_MM] = pico8_g__mm,
        [PICO8_METHOD_KJ] = pico8_fv_kj_dispatch,
        [PICO8_METHOD_HD] = pico8_fv_hd_dispatch,
    },
    pico8_fv_field,
    pico8_fv_set_field,
};

static void *pico8_fv_struct_new(void) {
    pico8_fv_t *self;

    assert(g_fv_up && "pico8_help_init() must run first");
    self = malloc(sizeof *self);
    pico8_g__struct_init(&self->g_, &g_fv_struct_class);
    self->m_ = 0;
    self->has_m_ = 0;
    self->g_.fa = pico8_fv_start.fa;
    self->g_.ra = &pico8_fv_start.ra;
    self->g_.fc = pico8_fv_start.fc;
    self->g_.k_ = pico8_fv_start.k_;
    self->g_.qo = pico8_fv_start.qo;
    self->g_.ro = pico8_fv_start.ro;
    self->g_.qg = pico8_fv_start.qg;
    self->g_.q_ = pico8_fv_start.q_;
    self->g_.pn = pico8_fv_start.pn;
    self->g_.du = pico8_fv_start.du;
    self->g_.py = pico8_fv_start.py;
    self->g_.pk = pico8_fv_start.pk;
    self->g_.ki = pico8_fv_start.ki;
    self->hi = &pico8_fv_start.hi;
    self->ca = pico8_fv_start.ca;
    self->gq = &pico8_fv_start.gq;
    self->eo = pico8_fv_start.eo;
    self->ff = pico8_fv_start.ff;
    self->kq = &pico8_fv_start.kq;
    self->ql = pico8_fv_start.ql;
    self->fg = pico8_fv_start.fg;
    self->rb = pico8_fv_start.rb;
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("fv");
#endif
    pico8_fv_pq(self);
    pico8_f_((pico8_struct_header_t *)self);
    pico8_ke_struct_pq(&self->g_.ke);
    return self;
}

/* This module, unlike most, does not call pico8_pi_init() itself, and `fv`
 * extends `g_`, not `ke`: both preconditions are asserted, not repaired. */
void pico8_help_init(void) {
    assert(pico8_ob_lookup_constructor('v') != NULL && "pico8_help_init() runs after pico8_pi_init()");
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
