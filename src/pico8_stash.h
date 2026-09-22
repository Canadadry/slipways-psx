#ifndef PICO8_STASH_H
#define PICO8_STASH_H

#include "pico8_mem.h"
#include "pico8_ob.h"

/* Returns a newly heap-allocated, NUL-terminated string the caller owns and
 * must free(). */
char *pico8_unstash(const pico8_mem_t *mem, int pd);

/* `mem` must already hold the cart's map data (pico8_load_map_json): the
 * cart parks these two spec strings in otherwise unused map memory. Safe to
 * call more than once -- it frees whatever a previous call built. */
void pico8_stash_init(pico8_mem_t *mem);

/* Safe to call more than once, and before the first pico8_stash_init(). */
void pico8_stash_shutdown(void);

/* Owned by this module (built by pico8_stash_init(), released by
 * pico8_stash_shutdown()); callers must not free them, and both read NULL
 * before the first pico8_stash_init(). */
const pico8_ob_table_t *pico8_r(void);
const pico8_ob_table_t *pico8_iz(void);

#endif
