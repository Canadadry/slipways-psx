#ifndef PICO8_DISPATCH_CONTEXT_H
#define PICO8_DISPATCH_CONTEXT_H

#include "pico8_gfx.h"
#include "pico8_input.h"
#include "pico8_mem.h"

/* The current frame's gfx/mem/input, for methods reached through
 * pico8_value_function_t, whose (self, argc, arg0, arg1) shape has no slot
 * for them. The frame loop sets these; a dispatch adapter reads them.
 *
 * All three are borrowed, never owned, and must outlive the frames they
 * are set for. Each getter returns NULL until its setter has run. */

void pico8_dispatch_set_gfx(pico8_gfx_t *gfx);
pico8_gfx_t *pico8_dispatch_gfx(void);

void pico8_dispatch_set_mem(pico8_mem_t *mem);
pico8_mem_t *pico8_dispatch_mem(void);

void pico8_dispatch_set_input(const pico8_input_t *input);
const pico8_input_t *pico8_dispatch_input(void);

#endif
