#include "game/pad_nav.h"

#include "engine/entity_id.h"
#include "engine/entity_registry.h"
#include "engine/geom.h"
#include "game/background.h"
#include "game/main_economy.h"
#include "game/menu.h"
#include "game/pad_jump.h"
#include "game/pad_panel.h"
#include "game/pad_target.h"
#include "game/planet.h"
#include "game/planet_family_trade.h"
#include "game/scroll.h"
#include "game/slipway.h"
#include "game/structure_icon.h"
#include "game/ui.h"
#include "p8/vram.h"

#include <assert.h>
#include <limits.h>

int pad_nav_panel_open(void) {
    return pad_panel_open();
}

pad_nav_tab_t pad_nav_tab(const pad_nav_t *nav) {
    return main_economy_typed() != NULL ? nav->tab : PAD_NAV_TAB_MAP;
}

int pad_nav_free_mode(const pad_nav_t *nav) {
    return main_economy_typed() != NULL && nav->tab == PAD_NAV_TAB_MAP && !pad_nav_panel_open() &&
           nav->map_mode == PAD_NAV_MAP_FREE;
}

/* NULL once the entity is gone or dead. */
static entity_t *live(entity_t *e) {
    return e != NULL && !e->is_dead ? e : NULL;
}

int pad_nav_jump_mode(const pad_nav_t *nav) {
    return main_economy_typed() != NULL && nav->tab == PAD_NAV_TAB_MAP && !pad_nav_panel_open() &&
           nav->map_mode == PAD_NAV_MAP_JUMP;
}

static entity_t *hovered_object(const pad_nav_t *nav) {
    entity_t *hovered = entity_get(nav->jump.hovered);
    return pad_target_is_candidate(hovered) ? hovered : NULL;
}

entity_t *pad_nav_hovered(const pad_nav_t *nav) {
    return nav->map_mode == PAD_NAV_MAP_JUMP ? hovered_object(nav) : NULL;
}

entity_t *pad_nav_selection(const pad_nav_t *nav) {
    return nav->map_mode == PAD_NAV_MAP_NETWORK ? live(entity_get(nav->selection)) : NULL;
}

entity_t *pad_nav_starting_planet(const pad_nav_t *nav) {
    return nav->map_mode == PAD_NAV_MAP_FREE ? live(entity_get(nav->starting_planet)) : NULL;
}

enum { SLIPWAYS_CAP = 32 };

typedef struct {
    entity_id_t slipway;
    int dx;
    int dy;
} heading_t;

/* Straight up opens the right half, straight down the left one. */
static int in_left_half(const heading_t *h) {
    return h->dx < 0 || (h->dx == 0 && h->dy >= 0);
}

static int turns_before(const heading_t *a, const heading_t *b) {
    if (in_left_half(a) != in_left_half(b)) {
        return in_left_half(b);
    }
    return a->dx * b->dy - a->dy * b->dx > 0;
}

entity_t *pad_nav_next_slipway(const entity_t *planet, const entity_t *current, int step) {
    assert(step >= -1 && step <= 1 && "pad_nav_next_slipway: step is -1, 0 or 1");
    assert((planet == NULL || planet_is_family(planet)) && "pad_nav_next_slipway: only a planet holds a slipways list");
    if (planet == NULL) {
        return NULL;
    }
    heading_t sorted[SLIPWAYS_CAP];
    int count = 0;
    vec_t from = entity_pos(planet);
    entity_id_list_iter_t it = entity_id_list_all(&((const planet_t *)planet)->slipways);
    entity_t *slipway;
    while (entity_id_list_iter_next(&it, &slipway)) {
        assert(!slipway->is_dead &&
               "pad_nav_next_slipway: a slipway dies only when its drop is refused, before it joins a planet's list");
        entity_t *other = slipway_other_end((const slipway_t *)slipway, planet);
        if (other == NULL) {
            continue;
        }
        assert(count < SLIPWAYS_CAP && "pad_nav_next_slipway: more slipways on one planet than SLIPWAYS_CAP");
        vec_t to = entity_pos(other);
        heading_t heading = { entity_id_link(slipway), p8_fixed_flr_int(to.x) - p8_fixed_flr_int(from.x),
                              p8_fixed_flr_int(to.y) - p8_fixed_flr_int(from.y) };
        int at = count++;
        for (; at > 0 && turns_before(&heading, &sorted[at - 1]); at--) {
            sorted[at] = sorted[at - 1];
        }
        sorted[at] = heading;
    }
    if (count == 0) {
        return NULL;
    }
    for (int i = 0; i < count; i++) {
        if (entity_get(sorted[i].slipway) == current) {
            return entity_get(sorted[(i + step + count) % count].slipway);
        }
    }
    return entity_get(sorted[0].slipway);
}

entity_t *pad_nav_build_icon(const pad_nav_t *nav) {
    entity_t *icon = entity_get(nav->build_icon);
    return pad_nav_tab(nav) == PAD_NAV_TAB_BUILD && structure_icon_is_revealed(icon) ? icon : NULL;
}

static entity_t *placing_structure(entity_t *e) {
    e = live(e);
    return e != NULL && planet_is_family(e) && e->state == ENTITY_STATE_PLACING ? e : NULL;
}

/* NULL unless the ui's left-held entity is a structure in its placing state. */
static entity_t *held_structure(void) {
    return main_ui() != NULL ? placing_structure(ui_left_held(main_ui())) : NULL;
}

entity_t *pad_nav_placing(const pad_nav_t *nav) {
    return nav->placement == PAD_NAV_PLACEMENT_CARRYING ? held_structure() : NULL;
}

entity_t *pad_nav_selected_slipway(const pad_nav_t *nav) {
    return pad_nav_next_slipway(pad_nav_selection(nav), live(entity_get(nav->selected_slipway)), 0);
}

int pad_nav_is_item(const entity_t *e) {
    return pad_panel_is_item(e);
}

entity_t *pad_nav_first_item(void) {
    return pad_panel_first_item();
}

entity_t *pad_nav_neighbor(const entity_t *item, int direction) {
    return pad_panel_neighbor(item, direction);
}

/* 0 when no direction went down; Up, Down, Left, Right is the priority order. */
static int pressed_direction(int pressed) {
    static const int order[4] = { P8_PAD_UP, P8_PAD_DOWN, P8_PAD_LEFT, P8_PAD_RIGHT };
    for (int i = 0; i < 4; i++) {
        if (pressed & order[i]) {
            return order[i];
        }
    }
    return 0;
}

enum { HELD_FRAMES_CAP = 41 };

