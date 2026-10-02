#ifndef PICO8_KE_H
#define PICO8_KE_H

#include "engine/entity_class.h"

/* Shuts the module down first. */
void pico8_ke_init(void);

/* Idempotent, and safe before the first pico8_ke_init(). */
void pico8_ke_shutdown(void);

/* 1 from pico8_ke_init() to pico8_ke_shutdown(), 0 otherwise. */
int pico8_ke_ready(void);

/* A NULL `self` is a no-op; any other `self` must be a struct. `me` and
 * `t` are written into its header. `me` NULL is nil; a string is
 * borrowed. */
void pico8_ke_ks(entity_t *self, const char *me);

/* Tags `self` as a struct of class `cls` and sets `me` and `t`, its class's
 * archetype's. `me` is borrowed. */
void pico8_ke_struct_init(entity_t *self, const entity_class_t *cls, const char *me,
                          p8_fixed_t t);

/* ke's pq for a struct. A child calls it exactly once, after its own pq,
 * or alone when it has none: the class walk's derived-to-base order and
 * its dedup of an inherited pq. */
void pico8_ke_struct_pq(entity_t *self);

#endif
