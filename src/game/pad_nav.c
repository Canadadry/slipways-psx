#include "game/pad_nav.h"

#include "engine/entity_id.h"
#include "engine/entity_registry.h"
#include "engine/frame_zone.h"
#include "engine/geom.h"
#include "game/background.h"
#include "game/main_economy.h"
#include "game/menu.h"
#include "game/message_box.h"
#include "game/pad_build.h"
#include "game/pad_jump.h"
#include "game/pad_panel.h"
#include "game/pad_select.h"
#include "game/pad_target.h"
#include "game/planet.h"
#include "game/scroll.h"
#include "game/slipway.h"
#include "game/ui.h"
#include "p8/vram.h"

#include <assert.h>

int pad_nav_panel_open(void) {
    return pad_panel_open();
}

pad_nav_tab_t pad_nav_tab(const pad_nav_t *nav) {
    return main_economy_typed() != NULL ? nav->tab : PAD_NAV_TAB_MAP;
}

entity_t *pad_nav_last_acted(const pad_nav_t *nav) {
    return main_economy_typed() != NULL ? pad_back_target(&nav->back, NULL) : NULL;
}

int pad_nav_select_held(const pad_nav_t *nav) {
    return nav->select.held_frames;
}

int pad_nav_year_chain_open(const pad_nav_t *nav) {
    return nav->select.chain_frames > 0;
}