static const int pan_directions[4] = { P8_PAD_UP, P8_PAD_DOWN, P8_PAD_LEFT, P8_PAD_RIGHT };

static void reset_free_mode(pad_nav_t *nav) {
    for (int i = 0; i < 4; i++) {
        nav->held_frames[i] = 0;
    }
    nav->cross_engaged = 0;
}

static int pan_speed(int held_frames) {
    if (held_frames == 0) {
        return 0;
    }
    if (held_frames <= 10) {
        return 1;
    }
    return held_frames <= 40 ? 2 : 4;
}

static void free_mode_pan(pad_nav_t *nav, int pad_buttons) {
    int speed[4];
    for (int i = 0; i < 4; i++) {
        if (!(pad_buttons & pan_directions[i])) {
            nav->held_frames[i] = 0;
        } else if (nav->held_frames[i] < HELD_FRAMES_CAP) {
            nav->held_frames[i]++;
        }
        speed[i] = pan_speed(nav->held_frames[i]);
    }
    camera_t *camera = main_camera_typed();
    if (camera != NULL) {
        camera_pan(camera, speed[3] - speed[2], speed[1] - speed[0]);
    }
}

static void clear_drag(pad_nav_t *nav) {
    nav->drag.phase = PAD_NAV_DRAG_NONE;
    nav->drag.target = ENTITY_ID_NONE;
    nav->drag.linked_before = 0;
}

/* A drag phase that acts on the next frame: tabs and pause wait for it. */
static int drag_due(const pad_nav_t *nav) {
    return nav->drag.phase != PAD_NAV_DRAG_NONE && nav->drag.phase != PAD_NAV_DRAG_HELD;
}

/* Keeps the jump state's game and seeded flag: a reset within a game reseeds from the centre, never on the first colony. */
static void reset_map_mode(pad_nav_t *nav) {
    nav->map_mode = PAD_NAV_MAP_JUMP;
    nav->jump.hovered = ENTITY_ID_NONE;
    pad_jump_repeat_clear(&nav->jump);
    nav->selection = ENTITY_ID_NONE;
    nav->selection_frames = 0;
    nav->selected_slipway = ENTITY_ID_NONE;
    nav->starting_planet = ENTITY_ID_NONE;
    nav->starting_slipway = ENTITY_ID_NONE;
    nav->last_planet = ENTITY_ID_NONE;
    nav->last_slipway = ENTITY_ID_NONE;
    nav->resume_start = ENTITY_ID_NONE;
    nav->placement = PAD_NAV_PLACEMENT_NONE;
    nav->suspended_carry = ENTITY_ID_NONE;
    clear_drag(nav);
}

static int opens_a_panel(pad_nav_tab_t tab) {
    return tab == PAD_NAV_TAB_TECH || tab == PAD_NAV_TAB_SCORE;
}

static void set_tab(pad_nav_t *nav, pad_nav_tab_t tab) {
    nav->tab = tab;
    nav->landing_due = opens_a_panel(tab);
    nav->cross_armed = 0;
}

void pad_nav_init(pad_nav_t *nav) {
    nav->focus = ENTITY_ID_NONE;
    nav->build_icon = ENTITY_ID_NONE;
    set_tab(nav, PAD_NAV_TAB_MAP);
    pad_jump_init(&nav->jump);
    reset_map_mode(nav);
    nav->release_pending = 0;
    nav->pause_due = 0;
    nav->pause_cross_armed = 0;
    nav->previous_pad = 0;
    nav->left_was_down = 0;
    nav->cursor_x = 0;
    nav->cursor_y = 0;
    reset_free_mode(nav);
}

entity_t *pad_nav_focus(const pad_nav_t *nav) {
    entity_t *focus = entity_get(nav->focus);
    return pad_nav_is_item(focus) ? focus : NULL;
}

/* A × held into pause presses no row until pressed again: pad_panel_step holds the left button while × is held. */
static void panel_step(pad_nav_t *nav, int pad_buttons, int pressed, int direction, p8_input_frame_t *played) {
    if (pause_menu_shown()) {
        nav->pause_cross_armed |= (pressed & P8_PAD_CROSS) != 0;
        if (!nav->pause_cross_armed) {
            pad_buttons &= ~P8_PAD_CROSS;
        }
    }
    nav->focus = entity_id_link(pad_panel_step(entity_get(nav->focus), pad_buttons, direction, &nav->cursor_x,
                                               &nav->cursor_y, &played->mouse_buttons));
}

/* The planet's world point minus the camera, floored: the rule of the world-space panel items. */
static void screen_point(const entity_t *e, int *x, int *y) {
    vec_t at = entity_pos(e);
    const camera_t *camera = main_camera_typed();
    if (camera != NULL) {
        at = vec_sub(at, entity_pos(&camera->header));
    }
    *x = p8_fixed_flr_int(at.x);
    *y = p8_fixed_flr_int(at.y);
}

static entity_t *entity_under(int x, int y, int (*accept)(const entity_t *e)) {
    return pad_panel_entity_under(x, y, PAD_PANEL_ANY_LAYER, accept);
}

static int presses_off_background(const entity_t *e) {
    return e->class->left_press != NULL && e->class != background_class();
}

/* Only these two states stop the dispatch in planet_left_press: any other lets the press reach the background, a probe. */
static int is_clickable_planet(const entity_t *e) {
    return planet_is_family(e) && (e->state == ENTITY_STATE_KNOWN || e->state == ENTITY_STATE_SETTLED);
}

/* A press at the cursor on this frame and a release on the next. */
static void click(pad_nav_t *nav, p8_input_frame_t *played) {
    played->mouse_buttons = P8_MOUSE_LEFT;
    nav->release_pending = 1;
}

/* A settled planet opens its panel on the release. */
static void click_planet(pad_nav_t *nav, const entity_t *planet, p8_input_frame_t *played) {
    click(nav, played);
    nav->map_mode = PAD_NAV_MAP_NETWORK;
    nav->selection = entity_id_link(planet);
}

/* A planet near the pad bound may stay a few pixels off the centre. */
static void centre_camera_on(const entity_t *e) {
    camera_t *camera = main_camera_typed();
    if (camera == NULL) {
        return;
    }
    vec_t at = entity_pos(e);
    vec_t from = camera_pos_before_shake(camera);
    camera_pan(camera, p8_fixed_flr_int(at.x) - p8_fixed_flr_int(from.x) - P8_SCREEN_WIDTH / 2,
               p8_fixed_flr_int(at.y) - p8_fixed_flr_int(from.y) - P8_SCREEN_HEIGHT / 2);
}

