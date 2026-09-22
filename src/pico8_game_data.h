#ifndef PICO8_GAME_DATA_H
#define PICO8_GAME_DATA_H

#include "pico8_fixed.h"
#include "pico8_ob.h"

/* Builds/rebuilds every table below. Safe to call more than once -- it
 * frees a previous build first. */
void pico8_game_data_init(void);
void pico8_game_data_shutdown(void);

/* Each table is owned by this module; callers must not free one, and each
 * reads NULL before the first pico8_game_data_init(). */
const pico8_ob_table_t *pico8_es(void);
const pico8_ob_table_t *pico8_ct(void);
const pico8_ob_table_t *pico8_b_(void);
const pico8_ob_table_t *pico8_co(void);
const pico8_ob_table_t *pico8_fb(void);

/* Bare Lua number literals rather than ob() spec fields, hence scalars
 * here and not another table. Constant: no init needed. */
pico8_fixed_t pico8_bn(void);
pico8_fixed_t pico8_bq(void);

#endif
