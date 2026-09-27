#ifndef PICO8_MOUSE_READER_H
#define PICO8_MOUSE_READER_H

#include "pico8_input.h"
#include "pico8_ke.h"
#include "pico8_mem.h"
#include "pico8_ob.h"
#include "pico8_struct.h"
#include "pico8_value.h"

/* Throughout this module a NULL struct, `mem` or `input` argument is a
 * safe no-op. */

/* `l` and `r` are not members: every nf reads and bu() mutates the same
 * two button tables, the class's shared state. Nothing frees the struct:
 * TROU-7. */
typedef struct {
    pico8_ke_t ke;
    pico8_value_t ra;
} pico8_nf_t;

/* Calls pico8_pi_init() first: the spec holds `v(0,0)`, so the 'v'
 * constructor must already be registered when it is parsed, and requires
 * pico8_ke_init() to have run -- both are asserted, not repaired. Starts
 * the shared `l`/`r` tables afresh, as rebuilding the class did in the cart. */
void pico8_nf_init(void);

/* Frees the spec and the shared `l`/`r` tables. */
void pico8_nf_shutdown(void);

/* 1 from pico8_nf_init() to pico8_nf_shutdown(), 0 otherwise. */
int pico8_nf_ready(void);

/* `nf`'s spec default `name`, then ke's; 0 when absent, or when not ready. A
 * REF is borrowed from the spec, valid until pico8_nf_shutdown(). */
int pico8_nf_default(const char *name, pico8_value_t *out_value);
const pico8_struct_class_t *pico8_nf_struct_class(void);

/* `overrides` is read, not kept, and must be NULL or empty. The struct
 * enables devkit mouse reporting, registers itself through `lt`, and
 * nothing frees it. */
pico8_nf_t *pico8_nf_new(const pico8_ob_table_t *overrides);

/* Rebinds `self.ra` to a fresh heap vector holding the mouse position,
 * then runs bu() over the shared `l`/`r` tables, which are mutated in
 * place and never rebound. */
void pico8_nf_ph(const pico8_input_t *input, pico8_nf_t *self);

#endif