static void land_on(pad_nav_t *nav, const entity_t *planet, const entity_t *slipway) {
    centre_camera_on(planet);
    nav->map_mode = PAD_NAV_MAP_NETWORK;
    nav->selection = entity_id_link(planet);
    nav->selected_slipway = entity_id_link(slipway);
    nav->starting_planet = ENTITY_ID_NONE;
    nav->starting_slipway = ENTITY_ID_NONE;
}

static void return_to_starting_planet(pad_nav_t *nav) {
    entity_t *start = pad_nav_starting_planet(nav);
    if (start != NULL) {
        land_on(nav, start, live(entity_get(nav->starting_slipway)));
    }
}

/* Plain free mode only: at game start, or once the last planet is dead, ○ does nothing. */
static void return_to_last_planet(pad_nav_t *nav) {
    entity_t *last = live(entity_get(nav->last_planet));
    if (last != NULL) {
        land_on(nav, last, live(entity_get(nav->last_slipway)));
    }
}

static void enter_free_mode(pad_nav_t *nav, const entity_t *selected) {
    centre_camera_on(selected);
    nav->starting_planet = entity_id_link(selected);
    nav->starting_slipway = nav->selected_slipway;
    nav->selected_slipway = ENTITY_ID_NONE;
    nav->map_mode = PAD_NAV_MAP_FREE;
    nav->selection = ENTITY_ID_NONE;
    nav->cursor_x = P8_SCREEN_WIDTH / 2;
    nav->cursor_y = P8_SCREEN_HEIGHT / 2;
}

/* Moves no camera: a snap here would shift every later point action of a replay off its reference world point. */
static void enter_plain_free_mode(pad_nav_t *nav) {
    nav->map_mode = PAD_NAV_MAP_FREE;
    nav->selection = ENTITY_ID_NONE;
    nav->selected_slipway = ENTITY_ID_NONE;
    nav->starting_planet = ENTITY_ID_NONE;
    nav->starting_slipway = ENTITY_ID_NONE;
    nav->last_planet = ENTITY_ID_NONE;
    nav->last_slipway = ENTITY_ID_NONE;
    nav->cursor_x = P8_SCREEN_WIDTH / 2;
    nav->cursor_y = P8_SCREEN_HEIGHT / 2;
    pad_jump_repeat_clear(&nav->jump);
}

/* NULL when no slipway joins the two planets. */
static entity_t *slipway_between(const entity_t *start, const entity_t *target) {
    assert(planet_is_family(start) && "slipway_between: only a planet holds a slipways list");
    entity_id_list_iter_t it = entity_id_list_all(&((const planet_t *)start)->slipways);
    entity_t *slipway;
    while (entity_id_list_iter_next(&it, &slipway)) {
        assert(!slipway->is_dead &&
               "slipway_between: a slipway dies only when its drop is refused, before it joins a planet's list");
        if (slipway_other_end((const slipway_t *)slipway, start) == target) {
            return slipway;
        }
    }
    return NULL;
}

/* The ui presses the highest layer first: a HUD item over the planet would take the press. */
static int pressed_before(const entity_t *planet, int x, int y) {
    return pad_panel_entity_under(x, y, p8_fixed_flr_int(planet->layer), pad_panel_is_candidate) != NULL;
}

/* NULL unless the ui holds the slipway the drag pressed out: from the frame after its press until its release. */
static entity_t *dragged_slipway(const pad_nav_t *nav) {
    entity_t *held = nav->drag.phase == PAD_NAV_DRAG_HELD && main_ui() != NULL ? live(ui_left_held(main_ui())) : NULL;
    return held != NULL && held->class == slipway_class() ? held : NULL;
}

/* The ui's own drop with no target: slipway_drop refuses it, kills it and charges nothing. A mouse release would not
 * do: over the starting planet it opens its panel, under a box the ui ignores it. */
static void cancel_drag(pad_nav_t *nav) {
    entity_t *held = dragged_slipway(nav);
    if (held != NULL) {
        held->class->drop(held, NULL);
        ui_set_left_held(main_ui(), NULL);
    }
    clear_drag(nav);
}

/* Plain free mode: no starting planet, so no drag starts again; ○ goes back to the planet it came from. */
static void leave_starting_planet(pad_nav_t *nav) {
    cancel_drag(nav);
    nav->last_planet = nav->starting_planet;
    nav->last_slipway = nav->starting_slipway;
    nav->starting_planet = ENTITY_ID_NONE;
    nav->starting_slipway = ENTITY_ID_NONE;
}

/* A refused press falls through to the background, which makes a probe or a wormhole. */
static int takes_slipway(const entity_t *start) {
    return !(start->class->tags & TAG(TAG_SLIPGATE)) || slipgate_takes_slipway((const planet_t *)start);
}

/* Returns whether the press went out: on the settled starting planet, whose left press hands the ui a slipway, as a
 * mouse press there. The press needs the left button up on the frame before. */
static int begin_drag(pad_nav_t *nav, p8_input_frame_t *played) {
    entity_t *start = pad_nav_starting_planet(nav);
    if (start == NULL || start->state != ENTITY_STATE_SETTLED || nav->left_was_down || !takes_slipway(start)) {
        return 0;
    }
    int x, y;
    screen_point(start, &x, &y);
    if (pressed_before(start, x, y)) {
        return 0;
    }
    nav->cursor_x = x;
    nav->cursor_y = y;
    played->mouse_buttons = P8_MOUSE_LEFT;
    nav->drag.phase = PAD_NAV_DRAG_HELD;
    return 1;
}

static int is_settled_planet(const entity_t *e) {
    return planet_is_family(e) && e->state == ENTITY_STATE_SETTLED;
}

/* Returns whether □ landed network mode on the settled planet under the centre, the drag dropped first. Never on the
 * starting planet: right after □ the centre is still on it, and □ there leaves for plain free mode. */
static int square_selects_under_centre(pad_nav_t *nav) {
    entity_t *planet = entity_under(nav->cursor_x, nav->cursor_y, is_settled_planet);
    if (planet == NULL || planet == pad_nav_starting_planet(nav)) {
        return 0;
    }
    cancel_drag(nav);
    land_on(nav, planet, NULL);
    return 1;
}

/* Returns whether △ clicked a planet under the centre, which lands network mode on it while its panel is up; once
 * the panel is gone, free mode comes back from the same starting planet. */
static int triangle_on_centre(pad_nav_t *nav, p8_input_frame_t *played) {
    entity_t *planet = entity_under(nav->cursor_x, nav->cursor_y, is_clickable_planet);
    if (planet == NULL) {
        return 0;
    }
    nav->resume_start = nav->starting_planet;
    nav->starting_planet = ENTITY_ID_NONE;
    click_planet(nav, planet, played);
    return 1;
}

