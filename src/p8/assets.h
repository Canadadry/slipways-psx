#ifndef PICO8_ASSETS_H
#define PICO8_ASSETS_H

#include "p8/mem.h"

/* Load an exported asset file -- {"width": w, "pixels": [...]} for the
 * sprite sheet, {"width": w, "cells": [...]} for the map -- into `mem`.
 * Returns 1 if at least one value was stored, 0 if the file is missing,
 * unreadable, has no positive "width", or holds no values. Values past the
 * end of their region are dropped by pico8_poke, not reported. */
int pico8_load_gfx_json(pico8_mem_t *mem, const char *path);

int pico8_load_map_json(pico8_mem_t *mem, const char *path);

#endif
