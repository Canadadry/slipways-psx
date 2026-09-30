#ifndef PICO8_MOUSE_READER_H
#define PICO8_MOUSE_READER_H

#include "pico8_game_data.h"
#include "pico8_input.h"
#include "pico8_ke.h"
#include "pico8_mem.h"
#include "pico8_mouse.h"
#include "pico8_struct.h"
#include "pico8_vec.h"

/* Throughout this module a NULL struct, `mem` or `input` argument is a
 * safe no-op. */

/* `l` and `r` are not members: every nf reads and bu() mutates the same
 * two button tables, the class's shared state. Nothing frees the struct:
 * TROU-7. */
typedef struct {
    pico8_struct_header_t header;
    const pico8_nf_def_t *def;
    pico8_vec_t *ra;
} pico8_nf_t;

/* Calls pico8_pi_init(), and requires pico8_ke_init() to have run --
 * asserted, not repaired. Starts the shared `l`/`r` tables afresh, as
 * rebuilding the class did in the cart. */
void pico8_nf_init(void);

/* Frees the shared `l`/`r` tables. */
void pico8_nf_shutdown(void);

/* The shared `l`/`r` tables, NULL outside pico8_nf_init()..pico8_nf_shutdown(). */
const pico8_nf_button_t *pico8_nf_l(void);
const pico8_nf_button_t *pico8_nf_r(void);

/* 1 from pico8_nf_init() to pico8_nf_shutdown(), 0 otherwise. */
int pico8_nf_ready(void);
const pico8_struct_class_t *pico8_nf_struct_class(void);

/* The struct enables devkit mouse reporting, registers itself through `lt`,
 * and nothing frees it. */
pico8_nf_t *pico8_nf_new(void);

/* Rebinds `self.ra` to a fresh heap vector holding the mouse position,
 * then runs bu() over the shared `l`/`r` tables, which are mutated in
 * place and never rebound. */
void pico8_nf_ph(const pico8_input_t *input, pico8_nf_t *self);

#endif