/* Returns whether the frame went to what the drag's release left due: the landing, the probe's press or △'s click. */
static int drag_due_step(pad_nav_t *nav, p8_input_frame_t *played) {
    pad_nav_drag_t drag = nav->drag;
    if (!drag_due(nav)) {
        return 0;
    }
    clear_drag(nav);
    if (drag.phase == PAD_NAV_DRAG_PROBE_DUE) {
        nav->cross_engaged = 1;
        played->mouse_buttons = P8_MOUSE_LEFT;
        return 1;
    }
    if (drag.phase == PAD_NAV_DRAG_TRIANGLE_DUE) {
        return triangle_on_centre(nav, played);
    }
    entity_t *start = pad_nav_starting_planet(nav);
    entity_t *target = live(entity_get(drag.target));
    entity_t *made = start != NULL && target != NULL && !drag.linked_before ? slipway_between(start, target) : NULL;
    if (made == NULL) {
        return 0;
    }
    land_on(nav, target, made);
    return 1;
}

/* The left button held at the centre is the mouse's drag; × lets it go there, so the ui's own drop decides. △ needs
 * the button up for a frame before its click. */
static void drag_step(pad_nav_t *nav, int pressed, int circle_returns, int release, p8_input_frame_t *played) {
    entity_t *start = pad_nav_starting_planet(nav);
    if (start == NULL || circle_returns) {
        cancel_drag(nav);
        return_to_starting_planet(nav);
        return;
    }
    if ((pressed & P8_PAD_TRIANGLE) && !release) {
        cancel_drag(nav);
        nav->drag.phase = PAD_NAV_DRAG_TRIANGLE_DUE;
        return;
    }
    if ((pressed & P8_PAD_CROSS) && !release) {
        entity_t *target = entity_under(nav->cursor_x, nav->cursor_y, planet_is_family);
        if (target == NULL && entity_under(nav->cursor_x, nav->cursor_y, presses_off_background) == NULL) {
            nav->drag.phase = PAD_NAV_DRAG_PROBE_DUE;
            return;
        }
        if (target != NULL && target != start) {
            nav->drag.phase = PAD_NAV_DRAG_DROPPED;
            nav->drag.target = entity_id_link(target);
            nav->drag.linked_before = slipway_between(start, target) != NULL;
            return;
        }
    }
    played->mouse_buttons = P8_MOUSE_LEFT;
}

/* On a click's release frame × and △ start nothing: their press would be forced up, and △ would still select. A
 * settled starting planet keeps a slipway dragged out of it whenever nothing else holds the left button. */
static void free_mode_step(pad_nav_t *nav, int pad_buttons, int pressed, int circle_returns, int release,
                           p8_input_frame_t *played) {
    nav->focus = ENTITY_ID_NONE;
    nav->cursor_x = P8_SCREEN_WIDTH / 2;
    nav->cursor_y = P8_SCREEN_HEIGHT / 2;
    free_mode_pan(nav, pad_buttons);
    if (drag_due_step(nav, played)) {
        return;
    }
    if ((pressed & P8_PAD_SQUARE) && !nav->cross_engaged) {
        if (square_selects_under_centre(nav)) {
            return;
        }
        if (pad_nav_starting_planet(nav) != NULL) {
            leave_starting_planet(nav);
            return;
        }
    }
    if (dragged_slipway(nav) != NULL) {
        drag_step(nav, pressed, circle_returns, release, played);
        return;
    }
    clear_drag(nav);
    if (!(pad_buttons & P8_PAD_CROSS)) {
        nav->cross_engaged = 0;
    } else if ((pressed & P8_PAD_CROSS) && !release) {
        nav->cross_engaged = entity_under(nav->cursor_x, nav->cursor_y, presses_off_background) == NULL;
    }
    if (nav->cross_engaged) {
        played->mouse_buttons = P8_MOUSE_LEFT;
    } else if ((pressed & P8_PAD_TRIANGLE) && !release && triangle_on_centre(nav, played)) {
        return;
    } else if (circle_returns && pad_nav_starting_planet(nav) != NULL) {
        return_to_starting_planet(nav);
    } else if (circle_returns) {
        return_to_last_planet(nav);
    } else if (!release) {
        (void)begin_drag(nav, played);
    }
}

/* The camera stays on the planet △ opened, so the drag resumes aimed at it. */
static void resume_free_mode(pad_nav_t *nav, entity_t *start, p8_input_frame_t *played) {
    nav->resume_start = ENTITY_ID_NONE;
    nav->starting_planet = entity_id_link(start);
    nav->map_mode = PAD_NAV_MAP_FREE;
    nav->selection = ENTITY_ID_NONE;
    nav->selected_slipway = ENTITY_ID_NONE;
    nav->cursor_x = P8_SCREEN_WIDTH / 2;
    nav->cursor_y = P8_SCREEN_HEIGHT / 2;
    (void)begin_drag(nav, played);
}

static void travel_to(pad_nav_t *nav, const entity_t *planet) {
    centre_camera_on(planet);
    nav->selection = entity_id_link(planet);
    nav->selection_frames = 0;
    screen_point(planet, &nav->cursor_x, &nav->cursor_y);
}

/* On △'s release frame the cursor must stay on the clicked planet: a move would drop its held slipway elsewhere. */
static void network_step(pad_nav_t *nav, const entity_t *selected, int pressed, int releasing, int circle_returns,
                         p8_input_frame_t *played) {
    nav->focus = ENTITY_ID_NONE;
    int turn = pressed_direction(pressed & (P8_PAD_LEFT | P8_PAD_RIGHT));
    entity_t *resumed = live(entity_get(nav->resume_start));
    if (resumed == NULL) {
        nav->resume_start = ENTITY_ID_NONE;
    } else if (!releasing && !nav->release_pending) {
        resume_free_mode(nav, resumed, played);
        return;
    }
    if (circle_returns && !releasing) {
        reset_map_mode(nav);
        nav->jump.hovered = entity_id_link(selected);
        screen_point(selected, &nav->cursor_x, &nav->cursor_y);
        return;
    }
    entity_t *slipway = pad_nav_selected_slipway(nav);
    nav->selected_slipway = entity_id_link(slipway);
    screen_point(selected, &nav->cursor_x, &nav->cursor_y);
    if ((pressed & P8_PAD_TRIANGLE) && !releasing && is_clickable_planet(selected)) {
        click_planet(nav, selected, played);
    } else if (pressed & P8_PAD_SQUARE) {
        enter_free_mode(nav, selected);
        int pressed_start = !releasing && begin_drag(nav, played);
        assert((pressed_start || releasing || selected->state != ENTITY_STATE_SETTLED || !takes_slipway(selected)) &&
               "network_step: centred, the settled planet lies clear of the HUD, so the press reaches it");
        (void)pressed_start;
    } else if (slipway == NULL || releasing) {
        return;
    } else if (pressed & P8_PAD_CROSS) {
        entity_t *other = slipway_other_end((const slipway_t *)slipway, selected);
        if (live(other) != NULL) {
            travel_to(nav, other);
        }
    } else if (turn != 0) {
        nav->selected_slipway = entity_id_link(pad_nav_next_slipway(selected, slipway, turn == P8_PAD_RIGHT ? 1 : -1));
    }
}

