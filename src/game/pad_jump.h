#ifndef GAME_PAD_JUMP_H
#define GAME_PAD_JUMP_H

#include "engine/entity_class.h"
#include "engine/entity_id_type.h"
#include "engine/entity_list.h"

#define PAD_JUMP_REPEAT_DELAY 18
#define PAD_JUMP_REPEAT_PERIOD 8
#define PAD_JUMP_SCROLL_MARGIN 20
/* 20 plus the bottom HUD row, which starts at P8_SCREEN_HEIGHT - 11. */
#define PAD_JUMP_SCROLL_MARGIN_BOTTOM 31
#define PAD_JUMP_SCROLL_SPEED 8

/* A game stays unseeded until its first hovered object: pad_jump_reseed then takes the first colony, later the
 * nearest. */
typedef struct {
    entity_id_t hovered;
    int seeded;
    int repeat_direction;
    int repeat_frames;
} pad_jump_t;

void pad_jump_init(pad_jump_t *jump);

/* NULL hovers nothing. Seeds nothing. */
void pad_jump_hover(pad_jump_t *jump, const entity_t *e);
/* The hovered object while it is a pad_target_is_candidate; NULL otherwise. */
entity_t *pad_jump_hovered(const pad_jump_t *jump);
/* Hovers e, and seeds when e is not NULL. */
void pad_jump_hover_seeded(pad_jump_t *jump, const entity_t *e);
/* Hovers, and returns, pad_jump_seed's pick while unseeded, else pad_jump_nearest's; seeds when it is not NULL. */
entity_t *pad_jump_reseed(pad_jump_t *jump, const entity_list_t *candidates, int centre_x, int centre_y);

/* pad_buttons are the bits held this frame, pressed the bits newly down. Returns the single direction bit to jump
 * this frame, or 0. */
int pad_jump_repeat_step(pad_jump_t *jump, int pad_buttons, int pressed);
void pad_jump_repeat_clear(pad_jump_t *jump);

/* 1 when the screen point lies in the inner rectangle, bounds included. */
int pad_jump_inside(int x, int y);
/* The camera shift for a target at screen point (x, y): per axis its distance outside the inner rectangle, clamped
 * to PAD_JUMP_SCROLL_SPEED. */
void pad_jump_scroll(int x, int y, int *dx, int *dy);

/* floor(map_pos * GALAXY_MAP_SCALE) per axis: never pos, so a bobbing structure's point stays still. */
void pad_jump_world_point(const entity_t *e, int *x, int *y);
/* The candidate whose world point is nearest the world pixel (x, y); the earlier in the list at equal distance; NULL
 * for a NULL or empty list. Returns a pointer into candidates. */
entity_t *pad_jump_nearest(int x, int y, const entity_list_t *candidates);
/* The first colony in list order (class planet_class(), state SETTLED), else the candidate nearest the world pixel
 * (centre_x, centre_y), else NULL. Returns a pointer into candidates. */
entity_t *pad_jump_seed(const entity_list_t *candidates, int centre_x, int centre_y);
/* The entries of nodes that pass pad_target_is_candidate, in order, in a list grown in zone, which owns its storage:
 * with frame_zone() it lives until the frame ends and the caller frees nothing. Empty for a NULL nodes. */
entity_list_t pad_jump_candidates(const allocator_t *zone, const entity_list_t *nodes);

#endif
