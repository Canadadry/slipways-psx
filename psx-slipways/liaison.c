#include "liaison.h"

#include <stddef.h>

#include "screen_map.h"
#include "world.h"

/* Flat top-level (non-static) globals, one scalar per liaison_drag_state_t
 * field (nested liaison_result_t structs flattened too), mirroring
 * cursor.c's g_cursor_* convention exactly -- so a future PCSX-Redux Lua
 * harness can address each field independently by symbol name in guest RAM
 * (see docs/prd/triage/psx-liaison-validity.md, "Stateful wrapper"). Never
 * written outside liaison_init/liaison_update. */
int g_liaison_dragging = 0;
int g_liaison_from_index = -1;
int g_liaison_hover_index = -1;
liaison_reason_t g_liaison_live_reason = LIAISON_VALID;
int g_liaison_live_blocking_planet = -1;
int g_liaison_live_blocking_route = -1;
liaison_reason_t g_liaison_last_reason = LIAISON_VALID;
int g_liaison_last_blocking_planet = -1;
int g_liaison_last_blocking_route = -1;
int g_liaison_last_valid_frames = 0;

/* Always-empty route list -- see liaison.h's LIAISON_MAX_ROUTES comment and
 * the PRD's "Never persist a committed route" guardrail. Nothing in this
 * chunk ever writes g_liaison_route_count above 0 or populates
 * g_liaison_routes; a future colonization step (Phase 3 step 10) is the
 * first to start writing into it. */
liaison_route_t g_liaison_routes[LIAISON_MAX_ROUTES];
int g_liaison_route_count = 0;

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

static liaison_result_t liaison_neutral_result(void) {
    liaison_result_t r;
    r.reason = LIAISON_VALID;
    r.blocking_planet = -1;
    r.blocking_route = -1;
    return r;
}

/* Shared "not dragging" shape: dragging/from_index/hover_index/live reset
 * to their idle convention. last/last_valid_frames are intentionally left
 * as prev's -- callers that want to set a real outcome (release) overwrite
 * them afterward; callers that don't (idle no-press, interrupted cancel)
 * leave them untouched, per the PRD's "no synthesized outcome" rule. */
static liaison_drag_state_t liaison_idle_state(liaison_drag_state_t prev) {
    liaison_drag_state_t next = prev;
    next.dragging = 0;
    next.from_index = -1;
    next.hover_index = -1;
    next.live = liaison_neutral_result();
    return next;
}

/* Resolves the live drag target for a held/released frame: hover_index via
 * liaison_pick_planet, then either that planet's index/position, or, when
 * nothing is hovered, to_index=-1 with the raw cursor position unprojected
 * to world space (mirrors jc:hq's {ra=mp,p=mp/128} fallback). */
static void liaison_resolve_target(int screen_x, int screen_y, const galaxy_t *galaxy,
                                    int *out_hover, int *out_to_index, planet_pos_t *out_to_pos) {
    int hover = liaison_pick_planet(screen_x, screen_y, galaxy, LIAISON_PICK_RADIUS_PX);
    *out_hover = hover;
    if (hover != -1) {
        *out_to_index = hover;
        *out_to_pos = galaxy->positions[hover];
    } else {
        *out_to_index = -1;
        *out_to_pos = screen_map_unproject(screen_x, screen_y);
    }
}

liaison_drag_state_t liaison_drag_step(liaison_drag_state_t prev,
                                        cursor_state_t cursor,
                                        const galaxy_t *galaxy,
                                        const liaison_route_t *routes, int route_count) {
    if (prev.dragging) {
        if (cursor.click_held) {
            liaison_drag_state_t next = prev;
            int to_index;
            planet_pos_t to_pos;
            liaison_resolve_target(cursor.x, cursor.y, galaxy, &next.hover_index, &to_index, &to_pos);
            next.live = liaison_check(prev.from_index, galaxy->positions[prev.from_index],
                                       to_index, to_pos, galaxy, routes, route_count);
            return next;
        }

        if (cursor.click_released) {
            int hover, to_index;
            planet_pos_t to_pos;
            liaison_resolve_target(cursor.x, cursor.y, galaxy, &hover, &to_index, &to_pos);
            liaison_result_t result = liaison_check(prev.from_index, galaxy->positions[prev.from_index],
                                                      to_index, to_pos, galaxy, routes, route_count);

            liaison_drag_state_t next = liaison_idle_state(prev);
            next.last = result;
            next.last_valid_frames = LIAISON_LAST_VALID_FRAMES;
            return next;
        }

        /* Neither held nor released this frame: the gesture was
         * interrupted (e.g. mouse mode toggled off mid-drag), not
         * concluded. Silently cancel -- no synthesized last outcome. */
        return liaison_idle_state(prev);
    }

    if (cursor.click_pressed) {
        int picked = liaison_pick_planet(cursor.x, cursor.y, galaxy, LIAISON_PICK_RADIUS_PX);
        if (picked != -1) {
            liaison_drag_state_t next = prev;
            next.dragging = 1;
            next.from_index = picked;
            next.hover_index = picked;
            next.live = liaison_check(picked, galaxy->positions[picked],
                                       picked, galaxy->positions[picked],
                                       galaxy, routes, route_count);
            return next;
        }
    }

    return liaison_idle_state(prev);
}

