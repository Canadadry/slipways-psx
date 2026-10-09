#ifndef GAME_PAD_NAV_H
#define GAME_PAD_NAV_H

#include "engine/entity_class.h"
#include "engine/entity_id_type.h"
#include "game/pad_jump.h"
#include "game/pad_slipway.h"
#include "p8/input.h"

typedef enum { PAD_NAV_MAP_JUMP, PAD_NAV_MAP_AIM, PAD_NAV_MAP_NETWORK, PAD_NAV_MAP_SLIPWAY } pad_nav_map_mode_t;

typedef enum {
    PAD_NAV_TAB_MAP,
    PAD_NAV_TAB_BUILD,
    PAD_NAV_TAB_END_TURN,
    PAD_NAV_TAB_TECH,
    PAD_NAV_TAB_SCORE,
    PAD_NAV_TAB_COUNT
} pad_nav_tab_t;

typedef enum {
    PAD_NAV_DRAG_NONE,
    PAD_NAV_DRAG_HELD,
    PAD_NAV_DRAG_DROPPED,
    PAD_NAV_DRAG_PROBE_DUE,
    PAD_NAV_DRAG_TRIANGLE_DUE
} pad_nav_drag_phase_t;

typedef enum {
    PAD_NAV_PLACEMENT_NONE,
    PAD_NAV_PLACEMENT_PRESSED,
    PAD_NAV_PLACEMENT_CARRYING,
    PAD_NAV_PLACEMENT_SUSPENDED
} pad_nav_placement_t;

typedef struct {
    pad_nav_drag_phase_t phase;
    entity_id_t target;
    int linked_before;
} pad_nav_drag_t;

typedef struct {
    entity_id_t focus;
    pad_nav_tab_t tab;
    entity_id_t build_icon;
    pad_nav_placement_t placement;
    entity_id_t suspended_carry;
    int landing_due;
    int cross_armed;
    int pause_due;
    int pause_cross_armed;
    pad_nav_map_mode_t map_mode;
    entity_id_t selection;
    entity_id_t selected_slipway;
    entity_id_t starting_planet;
    entity_id_t starting_slipway;
    entity_id_t last_planet;
    entity_id_t last_slipway;
    entity_id_t resume_start;
    pad_nav_drag_t drag;
    int selection_frames;
    int release_pending;
    int previous_pad;
    int chord_held;
    int left_was_down;
    int cursor_x;
    int cursor_y;
    int held_frames[4];
    int cross_engaged;
    pad_jump_t jump;
    pad_slipway_t slipway;
} pad_nav_t;

void pad_nav_init(pad_nav_t *nav);

/* Writes only the played frame's mouse_x, mouse_y and mouse_buttons, and the main ui's pad selection, pad tab and pad crosshair. */
void pad_nav_step(pad_nav_t *nav, int raw_buttons, p8_input_frame_t *played);

/* NULL when no panel is open, the panel has no item, or the focus is no longer one of its items. */
entity_t *pad_nav_focus(const pad_nav_t *nav);

int pad_nav_panel_open(void);

/* PAD_NAV_TAB_MAP whenever no game is running. */
pad_nav_tab_t pad_nav_tab(const pad_nav_t *nav);

/* A game is running on the map tab, no panel is open and the map mode is aim: the cursor is the screen centre and the D-pad pans the camera. */
int pad_nav_aim_mode(const pad_nav_t *nav);

/* A game is running on the map tab, no panel is open and the map mode is jump: the cursor sits on the hovered object. */
int pad_nav_jump_mode(const pad_nav_t *nav);

/* The hovered object while the map mode is jump, panel open or not; NULL in any other mode, when nothing is hovered,
 * or once the hovered object is no longer a pad_target_is_candidate. */
entity_t *pad_nav_hovered(const pad_nav_t *nav);

/* The slipway's start while the map mode is slipway; NULL in any other mode or once the start is dead. */
entity_t *pad_nav_slipway_start(const pad_nav_t *nav);

/* The aimed target while the map mode is slipway; NULL while aimed at the start, in any other mode, or once the
 * target is no longer a pad_target_is_candidate. */
entity_t *pad_nav_slipway_target(const pad_nav_t *nav);

/* The focused structure icon on the build tab; NULL on every other tab or once it is no longer a revealed icon. */
entity_t *pad_nav_build_icon(const pad_nav_t *nav);

/* The structure the ui holds in its placing state while placement carries it; NULL otherwise. */
entity_t *pad_nav_placing(const pad_nav_t *nav);

/* The selected planet in network mode, panel open or not; NULL in aim mode or once that planet is gone or dead. */
entity_t *pad_nav_selection(const pad_nav_t *nav);

/* The planet □ left network mode from, in aim mode, panel open or not; NULL in network mode, at game start, after a
 * second □ (plain aim mode), or once that planet is gone or dead. */
entity_t *pad_nav_starting_planet(const pad_nav_t *nav);

/* NULL outside network mode or when the selected planet has no slipway; never a slipway the planet does not hold. */
entity_t *pad_nav_selected_slipway(const pad_nav_t *nav);

/* The planet's slipways turn clockwise on screen from straight up: step 1 gives the one after current, -1 the one
 * before, 0 current itself. The first from straight up when current is not one of them; NULL when the planet has none. */
entity_t *pad_nav_next_slipway(const entity_t *planet, const entity_t *current, int step);

/* NULL when no panel is open or the panel has no item. */
entity_t *pad_nav_first_item(void);

/* NULL when no item lies in that direction. */
entity_t *pad_nav_neighbor(const entity_t *item, int direction);

int pad_nav_is_item(const entity_t *e);

#endif
