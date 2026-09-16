#include "liaison.h"

#include <stddef.h>

#include "screen_map.h"

/* Q16.16 throughout, int64_t intermediates wherever a product could
 * overflow int32_t -- same overflow-safety idiom as screen_map.c's
 * screen_map_axis and planetgen.c's dist_sq helpers. dx/dy/cax/cay stay
 * within PLANETGEN_PLACEMENT_SPAN's magnitude, so their pairwise products
 * (~1e10) and t's further *FX_ONE scaling (~1e15) both comfortably fit
 * int64_t. */
int liaison_point_blocks_segment(planet_pos_t a, planet_pos_t b, planet_pos_t c) {
    int64_t dx = (int64_t)fixed_sub(b.x, a.x);
    int64_t dy = (int64_t)fixed_sub(b.y, a.y);
    int64_t dot_dd = dx * dx + dy * dy;
    if (dot_dd == 0) {
        return 0;
    }

    int64_t cax = (int64_t)fixed_sub(c.x, a.x);
    int64_t cay = (int64_t)fixed_sub(c.y, a.y);
    int64_t dot_d_ca = dx * cax + dy * cay;

    /* t = dot(d, c-a) / dot(d,d), scaled by FX_ONE up front so the result
     * is a plain Q16.16 fixed_t (matches LIAISON_T_MARGIN_LOW/HIGH). */
    int64_t t = (dot_d_ca * (int64_t)FX_ONE) / dot_dd;

    if (t <= (int64_t)LIAISON_T_MARGIN_LOW || t >= (int64_t)LIAISON_T_MARGIN_HIGH) {
        return 0;
    }

    fixed_t tf = (fixed_t)t;
    fixed_t closest_x = fixed_add(a.x, fixed_mul((fixed_t)dx, tf));
    fixed_t closest_y = fixed_add(a.y, fixed_mul((fixed_t)dy, tf));

    int64_t ddx = (int64_t)fixed_sub(closest_x, c.x);
    int64_t ddy = (int64_t)fixed_sub(closest_y, c.y);
    int64_t dist_sq = ddx * ddx + ddy * ddy;

    return dist_sq < LIAISON_BLOCK_TOLERANCE_SQ;
}

/* Sign of the cross product (q-p) x (r-p): 1 = counter-clockwise,
 * -1 = clockwise, 0 = collinear. */
static int liaison_orientation(planet_pos_t p, planet_pos_t q, planet_pos_t r) {
    int64_t v1x = (int64_t)fixed_sub(q.x, p.x);
    int64_t v1y = (int64_t)fixed_sub(q.y, p.y);
    int64_t v2x = (int64_t)fixed_sub(r.x, p.x);
    int64_t v2y = (int64_t)fixed_sub(r.y, p.y);
    int64_t cross = v1x * v2y - v1y * v2x;
    if (cross > 0) {
        return 1;
    }
    if (cross < 0) {
        return -1;
    }
    return 0;
}

/* o1*o2 < 0 iff o1/o2 are strictly opposite nonzero signs -- any collinear
 * (zero) orientation among the four fails this, so a touching endpoint is
 * never reported as a crossing. */
int liaison_segments_cross(planet_pos_t p1, planet_pos_t p2, planet_pos_t q1, planet_pos_t q2) {
    int o1 = liaison_orientation(p1, p2, q1);
    int o2 = liaison_orientation(p1, p2, q2);
    int o3 = liaison_orientation(q1, q2, p1);
    int o4 = liaison_orientation(q1, q2, p2);

    return (o1 * o2 < 0) && (o3 * o4 < 0);
}

liaison_result_t liaison_check(int from_index, planet_pos_t from_pos,
                                int to_index, planet_pos_t to_pos,
                                const galaxy_t *galaxy,
                                const liaison_route_t *routes, int route_count) {
    liaison_result_t result = { LIAISON_VALID, -1, -1 };

    /* 1. Range check first; short-circuits everything below. */
    int64_t dx = (int64_t)fixed_sub(to_pos.x, from_pos.x);
    int64_t dy = (int64_t)fixed_sub(to_pos.y, from_pos.y);
    int64_t dist_sq = dx * dx + dy * dy;
    if (dist_sq > LIAISON_RANGE_SQ) {
        result.reason = LIAISON_TOO_FAR;
        return result;
    }

    /* 2. Already-connected; only meaningful against a real planet target. */
    if (to_index != -1) {
        for (int i = 0; i < route_count; i++) {
            if ((routes[i].a == from_index && routes[i].b == to_index) ||
                (routes[i].a == to_index && routes[i].b == from_index)) {
                result.reason = LIAISON_ALREADY_CONNECTED;
                return result;
            }
        }
    }

    /* 3. Blocked-by-planet; first blocker in galaxy index order wins. */
    if (galaxy != NULL) {
        for (int i = 0; i < galaxy->count; i++) {
            if (liaison_point_blocks_segment(from_pos, to_pos, galaxy->positions[i])) {
                result.reason = LIAISON_BLOCKED_BY_PLANET;
                result.blocking_planet = i;
                return result;
            }
        }
    }

    /* 4. Crosses-route, only reached if step 3 found nothing; first
     * crossing route in list order wins. */
    if (galaxy != NULL) {
        for (int i = 0; i < route_count; i++) {
            planet_pos_t qa = galaxy->positions[routes[i].a];
            planet_pos_t qb = galaxy->positions[routes[i].b];
            if (liaison_segments_cross(from_pos, to_pos, qa, qb)) {
                result.reason = LIAISON_CROSSES_ROUTE;
                result.blocking_route = i;
                return result;
            }
        }
    }

    /* 5. Otherwise valid. */
    return result;
}

/* Screen-pixel-space hit test: reuses screen_map_planet (world->screen)
 * per planet rather than a screen->world inverse (that inverse,
 * screen_map_unproject, exists for the live validity check's raw-cursor
 * case, not for picking). Squared pixel distance keeps this integer-only;
 * "nearest wins" tracks and compares the running minimum rather than
 * returning the first in-radius planet found. */
int liaison_pick_planet(int screen_x, int screen_y, const galaxy_t *galaxy, int pick_radius_px) {
    if (galaxy == NULL) {
        return -1;
    }

    int64_t radius_sq = (int64_t)pick_radius_px * (int64_t)pick_radius_px;
    int best_index = -1;
    int64_t best_dist_sq = 0;

    for (int i = 0; i < galaxy->count; i++) {
        int px, py;
        screen_map_planet(galaxy->positions[i], &px, &py);

        int64_t dx = (int64_t)(px - screen_x);
        int64_t dy = (int64_t)(py - screen_y);
        int64_t dist_sq = dx * dx + dy * dy;

        if (dist_sq <= radius_sq && (best_index == -1 || dist_sq < best_dist_sq)) {
            best_index = i;
            best_dist_sq = dist_sq;
        }
    }

    return best_index;
}