/* Resets every flat global to the idle state liaison_idle_state()'s own
 * scalars establish (dragging 0, from_index/hover_index -1, live/last
 * neutral {LIAISON_VALID,-1,-1}, last_valid_frames 0), and empties the route
 * list -- mirrors cursor_init()'s exact shape. */
void liaison_init(void) {
    g_liaison_dragging = 0;
    g_liaison_from_index = -1;
    g_liaison_hover_index = -1;
    g_liaison_live_reason = LIAISON_VALID;
    g_liaison_live_blocking_planet = -1;
    g_liaison_live_blocking_route = -1;
    g_liaison_last_reason = LIAISON_VALID;
    g_liaison_last_blocking_planet = -1;
    g_liaison_last_blocking_route = -1;
    g_liaison_last_valid_frames = 0;

    g_liaison_route_count = 0;
}

/* Reads the flat globals into a local liaison_drag_state_t, calls
 * world_get_galaxy() and liaison_drag_step(), then writes the result back
 * out into the flat globals -- mirrors cursor_update()'s exact
 * read-into-struct / call-pure-function / write-back-out shape. */
void liaison_update(cursor_state_t cursor) {
    liaison_drag_state_t prev;
    prev.dragging = g_liaison_dragging;
    prev.from_index = g_liaison_from_index;
    prev.hover_index = g_liaison_hover_index;
    prev.live.reason = g_liaison_live_reason;
    prev.live.blocking_planet = g_liaison_live_blocking_planet;
    prev.live.blocking_route = g_liaison_live_blocking_route;
    prev.last.reason = g_liaison_last_reason;
    prev.last.blocking_planet = g_liaison_last_blocking_planet;
    prev.last.blocking_route = g_liaison_last_blocking_route;
    prev.last_valid_frames = g_liaison_last_valid_frames;

    const galaxy_t *galaxy = world_get_galaxy();

    liaison_drag_state_t next = liaison_drag_step(prev, cursor, galaxy,
                                                    g_liaison_routes, g_liaison_route_count);

    g_liaison_dragging = next.dragging;
    g_liaison_from_index = next.from_index;
    g_liaison_hover_index = next.hover_index;
    g_liaison_live_reason = next.live.reason;
    g_liaison_live_blocking_planet = next.live.blocking_planet;
    g_liaison_live_blocking_route = next.live.blocking_route;
    g_liaison_last_reason = next.last.reason;
    g_liaison_last_blocking_planet = next.last.blocking_planet;
    g_liaison_last_blocking_route = next.last.blocking_route;
    g_liaison_last_valid_frames = next.last_valid_frames;
}

/* Assembles and returns a liaison_drag_state_t from the current flat
 * globals -- mirrors cursor_get_state() exactly. */
liaison_drag_state_t liaison_get_state(void) {
    liaison_drag_state_t state;
    state.dragging = g_liaison_dragging;
    state.from_index = g_liaison_from_index;
    state.hover_index = g_liaison_hover_index;
    state.live.reason = g_liaison_live_reason;
    state.live.blocking_planet = g_liaison_live_blocking_planet;
    state.live.blocking_route = g_liaison_live_blocking_route;
    state.last.reason = g_liaison_last_reason;
    state.last.blocking_planet = g_liaison_last_blocking_planet;
    state.last.blocking_route = g_liaison_last_blocking_route;
    state.last_valid_frames = g_liaison_last_valid_frames;
    return state;
}
