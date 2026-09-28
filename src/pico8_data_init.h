#ifndef PICO8_DATA_INIT_H
#define PICO8_DATA_INIT_H

#include "pico8_mem.h"
#include "pico8_ob.h"

/* The cart's one-shot top-level data build, in the source's own order. It
 * also runs pico8_research_init() before pico8_research_build_gb(), which
 * needs the hp class to exist. `mem` is not read: fd, dr and nb are built
 * from pico8_map_tables.h. Safe to call more than once -- it frees a
 * previous build first. */
void pico8_data_init(pico8_mem_t *mem);

/* Tears down gb and the hp class too, since pico8_data_init() built them.
 * Safe to call more than once, or before the first pico8_data_init(). */
void pico8_data_shutdown(void);

/* Each table is owned by this module; callers must not free one, and each
 * reads NULL before the first pico8_data_init(). */
const pico8_ob_table_t *pico8_df(void);
const pico8_ob_table_t *pico8_fd(void);
const pico8_ob_table_t *pico8_dr(void);
const pico8_ob_table_t *pico8_nb(void);
const pico8_ob_table_t *pico8_ek(void);

#endif