int pad_nav_aim_mode(const pad_nav_t *nav) {
    return main_economy_typed() != NULL && nav->tab == PAD_NAV_TAB_MAP && !pad_nav_panel_open() &&
           nav->map_mode == PAD_NAV_MAP_AIM;
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

entity_t *pad_nav_slipway_start(const pad_nav_t *nav) {
    return nav->map_mode == PAD_NAV_MAP_SLIPWAY ? live(entity_get(nav->slipway.start)) : NULL;
}

entity_t *pad_nav_slipway_target(const pad_nav_t *nav) {
    entity_t *target = nav->map_mode == PAD_NAV_MAP_SLIPWAY ? entity_get(nav->slipway.target) : NULL;
    return pad_target_is_candidate(target) ? target : NULL;
}

int pad_nav_build_bar_open(const pad_nav_t *nav) {
    return pad_nav_tab(nav) == PAD_NAV_TAB_BUILD;
}

int pad_nav_tech_open(const pad_nav_t *nav) {
    return pad_nav_tab(nav) == PAD_NAV_TAB_TECH;
}

int pad_nav_score_open(const pad_nav_t *nav) {
    return pad_nav_tab(nav) == PAD_NAV_TAB_SCORE;
}

entity_t *pad_nav_build_icon(const pad_nav_t *nav) {
    return pad_nav_tab(nav) == PAD_NAV_TAB_BUILD ? pad_build_icon(&nav->build) : NULL;
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

static void reset_aim_mode(pad_nav_t *nav) {
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

static void aim_pan(pad_nav_t *nav, int pad_buttons) {
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

/* Keeps the jump state's game and seeded flag: a reset within a game reseeds from the centre, never on the first colony. */
static void reset_map_mode(pad_nav_t *nav) {
    nav->map_mode = PAD_NAV_MAP_JUMP;
    nav->jump.hovered = ENTITY_ID_NONE;
    pad_jump_repeat_clear(&nav->jump);
    nav->marker_frames = 0;
    nav->placement = PAD_NAV_PLACEMENT_NONE;
    nav->suspended_carry = ENTITY_ID_NONE;
    pad_slipway_init(&nav->slipway);
    pad_select_init(&nav->select);
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
    pad_build_init(&nav->build);
    set_tab(nav, PAD_NAV_TAB_MAP);
    pad_jump_init(&nav->jump);
    pad_back_init(&nav->back);
    reset_map_mode(nav);
    nav->release_pending = 0;
    nav->pause_due = 0;
    nav->pause_cross_armed = 0;
    nav->menu_cross_armed = 0;
    nav->previous_pad = 0;
    nav->chord_held = 0;
    nav->left_was_down = 0;
    nav->cursor_x = 0;
    nav->cursor_y = 0;
    reset_aim_mode(nav);
}

entity_t *pad_nav_focus(const pad_nav_t *nav) {
    entity_t *focus = entity_get(nav->focus);
    return pad_nav_is_item(focus) ? focus : NULL;
}

static int planet_menu_open(const pad_nav_t *nav) {
    return nav->tab == PAD_NAV_TAB_MAP && pad_panel_box_open() && !pause_menu_shown();
}

/* A planet menu or the final score panel on the map, or the tech or score tab's panel: none opens on ×. */
static int armed_panel_open(const pad_nav_t *nav) {
    return (nav->tab == PAD_NAV_TAB_MAP || opens_a_panel(nav->tab)) && pad_nav_panel_open() && !pause_menu_shown();
}

/* A × held into pause or into a panel, or held from the click that opened a planet menu, presses no row until
 * pressed again: pad_panel_step holds the left button while × is held. */
static void panel_step(pad_nav_t *nav, int pad_buttons, int pressed, int direction, p8_input_frame_t *played) {
    if (pause_menu_shown()) {
        nav->pause_cross_armed |= (pressed & P8_PAD_CROSS) != 0;
        if (!nav->pause_cross_armed) {
            pad_buttons &= ~P8_PAD_CROSS;
        }
    } else if (armed_panel_open(nav)) {
        nav->menu_cross_armed |= (pressed & P8_PAD_CROSS) != 0;
        if (!nav->menu_cross_armed) {
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

/* A press at the cursor on this frame and a release on the next. */
static void click(pad_nav_t *nav, p8_input_frame_t *played) {
    played->mouse_buttons = P8_MOUSE_LEFT;
    nav->release_pending = 1;
}

/* Moves no camera: a snap here would shift every later point action of a replay off its reference world point. */
static void enter_aim_mode(pad_nav_t *nav) {
    nav->map_mode = PAD_NAV_MAP_AIM;
    nav->cursor_x = P8_SCREEN_WIDTH / 2;
    nav->cursor_y = P8_SCREEN_HEIGHT / 2;
    pad_jump_repeat_clear(&nav->jump);
}

/* The ui presses the highest layer first: a HUD item over the planet would take the press. */
static int pressed_before(const entity_t *planet, int x, int y) {
    return pad_panel_entity_under(x, y, p8_fixed_flr_int(planet->layer), pad_panel_is_candidate) != NULL;
}

/* NULL unless the ui holds a live slipway. */
static entity_t *held_slipway(void) {
    entity_t *held = main_ui() != NULL ? live(ui_left_held(main_ui())) : NULL;
    return held != NULL && held->class == slipway_class() ? held : NULL;
}

static void drop_held_slipway(void) {
    entity_t *held = held_slipway();
    if (held != NULL) {
        held->class->drop(held, NULL);
        ui_set_left_held(main_ui(), NULL);
    }
}

static void centre_world_point(int *x, int *y) {
    const camera_t *camera = main_camera_typed();
    vec_t at = camera != NULL ? camera_pos_before_shake(camera) : vec_make(0, 0);
    *x = p8_fixed_flr_int(at.x) + P8_SCREEN_WIDTH / 2;
    *y = p8_fixed_flr_int(at.y) + P8_SCREEN_HEIGHT / 2;
}

static void edge_scroll(const entity_t *hovered);

static entity_list_t live_candidates(void) {
    return pad_jump_candidates(frame_zone(), entities_by_tag(TAG_NODE));
}

/* Jump mode hovering e, or nothing when e is no candidate: the jump mode reseed then runs. */
static void back_to_jump_mode(pad_nav_t *nav, const entity_t *e) {
    nav->map_mode = PAD_NAV_MAP_JUMP;
    nav->jump.hovered = pad_target_is_candidate(e) ? entity_id_link(e) : ENTITY_ID_NONE;
    pad_slipway_init(&nav->slipway);
}

/* Jump mode on the target nearest the centre, or on nothing when the game has none: seeded, so jump mode never
 * reseeds on the first colony afterwards. */
static void leave_aim_mode(pad_nav_t *nav) {
    entity_list_t candidates = live_candidates();
    int x, y;
    centre_world_point(&x, &y);
    entity_t *nearest = pad_jump_nearest(x, y, &candidates);
    nav->map_mode = PAD_NAV_MAP_JUMP;
    nav->jump.hovered = entity_id_link(nearest);
    nav->jump.seeded |= nearest != NULL;
    pad_jump_repeat_clear(&nav->jump);
}

/* Returns whether a slipway was held, and so dropped: nothing built or charged, jump mode back on the start. */
static int cancel_slipway(pad_nav_t *nav) {
    if (nav->map_mode != PAD_NAV_MAP_SLIPWAY) {
        return 0;
    }
    int held = held_slipway() != NULL;
    drop_held_slipway();
    back_to_jump_mode(nav, pad_nav_slipway_start(nav));
    return held;
}

/* Returns whether the tap jumped: to the object needing attention after current, as a D-pad jump that landed there. */
static int select_tap(pad_nav_t *nav, const entity_t *current, const entity_t **shown) {
    entity_list_t candidates = live_candidates();
    entity_t *next = pad_select_next(&candidates, current);
    int jumps = next != NULL && next != current;
    if (jumps) {
        back_to_jump_mode(nav, next);
        nav->jump.seeded = 1;
        pad_jump_repeat_clear(&nav->jump);
        edge_scroll(next);
        *shown = next;
        screen_point(next, &nav->cursor_x, &nav->cursor_y);
    }
    return jumps;
}

/* On a click's release frame × starts nothing: its press would be forced up. △ does nothing here. */
static void aim_mode_step(pad_nav_t *nav, int pad_buttons, int pressed, int circle_returns, int release, int still,
                          int select_tapped, p8_input_frame_t *played, const entity_t **shown) {
    nav->focus = ENTITY_ID_NONE;
    nav->cursor_x = P8_SCREEN_WIDTH / 2;
    nav->cursor_y = P8_SCREEN_HEIGHT / 2;
    if (!still) {
        aim_pan(nav, pad_buttons);
    }
    if (select_tapped && !nav->cross_engaged && select_tap(nav, NULL, shown)) {
        return;
    }
    if ((pressed & P8_PAD_SQUARE) && !nav->cross_engaged) {
        leave_aim_mode(nav);
        return;
    }
    if (!(pad_buttons & P8_PAD_CROSS)) {
        nav->cross_engaged = 0;
    } else if ((pressed & P8_PAD_CROSS) && !release) {
        nav->cross_engaged = entity_under(nav->cursor_x, nav->cursor_y, presses_off_background) == NULL;
    }
    if (nav->cross_engaged) {
        played->mouse_buttons = P8_MOUSE_LEFT;
    } else if (circle_returns) {
        leave_aim_mode(nav);
    }
}

/* The title and a new game rebuild the id table above every old generation, so the old game's economy id resolves
 * to NULL: no title frame is needed in between. */
static void start_new_game(pad_nav_t *nav) {
    entity_t *economy = main_economy_typed() != NULL ? &main_economy_typed()->header : NULL;
    if (economy == NULL || entity_get(nav->jump.game) == economy) {
        return;
    }
    pad_jump_init(&nav->jump);
    pad_back_init(&nav->back);
    nav->jump.game = entity_id_link(economy);
    reset_map_mode(nav);
}

/* The title's difficulty rows are no box, so ○ cannot close them. Pause closes only by pause_step's press: cleared
 * here, on a release frame, the carry's resume would press on the map. */
static void circle_closes_the_box(void) {
    if (pad_panel_box_open() && !pause_menu_shown()) {
        ui_clear_ephemerals(main_ui());
    }
}

static void show_marker(pad_nav_t *nav, const entity_t *shown) {
    ui_t *ui = main_ui();
    if (shown == NULL) {
        nav->marker_frames = 0;
    }
    if (ui != NULL) {
        ui_set_pad_selection(ui, shown, nav->marker_frames);
    }
    if (shown != NULL) {
        nav->marker_frames = (nav->marker_frames + 1) % UI_PAD_BLINK_PERIOD;
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

/* A slipway released on this frame is judged on the next: tabs and pause wait for it. */
static int map_action_due(const pad_nav_t *nav) {
    return nav->map_mode == PAD_NAV_MAP_SLIPWAY && nav->slipway.phase == PAD_SLIPWAY_RELEASED;
}

static int tabs_can_switch(const pad_nav_t *nav, int release) {
    if ((foreign_panel_open(nav) && !planet_menu_open(nav)) || release || nav->landing_due) {
        return 0;
    }
    return nav->tab != PAD_NAV_TAB_MAP || !map_action_due(nav);
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
    if (pad_nav_placing(nav) != NULL) {
        return CANCELLED_STRUCTURE;
    }
    return cancel_slipway(nav) ? CANCELLED_SLIPWAY : CANCELLED_NOTHING;
}

/* The score backdrop hit-tests everywhere above the HUD, so a press anywhere reaches it and its on_click hides the panel.
 * L1 on score clicks with the cursor already on the bar's icon: the backdrop takes that press, never the icon. */
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

/* PAD_NAV_TAB_COUNT for none: R1 alone moves round the ring, L1 alone toggles the bar, both together do nothing. */
static pad_nav_tab_t shoulder_target(pad_nav_tab_t tab, int pressed) {
    int r1 = (pressed & P8_PAD_R1) != 0, l1 = (pressed & P8_PAD_L1) != 0;
    if (r1 == l1) {
        return PAD_NAV_TAB_COUNT;
    }
    if (r1) {
        return tab == PAD_NAV_TAB_MAP ? PAD_NAV_TAB_TECH : (pad_nav_tab_t)((tab + 1) % PAD_NAV_TAB_COUNT);
    }
    return tab == PAD_NAV_TAB_BUILD ? PAD_NAV_TAB_MAP : PAD_NAV_TAB_BUILD;
}

static void shoulder_switch(pad_nav_t *nav, pad_nav_tab_t target, p8_input_frame_t *played) {
    leave_tab(nav, played);
    set_tab(nav, target);
}

/* Above and left of the screen: no left_press there that × could reach. */
enum { PARKED_X = -16, PARKED_Y = -16 };

/* NULL on map and build, or once that button is gone. */
static entity_t *tab_button(pad_nav_tab_t tab) {
    static const int hud_button[PAD_NAV_TAB_COUNT] = { [PAD_NAV_TAB_END_TURN] = 1, [PAD_NAV_TAB_TECH] = 2,
                                                        [PAD_NAV_TAB_SCORE] = 3 };
    return hud_button[tab] != 0 ? (entity_t *)galaxy_menu_find_button(hud_button[tab]) : NULL;
}

/* × presses the icon as the mouse does: the ui's dispatch, after this step, hands its structure over. */
static void build_step(pad_nav_t *nav, int pressed, int direction, int release, p8_input_frame_t *played) {
    entity_t *icon = pad_build_focus(&nav->build);
    if (icon != NULL && (pressed & P8_PAD_CROSS) && !release) {
        played->mouse_buttons = P8_MOUSE_LEFT;
        nav->placement = PAD_NAV_PLACEMENT_PRESSED;
    }
    if (direction == P8_PAD_LEFT || direction == P8_PAD_RIGHT) {
        pad_build_pick(&nav->build, direction == P8_PAD_LEFT ? -1 : 1);
    }
}

static void tab_cursor(pad_nav_t *nav) {
    entity_t *button = tab_button(nav->tab);
    entity_t *icon = nav->tab == PAD_NAV_TAB_BUILD ? pad_build_focus(&nav->build) : NULL;
    if (nav->tab == PAD_NAV_TAB_MAP) {
        entity_t *hovered = pad_nav_hovered(nav);
        if (hovered != NULL) {
            screen_point(hovered, &nav->cursor_x, &nav->cursor_y);
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
    if (nav->map_mode == PAD_NAV_MAP_JUMP) {
        enter_aim_mode(nav);
    }
    return 1;
}

/* Outside the ui's panel box, where the structure's drag aims at the throwaway target and its drop is refused. */
static void cancel_point(pad_nav_t *nav) {
    entity_t *icon = pad_build_icon(&nav->build);
    if (icon != NULL) {
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
    aim_pan(nav, pad_buttons);
    if (pressed & P8_PAD_CROSS) {
        return;
    }
    played->mouse_buttons = P8_MOUSE_LEFT;
}

/* A game stays unseeded until its first candidate: the first colony then, else the nearest; later, the nearest. */
static void seed_hovered(pad_nav_t *nav) {
    if (hovered_object(nav) != NULL) {
        return;
    }
    entity_list_t candidates = live_candidates();
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

/* Never while the object is still outside the inner rectangle or under a HUD item. */
static int pressable(const entity_t *e) {
    const camera_t *camera = main_camera_typed();
    int x, y;
    if (camera == NULL) {
        return 0;
    }
    rest_screen_point(camera, e, &x, &y);
    if (!pad_jump_inside(x, y)) {
        return 0;
    }
    screen_point(e, &x, &y);
    return !pressed_before(e, x, y);
}

/* △'s click, and ×'s on a known planet: the press opens its menu, which no × held from here presses. Returns whether it
 * clicked. */
static int click_hovered(pad_nav_t *nav, const entity_t *hovered, p8_input_frame_t *played) {
    if (!pressable(hovered)) {
        return 0;
    }
    click(nav, played);
    nav->menu_cross_armed = 0;
    return 1;
}

/* Returns whether × pressed the hovered object, as the mouse does: its left press hands the ui a slipway, held from
 * the next frame. A refused start presses nothing, and no later frame retries it. */
static int cross_starts_slipway(pad_nav_t *nav, const entity_t *hovered, p8_input_frame_t *played) {
    if (!pad_slipway_can_start(hovered) || !pressable(hovered) || nav->left_was_down) {
        return 0;
    }
    screen_point(hovered, &nav->cursor_x, &nav->cursor_y);
    played->mouse_buttons = P8_MOUSE_LEFT;
    nav->map_mode = PAD_NAV_MAP_SLIPWAY;
    pad_slipway_init(&nav->slipway);
    nav->slipway.start = entity_id_link(hovered);
    nav->slipway.phase = PAD_SLIPWAY_PRESSED;
    return 1;
}

/* hovered stays when the cone is empty. */
static void jump_from(pad_nav_t *nav, const entity_t *hovered, int direction) {
    entity_list_t candidates = live_candidates();
    entity_t *target = pad_target_pick(hovered, direction, &candidates);
    if (target != NULL) {
        nav->jump.hovered = entity_id_link(target);
    }
}

/* On a click's press and release frames no jump runs, ○'s included, and the camera and the marker wait; the
 * hold-repeat count keeps running through them. ○ jumps to the last object × acted on and wins over the D-pad. */
static void jump_step(pad_nav_t *nav, int pad_buttons, int pressed, int release, int circle_returns,
                      int select_tapped, p8_input_frame_t *played, const entity_t **shown) {
    reset_aim_mode(nav);
    nav->focus = ENTITY_ID_NONE;
    seed_hovered(nav);
    if (pressed & P8_PAD_SQUARE) {
        enter_aim_mode(nav);
        return;
    }
    int direction = pad_jump_repeat_step(&nav->jump, pad_buttons, pressed);
    entity_t *hovered = hovered_object(nav);
    int clicking = release || nav->release_pending;
    entity_t *back = circle_returns && !clicking ? pad_back_target(&nav->back, hovered) : NULL;
    if (back != NULL) {
        nav->jump.hovered = entity_id_link(back);
        edge_scroll(back);
        *shown = back;
        screen_point(back, &nav->cursor_x, &nav->cursor_y);
        return;
    }
    if (hovered == NULL) {
        nav->cursor_x = P8_SCREEN_WIDTH / 2;
        nav->cursor_y = P8_SCREEN_HEIGHT / 2;
        return;
    }
    if (!clicking && (pressed & P8_PAD_TRIANGLE)) {
        clicking = click_hovered(nav, hovered, played);
    }
    if (!clicking && (pressed & P8_PAD_CROSS) && hovered->state == ENTITY_STATE_KNOWN) {
        clicking = click_hovered(nav, hovered, played);
        if (clicking) {
            pad_back_record(&nav->back, hovered);
        }
    }
    if (!clicking && (pressed & P8_PAD_CROSS) && cross_starts_slipway(nav, hovered, played)) {
        pad_back_record(&nav->back, hovered);
        return;
    }
    if (!clicking && select_tapped && select_tap(nav, hovered, shown)) {
        return;
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

/* The re-press after a release, from the start the outcome left: its screen point may lie off screen. Returns whether
 * it went out. */
static int press_start_again(pad_nav_t *nav, const entity_t *start, p8_input_frame_t *played) {
    if (!pad_slipway_can_start(start)) {
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
    return 1;
}

/* The left button held on the aimed object is the mouse's drag, so the game draws its own line; × lets it go on the
 * target, where the ui's drop builds or refuses. The press, release and re-press frames show no marker. */
static void slipway_step(pad_nav_t *nav, int pad_buttons, int pressed, int circle_returns, int release,
                         p8_input_frame_t *played, const entity_t **shown) {
    reset_aim_mode(nav);
    nav->focus = ENTITY_ID_NONE;
    entity_t *start = pad_nav_slipway_start(nav);
    if (start == NULL || !planet_is_family(start)) {
        drop_held_slipway();
        back_to_jump_mode(nav, pad_nav_slipway_target(nav));
        return;
    }
    if (nav->slipway.phase == PAD_SLIPWAY_RELEASED) {
        pad_slipway_outcome_t outcome =
            pad_slipway_outcome(start, entity_get(nav->slipway.target), nav->slipway.linked_before);
        if (outcome == PAD_SLIPWAY_BUILT) {
            pad_back_record(&nav->back, entity_get(nav->slipway.target));
        }
        pad_slipway_rebase(&nav->slipway, outcome);
        start = live(entity_get(nav->slipway.start));
        if (!press_start_again(nav, start, played)) {
            back_to_jump_mode(nav, start);
        }
        return;
    }
    if (nav->slipway.phase == PAD_SLIPWAY_PRESSED) {
        if (held_slipway() == NULL) {
            back_to_jump_mode(nav, start);
            return;
        }
        nav->slipway.phase = PAD_SLIPWAY_HELD;
    }
    if (circle_returns) {
        drop_held_slipway();
        back_to_jump_mode(nav, start);
        screen_point(start, &nav->cursor_x, &nav->cursor_y);
        return;
    }
    int direction = pad_jump_repeat_step(&nav->jump, pad_buttons, pressed);
    entity_t *target = pad_nav_slipway_target(nav);
    nav->slipway.target = entity_id_link(target);
    if ((pressed & P8_PAD_CROSS) && !release && target != NULL && pressable(target)) {
        pad_slipway_release(&nav->slipway, pad_slipway_between(start, target) != NULL);
        screen_point(target, &nav->cursor_x, &nav->cursor_y);
        return;
    }
    if (direction != 0 && !(pressed & P8_PAD_CROSS)) {
        entity_list_t candidates = live_candidates();
        entity_list_t targets = pad_slipway_candidates(frame_zone(), &candidates, start);
        entity_t *picked = pad_target_pick(target != NULL ? target : start, direction, &targets);
        if (picked != NULL) {
            target = picked;
            nav->slipway.target = entity_id_link(target);
        }
    }
    const entity_t *aimed = target != NULL ? target : start;
    edge_scroll(aimed);
    screen_point(aimed, &nav->cursor_x, &nav->cursor_y);
    played->mouse_buttons = P8_MOUSE_LEFT;
    *shown = aimed;
}

/* *jump_ran says whether jump mode ran this frame. */
static void map_step(pad_nav_t *nav, int pad_buttons, int pressed, int direction, int release, int circle_returns,
                     pad_select_event_t select, int year_release, p8_input_frame_t *played, const entity_t **shown,
                     int *jump_ran) {
    if (pad_nav_placing(nav) != NULL) {
        placement_step(nav, pad_buttons, pressed, played);
    } else if (pad_nav_panel_open()) {
        reset_aim_mode(nav);
        /* On the map tab only jump mode's click opens an ephemeral box: a later one that is no planet menu must keep △
         * from closing it here. */
        if ((pressed & P8_PAD_TRIANGLE) && !release && !nav->release_pending && planet_menu_open(nav)) {
            ui_clear_ephemerals(main_ui());
        } else {
            panel_step(nav, pad_buttons, pressed, direction, played);
        }
    } else if (pad_nav_aim_mode(nav)) {
        aim_mode_step(nav, pad_buttons, pressed, circle_returns, release, year_release, select == PAD_SELECT_TAP, played,
                      shown);
    } else if (nav->map_mode == PAD_NAV_MAP_SLIPWAY) {
        slipway_step(nav, pad_buttons, pressed, circle_returns, release, played, shown);
        *jump_ran = 1;
    } else {
        assert(pad_nav_jump_mode(nav) &&
               "map_step: after placement, panels, aim and slipway mode, only jump mode is left");
        jump_step(nav, pad_buttons, pressed, release, circle_returns, select == PAD_SELECT_TAP, played, shown);
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
    pad_nav_tab_t target = shoulder_target(nav->tab, pressed);
    if (target != PAD_NAV_TAB_COUNT && tabs_can_switch(nav, release)) {
        *cancelled = unwind_sub_modes(nav);
        shoulder_switch(nav, target, played);
        return 1;
    }
    if (nav->tab != PAD_NAV_TAB_MAP && (pressed & P8_PAD_CIRCLE) && !foreign_was_open && !release) {
        leave_tab(nav, played);
        set_tab(nav, PAD_NAV_TAB_MAP);
        return 1;
    }
    return 0;
}

/* NULL when Start cannot open pause: no game, a panel up, a click half done, a slipway's release acting next
 * frame, a fold held, or a tab whose own panel is due. */
static entity_t *pause_button_free(const pad_nav_t *nav, int release) {
    if (main_economy_typed() == NULL || pad_nav_panel_open() || release || map_action_due(nav) ||
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
        (void)cancel_slipway(nav);
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
    show_marker(nav, shown);
    if (main_ui() != NULL) {
        ui_set_pad_tab(main_ui(), tab_button(pad_nav_tab(nav)));
        ui_set_pad_crosshair(main_ui(), pad_nav_aim_mode(nav) && pad_nav_placing(nav) == NULL);
    }
}

/* A shown message box blocks nothing: the year box stays up through the next year. */
static int select_allowed(const pad_nav_t *nav, int tab_changing, int release) {
    return main_economy_typed() != NULL && nav->tab == PAD_NAV_TAB_MAP && !tab_changing && !pause_menu_shown() &&
           !pad_nav_panel_open() && (nav->map_mode == PAD_NAV_MAP_JUMP || nav->map_mode == PAD_NAV_MAP_AIM) &&
           nav->placement == PAD_NAV_PLACEMENT_NONE && !nav->cross_engaged && !release && !nav->release_pending;
}

/* Returns whether the year press went out: the HUD year-skip button's, as × on end turn makes it. A refused press is
 * never retried. */
static int select_ends_year(pad_nav_t *nav, p8_input_frame_t *played) {
    entity_t *button = tab_button(PAD_NAV_TAB_END_TURN);
    if (button == NULL || nav->left_was_down) {
        return 0;
    }
    pad_panel_focus_point(button, &nav->cursor_x, &nav->cursor_y);
    click(nav, played);
    pad_select_year_ended(&nav->select);
    return 1;
}

/* × and ○ going down on the same frame are a cheat chord: no mode sees either until both are up again. */
static int without_chord(pad_nav_t *nav, int raw_buttons) {
    const int face = P8_PAD_CROSS | P8_PAD_CIRCLE;
    if ((raw_buttons & ~nav->previous_pad & face) == face) {
        nav->chord_held = 1;
    } else if (!(raw_buttons & face)) {
        nav->chord_held = 0;
    }
    return nav->chord_held ? raw_buttons & ~face : raw_buttons;
}

void pad_nav_step(pad_nav_t *nav, int raw_buttons, p8_input_frame_t *played) {
    int pad_buttons = without_chord(nav, raw_buttons);
    int pressed = pad_buttons & ~nav->previous_pad;
    int direction = pressed_direction(pressed);
    int release = nav->release_pending;
    nav->previous_pad = raw_buttons;
    nav->release_pending = 0;
    played->mouse_buttons = 0;
    const entity_t *shown = NULL;
    if (!pause_menu_shown()) {
        nav->pause_cross_armed = 0;
    }
    if (!armed_panel_open(nav)) {
        nav->menu_cross_armed = 0;
    }
    start_new_game(nav);
    int tab_changed = begin_placement(nav);
    if (resume_placement(nav)) {
        release = 0;
    }
    int select_down = (pad_buttons & P8_PAD_SELECT) != 0;
    if (!tab_changed && pause_step(nav, pressed, release, played)) {
        (void)pad_select_step(&nav->select, select_down, 0);
        pad_jump_repeat_clear(&nav->jump);
        reset_aim_mode(nav);
        nav->focus = ENTITY_ID_NONE;
        finish_step(nav, release, NULL, played);
        return;
    }
    int foreign_was_open = foreign_panel_open(nav);
    /* ○ that closes a box this frame must not also reach aim mode's return. */
    int circle_returns = (pressed & P8_PAD_CIRCLE) && !pad_panel_box_open();
    if (pressed & P8_PAD_CIRCLE) {
        circle_closes_the_box();
        if (circle_returns && nav->tab == PAD_NAV_TAB_MAP && shown_message_box() != NULL) {
            message_box_close_shown();
            circle_returns = 0;
            pressed &= ~P8_PAD_CIRCLE;
        }
    }
    if (pad_nav_panel_open()) {
        (void)cancel_slipway(nav);
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
    int year_release = pad_select_year_just_ended(&nav->select);
    pad_select_event_t select =
        pad_select_step(&nav->select, select_down, select_allowed(nav, tab_changed || landed, release));
    /* The frame that changes the tab runs no map or tab step, so one press never acts twice. */
    if (tab_changed || landed) {
        reset_aim_mode(nav);
        nav->focus = ENTITY_ID_NONE;
        tab_cursor(nav);
        if (cancelled == CANCELLED_STRUCTURE) {
            cancel_point(nav);
        } else if (pad_nav_placing(nav) != NULL) {
            played->mouse_buttons = P8_MOUSE_LEFT;
        }
    } else if (select == PAD_SELECT_YEAR && select_ends_year(nav, played)) {
        reset_aim_mode(nav);
        nav->focus = ENTITY_ID_NONE;
    } else if (nav->tab == PAD_NAV_TAB_MAP) {
        map_step(nav, pad_buttons, pressed, direction, release, circle_returns, select, year_release, played, &shown,
                 &jump_ran);
    } else {
        reset_aim_mode(nav);
        tab_step(nav, pad_buttons, pressed, direction, release, played);
    }
    if (!jump_ran) {
        pad_jump_repeat_clear(&nav->jump);
    }
    finish_step(nav, release, shown, played);
}
