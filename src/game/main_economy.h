#ifndef PICO8_FM_H
#define PICO8_FM_H

#include "game/economy_formulas.h"
#include "p8/fixed.h"
#include "engine/pointer_list.h"
#include "engine/entity_class.h"

/* hy's archetype, here and not with the others: pico8_fm.h does not include
 * pico8_game_data.h. */
typedef struct {
    pico8_fixed_t nz;
    pico8_fixed_t ew;
    pico8_fixed_t lo;
    pico8_fixed_t yr;
} pico8_hy_def_t;

/* The economy, one per game, in that game's generation with `invented`;
 * never freed, it dies with its generation. `po`/`fu` are NULL before the first pico8_hy_lj(), then point
 * at `po_own`/`fu_own`, which every pico8_hy_lj() refills in place. `hl` is
 * absent before the first pico8_hy_lj(). */
typedef struct {
    entity_t header;
    const pico8_hy_def_t *def;
    pico8_fixed_t nz;
    pico8_fixed_t ge;
    pico8_fixed_t jk;
    pico8_fixed_t ew;
    pico8_ptrs_t *invented;
    pico8_fixed_t lo;
    pico8_fixed_t yr;
    pico8_qe_map_t *po;
    pico8_qe_map_t *fu;
    pico8_qe_map_t po_own;
    pico8_qe_map_t fu_own;
    pico8_fixed_t happiness;
    int has_happiness; /* 0: `happiness` is nil, and is then never read */
    const char *hl;
    int has_hl; /* 0: `hl` is nil; 1 with `hl` NULL: `hl` is false */
} pico8_hy_t;

/* The provisional `fm`: one static struct, never allocated nor freed, with
 * no method and no writer; `jk`/`nz` are written only by the cheat code, and
 * set back to nil by every `pico8_probe_init()`. */
typedef struct {
    entity_t header;
    pico8_fixed_t jk;
    int has_jk; /* 0: `jk` is nil, and is then never read */
    pico8_fixed_t nz;
    int has_nz; /* 0: `nz` is nil, and is then never read */
} pico8_fm_placeholder_t;

const entity_class_t *pico8_fm_placeholder_struct_class(void);

/* `fm`: the provisional, a static struct that has only `jk`/`nz` (written by
 * the cheat code), which `pico8_probe_init()` installs, then, from `eu()` on,
 * a `hy` struct; read it through pico8_fm_hy() and pico8_fm_placeholder().
 * NULL before init and after shutdown, which several callers test for. The
 * setter only repoints: from `_init` to the next `eu()` the slot still holds
 * the quit game's `hy`, alive until the flush that follows that `eu()`. */
entity_t *pico8_fm(void);
void pico8_probe_set_fm(entity_t *instance);

/* The `hy` of `fm`, NULL when the slot is empty and on the provisional. */
pico8_hy_t *pico8_fm_hy(void);

/* The provisional `fm`, NULL when the slot is empty or holds a `hy`. */
const pico8_fm_placeholder_t *pico8_fm_placeholder(void);

#endif
