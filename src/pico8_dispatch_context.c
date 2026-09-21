/* See pico8_dispatch_context.h's own top comment for the full rationale.
 * Plain module-level statics, defaulting to NULL (unset) -- mirrors
 * pico8_pi.c's own g_pi_gfx/g_pi_mem precedent exactly, just shared across
 * this chunk's own three new modules instead of re-declared in each. */

#include "pico8_dispatch_context.h"

#include <stddef.h>

static pico8_gfx_t *g_gfx = NULL;
static pico8_mem_t *g_mem = NULL;
static const pico8_input_t *g_input = NULL;

void pico8_dispatch_set_gfx(pico8_gfx_t *gfx) {
    g_gfx = gfx;
}

pico8_gfx_t *pico8_dispatch_gfx(void) {
    return g_gfx;
}

void pico8_dispatch_set_mem(pico8_mem_t *mem) {
    g_mem = mem;
}

pico8_mem_t *pico8_dispatch_mem(void) {
    return g_mem;
}

void pico8_dispatch_set_input(const pico8_input_t *input) {
    g_input = input;
}

const pico8_input_t *pico8_dispatch_input(void) {
    return g_input;
}
