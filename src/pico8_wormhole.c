#include "pico8_wormhole.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pico8_audio.h"
#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_ke.h"
#include "pico8_planet.h"
#include "pico8_probe.h"
#include "pico8_tech_gate.h"
#include "pico8_wormhole_display.h"

static int g_mz_up = 0;

static const pico8_struct_class_t g_mz_jj_struct_class = {0};

/* The only one: never allocated, never freed; no writer, so a write
 * reaching it stops the run. */
static pico8_mz_jj_t g_mz_jj_wrapper = {
    {.cls = &g_mz_jj_struct_class},
    pico8_mz_jj_qs,
};

void pico8_mz_pq(pico8_mz_t *self) {
    pico8_ents_t *heap_ox;
    pico8_ents_iter_t it;
    pico8_struct_header_t *p;

    if (self == NULL) {
        return;
    }

    pico8_rn(pico8_fixed_from_int(53));

    /* TROU-7 */
    heap_ox = malloc(sizeof *heap_ox);
    pico8_ents_init(heap_ox);
    it = pico8_ents_all(pico8_hv());
    while (pico8_ents_iter_next(&it, &p)) {
        if (p != NULL && p->cls->get_fx != NULL && p->cls->get_fx(p)) {
            pico8_ents_add(heap_ox, p);
        }
    }
    self->ox = heap_ox;
}

void pico8_mz_hq(pico8_mz_t *self, pico8_vec_t *mp, pico8_vec_t *rp) {
    pico8_vec_t p_vec;
    pico8_struct_header_t *nearest = NULL;
    pico8_fixed_t nq;
    pico8_fixed_t threshold;

    pico8_bv((pico8_struct_header_t *)self, mp, rp);
    if (self == NULL) {
        return;
    }

    p_vec = self->p != NULL ? *self->p : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    nq = pico8_ij_ents(p_vec, self->ox, &nearest);
    self->ij = nearest;

    threshold = pico8_ov("space folding", PICO8_FIXED_LIT(0x000030a3, 0.19), PICO8_FIXED_LIT(0x0000451e, 0.27));

    if (nq > threshold) {
        self->probe->et = (pico8_struct_header_t *)pico8_mz_jj_wrapper();
    }
}

static void pico8_mz_hq_dispatch(void *self, pico8_vec_t *qa, pico8_vec_t *ra) {
    pico8_mz_hq((pico8_mz_t *)self, qa, ra);
}

void pico8_mz_kz(pico8_mz_t *self, pico8_fixed_t t) {
    pico8_fixed_t threshold;
    pico8_fixed_t one;
    pico8_fixed_t td15;
    pico8_fixed_t min_val;
    pico8_fixed_t sin_term;
    pico8_fixed_t result;

    if (self == NULL) {
        return;
    }

    threshold = pico8_ov("space folding", pico8_fixed_from_int(29), pico8_fixed_from_int(35));

    one = pico8_fixed_from_int(1);
    td15 = pico8_fixed_div(t, pico8_fixed_from_int(15));
    min_val = (td15 < one) ? td15 : one;
    sin_term =
        pico8_fixed_mul(pico8_fixed_sin(pico8_fixed_div(t, pico8_fixed_from_int(40))), PICO8_FIXED_LIT(0x00001999, 0.1));

    result = pico8_fixed_mul(threshold, pico8_fixed_add(min_val, sin_term));
    self->oi = result;
}

static void pico8_mz_kz_dispatch(void *self, pico8_fixed_t t) {
    pico8_mz_kz((pico8_mz_t *)self, t);
}

void pico8_mz_ey(pico8_mz_t *self) {
    pico8_fixed_t t;
    int result;

    if (self == NULL) {
        return;
    }

    t = self->header.t;

    /* The `or` short-circuits: `d_`'s side effects must not run before tick
     * 15, so this call cannot be hoisted out of the else branch. */
    if (t < pico8_fixed_from_int(15)) {
        result = 1;
    } else {
        result = !pico8_d_((pico8_struct_header_t *)self);
    }

    self->p_ = result;
    self->has_p_ = 1;
}

static void pico8_mz_ey_dispatch(void *self, pico8_struct_header_t *kv) {
    (void)kv;
    pico8_mz_ey((pico8_mz_t *)self);
}

