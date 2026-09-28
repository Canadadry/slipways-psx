#ifndef PICO8_KE_H
#define PICO8_KE_H

#include "pico8_struct.h"
#include "pico8_value.h"

/* First member of every entity struct of a class descended from ke. Only
 * such structs are ever passed to lt, since only ke's pq calls it outside
 * tests. */
typedef struct {
    pico8_struct_header_t header;
    const char *me;
    pico8_fixed_t t;
} pico8_ke_t;

/* Shuts the module down first. */
void pico8_ke_init(void);

/* Idempotent, and safe before the first pico8_ke_init(). */
void pico8_ke_shutdown(void);

/* 1 from pico8_ke_init() to pico8_ke_shutdown(), 0 otherwise. */
int pico8_ke_ready(void);

/* A NULL `self` is a no-op; any other `self` must be a struct. `me` and
 * `t` are written into its pico8_ke_t, never through its field writer.
 * `me` is nil or a non-NULL string, borrowed; anything else aborts. */
void pico8_ke_ks(pico8_struct_header_t *self, pico8_value_t me);

/* Tags `self` as a struct of class `cls` and sets ke's defaults,
 * `pico8_ke_start`. */
void pico8_ke_struct_init(pico8_ke_t *self, const pico8_struct_class_t *cls);

/* ke's field reader, for a child's reader to fall back on: `me`, absent
 * when NULL, and `t`, always present. */
int pico8_ke_field(const void *self, const char *name, pico8_value_t *out_value);

/* ke's pq for a struct. A child calls it exactly once, after its own pq,
 * or alone when it has none: the class walk's derived-to-base order and
 * its dedup of an inherited pq. */
void pico8_ke_struct_pq(pico8_ke_t *self);

#endif
