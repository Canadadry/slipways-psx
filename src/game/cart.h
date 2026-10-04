#ifndef GAME_CART_H
#define GAME_CART_H

#include "engine/entity_registry.h"
#include "game/ui_button.h"

/* Runs every module's init, then the game data build (data_init(): the
 * research projects and the tech-gate flags), but only when
 * dispatch_context_mem() is set: without vram they stay unbuilt.
 * A second call, as the tests make, frees what the first built before
 * building it again, so a pointer into it taken in between dangles.
 *
 * The asserts in its body cover the ordering constraints that were found;
 * an adjacency they do not cover is not thereby free. Re-derive before
 * inserting a module init. */
void cart_load(void);

/* Flushes the exit queue; before each queued entity goes, clears what could
 * still point at it: main_ui()'s hovered and previously hovered entities
 * and the end of the slipway it holds, the shown message box, the main
 * score panel's last box, the planets' targeted_by; a quitting difficulty
 * becomes a copy of its fields. Then releases the game-zone generation the
 * last cart_init() or cart_start_game() left, if one is pending. Called by cart_update60()
 * after frame_zone_reset(), by cart_load() and cart_shutdown() after the
 * live entities are abandoned, and by the replay and the tests. */
void cart_flush_exit_queue(void);

/* Quits the game zone, then runs the shutdown of every module cart_load()
 * initialised, in reverse order. Idempotent, and safe before the first
 * cart_load(). */
void cart_shutdown(void);

/* Requires cart_load(). */
void cart_init(void);

/* cart_init() as a button's on_click: the button argument is discarded. */
void cart_init_method(button_t *self);

/* What cart_start_game() reserves for each registry: its peak over the
 * replays of make state, plus 1/8. cart_init() reserves nothing, so the
 * title screen does not add them to the peak of a return to the title. */
extern const entity_registry_reserve_t cart_game_registry_reserve;

/* Starts a game from the clicked difficulty button, which becomes the
 * difficulty (borrowed, not copied); a NULL button gives a NULL difficulty.
 * Requires cart_load(). */
void cart_start_game(button_t *chosen_option);

/* Requires cart_load() and at least one prior cart_init() or
 * cart_start_game(); a NULL main_ui() or empty live entities is a no-op
 * rather than an error. */
void cart_update60(void);

/* Reads vram through dispatch_context_mem(), which must be set:
 * clearing the screen writes through it. */
void cart_draw(void);

#endif