/* The title and a new game rebuild the id table above every old generation, so the old game's economy id, like an old
 * selection, resolves to NULL: no title frame is needed in between. */
static void start_new_game(pad_nav_t *nav) {
    entity_t *economy = main_economy_typed() != NULL ? &main_economy_typed()->header : NULL;
    if (economy == NULL || entity_get(nav->jump.game) == economy) {
        return;
    }
    pad_jump_init(&nav->jump);
    nav->jump.game = entity_id_link(economy);
    reset_map_mode(nav);
}

static void drop_stale_map_mode(pad_nav_t *nav) {
    if (nav->map_mode == PAD_NAV_MAP_NETWORK && pad_nav_selection(nav) == NULL) {
        reset_map_mode(nav);
    }
}

/* The title's difficulty rows are no box, so ○ cannot close them. Pause closes only by pause_step's press: cleared
 * here, on a release frame, the carry's resume would press on the map. */
static void circle_closes_the_box(void) {
    if (pad_panel_box_open() && !pause_menu_shown()) {
        ui_clear_ephemerals(main_ui());
    }
}

static void show_selection(pad_nav_t *nav, const entity_t *shown) {
    ui_t *ui = main_ui();
    if (shown == NULL) {
        nav->selection_frames = 0;
    }
    if (ui != NULL) {
        ui_set_pad_selection(ui, shown, shown != NULL ? pad_nav_selected_slipway(nav) : NULL, nav->selection_frames);
    }
    if (shown != NULL) {
        nav->selection_frames = (nav->selection_frames + 1) % UI_PAD_BLINK_PERIOD;
    }
}

/* The score panel during play is its backdrop alone; the final score panel has items, the pause menu is a box. */
static int own_panel_up(const pad_nav_t *nav) {
    if (nav->tab == PAD_NAV_TAB_TECH) {
        return pad_panel_box_open() && !pause_menu_shown();
    }
    if (nav->tab == PAD_NAV_TAB_SCORE) {
        return pad_nav_panel_open() && !pad_panel_box_open() && pad_panel_first_item() == NULL;
    }
    return 0;
}

static int foreign_panel_open(const pad_nav_t *nav) {
    return pad_nav_panel_open() && !own_panel_up(nav);
}

static int planet_menu_open(const pad_nav_t *nav) {
    return nav->tab == PAD_NAV_TAB_MAP && pad_panel_box_open() && !pause_menu_shown();
}

static int tabs_can_switch(const pad_nav_t *nav, int release) {
    if ((foreign_panel_open(nav) && !planet_menu_open(nav)) || release || nav->landing_due) {
        return 0;
    }
    return nav->tab != PAD_NAV_TAB_MAP || !drag_due(nav);
}

typedef enum { CANCELLED_NOTHING, CANCELLED_STRUCTURE, CANCELLED_SLIPWAY } cancelled_t;

/* A cancelled structure needs the frame's cursor on cancel_point, button up; a cancelled slipway is already dropped. */
static cancelled_t unwind_sub_modes(pad_nav_t *nav) {
    if (nav->tab != PAD_NAV_TAB_MAP) {
        return CANCELLED_NOTHING;
    }
    if (planet_menu_open(nav)) {
        ui_clear_ephemerals(main_ui());
    }
    cancelled_t cancelled = pad_nav_placing(nav) != NULL  ? CANCELLED_STRUCTURE
                            : dragged_slipway(nav) != NULL ? CANCELLED_SLIPWAY
                                                           : CANCELLED_NOTHING;
    cancel_drag(nav);
    return_to_starting_planet(nav);
    return cancelled;
}

/* The score backdrop hit-tests everywhere above the HUD, so a press anywhere reaches it and its on_click hides the panel. */
static void leave_tab(pad_nav_t *nav, p8_input_frame_t *played) {
    if (!own_panel_up(nav)) {
        return;
    }
    if (nav->tab == PAD_NAV_TAB_TECH) {
        ui_clear_ephemerals(main_ui());
    } else {
        click(nav, played);
    }
}

/* R1 moves forward, L1 back, both together nowhere: 0. */
static int shoulder_step(int pressed) {
    return ((pressed & P8_PAD_R1) != 0) - ((pressed & P8_PAD_L1) != 0);
}

static void shoulder_switch(pad_nav_t *nav, int step, p8_input_frame_t *played) {
    leave_tab(nav, played);
    set_tab(nav, (pad_nav_tab_t)((nav->tab + step + PAD_NAV_TAB_COUNT) % PAD_NAV_TAB_COUNT));
}

/* Above and left of the screen: no left_press there that × could reach. */
enum { PARKED_X = -16, PARKED_Y = -16 };

/* NULL on map and build, or once that button is gone. */
static entity_t *tab_button(pad_nav_tab_t tab) {
    static const int hud_button[PAD_NAV_TAB_COUNT] = { [PAD_NAV_TAB_END_TURN] = 1, [PAD_NAV_TAB_TECH] = 2,
                                                        [PAD_NAV_TAB_SCORE] = 3 };
    return hud_button[tab] != 0 ? (entity_t *)galaxy_menu_find_button(hud_button[tab]) : NULL;
}

/* NULL when no revealed icon lies on that side of x: step -1 looks left, 1 right. */
static entity_t *icon_beside(int x, int step) {
    entity_t *nearest = NULL;
    int nearest_x = 0;
    const entity_list_t *bucket = entities_by_message(ENTITY_MESSAGE_HIT_TEST);
    for (int i = 0; i < bucket->count; i++) {
        entity_t *e = bucket->items[i];
        int at_x, at_y;
        if (!structure_icon_is_revealed(e)) {
            continue;
        }
        pad_panel_focus_point(e, &at_x, &at_y);
        if ((step < 0 ? at_x < x : at_x > x) && (nearest == NULL || (step < 0 ? at_x > nearest_x : at_x < nearest_x))) {
            nearest = e;
            nearest_x = at_x;
        }
    }
    return nearest;
}

