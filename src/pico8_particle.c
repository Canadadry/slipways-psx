#include "pico8_particle.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_fixed.h"
#include "pico8_game_data.h"
#include "pico8_vec.h"
#include "pico8_ref_audit.h"
#include "pico8_type_tag.h"

static pico8_fixed_t pico8_particle_field_fixed(const pico8_struct_header_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_entity_get_field(inst, name, &v) && v.kind == PICO8_VALUE_FIXED) {
        return v.as.fixed;
    }
    return pico8_fixed_from_int(0);
}

static pico8_vec_t pico8_particle_field_vec(const pico8_struct_header_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_entity_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static pico8_list_t *pico8_particle_field_list(const pico8_struct_header_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_entity_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return PICO8_REF_AS_LIST(v.as.ref);
    }
    return NULL;
}

/* NULL when absent. */
static const pico8_int_seq_t *pico8_particle_field_k_(const pico8_struct_header_t *inst) {
    pico8_value_t v;
    if (inst != NULL && pico8_entity_get_field(inst, "k_", &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return (const pico8_int_seq_t *)v.as.ref;
    }
    return NULL;
}

static int pico8_particle_to_int(pico8_fixed_t x) {
    return pico8_fixed_flr_int(x);
}

/* TROU-7 */
void pico8_f_(pico8_struct_header_t *self) {
    pico8_list_t *qy;

    if (self == NULL) {
        return;
    }
    assert(pico8_type_tag_of(self) == PICO8_TYPE_TAG_STRUCT &&
           "f_: self is a struct; no class instance reaches g_'s functions any more");

    qy = malloc(sizeof *qy);
    pico8_list_init(qy);
    pico8_entity_set_field(self, "qy", pico8_value_ref(qy));
}

void pico8_ez(pico8_struct_header_t *self, pico8_fixed_t n) {
    pico8_list_t *qy;

    if (self == NULL) {
        return;
    }
    assert(pico8_type_tag_of(self) == PICO8_TYPE_TAG_STRUCT &&
           "ez: self is a struct; no class instance reaches g_'s functions any more");

    qy = pico8_particle_field_list(self, "qy");

    while (pico8_fixed_rnd_default() < n) {
        pico8_value_t result;
        if (qy != NULL && pico8_mv(self, "mm", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &result)) {
            pico8_list_add(qy, result);
        }
        n = pico8_fixed_sub(n, pico8_fixed_from_int(1));
    }
}

void pico8_ei(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_struct_header_t *self) {
    pico8_fixed_t fa;
    pico8_vec_t ra;
    pico8_fixed_t cx;
    pico8_fixed_t cy;
    pico8_fixed_t qo;
    pico8_fixed_t qg;
    pico8_fixed_t ro;
    pico8_fixed_t q_;
    const pico8_int_seq_t *cs;
    pico8_list_t *qy;
    int i;

    if (self == NULL) {
        return;
    }
    assert(pico8_type_tag_of(self) == PICO8_TYPE_TAG_STRUCT &&
           "ei: self is a struct; no class instance reaches g_'s functions any more");

    fa = pico8_particle_field_fixed(self, "fa");
    if (fa < pico8_fixed_from_int(10) && !pico8_gd(self)) {
        return;
    }

    ra = pico8_particle_field_vec(self, "ra");
    cx = ra.x;
    cy = ra.y;
    qo = pico8_particle_field_fixed(self, "qo");
    qg = pico8_particle_field_fixed(self, "qg");
    ro = pico8_particle_field_fixed(self, "ro");
    q_ = pico8_particle_field_fixed(self, "q_");
    cs = pico8_particle_field_k_(self);

    qy = pico8_particle_field_list(self, "qy");
    if (qy != NULL) {
        for (i = 0; i < qy->count; i++) {
            pico8_value_t p_value = qy->items[i];
            pico8_qy_t *p;
            pico8_vec_t pv;

            if (p_value.kind != PICO8_VALUE_REF || p_value.as.ref == NULL) {
                continue;
            }
            p = (pico8_qy_t *)p_value.as.ref;
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
                pico8_list_del(qy, p_value);
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

    pico8_ez(self, pico8_particle_field_fixed(self, "fc"));
}

void pico8_ei_dispatch(void *self, const pico8_vec_t *ra) {
    (void)ra;
    pico8_ei(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_struct_header_t *)self);
}
