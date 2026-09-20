#ifndef PICO8_DUMP_H
#define PICO8_DUMP_H

#include "pico8_mem.h"

#include <stdio.h>

int pico8_dump_screen(const pico8_mem_t *mem, FILE *out);

#endif