/* NULL when no icon is revealed; otherwise a stale focus has moved to the rightmost icon, the basic structure. */
static entity_t *repaired_build_icon(pad_nav_t *nav) {
    entity_t *icon = entity_get(nav->build_icon);
    if (!structure_icon_is_revealed(icon)) {
        icon = icon_beside(INT_MAX, -1);
        nav->build_icon = entity_id_link(icon);
    }
    return icon;
}

/* × presses the icon as the mouse does: the ui's dispatch, after this step, hands its structure over. */
static void build_step(pad_nav_t *nav, int pressed, int direction, int release, p8_input_frame_t *played) {
    entity_t *icon = repaired_build_icon(nav);
    if (icon != NULL && (pressed & P8_PAD_CROSS) && !release) {
        played->mouse_buttons = P8_MOUSE_LEFT;
        nav->placement = PAD_NAV_PLACEMENT_PRESSED;
    }
    if (icon == NULL || (direction != P8_PAD_LEFT && direction != P8_PAD_RIGHT)) {
        return;
    }
    int x, y;
    pad_panel_focus_point(icon, &x, &y);
    entity_t *next = icon_beside(x, direction == P8_PAD_LEFT ? -1 : 1);
    if (next != NULL) {
        nav->build_icon = entity_id_link(next);
    }
}

static void tab_cursor(pad_nav_t *nav) {
    entity_t *button = tab_button(nav->tab);
    entity_t *icon = nav->tab == PAD_NAV_TAB_BUILD ? repaired_build_icon(nav) : NULL;
    if (nav->tab == PAD_NAV_TAB_MAP) {
        entity_t *selection = pad_nav_selection(nav) != NULL ? pad_nav_selection(nav) : pad_nav_hovered(nav);
        if (selection != NULL) {
            screen_point(selection, &nav->cursor_x, &nav->cursor_y);
        } else {
            nav->cursor_x = P8_SCREEN_WIDTH / 2;
            nav->cursor_y = P8_SCREEN_HEIGHT / 2;
        }
    } else if (button != NULL) {
        pad_panel_focus_point(button, &nav->cursor_x, &nav->cursor_y);
    } else if (icon != NULL) {
        pad_panel_focus_point(icon, &nav->cursor_x, &nav->cursor_y);
    } else {
        nav->cursor_x = PARKED_X;
        nav->cursor_y = PARKED_Y;
    }
}

/* × acts only once pressed on this tab: a × held on arrival ends no year. */
static void tab_step(pad_nav_t *nav, int pad_buttons, int pressed, int direction, int release, p8_input_frame_t *played) {
    if (!(pad_buttons & P8_PAD_CROSS)) {
        nav->cross_armed = 0;
    } else if (pressed & P8_PAD_CROSS) {
        nav->cross_armed = 1;
    }
    if (pad_nav_panel_open()) {
        panel_step(nav, pad_buttons, pressed, direction, played);
        return;
    }
    nav->focus = ENTITY_ID_NONE;
    if (nav->tab == PAD_NAV_TAB_BUILD) {
        build_step(nav, pressed, direction, release, played);
    }
    tab_cursor(nav);
    if (nav->tab == PAD_NAV_TAB_END_TURN && nav->cross_armed && tab_button(nav->tab) != NULL) {
        played->mouse_buttons = P8_MOUSE_LEFT;
    }
}

/* Returns whether placement began: the press handed a structure over, so the map takes it from this frame. */
static int begin_placement(pad_nav_t *nav) {
    if (nav->placement == PAD_NAV_PLACEMENT_CARRYING && pad_nav_placing(nav) == NULL) {
        nav->placement = PAD_NAV_PLACEMENT_NONE;
    }
    if (nav->placement != PAD_NAV_PLACEMENT_PRESSED) {
        return 0;
    }
    if (held_structure() == NULL) {
        nav->placement = PAD_NAV_PLACEMENT_NONE;
        return 0;
    }
    set_tab(nav, PAD_NAV_TAB_MAP);
    nav->placement = PAD_NAV_PLACEMENT_CARRYING;
    if (pad_nav_selection(nav) != NULL) {
        enter_free_mode(nav, pad_nav_selection(nav));
    } else if (nav->map_mode == PAD_NAV_MAP_JUMP) {
        enter_plain_free_mode(nav);
    }
    return 1;
}

/* Outside the ui's panel box, where the structure's drag aims at the throwaway target and its drop is refused. */
static void cancel_point(pad_nav_t *nav) {
    entity_t *icon = entity_get(nav->build_icon);
    if (structure_icon_is_revealed(icon)) {
        pad_panel_focus_point(icon, &nav->cursor_x, &nav->cursor_y);
    } else {
        nav->cursor_x = PARKED_X;
        nav->cursor_y = PARKED_Y;
    }
    vec_t at = { p8_fixed_from_int(nav->cursor_x), p8_fixed_from_int(nav->cursor_y) };
    assert((ui_panel_box() == NULL || !box_contains(*ui_panel_box(), at)) &&
           "cancel_point: inside the ui's panel box the drop would be judged on the map, not cancelled");
}

/* The left button held is the mouse's drag; its release is the drop, where the ui runs the cart's own rules. */
static void placement_step(pad_nav_t *nav, int pad_buttons, int pressed, p8_input_frame_t *played) {
    nav->focus = ENTITY_ID_NONE;
    nav->cross_engaged = 0;
    if ((pressed & P8_PAD_CIRCLE) || pad_nav_panel_open()) {
        cancel_point(nav);
        return;
    }
    nav->cursor_x = P8_SCREEN_WIDTH / 2;
    nav->cursor_y = P8_SCREEN_HEIGHT / 2;
    free_mode_pan(nav, pad_buttons);
    if (pressed & P8_PAD_CROSS) {
        return;
    }
    played->mouse_buttons = P8_MOUSE_LEFT;
}

/* Above the TAG_NODE reservation in cart.c. */
enum { JUMP_CANDIDATES_CAP = 128 };

static void centre_world_point(int *x, int *y) {
    const camera_t *camera = main_camera_typed();
    vec_t at = camera != NULL ? camera_pos_before_shake(camera) : vec_make(0, 0);
    *x = p8_fixed_flr_int(at.x) + P8_SCREEN_WIDTH / 2;
    *y = p8_fixed_flr_int(at.y) + P8_SCREEN_HEIGHT / 2;
}

