#include "engine/entity_geom.h"

#include <assert.h>

#include <stddef.h>

#include "engine/entity_registry.h"
#include "engine/geom.h"

static vec_t zero_vec(void) {
    return vec_make(p8_fixed_from_int(0), p8_fixed_from_int(0));
}

/* `e`'s `ra` (`p` for the second), or (0,0) when `e` is NULL or the field is nil. */
static vec_t pos_of(const entity_t *e) {
    const vec_t *ra = e != NULL && e->pos != NULL ? *e->pos : NULL;

    return ra != NULL ? *ra : zero_vec();
}

static vec_t map_pos_of(const entity_t *e) {
    const vec_t *p = e != NULL && e->map_pos != NULL ? *e->map_pos : NULL;

    return p != NULL ? *p : zero_vec();
}

int entity_geom_contains_cursor(const entity_t *e, const vec_t *mp, const vec_t *rp) {
    p8_fixed_t fa = e != NULL && e->layer != NULL ? *e->layer : p8_fixed_from_int(0);
    vec_t ra = pos_of(e);
    const vec_t *chosen;
    vec_t point;
    const box_t *kq;

    chosen = (fa >= p8_fixed_from_int(10)) ? rp : mp;
    point = vec_sub(chosen != NULL ? *chosen : zero_vec(), ra);
    kq = e != NULL ? e->hit_box : NULL;
    if (kq != NULL) {
        return box_contains(kq, point);
    }
    return 0;
}

int entity_geom_is_on_screen(const entity_t *e) {
    vec_t e_pos = pos_of(e);
    vec_t main_camera_pos = pos_of(main_camera());
    p8_fixed_t sixty_four = p8_fixed_from_int(64);
    p8_fixed_t seventy_six = p8_fixed_from_int(76);

    return p8_fixed_abs(p8_fixed_sub(p8_fixed_sub(e_pos.x, main_camera_pos.x), sixty_four)) <= seventy_six &&
           p8_fixed_abs(p8_fixed_sub(p8_fixed_sub(e_pos.y, main_camera_pos.y), sixty_four)) <= seventy_six;
}

vec_t entity_geom_shake(vec_t p, entity_t *e) {
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
    return vec_add(p, vec_from_polar(clamped, p8_fixed_mul(t, P8_FIXED_LIT(0x00003333, 0.2))));
}

static void nearest_step(vec_t p, entity_t *o, p8_fixed_t *mk, entity_t **out_nearest) {
    vec_t o_p = map_pos_of(o);
    p8_fixed_t d = vec_length_squared(vec_sub(p, o_p));
    int same_position = (p.x == o_p.x && p.y == o_p.y);

    if (!same_position && d < *mk) {
        *mk = d;
        *out_nearest = o;
    }
}

p8_fixed_t entity_geom_nearest(vec_t p, const entity_list_t *j_, entity_t **out_nearest) {
    p8_fixed_t mk = p8_fixed_from_int(32767);
    entity_list_iter_t it;
    entity_t *o;

    *out_nearest = NULL;
    if (j_ == NULL) {
        return mk;
    }
    it = entity_list_all(j_);
    while (entity_list_iter_next(&it, &o)) {
        nearest_step(p, o, &mk, out_nearest);
    }
    return mk;
}

