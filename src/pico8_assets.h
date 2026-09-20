#ifndef PICO8_ASSETS_H
#define PICO8_ASSETS_H

#include "pico8_mem.h"

int pico8_load_gfx_json(pico8_mem_t *mem, const char *path);

int pico8_load_map_json(pico8_mem_t *mem, const char *path);

#endif