/* Points out at storage, JUMP_CANDIDATES_CAP entries the caller owns; empty when no TAG_NODE bucket exists. */
static void live_candidates(entity_t **storage, entity_list_t *out) {
    const entity_list_t *nodes = entities_by_tag(TAG_NODE);
    if (nodes == NULL) {
        *out = (entity_list_t){.items = storage, .count = 0, .capacity = JUMP_CANDIDATES_CAP};
        return;
    }
    (void)pad_jump_candidates(nodes, storage, JUMP_CANDIDATES_CAP, out);
}

/* A game stays unseeded until its first candidate: the first colony then, else the nearest; later, the nearest. */
static void seed_hovered(pad_nav_t *nav) {
    if (hovered_object(nav) != NULL) {
        return;
    }
    entity_t *storage[JUMP_CANDIDATES_CAP];
    entity_list_t candidates;
    live_candidates(storage, &candidates);
    int x, y;
    centre_world_point(&x, &y);
    entity_t *hovered = nav->jump.seeded ? pad_jump_nearest(x, y, &candidates) : pad_jump_seed(&candidates, x, y);
    nav->jump.hovered = entity_id_link(hovered);
    nav->jump.seeded |= hovered != NULL;
}

/* The rest point and the camera before shake: what the edge scroll and △ read, so neither follows the bob or a shake. */
static void rest_screen_point(const camera_t *camera, const entity_t *hovered, int *x, int *y) {
    vec_t from = camera_pos_before_shake(camera);
    pad_jump_world_point(hovered, x, y);
    *x -= p8_fixed_flr_int(from.x);
    *y -= p8_fixed_flr_int(from.y);
}

static void edge_scroll(const entity_t *hovered) {
    camera_t *camera = main_camera_typed();
    if (camera == NULL) {
        return;
    }
    int x, y, dx, dy;
    rest_screen_point(camera, hovered, &x, &y);
    pad_jump_scroll(x, y, &dx, &dy);
    if (dx != 0 || dy != 0) {
        camera_pan(camera, dx, dy);
    }
}

/* Returns whether △ clicked: never while the object is still outside the inner rectangle or under a HUD item. */
static int triangle_on_hovered(pad_nav_t *nav, const entity_t *hovered, p8_input_frame_t *played) {
    const camera_t *camera = main_camera_typed();
    int x, y;
    if (camera == NULL) {
        return 0;
    }
    rest_screen_point(camera, hovered, &x, &y);
    if (!pad_jump_inside(x, y)) {
        return 0;
    }
    screen_point(hovered, &x, &y);
    if (pressed_before(hovered, x, y)) {
        return 0;
    }
    click(nav, played);
    return 1;
}

/* hovered stays when the cone is empty. */
static void jump_from(pad_nav_t *nav, const entity_t *hovered, int direction) {
    entity_t *storage[JUMP_CANDIDATES_CAP];
    entity_list_t candidates;
    live_candidates(storage, &candidates);
    entity_t *target = pad_target_pick(hovered, direction, &candidates);
    if (target != NULL) {
        nav->jump.hovered = entity_id_link(target);
    }
}

/* × and ○ do nothing here. On a click's press and release frames no jump runs and the camera and the marker wait;
 * the hold-repeat count keeps running through them. */
static void jump_step(pad_nav_t *nav, int pad_buttons, int pressed, int release, p8_input_frame_t *played,
                      const entity_t **shown) {
    reset_free_mode(nav);
    nav->focus = ENTITY_ID_NONE;
    seed_hovered(nav);
    if (pressed & P8_PAD_SQUARE) {
        enter_plain_free_mode(nav);
        return;
    }
    int direction = pad_jump_repeat_step(&nav->jump, pad_buttons, pressed);
    entity_t *hovered = hovered_object(nav);
    int clicking = release || nav->release_pending;
    if (hovered == NULL) {
        nav->cursor_x = P8_SCREEN_WIDTH / 2;
        nav->cursor_y = P8_SCREEN_HEIGHT / 2;
        return;
    }
    if (!clicking && (pressed & P8_PAD_TRIANGLE)) {
        clicking = triangle_on_hovered(nav, hovered, played);
    }
    if (direction != 0 && !clicking) {
        jump_from(nav, hovered, direction);
        hovered = hovered_object(nav);
    }
    if (!clicking) {
        edge_scroll(hovered);
        *shown = hovered;
    }
    screen_point(hovered, &nav->cursor_x, &nav->cursor_y);
}

/* *jump_ran says whether jump mode ran this frame. */
static void map_step(pad_nav_t *nav, int pad_buttons, int pressed, int direction, int release, int circle_returns,
                     p8_input_frame_t *played, const entity_t **shown, int *jump_ran) {
    if (pad_nav_placing(nav) != NULL) {
        placement_step(nav, pad_buttons, pressed, played);
    } else if (pad_nav_panel_open()) {
        reset_free_mode(nav);
        panel_step(nav, pad_buttons, pressed, direction, played);
    } else if (pad_nav_free_mode(nav)) {
        free_mode_step(nav, pad_buttons, pressed, circle_returns, release, played);
    } else if (pad_nav_selection(nav) != NULL) {
        reset_free_mode(nav);
        network_step(nav, pad_nav_selection(nav), pressed, release, circle_returns, played);
        /* A click opens its box only in this frame's dispatch, the settled planet's on the release: neither frame shows the selection. */
        if (!release && !nav->release_pending) {
            *shown = pad_nav_selection(nav);
        }
    } else {
        assert(pad_nav_jump_mode(nav) && "map_step: drop_stale_map_mode leaves jump mode as the only other map mode");
        jump_step(nav, pad_buttons, pressed, release, played, shown);
        *jump_ran = 1;
    }
}

/* Returns whether the tab changed; *cancelled says what held thing a shoulder cancelled. */
static int switch_tab(pad_nav_t *nav, int pressed, int release, int foreign_was_open, cancelled_t *cancelled,
                      p8_input_frame_t *played) {
    *cancelled = CANCELLED_NOTHING;
    if (main_economy_typed() == NULL) {
        set_tab(nav, PAD_NAV_TAB_MAP);
        return 0;
    }
    int step = shoulder_step(pressed);
    if (step != 0 && tabs_can_switch(nav, release)) {
        *cancelled = unwind_sub_modes(nav);
        shoulder_switch(nav, step, played);
        return 1;
    }
    if (nav->tab != PAD_NAV_TAB_MAP && (pressed & P8_PAD_CIRCLE) && !foreign_was_open && !release) {
        leave_tab(nav, played);
        set_tab(nav, PAD_NAV_TAB_MAP);
        return 1;
    }
    return 0;
}

/* NULL when Start cannot open pause: no game, a panel up, a click half done, a drag's release acting next frame, a
 * fold held, or a tab whose own panel is due. */