void pico8_mz_pr(pico8_mz_t *self, pico8_fixed_t t) {
    pico8_fixed_t sr;
    pico8_vec_t self_p;
    const pico8_ents_t *jo_list;
    int i;

    if (self == NULL) {
        return;
    }

    sr = pico8_fixed_add(pico8_fixed_div(pico8_mz_ej(self, t), pico8_fixed_from_int(128)),
                          PICO8_FIXED_LIT(0x000007ef, 0.031));

    self_p = self->p != NULL ? *self->p : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    jo_list = pico8_cg("jo");
    if (jo_list != NULL) {
        for (i = 0; i < jo_list->count; i++) {
            pico8_struct_header_t *pt_val = jo_list->items[i];
            pico8_jo_t *pt;

            if (pt_val == NULL) {
                continue;
            }
            assert(pico8_jo_is_family(pt_val) && "mz:pr: cg.jo holds planets only");
            pt = (pico8_jo_t *)pt_val;

            if (pt->header.me != NULL && strcmp(pt->header.me, "unknown") == 0) {
                pico8_vec_t pt_p =
                    pt->p != NULL ? *pt->p : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
                pico8_vec_t diff = pico8_vec_sub(pt_p, self_p);
                if (pico8_vec_qx(diff) < sr) {
                    pico8_jo_hh(pt);
                }
            }
        }
    }

    self->p_ = t > pico8_fixed_from_int(25);
    self->has_p_ = 1;
}

static void pico8_mz_pr_dispatch(void *self, pico8_fixed_t t) {
    pico8_mz_pr((pico8_mz_t *)self, t);
}

pico8_fixed_t pico8_mz_ej(pico8_mz_t *self, pico8_fixed_t t) {
    pico8_fixed_t sq = pico8_fixed_sqrt(pico8_fixed_div(t, pico8_fixed_from_int(15)));
    pico8_fixed_t one = pico8_fixed_from_int(1);
    pico8_fixed_t min_val = (sq < one) ? sq : one;
    pico8_fixed_t oi = pico8_fixed_from_int(0);

    if (self != NULL) {
        oi = self->oi;
    }
    return pico8_fixed_mul(min_val, oi);
}

static void pico8_mz_kj_dispatch(void *self, const pico8_vec_t *ra) {
    pico8_vec_t p = ra != NULL ? *ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    pico8_mz_kj(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_mz_t *)self, p);
}

/* Lua's `mz.hq=bv` is rebound three statements later by `function mz:hq`,
 * so only that second definition is ever reachable. Putting
 * pico8_bv_dispatch in the HQ entry instead would look faithful and would
 * drop the nearest-anchor search. */
static pico8_probe_t *mz_probe_of(void *self) {
    return ((pico8_mz_t *)self)->probe;
}

static const pico8_vec_t *mz_get_ra(const void *self) {
    return ((const pico8_mz_t *)self)->ra;
}

static const pico8_vec_t *mz_get_p(const void *self) {
    return ((const pico8_mz_t *)self)->p;
}

static int mz_get_p_(const void *self) {
    const pico8_mz_t *mz = self;

    return mz->has_p_ && mz->p_;
}

static const pico8_struct_class_t g_mz_struct_class = {
    .kj = pico8_mz_kj_dispatch,
    .ev = pico8_dl_dispatch,
    .hq = pico8_mz_hq_dispatch,
    .ey = pico8_mz_ey_dispatch,
    .states =
        {
            [PICO8_STATE_KZ] = pico8_mz_kz_dispatch,
            [PICO8_STATE_PR] = pico8_mz_pr_dispatch,
        },
    .probe_of = mz_probe_of,
    .tags = PICO8_TAG(PICO8_TAG_MZ),
    .get_ra = mz_get_ra,
    .get_p = mz_get_p,
    .get_p_ = mz_get_p_,
};

static pico8_mz_t *pico8_mz_struct_new(void) {
    pico8_mz_t *self = malloc(sizeof *self);

    pico8_ke_struct_init(&self->header, &g_mz_struct_class, pico8_mz_def.me, pico8_mz_def.t);
    self->def = &pico8_mz_def;
    self->header.fa = &self->def->fa;
    self->probe = &self->probe_own;
    self->probe->def = &self->def->probe;
    self->oi = self->def->oi;
    self->probe->od = self->def->probe.od;
    self->probe->has_od = self->def->probe.has_od;
    self->jj = &g_mz_jj_wrapper;
    self->ox = NULL;
    self->ra = NULL;
    self->p = NULL;
    self->probe->et = NULL;
    self->ij = NULL;
    self->p_ = 0;
    self->has_p_ = 0;
#ifdef PICO8_CLASS_COVERAGE
    pico8_class_coverage_record("mz");
#endif
    pico8_mz_pq(self);
    pico8_ke_struct_pq(&self->header);
    return self;
}

void pico8_wormhole_init(void) {
    pico8_wormhole_shutdown();
    g_mz_up = 1;
}

void pico8_wormhole_shutdown(void) {
    g_mz_up = 0;
}

int pico8_mz_ready(void) {
    return g_mz_up;
}

const pico8_struct_class_t *pico8_mz_struct_class(void) {
    return &g_mz_struct_class;
}

pico8_mz_t *pico8_mz_new(void) {
    return pico8_mz_struct_new();
}

pico8_mz_jj_t *pico8_mz_jj_wrapper(void) {
    return &g_mz_jj_wrapper;
}

const pico8_struct_class_t *pico8_mz_jj_struct_class(void) {
    return &g_mz_jj_struct_class;
}
