#include "pico8_particle.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "pico8_dispatch_context.h"
#include "pico8_entity.h"
#include "pico8_entity_geom.h"
#include "pico8_fixed.h"
#include "pico8_vec.h"
#include "pico8_ref_audit.h"
#include "pico8_type_tag.h"

static pico8_fixed_t pico8_particle_field_fixed(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_entity_get_field(inst, name, &v) && v.kind == PICO8_VALUE_FIXED) {
        return v.as.fixed;
    }
    return pico8_fixed_from_int(0);
}

static pico8_vec_t pico8_particle_field_vec(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_entity_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static pico8_list_t *pico8_particle_field_list(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_entity_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return PICO8_REF_AS_LIST(v.as.ref);
    }
    return NULL;
}

static const pico8_ob_table_t *pico8_particle_field_table(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_entity_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        assert(pico8_type_tag_of(v.as.ref) == PICO8_TYPE_TAG_OB_TABLE && "particle field read as a table is not one");
        return (const pico8_ob_table_t *)v.as.ref;
    }
    return NULL;
}

/* Lua truthiness, sans la clause REF-NULL et sans assert : à l'unique site
 * d'appel, la conjonction suivante rejette déjà un `a` nil. L'absence est
 * inatteignable par construction, pas seulement inobservée. */
static int pico8_particle_truthy(pico8_value_t v) {
    return !(v.kind == PICO8_VALUE_BOOL && v.as.boolean == 0);
}

static int pico8_particle_to_int(pico8_fixed_t x) {
    return pico8_fixed_flr_int(x);
}

/* TROU-7 */
void pico8_f_(pico8_instance_t *self) {
    pico8_list_t *qy;

    if (self == NULL || (pico8_type_tag_of(self) == PICO8_TYPE_TAG_INSTANCE && pico8_instance_fields(self) == NULL)) {
        return;
    }

    qy = malloc(sizeof *qy);
    pico8_list_init(qy);
    pico8_entity_set_field(self, "qy", pico8_value_ref(qy));
}

void pico8_ez(pico8_instance_t *self, pico8_fixed_t n) {
    pico8_list_t *qy;

    if (self == NULL) {
        return;
    }

    qy = pico8_particle_field_list(self, "qy");

    while (pico8_fixed_rnd_default() < n) {
        pico8_value_t result;
        if (qy != NULL && pico8_mv(self, "mm", 0, pico8_value_ref(NULL), pico8_value_ref(NULL), &result)) {
            pico8_list_add(qy, result);
        }
        n = pico8_fixed_sub(n, pico8_fixed_from_int(1));
    }
}