static entity_t *pause_button_free(const pad_nav_t *nav, int release) {
    if (main_economy_typed() == NULL || pad_nav_panel_open() || release || drag_due(nav) ||
        nav->cross_engaged || opens_a_panel(nav->tab)) {
        return NULL;
    }
    return (entity_t *)galaxy_menu_find_button(4);
}

/* While the ui holds the structure, a row press would drag it onto the row and its release would drop it there. */
static void suspend_placement(pad_nav_t *nav, const entity_t *carried) {
    nav->suspended_carry = entity_id_link(carried);
    nav->placement = PAD_NAV_PLACEMENT_SUSPENDED;
    ui_set_left_held(main_ui(), NULL);
}

/* Returns whether the carry resumed on this frame. Every close of pause leaves the left button down on its frame, so
 * the carry's held button makes no press edge. */
static int resume_placement(pad_nav_t *nav) {
    if (nav->placement != PAD_NAV_PLACEMENT_SUSPENDED) {
        return 0;
    }
    entity_t *suspended = placing_structure(entity_get(nav->suspended_carry));
    if (suspended == NULL || main_economy_typed() == NULL) {
        nav->placement = PAD_NAV_PLACEMENT_NONE;
        nav->suspended_carry = ENTITY_ID_NONE;
        return 0;
    }
    if (pause_menu_shown()) {
        return 0;
    }
    ui_set_left_held(main_ui(), suspended);
    nav->placement = PAD_NAV_PLACEMENT_CARRYING;
    nav->suspended_carry = ENTITY_ID_NONE;
    return 1;
}

/* Returns whether the frame is pause's: its opening click, which needs the left button up on the frame before, or
 * its close by Start or ○, a press off every row that ui_update dispatches to nothing and that clears the box. */
static int pause_step(pad_nav_t *nav, int pressed, int release, p8_input_frame_t *played) {
    if (pause_menu_shown()) {
        if (!(pressed & (P8_PAD_START | P8_PAD_CIRCLE)) || release) {
            return 0;
        }
        nav->cursor_x = P8_SCREEN_WIDTH / 2;
        nav->cursor_y = P8_SCREEN_HEIGHT / 2;
        click(nav, played);
        return 1;
    }
    int due = nav->pause_due;
    nav->pause_due = 0;
    entity_t *button = due || (pressed & P8_PAD_START) ? pause_button_free(nav, release) : NULL;
    if (button == NULL) {
        return 0;
    }
    pad_panel_focus_point(button, &nav->cursor_x, &nav->cursor_y);
    entity_t *carried = pad_nav_placing(nav);
    if (carried != NULL) {
        suspend_placement(nav, carried);
    } else {
        cancel_drag(nav);
    }
    if (nav->left_was_down && !due) {
        nav->pause_due = 1;
    } else {
        click(nav, played);
    }
    return 1;
}

static void finish_step(pad_nav_t *nav, int release, const entity_t *shown, p8_input_frame_t *played) {
    /* The × that picks a row is still held once pause closes: on end turn it must end no year. */
    if (pause_menu_shown()) {
        nav->cross_armed = 0;
    }
    if (release) {
        played->mouse_buttons = 0;
    }
    nav->left_was_down = (played->mouse_buttons & P8_MOUSE_LEFT) != 0;
    played->mouse_x = nav->cursor_x;
    played->mouse_y = nav->cursor_y;
    show_selection(nav, shown);
    if (main_ui() != NULL) {
        ui_set_pad_tab(main_ui(), tab_button(pad_nav_tab(nav)));
        ui_set_pad_crosshair(main_ui(), pad_nav_free_mode(nav) && pad_nav_placing(nav) == NULL);
    }
}

void pad_nav_step(pad_nav_t *nav, int pad_buttons, p8_input_frame_t *played) {
    int pressed = pad_buttons & ~nav->previous_pad;
    int direction = pressed_direction(pressed);
    int release = nav->release_pending;
    nav->previous_pad = pad_buttons;
    nav->release_pending = 0;
    played->mouse_buttons = 0;
    const entity_t *shown = NULL;
    if (!pause_menu_shown()) {
        nav->pause_cross_armed = 0;
    }
    start_new_game(nav);
    drop_stale_map_mode(nav);
    int tab_changed = begin_placement(nav);
    if (resume_placement(nav)) {
        release = 0;
    }
    if (!tab_changed && pause_step(nav, pressed, release, played)) {
        pad_jump_repeat_clear(&nav->jump);
        reset_free_mode(nav);
        nav->focus = ENTITY_ID_NONE;
        finish_step(nav, release, NULL, played);
        return;
    }
    int foreign_was_open = foreign_panel_open(nav);
    /* ○ that closes a box this frame must not also reach free mode's return. */
    int circle_returns = (pressed & P8_PAD_CIRCLE) && !pad_panel_box_open();
    if (pressed & P8_PAD_CIRCLE) {
        circle_closes_the_box();
    }
    if (pad_nav_panel_open()) {
        cancel_drag(nav);
    }
    cancelled_t cancelled = CANCELLED_NOTHING;
    int jump_ran = 0;
    if (!tab_changed) {
        tab_changed = switch_tab(nav, pressed, release, foreign_was_open, &cancelled, played);
    }
    /* A leaving click and a landing click never share a frame: the landing waits for the release. */
    int clicks_due = release || nav->release_pending || cancelled;
    int landed = nav->landing_due && !clicks_due;
    if (landed) {
        nav->landing_due = 0;
        if (tab_button(nav->tab) != NULL) {
            click(nav, played);
        }
    } else if (!tab_changed && opens_a_panel(nav->tab) && !nav->landing_due && !clicks_due && !own_panel_up(nav)) {
        set_tab(nav, PAD_NAV_TAB_MAP);
        tab_changed = 1;
    }
    /* The frame that changes the tab runs no map or tab step, so one press never acts twice. */
    if (tab_changed || landed) {
        reset_free_mode(nav);
        nav->focus = ENTITY_ID_NONE;
        tab_cursor(nav);
        if (cancelled == CANCELLED_STRUCTURE) {
            cancel_point(nav);
        } else if (pad_nav_placing(nav) != NULL) {
            played->mouse_buttons = P8_MOUSE_LEFT;
        }
    } else if (nav->tab == PAD_NAV_TAB_MAP) {
        map_step(nav, pad_buttons, pressed, direction, release, circle_returns, played, &shown, &jump_ran);
    } else {
        reset_free_mode(nav);
        tab_step(nav, pad_buttons, pressed, direction, release, played);
    }
    if (!jump_ran) {
        pad_jump_repeat_clear(&nav->jump);
    }
    finish_step(nav, release, shown, played);
}
