#ifndef GAME_BACKGROUND_H
#define GAME_BACKGROUND_H

/* Inclusive: the last pixel row and column of the grid background_draw frames. */
#define BACKGROUND_GRID_EDGE 262

#include "game/game_data.h"
#include "p8/gfx.h"
#include "p8/vram.h"
#include "engine/entity_class.h"
#include "engine/vec.h"

/* background_draw returns early on a NULL graphics state or vram,
 * background_construct on NULL memory, background_right_press and
 * background_drag on a NULL struct. The background always answers the hit
 * test, since nothing lies behind it to miss. */

/* `drag_mouse` is a copy of the mouse position the right press or the drag
 * was last given, not that vector. */
typedef struct {
    entity_t header;
    const background_archetype_t *archetype;
    vec_t drag_mouse;
    int has_drag_mouse; /* 0: `drag_mouse` is absent (no right press yet, or a NULL position), read as the zero vector */
    int is_hovered;
} background_t;

void background_init(void);

/* Idempotent, and safe before the first
 * `background_init()`. */
void background_shutdown(void);

/* 1 from background_init() to background_shutdown(), 0 otherwise. */
int background_ready(void);

const entity_class_t *background_class(void);

/* The struct registers itself into the live entities and its messages
 * through entity_register; it lives in the current game-zone generation. */
background_t *background_new(void);

/* The tile choices are `self`'s, or `background_tile_choices_seq` for a
 * NULL `self`. An index outside their 1..15 range -- which extreme walk
 * values can produce, unchecked -- paints tile 0. */
void background_construct(p8_vram_t *memory, background_t *self);

/* Both strokes of the dashed grid are color 1: the second inherits the
 * color of the one right above it, so `1` is passed explicitly to each. */
void background_draw(p8_graphics_t *gfx, p8_vram_t *mem, const background_t *self);

/* Builds a probe when none exists, a wormhole otherwise, and returns it,
 * which ui_handle_button then holds as `left_held`; a class not yet
 * initialized asserts, never returns NULL. Both live in a grab arena: a
 * wormhole gives it back at its death, a probe unless it was placed. */
down_t background_left_press(background_t *self);

/* Copies the mouse position into `drag_mouse` and returns `self`, which
 * becomes `right_held` for as long as the button stays down. */
down_t background_right_press(background_t *self, vec_t *rp);

void background_drag(background_t *self, vec_t *rp);

#endif
