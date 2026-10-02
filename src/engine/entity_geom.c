#include "engine/entity_geom.h"

#include <assert.h>

#include <stddef.h>

#include "engine/entity_registry.h"
#include "engine/geom.h"

static pico8_vec_t pico8_entity_geom_zero(void) {
    return pico8_vec_v(p8_fixed_from_int(0), p8_fixed_from_int(0));
}

/* `e`'s `ra` (`p` for the second), or (0,0) when `e` is NULL or the field is nil. */
static pico8_vec_t pico8_entity_geom_ra(const entity_t *e) {
    const pico8_vec_t *ra = e != NULL && e->pos != NULL ? *e->pos : NULL;

    return ra != NULL ? *ra : pico8_entity_geom_zero();
}

static pico8_vec_t pico8_entity_geom_p(const entity_t *e) {
    const pico8_vec_t *p = e != NULL && e->map_pos != NULL ? *e->map_pos : NULL;

    return p != NULL ? *p : pico8_entity_geom_zero();
}

int pico8_gk(const entity_t *e, const pico8_vec_t *mp, const pico8_vec_t *rp) {
    p8_fixed_t fa = e != NULL && e->layer != NULL ? *e->layer : p8_fixed_from_int(0);
    pico8_vec_t ra = pico8_entity_geom_ra(e);
    const pico8_vec_t *chosen;
    pico8_vec_t point;
    const pico8_qz_box_t *kq;

    chosen = (fa >= p8_fixed_from_int(10)) ? rp : mp;
    point = pico8_vec_sub(chosen != NULL ? *chosen : pico8_entity_geom_zero(), ra);
    kq = e != NULL ? e->hit_box : NULL;
    if (kq != NULL) {
        return pico8_qz_call(kq, point);
    }
    return 0;
}

int pico8_gd(const entity_t *e) {
    pico8_vec_t e_ra = pico8_entity_geom_ra(e);
    pico8_vec_t mo_ra = pico8_entity_geom_ra(pico8_mo());
    p8_fixed_t sixty_four = p8_fixed_from_int(64);
    p8_fixed_t seventy_six = p8_fixed_from_int(76);

    return p8_fixed_abs(p8_fixed_sub(p8_fixed_sub(e_ra.x, mo_ra.x), sixty_four)) <= seventy_six &&
           p8_fixed_abs(p8_fixed_sub(p8_fixed_sub(e_ra.y, mo_ra.y), sixty_four)) <= seventy_six;
}

pico8_vec_t pico8_ma(pico8_vec_t p, entity_t *e) {
    p8_fixed_t *ma = NULL;
    p8_fixed_t decremented;
    p8_fixed_t clamped;
    p8_fixed_t t = p8_fixed_from_int(0);

    if (e != NULL) {
        assert(e->shake != NULL && "ma: e is a planet (jo:kj) or the camera (qb:ph)");
        ma = e->shake;
    }
    decremented = p8_fixed_sub(ma != NULL ? *ma : p8_fixed_from_int(0), P8_FIXED_LIT(0x00001999, 0.1));
    clamped = (decremented > 0) ? decremented : 0;
    if (e != NULL) {
        t = e->state_time;
        *ma = clamped;
    }
    return pico8_vec_add(p, pico8_vec_qm(clamped, p8_fixed_mul(t, P8_FIXED_LIT(0x00003333, 0.2))));
}

static void ij_step(pico8_vec_t p, entity_t *o, p8_fixed_t *mk, entity_t **out_nearest) {
    pico8_vec_t o_p = pico8_entity_geom_p(o);
    p8_fixed_t d = pico8_vec_len_sq(pico8_vec_sub(p, o_p));
    int same_position = (p.x == o_p.x && p.y == o_p.y);

    if (!same_position && d < *mk) {
        *mk = d;
        *out_nearest = o;
    }
}

p8_fixed_t pico8_ij_ents(pico8_vec_t p, const pico8_ents_t *j_, entity_t **out_nearest) {
    p8_fixed_t mk = p8_fixed_from_int(32767);
    pico8_ents_iter_t it;
    entity_t *o;

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

