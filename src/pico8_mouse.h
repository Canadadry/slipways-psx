#ifndef PICO8_MOUSE_H
#define PICO8_MOUSE_H

#include "pico8_input.h"
#include "pico8_ob.h"

/* `qn` is a plain nested table, not an instance: on/jn/hm are read and
 * written on it directly, with no class chain behind them. Both `input`
 * and `qn` are dereferenced, so neither may be NULL, and nothing here is
 * allocated or freed. `oa` is a raw stat(34) bitmask (1 left, 6 right),
 * masked with an ordinary integer `&` since neither side is fractional. */
void pico8_bu(const pico8_input_t *input, pico8_ob_table_t *qn, int oa);

#endif
