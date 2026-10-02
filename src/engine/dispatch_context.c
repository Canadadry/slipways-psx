#include "engine/dispatch_context.h"

#include <stddef.h>

static p8_graphics_t *g_gfx = NULL;
static p8_memory_t *g_mem = NULL;
static const p8_input_t *g_input = NULL;

void pico8_dispatch_set_gfx(p8_graphics_t *gfx) {
    g_gfx = gfx;
}

p8_graphics_t *pico8_dispatch_gfx(void) {
    return g_gfx;
}

void pico8_dispatch_set_mem(p8_memory_t *mem) {
    g_mem = mem;
}

p8_memory_t *pico8_dispatch_mem(void) {
    return g_mem;
}

void pico8_dispatch_set_input(const p8_input_t *input) {
    g_input = input;
}

const p8_input_t *pico8_dispatch_input(void) {
    return g_input;
}
