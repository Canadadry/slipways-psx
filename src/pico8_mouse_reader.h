#ifndef PICO8_MOUSE_READER_H
#define PICO8_MOUSE_READER_H

#include "pico8_class.h"
#include "pico8_input.h"
#include "pico8_mem.h"
#include "pico8_ob.h"

/* Throughout this module a NULL instance, `mem` or `input` argument is a
 * safe no-op. */

/* Calls pico8_pi_init() first: the spec holds `v(0,0)`, so the 'v'
 * constructor must already be registered when it is parsed, and requires
 * pico8_ke_init() to have run -- both are asserted, not repaired. `nf`
 * carries its own `pq` on top of the inherited `lt`, so an instance both
 * self-registers and enables devkit mouse reporting. */
void pico8_nf_init(void);
void pico8_nf_shutdown(void);

/* NULL before pico8_nf_init() and after pico8_nf_shutdown(). Not
 * reachable through pico8_class_lookup(). */
pico8_class_t *pico8_nf_class(void);

/* `overrides` is copied, not kept, and nothing ever frees the instance. */
pico8_instance_t *pico8_nf_new(const pico8_ob_table_t *overrides);

/* `self` is unused, kept so the signature matches pico8_class_pq_fn. */
void pico8_nf_pq(pico8_mem_t *mem, pico8_instance_t *self);

/* Rebinds `self.ra` to a fresh heap vector holding the mouse position,
 * then runs bu() over `self.l`/`self.r`, which are mutated in place and
 * never rebound. */
void pico8_nf_ph(const pico8_input_t *input, pico8_instance_t *self);

#endif
