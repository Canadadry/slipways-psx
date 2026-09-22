#ifndef PICO8_KE_H
#define PICO8_KE_H

#include "pico8_class.h"
#include "pico8_value.h"

/* ke's pq is lt itself, which is what makes every ke-descended instance
 * register into hv/cv/cg the moment it is instantiated, through
 * pico8_class_instantiate's pq walk: a class that does not descend from
 * ke is never visited by dc() or dg(). */
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

/* A NULL `self` is a no-op. */
void pico8_ke_ks(pico8_instance_t *self, pico8_value_t me);

#endif
