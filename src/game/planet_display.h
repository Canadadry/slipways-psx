#ifndef GAME_PLANET_DISPLAY_H
#define GAME_PLANET_DISPLAY_H

#include "game/research.h"
#include "engine/entity_class.h"
#include "p8/gfx.h"
#include "p8/mem.h"
#include "game/planet.h"
#include "engine/vec.h"

/* Throughout this module a NULL instance argument is a safe no-op (NULL or
 * 0 for the value-returning functions), and an absent or wrong-kind field
 * reads as absent rather than crashing. */

/* Rebinds the planet's layout row to a fresh row in a slot of its own, and
 * gives the previous row's slot back on the spot: nothing reads it any
 * more. */
void planet_refresh(planet_t *self);

/* Builds its rows, their texts and tuples in the frame zone and hands them
 * to message_box_open_box, whose buttons copy the rows and their texts. */
void planet_inspection_panel(const planet_t *self);

/* Answering "no" for the drag's own origin planet also opens that
 * planet's info panel. */
down_t planet_accept_drop(const planet_t *self, const entity_t *s);

/* Every button it builds carries the values its click callback needs --
 * see planet_projects_panel_on_click_method in planet_display.c. */
void planet_projects_panel(planet_t *self);

/* Draws, and mutates `self`: entity_geom_shake decays the shake, and the
 * `targeted_by` halo is cleared once drawn. */
void planet_draw(p8_graphics_t *graphics, p8_memory_t *memory, planet_t *self, vec_t screen_pos);

/* For a project with a class name, `self` is marked for removal and the
 * tail of the work lands on a fresh planet the caller never sees -- or on
 * nothing, when that name is neither "protostar" nor "burst". */
void planet_apply_project(planet_t *self, research_project_t *research_project_instance);

#endif
