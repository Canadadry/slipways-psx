#ifndef GAME_MAIN_ECONOMY_H
#define GAME_MAIN_ECONOMY_H

#include "game/economy_formulas.h"
#include "p8/fixed.h"
#include "engine/pointer_list.h"
#include "engine/entity_class.h"

/* hy's archetype, here and not with the others: pico8_fm.h does not include
 * pico8_game_data.h. */
typedef struct {
    p8_fixed_t credits;
    p8_fixed_t era;
    p8_fixed_t year_ticks;
    p8_fixed_t year;
} economy_archetype_t;

/* The economy, one per game, in that game's generation with `invented`;
 * never freed, it dies with its generation. `po`/`fu` are NULL before the first pico8_hy_lj(), then point
 * at `po_own`/`fu_own`, which every pico8_hy_lj() refills in place. `hl` is
 * absent before the first pico8_hy_lj(). */
typedef struct {
    entity_t header;
    const economy_archetype_t *archetype;
    p8_fixed_t credits;
    p8_fixed_t income;
    p8_fixed_t research_points;
    p8_fixed_t era;
    pointer_list_t *invented;
    p8_fixed_t year_ticks;
    p8_fixed_t year;
    resource_map_t *production;
    resource_map_t *demand;
    resource_map_t production_own;
    resource_map_t demand_own;
    p8_fixed_t happiness;
    int has_happiness; /* 0: `happiness` is nil, and is then never read */
    const char *end_status;
    int has_end_status; /* 0: `hl` is nil; 1 with `hl` NULL: `hl` is false */
} economy_t;

/* The provisional `fm`: one static struct, never allocated nor freed, with
 * no method and no writer; `jk`/`nz` are written only by the cheat code, and
 * set back to nil by every `pico8_probe_init()`. */
typedef struct {
    entity_t header;
    p8_fixed_t research_points;
    int has_research_points; /* 0: `jk` is nil, and is then never read */
    p8_fixed_t credits;
    int has_credits; /* 0: `nz` is nil, and is then never read */
} main_economy_placeholder_t;

const entity_class_t *main_economy_placeholder_class(void);

/* `fm`: the provisional, a static struct that has only `jk`/`nz` (written by
 * the cheat code), which `pico8_probe_init()` installs, then, from `eu()` on,
 * a `hy` struct; read it through pico8_fm_hy() and pico8_fm_placeholder().
 * NULL before init and after shutdown, which several callers test for. The
 * setter only repoints: from `_init` to the next `eu()` the slot still holds
 * the quit game's `hy`, alive until the flush that follows that `eu()`. */
entity_t *main_economy(void);
void probe_set_main_economy(entity_t *instance);

/* The `hy` of `fm`, NULL when the slot is empty and on the provisional. */
economy_t *main_economy_typed(void);

/* The provisional `fm`, NULL when the slot is empty or holds a `hy`. */
const main_economy_placeholder_t *main_economy_placeholder(void);

#endif
