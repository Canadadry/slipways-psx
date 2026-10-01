#include "engine/entity_geom.h"

#include <assert.h>

#include <stddef.h>

#include "engine/entity_registry.h"
#include "engine/geom.h"

static pico8_vec_t pico8_entity_geom_zero(void) {
    return pico8_vec_v(pico8_fixed_from_int(0), pico8_fixed_from_int(0));
}

/* `e`'s `ra` (`p` for the second), or (0,0) when `e` is NULL or the field is nil. */
static pico8_vec_t pico8_entity_geom_ra(const pico8_struct_header_t *e) {
    const pico8_vec_t *ra = e != NULL && e->ra != NULL ? *e->ra : NULL;

    return ra != NULL ? *ra : pico8_entity_geom_zero();
}

static pico8_vec_t pico8_entity_geom_p(const pico8_struct_header_t *e) {
    const pico8_vec_t *p = e != NULL && e->p != NULL ? *e->p : NULL;

    return p != NULL ? *p : pico8_entity_geom_zero();
}

int pico8_gk(const pico8_struct_header_t *e, const pico8_vec_t *mp, const pico8_vec_t *rp) {
    pico8_fixed_t fa = e != NULL && e->fa != NULL ? *e->fa : pico8_fixed_from_int(0);
    pico8_vec_t ra = pico8_entity_geom_ra(e);
    const pico8_vec_t *chosen;
    pico8_vec_t point;
    const pico8_qz_box_t *kq;

    chosen = (fa >= pico8_fixed_from_int(10)) ? rp : mp;
    point = pico8_vec_sub(chosen != NULL ? *chosen : pico8_entity_geom_zero(), ra);
    kq = e != NULL ? e->kq : NULL;
    if (kq != NULL) {
        return pico8_qz_call(kq, point);
    }
    return 0;
}

int pico8_gd(const pico8_struct_header_t *e) {
    pico8_vec_t e_ra = pico8_entity_geom_ra(e);
    pico8_vec_t mo_ra = pico8_entity_geom_ra(pico8_mo());
    pico8_fixed_t sixty_four = pico8_fixed_from_int(64);
    pico8_fixed_t seventy_six = pico8_fixed_from_int(76);

    return pico8_fixed_abs(pico8_fixed_sub(pico8_fixed_sub(e_ra.x, mo_ra.x), sixty_four)) <= seventy_six &&
           pico8_fixed_abs(pico8_fixed_sub(pico8_fixed_sub(e_ra.y, mo_ra.y), sixty_four)) <= seventy_six;
}

pico8_vec_t pico8_ma(pico8_vec_t p, pico8_struct_header_t *e) {
    pico8_fixed_t *ma = NULL;
    pico8_fixed_t decremented;
    pico8_fixed_t clamped;
    pico8_fixed_t t = pico8_fixed_from_int(0);

    if (e != NULL) {
        assert(e->ma != NULL && "ma: e is a planet (jo:kj) or the camera (qb:ph)");
        ma = e->ma;
    }
    decremented = pico8_fixed_sub(ma != NULL ? *ma : pico8_fixed_from_int(0), PICO8_FIXED_LIT(0x00001999, 0.1));
    clamped = (decremented > 0) ? decremented : 0;
    if (e != NULL) {
        t = e->t;
        *ma = clamped;
    }
    return pico8_vec_add(p, pico8_vec_qm(clamped, pico8_fixed_mul(t, PICO8_FIXED_LIT(0x00003333, 0.2))));
}

static void ij_step(pico8_vec_t p, pico8_struct_header_t *o, pico8_fixed_t *mk, pico8_struct_header_t **out_nearest) {
    pico8_vec_t o_p = pico8_entity_geom_p(o);
    pico8_fixed_t d = pico8_vec_len_sq(pico8_vec_sub(p, o_p));
    int same_position = (p.x == o_p.x && p.y == o_p.y);

    if (!same_position && d < *mk) {
        *mk = d;
        *out_nearest = o;
    }
}

pico8_fixed_t pico8_ij_ents(pico8_vec_t p, const pico8_ents_t *j_, pico8_struct_header_t **out_nearest) {
    pico8_fixed_t mk = pico8_fixed_from_int(32767);
    pico8_ents_iter_t it;
    pico8_struct_header_t *o;

    *out_nearest = NULL;
    if (j_ == NULL) {
        return mk;
    }
    it = pico8_ents_all(j_);
    while (pico8_ents_iter_next(&it, &o)) {
        ij_step(p, o, &mk, out_nearest);
    }
    return mk;
}

