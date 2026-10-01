#include "pico8_particle.h"

#include <assert.h>
#include <stddef.h>

#include "pico8_dispatch_context.h"
#include "pico8_entity_geom.h"
#include "pico8_fixed.h"
#include "pico8_game_data.h"
#include "pico8_vec.h"
#include "pico8_zone_particules.h"
#include "pico8_zone_transitoire.h"

typedef char pico8_qy_fits_its_place[sizeof(pico8_qy_t) <= PICO8_PARTICULE_PLACE ? 1 : -1];

/* `fa`, and `ra` into `*ra` (the zero vector when nil), of the host of `ei`. */
static pico8_fixed_t emitter_host_fa_ra(const pico8_struct_header_t *self, pico8_vec_t *ra) {
    assert(self->fa != NULL && self->ra != NULL &&
           "ei: a host that carries no fa or no ra, none of whose kj runs ei -- stop");
    *ra = *self->ra != NULL ? **self->ra : pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
    return *self->fa;
}

static int pico8_particle_to_int(pico8_fixed_t x) {
    return pico8_fixed_flr_int(x);
}

static void adosser(void) {
    pico8_zone_particules_adosser(&pico8_zone_transitoire);
}

pico8_qy_t *pico8_particule_prendre(void) {
    adosser();
    return (pico8_qy_t *)pico8_zone_alloc(pico8_zone_particules(PICO8_PARTICULES_PARTICULE), sizeof(pico8_qy_t));
}

const pico8_zone_choisie_t *pico8_ga_place_prendre(void) {
    adosser();
    return pico8_zone_particules_place(PICO8_PARTICULES_GA);
}

void pico8_f_(pico8_struct_header_t *self) {
    pico8_f__dans(self, PICO8_PARTICULES_QY);
}

void pico8_f__dans(pico8_struct_header_t *self, pico8_particules_sorte_t sorte) {
    const pico8_zone_choisie_t *place;
    pico8_emitter_t *emitter;
    pico8_ptrs_t *qy;

    if (self == NULL) {
        return;
    }
    assert(self->emitter != NULL && "f_: a class that carries no particle emitter has no `qy` -- stop");
    assert((sorte == PICO8_PARTICULES_QY || sorte == PICO8_PARTICULES_QY_FV) && "f_: a qy list is a qy or a qy_fv");

    emitter = self->emitter;
    pico8_qy_rendre(emitter);
    adosser();
    place = pico8_zone_particules_place(sorte);
    qy = (pico8_ptrs_t *)pico8_zone_alloc(place, sizeof *qy);
    pico8_ptrs_init_reserve(qy, place,
                            sorte == PICO8_PARTICULES_QY_FV ? PICO8_QY_FV_CAPACITE : PICO8_QY_CAPACITE);
    emitter->qy = qy;
    emitter->qy_sorte = sorte;
}

void pico8_qy_rendre(pico8_emitter_t *emitter) {
    pico8_ptrs_t *qy = emitter->qy;
    const pico8_zone_choisie_t *particules = pico8_zone_particules(PICO8_PARTICULES_PARTICULE);
    int i;

    if (qy == NULL) {
        return;
    }
    for (i = 0; i < qy->count; i++) {
        pico8_zone_release(particules, qy->items[i]);
    }
    /* The list lives in its own arena: `qy` dies with it. */
    pico8_zone_arene_rendre(qy->zone);
    emitter->qy = NULL;
}

void pico8_ez(pico8_struct_header_t *self, pico8_fixed_t n) {
    pico8_emitter_t *emitter;
    pico8_ptrs_t *qy;

    if (self == NULL) {
        return;
    }
    emitter = self->emitter;
    qy = emitter != NULL ? emitter->qy : NULL;

    while (pico8_fixed_rnd_default() < n) {
        if (qy != NULL && self->cls->mm != NULL) {
            pico8_ptrs_add(qy, self->cls->mm(self));
            pico8_zone_particules_noter_elements(emitter->qy_sorte, qy->count);
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
    assert(self->emitter != NULL && "ei: a class that carries no particle emitter is no receiver -- stop");

    emitter = self->emitter;
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
                pico8_zone_release(pico8_zone_particules(PICO8_PARTICULES_PARTICULE), (unsigned char *)p);
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

    pico8_ez(self, emitter->def->fc);
}

void pico8_ei_dispatch(pico8_struct_header_t *self, const pico8_vec_t *ra) {
    (void)ra;
    pico8_ei(pico8_dispatch_gfx(), pico8_dispatch_mem(), self);
}
