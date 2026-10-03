#ifndef GAME_HELP_H
#define GAME_HELP_H

#include "p8/gfx.h"
#include "p8/vram.h"
#include "game/probe.h"
#include "engine/entity_class.h"
#include "game/ui_button.h"
#include "engine/vec.h"

/* help_screen_construct() returns at once on a NULL instance;
 * help_screen_draw() skips the rows without graphics or memory but draws
 * the particles through them whatever they are; help_screen_draw_method()
 * reads a NULL position as (0, 0). */

/* help_init() asserts that the draw functions and the probe class are up,
 * calls help_shutdown(), then raises the ready flag: neither allocates, so
 * both are idempotent. Both also reset the start-game bridge to the
 * placeholder, so the real one must be installed after every init. */
void help_init(void);
void help_shutdown(void);

/* 1 from help_init() to help_shutdown(), 0 otherwise. */
int help_screen_ready(void);
const entity_class_t *help_screen_class(void);

/* GAP-8 */
/* The help screen's class carries the probe's tags, so entity_register()
 * files it under the probe and targeting tags too; it lives in the current
 * game-zone generation. Its position points at help_screen_archetype's,
 * shared by every help screen. Asserts when called before help_init(). */
probe_t *help_screen_new(void);

/* help_start_game_field_value() never returns NULL: an inert placeholder
 * until help_set_start_game() installs the real start, and again after
 * help_set_start_game(NULL). */
button_fn_t help_start_game_field_value(void);
void help_set_start_game(button_fn_t fn);

/* Opens the difficulty box: one button per row of difficulty_options_table,
 * built on a local copy of its keys, with the start-game bridge current at
 * call time as on_click; buttons built before a help_set_start_game() keep
 * what they were given. A NULL instance returns at once. */
void help_screen_construct(probe_t *self);

void help_screen_draw(p8_graphics_t *gfx, p8_vram_t *mem, probe_t *self, vec_t p);

void help_screen_draw_method(entity_t *self, const vec_t *screen_pos);

#endif
