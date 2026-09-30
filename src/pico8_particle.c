#include "pico8_particle.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_fixed.h"
#include "pico8_game_data.h"
#include "pico8_planet.h"
#include "pico8_probe.h"
#include "pico8_vec.h"

/* `fa`, and `ra` into `*ra` (the zero vector when nil), which the host of
 * `ei` holds for itself: `g_`'s line, or jo's family (`hj`, `pm`, `mx`, `ga`). */
static pico8_fixed_t emitter_host_fa_ra(pico8_struct_header_t *self, pico8_vec_t *ra) {
    const pico8_g__t *g_ = pico8_g__of(self);
    const pico8_jo_t *jo;

    if (g_ != NULL) {
        *ra = g_->ra != NULL ? *g_->ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
        return g_->def->fa;
    }
    assert(pico8_jo_is_family(self) &&
           "ei: a host neither of g_'s line nor of jo's family, none of whose kj runs ei -- stop");
    jo = (const pico8_jo_t *)self;
    *ra = jo->ra != NULL ? *jo->ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    return jo->def->fa;
}

static int pico8_particle_to_int(pico8_fixed_t x) {
    return pico8_fixed_flr_int(x);
}

/* TROU-7 */
void pico8_f_(pico8_struct_header_t *self) {
    pico8_emitter_t *emitter;
    pico8_ptrs_t *qy;

    if (self == NULL) {
        return;
    }
    assert(self->cls->emitter_of != NULL && "f_: a class that carries no particle emitter has no `qy` -- stop");

    emitter = self->cls->emitter_of(self);
    qy = malloc(sizeof *qy);
    pico8_ptrs_init(qy);
    emitter->qy = qy;
}

void pico8_ez(pico8_struct_header_t *self, pico8_fixed_t n) {
    pico8_emitter_t *emitter;
    pico8_ptrs_t *qy;

    if (self == NULL) {
        return;
    }
    emitter = self->cls->emitter_of != NULL ? self->cls->emitter_of(self) : NULL;
    qy = emitter != NULL ? emitter->qy : NULL;

    while (pico8_fixed_rnd_default() < n) {
        if (qy != NULL && self->cls->mm != NULL) {
            pico8_ptrs_add(qy, self->cls->mm(self));
        }
        n = pico8_fixed_sub(n, pico8_fixed_from_int(1));
    }
}

void pico8_ei(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_struct_header_t *self) {
    const pico8_emitter_t *emitter;
    pico8_fixed_t fa;
    pico8_vec_t ra;
    pico8_fixed_t cx;
    pico8_fixed_t cy;
    pico8_fixed_t qo;
    pico8_fixed_t qg;
    pico8_fixed_t ro;
    pico8_fixed_t q_;
    const pico8_int_seq_t *cs;
    pico8_ptrs_t *qy;
    int i;

    if (self == NULL) {
        return;
    }
    assert(self->cls->emitter_of != NULL && "ei: a class that carries no particle emitter is no receiver -- stop");

    emitter = self->cls->emitter_of(self);
    fa = emitter_host_fa_ra(self, &ra);
    if (fa < pico8_fixed_from_int(10) && !pico8_gd(self)) {
        return;
    }

    cx = ra.x;
    cy = ra.y;
    qo = emitter->qo;
    qg = emitter->qg;
    ro = emitter->ro;
    q_ = emitter->q_;
    cs = emitter->k_;

    qy = emitter->qy;
    if (qy != NULL) {
        for (i = 0; i < qy->count; i++) {
            pico8_qy_t *p = qy->items[i];
            pico8_vec_t pv;

            if (p == NULL) {
                continue;
            }
            pv = p->v;

            p->p = pico8_vec_add(p->p, pv);

            p->v.x = pico8_fixed_add(pico8_fixed_mul(qo, pv.x), pico8_fixed_mul(ro, pv.y));
            /* In Lua `pv` aliases `p.v`, so the second row reads the x
             * just written. Passing the snapshot's `pv.x` here instead
             * would be a silent divergence. */
            p->v.y = pico8_fixed_add(pico8_fixed_mul(qg, p->v.x), pico8_fixed_mul(q_, pv.y));

            if (p->has_a) {
                p->v = pico8_vec_add(p->v, p->a);
            }

            p->l = pico8_fixed_sub(p->l, PICO8_FIXED_LIT(0x000023d7, 0.14));

            if (p->l <= pico8_fixed_from_int(1)) {
                /* The Lua loop is `pairs()`, not `all()`: the element
                 * that slides into the vacated slot is skipped this
                 * frame. Reproduced by never compensating the index. */
                pico8_ptrs_del(qy, p);
            } else {
                int px = pico8_particle_to_int(pico8_fixed_add(cx, p->p.x));
                int py = pico8_particle_to_int(pico8_fixed_add(cy, p->p.y));
                int col = 0;
                int idx = pico8_particle_to_int(pico8_fixed_flr(p->l));

                if (cs != NULL) {
                    pico8_int_seq_at(cs->items, cs->count, idx, &col);
                }

                pico8_pset(gfx, mem, px, py, col);
            }
        }
    }

    pico8_ez(self, emitter->fc);
}

void pico8_ei_dispatch(void *self, const pico8_vec_t *ra) {
    (void)ra;
    pico8_ei(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_struct_header_t *)self);
}