void pico8_ei(const pico8_gfx_t *gfx, pico8_mem_t *mem, pico8_instance_t *self) {
    pico8_fixed_t fa;
    pico8_vec_t ra;
    pico8_fixed_t cx;
    pico8_fixed_t cy;
    pico8_fixed_t qo;
    pico8_fixed_t qg;
    pico8_fixed_t ro;
    pico8_fixed_t q_;
    const pico8_ob_table_t *cs;
    pico8_list_t *qy;
    int i;

    if (self == NULL) {
        return;
    }

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
    cs = pico8_particle_field_table(self, "k_");

    qy = pico8_particle_field_list(self, "qy");
    if (qy != NULL) {
        for (i = 0; i < qy->count; i++) {
            pico8_value_t p_value = qy->items[i];
            pico8_ob_table_t *p;
            pico8_value_t v_value;
            pico8_vec_t *v_ptr;
            pico8_vec_t pv;
            pico8_value_t p_pos_value;
            pico8_vec_t p_pos;
            pico8_vec_t *new_pos;
            pico8_fixed_t new_x;
            pico8_fixed_t new_y;
            pico8_value_t a_value;
            pico8_value_t l_value;
            pico8_fixed_t old_l;
            pico8_fixed_t new_l;

            if (p_value.kind != PICO8_VALUE_REF || p_value.as.ref == NULL) {
                continue;
            }
            assert(pico8_type_tag_of(p_value.as.ref) == PICO8_TYPE_TAG_OB_TABLE &&
                   "a qy element is a particle record; see pico8_particle.h for its shape");
            p = (pico8_ob_table_t *)p_value.as.ref;

            if (!pico8_ob_table_get_name(p, "v", &v_value) || v_value.kind != PICO8_VALUE_REF || v_value.as.ref == NULL) {
                continue;
            }
            v_ptr = (pico8_vec_t *)v_value.as.ref;
            pv = *v_ptr;

            if (pico8_ob_table_get_name(p, "p", &p_pos_value) && p_pos_value.kind == PICO8_VALUE_REF &&
                p_pos_value.as.ref != NULL) {
                p_pos = *(const pico8_vec_t *)p_pos_value.as.ref;
            } else {
                p_pos = pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
            }
            /* TROU-7 */
            new_pos = malloc(sizeof *new_pos);
            *new_pos = pico8_vec_add(p_pos, pv);
            pico8_ob_table_set_name(p, "p", pico8_value_ref(new_pos));

            new_x = pico8_fixed_add(pico8_fixed_mul(qo, pv.x), pico8_fixed_mul(ro, pv.y));
            v_ptr->x = new_x;
            /* In Lua `pv` aliases `p.v`, so the second row reads the x
             * just written. Passing the snapshot's `pv.x` here instead
             * would be a silent divergence. */
            new_y = pico8_fixed_add(pico8_fixed_mul(qg, v_ptr->x), pico8_fixed_mul(q_, pv.y));
            v_ptr->y = new_y;

            if (pico8_ob_table_get_name(p, "a", &a_value) && pico8_particle_truthy(a_value) &&
                a_value.kind == PICO8_VALUE_REF && a_value.as.ref != NULL) {
                pico8_vec_t *new_v = malloc(sizeof *new_v);
                *new_v = pico8_vec_add(*v_ptr, *(const pico8_vec_t *)a_value.as.ref);
                pico8_ob_table_set_name(p, "v", pico8_value_ref(new_v));
            }

            old_l = pico8_fixed_from_int(0);
            if (pico8_ob_table_get_name(p, "l", &l_value) && l_value.kind == PICO8_VALUE_FIXED) {
                old_l = l_value.as.fixed;
            }
            new_l = pico8_fixed_sub(old_l, PICO8_FIXED_LIT(0x000023d7, 0.14));
            pico8_ob_table_set_name(p, "l", pico8_value_fixed(new_l));

            if (new_l <= pico8_fixed_from_int(1)) {
                /* The Lua loop is `pairs()`, not `all()`: the element
                 * that slides into the vacated slot is skipped this
                 * frame. Reproduced by never compensating the index. */
                pico8_list_del(qy, p_value);
            } else {
                int px = pico8_particle_to_int(pico8_fixed_add(cx, new_pos->x));
                int py = pico8_particle_to_int(pico8_fixed_add(cy, new_pos->y));
                int col = 0;
                pico8_value_t col_value;
                int idx = pico8_particle_to_int(pico8_fixed_flr(new_l));

                if (cs != NULL && pico8_ob_table_get_index(cs, idx, &col_value) && col_value.kind == PICO8_VALUE_FIXED) {
                    col = pico8_particle_to_int(col_value.as.fixed);
                }

                pico8_pset(gfx, mem, px, py, col);
            }
        }
    }

    pico8_ez(self, pico8_particle_field_fixed(self, "fc"));
}

pico8_value_t pico8_ei_dispatch(void *self, int argc, pico8_value_t arg0, pico8_value_t arg1) {
    (void)argc;
    (void)arg0;
    (void)arg1;
    pico8_ei(pico8_dispatch_gfx(), pico8_dispatch_mem(), (pico8_instance_t *)self);
    return pico8_value_ref(NULL);
}
