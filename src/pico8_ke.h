#ifndef PICO8_KE_H
#define PICO8_KE_H

#include "pico8_class.h"
#include "pico8_struct.h"
#include "pico8_value.h"

/* First member of every entity struct of a class descended from ke. Only
 * such structs are ever passed to lt, since only ke's pq calls it outside
 * tests. */
typedef struct {
    pico8_struct_header_t header;
    pico8_value_t me;
    pico8_value_t t;
} pico8_ke_t;

/* ke's pq is lt itself, which is what makes every ke-descended entity
 * register into hv/cv/cg the moment it is built -- an instance through
 * pico8_class_instantiate's pq walk, a struct through
 * pico8_ke_struct_pq(): a class that does not descend from ke is never
 * visited by dc() or dg(). */
void pico8_ke_init(void);

/* Frees the class. Every class declared against it keeps a `parent`
 * pointer, so a rebuild must redeclare those too -- pico8_jl()
 * (pico8_bootstrap.c) builds ke once and never again for that reason. */
void pico8_ke_shutdown(void);

/* NULL before pico8_ke_init(), and after pico8_ke_shutdown(). ke's spec
 * sets no `fz`, so it registers under the shared "" slot and
 * pico8_class_lookup() will not find it: this accessor is the only
 * reliable way to reach it. */
pico8_class_t *pico8_ke_class(void);

/* ke's spec default `name`, the end of every ke-descended module's default
 * reader; 0 when absent or before pico8_ke_init(). A REF is borrowed from
 * the spec, valid until pico8_ke_shutdown(). */
int pico8_ke_default(const char *name, pico8_value_t *out_value);

/* A NULL `self` is a no-op; any other `self` must be a struct. `me` and
 * `t` are written into its pico8_ke_t, never through its field writer. */
void pico8_ke_ks(pico8_instance_t *self, pico8_value_t me);

/* Tags `self` as a struct of class `cls` and sets ke's defaults. */
void pico8_ke_struct_init(pico8_ke_t *self, const pico8_struct_class_t *cls);

/* ke's field reader, for a child's reader to fall back on: `me` and `t`,
 * absent when nil. */
int pico8_ke_field(const void *self, const char *name, pico8_value_t *out_value);

/* ke's pq for a struct. A child calls it exactly once, after its own pq,
 * or alone when it has none: the class walk's derived-to-base order and
 * its dedup of an inherited pq. */
void pico8_ke_struct_pq(pico8_ke_t *self);

#endif
