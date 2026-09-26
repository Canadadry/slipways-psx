#include "pico8_entity_geom.h"

#include <assert.h>

#include "pico8_type_tag.h"

#include <stddef.h>

#include "pico8_entity.h"
#include "pico8_geom.h"

static pico8_vec_t pico8_entity_geom_zero(void) {
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

static pico8_vec_t pico8_entity_geom_field_vec(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_entity_get_field(inst, name, &v) && v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_entity_geom_zero();
}

static pico8_vec_t pico8_entity_geom_value_vec(pico8_value_t v) {
    if (v.kind == PICO8_VALUE_REF && v.as.ref != NULL) {
        return *(const pico8_vec_t *)v.as.ref;
    }
    return pico8_entity_geom_zero();
}

static pico8_fixed_t pico8_entity_geom_field_fixed(const pico8_instance_t *inst, const char *name) {
    pico8_value_t v;
    if (inst != NULL && pico8_entity_get_field(inst, name, &v) && v.kind == PICO8_VALUE_FIXED) {
        return v.as.fixed;
    }
    return pico8_fixed_from_int(0);
}

pico8_value_t pico8_gk(const pico8_instance_t *e, pico8_value_t mp, pico8_value_t rp) {
    pico8_fixed_t fa = pico8_entity_geom_field_fixed(e, "fa");
    pico8_vec_t ra = pico8_entity_geom_field_vec(e, "ra");
    pico8_value_t chosen = (fa >= pico8_fixed_from_int(10)) ? rp : mp;
    pico8_vec_t point = pico8_vec_sub(pico8_entity_geom_value_vec(chosen), ra);
    pico8_value_t kq;

    if (e != NULL && pico8_entity_get_field(e, "kq", &kq) && kq.kind == PICO8_VALUE_REF && kq.as.ref != NULL) {
        return pico8_value_bool(pico8_qz_call((const pico8_qz_box_t *)kq.as.ref, point));
    }
    return pico8_value_bool(0);
}

int pico8_gd(const pico8_instance_t *e) {
    pico8_vec_t e_ra = pico8_entity_geom_field_vec(e, "ra");
    pico8_vec_t mo_ra = pico8_entity_geom_field_vec(pico8_mo(), "ra");
    pico8_fixed_t sixty_four = pico8_fixed_from_int(64);
    pico8_fixed_t seventy_six = pico8_fixed_from_int(76);

    return pico8_fixed_abs(pico8_fixed_sub(pico8_fixed_sub(e_ra.x, mo_ra.x), sixty_four)) <= seventy_six &&
           pico8_fixed_abs(pico8_fixed_sub(pico8_fixed_sub(e_ra.y, mo_ra.y), sixty_four)) <= seventy_six;
}

void pico8_na(pico8_instance_t *self, pico8_fixed_t t) {
    pico8_value_t ra;
    if (self != NULL && pico8_entity_get_field(self, "ra", &ra) && ra.kind == PICO8_VALUE_REF && ra.as.ref != NULL) {
        pico8_vec_t *ra_vec = (pico8_vec_t *)ra.as.ref;
        pico8_fixed_t delta = pico8_fixed_mul(pico8_fixed_sin(pico8_fixed_mul(t, PICO8_FIXED_LIT(0x00000147, 0.005))),
                                               PICO8_FIXED_LIT(0x00000f5c, 0.06));
        ra_vec->y = pico8_fixed_add(ra_vec->y, delta);
    }
}

pico8_vec_t pico8_ma(pico8_vec_t p, pico8_instance_t *e) {
    pico8_fixed_t ma = pico8_entity_geom_field_fixed(e, "ma");
    pico8_fixed_t decremented = pico8_fixed_sub(ma, PICO8_FIXED_LIT(0x00001999, 0.1));
    pico8_fixed_t clamped = (decremented > 0) ? decremented : 0;
    pico8_fixed_t t = pico8_entity_geom_field_fixed(e, "t");

    if (e != NULL) {
        pico8_entity_set_field(e, "ma", pico8_value_fixed(clamped));
    }
    return pico8_vec_add(p, pico8_vec_qm(clamped, pico8_fixed_mul(t, PICO8_FIXED_LIT(0x00003333, 0.2))));
}

pico8_fixed_t pico8_ij(pico8_vec_t p, const pico8_list_t *j_, pico8_value_t *out_nearest) {
    pico8_fixed_t mk = pico8_fixed_from_int(32767);
    pico8_list_iter_t it;
    pico8_value_t e;

    *out_nearest = pico8_value_ref(NULL);
    if (j_ == NULL) {
        return mk;
    }
    it = pico8_list_all(j_);
    while (pico8_list_iter_next(&it, &e)) {
        pico8_instance_t *o;

        assert(e.kind == PICO8_VALUE_REF &&
               (e.as.ref == NULL || pico8_type_tag_of(e.as.ref) == PICO8_TYPE_TAG_INSTANCE ||
                pico8_type_tag_of(e.as.ref) == PICO8_TYPE_TAG_STRUCT) &&
               "ij walks a list of entities");
        o = (pico8_instance_t *)e.as.ref;
        pico8_vec_t o_p = pico8_entity_geom_field_vec(o, "p");
        pico8_fixed_t d = pico8_vec_len_sq(pico8_vec_sub(p, o_p));
        int same_position = (p.x == o_p.x && p.y == o_p.y);

        if (!same_position && d < mk) {
            mk = d;
            *out_nearest = e;
        }
    }
    return mk;
}

