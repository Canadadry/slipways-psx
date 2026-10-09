#ifndef GAME_PAD_NAV_H
#define GAME_PAD_NAV_H

#include "engine/entity_class.h"
#include "engine/entity_id_type.h"
#include "game/pad_back.h"
#include "game/pad_build.h"
#include "game/pad_cycle.h"
#include "game/pad_jump.h"
#include "game/pad_select.h"
#include "game/pad_slipway.h"
#include "p8/input.h"

typedef enum { PAD_NAV_MAP_JUMP, PAD_NAV_MAP_AIM, PAD_NAV_MAP_SLIPWAY } pad_nav_map_mode_t;

typedef enum {
    PAD_NAV_PLACEMENT_NONE,
    PAD_NAV_PLACEMENT_PRESSED,
    PAD_NAV_PLACEMENT_CARRYING,
    PAD_NAV_PLACEMENT_SUSPENDED
} pad_nav_placement_t;

typedef struct {
    entity_id_t focus;
    pad_build_t build;
    pad_cycle_t cycle;
    pad_nav_placement_t placement;
    entity_id_t suspended_carry;
    int pause_due;
    int pause_cross_armed;
    int menu_cross_armed;
    pad_nav_map_mode_t map_mode;
    int marker_frames;
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
    pad_back_t back;
    pad_select_t select;
} pad_nav_t;

void pad_nav_init(pad_nav_t *nav);

/* Writes only the played frame's mouse_x, mouse_y and mouse_buttons, and the main ui's pad selection, pad crosshair and
 * pad tab: the open cycle panel's HUD button (tech or score), from the R1 frame that opens it until it closes; NULL with
 * no game or no cycle panel open, the bar included. */
void pad_nav_step(pad_nav_t *nav, int raw_buttons, p8_input_frame_t *played);

/* NULL when no panel is open, the panel has no item, or the focus is no longer one of its items. */
entity_t *pad_nav_focus(const pad_nav_t *nav);

int pad_nav_panel_open(void);

/* A game is running on the map (the bar closed, no cycle panel open), no panel is open and the map mode is aim: the cursor is the screen centre and the D-pad pans the camera. */
int pad_nav_aim_mode(const pad_nav_t *nav);

/* A game is running on the map (the bar closed, no cycle panel open), no panel is open and the map mode is jump: the cursor sits on the hovered object. */
int pad_nav_jump_mode(const pad_nav_t *nav);

/* The hovered object while the map mode is jump, panel open or not; NULL in any other mode, when nothing is hovered,
 * or once the hovered object is no longer a pad_target_is_candidate. */
entity_t *pad_nav_hovered(const pad_nav_t *nav);

/* The slipway's start while the map mode is slipway; NULL in any other mode or once the start is dead. */
entity_t *pad_nav_slipway_start(const pad_nav_t *nav);

/* The last object × acted on in this game (a known planet whose menu it opened, a slipway's start or built target),
 * whatever the map mode; NULL with no game, when nothing was recorded, or once it is no longer a
 * pad_target_is_candidate. */
entity_t *pad_nav_last_acted(const pad_nav_t *nav);

/* The frames the current Select press has been down; 0 when Select is up, and from a blocked frame until its release. */
int pad_nav_select_held(const pad_nav_t *nav);

/* 1 while a fresh Select press would end one more year on its press frame. */
int pad_nav_year_chain_open(const pad_nav_t *nav);

/* The aimed target while the map mode is slipway; NULL while aimed at the start, in any other mode, or once the
 * target is no longer a pad_target_is_candidate. */
entity_t *pad_nav_slipway_target(const pad_nav_t *nav);

/* 1 while a game runs and the build bar is open; 0 with no game. */
int pad_nav_build_bar_open(const pad_nav_t *nav);

/* 1 while a game runs, from the frame R1 opens the tech panel, whether its landing is due or its panel drawn; 0 with no game. */
int pad_nav_tech_open(const pad_nav_t *nav);

/* 1 while a game runs, from the frame R1 opens the score panel, whether its landing is due or its panel drawn; 0 with no game. */
int pad_nav_score_open(const pad_nav_t *nav);

/* The focused structure icon while the bar is open; NULL while it is closed or once it is no longer a revealed icon. */
entity_t *pad_nav_build_icon(const pad_nav_t *nav);

/* The structure the ui holds in its placing state while placement carries it; NULL otherwise. */
entity_t *pad_nav_placing(const pad_nav_t *nav);

/* NULL when no panel is open or the panel has no item. */
entity_t *pad_nav_first_item(void);

/* NULL when no item lies in that direction. */
entity_t *pad_nav_neighbor(const entity_t *item, int direction);

int pad_nav_is_item(const entity_t *e);

#endif
