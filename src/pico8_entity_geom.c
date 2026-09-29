#include "pico8_entity_geom.h"

#include <assert.h>

#include "pico8_type_tag.h"

#include <stddef.h>

#include "pico8_entity.h"
#include "pico8_geom.h"
#include "pico8_planet.h"

static pico8_vec_t pico8_entity_geom_zero(void) {
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

/* `e`'s `ra` (`p` for the second), or (0,0) when `e` is NULL or the field is nil. */
static pico8_vec_t pico8_entity_geom_ra(const pico8_struct_header_t *e) {
    const pico8_vec_t *ra = e != NULL && e->cls->get_ra != NULL ? e->cls->get_ra(e) : NULL;

    return ra != NULL ? *ra : pico8_entity_geom_zero();
}

static pico8_vec_t pico8_entity_geom_p(const pico8_struct_header_t *e) {
    const pico8_vec_t *p = e != NULL && e->cls->get_p != NULL ? e->cls->get_p(e) : NULL;

    return p != NULL ? *p : pico8_entity_geom_zero();
}

int pico8_gk(const pico8_struct_header_t *e, const pico8_vec_t *mp, const pico8_vec_t *rp) {
    assert((e == NULL || pico8_type_tag_of(e) == PICO8_TYPE_TAG_STRUCT) &&
           "gk: e is a struct; no class instance reaches g_'s functions any more");
    pico8_fixed_t fa = pico8_fixed_from_int(0);
    pico8_vec_t ra = pico8_entity_geom_ra(e);
    const pico8_vec_t *chosen;
    pico8_vec_t point;
    const pico8_qz_box_t *kq;

    if (e != NULL && e->cls->get_fa != NULL) {
        e->cls->get_fa(e, &fa);
    }
    chosen = (fa >= pico8_fixed_from_int(10)) ? rp : mp;
    point = pico8_vec_sub(chosen != NULL ? *chosen : pico8_entity_geom_zero(), ra);
    kq = e != NULL && e->cls->get_kq != NULL ? e->cls->get_kq(e) : NULL;
    if (kq != NULL) {
        return pico8_qz_call(kq, point);
    }
    return 0;
}

int pico8_gd(const pico8_struct_header_t *e) {
    pico8_vec_t e_ra = pico8_entity_geom_ra(e);
    const pico8_qb_t *mo = pico8_mo_qb();
    pico8_vec_t mo_ra = mo != NULL && mo->ra != NULL ? *mo->ra : pico8_entity_geom_zero();
    pico8_fixed_t sixty_four = pico8_fixed_from_int(64);
    pico8_fixed_t seventy_six = pico8_fixed_from_int(76);

    return pico8_fixed_abs(pico8_fixed_sub(pico8_fixed_sub(e_ra.x, mo_ra.x), sixty_four)) <= seventy_six &&
           pico8_fixed_abs(pico8_fixed_sub(pico8_fixed_sub(e_ra.y, mo_ra.y), sixty_four)) <= seventy_six;
}

void pico8_na(pico8_struct_header_t *self, pico8_fixed_t t) {
    pico8_vec_t *ra_vec;

    assert((self == NULL || pico8_type_tag_of(self) == PICO8_TYPE_TAG_STRUCT) &&
           "na: self is a struct; no class instance reaches g_'s functions any more");
    assert((self == NULL || pico8_jo_is_family(self)) && "na: self is a planet (qh:ly, nl:ly) -- stop");
    ra_vec = self != NULL ? ((pico8_jo_t *)self)->ra : NULL;
    if (ra_vec != NULL) {
        pico8_fixed_t delta = pico8_fixed_mul(pico8_fixed_sin(pico8_fixed_mul(t, PICO8_FIXED_LIT(0x00000147, 0.005))),
                                               PICO8_FIXED_LIT(0x00000f5c, 0.06));
        ra_vec->y = pico8_fixed_add(ra_vec->y, delta);
    }
}

pico8_vec_t pico8_ma(pico8_vec_t p, pico8_struct_header_t *e) {
    pico8_fixed_t *ma = NULL;
    pico8_fixed_t decremented;
    pico8_fixed_t clamped;
    pico8_fixed_t t = pico8_fixed_from_int(0);

    if (e != NULL) {
        assert((pico8_jo_is_family(e) || e->cls == pico8_qb_struct_class()) &&
               "ma: e is a planet (jo:kj) or the camera (qb:ph)");
        ma = pico8_jo_is_family(e) ? &((pico8_jo_t *)e)->ma : &((pico8_qb_t *)e)->ma;
    }
    decremented = pico8_fixed_sub(ma != NULL ? *ma : pico8_fixed_from_int(0), PICO8_FIXED_LIT(0x00001999, 0.1));
    clamped = (decremented > 0) ? decremented : 0;
    if (e != NULL) {
        t = ((const pico8_ke_t *)e)->t;
        *ma = clamped;
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
        pico8_struct_header_t *o;

        assert(e.kind == PICO8_VALUE_REF &&
               (e.as.ref == NULL || pico8_type_tag_of(e.as.ref) == PICO8_TYPE_TAG_STRUCT) &&
               "ij walks a list of entities");
        o = (pico8_struct_header_t *)e.as.ref;
        pico8_vec_t o_p = pico8_entity_geom_p(o);
        pico8_fixed_t d = pico8_vec_len_sq(pico8_vec_sub(p, o_p));
        int same_position = (p.x == o_p.x && p.y == o_p.y);

        if (!same_position && d < mk) {
            mk = d;
            *out_nearest = e;
        }
    }
    return mk;
}

