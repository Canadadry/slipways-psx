#ifndef GAME_WORMHOLE_H
#define GAME_WORMHOLE_H

#include "engine/entity_list.h"
#include "p8/fixed.h"
#include "game/game_data.h"
#include "game/targeting.h"
#include "engine/entity_class.h"
#include "engine/vec.h"

/* Throughout this module a NULL struct argument is a safe no-op, and a nil
 * or wrong-kind member reads as its zero/false default rather than
 * crashing. */

/* `mz.jj`: no parent, never an entity, and no writer. */
typedef struct {
    entity_t header;
    const char *status_text;
} wormhole_too_far_target_t;

/* `jj` points at the one `pico8_mz_jj_t`, shared by every `mz` and never
 * modified in place. `od` and `et` live in the probe; `probe` points at
 * `probe_own`: never copy the struct. The struct and `ox` live in one grab
 * arena, which its `rendre` gives back whatever its state. */
typedef struct {
    entity_t header;
    const wormhole_archetype_t *archetype;
    p8_fixed_t radius;
    wormhole_too_far_target_t *too_far_target;
    entity_list_t *anchors;
    const vec_t *pos;
    const vec_t *map_pos;
    entity_t *anchor; /* borrowed, never freed */
    int is_dead;
    targeting_t *targeting;
    targeting_t probe_own;
} wormhole_t;

void wormhole_init(void);

/* Gives back no `mz` nor the `jj` wrapper. Idempotent, and safe before
 * the first `pico8_wormhole_init()`. */
void wormhole_shutdown(void);

/* 1 from pico8_wormhole_init() to pico8_wormhole_shutdown(), 0 otherwise. */
int wormhole_ready(void);
const entity_class_t *wormhole_class(void);

/* Built in a grab arena (`pico8_jo_prise_ouvrir`), so no other grab may be
 * open; the struct registers itself into `hv`/`cv`/`cg` through `lt`. */
wormhole_t *wormhole_new(void);

/* The one `mz.jj`, shared by every `mz`: static, never NULL, never
 * freed, and without a writer, so a write reaching it stops the run. */
wormhole_too_far_target_t *wormhole_too_far_target_wrapper(void);
const entity_class_t *wormhole_too_far_target_class(void);

/* `ox` taken from the construction zone, reserved to the exact count of
 * `fx` entities of `hv`: it never grows. */
void wormhole_construct(wormhole_t *self);

/* Calls `pico8_bv` first, which is what sets `ra`/`p` and a first `et`;
 * the "too far" branch here then overwrites `et`. */
void wormhole_drag(wormhole_t *self, vec_t *mp, vec_t *rp);

void wormhole_update_placing(wormhole_t *self, p8_fixed_t t);

void wormhole_drop(wormhole_t *self);

/* TROU-3 */
void wormhole_update_unfolding(wormhole_t *self, p8_fixed_t t);

/* Reads `self.oi` only; `self` is non-const for signature uniformity. */
p8_fixed_t wormhole_reveal_radius(wormhole_t *self, p8_fixed_t t);

void wormhole_draw_method(entity_t *self, const vec_t *ra);

#endif
