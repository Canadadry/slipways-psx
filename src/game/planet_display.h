#ifndef GAME_PLANET_DISPLAY_H
#define GAME_PLANET_DISPLAY_H

#include "game/research.h"
#include "engine/entity_class.h"
#include "p8/gfx.h"
#include "p8/mem.h"
#include "game/planet.h"
#include "engine/vec.h"

/* Throughout this module a NULL instance argument is a safe no-op (Lua nil
 * for the value-returning functions), and an absent or wrong-kind field
 * reads as absent rather than crashing. */

/* Rebinds `self.lq` to a fresh row in a place of its own, and gives the
 * previous row's place back on the spot: nothing reads it any more. */
void planet_refresh(planet_t *self);

/* Builds its rows, their texts and tuples in the frame zone and hands them
 * to `pz()`, whose buttons copy the rows and their texts. */
void planet_inspection_panel(const planet_t *self);

/* Answering "no" for the drag's own origin planet also opens that
 * planet's info panel. */
down_t planet_accept_drop(const planet_t *self, const entity_t *s);

/* Every row it builds carries the captured values of its `my` callback --
 * see `pico8_jo_fe_my_dispatch` in the .c. */
void planet_projects_panel(planet_t *self);

/* Draws, and mutates `self`: `ma()` decays the halo, and the `fo` flash is
 * cleared once drawn. */
void planet_draw(p8_graphics_t *graphics, p8_memory_t *memory, planet_t *self, vec_t screen_pos);

/* In the `hp.cw` branch `self` is marked for removal and the tail of the
 * work lands on a fresh planet the caller never sees -- or on nothing,
 * when `hp.cw` is neither "pm" nor "ga". */
void planet_apply_project(planet_t *self, research_project_t *research_project_instance);

#endif
